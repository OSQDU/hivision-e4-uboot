/*
 * Copyright (C) 2016 Fullhan Corporation
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
#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>

//#define FH_SPI_DEBUG printf
#define FH_SPI_DEBUG(...)

#define SPI_FIFO_DEPTH			(32)
#define	SPI0_REG_BASE						(0xf0500000)
#define	SPI1_REG_BASE						(0xf0600000)

#define SPI_CTRL0_OFFSET            		(0x00)
#define SPI_CTRL1_OFFSET            		(0x04)
#define SPI_SSIENR_OFFSET               	(0x08)
#define SPI_MWCR_OFFSET             		(0x0c)
#define SPI_SER_OFFSET              		(0x10)
#define SPI_BAUD_OFFSET             		(0x14)
#define SPI_TXFTL_OFFSET            		(0x18)
#define SPI_RXFTL_OFFSET            		(0x1c)
#define SPI_TXFL_OFFSET             		(0x20)
#define SPI_RXFL_OFFSET             		(0x24)
#define SPI_STATUS_OFFSET               	(0x28)
#define SPI_IMR_OFFSET              		(0x2c)
#define SPI_ISR_OFFSET              		(0x30)
#define SPI_RISR_OFFSET             		(0x34)
#define SPI_TXOIC_OFFSET            		(0x38)
#define SPI_RXOIC_OFFSET            		(0x3c)
#define SPI_RXUIC_OFFSET            		(0x40)
#define SPI_MSTIC_OFFSET            		(0x44)
#define SPI_INTCLR_OFFSET               	(0x48)
#define SPI_DMACTRL_OFFSET              	(0x4c)
#define SPI_DMATDL_OFFSET               	(0x50)
#define SPI_DMARDL_OFFSET               	(0x54)
#define SPI_IDR_OFFSET              		(0x58)
#define SPI_SSI_COMPVER_OFFSET              (0x5c)
#define SPI_DATA_OFFSET             		(0x60)
#define SPI_INTERVAL                		(0x100000)
#define REG_SPI_CTRLR0(n)                  	(n + SPI_CTRL0_OFFSET)
#define REG_SPI_CTRLR1(n)                   (n + SPI_CTRL1_OFFSET)
#define REG_SPI_SSIENR(n)                   (n + SPI_SSIENR_OFFSET)
#define REG_SPI_MWCR(n)                     (n + SPI_MWCR_OFFSET)
#define REG_SPI_SER(n)                      (n  + SPI_SER_OFFSET)
#define REG_SPI_BAUDR(n)                    (n  + SPI_BAUD_OFFSET)
#define REG_SPI_TXFTLR(n)                   (n  + SPI_TXFTL_OFFSET)
#define REG_SPI_RXFTLR(n)                   (n  + SPI_RXFTL_OFFSET)
#define REG_SPI_TXFLR(n)                    (n + SPI_TXFL_OFFSET)
#define REG_SPI_RXFLR(n)                    (n  + SPI_RXFL_OFFSET)
#define REG_SPI_SR(n)                       (n  + SPI_STATUS_OFFSET)
#define REG_SPI_IMR(n)                      (n  + SPI_IMR_OFFSET)
#define REG_SPI_ISR(n)                      (n  + SPI_ISR_OFFSET)
#define REG_SPI_RISR(n)                     (n  + SPI_RISR_OFFSET)
#define REG_SPI_TXOICR(n)                   (n  + SPI_TXOIC_OFFSET)
#define REG_SPI_RXOICR(n)                   (n  + SPI_RXOIC_OFFSET)
#define REG_SPI_RXUICR(n)                   (n  + SPI_RXUIC_OFFSET)
#define REG_SPI_MSTICR(n)                   (n  + SPI_MSTIC_OFFSET)
#define REG_SPI_ICR(n)                      (n + SPI_INTCLR_OFFSET)
#define REG_SPI_DMACR(n)                    (n  + SPI_DMACTRL_OFFSET)
#define REG_SPI_DMATDLR(n)                  (n  + SPI_DMATDL_OFFSET)
#define REG_SPI_DMARDLR(n)                  (n  + SPI_DMARDL_OFFSET)
#define REG_SPI_IDR(n)                      (n  + SPI_IDR_OFFSET)
#define REG_SPI_SSI_COMP_VERSION(n)         (n  + SPI_SSI_COMPVER_OFFSET)
#define REG_SPI_SSI_DR(n)                   (n  + SPI_DATA_OFFSET)


#define SPI_CONTINUE_READ_MAX_DATA_NUM		0x10000  //64K
#define SPI_CONTINUE_READ_MIN_DATA_NUM		0x01     //one byte
#define SPI_TX_FIFO_DEPTH					16
#define SPI_RX_FIFO_DEPTH					16
#define SPI_IRQ_TXEIS						(lift_shift_bit_num(0))
#define SPI_IRQ_TXOIS						(lift_shift_bit_num(1))
#define SPI_IRQ_RXUIS						(lift_shift_bit_num(2))
#define SPI_IRQ_RXOIS						(lift_shift_bit_num(3))
#define SPI_IRQ_RXFIS						(lift_shift_bit_num(4))
#define SPI_IRQ_MSTIS						(lift_shift_bit_num(5))
#define SPI_STATUS_BUSY        				(lift_shift_bit_num(0))
#define SPI_STATUS_TFNF         			(lift_shift_bit_num(1))
#define SPI_STATUS_TFE         				(lift_shift_bit_num(2))
#define SPI_STATUS_RFNE        				(lift_shift_bit_num(3))
#define SPI_STATUS_RFF         				(lift_shift_bit_num(4))
#define SPI_STATUS_TXE         				(lift_shift_bit_num(5))
#define SPI_STATUS_DCOL        				(lift_shift_bit_num(6))
#define SPI_READ_BYTE_TIME_LIMIT			5000
#define SPI_WRITE_BYTE_TIME_LIMIT			5000
#define SPI_WRITE_READ_BYTE_TIME_LIMIT		5000
#define SPI_EEPROM_WREN						0x06
#define SPI_EEPROM_RDSR						0x05
#define SPI_EEPROM_READ						0x03
#define SPI_EEPROM_WRITE					0x02
#ifndef CONFIG_DEFAULT_SPI_BUS
#define CONFIG_DEFAULT_SPI_BUS				0
#endif

#ifndef CONFIG_DEFAULT_SPI_MODE
#define CONFIG_DEFAULT_SPI_MODE 			SPI_MODE_0
#endif

//error status
enum {
	CONFIG_OK = 0,
	CONFIG_PARA_ERROR = lift_shift_bit_num(0),
	//only for the set slave en/disable
	CONFIG_BUSY = lift_shift_bit_num(1),
	//only for write_read mode
	WRITE_READ_OK = 0,
	WRITE_READ_ERROR = lift_shift_bit_num(2),
	WRITE_READ_TIME_OUT = lift_shift_bit_num(3),
	//only for write only mode
	WRITE_ONLY_OK = 0,
	WRITE_ONLY_ERROR = lift_shift_bit_num(4),
	WRITE_ONLY_TIME_OUT = lift_shift_bit_num(5),
	//only for read only mode
	READ_ONLY_OK = 0,
	READ_ONLY_ERROR = lift_shift_bit_num(6),
	READ_ONLY_TIME_OUT = lift_shift_bit_num(7),
	//eeprom mode
	EEPROM_OK = 0,
	EEPROM_ERROR = lift_shift_bit_num(8),
	EEPROM_TIME_OUT = lift_shift_bit_num(9),
	/* if read/write/eeprom error,the error below could give you more
	 * info by reading the 'Spi_ReadTransferError' function
	 */
	MULTI_MASTER_ERROR = lift_shift_bit_num(10),
	TX_OVERFLOW_ERROR = lift_shift_bit_num(11),
	RX_OVERFLOW_ERROR = lift_shift_bit_num(12),
};

