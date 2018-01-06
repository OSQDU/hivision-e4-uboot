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
#include <i2c.h>
#include <malloc.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/arch/hardware.h>

#define		I2C_REG_BASE		(0xf0b00000)

#define     REG_I2C_CON             (I2C_REG_BASE + 0x0000)
#define     REG_I2C_TAR             (I2C_REG_BASE + 0x0004)
#define     REG_I2C_SAR             (I2C_REG_BASE + 0x0008)
#define     REG_I2C_HS_MADDR        (I2C_REG_BASE + 0x000C)
#define     REG_I2C_DATA_CMD        (I2C_REG_BASE + 0x0010)
#define     REG_I2C_SS_SCL_HCNT     (I2C_REG_BASE + 0x0014)
#define     REG_I2C_SS_SCL_LCNT     (I2C_REG_BASE + 0x0018)
#define     REG_I2C_FS_SCL_HCNT     (I2C_REG_BASE + 0x001C)
#define     REG_I2C_FS_SCL_LCNT     (I2C_REG_BASE + 0x0020)
#define     REG_I2C_HS_SCL_HCNT     (I2C_REG_BASE + 0x0024)
#define     REG_I2C_HS_SCL_LCNT     (I2C_REG_BASE + 0x0028)
#define     REG_I2C_INTR_STAT       (I2C_REG_BASE + 0x002c)
#define     REG_I2C_INTR_MASK       (I2C_REG_BASE + 0x0030)
#define     REG_I2C_RAW_INTR_STAT   (I2C_REG_BASE + 0x0034)
#define     REG_I2C_RX_TL           (I2C_REG_BASE + 0x0038)
#define     REG_I2C_TX_TL           (I2C_REG_BASE + 0x003c)
#define     REG_I2C_CLR_INTR        (I2C_REG_BASE + 0x0040)
#define     REG_I2C_ENABLE          (I2C_REG_BASE + 0x006c)
#define     REG_I2C_STATUS          (I2C_REG_BASE + 0x0070)
#define     REG_I2C_TXFLR           (I2C_REG_BASE + 0x0074)
#define     REG_I2C_RXFLR           (I2C_REG_BASE + 0x0078)
#define     REG_I2C_DMA_CR          (I2C_REG_BASE + 0x0088)
#define     REG_I2C_DMA_TDLR        (I2C_REG_BASE + 0x008c)
#define     REG_I2C_DMA_RDLR        (I2C_REG_BASE + 0x0090)

#define I2C_MODE_SIL9134            (0x10)
#define I2C_MODE_SENSOR_8BIT        (0x00)
#define I2C_MODE_SENSOR_16BIT       (0x03)
#define I2C_HANG_CYC                 100000

/* i2c interrput definition */
#define M_GEN_CALL      (1<<11)
#define M_START_DET     (1<<10)
#define M_STOP_DET      (1<<9)
#define M_ACTIVITY      (1<<8)
#define M_RX_DONE       (1<<7)
#define M_TX_ABRT       (1<<6)
#define M_RD_REQ        (1<<5)
#define M_TX_EMPTY      (1<<4)
#define M_TX_OVER       (1<<3)
#define M_RX_FULL       (1<<2)
#define M_RX_OVER       (1<<1)
#define M_RX_UNDER      (1<<0)
#define M_NONE          (0)

#define tfifo_empty           0x05
#define rfifo_nempty          0x08
#define I2C_HANG_CYC_eeprom   20000

enum BUS_STATUS {
	I2C_BUSY,
	I2C_IDLE

};
enum RESULT {
	SUCCESS,
	FAILURE

};
enum ENABLE_SET {
	DISABLE,
	ENABLE

};
enum SPEED_MODE {
	SSPEED = 1,
	FSPEED = 2,
	HSPEED = 3,
};

#define I2c_SetTxRxTl(txtl,rxtl)    \
    SET_REG(REG_I2C_TX_TL, txtl);   \
    SET_REG(REG_I2C_RX_TL, rxtl)
