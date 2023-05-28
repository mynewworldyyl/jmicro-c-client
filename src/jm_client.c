#include "jm_client.h"

#include "debug.h"
#include "jm_constants.h"
#include "jm_mem.h"
#include "jm_stdcimpl.h"

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

typedef struct msg_handler_register_item{
	sint8_t type;
	client_msg_hander_fn handler;
	struct msg_handler_register_item *next;
} CHRI;

typedef struct _c_msg_result{
	BOOL in_used;
	sint32_t msg_id;
	//jm_msg_t *msg;
	client_rpc_callback_fn callback;
	void *cbArg;
	struct _c_msg_result *next;
} client_msg_result_t;

typedef struct _pubsub_listener_item{
	client_PubsubListenerFn lis;
	sint8_t type;
	struct _pubsub_listener_item *next;
} ps_listener_item_t;

typedef struct _pubsub_listener_map{
	BOOL rm;//是否可处理远程消息，有些命令只能处理本地局域网的请求，不接受通过服务器转发的远程命令
	char *topic;
	sint64_t subMsgId;
	sint32_t subId;
	ps_listener_item_t *listeners;
	struct _pubsub_listener_map *next;
} ps_listener_map;

static client_send_msg_fn msg_sender = NULL;

static client_p2p_msg_sender_fn msg_p2p_sender = NULL;

static byte_buffer_t *sendBuf = NULL;

static CHRI *handlers = NULL;

static ps_listener_map *ps_listener = NULL;

static sint32_t msgId = 0;

static uint16_t jmPort = 0;
static char *jmHost = NULL;
static uint8_t useUdp = NULL;
static uint8_t hostLen = 0;

const static char *TOPIC_PREFIX = "/__act/dev/";

static client_getSystemTime_fn client_getSystemTime;

//const static char *MSG_TYPE = "__msgType";
//static char *DEVICE_ID = "/testdevice001";

#define LLSIZE 10//登录监听器最大数量
static sint8_t llCnt = 0;//当前登录监听器数量
static client_login_listener_fn loginLises[LLSIZE]={NULL};//登录监听器数组

static BOOL inited = 0;

static uint64_t lastSendHearbeatTime=0;
static uint64_t lastActiveTime=0; //最后一次连接JM平台时间，用于发送心跳包

static uint64_t lastLoginTime=0; //最后一次登录时间
static char *loginKey = NULL;
static sint32_t actId = 0;//设备关联账号
static char *deviceId = NULL;//当前登录设备ID
static uint32_t clientId = 0;
static sint32_t loginCode = LOGOUT;//登录结果码，0表示 成功，其他失败
static char *loginMsg  = NULL;

static BOOL connected = true;

static client_msg_result_t *wait_for_resps = NULL;

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg);
static ICACHE_FLASH_ATTR CHRI* _c_GetMsgHandler(sint8_t type);

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg);

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg);
ICACHE_FLASH_ATTR static void _c_subTopicAfterjmLogin();

static ICACHE_FLASH_ATTR char* _c_getTopic();

static timer_check check;

ICACHE_FLASH_ATTR timer_check* client_getCheck() {
	return &check;
}

ICACHE_FLASH_ATTR void client_setSysTimeFn(client_getSystemTime_fn sfn) {
	 client_getSystemTime = sfn;
}

ICACHE_FLASH_ATTR void client_setJmInfo(char *jmh, uint16_t port, uint8_t udp) {
	useUdp = udp;
	jmPort = port;
	jmHost = jmh;
	hostLen = os_strlen(jmh);
}

ICACHE_FLASH_ATTR void _c_hearbeetResult() {
	INFO("_c_hearbeetResult result!\n");//获取心跳结果
}

ICACHE_FLASH_ATTR void _c_sendHearbeet() {
	uint64_t cur = client_getSystemTime();
	if((cur - lastSendHearbeatTime) < JM_HEARBEET_INTERVAL) return;
	if((cur - lastActiveTime) < JM_HEARBEET_INTERVAL) return;//最后一次活动时间小于30秒，不需要发心跳
	INFO("_c_sendHearbeet send hearbeet!\n");
	lastSendHearbeatTime = client_getSystemTime();
	client_invokeRpc(885612323,NULL,_c_hearbeetResult,NULL);
}

ICACHE_FLASH_ATTR BOOL client_main_timer(void *arg) {
	INFO("client_main_timer start\n");
	timer_check* checker = client_getCheck();

	if(checker->jm_checkNet) {
		if(!checker->jm_checkNet()) {
			//网络不可用
			INFO("jm_main_timer jm_checkNet fail!\n");
			return false;
		}
	} else {
		INFO("jm_main_timer jm_checkNet not set!\n");
		return false;
	}

	if(useUdp) {
		//UDP连接JM平台
		if(checker->jm_checkLocalServer) {
			//本地服务失败
			if(!checker->jm_checkLocalServer()) {
				INFO("jm_main_timer jm_checkLocalServer fail!\n");
				return false;
			}

			INFO("jm_main_timer check login begin!\n");
			if(!checker->jm_checkLoginStatus()) {
				//账号登录验证失败
				INFO("jm_main_timer jm_checkLoginStatus fail!\n");
				return false;
			}
		} else {
			INFO("jm_main_timer jm_checkLocalServer is NULL!\n");
			return false;
		}
	} else {
		//TCP连接JM平台
		if(checker->jm_checkConCheck && checker->jm_checkConCheck()) {
			//网络连接成功才做登录校验
			INFO("jm_main_timer check login begin!\n");
			if(!checker->jm_checkLoginStatus()) {
				//账号登录验证失败
				INFO("jm_main_timer jm_checkLoginStatus fail!\n");
			} /*else {
				INFO("jm_main_timer check login success!\n");
			}*/
		}else {
			//JM平台连接服务失败
			INFO("jm_main_timer jm_checkConCheck fail!\n");
		}
	}

	 _c_sendHearbeet();

	INFO("client_main_timer end\n");
	return true;
}

ICACHE_FLASH_ATTR BOOL client_isLogin(){
	return loginCode == LSUCCESS;
}

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
 */
static ICACHE_FLASH_ATTR void _c_rebackRpcWaitRorResponse(client_msg_result_t *m){
	m->in_used = false;
	m->callback = NULL;
	m->msg_id = 0;
	m->cbArg = NULL;
}

static ICACHE_FLASH_ATTR uint8_t _c_loginResult(msg_extra_data_t *resultMap, sint32_t code, char *errMsg, void *arg){

	if(loginMsg) {
		os_free(loginMsg);
		loginMsg = NULL;
	}
	loginCode = 0;

	//msg_extra_data_t *resultMap = extra_decode(buf);
	if(code != 0) {
		loginCode = code;

		if(errMsg) {
			int len = os_strlen(errMsg)+1;
			char *cp = os_zalloc(len);
			memcpy(cp,errMsg,len);
			cp[len] = '\0';
			loginMsg = cp;
		}
		INFO("_c_loginResult error code:%d, msg:%s!",code, loginMsg);
		return loginCode;
	}

	if(resultMap == NULL) {
		loginCode = HANDLE_MSG_FAIL;

		errMsg = "got invalid login NULL result";
		int len = os_strlen(errMsg)+1;
		char *cp = os_zalloc(len);
		memcpy(cp,errMsg,len);
		cp[len] = '\0';

		INFO("_c_loginResult %s!",loginMsg);
		return loginCode;
	}

	//RespJRso.data
	msg_extra_data_t *respDataEx1 = extra_sget(resultMap,"data");
	if(respDataEx1 == NULL) {
		INFO("_c_loginResult Invalid data response!");
		return INVALID_RESP_DATA;
	}

	//Map<String,Object>
	msg_extra_data_t *respDataEx = (msg_extra_data_t *)respDataEx1->value.bytesVal;

	msg_extra_data_t *loginKeyEx = extra_sget(respDataEx,"loginKey");
	if(loginKeyEx) {
		loginKey = loginKeyEx->value.bytesVal;
		int len = os_strlen(loginKey)+1;
		char *cp = os_zalloc(len);
		memcpy(cp,loginKey,len);
		cp[len] = '\0';
		loginKey = cp;
	}

	clientId = extra_sgetS32(respDataEx,"clientId");
	//actId = extra_sgetS32(respDataEx,"actId");

	INFO("_c_loginResult code:%d, msg:%s, loginKey:%s\n", loginCode, loginMsg, loginKey);

	if(llCnt > 0) {
		for(int i = 0; i< LLSIZE; i++) {
			if(loginLises[i] != NULL)
				loginLises[i](loginCode,loginMsg,loginKey,actId);
		}
	}

	/*if(client_isLogin()) {
		_c_subTopicAfterjmLogin();
	}*/

	return JM_SUCCESS;
}

