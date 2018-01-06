/*
 * hik_api_demo.c
 *
 *  Created on: Aug 21, 2016
 *      Author: duobao
 */
#include "hik_api_demo.h"

#ifndef AF_ALG
#define AF_ALG 38
#endif
#ifndef SOL_ALG
#define SOL_ALG 279
#endif

#define HIK_OPEN_AES_FLAG_CPU				1<<0
#define HIK_OPEN_AES_FLAG_EFUSE				1<<1

struct crypt_method {
	char *type;
	char *mode;
	char *src;
	char *dst;
	char *key_buf;
	char *iv_buf;
	int key_len;
	int iv_len;
	int text_len;
};

struct efuse_status {
	//bit 1 means could write..0 not write
	u32 protect_bits[2];
	//bit 1 means cpu couldn't read efuse entry data...
	u32 efuse_apb_lock;
	u32 aes_ahb_lock;
	u32 error;

};

typedef struct {
	//write key
	u32 efuse_entry_no;	//from 0 ~ 31
	u8 *key_buff;
	u32 key_size;
	//trans key
	u32 trans_key_start_no; //from 0 ~ 7
	u32 trans_key_size;		//max 8

	//u32 swap_data[32];
	//status
	struct efuse_status status;

//u32 status;			//process status back to app.

} EFUSE_INFO;

typedef struct hikHIK_DRIVER_HANDLE {
	//struct crypt_cipher *cipher;
	struct crypt_method method;
	struct rt_crypto_obj *p_crypto_obj;
	EFUSE_INFO efuse_info_obj;
	struct rt_crypto_request request;
//	//efuse...
//	int fd;
//	EFUSE_INFO * efuse_info;
} HIK_DRIVER_HANDLE;

struct HIK_CRYPT_INTF_WORK_MODE_MAP {
	int no;
	char *string;
};

struct HIK_CRYPT_INTF_ALG_MAP {
	int no;
	char *string;
};

struct HIK_CRYPT_INTF_KEY_LEN_MAP {
	int no;
	int len;
};

struct HIK_CRYPT_INTF_IV_KEY_LENGTH_MAP {
	int no;
	int len;
};

#define _MAP_MODE_INIT(c,name)     {.no = c, .string = name,}

#define _MAP_DATA_INIT(c,data)     {.no = c, .len = data,}

#define IOCTL_EFUSE_CHECK_PRO						0
#define IOCTL_EFUSE_WRITE_KEY						1
#define IOCTL_EFUSE_CHECK_LOCK						2
#define IOCTL_EFUSE_TRANS_KEY						3
#define IOCTL_EFUSE_SWITCH_CPU_KEY_MODE				4
#define IOCTL_EFUSE_SWITCH_EFUSE_KEY_MODE			5
#define IOCTL_EFUSE_CHECK_ERROR						6

struct HIK_CRYPT_INTF_WORK_MODE_MAP work_mode_map[] = {
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_ECB, "ecb"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_CBC, "cbc"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_CFB, "cfb"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_OFB, "ofb"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_CTR, "ctr"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_CCM, "ccm"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_GCM, "gcm"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_WORK_MODE_CBC_CTS, "cbc_cts"),
};


struct HIK_CRYPT_INTF_ALG_MAP alg_map[] = {
	_MAP_MODE_INIT(HIK_CRYPT_INTF_ALG_DES, "des"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_ALG_3DES, "des3"),
	_MAP_MODE_INIT(HIK_CRYPT_INTF_ALG_DES, "aes"),
};


struct HIK_CRYPT_INTF_KEY_LEN_MAP key_len_map[] = {
	_MAP_DATA_INIT(HIK_CRYPT_INTF_KEY_AES_128BIT, 16),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_KEY_AES_192BIT, 24),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_KEY_AES_256BIT, 32),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_KEY_DES_3KEY, 24),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_KEY_DES_2KEY, 16),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_KEY_DES, 8),
};

struct HIK_CRYPT_INTF_IV_KEY_LENGTH_MAP iv_key_len_map[] = {
	_MAP_DATA_INIT(HIK_CRYPT_INTF_IV_KEY_AES_0BIT, 0),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_IV_KEY_AES_64BIT, 8),
	_MAP_DATA_INIT(HIK_CRYPT_INTF_IV_KEY_AES_128BIT, 16),
};

#define _FIND_MAP_STRING(p_map,no)     p_map[no].string
#define _FIND_MAP_DATTA(p_map,no)     p_map[no].len

