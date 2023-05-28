
#ifndef JMICRO_JM_MSG_MSG_H_
#define JMICRO_JM_MSG_MSG_H_

#include "c_types.h"
#include "jm_buffer.h"

#define HEADER_LEN  13 // 2(flag)+2(data len with short)+1(type)

#define EXT_HEADER_LEN  2

//public static final int SEC_LEN  128

#define  PROTOCOL_BIN  0
#define  PROTOCOL_JSON  1
#define  PROTOCOL_EXTRA  2
#define  PROTOCOL_RAW 3

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

//public static final long MAX_LONG_VALUE  Long.MAX_VALUE*2

//public static final byte MSG_VERSION  (byte)1

#define FLAG_LENGTH_INT  (1 << 0)

#define FLAG_UP_PROTOCOL  1
#define FLAG_DOWN_PROTOCOL  8
#define FLAG_UP_PROTOCOL_MASK  0xFFF9
#define FLAG_DOWN_PROTOCOL_MASK  0xFCFF

#define FLAG_MONITORABLE  (1 << 3)

#define FLAG_EXTRA  (1 << 4)

#define FLAG_OUT_MESSAGE  (1 << 5)

#define FLAG_ERROR  (1 << 6)

#define FLAG_FORCE_RESP_JSON  (1 << 7)

#define FLAG_DEV  (1 << 10)

#define FLAG_RESP_TYPE  11
#define FLAG_RESP_TYPE_MASK  0xE7FF //1110 0111 1111 1111

#define FLAG_LOG_LEVEL  13
#define FLAG_LOG_LEVEL_MASK  0x1FFF //0001 1111 1111 1111

/****************  extra constants flag   *********************/

#define EXTRA_FLAG_DEBUG_MODE  (1 << 0)

#define EXTRA_FLAG_PRIORITY  1

#define EXTRA_FLAG_DUMP_UP  (1 << 3)

#define EXTRA_FLAG_DUMP_DOWN  (1 << 4)

#define EXTRA_FLAG_UP_SSL  (1 << 5)

#define EXTRA_FLAG_DOWN_SSL  (1 << 6)

#define EXTRA_FLAG_IS_SEC  (1 <<8)

#define EXTRA_FLAG_IS_SIGN  (1 << 9)

#define EXTRA_FLAG_ENC_TYPE  (1 << 10)

#define EXTRA_FLAG_RPC_MCODE  (1 << 11)

#define EXTRA_FLAG_SECTET_VERSION  (1 << 12)

#define EXTRA_FLAG_INS_ID  (1 << 13)

#define EXTRA_FLAG_FROM_APIGATEWAY  (1 << 14)

#define EXTRA_FLAG_UDP  (1 << 15)

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

#define EXTRA_KEY_PS_OP_CODE -114//
#define EXTRA_KEY_PS_ARGS -113 //

#define EXTRA_KEY_UDP_PORT -111//UDP远程端口
#define EXTRA_KEY_UDP_HOST -110//UDP远程主机地址
#define EXTRA_KEY_UDP_ACK -109//UDP是否需要应答，true需要应答，false不需要应答

#define EXTRA_SKEY_UDP_PORT "-111"//UDP远程端口
#define EXTRA_SKEY_UDP_HOST "-110"//UDP远程主机地址
#define EXTRA_SKEY_UDP_ACK "-109"//UDP是否需要应答，true需要应答，false不需要应答

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
#define MSG_TYPE_PINGPONG  0

#define MSG_TYPE_NO_RESP  1

#define MSG_TYPE_MANY_RESP  2


#define  PREFIX_TYPE_ID -128

#define  GET_PREFIX(n) (PREFIX_TYPE_ID+n)

#define  PREFIX_TYPE_NULL GET_PREFIX(0) //-128

//FINAL
#define  PREFIX_TYPE_FINAL GET_PREFIX(1) //-127

#define  PREFIX_TYPE_SHORT (GET_PREFIX(2))//-126
#define  PREFIX_TYPE_STRING (GET_PREFIX(3))//-125

#define  PREFIX_TYPE_LIST GET_PREFIX(4)//-124
#define  PREFIX_TYPE_SET GET_PREFIX(5)//-123
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

typedef union _msg_extra_data_val {
	char* bytesVal;
	char charVal;
	sint8_t s8Val;
	sint16_t s16Val;
	sint32_t s32Val;
	sint64_t s64Val;
	BOOL boolVal;
} msg_extra_data_val;

