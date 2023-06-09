/*
 * jmconstants.h
 *
 *  Created on: 2023��4��15��
 *      Author: yeyulei
 */

#ifndef TESTCASE_JMCONSTANTS_H_
#define TESTCASE_JMCONSTANTS_H_

#define RESP_CODE  "code" //响应码键
#define RESP_MSG  "msg" //响应错误时错误消键
#define RESP_OP  "op" //操作码，可选，操作实现可使用此值做操作分类，如 查询， 增加，删除，更新，一般默认0表示 查询

#define RESP_OP_QRY 0
#define RESP_OP_ADD 1
#define RESP_OP_DEL 2
#define RESP_OP_UP 3
#define RESP_OP_CODE_DESC "Op Code"
#define RESP_OP_DEVICEID_DESC "Device ID"
#define RESP_OP_ACTID_DESC "Act ID"

/*public static final long */
#define DAY_IN_MILLIS (24*60*60*1000)

//public static final String  HTTP_txtContext= "_txt_";
/*public static final String  */
#define HTTP_binContext  "/_bin_"//ר����web Socket rpc
/*public static final String */
#define HTTP_httpContext "/_http_"//ר����HTTP RPC
/*public static final String  */
#define HTTP_fsContext "/_fs_"//ר�����ļ�����
/*public static final String */
#define HTTP_statis "/statis"//��̬��Դ����Ŀ¼

/*public static final String */
#define HTTP_METHOD_POST "POST"
/*public static final String  */
#define HTTP_METHOD_GET "GET"

/*public static final String  */
#define HTTP_JSON_CONTENT_TYPE "application/json;chartset=utf-8"
/*public static final String  */
#define HTTP_ALL_CONTENT_TYPE  "*"

//���ı����ݣ���Ӧ��spring-web��restcontrollerע��
/*public static final byte */
#define HTTP_RESP_TYPE_RESTFULL 1

//����HTML����˷����ļ���ͼ·�������ض�ȡ�ļ������������Ϻ󷵻ظ��ͻ���
/*public static final byte  */
#define HTTP_RESP_TYPE_VIEW 2

//���ؽ����������ͨ���ļ�ϵͳ�����ļ�ID�������ض�ȡ�ļ������ظ��ͻ���
/*public static final byte  */
#define HTTP_RESP_TYPE_STREAM 3

//����ԭʼ���ݣ��������ֵ��RespJRsoʵ�����򷵻�RespJRso.data������ֱ�ӷ���resp
//��������֧���첽֪ͨ��Ҫ����"SUCCESS"�ַ�����
/*public static final byte */
#define HTTP_RESP_TYPE_ORIGIN 4

/*public static final Set<String> HTTP_PREFIX_LIST;
static {
	HashSet<String> h = new HashSet<>();
	h.add(Constants.HTTP_binContext);
	h.add(Constants.HTTP_fsContext);
	h.add(Constants.HTTP_httpContext);
	h.add(Constants.HTTP_statis);
	//h.add(Constants.HTTP_txtContext);
	HTTP_PREFIX_LIST = Collections.unmodifiableSet(h);
}
*/

/*public static final String*/
#define BASE64_IMAGE_PREFIX  "data:image/jpeg;base64,"
/*public static final String*/
#define BASE64_IMAGE_SUBFIX  "=="

/*public static final String*/
#define JMICRO_READY_METHOD_NAME  "jready"

//public static final String JMICRO_READY = "jmicroReady";

/*public static final String */
#define CORE_CLASS "cn.jmicro.api"
/*public static final String */
#define INVALID_LOG_DESC  "nl"
/*public static final byte */
#define USE_SYSTEM_CLIENT_ID  -2

/*public static final String */
#define LOGIN_KEY  "-119"

/*public static final int */
#define NO_CLIENT_ID  -1

/*public static final byte */
#define LIMIT_TYPE_LOCAL  1

/*public static final byte */
#define LIMIT_TYPE_SS  2

/*public static final byte */
#define FOR_TYPE_USER  1

/*public static final byte */
#define FOR_TYPE_SYS  2

/*public static final byte */
#define FOR_TYPE_ALL  3

/*public static final byte */
#define LICENSE_TYPE_FREE  0

/*public static final byte */
#define LICENSE_TYPE_CLIENT  1

