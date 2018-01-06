
#if 0
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/mtd/spi-nand.h>
#endif

#include <common.h>
#include <malloc.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/compat.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <asm-generic/errno.h>

#include <spi.h>
#include "spi-nand.h"
#include "spi-nand-ids.h"

#ifdef SPINAND_DEBUG
# define fh_dev_debug(dev, fmt, args...)  printk(fmt, ##args)
#else
# define fh_dev_debug(...)
#endif

static int spi_nand_read_id(struct spi_nand_chip *chip, u8 *buf)
{
	struct spi_slave *spi = chip->spi;
	struct spi_nand_cmd cmd = { 0 };

	cmd.cmd = SPINAND_CMD_READ_ID;
	cmd.n_rx = SPINAND_MAX_ID_LEN;
	cmd.rx_buf = buf;

	return spi_nand_send_cmd(spi, &cmd);
}

static void spi_nand_ecc_status(struct spi_nand_chip *chip, unsigned int status,
				      unsigned int *corrected, unsigned int *ecc_error)
{
	unsigned int ecc_status = (status >> SPI_NAND_ECC_SHIFT) &
				  chip->ecc_mask;

	*ecc_error = (ecc_status >= chip->ecc_uncorr);
	if (*ecc_error == 0)
		*corrected = ecc_status;
}

static int spi_nand_device_probe(struct spi_slave *spi, struct mtd_info* mtd)
{
	struct spi_nand_chip *chip;
	int ret;
	/*	struct mtd_part_parser_data ppdata;*/
/*
	struct mtd_partition *parts = NULL;
	int nr_parts = 0;
	int ret, i;
	struct flash_platform_data	*data;
*/

	fh_dev_debug(&spi->dev, "%s with spi%d:%d \n", __func__, spi->bus, spi->cs);

	chip = kmalloc(sizeof(struct spi_nand_chip), GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto err1;
	}
	chip->spi = spi;

	mtd->priv = chip;
	chip->mtd = mtd;
	/*
	 * read ID command format might be different for different manufactory
	 * such as, Micron SPI NAND need extra one dummy byte after perform
	 * read ID command but Giga device don't need.
	 *
	 * So, specify manufactory of device in device tree is obligatory
	 */
/*	variant = spi_get_device_id(spi)->driver_data;
	switch (variant) {
	case SPI_NAND_MT29F:
		chip->read_id = spi_nand_mt29f_read_id;
		break;
	case SPI_NAND_GD5F:
		chip->read_id = spi_nand_gd5f_read_id;
		break;
	default:
		dev_err(&spi->dev, "unknown device, id %d\n", variant);
		ret = -ENODEV;
		goto err3;
	}*/

	chip->read_id = spi_nand_read_id;
	ret = spi_nand_scan_ident(mtd);
	if (ret){
		ret = -ENODEV;
		goto err3;
	}
        chip->get_ecc_status = spi_nand_ecc_status;

        ret = spi_nand_scan_tail(mtd);
        if (ret) {
                fh_dev_debug(&spi->dev, "goto err4 %s\n", __func__);
                goto err4;
        }

	spi_nand_scan_tail_release(mtd);
	fh_dev_debug(&spi->dev, "Leave %s\n", __func__);
	return 0;

err4:
	spi_nand_scan_ident_release(mtd);
err3:
	memset(mtd, 0, sizeof(struct mtd_info));
//err2:
	free(chip);
err1:
	return ret;
}

int spi_nand_device_remove(struct mtd_info *mtd)
{
	struct spi_nand_chip *chip = (struct spi_nand_chip *)mtd->priv;

	spi_nand_release(mtd);
	kfree(mtd);
	kfree(chip);

	return 0;
}

#ifndef CONFIG_SPI_NAND_BUS
# define CONFIG_SPI_NAND_BUS	1
#endif
#ifndef CONFIG_SPI_NAND_CS
# define CONFIG_SPI_NAND_CS	0
#endif
#ifndef CONFIG_SPI_NAND_HZ
# define CONFIG_SPI_NAND_HZ	CONFIG_SF_DEFAULT_SPEED
#endif
#ifndef CONFIG_SPI_NAND_MODE
# define CONFIG_SPI_NAND_MODE	SPI_MODE_3
#endif

#include <nand.h>
int nand_curr_device = -1;
#ifndef CONFIG_SYS_MAX_NAND_DEVICE
# define CONFIG_SYS_MAX_NAND_DEVICE 2
#endif
nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];

int spi_nand_init(void)
{
	struct spi_slave *spi;
	int index;
	int ret;

	spi = spi_setup_slave(CONFIG_SPI_NAND_BUS, CONFIG_SPI_NAND_CS,
			CONFIG_SPI_NAND_HZ, CONFIG_SPI_NAND_MODE);
	if (!spi) {
		debug("SPI NAND: Failed to set up slave\n");
		return -1;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SPI NAND: Failed to claim SPI bus: %d\n", ret);
		goto err_claim_bus;
	}

	ret = spi_nand_device_probe(spi, nand_info);
	if (0 == ret){
		nand_curr_device = 0;
		return 0;
	}

err_claim_bus:
	spi_free_slave(spi);
	return -1;
}
