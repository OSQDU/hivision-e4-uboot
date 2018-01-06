#include <asm/arch/hardware.h>
#include <common.h>
#include "ENCReg.h"

static  UINT8   Enc28j60Bank;
static  UINT16  NextPacketPtr;

#define ETH_CS_PIN     (23) //use GPIO55 as CS signal
#define ENC28J_CLK_DIV (8)

//GPIO define
#define SET_DATA	(0xf4000000 + 0x0000)	// GPio data bit set register
#define DIRECTION	(0xf4000000 + 0x0004)	// GPio direction register: 0:Input 1:Output
#define DATA_IN		(0xf4000000 + 0x0050)	// GPio data input register



//SPI0 define
#define ENC_SPI_BASE     (0xf0500000)
#define ENC_CTRL0_DEFVAL (0x07)
#define	ENC_TX		 (0x01<<8)
#define	ENC_TXANDRX	 (0x03<<8)

#define ENC_SPI_CTRL0    (ENC_SPI_BASE+0x00)
#define ENC_SPI_RECV_NUM (ENC_SPI_BASE+0x04)
#define ENC_SPI_EN       (ENC_SPI_BASE+0x08)
#define ENC_SPI_CS_EN    (ENC_SPI_BASE+0x10)
#define ENC_SPI_BAUDR    (ENC_SPI_BASE+0x14)
#define ENC_SPI_IMR      (ENC_SPI_BASE+0x2c)
#define ENC_SPI_DR       (ENC_SPI_BASE+0x60)
#define ENC_SPI_SR       (ENC_SPI_BASE+0x28)
#define HW_REG(reg)      (*((volatile unsigned int *)(reg)))

#define ENC_CS_1() do {HW_REG(SET_DATA) = HW_REG(DATA_IN) | (1<<ETH_CS_PIN);}while(0)
#define ENC_CS_0() do {HW_REG(SET_DATA) = HW_REG(DATA_IN) & (~(1<<ETH_CS_PIN));}while(0)


#define SPI_CS_EN(x) do {HW_REG(ENC_SPI_CS_EN) = (x<<1);}while(0)
#define     REG_PMU_PAD_SSI0_CSN_1_CFG      (0xf0000000 + 0x160)

static void eth_spi_setup(void)
{
	//set ENC28J60's CS signal.
	//SET_REG(REG_PMU_PAD_GPIO_12_CFG, 0x1110);  
	SET_REG(REG_PMU_PAD_SSI0_CSN_1_CFG, 0x111110);  
	HW_REG(DIRECTION) = HW_REG(DIRECTION) | (1<<ETH_CS_PIN); //output mode 
	ENC_CS_1(); //default to high

	//init SPI.
	HW_REG(ENC_SPI_EN) = 0;
	SPI_CS_EN(0);
	HW_REG(ENC_SPI_BAUDR) = ENC28J_CLK_DIV; // 54MHz/16=3.375MHz.
	HW_REG(ENC_SPI_IMR) = 0; // mask interrupt.
	HW_REG(ENC_SPI_CTRL0) = ENC_CTRL0_DEFVAL;
	HW_REG(ENC_SPI_RECV_NUM) = 0;
}

static void NETCiWriteOp(UINT8 op,  //命令
                UINT8 addr,//地址
                UINT8 dat  //数据
                ) 
{
	UINT32 status;

	ENC_CS_0();
	
	HW_REG(ENC_SPI_EN) = 0;
	SPI_CS_EN(0);
	HW_REG(ENC_SPI_BAUDR) = ENC28J_CLK_DIV; // 54MHz/16=3.375MHz.

	HW_REG(ENC_SPI_CTRL0) = ENC_CTRL0_DEFVAL | ENC_TX;
	HW_REG(ENC_SPI_EN) = 1;
	HW_REG(ENC_SPI_DR) = op | (addr & ADDR_MASK);
	
	if (op != ENC28J60_SOFT_RESET) {
		HW_REG(ENC_SPI_DR) = dat;
	}
	
	SPI_CS_EN(1);

	do {	status = HW_REG(ENC_SPI_SR);}while ((status&1) || !(status&4)); //wait TX finish

	ENC_CS_1();
}    


