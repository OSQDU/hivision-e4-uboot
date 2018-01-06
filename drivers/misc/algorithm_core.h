/*
 * File      : algorithm.h
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

#ifndef ALGORITHM_CORE_H_
#define ALGORITHM_CORE_H_


#include <linux/list.h>
#include <config.h>
#include <common.h>
#include <command.h>
#include <malloc.h>
//#include <string.h>
#include <linux/string.h>
//#include <rtthread.h>
/****************************************************************************
* #include section
*	add #include here if any
***************************************************************************/

//#define ALGO_DEBUG

#define DES_KEY_SIZE			8
#define DES3_EDE_KEY_SIZE		(3 * DES_KEY_SIZE)


#define AES_KEYSIZE_128			16
#define AES_KEYSIZE_192			24
#define AES_KEYSIZE_256			32

#define AES_MIN_KEY_SIZE		AES_KEYSIZE_128
#define AES_MAX_KEY_SIZE		AES_KEYSIZE_256
#define MAX_NAME			12
/****************************************************************************
* #define section
*	add constant #define here if any
***************************************************************************/

enum rt_algo_class_type {
	RT_Algo_Class_Crypto = 0, /**< The algo is a crypto. */
	// RT_Algo_Class_Compress,                         /**< The algo is a compress. */
	RT_Algo_Class_Unknown, /**< The algo is unknown. */
};

typedef enum {
	FALSE = 0,
	TRUE = 1
} boolean_t;
/****************************************************************************
 * ADT section
 *	add Abstract Data Type definition here
 ***************************************************************************/
//basic class
struct rt_algo_information {

	//struct rt_device parent;
	char * class_name;
	enum rt_algo_class_type type;
	struct list_head list_head;

};

//crypto class..

struct rt_crypto_request {
	u8 *iv;			//core will check this if 'flag = USING_IV'
	u32 iv_size;

	//cpu or efuse
	u32 key_flag;
	u8 *key;
	u32 key_size;	//core will check if this value larger than 'key_size'

	u8 *data_src;				//core will check the allign
	u8 *data_dst;				//core will check the allign
	u32 data_size;//core will check the if the data size is multi of the 'block_size'

};

struct rt_crypto_obj {
	struct list_head list;			//bind to the crypto list...

	char name[MAX_NAME];		//each crypto copy name when register..
//#define USING_IV			0x55
	//RT_TRUE / RT_FALSE
	int flag;
	//uint : Byte
	u32 block_size;	//each crypto should have its own process block size... ecb(des) = 8 Bytes
	//uint : Byte
	u32 src_allign_size;	//each crypto may have the src allign...
	//uint : Byte
	u32 dst_allign_size;	//each crypto may have the dst allign...
	//uint : Byte
	u32 max_key_size;	//each crypto may have the key max size...
	u32 min_key_size;	//each crypto may have the key max size...
	//uint : Byte
	u32 otp_size;		//driver one time process how many bytes data..
	void * driver_private;//driver could use the point to save private data...    fh aes 2048 Bytes
	struct {
		int (*set_key)(struct rt_crypto_obj *obj_p,
		                struct rt_crypto_request *request_p);
		int (*encrypt)(struct rt_crypto_obj *obj_p,
		                struct rt_crypto_request *request_p);
		int (*decrypt)(struct rt_crypto_obj *obj_p,
		                struct rt_crypto_request *request_p);
	} ops;
	void * user_private;
	int (*complete)(void *user_private);

};

/****************************************************************************
 *  extern variable declaration section
 ***************************************************************************/

/****************************************************************************
 *  section
 *	add function prototype here if any
 ***************************************************************************/
int rt_algo_init(void);
int rt_algo_register(u8 type, void *obj, const char * obj_name, void *private);
void* rt_algo_obj_find(u8 type, char *name);
/********************************End Of File********************************/
int rt_algo_crypto_decrypt(struct rt_crypto_obj *obj_p,
                struct rt_crypto_request *request_p);
int rt_algo_crypto_encrypt(struct rt_crypto_obj *obj_p,
                struct rt_crypto_request *request_p);
int rt_algo_crypto_setkey(struct rt_crypto_obj *obj_p,
                struct rt_crypto_request *request_p);

#endif

