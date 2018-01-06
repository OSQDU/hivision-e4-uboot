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
#include <command.h>
#include <net.h>
#include <malloc.h>
#include "fh81_gmac.h"

#define 	GMAC_REG_BASE		(0xEF000000)

//GMAC-MAC
#define		REG_GMAC_CONFIG				(GMAC_REG_BASE + 0x0000)
#define		REG_GMAC_FRAME_FILTER		(GMAC_REG_BASE + 0x0004)
#define		REG_GMAC_GMII_ADDRESS		(GMAC_REG_BASE + 0x0010)
#define		REG_GMAC_GMII_DATA			(GMAC_REG_BASE + 0x0014)
#define		REG_GMAC_DEBUG				(GMAC_REG_BASE + 0x0024)
#define		REG_GMAC_MAC_HIGH 			(GMAC_REG_BASE + 0x0040)
#define		REG_GMAC_MAC_LOW 			(GMAC_REG_BASE + 0x0044)

//GMAC-DMA
#define		REG_GMAC_BUS_MODE			(GMAC_REG_BASE + 0x1000)
#define		REG_GMAC_TX_POLL_DEMAND		(GMAC_REG_BASE + 0x1004)
#define		REG_GMAC_RX_POLL_DEMAND		(GMAC_REG_BASE + 0x1008)
#define		REG_GMAC_RX_DESC_ADDR		(GMAC_REG_BASE + 0x100C)
#define		REG_GMAC_TX_DESC_ADDR		(GMAC_REG_BASE + 0x1010)
#define		REG_GMAC_STATUS				(GMAC_REG_BASE + 0x1014)
#define		REG_GMAC_OP_MODE			(GMAC_REG_BASE + 0x1018)
#define		REG_GMAC_INTR_EN			(GMAC_REG_BASE + 0x101C)
#define		REG_GMAC_ERROR_COUNT		(GMAC_REG_BASE + 0x1020)
#define		REG_GMAC_AXI_BUS_MODE		(GMAC_REG_BASE + 0x1028)
#define		REG_GMAC_AXI_STATUS			(GMAC_REG_BASE + 0x102C)
#define 	REG_GMAC_CURR_TX_DESC		(GMAC_REG_BASE + 0x1048)
#define 	REG_GMAC_CURR_RX_DESC		(GMAC_REG_BASE + 0x104C)

static void GMAC_SetMacAddress(UINT8* mac)
{
	UINT32 macHigh = mac[5] << 8 | mac[4];
	UINT32 macLow = mac[3] << 24 | mac[2] << 16 | mac[1] << 8 | mac[0];

	SET_REG(REG_GMAC_MAC_HIGH, macHigh);
	SET_REG(REG_GMAC_MAC_LOW, macLow);
}

static void GMAC_SetPhyRegister(int regNum, UINT32 data)
{
	SET_REG(REG_GMAC_GMII_DATA, data);
	SET_REG(REG_GMAC_GMII_ADDRESS,
			0x1 << 1 | PHYT_ID << 11 | gmac_gmii_clock_100_150 << 2
					| regNum << 6 | 0x1);
	while (GET_REG(REG_GMAC_GMII_ADDRESS) & 0x1) {

	}
}

static UINT32 GMAC_GetPhyRegister(int regNum)
{
	SET_REG(REG_GMAC_GMII_ADDRESS,
			PHYT_ID << 11 | gmac_gmii_clock_100_150 << 2 | regNum << 6 | 0x1);
	while (GET_REG(REG_GMAC_GMII_ADDRESS) & 0x1) {

	}
	return GET_REG(REG_GMAC_GMII_DATA);
}

static UINT32 GMAC_GetPhyRegisterEx(int phyid, int regNum)
{
	SET_REG(REG_GMAC_GMII_ADDRESS,
			phyid << 11 | gmac_gmii_clock_20_35 << 2 | regNum << 6 | 0x1);
	while (GET_REG(REG_GMAC_GMII_ADDRESS) & 0x1) {

	}
	return GET_REG(REG_GMAC_GMII_DATA);
}