S32 HIK_CRYPT_INTF_Init(void)
{

	return 0;
}

S32 HIK_CRYPT_INTF_DeInit(void)
{

	return 0;
}

HIK_DRIVER_HANDLE g_hik_handle;

S32 HIK_CRYPT_INTF_CreateHandle(HIK_HANDLE *phCIPHER)
{

	HIK_DRIVER_HANDLE *p_driver;
	//p_driver = malloc(sizeof(HIK_DRIVER_HANDLE));

//	if(p_driver == NULL){
//		printf("creat handle no mem...\n");
//		return HIK_ERR_CRYPT_FAILED_INIT;
//	}
	p_driver = &g_hik_handle;
	*phCIPHER = (u32) p_driver;
	return 0;
}

S32 HIK_CRYPT_INTF_DestroyHandle(HIK_HANDLE hCIPHER)
{

	HIK_DRIVER_HANDLE *p_driver;
	p_driver = (HIK_DRIVER_HANDLE *) hCIPHER;
	free(p_driver);
	return 0;
}

void debug_method_print(HIK_DRIVER_HANDLE *phCIPHER)
{

	struct crypt_method *p_method;
	p_method = &phCIPHER->method;

	printf(
		"type:%s ,mode:%s ,key len:%d ,iv_len:%d ,text len:%d ,src add:%x , dst add:%x\n",
		p_method->type, p_method->mode, p_method->key_len,
		p_method->iv_len, p_method->text_len,
		(u32) p_method->src, (u32) p_method->dst);

}

#if(0)
void test()
{

	static struct rt_crypto_obj *p_crypto_obj;
	p_crypto_obj = rt_algo_obj_find(RT_Algo_Class_Crypto, "aes-ecb");
	printf("find driver name is %s\n", p_crypto_obj->name);
	static struct rt_crypto_request request;
	int i;
	unsigned int temp;

	u32 *plain_ecb_256_text;
	u32 *plain_ecb_256_out_buff;
	u32 *plain_ecb_key;
	u32 *cipher_ecb_256_text;

	request.data_src = (u8 *)_plain_ecb_256_text;
	request.data_dst = (u8 *)_plain_ecb_256_out_buff;

	request.data_size = 16 * 4;
	request.key = (u8 *)_plain_ecb_key;
	request.key_size = AES_KEYSIZE_256;
	request.key_flag = AES_FLAG_SUPPORT_EFUSE_SET_KEY;

	//set efuse
	EFUSE_INFO efuse_info_obj;
	EFUSE_INFO *efuse_info = &efuse_info_obj;
	efuse_info->efuse_entry_no = 0;
	efuse_info->key_buff = (unsigned char *)_plain_ecb_key;
	efuse_info->key_size = 32;

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_CHECK_PRO, 0);

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_CHECK_LOCK, 0);

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_WRITE_KEY, 0);

	printf("src add is %x, dst add is %x, size is %d", (u32)request.data_src,
	       (u32)request.data_dst, request.data_size);

	rt_algo_crypto_setkey(p_crypto_obj, &request);

	rt_algo_crypto_encrypt(p_crypto_obj, &request);

	for (i = 0; i < 16; i++) {
		temp = _plain_ecb_256_out_buff[i];
		printf("%x\n", temp);
	}

	for (i = 0; i < 64 / sizeof(u32); i++) {
		//p_src is 32bit point...
		aes_biglittle_swap((u8 *)(_cipher_ecb_256_text + i));

	}
	//rt_int32_t rt_memcmp(const void *cs, const void *ct, rt_ubase_t count)
	if (memcmp(_plain_ecb_256_out_buff, _cipher_ecb_256_text, 64))
		printf("encry error\n");

	else
		printf("encry ok\n");
}
#endif

