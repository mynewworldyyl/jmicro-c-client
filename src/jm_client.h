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

#define JM_HEARBEET_INTERVAL 30000//向JM平台发送心跳间隔，超过30秒发送一次

//第5，6两位一起表示data字段的编码类型
#define FLAG_DATA_TYPE 5

#define FLAG_DATA_STRING 0
#define FLAG_DATA_BIN 1
#define FLAG_DATA_JSON 2
#define FLAG_DATA_EXTRA 3

#define client_setPSItemDataType(v,flag) (*flag = (v << FLAG_DATA_TYPE) | *flag)

#define TOPIC_P2P "/__act/dev/p2pctrl" //设备与手机端对端消息主题，不需要登录设备，也不能从服务器下发

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

typedef uint8_t (*client_rpc_callback_fn)(void *resultMap, sint32_t code, char *errMsg, void *arg);

typedef uint8_t (*client_PubsubListenerFn)(jm_pubsub_item_t *psItem);

typedef void (*client_login_listener_fn)(sint32_t code, char *msg, char *loginKey, sint32_t actId);


typedef void (*client_conn_discon_fn)();

typedef client_send_msg_result_t (*client_send_msg_fn)(byte_buffer_t *buf);

//消息广播发送器
typedef client_send_msg_result_t (*client_p2p_msg_sender_fn)(byte_buffer_t *buf, char* host, uint16_t port, uint16_t hlen);

typedef bool (*client_timer_check_fn)(/*void *arg*/);

typedef uint64_t (*client_getSystemTime_fn)();

typedef struct _c_timer_check{
	client_timer_check_fn jm_checkNet;//本地网络状态检测
	client_timer_check_fn jm_checkConCheck;//与JM物联网平台的连接
	client_timer_check_fn jm_checkLocalServer;//本地设备端对端连接
	client_timer_check_fn jm_checkLoginStatus;//物联网平台登录状态

} timer_check;

#ifdef __cplusplus
extern "C"
{
#endif

//设置系统时间获取器
ICACHE_FLASH_ATTR void client_setSysTimeFn(client_getSystemTime_fn fn);

//设备JM平台服务地址
ICACHE_FLASH_ATTR void client_setJmInfo(char *jmh, uint16_t port, uint8_t udp);

//定时检测系统各服务状态，确保系统在条件充许性况下可以正常运行
ICACHE_FLASH_ATTR BOOL client_main_timer();

ICACHE_FLASH_ATTR timer_check* client_getCheck();

ICACHE_FLASH_ATTR BOOL client_isLogin();

ICACHE_FLASH_ATTR BOOL client_init(/*sint32_t actId, char *deviceId, BOOL doLogin*/);

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
ICACHE_FLASH_ATTR sint64_t client_invokeRpc(sint32_t mcode, msg_extra_data_t *params,
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
ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type, BOOL p2p);

/**
 *
 */
ICACHE_FLASH_ATTR BOOL client_subscribeByType(client_PubsubListenerFn listener, sint8_t type,BOOL p2p);

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
ICACHE_FLASH_ATTR client_send_msg_result_t client_login(sint32_t actId, char *deviceId);

/**
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_logout();


/*************************************KEY Value Storage begin***********************************************/
ICACHE_FLASH_ATTR BOOL kv_delete(char *name, client_rpc_callback_fn cb);
ICACHE_FLASH_ATTR BOOL kv_update(char *name, char *desc, void *val, sint8_t type, client_rpc_callback_fn cb);
ICACHE_FLASH_ATTR BOOL kv_get(char *name, client_rpc_callback_fn cb);
ICACHE_FLASH_ATTR BOOL kv_add(char *name, void *val, char *desc, sint8_t type, client_rpc_callback_fn cb);

/*************************************KEY Value Storage end***********************************************/


/*************************************Client extra begin***********************************************/
ICACHE_FLASH_ATTR BOOL client_encodeExtra(byte_buffer_t *b, msg_extra_data_t *extras, sint8_t type);
ICACHE_FLASH_ATTR msg_extra_data_t* client_decodeExtra(byte_buffer_t *b);
/*************************************Client extra end***********************************************/

#ifdef __cplusplus
}
#endif

#endif /* TESTCASE_JM_CLIENT_H_ */
