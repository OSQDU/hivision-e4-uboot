/*
 *  (c) 2015 fullhan.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

/****************************************************************************
 * #include section
 *	add #include here if any
 ***************************************************************************/

/****************************************************************************
 * #define section
 *	add constant #define here if any
 ***************************************************************************/

#define		UART0_REG_BASE			(0xf0700000)
#define		UART1_REG_BASE			(0xf0800000)
#define     REG_UART_RBR         	(0x0000)
#define     REG_UART_THR         	(0x0000)
#define     REG_UART_DLL         	(0x0000)
#define     REG_UART_DLH         	(0x0004)
#define     REG_UART_IER         	(0x0004)
#define     REG_UART_IIR         	(0x0008)
#define     REG_UART_FCR         	(0x0008)
#define     REG_UART_LCR         	(0x000c)
#define     REG_UART_MCR         	(0x0010)
#define     REG_UART_LSR         	(0x0014)
#define     REG_UART_MSR         	(0x0018)
#define     REG_UART_SCR         	(0x001c)
#define     REG_UART_FAR         	(0x0070)
#define     REG_UART_TFR 			(0x0074)
#define     REG_UART_RFW         	(0x0078)
#define     REG_UART_USR         	(0x007c)
#define     REG_UART_TFL         	(0x0080)
#define     REG_UART_RFL         	(0x0084)
#define     REG_UART_SRR         	(0x0088)
#define     REG_UART_SFE         	(0x0098)
#define     REG_UART_SRT         	(0x009c)
#define     REG_UART_STET        	(0x00a0)
#define     REG_UART_HTX         	(0x00a4)
#define     REG_UART_DMASA			(0x00a8)
#define     REG_UART_CPR         	(0x00f4)
#define     REG_UART_UCV         	(0x00f8)
#define     REG_UART_CTR         	(0x00fc)

#if (CONFIG_FH_SERIAL_CONSOLE==1)
#define 	CONSOLE_PORT 			(UART1_REG_BASE)
#else
#define 	CONSOLE_PORT 			(UART0_REG_BASE)
#endif

#define 	UART_LCR_DLAB_POS       (lift_shift_bit_num(7))
#define 	UART_READ_RX_DW_FIFO_OK				0

/****************************************************************************
* ADT section
*	add Abstract Data Type definition here
***************************************************************************/
//error status
enum {
	UART_CONFIG_OK = 0,
	UART_CONFIG_FIFO_OK = 0,
	UART_CONFIG_LINE_OK = 0,
	UART_CONFIG_DIVISOR_OK = 0,
	UART_WRITE_DATA_OK = 0,
	UART_READ_DATA_OK = 0,
	UART_CLEAR_ERROR_OK = 0,
	UART_RESET_RX_POOL_OK = 0,
	UART_CLEAR_RX_DATA_READY_OK = 0,
	UART_INIT_OK = 0,
	UART_CONFIG_PARA_ERROR = lift_shift_bit_num(0),
	UART_CONFIG_FIFO_ERROR = lift_shift_bit_num(1),
	UART_IS_BUSY = lift_shift_bit_num(2),
	UART_DW_FIFO_OVERFLOW = lift_shift_bit_num(3), //dw rxfifo overflow ,maybe rxisr is closed or main clock is too low
	UART_SW_FIFO_OVERFLOW = lift_shift_bit_num(4),//soft rxfifo overflow , maybe main clk is too low
	UART_PARITY_ERROR = lift_shift_bit_num(5),
	UART_FRAME_ERROR = lift_shift_bit_num(6),
	UART_BREAK_ERROR = lift_shift_bit_num(7),
	UART_FIFO_EMPTY = lift_shift_bit_num(8),
};

//interrupt enable
typedef enum enum_uart_irq {
	UART_INT_PTIME_POS = (lift_shift_bit_num(7)),
	UART_INT_EDSSI_POS = (lift_shift_bit_num(3)),
	UART_INT_ELSI_POS = (lift_shift_bit_num(2)),
	UART_INT_ETBEI_POS = (lift_shift_bit_num(1)),
	UART_INT_ERBFI_POS = (lift_shift_bit_num(0)),
	UART_INT_ALL = 0x0f,
} uart_irq_e;