typedef struct _msg_extra_data {
	sint8_t key;
	char* strKey;
	BOOL neddFreeBytes;
	msg_extra_data_val value;
	sint8_t type;
	uint16_t len;
	struct _msg_extra_data *next;
} msg_extra_data_t;

typedef struct _msg_extra_data_iterator {
	msg_extra_data_t *header;
	msg_extra_data_t *cur;
} msg_extra_data_iterator_t;

/**
 * Messge format:

 */
typedef struct _jm_msg {
	//0B00111000 5---3
	//public static final short FLAG_LEVEL = 0X38;

	//�Ƿ����÷���log
	//public static final short FLAG_LOGGABLE = 1 << 3;
	uint64_t startTime;

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
	 * 11��12   Resp type  MSG_TYPE_PINGPONG��MSG_TYPE_NO_RESP��MSG_TYPE_MANY_RESP
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
	 * 5 	    US        ����SSL  0:no encrypt 1:encrypt
	 * 6        DS        ����SSL  0:no encrypt 1:encrypt
	 * 7
	 * 8        MK        RPC��������
	 * 9        SV        �Գ���Կ�汾
	 * 10       SE        ����
	 * 11       SI        �Ƿ���ǩ��ֵ 0���ޣ�1����
	 * 12       ENT       encrypt type 0:�ԳƼ��ܣ�1��RSA �ǶԳƼ���
	 * 13       ERROR     0:�������� 1��������Ӧ��
	 * 14       GW        API gateway forward message   EXTRA_FLAG_FROM_APIGATEWAY
	 * 15       UDP       通过UDP传输报文，1: UDP, 0:TCP
	 UDP  GW  E  ENT SI  SE  WE MK SV  DS   US   DO   UP  P    P   dm
	 |    |   |   |  |   |   |  |  |   |    |    |    |   |    |   |
     15  14  13  12  11  10  9  8  7   6    5    4    3   2    1   0

	 |    |   |   |  |   |   |    |   |   |    |    |    |   |    |   |
     31  30  29  28  27  26  25   24  23  22   21   20   19  18   17  16

	 *
	 * @return
	 */
	sint32_t extrFlag ;

	//uint8_t *extra;

	//jm_req_t *req;

	struct _jm_msg *cacheNext;
} jm_msg_t;

typedef void (*extra_iterator_fn)(sint8_t key, void *val, sint8_t type);

typedef void (*extra_siterator_fn)(char *key, void *val, sint8_t type);

