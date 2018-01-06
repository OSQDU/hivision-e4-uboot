/*
 * linbao.h
 *
 *  Created on: Jun 20, 2016
 *      Author: duobao
 */

#ifndef LINBAO_H_
#define LINBAO_H_

#define PMU_REG_BASE            (0xF0000000)

#define REG_PMU_CHIP_ID                  (PMU_REG_BASE + 0x0000)
#define REG_PMU_IP_VER                   (PMU_REG_BASE + 0x0004)
#define REG_PMU_FW_VER                   (PMU_REG_BASE + 0x0008)
#define REG_PMU_SYS_CTRL                 (PMU_REG_BASE + 0x000c)
#define REG_PMU_PLL0                     (PMU_REG_BASE + 0x0010)
#define REG_PMU_PLL1                     (PMU_REG_BASE + 0x0014)
#define REG_PMU_PLL2                     (PMU_REG_BASE + 0x0018)
#define REG_PMU_CLK_GATE                 (PMU_REG_BASE + 0x001c)
#define REG_PMU_CLK_SEL                  (PMU_REG_BASE + 0x0020)
#define REG_PMU_CLK_DIV0                 (PMU_REG_BASE + 0x0024)
#define REG_PMU_CLK_DIV1                 (PMU_REG_BASE + 0x0028)
#define REG_PMU_CLK_DIV2                 (PMU_REG_BASE + 0x002c)
#define REG_PMU_CLK_DIV3                 (PMU_REG_BASE + 0x0030)
#define REG_PMU_CLK_DIV4                 (PMU_REG_BASE + 0x0034)
#define REG_PMU_CLK_DIV5                 (PMU_REG_BASE + 0x0038)
#define REG_PMU_CLK_DIV6                 (PMU_REG_BASE + 0x003c)
#define REG_PMU_SWRST_MAIN_CTRL          (PMU_REG_BASE + 0x0040)
#define REG_PMU_SWRST_AXI_CTRL           (PMU_REG_BASE + 0x0044)
#define REG_PMU_SWRST_AHB_CTRL           (PMU_REG_BASE + 0x0048)
#define REG_PMU_SWRST_APB_CTRL           (PMU_REG_BASE + 0x004c)
#define REG_PMU_SPC_IO_STATUS            (PMU_REG_BASE + 0x0054)
#define REG_PMU_SPC_FUN                  (PMU_REG_BASE + 0x0058)
#define REG_PMU_CLK_DIV7                 (PMU_REG_BASE + 0x005c)
#define REG_PMU_CLK_GATE_S               (PMU_REG_BASE + 0x0060)
#define REG_PMU_DBG_SPOT0                (PMU_REG_BASE + 0x0064)
#define REG_PMU_DBG_SPOT1                (PMU_REG_BASE + 0x0068)
#define REG_PMU_DBG_SPOT2                (PMU_REG_BASE + 0x006c)
#define REG_PMU_DBG_SPOT3                (PMU_REG_BASE + 0x0070)