#define I2c_SetDeviceId(deviceID)   SET_REG(REG_I2C_TAR,deviceID)
#define I2c_Read()   				(GET_REG(REG_I2C_DATA_CMD)&0xff)
#define I2c_Write(data)  			SET_REG(REG_I2C_DATA_CMD,data)
#define I2c_IsActiveMst()   		(GET_REG(REG_I2C_STATUS)>>5 & 1)
#define I2c_SetCon(config)      	SET_REG(REG_I2C_CON,config)
#define I2c_Status()     			GET_REG(REG_I2C_STATUS)
#define I2c_SetTar(id)      		SET_REG(REG_I2C_TAR,id)
#define I2c_SetIntrMask(mask)   	SET_REG(REG_I2C_INTR_MASK,mask)
#define I2c_TxEmpty()   			(GET_REG(REG_I2C_RAW_INTR_STAT) & M_TX_EMPTY)
#define I2c_RxFull()    			(GET_REG(REG_I2C_RAW_INTR_STAT) & M_RX_FULL)

/* register define */
typedef union {
	struct {
		UINT32 MASTER_MODE      	: 1;
		UINT32 SPEED            	: 2;
		UINT32 IC_10BITADDR_SLAVE   : 1;
		UINT32 IC_10BITADDR_MASTER  : 1;
		UINT32 IC_RESTART_EN        : 1;
		UINT32 IC_SLAVE_DISABLE     : 1;
		UINT32 reserved_31_7        : 25;
	} x;
	UINT32 dw;
} Reg_I2c_Con;

#define I2C_CLOCK   13500000    //13.5MHZ

/* standard speed mode , for
 *  100 kbps*/
#define SSCL_HIGH       6000	//ns
#define SSCL_LOW        6700	//ns

/* fast speed mode , for 400kbps */
#define FSCL_HIGH       600 	//ns
#define FSCL_LOW        1300

/* high speed mode , for 3.4Mbps */
#define HSCL_HIGH       60  	//ns
#define HSCL_LOW        120

/* i2c   status                  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : I2c_SetSpeed
* Description    : set the I2C bus speed mode
* Input          : model:SSPEED:stander speed100K/s    FSPEED:fast speed  400K/s  HSPEED:hige speed 3.4M/s
* Output         : None
* Return         : None
*
 *******************************************************************************/
static void I2c_SetSpeed(UINT8 mode)//int spe,
{
	UINT16 scl_low, scl_high;
	UINT8 SpeedMode = mode;

	switch (SpeedMode) {
	case SSPEED:
		scl_low = (UINT16)((SSCL_LOW * (I2C_CLOCK / 1000000)) / 1000);
		scl_high = (UINT16)((SSCL_HIGH * (I2C_CLOCK / 1000000)) / 1000);
		SET_REG(REG_I2C_SS_SCL_HCNT, scl_high); //IC_SS_SCL_HCNT
		SET_REG(REG_I2C_SS_SCL_LCNT, scl_low);  //IC_SS_SCL_LCNT
		break;

	case FSPEED:
		scl_low = (UINT16)((FSCL_LOW * (I2C_CLOCK / 1000000)) / 1000);
		scl_high = (UINT16)((FSCL_HIGH * (I2C_CLOCK / 1000000)) / 1000);
		SET_REG(REG_I2C_FS_SCL_HCNT, scl_high); //IC_SS_SCL_HCNT
		SET_REG(REG_I2C_FS_SCL_LCNT, scl_low);  //IC_SS_SCL_LCNT
		break;

	case HSPEED:
		scl_low = (UINT16)((HSCL_LOW * (I2C_CLOCK / 1000000)) / 1000);
		scl_high = (UINT16)((HSCL_HIGH * (I2C_CLOCK / 1000000)) / 1000);
		SET_REG(REG_I2C_HS_SCL_HCNT, scl_high); //IC_SS_SCL_HCNT
		SET_REG(REG_I2C_HS_SCL_LCNT, scl_low);  //IC_SS_SCL_LCNT
		break;

	default	:
		break;
	}
}

