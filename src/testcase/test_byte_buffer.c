#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "test.h"

#include "../jm_buffer.h"

void test_bytes(byte_buffer_t *buf) {

	printf("test bytes:\n");
	printf("Write: 1,2,3,4,5\n");

	uint8_t byte1[] = {1,2,3,4,5};

	assert(bb_put_bytes(buf,byte1,5));

	uint8_t u1[5], u2[5];
	bb_get_bytes(buf,u1,5);

	printf("%d,%d,%d,%d,%d,\n",u1[0],u1[1],u1[2],u1[3],u1[4]);
	//printf("%c\n",u2);
}


void test_char(byte_buffer_t *buf){

	printf("test char:\n");
	printf("Write: A a\n");

	assert(bb_put_char(buf,'A'));
	assert(bb_put_char(buf,'a'));

	char u1, u2;
	assert(bb_get_char(buf,&u1));
	assert(bb_get_char(buf,&u2));

	printf("Read: %c, %c\n\n",u1,u2);
}

void test_bool(byte_buffer_t *buf){

	printf("test bool:\n");
	printf("Write: true false\n");

	assert(bb_put_bool(buf,true));
	assert(bb_put_bool(buf,false));

	bool u1, u2;
	assert(bb_get_bool(buf,&u1));
	assert(bb_get_bool(buf,&u2));

	printf("Read: %d, %d\n\n",u1,u2);
}

void test_u64(byte_buffer_t *buf){

	printf("test unsigned long:\n");
	printf("Write: 99999999999 77777777777\n");

	assert(bb_put_u64(buf,99999999999));
	assert(bb_put_u64(buf,77777777777));

	uint64_t u1, u2;
	assert(bb_get_u64(buf,&u1));
	assert(bb_get_u64(buf,&u2));

	printf("Read: %llu, %llu\n\n",u1,u2);
}

void test_s64(byte_buffer_t *buf){

	printf("test signed long:\n");
	printf("Write: -2222 -3333\n");

	assert(bb_put_s64(buf,-2222));
	assert(bb_put_s64(buf,-3333));

	//printf("data: %d, %d\n\n",u1,u2);

	sint64_t u1, u2;
	assert(bb_get_s64(buf,&u1));
	assert(bb_get_s64(buf,&u2));

	printf("Read: %d",u1);
	printf(" %d \n\n",u2);
}


void test_u32(byte_buffer_t *buf){

	printf("test unsigned int:\n");
	printf("Write: 99999999 777777777\n");

	assert(bb_put_u32(buf,99999999));
	assert(bb_put_u32(buf,777777777));

	uint32_t u1, u2;
	assert(bb_get_u32(buf,&u1));
	assert(bb_get_u32(buf,&u2));

	printf("Read: %u, %u\n\n",u1,u2);
}

void test_s32(byte_buffer_t *buf){

	printf("test signed int:\n");
	printf("Write: -99999999 99999999\n");

	assert(bb_put_s32(buf,-99999999));
	assert(bb_put_s32(buf,99999999));

	sint32_t u1, u2;

	assert(bb_get_s32(buf,&u1));
	assert(bb_get_s32(buf,&u2));

	printf("Read: %d,%d\n\n",u1,u2);
}


void test_s16(byte_buffer_t *buf){

	printf("test signed short:-23 -32767 -3333\n");
	printf("write: -23 -32767 -3333\n");

	bb_put_s8(buf,-23);
	bb_put_s16(buf,-32767);
	bb_put_s16(buf,-3333);

	sint8_t b8;
	assert(bb_get_s8(buf,&b8));
	sint16_t u1, u2;
	if(!bb_get_s16(buf,&u1)) {
		printf("get uint16_t fail");
	}

	if(!bb_get_s16(buf,&u2)) {
		printf("get uint16_t fail");
	}

	printf("Read: %d,%d,%d\n\n",b8,u1,u2);
}


void test_u16(byte_buffer_t *buf){

	printf("test unsigned short:\n");
	printf("Write: 23 32767 65535\n");

	bb_put_u8(buf,23);
	bb_put_u16(buf,32767);
	bb_put_u16(buf,65535);

	uint8_t b8;
	assert(bb_get_u8(buf,&b8));
	uint16_t u1, u2;
	if(!bb_get_u16(buf,&u1)) {
		printf("get uint16_t fail");
	}

	if(!bb_get_u16(buf,&u2)) {
		printf("get uint16_t fail");
	}

	printf("Read: %d,%u,%u\n\n",b8,u1,u2);

}

