/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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

/*
 * SPI Read/Write Utilities
 */

#include <common.h>
#include <command.h>
#include <spi.h>

#include <asm/io.h>

/*-----------------------------------------------------------------------
 * Definitions
 */

#ifndef MAX_SPI_BYTES
#   define MAX_SPI_BYTES 32	/* Maximum number of bytes we can handle */
#endif

#ifndef CONFIG_DEFAULT_SPI_BUS
#   define CONFIG_DEFAULT_SPI_BUS	0
#endif
#ifndef CONFIG_DEFAULT_SPI_MODE
#   define CONFIG_DEFAULT_SPI_MODE	SPI_MODE_0
#endif

/*
 * Values from last command.
 */

static int __do_spi(int device, unsigned char* dout, unsigned char* din, int bitlen)
{
	int   bus, cs;
	struct spi_slave *slave;
	int   j;
	int   rcode = 0;
	/* FIXME: Make these parameters run-time configurable */
	bus = device / 2;
	cs = device % 2;
	slave = spi_setup_slave(bus, cs, 1000000, CONFIG_DEFAULT_SPI_MODE);
	if (!slave) {
		printf("Invalid device %d, giving up.\n", device);
		return 1;
	}

	debug ("spi chipsel = %08X\n", device);

	spi_claim_bus(slave);
	if(spi_xfer(slave, bitlen, dout, din,
				SPI_XFER_BEGIN | SPI_XFER_END) != 0) {
		printf("Error with the SPI transaction.\n");
		rcode = 1;
	} else {
		if (bitlen < 32*8) {
			for (j = 0; j < ((bitlen + 7) / 8); j++) {
				printf("%02X", din[j]);
			}
			printf("\n");
		}
	}
	spi_release_bus(slave);
	spi_free_slave(slave);

	return rcode;
}

int _do_spi_dump (int flag, int argc, char *argv[])
{
	char  *cp = 0;
	uchar tmp;
	unsigned long addr;
	void *buf;
	char *endp;
	int   j;
	int device = -1;
	int bitlen = -1;
	unsigned char dout[MAX_SPI_BYTES];
	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		addr = simple_strtoul(argv[1], &endp, 16);
		if (*argv[1] == 0 || *endp != 0)
			return 1;
		if (argc >= 3)
			device = simple_strtoul(argv[2], NULL, 10);
		if (argc >= 4)
			bitlen = simple_strtoul(argv[3], NULL, 10);
		if (argc >= 5) {
			cp = argv[4];
			for(j = 0; *cp; j++, cp++) {
				tmp = *cp - '0';
				if(tmp > 9)
					tmp -= ('A' - '0') - 10;
				if(tmp > 15)
					tmp -= ('a' - 'A');
				if(tmp > 15) {
					printf("Hex conversion error on %c, giving up.\n", *cp);
					return 1;
				}
				if((j % 2) == 0)
					dout[j / 2] = (tmp << 4);
				else
					dout[j / 2] |= tmp;
			}
		}
	}
	buf = map_physmem(addr, bitlen/8, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	if (bitlen < 0)  {
		printf("Invalid bitlen %d, giving up.\n", bitlen);
		return 1;
	}
	return __do_spi(device, dout, (unsigned char*)buf, bitlen);
}

int _do_spi (int flag, int argc, char *argv[])
{
	char  *cp = 0;
	uchar tmp;
	int   j;
	int device = -1;
	int bitlen = -1;
	unsigned char dout[MAX_SPI_BYTES];
	unsigned char din[MAX_SPI_BYTES];
	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		if (argc >= 2)
			device = simple_strtoul(argv[1], NULL, 10);
		if (argc >= 3)
			bitlen = simple_strtoul(argv[2], NULL, 10);
		if (argc >= 4) {
			cp = argv[3];
			for(j = 0; *cp; j++, cp++) {
				tmp = *cp - '0';
				if(tmp > 9)
					tmp -= ('A' - '0') - 10;
				if(tmp > 15)
					tmp -= ('a' - 'A');
				if(tmp > 15) {
					printf("Hex conversion error on %c, giving up.\n", *cp);
					return 1;
				}
				if((j % 2) == 0)
					dout[j / 2] = (tmp << 4);
				else
					dout[j / 2] |= tmp;
			}
		}
	}

	if ((bitlen < 0) || (bitlen >  (MAX_SPI_BYTES * 8))) {
		printf("Invalid bitlen %d, giving up.\n", bitlen);
		return 1;
	}
	return __do_spi(device, dout, din, bitlen);
}

/*
 * SPI read/write
 *
 * Syntax:
 *   spi {dev} {num_bits} {dout}
 *     {dev} is the device number for controlling chip select (see TBD)
 *     {num_bits} is the number of bits to send & receive (base 10)
 *     {dout} is a hexadecimal string of data to send
 * The command prints out the hexadecimal string received via SPI.
 */
static int do_spi (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "dump") == 0)
		return _do_spi_dump(flag, argc - 1, argv + 1);

	if (_do_spi(flag, argc, argv) )
		goto usage;
	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}
/***************************************************/

U_BOOT_CMD(
	sspi,	6,	1,	do_spi,
	"SPI utility commands",
	"sspi <device> <bit_len> <dout> - Send <bit_len> bits from <dout> out the SPI\n"
	"\t<device>  - Identifies the chip select of the device\n"
	"\t<bit_len> - Number of bits to send (base 10)\n"
	"\t<dout>    - Hexadecimal string that gets sent\n"
	"sspi dump <addr> <device> <bit_len> <dout> - Send <bit_len> bits from <dout> \n"
	"                                             out the SPI to memory at <addr>\n"
);
