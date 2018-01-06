#include <common.h>
#include <command.h>
#include <spi_flash.h>

#ifdef CONFIG_CMD_PRINT_PTS
#define reg_read(addr) (*((volatile unsigned int *)(addr)))
#define reg_write(addr,value) (*(volatile unsigned int *)(addr)=(value))
#define GET_REG(addr) reg_read(addr)
#define SET_REG(addr,value) reg_write(addr,value)

int print_pts(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

	static struct spi_flash *flash;
	unsigned int count;
	unsigned int cur_reg,last_reg;


	flash = spi_flash_probe(1,1,1,1);
	if(flash == NULL) {
		printf("get flash error..\n ");
		return -1;
	}




	flash->read(flash,0xa00000,4,&last_reg);
	printf("flash pts is %x\n",last_reg);

	cur_reg = GET_REG(0xec100040);
	printf("current pts is %x\n",cur_reg);

	printf("offset is %x\n",(cur_reg - last_reg));

	count=0x10000;
	flash->erase(flash,0xa00000,count);
	flash->write(flash,0xa00000,4,(u8*)&cur_reg);



	spi_flash_free(flash);




	return 0;
}

U_BOOT_CMD(
		print_pts, 5, 1, print_pts,
		"print pts and write the value to 0xA00000 in flash",
		"print pts"
		"print pts	\n"
);

#endif