S32 HIK_CRYPT_INTF_ConfigHandle(HIK_HANDLE hCIPHER,
				HIK_CRYPT_INTF_CTRL_S *pstCtrl)
{

	HIK_DRIVER_HANDLE *phCIPHER;
	//phCIPHER = (HIK_DRIVER_HANDLE*)hCIPHER.driver_pri;
	phCIPHER = (HIK_DRIVER_HANDLE *) hCIPHER;

	struct rt_crypto_obj *p_crypto_obj;
	char aes_dev_name[20] = { 0 };

	if (!pstCtrl)
		return HIK_ERR_CRYPT_INVALID_POINT;

	phCIPHER->method.type = _FIND_MAP_STRING(alg_map, pstCtrl->enAlg);
	phCIPHER->method.mode = _FIND_MAP_STRING(work_mode_map,
				pstCtrl->enWorkMode);
	phCIPHER->method.key_len = _FIND_MAP_DATTA(key_len_map,
				   pstCtrl->enKeyLen);
	phCIPHER->method.iv_len = _FIND_MAP_DATTA(iv_key_len_map,
				  pstCtrl->iv_len);
	phCIPHER->method.iv_buf = (char *) pstCtrl->u32IV;
	phCIPHER->method.key_buf = (char *) pstCtrl->u32Key;

	printf("type is %s.  mode is %s. value:%x:%x\n", phCIPHER->method.type,
	       phCIPHER->method.mode, pstCtrl->enAlg,
	       pstCtrl->enWorkMode);

	sprintf(aes_dev_name, "%s%s%s", phCIPHER->method.type, "-",
		phCIPHER->method.mode);
	printf("find driver name is %s\n", aes_dev_name);
	//find the aes driver...
	p_crypto_obj = rt_algo_obj_find(RT_Algo_Class_Crypto, aes_dev_name);
	if (!p_crypto_obj) {
		printf("can't find the %s driver support..\n", aes_dev_name);
		return HIK_ERR_CRYPT_FAILED_GETHANDLE;
	}
	phCIPHER->p_crypto_obj = p_crypto_obj;

	phCIPHER->request.key_flag = HIK_OPEN_AES_FLAG_CPU;
	return 0;
	//add enum change to string

}

S32 HIK_CRYPT_INTF_Encrypt(HIK_HANDLE hCIPHER, U32 u32SrcPhyAddr,
			   U32 u32DestPhyAddr, U32 u32ByteLength)
{

	struct rt_crypto_request *request;
	HIK_DRIVER_HANDLE *phCIPHER;
	struct crypt_method *method;
	struct rt_crypto_obj *p_crypto_obj;
	//phCIPHER = (HIK_DRIVER_HANDLE*)hCIPHER.driver_pri;
	phCIPHER = (HIK_DRIVER_HANDLE *) hCIPHER;

	phCIPHER->method.src = (char *) u32SrcPhyAddr;
	phCIPHER->method.dst = (char *) u32DestPhyAddr;
	phCIPHER->method.text_len = u32ByteLength;
	debug_method_print(phCIPHER);

	method = &phCIPHER->method;
	request = &phCIPHER->request;

	request->data_src = (u8 *) method->src;
	request->data_dst = (u8 *) method->dst;

	request->data_size = method->text_len;
	request->key = (u8 *) method->key_buf;
	request->key_size = method->key_len;
	if (method->iv_len != 0) {
		printf("iv len is %d\n", method->iv_len);
		request->iv = (u8 *) method->iv_buf;
		request->iv_size = method->iv_len;
	} else {
		request->iv = NULL;
		request->iv_size = 0;
	}

	p_crypto_obj = phCIPHER->p_crypto_obj;

	rt_algo_crypto_setkey(p_crypto_obj, request);

	rt_algo_crypto_encrypt(p_crypto_obj, request);

	return 0;

}

S32 HIK_CRYPT_INTF_Decrypt(HIK_HANDLE hCIPHER, U32 u32SrcPhyAddr,
			   U32 u32DestPhyAddr, U32 u32ByteLength)
{
	struct rt_crypto_request *request;
	HIK_DRIVER_HANDLE *phCIPHER;
	struct crypt_method *method;
	struct rt_crypto_obj *p_crypto_obj;
	//phCIPHER = (HIK_DRIVER_HANDLE*)hCIPHER.driver_pri;
	phCIPHER = (HIK_DRIVER_HANDLE *) hCIPHER;

	phCIPHER->method.src = (char *) u32SrcPhyAddr;
	phCIPHER->method.dst = (char *) u32DestPhyAddr;
	phCIPHER->method.text_len = u32ByteLength;
	debug_method_print(phCIPHER);

	method = &phCIPHER->method;
	request = &phCIPHER->request;

	request->data_src = (u8 *) method->src;
	request->data_dst = (u8 *) method->dst;

	request->data_size = method->text_len;
	request->key = (u8 *) method->key_buf;
	request->key_size = method->key_len;
	if (method->iv_len != 0) {
		request->iv = (u8 *) method->iv_buf;
		request->iv_size = method->iv_len;
	} else {
		request->iv = NULL;
		request->iv_size = 0;
	}
	//request->key_flag = HIK_OPEN_AES_FLAG_CPU;

	p_crypto_obj = phCIPHER->p_crypto_obj;

	rt_algo_crypto_setkey(p_crypto_obj, request);

	rt_algo_crypto_decrypt(p_crypto_obj, request);

	return 0;

}

