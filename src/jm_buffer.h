/*
 * jm_buffer.h
 *
 *  Created on: 2023锟斤拷4锟斤拷9锟斤拷
 *      Author: yeyulei
 */


#ifndef JMICRO_MQTT_JM_BUFFER_H_
#define JMICRO_MQTT_JM_BUFFER_H_

#include "c_types.h"

#define MAX_SHORT_VALUE  32767

#define MAX_BYTE_VALUE  127

#define MAX_INT_VALUE  (1024*1024*10)  //10M

#define EXTRA_KEY_TYPE_BYTE 0
#define EXTRA_KEY_TYPE_STRING 1

//第5，6两位一起表示data字段的编码类型
#define FLAG_DATA_TYPE 5

#define FLAG_DATA_STRING 0
#define FLAG_DATA_BIN 1
#define FLAG_DATA_JSON 2
#define FLAG_DATA_EXTRA 3

#define BB_EMPTY 0x01 //锟秸伙拷锟斤拷
#define BB_FULL 0x02 //锟斤拷锟斤拷锟斤拷

#ifdef NET_DATA_LITTLE_END
#define NET_DATA_BIG_END false  //锟斤拷锟斤拷锟斤拷锟捷达拷锟�
#else
#define NET_DATA_BIG_END true //锟斤拷锟斤拷锟斤拷锟斤拷小锟斤拷
#endif

typedef enum _client_send_msg_result{
	JM_SUCCESS=-100,
	SOCKET_SENDER_NULL=-99,//底层SOCKET没建立
	ENCODE_MSG_FAIL=-98,//消息编码失败
	HANDLE_MSG_FAIL=-97,//消息处理器未找到
	MSG_CREATE_FAIL=-96,//消息创建失败
	MEMORY_OUTOF_RANGE=-95,//内存申请失败，也就是内存溢出
	MSG_WAIT_NOT_FOUND=-94,//没找到等待响应的消息
	SEND_DATA_ERROR=-93,//发送数据错误
	NO_DATA_TO_SEND=-92,//无数据可发送,
	INVALID_PS_DATA=-91, //PUBSUB数据无效

	SEND_QUEQUE_EXCEED, //发送队列已满

} client_send_msg_result_t;


typedef struct _jm_buffer
{
	 char *data;
	uint16_t capacity;
	//锟斤拷一锟斤拷锟街节存储锟斤拷前锟斤拷锟斤拷状态
	uint8_t status;
	uint16_t rpos ;

	uint16_t wpos;

	struct _jm_buffer *wrap_buf;
	BOOL rw_flag;// true:只锟斤拷锟斤拷false:只写

	uint16_t rmark ;//锟斤拷录锟斤拷前锟斤拷rpos位锟矫ｏ拷锟斤拷锟斤拷rmark_reset锟斤拷原
	uint8_t rmark_status;//锟斤拷录锟斤拷前锟斤拷status锟斤拷锟斤拷锟斤拷rmark_reset锟斤拷原

} byte_buffer_t;

/*
 * jm_buffer.c
 *
 *  Created on: 2023锟斤拷4锟斤拷9锟斤拷
 *      Author: yeyulei
 */
#include "c_types.h"
#include "jm_buffer.h"
#include "debug.h"

void ICACHE_FLASH_ATTR bb_print(byte_buffer_t *buf);

void ICACHE_FLASH_ATTR bb_clear(byte_buffer_t *buf);

void ICACHE_FLASH_ATTR bb_rmark(byte_buffer_t *buf);
BOOL ICACHE_FLASH_ATTR bb_rmark_reset(byte_buffer_t *buf);

BOOL ICACHE_FLASH_ATTR bb_reset(byte_buffer_t *buf);

uint16_t ICACHE_FLASH_ATTR bb_get_rpos(byte_buffer_t *buf);
uint16_t ICACHE_FLASH_ATTR bb_get_wpos(byte_buffer_t *buf);

ICACHE_FLASH_ATTR char* bb_readString(byte_buffer_t *buf,sint8_t *flag);

BOOL ICACHE_FLASH_ATTR bb_set_rpos(byte_buffer_t *buf, uint16_t rpos);
BOOL ICACHE_FLASH_ATTR bb_set_wpos(byte_buffer_t *buf, uint16_t wpos);
BOOL ICACHE_FLASH_ATTR bb_rmove_forward(byte_buffer_t * buf, uint16_t forwarnCnt);

