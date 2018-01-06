/*
 * Interface to SPI flash
 *
 * Copyright (C) 2008 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include <spi.h>
#include <linux/types.h>

struct spi_flash_region {
	unsigned int	count;
	unsigned int	size;
};

struct spi_flash {
	struct spi_slave *spi;

	const char	*name;

	u32		size;
	u32 	sector_size; //FIXME: + for mtd support, enalbe in winbond ,to add to others

	int		(*read)(struct spi_flash *flash, u32 offset,
				size_t len, void *buf);
	int		(*write)(struct spi_flash *flash, u32 offset,
				size_t len, const void *buf);
	int		(*erase)(struct spi_flash *flash, u32 offset,
				size_t len);
    int     (*protect)(struct spi_flash *flash, int on);
};

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);
void spi_flash_free(struct spi_flash *flash);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
    if(offset + len <= flash->size)
    {
        return flash->read(flash, offset, len, buf);
    }
    else
    {
        printf("flash read address is out of range\n");
        return -1;
    }
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
    if(offset + len <= flash->size)
    {
        return flash->write(flash, offset, len, buf);
    }
    else
    {
        printf("flash write address is out of range\n");
        return -1;
    }
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{
    if(offset + len <= flash->size)
    {
        return flash->erase(flash, offset, len);
    }
    else
    {
        printf("flash erase address is out of range\n");
        return -1;
    }
}

static inline int spi_flash_protect(struct spi_flash *flash, int on)
{
    return flash->protect(flash, on);
}

#endif /* _SPI_FLASH_H_ */