//enable spi
typedef enum enum_spi_enable {
	SPI_DISABLE = 0,
	SPI_ENABLE = (lift_shift_bit_num(0)),
} spi_enable_e;

//polarity
typedef enum enum_spi_polarity {
	SPI_POLARITY_LOW = 0,
	SPI_POLARITY_HIGH = (lift_shift_bit_num(7)),
	//bit pos
	SPI_POLARITY_RANGE = (lift_shift_bit_num(7)),
} spi_polarity_e;

//phase
typedef enum enum_spi_phase {
	SPI_PHASE_RX_FIRST = 0,
	SPI_PHASE_TX_FIRST = (lift_shift_bit_num(6)),
	//bit pos
	SPI_PHASE_RANGE = (lift_shift_bit_num(6)),
} spi_phase_e;

//frame format
typedef enum enum_spi_format {
	SPI_MOTOROLA_MODE = 0x00,
	SPI_TI_MODE = 0x10, SPI_MICROWIRE_MODE = 0x20,
	//bit pos
	SPI_FRAME_FORMAT_RANGE = 0x30,
} spi_format_e;

//data size
typedef enum enum_spi_data_size {
	SPI_DATA_SIZE_4BIT = 0x03,
	SPI_DATA_SIZE_5BIT = 0x04,
	SPI_DATA_SIZE_6BIT = 0x05,
	SPI_DATA_SIZE_7BIT = 0x06,
	SPI_DATA_SIZE_8BIT = 0x07,
	SPI_DATA_SIZE_9BIT = 0x08,
	SPI_DATA_SIZE_10BIT = 0x09,
	//bit pos
	SPI_DATA_SIZE_RANGE = 0x0f,
} spi_data_size_e;