byte_buffer_t * ICACHE_FLASH_ATTR bb_buffer_create(char *data, uint16_t cap);
byte_buffer_t * ICACHE_FLASH_ATTR bb_buffer_wrap(byte_buffer_t *src,  uint16_t cap, BOOL rw_flag);
byte_buffer_t* ICACHE_FLASH_ATTR bb_create(int capacity);
void ICACHE_FLASH_ATTR bb_release(byte_buffer_t * buf);

BOOL ICACHE_FLASH_ATTR bb_is_full(byte_buffer_t *buf);
BOOL ICACHE_FLASH_ATTR bb_is_empty(byte_buffer_t *buf);


//锟斤拷锟皆讹拷锟街斤拷锟斤拷
uint16_t ICACHE_FLASH_ATTR bb_readable_len(byte_buffer_t *buf);
//锟斤拷锟斤拷写锟街斤拷锟斤拷
uint16_t ICACHE_FLASH_ATTR bb_writeable_len(byte_buffer_t *buf);

BOOL ICACHE_FLASH_ATTR bb_get_u8(byte_buffer_t *buf,uint8_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s8(byte_buffer_t *buf, sint8_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_bool(byte_buffer_t *buf, BOOL *rst);
BOOL ICACHE_FLASH_ATTR bb_get_char(byte_buffer_t *buf,char *rst);

//锟斤拷指锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
byte_buffer_t* ICACHE_FLASH_ATTR bb_read_buf(byte_buffer_t *buf);
char* ICACHE_FLASH_ATTR bb_read_chars(byte_buffer_t *buf);
BOOL ICACHE_FLASH_ATTR bb_get_bytes(byte_buffer_t *buf, uint8_t *bytes, uint16_t len);
BOOL ICACHE_FLASH_ATTR bb_get_chars(byte_buffer_t *buf, char *chars, uint16_t len);
BOOL ICACHE_FLASH_ATTR bb_get_buf(byte_buffer_t *buf, byte_buffer_t *dest, uint16_t len);
ICACHE_FLASH_ATTR BOOL bb_writeString(byte_buffer_t *buf, char *str, uint16_t len);

//取锟斤拷锟斤拷锟斤拷指锟斤拷位锟矫碉拷一锟斤拷锟街节ｏ拷锟剿凤拷锟斤拷锟斤拷锟侥憋拷锟街革拷锟斤拷锟�
char ICACHE_FLASH_ATTR bb_get_by_index(byte_buffer_t *buf,  uint16_t index);
BOOL ICACHE_FLASH_ATTR bb_get_u16(byte_buffer_t *buf, uint16_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s16(byte_buffer_t *buf, sint16_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s32(byte_buffer_t *buf, sint32_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_u32(byte_buffer_t *buf,uint32 *rst);
BOOL ICACHE_FLASH_ATTR bb_get_u64(byte_buffer_t *buf, uint64_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s64(byte_buffer_t *buf, sint64_t *rst);
/*******************************Write method begin********************************************/

BOOL ICACHE_FLASH_ATTR check_write_len(byte_buffer_t *buf, uint16_t len);
BOOL ICACHE_FLASH_ATTR bb_put_u8(byte_buffer_t *buf, uint8_t x);
BOOL ICACHE_FLASH_ATTR bb_put_s8(byte_buffer_t *buf, sint8_t x);
BOOL ICACHE_FLASH_ATTR bb_put_bytes(byte_buffer_t *buf, uint8_t *bytes, uint16_t len);
BOOL ICACHE_FLASH_ATTR bb_put_chars(byte_buffer_t *buf, char *bytes, uint16_t len);

BOOL ICACHE_FLASH_ATTR bb_put_char(byte_buffer_t *buf, char x);
BOOL ICACHE_FLASH_ATTR bb_put_bool(byte_buffer_t *buf, BOOL x);
BOOL ICACHE_FLASH_ATTR bb_put_u16(byte_buffer_t *buf, uint16_t x);
BOOL ICACHE_FLASH_ATTR bb_put_s16(byte_buffer_t *buf, sint16_t x);
BOOL ICACHE_FLASH_ATTR bb_put_u32(byte_buffer_t *buf, uint32 x);
BOOL ICACHE_FLASH_ATTR bb_put_s32(byte_buffer_t *buf, sint32_t x);
BOOL ICACHE_FLASH_ATTR bb_put_u64(byte_buffer_t *buf, uint64_t x);
BOOL ICACHE_FLASH_ATTR bb_put_s64(byte_buffer_t *buf, sint64_t x);
BOOL ICACHE_FLASH_ATTR bb_put_buf(byte_buffer_t *buf, byte_buffer_t *src);

#endif /* JMICRO_MQTT_JM_BUFFER_H_ */
