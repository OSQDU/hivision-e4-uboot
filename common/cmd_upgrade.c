#include <common.h>
#include <command.h>
#include <spi_flash.h>

#ifdef CONFIG_CMD_UPGRADE

/*crc */

typedef struct
{
	u32 magic;					//4 byte	valid
	u32 product;//4 byte	valid
	u32 version;//4 byte	valid
	u32 imgCount;//4 byte	valid
	u32 paramCount;//4 byte	invalid
	u32 hdrsize;//4 byte	invalid
	u32 flashswitch;
	u32 imgAddr[28];//29*4 byte	invalid
	u32 imgVal[29];//29*4 byte invalid

}FLASH_ROOT_HEADER_S;

typedef struct
{
	u8 name[32];				//32 byte	valid
	u32 size;//4 byte	valid
	u32 flashAdd;//4 byte	valid
	u32 memAdd;//4 byte	valid
	u32 entry;//4 byte	only valid when "type" == KEY_FILE
	u32 type;//4 byte	valid
	u32 crc; //4 byte	valid
	u32 rsvd[2];//4*3 byte	only the first 4byte valid
}FLASH_IMAGE_HEADER_S; /* total 64 bytes */

u32 crc32_table[256];

void RomBoot_MakeCRCTable(u32 seed)
{
	u32 c;
	int i = 0;
	int bit = 0;

	for(i = 0; i < 256; i++) {
		c = (u32)i;

		for(bit = 0; bit < 8; bit++) {
			if(c&1) {
				c = (c >> 1)^(seed);
			}
			else {
				c = c >> 1;
			}

		}
		crc32_table[i] = c;
	}
}

u32 RomBoot_CalCRC(u8 *buf, u32 size)
{
	u32 crc = 0xffffffff;
	while(size--) {
		crc = (crc >> 8)^(crc32_table[(crc ^ *buf++)&0xff]);
	}

	return crc;
}

u8 calc_chksum(u8 *buf, u32 size)
{
	u8 chk = 0;
	u32 i;
	for( i = 0; i < size; i++)
	chk += buf[i];

	return chk;
}

ulong offset,len,buf,ret;
int do_upgrade(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
	static struct spi_flash *flash;
	char *image_name;
	char cmd[64];
	char* tftp = "tftp";
	int flag_cmd=0,count;
	u32 image_len;
	int ret;
	//tftp commond
	image_name = getenv ( "imgname");
	if (!image_name) {
		printf("set imgname to load\n");
		return -1;
	}
	sprintf(cmd,"%s %x %s",tftp,CONFIG_FH_MEMORY_ROOT_HEADER_BASE,image_name);
	//tftp load file to ddr

	ret = run_command (cmd, flag_cmd);
	if(ret != 1) {
		printf("run command error.\n");
		return -1;
	}

	image_len = NetBootFileXferSize;
	if(image_len == 0) {
		printf(" flash.img size is 0,please check the flash.img\n");
		return -1;
	}
	printf(" flash.img size is 0x%x\n ",image_len);

	flash = spi_flash_probe(1,1,1,1);
	if(flash == NULL) {
		printf("get flash error..\n ");
		return -1;
	}
	//flash erase now is 64K allign.
	count = image_len / 0x10000;
	if(image_len % 0x10000 != 0){
		count ++;
	}
	count *=0x10000;
	flash->erase(flash,0,count);
	flash->write(flash,0,image_len,(u8*)CONFIG_FH_MEMORY_ROOT_HEADER_BASE);
	run_command ("saveenv", flag_cmd);
	printf( "flash upgrade finish \n");
	spi_flash_free(flash);

	return 0;

}

U_BOOT_CMD(

		upgrade,5,1,do_upgrade,"usage:upgrade, set imgname to load\n","upgrage:addr \n"

);

#endif
