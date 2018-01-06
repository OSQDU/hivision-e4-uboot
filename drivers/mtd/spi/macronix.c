/*
 * Copyright 2009(C) Marvell International Ltd. and its affiliates
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Based on drivers/mtd/spi/stmicro.c
 *
 * Copyright 2008, Network Appliance Inc.
 * Jason McMullan <mcmullan@netapp.com>
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* MX25xx-specific commands */
#define CMD_MX25XX_WREN		0x06	/* Write Enable */
#define CMD_MX25XX_WRDI		0x04	/* Write Disable */
#define CMD_MX25XX_RDSR		0x05	/* Read Status Register */
#define CMD_MX25XX_RDCR     0x15    /* Read Status Register */
#define CMD_MX25XX_WRSR		0x01	/* Write Status Register */
#define CMD_MX25XX_READ		0x03	/* Read Data Bytes */
#define CMD_MX25XX_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_MX25XX_PP		0x02	/* Page Program */
#define CMD_MX25XX_SE		0x20	/* Sector Erase */
#define CMD_MX25XX_BE		0xD8	/* Block Erase */
#define CMD_MX25XX_CE		0xc7	/* Chip Erase */
#define CMD_MX25XX_DP		0xb9	/* Deep Power-down */
#define CMD_MX25XX_RES		0xab	/* Release from DP, and Read Signature */

#define MACRONIX_SR_WIP		(1 << 0)	/* Write-in-Progress */
#define MACRONIX_SR_WEL     (1 << 1)
#define MACRONIX_SR_BP0     (1 << 2)
#define MACRONIX_SR_BP1     (1 << 3)
#define MACRONIX_SR_BP2     (1 << 4)
#define MACRONIX_SR_BP3     (1 << 5)
#define MACRONIX_SR_QE      (1 << 6)
#define MACRONIX_SR_SRWD    (1 << 7)

#define MACRONIX_CR_ODS0    (1 << 0)    /* Write-in-Progress */
#define MACRONIX_CR_ODS1    (1 << 1)
#define MACRONIX_CR_ODS2    (1 << 2)
#define MACRONIX_CR_TB      (1 << 3)
#define MACRONIX_CR_DC0     (1 << 6)
#define MACRONIX_CR_DC1     (1 << 7)

#define SIZE_4K 		4096
#define SIZE_16M		(16*1024*1024)
struct macronix_spi_flash_params {
	u16 idcode;
	u16 page_size;
	u16 pages_per_sector;
	u16 sectors_per_block;
	u16 nr_blocks;
	const char *name;
	u8 four_byte; /* 4 bytes mod */
};

struct macronix_spi_flash {
	struct spi_flash flash;
	const struct macronix_spi_flash_params *params;
};

static inline struct macronix_spi_flash *to_macronix_spi_flash(
                struct spi_flash *flash)
{
	return container_of(flash, struct macronix_spi_flash, flash);
}

static const struct macronix_spi_flash_params macronix_spi_flash_table[] = {
	{
		.idcode = 0x2015,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 32,
		.name = "MX25L1605D",
	},
	{
		.idcode = 0x2016,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 64,
		.name = "MX25L3205D",
	},
	{
		.idcode = 0x2017,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 128,
		.name = "MX25L6405D",
	},
	{
		.idcode = 0x2018,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 256,
		.name = "MX25L12805D",
	},
	{
		.idcode = 0x2618,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 256,
		.name = "MX25L12855E",
	},
	{
		.idcode = 0x2019,
		.page_size = 256,
		.pages_per_sector = 16,
		.sectors_per_block = 16,
		.nr_blocks = 512,
		.name = "MX25L25635E",
	},

};

static int spi_flash_get_byte_mode(struct spi_flash *flash)
{
	struct macronix_spi_flash *mcx = to_macronix_spi_flash(flash);
	return mcx->params->four_byte;
}

static int macronix_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 status;
	u8 cmd = CMD_MX25XX_RDSR;

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: Failed to send command %02x: %d\n", cmd, ret);
		return ret;
	}

	timebase = get_timer(0);
	do {
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if ((status & MACRONIX_SR_WIP) == 0)
			break;

	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & MACRONIX_SR_WIP) == 0)
		return 0;

	/* Timed out */
	return -1;
}

