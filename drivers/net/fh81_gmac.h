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

#ifndef FH81_GMAC_H_
#define FH81_GMAC_H_

#include "fh81_gmac_phyt.h"
#include "fh81_gmac_dma.h"
#include <asm/arch/hardware.h>

#define GMAC_TIMEOUT_AUTODONE	(40000)		//2s
#define GMAC_TIMEOUT_PHYLINK 	(20000)		//2s
#define GMAC_TIMEOUT_RECV		(10000)		//1s
#define GMAC_TIMEOUT_SEND		(10000)		//1s

//#define FH81_GMAC_DEBUG
#define RX_DESC_NUM 4

enum {
	gmac_gmii_clock_60_100,
	gmac_gmii_clock_100_150,
	gmac_gmii_clock_20_35,
	gmac_gmii_clock_35_60,
	gmac_gmii_clock_150_250,
	gmac_gmii_clock_250_300
};

enum {
	gmac_interrupt_all = 0x0001ffff,
	gmac_interrupt_none = 0x0
};

enum {
	gmac_mii,
	gmac_rmii
};

typedef struct Gmac_Object {
	UINT8 local_mac_address[6];
	int phy_interface;
	int full_duplex;			//read only
	int speed_100m;				//read only
	volatile Gmac_Rx_DMA_Descriptors *rx_dma_descriptors;
	volatile Gmac_Tx_DMA_Descriptors *tx_dma_descriptors;
} Gmac_Object;

extern int fh81_gmac_initialize(bd_t *bis);

#endif /* FH81_GMAC_H_ */
