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

#ifndef __CONFIG_H
#define __CONFIG_H

//#define DEBUG
/* support device tree  */
//#define CONFIG_OF_LIBFDT
#define CONFIG_FH
#define CONFIG_SOC_FH62
/* no flash support */
#define CONFIG_SYS_NO_FLASH


# ifdef CONFIG_SYS_NO_FLASH
#  undef CONFIG_CMD_FLASH
#  undef CONFIG_CMD_IMLS
# else
#  define CONFIG_CMD_JFFS2
# endif
# ifdef CONFIG_CMD_JFFS2
#  define CONFIG_JFFS2_SUMMARY

#  define CONFIG_SYS_MAX_FLASH_BANKS	0
#  define CONFIG_SYS_MAX_FLASH_SECT		256
#  define CONFIG_SYS_FLASH_BASE			0
# endif


#define CONFIG_SPI_FLASH_MTD
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT			"nor0=spi0.0"
#define MTDPARTS_DEFAULT		"mtdparts=spi0.0:64k(bootstrap)," \
								"64k(u-boot-env),192k(u-boot)," \
								"4M(kernel),-(user)"


#define CONFIG_SYS_HZ		1000

#define CONFIG_ARM1176	1	/* This is an ARM1176 Core	*/

#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1
#define CONFIG_PHYMODE_TAG      1

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT

#define CONFIG_CMDLINE_EDITING 	/* command history	*/
#define CONFIG_AUTO_COMPLETE  	/* tab complete */
/*
 * Hardware drivers
 */
#define CONFIG_FH81_SERIAL
#define CONFIG_FH_SERIAL_CONSOLE 	1


#define CONFIG_FH62

#define CONFIG_FH81_GMAC

#ifdef CONFIG_FH62
#define SD0_CARD_EN
#define CONFIG_ENC28J60
#undef CONFIG_FH81_GMAC
#endif

#ifdef FPGA_OLD
#define CONFIG_FH81_TIMER1_CLOCK	(27000000)
#else
#define CONFIG_FH81_TIMER1_CLOCK	(1000000)
#endif

#define CONFIG_FH81_PTS_CLOCK	(1000000)

#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2

#define CONFIG_BOOTDELAY			3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE	1
#define CONFIG_BOOTP_BOOTPATH		1
#define CONFIG_BOOTP_GATEWAY		1
#define CONFIG_BOOTP_HOSTNAME		1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_DATE

#define CONFIG_CMD_SF
#define CONFIG_CMD_PING		1
#define CONFIG_CMD_DHCP		1

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE	0xA0000000
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_SIZE			0x01000000	/* 16 megs */

/* spi */
#define CONFIG_SPI
#define CONFIG_CMD_SPI
#define CONFIG_FH81_SPI
#define CONFIG_SPI1_ENABLE
#define SPI_MAX_BUS_NUM			1
#define SPI0_PORT			0
#define SPI1_PORT			1
#define SPI_MAX_CS_NUM			2

#define SPI_FLASH_CS0                   54
#define SPI_FLASH_CS1                   55

/* SPIFlash */
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ALL
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_BOYA

#define CONFIG_SPI_FLASH_DEFAULT

#define CONFIG_SYS_SPI_WRITE_TOUT	(5*CONFIG_SYS_HZ)
#define CONFIG_SF_DEFAULT_SPEED		6750000   /* 6.75M */

/* I2C RTC*/
#define CONFIG_FH81_I2C
#define CONFIG_RTC_PCF8563
#define CONFIG_CMD_DATE
#define CONFIG_CMD_I2C
#ifndef CONFIG_SYS_I2C_RTC_ADDR
# define CONFIG_SYS_I2C_RTC_ADDR	0x51
#endif
#define CONFIG_SYS_I2C_SPEED		80000
/*
 * fh81 board specific data
 */

/* total memory available to uboot */
#define CONFIG_SYS_UBOOT_SIZE		(1024 * 1024)


/* Ethernet */
//#define CONFIG_MACB			1
#define CONFIG_PHY_RTL8201      1
//#define CONFIG_PHY_TI83848    1
//#define CONFIG_PHY_IP101G     1
#define CONFIG_PHY_MODE         "MII"
#define CONFIG_NET_MULTI		1
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R		1
#define CONFIG_SYS_RX_ETH_BUFFER	5


