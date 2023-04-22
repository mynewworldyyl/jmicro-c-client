/*
 * jm_buffer.c
 *
 *  Created on: 2023年4月9日
 *      Author: yeyulei
 */
#include "c_types.h"
#include "jm_buffer.h"
#include "debug.h"

#include "testcase/test.h"

void bb_print(byte_buffer_t *buf){
	if(!buf) {
		printf("No data buff\n");
		return;
	}
	int len = bb_readable_len(buf);

	char c;
	for(int i = 0; i < len; i++) {
		c = bb_get_by_index(buf,i);
		printf("%c",c);
	}
	printf("\n");

}

static BOOL ICACHE_FLASH_ATTR _bb_is_wrap(byte_buffer_t *buf) {
	return buf->wrap_buf != NULL;
}

static BOOL ICACHE_FLASH_ATTR bb_buffer_init0(byte_buffer_t *buf, char *data, uint16_t cap) {
	 buf->capacity = cap;

	 if(data) {
		 buf->data = data;
	 }

	 buf->status = BB_EMPTY; //初始状态是空
	 buf->rpos = 0;
	 buf->wpos = 0;
	 buf->wrap_buf  = NULL;
	 buf->rw_flag = true;

	 if(cap == 0) return true;//只申请一buffer,之后再单独初始化

	 if(!data)
	 {
		 buf->data = (char*)os_zalloc(cap);
	 }
	 return true;
}

byte_buffer_t* ICACHE_FLASH_ATTR bb_allocate(int capacity) {
	if (capacity < 0) {
		INFO("ERROR: bb_allocate capacity invalid: %d\r\n", capacity);
		return NULL;
	}
	byte_buffer_t * bb = (byte_buffer_t*)os_zalloc(sizeof(struct _jm_buffer));
	bb_buffer_init0(bb,NULL,capacity);
	return bb;
}

byte_buffer_t* ICACHE_FLASH_ATTR bb_buffer_create(char *data, uint16_t cap) {
	byte_buffer_t * bb = os_zalloc(sizeof(byte_buffer_t));
	if(bb == NULL) return NULL;
	bb_buffer_init0(bb, data, cap);
	return bb;
}

byte_buffer_t* ICACHE_FLASH_ATTR bb_buffer_wrap(byte_buffer_t *src,  uint16_t cap, BOOL rw_flag) {

	uint16_t slen;
	if(rw_flag) {
		//只读
		slen = bb_readable_len(src);
	} else {
		//只写
		slen = bb_writeable_len(src);
	}

	if(slen <= 0 || slen < cap) return NULL;//无足够可读数据或可写空间

	byte_buffer_t *dest = bb_allocate(0);
	dest->wrap_buf = src;
	dest->rw_flag = rw_flag;
	dest->capacity = cap;
	//源BUF可读数据大于需要数据
	dest->rpos = 0;
	dest->wpos = 0;
	dest->data = 0;
	dest->status = 0;
	return dest;
}

BOOL ICACHE_FLASH_ATTR bb_reset(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) return false;//对wrap类Buf无效
	 buf->status = BB_EMPTY; //初始状态是空
	 buf->rpos = 0;
	 buf->wpos = 0;
	 return true;
}

//记录当前读位置
void ICACHE_FLASH_ATTR bb_rmark(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) {
		 bb_rmark(buf->wrap_buf);
	} else {
		buf->rmark = buf->rpos;
		buf->rmark_status = buf->status;
	}
}

BOOL ICACHE_FLASH_ATTR bb_rmark_reset(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) {
		return bb_rmark_reset(buf->wrap_buf);
	} else {
		if(buf->rmark == -1) return false;

		buf->rpos = buf->rmark;
		buf->status = buf->rmark_status;

		buf->rmark = -1;
		buf->rmark_status = 0;

		return true;
	}

}

