/*
 * (c) 2015 fullhan.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __FH81_H__
#define __FH81_H__

#define     PMU_REG_BASE        (0xf0000000)
//PMU
#define     REG_PMU_CHIP_ID                 (PMU_REG_BASE + 0x000)
#define     REG_PMU_IP_VER                  (PMU_REG_BASE + 0x004)
#define     REG_PMU_FW_VER                  (PMU_REG_BASE + 0x008)
#define     REG_PMU_SYS_CTRL                (PMU_REG_BASE + 0x00c)
#define     REG_PMU_PLL0_CTRL               (PMU_REG_BASE + 0x010)
#define     REG_PMU_PLL1_CTRL               (PMU_REG_BASE + 0x014)
#define     REG_PMU_ARC_CLK_GATE            (PMU_REG_BASE + 0x018)
#define     REG_PMU_CLK_GATE                (PMU_REG_BASE + 0x01c)
#define     REG_PMU_CLK_SEL                 (PMU_REG_BASE + 0x020)
#define     REG_PMU_CLK_DIV0                (PMU_REG_BASE + 0x024)
#define     REG_PMU_CLK_DIV1                (PMU_REG_BASE + 0x028)
#define     REG_PMU_CLK_DIV2                (PMU_REG_BASE + 0x02c)
#define     REG_PMU_CLK_DIV3                (PMU_REG_BASE + 0x030)
#define     REG_PMU_CLK_DIV4                (PMU_REG_BASE + 0x034)
#define     REG_PMU_CLK_DIV5                (PMU_REG_BASE + 0x038)
#define     REG_PMU_CLK_DIV6                (PMU_REG_BASE + 0x03c)
#define     REG_PMU_SWRST_MAIN_CTRL         (PMU_REG_BASE + 0x040)
#define     REG_PMU_SWRST_AXI_CTRL          (PMU_REG_BASE + 0x044)
#define     REG_PMU_SWRST_AHB_CTRL          (PMU_REG_BASE + 0x048)
#define     REG_PMU_SWRST_APB_CTRL          (PMU_REG_BASE + 0x04c)
#define     REG_PMU_VDAC_CTRL               (PMU_REG_BASE + 0x050)
#define     REG_PMU_PAD_MAC_REF_CLK_CFG     (PMU_REG_BASE + 0x0a4)
#define     REG_PMU_PAD_MAC_MDC_CFG         (PMU_REG_BASE + 0x0a8)
#define     REG_PMU_PAD_MAC_MDIO_CFG        (PMU_REG_BASE + 0x0ac)
#define     REG_PMU_PAD_MAC_COL_CFG         (PMU_REG_BASE + 0x0b0)
#define     REG_PMU_PAD_MAC_CRS_CFG         (PMU_REG_BASE + 0x0b4)
#define     REG_PMU_PAD_MAC_RXCK_CFG        (PMU_REG_BASE + 0x0b8)
#define     REG_PMU_PAD_MAC_RXD0_CFG        (PMU_REG_BASE + 0x0bc)
#define     REG_PMU_PAD_MAC_RXD1_CFG        (PMU_REG_BASE + 0x0c0)
#define     REG_PMU_PAD_MAC_RXD2_CFG        (PMU_REG_BASE + 0x0c4)
#define     REG_PMU_PAD_MAC_RXD3_CFG        (PMU_REG_BASE + 0x0c8)
#define     REG_PMU_PAD_MAC_RXDV_CFG        (PMU_REG_BASE + 0x0cc)
#define     REG_PMU_PAD_MAC_TXCK_CFG        (PMU_REG_BASE + 0x0d0)
#define     REG_PMU_PAD_MAC_TXD0_CFG        (PMU_REG_BASE + 0x0d4)
#define     REG_PMU_PAD_MAC_TXD1_CFG        (PMU_REG_BASE + 0x0d8)
#define     REG_PMU_PAD_MAC_TXD2_CFG        (PMU_REG_BASE + 0x0dc)
#define     REG_PMU_PAD_MAC_TXD3_CFG        (PMU_REG_BASE + 0x0e0)
#define     REG_PMU_PAD_MAC_TXEN_CFG        (PMU_REG_BASE + 0x0e4)
#define     REG_PMU_PAD_MAC_RXER_CFG        (PMU_REG_BASE + 0x0e8)
#define     REG_PMU_PAD_GPIO_5_CFG          (PMU_REG_BASE + 0x100)
#define     REG_PMU_PAD_GPIO_6_CFG          (PMU_REG_BASE + 0x104)
#define     REG_PMU_PAD_GPIO_7_CFG          (PMU_REG_BASE + 0x108)
#define     REG_PMU_PAD_GPIO_8_CFG          (PMU_REG_BASE + 0x10c)
#define     REG_PMU_PAD_GPIO_9_CFG          (PMU_REG_BASE + 0x110)
#define     REG_PMU_PAD_GPIO_10_CFG         (PMU_REG_BASE + 0x0114)
#define     REG_PMU_PAD_GPIO_11_CFG         (PMU_REG_BASE + 0x0118)
#define     REG_PMU_PAD_GPIO_12_CFG         (PMU_REG_BASE + 0x011c)



#define     REG_PMU_PAD_UART0_IN_CFG        (PMU_REG_BASE + 0x13c)
#define     REG_PMU_PAD_UART0_OUT_CFG       (PMU_REG_BASE + 0x140)
#define     REG_PMU_PAD_SSI0_CLK_CFG        (PMU_REG_BASE + 0x154)
#define     REG_PMU_PAD_SSI0_TXD_CFG        (PMU_REG_BASE + 0x158)
#define     REG_PMU_PAD_SSI0_CSN_0_CFG      (PMU_REG_BASE + 0x15c)
#define     REG_PMU_PAD_SSI0_CSN_1_CFG      (PMU_REG_BASE + 0x160)
#define     REG_PMU_PAD_SSI0_RXD_CFG        (PMU_REG_BASE + 0x164)
#define     REG_PMU_PAD_MAC_TXER_CFG        (PMU_REG_BASE + 0x1cc)

#define     PAD_PMU_PAD_SD1_CLCK_CFG        (PMU_REG_BASE + 0x188)
#define     PAD_PMU_PAD_SD1_DATA_0_CFG      (PMU_REG_BASE + 0x194)
#define     PAD_PMU_PAD_SD1_DATA_1_CFG      (PMU_REG_BASE + 0x198)
#define     PAD_PMU_PAD_SD1_DATA_2_CFG      (PMU_REG_BASE + 0x19c)
#define     PAD_PMU_PAD_SD1_CMD_RSP_CFG         (PMU_REG_BASE + 0x1a4)
#define     PAD_PMU_PAD_SD1_CD_CFG          (PMU_REG_BASE + 0x18c)
#define     PAD_PMU_PAD_SD1_WP_CFG          (PMU_REG_BASE + 0x190)

#endif /*__FH81_H__*/
