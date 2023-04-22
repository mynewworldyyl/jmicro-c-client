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

typedef enum _client_send_msg_result{
	SUCCESS=-100,
	SOCKET_SENDER_NULL=-99,//底层SOCKET没建立
	ENCODE_MSG_FAIL=-98,//消息编码失败
	HANDLE_MSG_FAIL=-97,//消息处理器未找到
	MSG_CREATE_FAIL=-96,//消息创建失败
	MEMORY_OUTOF_RANGE=-95,//内存申请失败，也就是内存溢出
	MSG_WAIT_NOT_FOUND=-94,//没找到等待响应的消息
	SEND_DATA_ERROR=-93,//发送数据错误
	NO_DATA_TO_SEND=-92,//无数据可发送,
	INVALID_PS_DATA=-91, //PUBSUB数据无效

} client_send_msg_result_t;

typedef enum _client_act_login_status{
	LSUCCESS=0,//登录成功
	LOGIN_FAIL,//登录失败
	LOGOUT,//没登录
} client_act_login_status;

typedef struct _c_pubsub_item{

	//消息ID,唯一标识一个消息
	//private long
	sint64_t id;

	//源租户，设备服务商租户
	//private int
	sint32_t srcClientId;

	//消息来源，对应设备ID
	//private Integer
	sint32_t fr;

	//消息发送给谁，对应设备ID
	//private Integer
	sint32_t to;

	//消息数据
	//private Object
	byte_buffer_t *data;

	//消息发送结果回调的RPC方法，用于消息服务器给发送者回调
	//private String
	//char *callback ;

	//本地回调
	//private transient ILocalCallback localCallback;

	//客户端发送失败次数，用于重发计数，如果消息失败次数到达一定量，将消息丢弃，并调用localCallback（如果存在）通知调用者，
	//private transient int failCnt = 0;

	//消息上下文
	/*private Map<String,Object>*/
	msg_extra_data_t *cxt;


	//主题
	//private String
	char *topic;

	//标志
	//private byte
	sint8_t flag;

	sint8_t type;//消息类型，根据类型做消息转发

	//延迟多久发送，单位是秒
	//private long
	sint8_t delay;

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


/*
 * 向网络中发关消息， 不同平台需实现此接口
 */
typedef client_send_msg_result_t (*client_send_msg_fn)(byte_buffer_t *buf);

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR BOOL client_init();

ICACHE_FLASH_ATTR BOOL client_registMessageSender(client_send_msg_fn sender);

//发送消息
ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg);

//注册消息处理器
ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn msg, sint8_t type);

//处理接收到的消息
ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg);

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
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content);

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item);

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