//interrupt id
enum {
	UART_INT_ID_MODEM = 0,
	UART_INT_ID_NO_INT = 1,
	UART_INT_ID_THR_EMPTY = 2,
	UART_INT_ID_RECEIVE_DATA = 4,
	UART_INT_ID_RECEIVE_LINE = 6,
	UART_INT_ID_BUSY = 7,
	UART_INT_ID_TIME_OUT = 12,

	UART_FIFO_IS_ENABLE = 0xc0,
};

typedef enum enum_uart_line {
	Uart_line_5n1 = 0x00,   // 5 data bits, no parity, 1 stop bit
	Uart_line_5n1_5 = 0x04, // 5 data bits, no parity, 1.5 stop bits
	Uart_line_5e1 = 0x18,   // 5 data bits, even parity, 1 stop bit
	Uart_line_5e1_5 = 0x1c, // 5 data bits, even parity, 1.5 stop bits
	Uart_line_5o1 = 0x08,   // 5 data bits, odd parity, 1 stop bit
	Uart_line_5o1_5 = 0x0c, // 5 data bits, odd parity, 1.5 stop bits
	Uart_line_6n1 = 0x01,   // 6 data bits, no parity, 1 stop bit
	Uart_line_6n2 = 0x05,   // 6 data bits, no parity, 2 stop bits
	Uart_line_6e1 = 0x19,   // 6 data bits, even parity, 1 stop bit
	Uart_line_6e2 = 0x1d,   // 6 data bits, even parity, 2 stop bits
	Uart_line_6o1 = 0x09,   // 6 data bits, odd parity, 1 stop bit
	Uart_line_6o2 = 0x0d,   // 6 data bits, odd parity, 2 stop bits
	Uart_line_7n1 = 0x02,   // 7 data bits, no parity, 1 stop bit
	Uart_line_7n2 = 0x06,   // 7 data bits, no parity, 2 stop bits
	Uart_line_7e1 = 0x1a,   // 7 data bits, even parity, 1 stop bit
	Uart_line_7e2 = 0x1e,   // 7 data bits, even parity, 2 stop bits
	Uart_line_7o1 = 0x0a,   // 7 data bits, odd parity, 1 stop bit
	Uart_line_7o2 = 0x0e,   // 7 data bits, odd parity, 2 stop bits
	Uart_line_8n1 = 0x03,   // 8 data bits, no parity, 1 stop bit
	Uart_line_8n2 = 0x07,   // 8 data bits, no parity, 2 stop bits
	Uart_line_8e1 = 0x1b,   // 8 data bits, even parity, 1 stop bit
	Uart_line_8e2 = 0x1f,   // 8 data bits, even parity, 2 stop bits
	Uart_line_8o1 = 0x0b,   // 8 data bits, odd parity, 1 stop bit
	Uart_line_8o2 = 0x0f    // 8 data bits, odd parity, 2 stop bits
} uart_line_e;

//rx & tx fifo config
typedef enum enum_uart_fifo {
	UART_INT_RXFIFO_DEPTH_1 = 0x01,        	//fifo enable, rx 1 byte, set rx int
	UART_INT_RXFIFO_DEPTH_QUARTER = 0x41,//fifo enable, rx 1/4 fifo, set rx int
	UART_INT_RXFIFO_DEPTH_HALF = 0x81,	//fifo enable, rx 1/2 fifo, set rx int
	UART_INT_RXFIFO_2LESS_THAN_FULL = 0xc1,//fifo enable, rx 2 less than full,  set rx int
} uart_fifo_e;

//line status
enum {
	UART_LINE_STATUS_RFE = (lift_shift_bit_num(7)),
	UART_LINE_STATUS_TEMT = (lift_shift_bit_num(6)),
	UART_LINE_STATUS_THRE = (lift_shift_bit_num(5)),
	UART_LINE_STATUS_BI = (lift_shift_bit_num(4)),
	UART_LINE_STATUS_FE = (lift_shift_bit_num(3)),
	UART_LINE_STATUS_PE = (lift_shift_bit_num(2)),
	UART_LINE_STATUS_OE = (lift_shift_bit_num(1)),
	UART_LINE_STATUS_DR = (lift_shift_bit_num(0)),
};

