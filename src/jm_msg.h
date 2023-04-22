
#ifndef JMICRO_JM_MSG_MSG_H_
#define JMICRO_JM_MSG_MSG_H_

#include "c_types.h"
#include "jm_buffer.h"

#define HEADER_LEN  13 // 2(flag)+2(data len with short)+1(type)

#define EXT_HEADER_LEN  2 //附加数据头部长度

//public static final int SEC_LEN  128

#define  PROTOCOL_BIN  0
#define  PROTOCOL_JSON  1

#define  PRIORITY_0  0
#define  PRIORITY_1  1
#define  PRIORITY_2  2
#define  PRIORITY_3  3
#define  PRIORITY_4  4
#define  PRIORITY_5  5
#define  PRIORITY_6  6
#define  PRIORITY_7  7

#define  PRIORITY_MIN  PRIORITY_0
#define  PRIORITY_NORMAL  PRIORITY_3
#define  PRIORITY_MAX  PRIORITY_7

#define MAX_SHORT_VALUE  32767

#define MAX_BYTE_VALUE  127

#define MAX_INT_VALUE  (1024*1024*10)  //10M

//public static final long MAX_LONG_VALUE  Long.MAX_VALUE*2

//public static final byte MSG_VERSION  (byte)1

//长度字段类型，1表示整数，0表示短整数
#define FLAG_LENGTH_INT  (1 << 0)

#define FLAG_UP_PROTOCOL  (1<<1)
#define FLAG_DOWN_PROTOCOL  (1 << 2)

//可监控消息
#define FLAG_MONITORABLE  (1 << 3)

//包含Extra数据
#define FLAG_EXTRA  (1 << 4)

//来自外网消息，由网关转发到内网
#define FLAG_OUT_MESSAGE  (1 << 5)

#define FLAG_ERROR  (1 << 6)

//需要响应的请求  or down message is error
#define FLAG_FORCE_RESP_JSON  (1 << 7)

#define FLAG_RESP_TYPE  11

#define FLAG_LOG_LEVEL  13

/****************  extra constants flag   *********************/

//调试模式
#define EXTRA_FLAG_DEBUG_MODE  (1 << 0)

#define EXTRA_FLAG_PRIORITY  1

//DUMP上行数据
#define EXTRA_FLAG_DUMP_UP  (1 << 3)

//DUMP下行数据
#define EXTRA_FLAG_DUMP_DOWN  (1 << 4)

//加密参数 0：没加密，1：加密
#define EXTRA_FLAG_UP_SSL  (1 << 5)

//是否签名
#define EXTRA_FLAG_DOWN_SSL  (1 << 6)

#define EXTRA_FLAG_IS_SEC  (1 <<8)

//是否签名： 0:无签名； 1：有签名
#define EXTRA_FLAG_IS_SIGN  (1 << 9)

//加密方式： 0:对称加密，1：RSA 非对称加密
#define EXTRA_FLAG_ENC_TYPE  (1 << 10)

#define EXTRA_FLAG_RPC_MCODE  (1 << 11)

#define EXTRA_FLAG_SECTET_VERSION  (1 << 12)

//是否包含实例ID
#define EXTRA_FLAG_INS_ID  (1 << 13)

#define EXTRA_FLAG_FROM_APIGATEWAY  (1 << 14)

#define EXTRA_KEY_LINKID  -127
#define EXTRA_KEY_INSID  -126
#define EXTRA_KEY_TIME  -125
#define EXTRA_KEY_SM_CODE  -124
#define EXTRA_KEY_SM_NAME  -123
#define EXTRA_KEY_SIGN  -122
#define EXTRA_KEY_SALT  -121
#define EXTRA_KEY_SEC  -120
#define EXTRA_KEY_LOGIN_KEY  -119

//public static final Byte EXTRA_KEY_ARRAY  -116
#define EXTRA_KEY_FLAG  -118

#define EXTRA_KEY_MSG_ID  -117

#define EXTRA_KEY_LOGIN_SYS  -116
#define EXTRA_KEY_ARG_HASH -115

#define EXTRA_KEY_PS_OP_CODE -114//操作码
#define EXTRA_KEY_PS_ARGS -113 //参数

//服务器返回全局唯一标识ID
#define EXTRA_KEY_SMSG_ID -112

//rpc method name
#define EXTRA_KEY_METHOD  127
#define EXTRA_KEY_EXT0  126
#define EXTRA_KEY_EXT1  125
#define EXTRA_KEY_EXT2  124
#define EXTRA_KEY_EXT3  123