//transfer mode
typedef enum enum_spi_transfer_mode {
	SPI_TX_RX_MODE = 0x000,
	SPI_ONLY_TX_MODE = 0x100,
	SPI_ONLY_RX_MODE = 0x200,
	SPI_EEPROM_MODE = 0x300,
	//bit pos
	SPI_TRANSFER_MODE_RANGE = 0x300,
} spi_transfer_mode_e;

//spi baudrate
typedef enum enum_spi_baudrate {
	SPI_SCLKIN = 50000000,      //54M clk_in
	SPI_SCLKOUT_27000000 = (SPI_SCLKIN / 27000000),  //27M
	SPI_SCLKOUT_13500000 = (SPI_SCLKIN / 13500000),  //13.5M
	SPI_SCLKOUT_6000000 = (SPI_SCLKIN / 6000000),  //6.75M
	SPI_SCLKOUT_6750000 = (SPI_SCLKIN / 6750000),  //6.75M
	SPI_SCLKOUT_4500000 = (SPI_SCLKIN / 4500000),	 //4.5M
	SPI_SCLKOUT_3375000 = (SPI_SCLKIN / 3375000),  //3.375M
	SPI_SCLKOUT_2700000 = (SPI_SCLKIN / 2700000),	 //2.7M
	SPI_SCLKOUT_1500000 = (SPI_SCLKIN / 1500000),  //1.5M
	SPI_SCLKOUT_100000 = (SPI_SCLKIN / 100000),  //0.1M
	SPI_SCLKOUT_1000000 = (SPI_SCLKIN / 1000000),  //1M
} spi_baudrate_e;

//spi_irq
typedef enum enum_spi_irq {
	SPI_IRQ_TXEIM = (lift_shift_bit_num(0)),
	SPI_IRQ_TXOIM = (lift_shift_bit_num(1)),
	SPI_IRQ_RXUIM = (lift_shift_bit_num(2)),
	SPI_IRQ_RXOIM = (lift_shift_bit_num(3)),
	SPI_IRQ_RXFIM = (lift_shift_bit_num(4)),
	SPI_IRQ_MSTIM = (lift_shift_bit_num(5)),
	SPI_IRQ_ALL = 0x3f,
} spi_irq_e;

