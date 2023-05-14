#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "../jm_client.h"
#include "../jm_buffer.h"
#include "../debug.h"
#include "../jm_mem.h"

#include "./test.h"

extern BOOL client_recv_one_loop();

extern void client_ws_init();

static uint8_t test_onPubsubItemType2Listener(jm_pubsub_item_t *item) {
	printf("test_onPubsubItemType2Listener: data= %s, fr= %d, type= %d \n",item->data, item->fr, item->type);

	//client_subscribeByType();
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
	return JM_SUCCESS;
}

static uint8_t test_onPubsubItemType1Listener(jm_pubsub_item_t *item) {
	printf("test_onPubsubItemType1Listener: data= %s, fr= %d, type= %d \n",item->data, item->fr, item->type);

	//client_subscribeByType();
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
	return JM_SUCCESS;
}

static uint8_t test_onPubsubItemType3Listener(jm_pubsub_item_t *item) {
	printf("test_onPubsubItemType3Listener: data= %s, fr= %d, type= %d \n",item->data, item->fr, item->type);

	//client_subscribeByType();
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
	return JM_SUCCESS;
}

static void test_jmLoginListener(sint32_t code, char *msg, char *loginKey, sint32_t actId) {
	printf("Listener1 got login result: %s, %s, %d, %d\n",loginKey,msg,code,actId);

	printf("test_jmLoginListener �����첽��Ϣ begin \n");

	if(client_subscribeByType(test_onPubsubItemType1Listener,1)) {
		printf("test_jmLoginListener1 �����첽��Ϣ�ɹ�  \n");
	} else {
		printf("test_jmLoginListener1 �����첽��Ϣʧ��  \n");
	}

	if(client_subscribeByType(test_onPubsubItemType2Listener,2)) {
		printf("test_jmLoginListener2 �����첽��Ϣ�ɹ�  \n");
	} else {
		printf("test_jmLoginListener2 �����첽��Ϣʧ��  \n");
	}

	if(client_subscribeByType(test_onPubsubItemType3Listener,3)) {
		printf("test_jmLoginListener3 �����첽��Ϣ�ɹ�  \n");
	} else {
		printf("test_jmLoginListener3 �����첽��Ϣʧ��  \n");
	}

	char *topic = "/__act/dev/25500/testDeivceMsg";
	sint8_t type = -126;
	char *content = "Hello, I am esp8266 device";
	//msg_extra_data_t *extra = NULL;

	//client_publishStrItemByTopic(topic, type, content);
	//test_publishJsonItem(topic);
	//test_publishExtraDataItem(topic);
	test_publishBinDataItem(topic);
	/*
	printf("client_unregistLoginListener begin \n");
	client_unregistLoginListener(test_jmLoginListener);
	printf("client_unregistLoginListener succesfully\n");
	*/
}

ICACHE_FLASH_ATTR client_send_msg_result_t test_publishJsonItem(char *topic){
	if(topic == NULL || os_strlen(topic) == 0) {
		INFO("test_publishJsonItem topic is NULL\n");
		return INVALID_PS_DATA;
	}

	sint8_t type = 0;
	char *jsonContent = "{\"msg\":\"hello world jmicro pubsub\" }";

	jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true);
	if(item == NULL) {
		INFO("test_publishJsonItem create PS item fail\n");
		return MEMORY_OUTOF_RANGE;
	}

	client_initPubsubItem(item, FLAG_DATA_JSON);
	item->data = jsonContent;
	item->type = type;
	item->topic = topic;

	INFO("test_publishJsonItem publish item topic: %s, %u\n", item->topic, item->id);

	client_send_msg_result_t rst = client_publishPubsubItem(item, client_topicForwardExtra(topic));

	extra_release(item->cxt);

	cache_back(CACHE_PUBSUB_ITEM,item);
	//os_free(item);

	//if(buf)
	//	bb_release(buf);

	return rst;
}

ICACHE_FLASH_ATTR client_send_msg_result_t test_publishExtraDataItem(char *topic){
	if(topic == NULL || os_strlen(topic) == 0) {
		INFO("test_publishExtraDataItem topic is NULL\n");
		return INVALID_PS_DATA;
	}

	jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true);
	if(item == NULL) {
		INFO("test_publishExtraDataItem create PS item fail\n");
		return MEMORY_OUTOF_RANGE;
	}


	client_initPubsubItem(item, FLAG_DATA_EXTRA);

	item->type = 0;
	item->topic = topic;

	msg_extra_data_t *header = extra_strKeyPut(NULL, "psItemExtraData0", PREFIX_TYPE_BYTE);
	header->value.s8Val = 33;

	char *pstr = "hello ps extra data";
	msg_extra_data_t *extraData = extra_strKeyPut(header, "psItemExtraData1", PREFIX_TYPE_STRINGG);
	extraData->value.bytesVal = pstr;
	extraData->len = strlen(pstr);
	extraData->neddFreeBytes = false;

	item->data = header;

	INFO("test_publishExtraDataItem publish item topic: %s, %u\n", item->topic, item->id);

	client_send_msg_result_t rst = client_publishPubsubItem(item, client_topicForwardExtra(topic));

	extra_release(item->cxt);

	cache_back(CACHE_PUBSUB_ITEM,item);

	return rst;
}

ICACHE_FLASH_ATTR client_send_msg_result_t test_publishBinDataItem(char *topic){
	if(topic == NULL || os_strlen(topic) == 0) {
		INFO("test_publishExtraDataItem topic is NULL\n");
		return INVALID_PS_DATA;
	}

	jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true);
	if(item == NULL) {
		INFO("test_publishExtraDataItem create PS item fail\n");
		return MEMORY_OUTOF_RANGE;
	}

	client_initPubsubItem(item, FLAG_DATA_BIN);

	item->type = 0;
	item->topic = topic;

	byte_buffer_t *data = bb_create(4);

	bb_put_s32(data, 128);

	//INFO("%u, %u\n", data->data[0], data->data[1]);
	//INFO("%u, %u\n", data->data[2], data->data[3]);

	//char arr[4]={data->data[0], data->data[1],data->data[2], data->data[3]};

	/*sint32_t wv=0;
	bb_get_s32(data,&wv);*/

	item->data = data;

	//INFO("%u, %u\n", item->data->data[0], item->data->data[1]);
	//INFO("%u, %u\n", item->data->data[2], item->data->data[3]);

	INFO("test_publishExtraDataItem publish item topic: %s, %u\n", item->topic, item->id);

	client_send_msg_result_t rst = client_publishPubsubItem(item, client_topicForwardExtra(topic));

	bb_release(data);

	extra_release(item->cxt);

	cache_back(CACHE_PUBSUB_ITEM,item);


	return rst;
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



	client_ws_init();
	client_init("test00","1",true);

	client_registLoginListener(test_jmLoginListener);
	//client_registLoginListener(test_jmLoginListener2);
	//client_registLoginListener(test_jmLoginListener3);

	while(1) {
		//sleep(1);
		//printf("One loop: ");
		if(!client_recv_one_loop()) {
			//printf("client_recv_one_loop:fail\n");
		}
	}
}