ICACHE_FLASH_ATTR BOOL client_registLoginListener(client_login_listener_fn fn){
	if(llCnt == LLSIZE) return false;
	llCnt++;

	int uidx = -1;
	for(int i = 0; i < LLSIZE; i++) {
		if(loginLises[i] == NULL) {
			if(fn == loginLises[i]) return true;//已经注册同一个方法
			if(uidx == -1 && loginLises[i] == NULL) {
				uidx = i;
			}
		}
	}

	if(uidx == -1) {
		INFO("client_registLoginListener to max count login listener: %d",LLSIZE);
		return false;
	} else {
		loginLises[uidx] = fn;
		return true;
	}

}

ICACHE_FLASH_ATTR BOOL client_unregistLoginListener(client_login_listener_fn fn){
	for(int i = 0; i < LLSIZE; i++) {
		if(loginLises[i] == fn) {
			loginLises[i] = NULL;
			llCnt--;
			return true;
		}
	}
	return false;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_login(sint32_t aid, char *did){

	if(client_isLogin()) return JM_SUCCESS;//已经登录

	uint64_t cur = client_getSystemTime();

	if(lastLoginTime != 0 && (cur - lastLoginTime) < 30000) {
		//最多30秒后重新请求登录
		INFO("client_login interval %d, BUSSY\n", (cur - lastLoginTime));
		return BUSSY;
	}

	lastLoginTime = cur;

	if(aid <= 0) {
		INFO("client_login invalid account id %d\n", aid);
		return SEND_INVALID_ACCOUNT;
	}

	if(os_strlen(did) == 0) {
		INFO("client_login deviceId is NULL\n");
		return SEND_INVALID_DEVICE_ID;
	}

	actId = aid;
	deviceId = did;

	INFO("client_login send login request u=%d, deviceId=%s\n",actId, deviceId);

	msg_extra_data_t *rpcPsList = extra_sput(NULL, "rpcPsList", PREFIX_TYPE_LIST);

/*	msg_extra_data_t *psObj1 = extra_sput(NULL, "psObj1", PREFIX_TYPE_MAP);
	rpcPsList->value->bytesVal = psObj1;*/

	msg_extra_data_t* psObj1 = extra_sputS32(NULL, "actId", actId,-1);
	rpcPsList->value.bytesVal = psObj1;

	extra_sputStr(psObj1, "deviceId", deviceId,-1);

	//client_send_msg_result_t rst = client_invokeRpc(-1678356186, header, _c_loginResult, NULL);
	//deviceLogin
	sint64_t msgId = client_invokeRpc(-1239310325, rpcPsList, _c_loginResult, NULL);

	extra_release(rpcPsList);

	if(msgId < 0) {
		INFO("client_login send login request error %d \n",msgId);
		return msgId;
	}else {
		INFO("client_login send login request end\n");
		return msgId;
	}

}

ICACHE_FLASH_ATTR client_send_msg_result_t _c_client_login(/*sint32_t actId, char *deviceId*/){
	return client_login(actId, deviceId);
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_logout(){
	loginCode = LOGOUT;
	loginKey = NULL;
	actId = 0;
	clientId = 0;
	inited = false;
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

ICACHE_FLASH_ATTR BOOL client_registP2PMessageSender(client_p2p_msg_sender_fn sender){
	if(sender != NULL) {
		msg_p2p_sender = sender;
		return true;
	} else {
		return false;
	}
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg){

	msg_extra_data_t* eh = extra_get(msg->extraMap, EXTRA_KEY_UDP_HOST);
	uint32_t port = extra_getS32(msg->extraMap, EXTRA_KEY_UDP_PORT);

	//eh != NULL 设备与设备间的消息
	//useUdp JM平台指定消息
	if(eh != NULL || useUdp) {
		if(msg_p2p_sender == NULL && msg_isUdp(msg)) {
			INFO("client_sendMessage udp sender is NULL\n");
			return SOCKET_SENDER_NULL;
		}
		msg_setUdp(msg, useUdp);//通过UDP发送到JM平台或设备
	} else {
		if(msg_sender == NULL) {
			INFO("client_sendMessage tcp sender is NULL\n");
			return SOCKET_SENDER_NULL;
		}
		msg_setUdp(msg,false);//TCP发送JM平台
	}

	if(eh == NULL && loginKey != NULL) {
		//平台消息
		msg->extraMap = extra_putChars(msg->extraMap, EXTRA_KEY_LOGIN_KEY, loginKey, os_strlen(loginKey));
	}

	if(!msg_encode(msg, sendBuf)) {
		bb_reset(sendBuf);
		INFO("client_sendMessage encode msg fail\n");
		return ENCODE_MSG_FAIL;
	}

	/******************DUMP SEND DATA BEGIN*********************/
	/*bb_rmark(sendBuf);
	int len = bb_readable_len(sendBuf);
	while(bb_readable_len(sendBuf)> 0) {
		uint8_t d;
		bb_get_u8(sendBuf,&d);
		INFO("%x,",d);
	}
	INFO("\nlen: %d\n",len);
	bb_rmark_reset(sendBuf);*/
	/******************DUMP SEND DATA END*********************/

	if(eh != NULL) {
		INFO("client_sendMessage udp return phone msg\n");
		return msg_p2p_sender(sendBuf, eh->value.bytesVal, port, eh->len);
	} else if(useUdp){
		INFO("client_sendMessage udp send JM server msg\n");
		//没有指定主机IP，说明是主动发送的请求，发往JM平台
		return msg_p2p_sender(sendBuf, jmHost, jmPort, hostLen);
	} else {
		INFO("client_sendMessage tcp send JM server msg\n");
		//TCP发送
		return msg_sender(sendBuf);
	}

}

ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg){

	lastActiveTime = client_getSystemTime(); //更新与服务器交互的最后活动时间
	INFO("client_onMessage got one msg type: %d\n",msg->type);
	CHRI *h = _c_GetMsgHandler(msg->type);
	if(h == NULL) {
		INFO("client_onMessage no handler for type: %d\n",msg->type);
		return HANDLE_MSG_FAIL;
	}

	INFO("client_onMessage to handle msg: %d\n",msg->type);
	return h->handler(msg);
}

ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn hdl, sint8_t type){
	CHRI *h = _c_GetMsgHandler(type);
	if(h != NULL) {
		return false;
	}

	h = (CHRI *)os_zalloc(sizeof(struct msg_handler_register_item));
	h->handler = hdl;
	h->type = type;
	h->next = NULL;

	if(handlers == NULL) {
		handlers = h;
	} else {
		h->next = handlers;
		handlers = h;
	}

	return true;

}

ICACHE_FLASH_ATTR sint64_t client_invokeRpc(sint32_t mcode, msg_extra_data_t *params,
		client_rpc_callback_fn callback, void *cbArgs){

	byte_buffer_t *paramBuf = NULL;
	if(params) {
		paramBuf = bb_create(128);
		INFO("client_invokeRpc encode RPC params begin\n");
		if(!client_encodeExtra(paramBuf, params, params->type)) {
			INFO("client_invokeRpc encode params failure\n");
			return MEMORY_OUTOF_RANGE;
		}
		INFO("client_invokeRpc encode RPC params end\n");
	}

	jm_msg_t *msg = msg_create_rpc_msg(mcode, paramBuf);
	if(msg == NULL) {
		INFO("client_invokeRpc create msg fail\n");
		return MEMORY_OUTOF_RANGE;
	}

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

	client_send_msg_result_t sendRst = client_sendMessage(msg);

	if(sendRst != JM_SUCCESS) {
		msg->payload = NULL;
		msg_release(msg);
		_c_rebackRpcWaitRorResponse(wait);
		INFO("client_invokeRpc send msg fail \n");
		return sendRst;
	}

	if(callback == NULL) {
		return msg->msgId;
	}

	if(paramBuf) {
		bb_release(paramBuf);
	}

	sint64_t msgId = msg->msgId;
	msg->payload = NULL;
	msg_release(msg);
	INFO("client_invokeRpc send msg success msgId %d \n", msgId);
	return msgId;
}

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

static ICACHE_FLASH_ATTR void* _c_parseRpcPayload(jm_msg_t *msg, sint32_t *code, char **errMsg) {

	INFO("_c_parseRpcPayload begin code: %d\n", *code);

	if(msg->type != MSG_TYPE_RRESP_JRPC) {
		*code = 1;
		char *err = os_zalloc(25);//接收者需要释放这个内存
		os_sprintf(err,"Not RPC message type: %d\n", msg->type);
		*errMsg = err;
		INFO(*errMsg);
		return NULL;
	}

	//INFO("_c_parseRpcPayload 1\n");

	sint8_t pro = msg_getDownProtocol(msg);

	//PROTOCOL_BIN和JSON数据由接收者处理
	//if(pro == PROTOCOL_BIN || pro == PROTOCOL_JSON || pro == PROTOCOL_RAW) return msg->payload;
	if(pro != PROTOCOL_EXTRA){
		INFO("_c_parseRpcPayload pro: %d\n",pro);
		 return msg->payload;//平台只处理PROTOCOL_EXTRA解码，其他由接收者处理
	}

	if(msg->payload == NULL) {
		INFO("_c_parseRpcPayload got NULL msg payload!");
		return NULL;
	}

	//INFO("_c_parseRpcPayload 2\n");

	msg_extra_data_t *resultMap = client_decodeExtra(msg->payload);

	if(resultMap->type == PREFIX_TYPE_PROXY
			|| resultMap->type == PREFIX_TYPE_MAP
			|| resultMap->type == PREFIX_TYPE_SET
			|| resultMap->type == PREFIX_TYPE_LIST) {
		msg_extra_data_t *rm  = resultMap->value.bytesVal;
		resultMap->value.bytesVal = NULL;
		resultMap->neddFreeBytes = false;
		extra_release(resultMap);
		resultMap = rm;
	}

	//INFO("_c_parseRpcPayload 3\n");

	if(resultMap == NULL) {
		*code = 2;
		char *err = os_zalloc(25);//接收者需要释放这个内存
		os_sprintf(err,"Null result %d\n",msg->msgId);
		*errMsg = err;
		INFO(*errMsg);
		return NULL;
	}

	//INFO("_c_parseRpcPayload 4\n");

	if(msg_isError(msg)) {
		//RespJRso.data
		INFO("_c_parseRpcPayload Error msg msgId:%d!\n", msg->msgId);
		*code = extra_sgetS32(resultMap,"code");

		msg_extra_data_t *emStr = extra_sget(resultMap,"msg");
		if(emStr) {
			emStr->neddFreeBytes = false;//保证系统不自动释放字符串内存
			*errMsg = emStr->value.bytesVal;
			INFO(*errMsg);
		}
		extra_release(resultMap);
		return NULL;
	}

	INFO("_c_parseRpcPayload 5\n");

	*code = 0;
	*errMsg = NULL;
	//msg_extra_data_t* data = extra_sget(resultMap,"data");

	INFO("_c_parseRpcPayload end code: %d\n", *code);
	return resultMap;

}

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg){
	INFO("_c_rpcMsgHandle got rpc msgId: %d \n",msg->msgId);
	client_msg_result_t * wait = _c_GetRpcWaitForResponse(msg->msgId);
	if(wait == NULL) {
		INFO("_c_rpcMsgHandle not wait for msgId:% \n",msg->msgId);
		msg_release(msg);
		return MSG_WAIT_NOT_FOUND;
	}

	INFO("_c_rpcMsgHandle notify caller msgId: %d\n",msg->msgId);

	sint32_t code = 0;
	char *p = NULL;

	void *rst = _c_parseRpcPayload(msg, &code, &p);
	if(code != 0) {
		INFO("Go error result, code:%d, err: %s\n", code, p);
	}

	wait->callback(rst, code, p, wait->cbArg);

	if(rst!= NULL && msg_getDownProtocol(msg) == PROTOCOL_EXTRA) {
		extra_release(rst);
	}

	if(p) {
		os_free(p);
	}

	//msg_release(msg);
	INFO("_c_rpcMsgHandle notify caller finish: %d\n",msg->msgId);

	_c_rebackRpcWaitRorResponse(wait);

	return JM_SUCCESS;
}


