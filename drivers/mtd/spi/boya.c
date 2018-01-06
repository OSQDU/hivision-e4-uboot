/*
 * boya.c
 *
 *  Created on: Apr 25, 2016
 *      Author: duobao
 */


/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* M25Pxx-specific commands */
#define CMD_W25_WREN		0x06	/* Write Enable */
#define CMD_W25_WRDI		0x04	/* Write Disable */
#define CMD_W25_RDSR		0x05	/* Read Status Register */
#define CMD_W25_WRSR		0x01	/* Write Status Register */
#define CMD_W25_READ		0x03	/* Read Data Bytes */
#define CMD_W25_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_W25_PP		0x02	/* Page Program */
#define CMD_W25_SE		0x20	/* Sector (4K) Erase */
#define CMD_W25_BE		0xd8	/* Block (64K) Erase */
#define CMD_W25_CE		0xc7	/* Chip Erase */
#define CMD_W25_DP		0xb9	/* Deep Power-down */
#define CMD_W25_RES		0xab	/* Release from DP, and Read Signature */

#define CMD_W25_RDSR2        0x35    /* Read Status Register 2 */
#define CMD_W25_WRENV        0x50    /* Read Status Register 2 */

#define BOYA_ID_25Q16A		0x4015



#define WINBOND_SR_WIP		(1 << 0)	/* Write-in-Progress */
#define WINBOND_SR_WEL      (1 << 1)
#define WINBOND_SR_BP0      (1 << 2)
#define WINBOND_SR_BP1      (1 << 3)
#define WINBOND_SR_BP2      (1 << 4)
#define WINBOND_SR_TB       (1 << 5)
#define WINBOND_SR_SEC      (1 << 6)
#define WINBOND_SR_SRP0     (1 << 7)
#define WINBOND_SR_SRP1     (1 << 8)


struct winbond_spi_flash_params {
	uint16_t	id;
	/* Log2 of page size in power-of-two mode */
	uint8_t		l2_page_size;
	uint16_t	pages_per_sector;
	uint16_t	sectors_per_block;
	uint16_t	nr_blocks;
	const char	*name;

	u8			four_byte; /* 4 bytes mod */
};

/* spi_flash needs to be first so upper layers can free() it */
struct winbond_spi_flash {
	struct spi_flash flash;
	const struct winbond_spi_flash_params *params;
};

static inline struct winbond_spi_flash *
to_winbond_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct winbond_spi_flash, flash);
}

#define SIZE_4K 		4096
#define SIZE_16M		(16*1024*1024)

static struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.id			= BOYA_ID_25Q16A,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 32,
		.name			= "BY25Q16A",
	},



};

static int spi_flash_get_byte_mode(struct spi_flash *flash)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	return stm->params->four_byte;
}

static int winbond_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 status;
	u8 cmd[4] = { CMD_W25_RDSR, 0xff, 0xff, 0xff };

	ret = spi_xfer(spi, 32, &cmd[0], NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: Failed to send command %02x: %d\n", cmd, ret);
		return ret;
	}

	timebase = get_timer(0);
	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret) {
			debug("SF: Failed to get status for cmd %02x: %d\n", cmd, ret);
			return -1;
		}

		if ((status & WINBOND_SR_WIP) == 0)
			break;

	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & 0x03) == 0)
		return 0;

	debug("SF: Timed out on command %02x: %d\n", cmd, ret);
	/* Timed out */
	return -1;
}

/*
 * Assemble the address part of a command for Winbond devices in
 * non-power-of-two page size mode.
 */
static void winbond_build_address(struct winbond_spi_flash *stm, u8 *cmd,
				  u32 offset)
{
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;
	u8 cmd_index, four_byte = stm->params->four_byte;

	/*
	 * The "extra" space per page is the power-of-two page size
	 * divided by 32.
	 */
	page_shift = stm->params->l2_page_size;
	page_size = (1 << page_shift);
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	cmd_index = 0;
	if (four_byte)
		cmd[cmd_index++] = page_addr >> (16);
	cmd[cmd_index++] = page_addr >> (16 - page_shift);
	cmd[cmd_index++] = page_addr << (page_shift - 8) | (byte_addr >> 8);
	cmd[cmd_index++] = byte_addr;
}

static int winbond_read_fast(struct spi_flash *flash,
			     u32 offset, size_t len, void *buf)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	u8 cmd[6], size_cmd;

	cmd[0] = CMD_READ_ARRAY_FAST;
	winbond_build_address(stm, cmd + 1, offset);
	if (stm->params->four_byte)
		size_cmd = 6;
	else
		size_cmd = 5;
	cmd[size_cmd - 1] = 0x00;

	return spi_flash_read_common(flash, cmd, size_cmd, buf, len);
}

