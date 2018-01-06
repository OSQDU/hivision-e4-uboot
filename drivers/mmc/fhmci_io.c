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

#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include "fhmci.h"

#define SIMPLE_0            0
#define SIMPLE_90           1
#define SIMPLE_180          2
#define SIMPLE_270          3

#define SMP_SHIFT           SIMPLE_0

static void fh_mci_sys_init(void)
{
	unsigned int pctrl_gpio;
	unsigned int drv_shift;
	unsigned int sam_shift;
	unsigned int reg;

#ifdef CONFIG_MMC_SDC1
	drv_shift = 12;
	sam_shift = 8;
	pctrl_gpio = 6;
#else /* SDC0 */
	drv_shift = 20;
	sam_shift = 16;
	pctrl_gpio = SD0_POWER_CONTROL_GPIO;

#endif

	/*//Power on.*/
#ifdef CONFIG_HIK_MACH
	gpio_direction_output(pctrl_gpio, GPIO_LEVEL_HIGH);
#else
	gpio_direction_output(pctrl_gpio, GPIO_LEVEL_LOW);
#endif

	/*//adjust clock phase...*/
	reg = readl(REG_PMU_CLK_SEL);

	reg &= ~(3 << drv_shift);
	reg &= ~(3 << sam_shift);
	reg |= (2 << drv_shift); /*//now drv fixed to 180.*/
	reg |= (SMP_SHIFT << sam_shift);

	writel(reg, REG_PMU_CLK_SEL);
}

static void fh_mci_ctr_reset(void)
{
	//todo
}

static void fh_mci_ctr_undo_reset(void)
{
	//todo
}