void test_chars(byte_buffer_t *buf){

	char *chars = "Hello world";
	assert(bb_put_u16(buf,11));
    assert(bb_put_chars(buf,chars,11));

    char *chars1 = "zhongguonihao";
	assert(bb_put_u16(buf,13));
	assert(bb_put_chars(buf,chars1,13));

    uint16_t clen;
	assert(bb_get_u16(buf,&clen));
	char *rchars = (char*)os_zalloc(clen+1);
	assert(bb_get_chars(buf,rchars,clen));
	rchars[clen] = '\0';
	printf("chars: %s\n",rchars);

	assert(bb_get_u16(buf,&clen));
	char *rchars1 = (char*)os_zalloc(clen+1);
	assert(bb_get_chars(buf,rchars1,clen));
	rchars1[clen] = '\0';
	printf("chars: %s\n",rchars1);

}

void test_data_packge(byte_buffer_t *buf) {

	assert(bb_put_u8(buf,8));
	assert(bb_put_u16(buf,16));
	assert(bb_put_u32(buf,32));
	assert(bb_put_u64(buf,64));

	assert(bb_put_s8(buf,-8));
	assert(bb_put_s16(buf,-16));
	assert(bb_put_s32(buf,-32));
	assert(bb_put_s64(buf,-64));

	assert(bb_put_bool(buf,true));
	assert(bb_put_bool(buf,false));
	assert(bb_put_char(buf,'A'));
	assert(bb_put_char(buf,'a'));

	uint8_t arr[5] = {1,2,3,4,5};
	assert(bb_put_bytes(buf,arr,5));

	char *chars = "Hello world";
	assert(bb_put_u16(buf,11));
    assert(bb_put_chars(buf,chars,11));

	uint8_t u8;
	assert(bb_get_u8(buf,&u8));
	printf("u8: %d\n",u8);

	uint16_t u16;
	assert(bb_get_u16(buf,&u16));
	printf("u16: %u\n",u16);

	uint32_t u32;
	assert(bb_get_u32(buf,&u32));
	printf("u16: %d\n",u32);

	uint64_t u64;
	assert(bb_get_u64(buf,&u64));
	printf("u16: %u\n",u64);

	sint8_t s8;
	assert(bb_get_s8(buf,&s8));
	printf("s8: %d\n",s8);

	sint16_t s16;
	assert(bb_get_s16(buf,&s16));
	printf("s16: %d\n",s16);

	sint32_t s32;
	assert(bb_get_s32(buf,&s32));
	printf("s32: %d\n",s32);

	sint64_t s64;
	assert(bb_get_s64(buf,&s64));
	printf("s64: %d\n",s64);

	bool bt;
	assert(bb_get_bool(buf,&bt));
	printf("bool true: %d\n",bt);

	bool bf;
	assert(bb_get_bool(buf,&bf));
	printf("bool false: %d\n",bf);

	char ca;
	assert(bb_get_char(buf,&ca));
	printf("char A: %c\n",ca);

	char cb;
	assert(bb_get_char(buf,&cb));
	printf("char a: %c\n",cb);

	uint8_t rarr[5]={0};
	assert(bb_get_bytes(buf,rarr,5));
	printf("byte array %d,%d,%d,%d,%d,\n",rarr[0],rarr[1],rarr[2],rarr[3],rarr[4]);

	uint16_t clen;
	assert(bb_get_u16(buf,&clen));
	char *rchars = (char*)os_zalloc(clen+1);
	assert(bb_get_chars(buf,rchars,11));
	rchars[clen] = '\0';
	printf("chars: %s\n",rchars);

}

int test_byte_buffer() {
	byte_buffer_t *buf = bb_allocate(64);
	//test_data_packge(buf);
	test_chars(buf);
	/*
	test_u16(buf);
	test_s16(buf);
	test_s32(buf);
	test_u32(buf);

	test_s64(buf);
	test_u64(buf);

	test_bool(buf);
	test_char(buf);

	test_bytes(buf);
	*/

	return 0;
}