/******************************************************************
*函数名称:NETCiReadOp
*输    入:op 命令,addr 地址
*输    出:无
*说    明:网络 读控制寄存器
*******************************************************************/ 
static UINT8 NETCiReadOp (UINT8 op,  //命令
                   UINT8 addr//地址
                  ) 
{
	UINT32 status;
	UINT32 cnt = 1;
	UINT8   d = 0;

	ENC_CS_0();
	
	if(addr & 0x80) {
		cnt++;
	}
	
	HW_REG(ENC_SPI_EN) = 0;
	SPI_CS_EN(0);
	HW_REG(ENC_SPI_BAUDR) = ENC28J_CLK_DIV; // 54MHz/16=3.375MHz.

	HW_REG(ENC_SPI_CTRL0) = ENC_CTRL0_DEFVAL | ENC_TXANDRX;
	HW_REG(ENC_SPI_RECV_NUM) = cnt-1;
	
	HW_REG(ENC_SPI_EN) = 1;
	HW_REG(ENC_SPI_DR) = op | (addr & ADDR_MASK);
	SPI_CS_EN(1);

	while (cnt > 0) {
		do {	status = HW_REG(ENC_SPI_SR);}while (!(status&8));
		d = HW_REG(ENC_SPI_DR);
		cnt--;
	}

	do {	status = HW_REG(ENC_SPI_SR);}while ((status&1) || !(status&4)); //wait TX finish

	ENC_CS_1();
	
	return d;
}

static void NETCiReadBuffer(UINT16 len,  //长度
                     UINT8 *buf  //数据
                   )
{
	UINT32 status;

	if (len < 1) {
		return;
	}

	ENC_CS_0();
	
	HW_REG(ENC_SPI_EN) = 0;
	SPI_CS_EN(0);
	HW_REG(ENC_SPI_BAUDR) = ENC28J_CLK_DIV; // 54MHz/16=3.375MHz.

	HW_REG(ENC_SPI_CTRL0) = ENC_CTRL0_DEFVAL | ENC_TXANDRX;
	HW_REG(ENC_SPI_RECV_NUM) = len-1;
	
	HW_REG(ENC_SPI_EN) = 1;
	HW_REG(ENC_SPI_DR) = ENC28J60_READ_BUF_MEM;
	SPI_CS_EN(1);

	while (len > 0) {
		do {	status = HW_REG(ENC_SPI_SR);}while (!(status&8));
		*(buf++) = HW_REG(ENC_SPI_DR);
		len--;
	}

	do {	status = HW_REG(ENC_SPI_SR);}while ((status&1) || !(status&4)); //wait TX finish    

	ENC_CS_1();
}		

static void NETCiWriteBuffer(UINT16 len, //长度
                      UINT8 *buf //数据
                    )
{
	UINT32 status;

	if (len < 1) {
		return;
	}

	ENC_CS_0();

	HW_REG(ENC_SPI_EN) = 0;
	SPI_CS_EN(0);
	HW_REG(ENC_SPI_BAUDR) = ENC28J_CLK_DIV; // 54MHz/16=3.375MHz.

	HW_REG(ENC_SPI_CTRL0) = ENC_CTRL0_DEFVAL | ENC_TX;
	HW_REG(ENC_SPI_EN) = 1;
	
	HW_REG(ENC_SPI_DR) = ENC28J60_WRITE_BUF_MEM;

	while (len > 0) {
		status = HW_REG(ENC_SPI_SR);
		if (!(status & 2)) { //tx FIFO full
			break;
		}
		HW_REG(ENC_SPI_DR) = *(buf++);
		len--;
	}
	SPI_CS_EN(1);

	while (len > 0) {
		do {	status = HW_REG(ENC_SPI_SR);}while (!(status & 2));
		HW_REG(ENC_SPI_DR) = *(buf++);
		len--;
	}

	do {	status = HW_REG(ENC_SPI_SR);}while ((status&1) || !(status&4)); //wait TX finish

	ENC_CS_1();
}

static void NETCiSetBank(UINT8 addr)
{
	// set the bank (if needed)
	if((addr & BANK_MASK) != Enc28j60Bank)
	{
		// clear the set
		NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		// set the bank
		NETCiWriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (addr & BANK_MASK)>>5); 
		
		Enc28j60Bank = (addr & BANK_MASK);
	}
} 	
/******************************************************************
*函数名称:NETCiRead
*输    入:addr 地址
*输    出:一字节数据
*说    明:从某一个地址读取1字节数据
*******************************************************************/     
static UINT8 NETCiRead(UINT8 addr)
{
   	// set the bank
	NETCiSetBank(addr);
	// do the read
	return NETCiReadOp(ENC28J60_READ_CTRL_REG, addr);

}  		 

