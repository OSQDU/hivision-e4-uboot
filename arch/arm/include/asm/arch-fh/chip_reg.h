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

#ifndef CHIP_REG_H_
#define CHIP_REG_H_
#if defined(CONFIG_SOC_FH81) || defined(CONFIG_SOC_FH62)
#include "fh81.h"
#elif defined(CONFIG_SOC_LINBAO)
#include "linbao.h"
#endif

#endif /* CHIP_REG_H_ */