#ifdef __cplusplus
extern "C" {
#endif
/***************************************EXTRA DATA OPERATION BEGIN***************************************/
/**
 */
ICACHE_FLASH_ATTR msg_extra_data_t *extra_decode(byte_buffer_t *b);

ICACHE_FLASH_ATTR BOOL extra_encode(msg_extra_data_t *extras, byte_buffer_t *b, uint16_t *wl,uint8_t keyType);

ICACHE_FLASH_ATTR void extra_release(msg_extra_data_t *extra);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_create();

ICACHE_FLASH_ATTR msg_extra_data_t * extra_pullAll(msg_extra_data_t *from, msg_extra_data_t *to);

ICACHE_FLASH_ATTR msg_extra_data_t * extra_iteNext();

ICACHE_FLASH_ATTR msg_extra_data_t* extra_sget(msg_extra_data_t *header, char *key);
ICACHE_FLASH_ATTR sint16_t extra_sgetS16(msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR sint8_t extra_sgetS8(msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR sint64_t extra_sgetS64(msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR sint32_t extra_sgetS32(msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR char extra_sgetChar(msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR BOOL extra_sgetBool(msg_extra_data_t *e, char *strKey);

ICACHE_FLASH_ATTR char* extra_sgetChars(msg_extra_data_t *e, char *strKey);
ICACHE_FLASH_ATTR  char* extra_sgetCharsCpy(msg_extra_data_t *e, char *strKey);

ICACHE_FLASH_ATTR msg_extra_data_t* extra_sput(msg_extra_data_t *header, char *strKey, sint8_t type);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputByte(msg_extra_data_t *e, char *strKey, sint8_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputShort(msg_extra_data_t *e, char *strKey, sint16_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputInt(msg_extra_data_t *e, char *strKey, sint32_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputLong(msg_extra_data_t *e, char *strKey, sint64_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputChar(msg_extra_data_t *e, char *strKey, char val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputBool(msg_extra_data_t *e, char *strKey, BOOL val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_sputChars(msg_extra_data_t *e, char *strKey, char* val, uint16_t len);

ICACHE_FLASH_ATTR msg_extra_data_t* extra_get(msg_extra_data_t *header, sint8_t key);
ICACHE_FLASH_ATTR  sint16_t extra_getS16(msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  sint8_t extra_getS8(msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  sint64_t extra_getS64(msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  sint32_t extra_getS32(msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  char extra_getChar(msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  BOOL extra_getBool(msg_extra_data_t *e, sint8_t key);
ICACHE_FLASH_ATTR  char* extra_getChars(msg_extra_data_t *e, sint8_t key);
//ICACHE_FLASH_ATTR void msg_putExtra(jm_msg_t *msg, sint8_t key, void *val, sint8_t type);

ICACHE_FLASH_ATTR msg_extra_data_t* extra_put(msg_extra_data_t *header, sint8_t key, sint8_t type);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putByte(msg_extra_data_t *e, sint8_t key, sint8_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putShort(msg_extra_data_t *e, sint8_t key, sint16_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putInt(msg_extra_data_t *e, sint8_t key, sint32_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putLong(msg_extra_data_t *e, sint8_t key, sint64_t val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putChar(msg_extra_data_t *e, sint8_t key, char val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putBool(msg_extra_data_t *e, sint8_t key, BOOL val);
ICACHE_FLASH_ATTR msg_extra_data_t* extra_putChars(msg_extra_data_t *e, sint8_t key, char* val, uint16_t len);

/**************************************EXTRA DATA OPERATION END**********************************/

ICACHE_FLASH_ATTR BOOL msg_isUpSsl(jm_msg_t *msg);
ICACHE_FLASH_ATTR void msg_setUpSsl(jm_msg_t *msg, BOOL f);

ICACHE_FLASH_ATTR BOOL msg_isUdp(jm_msg_t *msg);
ICACHE_FLASH_ATTR void msg_setUdp(jm_msg_t *msg, BOOL f);

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
ICACHE_FLASH_ATTR  void msg_setDev(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR  BOOL msg_isDev(jm_msg_t *msg);
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
 */
ICACHE_FLASH_ATTR  void msg_setLengthType(jm_msg_t *msg, BOOL f);
ICACHE_FLASH_ATTR BOOL msg_isLengthInt(jm_msg_t *msg);
ICACHE_FLASH_ATTR  sint8_t msg_getPriority(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_setPriority(jm_msg_t *msg, sint32_t l);
ICACHE_FLASH_ATTR  sint8_t msg_getLogLevel(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_setLogLevel(jm_msg_t *msg, sint16_t v);
ICACHE_FLASH_ATTR sint8_t msg_getRespType(jm_msg_t *msg);
ICACHE_FLASH_ATTR  BOOL msg_setRespType(jm_msg_t *msg, sint16_t v);
ICACHE_FLASH_ATTR  sint8_t msg_getUpProtocol(jm_msg_t *msg );
ICACHE_FLASH_ATTR  void msg_setUpProtocol(jm_msg_t *msg, sint16_t protocol);
ICACHE_FLASH_ATTR  sint8_t msg_getDownProtocol(jm_msg_t *msg);
ICACHE_FLASH_ATTR  void msg_setDownProtocol(jm_msg_t *msg, sint16_t protocol);

ICACHE_FLASH_ATTR jm_msg_t *msg_decode(byte_buffer_t *b);
ICACHE_FLASH_ATTR BOOL msg_encode(jm_msg_t *msg, byte_buffer_t *buf);
ICACHE_FLASH_ATTR jm_msg_t *msg_readMessage(byte_buffer_t *buf);

ICACHE_FLASH_ATTR jm_msg_t* msg_create_rpc_msg(sint32_t mcode, byte_buffer_t *payload);
ICACHE_FLASH_ATTR jm_msg_t* msg_create_ps_msg(byte_buffer_t *payload);
ICACHE_FLASH_ATTR jm_msg_t* msg_create_msg(sint8_t tyep, byte_buffer_t *payload);
ICACHE_FLASH_ATTR void msg_release(jm_msg_t *msg);

#ifdef __cplusplus
}
#endif
#endif /* JMICRO_JM_MSG_MSG_H_ */
