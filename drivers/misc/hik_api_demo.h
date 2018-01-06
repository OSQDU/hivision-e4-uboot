/*
 * hik_api_demo.h
 *
 *  Created on: Aug 21, 2016
 *      Author: duobao
 */

#ifndef HIK_API_DEMO_H_
#define HIK_API_DEMO_H_

#include "algorithm_core.h"

typedef unsigned int	   		U32;
typedef signed int	   			S32;
typedef unsigned short	  		U16;
typedef signed short	  		S16;
typedef unsigned char	  		U8;
typedef signed char	  			S8;


typedef U32 HIK_HANDLE;

#if(1)
typedef enum hikHIK_CRYPT_ERROR_E
{
	HIK_ERR_CRYPT_NOT_INIT  = 0x804D0001,
	HIK_ERR_CRYPT_INVALID_HANDLE = 0x804D0002,
	HIK_ERR_CRYPT_INVALID_POINT  = 0x804D0003,
	HIK_ERR_CRYPT_INVALID_PARA = 0x804D0004,
	HIK_ERR_CRYPT_FAILED_INIT  = 0x804D0005,
	HIK_ERR_CRYPT_FAILED_GETHANDLE = 0x804D0006,
	HIK_FAILURE  = -1,
}HIK_CRYPT_ERROR_E;

/*
错误代码			宏定义								描述
0x804D0001		HIK_ERR_CRYPT_NOT_INIT				设备未初始化
0x804D0002		HIK_ERR_CRYPT_INVALID_HANDLE		Handle 号无效
0x804D0003		HIK_ERR_CRYPT_INVALID_POINT			参数中有空指针
0x804D0004		HIK_ERR_CRYPT_INVALID_PARA			无效参数
0x804D0005		HIK_ERR_CRYPT_FAILED_INIT			初始化失败
0x804D0006		HIK_ERR_CRYPT_FAILED_GETHANDLE		获取 handle 失败
-1				HIK_FAILURE							操作失败
*/


typedef enum hikHIK_CRYPT_INTF_WORK_MODE_E
{
	HIK_CRYPT_INTF_WORK_MODE_ECB,
	HIK_CRYPT_INTF_WORK_MODE_CBC,
	HIK_CRYPT_INTF_WORK_MODE_CFB,
	HIK_CRYPT_INTF_WORK_MODE_OFB,
	HIK_CRYPT_INTF_WORK_MODE_CTR,
	HIK_CRYPT_INTF_WORK_MODE_CCM,
	HIK_CRYPT_INTF_WORK_MODE_GCM,
	HIK_CRYPT_INTF_WORK_MODE_CBC_CTS,
	HIK_CRYPT_INTF_WORK_MODE_BUTT    = 0xffffffff
}HIK_CRYPT_INTF_WORK_MODE_E;
/*
成员名称								描述
HIK_CRYPT_INTF_WORK_MODE_ECB		ECB（Electronic CodeBook）模式
HIK_CRYPT_INTF_WORK_MODE_CBC		CBC（Cipher Block Chaining）模式
HIK_CRYPT_INTF_WORK_MODE_CFB		CFB（Cipher FeedBack）模式
HIK_CRYPT_INTF_WORK_MODE_OFB		OFB（Output FeedBack）模式
HIK_CRYPT_INTF_WORK_MODE_CTR		CTR（Counter）模式
HIK_CRYPT_INTF_WORK_MODE_CCM		CCM（Counter with Cipher Block Chaini ng-Message Authentication  ）模式
HIK_CRYPT_INTF_WORK_MODE_GCM		GCM（Galois/Counter Mode）模式
HIK_CRYPT_INTF_WORK_MODE_CBC_CTS	CBC CTS（Community Tissue Services）模式
HIK_CRYPT_INTF_WORK_MODE_BUTT		无效模式
*/

