/*
 *  (c) 2015 fullhan.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _FHMCI_H_
#define _FHMCI_H_

#include <mmc.h>

#define POWER_ON          1
#define POWER_OFF         0

#define CARD_UNPLUGED     1
#define CARD_PLUGED       0

#define ENABLE            1
#define DISABLE           0

#define FH_MCI_DETECT_TIMEOUT	(HZ/4)

#define FH_MCI_REQUEST_TIMEOUT	(5 * HZ)

#define MAX_RETRY_COUNT		1000000

#define MMC_CLK			50000000
#define MMC_CCLK_MAX		50000000
#define MMC_CCLK_MIN		60000

/* Base address of SD card register */
#define FH_MCI_INTR		(48+32)

#define FH_MCI_DEBUG		DISABLE

#if FH_MCI_DEBUG
extern int debug_type;
#define fhmci_DEBUG_TYPE	(FHMCI_DEBUG_TYPE_REG | \
				FHMCI_DEBUG_TYPE_FUN | \
				FHMCI_DEBUG_TYPE_CMD | \
				FHMCI_DEBUG_TYPE_INFO | \
				FHMCI_DEBUG_TYPE_ERR)

#define FHMCI_DEBUG_TYPE_REG	(0x01 << 0)
#define FHMCI_DEBUG_TYPE_FUN	(0x01 << 1)
#define FHMCI_DEBUG_TYPE_CMD	(0x01 << 2)
#define FHMCI_DEBUG_TYPE_INFO	(0x01 << 3)
#define FHMCI_DEBUG_TYPE_ERR	(0x01 << 4)

#define FHMCI_DEBUG_FMT "FHMCI_DEBUG "

#define FHMCI_DEBUG(type, msg...) do { \
	if (debug_type & type) { \
		printf(FHMCI_DEBUG_FMT "(%s) %s:%d: ", \
		get_debug_type_string(type), \
		__func__, __LINE__); \
		printf(msg); \
		printf("\n"); \
	} \
} while (0)

#define FHMCI_DEBUG_REG(msg...)	FHMCI_DEBUG(FHMCI_DEBUG_TYPE_REG, msg)
#define FHMCI_DEBUG_FUN(msg...)	FHMCI_DEBUG(FHMCI_DEBUG_TYPE_FUN, msg)
#define FHMCI_DEBUG_CMD(msg...)	FHMCI_DEBUG(FHMCI_DEBUG_TYPE_CMD, msg)
#define FHMCI_DEBUG_INFO(msg...) FHMCI_DEBUG(FHMCI_DEBUG_TYPE_INFO, msg)
#define FHMCI_DEBUG_ERR(msg...)	FHMCI_DEBUG(FHMCI_DEBUG_TYPE_ERR, msg)

#define FHMCI_ASSERT_FMT "FHMCI_ASSERT "

#define FHMCI_ASSERT(cond) do { \
	if (!(cond)) {\
		printf(FHMCI_ASSERT_FMT "%s:%d\n", __func__, __LINE__); \
		BUG(); \
	} \
} while (0)
#else
#define FHMCI_DEBUG(type, msg...)
#define FHMCI_DEBUG_REG(msg...)
#define FHMCI_DEBUG_FUN(msg...)
#define FHMCI_DEBUG_CMD(msg...)
#define FHMCI_DEBUG_INFO(msg...)
#define FHMCI_DEBUG_ERR(msg...)
#define FHMCI_ASSERT(cond)
#endif

#define FHMCI_ERROR_FMT "FHMCI_ERROR "

#define FHMCI_ERROR(s...) do { \
	printf(FHMCI_ERROR_FMT "%s:%d: ", __func__, __LINE__); \
	printf(s); \
	printf("\n"); \
} while (0)

#define fhmci_readl(addr)({unsigned int reg = readl((unsigned int)addr); \
	FHMCI_DEBUG_REG("readl(0x%04X) = 0x%08X", (unsigned int)addr, reg); \
	reg; })

#define fhmci_writel(v, addr) do { writel(v, (unsigned int)addr); \
	FHMCI_DEBUG_REG("writel(0x%04X) = 0x%08X", (unsigned int)addr,\
		 (unsigned int)(v)); \
} while (0)


struct fhmci_dma_des {
	unsigned long idmac_des_ctrl;
	unsigned long idmac_des_buf_size;
	unsigned long idmac_des_buf_addr;
	unsigned long idmac_des_next_addr;
};

struct fhmci_host {
	struct mmc       mmc;
	unsigned long    base;
	unsigned int     card_status;
	int              cmd_id;
	struct mmc_cmd *cmd;
	struct fhmci_dma_des *dma_des;
};

typedef union {
	unsigned int cmd_arg;
	struct cmd_bits_arg{
		unsigned int cmd_index:6;
		unsigned int response_expect:1;
		unsigned int response_length:1;
		unsigned int check_response_crc:1;
		unsigned int data_transfer_expected:1;
		unsigned int read_write:1;
		unsigned int transfer_mode:1;
		unsigned int send_auto_stop:1;
		unsigned int wait_prvdata_complete:1;
		unsigned int stop_abort_cmd:1;
		unsigned int send_initialization:1;
		unsigned int card_number:5;
		unsigned int update_clk_reg_only:1; /* bit 21 */
		unsigned int read_ceata_device:1;
		unsigned int ccs_expected:1;
		unsigned int enable_boot:1;
		unsigned int expect_boot_ack:1;
		unsigned int disable_boot:1;
		unsigned int boot_mode:1;
		unsigned int volt_switch:1;
		unsigned int use_hold_reg:1;
		unsigned int reserved:1;
		unsigned int start_cmd:1; /* HSB */
	} bits;
} cmd_arg_s;

#endif