#define REG_PMU_PAD_CIS_HSYNC_CFG        (PMU_REG_BASE + 0x0080)
#define REG_PMU_PAD_CIS_VSYNC_CFG        (PMU_REG_BASE + 0x0084)
#define REG_PMU_PAD_CIS_PCLK_CFG         (PMU_REG_BASE + 0x0088)
#define REG_PMU_PAD_CIS_D_0_CFG          (PMU_REG_BASE + 0x008c)
#define REG_PMU_PAD_CIS_D_1_CFG          (PMU_REG_BASE + 0x0090)
#define REG_PMU_PAD_CIS_D_2_CFG          (PMU_REG_BASE + 0x0094)
#define REG_PMU_PAD_CIS_D_3_CFG          (PMU_REG_BASE + 0x0098)
#define REG_PMU_PAD_CIS_D_4_CFG          (PMU_REG_BASE + 0x009c)
#define REG_PMU_PAD_CIS_D_5_CFG          (PMU_REG_BASE + 0x00a0)
#define REG_PMU_PAD_CIS_D_6_CFG          (PMU_REG_BASE + 0x00a4)
#define REG_PMU_PAD_CIS_D_7_CFG          (PMU_REG_BASE + 0x00a8)
#define REG_PMU_PAD_CIS_D_8_CFG          (PMU_REG_BASE + 0x00ac)
#define REG_PMU_PAD_CIS_D_9_CFG          (PMU_REG_BASE + 0x00b0)
#define REG_PMU_PAD_CIS_D_10_CFG         (PMU_REG_BASE + 0x00b4)
#define REG_PMU_PAD_CIS_D_11_CFG         (PMU_REG_BASE + 0x00b8)
#define REG_PMU_PAD_MAC_RMII_CLK_CFG     (PMU_REG_BASE + 0x00bc)
#define REG_PMU_PAD_MAC_REF_CLK_CFG      (PMU_REG_BASE + 0x00c0)
#define REG_PMU_PAD_MAC_MDC_CFG          (PMU_REG_BASE + 0x00c4)
#define REG_PMU_PAD_MAC_MDIO_CFG         (PMU_REG_BASE + 0x00c8)
#define REG_PMU_PAD_MAC_COL_CFG          (PMU_REG_BASE + 0x00cc)
#define REG_PMU_PAD_MAC_CRS_CFG          (PMU_REG_BASE + 0x00d0)
#define REG_PMU_PAD_MAC_RXCK_CFG         (PMU_REG_BASE + 0x00d4)
#define REG_PMU_PAD_MAC_RXD0_CFG         (PMU_REG_BASE + 0x00d8)
#define REG_PMU_PAD_MAC_RXD1_CFG         (PMU_REG_BASE + 0x00dc)
#define REG_PMU_PAD_MAC_RXD2_CFG         (PMU_REG_BASE + 0x00e0)
#define REG_PMU_PAD_MAC_RXD3_CFG         (PMU_REG_BASE + 0x00e4)
#define REG_PMU_PAD_MAC_RXDV_CFG         (PMU_REG_BASE + 0x00e8)
#define REG_PMU_PAD_MAC_TXCK_CFG         (PMU_REG_BASE + 0x00ec)
#define REG_PMU_PAD_MAC_TXD0_CFG         (PMU_REG_BASE + 0x00f0)
#define REG_PMU_PAD_MAC_TXD1_CFG         (PMU_REG_BASE + 0x00f4)
#define REG_PMU_PAD_MAC_TXD2_CFG         (PMU_REG_BASE + 0x00f8)
#define REG_PMU_PAD_MAC_TXD3_CFG         (PMU_REG_BASE + 0x00fc)
#define REG_PMU_PAD_MAC_TXEN_CFG         (PMU_REG_BASE + 0x0100)
#define REG_PMU_PAD_MAC_RXER_CFG         (PMU_REG_BASE + 0x0104)
#define REG_PMU_PAD_MAC_TXER_CFG         (PMU_REG_BASE + 0x0108)
#define REG_PMU_PAD_GPIO_0_CFG           (PMU_REG_BASE + 0x010c)
#define REG_PMU_PAD_GPIO_1_CFG           (PMU_REG_BASE + 0x0110)
#define REG_PMU_PAD_GPIO_2_CFG           (PMU_REG_BASE + 0x0114)
#define REG_PMU_PAD_GPIO_3_CFG           (PMU_REG_BASE + 0x0118)
#define REG_PMU_PAD_GPIO_4_CFG           (PMU_REG_BASE + 0x011c)
#define REG_PMU_PAD_GPIO_5_CFG           (PMU_REG_BASE + 0x0120)
#define REG_PMU_PAD_GPIO_6_CFG           (PMU_REG_BASE + 0x0124)
#define REG_PMU_PAD_GPIO_7_CFG           (PMU_REG_BASE + 0x0128)
#define REG_PMU_PAD_GPIO_8_CFG           (PMU_REG_BASE + 0x012c)
#define REG_PMU_PAD_GPIO_9_CFG           (PMU_REG_BASE + 0x0130)
#define REG_PMU_PAD_GPIO_10_CFG          (PMU_REG_BASE + 0x0134)
#define REG_PMU_PAD_GPIO_11_CFG          (PMU_REG_BASE + 0x0138)
#define REG_PMU_PAD_GPIO_12_CFG          (PMU_REG_BASE + 0x013c)
#define REG_PMU_PAD_GPIO_13_CFG          (PMU_REG_BASE + 0x0140)
#define REG_PMU_PAD_GPIO_14_CFG          (PMU_REG_BASE + 0x0144)
#define REG_PMU_PAD_UART_RX_CFG          (PMU_REG_BASE + 0x0148)
#define REG_PMU_PAD_UART_TX_CFG          (PMU_REG_BASE + 0x014c)
#define REG_PMU_PAD_CIS_SCL_CFG          (PMU_REG_BASE + 0x0150)
#define REG_PMU_PAD_CIS_SDA_CFG          (PMU_REG_BASE + 0x0154)
#define REG_PMU_PAD_I2C_SCL_CFG          (PMU_REG_BASE + 0x0158)
#define REG_PMU_PAD_I2C_SDA_CFG          (PMU_REG_BASE + 0x015c)
#define REG_PMU_PAD_SSI0_CLK_CFG         (PMU_REG_BASE + 0x0160)
#define REG_PMU_PAD_SSI0_TXD_CFG         (PMU_REG_BASE + 0x0164)
#define REG_PMU_PAD_SSI0_CSN_0_CFG       (PMU_REG_BASE + 0x0168)
#define REG_PMU_PAD_SSI0_CSN_1_CFG       (PMU_REG_BASE + 0x016c)
#define REG_PMU_PAD_SSI0_RXD_CFG         (PMU_REG_BASE + 0x0170)
#define REG_PMU_PAD_SD0_CD_CFG           (PMU_REG_BASE + 0x0174)
#define REG_PMU_PAD_SD0_WP_CFG           (PMU_REG_BASE + 0x0178)
#define REG_PMU_PAD_SD0_CLK_CFG          (PMU_REG_BASE + 0x017c)
#define REG_PMU_PAD_SD0_CMD_RSP_CFG      (PMU_REG_BASE + 0x0180)
#define REG_PMU_PAD_SD0_DATA_0_CFG       (PMU_REG_BASE + 0x0184)
#define REG_PMU_PAD_SD0_DATA_1_CFG       (PMU_REG_BASE + 0x0188)
#define REG_PMU_PAD_SD0_DATA_2_CFG       (PMU_REG_BASE + 0x018c)
#define REG_PMU_PAD_SD0_DATA_3_CFG       (PMU_REG_BASE + 0x0190)
#define REG_PMU_PAD_SD1_CD_CFG           (PMU_REG_BASE + 0x0194)
#define REG_PMU_PAD_SD1_WP_CFG           (PMU_REG_BASE + 0x0198)
#define REG_PMU_PAD_SD1_CLK_CFG          (PMU_REG_BASE + 0x019c)
#define REG_PMU_PAD_SD1_CMD_RSP_CFG      (PMU_REG_BASE + 0x01a0)
#define REG_PMU_PAD_SD1_DATA_0_CFG       (PMU_REG_BASE + 0x01a4)
#define REG_PMU_PAD_SD1_DATA_1_CFG       (PMU_REG_BASE + 0x01a8)
#define REG_PMU_PAD_SD1_DATA_2_CFG       (PMU_REG_BASE + 0x01ac)
#define REG_PMU_PAD_SD1_DATA_3_CFG       (PMU_REG_BASE + 0x01b0)
#define REG_PMU_AXI0_PRIO_CFG0           (PMU_REG_BASE + 0x01b4)
#define REG_PMU_AXI0_PRIO_CFG1           (PMU_REG_BASE + 0x01b8)
#define REG_PMU_AXI1_PRIO_CFG0           (PMU_REG_BASE + 0x01bc)
#define REG_PMU_AXI1_PRIO_CFG1           (PMU_REG_BASE + 0x01c0)
#define REG_PMU_SWRSTN_NSR               (PMU_REG_BASE + 0x01c4)
#define REG_PMU_REMAP                    (PMU_REG_BASE + 0x01c8)
#define REG_PMU_A625_START_CTRL          (PMU_REG_BASE + 0x01cc)
#define REG_PMU_A625BOOT0                (PMU_REG_BASE + 0x01d0)
#define REG_PMU_A625BOOT1                (PMU_REG_BASE + 0x01d4)
#define REG_PMU_A625BOOT2                (PMU_REG_BASE + 0x01d8)
#define REG_PMU_A625BOOT3                (PMU_REG_BASE + 0x01dc)
#define REG_PMU_ARM_INT_0                (PMU_REG_BASE + 0x01e0)
#define REG_PMU_ARM_INT_1                (PMU_REG_BASE + 0x01e4)
#define REG_PMU_ARM_INT_2                (PMU_REG_BASE + 0x01e8)
#define REG_PMU_A625_INT_0               (PMU_REG_BASE + 0x01ec)
#define REG_PMU_A625_INT_1               (PMU_REG_BASE + 0x01f0)
#define REG_PMU_A625_INT_2               (PMU_REG_BASE + 0x01f4)
#define REG_PMU_DMA                      (PMU_REG_BASE + 0x01f8)
#define REG_PMU_WDT_CTRL                 (PMU_REG_BASE + 0x01fc)
#define REG_PMU_VDAC_CTRL                (PMU_REG_BASE + 0x0200)
#define REG_PMU_VDAC_STAT                (PMU_REG_BASE + 0x0204)
#define REG_PMU_DBG_STAT0                (PMU_REG_BASE + 0x0208)
#define REG_PMU_DBG_STAT1                (PMU_REG_BASE + 0x020c)
#define REG_PMU_DBG_STAT2                (PMU_REG_BASE + 0x0210)
#define REG_PMU_DBG_STAT3                (PMU_REG_BASE + 0x0214)
#define REG_PMU_AXI2_PRIO_CFG            (PMU_REG_BASE + 0x0218)


#endif /* LINBAO_H_ */