/*
 * jm_client.h
 *
 *  Created on: 2023��4��16��
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

//��5��6��λһ���ʾdata�ֶεı�������
#define FLAG_DATA_TYPE 5
#define FLAG_DATA_STRING 0
#define FLAG_DATA_BIN 1
#define FLAG_DATA_JSON 2
#define FLAG_DATA_NONE 3

#define client_setPSItemDataType(v,flag) (*flag = (v << FLAG_DATA_TYPE) | *flag)

#define MSG_OP_CODE_SUBSCRIBE 1//锟斤拷锟斤拷锟斤拷息
#define MSG_OP_CODE_UNSUBSCRIBE 2//取锟斤拷锟斤拷锟斤拷锟斤拷息
#define MSG_OP_CODE_FORWARD 3//转锟斤拷锟斤拷息
#define MSG_OP_CODE_FORWARD_BY_TOPIC 4//����������ת��

typedef enum _client_act_login_status{
	LSUCCESS=0,//��¼�ɹ�
	LOGIN_FAIL,//��¼ʧ��
	LOGOUT,//û��¼
} client_act_login_status;

typedef struct _c_pubsub_item{

	//��־
	uint8_t dataFlag;

	//private byte
	uint8_t flag;

	//��Ϣ��Դ����Ӧ�豸ID
	//private Integer
	sint32_t fr;

	//��ϢID,Ψһ��ʶһ����Ϣ
	//private long
	sint64_t id; //0

	sint8_t type;//1 ��Ϣ���ͣ�������������Ϣת��

	//����
	//private String
	char *topic; //2

	//Դ�⻧���豸�������⻧
	//private int
	sint32_t srcClientId; //3

	//��Ϣ���͸�˭����Ӧ�豸ID
	//private Integer
	sint32_t to; //4

	//��Ϣ���ͽ���ص���RPC������������Ϣ�������������߻ص�
	//private String
	char *callback ; //5

	//�ӳٶ�÷��ͣ���λ����
	//private long
	uint8_t delay;  //6

	//��Ϣ������
	/*private Map<String,Object>*/
	msg_extra_data_t *cxt; //7

	//��Ϣ����
	//private Object
	//byte_buffer_t *data;  //8

	void *data;

	//���ػص�
	//private transient ILocalCallback localCallback;

	//�ͻ��˷���ʧ�ܴ����������ط������������Ϣʧ�ܴ�������һ����������Ϣ������������localCallback��������ڣ�֪ͨ�����ߣ�
	//private transient int failCnt = 0;

} jm_pubsub_item_t;

/**
 * ��Ϣ������
 * ÿ����Ϣtype��Ҫһ����Ϣ������������
 *
 */
typedef client_send_msg_result_t (*client_msg_hander_fn)(jm_msg_t *msg);

/**
 * �첽��Ϣ������
 * ����ͻ����첽��Ϣ�ַ�
 */
//typedef void (*MqttCallback)(uint32_t *args);
typedef uint8_t (*client_on_async_msg_fn)(jm_pubsub_item_t *item);

typedef uint8_t (*client_rpc_callback_fn)(byte_buffer_t *payload, void *cbArg);

typedef uint8_t (*client_PubsubListenerFn)(jm_pubsub_item_t *psItem);

//�˺ŵ�¼״̬������
typedef void (*client_login_listener_fn)(sint32_t code, char *msg, char *loginKey, sint32_t actId);


typedef void (*client_conn_discon_fn)();

/*
 * �������з�����Ϣ�� ��ͬƽ̨��ʵ�ִ˽ӿ�
 */
typedef client_send_msg_result_t (*client_send_msg_fn)(byte_buffer_t *buf);

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR BOOL client_init();

//�ײ��������ӶϿ�
ICACHE_FLASH_ATTR BOOL client_socketDisconCb();

//�ײ��������ӳɹ�
ICACHE_FLASH_ATTR BOOL client_socketConedCb();

ICACHE_FLASH_ATTR BOOL client_socketSendTimeoutCb();

//ע�����ݷ��������ɾ�������ʵ���ߵ���
ICACHE_FLASH_ATTR BOOL client_registMessageSender(client_send_msg_fn sender);

//������յ�����Ϣ
ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg);

//������Ϣ
ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg);

//ע����Ϣ������
ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn msg, sint8_t type);

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithArrayArgs(sint32_t mcode, cJSON *args,
		client_rpc_callback_fn callback, void *cbArgs);

ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpcWithStrArgs(sint32_t mcode, char *payload,
		client_rpc_callback_fn callback, void *cbArgs);

/**
 * RPC ��������
 * mcode����������
 * payload�� ����
 * rst�� �����ŵ�ַ��rst==NULL��������Ӧ���
 * ����ֵ 0��ʾ���óɹ�, ������ʾʧ��
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_invokeRpc(sint32_t mcode, byte_buffer_t *payload,
		client_rpc_callback_fn callback, void *cbArgs);

/**
 * �����첽��Ϣ
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content, msg_extra_data_t *extra);

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item, msg_extra_data_t *extra);

/**
 * �����첽��Ϣ
 */
ICACHE_FLASH_ATTR BOOL client_subscribe(char *topic, client_PubsubListenerFn listener, sint8_t type);

/**
 *
 */
ICACHE_FLASH_ATTR BOOL client_subscribeByType(client_PubsubListenerFn listener, sint8_t type);

/**
 * ȡ������
 */
ICACHE_FLASH_ATTR BOOL client_unsubscribe(char *topic, client_PubsubListenerFn listener);

/**
 * ע���¼״̬��������������ע��5��������
 */
ICACHE_FLASH_ATTR BOOL client_registLoginListener(client_login_listener_fn fn);

/**
 * ɾ����¼״̬������
 */
ICACHE_FLASH_ATTR BOOL client_unregistLoginListener(client_login_listener_fn fn);

/**
 * ��¼JMicroƽ̨
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_login(char *actName, char *pwd);

/**
 * �ǳ�JMicroƽ̨
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_logout();

#ifdef __cplusplus
}
#endif

#endif /* TESTCASE_JM_CLIENT_H_ */