//spi_slave_port
typedef enum enum_spi_slave {
	SPI_SLAVE_PORT0 = (lift_shift_bit_num(0)),
	SPI_SLAVE_PORT1 = (lift_shift_bit_num(1)),
} spi_slave_e;

//dma control
typedef enum enum_spi_dma_control_mode {
	SPI_DMA_RX_POS = (lift_shift_bit_num(0)),
	SPI_DMA_TX_POS = (lift_shift_bit_num(1)),
	//bit pos
	SPI_DMA_CONTROL_RANGE = 0x03,
} spi_dma_control_mode_e;

typedef struct _fh81_spi_controller {
	unsigned int base;
	unsigned int freq; /* Default frequency */
	unsigned int mode;
	struct spi_slave slave;
} fh81_spi_controller;

const unsigned int SPI_FLASH_CS[SPI_MAX_BUS_NUM*2] = {
	SPI_FLASH_CS0,
	SPI_FLASH_CS1,
#if SPI_MAX_BUS_NUM>1
	SPI1_CS0,
	SPI1_CS1,
#endif
};

const unsigned int FH_SPI_BASE[] = {
	SPI0_REG_BASE,
	SPI1_REG_BASE,
};

SINT32 Spi_Enable(UINT32 base, spi_enable_e enable)
{
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, enable);
	SET_REG(REG_SPI_SSIENR(base), enable);
	return CONFIG_OK;
}

SINT32 Spi_SetPolarity(UINT32 base, spi_polarity_e polarity)
{
	UINT32 data;

	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, polarity);
	data = GET_REG(REG_SPI_CTRLR0(base));
	data &= ~(UINT32) SPI_POLARITY_RANGE;
	data |= polarity;
	SET_REG(REG_SPI_CTRLR0(base), data);
	return CONFIG_OK;
}

SINT32 Spi_SetPhase(UINT32 base, spi_phase_e phase)
{
	UINT32 data;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, phase);
	data = GET_REG(REG_SPI_CTRLR0(base));
	data &= ~(UINT32) SPI_PHASE_RANGE;
	data |= phase;
	SET_REG(REG_SPI_CTRLR0(base), data);
	return CONFIG_OK;
}

SINT32 Spi_SetFrameFormat(UINT32 base, spi_format_e format)
{
	UINT32 data = 0;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, format);
	data = GET_REG(REG_SPI_CTRLR0(base));
	data &= ~(UINT32) SPI_FRAME_FORMAT_RANGE;
	data |= format;
	SET_REG(REG_SPI_CTRLR0(base), data);
	return CONFIG_OK;
}

SINT32 Spi_SetDataSize(UINT32 base, spi_data_size_e size)
{
	UINT32 data = 0;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, size);
	data = GET_REG(REG_SPI_CTRLR0(base));
	data &= ~(UINT32) SPI_DATA_SIZE_RANGE;
	data |= size;
	SET_REG(REG_SPI_CTRLR0(base), data);
	return CONFIG_OK;
}

SINT32 Spi_SetTransferMode(UINT32 base, spi_transfer_mode_e mode)
{
	UINT32 data = 0;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, mode);
	data = GET_REG(REG_SPI_CTRLR0(base));
	data &= ~(UINT32) SPI_TRANSFER_MODE_RANGE;
	data |= mode;
	SET_REG(REG_SPI_CTRLR0(base), data);
	return CONFIG_OK;
}

SINT32 Spi_SetContinueReadDataNum(UINT32 base, UINT32 num)
{
	UINT32 data = 0;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, num);
	data = num - 1;
	SET_REG(REG_SPI_CTRLR1(base), data);
	return CONFIG_OK;
}

SINT32 Spi_SetBaudrate(UINT32 base, spi_baudrate_e baudrate)
{
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, baudrate);
	SET_REG(REG_SPI_BAUDR(base), baudrate);
	return CONFIG_OK;
}

SINT32 Spi_DisableIrq(UINT32 base, UINT32 irq)
{
	UINT32 data = 0;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, irq);
	data = GET_REG(REG_SPI_IMR(base));
	data &= ~irq;
	SET_REG(REG_SPI_IMR(base), data);
	return CONFIG_OK;
}