typedef enum hikHIK_CRYPT_INTF_ALG_E
{
	HIK_CRYPT_INTF_ALG_DES  = 0x0,
	HIK_CRYPT_INTF_ALG_3DES = 0x1,
	HIK_CRYPT_INTF_ALG_AES  = 0x2,
	HIK_CRYPT_INTF_ALG_BUTT = 0x3
}HIK_CRYPT_INTF_ALG_E;
/*
成员名称								描述
HIK_CRYPT_INTF_ALG_DES				DES 算法
HIK_CRYPT_INTF_ALG_3DES				3DES 算法
HIK_CRYPT_INTF_ALG_AES				AES 算法
*/


typedef enum hikHIK_CRYPT_INTF_KEY_LENGTH_E
{
	HIK_CRYPT_INTF_KEY_AES_128BIT = 0x0,
	HIK_CRYPT_INTF_KEY_AES_192BIT = 0x1,
	HIK_CRYPT_INTF_KEY_AES_256BIT = 0x2,
	HIK_CRYPT_INTF_KEY_DES_3KEY = 0x3,
	HIK_CRYPT_INTF_KEY_DES_2KEY = 0x4,
	HIK_CRYPT_INTF_KEY_DES = 0x5,
}HIK_CRYPT_INTF_KEY_LENGTH_E;



typedef enum hikHIK_CRYPT_INTF_IV_KEY_LENGTH_E
{
	HIK_CRYPT_INTF_IV_KEY_AES_0BIT = 0x0,
	HIK_CRYPT_INTF_IV_KEY_AES_64BIT = 0x1,
	HIK_CRYPT_INTF_IV_KEY_AES_128BIT = 0x2,

}HIK_CRYPT_INTF_IV_KEY_LENGTH_E;


/*
成员名称								描述
HIK_CRYPT_INTF_KEY_AES_128BIT		AES 运算方式下采用 128bit 密钥长度
HIK_CRYPT_INTF_KEY_AES_192BIT		AES 运算方式下采用 192bit 密钥长度
HIK_CRYPT_INTF_KEY_AES_256BIT		AES 运算方式下采用 256bit 密钥长度
HIK_CRYPT_INTF_KEY_DES_3KEY			3DES 运算方式下采用 3 个 key
HIK_CRYPT_INTF_KEY_DES_2KEY			3DES 运算方式下采用 2 个 key
*/



typedef enum hikHIK_CRYPT_INTF_BIT_WIDTH_E
{
	HIK_CRYPT_INTF_BIT_WIDTH_64BIT  = 0x0,
	HIK_CRYPT_INTF_BIT_WIDTH_8BIT   = 0x1,
	HIK_CRYPT_INTF_BIT_WIDTH_1BIT   = 0x2,
	HIK_CRYPT_INTF_BIT_WIDTH_128BIT = 0x3,
}HIK_CRYPT_INTF_BIT_WIDTH_E;

/*
成员名称								描述
HIK_CRYPT_INTF_BIT_WIDTH_64BIT		64bit 位宽
HIK_CRYPT_INTF_BIT_WIDTH_8BIT		8bit 位宽
HIK_CRYPT_INTF_BIT_WIDTH_1BIT		1bit 位宽
HIK_CRYPT_INTF_BIT_WIDTH_128BIT		128bit 位宽
*/



typedef enum hikHIK_CRYPT_INTF_KEY_SRC_E
{
	HIK_CRYPT_INTF_KEY_SRC_USER = 0x0,
	HIK_CRYPT_INTF_KEY_SRC_EFUSE_0,
	HIK_CRYPT_INTF_KEY_SRC_EFUSE_1,
	HIK_CRYPT_INTF_KEY_SRC_EFUSE_2,
	HIK_CRYPT_INTF_KEY_SRC_EFUSE_3,
	HIK_CRYPT_INTF_KEY_SRC_BUTT
} HIK_CRYPT_INTF_KEY_SRC_E;

