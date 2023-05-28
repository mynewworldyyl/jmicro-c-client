#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "jm_client.h"
#include "jm_mem.h"

#include "test.h"

#include<pthread.h>
//#include<Windows.h>
#pragma comment(lib, "pthreadVC2.lib")  //必须加上这句

extern void test_jm_client_pubsub(void* Param);
extern BOOL udp_client_recv_one_loop();
extern void udp_client_ws_init();

static uint8_t test_onPubsubItemType1Listener(jm_pubsub_item_t *it) {
	msg_extra_data_t* ehost = extra_sget(it->cxt, EXTRA_SKEY_UDP_HOST);
	msg_extra_data_t* pport = extra_sget(it->cxt, EXTRA_SKEY_UDP_PORT);
	msg_extra_data_t* isUdphost = extra_sget(it->cxt, EXTRA_SKEY_UDP_ACK);

	char *host = ehost == NULL? "":ehost->value.bytesVal;
	sint32_t port = pport == NULL? "":pport->value.s32Val;
	sint8_t isUdp = isUdphost == NULL? "":isUdphost->value.s8Val;

	printf("test_onPubsubItemType1Listener: host= %s, port= %d, isUdphost= %d \n",host, port, isUdp);

	jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true);
	if(item == NULL) {
		INFO("test_publishJsonItem create PS item fail\n");
		return MEMORY_OUTOF_RANGE;
	}

	item->id = it->id;//应答报文
	item->callback = it->callback;

	client_initPubsubItem(item, FLAG_DATA_STRING);

	item->data = "test content";
	item->type = 1;
	item->topic = TOPIC_P2P;

	INFO("test_publishJsonItem publish item topic: %s, %u\n", item->topic, item->id);

	msg_extra_data_t *ex = extra_putChars(NULL, EXTRA_KEY_UDP_HOST, host, os_strlen(host));
	ex = extra_putInt(ex, EXTRA_KEY_UDP_PORT,port);
	ex = extra_putByte(ex, EXTRA_KEY_UDP_ACK,1);

	client_send_msg_result_t rst = client_publishPubsubItem(item, ex);

	extra_release(item->cxt);

	cache_back(CACHE_PUBSUB_ITEM,item);

	return JM_SUCCESS;
}

int create_tcpclient_thread() {
     pthread_t pid;
     pthread_create(&pid, NULL, test_jm_client_pubsub,NULL);
     return 1;
}

int test_ctrl_jm()
{
	INFO("test_udp_jm_client_pubsub starting\n");

	create_tcpclient_thread();

	//client_init("test00","1",false);

	udp_client_ws_init();

	ctrl_init();

	/*if(client_subscribeP2PByType(test_onPubsubItemType1Listener,-128)) {
		printf("test_jmLoginListener1注册成功\n");
	} else {
		printf("test_jmLoginListener1注册失败\n");
	}
*/
	while(1) {
		//sleep(1);
		//printf("One loop: ");
		if(!udp_client_recv_one_loop()) {
			printf("udp_client_recv_one_loop:fail\n");
		}
	}
}