//uart status
enum {
	UART_STATUS_RFF = (lift_shift_bit_num(4)),
	UART_STATUS_RFNE = (lift_shift_bit_num(3)),
	UART_STATUS_TFE = (lift_shift_bit_num(2)),
	UART_STATUS_TFNF = (lift_shift_bit_num(1)),
	UART_STATUS_BUSY = (lift_shift_bit_num(0)),
};

#define 	UART_CLOCK_FREQ   	(30000000)   //15MHZ
typedef enum enum_uart_baudrate {
	BAUDRATE_9600 = (((UART_CLOCK_FREQ / 9600) + 8) / 16),
	BAUDRATE_19200 = (((UART_CLOCK_FREQ / 19200) + 8) / 16),
	BAUDRATE_38400 = (((UART_CLOCK_FREQ / 38400) + 8) / 16),
	BAUDRATE_57600 = (((UART_CLOCK_FREQ / 57600) + 8) / 16),
	BAUDRATE_115200 = (((UART_CLOCK_FREQ / 115200) + 8) / 16),
	BAUDRATE_194000 = (((UART_CLOCK_FREQ / 194000) + 8) / 16),
} uart_baudrate_e;

#define SET_UART_BAUD(n)	(((UART_CLOCK_FREQ/(n))+8)/16)

typedef struct _UartDesc {
	UINT32 port;		// uart port
	uart_baudrate_e baudrate;
	uart_line_e line_ctrl;
	uart_fifo_e fifo_ctrl;
} UartDesc;

/****************************************************************************
 *  extern variable declaration section
 ***************************************************************************/

/****************************************************************************
 *  section
 *	add function prototype here if any
 ***************************************************************************/
SINT32 Uart_Init(UartDesc *desc);
SINT32 Uart_Write_Data(UartDesc *desc, const UINT8 *buf, UINT32 size);
SINT32 Uart_Read_Data(UartDesc *desc, UINT8 *buf, UINT32 size);
SINT32 Uart_Read_Rx_Fifo_Level(UartDesc *desc);
SINT32 Uart_Read_Control_Status(UartDesc *desc);
SINT32 Uart_Read_Line_Status(UartDesc *desc);
/********************************End Of File********************************/

/*****************************************************************************
 * Define section
 * add all #define here
 *****************************************************************************/
#define UART_RCV_TIMEOUT	1000000

/****************************************************************************
 *  global variable declaration section
 ***************************************************************************/
UartDesc uart;                                 //UART struct

/* function body */
SINT32 Uart_Disable_Irq(UartDesc *desc, uart_irq_e interrupts)
{
	UINT32 ret;
	UINT32 base = desc->port;

	ret = GET_REG(base + REG_UART_IER);
	ret &= ~interrupts;
	SET_REG(base + REG_UART_IER, ret);

	return UART_CONFIG_OK;
}

SINT32 Uart_Enable_Irq(UartDesc *desc, uart_irq_e interrupts)
{
	UINT32 ret;
	UINT32 base = desc->port;

	ret = GET_REG(base);
	ret |= interrupts;
	SET_REG(base + REG_UART_IER, ret);

	return UART_CONFIG_OK;

}

SINT32 Uart_Fifo_Config(UartDesc *desc)
{
	UINT32 ret;
	UINT32 base = desc->port;

	SET_REG(base + REG_UART_FCR, desc->fifo_ctrl);
	ret = GET_REG(base + REG_UART_IIR);

	if (ret & UART_FIFO_IS_ENABLE)
		return UART_CONFIG_FIFO_OK;
	else
		return UART_CONFIG_FIFO_ERROR;
}

SINT32 Uart_Read_Control_Status(UartDesc *desc)
{
	UINT32 base = desc->port;
	return GET_REG(base + REG_UART_USR);

}