/*public static final byte */
#define LICENSE_TYPE_PRIVATE  2

/*public static final String */
#define SYSTEM_PCK_NAME_PREFIXE  "cn.jmicro"

/*public static final String[] */
#define SYSTEM_PCK_NAME_PREFIXES  ({SYSTEM_PCK_NAME_PREFIXE,"sun.reflect","com.alibaba.fastjson"})

/*public static final String */
#define PUBLIC_KEYS_FILES  "publicKeyFiles"

/*public static final String */
#define PRIVATE_KEY_PWD  "priKeyPwd"

/*public static final String */
#define JMICRO_VERSION  "0.0.2"

/*public static final String */
#define JMICRO_RELEASE_LABEL  "SNAPSHOT"

/*public static final String */
#define PATH_EXCAPE  "#@!"

/*public static final String */
#define TOPIC_SEPERATOR ","

/*public static final String */
#define CLIENT_ID "clientId"

/*public static final String */
#define CLIENT_NAME "actName"

/*public static final String */
#define SYSTEM_LOG_LEVEL "sysLogLevel"

//public static final String ADMIN_CLIENT_ID = "adminClientId";

/*public static final String */
#define CLIENT_ONLY  "client"

/*public static final String */
#define SERVER_ONLY  "server"

/*public static final String */
#define TYPE_CODE_PRODUCER  "typeCodeProducer"

/*public static final String */
#define CFG_ROOT "/jmicro"

/*public static final String */
#define MASTER_SLAVE_TAG  "masterSlaveTag"

/*public static final String */
#define LOCAL_DATA_DIR  "localDataDir"
/*public static final String */
#define INSTANCE_DATA_DIR  "instanceDataDir"

/*public static final String */
#define DEFAULT_PREFIX "JMICRO"

/*public static final String */
#define SIDE_COMSUMER  "comsumer"
/*public static final String */
#define SIDE_PROVIDER "provider"
/*public static final String */
#define SIDE_ANY "any"

//public static final String MONITOR_ENABLE_KEY = "monitorEnableKey";
//public static final String DEFAULT_MONITOR="defaultMonitor";
/*public static final String */
#define FROM_MONITOR "fromMonitor"

/*public static final String */
#define FROM_PUBSUB "fromPubsub"
/*public static final String */
#define FROM_MONITOR_CLIENT "_fromMonitorManager"

/*public static final int */
#define CONN_CONNECTED 1
/*public static final int */
#define CONN_RECONNECTED 2
/*public static final int */
#define CONN_LOST 3

/*public static final String */
#define OBJ_FACTORY_KEY "objFactory"
/*public static final String */
#define DEFAULT_OBJ_FACTORY "defaultObjFactory"

/*public static final String */
#define DATA_OPERATOR "dataOperator"
/*public static final String */
#define DEFAULT_DATA_OPERATOR  "ZKDataOperator"

/*public static final String */
#define REGISTRY_KEY "registry"
/*public static final String*/
#define DEFAULT_REGISTRY "defaultRegistry"

/*public static final String */
#define PUBSUB_KEY "pubsub"
/*public static final String */
#define STATIS_KEY "statis"

/*public static final String */
#define EXECUTOR_POOL "executorPool"

/*public static final String */
#define EXECUTOR_GATEWAY_KEY  "/gatewayModel"
/*public static final String */
#define EXECUTOR_RECEIVE_KEY  "/serverReceiver"

/*public static final String */
#define DEFAULT_PUBSUB "org.jmicro.pubsub.PubSubServer"

/*public static final String */
#define DEFAULT_CODEC_FACTORY  "defaultCodecFactory"
/*public static final String */
#define DEFAULT_SERVER  "defaultServer"
/*public static final String */
#define DEFAULT_HANDLER  "defaultHandler"
/*public static final String */
#define LAST_INTERCEPTOR  "lastInterceptor"
/*public static final String */
#define FIRST_INTERCEPTOR "firstInterceptor"

/*public static final String */
#define DEFAULT_CLIENT_HANDLER  "defaultClientHandler"
/*public static final String */
#define LAST_CLIENT_INTERCEPTOR  "lastClientInterceptor"
/*public static final String */
#define FIRST_CLIENT_INTERCEPTOR  "firstClientInterceptor"
/*public static final String */
#define PROXY  "proxy"