BOOL ICACHE_FLASH_ATTR bb_set_rpos(byte_buffer_t *buf, uint16_t rpos) {
	if(_bb_is_wrap(buf)) return false;//对wrap类Buf无效
	buf->rpos = rpos;
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_set_wpos(byte_buffer_t *buf, uint16_t wpos) {
	if(_bb_is_wrap(buf)) return false;//对wrap类Buf无效
	buf->wpos = wpos;
	return true;
}

//记录当前读位置
uint16_t ICACHE_FLASH_ATTR bb_get_rpos(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) return buf->wrap_buf->rpos;
	else return buf->rpos;
}

uint16_t ICACHE_FLASH_ATTR bb_get_wpos(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) return buf->wrap_buf->wpos;
	else return buf->wpos;
}

static void ICACHE_FLASH_ATTR _bb_set_full(byte_buffer_t *buf, BOOL v) {
	if(_bb_is_wrap(buf)) return;
	if(v) {
		buf->status |= BB_FULL;
	} else {
		buf->status &= ~BB_FULL;
	}
}

static void ICACHE_FLASH_ATTR _bb_set_empty(byte_buffer_t *buf, BOOL v) {
	if(_bb_is_wrap(buf)) return;
	if(v) {
		buf->status |= BB_EMPTY;
	} else {
		buf->status &= ~BB_EMPTY;
	}
}

BOOL ICACHE_FLASH_ATTR bb_is_full(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只读
			return buf->capacity == bb_readable_len(buf->wrap_buf);
		} else {
			//只写
			return buf->capacity == bb_writeable_len(buf->wrap_buf);
		}
	}
	return  (buf->rpos == buf->wpos) && (buf->status & BB_FULL);
}

BOOL ICACHE_FLASH_ATTR bb_is_empty(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只读
			return 0 == bb_readable_len(buf->wrap_buf);
		} else {
			//只写
			return 0 == bb_writeable_len(buf->wrap_buf);
		}
	}
	return (buf->rpos == buf->wpos) && (buf->status & BB_EMPTY);
}

static uint8_t ICACHE_FLASH_ATTR _bb_get_u8(byte_buffer_t *buf) {

	if(_bb_is_wrap(buf)) {
		buf->rpos++;//记录当前封装已读数节数
		return _bb_get_u8(buf->wrap_buf);
	}

	if(bb_is_full(buf)) {
		_bb_set_full(buf,false);
	}

	uint8_t v = buf->data[buf->rpos];
	buf->rpos = (buf->rpos + 1) % buf->capacity;

	if(buf->rpos == buf->wpos) {
		//如果读完了，设置缓存为空状态
		_bb_set_empty(buf,true);
	}
	return v;
}

//可以读字节数
uint16_t ICACHE_FLASH_ATTR bb_readable_len(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			return buf->capacity - buf->rpos;
		}else {
			//只写缓存不能写
			return 0;
		}
	}

	if(buf->wpos == buf->rpos ) {
		if(bb_is_full(buf)) {
			return buf->capacity;
		}else {
			return 0;
		}
	}else if(buf->wpos > buf->rpos ) {
		return buf->wpos - buf->rpos;
	}else {
		return buf->capacity - (buf->rpos - buf->wpos);
	}
}

//可以写字节数
uint16_t ICACHE_FLASH_ATTR bb_writeable_len(byte_buffer_t *buf) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只读缓存不能写
			return 0;
		}else {
			return buf->capacity - buf->wpos;;
		}
	}

	if(buf->wpos == buf->rpos ) {
		if(bb_is_empty(buf)) {
			return buf->capacity;
		}else {
			return 0;
		}
	}else if(buf->wpos > buf->rpos ) {
		return buf->capacity - (buf->wpos - buf->rpos) ;
	}else {
		return buf->rpos - buf->wpos;
	}
}

void ICACHE_FLASH_ATTR bb_free(byte_buffer_t * buf) {
	if (buf == NULL) {
		return;
	}

	if(!_bb_is_wrap(buf)) {
		if(buf->data) {
			os_free(buf->data);
			buf->data = NULL;
		}
	}

	//wrap_buf 由创建者释放

	os_free(buf);
}