/*
UINT16 NETCiPhyRead(UINT8 addr)
{
	UINT16 result;
	
	// Set the right address and start the register read operation
	NETCiWrite(MIREGADR, addr);
	NETCiWrite(MICMD, MICMD_MIIRD);

	// wait until the PHY read completes
	while(NETCiRead(MISTAT) & MISTAT_BUSY);

	// reset reading bit
	NETCiWrite(MICMD, 0x00);

	result   = NETCiRead(MIRDL);
	result |= (NETCiRead(MIRDH) << 8);
	
	return result;
}
*/

/******************************************************************
*函数名称:NETCiWrite
*输    入:addr 地址  dat 数据
*输    出:无
*说    明:从某一个地址写入1字节数据
*******************************************************************/ 
static void NETCiWrite(UINT8 addr,
                UINT8 dat)
{
	// set the bank
	NETCiSetBank(addr);
	// do the write
	NETCiWriteOp(ENC28J60_WRITE_CTRL_REG, addr, dat);
}
/******************************************************************
*函数名称:NETCiPhyWrite
*输    入:addr 地址, dat 16位数据
*输    出:无
*说    明:设置PHY
*******************************************************************/
static void NETCiPhyWrite(UINT8  addr, 
                 UINT16 dat)
{
	// set the PHY register address
	NETCiWrite(MIREGADR, addr);
	
	// write the PHY data
	NETCiWrite(MIWRL, dat&0xff);
	NETCiWrite(MIWRH, dat>>8);

	// wait until the PHY write completes
	while(NETCiRead(MISTAT) & MISTAT_BUSY);
} 

static void SendSystemReset(void)
{
	UINT8 i;
	
	do {
		// Note: The power save feature may prevent the reset from executing, so
		// we must make sure that the device is not in power save before issuing
		// a reset.
		NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, ECON2, ECON2_PWRSV);

		// Give some opportunity for the regulator to reach normal regulation and
		// have all clocks running
		//DelayNms(2);
		udelay(2*1000);

		// Execute the System Reset command
		NETCiWriteOp(ENC28J60_SOFT_RESET, 0, 0);
    
		// Wait for the oscillator start up timer and PHY to become ready
		udelay(2*1000);
		//DelayNms(2);
		
		i = NETCiRead(ESTAT);
	} while((i & 0x08) || !(i & ESTAT_CLKRDY));
}

