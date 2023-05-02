#include "jm_client.h"

#include "jm_mem.h"

#ifndef WIN32
#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#endif

#ifdef WIN32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./testcase/test.h"
#endif

#include "c_types.h"
#include "debug.h"
#include "jm_constants.h"
#include "jm_stdcimpl.h"
#include "jm_buffer.h"

//閿熸枻鎷锋伅閿熸枻鎷稭essage閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹褰遍敓鏂ゆ嫹
typedef struct msg_handler_register_item{
	sint8_t type;
	client_msg_hander_fn handler;
	struct msg_handler_register_item *next;
} CHRI;

//RPC閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿燂拷
typedef struct _c_msg_result{
	BOOL in_used;
	sint32_t msg_id;
	//jm_msg_t *msg;
	client_rpc_callback_fn callback;
	void *cbArg;
	struct _c_msg_result *next;//閿熸枻鎷蜂竴閿熸枻鎷峰厓閿熸枻鎷�
	//struct _c_msg_result *pre;//鍓嶄竴閿熸枻鎷峰厓閿熸枻鎷�
} client_msg_result_t;

//閿熸枻鎷锋伅閿熸枻鎷稰SData閿熸枻鎷烽敓鏂ゆ嫹閿熶茎纭锋嫹閿熸枻鎷烽敓鏂ゆ嫹
typedef struct _pubsub_listener_item{
	client_PubsubListenerFn lis;
	sint8_t type;//閿熸枻鎷烽敓瑙掗潻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鎭敓鏂ゆ嫹閿熸枻鎷疯叮閿熸枻鎷烽敓鏂ゆ嫹閿熺但ype == 0,閿熸枻鎷烽敓楗侯偓鎷烽敓鏂ゆ嫹閿熸枻鎷烽�忚姱閿熸枻鎷烽敓鏂ゆ嫹閿熼ズ锟�
	struct _pubsub_listener_item *next;
} ps_listener_item_t;

//閿熸枻鎷烽敓瑙ｅ埌閿熸枻鎷锋伅閿熶茎纭锋嫹閿熸枻鎷烽敓鏂ゆ嫹閿熷彨鎲嬫嫹褰遍敓鏂ゆ嫹
//涓�閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰嵏閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
typedef struct _pubsub_listener_map{
	char *topic;
	sint64_t subMsgId;//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓杈冾敮D閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰尮閿熸枻鎷烽敓鏂ゆ嫹搴旈敓鏂ゆ嫹鎭�
	sint32_t subId;//閿熸枻鎷烽敓渚ユ垚鐧告嫹閿熻锛屽嚖鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸埅鐨勬娆¤鎷烽敓鏂ゆ嫹鍞竴閿熸枻鎷疯瘑閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰彇閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
	ps_listener_item_t *listeners;
	struct _pubsub_listener_map *next;
} ps_listener_map;

//閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
static client_send_msg_fn msg_sender = NULL;

static byte_buffer_t *sendBuf = NULL;

static CHRI *handlers = NULL;

static ps_listener_map *ps_listener = NULL;

static sint32_t msgId = 0;

const static char *TOPIC_PREFIX = "/__act/dev/";
//const static char *MSG_TYPE = "__msgType";
static char *DEVICE_ID = "/testdevice001";

#define llSize 5//閿熸枻鎷烽敓鏂ゆ嫹閿熷姭闈╂嫹閿熸枻鎷峰綍鐘舵�侀敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
static sint8_t llCnt = 0;//閿熸枻鎷峰墠閿熷彨璁规嫹閿熷姭闈╂嫹閿熸枻鎷峰綍鐘舵�侀敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
static client_login_listener_fn loginLises[llSize]={NULL};//鐘舵�侀敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹

static char *loginKey = NULL;
static sint32_t actId = 0;
static sint32_t clientId = 0;
static sint32_t loginCode = LOGOUT;//榛橀敓鏂ゆ嫹娌￠敓鍙鎷峰綍
static char *loginMsg  = NULL;

static BOOL connected = true;//閿熸枻鎷峰墠閿熼樁璇ф嫹閿熸枻鎷烽敓鏂ゆ嫹閿熻鏂尅鎷风姸鎬侀敓鏂ゆ嫹true閿熸枻鎷风ず閿熺獤鎾呮嫹閿熸枻鎷烽敓鎺ワ綇鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹浣块敓鏂ゆ嫹, false閿熸枻鎷风ず閿熺獤鎾呮嫹閿熻緝鍖℃嫹

//閿熼ズ杈炬嫹閿熸枻鎷峰簲閿熸枻鎷烽敓鏂ゆ嫹
static client_msg_result_t *wait_for_resps = NULL;

//閿熸枻鎷烽敓鏂ゆ嫹RPC閿熸枻鎷锋伅
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg);
//鍙栭敓鏂ゆ嫹閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
static ICACHE_FLASH_ATTR CHRI* _c_GetMsgHandler(sint8_t type);

//閿熸枻鎷烽敓鏂ゆ嫹閿熷眾姝ラ敓鏂ゆ嫹搴旈敓鏂ゆ嫹鎭�
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg);

//subscribe, unsubscribe閿熸枻鎷峰簲閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg);

static ICACHE_FLASH_ATTR char* _c_getTopic();
//閿熸枻鎷峰綍閿熺即鐧告嫹閿熻锛岃鎷烽敓鏂ゆ嫹閿熷�熷鍏ㄩ敓鏂ゆ嫹閿熸枻鎷锋伅
//static ICACHE_FLASH_ATTR void _c_doSubscribeDeviceMessage();

/**
 * 閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋伅ID鍙栭敓鐭揪鎷烽敓鏂ゆ嫹搴斿疄閿熸枻鎷�
 */
static ICACHE_FLASH_ATTR client_msg_result_t* _c_GetRpcWaitForResponse(sint32_t msgId){
	client_msg_result_t *m = wait_for_resps;
	if(m != NULL) {
		while(m != NULL) {
			if(m->msg_id == msgId) {
				return m;
			}else {
				m = m->next;
			}
		}
	}
	return NULL;
}

/**
 * 閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹搴斿疄閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熷彨浼欐嫹閿熸枻鎷烽敓鏂ゆ嫹鍙栭敓鏂ゆ嫹瀹為敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鐭紮鎷烽攲閿熸枻鎷烽敓缁炵鎷烽敓鏂ゆ嫹閿熸枻鎷疯櫦鍞囬敓鏂ゆ嫹閿熺粸纰夋嫹閿燂拷
 */