#define CONFIG_IPADDR		10.81.81.81
#define CONFIG_SERVERIP		10.81.0.1
#define CONFIG_ETHADDR		10:20:30:40:50:60
#define CONFIG_OVERWRITE_ETHADDR_ONCE

/*
 * Boot option
 */
#define CONFIG_SYS_LOAD_ADDR			0xA0000000	/* load kernel address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			(PHYS_SDRAM + 0x00400000)


/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_SPI_FLASH		1
#define CONFIG_SYS_SPIFLASH_BASE_ADDR  	0
/* The following #defines are needed to get flash environment right */
#define CONFIG_SYS_MONITOR_BASE			(CONFIG_SYS_SPIFLASH_BASE_ADDR + 0x20000)
#define CONFIG_ENV_OFFSET				0x40000
#define CONFIG_ENV_ADDR					(CONFIG_SYS_SPIFLASH_BASE_ADDR + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE					0x10000 	/* Total Size of Environment Sector 64k */
#define CONFIG_ENV_SECT_SIZE			0x1000 		/* Env sector Size 4k */
#if TFTP_BOOT
#define CONFIG_BOOTCOMMAND	"tftp 0xA0000000 rtthread.bin; go A0000000"
#else
#define CONFIG_BOOTCOMMAND	"sf probe 0; sf read 0xA0000000 0x80000 0x200000; go A0000000"
#endif
#define CONFIG_BOOTARGS		""

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }
/*
 * Shell Settings
 */
#define CONFIG_SYS_PROMPT		"U-Boot # "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		32 // 15 -> 32
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_ENV_OVERWRITE	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif


/* upgrade */
#define CRC_SEED	0xEDB88320
#define CONFIG_FH_MEMORY_ROOT_HEADER_BASE 	0xA1000000
#define CONFIG_FH_MEMORY_IMG_HEADER_BASE  (CONFIG_FH_MEMORY_ROOT_HEADER_BASE+0x100)
#define FLASH_IMAGE_HEADER_SIZE 0x40
#define CONFIG_FH_MEMORY_IMAGE_BASE (CONFIG_FH_MEMORY_ROOT_HEADER_BASE+0x1000)

//wdt
#ifndef CONFIG_SPL_BUILD
#define CONFIG_FH81_WDT
#define CONFIG_HW_WATCHDOG
#define CONFIG_CMD_WDT
#endif

#ifdef CONFIG_SELFTEST_CMD
#define CONFIG_CMD_UPGRADE
#define CONFIG_CMD_PRINT_PTS
//cpu test case
#define CONFIG_CMD_CPU_TEST
//ddr test case
#define CONFIG_CMD_DDR_FREQ_TEST
#define CONFIG_CMD_REFCOUNT
#endif

//MMC
#define CONFIG_MMC
#ifdef CONFIG_MMC
# define __LITTLE_ENDIAN                         1
# define SD0_POWER_CONTROL_GPIO                  51
# define CONFIG_DOS_PARTITION	1
# define CONFIG_CMD_FAT          1
# define CONFIG_FH81_MMC			1
# define CONFIG_GENERIC_MMC		1
# define CONFIG_CMD_MMC
/* mmc options default using: SDC0,4BIT*/
/* #define CONFIG_MMC_SDC1 */
# define CONFIG_MMC_1BIT
#endif

#define CONFIG_SYS_UBOOT_BASE        (CONFIG_SYS_SDRAM_BASE + 0x00800000)

// SPL
#define CONFIG_SPL
#ifdef CONFIG_SPL
# define CONFIG_SPL_FRAMEWORK
# define CONFIG_SPL_SERIAL_SUPPORT
# define CONFIG_SPL_ENV_SUPPORT
# define CONFIG_SPL_LIBGENERIC_SUPPORT
# define CONFIG_SPL_LIBCOMMON_SUPPORT

# define CONFIG_SPL_UBOOT_BASE        (CONFIG_SYS_SDRAM_BASE + 0x00400000)

#define CONFIG_SPL_YMODEM_SUPPORT
# define CONFIG_SPL_SPI_SUPPORT
#endif

#endif
