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

typedef enum _client_send_msg_result{
	SUCCESS=-100,
	SOCKET_SENDER_NULL=-99,//�ײ�SOCKETû����
	ENCODE_MSG_FAIL=-98,//��Ϣ����ʧ��
	HANDLE_MSG_FAIL=-97,//��Ϣ������δ�ҵ�
	MSG_CREATE_FAIL=-96,//��Ϣ����ʧ��
	MEMORY_OUTOF_RANGE=-95,//�ڴ�����ʧ�ܣ�Ҳ�����ڴ����
	MSG_WAIT_NOT_FOUND=-94,//û�ҵ��ȴ���Ӧ����Ϣ
	SEND_DATA_ERROR=-93,//�������ݴ���
	NO_DATA_TO_SEND=-92,//�����ݿɷ���,
	INVALID_PS_DATA=-91, //PUBSUB������Ч

} client_send_msg_result_t;

typedef enum _client_act_login_status{
	LSUCCESS=0,//��¼�ɹ�
	LOGIN_FAIL,//��¼ʧ��
	LOGOUT,//û��¼
} client_act_login_status;

typedef struct _c_pubsub_item{

	//��ϢID,Ψһ��ʶһ����Ϣ
	//private long
	sint64_t id;

	//Դ�⻧���豸�������⻧
	//private int
	sint32_t srcClientId;

	//��Ϣ��Դ����Ӧ�豸ID
	//private Integer
	sint32_t fr;

	//��Ϣ���͸�˭����Ӧ�豸ID
	//private Integer
	sint32_t to;

	//��Ϣ����
	//private Object
	byte_buffer_t *data;

	//��Ϣ���ͽ���ص���RPC������������Ϣ�������������߻ص�
	//private String
	//char *callback ;

	//���ػص�
	//private transient ILocalCallback localCallback;

	//�ͻ��˷���ʧ�ܴ����������ط������������Ϣʧ�ܴ�������һ����������Ϣ������������localCallback��������ڣ�֪ͨ�����ߣ�
	//private transient int failCnt = 0;

	//��Ϣ������
	/*private Map<String,Object>*/
	msg_extra_data_t *cxt;


	//����
	//private String
	char *topic;

	//��־
	//private byte
	sint8_t flag;

	sint8_t type;//��Ϣ���ͣ�������������Ϣת��

	//�ӳٶ�÷��ͣ���λ����
	//private long
	sint8_t delay;

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


/*
 * �������з�����Ϣ�� ��ͬƽ̨��ʵ�ִ˽ӿ�
 */
typedef client_send_msg_result_t (*client_send_msg_fn)(byte_buffer_t *buf);

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR BOOL client_init();

ICACHE_FLASH_ATTR BOOL client_registMessageSender(client_send_msg_fn sender);

//������Ϣ
ICACHE_FLASH_ATTR client_send_msg_result_t client_sendMessage(jm_msg_t *msg);

//ע����Ϣ������
ICACHE_FLASH_ATTR BOOL client_registMessageHandler(client_msg_hander_fn msg, sint8_t type);

//������յ�����Ϣ
ICACHE_FLASH_ATTR client_send_msg_result_t client_onMessage(jm_msg_t *msg);

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
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishStrItem(char *topic, sint8_t type, char *content);

/**
 *
 */
ICACHE_FLASH_ATTR client_send_msg_result_t client_publishPubsubItem(jm_pubsub_item_t *item);

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
