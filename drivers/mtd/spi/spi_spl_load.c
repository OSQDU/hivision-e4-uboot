/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/errno.h>
#include <spl.h>

#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED    1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE     SPI_MODE_3
#endif
#ifndef CONFIG_SF_DEFAULT_CS
# define CONFIG_SF_DEFAULT_CS       0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
# define CONFIG_SF_DEFAULT_BUS      0
#endif

#define END_MAGIC                   (0x55aa55aa)
#define UBOOT_IMAGE_TYPE            (0x12)

//fh flash head info
typedef struct
{
    unsigned int  magic;                  //4 byte    valid
    unsigned int  product;                //4 byte    valid
    unsigned int  version;                //4 byte    valid
    unsigned int  imgCount;               //4 byte    valid
    unsigned int  paramCount;             //4 byte    invalid
    unsigned int  hdrsize;                //4 byte    invalid
    unsigned int  flashswitch;
    unsigned int  imgAddr[28];            //29*4 byte invalid
    unsigned int  imgVal[29];             //29*4 byte invalid
}FLASH_ROOT_HEADER_S;                   /* total 256 bytes */

typedef struct
{
    unsigned char   name[32];               //32 byte   valid
    unsigned int  size;                   //4 byte    valid
    unsigned int  flashAdd;                   //4 byte    valid
    unsigned int  memAdd;                 //4 byte    valid
    unsigned int  entry;                  //4 byte    only valid when "type" == KEY_FILE
    unsigned int  type;                   //4 byte    valid
    unsigned int  crc;                    //4 byte    valid
    //unsigned int    flashswitch;                    //4 byte    valid
    unsigned int  rsvd[2];                //4*3 byte  only the first 4byte valid
}FLASH_IMAGE_HEADER_S;              /* total 64 bytes */




static unsigned int fh_get_image_offset(struct spi_flash *flash)
{
    FLASH_ROOT_HEADER_S rootHeader;
    int err, i;

    err = spi_flash_read(flash, 0, sizeof(FLASH_ROOT_HEADER_S),
                 (void *)&rootHeader);

    if (err)
    {
        printf("ERROR: get fh flash root header error: %d\n", err);
        return 0;
    }

    for (i = 0; i < rootHeader.imgCount; i++)
    {
        FLASH_IMAGE_HEADER_S imageHeader;

        err = spi_flash_read(flash,
                sizeof(FLASH_ROOT_HEADER_S) + i * sizeof(FLASH_IMAGE_HEADER_S),
                sizeof(FLASH_IMAGE_HEADER_S),
                     (void *)&imageHeader);

        if (err)
        {
            printf("ERROR: get fh flash img header error: %d\n", err);
            return 0;
        }

        if (UBOOT_IMAGE_TYPE == imageHeader.type)
        {
            return imageHeader.flashAdd;
        }
    }

    return 0;
}

#ifdef CONFIG_SPL_OS_BOOT
/*
 * Load the kernel, check for a valid header we can parse, and if found load
 * the kernel and then device tree.
 */
static int spi_load_image_os(struct spi_flash *flash,
			     struct image_header *header)
{
	/* Read for a header, parse or error out. */
	spi_flash_read(flash, CONFIG_SYS_SPI_KERNEL_OFFS, 0x40,
		       (void *)header);

	if (image_get_magic(header) != IH_MAGIC)
		return -1;

	spl_parse_image_header(header);

	spi_flash_read(flash, CONFIG_SYS_SPI_KERNEL_OFFS,
		       spl_image.size, (void *)spl_image.load_addr);

	/* Read device tree. */
	spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
		       CONFIG_SYS_SPI_ARGS_SIZE,
		       (void *)CONFIG_SYS_SPL_ARGS_ADDR);

	return 0;
}
#endif

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
int spl_spi_load_image(void)
{
	int err = 0;
	struct spi_flash *flash;
	struct image_header *header;
	unsigned int end_magic;
	unsigned int spi_uboot_offset;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 */

	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		puts("SPI probe failed.\n");
		return -ENODEV;
	}

	/* use CONFIG_SYS_TEXT_BASE as temporary storage area */
	header = (struct image_header *)(CONFIG_SYS_UBOOT_BASE);

#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || spi_load_image_os(flash, header))
#endif
	{
	    spi_uboot_offset = fh_get_image_offset(flash);

		/* Load u-boot, mkimage header is 64 bytes. */
		err = spi_flash_read(flash, spi_uboot_offset, 0x40,
				     (void *)header);
		if (err)
			return err;

		err = spl_parse_image_header(header);
		if(!err)
		{
		    err = spi_flash_read(flash, spi_uboot_offset + spl_image.size - 4, 0x4,
		            &end_magic);

		    if(!err)
		    {
		        if(end_magic != END_MAGIC)
		        {
		            printf("incorrect end magic: 0x%x\n", end_magic);
		            return -EIO;
		        }
		    }

	        if (!strcmp ("rtthread", spl_image.name))
	        {
	            err = spi_flash_read(flash, spi_uboot_offset + 0x40,
	                                   spl_image.size, (void *)spl_image.load_addr);
	        }
	        else
	        {
                err = spi_flash_read(flash, spi_uboot_offset,
                           spl_image.size, (void *)spl_image.load_addr);
	        }
		}

	}

	return err;
}