SINT32 Uart_Set_Line_Control(UartDesc *desc)
{
	UINT32 ret;
	UINT32 base = desc->port;

	ret = Uart_Read_Control_Status(desc);
	if (ret & UART_STATUS_BUSY)
		return UART_IS_BUSY;

	SET_REG(base + REG_UART_LCR, desc->line_ctrl);
	return UART_CONFIG_LINE_OK;
}

SINT32 Uart_Read_Line_Status(UartDesc *desc)
{
	UINT32 base = desc->port;
	return GET_REG(base + REG_UART_LSR);
}

SINT32 Uart_Set_Clock_Divisor(UartDesc *desc)
{
	UINT32 low, high, ret;
	UINT32 base = desc->port;

	low = desc->baudrate & 0x00ff;
	high = (desc->baudrate & 0xff00) >> 8;

	ret = Uart_Read_Control_Status(desc);
	if (ret & UART_STATUS_BUSY)
		return UART_IS_BUSY;

	ret = GET_REG(base + REG_UART_LCR);
	// if DLAB not set
	if (!(ret & UART_LCR_DLAB_POS)) {
		ret |= UART_LCR_DLAB_POS;
		SET_REG(base + REG_UART_LCR, ret);
	}
	SET_REG(base + REG_UART_DLL, low);
	SET_REG(base + REG_UART_DLH, high);

	/* clear DLAB */
	ret = ret & 0x7f;
	SET_REG(base + REG_UART_LCR, ret);

	return UART_CONFIG_DIVISOR_OK;
}

SINT32 Uart_Read_Rx_Fifo_Level(UartDesc *desc)
{
	return GET_REG(desc->port + REG_UART_RFL);
}

SINT32 Uart_Read_Data(UartDesc *desc, UINT8 *buf, UINT32 size)
{
	UINT8 ret;

	while (size--) {
		do {
			ret = (UINT8) Uart_Read_Line_Status(desc);
			WATCHDOG_RESET();
		} while (!(ret & UART_LINE_STATUS_DR));
		*buf++ = GET_REG(desc->port + REG_UART_RBR);
	}
	return UART_READ_RX_DW_FIFO_OK;
}

SINT32 Uart_Write_Data(UartDesc *desc, const UINT8 *buf, UINT32 size)
{
	UINT8 ret;
	if ((buf == (void *) 0))
		return UART_CONFIG_PARA_ERROR;
	while (size--) {
		do {
			ret = (UINT8) Uart_Read_Control_Status(desc);
		}
		//wait txfifo is full
		while (!(ret & UART_STATUS_TFNF)); //0 means full.	 1 means not full
		SET_REG(desc->port + REG_UART_THR, *buf++);
	}
	return UART_WRITE_DATA_OK;

}

SINT32 Uart_Init(UartDesc *desc)
{

	UINT8 test_init_status = 0;
	//reset fifo
	SET_REG(desc->port + REG_UART_FCR, 6);
	test_init_status |= Uart_Set_Clock_Divisor(desc);
	test_init_status |= Uart_Set_Line_Control(desc);
	test_init_status |= Uart_Fifo_Config(desc);
	if (test_init_status != 0)
		return test_init_status;
	Uart_Disable_Irq(desc, UART_INT_ALL);
	return 0;
}

void serial_setbrg(void)
{

}

int serial_init(void)
{
	uart.port = CONSOLE_PORT;
	uart.baudrate = BAUDRATE_115200;
	uart.line_ctrl = Uart_line_8n2;
	uart.fifo_ctrl = UART_INT_RXFIFO_DEPTH_QUARTER;
	Uart_Init(&uart);
	return 0;
}

int serial_getc(void)
{
	UINT8 ret_data;
	Uart_Read_Data(&uart, &ret_data, 1);
	return (int) ret_data;
}

void serial_putc(const char c)
{
	Uart_Write_Data(&uart, (UINT8 *) &c, 1);
	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

int serial_tstc(void)
{
	UINT8 ret;
	ret = (UINT8) Uart_Read_Line_Status(&uart);
	return ((ret & UART_LINE_STATUS_DR) != 0);
}

void serial_puts(const char *s)
{
	while (*(s) != '\0')
		serial_putc(*s++);

}