#define EXTRA_KEY_CLIENT_ID  122
#define EXTRA_KEY_EXT4  121
#define EXTRA_KEY_EXT5  120
#define MSG_TYPE_PINGPONG  0//默认请求响应模式

#define MSG_TYPE_NO_RESP  1//单向模式

#define MSG_TYPE_MANY_RESP  2//多个响应模式，如消息订阅


#define  PREFIX_TYPE_ID -128

#define  GET_PREFIX(n) (PREFIX_TYPE_ID+n)

//空值编码
#define  PREFIX_TYPE_NULL GET_PREFIX(0) //-128

//FINAL
#define  PREFIX_TYPE_FINAL GET_PREFIX(1) //-127

//类型编码写入编码中
#define  PREFIX_TYPE_SHORT (GET_PREFIX(2))//-126
//全限定类名作为前缀串写入编码中
#define  PREFIX_TYPE_STRING (GET_PREFIX(3))//-125

//以下对高使用频率非final类做快捷编码

//列表类型编码，指示接下业读取一个列表，取列表编码器直接解码
#define  PREFIX_TYPE_LIST GET_PREFIX(4)//-124
//集合类型编码，指示接下来读取一个集合，取SET编码器直接解码
#define  PREFIX_TYPE_SET GET_PREFIX(5)//-123
//Map类型编码，指示接下来读取一个Map，取Map编码器直接解码
#define  PREFIX_TYPE_MAP GET_PREFIX(6)//-122

#define  PREFIX_TYPE_BYTE GET_PREFIX(7)//-121
#define  PREFIX_TYPE_SHORTT GET_PREFIX(8)//-120
#define  PREFIX_TYPE_INT GET_PREFIX(9)//-119
#define  PREFIX_TYPE_LONG GET_PREFIX(10)//-118
#define  PREFIX_TYPE_FLOAT GET_PREFIX(11)//-117
#define  PREFIX_TYPE_DOUBLE GET_PREFIX(12)//-116
#define  PREFIX_TYPE_CHAR GET_PREFIX(13)//-115
#define  PREFIX_TYPE_BOOLEAN GET_PREFIX(14)//-114
#define  PREFIX_TYPE_STRINGG GET_PREFIX(15)//-113
#define  PREFIX_TYPE_DATE GET_PREFIX(16)//-112
#define  PREFIX_TYPE_BYTEBUFFER GET_PREFIX(17)//-111
#define  PREFIX_TYPE_REQUEST GET_PREFIX(18)//-110
#define  PREFIX_TYPE_RESPONSE GET_PREFIX(19)//-109
#define  PREFIX_TYPE_PROXY GET_PREFIX(20)//-108


