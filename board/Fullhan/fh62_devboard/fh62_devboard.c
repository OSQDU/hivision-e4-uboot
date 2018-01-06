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

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/fh81.h>
#include <asm/arch/chip_reg.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>
DECLARE_GLOBAL_DATA_PTR;
/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
void iomux_init(void)
{

#ifdef CONFIG_RMII
	SET_REG_M(REG_PMU_PAD_MAC_TXER_CFG, 0x100000, 0x100000);
	SET_REG(REG_PMU_PAD_MAC_REF_CLK_CFG, 0x3130);
#else
	SET_REG(REG_PMU_PAD_MAC_REF_CLK_CFG, 0x1100);
#endif
	SET_REG(REG_PMU_PAD_MAC_MDC_CFG, 0x1100);
	SET_REG(REG_PMU_PAD_MAC_MDIO_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_COL_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_CRS_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_RXCK_CFG, 0x111);
	SET_REG(REG_PMU_PAD_MAC_RXD0_CFG, 0x11100);
	SET_REG(REG_PMU_PAD_MAC_RXD1_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_RXD2_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_RXD3_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_RXDV_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_TXCK_CFG, 0x111);
	SET_REG(REG_PMU_PAD_MAC_TXD0_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_TXD1_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_TXD2_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_TXD3_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_TXEN_CFG, 0x11110);
	SET_REG(REG_PMU_PAD_MAC_RXER_CFG, 0x11110);

	//SPI0
	SET_REG(REG_PMU_PAD_SSI0_CLK_CFG, 0x10);
	SET_REG(REG_PMU_PAD_SSI0_TXD_CFG, 0x10);
	SET_REG(REG_PMU_PAD_SSI0_CSN_0_CFG, 0x121110);
	SET_REG(REG_PMU_PAD_SSI0_CSN_1_CFG, 0x121110);
	SET_REG(REG_PMU_PAD_SSI0_RXD_CFG, 0x10010);

	//ARM JTAG
	SET_REG(REG_PMU_PAD_GPIO_5_CFG, 0x1110);
	SET_REG(REG_PMU_PAD_GPIO_6_CFG, 0x1110);
	SET_REG(REG_PMU_PAD_GPIO_7_CFG, 0x1011);
	SET_REG(REG_PMU_PAD_GPIO_8_CFG, 0x1110);
	SET_REG(REG_PMU_PAD_GPIO_9_CFG, 0x1110);

	//uart0
	//uart0 in
	SET_REG(REG_PMU_PAD_GPIO_10_CFG,0x121110);  // MFS=0, PULLUP,IE=1,SMT=1,E=4mA, SR=Fast,
	//uart0 out
	SET_REG(REG_PMU_PAD_GPIO_11_CFG,0x111110);  // MFS=0, PULLDN,IE=1,SMT=1,E=4mA, SR=Fast

	//REG_PMU_VDAC_CTRL
	SET_REG(REG_PMU_VDAC_CTRL, 0x2);

	/*func gpio51 for sdc0 poweron*/
	SET_REG(0xf0000150,0x101110);

}

void clock_init(void)
{
	//UINT32 reg;
	//gate enable, spi0, gmac, uart0, timer0, wdt, pts
	SET_REG_M(REG_PMU_CLK_GATE, 0x720aa080, 0x720aa080);
	//SPI0
	SET_REG_M(REG_PMU_CLK_DIV3, 0xb, 0xff);
	//GMAC
	SET_REG_M(REG_PMU_CLK_DIV6, 0x5000000, 0xf000000);
	//UART0
	SET_REG_M(REG_PMU_CLK_DIV4, 0x1, 0xf);
	//TIMER0
	SET_REG_M(REG_PMU_CLK_DIV5, 0x1d0000, 0x3f0000);
	//PTS
	SET_REG_M(REG_PMU_CLK_DIV2, 0x23, 0x3f);
	//WDT
	SET_REG_M(REG_PMU_CLK_DIV5, 0x1d00, 0x3f00);
	SET_REG_M(REG_PMU_CLK_GATE, 0, 0x720aa080);
}

int check_wdt_status(void)
{
	return 0;
}

extern void hw_board_wdt_init(void);

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();
	gd->bd->bi_arch_number = MACH_TYPE_FH81;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	iomux_init();
	clock_init();
#ifndef CONFIG_SPL_BUILD
	hw_board_wdt_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
#ifdef CONFIG_MACB
	/*
	 * Initialize ethernet HW addr prior to starting Linux,
	 * needed for nfsroot
	 */
	eth_init(gd->bd);
#endif
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)AT91SAM9260_BASE_EMAC, 0x00);
#endif
	return rc;
}

void show_boot_progress(int val)
{

}