extern long fh_efuse_ioctl(EFUSE_INFO *efuse_user_info, unsigned int cmd,
			   unsigned long arg);
S32 HIK_CRYPT_INTF_WriteOTPKey(U32 u32OptId, const U8 *pu8Key, U32 u32KeyLen)
{

	g_hik_handle.request.key_flag = HIK_OPEN_AES_FLAG_EFUSE;

	//set efuse
	EFUSE_INFO efuse_info_obj;
	EFUSE_INFO *efuse_info = &efuse_info_obj;
	efuse_info->efuse_entry_no = 0;
	efuse_info->key_buff = (unsigned char *) pu8Key;
	efuse_info->key_size = u32KeyLen;

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_CHECK_PRO, 0);

	if (!(efuse_info->status.protect_bits[0] & 1))
		printf("efuse write only once...\n");

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_CHECK_LOCK, 0);

	fh_efuse_ioctl(efuse_info, IOCTL_EFUSE_WRITE_KEY, 0);

	return 0;

}

#if(0)
static const unsigned int aes_cbc_key_buf[] = {
	0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c
};

static const unsigned int aes_cbc_iv_buf[] = {
	0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f
};

static const unsigned int plain_text[] = {
	0x6bc1bee2, 0x2e409f96, 0xe93d7e11, 0x7393172a,
	0xae2d8a57, 0x1e03ac9c, 0x9eb76fac, 0x45af8e51,
	0x30c81c46, 0xa35ce411, 0xe5fbc119, 0x1a0a52ef,
	0xf69f2445, 0xdf4f9b17, 0xad2b417b, 0xe66c3710
};

static const unsigned int aes_cbc_encrypt_text[] = {
	0x7649abac, 0x8119b246, 0xcee98e9b, 0x12e9197d,
	0x5086cb9b, 0x507219ee, 0x95db113a, 0x917678b2,
	0x73bed6b8, 0xe3c1743b, 0x7116e69e, 0x22229516,
	0x3ff1caa1, 0x681fac09, 0x120eca30, 0x7586e1a7
};
#endif

//
////ecb aes 256
//#define AES_ECB_KEY0	0x603deb10
//#define AES_ECB_KEY1	0x15ca71be
//#define AES_ECB_KEY2	0x2b73aef0
//#define AES_ECB_KEY3	0x857d7781
//#define AES_ECB_KEY4	0x1f352c07
//#define AES_ECB_KEY5	0x3b6108d7
//#define AES_ECB_KEY6	0x2d9810a3
//#define AES_ECB_KEY7	0x0914dff4
//
//
//static  unsigned int _plain_ecb_key[8] = {
//		AES_ECB_KEY0,AES_ECB_KEY1,
//		AES_ECB_KEY2,AES_ECB_KEY3,
//		AES_ECB_KEY4,AES_ECB_KEY5,
//		AES_ECB_KEY6,AES_ECB_KEY7,
//};
//
//static  unsigned int _plain_ecb_256_text[16] = {
//	0x6bc1bee2, 0x2e409f96, 0xe93d7e11, 0x7393172a,
//	0xae2d8a57, 0x1e03ac9c, 0x9eb76fac, 0x45af8e51,
//	0x30c81c46, 0xa35ce411, 0xe5fbc119, 0x1a0a52ef,
//	0xf69f2445, 0xdf4f9b17, 0xad2b417b, 0xe66c3710,
//};
//
//
//static  unsigned int  _cipher_ecb_256_text[16] = {
//	0xf3eed1bd, 0xb5d2a03c, 0x064b5a7e, 0x3db181f8,
//	0x591ccb10, 0xd410ed26, 0xdc5ba74a, 0x31362870,
//	0xb6ed21b9, 0x9ca6f4f9, 0xf153e7b1, 0xbeafed1d,
//	0x23304b7a, 0x39f9f3ff, 0x067d8d8f, 0x9e24ecc7,
//};
//

