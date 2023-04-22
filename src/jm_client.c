#include "jm_client.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "c_types.h"
#include "debug.h"
#include "jm_constants.h"
#include "testcase/test.h"

#define MSG_OP_CODE_SUBSCRIBE 1//������Ϣ
#define MSG_OP_CODE_UNSUBSCRIBE 2//ȡ��������Ϣ
#define MSG_OP_CODE_FORWARD 3//ת����Ϣ

//��Ϣ��Message��������Ӱ��
typedef struct msg_handler_register_item{
	sint8_t type;
	client_msg_hander_fn handler;
	struct msg_handler_register_item *next;
} CHRI;

//RPC���������
typedef struct _c_msg_result{
	BOOL in_used;
	sint32_t msg_id;
	//jm_msg_t *msg;
	client_rpc_callback_fn callback;
	void *cbArg;
	struct _c_msg_result *next;//��һ��Ԫ��
	//struct _c_msg_result *pre;//ǰһ��Ԫ��
} client_msg_result_t;

//��Ϣ��PSData�����ļ�����
typedef struct _pubsub_listener_item{
	client_PubsubListenerFn lis;
	sint8_t type;//���Ǹ�������Ϣ����Ȥ�����type == 0,���ȫ�����͸о�����Ȥ
	sint64_t subMsgId;//�����������ϢID������ƥ����Ӧ��Ϣ
	sint32_t subId;//���ĳɹ��󣬷��������صĴ˴ζ���Ψһ��ʶ������ȡ����������
	struct _pubsub_listener_item *next;
} ps_listener_item_t;

//���⵽��Ϣ�ļ������б�Ӱ��
//һ����������ж������������
typedef struct _pubsub_listener_map{
	char *topic;
	ps_listener_item_t *listeners;
	struct _pubsub_listener_map *next;
} ps_listener_map;

//��Ϣ������
static client_send_msg_fn msg_sender = NULL;

static byte_buffer_t *sendBuf = NULL;

static CHRI *handlers = NULL;

static ps_listener_map *ps_listener = NULL;

static sint32_t msgId = 0;

const static char *TOPIC_PREFIX = "/__act/dev/";
//const static char *MSG_TYPE = "__msgType";
static char *DEVICE_ID = "/testdevice001";

static sint8_t llCnt = 0;//��ǰ�ж��ٸ���¼״̬������
static client_login_listener_fn loginLises[5]={NULL};//״̬����������

static char *loginKey = NULL;
static sint32_t actId = 0;
static sint32_t loginCode = LOGOUT;//Ĭ��û�е�¼
static char *loginMsg  = NULL;

//�ȴ���Ӧ����
static client_msg_result_t *wait_for_resps = NULL;

//����RPC��Ϣ
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_rpcMsgHandle(jm_msg_t *msg);
//ȡ����Ϣ������
static ICACHE_FLASH_ATTR CHRI* _c_GetMsgHandler(sint8_t type);

//�����첽��Ӧ��Ϣ
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg);

//subscribe, unsubscribe��Ӧ��Ϣ������
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg);

//��¼�ɹ��󣬶����豸ȫ����Ϣ
static ICACHE_FLASH_ATTR void _c_doSubscribeDeviceMessage();

/**
 * ������ϢIDȡ�ô���Ӧʵ��
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
 * ��������Ӧʵ���������л�����ȡ��ʵ�������������û�п���ʵ�����򴴽���ʵ��
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
 * �黹ʵ�����޸�ʹ��״̬Ϊfalse
 */
static ICACHE_FLASH_ATTR void _c_rebackRpcWaitRorResponse(client_msg_result_t *m){
	m->in_used = false;
	m->callback = NULL;
	m->msg_id = 0;
	m->cbArg = NULL;
	//�ͷŹ����ʵ��
}

