/*
 * jm_client.h
 *
 *  Created on: 2023年4月16日
 *      Author: yeyulei
 */

#ifndef TESTCASE_JM_CLIENT_H_
#define TESTCASE_JM_CLIENT_H_

#include "jm_buffer.h"
#include "jm_msg.h"
#include "cJSON.h"

#include "c_types.h"

#define CACHE_PUBSUB_ITEM "C_PS_ITEM_"
#define CACHE_PUBSUB_ITEM_EXTRA "C_PS_ITEM_EXTRA_"

#define CACHE_MESSAGE "C_MESSAGE_"
#define CACHE_MESSAGE_EXTRA "C_MESSAGE_EXTRA_"

//第5，6两位一起表示data字段的编码类型
#define FLAG_DATA_TYPE 5
#define FLAG_DATA_STRING 0
#define FLAG_DATA_BIN 1
#define FLAG_DATA_JSON 2
#define FLAG_DATA_NONE 3

#define client_setPSItemDataType(v,flag) (*flag = (v << FLAG_DATA_TYPE) | *flag)

#define MSG_OP_CODE_SUBSCRIBE 1//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋伅
#define MSG_OP_CODE_UNSUBSCRIBE 2//鍙栭敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋伅
#define MSG_OP_CODE_FORWARD 3//杞敓鏂ゆ嫹閿熸枻鎷锋伅
#define MSG_OP_CODE_FORWARD_BY_TOPIC 4//根据主题做转发

typedef enum _client_act_login_status{
	LSUCCESS=0,//登录成功
	LOGIN_FAIL,//登录失败
	LOGOUT,//没登录
} client_act_login_status;

typedef struct _c_pubsub_item{

	//标志
	uint8_t dataFlag;

	//private byte
	uint8_t flag;

	//消息来源，对应设备ID
	//private Integer
	sint32_t fr;

	//消息ID,唯一标识一个消息
	//private long
	sint64_t id; //0

	sint8_t type;//1 消息类型，根据类型做消息转发

	//主题
	//private String
	char *topic; //2

	//源租户，设备服务商租户
	//private int
	sint32_t srcClientId; //3

	//消息发送给谁，对应设备ID
	//private Integer
	sint32_t to; //4

	//消息发送结果回调的RPC方法，用于消息服务器给发送者回调
	//private String
	char *callback ; //5

	//延迟多久发送，单位是秒
	//private long
	uint8_t delay;  //6

	//消息上下文
	/*private Map<String,Object>*/
	msg_extra_data_t *cxt; //7

	//消息数据
	//private Object
	//byte_buffer_t *data;  //8

	void *data;

	//本地回调
	//private transient ILocalCallback localCallback;

	//客户端发送失败次数，用于重发计数，如果消息失败次数到达一定量，将消息丢弃，并调用localCallback（如果存在）通知调用者，
	//private transient int failCnt = 0;

} jm_pubsub_item_t;

/**
 * 消息处理器
 * 每个消息type需要一个消息处理器来处理
 *
 */
typedef client_send_msg_result_t (*client_msg_hander_fn)(jm_msg_t *msg);

/**
 * 异步消息监听器
 * 处理客户端异步消息分发
 */
//typedef void (*MqttCallback)(uint32_t *args);
typedef uint8_t (*client_on_async_msg_fn)(jm_pubsub_item_t *item);

typedef uint8_t (*client_rpc_callback_fn)(byte_buffer_t *payload, void *cbArg);

typedef uint8_t (*client_PubsubListenerFn)(jm_pubsub_item_t *psItem);

//账号登录状态监听器
typedef void (*client_login_listener_fn)(sint32_t code, char *msg, char *loginKey, sint32_t actId);


typedef void (*client_conn_discon_fn)();

/*
 * 向网络中发关消息， 不同平台需实现此接口
 */
typedef client_send_msg_result_t (*client_send_msg_fn)(byte_buffer_t *buf);

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR BOOL client_init();

//底层网络连接断开
ICACHE_FLASH_ATTR BOOL client_socketDisconCb();

//底层网络连接成功
ICACHE_FLASH_ATTR BOOL client_socketConedCb();

ICACHE_FLASH_ATTR BOOL client_socketSendTimeoutCb();

//注册数据发送器，由具体网终实层者调用
ICACHE_FLASH_ATTR BOOL client_registMessageSender(client_send_msg_fn sender);

//处理接收到的消息
ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg);

//发送消息
ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg);

//注册消息处理器
ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn msg, sint8_t type);

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithArrayArgs(sint32_t mcode, cJSON *args,
		client_rpc_callback_fn callback, void *cbArgs);

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithStrArgs(sint32_t mcode, char *payload,
		client_rpc_callback_fn callback, void *cbArgs);

/**
 * RPC 方法调用
 * mcode：方法编码
 * payload： 参数
 * rst： 结果存放地址，rst==NULL则无需响应结果
 * 返回值 0表示调用成功, 其他表示失败
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpc(sint32_t mcode, byte_buffer_t *payload,
		client_rpc_callback_fn callback, void *cbArgs);

/**
 * 发送异步消息
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content, msg_extra_data_t *extra);

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item, msg_extra_data_t *extra);

/**
 * 订阅异步消息
 */
ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type);

/**
 *
 */
ICACHE_FLASH_ATTR BOOL client_subscribeByType(client_PubsubListenerFn listener, sint8_t type);

/**
 * 取消订阅
 */
ICACHE_FLASH_ATTR BOOL client_unsubscribe(char *topic, client_PubsubListenerFn listener);

/**
 * 注册登录状态监听器，最多可以注册5个监听器
 */
ICACHE_FLASH_ATTR BOOL client_registLoginListener(client_login_listener_fn fn);

/**
 * 删除登录状态监听器
 */
ICACHE_FLASH_ATTR BOOL client_unregistLoginListener(client_login_listener_fn fn);

/**
 * 登录JMicro平台
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_login(char *actName, char *pwd);

/**
 * 登出JMicro平台
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_logout();

#ifdef __cplusplus
}
#endif

#endif /* TESTCASE_JM_CLIENT_H_ */