static void GMAC_DmaTxDescriptorInit(Gmac_Object* gmac)
{
	gmac->tx_dma_descriptors->desc0.dw = 0;
	gmac->tx_dma_descriptors->desc1.dw = 0;
	gmac->tx_dma_descriptors->desc2.dw = 0;
	gmac->tx_dma_descriptors->desc3.dw = 0;

	gmac->tx_dma_descriptors->desc1.bit.first_segment = 1;
	gmac->tx_dma_descriptors->desc1.bit.last_segment = 1;
	gmac->tx_dma_descriptors->desc1.bit.end_of_ring = 1;
	gmac->tx_dma_descriptors->desc1.bit.second_address_chained = 0;
	gmac->tx_dma_descriptors->desc3.bit.buffer_address_pointer =
			(UINT32) gmac->tx_dma_descriptors;
}

static void GMAC_DmaRxDescriptorInit(Gmac_Object* gmac)
{
	int i;
	for (i = 0; i < RX_DESC_NUM; i++) {
		gmac->rx_dma_descriptors[i].desc0.dw = 0;
		gmac->rx_dma_descriptors[i].desc1.dw = 0;
		gmac->rx_dma_descriptors[i].desc2.dw = 0;
		gmac->rx_dma_descriptors[i].desc3.dw = 0;
		gmac->rx_dma_descriptors[i].desc1.bit.buffer1_size = 2000;
		gmac->rx_dma_descriptors[i].desc1.bit.end_of_ring = 0;
		gmac->rx_dma_descriptors[i].desc1.bit.second_address_chained = 1;
		gmac->rx_dma_descriptors[i].desc3.bit.buffer_address_pointer =
				(UINT32) &gmac->rx_dma_descriptors[i + 1];
		gmac->rx_dma_descriptors[i].desc2.bit.buffer_address_pointer =
				(UINT32) (NetRxPackets[i]);
		gmac->rx_dma_descriptors[i].desc0.bit.own = 1;

	}
	gmac->rx_dma_descriptors[RX_DESC_NUM - 1].desc3.bit.buffer_address_pointer =
			(UINT32) &gmac->rx_dma_descriptors[0];
	gmac->rx_dma_descriptors[RX_DESC_NUM - 1].desc1.bit.end_of_ring = 1;
}

static int GMAC_GetPhyAutoNegotiationStatus(Gmac_Object* gmac)
{
	Reg_Phyt_Basic_Ctrl basic_ctrl;
	Reg_Phyt_Basic_Status basic_status;
	basic_ctrl.dw = 0;
	basic_status.dw = 0;
	int i = 0;

	while (!basic_status.bit.auto_negotiate_complete) {
		basic_status.dw = GMAC_GetPhyRegister(gmac_phyt_reg_basic_status);
		udelay(10);
		i++;
		if (i > GMAC_TIMEOUT_AUTODONE) {
			printf("***ERROR: auto negotiation timeout\n");
			return -1;
		}
	}

	while (!basic_status.bit.link_status) {
		basic_status.dw = GMAC_GetPhyRegister(gmac_phyt_reg_basic_status);
		udelay(10);
		i++;
		if (i > GMAC_TIMEOUT_PHYLINK) {
			printf("***ERROR: could not establish link\n");
			return -1;
		}
	}

	basic_ctrl.dw = GMAC_GetPhyRegister(gmac_phyt_reg_basic_ctrl);

	gmac->full_duplex = basic_ctrl.bit.duplex_mode;
	gmac->speed_100m = basic_ctrl.bit.speed_select;

	if (gmac->speed_100m)
		SET_REG_M(REG_PMU_SYS_CTRL, 0x1000000, 0x1000000);
	else
		SET_REG_M(REG_PMU_SYS_CTRL, 0, 0x1000000);

	printf("operating at ");
	switch (basic_ctrl.bit.speed_select) {
	case 0:
		printf("10M ");
		break;
	case 1:
		printf("100M ");
		break;
	default:
		printf("unknown: %d ", basic_ctrl.bit.speed_select);
		break;
	}

	switch (basic_ctrl.bit.duplex_mode) {
	case 0:
		printf("half duplex ");
		break;
	case 1:
		printf("full duplex ");
		break;
	default:
		printf("unknown: %d ", basic_ctrl.bit.duplex_mode);
		break;
	}
	printf("mode\n");

	return 0;

}

static void GMAC_SetIomux(void)
{

}