static ICACHE_FLASH_ATTR client_msg_result_t* _c_createRpcWaitForResponse(){
	client_msg_result_t *m = wait_for_resps;
	if(m != NULL) {
		while(m != NULL) {
			if(m->in_used) {
				m = m->next;
			} else {
				return m;
			}
		}
	}

	m = os_zalloc(sizeof(struct _c_msg_result));
	if(m == NULL) {
		return NULL;
	}

	m->in_used = true;
	m->next = NULL;
	m->callback = NULL;
	m->msg_id = 0;
	m->cbArg = NULL;

	if(wait_for_resps == NULL) {
		wait_for_resps = m;
	} else {
		m->next = wait_for_resps;
		wait_for_resps = m;
	}
	return m;
}

/**
 * 閿熶粙杩樺疄閿熸枻鎷烽敓鏂ゆ嫹閿熺潾闈╂嫹浣块敓鏂ゆ嫹鐘舵�佷负false
 */
static ICACHE_FLASH_ATTR void _c_rebackRpcWaitRorResponse(client_msg_result_t *m){
	m->in_used = false;
	m->callback = NULL;
	m->msg_id = 0;
	m->cbArg = NULL;
	//閿熼叺鏀剧櫢鎷烽敓鏂ゆ嫹閿熺粸纰夋嫹閿燂拷
}

