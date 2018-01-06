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
#include <watchdog.h>
#include <asm/arch/hardware.h>

#define		WDT_APB_CLK			(54000000)
#define		WDT_REG_BASE		(0xF0D00000)
/*
 * Set the watchdog time interval in 1/256Hz (write-once)
 * Counter is 12 bit.
 */
#define TABLE_SIZE		16
unsigned int clk2value_table[TABLE_SIZE]; //unit:ms

static inline void clk2value_table_init(unsigned int wdt_inclk)
{
	unsigned int i;
	volatile unsigned int temp1, temp2;
	temp2 = wdt_inclk / 1000;
	for (i = 0; i < TABLE_SIZE; i++) {
		temp1 = 1 << (16 + i);
		temp1 /= temp2;
		clk2value_table[i] = temp1;
	}
}

static inline unsigned int fh81_wdt_get_clk(unsigned int apb_clk)
{
	unsigned int div;
	div = reg_read(PMU_REG_BASE + 0x38) >> 8;
	div &= 0x1f;
	return (apb_clk / (div + 1));
}

void fh81_wdt_settimeout(unsigned int timeout)
{
	//first set the cnt,then kick the wdt
	unsigned int i;
	for (i = 0; i < TABLE_SIZE; i++) {
		if (timeout < clk2value_table[i])
			break;
	}
	if (i >= TABLE_SIZE)
		i = TABLE_SIZE - 1;

	printf("actually time is:%dms\n", clk2value_table[i]);
	SET_REG(WDT_REG_BASE + 0x04, i);
	//kick the wdt
	SET_REG(WDT_REG_BASE+ 0x0c, 0x76);
}

void fh81_wdt_settimemax(void)
{
	SET_REG(WDT_REG_BASE + 0x04, 15);
	//kick the wdt
	SET_REG(WDT_REG_BASE+ 0x0c, 0x76);
}

void hw_watchdog_reset(void)
{
	SET_REG( WDT_REG_BASE+ 0x0c, 0x76 );
}

void hw_watchdog_init(unsigned int timeout)
{
	unsigned int wdt_clk, wdt_status;
	wdt_clk = fh81_wdt_get_clk(WDT_APB_CLK);
	wdt_status = GET_REG(WDT_REG_BASE);

	if (wdt_status & 0x01) {
		clk2value_table_init(wdt_clk);
		fh81_wdt_settimeout(timeout);
	} else {
		clk2value_table_init(wdt_clk);
		fh81_wdt_settimeout(timeout);
		SET_REG(WDT_REG_BASE, 0x01);
	}
}

void hw_board_wdt_init(void)
{
	unsigned int wdt_clk, wdt_status;

	wdt_clk = fh81_wdt_get_clk(WDT_APB_CLK);
	wdt_status = GET_REG(WDT_REG_BASE);
	if (wdt_status & 0x01) {
		clk2value_table_init(wdt_clk);
		fh81_wdt_settimemax();
	}
}
