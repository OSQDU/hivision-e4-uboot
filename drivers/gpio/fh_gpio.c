/*
 *  Copyright (C) 2016 Fullhan
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>

static unsigned int gpio_base[2] = {
	GPIO0_BASE_ADDR,
	GPIO1_BASE_ADDR,
};

void gpio_dir_set(u8 bank, int gpiono, int dir)
{
	unsigned int tmp;

	tmp = readl(gpio_base[bank] + GPIO_SWPORTA_DDR);

	if (dir) //0:input  1:output
		tmp |= (1 << gpiono);
	else
		tmp &= ~(1 << gpiono);

	writel(tmp, gpio_base[bank] + GPIO_SWPORTA_DDR);
}

void gpio_data_set(u8 bank, int gpiono, int val)
{
	unsigned int tmp;

	tmp = readl(gpio_base[bank] + GPIO_SWPORTA_DR);

	if (val)
		tmp |= (1 << gpiono);
	else
		tmp &= ~(1 << gpiono);

	writel(tmp, gpio_base[bank] + GPIO_SWPORTA_DR);
}

unsigned int gpio_data_get(u8 bank, int gpiono)
{
	unsigned int tmp;

	tmp = readl(gpio_base[bank] + GPIO_SWPORTA_DR);

	return tmp;
}

void gpio_direction_output(int gpio, int value)
{
	u8 bank, no;
	bank = gpio / 32;
	no = gpio % 32;

	gpio_dir_set(bank, no, GPIO_DIR_OUT);
	gpio_data_set(bank, no, value);
}