//================ aes ecb =====================
static const unsigned char aes_ecb_key_buf[] = {
	0x60,0x3d,0xeb,0x10, 0x15,0xca,0x71,0xbe, 0x2b,0x73,0xae,0xf0, 0x85,0x7d,0x77,0x81,
	0x1f,0x35,0x2c,0x07, 0x3b,0x61,0x08,0xd7, 0x2d,0x98,0x10,0xa3, 0x09,0x14,0xdf,0xf4,
};

//self test data .........start...
static __attribute__((aligned(32)))   const unsigned char plain_text[] = {
	0x6b,0xc1,0xbe,0xe2, 0x2e,0x40,0x9f,0x96, 0xe9,0x3d,0x7e,0x11, 0x73,0x93,0x17,0x2a,
	0xae,0x2d,0x8a,0x57, 0x1e,0x03,0xac,0x9c, 0x9e,0xb7,0x6f,0xac, 0x45,0xaf,0x8e,0x51,
	0x30,0xc8,0x1c,0x46, 0xa3,0x5c,0xe4,0x11, 0xe5,0xfb,0xc1,0x19, 0x1a,0x0a,0x52,0xef,
	0xf6,0x9f,0x24,0x45, 0xdf,0x4f,0x9b,0x17, 0xad,0x2b,0x41,0x7b, 0xe6,0x6c,0x37,0x10,
};


static __attribute__((aligned(32))) const unsigned char aes_ecb_encrypt_text[] =
{
	0xf3,0xee,0xd1,0xbd, 0xb5,0xd2,0xa0,0x3c, 0x06,0x4b,0x5a,0x7e, 0x3d,0xb1,0x81,0xf8,
	0x59,0x1c,0xcb,0x10, 0xd4,0x10,0xed,0x26, 0xdc,0x5b,0xa7,0x4a, 0x31,0x36,0x28,0x70,
	0xb6,0xed,0x21,0xb9, 0x9c,0xa6,0xf4,0xf9, 0xf1,0x53,0xe7,0xb1, 0xbe,0xaf,0xed,0x1d,
	0x23,0x30,0x4b,0x7a, 0x39,0xf9,0xf3,0xff, 0x06,0x7d,0x8d,0x8f, 0x9e,0x24,0xec,0xc7,
};

//		{
//			.hik_data.enAlg = HIK_CRYPT_INTF_ALG_AES,
//			.hik_data.enKeyLen = HIK_CRYPT_INTF_KEY_AES_128BIT,
//			.hik_data.enWorkMode = HIK_CRYPT_INTF_WORK_MODE_CBC,
//			.hik_data.iv_len = HIK_CRYPT_INTF_IV_KEY_AES_128BIT,
//			.key_buf = (u8*)aes_cbc_key_buf,
//			.key_size = sizeof(aes_cbc_key_buf),
//			.iv_buf =(u8*)aes_cbc_iv_buf,
//			.iv_size = sizeof(aes_cbc_iv_buf),
//			.en_cry_buf = (u8*)plain_text,
//			.de_cry_buf = (u8*)aes_cbc_encrypt_text,
//			.test_size = 64,
//		},


static __attribute__((aligned(32)))  const unsigned char aes_cbc_key_buf[] = {
	0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6, 0xab,0xf7,0x15,0x88, 0x09,0xcf,0x4f,0x3c,
};

static __attribute__((aligned(32)))  const unsigned char aes_cbc_iv_buf[] = {
	0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
};

//static __attribute__((aligned(32))) static const unsigned char plain_text[] = {
//	0x6b,0xc1,0xbe,0xe2, 0x2e,0x40,0x9f,0x96, 0xe9,0x3d,0x7e,0x11, 0x73,0x93,0x17,0x2a,
//	0xae,0x2d,0x8a,0x57, 0x1e,0x03,0xac,0x9c, 0x9e,0xb7,0x6f,0xac, 0x45,0xaf,0x8e,0x51,
//	0x30,0xc8,0x1c,0x46, 0xa3,0x5c,0xe4,0x11, 0xe5,0xfb,0xc1,0x19, 0x1a,0x0a,0x52,0xef,
//	0xf6,0x9f,0x24,0x45, 0xdf,0x4f,0x9b,0x17, 0xad,0x2b,0x41,0x7b, 0xe6,0x6c,0x37,0x10,
//};