/*public static final String */
#define DEFAULT_SELECTOR  "defaultSelector"
/*public static final String */
#define DEFAULT_INVOCATION_HANDLER  "defaultInvocationHandler"
//public static final String SPECIAL_INVOCATION_HANDLER = "specailInvocationHandler";

/*public static final String */
#define VERSION  "0.0.5"
/*public static final String */
#define DEFAULT_IDGENERATOR  "defaultGenerator"

/*public static final String */
#define CHARSET  "UTF-8"
/*public static final String */
#define IO_SESSION_KEY  "_iosessionKey"

/*public static final String */
#define IO_BIN_SESSION_KEY "_bin_iosessionKey"

//public static final String CONFIG_ROOT = CFG_ROOT + "/config";
//public static final String RAFT_CONFIG_ROOT_KEY = "configRoot";
/*public static final String */
#define REGISTRY_URL_KEY  "registryUrl"
//public static final String BIND_IP = "bindIp";
/*public static final String */
#define ExportSocketIP  "exportSocketIP"
/*public static final String */
#define ExportHttpIP  "exportHttpIP"

/*public static final String */
#define ListenSocketIP  "listenSocketIP"
/*public static final String */
#define ListenHttpIP  "listenHttpIP"

/*public static final String */
#define RemoteIp  "remoteIp"
/*public static final String */
#define LocalIp "localIp"

/*public static final String */
#define BASE_PACKAGES_KEY  "basePackages"
/*public static final String */
#define JMICRO_COMPONENTS_KEY  "components"
/*public static final String */
#define INSTANCE_PREFIX  "instanceName"
/*public static final String */
#define INSTANCE_NAME_GEN_CLASS  "instanceNameGenClass"

/*public static final String */
#define LOCAL_INSTANCE_NAME  "localInstanceName"

//public static final String CONTEXT_CALLBACK_SERVICE = "ServiceCallback";
//public static final String CONTEXT_CALLBACK_CLIENT = "ClientCallback";
//public static final String CONTEXT_SERVICE_RESPONSE = "serviceResponse";

/*public static final String */
#define SERVICE_ITEM_KEY  "serviceItemKey"

/*public static final String */
#define ASYNC  "_async"

/*public static final String */
#define SERVICE_NAME_KEY  "nameKey"

/*public static final String */
#define SERVICE_NAMESPACE_KEY  "namespaceKey"

/*public static final String */
#define SERVICE_VERSION_KEY  "versionKey"

//public static final String SERVICE_SPECIFY_ITEM_KEY = "specifyServiceItemKey";

/*public static final String */
#define SERVICE_METHOD_KEY  "serviceMethodKey"
/*public static final String */
#define ASYNC_CONFIG  "asyncConfig"
/*public static final String */
#define NEW_LINKID  "newLinkId"
/*public static final String */
#define SERVICE_OBJ_KEY  "serviceObjKey"
/*public static final String */
#define CLIENT_REF_METHOD  "reflectMethod"

/*public static final String */
#define TRANSPORT_JDKHTTP  "jdkhttp"
/*public static final String */
#define TRANSPORT_MINA "mina"
/*public static final String */
#define TRANSPORT_NETTY  "netty"
/*public static final String */
#define TRANSPORT_NETTY_HTTP  "nettyhttp"

/*public static final String */
#define BREAKER_TEST_CONTEXT  "breakerTestContext"

//public static final String REF_ANNO = "referenceAnno";
/*public static final String */
#define DIRECT_SERVICE_ITEM  "directServiceItem"

/*public static final String */
#define DIRECT_SERVICE_ITEM_CLIENT_ID  "directServiceItemClientId"

/*public static final String */
#define DIRECT_SERVICE_ITEM_INS_NAME  "directServiceItemInsName"

/*public static final String */
#define DIRECT_SERVICE_ITEM_INS_PREFIX  "directServiceItemInsPrefix"

/*public static final String */
#define ROUTER_TYPE  "routerKey"

/*public static final int */
#define TYPE_HTTP  1
/*public static final int */
#define TYPE_SOCKET  2
/*public static final int */
#define TYPE_WEBSOCKET  3

/*public static final String */
#define HTTP_HEADER_ENCODER  "DataEncoderType"
/*public static final String */
#define START_HTTP  "startHttp"

