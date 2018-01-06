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
#include <asm/arch/chip_reg.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>
#include <asm/arch/gpio.h>
DECLARE_GLOBAL_DATA_PTR;
/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initializations
 */
void iomux_init(void)
{

	SET_REG(REG_PMU_PAD_GPIO_0_CFG, 0x4001000);
	SET_REG(REG_PMU_PAD_GPIO_1_CFG, 0x4001000);
	SET_REG(REG_PMU_PAD_GPIO_2_CFG, 0x4001000);
	SET_REG(REG_PMU_PAD_GPIO_3_CFG, 0x4001000);
	SET_REG(REG_PMU_PAD_GPIO_4_CFG, 0x4001000);

#ifdef CONFIG_LINBAO_QFN
	SET_REG(REG_PMU_PAD_MAC_REF_CLK_CFG, 0x10001000);
	SET_REG(REG_PMU_PAD_MAC_RMII_CLK_CFG,  0x1000);
	SET_REG(REG_PMU_PAD_MAC_MDC_CFG,  0x1101000);
	SET_REG(REG_PMU_PAD_MAC_MDIO_CFG, 0x1101000);
	SET_REG(REG_PMU_PAD_GPIO_3_CFG,  0x5101000);
	SET_REG(REG_PMU_PAD_GPIO_4_CFG, 0x5101000);
	SET_REG(REG_PMU_PAD_MAC_RXD0_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXD1_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXDV_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXD0_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXD1_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXEN_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXER_CFG, 0x101000);
#else
	SET_REG(REG_PMU_PAD_MAC_REF_CLK_CFG, 0x10001000);
	SET_REG(REG_PMU_PAD_MAC_RMII_CLK_CFG,  0x1000);
	SET_REG(REG_PMU_PAD_MAC_MDC_CFG,  0x101000);
	SET_REG(REG_PMU_PAD_MAC_MDIO_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_COL_CFG,  0x101000);
	SET_REG(REG_PMU_PAD_MAC_CRS_CFG,  0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXCK_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXD0_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXD1_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXD2_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXD3_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXDV_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXCK_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXD0_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXD1_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXD2_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXD3_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_TXEN_CFG, 0x101000);
	SET_REG(REG_PMU_PAD_MAC_RXER_CFG, 0x101000);
#endif

	gpio_direction_output(55, GPIO_LEVEL_LOW);
	udelay(5000);
	gpio_direction_output(55, GPIO_LEVEL_HIGH);

	//SPI0
	SET_REG(REG_PMU_PAD_SSI0_CLK_CFG,   0x1000);
	SET_REG(REG_PMU_PAD_SSI0_TXD_CFG,   0x1000);
	SET_REG(REG_PMU_PAD_SSI0_CSN_0_CFG, 0x1001000);
	SET_REG(REG_PMU_PAD_SSI0_CSN_1_CFG, 0x1001000);
	SET_REG(REG_PMU_PAD_SSI0_RXD_CFG,   0x1000);

	//uart0
	//uart0 in
	SET_REG(REG_PMU_PAD_UART_RX_CFG,0x1000);  // MFS=0, PULLUP,IE=1,SMT=1,E=4mA, SR=Fast,
	//uart0 out
	SET_REG(REG_PMU_PAD_UART_TX_CFG,0x1000);  // MFS=0, PULLDN,IE=1,SMT=1,E=4mA, SR=Fast

	//REG_PMU_VDAC_CTRL
	SET_REG(REG_PMU_VDAC_CTRL, 0x1100);

}

void clock_init(void)
{
	//gate enable, spi0, gmac, uart0, timer0, wdt, pts
	SET_REG_M(REG_PMU_CLK_GATE, 0x720a2080, 0x720a2080);
	//SPI0
	SET_REG_M(REG_PMU_CLK_DIV3, 0xb, 0xff);
	//GMAC
	SET_REG_M(REG_PMU_CLK_DIV6, 0xb000000, 0xf000000);
	//UART0
	SET_REG_M(REG_PMU_CLK_DIV4, 0x1, 0x1f);
	//TIMER0
	SET_REG_M(REG_PMU_CLK_DIV5, 0x3b0000, 0xff0000);
	//PTS
	SET_REG_M(REG_PMU_CLK_DIV2, 0x3b, 0xff);
	//WDT
	SET_REG_M(REG_PMU_CLK_DIV5, 0x3b00, 0xff00);
	SET_REG_M(REG_PMU_CLK_GATE, 0, 0x720a2080);
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
	gd->bd->bi_arch_number = MACH_TYPE_LINBAO;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	iomux_init();
	clock_init();
	hw_board_wdt_init();
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

void do_arc_jump(ulong addr)
{
    unsigned int arc_addr;

    arc_addr = ((addr & 0xffff) << 16) | (addr >> 16);

    printf("arc addr: 0x%x\n", arc_addr);

    // ARC Reset
    SET_REG(REG_PMU_SWRSTN_NSR, 0xFFBFFFFF);

    SET_REG(REG_PMU_A625BOOT0 , 0x7940266B);
    SET_REG(REG_PMU_A625BOOT1 , arc_addr);  // Configure ARC Bootcode start address
    SET_REG(REG_PMU_A625BOOT2 , 0x0F802020);
    SET_REG(REG_PMU_A625BOOT3 , arc_addr);

    SET_REG(REG_PMU_REMAP , 0 );  // Disable A2X BUS Remap and Resize

    // ARC reset released
    SET_REG( REG_PMU_SWRSTN_NSR, 0xFFFFFFFF);

    // wait ramloader done, about 1024 ARC CPU cycle
    udelay(100);

    // start ARC625
    SET_REG_M( REG_PMU_A625_START_CTRL, 0xFF, 0x10);
}