/*
成员名称								描述
HIK_CRYPT_INTF_KEY_SRC_USER			用户配置的 Key
HIK_CRYPT_INTF_KEY_SRC_EFUSE_0		Efuse 的第 0 组 Key
HIK_CRYPT_INTF_KEY_SRC_EFUSE_1		Efuse 的第 1 组 Key
HIK_CRYPT_INTF_KEY_SRC_EFUSE_2		Efuse 的第 2 组 Key
HIK_CRYPT_INTF_KEY_SRC_EFUSE_3		Efuse 的第 3 组 Key
HIK_CRYPT_INTF_KEY_SRC_BUTT			无效类型
*/


typedef struct hikUNF_CIPHER_CCM_INFO_S
{
	U8  u8Nonce[16];
	U8 *pu8Aad;
	U32 u32ALen;
	U32 u32MLen;
	U8  u8NLen;
	U8  u8TLen;
	U8  u8Reserve[2];
}HIK_CRYPT_INTF_CCM_INFO_S;

/*
成员名称								描述
u8Nonce								CCM 模式下的 NONCE 数据
pu8Aad								CCM 模式下的额外数据 A 的指针
u32ALen								CCM 模式下的额外数据的长度
u32MLen								消息数据的长度
u8NLen								NONCE 数据的长度
u8TLen								CCM 模式下的校验值 TAG 的长度
u8Reserve							保留字段
【注意事项】
额外数据 A 只参与校验值的生成，不参与数据的加解密,    即其仅影响 TAG 的
值，不会影响加解密结果。
【相关数据类型及接口】
无。
*/

typedef struct hikUNF_CIPHER_GCM_INFO_S
{
	U8 *pu8Aad;
	U32 u32ALen;
	U32 u32MLen;
	U32 u32IVLen;
}HIK_CRYPT_INTF_GCM_INFO_S;

/*
成员名称								描述
pu8Aad								GCM 模式下的额外数据 A 的指针
u32ALen								GCM 模式下的额外数据的长度
u32MLen								消息数据的长度
u32IVLen							GCM 模式下的 IV 的长度
【注意事项】
额外数据 A 只参与校验值的生成，不参与数据的加解密,    即其仅影响 TAG 的
值，不会影响加解密结果。
u32IVLen 的值必须大于或等于 1，且小于或等于 16。
【相关数据类型及接口】
无。
*/


typedef struct hikHIK_CRYPT_CTRL_CHANGE_FLAG_S
{
	U32   bit1IV:     1;
	U32   bitsResv:  31;
} HIK_CRYPT_INTF_CTRL_CHANGE_FLAG_S;

/*
【成员】
成员名称								描述
bit1IV								向量变更
bitsResv  							保留
【注意事项】
无。
【相关数据类型及接口】
无。
*/


typedef struct hikHIK_CRYPT_INTF_CTRL_S
{
	U32                     u32Key[8];
	U32                     u32IV[4];
	HIK_CRYPT_INTF_ALG_E        enAlg;
	HIK_CRYPT_INTF_BIT_WIDTH_E  enBitWidth;
	HIK_CRYPT_INTF_WORK_MODE_E  enWorkMode;
	HIK_CRYPT_INTF_KEY_LENGTH_E enKeyLen;
	HIK_CRYPT_INTF_IV_KEY_LENGTH_E iv_len;
	HIK_CRYPT_INTF_CTRL_CHANGE_FLAG_S stChangeFlags;
	HIK_CRYPT_INTF_KEY_SRC_E enKeySrc;

	union
	{
	HIK_CRYPT_INTF_CCM_INFO_S stCCM;
	HIK_CRYPT_INTF_GCM_INFO_S stGCM;
	}unModeInfo;
} HIK_CRYPT_INTF_CTRL_S;
/*
成员名称								描述
u32Key[8]							密钥
u32IV[4]							初始向量
enAlg  								加密算法
enBitWidth  						加密或解密的位宽
enWorkMode  						工作模式
enKeyLen							密钥长度
stChangeFlags  						更新标志位，表示 IV 等是否需要更新
enKeySrc							密钥的来源
unModeInfo 							模式信息

【注意事项】
ECB 模式下不需要初始向量。
结构体中的成员 unModeInfo 对 HI3518EV200 无效。
【相关数据类型及接口】
无
*/


