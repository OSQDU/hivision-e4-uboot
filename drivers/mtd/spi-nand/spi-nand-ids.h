
#ifndef DRIVERS_MTD_SPI_NAND_SPI_NAND_IDS_H_
#define DRIVERS_MTD_SPI_NAND_SPI_NAND_IDS_H_

#include "spi-nand.h"

enum spi_nand_device_variant {
	SPI_NAND_GENERIC,
	SPI_NAND_MT29F,
	SPI_NAND_GD5F,
};

int spi_nand_scan_id_table(struct spi_nand_chip *chip, unsigned char *id);

#endif /* DRIVERS_MTD_SPI_NAND_SPI_NAND_IDS_H_ */