BOOL ICACHE_FLASH_ATTR bb_rmove_forward(byte_buffer_t *buf, uint16_t forwarnCnt) {

	if(bb_is_full(buf)) {
		_bb_set_full(buf,false);
	}

	buf->rpos = (buf->rpos+forwarnCnt) % buf->capacity;

	if(buf->rpos == buf->wpos) {
		//如果读完了，设置缓存为空状态
		_bb_set_empty(buf,true);
	}

	return true;
}



static BOOL ICACHE_FLASH_ATTR bb_check_read_len(byte_buffer_t *buf, uint16_t len) {
	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			//只读缓存不能写
			return bb_readable_len(buf) >= len;
		}else {
			//只写缓存
			return false;
		}
	}

	if(bb_is_empty(buf)) {
		INFO("ERROR: bb_is_empty NULL buffer rpos: %d, wpos:%d\r\n", buf->rpos, buf->wpos);
		return false;
	}

	if(bb_readable_len(buf) < len) {
		INFO("ERROR: bb_readable_len readable_len: %d, read len: %d\r\n", bb_readable_len(buf),len);
		return false;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_u8(byte_buffer_t *buf, uint8_t *rst) {
	if(!bb_check_read_len(buf,1)) {
		INFO("ERROR: bb_get_u8 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}
	*rst = _bb_get_u8(buf);
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_s8(byte_buffer_t *buf, sint8_t *rst) {
	if(!bb_check_read_len(buf,1)) {
		INFO("ERROR: bb_get_s8 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}
	*rst = (sint8_t)_bb_get_u8(buf);
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_bool(byte_buffer_t *buf, BOOL *rst) {
	if(!bb_check_read_len(buf,1)) {
		INFO("ERROR: bb_get_bool NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}
	*rst = _bb_get_u8(buf) == 0 ? false : true;
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_char(byte_buffer_t *buf, char *rst) {
	if(!bb_check_read_len(buf,1)) {
		INFO("ERROR: bb_get_char NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	*rst = _bb_get_u8(buf);
	return true;
}

//读指定长度数据
BOOL ICACHE_FLASH_ATTR bb_get_bytes(byte_buffer_t *buf, uint8_t *dest, uint16_t len) {
	if(!bb_check_read_len(buf,len)) {
		INFO("ERROR: bb_get_bytes buffer rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	for(;len > 0; len--) {
		*dest = _bb_get_u8(buf);
		dest++;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_chars(byte_buffer_t *buf, char *chars, uint16_t len) {
	if(!bb_check_read_len(buf,len)) {
		INFO("ERROR: bb_put_bytes buffer rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	for(;len > 0; len--) {
		bb_get_char(buf,chars);
		chars++;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_buf(byte_buffer_t *buf, byte_buffer_t *dest, uint16_t len){
	if(!bb_check_read_len(buf,len)) {
		INFO("ERROR: bb_get_buf buffer rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	uint8_t p = 0;
	for(;len > 0; len--) {
		if(!bb_get_u8(buf,&p)){
			return false;
		}
		bb_put_u8(dest,p);

	}
	return true;
}

byte_buffer_t* ICACHE_FLASH_ATTR bb_read_buf(byte_buffer_t *buf){

	uint16_t len;
	if(!bb_get_s16(buf,&len)) {
		INFO("ERROR: bb_read_buf read len fail\r\n", buf->rpos,len);
		return false;
	}

	if(!bb_check_read_len(buf,len)) {
		INFO("ERROR: bb_read_buf buffer rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	if(len > 0){
		byte_buffer_t *pl = bb_allocate(len);
		if(!bb_get_buf(buf,pl,len)) {
			INFO("ERROR:bb_read_buf read buffer fail\r\n");
			os_free(pl);
			return NULL;
		}
		return pl;
	}
	return NULL;
}

char* ICACHE_FLASH_ATTR bb_read_chars(byte_buffer_t *buf){

	uint16_t len;
	if(!bb_get_s16(buf,&len)) {
		INFO("ERROR: bb_read_chars read len fail\r\n", buf->rpos,len);
		return false;
	}

	if(!bb_check_read_len(buf,len)) {
		INFO("ERROR: bb_read_chars chars rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	if(len > 0){
		char *cs = bb_allocate(len);
		if(cs == NULL) return NULL;
		if(!bb_get_chars(buf,cs,len)) {
			INFO("ERROR:bb_read_chars read chars fail\r\n");
			os_free(cs);
			return NULL;
		}
		return cs;
	}
	return NULL;
}


//取得数组指定位置的一个字节，此方法不改变读指针向
char ICACHE_FLASH_ATTR bb_get_by_index(byte_buffer_t *buf,  uint16_t index) {
	if(_bb_is_wrap(buf)) {
		return bb_get_by_index(buf->wrap_buf,index);
	}
	uint16_t idx = (index+buf->rpos)%buf->capacity;
	return buf->data[idx];
}

BOOL ICACHE_FLASH_ATTR bb_get_u16(byte_buffer_t *buf, uint16_t *rst) {
	if(!bb_check_read_len(buf,2)) {
		INFO("ERROR: bb_get_u16 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	uint16_t first = _bb_get_u8(buf);
	uint16_t second = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//大端
		*rst = first<<8 | second;
	} else {
		//小端
		*rst = second<<8 | first;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_s16(byte_buffer_t *buf, sint16_t *rst) {
	if(!bb_check_read_len(buf,2)) {
		INFO("ERROR: bb_get_s16 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	sint16_t first = _bb_get_u8(buf);
	sint16_t second = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//大端
		*rst = (sint16_t)(first<<8 | second);
	} else {
		//小端
		*rst = (sint16_t)(first | second<<8) ;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_s32(byte_buffer_t *buf, sint32_t *rst) {
	if(!bb_check_read_len(buf,4)) {
		INFO("ERROR: bb_get_s32 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	sint32_t first = _bb_get_u8(buf);
	sint32_t second = _bb_get_u8(buf);
	sint32_t third = _bb_get_u8(buf);
	sint32_t forth = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//大端
		*rst = (sint32_t)(first <<24 | second<<16 | third<<8 | forth<<0) ;
	} else {
		//小端
		*rst = (sint32_t)(first <<0 | second<<8 | third<<16 | forth<<24) ;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_u32(byte_buffer_t *buf,uint32_t *rst) {
	if(!bb_check_read_len(buf,4)) {
		INFO("ERROR: bb_get_u32 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	uint32_t first = _bb_get_u8(buf);
	uint32_t second = _bb_get_u8(buf);
	uint32_t third = _bb_get_u8(buf);
	uint32_t forth = _bb_get_u8(buf);


	if(NET_DATA_BIG_END) {
		//大端
		*rst = first <<24 | second<<16 | third<<8 | forth<<0 ;
	} else {
		//小端
		*rst =  first <<0 | second<<8 | third<<16 | forth<<24 ;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_u64(byte_buffer_t *buf, uint64_t *rst) {
	if(!bb_check_read_len(buf,8)) {
		INFO("ERROR: bb_get_u64 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	uint64_t first = _bb_get_u8(buf);
	uint64_t second = _bb_get_u8(buf);
	uint64_t third = _bb_get_u8(buf);
	uint64_t forth = _bb_get_u8(buf);

	uint64_t five = _bb_get_u8(buf);
	uint64_t six = _bb_get_u8(buf);
	uint64_t seven = _bb_get_u8(buf);
	uint64_t eight = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//大端
		*rst = first <<56 | second<<48 | third<<40 | forth<<32 | five <<24 | six<<16 | seven<<8 | eight<<0;
	} else {
		//小端
		*rst = first <<0 | second<<8 | third<<16 | forth<<24 | five <<32 | six<<40 | seven<<48 | eight<<56;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_get_s64(byte_buffer_t *buf, sint64_t *rst) {
	if(!bb_check_read_len(buf,8)) {
		INFO("ERROR: bb_get_s64 NULL buffer rpos: %d\r\n", buf->rpos);
		return false;
	}

	uint64_t first = _bb_get_u8(buf);
	uint64_t second = _bb_get_u8(buf);
	uint64_t third = _bb_get_u8(buf);
	uint64_t forth = _bb_get_u8(buf);

	uint64_t five = _bb_get_u8(buf);
	uint64_t six = _bb_get_u8(buf);
	uint64_t seven = _bb_get_u8(buf);
	uint64_t eight = _bb_get_u8(buf);

	if(NET_DATA_BIG_END) {
		//大端
		*rst = (sint64_t)(first <<56 | second<<48 | third<<40 | forth<<32 | five <<24 | six<<16 | seven<<8 | eight<<0);
	} else {
		//小端
		*rst = (sint64_t)(first <<0 | second<<8 | third<<16 | forth<<24 | five <<32 | six<<40 | seven<<48 | eight<<56);
	}
	return true;
}

/*******************************Write method begin********************************************/

static BOOL ICACHE_FLASH_ATTR bb_check_write_len(byte_buffer_t *buf, uint16_t len) {
	if(bb_is_full(buf)) {
		INFO("ERROR: bb_check_write_len FULL buffer rpos: %d, wpos:%d\r\n", buf->rpos, buf->wpos);
		return false;
	}

	if(bb_writeable_len(buf) < len) {
		INFO("ERROR: bb_check_write_len bb_writeable_len: %d, write len: %d\r\n", bb_writeable_len(buf),len);
		return false;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_u8(byte_buffer_t *buf, uint8_t x) {
	if(bb_check_write_len(buf,1) == false) {
		INFO("ERROR: bb_put_u8 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			return false;
		}else {
			if(bb_put_u8(buf->wrap_buf, x)) {
				buf->wpos++;
			}
		}
	}

    if(bb_is_empty(buf)) {
		_bb_set_empty(buf,false);
	}

    buf->data[buf->wpos] = x;
    buf->wpos = (buf->wpos + 1) % buf->capacity;

    if(buf->wpos == buf->rpos) {
    	_bb_set_full(buf,true);
    }
    return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_s8(byte_buffer_t *buf, sint8_t x) {
	if(bb_check_write_len(buf,1) == false) {
		INFO("ERROR: bb_put_s8 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(_bb_is_wrap(buf)) {
		if(buf->rw_flag) {
			return false;
		} else {
			if(bb_put_s8(buf->wrap_buf, x)) {
				buf->wpos++;
			}
		}
	}

    if(bb_is_empty(buf)) {
		_bb_set_empty(buf,false);
	}

    buf->data[buf->wpos] = x;
    buf->wpos = (buf->wpos + 1) % buf->capacity;

    if(buf->wpos == buf->rpos) {
    	_bb_set_full(buf,true);
    }
    return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_bytes(byte_buffer_t *buf, uint8_t *bytes, uint16_t len) {
	if(bb_check_write_len(buf,len) == false) {
		INFO("ERROR: bb_put_bytes buffer rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	for(;len > 0; len--) {
		bb_put_u8(buf,*bytes);
		bytes++;
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_buf(byte_buffer_t *buf, byte_buffer_t *src) {
	uint16_t len = bb_readable_len(src);
	if(bb_check_write_len(buf,len) == false) {
		INFO("ERROR: bb_put_buf buffer writeable len: %d, need len: %d\r\n", bb_writeable_len(buf),len);
		return false;
	}

	while(!bb_is_empty(src)) {
		bb_put_u8(buf, _bb_get_u8(src));
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_chars(byte_buffer_t *buf, char *chars, uint16_t len){
	if(len < 0 || bb_check_write_len(buf,len) == false) {
		INFO("ERROR: bb_put_chars buffer rpos: %d, need len: %d\r\n", buf->rpos,len);
		return false;
	}

	//允许空字符串
	if(len > 0) {
		for(;len > 0; len--) {
			bb_put_s8(buf,*chars);
			chars++;
		}
	}

	return true;
}


BOOL ICACHE_FLASH_ATTR bb_put_char(byte_buffer_t *buf, char x) {
	return bb_put_u8(buf,x);
}

BOOL ICACHE_FLASH_ATTR bb_put_bool(byte_buffer_t *buf, BOOL x) {
	return bb_put_u8(buf,x);
}

BOOL ICACHE_FLASH_ATTR bb_put_u16(byte_buffer_t *buf, uint16_t x) {
	if(!bb_check_write_len(buf,2)) {
		INFO("ERROR: bb_put_u16 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//大端
		bb_put_u8(buf,(uint8_t)(x>>8));
		bb_put_u8(buf,(uint8_t)(x));
	} else {
		//小端
		bb_put_u8(buf,(uint8_t)(x));
		bb_put_u8(buf,(uint8_t)(x>>8));
	}

	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_s16(byte_buffer_t *buf, sint16_t x) {
	if(!bb_check_write_len(buf,2)) {
		INFO("ERROR: bb_put_s16 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//大端
		bb_put_u8(buf,(sint8_t)(x>>8));
		bb_put_u8(buf,(uint8_t)(x));
	} else {
		//小端
		bb_put_u8(buf,(uint8_t)(x));
		bb_put_u8(buf,(sint8_t)(x>>8));
	}

	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_u32(byte_buffer_t *buf, uint32_t x) {
	if(!bb_check_write_len(buf,4)) {
		INFO("ERROR: bb_put_u32 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//大端
		bb_put_u8(buf,(uint8_t)(x>>24));
		bb_put_u8(buf,(uint8_t)(x>>16));
		bb_put_u8(buf,(uint8_t)(x>>8));
		bb_put_u8(buf,(uint8_t)(x));
	} else {
		//小端
		bb_put_u8(buf,(uint8_t)(x));
		bb_put_u8(buf,(uint8_t)(x>>8));
		bb_put_u8(buf,(uint8_t)(x>>16));
		bb_put_u8(buf,(uint8_t)(x>>24));
	}

	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_s32(byte_buffer_t *buf, sint32_t x) {
	if(!bb_check_write_len(buf,4)) {
		INFO("ERROR: bb_put_s32 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//大端
		bb_put_u8(buf,x>>24 & 0xFF);
		bb_put_u8(buf,x>>16 & 0xFF);
		bb_put_u8(buf,x>>8 & 0xFF);
		bb_put_u8(buf,x & 0xFF);
	} else {
		//小端
		bb_put_u8(buf,x & 0xFF);
		bb_put_u8(buf,x>>8 & 0xFF);
		bb_put_u8(buf,x>>16 & 0xFF);
		bb_put_u8(buf,x>>24 & 0xFF);
	}

	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_u64(byte_buffer_t *buf, uint64_t x) {
	if(!bb_check_write_len(buf,8)) {
		INFO("ERROR: bb_put_u64 exceed wpos: %d, rpos:%d, cap:%d\r\n",buf->wpos,buf->rpos,buf->capacity);
		return false;
	}

	if(NET_DATA_BIG_END) {
		//大端
		bb_put_u8(buf,(uint8_t)(x>>56 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x>>48 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x>>40 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x>>32 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x>>24 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x>>16 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x>>8 & 0xFF));
		bb_put_u8(buf,(uint8_t)(x & 0xFF));
	} else {
		//小端
		bb_put_u8(buf,x & 0xFF);
		bb_put_u8(buf,x>>8 & 0xFF);
		bb_put_u8(buf,x>>16 & 0xFF);
		bb_put_u8(buf,x>>24 & 0xFF);
		bb_put_u8(buf,x>>32 & 0xFF);
		bb_put_u8(buf,x>>40 & 0xFF);
		bb_put_u8(buf,x>>48 & 0xFF);
		bb_put_u8(buf,x>>56 & 0xFF);
	}
	return true;
}

BOOL ICACHE_FLASH_ATTR bb_put_s64(byte_buffer_t *buf, sint64_t x) {
	return bb_put_u64(buf,x);
}

