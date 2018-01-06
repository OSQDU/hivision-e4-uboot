/*
 * cmd_ddr_freq_test.c
 *
 *  Created on: Mar 13, 2015
 *      Author: duobao
 */



#include <common.h>
#include <command.h>
#include <asm/io.h>
#ifdef CONFIG_CMD_DDR_FREQ_TEST

#define DDR_TEST_300M		(1)
#define DDR_TEST_216M		(2)





typedef char				SINT8;
typedef short				SINT16;
typedef int					SINT32;
typedef long long			SINT64;
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
typedef unsigned long long	UINT64;

typedef float          ieee_single;
typedef double         ieee_double;

typedef unsigned long  boolean;

#define lift_shift_bit_num(bit_num)			(1<<bit_num)

#define reg_read(addr) (*((volatile UINT32 *)(addr)))
#define reg_write(addr,value) (*(volatile UINT32 *)(addr)=(value))

#define GET_REG(addr) reg_read(addr)
#define SET_REG(addr,value) reg_write(addr,value)
#define SET_REG_M(addr,value,mask) reg_write(addr,(reg_read(addr)&(~(mask)))|((value)&(mask)))
#define SET_REG_B(addr,element,highbit,lowbit) SET_REG_M((addr),((element)<<(lowbit)),(((1<<((highbit)-(lowbit)+1))-1)<<(lowbit)))

#define GET_REG8(addr) (*((volatile UINT8 *)(addr)))
#define SET_REG8(addr,value) (*(volatile UINT8 *)(addr)=(value))

#define read_reg(addr)  (*((volatile uint32 *)(addr)))
#define write_reg(addr, reg)  (*((volatile uint32 *)(addr))) = (uint32)(reg)
#define inw(addr)  (*((volatile uint32 *)(addr)))
#define outw(addr, reg)  (*((volatile uint32 *)(addr))) = (uint32)(reg)

#if(1)
int ddr_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])

{
	unsigned int test_no,ret;

	char *endp;

	//argv[0] = wdt,argv[1]: value the function need to handle


	test_no = simple_strtoul(argv[1], &endp, 0);
	switch(test_no){
	case DDR_TEST_300M:


		ret = GET_REG(0xf0000020);
		ret &= ~ 1<<24;
		SET_REG(0xf0000020,ret);

		printf("0xf0000020 value: %x\n",ret);
		printf("ddr changed to 300M..\n");
		break;
	case DDR_TEST_216M:

		ret = GET_REG(0xf0000020);
		ret |= 1<<24;
		SET_REG(0xf0000020,ret);

		printf("0xf0000020 value: %x\n",ret);
		printf("ddr changed to 216M..\n");
		break;


	default :
		printf("undefined cmd..\n");

	}



	//printf("wdt time out:%d\n",user_time_ms);


//printf("do wdt \n");

return 0;



}

U_BOOT_CMD(

		ddr_test,	5,	1,	ddr_test,
		"ddr test freq",
		"test no		- `test no' means that: 1 for 300M; 2 for 216M. \n"
);

#endif


#endif