#ifdef __cplusplus
extern "C"
{
#endif

typedef union _msg_extra_data_val {
	char* bytesVal;
	char charVal;
	sint8_t s8Val;
	sint16_t s16Val;
	sint32_t s32Val;
	sint64_t s64Val;
	BOOL boolVal;

	/*uint8_t u8Val;
	uint16_t u16Val;
	uint32_t u32Val;
	uint64_t u64Val;*/
} msg_extra_data_val;

typedef struct _msg_extra_data {
	sint8_t key;
	msg_extra_data_val value;
	sint8_t type;
	uint16_t len;
	struct _msg_extra_data *next;
} msg_extra_data_t;

typedef struct _jm_req {

} jm_req_t;


/**
 * Messge format:
 *
 * +++ 2 bytes （flag） +++  2 or 4 bytes （len） +++ 1 byte （type） +++ 4 bytes （extra flag） +++ 2 bytes extra data len +++ extra data +++ payload data +++

 * a. 2 bytes flag: 固定2字节不变
 * b. 2 or 4 bytes len 根据Message.FLAG_LENGTH_INT值确定是2字节还是4字节，1表示4字节，0表示两字节，len的值等于 4（如果存在，extra flag长度）+
 * 2（如果存在，extra长度） + extra data长度（如果存在） + data长度

 	如FLAG_EXTRA=1,则包含以下信息
 * c. 4 bytes （extra flag）
 * d. 2 bytes extra data len  表示附加数据长度
 * e. extra data

 */
typedef struct _jm_msg {
	//0B00111000 5---3
	//public static final short FLAG_LEVEL = 0X38;

	//是否启用服务级log
	//public static final short FLAG_LOGGABLE = 1 << 3;
	uint64_t startTime;

	//此消息所占字节数，用于记录流量
	//uint32_t len = -1;

	//1 byte length
	//private byte version;

	//payload length with byte,4 byte length
	//private int len;

	/**
	 * 0        S:       data length type 0:short 1:int
	 * 1        UPR:     up protocol  0: bin,  1: json
	 * 2        DPR:     down protocol 0: bin, 1: json
	 * 3        M:       Monitorable
	 * 4        Extra    Contain extra data
	 * 5        Innet    message from outer network
	 * 6
	 * 7
	 * 8
	 * 9
	 * 10
	 * 11，12   Resp type  MSG_TYPE_PINGPONG，MSG_TYPE_NO_RESP，MSG_TYPE_MANY_RESP
	 * 13 14 15 LLL      Log level
	 * @return
	 */
	sint16_t flag;

	// 1 byte
	uint8_t type;

	//2 byte length
	//private byte ext;

	//normal message ID	or JRPC request ID
	sint64_t msgId;

	byte_buffer_t *payload;

	//private Map<Byte,Object> extraMap;
	msg_extra_data_t *extraMap;

	/**
	 * 0        dm:       is development mode EXTRA_FLAG_DEBUG_MODE = 1 << 1;
	 * 1,2      PP:       Message priority   EXTRA_FLAG_PRIORITY
	 * 3        up:       dump up stream data
	 * 4        do:       dump down stream data
	 * 5 	    US        上行SSL  0:no encrypt 1:encrypt
	 * 6        DS        下行SSL  0:no encrypt 1:encrypt
	 * 7
	 * 8        MK        RPC方法编码
	 * 9        SV        对称密钥版本
	 * 10       SE        密码
	 * 11       SI        是否有签名值 0：无，1：有
	 * 12       ENT       encrypt type 0:对称加密，1：RSA 非对称加密
	 * 13       ERROR     0:正常包， 1：错误响应包
	          E  ENT SI  SE  WE MK SV  DS   US   DO   UP  P    P   dm
	 |    |   |   |  |   |   |  |  |   |    |    |    |   |    |   |
     15  14  13  12  11  10  9  8  7   6    5    4    3   2    1   0

	 |    |   |   |  |   |   |    |   |   |    |    |    |   |    |   |
     31  30  29  28  27  26  25   24  23  22   21   20   19  18   17  16

	 *
	 * @return
	 */
	sint32_t extrFlag ;

	//附加数居
	//uint8_t *extra;

	//jm_req_t *req;

	//****************extra data begin*******************//
} jm_msg_t;

ICACHE_FLASH_ATTR BOOL msg_isUpSsl(jm_msg_t *msg);
ICACHE_FLASH_ATTR void msg_setUpSsl(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR BOOL msg_isDownSsl(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setDownSsl(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isFromApiGateway(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setFromApiGateway(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isRsaEnc(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setEncType(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isSecretVersion(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setSecretVersion(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isSign(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setSign(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isSec(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setSec(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isRpcMk(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setRpcMk(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isDumpUpStream(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setDumpUpStream(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isDumpDownStream(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setDumpDownStream(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  bool msg_isLoggable(jm_msg_t *msg);
ICACHE_FLASH_ATTR  bool msg_isDebugMode(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setDebugMode(jm_msg_t *msg, bool f);
ICACHE_FLASH_ATTR  BOOL msg_isMonitorable(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setMonitorable(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isError(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setError(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isOuterMessage(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setOuterMessage(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isForce2Json(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setForce2Json(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isNeedResponse(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_isPubsubMessage(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_isPingPong(jm_msg_t *msg);

/**
 * @param f true 表示整数，false表示短整数
 */
ICACHE_FLASH_ATTR  void msg_setLengthType(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR BOOL msg_isLengthInt(jm_msg_t *msg);
ICACHE_FLASH_ATTR  sint8_t msg_getPriority(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_setPriority(jm_msg_t *msg, sint8_t l);
ICACHE_FLASH_ATTR  sint8_t msg_getLogLevel(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_setLogLevel(jm_msg_t *msg, sint8_t v);
ICACHE_FLASH_ATTR sint8_t msg_getRespType(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_setRespType(jm_msg_t *msg, sint16_t v);
ICACHE_FLASH_ATTR  sint8_t msg_getUpProtocol(jm_msg_t *msg );
ICACHE_FLASH_ATTR  void msg_setUpProtocol(jm_msg_t *msg, sint8_t protocol);
ICACHE_FLASH_ATTR  sint8_t msg_getDownProtocol(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setDownProtocol(jm_msg_t *msg, sint8_t protocol);

//ICACHE_FLASH_ATTR  void msg_setInsId(jm_msg_t *msg, sint32_t *insId);
//ICACHE_FLASH_ATTR  sint32_t msg_getInsId(jm_msg_t *msg);
//ICACHE_FLASH_ATTR  void msg_setSignData(jm_msg_t *msg, char *data);
//ICACHE_FLASH_ATTR  char* msg_getSignData(jm_msg_t *msg);
//ICACHE_FLASH_ATTR  void msg_setLinkId(jm_msg_t *msg, sint64_t *insId);
//ICACHE_FLASH_ATTR  sint64_t msg_getLinkId(jm_msg_t *msg);
//ICACHE_FLASH_ATTR void msg_setSaltData(jm_msg_t *msg, char* data);
//ICACHE_FLASH_ATTR char* msg_getSaltData(jm_msg_t *msg);
//ICACHE_FLASH_ATTR  void msg_setSecData(jm_msg_t *msg, char* data);
//ICACHE_FLASH_ATTR char* msg_getSecData(jm_msg_t *msg, char **rst);
//ICACHE_FLASH_ATTR  void msg_setSmKeyCode(jm_msg_t *msg, sint32_t *code);
//ICACHE_FLASH_ATTR  sint32_t msg_getSmKeyCode(jm_msg_t *msg);
//ICACHE_FLASH_ATTR  void msg_setMethod(jm_msg_t *msg, char* method);
//ICACHE_FLASH_ATTR  char* msg_getMethod(jm_msg_t *msg);
//ICACHE_FLASH_ATTR  void msg_setTime(jm_msg_t *msg, sint64_t *time);
//ICACHE_FLASH_ATTR sint64 msg_getTime(jm_msg_t *msg);

ICACHE_FLASH_ATTR jm_msg_t *msg_decode(byte_buffer_t *b);
ICACHE_FLASH_ATTR BOOL msg_encode(jm_msg_t *msg, byte_buffer_t *buf);
ICACHE_FLASH_ATTR jm_msg_t *msg_readMessage(byte_buffer_t *buf);

ICACHE_FLASH_ATTR jm_msg_t* msg_create_rpc_msg(sint32_t mcode, byte_buffer_t *payload);
ICACHE_FLASH_ATTR jm_msg_t* msg_create_ps_msg(byte_buffer_t *payload);
ICACHE_FLASH_ATTR jm_msg_t* msg_create_msg(sint8_t tyep, byte_buffer_t *payload);
ICACHE_FLASH_ATTR void msg_release(jm_msg_t *msg);

ICACHE_FLASH_ATTR  sint16_t msg_getS16Extra(jm_msg_t *msg, sint8_t key);
ICACHE_FLASH_ATTR  sint8_t msg_getS8Extra(jm_msg_t *msg, sint8_t key);
ICACHE_FLASH_ATTR  sint64_t msg_getS64Extra(jm_msg_t *msg, sint8_t key);
ICACHE_FLASH_ATTR  sint32_t msg_getS32Extra(jm_msg_t *msg, sint8_t key);
ICACHE_FLASH_ATTR  char* msg_getCharsExtra(jm_msg_t *msg, sint8_t key);
//ICACHE_FLASH_ATTR void msg_putExtra(jm_msg_t *msg, sint8_t key, void *val, sint8_t type);

ICACHE_FLASH_ATTR msg_extra_data_t* msg_getExtra(jm_msg_t *msg, sint8_t key);
ICACHE_FLASH_ATTR BOOL msg_putByteExtra(jm_msg_t *msg, sint8_t key, sint8_t val);
ICACHE_FLASH_ATTR BOOL msg_putShortExtra(jm_msg_t *msg, sint8_t key, sint16_t val);
ICACHE_FLASH_ATTR BOOL msg_putIntExtra(jm_msg_t *msg, sint8_t key, sint32_t val);
ICACHE_FLASH_ATTR BOOL msg_putLongExtra(jm_msg_t *msg, sint8_t key, sint64_t val);
ICACHE_FLASH_ATTR BOOL msg_putCharExtra(jm_msg_t *msg, sint8_t key, char val);
ICACHE_FLASH_ATTR BOOL msg_putBoolExtra(jm_msg_t *msg, sint8_t key, BOOL val);
ICACHE_FLASH_ATTR BOOL msg_putCharsExtra(jm_msg_t *msg, sint8_t key, const char* val, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif /* JMICRO_JM_MSG_MSG_H_ */