void GMAC_EarlyInit(Gmac_Object* gmac)
{
	GMAC_SetIomux();

	if (gmac->phy_interface == gmac_rmii) {
		//mac speed: 1-100M
		//rmii mode: rmii
		//phy interface select: 1-rmii
		SET_REG_M(REG_PMU_SYS_CTRL, 0x7000000, 0x7000000);
		//pmu reset
		SET_REG(REG_PMU_SWRST_AHB_CTRL, 0xfffdffff);
		while (GET_REG(REG_PMU_SWRST_AHB_CTRL) != 0xffffffff) {

		}
	} else if (gmac->phy_interface == gmac_mii) {
		SET_REG_M(REG_PMU_SYS_CTRL, 0x1000000, 0x7000000);

		//pmu reset
		SET_REG(REG_PMU_SWRST_AHB_CTRL, 0xfffdffff);
		while (GET_REG(REG_PMU_SWRST_AHB_CTRL) != 0xffffffff) {

		}
	}
}

void GMAC_GetPhyID(Gmac_Object* gmac)
{
	UINT32 reg0, reg2, reg3;
	while (1) {
		reg0 = GMAC_GetPhyRegisterEx(3, gmac_phyt_reg_basic_ctrl);
		reg2 = GMAC_GetPhyRegisterEx(3, gmac_phyt_reg_phy_id1);
		reg3 = GMAC_GetPhyRegisterEx(3, gmac_phyt_reg_phy_id2);
		printf("phy id: %d, reg0: 0x%x, reg2: 0x%x, reg3: 0x%x\n", 3, reg0,
				reg2, reg3);
	}

}

void GMAC_PhyInit(Gmac_Object* gmac)
{
	UINT32 rmii_mode;

	//GMAC_GetPhyID(gmac);
#ifdef CONFIG_PHY_RTL8201
	//switch to page7
	GMAC_SetPhyRegister(gmac_phyt_rtl8201_page_select, 7);

	//phy rmii register settings
	rmii_mode = GMAC_GetPhyRegister(gmac_phyt_rtl8201_rmii_mode);
#endif

#ifdef CONFIG_PHY_IP101G
	GMAC_SetPhyRegister(gmac_phyt_ip101g_page_select, 16);
	//gmac_phyt_rtl8201_rmii_mode == gmac_phyt_ip101g_rmii_mode
	rmii_mode = GMAC_GetPhyRegister(gmac_phyt_rtl8201_rmii_mode);
#endif

#ifdef CONFIG_PHY_TI83848
	rmii_mode = GMAC_GetPhyRegister(gmac_phyt_ti83848_rmii_mode);
#endif
	if (gmac->phy_interface == gmac_rmii) {
#ifdef CONFIG_PHY_RTL8201
		//rmii_mode |= 0x1008;
		//0x7ffb when an external clock inputs to the CKXTAL2 pin
		rmii_mode = 0x7ffb;
#endif

#ifdef CONFIG_PHY_IP101G
        rmii_mode = 0x1006;
#endif

#ifdef CONFIG_PHY_TI83848
		rmii_mode |= 0x20;
#endif

	} else if (gmac->phy_interface == gmac_mii) {

#ifdef CONFIG_PHY_RTL8201
		//rmii_mode &= ~(0x1008);
		rmii_mode = 0x6ff3;
		//rmii_mode = 0xff2;
#endif

#ifdef CONFIG_PHY_IP101G
        rmii_mode = 0x2;
#endif

#ifdef CONFIG_PHY_TI83848
		rmii_mode &= ~(0x20);
#endif

	}
#ifdef CONFIG_PHY_RTL8201
	GMAC_SetPhyRegister(gmac_phyt_rtl8201_rmii_mode, rmii_mode);
	//back to page0
	GMAC_SetPhyRegister(gmac_phyt_rtl8201_page_select, 0);
#endif

#ifdef CONFIG_PHY_IP101G
    GMAC_SetPhyRegister(gmac_phyt_rtl8201_rmii_mode, rmii_mode);
    //back to page0
    GMAC_SetPhyRegister(gmac_phyt_ip101g_page_select, 0x10);
#endif

#ifdef CONFIG_PHY_TI83848
	GMAC_SetPhyRegister(gmac_phyt_ti83848_rmii_mode, rmii_mode);
#endif
}