/*public static final int */
#define DEFAULT_RESP_BUFFER_SIZE  1024*4


/*=====================Message Begin=======================*/

/*public static final byte */
#define MSG_TYPE_REQ_JRPC  0x01
/*public static final byte */
#define MSG_TYPE_RRESP_JRPC  0x02

/*public static final byte */
#define MSG_TYPE_PUBSUB  0x03
/*public static final byte */
#define MSG_TYPE_PUBSUB_RESP  0x04

/*public static final byte */
#define MSG_TYPE_ASYNC_REQ  0x05
/*public static final byte */
#define MSG_TYPE_ASYNC_RESP  0x06

/*public static final byte */
#define MSG_TYPE_API_CLASS_REQ  ((sint8_t)0x07)
/*public static final byte */
#define MSG_TYPE_API_CLASS_RESP ((sint8_t)0x08)

/*public static final byte */
#define MSG_TYPE_EXSECRET_REQ ((sint8_t)0x09)
/*public static final byte */
#define MSG_TYPE_EXSECRET_RESP ((sint8_t)0x0A)

/*public static final byte */
#define MSG_TYPE_ID_REQ ((sint8_t)0x0B)
/*public static final byte */
#define MSG_TYPE_ID_RESP ((sint8_t)0x0C)

/*public static final byte */
#define MSG_TYPE_HEARBEAT_REQ ((sint8_t)0x0D)
/*public static final byte */
#define MSG_TYPE_HEARBEAT_RESP ((sint8_t)0x0E)

/*public static final byte */
#define MSG_TYPE_SYSTEM_REQ_JRPC  0x0F)
/*public static final byte */
#define MSG_TYPE_SPECAIL_RRESP_JRPC  0x10//16

/*public static final byte */
#define MSG_TYPE_OBJECT_STORAGE  0x11
/*public static final byte */
#define MSG_TYPE_OBJECT_STORAGE_RESP  0x12//18

/*=====================Message END=======================*/

//time unit constant
/*public static final */
#define String TIME_DAY  "D" //��
/*public static final String */
#define TIME_HOUR  "H" //ʱ
/*public static final String */
#define TIME_MINUTES  "M" //��
/*public static final String */
#define TIME_SECONDS  "S" //��
/*public static final String */
#define TIME_MILLISECONDS  "MS" //����
/*public static final String */
#define TIME_MICROSECONDS  "MC" //΢��
/*public static final String */
#define TIME_NANOSECONDS  "N" //����


/*==============================================================*/

//�ֶ����ͣ�0����ʾ���룬1����ʾ����
/*public static final byte */
#define TYPE_VAL  0X40

/*public static final byte */
#define SIZE_NOT_ZERO  0X20

/*public static final byte */
#define HEADER_ELETMENT  0X10

/*public static final byte */
#define GENERICTYPEFINAL  0X08

/*public static final byte */
#define ELEMENT_TYPE_CODE  0X04

/*public static final byte */
#define EXT0  0X02

/*public static final byte */
#define EXT1 0X80

/*public static final byte */
#define NULL_VAL 0X01

/*==============================================================*/

/*public static final String */
#define CACHE_DIR_PREFIX  "CP:"

/*public static final byte */
#define CACHE_TYPE_NO  0

/*public static final byte */
#define CACHE_TYPE_MCODE  1

/*public static final byte */
#define CACHE_TYPE_ACCOUNT  2

/*public static final byte */
#define CACHE_TYPE_PAYLOAD  3

/*public static final byte */
#define CACHE_TYPE_PAYLOAD_AND_ACT  4

/*public static final byte */
#define OP_TYPE_ADD  1
/*public static final byte */
#define OP_TYPE_UPDATE 2
/*public static final byte */
#define OP_TYPE_DELETE  3
/*public static final byte */
#define OP_TYPE_QUERY  4
/*public static final byte */
#define OP_TYPE_START  5//��ʼ
/*public static final byte */
#define OP_TYPE_END  6//����
/*public static final byte */
#define OP_TYPE_PAUSE  7//��ͣ
/*public static final byte */
#define OP_TYPE_RESUME  8//����
/*public static final byte */
#define OP_TYPE_STATUS  9

#endif /* TESTCASE_JMCONSTANTS_H_ */
