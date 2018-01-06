/*
 * File      : fh_aes.c
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


/*****************************************************************************
 *  Include Section
 *  add all #include here
 *****************************************************************************/

#include "fh_aes.h"



/*****************************************************************************
 * Define section
 * add all #define here
 *****************************************************************************/
//#define AES_DEBUG
#ifdef AES_DEBUG
#define AES_DBG(fmt, args...)     \
    do                                  \
    {                                   \
        printf("[AES]: ");   \
        printf(fmt, ## args);       \
    }                                   \
    while(0)
#else
#define AES_DBG(fmt, args...)  do { } while (0)
#endif

#define AES_ERR(fmt, args...)     \
    do                                  \
    {                                   \
        printf("[AES ERROR]: ");   \
        printf(fmt, ## args);       \
    }                                   \
    while(0)

#define CRYPTO_QUEUE_LEN    (1)
#define CRYPTION_POS		(0)
#define METHOD_POS			(1)
#define EMODE_POS			(4)

#define __raw_writeb(v,a)       (*(volatile unsigned char  *)(a) = (v))
#define __raw_writew(v,a)       (*(volatile unsigned short *)(a) = (v))
#define __raw_writel(v,a)       (*(volatile unsigned int   *)(a) = (v))

#define __raw_readb(a)          (*(volatile unsigned char  *)(a))
#define __raw_readw(a)          (*(volatile unsigned short *)(a))
#define __raw_readl(a)          (*(volatile unsigned int   *)(a))

#define aes_readl(aes, name) \
	__raw_readl(&(((struct fh_aes_reg *)aes->regs)->name))

#define aes_writel(aes, name, val) \
	__raw_writel((val), &(((struct fh_aes_reg *)aes->regs)->name))

#define aes_readw(aes, name) \
	__raw_readw(&(((struct fh_aes_reg *)aes->regs)->name))

#define aes_writew(aes, name, val) \
	__raw_writew((val), &(((struct fh_aes_reg *)aes->regs)->name))

#define aes_readb(aes, name) \
	__raw_readb(&(((struct fh_aes_reg *)aes->regs)->name))

#define aes_writeb(aes, name, val) \
	__raw_writeb((val), &(((struct fh_aes_reg *)aes->regs)->name))

#define wrap_readl(wrap, name) \
	__raw_readl(&(((struct wrap_efuse_reg *)wrap->regs)->name))

#define wrap_writel(wrap, name, val) \
	__raw_writel((val), &(((struct wrap_efuse_reg *)wrap->regs)->name))

#define wrap_readw(wrap, name) \
	__raw_readw(&(((struct wrap_efuse_reg *)wrap->regs)->name))

#define wrap_writew(wrap, name, val) \
	__raw_writew((val), &(((struct wrap_efuse_reg *)wrap->regs)->name))

#define wrap_readb(wrap, name) \
	__raw_readb(&(((struct wrap_efuse_reg *)wrap->regs)->name))

#define wrap_writeb(wrap, name, val) \
	__raw_writeb((val), &(((struct wrap_efuse_reg *)wrap->regs)->name))

#define ASSERT( expr) if( !( expr ) ) { \
		printf( "Assertion failed! %s:line %d \n", \
		__FUNCTION__,__LINE__); \
		while(1);\
		}

#define DMA_MAX_PROCESS_SIZE			2048
#define AES_REG_BASE 					(0xE8200000)
#define EFUSE_REG_BASE				 	(0xf1600000)

#define IOCTL_EFUSE_CHECK_PRO			0
#define IOCTL_EFUSE_WRITE_KEY			1
#define IOCTL_EFUSE_CHECK_LOCK			2
#define IOCTL_EFUSE_TRANS_KEY			3
#define IOCTL_EFUSE_CHECK_ERROR			4

#define  AES_CONTROLLER_SUPPORT_SET_KEY			( AES_FLAG_SUPPORT_CPU_SET_KEY | AES_FLAG_SUPPORT_EFUSE_SET_KEY )

struct wrap_efuse_reg {
	u32 efuse_cmd;				//0x0
	u32 efuse_config;			//0x4
	u32 efuse_match_key;		//0x8
	u32 efuse_timing0;			//0xc
	u32 efuse_timing1;			//0x10
	u32 efuse_timing2;			//0x14
	u32 efuse_timing3;			//0x18
	u32 efuse_timing4;			//0x1c
	u32 efuse_timing5;			//0x20
	u32 efuse_timing6;			//0x24
	u32 efuse_dout;				//0x28
	u32 efuse_status0;			//0x2c
	u32 efuse_status1;			//0x30
	u32 efuse_status2;			//0x34
	u32 efuse_status3;			//0x38
	u32 efuse_status4;			//0x3c
	u32 efuse_mem_info;
};

struct wrap_efuse_obj s_efuse_obj = { 0 };
struct fh_aes_driver g_aes_driver = { 0 };

#define EFUSE_MAX_ENTRY			60

/****************************************************************************
 * ADT section
 *  add definition of user defined Data Type that only be used in this file  here
 ***************************************************************************/
enum {
	CMD_TRANS_AESKEY = 4, CMD_WFLGA_AUTO = 8,
};

enum {
	ENCRYPT = 0 << CRYPTION_POS, DECRYPT = 1 << CRYPTION_POS,
};

enum {
	ECB_MODE = 0 << EMODE_POS, CBC_MODE = 1 << EMODE_POS, CTR_MODE = 2
			<< EMODE_POS, CFB_MODE = 4 << EMODE_POS, OFB_MODE = 5
					<< EMODE_POS,
};

enum {
	DES_METHOD = 0 << METHOD_POS,
	TRIPLE_DES_METHOD = 1 << METHOD_POS,
	AES_128_METHOD = 4 << METHOD_POS,
	AES_192_METHOD = 5 << METHOD_POS,
	AES_256_METHOD = 6 << METHOD_POS,
};

/*****************************************************************************

 *  static fun;
 *****************************************************************************/
static int fh_aes_set_key(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p);
static int fh_aes_encrypt(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p);
static int fh_aes_decrypt(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p);
static void fh_set_aes_key_reg(struct rt_crypto_obj *obj_p,
			       struct rt_crypto_request *request_p);

static void aes_biglittle_swap(u8 *buf);

static int fh_aes_cbc_encrypt(struct rt_crypto_obj *obj_p,
			      struct rt_crypto_request *request_p);
static int fh_aes_cbc_decrypt(struct rt_crypto_obj *obj_p,
			      struct rt_crypto_request *request_p);

/*****************************************************************************
 * Global variables section - Local
 * define global variables(will be refered only in this file) here,
 * static keyword should be used to limit scope of local variable to this file
 * e.g.
 *  static uint8_t ufoo;
 *****************************************************************************/

struct rt_crypto_obj crypto_table[] =  {
	{
		.name = "aes-ecb",
		.flag = FALSE,
		.block_size = 16,
		.src_allign_size = 4,
		.dst_allign_size = 4,
		.max_key_size = AES_MAX_KEY_SIZE,
		.min_key_size = AES_MIN_KEY_SIZE,
		.otp_size = DMA_MAX_PROCESS_SIZE,
		.ops.set_key = fh_aes_set_key,
		.ops.encrypt = fh_aes_encrypt,
		.ops.decrypt = fh_aes_decrypt,
	},
	{
		.name = "aes-cbc",
		.flag = TRUE,
		.block_size = 16,
		.src_allign_size = 4,
		.dst_allign_size = 4,
		.max_key_size = AES_MAX_KEY_SIZE,
		.min_key_size = AES_MIN_KEY_SIZE,
		.otp_size = DMA_MAX_PROCESS_SIZE,
		.ops.set_key = fh_aes_set_key,
		.ops.encrypt = fh_aes_cbc_encrypt,
		.ops.decrypt = fh_aes_cbc_decrypt,
	},
};

static void fh_set_aes_key_reg(struct rt_crypto_obj *obj_p,
			       struct rt_crypto_request *request_p)
{

	struct fh_aes_driver *p_driver;
	int i;
	u32 method;
	u8 *key;

	u32 temp_key_buf[32];
	u32 temp_iv_buf[32];
	u32 *p_dst = NULL;

	key = request_p->key;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;
	u32 key_size = 0;
	EFUSE_INFO efuse_info;

	if (p_driver->iv_flag == TRUE) {
		//set iv
		//printf("set iv............................\n");
		//if aes mode ....set 128 bit iv,  des set 64bit iv..
		AES_DBG("set iv reg\n");
		if ((p_driver->control_mode & AES_128_METHOD)
		    || ((p_driver->control_mode & AES_192_METHOD))
		    || (p_driver->control_mode & AES_256_METHOD)) {
			ASSERT(request_p->iv != NULL);
			AES_DBG("aes iv mode...\n");

			memcpy((u8 *) &temp_iv_buf[0], request_p->iv, 16);
			p_dst = &temp_iv_buf[0];
			for (i = 0; i < 16 / sizeof(u32); i++) {
				aes_biglittle_swap((u8 *)(p_dst + i));
				//printf("set iv is %x\n",*(p_dst + i));
			}

			memcpy(
				(u8 *) & ((struct fh_aes_reg *) p_driver->regs)->initial_vector0,
				temp_iv_buf, 16);

		} else {
			AES_DBG("des iv mode...\n");

			memcpy((u8 *) &temp_iv_buf[0], request_p->iv, 8);
			p_dst = &temp_iv_buf[0];
			for (i = 0; i < 8 / sizeof(u32); i++)
				aes_biglittle_swap((u8 *)(p_dst + i));
			memcpy(
				(u8 *) & ((struct fh_aes_reg *) p_driver->regs)->initial_vector0,
				temp_iv_buf, 8);
		}
	}
	//set key...
	method = p_driver->control_mode & 0x0e;
	AES_DBG("set key reg\n");
	switch (method) {
	case AES_128_METHOD:
		AES_DBG("set key aes 128 mode..\n");

		key_size = 16;
		break;
	case AES_192_METHOD:
		AES_DBG("set key aes 192 mode..\n");

		key_size = 24;
		break;

	case AES_256_METHOD:
		AES_DBG("set key aes 256 mode..\n");

		key_size = 32;
		break;

	case DES_METHOD:
		AES_DBG("set key des normal mode..\n");

		key_size = 8;
		break;

	case TRIPLE_DES_METHOD:
		AES_DBG("set key des triple mode..\n");

		key_size = 24;
		break;

	default:
		AES_DBG("error method!!\n");
		break;

	}

	if (request_p->key_flag == AES_FLAG_SUPPORT_EFUSE_SET_KEY) {
		efuse_info.trans_key_start_no = 0;
		efuse_info.trans_key_size = key_size / 4;
		efuse_trans_key(&s_efuse_obj, efuse_info.trans_key_start_no,
				efuse_info.trans_key_size);

		printf("aes use efuse trans key...\n");
	} else {

		memcpy((u8 *) &temp_key_buf[0], key, key_size);
		p_dst = &temp_key_buf[0];
		for (i = 0; i < key_size / sizeof(u32); i++) {
			aes_biglittle_swap((u8 *)(p_dst + i));

			//printf("set key is %x\n",*(p_dst + i));
		}

		memcpy((u8 *) & ((struct fh_aes_reg *) p_driver->regs)->security_key0,
		       temp_key_buf, key_size);

	}

}

static void aes_biglittle_swap(u8 *buf)
{
	u8 tmp, tmp1;
	tmp = buf[0];
	tmp1 = buf[1];
	buf[0] = buf[3];
	buf[1] = buf[2];
	buf[2] = tmp1;
	buf[3] = tmp;
}

static int fh_set_indata(struct rt_crypto_obj *obj_p,
			 struct rt_crypto_request *request_p)
{

//	u32 i;
//	for (i = 0; i < request_p->data_size / sizeof(u32); i++) {
//		//p_src is 32bit point...
//		aes_biglittle_swap((u8*) (request_p->data_src + i*4));
//	}
	return 0;

}

static int fh_set_outdata(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p)
{

	return 0;
}

static void fh_unset_indata(struct rt_crypto_obj *obj_p,
			    struct rt_crypto_request *request_p)
{
	//u32 i;
//	for (i = 0; i < request_p->data_size / sizeof(u32); i++) {
//		//p_src is 32bit point...
//		aes_biglittle_swap((u8*) (request_p->data_src + i*4));
//	}
}

static void fh_unset_outdata(struct rt_crypto_obj *obj_p,
			     struct rt_crypto_request *request_p)
{
//	u32 i;
//	for (i = 0; i < request_p->data_size / sizeof(u32); i++) {
//		//p_src is 32bit point...
//		aes_biglittle_swap((u8*) (request_p->data_dst + i*4));
//	}
}

static void fh_set_dma_indata(struct rt_crypto_obj *obj_p,
			      struct rt_crypto_request *request_p)
{
	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;
	aes_writel(p_driver, dma_src_add, (u32)request_p->data_src);
	AES_DBG("dma indata add is :%x\n", (u32)request_p->data_src);

	aes_writel(p_driver, dma_trans_size, request_p->data_size);
	AES_DBG("dma trans size is :%x\n", request_p->data_size);
}

static void fh_set_dma_outdata(struct rt_crypto_obj *obj_p,
			       struct rt_crypto_request *request_p)
{
	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;
	aes_writel(p_driver, dma_dst_add, (u32)request_p->data_dst);
	AES_DBG("dma outdata add is :%x\n", (u32)request_p->data_dst);
	aes_writel(p_driver, dma_trans_size, request_p->data_size);
	AES_DBG("dma trans size is :%x\n", request_p->data_size);
}

static void fh_aes_handle_request(struct rt_crypto_obj *obj_p,
				  struct rt_crypto_request *request_p)
{

	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;

	u32 *mode;
	u32 outfifo_thold;
	u32 infifo_thold;
	u32 isr;
	int err;
	mode = &p_driver->control_mode;
	u32 isr_status;

	//check if driver open flag suit the request flag...
	if (!(p_driver->open_flag & request_p->key_flag)) {
		printf(
			"aes don't support this set key mode...driver support flag %x,request flag is %x\n",
			p_driver->open_flag,
			request_p->key_flag);
		ASSERT(0);
	}

	//explain the software para(mode) to the hardware para
	if ((*mode & CBC_MODE) || (*mode & CTR_MODE) || (*mode & CFB_MODE)
	    || (*mode & OFB_MODE)) {
		*mode |= 1 << 7;
		p_driver->iv_flag = TRUE;
	} else
		p_driver->iv_flag = FALSE;
	//emode & method

	//iv & key already get...
	// fifo threshold ..
	outfifo_thold = 0;
	infifo_thold = 8;
	isr = p_driver->irq_en;

	//here set the hardware reg.....lock now....
	//rt_sem_take(&p_driver->lock, RT_WAITING_FOREVER);

	p_driver->cur_request = request_p;
	p_driver->cur_crypto = obj_p;
	AES_DBG("control_reg:0x%x\n", p_driver->control_mode);
	aes_writel(p_driver, encrypt_control, p_driver->control_mode);

	//set key...
	fh_set_aes_key_reg(obj_p, request_p);

	err = fh_set_indata(obj_p, request_p);
	if (err)
		goto indata_error;

	err = fh_set_outdata(obj_p, request_p);
	if (err)
		goto outdata_error;

	//set hw para...
	fh_set_dma_indata(obj_p, request_p);
	fh_set_dma_outdata(obj_p, request_p);

	//set fifo..
	AES_DBG("outfifo thold:%x\n", outfifo_thold);
	AES_DBG("infifo thold:%x\n", infifo_thold);
	aes_writel(p_driver, fifo_threshold, outfifo_thold << 8 | infifo_thold);

	//set isr..
	//close all the isr...
	aes_writel(p_driver, intr_enable, 0);

	//enable dma go..
	aes_writel(p_driver, dma_control, 1);
	//rt_sem_release(&p_driver->lock);

	do {
		isr_status = aes_readl(p_driver, intr_src);
	} while (!(isr_status & 0x01));

	//add big 2 little
//	for (i = 0; i < request_p->data_size / sizeof(u32); i++) {
//		//p_src is 32bit point...
//		aes_biglittle_swap((u8*) (request_p->data_dst + i*4));
//	}

	fh_unset_indata(obj_p, request_p);
	fh_unset_outdata(obj_p, request_p);
	return;

outdata_error:
	AES_DBG("outdata_error ..\n");
indata_error:
	AES_DBG("indata_error ..\n");

}

static int fh_aes_set_key(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p)
{

	return 0;
}

static int fh_aes_encrypt(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p)
{

	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;

	switch (request_p->key_size) {
	case AES_KEYSIZE_128:
		p_driver->control_mode = AES_128_METHOD;
		break;
	case AES_KEYSIZE_192:
		p_driver->control_mode = AES_192_METHOD;
		break;
	case AES_KEYSIZE_256:
		p_driver->control_mode = AES_256_METHOD;
		break;
	default:
		AES_ERR("%s wrong key size..\n", __func__);
		ASSERT(0);
		break;
	}
	p_driver->control_mode |= ECB_MODE | ENCRYPT;

	fh_aes_handle_request(obj_p, request_p);
	return 0;

}

static int fh_aes_decrypt(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p)
{

	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;

	switch (request_p->key_size) {
	case AES_KEYSIZE_128:
		p_driver->control_mode = AES_128_METHOD;
		break;
	case AES_KEYSIZE_192:
		p_driver->control_mode = AES_192_METHOD;
		break;
	case AES_KEYSIZE_256:
		p_driver->control_mode = AES_256_METHOD;
		break;
	default:
		AES_ERR("%s wrong key size..\n", __func__);
		ASSERT(0);
	}
	p_driver->control_mode |= ECB_MODE | DECRYPT;

	fh_aes_handle_request(obj_p, request_p);

	return 0;
}

static int fh_aes_cbc_encrypt(struct rt_crypto_obj *obj_p,
			      struct rt_crypto_request *request_p)
{
	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;

	switch (request_p->key_size) {
	case AES_KEYSIZE_128:
		p_driver->control_mode = AES_128_METHOD;
		break;
	case AES_KEYSIZE_192:
		p_driver->control_mode = AES_192_METHOD;
		break;
	case AES_KEYSIZE_256:
		p_driver->control_mode = AES_256_METHOD;
		break;
	default:
		AES_ERR("%s wrong key size..\n", __func__);
		ASSERT(0);
		break;
	}
	p_driver->control_mode |= CBC_MODE | ENCRYPT;

	fh_aes_handle_request(obj_p, request_p);
	return 0;
}

static int fh_aes_cbc_decrypt(struct rt_crypto_obj *obj_p,
			      struct rt_crypto_request *request_p)
{
	struct fh_aes_driver *p_driver;
	p_driver = (struct fh_aes_driver *) obj_p->driver_private;

	switch (request_p->key_size) {
	case AES_KEYSIZE_128:
		p_driver->control_mode = AES_128_METHOD;
		break;
	case AES_KEYSIZE_192:
		p_driver->control_mode = AES_192_METHOD;
		break;
	case AES_KEYSIZE_256:
		p_driver->control_mode = AES_256_METHOD;
		break;
	default:
		AES_ERR("%s wrong key size..\n", __func__);
		ASSERT(0);
		break;
	}
	p_driver->control_mode |= CBC_MODE | DECRYPT;

	fh_aes_handle_request(obj_p, request_p);
	return 0;
}

void test_aes(void);

extern void test_hik_aes(void);

int fh_aes_probe(void)
{

	struct fh_aes_driver *fh_aes_driver_obj;

	struct rt_crypto_obj *p_crypto;
	u32 i;

	//efuse set..
	s_efuse_obj.regs = (void *) EFUSE_REG_BASE;

	//ase set..
	fh_aes_driver_obj = &g_aes_driver;
	fh_aes_driver_obj->id = 0;
	fh_aes_driver_obj->irq_no = 0;
	fh_aes_driver_obj->regs = (void *) AES_REG_BASE;
	//support open flag..
	fh_aes_driver_obj->open_flag = AES_CONTROLLER_SUPPORT_SET_KEY;

	AES_DBG("id:%d, irq:%d, regs:%x..\n", fh_aes_driver_obj->id,
		fh_aes_driver_obj->irq_no,
		(u32)fh_aes_driver_obj->regs);

	//register the algo ....
	for (i = 0, p_crypto = &crypto_table[0]; i < ARRAY_SIZE(crypto_table);
	     i++, p_crypto++)
		rt_algo_register(RT_Algo_Class_Crypto, p_crypto, p_crypto->name,
				 (void *) fh_aes_driver_obj);

	//test_aes();
	//test_hik_aes();
	return 0;
}

//efuse driver....
void efuse_detect_complete(struct wrap_efuse_obj *obj, int pos)
{
	unsigned int rdata;
//  printf("efuse wait pos %x...\n",pos);
	do {
		rdata = wrap_readl(obj, efuse_status0);
	} while ((rdata & (1 << pos)) != 1 << pos);
	// printf("efuse wait pos done...\n",pos);
}

void auto_check_efuse_pro_bits(struct wrap_efuse_obj *obj, u32 *buff)
{

	//first set auto check cmd
	wrap_writel(obj, efuse_cmd, CMD_WFLGA_AUTO);
	efuse_detect_complete(obj, 8);

	buff[0] = wrap_readl(obj, efuse_status1);
	buff[1] = wrap_readl(obj, efuse_status2);

}

void efuse_write_key_byte(struct wrap_efuse_obj *obj, u32 entry, u8 data)
{

	u32 temp = 0;
	temp = (u32) data;
	//0~255
	temp &= ~0xffffff00;
	temp <<= 12;
	//0~63
	entry &= 0x3f;
	temp |= (entry << 4) | (0x02);

	wrap_writel(obj, efuse_cmd, temp);
	efuse_detect_complete(obj, 2);
}

void efuse_load_usrcmd(struct wrap_efuse_obj *obj)
{

	wrap_writel(obj, efuse_cmd, 1);
	efuse_detect_complete(obj, 1);

}

void efuse_trans_key(struct wrap_efuse_obj *obj, u32 start_no, u32 size)
{

	int i;
	for (i = 0; i < size; i++) {
		wrap_writel(obj, efuse_cmd, ((start_no + i) << 20) + 0x04);
		efuse_detect_complete(obj, 4);
	}

}

void efuse_get_lock_status(struct wrap_efuse_obj *obj,
			   struct efuse_status *status)
{

	status->efuse_apb_lock = (wrap_readl(obj, efuse_status0) >> 20) & 0x0f;
	status->aes_ahb_lock = (wrap_readl(obj, efuse_status0) >> 24) & 0x0f;
}

long fh_efuse_ioctl(EFUSE_INFO *efuse_user_info, unsigned int cmd,
		    unsigned long arg)
{

	int i;
	EFUSE_INFO efuse_info = { 0 };

	u32 temp_swap_data[32] = { 0 };
	u32 *p_dst;
	u8 *p_dst_8;

	memcpy(&efuse_info, efuse_user_info, sizeof(EFUSE_INFO));

	switch (cmd) {
	case IOCTL_EFUSE_CHECK_PRO:

		auto_check_efuse_pro_bits(&s_efuse_obj,
					  efuse_info.status.protect_bits);

		break;
	case IOCTL_EFUSE_WRITE_KEY:

//		for(i=0;i<efuse_info.key_size;i++){
//			efuse_write_key_byte(&s_efuse_obj,efuse_info.efuse_entry_no+i,*(efuse_info.key_buff +i));
//		}
//		efuse_load_usrcmd(&s_efuse_obj);
//
//

		memcpy((u8 *) &temp_swap_data[0], efuse_info.key_buff,
		       efuse_info.key_size);
		p_dst = &temp_swap_data[0];
		for (i = 0; i < efuse_info.key_size / sizeof(u32); i++) {
			aes_biglittle_swap((u8 *)(p_dst + i));
			//printk("swap data is %x\n",*(p_dst + i));
		}
		p_dst_8 = (u8 *) &temp_swap_data[0];

		for (i = 0; i < efuse_info.key_size; i++)
			efuse_write_key_byte(&s_efuse_obj,
					     efuse_info.efuse_entry_no + i,
					     *(p_dst_8 + i));
		efuse_load_usrcmd(&s_efuse_obj);

		break;

	case IOCTL_EFUSE_CHECK_LOCK:

		efuse_load_usrcmd(&s_efuse_obj);
		efuse_get_lock_status(&s_efuse_obj, &efuse_info.status);
		break;
	case IOCTL_EFUSE_TRANS_KEY:
		efuse_trans_key(&s_efuse_obj, efuse_info.trans_key_start_no,
				efuse_info.trans_key_size);
		break;

	case IOCTL_EFUSE_CHECK_ERROR:
		break;

	default:
		break;
	}

	memcpy(&efuse_user_info->status, &efuse_info.status,
	       sizeof(struct efuse_status));

	return 0;

}

//ecb aes 256
#define AES_ECB_KEY0	0x603deb10
#define AES_ECB_KEY1	0x15ca71be
#define AES_ECB_KEY2	0x2b73aef0
#define AES_ECB_KEY3	0x857d7781
#define AES_ECB_KEY4	0x1f352c07
#define AES_ECB_KEY5	0x3b6108d7
#define AES_ECB_KEY6	0x2d9810a3
#define AES_ECB_KEY7	0x0914dff4

static unsigned int _plain_ecb_key[16] = {
	AES_ECB_KEY0, AES_ECB_KEY1,
	AES_ECB_KEY2, AES_ECB_KEY3,
	AES_ECB_KEY4, AES_ECB_KEY5,
	AES_ECB_KEY6, AES_ECB_KEY7,
};

static unsigned int _plain_ecb_256_text[16] = { 0x6bc1bee2, 0x2e409f96,
						0xe93d7e11, 0x7393172a, 0xae2d8a57, 0x1e03ac9c, 0x9eb76fac,
						0x45af8e51, 0x30c81c46, 0xa35ce411, 0xe5fbc119, 0x1a0a52ef,
						0xf69f2445, 0xdf4f9b17, 0xad2b417b, 0xe66c3710,
					      };

static unsigned int _cipher_ecb_256_text[16] = { 0xf3eed1bd, 0xb5d2a03c,
						 0x064b5a7e, 0x3db181f8, 0x591ccb10, 0xd410ed26, 0xdc5ba74a,
						 0x31362870, 0xb6ed21b9, 0x9ca6f4f9, 0xf153e7b1, 0xbeafed1d,
						 0x23304b7a, 0x39f9f3ff, 0x067d8d8f, 0x9e24ecc7,
					       };

static unsigned int _plain_ecb_256_out_buff[16] = { 0 };

void test_aes(void)
{
	static struct rt_crypto_obj *p_crypto_obj;
	p_crypto_obj = rt_algo_obj_find(RT_Algo_Class_Crypto, "aes-ecb");
	printf("find driver name is %s\n", p_crypto_obj->name);
	static struct rt_crypto_request request;
	int i;
	unsigned int temp;

	request.data_src = (u8 *) _plain_ecb_256_text;
	request.data_dst = (u8 *) _plain_ecb_256_out_buff;

	request.data_size = 16 * 4;
	request.key = (u8 *) _plain_ecb_key;
	request.key_size = AES_KEYSIZE_256;
	request.key_flag = AES_FLAG_SUPPORT_EFUSE_SET_KEY;

	//set efuse
	EFUSE_INFO efuse_info_obj;
	EFUSE_INFO *efuse_info = &efuse_info_obj;
	efuse_info->efuse_entry_no = 0;
	efuse_info->key_buff = (unsigned char *) _plain_ecb_key;
	efuse_info->key_size = 32;

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_CHECK_PRO, 0);

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_CHECK_LOCK, 0);

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_WRITE_KEY, 0);

	printf("src add is %x, dst add is %x, size is %d",
	       (u32) request.data_src, (u32) request.data_dst,
	       request.data_size);

	rt_algo_crypto_setkey(p_crypto_obj, &request);

	rt_algo_crypto_encrypt(p_crypto_obj, &request);

	for (i = 0; i < 16; i++) {
		temp = _plain_ecb_256_out_buff[i];
		printf("%x\n", temp);
	}

//	for (i = 0; i < 64 / sizeof(u32); i++) {
//		//p_src is 32bit point...
//		aes_biglittle_swap((u8*) (_cipher_ecb_256_text + i));
//
//	}
	//rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_ubase_t count)
	if (memcmp(_plain_ecb_256_out_buff, _cipher_ecb_256_text, 64))
		printf("encry error\n");

	else
		printf("encry ok\n");
}
