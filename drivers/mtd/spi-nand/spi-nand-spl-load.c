/*
 * spi-nand-spl-load.c
 *
 * Copyright (C) 2008 fullhan Corporation
 * Licensed under the GPL-2 or later.
 *
 */

#include <common.h>
#include <spi.h>
#include <nand.h>
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
typedef struct {
	unsigned int magic;                  //4 byte    valid
	unsigned int product;                //4 byte    valid
	unsigned int version;                //4 byte    valid
	unsigned int imgCount;               //4 byte    valid
	unsigned int paramCount;             //4 byte    invalid
	unsigned int hdrsize;                //4 byte    invalid
	unsigned int flashswitch;
	unsigned int imgAddr[28];            //29*4 byte invalid
	unsigned int imgVal[29];             //29*4 byte invalid
} FLASH_ROOT_HEADER_S; /* total 256 bytes */

typedef struct {
	unsigned char name[32];               //32 byte   valid
	unsigned int size;                   //4 byte    valid
	unsigned int flashAdd;                   //4 byte    valid
	unsigned int memAdd;                 //4 byte    valid
	unsigned int entry;       //4 byte    only valid when "type" == KEY_FILE
	unsigned int type;                   //4 byte    valid
	unsigned int crc;                    //4 byte    valid
	//unsigned int    flashswitch;                    //4 byte    valid
	unsigned int rsvd[2];             //4*3 byte  only the first 4byte valid
} FLASH_IMAGE_HEADER_S; /* total 64 bytes */

#define HEAD_SIZE sizeof(FLASH_ROOT_HEADER_S)
#define IMG_SIZE sizeof(FLASH_IMAGE_HEADER_S)

static unsigned int fh_get_image_offset(nand_info_t *nand)
{
	FLASH_ROOT_HEADER_S rootHeader;
	int err, i, size;

	size = HEAD_SIZE;
	err = nand_read_skip_bad(nand, 0, &size,
	                (u_char *) &rootHeader);
	if (err) {
		printf("ERROR: get fh flash root header error: %d\n", err);
		return 0;
	}

	for (i = 0; i < rootHeader.imgCount; i++) {
		FLASH_IMAGE_HEADER_S imageHeader;

		size = IMG_SIZE;
		err = nand_read_skip_bad(nand, HEAD_SIZE + i * IMG_SIZE,
		         &size, (void *) &imageHeader);

		if (err) {
			printf("ERROR: get fh flash img header error: %d\n",
			                err);
			return 0;
		}

		if (UBOOT_IMAGE_TYPE == imageHeader.type) {
			return imageHeader.flashAdd;
		}
	}

	return 0;
}

int spl_spinand_load_image(void)
{
	int err = 0, size;
	nand_info_t *nand;
	struct image_header *header;
	unsigned int end_magic;
	unsigned int uboot_offset;

	nand = &nand_info[nand_curr_device];

	/* use CONFIG_SYS_TEXT_BASE as temporary storage area */
	header = (struct image_header *) (CONFIG_SYS_UBOOT_BASE);

#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || spi_nand_load_image_os(nand, header))
#endif
	{
		uboot_offset = fh_get_image_offset(nand);

		/* Load u-boot, mkimage header is 64 bytes. */
		size = 64;
		err = nand_read_skip_bad(nand, uboot_offset, &size,
		                (u_char *) header);
		if (err)
			return err;

		err = spl_parse_image_header(header);
		if (err)
			return err;

		size = 4;
		err = nand_read_skip_bad(nand,
				uboot_offset +spl_image.size - 4,
				&size,
				(u_char *) &end_magic);
		if (!err) {
			if (end_magic != END_MAGIC) {
				printf("incorrect end magic: 0x%x\n",
						end_magic);
				return -EIO;
			}
		}

		if (!strcmp("rtthread", spl_image.name)) {
			size = spl_image.size;
			err = nand_read_skip_bad(nand,
					uboot_offset + 0x40,
					&size,
					(u_char *) spl_image.load_addr);
		} else {
			size = spl_image.size;
			err = nand_read_skip_bad(nand,
					uboot_offset,
					&size,
					(u_char *) spl_image.load_addr);
		}
	}

	return err;
}