//閿熸枻鎷峰綍閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿燂拷
static ICACHE_FLASH_ATTR uint8_t _c_loginResult(byte_buffer_t *buf, void *arg){
	bb_print(buf);
	cJSON *json = cJSON_ParseWithLength(buf->data, bb_readable_len(buf));
	if(!json) {
		INFO("_c_loginResult login result is invalid!");
		return MEMORY_OUTOF_RANGE;
	}

	//INFO("_c_loginResult login result JSON: %s ", cJSON_PrintUnformatted(json));

	cJSON *item = cJSON_GetObjectItemCaseSensitive(json,"code");
	loginCode = item == NULL ? 1 : (sint32_t)cJSON_GetNumberValue(item);

	item = cJSON_GetObjectItemCaseSensitive(json,"actId");
	actId = item == NULL ? 1 : (sint32_t)cJSON_GetNumberValue(item);

	item = cJSON_GetObjectItemCaseSensitive(json,"msg");
	loginMsg = item == NULL ? NULL : cJSON_GetStringValue(item);
	if(loginMsg) {
		int len = os_strlen(loginMsg)+1;
		char *cp = os_zalloc(len);
		memcpy(cp,loginMsg,len);
		cp[len] = '\0';
		loginMsg = cp;
	}

	item = cJSON_GetObjectItemCaseSensitive(json,"data");
	loginKey = item == NULL ? NULL : cJSON_GetStringValue(item);
	if(loginKey) {
		int len = os_strlen(loginKey)+1;
		char *cp = os_zalloc(len);
		memcpy(cp,loginKey,len);
		cp[len] = '\0';
		loginKey = cp;
	}
	cJSON_Delete(json);

	INFO("_c_loginResult code:%d, msg:%s, loginKey:%s\n", loginCode, loginMsg, loginKey);

	//if(loginCode == 0)
	//	_c_doSubscribeDeviceMessage(); //閿熸枻鎷烽敓鏂ゆ嫹閿熷�熷閿熸枻鎷锋伅

	if(llCnt > 0) {
		//閫氱煡閿熸枻鎷峰綍鐘舵�侀敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
		for(int i = 0; i< llSize; i++) {
			if(loginLises[i] != NULL)
				loginLises[i](loginCode,loginMsg,loginKey,actId);
		}
	}

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR BOOL client_registLoginListener(client_login_listener_fn fn){
	if(llCnt == llSize) return false;//閿熺獤鎾呮嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
	llCnt++;
	for(int i = 0; i < llSize; i++) {
		if(loginLises[i] == NULL) {
			loginLises[i] = fn;
			return true;
		}
	}
	return false;
}

ICACHE_FLASH_ATTR BOOL client_unregistLoginListener(client_login_listener_fn fn){
	for(int i = 0; i < llSize; i++) {
		if(loginLises[i] == fn) {
			loginLises[i] = NULL;
			llCnt--;
			return true;
		}
	}
	return false;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_login(char *actName, char *pwd){
	INFO("send login request u=%s,p=%s\n",actName,pwd);
	cJSON *arr = cJSON_CreateArray();
	cJSON_AddItemToArray(arr,cJSON_CreateString(actName));
	cJSON_AddItemToArray(arr,cJSON_CreateString(pwd));

	client_invokeRpcWithArrayArgs(-1678356186, arr, _c_loginResult, NULL);

	if(msgId <= 0) {
		return msgId;
	}
	return msgId;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_logout(){

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR BOOL client_distroy() {
	if(sendBuf) {
		bb_release(sendBuf);
		sendBuf = NULL;
	}

	CHRI *chri = handlers;
	while(chri) {
		CHRI *n = chri->next;
		os_free(chri);
		chri = n;
	}
	handlers = NULL;

	ps_listener_map *pi = ps_listener;
	while(pi) {
		ps_listener_map *n = pi->next;
		os_free(pi);
		pi = n;
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL client_registMessageSender(client_send_msg_fn sender){
	if(sender != NULL) {
		msg_sender = sender;
		return true;
	}else {
		return false;
	}
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg){

	if(msg_sender == NULL) {
		INFO("client_sendMessage sender is NULL\n");
		return SOCKET_SENDER_NULL;
	}

	if(loginKey) {
		msg->extraMap = extra_putChars(msg->extraMap, EXTRA_KEY_LOGIN_KEY, loginKey, os_strlen(loginKey));
	}

	if(!msg_encode(msg,sendBuf)) {
		//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹澶遍敓鏂ゆ嫹
		bb_reset(sendBuf);
		INFO("client_sendMessage encode msg fail\n");
		return ENCODE_MSG_FAIL;
	}

	return msg_sender(sendBuf);
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg){

	INFO("client_onMessage got one msg type: %d\n",msg->type);
	CHRI *h = _c_GetMsgHandler(msg->type);
	if(h == NULL) {
		INFO("client_onMessage no handler for type: %d\n",msg->type);
		return HANDLE_MSG_FAIL;
	}

	INFO("client_onMessage to handle msg: %d\n",msg->type);
	//杞敓鏂ゆ嫹閿熸枻鎷穐andler閿熸枻鎷烽敓鏂ゆ嫹
	return h->handler(msg);
}

ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn hdl, sint8_t type){
	CHRI *h = _c_GetMsgHandler(type);
	if(h != NULL) {
		//閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓绐栨拝鎷烽敓鏂ゆ嫹閿熸枻鎷�
		return false;
	}

	h = (CHRI *)os_zalloc(sizeof(struct msg_handler_register_item));
	h->handler = hdl;
	h->type = type;
	h->next = NULL;

	if(handlers == NULL) {
		handlers = h;
	} else {
		//澶撮敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
		h->next = handlers;
		handlers = h;
	}

	return true;

}

/**
 *args 閿熸枻鎷蜂竴閿熸枻鎷穋JSON閿熸枻鎷烽敓閰电殑璇ф嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�,  涓�閿熸枻鎷烽敓鏂ゆ嫹 cJSON_CreateArray閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷风劧閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷穋JSON_AddItemToArray
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithArrayArgs(sint32_t mcode, cJSON *args,
		client_rpc_callback_fn callback, void *cbArgs){

	cJSON *jo = cJSON_CreateObject();
	if(jo == NULL) {
		INFO("client_invokeRpcWithArrayArgs cJSON ins NULL %d\n",mcode);
		return MEMORY_OUTOF_RANGE;
	}

	cJSON_AddItemToObject(jo,"args",args);

	cJSON *params = cJSON_CreateObject();

	cJSON_AddItemToObject(jo,"params",params);
	cJSON_AddItemToObject(params,"NCR",cJSON_CreateString(""));

	char *jsonbody = cJSON_PrintUnformatted(jo);

	INFO("Invoke RPC mcode: %d\n",mcode);
	client_send_msg_result_t rst = client_invokeRpcWithStrArgs(mcode, jsonbody, callback,NULL);

	cJSON_Delete(jo);

	return rst;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithStrArgs(sint32_t mcode, char *payload,
		client_rpc_callback_fn callback, void *cbArgs){

	sint16_t len = os_strlen(payload);
	byte_buffer_t * pl = bb_create(len);
	if(pl == NULL) {
		INFO("client_invokeRpcWithStrArgs fail memory len: %d  \n",len);
		return MEMORY_OUTOF_RANGE;
	}

	if(!bb_put_chars(pl,payload,len)) {
		bb_release(pl);
		INFO("client_invokeRpcWithStrArgs put data fail: %d \n",len);
		return SEND_DATA_ERROR;
	}

	client_send_msg_result_t rst = client_invokeRpc(mcode,pl,callback,NULL);

	INFO("client_invokeRpcWithStrArgs Invoke RPC success msgId: %d  \n",rst);
	bb_release(pl);

	return rst;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpc(sint32_t mcode, byte_buffer_t *payload,
		client_rpc_callback_fn callback, void *cbArgs){
	jm_msg_t *msg = msg_create_rpc_msg(mcode, payload);
	if(msg == NULL) {
		INFO("client_invokeRpc create msg fail\n");
		return MEMORY_OUTOF_RANGE;
	}

	//閿熸枻鎷烽敓鑺ヤ紶閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷穚ayload閿熸枻鎷烽敓閰靛嚖鎷烽敓鑺傚瓨锛岄敓鏂ゆ嫹閿熸枻鎷烽敓璇閿熸枻鎷烽敓閰靛嚖鎷�
	client_msg_result_t *wait = _c_createRpcWaitForResponse();
	if(wait == NULL) {
		INFO("client_invokeRpc create response fail \n");
		msg->payload = NULL;
		msg_release(msg);
		return MEMORY_OUTOF_RANGE;
	}

	if(callback != NULL) {
		wait->msg_id = msg->msgId;
		//wait->msg = msg;
		wait->callback = callback;
		wait->in_used = true;
		wait->cbArg = cbArgs;
	}

	client_send_msg_result_t sendRst = client_sendMessage(msg);//閿熸枻鎷稲PC閿熸枻鎷烽敓鏂ゆ嫹

	if(sendRst != JM_SUCCESS) {//callback == NULL閿熸枻鎷风ず 閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰簲閿熸枻鎷锋伅
		msg->payload = NULL;
		msg_release(msg);//閿熼叺鍑ゆ嫹閿熻妭杈炬嫹
		_c_rebackRpcWaitRorResponse(wait);
		INFO("client_invokeRpc send msg fail \n");
		return sendRst;
	}

	if(callback == NULL) {
		//閿熸枻鎷烽敓鍊熻繑閿熸枻鎷峰�奸敓鏂ゆ嫹RPC
		return msg->msgId;
	}
	sint64_t msgId = msg->msgId;
	msg->payload = NULL;
	msg_release(msg);
	INFO("client_invokeRpc send msg success msgId \n", msgId);
	return msgId;
}

//鍙栭敓鏂ゆ嫹閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
static ICACHE_FLASH_ATTR CHRI* _c_GetMsgHandler(sint8_t type){
	CHRI *h;
	if(handlers != NULL) {
		h = handlers;
		while(h != NULL) {
			if(h->type == type) {
				return h;
			}else {
				h = h->next;
			}
		}
	}
	return NULL;
}

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg){
	INFO("_c_rpcMsgHandle got rpc msgId: %d \n",msg->msgId);
	client_msg_result_t * wait = _c_GetRpcWaitForResponse(msg->msgId);
	if(wait == NULL) {
		INFO("_c_rpcMsgHandle not wait for msgID:% \n",msg->msgId);
		msg_release(msg);
		return MSG_WAIT_NOT_FOUND;
	}

	INFO("_c_rpcMsgHandle notify caller: %d\n",msg->msgId);
	//
	wait->callback(msg->payload, wait->cbArg);
	//msg_release(msg);
	INFO("_c_rpcMsgHandle notify caller finish: %d\n",msg->msgId);

	_c_rebackRpcWaitRorResponse(wait);

	return JM_SUCCESS;
}


/*===============================閿熷眾姝ラ敓鏂ゆ嫹鎭敓鏂ゆ嫹閿熸枻鎷� 閿熸枻鎷峰==========================================*/

static ICACHE_FLASH_ATTR jm_pubsub_item_t* _c_createPubsubItem(){
	/*size_t s = sizeof(struct _c_pubsub_item);
	jm_pubsub_item_t *it = os_zalloc(s);
	memset(it,0,s);
	return it;
	*/
	return cache_get(CACHE_PUBSUB_ITEM,true);
}

static ICACHE_FLASH_ATTR void _c_pubsubItemRelease(jm_pubsub_item_t *it){
	if(!it) return;

	if(it->data) {
		bb_release(it->data);
		it->data = NULL;
	}

	if(it->cxt) {
		extra_release(it->cxt);
		it->cxt = NULL;
	}

	if(it->topic) {
		os_free(it->topic);
		it->topic = NULL;
	}

	//os_free(it);
	cache_back(CACHE_PUBSUB_ITEM,it);

}

static ICACHE_FLASH_ATTR ps_listener_map* _c_getPubsubListenerMap(char *topic){
	ps_listener_map *h;
	if(ps_listener != NULL) {
		h = ps_listener;
		while(h != NULL) {
			if(0 == strcmp(topic, h->topic)) {
				return h;
			}else {
				h = h->next;
			}
		}
	}
	 return NULL;
}

//鍙栭敓鏂ゆ嫹閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
static ICACHE_FLASH_ATTR ps_listener_map* _c_createPubsubListenerMap(char *topic){
	ps_listener_map *h = _c_getPubsubListenerMap(topic);
	if(h) return h;

	//閿熸枻鎷烽敓鏂ゆ嫹涓�閿熸枻鎷烽敓閾扮殑纭锋嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰奖閿熸枻鎷�
	h = (ps_listener_map*)os_zalloc(sizeof(struct _pubsub_listener_map));
	if(h == NULL) return NULL;//閿熻妭杈炬嫹閿熸枻鎷烽敓锟�

	h->listeners = NULL;
	h->next = NULL;
	h->topic = topic;

	if(ps_listener == NULL) {
		ps_listener = h;
	} else {
		h->next = ps_listener;
		ps_listener = h;
	}

	return h;
}

//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰簲閿熸枻鎷烽敓锟� client_subscribe  client_unsubscribe
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg) {
	sint8_t code = extra_getS8(msg->extraMap,EXTRA_KEY_PS_OP_CODE);

	INFO("_c_pubsubOpMsgHandle opCode:%d \n",code);

	if(code == MSG_OP_CODE_SUBSCRIBE) {
		//閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰簲
		sint32_t subId = extra_getS8(msg->extraMap, EXTRA_KEY_EXT0);
		char *topic = extra_getChars(msg->extraMap, EXTRA_KEY_PS_ARGS);

		ps_listener_map *m = _c_getPubsubListenerMap(topic);
		if(m == NULL) {
			INFO("閿熺Ц纰夋嫹閿熸枻鎷锋晥閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰簲topic閿熸枻鎷�%s \n",topic);
			//閿熻妭杈炬嫹閿熸枻鎷烽敓锟�
			return MEMORY_OUTOF_RANGE;
		}
		if(m->subMsgId == msg->msgId){
			m->subId = subId;
		}
	}

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR BOOL _c_addListenerItem(ps_listener_map *m, client_PubsubListenerFn listener, sint8_t type){
	if(m->listeners) {
		//閿熺獤鎾呮嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓瑙ｏ紝閿熶粙鐪嬮敓瑙掑嚖鎷烽敓鏂ゆ嫹閿熼叺顑紮鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
		ps_listener_item_t *item = m->listeners;
		while(item) {
			if(item->lis == listener){
				item->type = type;
				//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓绐栨拝鎷烽敓鏂ゆ嫹閿熻妭锝忔嫹鐩撮敓鎺ュ嚖鎷烽敓鎴垚鐧告嫹
				INFO("ERROR: client_subscribe aready sub topic: %s \n",m->topic);
				return true;
			}
			item = item->next;
		}
	}

	ps_listener_item_t *item = (ps_listener_item_t*)os_zalloc(sizeof(struct _pubsub_listener_item));
	if(item == NULL) {
		INFO("ERROR: client_subscribe create item fail topic: %s, %d \n",m->topic, type);
		return false;//閿熻妭杈炬嫹閿熸枻鎷烽敓缁炑嶆嫹閿燂拷
	}

	INFO("client_subscribe add item success topic: %s, %d \n", m->topic, type);

	item->lis = listener;
	item->next = NULL;
	item->type = type;

	if(m->listeners == NULL) {
		m->listeners = item;
	} else {
		//澶撮敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
		item->next = m->listeners;
		m->listeners = item;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL _c_doSubscribe(ps_listener_map *m){
	jm_msg_t *msg = NULL;

	msg = msg_create_msg(MSG_TYPE_PUBSUB,NULL);
	if(msg == NULL) {
		//閿熻妭杈炬嫹閿熸枻鎷烽敓锟�
		INFO("ERROR: client_subscribe create msg: %s \n",m->topic);
		return false;
	}

	m->subMsgId = msg->msgId;
	m->subId = 0;
	//閿熸枻鎷烽敓渚ヨ鎷烽敓鏂ゆ嫹
	msg->extraMap = extra_putByte(msg->extraMap, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_SUBSCRIBE);
	msg->extraMap = extra_putChars(msg->extraMap, EXTRA_KEY_PS_ARGS, m->topic, os_strlen( m->topic));
	//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓瑗燂紝鍑ゆ嫹閿熸埅鍑ゆ嫹閿熸枻鎷峰敮涓�閿熸枻鎷锋伅ID
	/*client_send_msg_result_t subRes = */
	client_sendMessage(msg);
	/*if(!subRes) {
		return false;
	}*/
	//閿熼叺鍑ゆ嫹閿熻妭杈炬嫹
	msg_release(msg);

	return true;
}

//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type){

	if(listener == NULL) {
		INFO("client_subscribe listener is NULL %s, type:%d\n",topic,type);
		return false;
	}

	if(topic == NULL || os_strlen(topic) == 0) {
		INFO("client_subscribe topic len is NULL %s\n",topic);
		return false;
	}

	BOOL isNewTopic = false;
	ps_listener_map *m = _c_getPubsubListenerMap(topic);
	if(m == NULL) {
		m = _c_createPubsubListenerMap(topic);
		if(!m) {
			//閿熻妭杈炬嫹閿熸枻鎷烽敓锟�
			INFO("ERROR: client_subscribe memory out topic: %s \n",topic);
			return false;
		}
		isNewTopic = true;
	}

	if(!_c_addListenerItem(m,listener,type)) {
		INFO("ERROR: client_subscribe add lis item fail topic: %s \n",topic);
		return false;
	}

	if(isNewTopic) {
		_c_doSubscribe(m);
		INFO("client_subscribe subscribe success\n");
	}else {
		INFO("client_subscribe topic is subscribed %s\n",topic);
	}

	//閿熸枻鎷烽敓渚ユ垚鐧告嫹
	return true;
}

ICACHE_FLASH_ATTR BOOL client_subscribeByType(client_PubsubListenerFn listener, sint8_t type){
	if(loginCode != LSUCCESS) {
		INFO("client_subscribeByType 閿熷壙鐚存嫹鏈敓鏂ゆ嫹褰曢敓鏂ゆ嫹閿熸枻鎷烽敓鏉拌鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹\n");
		return false;
	}

	char *topic = _c_getTopic();
	if(!topic) {
		INFO("client_subscribeByType 閿熻妭杈炬嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷穃n");
		return false;
	}

	//_c_pubsubMsgHandle type = 0,閿熸枻鎷风ず閿熸枻鎷烽敓鏂ゆ嫹鍏ㄩ敓鏂ゆ嫹閿熷�熷閿熸枻鎷锋伅
	if(!client_subscribe(topic, listener, type)) {
		INFO("client_subscribeByType subscribe fail to topic:%s\n",topic);
		os_free(topic);
	}
	return true;
}

//鍙栭敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
ICACHE_FLASH_ATTR BOOL client_unsubscribe(char *topic, client_PubsubListenerFn listener){
	if(listener == NULL) return false;
	if(topic == NULL || os_strlen(topic) == 0) return false;

	ps_listener_map *m = _c_getPubsubListenerMap(topic);
	if(m == NULL || m->listeners == NULL) {
		//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷蜂憨閿熸枻鎷烽敓锟�
		return true;
	}

	ps_listener_item_t *it, *pre, *cit;

	it = pre = NULL;
	cit = m->listeners;

	while(cit) {
		if(cit->lis == listener) {
			it = cit;
			break;//閿熸彮纰夋嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鐩�
		}
		pre = cit;
		cit = cit->next;//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷蜂竴閿熸枻鎷�
	}

	if(it == NULL) return true;

	//閿熸彮纰夋嫹閿熸枻鎷烽敓鏂ゆ嫹鍒犻敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
	if(pre != NULL) {
		pre->next = cit->next;
		cit->next = NULL;
		os_free(it);
		return true;
	} else {
		//鍙敓鏂ゆ嫹涓�閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹,閿熸枻鎷烽敓鏂ゆ嫹閿熸彮浼欐嫹閿燂拷
		m->listeners = NULL;
		os_free(cit);
	}

	jm_msg_t* msg = msg_create_msg(MSG_TYPE_PUBSUB,NULL);
	if(msg == NULL) {
		INFO("ERROR: client_unsubscribe create msg: %s \n",topic);
		return false;
	}

	/**
	 let ps = [{k:Constants.EXTRA_KEY_PS_OP_CODE, v:MSG_OP_CODE_UNSUBSCRIBE, t:Constants.PREFIX_TYPE_BYTE},
	{k:Constants.EXTRA_KEY_PS_ARGS, v:callback.id, t:Constants.PREFIX_TYPE_INT}]
	 */
	//閿熸枻鎷烽敓渚ヨ鎷烽敓鏂ゆ嫹
	msg->extraMap = extra_putByte(msg->extraMap, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_UNSUBSCRIBE);
	msg->extraMap = extra_putInt(msg->extraMap, EXTRA_KEY_PS_ARGS, m->subId);
	//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓瑗燂紝鍑ゆ嫹閿熸埅鍑ゆ嫹閿熸枻鎷峰敮涓�閿熸枻鎷锋伅ID
	client_send_msg_result_t subRes = client_sendMessage(msg);
	/*if(!subRes) {
		return false;
	}*/
	//閿熼叺鍑ゆ嫹閿熻妭杈炬嫹
	msg_release(msg);

	//閿熸枻鎷烽敓渚ユ垚鐧告嫹
	return true;
}

//閿熻鍑ゆ嫹閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
static ICACHE_FLASH_ATTR void _c_dispachPubsubItem(jm_pubsub_item_t *it){
	if(it==NULL) return;

	ps_listener_map *m = _c_getPubsubListenerMap(it->topic);
	if(m == NULL || m->listeners == NULL) {
		//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷蜂憨閿熸枻鎷烽敓锟�
		INFO("_c_dispachPubsubItem Listener is NULL for topic: %s \n",it->topic);
		return;
	}

	BOOL find = false;
	ps_listener_item_t *lis_item = m->listeners;
	while(lis_item) {
		if(lis_item->type == 0 || lis_item->type == it->type) {
			find = true;
			lis_item->lis(it);
		}
		lis_item = lis_item->next;
	}

	if(!find) {
		//閿熺Ц纰夋嫹閿熸枻鎷锋晥閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹娌￠敓鏂ゆ嫹娉ㄩ敓鏂ゆ嫹閿熸枻鎷峰簲閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
		INFO("_c_dispachPubsubItem No listener for topic: %s type:%d \n",it->topic,it->type);
	}
}

static ICACHE_FLASH_ATTR uint8_t _c_setDataFlag(int idx, uint8_t dataFlag) {
	return dataFlag | (1 << idx);
}

/*private void clearDataFlag(int idx) {
	this.dataFlag &= ~(1 << idx);
}*/

static ICACHE_FLASH_ATTR BOOL _c_isDataFlag(int idx, uint8_t dataFlag) {
	return (dataFlag & (1 << idx)) != 0;
}

static ICACHE_FLASH_ATTR uint8_t _c_getDataType(uint8_t flag) {
	return (flag >> FLAG_DATA_TYPE) & 0x03;
}

static ICACHE_FLASH_ATTR void _c_pubsubItemParseBin(jm_msg_t *msg){
	jm_pubsub_item_t *it = _c_createPubsubItem();
	if(!it){
		goto error;
	}

	byte_buffer_t *buf = msg->payload;

	if(!bb_get_u8(buf,&it->dataFlag)) {
		INFO("_c_pubsubItemParseBin fail to read dataFlag\n");
		return;
	}

	if(!bb_get_u8(buf,&it->flag)) {
		INFO("_c_pubsubItemParseBin fail to read flag\n");
		return;
	}

	if(!bb_get_s32(buf,&it->fr)) {
		INFO("_c_pubsubItemParseBin fail to read fr\n");
		return;
	}

	if(_c_isDataFlag(0,it->dataFlag)) {
		if(!bb_get_s64(buf,&it->id)) {
			INFO("_c_pubsubItemParseBin fail to read id\n");
			return;
		}
	}

	if(_c_isDataFlag(1,it->dataFlag)) {
		if(!bb_get_s8(buf,&it->type)) {
			INFO("_c_pubsubItemParseBin fail to read type\n");
			return;
		}
	}

	if(_c_isDataFlag(2,it->dataFlag)) {
		sint8_t flag;
		it->topic  = bb_readString(buf,&flag);
		if(flag != JM_SUCCESS){
			INFO("_c_pubsubItemParseBin fail to read topic\n");
			return;
		}
	}

	if(_c_isDataFlag(3,it->dataFlag)) {
		if(!bb_get_s32(buf,&it->srcClientId)) {
			INFO("_c_pubsubItemParseBin fail to read srcClientId\n");
			return;
		}
	}

	if(_c_isDataFlag(4,it->dataFlag)) {
		if(!bb_get_s32(buf,&it->to)) {
			INFO("_c_pubsubItemParseBin fail to read to\n");
			return;
		}
	}

	if(_c_isDataFlag(5,it->dataFlag)) {
		sint8_t flag;
		it->callback  = bb_readString(buf,&flag);
		if(flag != JM_SUCCESS){
			INFO("_c_pubsubItemParseBin fail to read callback\n");
			return;
		}
	}

	/*if(_c_isDataFlag(6,it->dataFlag)) {
		if(!bb_get_u8(buf,&it->delay)) {
			INFO("_c_pubsubItemParseBin fail to read delay\n");
			return;
		}
	}*/

	if(_c_isDataFlag(6,it->dataFlag)) {
		//extra
		it->cxt = extra_decode(buf);
	}

	if(_c_isDataFlag(7,it->dataFlag)) {
		uint8 dt = _c_getDataType(it->flag);
		if(FLAG_DATA_BIN == dt) {
			//依赖于接口实现数据编码，服务提供方和使用方需要协商好数据编码和解码方式
			/*if(data instanceof ISerializeObject) {
				((ISerializeObject)this).decode(in);
			} else {
				throw new CommonException(data.getClass().getName() +" not implement "+ISerializeObject.class.getName());
			}*/
			uint16_t size = bb_readable_len(buf);
			if(size > 0) {
				byte_buffer_t *b = bb_create(size);
				if(!bb_get_buf(buf,b,size)) {
					INFO("_c_pubsubItemParseBin fail to read bin payload size:%d\n",size);
					return;
				}
				it->data = b;
			}else {
				it->data = NULL;
			}

		}else if(FLAG_DATA_STRING == dt){
			//this.data = in.readUTF();
			sint8_t flag;
			it->data  = bb_readString(buf,&flag);
			if(flag != JM_SUCCESS){
				INFO("_c_pubsubItemParseBin fail to read string data\n");
				return;
			}
		}else if(FLAG_DATA_JSON== dt){
			sint8_t flag;
			char *p  = bb_readString(buf,&flag);
			if(flag != JM_SUCCESS){
				INFO("_c_pubsubItemParseBin fail to read json data\n");
				return;
			}

			cJSON *json = cJSON_ParseWithLength(p,0);
			//cJSON *json = cJSON_Parse(buf->data);
			if(!json) {
				INFO("ERROR: parse json %s\n",p);
				return;
			}

			if(cJSON_GetErrorPtr()) {
				INFO("ERROR: %s\n",cJSON_GetErrorPtr());
				return;
			}

			it->data = json;
			/*String json = in.readUTF();
			this.data = json;*/
			//out.writeUTF(JsonUtils.getIns().toJson(this.data));
		} else {
			//对几种基本数据类型做解码
			it->data = extra_decode(buf);
		}
	}

	_c_dispachPubsubItem(it);

	error:
		_c_pubsubItemRelease(it);
		return;
}

static ICACHE_FLASH_ATTR void _c_pubsubItemParseJson(jm_msg_t *msg){

	INFO("_c_pubsubItemParseJson got one json msg: %d\n",msg->msgId);

	jm_pubsub_item_t *it = _c_createPubsubItem();
	if(!it) goto error;

	byte_buffer_t *buf = msg->payload;

	if(buf== NULL) goto error;

	size_t s = bb_readable_len(buf);
	cJSON *json = cJSON_ParseWithLength(buf->data, s);
	//cJSON *json = cJSON_Parse(buf->data);
	if(!json) goto error;

	if(cJSON_GetErrorPtr()) {
		INFO("ERROR: %s\n",cJSON_GetErrorPtr());
		return;
	}

	cJSON *item = cJSON_GetObjectItem(json,"flag");
	it->flag = item == NULL ? 0: (sint8_t)item->valueint;

	item = cJSON_GetObjectItem(json,"id");
	it->id = item == NULL ? 0: (sint64_t)item->valuedouble;

	item = cJSON_GetObjectItem(json,"srcClientId");
	it->srcClientId = item == NULL ? 0: (sint32_t)item->valueint;

	item = cJSON_GetObjectItem(json,"fr");
	it->fr = item == NULL ? 0: (sint32_t)item->valueint;

	item = cJSON_GetObjectItem(json,"to");
	it->to = item == NULL ? 0: (sint32_t)item->valueint;

	item = cJSON_GetObjectItem(json,"delay");
	it->delay = item == NULL ? 0: (sint8_t)item->valueint;

	item = cJSON_GetObjectItem(json,"type");
	it->type = item == NULL ? 0: (sint8_t)item->valueint;

	item = cJSON_GetObjectItem(json,"data");
	char* data = item->valuestring;// cJSON_PrintUnformatted(item);//閿熸枻鎷峰簲閿熺煫鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
	if(data != NULL) {
		sint16_t sl = os_strlen(data)+1;
		byte_buffer_t *b = bb_create(sl);
		bb_put_chars(b,data,sl-1);
		bb_put_char(b,'\0');
		it->data = b;
	}

	INFO("type: %d, data:%s\n",it->type,data);

	/*
	item = cJSON_GetObjectItemCaseSensitive(json,"cxt");
	it->id = item == NULL ? 0: (sint64_t)cJSON_GetNumberValue(item);
	it->cxt = msg_decodeExtra(buf, elen);
	if(it->cxt == NULL) {
		INFO("ERROR: read extra data fail\r\n");
		goto error;
	}
	*/

	item = cJSON_GetObjectItem(json,"topic");
	it->topic = item == NULL ? "": (char*)cJSON_GetStringValue(item);
	if(it->topic == NULL) {
		INFO("ERROR: read topic fail\r\n");
		goto error;
	}

	_c_dispachPubsubItem(it);
	INFO("_c_pubsubItemParseJson success finish: %d\n",msg->msgId);

	error:
		INFO("_c_pubsubItemParseJson finish free json: %d\n",msg->msgId);
		if(json) cJSON_Delete(json);
		_c_pubsubItemRelease(it);
		return;
}

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg){
	INFO("_c_pubsubMsgHandle got pubsub msg msgID:%d\n",msg->msgId);
	if(msg_getDownProtocol(msg) == PROTOCOL_JSON) {
		_c_pubsubItemParseJson(msg);
	} else {
		_c_pubsubItemParseBin(msg);
	}
	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItemByTopic(char *topic, sint8_t type, char *content/*, msg_extra_data_t *extra*/){
	if(topic == NULL || os_strlen(topic) == 0) {
		INFO("client_publishStrItem topic is NULL\n");
		return INVALID_PS_DATA;
	}

	msg_extra_data_t *msgExtra = extra_putByte(NULL, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC);
	msgExtra = extra_putChars(msgExtra, EXTRA_KEY_PS_ARGS, topic, strlen(topic));
	return client_publishStrItem(topic, type, content, msgExtra);
}

/**
 * 閿熸枻鎷烽敓鏂ゆ嫹閿熷眾姝ラ敓鏂ゆ嫹鎭�
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content, msg_extra_data_t *extra){
	if(topic == NULL || os_strlen(topic) == 0) {
		INFO("client_publishStrItem topic is NULL\n");
		return INVALID_PS_DATA;
	}

	jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true);
	//(jm_pubsub_item_t*)os_zalloc(sizeof(struct _c_pubsub_item));
	//os_memset(item,0,sizeof(struct _c_pubsub_item));

	if(item == NULL) {
		INFO("client_publishStrItem create PS item fail\n");
		return MEMORY_OUTOF_RANGE;
	}

	/*int len = os_strlen(content);
	byte_buffer_t *buf = NULL;
	if(len > 0) {
		buf = bb_create(len);
		bb_put_chars(buf,content,len);
	}*/

	item->flag = 0;
	item->cxt = NULL;
	//item->data = buf;
	item->delay = 0;
	item->fr = actId;
	item->to = 0;
	item->srcClientId = 0;
	item->topic = topic;
	item->id = ++msgId;
	item->type = type;

	item->data = content;
	client_setPSItemDataType(FLAG_DATA_STRING, &item->flag);

	INFO("client_publishStrItem publish item: %s, %u\n",item->topic,item->id);
	client_send_msg_result_t rst = client_publishPubsubItem(item, extra);

	extra_release(item->cxt);

	cache_back(CACHE_PUBSUB_ITEM,item);
	//os_free(item);

	//if(buf)
	//	bb_release(buf);

	return rst;
}

ICACHE_FLASH_ATTR byte_buffer_t *_c_serialPsItem(jm_pubsub_item_t *it){

	byte_buffer_t *buf = bb_create(256);
	if(buf == NULL) {
		INFO("_client_serialItem mom");
		return NULL;
	}

	if(it->id != 0) it->dataFlag=_c_setDataFlag(0,it->dataFlag);
	if(it->type != 0) it->dataFlag=_c_setDataFlag(1,it->dataFlag);
	if(it->topic != NULL) it->dataFlag=_c_setDataFlag(2,it->dataFlag);
	if(it->srcClientId != 0) it->dataFlag=_c_setDataFlag(3,it->dataFlag);
	if(it->to != 0) it->dataFlag=_c_setDataFlag(4,it->dataFlag);
	if(it->callback != NULL) it->dataFlag=_c_setDataFlag(5,it->dataFlag);
	//if(it->delay != 0) it->dataFlag=_c_setDataFlag(6,it->dataFlag);
	if(it->cxt != NULL) it->dataFlag=_c_setDataFlag(6,it->dataFlag);
	if(it->data != NULL) it->dataFlag=_c_setDataFlag(7,it->dataFlag);

	bb_put_u8(buf,it->dataFlag);
	bb_put_u8(buf,it->flag);
	bb_put_s32(buf,it->fr);

	if(it->id != 0) {
		if(!bb_put_s64(buf,it->id)) {
			INFO("_client_serialItem write id error %s\n",it->id);
			return NULL;
		}
	}

	if(it->type != 0) {
		if(!bb_put_u8(buf,it->type)) {
			INFO("_client_serialItem write type error %s\n",it->type);
			return NULL;
		}
	}

	if(it->topic != NULL) {
		if(!bb_writeString(buf,it->topic,os_strlen(it->topic))) {
			INFO("_client_serialItem write topic error %s\n",it->topic);
			return NULL;
		}
	}

	if(it->srcClientId != 0) {
		if(!bb_put_s32(buf,it->srcClientId)) {
			INFO("_client_serialItem write srcClientId error %s\n",it->srcClientId);
			return NULL;
		}
	}

	if(it->to != 0) {
		if(!bb_put_s32(buf,it->to)) {
			INFO("_client_serialItem write to error %s\n",it->to);
			return NULL;
		}
	}

	if(it->callback != NULL) {
		if(!bb_writeString(buf,it->callback,os_strlen(it->callback))) {
			INFO("_client_serialItem write callback error %s\n",it->callback);
			return NULL;
		}
	}

	/*if(it->delay != 0) {
		if(!bb_put_u8(buf,it->delay)) {
			INFO("_client_serialItem write delay error %s\n",it->delay);
			return NULL;
		}
	}*/

	if(it->cxt != NULL) {
		uint16_t len;
		if(!extra_encode(buf, it->cxt, &len, EXTRA_KEY_TYPE_STRING)){
			INFO("_client_serialItem write cxt error\n");
			return NULL;
		}
	}

	if(it->data != NULL) {
		uint8 dt = _c_getDataType(it->flag);
		if(FLAG_DATA_BIN == dt) {
			byte_buffer_t *bb = (byte_buffer_t*)it->data;
			uint16_t len = bb_readable_len(bb);
			bb_put_u16(buf,len);
			if(len > 0) {
				if(!bb_put_buf(buf,bb)) {
					INFO("_client_serialItem write byte_buffer error %d\n",bb_readable_len(bb));
					return NULL;
				}
			}
		}else if(FLAG_DATA_STRING == dt){
			char *bb = (char*)it->data;
			if(!bb_writeString(buf,bb,os_strlen(bb))) {
				INFO("_client_serialItem write String error %s\n",bb);
				return NULL;
			}
		}else if(FLAG_DATA_JSON== dt){
			char *bb = (char*)it->data;
			if(!bb_writeString(buf,bb,os_strlen(bb))) {
				INFO("_client_serialItem write String JSON error %s\n",bb);
				return NULL;
			}
		} else if(FLAG_DATA_EXTRA == dt){
			//对几种基本数据类型做编码
			uint16_t wl;
			if(!extra_encode(it->data,buf,&wl,EXTRA_KEY_TYPE_STRING)){
				INFO("_client_serialItem write extra data error %d\n",wl);
				return NULL;
			}
		}else {
			INFO("_client_serialItem not support data type %d\n",dt);
			return NULL;
		}
	}

	return buf;
}

ICACHE_FLASH_ATTR static byte_buffer_t* _c_psItem2Json(jm_pubsub_item_t *item) {
	cJSON *json = cJSON_CreateObject();

	//cJSON *ji = cJSON_CreateNumber(item->flag);
	cJSON_AddNumberToObject(json,"flag", item->flag);

	//ji = cJSON_CreateNumber(item->id);
	cJSON_AddNumberToObject(json,"id", item->id);

	//ji = cJSON_CreateNumber(item->srcClientId);
	cJSON_AddNumberToObject(json,"srcClientId", item->srcClientId);

	//ji = cJSON_CreateNumber(item->fr);
	cJSON_AddNumberToObject(json,"fr", item->fr);

	//ji = cJSON_CreateNumber(item->to);
	cJSON_AddNumberToObject(json,"to", item->to);

	//ji = cJSON_CreateNumber(item->delay);
	cJSON_AddNumberToObject(json,"delay", item->delay);

	cJSON *ji = cJSON_CreateString(item->topic);
	cJSON_AddItemToObject(json,"topic", ji);

	if(item->data) {
		ji = cJSON_CreateRaw(item->data, bb_readable_len(item->data));
		cJSON_AddItemToObject(json,"data", ji);
		//cJSON_AddItemReferenceToArray(ji,json);
	}

	if(item->cxt) {
		msg_extra_data_t *ic = item->cxt;
		while(ic) {
			ic = ic->next;
		}
	}

	char *itemData = cJSON_PrintUnformatted(json);
	int len = os_strlen(itemData);

	INFO("%s",itemData);

	byte_buffer_t *buf = bb_create(len);
	bb_put_chars(buf,itemData,len);

	cJSON_Delete(json);
	os_free(itemData);

	return buf;
}

ICACHE_FLASH_ATTR void client_initPubsubItem(jm_pubsub_item_t *item,uint8_t dataType){
	item->flag = 0;
	item->cxt = NULL;
	item->delay = 0;
	item->to = 0;
	item->topic = NULL;
	item->type = 0;
	item->fr = actId;
	item->srcClientId = clientId;
	item->id = ++msgId;
	client_setPSItemDataType(dataType, &item->flag);
}

ICACHE_FLASH_ATTR msg_extra_data_t * client_topicForwardExtra(char *topic) {
	msg_extra_data_t *msgExtra = extra_putByte(NULL, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC);
	msgExtra = extra_putChars(msgExtra, EXTRA_KEY_PS_ARGS, topic, strlen(topic));
	return msgExtra;
}

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item, msg_extra_data_t *extra){
	if(item == NULL || item->topic == NULL || os_strlen(item->topic) == 0) {
		INFO("client_publishPubsubItem topic is NULL: %s, %\n",item->topic, item->id);
		return INVALID_PS_DATA;
	}

	INFO("client_publishPubsubItem create item JOSN body\n");

    byte_buffer_t *buf = _c_serialPsItem(item);

	jm_msg_t* msg = msg_create_ps_msg(buf);
	if(msg == NULL) {
		INFO("client_publishPubsubItem msg is NULL : %s, %u\n",item->topic,item->id);
		return MEMORY_OUTOF_RANGE;
	}

	msg_setDownProtocol(msg, PROTOCOL_BIN);
	msg_setUpProtocol(msg, PROTOCOL_BIN);

	msg->extraMap = extra_pullAll(extra, msg->extraMap);

	INFO("client_publishPubsubItem Begin send msg\n");
	client_send_msg_result_t sendRst = client_sendMessage(msg);//閿熸枻鎷稲PC閿熸枻鎷烽敓鏂ゆ嫹
	INFO("client_publishPubsubItem End send result: \n",sendRst);

	//閿熼叺鍑ゆ嫹閿熻妭杈炬嫹
	//extra_release(msg->extraMap);
	msg_release(msg);

	return sendRst;
}

static  ICACHE_FLASH_ATTR char* _c_getTopic() {
	 char actIdStr[32];
	 jm_itoa(actId, actIdStr);

	size_t len = os_strlen(TOPIC_PREFIX) + os_strlen(DEVICE_ID) + os_strlen(actIdStr) + 1;
	char *topic = os_zalloc(len);
	if(topic == NULL) {
		INFO("_c_getTopic memory out \n");
		return NULL;
	}

	INFO("_c_getTopic TOPIC_PREFIX:%s, actId:%d, DEVICE_ID:%s \n",TOPIC_PREFIX,actId, DEVICE_ID);

	memset(topic,0,len);
	strncpy(topic, TOPIC_PREFIX, os_strlen(TOPIC_PREFIX));
	strcat(topic, actIdStr);
	strcat(topic, DEVICE_ID);

	INFO("_c_getTopic topic:%s \n",topic);

	return topic;
}

//閿熼樁璇ф嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鎺ユ柇鍖℃嫹
ICACHE_FLASH_ATTR BOOL client_socketDisconCb() {
	connected = false;
	return true;
}

//閿熼樁璇ф嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鎺ユ垚鐧告嫹 閿熸枻鎷� 閿熸枻鎷烽敓鏂ゆ嫹閿熺即鐧告嫹
ICACHE_FLASH_ATTR BOOL client_socketConedCb(){
	INFO("client_socketConedCb connection ready %s");
	connected = true;
	if(loginCode == LSUCCESS) {
		//閿熺獤鎾呮嫹閿熸枻鎷峰綍閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽湶閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
		ps_listener_map *pi = ps_listener;
		while(pi) {
			INFO("client_socketConedCb resub topic: %s",pi->topic);
			_c_doSubscribe(pi);
			pi = pi->next;
		}
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL client_socketSendTimeoutCb(){
	return true;
}

static uint8_t test_onPubsubItemType1Listener(jm_pubsub_item_t *item) {
	INFO("test_onPubsubItemType1Listener: data= %s, fr= %d, type= %d \n",item->data, item->fr, item->type);
	return JM_SUCCESS;
}

static void test_jmLoginListener(sint32_t code, char *msg, char *loginKey, sint32_t actId) {
	INFO("Listener1 got login result: %s, %s, %d, %d\n",loginKey,msg,code,actId);
	INFO("test_jmLoginListener begin \n");
	if(client_subscribeByType(test_onPubsubItemType1Listener,-128)) {
		INFO("test_onPubsubItemType1Listener\n");
	} else {
		INFO("test_onPubsubItemType1Listener \n");
	}
}

ICACHE_FLASH_ATTR BOOL client_init(char *actName, char *pwd) {

	cache_init(CACHE_MESSAGE, sizeof(struct _jm_msg));
	cache_init(CACHE_MESSAGE_EXTRA, sizeof(struct _msg_extra_data));

	cache_init(CACHE_PUBSUB_ITEM, sizeof(struct _c_pubsub_item));
	cache_init(CACHE_PUBSUB_ITEM_EXTRA, sizeof(struct _msg_extra_data));

	sendBuf = bb_create(1024);

	//娉ㄩ敓鏂ゆ嫹閬ラ敓绱洪敓鏂ゆ嫹鎭敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
	client_registMessageHandler(_c_rpcMsgHandle, MSG_TYPE_RRESP_JRPC);
	//閿熷眾姝ラ敓鏂ゆ嫹鎭敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
	client_registMessageHandler(_c_pubsubMsgHandle, MSG_TYPE_ASYNC_RESP);
	//閿熸枻鎷锋伅閿熸枻鎷烽敓渚ョ尨鎷峰彇閿熸枻鎷烽敓鏂ゆ嫹閿熶茎锝忔嫹閿熸枻鎷锋伅杞敓鏂ゆ嫹 閿熸枻鎷烽敓鏂ゆ嫹鍊奸敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
	client_registMessageHandler(_c_pubsubOpMsgHandle, MSG_TYPE_PUBSUB_RESP);

	//#ifdef MQTT_DEBUG_ON
	//閿熸枻鎷烽敓鏂ゆ嫹閿熻妭璇ф嫹閿熸枻鎷蜂娇閿熸枻鎷�
	INFO("Regist login result listener\n");
	client_registLoginListener(test_jmLoginListener);
	//#endif

	//閿熸枻鎷峰閿熸枻鎷峰綍JMicro閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�,閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹骞冲彴閿熸枻鎷烽敓鏂ゆ嫹
	client_login(actName,pwd);

	return true;
}

/*===============================閿熷眾姝ラ敓鏂ゆ嫹鎭敓鏂ゆ嫹閿熸枻鎷� 閿熸枻鎷烽敓鏂ゆ嫹 ==========================================*/