static void macronix_build_address(struct macronix_spi_flash *mcx, u8 *cmd,
                u32 offset)
{
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;
	u8 cmd_index, four_byte = mcx->params->four_byte;

	/*
	 * The "extra" space per page is the power-of-two page size
	 * divided by 32.
	 */
	//page_shift = stm->params->l2_page_size;
	page_shift = 8; //means page size = 2 pow 8;
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

static int macronix_read_fast(struct spi_flash *flash, u32 offset, size_t len,
                void *buf)
{
	struct macronix_spi_flash *stm = to_macronix_spi_flash(flash);
	u8 cmd[6], size_cmd;

	cmd[0] = CMD_READ_ARRAY_FAST;
	macronix_build_address(stm, cmd + 1, offset);
	if (stm->params->four_byte)
		size_cmd = 6;
	else
		size_cmd = 5;
	cmd[size_cmd - 1] = 0x00;

	return spi_flash_read_common(flash, cmd, size_cmd, buf, len);
}

static int macronix_write(struct spi_flash *flash, u32 offset, size_t len,
                const void *buf)
{
	struct macronix_spi_flash *stm = to_macronix_spi_flash(flash);
	unsigned long page_addr;
	unsigned long byte_addr;
	unsigned long page_size;
	unsigned int page_shift;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[5];
	u8 cmd_index, four_byte;

	//page_shift = stm->params->l2_page_size;
	page_shift = 8;
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
		cmd[cmd_index++] = CMD_MX25XX_PP;
		if (four_byte)
			cmd[cmd_index++] = page_addr >> (16);
		cmd[cmd_index++] = page_addr >> (16 - page_shift);
		cmd[cmd_index++] = page_addr << (page_shift - 8)
		                | (byte_addr >> 8);
		cmd[cmd_index++] = byte_addr;
		debug(
		                "PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
		                buf + actual, cmd[0], cmd[1], cmd[2], cmd[3],
		                chunk_len);

		ret = spi_flash_cmd(flash->spi, CMD_MX25XX_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, (four_byte ? 5 : 4),
		                buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: macronix Page Program failed\n");
			goto out;
		}

		ret = macronix_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret < 0) {
			debug("SF: macronix page programming timed out\n");
			goto out;
		}
		page_addr++;
		byte_addr = 0;
	}
	printf(" done!\n");

	debug("SF: macronix: Successfully programmed %u bytes @ 0x%x\n", len,
	                offset);
	ret = 0;

	out: spi_release_bus(flash->spi);
	return ret;
}

int macronix_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	struct macronix_spi_flash *stm = to_macronix_spi_flash(flash);

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

	//page_shift = stm->params->l2_page_size;
	page_shift = 8;
	block_size = (1 << page_shift) * stm->params->pages_per_sector
	                * stm->params->sectors_per_block;

	if (offset % block_size || len % block_size) {
		debug("SF: Erase offset/length not multiple of block size\n");
		return -1;
	}

	len /= block_size;
	cmd[0] = CMD_MX25XX_BE;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	four_byte = spi_flash_get_byte_mode(flash);
	for (actual = 0; actual < len; actual++) {
		macronix_build_address(stm, &cmd[1],
		                offset + actual * block_size);
		if (stm->params->four_byte)
			printf("Erase: %02x %02x %02x %02x %02x\n", cmd[0],
			                cmd[1], cmd[2], cmd[3], cmd[4]);
		else
			printf("Erase: %02x %02x %02x %02x\n", cmd[0], cmd[1],
			                cmd[2], cmd[3]);

		ret = spi_flash_cmd(flash->spi, CMD_MX25XX_WREN, NULL, 0);
		if (ret < 0) {
			debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, (four_byte ? 5 : 4),
		                NULL, 0);
		if (ret < 0) {
			debug("SF: macronix block erase failed\n");
			goto out;
		}

		ret = macronix_wait_ready(flash,
		                SPI_FLASH_SECTOR_ERASE_TIMEOUT);
		if (ret < 0) {
			debug("SF: macronix block erase timed out\n");
			goto out;
		}
	}

	debug("SF: macronix: Successfully erased %u bytes @ 0x%x\n",
	                len * block_size, offset);
	ret = 0;

	out: spi_release_bus(flash->spi);
	return ret;
}