int GMAC_DmaInit(Gmac_Object* gmac) {

	//confirm that all previously initiated (before software-reset) or ongoing AHB or AXI transactions are complete.
	while (GET_REG(REG_GMAC_AXI_STATUS) & 0x3) {
		//axi_status = GET_REG(REG_GMAC_AXI_STATUS);
	}
	//initialize dma bus mode reg0
	//address_aligned_beats
	//fixed_burst
	//burst_length = 32
	SET_REG(REG_GMAC_BUS_MODE, 1 << 25 | 1 << 16 | 32 << 8);

	//init dma descriptor reg 3 & 4
	GMAC_DmaTxDescriptorInit(gmac);
	GMAC_DmaRxDescriptorInit(gmac);
	SET_REG(REG_GMAC_RX_DESC_ADDR, (UINT32 )gmac->rx_dma_descriptors);
	SET_REG(REG_GMAC_TX_DESC_ADDR, (UINT32 )gmac->tx_dma_descriptors);

	//op mode, reg 6
	//transmit_store_forward
	//receive_store_forward
	//operate_on_second_frame
	SET_REG(REG_GMAC_OP_MODE, 0 << 25 | 0 << 21 | 0 << 2);

	//clear & disable all interrupts
	SET_REG(REG_GMAC_STATUS, 0xffffffff);

	//After a hardware or software reset, all interrupts are disabled
	//normal_interrupt_summary
	//receive_complete
	//SET_REG(REG_GMAC_INTR_EN, 1<<16 | 1<<15 | 1<<7 | 1<<6);
	SET_REG(REG_GMAC_INTR_EN, 0xffffbaff);

	//start tx & rx
	//start_stop_receive
	//start_stop_transmission_cmd
	//SET_REG(REG_GMAC_OP_MODE, 1<<13 | 1<<1);
	return 0;

}

int GMAC_MacInit(Gmac_Object* gmac)
{
	GMAC_GetPhyAutoNegotiationStatus(gmac);

	GMAC_SetMacAddress(gmac->local_mac_address);

	//receive all frames without any filters
	//SET_REG(REG_GMAC_FRAME_FILTER, (UINT32)1<<31);
	SET_REG(REG_GMAC_FRAME_FILTER, 1 << 5 | 1 << 9);
	//reg6 flow control

	//interrupt

	//set reg0
	//auto_pad_crc_stripping
	//duplex_mode
	//speed
	//port_select = MII
	//transmitter_enable
	//receiver_enable
	SET_REG(REG_GMAC_CONFIG,
			1 << 15 | gmac->speed_100m << 14 | gmac->full_duplex << 11 | 1 << 7
					| 1 << 3 | 1 << 2);

	return 0;
}

int GMAC_Probe(void)
{
	UINT32 phyReg2, phyReg3, ouiVal;
	phyReg2 = GMAC_GetPhyRegister(gmac_phyt_reg_phy_id1);
	phyReg3 = GMAC_GetPhyRegister(gmac_phyt_reg_phy_id2);
	ouiVal = ((phyReg3 & 0xfc00) << 6) | phyReg2;
	printf("PHY OUI: 0x%x\n", ouiVal);
	return 0;
}

int GMAC_SetMII(Gmac_Object* gmac)
{
    char* s;

    s = getenv("phymode");

    if(strcmp(s, "RMII"))
    {
        printf("set to MII\n");
        gmac->phy_interface = gmac_mii;
    }
    else
    {
        printf("set to RMII\n");
        gmac->phy_interface = gmac_rmii;
    }

    return 0;
}

