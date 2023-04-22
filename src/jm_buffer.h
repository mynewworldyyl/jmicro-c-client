/*
 * jm_buffer.h
 *
 *  Created on: 2023��4��9��
 *      Author: yeyulei
 */


#ifndef JMICRO_MQTT_JM_BUFFER_H_
#define JMICRO_MQTT_JM_BUFFER_H_

#include "c_types.h"

#ifdef __cplusplus
extern "C"
{
#endif



#define BB_EMPTY 0x01 //�ջ���
#define BB_FULL 0x02 //������

#ifdef NET_DATA_LITTLE_END
#define NET_DATA_BIG_END false  //�������ݴ��
#else
#define NET_DATA_BIG_END true //��������С��
#endif


typedef struct _jm_buffer
{
	volatile char *data;
	uint16_t capacity;
	//��һ���ֽڴ洢��ǰ����״̬
	volatile uint8_t status;
	volatile uint16_t rpos ;

	volatile uint16_t wpos;

	struct _jm_buffer *wrap_buf;
	BOOL rw_flag;// true:ֻ����false:ֻд

	uint16_t rmark ;//��¼��ǰ��rposλ�ã�����rmark_reset��ԭ
	uint8_t rmark_status;//��¼��ǰ��status������rmark_reset��ԭ

} byte_buffer_t;

/*
 * jm_buffer.c
 *
 *  Created on: 2023��4��9��
 *      Author: yeyulei
 */
#include "c_types.h"
#include "jm_buffer.h"
#include "debug.h"

void bb_print(byte_buffer_t *buf);

void ICACHE_FLASH_ATTR bb_clear(byte_buffer_t *buf);

void ICACHE_FLASH_ATTR bb_rmark(byte_buffer_t *buf);
BOOL ICACHE_FLASH_ATTR bb_rmark_reset(byte_buffer_t *buf);

BOOL ICACHE_FLASH_ATTR bb_reset(byte_buffer_t *buf);

uint16_t ICACHE_FLASH_ATTR bb_get_rpos(byte_buffer_t *buf);
uint16_t ICACHE_FLASH_ATTR bb_get_wpos(byte_buffer_t *buf);

BOOL ICACHE_FLASH_ATTR bb_set_rpos(byte_buffer_t *buf, uint16_t rpos);
BOOL ICACHE_FLASH_ATTR bb_set_wpos(byte_buffer_t *buf, uint16_t wpos);
BOOL ICACHE_FLASH_ATTR bb_rmove_forward(byte_buffer_t * buf, uint16_t forwarnCnt);

byte_buffer_t * ICACHE_FLASH_ATTR bb_buffer_create(char *data, uint16_t cap);
byte_buffer_t * ICACHE_FLASH_ATTR bb_buffer_wrap(byte_buffer_t *src,  uint16_t cap, BOOL rw_flag);
byte_buffer_t* ICACHE_FLASH_ATTR bb_allocate(int capacity);
void ICACHE_FLASH_ATTR bb_free(byte_buffer_t * buf);

BOOL ICACHE_FLASH_ATTR bb_is_full(byte_buffer_t *buf);
BOOL ICACHE_FLASH_ATTR bb_is_empty(byte_buffer_t *buf);


//���Զ��ֽ���
uint16_t ICACHE_FLASH_ATTR bb_readable_len(byte_buffer_t *buf);
//����д�ֽ���
uint16_t ICACHE_FLASH_ATTR bb_writeable_len(byte_buffer_t *buf);

BOOL ICACHE_FLASH_ATTR bb_get_u8(byte_buffer_t *buf,uint8_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s8(byte_buffer_t *buf, sint8_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_bool(byte_buffer_t *buf, BOOL *rst);
BOOL ICACHE_FLASH_ATTR bb_get_char(byte_buffer_t *buf,char *rst);

//��ָ����������
byte_buffer_t* ICACHE_FLASH_ATTR bb_read_buf(byte_buffer_t *buf);
char* ICACHE_FLASH_ATTR bb_read_chars(byte_buffer_t *buf);
BOOL ICACHE_FLASH_ATTR bb_get_bytes(byte_buffer_t *buf, uint8_t *bytes, uint16_t len);
BOOL ICACHE_FLASH_ATTR bb_get_chars(byte_buffer_t *buf, char *chars, uint16_t len);
BOOL ICACHE_FLASH_ATTR bb_get_buf(byte_buffer_t *buf, byte_buffer_t *dest, uint16_t len);

//ȡ������ָ��λ�õ�һ���ֽڣ��˷������ı��ָ����
char ICACHE_FLASH_ATTR bb_get_by_index(byte_buffer_t *buf,  uint16_t index);
BOOL ICACHE_FLASH_ATTR bb_get_u16(byte_buffer_t *buf, uint16_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s16(byte_buffer_t *buf, sint16_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_s32(byte_buffer_t *buf, sint32_t *rst);
BOOL ICACHE_FLASH_ATTR bb_get_u32(byte_buffer_t *buf,uint32_t *rst);
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
BOOL ICACHE_FLASH_ATTR bb_put_u32(byte_buffer_t *buf, uint32_t x);
BOOL ICACHE_FLASH_ATTR bb_put_s32(byte_buffer_t *buf, sint32_t x);
BOOL ICACHE_FLASH_ATTR bb_put_u64(byte_buffer_t *buf, uint64_t x);
BOOL ICACHE_FLASH_ATTR bb_put_s64(byte_buffer_t *buf, sint64_t x);
BOOL ICACHE_FLASH_ATTR bb_put_buf(byte_buffer_t *buf, byte_buffer_t *src);
#ifdef __cplusplus
}
#endif
#endif /* JMICRO_MQTT_JM_BUFFER_H_ */