int macronix_protect(struct spi_flash *flash, int on)
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
	ret = spi_flash_cmd(flash->spi, CMD_MX25XX_RDSR, &status1, 1);
	if (ret < 0) {
		debug("SF: Enabling Write failed\n");
		goto out;
	}

	if (status1 & MACRONIX_SR_SRWD) {
		printf(
		                "flash status register is protected, please set SRWD to low in order to unprotected it.\n");
		goto out;
	}

	status2 = 0;

	/* read status 2 */
	ret = spi_flash_cmd(flash->spi, CMD_MX25XX_RDCR, &status2, 1);
	if (ret < 0) {
		debug("SF: Enabling Write failed\n");
		goto out;
	}

	printf("status register1: 0x%x, status register2: 0x%x\n", status1,
	                status2);

	if (on) {
		status1 |= (MACRONIX_SR_BP2);
		status1 &= (~(MACRONIX_SR_BP0 | MACRONIX_SR_BP1
		                | MACRONIX_SR_BP3));
		status2 |= MACRONIX_CR_TB;
	} else {
		status1 = 0;
		status2 = 0;
	}

	/* write status */
	ret = spi_flash_cmd(flash->spi, CMD_MX25XX_WREN, NULL, 0);
	if (ret < 0) {
		debug("SF: Enabling Write failed\n");
		goto out;
	}

	cmd[0] = CMD_MX25XX_WRSR;
	cmd[1] = status1;
	cmd[2] = status2;
	ret = spi_xfer(spi, 24, &cmd[0], NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: Failed to send command %02x: %d\n", cmd, ret);
		goto out;
	}

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	out: spi_release_bus(flash->spi);
	return ret;
}

void spi_flash_reset_4byte_mx(struct spi_slave *spi, u8 four_byte)
{
	u8 idcode[5];
	if (four_byte) {
		spi_flash_cmd(spi, 0x2b, idcode, 1);
		if (idcode[0] & 0x04)
			debug("flash already in 4bytes mode\n");
		else {
			spi_flash_cmd(spi, 0xb7, NULL, 0);
			debug("flash now in the 4bytes mode\n");
		}
	} else {
		spi_flash_cmd(spi, 0x2b, idcode, 1);
		if (idcode[0] & 0x04) {
			debug("change flash to 3bit mode\n");
			spi_flash_cmd(spi, 0xe9, NULL, 0);
			spi_flash_cmd(spi, 0x06, NULL, 0);
			idcode[0] = 0xc5;
			idcode[1] = 0x00;
			spi_flash_cmd_write(spi, idcode, 2, NULL, 0);
		} else
			debug("flash now in the 3bit mode\n");
	}
}

static int inline spi_flash_checksize(
                const struct macronix_spi_flash_params *params)
{
	return params->nr_blocks * params->sectors_per_block * SIZE_4K;
}

static struct macronix_spi_flash *reset_spi;

static void spi_flash_soft_reset(void)
{
	struct macronix_spi_flash *stm = reset_spi;
	if (spi_flash_checksize(stm->params) > SIZE_16M)
		spi_flash_reset_4byte_mx(stm->flash.spi, 0);
}

struct spi_flash *spi_flash_probe_macronix(struct spi_slave *spi, u8 *idcode)
{
	struct macronix_spi_flash_params *params;
	unsigned long page_size;
	struct macronix_spi_flash *stm;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(macronix_spi_flash_table); i++) {
		params = &macronix_spi_flash_table[i];
		if (params->idcode == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(macronix_spi_flash_table)) {
		debug("SF: Unsupported MX ID %02x%02x\n", idcode[1], idcode[2]);
		return NULL;
	}

	stm = malloc(sizeof(struct macronix_spi_flash));
	if (!stm) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	if (spi_flash_checksize(params) > SIZE_16M) {
		spi_flash_reset_4byte_mx(spi, 1);
		params->four_byte = 1;
	} else
		params->four_byte = 0;

	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;

	/* Assuming power-of-two page size initially. */
	page_size = params->page_size;

	stm->flash.write = macronix_write;
	stm->flash.erase = macronix_erase;
	stm->flash.read = macronix_read_fast;
	stm->flash.protect = macronix_protect;
	stm->flash.size = (u32) page_size * (u32) params->pages_per_sector
	                * (u32) params->sectors_per_block
	                * (u32) params->nr_blocks;
	//++
	stm->flash.sector_size = (u32) page_size
	                * (u32) params->pages_per_sector;

	debug("SF: Detected %s with page size %u, total %u bytes\n",
	                params->name, page_size, stm->flash.size);

	reset_spi = stm;

	add_soft_reset((void*) spi_flash_soft_reset);

	return &stm->flash;
}