/*******************************************************************************
* Function Name  : I2c_Enable
* Description    : set the I2C bus enable
* Input          : enable:1 enable   0  disable
* Output         : None
* Return         : None
*
 *******************************************************************************/
static  void I2c_Enable(int enable)
{
	//enable IIC  bus
	SET_REG(REG_I2C_ENABLE, enable);
}

/*******************************************************************************
* Function Name  : I2c_Disable
* Description    : disable I2C bus
* Input          : None
* Output         : None
* Return         : success or failure
*
 *******************************************************************************/
static int I2c_Disable(void)
{
	//disable IIC bus
	I2c_Enable(DISABLE);

	return 0;
}

static void I2c_Init(UINT16 slave_addr, enum SPEED_MODE speed, int txtl, int rxtl)
{
	Reg_I2c_Con config;

	config.dw = 0;
	I2c_Disable();

	config.x.IC_SLAVE_DISABLE = 1;
	config.x.IC_RESTART_EN = 0;
	config.x.IC_10BITADDR_MASTER = 0;
	config.x.IC_10BITADDR_SLAVE = 0;
	config.x.SPEED = speed;     // standard
	config.x.MASTER_MODE = 1;

	I2c_SetCon(config.dw);
	I2c_SetTar(slave_addr);

	I2c_SetSpeed(speed);
	I2c_SetIntrMask(0x0);     //
	I2c_SetTxRxTl(txtl, rxtl);      // tx empty threshold 4, rx full threshold 1

	I2c_Enable(ENABLE);
}

//core interface..
void flush_rx(void)
{

}

void i2c_init(int speed, int slaveadd)
{

	I2c_Init(slaveadd, SSPEED, 6, 6);
	printf("fh81 i2c init done.\n");
}

int i2c_probe(u_int8_t chip)
{

	printf("fh81 i2c probe done.\n");
	return 0;
}

int i2c_read(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	int i, j;

	I2c_Init(0x51, SSPEED, 6, 6);
	if (alen == 1)
		I2c_Write((UINT8 )addr);
	else {
		I2c_Write((UINT8 )addr >> 8);
		I2c_Write((UINT8 )addr);
	}

	for (i = 0; i < len; i++) {
		if (i == (len - 1)) {
			I2c_Write(0x300);
			for (j = 0; j < I2C_HANG_CYC_eeprom; j++) {
				if (0x8 == (I2c_Status() & 0x9)) {
					if (len == 1)
						buf[0] = (UINT8) I2c_Read();
					else
						buf[i - 1] = (UINT8) I2c_Read();
					if (len > 1) {
						buf[i] = I2c_Read();
						break;
					}
				}
			}
		} else {
			I2c_Write(0x100);
			if (i > 0) {
				for (j = 0; j < I2C_HANG_CYC_eeprom; j++) {
					if (0x9 == (I2c_Status() & 0x9)) {
						buf[i - 1] = I2c_Read();
						break;
					}
				}
			}
		}

	}

	while (0x6 != GET_REG(REG_I2C_STATUS))
		;

	return (0);
}

int i2c_write(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	int i;
	int addrH;

	I2c_Init(0x51, SSPEED, 6, 6);
	if (alen == 1) {	// less than 2KB

	} else

		addrH = (addr >> 8) & 0xff;

	if (alen == 2)
		I2c_Write((UINT8 )addrH);
	I2c_Write((UINT8 )addr & 0xff);
	for (i = 0; i < len - 1; i++) {
		while (!(I2c_Status() & 0x2))
			;
		I2c_Write((UINT8 )buf[i]);
	}
	while (!(I2c_Status() & 0x2))
		;
	I2c_Write((UINT8 )buf[len - 1] | 0x200);
	while (4 != (I2c_Status() & 0x5))
		;

	return (0);
}
