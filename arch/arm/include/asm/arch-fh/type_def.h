/*
 * type_def.h
 *
 *  Created on: Jun 20, 2016
 *      Author: duobao
 */

#ifndef TYPE_DEF_H_
#define TYPE_DEF_H_



#include <asm/types.h>

typedef char SINT8;
typedef short SINT16;
typedef int SINT32;
typedef long long SINT64;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef float ieee_single;
typedef double ieee_double;
typedef unsigned long boolean;
#define lift_shift_bit_num(bit_num)         (1<<bit_num)
#define reg_read(addr)                  (*((volatile UINT32 *)(addr)))
#define reg_write(addr,value)           (*(volatile UINT32 *)(addr)=(value))
#define GET_REG(addr)                   reg_read(addr)
#define SET_REG(addr,value)             reg_write(addr,value)
#define SET_REG_M(addr,value,mask)      reg_write(addr,(reg_read(addr)&(~(mask)))|((value)&(mask)))
#define SET_REG_B(addr,element,highbit,lowbit) SET_REG_M((addr),((element)<<(lowbit)),(((1<<((highbit)-(lowbit)+1))-1)<<(lowbit)))
#define GET_REG8(addr)                  (*((volatile UINT8 *)(addr)))
#define SET_REG8(addr,value)            (*(volatile UINT8 *)(addr)=(value))
#define read_reg(addr)                  (*((volatile uint32 *)(addr)))
#define write_reg(addr, reg)            (*((volatile uint32 *)(addr))) = (uint32)(reg)
#define inw(addr)                       (*((volatile uint32 *)(addr)))
#define outw(addr, reg)                 (*((volatile uint32 *)(addr))) = (uint32)(reg)

#define DELAY(times)  do{\
    int i=0;\
    for(i=0;i<times;i++){}\
}while(0)



#endif /* TYPE_DEF_H_ */