//��¼���������
static ICACHE_FLASH_ATTR uint8_t _c_loginResult(byte_buffer_t *buf, void *arg){
	cJSON *json = cJSON_ParseWithLength(buf->data, bb_readable_len(buf));
	if(!json) {
		INFO("�˺ŵ�¼����������");
		return MEMORY_OUTOF_RANGE;
	}

	cJSON *item = cJSON_GetObjectItemCaseSensitive(json,"code");
	loginCode = item == NULL ? 1 : (sint32_t)cJSON_GetNumberValue(item);

	item = cJSON_GetObjectItemCaseSensitive(json,"actId");
	actId = item == NULL ? 1 : (sint32_t)cJSON_GetNumberValue(item);

	item = cJSON_GetObjectItemCaseSensitive(json,"msg");
	loginMsg = item == NULL ? NULL : cJSON_GetStringValue(item);
	if(loginMsg) {
		int len = strlen(loginMsg)+1;
		char *cp = os_zalloc(len);
		memcpy(cp,loginMsg,len);
		cp[len] = '\0';
		loginMsg = cp;
	}

	item = cJSON_GetObjectItemCaseSensitive(json,"data");
	loginKey = item == NULL ? NULL : cJSON_GetStringValue(item);
	if(loginKey) {
		int len = strlen(loginKey)+1;
		char *cp = os_zalloc(len);
		memcpy(cp,loginKey,len);
		cp[len] = '\0';
		loginKey = cp;
	}
	cJSON_Delete(json);

	INFO("�˺ŵ�¼����������code:%s,msg:%s",loginCode,loginMsg);

	if(llCnt > 0) {
		//֪ͨ��¼״̬������
		for(int i = 0; i< llCnt; i++) {
			loginLises[i](loginCode,loginMsg,loginKey);
		}
	}

	if(loginCode == 0)
		_c_doSubscribeDeviceMessage(); //�����豸��Ϣ

	return SUCCESS;
}