typedef struct hikHIK_CRYPT_INTF_DATA_S
{
	U32 u32SrcPhyAddr;
	U32 u32DestPhyAddr;
	U32 u32ByteLength;
} HIK_CRYPT_INTF_DATA_S;
/*

成员名称								描述
u32SrcPhyAddr：						源数据物理地址
u32DestPhyAddr：						目的数据物理地址
u32ByteLength：						加解密数据长度
【注意事项】
无。
【相关数据类型及接口】
无。
*/

/****************************************************************************
 * func section
 *  add func in this file  here
 ***************************************************************************/


S32 HIK_CRYPT_INTF_Init(void);
/*

	功能说明
初始化加密模块设备。
	输入参数
无
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
*/



S32 HIK_CRYPT_INTF_DeInit(void);
/*

	功能说明
去初始化加密模块设备。
	输入参数
无。
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
*/



S32 HIK_CRYPT_INTF_CreateHandle(HIK_HANDLE* phCIPHER);
/*

	功能说明
创建一路加密通道，并获取句柄。
	输入参数
phCIPHER：加解密通道句柄；
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
*/


S32 HIK_CRYPT_INTF_DestroyHandle(HIK_HANDLE hCIPHER);

/*

	功能说明
销毁一路加密句柄。
	输入参数
hCIPHER：加解密通道句柄；
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
*/



S32 HIK_CRYPT_INTF_ConfigHandle(HIK_HANDLE hCIPHER, HIK_CRYPT_INTF_CTRL_S* pstCtrl);
/*
	功能说明
配置加解密控制信息。详细配置请参见结构体HIK_CRYPT_INTF_CTRL_S。
	输入参数
hCIPHER：加解密通道句柄；
pstCtrl：控制信息指针。
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
*/


S32 HIK_CRYPT_INTF_Encrypt(HIK_HANDLE hCIPHER, U32 u32SrcPhyAddr, U32 u32DestPhyAddr, U32 u32ByteLength);
/*

	功能说明
对数据进行加密。
	输入参数
hCIPHER：加解密通道句柄；
u32SrcPhyAddr：源数据（待加密的数据）的物理地址；
u32DestPhyAddr：存放加密结果的物理地址；
u32ByteLength：数据的长度（单位：字节）；
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	加密通道句柄必须已创建。
	可多次调用。
	数据的长度至少为 16 字节，且不能大于或等于 1024*1024 字节。
*/




S32 HIK_CRYPT_INTF_Decrypt(HIK_HANDLE hCIPHER, U32 u32SrcPhyAddr, U32 u32DestPhyAddr, U32 u32ByteLength);
/*
	功能说明
对数据进行解密。
	输入参数
hCIPHER：加解密通道句柄；
u32SrcPhyAddr：源数据（待解密的数据）的物理地址；
u32DestPhyAddr：存放解密结果的物理地址；
u32ByteLength：数据的长度（单位：字节）；
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	加密通道句柄必须已创建。
	可多次调用。
	数据的长度至少为 16 字节，且不能大于或等于 1024*1024 字节。
*/



