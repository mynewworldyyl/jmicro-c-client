/*
 * jm_client.h
 *
 *  Created on: 2023��4��16��
 *      Author: yeyulei
 */

#ifndef TESTCASE_JM_CLIENT_H_
#define TESTCASE_JM_CLIENT_H_

#include "jm_msg.h"
#include "jm_buffer.h"
#include "c_types.h"

#define client_setPSItemDataType(v,flag) (*flag = (v << FLAG_DATA_TYPE) | *flag)

#define MSG_OP_CODE_SUBSCRIBE 1//订阅消息操作码
#define MSG_OP_CODE_UNSUBSCRIBE 2//取消订阅消息操作码
#define MSG_OP_CODE_FORWARD 3//依据账号ID做消息转发
#define MSG_OP_CODE_FORWARD_BY_TOPIC 4//根据主题做消息转发


typedef enum _client_act_login_status{
	LSUCCESS=0,//登录成功
	LOGIN_FAIL,//登录失败
	LOGOUT,//登出
} client_act_login_status;

typedef struct _c_pubsub_item{

	uint8_t dataFlag;

	//private byte
	uint8_t flag;

	//private Integer
	sint32_t fr;

	//private long
	sint64_t id; //0

	sint8_t type;

	//private String
	char *topic; //2

	//private int
	sint32_t srcClientId; //3

	//private Integer
	sint32_t to; //4

	//private String
	char *callback ; //5

	//private long
	uint8_t delay;  //6

	/*private Map<String,Object>*/
	msg_extra_data_t *cxt; //7

	//private Object
	//byte_buffer_t *data;  //8

	void *data;

	//private transient ILocalCallback localCallback;

	//private transient int failCnt = 0;

} jm_pubsub_item_t;


typedef client_send_msg_result_t (*client_msg_hander_fn)(jm_msg_t *msg);

//typedef void (*MqttCallback)(uint32_t *args);
typedef uint8_t (*client_on_async_msg_fn)(jm_pubsub_item_t *item);

typedef uint8_t (*client_rpc_callback_fn)(byte_buffer_t *payload, void *cbArg);

typedef uint8_t (*client_PubsubListenerFn)(jm_pubsub_item_t *psItem);

typedef void (*client_login_listener_fn)(sint32_t code, char *msg, char *loginKey, sint32_t actId);


typedef void (*client_conn_discon_fn)();

typedef client_send_msg_result_t (*client_send_msg_fn)(byte_buffer_t *buf);

//消息广播发送器
typedef client_send_msg_result_t (*client_p2p_msg_sender_fn)(byte_buffer_t *buf, char* host, uint16_t port);

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR BOOL client_init(char *actName, char *pwd, BOOL doLogin);

ICACHE_FLASH_ATTR BOOL client_socketDisconCb();

ICACHE_FLASH_ATTR BOOL client_socketConedCb();

ICACHE_FLASH_ATTR BOOL client_socketSendTimeoutCb();

ICACHE_FLASH_ATTR BOOL client_registMessageSender(client_send_msg_fn sender);

ICACHE_FLASH_ATTR BOOL client_registP2PMessageSender(client_p2p_msg_sender_fn sender);

ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg);

ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg);

ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn msg, sint8_t type);

/**
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpc(sint32_t mcode, msg_extra_data_t *params,
		client_rpc_callback_fn callback, void *cbArgs);

ICACHE_FLASH_ATTR void client_initPubsubItem(jm_pubsub_item_t *item,uint8_t dataType);
ICACHE_FLASH_ATTR msg_extra_data_t * client_topicForwardExtra(char *topic);

/**
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content, msg_extra_data_t *extra);

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item, msg_extra_data_t *extra);

/**
 */
ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type);

/**
 *
 */
ICACHE_FLASH_ATTR BOOL client_subscribeByType(client_PubsubListenerFn listener, sint8_t type);

/**
 */
ICACHE_FLASH_ATTR BOOL client_unsubscribe(char *topic, client_PubsubListenerFn listener);

/**
 * ע���¼״̬��������������ע��5��������
 */
ICACHE_FLASH_ATTR BOOL client_registLoginListener(client_login_listener_fn fn);

/**
 */
ICACHE_FLASH_ATTR BOOL client_unregistLoginListener(client_login_listener_fn fn);

/**
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_login(char *actName, char *pwd);

/**
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_logout();

#ifdef __cplusplus
}
#endif

#endif /* TESTCASE_JM_CLIENT_H_ */
