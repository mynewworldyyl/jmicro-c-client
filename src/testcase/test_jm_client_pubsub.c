#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "../jm_client.h"
#include "../jm_buffer.h"
#include "../debug.h"

extern BOOL client_recv_one_loop();

extern void client_ws_init();

static uint8_t test_onPubsubItemType2Listener(jm_pubsub_item_t *item) {
	printf("test_onPubsubItemType2Listener: data= %s, fr= %d, type= %d \n",item->data->data, item->fr, item->type);

	//client_subscribeByType();
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
	return SUCCESS;
}

static uint8_t test_onPubsubItemType1Listener(jm_pubsub_item_t *item) {
	printf("test_onPubsubItemType1Listener: data= %s, fr= %d, type= %d \n",item->data->data, item->fr, item->type);

	//client_subscribeByType();
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
	return SUCCESS;
}

static uint8_t test_onPubsubItemType3Listener(jm_pubsub_item_t *item) {
	printf("test_onPubsubItemType3Listener: data= %s, fr= %d, type= %d \n",item->data->data, item->fr, item->type);

	//client_subscribeByType();
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
	return SUCCESS;
}

static void test_jmLoginListener(sint32_t code, char *msg, char *loginKey, sint32_t actId) {
	printf("Listener1 got login result: %s, %s, %d, %d\n",loginKey,msg,code,actId);

	printf("test_jmLoginListener 订阅异步消息 begin \n");

	if(client_subscribeByType(test_onPubsubItemType1Listener,1)) {
		printf("test_jmLoginListener1 订阅异步消息成功  \n");
	} else {
		printf("test_jmLoginListener1 订阅异步消息失败  \n");
	}

	if(client_subscribeByType(test_onPubsubItemType2Listener,2)) {
		printf("test_jmLoginListener2 订阅异步消息成功  \n");
	} else {
		printf("test_jmLoginListener2 订阅异步消息失败  \n");
	}

	if(client_subscribeByType(test_onPubsubItemType3Listener,3)) {
		printf("test_jmLoginListener3 订阅异步消息成功  \n");
	} else {
		printf("test_jmLoginListener3 订阅异步消息失败  \n");
	}

	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
}

/*static void test_jmLoginListener2(sint32_t code, char *msg, char *loginKey, sint32_t actId) {
	printf("Listener2 got login result: %s, %s, %d, %d\n",loginKey,msg,code,actId);

	printf("client_unregistLoginListener2 begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener2 succesfully\n");
}

static void test_jmLoginListener3(sint32_t code, char *msg, char *loginKey, sint32_t actId) {
	printf("Listener3 got login result: %s, %s, %d, %d\n",loginKey,msg,code,actId);

	printf("client_unregistLoginListener3 begin \n");
	client_unregistLoginListener(test_jmLoginListener3);
	printf("client_unregistLoginListener succesfully\n");
}*/



int test_jm_client_pubsub()
{
	INFO("test_jm_client_pubsub starting");

	setbuf(stdout,NULL);

	client_ws_init();//建立TCP连接
	client_init("test00","1");//初始化客户端,登录物联网平台

	client_registLoginListener(test_jmLoginListener);
	//client_registLoginListener(test_jmLoginListener2);
	//client_registLoginListener(test_jmLoginListener3);

	while(1) {
		//sleep(1);
		//printf("One loop: ");
		if(!client_recv_one_loop()) {
			printf("client_recv_one_loop:fail\n");
		}
	}
}