S32 HIK_CRYPT_INTF_EncryptMulti(HIK_HANDLE hCIPHER, HIK_CRYPT_INTF_DATA_S *pstDataPkg, U32 u32DataPkgNum);
/*
	功能说明
进行多个包数据的加密。
	输入参数
hCIPHER：加解密通道句柄；
*pstDataPkg：待加密的数据包；
u32DataPkgNum：待加密的数据包个数；
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	加密通道句柄必须已创建。
	可多次调用。
	每次加密的数据包个数最多不超过 128 个。
	对于多个包的操作，每个包都使用HIK_CRYPT_INTF_ConfigHandle 配置的向量进行运算，前一个包的向量运算结果不会作用于下一个包的运算，每个包都是独立运算的。前一次函数调用的结果也不会影响后一次函数调用的运算结果。
*/

S32 HIK_CRYPT_INTF_DecryptMulti(HIK_HANDLE hCIPHER, HIK_CRYPT_INTF_DATA_S*pstDataPkg, U32 u32DataPkgNum);
/*
	功能说明
进行多个包数据的解密。
	输入参数
hCIPHER：加解密通道句柄；
*pstDataPkg：待解密的数据包；
u32DataPkgNum：待解密的数据包个数。
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	加密通道句柄必须已创建。
	可多次调用。
	每次解密的数据包个数最多不超过 128 个。
	对于多个包的操作，每个包都使用HIK_CRYPT_INTF_ConfigHandle 配置的向量进行运算，前一个包的向量运算结果不会作用于下一个包的运算，每个包都是独立运算的。前一次函数调用的结果也不会影响后一次函数调用的运算结果。
*/

S32 HIK_CRYPT_INTF_EncryptMultiEx(HIK_HANDLE hCIPHER,
		HIK_CRYPT_INTF_CTRL_S* pstCtrl, HIK_CRYPT_INTF_DATA_S *pstDataPkg,
		U32 u32DataPkgNum);
/*
	功能说明
进行多个包数据的加密。
	输入参数
hCIPHER：加解密通道句柄；
pstCtrl：控制信息指针；
*pstDataPkg：待加密的数据包；
u32DataPkgNum：待加密的数据包个数。
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	加密通道句柄必须已创建。
	调用前无需调用HIK_CRYPT_INTF_ConfigHandle。
	可多次调用。
	每次解密的数据包个数最多不超过 128 个。
	对于多个包的操作，每个包都使用HIK_CRYPT_INTF_ConfigHandle 配置的向量进行运算，前一个包的向量运算结果不会作用于下一个包的运算，每个包都是独立运算的。前一次函数调用的结果也不会影响后一次函数调用的运算结果。
	CCM 和 GCM 模式不支持多包加解密。
*/




S32 HIK_CRYPT_INTF_GetTag(HIK_HANDLE hCIPHER, U8 *pstTag);
/*
	功能说明
CCM/GCM  模式加解密后获取 TAG 值。
	输入参数
hCIPHER：加解密通道句柄；
pstTag：TAG 值；
	输出参数
无。
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	只有在 CCM、GCM 模式下此接口才有效。
*/

S32 HIK_CRYPT_INTF_GetRandomNumber(U32 *pu32RandomNumber);
/*
	功能说明
生成随机数。
	输入参数
pu32RandomNumber：输出的随机数；
	输出参数
pu32RandomNumber：输出的随机数；
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	无
*/


S32 HIK_CRYPT_INTF_WriteOTPKey(U32 u32OptId, const U8 *pu8Key, U32 u32KeyLen);
/*
	功能说明
烧写 Key 到 OTP 区域。
	输入参数
u32OptId：烧写的 OTP 区域，取值范围[0, 3]；
pu8Key：待烧写的 Key 数据；
u32KeyLen：Key 数据长度，最大不超过 16，单位是 byte。
	输出参数
无；
	返回值
0：函数调用成功。
其他：失败，函数调用失败的原因见出错信息。
	注意
	调用此接口之前，加解密模块必须已初始化。
	可烧写的 OTP 区域共有 4 个，分别为 0，1，2，3。
	每个 OTP 区域只能烧写一次，且不能读取。烧写的 Key 长度最大不超过 16 个字节。
*/
#endif
#endif /* HIK_API_DEMO_H_ */