int GMAC_Init(struct eth_device* dev, bd_t* bd)
{
	Gmac_Object* gmac;
	gmac = (Gmac_Object*) dev->priv;
	UINT8 mac[6] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	memcpy(dev->enetaddr, mac, 6);

	char* s;
	char* e;
	s = getenv("ethaddr");
	int i;
	for (i = 0; i < 6; i++) {
		dev->enetaddr[i] = s ? simple_strtoul(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	memcpy(gmac->local_mac_address, dev->enetaddr, 6);

	printf("MAC: %pM\n", dev->enetaddr);

	GMAC_SetMII(gmac);

    GMAC_PhyInit(gmac);
    GMAC_EarlyInit(gmac);

	if (GMAC_Probe() < 0) {
		return -1;
	}

	if (GMAC_DmaInit(gmac) < 0 || GMAC_MacInit(gmac) < 0) {
		return -1;
	}

	return 0;
}

void GMAC_Halt(struct eth_device* dev)
{
	//reset
	//GMAC_SetPhyRegister(gmac_phyt_reg_basic_ctrl, 0x1);
	//power down
	//GMAC_SetPhyRegister(gmac_phyt_reg_basic_ctrl, 0x800);
	//clear & disable interrupts
	SET_REG(REG_GMAC_STATUS, 0xffffffff);
	SET_REG(REG_GMAC_INTR_EN, 0x0);
	//tx & rx disable
	SET_REG_M(REG_GMAC_CONFIG, 0 << 3, 1 << 3);
	SET_REG_M(REG_GMAC_CONFIG, 0 << 2, 1 << 2);
}

int GMAC_Send(struct eth_device* dev, volatile void *packet, int length)
{
	int i = 0;
	SET_REG(REG_GMAC_STATUS, 0xffffffff);
	Gmac_Object* gmac;
	gmac = (Gmac_Object*) dev->priv;
	gmac->tx_dma_descriptors->desc1.bit.buffer1_size = length;
	gmac->tx_dma_descriptors->desc2.bit.buffer_address_pointer =
			(UINT32) packet;
	gmac->tx_dma_descriptors->desc0.bit.own = 1;

	SET_REG_M(REG_GMAC_OP_MODE, 1 << 13, 1 << 13);
	SET_REG(REG_GMAC_TX_POLL_DEMAND, 0);

	while (gmac->tx_dma_descriptors->desc0.bit.own) {
#ifdef FH81_GMAC_DEBUG
		UINT32 status;
		status = GET_REG(REG_GMAC_STATUS);
		if(status & 0xa0b2)
		{
			//fatal bus error, read Error Bits status[25:23]
			//or
			//abnormal interrupt
			printf("***ERROR: tx status: %x\n", status);
			return -1;
		}
#endif

		udelay(1);
		i++;
		if (i > GMAC_TIMEOUT_SEND) {
			printf("***ERROR: send timeout\n");
			return -2;
		}

	}
#ifdef FH81_GMAC_DEBUG
	if(gmac->tx_dma_descriptors->desc0.bit.error_summary)
	{
		printf("***ERROR: tx desc0: %x\n", gmac->tx_dma_descriptors->desc0.dw);
		return -2;

	}
#endif

	SET_REG_M(REG_GMAC_OP_MODE, 0 << 13, 1 << 13);

	return 0;
}

int GMAC_Receive(struct eth_device* dev)
{
	int j;
	SET_REG(REG_GMAC_STATUS, 0xffffffff);
	Gmac_Object* gmac;
	gmac = (Gmac_Object*) dev->priv;

	SET_REG_M(REG_GMAC_OP_MODE, 1 << 1, 1 << 1);
	SET_REG(REG_GMAC_TX_POLL_DEMAND, 0);

	for (j = 0; j < RX_DESC_NUM; j++) {
		if (!gmac->rx_dma_descriptors[j].desc0.bit.own) {
			int size = gmac->rx_dma_descriptors[j].desc0.bit.frame_length;
			NetReceive(NetRxPackets[j], size);
			gmac->rx_dma_descriptors[j].desc0.dw = 0;
			gmac->rx_dma_descriptors[j].desc0.bit.own = 1;
		}
	}

	SET_REG_M(REG_GMAC_OP_MODE, 0 << 1, 1 << 1);
	return 1;
}

int fh81_gmac_initialize(bd_t* bis)
{
	Gmac_Object* gmac;
	gmac = (Gmac_Object *) malloc(sizeof(*gmac));
	struct eth_device* dev;
	volatile Gmac_Rx_DMA_Descriptors* rx_desc;
	volatile Gmac_Tx_DMA_Descriptors* tx_desc;

	dev = (struct eth_device *) malloc(sizeof(*dev));
	if (dev == NULL) {
		printf("fh81_eth_initialize: Cannot allocate eth_device %d\n", 0);
		return (-1);
	}
	memset(dev, 0, sizeof(*dev));

	if (gmac == NULL) {
		printf("fh81_eth_initialize: Cannot allocate Gmac_Object %d\n", 1);
		return (-1);
	}

	memset(gmac, 0, sizeof(*gmac));

	rx_desc = (Gmac_Rx_DMA_Descriptors *) malloc(
			sizeof(*rx_desc) * RX_DESC_NUM);
	memset((void*) rx_desc, 0, sizeof(*rx_desc));

	tx_desc = (Gmac_Tx_DMA_Descriptors *) malloc(sizeof(*tx_desc));
	memset((void*) tx_desc, 0, sizeof(*tx_desc));

	gmac->rx_dma_descriptors = rx_desc;
	gmac->tx_dma_descriptors = tx_desc;

	sprintf(dev->name, "FH EMAC");
	dev->init = GMAC_Init;
	dev->halt = GMAC_Halt;
	dev->send = GMAC_Send;
	dev->recv = GMAC_Receive;
	dev->priv = (void *) gmac;

	eth_register(dev);

	GMAC_SetMII(gmac);

	return 0;
}

