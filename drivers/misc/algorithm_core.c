/*
 * File      : algorithm_core.c
 *
 * Copyright (c) 2016 Shanghai Fullhan Microelectronics Co., Ltd.
 * All rights reserved
 *
 */

#include "algorithm_core.h"

#ifdef ALGO_DEBUG
#define ALGO_DBG(fmt, args...)     \
    do                                  \
    {                                   \
        printf("[ALGO]: ");   \
        printf(fmt, ## args);       \
    }                                   \
    while(0)
#else
#define ALGO_DBG(fmt, args...)  do { } while (0)
#endif


#define ALGO_ERROR(fmt, args...)     \
    do                                  \
    {                                   \
        printf("[ALGO ERROR]: ");   \
        printf(fmt, ## args);       \
    }                                   \
    while(0)


#define _ALGO_CONTAINER_LIST_INIT(c,name)     {.type = c, .class_name = name,\
	 .list_head.next = &(rt_algo_container[c].list_head), \
	 .list_head.prev = &(rt_algo_container[c].list_head)}

#define ASSERT( expr) if( !( expr ) ) { \
		printf( "Assertion failed! %s:line %d \n", \
		__FUNCTION__,__LINE__); \
		while(1);\
		}


//here is a loose list to save diff algo basic class..
struct rt_algo_information rt_algo_container[RT_Algo_Class_Unknown] = {
	_ALGO_CONTAINER_LIST_INIT(RT_Algo_Class_Crypto, "crypto"),
};


int rt_algo_init(void)

{
	return 0;
}

static int rt_algo_crypto_register(void *obj, const char *obj_name,
				   void *private)
{
	struct rt_crypto_obj *crypto_obj;
	struct rt_crypto_obj *check_crypto_obj;
	crypto_obj = (struct rt_crypto_obj *) obj;

	//first check if the name has been register already..
	struct list_head *node;

	if ((obj_name == NULL))
		return -1;

	for (node = rt_algo_container[RT_Algo_Class_Crypto].list_head.next;
	     node
	     != &(rt_algo_container[RT_Algo_Class_Crypto].list_head);
	     node = node->next) {
		check_crypto_obj = list_entry(node, struct rt_crypto_obj, list);
		if (strncmp(check_crypto_obj->name, obj_name, MAX_NAME) == 0) {
			ALGO_ERROR("crypto has the same name algo..\n");
			return -1;

		}
	}

	INIT_LIST_HEAD(&crypto_obj->list);
	crypto_obj->driver_private = private;
	strncpy(crypto_obj->name, obj_name, MAX_NAME);
	//here insert "before" and test is "after" will make the list like a fifo...
	//rt_list_insert_before(&rt_algo_container[RT_Algo_Class_Crypto].list_head, &crypto_obj->list);
	list_add_tail(&crypto_obj->list,
		      &rt_algo_container[RT_Algo_Class_Crypto].list_head);
	return (0);
}

static struct rt_crypto_obj *rt_algo_crypto_find(char *name)
{

	struct rt_crypto_obj *object;
	struct list_head *node;
	struct rt_algo_information *information = NULL;

	if ((name == NULL))
		return NULL;

	/* try to find object */
	if (information == NULL)
		information = &rt_algo_container[RT_Algo_Class_Crypto];
	for (node = information->list_head.next;
	     node != &(information->list_head); node = node->next) {
		object = list_entry(node, struct rt_crypto_obj, list);
		if (strncmp(object->name, name, MAX_NAME) == 0)
			return object;
	}

	return NULL;
}

static void rt_algo_crypto_check_para(struct rt_crypto_obj *obj_p,
				      struct rt_crypto_request *request_p)
{

	ASSERT(obj_p != NULL);
	ASSERT(request_p != NULL);
	if (obj_p->flag == TRUE)
		ASSERT(request_p->iv != NULL);
	ASSERT(obj_p->max_key_size >= request_p->key_size);
	ASSERT(obj_p->min_key_size <= request_p->key_size);

	ASSERT(((u32)request_p->data_src % obj_p->src_allign_size) == 0);
	ASSERT(((u32)request_p->data_dst % obj_p->dst_allign_size) == 0);
	ASSERT(((u32)request_p->data_size % obj_p->block_size) == 0);

}

int rt_algo_crypto_setkey(struct rt_crypto_obj *obj_p,
			  struct rt_crypto_request *request_p)
{

	//para check...
	rt_algo_crypto_check_para(obj_p, request_p);
	if (obj_p->ops.set_key)
		obj_p->ops.set_key(obj_p, request_p);

	return 0;
}

int rt_algo_crypto_encrypt(struct rt_crypto_obj *obj_p,
			   struct rt_crypto_request *request_p)
{

	//para check...
	rt_algo_crypto_check_para(obj_p, request_p);

	if (obj_p->ops.encrypt)
		obj_p->ops.encrypt(obj_p, request_p);
	return 0;
}

int rt_algo_crypto_decrypt(struct rt_crypto_obj *obj_p,
			   struct rt_crypto_request *request_p)
{

	//para check...
	rt_algo_crypto_check_para(obj_p, request_p);

	if (obj_p->ops.decrypt)
		obj_p->ops.decrypt(obj_p, request_p);

	return 0;
}

int rt_algo_register(u8 type, void *obj, const char *obj_name, void *private)
{

	int ret;
	if ((obj_name == NULL) || (type >= RT_Algo_Class_Unknown)
	    || (obj == NULL))
		return -1;

	switch (type) {
	case RT_Algo_Class_Crypto:
		ALGO_DBG("'Crypto' register name is %s\n", obj_name);
		ret = rt_algo_crypto_register(obj, obj_name, private);
		break;
	default:
		ret = -2;
		ALGO_ERROR("please check the type..\n");
	}

	return ret;
}

int rt_algo_unregister(u8 type, void *obj)
{

	return (0);

}

void *rt_algo_obj_find(u8 type, char *name)
{
	void *obj;
	//struct rt_crypto_obj *crypto_obj;
	if ((name == NULL) || (type >= RT_Algo_Class_Unknown))
		return NULL;

	switch (type) {
	case RT_Algo_Class_Crypto:
		ALGO_DBG("'Crypto' find name is %s\n", name);
		obj = rt_algo_crypto_find(name);
		break;
	default:
		obj = NULL;
		break;

	}
	return obj;
}