static int winbond_write(struct spi_flash *flash,
			 u32 offset, size_t len, const void *buf)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[5];
	u8 cmd_index, four_byte;

	page_shift = stm->params->l2_page_size;
	page_size = (1 << page_shift);
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	printf("Write: ");
	four_byte = spi_flash_get_byte_mode(flash);
	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);
		if (0 == actual % (chunk_len << 8))
			printf(".");

		cmd_index = 0;
		cmd[cmd_index++] = CMD_W25_PP;
		if (four_byte)
			cmd[cmd_index++] = page_addr >> (16);
		cmd[cmd_index++] = page_addr >> (16 - page_shift);
		cmd[cmd_index++] = page_addr << (page_shift - 8) | (byte_addr >> 8);
		cmd[cmd_index++] = byte_addr;
		debug("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
		      buf + actual,
		      cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, (four_byte ? 5 : 4),
					  buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: Winbond Page Program failed\n");
			goto out;
		}

		ret = winbond_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: Winbond page programming timed out\n");
			goto out;
		}
		page_addr++;
		byte_addr = 0;
	}
	printf(" done!\n");

	debug("SF: Winbond: Successfully programmed %u bytes @ 0x%x\n",
	      len, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int winbond_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);

	unsigned long block_size;
	unsigned int page_shift;
	size_t actual;
	int ret;
	u8 cmd[5];
	u8 four_byte;

	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */

	page_shift = stm->params->l2_page_size;
	block_size = (1 << page_shift) * stm->params->pages_per_sector *
		     stm->params->sectors_per_block;

	if (offset % block_size || len % block_size) {
		debug("SF: Erase offset/length not multiple of block size\n");
		return -1;
	}

	len /= block_size;
	cmd[0] = CMD_W25_BE;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	four_byte = spi_flash_get_byte_mode(flash);
	for (actual = 0; actual < len; actual++) {
		winbond_build_address(stm, &cmd[1], offset + actual * block_size);
		printf("Erase: %02x %02x %02x %02x\n",
		       cmd[0], cmd[1], cmd[2], cmd[3]);

		ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, (four_byte ? 5 : 4), NULL, 0);
		if (ret < 0) {
			debug("SF: Winbond block erase failed\n");
			goto out;
		}

		ret = winbond_wait_ready(flash, SPI_FLASH_SECTOR_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: Winbond block erase timed out\n");
			goto out;
		}
	}

	debug("SF: Winbond: Successfully erased %u bytes @ 0x%x\n",
	      len * block_size, offset);
	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int winbond_protect(struct spi_flash *flash, int on)
{
    struct spi_slave *spi = flash->spi;
    int ret;
    u8 status1, status2;
    u8 cmd[4] = { 0xff, 0xff, 0xff, 0xff };

    ret = spi_claim_bus(flash->spi);
    if (ret) {
        debug("SF: Unable to claim SPI bus\n");
        return ret;
    }

    /* read status 1 */
    ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR, &status1, 1);
    if (ret < 0) {
        debug("SF: Enabling Write failed\n");
        goto out;
    }

    if(status1 & (WINBOND_SR_SRP0 | WINBOND_SR_SRP1))
    {
        printf("flash is hardware protected, please set WP to high in order to unprotected it.\n");
        goto out;
    }

    /* read status 2 */
    ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR2, &status2, 1);
    if (ret < 0) {
        debug("SF: Enabling Write failed\n");
        goto out;
    }

    printf("status register1: 0x%x, status register2: 0x%x\n", status1, status2);

    if(on)
    {
        status1 |= (WINBOND_SR_BP0 | WINBOND_SR_TB);
        status1 &= (~(WINBOND_SR_BP1 | WINBOND_SR_BP2 | WINBOND_SR_SEC));
    }
    else
    {
        status1 = 0;
    }

    /* write status */
    ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
    if (ret < 0) {
        debug("SF: Enabling Write failed\n");
        goto out;
    }

    cmd[0] = CMD_W25_WRSR;
    cmd[1] = status1;
    ret = spi_xfer(spi, 16, &cmd[0], NULL, SPI_XFER_BEGIN);
    if (ret) {
        debug("SF: Failed to send command %02x: %d\n", cmd, ret);
        goto out;
    }

    spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);


out:
    spi_release_bus(flash->spi);
    return ret;
}




struct spi_flash *spi_flash_probe_boya(struct spi_slave *spi, u8 *idcode)
{
	struct winbond_spi_flash_params *params;
	unsigned long page_size;
	struct winbond_spi_flash *stm;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		debug("SF: Unsupported boya ID %02x%02x\n",
		      idcode[1], idcode[2]);
		return NULL;
	}

	stm = malloc(sizeof(struct winbond_spi_flash));
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}


	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;

	/* Assuming power-of-two page size initially. */
	page_size = 1 << params->l2_page_size;

	stm->flash.write = winbond_write;
	stm->flash.erase = winbond_erase;
	stm->flash.read = winbond_read_fast;
	stm->flash.protect = winbond_protect;
	stm->flash.size = (u32)page_size * (u32)params->pages_per_sector
			  * (u32)params->sectors_per_block
			  * (u32)params->nr_blocks;
	//++
	stm->flash.sector_size = (u32)page_size * (u32)params->pages_per_sector;

	debug("SF: Detected %s with page size %u, total %u bytes\n",
	      params->name, page_size, stm->flash.size);





	return &stm->flash;
}