SINT32 Spi_ReadStatus(UINT32 base)
{
	FH_SPI_DEBUG("sf %s(%x)\n", __func__, base);
	return (UINT8) GET_REG(REG_SPI_SR(base));
}

SINT32 Spi_EnableSlaveen(UINT32 base, int bus, int cs)
{
	UINT32 data;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, cs);
	gpio_direction_output(SPI_FLASH_CS[SPI_MAX_CS_NUM * bus + cs], GPIO_LEVEL_LOW);

	data = GET_REG(REG_SPI_SER(base));
	data |= cs + SPI_SLAVE_PORT0;
	SET_REG(REG_SPI_SER(base), data);
	return CONFIG_OK;
}

static UINT32 Spi_ReadTxfifolevel(UINT32 base)
{
	//UINT32 data;
	FH_SPI_DEBUG("sf %s(%x)\n", __func__, base);
	return GET_REG(REG_SPI_TXFLR(base));
}

static inline UINT32 tx_max(UINT32 base, UINT32 tx_len)
{
	UINT32 hw_tx_level;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, tx_len);
	hw_tx_level = Spi_ReadTxfifolevel(base);
	hw_tx_level = SPI_FIFO_DEPTH - hw_tx_level;
	hw_tx_level /= 2;
	return MIN(hw_tx_level, tx_len);
}

SINT32 Spi_DisableSlaveen(UINT32 base, int bus, int cs)
{
	UINT32 data;
	FH_SPI_DEBUG("sf %s(%x,%d)\n", __func__, base, cs);
	gpio_direction_output(SPI_FLASH_CS[SPI_MAX_CS_NUM * bus + cs], GPIO_LEVEL_HIGH);

	data = GET_REG(REG_SPI_SER(base));
	data &= ~(cs + SPI_SLAVE_PORT0);
	SET_REG(REG_SPI_SER(base), data);
	return CONFIG_OK;
}

static inline fh81_spi_controller *to_fh81_spi(struct spi_slave *slave)
{
	return container_of(slave, fh81_spi_controller, slave);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if ((bus >= SPI_MAX_BUS_NUM) || (cs >= SPI_MAX_CS_NUM))
		return 0;
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
	WATCHDOG_RESET();
	fh81_spi_controller *fh81_spi = to_fh81_spi(slave);
	FH_SPI_DEBUG("sf %s(%x)\n", __func__, slave);
	Spi_EnableSlaveen(fh81_spi->base, fh81_spi->slave.bus ,
			  fh81_spi->slave.cs);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	WATCHDOG_RESET();
	FH_SPI_DEBUG("sf %s(%x)\n", __func__, slave);
	fh81_spi_controller *fh81_spi = to_fh81_spi(slave);
	Spi_DisableSlaveen(fh81_spi->base, fh81_spi->slave.bus ,
			   fh81_spi->slave.cs);
}

void spi_init(void)
{

}

void spi_init_f(void)
{

}

ssize_t spi_read(uchar *addr, int alen, uchar *buffer, int len)
{
	return 0;
}