/*=========================================================================*/

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

static ICACHE_FLASH_ATTR ps_listener_map* _c_createPubsubListenerMap(char *topic){
	ps_listener_map *h = _c_getPubsubListenerMap(topic);
	if(h) return h;

	h = (ps_listener_map*)os_zalloc(sizeof(struct _pubsub_listener_map));
	if(h == NULL) return NULL;

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

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg) {
	sint8_t code = extra_getS8(msg->extraMap,EXTRA_KEY_PS_OP_CODE);

	INFO("_c_pubsubOpMsgHandle opCode:%d \n",code);

	if(code == MSG_OP_CODE_SUBSCRIBE) {
		sint32_t subId = extra_getS32(msg->extraMap, EXTRA_KEY_EXT0);
		char *topic = extra_getChars(msg->extraMap, EXTRA_KEY_PS_ARGS);

		ps_listener_map *m = _c_getPubsubListenerMap(topic);
		if(m == NULL) {
			INFO("%s \n",topic);
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
		ps_listener_item_t *item = m->listeners;
		while(item) {
			if(item->lis == listener){
				item->type = type;
				INFO("_c_addListenerItem WARN: client_subscribe already sub topic: %s \n",m->topic);
				return true;
			}
			item = item->next;
		}
	}

	ps_listener_item_t *item = (ps_listener_item_t*)os_zalloc(sizeof(struct _pubsub_listener_item));
	if(item == NULL) {
		INFO("_c_addListenerItem ERROR: client_subscribe create item fail topic: %s, %d \n",m->topic, type);
		return false;
	}

	INFO("_c_addListenerItem add item success topic: %s, %d \n", m->topic, type);

	item->lis = listener;
	item->next = NULL;
	item->type = type;

	if(m->listeners == NULL) {
		m->listeners = item;
	} else {
		item->next = m->listeners;
		m->listeners = item;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL _c_doSubscribeTopic(ps_listener_map *m){
	jm_msg_t *msg = NULL;

	msg = msg_create_msg(MSG_TYPE_PUBSUB,NULL);
	if(msg == NULL) {
		INFO("ERROR: client_subscribe create msg: %s \n",m->topic);
		return false;
	}

	m->subMsgId = msg->msgId;
	m->subId = 0;
	msg->extraMap = extra_putByte(msg->extraMap, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_SUBSCRIBE);
	msg->extraMap = extra_putChars(msg->extraMap, EXTRA_KEY_PS_ARGS, m->topic, os_strlen( m->topic));
	/*client_send_msg_result_t subRes = */

	client_send_msg_result_t resultCode = client_sendMessage(msg);
	if(resultCode != JM_SUCCESS) {
		INFO("ERROR: client_subscribe sub topic fail: %s, result code:%d \n", m->topic, resultCode);
		return false;
	}
	/*if(!subRes) {
		return false;
	}*/
	msg_release(msg);

	return true;
}

ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type, BOOL rm){

	/*if(rm && !client_isLogin()) {
		INFO("client_subscribe need login to regist subcribe topic:%s\n",topic);
		return false;
	}*/

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
			INFO("ERROR: client_subscribe memory out topic: %s \n",topic);
			return false;
		}
		isNewTopic = true;
		m->rm = rm;
	}

	if(!_c_addListenerItem(m, listener, type)) {
		INFO("ERROR: client_subscribe add lis item fail topic: %s \n",topic);
		return false;
	}

	if(rm && isNewTopic && client_isLogin()) {
		_c_doSubscribeTopic(m);
		INFO("client_subscribe subscribe success\n");
	}

	INFO("client_subscribe listener success topic: %s, type:%d\n",topic,type);

	return true;
}