static __attribute__((aligned(32)))  const unsigned char aes_cbc_encrypt_text[] = {
	0x76,0x49,0xab,0xac, 0x81,0x19,0xb2,0x46, 0xce,0xe9,0x8e,0x9b, 0x12,0xe9,0x19,0x7d,
	0x50,0x86,0xcb,0x9b, 0x50,0x72,0x19,0xee, 0x95,0xdb,0x11,0x3a, 0x91,0x76,0x78,0xb2,
	0x73,0xbe,0xd6,0xb8, 0xe3,0xc1,0x74,0x3b, 0x71,0x16,0xe6,0x9e, 0x22,0x22,0x95,0x16,
	0x3f,0xf1,0xca,0xa1, 0x68,0x1f,0xac,0x09, 0x12,0x0e,0xca,0x30, 0x75,0x86,0xe1,0xa7,
};

static const unsigned char out_buf[256] = { 0 };

void test_hik_aes(void)
{
	HIK_HANDLE hik_cry_handle;
	HIK_CRYPT_INTF_Init();
	HIK_CRYPT_INTF_CreateHandle(&hik_cry_handle);

	HIK_CRYPT_INTF_CTRL_S hikCtrl;
	hikCtrl.enAlg = HIK_CRYPT_INTF_ALG_AES;
	hikCtrl.enWorkMode = HIK_CRYPT_INTF_WORK_MODE_ECB;
	hikCtrl.iv_len = HIK_CRYPT_INTF_IV_KEY_AES_0BIT;
	hikCtrl.enKeyLen = HIK_CRYPT_INTF_KEY_AES_256BIT;
	memcpy(hikCtrl.u32Key, (char *) aes_ecb_key_buf,
	       sizeof(aes_ecb_key_buf));

	HIK_CRYPT_INTF_ConfigHandle(hik_cry_handle, &hikCtrl);
	//HIK_CRYPT_INTF_WriteOTPKey(0,aes_ecb_key_buf,sizeof(aes_ecb_key_buf));
	HIK_CRYPT_INTF_Encrypt(hik_cry_handle, (u32) plain_text,
			       (u32) & out_buf[0], sizeof(plain_text));

	if (0 != memcmp(&out_buf[0], aes_ecb_encrypt_text,
			sizeof(aes_ecb_encrypt_text)))
		printf("FAIL Encrypt test\n");

	else
		printf("PASS Encrypt test\n");

	//decrypt;
	HIK_CRYPT_INTF_Decrypt(hik_cry_handle, (u32) aes_ecb_encrypt_text,
			       (u32) & out_buf[0], sizeof(aes_ecb_encrypt_text));

	if (0 != memcmp(&out_buf[0], plain_text, sizeof(plain_text)))
		printf("FAIL Encrypt test\n");
	else
		printf("PASS Encrypt test\n");

	//cbc

	hikCtrl.enAlg = HIK_CRYPT_INTF_ALG_AES;
	hikCtrl.enWorkMode = HIK_CRYPT_INTF_WORK_MODE_CBC;
	hikCtrl.iv_len = HIK_CRYPT_INTF_IV_KEY_AES_128BIT;
	hikCtrl.enKeyLen = HIK_CRYPT_INTF_KEY_AES_128BIT;
	memcpy(hikCtrl.u32Key, (char *) aes_cbc_key_buf,
	       sizeof(aes_cbc_key_buf));
	memcpy(hikCtrl.u32IV, (char *) aes_cbc_iv_buf, sizeof(aes_cbc_iv_buf));

	HIK_CRYPT_INTF_ConfigHandle(hik_cry_handle, &hikCtrl);
	//HIK_CRYPT_INTF_WriteOTPKey(0,aes_ecb_key_buf,sizeof(aes_ecb_key_buf));
	HIK_CRYPT_INTF_Encrypt(hik_cry_handle, (u32) plain_text,
			       (u32) & out_buf[0], sizeof(plain_text));

	if (0 != memcmp(&out_buf[0], aes_cbc_encrypt_text,
			sizeof(aes_cbc_encrypt_text)))
		printf("FAIL Encrypt test\n");

	else
		printf("PASS Encrypt test\n");

	//decrypt;
	HIK_CRYPT_INTF_Decrypt(hik_cry_handle, (u32) aes_cbc_encrypt_text,
			       (u32) & out_buf[0], sizeof(aes_cbc_encrypt_text));

	if (0 != memcmp(&out_buf[0], plain_text, sizeof(plain_text)))
		printf("FAIL Encrypt test\n");
	else
		printf("PASS Encrypt test\n");

}