ssize_t spi_write(uchar *addr, int alen, uchar *buffer, int len)
{
	return 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	fh81_spi_controller *fh81_spi;
	FH_SPI_DEBUG("sf %s(%d,%d,%d,%d)\n", __func__, bus, cs, max_hz, mode);
	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	fh81_spi = malloc(sizeof(fh81_spi_controller));
	if (!fh81_spi)
		return NULL;
	memset(fh81_spi, 0, sizeof(fh81_spi_controller));
	//para is default by test
	fh81_spi->freq = SPI_SCLKIN /
			 max_hz; /*SPI_SCLKOUT_6000000; SPI_SCLKOUT_100000; */
	fh81_spi->base = FH_SPI_BASE[bus]; /* SPI0_PORT; */
	fh81_spi->mode = mode;
	fh81_spi->slave.bus = bus;
	fh81_spi->slave.cs = cs;
	FH_SPI_DEBUG(" fh81_spi freq=%d, base=%x, mode=%d, bus=%d, cs=%d\n",
		     fh81_spi->freq, fh81_spi->base, fh81_spi->mode,
		     fh81_spi->slave.bus, fh81_spi->slave.cs);

	return &fh81_spi->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	fh81_spi_controller *fh81_spi = to_fh81_spi(slave);
	FH_SPI_DEBUG("sf %s(%x)\n", __func__, slave);
	free(fh81_spi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	fh81_spi_controller *fh81_spi = to_fh81_spi(slave);
	if (!fh81_spi)
		return -1;
	FH_SPI_DEBUG("sf %s(%x), spi(%x)\n", __func__, slave, fh81_spi);
	Spi_Enable(fh81_spi->base, SPI_DISABLE);
	//frame format
	Spi_SetFrameFormat(fh81_spi->base, SPI_MOTOROLA_MODE);
	//spi polarty
	Spi_SetPolarity(fh81_spi->base, SPI_POLARITY_HIGH);
	//spi phase
	Spi_SetPhase(fh81_spi->base, SPI_PHASE_TX_FIRST);
	//transfer data size
	Spi_SetDataSize(fh81_spi->base, SPI_DATA_SIZE_8BIT);
	//baudrate
	Spi_SetBaudrate(fh81_spi->base, 2);
	(void) Spi_DisableIrq(fh81_spi->base, SPI_IRQ_ALL);
	Spi_SetTransferMode(fh81_spi->base, SPI_TX_RX_MODE);
	Spi_Enable(fh81_spi->base, SPI_ENABLE);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	fh81_spi_controller *fh81_spi = to_fh81_spi(slave);
	FH_SPI_DEBUG("sf %s(%x)\n", __func__, slave);
	Spi_Enable(fh81_spi->base, SPI_DISABLE);
	Spi_Enable(fh81_spi->base, SPI_ENABLE);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	uint txupto, rxupto;
	uint temp;
	uchar *rxp;
	const uchar *txp;
	/*	uint status;*/
	FH_SPI_DEBUG("sf %s(%x,%d,%x,%x,%d)\n", __func__, slave, bitlen, dout, din,
		     flags);
	rxp = din;
	txp = dout;
	fh81_spi_controller *fh81_spi = to_fh81_spi(slave);
	unsigned int rx_fifo_len;
	unsigned int tx_fifo_len;
	unsigned int i;
	unsigned int data_addr;
//	unsigned int tx_flag = 0;
//	unsigned int rx_flag = 0;
	if (bitlen % 8) {
		flags |= SPI_XFER_END;
		goto out;
	}

	data_addr = REG_SPI_SSI_DR(fh81_spi->base);

	spi_release_bus(slave);
	if ((flags & SPI_XFER_BEGIN))
		spi_cs_activate(slave);

	txupto = rxupto = bitlen / 8;
	while (rxupto) {
		//read...
		//1:read fifo
		rx_fifo_len = GET_REG(REG_SPI_RXFLR(fh81_spi->base));
		//2:read data
		if (rxp) {
			for (i = 0; i < rx_fifo_len; i++) {
				temp = (UINT8) GET_REG(data_addr);
				*rxp++ = (UINT8) temp;
			}
		} else {
			for (i = 0; i < rx_fifo_len; i++)
				temp = (UINT8) GET_REG(data_addr);
		}
		rxupto -= rx_fifo_len;
		//write..
		//find need to write or not
		if (txupto) {
			//find how much to wirte...always write half fifo level
			tx_fifo_len = tx_max(fh81_spi->base, txupto);
			if (txp)
				for (i = 0; i < tx_fifo_len; i++) {
					temp = *txp++;
					SET_REG(data_addr, (UINT16)temp);
				}
			else
				for (i = 0; i < tx_fifo_len; i++)
					SET_REG(data_addr, 0xFF);
			txupto -= tx_fifo_len;
		}
	}
out:
	if ((flags & SPI_XFER_END))
		spi_cs_deactivate(slave);
	return 0;
}