ICACHE_FLASH_ATTR BOOL client_subscribeByType(client_PubsubListenerFn listener, sint8_t type, BOOL rm){
	if(rm && !client_isLogin()) {
		//远程控制必须先登录才能订阅
		INFO("client_subscribeByType not login\n");
		return false;
	}

	char *topic = _c_getTopic();
	if(!topic) {
		INFO("client_subscribeByType \n");
		return false;
	}

	if(!client_subscribe(topic, listener, type, rm)) {
		INFO("client_subscribeByType subscribe fail to topic:%s\n",topic);
		os_free(topic);
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL client_subscribeP2PByType(client_PubsubListenerFn listener, sint8_t type){
	//BOOL isNewTopic = false;
	ps_listener_map *m = _c_getPubsubListenerMap(TOPIC_P2P);
	if(m == NULL) {
		m = _c_createPubsubListenerMap(TOPIC_P2P);
		if(!m) {
			INFO("ERROR: client_subscribe memory out topic: %s \n",TOPIC_P2P);
			return false;
		}
	}

	if(!_c_addListenerItem(m,listener,type)) {
		INFO("ERROR: client_subscribe add lis item fail topic: %s \n",TOPIC_P2P);
		return false;
	}
	return true;
}

ICACHE_FLASH_ATTR BOOL client_unsubscribe(char *topic, client_PubsubListenerFn listener){
	if(listener == NULL) return false;
	if(topic == NULL || os_strlen(topic) == 0) return false;

	ps_listener_map *m = _c_getPubsubListenerMap(topic);
	if(m == NULL || m->listeners == NULL) {
		return true;
	}

	ps_listener_item_t *it, *pre, *cit;

	it = pre = NULL;
	cit = m->listeners;

	while(cit) {
		if(cit->lis == listener) {
			it = cit;
			break;
		}
		pre = cit;
		cit = cit->next;
	}

	if(it == NULL) return true;

	if(pre != NULL) {
		pre->next = cit->next;
		cit->next = NULL;
		os_free(it);
		return true;
	} else {
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
	msg->extraMap = extra_putByte(msg->extraMap, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_UNSUBSCRIBE);
	msg->extraMap = extra_putInt(msg->extraMap, EXTRA_KEY_PS_ARGS, m->subId);
	client_send_msg_result_t subRes = client_sendMessage(msg);
	/*if(!subRes) {
		return false;
	}*/
	msg_release(msg);

	return true;
}

static ICACHE_FLASH_ATTR void _c_dispachPubsubItem(jm_pubsub_item_t *it){
	if(it==NULL) return;

	ps_listener_map *m = _c_getPubsubListenerMap(it->topic);
	if(m == NULL || m->listeners == NULL) {
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
		INFO("_c_dispachPubsubItem No listener for topic: %s type:%d \n",it->topic,it->type);
	}
}

static ICACHE_FLASH_ATTR uint8_t _psitem_setDataFlag(int idx, uint8_t dataFlag) {
	return dataFlag | (1 << idx);
}

/*private void clearDataFlag(int idx) {
	this.dataFlag &= ~(1 << idx);
}*/

static ICACHE_FLASH_ATTR BOOL _psitem_isDataFlag(int idx, uint8_t dataFlag) {
	return (dataFlag & (1 << idx)) != 0;
}

static ICACHE_FLASH_ATTR uint8_t _psitem_getDataType(uint8_t flag) {
	return (flag >> FLAG_DATA_TYPE) & 0x03;
}

static ICACHE_FLASH_ATTR void _psitem_pubsubItemParseBin(jm_msg_t *msg){
	jm_pubsub_item_t *it = _c_createPubsubItem();
	if(!it){
		goto error;
	}

	byte_buffer_t *buf = msg->payload;

	if(!bb_get_u8(buf,&it->dataFlag)) {
		INFO("_psitem_pubsubItemParseBin fail to read dataFlag\n");
		return;
	}

	if(!bb_get_u8(buf,&it->flag)) {
		INFO("_psitem_pubsubItemParseBin fail to read flag\n");
		return;
	}

	if(!bb_get_s32(buf,&it->fr)) {
		INFO("_psitem_pubsubItemParseBin fail to read fr\n");
		return;
	}

	if(_psitem_isDataFlag(0,it->dataFlag)) {
		if(!bb_get_s64(buf,&it->id)) {
			INFO("_psitem_pubsubItemParseBin fail to read id\n");
			return;
		}
	}

	if(_psitem_isDataFlag(1,it->dataFlag)) {
		if(!bb_get_s8(buf,&it->type)) {
			INFO("_psitem_pubsubItemParseBin fail to read type\n");
			return;
		}
	}

	if(_psitem_isDataFlag(2,it->dataFlag)) {
		sint8_t flag;
		it->topic  = bb_readString(buf,&flag);
		if(flag != JM_SUCCESS){
			INFO("_psitem_pubsubItemParseBin fail to read topic\n");
			return;
		}
	}

	if(_psitem_isDataFlag(3,it->dataFlag)) {
		if(!bb_get_s32(buf,&it->srcClientId)) {
			INFO("_psitem_pubsubItemParseBin fail to read srcClientId\n");
			return;
		}
	}

	if(_psitem_isDataFlag(4,it->dataFlag)) {
		if(!bb_get_s32(buf,&it->to)) {
			INFO("_psitem_pubsubItemParseBin fail to read to\n");
			return;
		}
	}

	if(_psitem_isDataFlag(5,it->dataFlag)) {
		sint8_t flag;
		it->callback  = bb_readString(buf,&flag);
		if(flag != JM_SUCCESS){
			INFO("_psitem_pubsubItemParseBin fail to read callback\n");
			return;
		}
	}

	/*if(_psitem_isDataFlag(6,it->dataFlag)) {
		if(!bb_get_u8(buf,&it->delay)) {
			INFO("_psitem_pubsubItemParseBin fail to read delay\n");
			return;
		}
	}*/

	if(_psitem_isDataFlag(6,it->dataFlag)) {
		//extra
		it->cxt = extra_decode(buf);
	}

	if(_psitem_isDataFlag(7,it->dataFlag)) {
		uint8 dt = _psitem_getDataType(it->flag);
		if(FLAG_DATA_BIN == dt) {
			/*if(data instanceof ISerializeObject) {
				((ISerializeObject)this).decode(in);
			} else {
				throw new CommonException(data.getClass().getName() +" not implement "+ISerializeObject.class.getName());
			}*/
			uint16_t size = bb_readable_len(buf);
			if(size > 0) {
				byte_buffer_t *b = bb_create(size);
				if(!bb_get_buf(buf,b,size)) {
					INFO("_psitem_pubsubItemParseBin fail to read bin payload size:%d\n",size);
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
				INFO("_psitem_pubsubItemParseBin fail to read string data\n");
				return;
			}
		}else if(FLAG_DATA_JSON== dt){
			sint8_t flag;
			char *p  = bb_readString(buf,&flag);
			if(flag != JM_SUCCESS){
				INFO("_psitem_pubsubItemParseBin fail to read json data\n");
				return;
			}
			it->data = p;
		} else {
			it->data = extra_decode(buf);
		}
	}

	char *host = extra_getChars(msg->extraMap, EXTRA_KEY_UDP_HOST);
	uint32_t port = extra_getS32(msg->extraMap, EXTRA_KEY_UDP_PORT);
	sint8_t isUdp = extra_getS16(msg->extraMap, EXTRA_KEY_UDP_ACK);

	if(port) {
		msg_extra_data_t *ex = extra_sput(it->cxt,EXTRA_SKEY_UDP_PORT,PREFIX_TYPE_INT);
		ex->value.s32Val = port;
		if(it->cxt == NULL) it->cxt = ex;

	}

	if(host) {
		msg_extra_data_t *ex = extra_sput(it->cxt, EXTRA_SKEY_UDP_HOST, PREFIX_TYPE_STRINGG);
		ex->value.bytesVal = host;
		ex->neddFreeBytes = false;
		ex->len = 4/*os_strlen(host)*/;//IPv4 四个字节长度
		if(it->cxt == NULL) it->cxt = ex;
	}

	msg_extra_data_t *ex = extra_sput(it->cxt,EXTRA_SKEY_UDP_ACK,PREFIX_TYPE_BOOLEAN);
	ex->value.s8Val = isUdp;

	_c_dispachPubsubItem(it);

	error:
		_c_pubsubItemRelease(it);
		return;
}

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg){
	INFO("_c_pubsubMsgHandle got pubsub msg msgID:%d\n",msg->msgId);
	if(msg_getDownProtocol(msg) == PROTOCOL_JSON) {
		//_c_pubsubItemParseJson(msg);
		os_printf("");
	} else {
		_psitem_pubsubItemParseBin(msg);
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

	if(it->id != 0) it->dataFlag=_psitem_setDataFlag(0,it->dataFlag);
	if(it->type != 0) it->dataFlag=_psitem_setDataFlag(1,it->dataFlag);
	if(it->topic != NULL) it->dataFlag=_psitem_setDataFlag(2,it->dataFlag);
	if(it->srcClientId != 0) it->dataFlag=_psitem_setDataFlag(3,it->dataFlag);
	if(it->to != 0) it->dataFlag=_psitem_setDataFlag(4,it->dataFlag);
	if(it->callback != NULL) it->dataFlag=_psitem_setDataFlag(5,it->dataFlag);
	//if(it->delay != 0) it->dataFlag=_psitem_setDataFlag(6,it->dataFlag);
	if(it->cxt != NULL) it->dataFlag=_psitem_setDataFlag(6,it->dataFlag);
	if(it->data != NULL) it->dataFlag=_psitem_setDataFlag(7,it->dataFlag);

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
		if(!extra_encode(it->cxt, buf, &len, EXTRA_KEY_TYPE_STRING)){
			INFO("_client_serialItem write cxt error\n");
			return NULL;
		}
	}

	if(it->data != NULL) {
		uint8 dt = _psitem_getDataType(it->flag);
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
			uint16_t wl;
			if(!extra_encode(it->data, buf, &wl, EXTRA_KEY_TYPE_STRING)){
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

/*
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
		ji = cJSON_CreateRaw(item->data);
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
*/

ICACHE_FLASH_ATTR void client_initPubsubItem(jm_pubsub_item_t *item,uint8_t dataType){
	item->flag = 0;
	item->cxt = NULL;
	item->delay = 0;
	item->to = 0;
	item->topic = NULL;
	item->type = 0;
	item->fr = actId;

	if(item->id <= 0) {
		item->id = ++msgId;
	}

	item->srcClientId = clientId;
	client_setPSItemDataType(dataType, &item->flag);
}

ICACHE_FLASH_ATTR msg_extra_data_t * client_topicForwardExtra(char *topic) {
	msg_extra_data_t *msgExtra = extra_putByte(NULL, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_FORWARD_BY_TOPIC);
	msgExtra = extra_putChars(msgExtra, EXTRA_KEY_PS_ARGS, topic, strlen(topic));
	return msgExtra;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item, msg_extra_data_t *extra){
	if(item == NULL || item->topic == NULL || os_strlen(item->topic) == 0) {
		INFO("client_publishPubsubItem topic is NULL: %s, %\n",item->topic, item->id);
		return INVALID_PS_DATA;
	}

	INFO("client_publishPubsubItem create item body\n");

    byte_buffer_t *buf = _c_serialPsItem(item);

	jm_msg_t* msg = msg_create_ps_msg(buf);
	if(msg == NULL) {
		INFO("client_publishPubsubItem msg is NULL : %s, %u\n",item->topic,item->id);
		return MEMORY_OUTOF_RANGE;
	}

	msg_setDownProtocol(msg, PROTOCOL_BIN);
	msg_setUpProtocol(msg, PROTOCOL_RAW);

	msg->extraMap = extra_pullAll(extra, msg->extraMap);

	if(extra_getBool(msg->extraMap, EXTRA_KEY_UDP_ACK)) {
		msg_setUdp(msg,true);
	}

	INFO("client_publishPubsubItem Begin send msg\n");
	client_send_msg_result_t sendRst = client_sendMessage(msg);
	INFO("client_publishPubsubItem End send result: \n",sendRst);

	//extra_release(msg->extraMap);
	msg_release(msg);

	return sendRst;
}

static ICACHE_FLASH_ATTR BOOL _c_isValidLoginInfo(){
	if(os_strlen(deviceId) == 0) {
		return false;
	}

	if(actId <= 0) {
		return false;
	}
	return true;
}

static ICACHE_FLASH_ATTR char* _c_getTopic() {
	if(!_c_isValidLoginInfo()) {
		return NULL;
	}

	 char actIdStr[32];
	 jm_itoa(actId, actIdStr);

	size_t len = os_strlen(TOPIC_PREFIX) + os_strlen(deviceId) + os_strlen(actIdStr) + 2;
	char *topic = os_zalloc(len);
	if(topic == NULL) {
		INFO("_c_getTopic memory out \n");
		return NULL;
	}

	INFO("_c_getTopic TOPIC_PREFIX:%s, actId:%d, DEVICE_ID:%s \n",TOPIC_PREFIX,actId, deviceId);

	os_memset(topic,0,len);
	os_strncpy(topic, TOPIC_PREFIX, os_strlen(TOPIC_PREFIX));
	jm_strcat(topic, actIdStr);
	jm_strcat(topic, "/");
	jm_strcat(topic, deviceId);

	INFO("_c_getTopic topic:%s \n",topic);

	return topic;
}

ICACHE_FLASH_ATTR BOOL client_socketDisconCb() {
	connected = false;
	//loginCode = LOGOUT;
	return true;
}

ICACHE_FLASH_ATTR BOOL client_socketConedCb(){
	INFO("client_socketConedCb connection ready\n");
	connected = true;
	if(!client_isLogin()) {
		INFO("client_socketConedCb device try to login\n");
		client_login(actId,deviceId);
		return true;
	}
	INFO("client_socketConedCb return\n");
	return true;
}

ICACHE_FLASH_ATTR BOOL client_socketSendTimeoutCb(){
	return true;
}

ICACHE_FLASH_ATTR static void _c_subTopicAfterjmLogin(sint32_t code, char *msg, char *loginKey, sint32_t actId) {
	INFO("_c_subTopicAfterjmLogin got login result: %s, %s, %d, %d\n",loginKey,msg,code,actId);

	if(loginCode == LSUCCESS) {
		ps_listener_map *pi = ps_listener;
		while(pi) {
			if(!pi->rm) {pi = pi->next;continue;} //p2p主题不能经服务器转发
			if(os_strcmp(pi->topic,TOPIC_P2P) == 0) {
				pi = pi->next;
				continue;
			}
			INFO("_c_subTopicAfterjmLogin resub topic: %s\n",pi->topic);
			_c_doSubscribeTopic(pi);
			pi = pi->next;
		}
	}
}

ICACHE_FLASH_ATTR BOOL client_init(sint32_t aid, char *did) {

	if(inited) return false;//已经初始化

	if(actId > 0) {
		actId = aid;
	}

	if(os_strlen(did) > 0) {
		deviceId = did;
	}

	//check.jm_checkLoginStatus = client_login;

	cache_init(CACHE_MESSAGE, sizeof(struct _jm_msg));
	cache_init(CACHE_MESSAGE_EXTRA, sizeof(struct _msg_extra_data));

	cache_init(CACHE_PUBSUB_ITEM, sizeof(struct _c_pubsub_item));
	cache_init(CACHE_PUBSUB_ITEM_EXTRA, sizeof(struct _msg_extra_data));

	sendBuf = bb_create(1024);

	client_registMessageHandler(_c_rpcMsgHandle, MSG_TYPE_RRESP_JRPC);
	client_registMessageHandler(_c_pubsubMsgHandle, MSG_TYPE_ASYNC_RESP);
	client_registMessageHandler(_c_pubsubOpMsgHandle, MSG_TYPE_PUBSUB_RESP);

	//client_subscribeByType(test_onPubsubItemType1Listener,-128,false);

	//#ifdef JMICRO_DEBUG_ON
	//仅用于测试环境
	//INFO("client_init regist login result listener\n");
	client_registLoginListener(_c_subTopicAfterjmLogin);
	//#endif

	/*if(doLogin && actId > 0) {
		client_login(deviceId,actId);
	}else {
		INFO("client_init invalid actId:%d, deviceId: %s\n",actId,deviceId);
	}*/
	inited = 1;
	return true;
}


/***********************************KEY VALUE BEGIN*******************************************/

ICACHE_FLASH_ATTR BOOL _kv_getStrVal(uint8_t type, void *val, char *str) {

	if(val == NULL) {
		INFO("_kv_getStrVal NULL val\n");
		return false;
	}

	if(type== PREFIX_TYPE_BYTE){
		jm_itoa(*((sint8_t*)val), str);
	}else if(type==PREFIX_TYPE_SHORTT) {
		jm_itoa(*((sint16_t*)val), str);
	}else if(type==PREFIX_TYPE_INT) {
		jm_itoa(*((sint32_t*)val), str);
	}else if(type==PREFIX_TYPE_LONG) {
		jm_itoa(*((sint64_t*)val), str);
	}/*else if(type==PREFIX_TYPE_STRINGG) {
		vstr = (char*)val;
	}*/else if(type==PREFIX_TYPE_BOOLEAN) {
		if(jm_strcmp("0",val)) {
			str[0] = '0';
		}else {
			str[0] = '1';
		}
		str[1] = '\0';
	}else if(type==PREFIX_TYPE_CHAR) {
		char *strp = (char*)val;
		str[0] = *strp;
		str[1] = '\0';
	} else {
		INFO("kv_add not support value type: %d!\n",type);
		return false;
	}

	return true;
}


ICACHE_FLASH_ATTR BOOL kv_add(char *name, void *val, char *desc, sint8_t type, client_rpc_callback_fn cb) {
	if(!client_isLogin()) {
		INFO("kv_add device not login!\n");
		return false;
	}

	if(name == NULL || os_strlen(name) == 0) {
		INFO("kv_add kv name is NULL!\n");
		return false;
	}

	msg_extra_data_t *kv = extra_sputStr(NULL,"name",name,-1);
	if(kv == NULL) {
		INFO("kv_add memory out!\n");
		return false;
	}

	char *vstr;
	//全部值传为字符串存储
	char str[9];

	if(type == PREFIX_TYPE_STRINGG) {
		vstr = (char*)val;
	} else if(_kv_getStrVal(type, val, str)) {
		vstr = str;
	} else {
		INFO("kv_add not support value type: %d!\n",type);
		return false;
	}
	extra_sputStr(kv, "value", vstr, -1);
	extra_sputStr(kv, "desc", desc, -1);
	extra_sputS8(kv, "type", type);
	//extra_sputByType(kv, "value", val, type);

	sint32_t msgId = client_invokeRpc(360214249, kv, cb, NULL);

	if(msgId <= 0) {
		INFO("kv_add save kv fail name=%\n",name);
		return false;
	}

	return true;
}

ICACHE_FLASH_ATTR void _kv_getResult(
		void *resultMap, sint32_t code, char *errMsg, void *arg
/*msg_extra_data_t *rr, sint32_t code, char *errMsg, client_rpc_callback_fn cb*/) {

	client_rpc_callback_fn cb = (client_rpc_callback_fn)arg;

	if(code != 0) {
		INFO("_kv_getResult get key fail with code:%d: err: %s!\n",code,errMsg);
		cb(NULL, code, errMsg, NULL);
		return;
	}

	msg_extra_data_t *rr = (msg_extra_data_t *)rr;

	msg_extra_data_t *r = (msg_extra_data_t *)extra_sget(rr, "data")->value.bytesVal;

	sint8_t type = extra_sgetS8(r,"type");
	char* val = extra_sgetChars(r,"val");

	if(val == NULL || os_strlen(val) == 0) {
		cb(NULL, code, errMsg, NULL);
		return;
	}

	msg_extra_data_t *v = extra_sput(NULL,"val", type);
	extra_sputS8(v,"type", type);

	if(type== PREFIX_TYPE_BYTE){
		v->value.s8Val = (sint8_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_SHORTT) {
		v->value.s16Val = (sint16_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_INT) {
		v->value.s32Val = (sint32_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_LONG) {
		v->value.s64Val = (sint64_t)jm_atoi(val);
	}else if(type==PREFIX_TYPE_STRINGG) {
		v->value.bytesVal = (char*)val;
	}else if(type==PREFIX_TYPE_BOOLEAN) {
		v->value.boolVal = os_strcmp("1",val);
	}else if(type==PREFIX_TYPE_CHAR) {
		v->value.charVal = val[0];
	} else {
		INFO("_kv_getResult not support value type: %d!\n",type);
		if(v) extra_release(v);
		v = NULL;
	}

	cb(v, code, errMsg, NULL);
	if(v) extra_release(v);
}

ICACHE_FLASH_ATTR BOOL kv_get(char *name, client_rpc_callback_fn cb) {
	if(!client_isLogin()) {
		INFO("kv_get device not login!\n");
		return false;
	}

	if(name == NULL || os_strlen(name) == 0) {
		INFO("kv_get kv name is NULL\n");
		return false;
	}

	msg_extra_data_t *kv = extra_sputStr(NULL,"name",name,-1);
	if(kv == NULL) {
		INFO("kv_get memory out\n");
		return false;
	}

	sint32_t msgId = client_invokeRpc(-1246466186, kv, _kv_getResult, cb);

	if(msgId <= 0) {
		INFO("kv_get get kv fail name=%\n",name);
		return false;
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL kv_update(char *name, char *desc, void *val, sint8_t type, client_rpc_callback_fn cb) {

	if(!client_isLogin()) {
		INFO("kv_update device not login!\n");
		return false;
	}

	if(name == NULL || os_strlen(name) == 0) {
		INFO("kv_update kv name is NULL!\n");
		return false;
	}

	msg_extra_data_t *kv = extra_sputStr(NULL,"name",name,-1);
	if(kv == NULL) {
		INFO("kv_update memory out!\n");
		return false;
	}

	char *vstr;
	//全部值传为字符串存储
	char str[9];

	if(type == PREFIX_TYPE_STRINGG) {
		vstr = (char*)val;
	} else if(_kv_getStrVal(type, val, str)) {
		vstr = str;
	} else {
		INFO("kv_update not support value type: %d!\n",type);
		return false;
	}

	extra_sputStr(kv, "value", vstr, -1);
	extra_sputStr(kv, "desc", desc, -1);

	//extra_sputByType(kv, "value", val, type);
	//extra_sputStr(kv,"deviceId",deviceId,-1);
	//extra_sputS32(kv,"srcActId",actId);

	sint32_t msgId = client_invokeRpc(247475691, kv, cb, NULL);

	if(msgId <= 0) {
		INFO("kv_add save kv fail name=%\n",name);
		return false;
	}

	return true;
}

ICACHE_FLASH_ATTR BOOL kv_delete(char *name, client_rpc_callback_fn cb) {
	if(!client_isLogin()) {
		INFO("kv_delete device not login!\n");
		return false;
	}

	if(name == NULL || os_strlen(name) == 0) {
		INFO("kv_delete kv name is NULL\n");
		return false;
	}

	msg_extra_data_t *kv = extra_sputStr(NULL,"name",name,-1);
	if(kv == NULL) {
		INFO("kv_delete memory out\n");
		return false;
	}

	//extra_sputStr(kv,"deviceId",deviceId,-1);
	//extra_sputS32(kv,"srcActId",actId);

	sint32_t msgId = client_invokeRpc(1604442957, kv, cb, NULL);

	if(msgId <= 0) {
		INFO("kv_delete get kv fail name=%\n",name);
		return false;
	}

	return true;
}

/***********************************KEY VALUE END*********************************************/


/***********************************解码Extra数据开始*********************************************/
ICACHE_FLASH_ATTR static uint16_t _c_encodeWriteLen(byte_buffer_t *b, msg_extra_data_t *extras);

//decode
ICACHE_FLASH_ATTR static BOOL _c_decodeMap(byte_buffer_t *b, msg_extra_data_t *m) {

	m->len = 0;
	uint16_t eleLen;//元素的个数,最多可以存放255个元素
	if(!bb_get_u16(b, &eleLen)) {
		INFO("decodeMap: read extra data length fail\r\n");
		return false;
	}

	INFO("decodeMap: eleNum: %d\n",eleLen);

	m->len = eleLen;
	m->neddFreeBytes = true;

	if(eleLen == 0) return true;//无元素

	msg_extra_data_t  *tail = NULL;

	while(eleLen-- > 0) {
		sint8_t flag;
		char *p =  bb_readString(b,&flag);
		if(flag != JM_SUCCESS){
			INFO("_c_pubsubItemParseBin fail to read keyType\n");
			return false;
		}

		INFO("decodeMap: idx:%d, key: %s\n",eleLen,p);

		msg_extra_data_t *v = client_decodeExtra(b);
		if(v == NULL) {
			continue;
		}

		v->strKey = p;

		if(tail == NULL) {
			m->value.bytesVal = v;
		} else {
			tail->next = v;
		}
		tail = v;
	}

	return true;
}

ICACHE_FLASH_ATTR static BOOL _c_decodeColl(byte_buffer_t *b, msg_extra_data_t *s){
	uint16_t size;//元素的个数,最多可以存放255个元素
	if(!bb_get_u16(b, &size)) {
		INFO("ERROR: read extra data length fail\r\n");
		return false;
	}

	s->len = size;
	s->neddFreeBytes = true;

	if(size == 0) return true;//无元素

	msg_extra_data_t *tail = NULL;
	while(size-- > 0) {
		msg_extra_data_t *v = client_decodeExtra(b);
		if(v != NULL) {
			if(tail == NULL) {
				s->value.bytesVal = v;
			} else {
				tail->next = v;
			}
			tail = v;
		} else {
			INFO("_c_decodeColl list NULL element idx: %d\n",size);
		}
	}
	return true;
}

ICACHE_FLASH_ATTR msg_extra_data_t* client_decodeExtra(byte_buffer_t *b) {

	sint8_t type;
	if(!bb_get_s8(b,&type)){
		INFO("client_decodeExtra get type %d error\n",type);
		goto error;
	}

	msg_extra_data_t *rst = extra_create();
	//void *val = NULL;
	uint16_t len = 0;

	if(type == PREFIX_TYPE_NULL) {
		rst->value.bytesVal = NULL;
		INFO("client_decodeExtra NULL\n",type);
	}else if(PREFIX_TYPE_BYTEBUFFER == type){
		INFO("client_decodeExtra bytebuffer type %d\n",type);
		if(!bb_get_u16(b,&len)) {
			goto error;
		}

		rst->len = len;

		if(len == 0) {
			rst->value.bytesVal = NULL;
		} else {
			uint8_t *arr = (uint8_t*)os_zalloc(len);
			if(!bb_get_bytes(b,arr,len)) {
				if(arr) os_free(arr);
				goto error;
			}
			rst->neddFreeBytes = true;
			rst->value.bytesVal = arr;
		}
	}else if(type == PREFIX_TYPE_INT){
		sint32_t v;
		if(!bb_get_s32(b,&v)) {
			goto error;
		} else {
			rst->value.s32Val = v;
		}
		INFO("client_decodeExtra INT %d\n",rst->value.s32Val);
	}else if(PREFIX_TYPE_BYTE == type){
		sint8_t v;
		if(!bb_get_s8(b,&v)) {
			goto error;
		} else {
			rst->value.s8Val = v;
		}
		INFO("client_decodeExtra s8 %d\n",rst->value.s8Val);
	}else if(PREFIX_TYPE_SHORTT == type){
		sint16_t v;
		if(!bb_get_s16(b,&v)) {
			goto error;
		} else {
			rst->value.s16Val = v;
		}
		INFO("client_decodeExtra s16 %d\n",rst->value.s16Val);
	}else if(PREFIX_TYPE_LONG == type){
		sint64_t v;
		if(!bb_get_s64(b,&v)) {
			goto error;
		} else {
			rst->value.s64Val = v;
		}
		INFO("client_decodeExtra s64 %d\n",rst->value.s64Val);
	}else if(PREFIX_TYPE_FLOAT == type){
		INFO("client_decodeExtra unsupport FLOAT type: %d\n",type);
	}else if(PREFIX_TYPE_DOUBLE == type){
		INFO("client_decodeExtra unsupport DOUBLE type: %d\n",type);
	}else if(PREFIX_TYPE_BOOLEAN == type){
		BOOL v;
		if(!bb_get_bool(b,&v)) {
			goto error;
		} else {
			rst->value.boolVal = v;
		}
		INFO("client_decodeExtra boolean type: %d\n",v);
	}else if(PREFIX_TYPE_CHAR == type){
		char v;
		if(!bb_get_char(b,&v)) {
			goto error;
		} else {
			rst->value.charVal = v;
		}
		INFO("client_decodeExtra char type: %d\n",v);
	}else if(PREFIX_TYPE_STRINGG == type){
		INFO("client_decodeExtra string type: %d\n",type);
		sint8_t slen;
		if(!bb_get_s8(b,&slen)) {
			goto error;
		}
		len = slen;

		rst->neddFreeBytes = true;

		if(len == -1) {
			rst->value.bytesVal = NULL;
		}else if(len == 0) {
			char* vptr = (char*)os_zalloc(sizeof(char));
			rst->value.bytesVal = vptr;
		}else {
			sint32_t ilen = len;
			if(len == 127) {
				sint16_t slen;
				if(!bb_get_s16(b,&slen)) {
					goto error;
				}
				ilen = slen;

				if(slen == 32767) {
					if(!bb_get_s32(b,&ilen)) {
						goto error;
					}
				}
			}

			rst->len = ilen;

			uint8_t* vptr = (uint8_t*)os_zalloc(ilen + 1);
			if(vptr == NULL) {
				goto error;
			}

			if(!bb_get_bytes(b,vptr,(uint16_t)ilen)) {
				goto error;
			}

			vptr[ilen] = '\0';
			rst->value.bytesVal = vptr;
			INFO("client_decodeExtra string val: %s\n",vptr);
		}
	}else if(PREFIX_TYPE_MAP == type || PREFIX_TYPE_PROXY == type){
		INFO("client_decodeExtra map type: %d\n",type);
		if(!_c_decodeMap(b,rst)) {
			INFO("client_decodeExtra: decode map error %d\n");
			goto error;
		}
		rst->neddFreeBytes = true;//记录要释放内存
	}else if(PREFIX_TYPE_SET == type || PREFIX_TYPE_LIST == type){
		INFO("client_decodeExtra list type: %d\n",type);
		if(!_c_decodeColl(b,rst)) {
			INFO("client_decodeExtra: decode map error %d\n");
			goto error;
		}
		rst->neddFreeBytes = true;//记录要释放内存
	}else {
		INFO("client_decodeExtra unsupport type: %d\n",type);
		goto error;
	}

	rst->type = type;
	rst->next = NULL;

	return rst;

	error:
		INFO("client_decodeExtra error: %d\n",type);
		if(rst) os_free(rst);
		return NULL;

}

ICACHE_FLASH_ATTR BOOL _c_encodeExtra(byte_buffer_t *b, msg_extra_data_t *extras);

ICACHE_FLASH_ATTR static uint16_t _c_encodeWriteLen(byte_buffer_t *b, msg_extra_data_t *extras)  {
	INFO("_c_encodeWriteLen count len, extras addr: %u\n",extras);
	int eleCnt = 0;
	msg_extra_data_t *te = extras;
	while(te != NULL) {
		eleCnt++;
		te = te->next;
		INFO("_c_encodeWriteLen count len, te: %u\n",te);
	}

	INFO("_c_encodeWriteLen write len\n");
	if(!bb_put_u16(b,eleCnt)){
		return 0;
	}
	return eleCnt;
}

//Endoce
ICACHE_FLASH_ATTR static BOOL _c_encodeColl(byte_buffer_t *b, msg_extra_data_t *extras)  {

	INFO("_c_encodeColl begin\n");

	int eleCnt = _c_encodeWriteLen(b, extras);
	if(eleCnt == 0) {
		INFO("_c_encodeColl element len is zero\n");
		return true;//锟睫革拷锟斤拷锟斤拷锟斤拷
	}

	INFO("_c_encodeColl encode loop begin\n");

	while(extras != NULL) {
		if(!client_encodeExtra(b, extras, extras->type)) {
			INFO("_c_encodeColl fail\n");
			return false;
		}
		extras = extras->next;
	}

	INFO("_c_encodeColl end write\n");
	return true;

}

ICACHE_FLASH_ATTR static BOOL _c_encodeExtraMap(byte_buffer_t *b, msg_extra_data_t *extras){

	INFO("_c_encodeExtraMap begin\n");

	//INFO("extra_encode count len, extras: %u\n",extras);
	int eleCnt = _c_encodeWriteLen(b, extras);
	if(eleCnt == 0) {
		INFO("_c_encodeExtraMap element len is zero\n");
		return true;//锟睫革拷锟斤拷锟斤拷锟斤拷
	}

	INFO("_c_encodeExtraMap encode loop begin\n");

	while(extras != NULL) {
		INFO("_c_encodeExtraMap skey: %s \n",extras->strKey);
		if(!bb_writeString(b, extras->strKey, os_strlen(extras->strKey))) {
			INFO("extra_encode fail to write string key: %s", extras->strKey);
			return false;
		}

		if(!client_encodeExtra(b,extras,extras->type)) {
			INFO("_c_encodeExtraMap encode value\n");
			return false;
		}
		extras = extras->next;
	}

	INFO("_c_encodeExtraMap end write\n");
	return true;
}

ICACHE_FLASH_ATTR BOOL client_encodeExtra(byte_buffer_t *b, msg_extra_data_t *extras, sint8_t type) {

	//byte_buffer_t *b = bb_create(512);
	//sint8_t type = extras->type;

	if((PREFIX_TYPE_STRINGG == type
			|| PREFIX_TYPE_BYTEBUFFER == type
			|| PREFIX_TYPE_MAP ==type
			|| PREFIX_TYPE_LIST == type
			|| PREFIX_TYPE_SET == type
	) && extras->value.bytesVal == NULL) {
		INFO("_c_encodeOneExtra write PREFIX_TYPE_NULL: %d\n", PREFIX_TYPE_NULL);
		if(!bb_put_s8(b, PREFIX_TYPE_NULL)) {
			INFO("_c_encodeOneExtra fail to write PREFIX_TYPE_NULL: %d\n", PREFIX_TYPE_NULL);
			return false;
		}
		return true;
	}

	if(!bb_put_s8(b,type)) {
		INFO("_c_encodeOneExtra fail to write type: %d\n", type);
		return false;
	}

	if (PREFIX_TYPE_BYTEBUFFER == type) {
		INFO("client_encodeExtra List val\n");
		uint8_t *ptr = extras->value.bytesVal;
		if(extras->len <= 0) {
			bb_put_u16(b, 0);
			return false ;
		}

		if (!bb_put_u16(b, extras->len)) {
			return false ;
		}
		if (!bb_put_bytes(b, ptr, extras->len)) {
			return false;
		}
		return true;
	} else if(type == PREFIX_TYPE_INT) {
		INFO("client_encodeExtra Integer val: %d\n", extras->value.s32Val);
		if (!bb_put_s32(b, extras->value.s32Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_BYTE == type) {
		INFO("client_encodeExtra Byte val: %d\n", extras->value.s8Val);
		if (!bb_put_s8(b, extras->value.s8Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_SHORTT == type) {
		INFO("client_encodeExtra Short val: %d\n", extras->value.s16Val);
		if (!bb_put_s16(b, extras->value.s16Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_LONG == type) {
		INFO("client_encodeExtra Long val: %d\n", extras->value.s64Val);
		if (!bb_put_s64(b, extras->value.s64Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_FLOAT == type) {
		INFO("client_encodeExtra Float val: %d\n", extras->value.s64Val);

	} else if (PREFIX_TYPE_DOUBLE == type) {
		INFO("client_encodeExtra Double val: %d\n", extras->value.s64Val);

	} else if (PREFIX_TYPE_BOOLEAN == type) {
		INFO("client_encodeExtra Boolean val: %d\n", extras->value.boolVal);
		if (!bb_put_bool(b, extras->value.boolVal)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_CHAR == type) {
		INFO("client_encodeExtra Char val: %d\n", extras->value.charVal);
		if (!bb_put_char(b, extras->value.charVal)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_STRINGG == type) {
		INFO("client_encodeExtra string len:%d, value: %s\n", extras->len, extras->value.bytesVal);
		sint8_t len = extras->len;
		if(len < MAX_BYTE_VALUE) {
			bb_put_s8(b,len);
		}else if(len < MAX_SHORT_VALUE) {
			//0X7F=01111111=127 byte
			//0X0100=00000001 00000000=128 short
			bb_put_s8(b,MAX_BYTE_VALUE);
			bb_put_s16(b,len);
		}else if(len < MAX_INT_VALUE) {
			bb_put_s8(b,MAX_BYTE_VALUE);
			bb_put_s16(b,MAX_SHORT_VALUE);
			bb_put_s32(b,len);
		} else {
			INFO("client_encodeExtra write len fail by exceed %d\n", len);
			return false;
		}

		if(len > 0) {
			if(!bb_put_chars(b,extras->value.bytesVal,len)) {
				INFO("client_encodeExtra write value fail\n");
				return false;
			}
			//INFO("extra_encode write string value success %s\n",extras->value.bytesVal);
		}else {
			INFO("client_encodeExtra len less than zero %d\n",len);
		}
	}else if(PREFIX_TYPE_MAP == type || PREFIX_TYPE_PROXY == type) {
		INFO("client_encodeExtra map value type: %d\n",type);
		//请求参数是Map
		/*if(!_c_encodeExtraMap(b,extras->value.bytesVal)) {
			//编码Map失败
			return false;
		}*/
		if(!_c_encodeExtraMap(b, extras->value.bytesVal)) {
			//编码Map失败
			return false;
		}
	}else if(PREFIX_TYPE_SET == type || PREFIX_TYPE_LIST == type) {
		INFO("client_encodeExtra collection value type: %d\n",type);
		//请求参数是Map
		if(!_c_encodeColl(b, extras->value.bytesVal)) {
			//编码Map失败
			return false;
		}
	}
	return true;
}

/***********************************解码Extra数据结束*********************************************/