ICACHE_FLASH_ATTR BOOL client_init(char *actName, char *pwd) {

	sendBuf = bb_allocate(1024);

	//ע��ң�C��Ϣ������
	client_registMessageHandler(_c_rpcMsgHandle, MSG_TYPE_RRESP_JRPC);
	//�첽��Ϣ������
	client_registMessageHandler(_c_pubsubMsgHandle, MSG_TYPE_ASYNC_RESP);
	//��Ϣ���ĺ�ȡ�����ģ���Ϣת�� ����ֵ������
	client_registMessageHandler(_c_pubsubOpMsgHandle, MSG_TYPE_PUBSUB_RESP);

	//��ʼ��¼JMicro������,������������ƽ̨����
	client_login(actName,pwd);

	return true;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_login(char *actName, char *pwd){
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

	return SUCCESS;
}

ICACHE_FLASH_ATTR BOOL client_distroy() {
	if(sendBuf) {
		bb_free(sendBuf);
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

	if(msg_sender == NULL) return SOCKET_SENDER_NULL;

	if(loginKey) {
		msg_putCharsExtra(msg, EXTRA_KEY_LOGIN_KEY, loginKey, strlen(loginKey));
	}

	if(!msg_encode(msg,sendBuf)) {
		//��������ʧ��
		bb_reset(sendBuf);
		return ENCODE_MSG_FAIL;
	}

	return msg_sender(sendBuf);
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg){

	//bb_print(msg->payload);

	CHRI *h = _c_GetMsgHandler(msg->type);
	if(h == NULL) {
		return HANDLE_MSG_FAIL;
	}
	//ת����handler����
	return h->handler(msg);
}

ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn hdl, sint8_t type){
	CHRI *h = _c_GetMsgHandler(type);
	if(h != NULL) {
		//��Ϣ�������Ѿ�����
		return false;
	}

	h = (CHRI *)os_zalloc(sizeof(struct msg_handler_register_item));
	h->handler = hdl;
	h->type = type;
	h->next = NULL;

	if(handlers == NULL) {
		handlers = h;
	} else {
		//ͷ������
		h->next = handlers;
		handlers = h;
	}

	return true;

}

/**
 *args ��һ��cJSON���͵Ĳ�������,  һ���� cJSON_CreateArray������Ȼ������cJSON_AddItemToArray
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithArrayArgs(sint32_t mcode, cJSON *args,
		client_rpc_callback_fn callback, void *cbArgs){

	cJSON *jo = cJSON_CreateObject();
	if(jo == NULL) {
		return MEMORY_OUTOF_RANGE;
	}

	cJSON_AddItemToObject(jo,"args",args);

	cJSON *params = cJSON_CreateObject();

	cJSON_AddItemToObject(jo,"params",params);
	cJSON_AddItemToObject(params,"NCR",cJSON_CreateString(""));

	char *jsonbody = cJSON_PrintUnformatted(jo);

	client_send_msg_result_t rst = client_invokeRpcWithStrArgs(mcode, jsonbody, callback,NULL);

	cJSON_Delete(jo);

	return rst;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithStrArgs(sint32_t mcode, char *payload,
		client_rpc_callback_fn callback, void *cbArgs){

	sint16_t len = strlen(payload);
	byte_buffer_t * pl = bb_allocate(len);
	if(pl == NULL) {
		return MEMORY_OUTOF_RANGE;
	}

	if(!bb_put_chars(pl,payload,len)) {
		bb_free(pl);
		return SEND_DATA_ERROR;
	}

	client_send_msg_result_t rst = client_invokeRpc(mcode,pl,callback,NULL);
	bb_free(pl);

	return rst;
}

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpc(sint32_t mcode, byte_buffer_t *payload,
		client_rpc_callback_fn callback, void *cbArgs){
	jm_msg_t *msg = msg_create_rpc_msg(mcode, payload);
	if(msg == NULL) {
		return MEMORY_OUTOF_RANGE;
	}

	//���洫������payload���ͷ��ڴ棬�����봦���ͷ�

	client_msg_result_t *wait = _c_createRpcWaitForResponse();
	if(wait == NULL) {
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

	client_send_msg_result_t sendRst = client_sendMessage(msg);//��RPC����

	if(sendRst != SUCCESS) {//callback == NULL��ʾ ������Ӧ��Ϣ
		msg->payload = NULL;
		msg_release(msg);//�ͷ��ڴ�
		_c_rebackRpcWaitRorResponse(wait);
		return sendRst;
	}

	if(callback == NULL) {
		//���践��ֵ��RPC
		return msg->msgId;
	}
	sint64_t msgId = msg->msgId;
	msg->payload = NULL;
	msg_release(msg);

	return msgId;
}

//ȡ����Ϣ������
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
	client_msg_result_t * wait = _c_GetRpcWaitForResponse(msg->msgId);
	if(wait == NULL) {
		msg_release(msg);
		return MSG_WAIT_NOT_FOUND;
	}

	//֪ͨ�ص�����
	wait->callback(msg->payload,wait->cbArg);
	//msg_release(msg);

	_c_rebackRpcWaitRorResponse(wait);

	return SUCCESS;
}


/*===============================�첽��Ϣ���� ��ʼ==========================================*/

static ICACHE_FLASH_ATTR jm_pubsub_item_t* _c_createPubsubItem(){
	size_t s = sizeof(struct _c_pubsub_item);
	jm_pubsub_item_t *it = os_zalloc(s);
	memset(it,0,s);
	return it;
}

static ICACHE_FLASH_ATTR void _c_pubsubItemRelease(jm_pubsub_item_t *it){
	if(!it) return;

	if(it->data) {
		bb_free(it->data);
		it->data = NULL;
	}

	if(it->cxt) {
		msg_extra_release(it->cxt);
		it->cxt = NULL;
	}

	if(it->topic) {
		os_free(it->topic);
	}

	os_free(it);

}

//ȡ����Ϣ������
static ICACHE_FLASH_ATTR ps_listener_map* _c_getPubsubListenerMap(char *topic, BOOL docreate){
	ps_listener_map *h;
	if(ps_listener != NULL) {
		h = ps_listener;
		while(h != NULL) {
			if(0 == strcmp(topic,h->topic)) {
				return h;
			}else {
				h = h->next;
			}
		}
	}

	if(!docreate) return NULL;

	//����һ���µļ�������Ӱ��
	h = (ps_listener_map*)os_zalloc(sizeof(struct _pubsub_listener_map));
	if(h == NULL) return NULL;//�ڴ����

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

//����������Ӧ��� client_subscribe  client_unsubscribe
static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubOpMsgHandle(jm_msg_t *msg) {
	sint8_t code = msg_getS8Extra(msg,EXTRA_KEY_PS_OP_CODE);
	if(code == MSG_OP_CODE_SUBSCRIBE) {
		//��Ϣ������Ӧ
		sint32_t subId = msg_getS8Extra(msg, EXTRA_KEY_EXT0);
		char *topic = msg_getCharsExtra(msg, EXTRA_KEY_PS_ARGS);

		ps_listener_map *m = _c_getPubsubListenerMap(topic,false);
		if(m == NULL) {
			INFO("�յ���Ч������Ӧtopic��%s",topic);
			//�ڴ����
			return MEMORY_OUTOF_RANGE;
		}

		ps_listener_item_t *it = NULL;
		if(m->listeners) {
			ps_listener_item_t *item = m->listeners;
			while(item) {
				if(item->subMsgId == msg->msgId) {
					it = item;
					break;
				}
				item = item->next;
			}
		}

		if(it) {
			it->subId = subId;
		}
	}

	return SUCCESS;
}

//��������
ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type){

	if(listener == NULL) return false;
	if(topic == NULL || strlen(topic) == 0) return false;

	ps_listener_map *m = _c_getPubsubListenerMap(topic,true);
	if(m == NULL) {
		//�ڴ����
		return false;
	}

	jm_msg_t *msg = msg_create_msg(MSG_TYPE_PUBSUB,NULL);
	if(msg == NULL) {
		return false;
	}

	if(m->listeners) {
		ps_listener_item_t *item = m->listeners;
		while(item) {
			if(item->lis == listener) return true;//�������Ѿ����ڣ�ֱ�ӷ��سɹ�
			item = item->next;
		}
	}

	ps_listener_item_t *item = (ps_listener_item_t*)os_zalloc(sizeof(struct _pubsub_listener_item));
	if(item == NULL) return false;//�ڴ����ʧ��

	item->lis = listener;
	item->next = NULL;
	item->type = type;
	item->subMsgId = msg->msgId;
	item->subId = 0;

	if(m->listeners == NULL) {
		m->listeners = item;
	} else {
		//ͷ������
		item->next = m->listeners;
		m->listeners = item;
	}

	//���Ĳ���
	msg_putByteExtra(msg, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_SUBSCRIBE);
	msg_putCharsExtra(msg, EXTRA_KEY_PS_ARGS, topic, strlen(topic));
	//�������󣬷��ط���Ψһ��ϢID
	/*client_send_msg_result_t subRes = */
	client_sendMessage(msg);
	/*if(!subRes) {
		return false;
	}*/
	//�ͷ��ڴ�
	msg_release(msg);

	//���ĳɹ�
	return true;
}

//ȡ����������
ICACHE_FLASH_ATTR BOOL client_unsubscribe(char *topic, client_PubsubListenerFn listener){
	if(listener == NULL) return false;
	if(topic == NULL || strlen(topic) == 0) return false;

	ps_listener_map *m = _c_getPubsubListenerMap(topic,false);
	if(m == NULL) {
		//����������Ķ���
		return true;
	}

	if(m->listeners == NULL) {
		return true;//�����ڼ�������ֱ�ӷ��سɹ�
	}

	ps_listener_item_t *it, *pre, *cit;

	it = pre = NULL;
	cit = m->listeners;

	while(cit) {
		if(cit->lis == listener) {
			it = cit;
			break;//�ҵ���������Ŀ
		}
		pre = cit;
		cit = cit->lis;//������һ��
	}

	if(cit) {
		//�ҵ�����ɾ������
		if(pre == NULL) {
			//ֻ��һ��������
			m->listeners = NULL;
		} else {
			pre->next = cit->next;
			cit->next = NULL;
		}

		jm_msg_t* msg = msg_create_msg(MSG_TYPE_PUBSUB,NULL);
		if(msg == NULL) {
			return false;
		}

		/**
		 let ps = [{k:Constants.EXTRA_KEY_PS_OP_CODE, v:MSG_OP_CODE_UNSUBSCRIBE, t:Constants.PREFIX_TYPE_BYTE},
		{k:Constants.EXTRA_KEY_PS_ARGS, v:callback.id, t:Constants.PREFIX_TYPE_INT}]
		 */
		//���Ĳ���
		msg_putByteExtra(msg, EXTRA_KEY_PS_OP_CODE, MSG_OP_CODE_UNSUBSCRIBE);
		msg_putIntExtra(msg, EXTRA_KEY_PS_ARGS, cit->subId);
		//�������󣬷��ط���Ψһ��ϢID
		client_send_msg_result_t subRes = client_sendMessage(msg);
		/*if(!subRes) {
			return false;
		}*/
		//�ͷ��ڴ�
		msg_release(msg);

		os_free(cit);
	}

	//���ĳɹ�
	return true;
}

//�ַ���Ϣ��������
static ICACHE_FLASH_ATTR void _c_dispachPubsubItem(jm_pubsub_item_t *it){
	if(it==NULL) return;

	ps_listener_map *m = _c_getPubsubListenerMap(it->topic,false);
	if(m == NULL || m->listeners == NULL) {
		//����������Ķ���
		return;
	}

	ps_listener_item_t *lis_item = m->listeners;
	while(lis_item) {
		if(lis_item->type == 0 || lis_item->type == it->type) {
			lis_item->lis(it);
		}
		lis_item = lis_item->next;
	}
}

static ICACHE_FLASH_ATTR void _c_pubsubItemParseBin(jm_msg_t *msg){
	jm_pubsub_item_t *it = _c_createPubsubItem();
	if(!it){
		goto error;
	}

	byte_buffer_t *buf = msg->payload;

	if(!bb_get_s8(buf,&it->flag)) {
		goto error;
	}

	if(!bb_get_s64(buf,&it->id)) {
		goto error;
	}

	if(!bb_get_s32(buf,&it->srcClientId)) {
		goto error;
	}

	if(!bb_get_s32(buf,&it->fr)) {
		goto error;
	}

	if(!bb_get_s32(buf,&it->to)) {
		goto error;
	}

	if(!bb_get_s8(buf,&it->delay )) {
		goto error;
	}

	if(!bb_get_s8(buf,&it->type )) {
		goto error;
	}

	it->data = bb_read_buf(buf);
	if(it->data == NULL) {
		goto error;
	}

	uint16_t elen;
	if(!bb_get_u16(buf, &elen)) {
		INFO("ERROR:_c_ps_item_parse_bin read extra data length fail\r\n");
		goto error;
	}

	it->cxt = msg_decodeExtra(buf, elen);
	if(it->cxt == NULL) {
		INFO("ERROR: read extra data fail\r\n");
		goto error;
	}

	it->topic = bb_read_chars(buf);
	if(it->topic == NULL) {
		INFO("ERROR: read topic fail\r\n");
		goto error;
	}

	_c_dispachPubsubItem(it);

	error:
		_c_pubsubItemRelease(it);
		return;
}

static ICACHE_FLASH_ATTR void _c_pubsubItemParseJson(jm_msg_t *msg){
	jm_pubsub_item_t *it = _c_createPubsubItem();
	if(!it) goto error;

	byte_buffer_t *buf = msg->payload;

	if(buf== NULL) goto error;

	//bb_print(buf);

	size_t s = bb_readable_len(buf);
	cJSON *json = cJSON_ParseWithLength(buf->data, s);
	//cJSON *json = cJSON_Parse(buf->data);
	if(!json) goto error;

	if(cJSON_GetErrorPtr()) {
		printf("ERROR: %s",cJSON_GetErrorPtr());
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
	char* data = item->valuestring;// cJSON_PrintUnformatted(item);//��Ӧ�ý�����������
	if(data != NULL) {
		sint16_t sl = strlen(data)+1;
		byte_buffer_t *b = bb_allocate(sl);
		bb_put_chars(b,data,sl);
		bb_put_char(b,'\0');
		it->data = b;
	}

	//printf("type: %d, data:%s\n",it->type,data);

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

	error:
		if(json) cJSON_Delete(json);
		_c_pubsubItemRelease(it);
		return;
}

static ICACHE_FLASH_ATTR client_send_msg_result_t _c_pubsubMsgHandle(jm_msg_t *msg){
	if(msg_getDownProtocol(msg) == PROTOCOL_JSON) {
		_c_pubsubItemParseJson(msg);
	} else {
		_c_pubsubItemParseBin(msg);
	}
	return SUCCESS;
}

/**
 * �����첽��Ϣ
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content){
	if(topic == NULL || strlen(topic) == 0) return INVALID_PS_DATA;

	jm_pubsub_item_t *item = (jm_pubsub_item_t*)os_zalloc(sizeof(struct _c_pubsub_item));
	if(item == NULL) {
		return MEMORY_OUTOF_RANGE;
	}

	int len = strlen(content);
	byte_buffer_t *buf = NULL;
	if(len > 0) {
		buf = bb_allocate(len);
		bb_put_chars(buf,content,len);
	}

	item->flag = 0;
	item->cxt = NULL;
	item->data = buf;
	item->delay = 0;
	item->fr=0;
	item->to = 0;
	item->srcClientId = 0;
	item->topic = topic;
	item->id = ++msgId;
	item->type = type;

	client_send_msg_result_t rst = client_publishPubsubItem(item);

	os_free(item);
	if(buf)
		bb_free(buf);

	return rst;
}

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item){
	if(item == NULL || item->topic == NULL || strlen(item->topic) == 0) return INVALID_PS_DATA;

	cJSON *json = cJSON_CreateObject();

	cJSON *ji = cJSON_CreateNumber(item->flag);
	cJSON_AddItemToObject(json,"flag", ji);

	ji = cJSON_CreateNumber(item->id);
	cJSON_AddItemToObject(json,"id", ji);

	ji = cJSON_CreateNumber(item->srcClientId);
	cJSON_AddItemToObject(json,"srcClientId", ji);

	ji = cJSON_CreateNumber(item->fr);
	cJSON_AddItemToObject(json,"fr", ji);

	ji = cJSON_CreateNumber(item->to);
	cJSON_AddItemToObject(json,"to", ji);

	ji = cJSON_CreateNumber(item->delay);
	cJSON_AddItemToObject(json,"delay", ji);

	ji = cJSON_CreateString(item->topic);
	cJSON_AddItemToObject(json,"topic", ji);

	if(item->data) {
		ji = cJSON_CreateIntArray(item->data->data, bb_readable_len(item->data));
		cJSON_AddItemToObject(json,"data", ji);
	}

	if(item->cxt) {
		msg_extra_data_t *ic = item->cxt;
		while(ic) {

			ic = ic->next;
		}
	}

	char *itemData = cJSON_PrintUnformatted(json);
	int len = strlen(itemData);

	byte_buffer_t *buf = bb_allocate(len);
	bb_put_chars(buf,itemData,len);

	jm_msg_t* msg = msg_create_ps_msg(buf);
	client_send_msg_result_t sendRst = client_sendMessage(msg);//��RPC����

	//�ͷ��ڴ�
	msg_release(msg);
	cJSON_Delete(json);
	os_free(itemData);

	return sendRst;
}

//�����豸�첽��Ϣ
static ICACHE_FLASH_ATTR uint8_t _c_dispatchPubsubItemByType(jm_pubsub_item_t *psItem){
	printf("_c_dispatchPubsubItemByType: ");
	printf("type: %d, data:%s\n",psItem->type,psItem->data->data);
	return SUCCESS;
}

static ICACHE_FLASH_ATTR void _c_doSubscribeDeviceMessage(){
	if(loginCode != LSUCCESS) {
		INFO("�˺�δ��¼�����ܶ�������");
	}

	 char actIdStr[32];
	 itoa(actId, actIdStr, 10);

	size_t len = strlen(TOPIC_PREFIX) + strlen(DEVICE_ID) + strlen(actIdStr) + 1;
	char *topic = os_zalloc(len);
	if(topic == NULL) {
		INFO("do_subscribe_topic�ڴ����");
		return;
	}

	memset(topic,0,len);
	strncpy(topic, TOPIC_PREFIX, strlen(TOPIC_PREFIX));
	strcat(topic, actIdStr);
	strcat(topic, DEVICE_ID);

	//_c_pubsubMsgHandle type = 0,��ʾ����ȫ���豸��Ϣ
	client_subscribe(/*TOPIC_PREFIX*/topic, _c_dispatchPubsubItemByType, 0);

	//os_free(topic);
}

/*===============================�첽��Ϣ���� ���� ==========================================*/
