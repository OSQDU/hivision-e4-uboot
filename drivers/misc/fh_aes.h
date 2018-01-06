/*
 * File      : fh_aes.h
 * This file is part of FH8620 BSP for RT-Thread distribution.
 *
 * Copyright (c) 2016 Shanghai Fullhan Microelectronics Co., Ltd.
 * All rights reserved
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Visit http://www.fullhan.com to get contact with Fullhan.
 *
 * Change Logs:
 * Date           Author       Notes
 */


#ifndef FH_AES_H_
#define FH_AES_H_


#include "algorithm_core.h"


struct fh_aes_reg {
	u32 encrypt_control;		//0
	u32 reserved_4_8;			//4
	u32 fifo_status;			//8
	u32 parity_error;			//c
	u32 security_key0;			//10
	u32 security_key1;			//14
	u32 security_key2;			//18
	u32 security_key3;			//1c
	u32 security_key4;			//20
	u32 security_key5;			//24
	u32 security_key6;			//28
	u32 security_key7;			//2c
	u32 initial_vector0;		//30
	u32 initial_vector1;		//34
	u32 initial_vector2;		//38
	u32 initial_vector3;		//3c
	u32 reserved_40_44;			//40
	u32 reserved_44_48;			//44
	u32 dma_src_add;			//48
	u32 dma_dst_add;			//4c
	u32 dma_trans_size;			//50
	u32 dma_control;			//54
	u32 fifo_threshold;			//58
	u32 intr_enable;			//5c
	u32 intr_src;				//60
	u32 mask_intr_status;		//64
	u32 intr_clear_status;		//68
	u32 reserved_6c_70;			//6c
	u32 revision;				//70
	u32 feature;				//74
	u32 reserved_78_7c;			//78
	u32 reserved_7c_80;			//7c
	u32 last_initial_vector0;	//80
	u32 last_initial_vector1;	//84
	u32 last_initial_vector2;	//88
	u32 last_initial_vector3;	//8c
};

struct fh_aes_platform_data{
	u32 id;
	u32 irq;
	u32 base;
};


struct fh_aes_driver {

	void *regs;
	u32 id;
	u32 irq_no;					//board info...
	u32 open_flag;
	//below are driver use
	u32 irq_en;
	//isr
   // struct rt_completion transfer_completion;
	u32 control_mode;
	int iv_flag;

	//driver could find the core data...
	struct rt_crypto_obj *cur_crypto;
	struct rt_crypto_request *cur_request;

};
#endif




struct efuse_status{
	//bit 1 means could write..0 not write
	u32 protect_bits[2];
	//bit 1 means cpu couldn't read efuse entry data...
	u32	efuse_apb_lock;
	u32 aes_ahb_lock;
	u32 error;
};

typedef struct{
	//write key
	u32 efuse_entry_no;	//from 0 ~ 31
	u8 *key_buff;
	u32 key_size;
	//trans key
	u32 trans_key_start_no; //from 0 ~ 7
	u32 trans_key_size;		//max 8

	//status
	struct efuse_status status;
	//u32 status;			//process status back to app.

}EFUSE_INFO;

struct wrap_efuse_obj {
	void *regs;
//u32 pro_bits[2];		//read the efuse to check which entry has been programmed already. 1:means could write.
};

#define AES_FLAG_SUPPORT_CPU_SET_KEY			1<<0
#define AES_FLAG_SUPPORT_EFUSE_SET_KEY			1<<1


long fh_efuse_ioctl(EFUSE_INFO *efuse_user_info, unsigned int cmd,
		unsigned long arg);

void efuse_trans_key(struct wrap_efuse_obj *obj, u32 start_no, u32 size);

/* FH_AES_H_ */