/******************************************************************
*函数名称:NETCiInit
*输    入:mac 地址
*输    出:无
*说    明:网络设备初始化
*******************************************************************/
static int NETCiInit(UINT8 *mac)
{
	eth_spi_setup();

	Enc28j60Bank = 0;
	NextPacketPtr = RXSTART_INIT;


	SendSystemReset();
	
	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// set receive buffer start address
	NETCiWrite(ERXSTL, RXSTART_INIT&0xFF);
	NETCiWrite(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	NETCiWrite(ERXRDPTL, RXSTOP_INIT&0xFF);
	NETCiWrite(ERXRDPTH, RXSTOP_INIT>>8);
	// set receive buffer end
	// ERXND defaults to 0x1FFF (end of ram)
	NETCiWrite(ERXNDL, RXSTOP_INIT&0xFF);
	NETCiWrite(ERXNDH, RXSTOP_INIT>>8);
	// set transmit buffer start
	// ETXST defaults to 0x0000 (beginnging of ram)
	NETCiWrite(ETXSTL, TXSTART_INIT&0xFF);
	NETCiWrite(ETXSTH, TXSTART_INIT>>8);   
	// Tx End
	NETCiWrite(ETXNDL, TXSTOP_INIT&0xFF);
	NETCiWrite(ETXNDH, TXSTOP_INIT>>8);  

	// do bank 1 stuff, packet filter,Unicast/Broadcast packets are accepted.
	NETCiWrite(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN);	

	// do bank 2 stuff
	// bring MAC out of reset
	NETCiWrite(MACON2, 0x00);
	// enable MAC receive
	NETCiWrite(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// Pad packets to 60 bytes, add CRC, and check Type/Length field.
	NETCiWrite(MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	// set inter-frame gap (back-to-back)
	NETCiWrite(MABBIPG, 0x12);

	// Allow infinite deferals if the medium is continuously busy
	// (do not time out a transmission if the half duplex medium is
	// completely saturated with other people's data)
	NETCiWrite(MACON4, MACON4_DEFER);

	// Late collisions occur beyond 63+8 bytes (8 bytes for preamble/start of frame delimiter)
	// 55 is all that is needed for IEEE 802.3, but ENC28J60 B5 errata for improper link pulse
	// collisions will occur less often with a larger number.
	NETCiWrite(MACLCON2, 63);

	// set inter-frame gap (non-back-to-back)
	NETCiWrite(MAIPGL, 0x12);
	NETCiWrite(MAIPGH, 0x0C);
	
	// Set the maximum packet size which the controller will accept
	NETCiWrite(MAMXFLL, (MAX_FRAME_LEN+20)&0xFF);
	NETCiWrite(MAMXFLH, (MAX_FRAME_LEN+20)>>8);

	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	NETCiWrite(MAADR5, mac[0]);
	NETCiWrite(MAADR4, mac[1]);
	NETCiWrite(MAADR3, mac[2]);
	NETCiWrite(MAADR2, mac[3]);
	NETCiWrite(MAADR1, mac[4]);
	NETCiWrite(MAADR0, mac[5]);

	// Disable the CLKOUT output to reduce EMI generation
	NETCiWrite(ECOCON, 0x00);

	// Get the Rev ID so that we can implement the correct errata workarounds
	//ENCRevID = NETCiRead(EREVID);

 	// no loopback of transmitted frames
	NETCiPhyWrite(PHCON2, PHCON2_HDLDIS);

	// Configure LEDA to display LINK status, LEDB to display TX/RX activity	
	NETCiPhyWrite(PHLCON,0xD76);	 
	//NETCiPhyWrite(PHLCON,0x3472); //SetLEDConfig(), microchip use this config.

	// Set the MAC and PHY into the proper duplex state
 	NETCiPhyWrite(PHCON1, 0x0000);

	// switch to bank 0
	NETCiSetBank(ECON1);
	
	// enable interrutps
	NETCiWriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	// enable packet reception
	NETCiWriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	return 0;
}

static void WaitTxReady(void)
{
	UINT8  status;
	UINT16 AttemptCounter;

	//wait until previous transmit finish.
	status = NETCiReadOp (ENC28J60_READ_CTRL_REG, ECON1);
	if (status & ECON1_TXRTS) {
		AttemptCounter = 0;
		// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
		do {
			status = NETCiRead(EIR);
		} while(!(status & (EIR_TXERIF | EIR_TXIF)) && (++AttemptCounter < 1000));

		if((status & (EIR_TXERIF)) || (AttemptCounter >= 1000u))
		{
			// Cancel the previous transmission if it has become stuck set
			printf("IF: TXRST\r\n");
			NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
			
			NETCiWriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
			NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
		}
	}
	NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF | EIR_TXIF);
}

/******************************************************************
*函数名称:NETCiPacketSend
*输    入:pDestAddr 目标IP地址
          txdnet    网络
          buffer_length 长度
          packet_type  数据包类型 
*输    出:无
*说    明:网络设备发送数据
*******************************************************************/
static void NETCiPacketSend(UINT8 *packet,UINT16 len)
{
	eth_spi_setup();

	WaitTxReady();

	NETCiWrite(ETXSTL, TXSTART_INIT&0xFF);
	NETCiWrite(ETXSTH, TXSTART_INIT>>8);   

	NETCiWrite(EWRPTL, TXSTART_INIT&0xFF);
	NETCiWrite(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	NETCiWrite(ETXNDL, (TXSTART_INIT+len)&0xFF);
	NETCiWrite(ETXNDH, (TXSTART_INIT+len)>>8);
	// write per-packet control byte (0x00 means use macon3 settings)
	NETCiWriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	// copy the packet into the transmit buffer
	NETCiWriteBuffer(len, packet);

	// send the contents of the transmit buffer onto the network
	NETCiWriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

    // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
    /*while(!(NETCiRead(EIR) & (EIR_TXERIF | EIR_TXIF)) && (++AttemptCounter < 1000u));
	if((NETCiRead(EIR) & (EIR_TXERIF)) || (AttemptCounter >= 1000u))
	{
		// Cancel the previous transmission if it has become stuck set
		NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}*/
}

/******************************************************************
*函数名称:NETCiPacketReceive
*输    入:rxdnet    网络
*输    出:长度
*说    明:网络设备接收数据
*******************************************************************/
static void reset_enc28j60(void)
{
	UINT16 wpt;

	printf("IF: Reset!\r\n");
	
	while(1) {
		wpt   = NETCiRead(ERXWRPTL);
		wpt |= (NETCiRead(ERXWRPTH) << 8);
		if (NETCiRead(EPKTCNT) == 0) {
			break;
		}
		NETCiWriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	}	

	NextPacketPtr = wpt;
	if (wpt ==0) {
		wpt = RXSTOP_INIT;
	}
	else {
		wpt--;
	}

	NETCiWrite(ERXRDPTL, (wpt)&0xff);
	NETCiWrite(ERXRDPTH, (wpt)>>8);
	NETCiWriteOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_RXERIF);
}

static UINT16 NETCiPacketReceive(UINT8 *packet,UINT16 maxlen)
{
	UINT16 rxstat;
	UINT16 len;
	UINT16 estimate;
	UINT16 original_ptr = NextPacketPtr;
	UINT16 new_ptr;
	
	eth_spi_setup();

	if ( NETCiRead(EPKTCNT) == 0 ) {
		return(0);
	}

	//init RD pointer
	NETCiWrite(ERDPTL, (original_ptr)&0xff);
	NETCiWrite(ERDPTH, (original_ptr)>>8);
	
	// read the next packet pointer
	new_ptr  = NETCiReadOp(ENC28J60_READ_BUF_MEM, 0);
	new_ptr |= NETCiReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	
	// read the packet length (see datasheet page 43)
	len  = NETCiReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= (NETCiReadOp(ENC28J60_READ_BUF_MEM, 0)<<8);	

	// read the receive status (see datasheet page 43)
	rxstat  = NETCiReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= (UINT16)(NETCiReadOp(ENC28J60_READ_BUF_MEM, 0)<<8);

	// check CRC and symbol errors (see datasheet page 44, table 7-3):
	// The ERXFCON.CRCEN is set by default. Normally we should not
	// need to check this.
	estimate = original_ptr + len + 6;
	if (estimate > RXSTOP_INIT) {
		estimate -= (RXSTOP_INIT+1);
	}
	if (estimate&1) {
		estimate++;
	}
	if (estimate > RXSTOP_INIT) {
		estimate = 0;
	}
	if (len <= (14+4/*CRC*/) || len > (RXSTOP_INIT-RXSTART_INIT-6) || estimate != new_ptr || (rxstat & ((1<<15)|(1<<4))) || !(rxstat & (1<<7))  ) {
		reset_enc28j60();
		return 0;
	}

	len -= 4; //remove the CRC count
	if (len > maxlen) {
      		len = 0; //drop the packet, it's length exceed our max receive buffer.
	}
	NETCiReadBuffer(len, packet);
    
	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	if (estimate == 0) {
		estimate = RXSTOP_INIT;
	}
	else {
		estimate--;
	}	
	NETCiWrite(ERXRDPTL, (estimate)&0xff);
	NETCiWrite(ERXRDPTH, (estimate)>>8);
	
	// decrement the packet counter indicate we are done with this packet
	NETCiWriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

	NextPacketPtr = new_ptr;

	return(len);	
}

static int ENC28J60_Init(struct eth_device* dev, bd_t* bd)
{
	int   i;
	char* s;
	char* e;
	UINT8 mac[6] = { 0x11, 0x22, 0x37, 0x44, 0x55, 0x66 };

	
	memcpy(dev->enetaddr, mac, 6);
	s = getenv("ethaddr");
	for (i = 0; i < 6; i++) {
		dev->enetaddr[i] = s ? simple_strtoul(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	printf("MAC: %pM\n", dev->enetaddr);

	if (NETCiInit(dev->enetaddr) < 0) {
		return -1;
	}

	return 0;
}

static void ENC28J60_Halt(struct eth_device* dev)
{
}

static int ENC28J60_Send(struct eth_device* dev, volatile void *packet, int length)
{
	NETCiPacketSend((UINT8*)packet, (UINT16)length);
	return 0;
}

static int ENC28J60_Receive(struct eth_device* dev)
{
	UINT16 len;
	static UINT8 nbuf[1600];

	len = NETCiPacketReceive(nbuf, sizeof(nbuf));
	if (len > 0) {
		NetReceive(nbuf, len);
	}

	return 1;
}


int fh81_enc28j60_initialize(bd_t* bis)
{
	static struct eth_device devobj;
	struct eth_device* dev;

	dev = &devobj;
	memset(dev, 0, sizeof(*dev));

	sprintf(dev->name, "ENC28J60-MAC");
	dev->init = ENC28J60_Init;
	dev->halt = ENC28J60_Halt;
	dev->send = ENC28J60_Send;
	dev->recv = ENC28J60_Receive;

	eth_register(dev);

	return 0;
}
