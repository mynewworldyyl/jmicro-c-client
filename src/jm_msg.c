/*
 * jm_msg.h
 *
 *  Created on: 2023锟斤拷4锟斤拷10锟斤拷
 *      Author: yeyulei
 */

#ifndef JMICRO_MQTT_JM_MSG_H_
#define JMICRO_MQTT_JM_MSG_H_

#include "jm_msg.h"

#ifndef WIN32
#include <osapi.h>
#include "mem.h"
#endif

#ifdef WIN32
#include "stdio.h"
#include "string.h"
#include "./testcase/test.h"
#endif

#include <stddef.h>
#include "debug.h"
#include "jm_constants.h"
#include "jm_mem.h"

static sint64_t msgId = 0;

/********************EXTRA DATA OPERATION BEGIN**********************/

ICACHE_FLASH_ATTR void extra_release(msg_extra_data_t *extra) {
	if(!extra) return;

	msg_extra_data_t *em = extra;

	while(em) {
		if((PREFIX_TYPE_LIST == em->type || PREFIX_TYPE_STRINGG == em->type)
				&& em->value.bytesVal && em->neddFreeBytes) {
			os_free(em->value.bytesVal);
		}

		msg_extra_data_t *n = em->next;
		em->next = NULL;

		cache_back(CACHE_PUBSUB_ITEM_EXTRA,em);

		em = n;
		if(em == NULL) break;
	}
}

ICACHE_FLASH_ATTR msg_extra_data_t *extra_create() {
	return cache_get(CACHE_PUBSUB_ITEM_EXTRA,true);
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_get(msg_extra_data_t *header, sint8_t key) {
	msg_extra_data_t *em = header;
	while(em != NULL) {
		if(em->key == key) {
			return em;
		}
		em = em->next;
	}
	return NULL;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_put(msg_extra_data_t *header, sint8_t key, sint8_t type) {
	msg_extra_data_t *eem = extra_get(header, key);
	if(eem != NULL) {
		eem->key = key;
		eem->type = type;
		return eem;
	}

	msg_extra_data_t *em = extra_create();
	if(em==NULL) return NULL;

	em->key = key;
	//em->value = val;
	em->type = type;
	em->len = 0;

	if(header == NULL) {
		em->next = NULL;
	} else {
		//头锟斤拷锟斤拷锟斤拷
		em->next = header;
		//msg->extraMap = em;
		//msg->extraMap->next = NULL;
	}
	return em;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_strKeyGet(msg_extra_data_t *header, char *key) {
	msg_extra_data_t *em = header;
	while(em != NULL) {
		if(em->strKey == key) {
			//常量池字符串内存地址肯定相等
			return em;
		}else if(os_strcmp(key,em->strKey)==0) {
			//非常量，如malloc申请的内存字符串
			return em;
		}
		em = em->next;
	}
	return NULL;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_strKeyPut(msg_extra_data_t *header, char *strKey, sint8_t type) {
	msg_extra_data_t *eem = extra_strKeyGet(header, strKey);
	if(eem != NULL) {
		eem->strKey = strKey;
		eem->type = type;
		return eem;
	}

	msg_extra_data_t *em = extra_create();
	if(em==NULL) return NULL;

	em->strKey = strKey;
	//em->value = val;
	em->type = type;
	em->len = 0;

	if(header == NULL) {
		em->next = NULL;
	} else {
		//头锟斤拷锟斤拷锟斤拷
		em->next = header;
		//msg->extraMap = em;
		//msg->extraMap->next = NULL;
	}
	return em;
}

ICACHE_FLASH_ATTR static msg_extra_data_t * extra_decodeVal(byte_buffer_t *b) {

	sint8_t type;
	if(!bb_get_s8(b,&type)){
		goto error;
	}

	msg_extra_data_t *rst = extra_create();
	//void *val = NULL;
	sint16_t len = 0;

	if(type == PREFIX_TYPE_NULL) {
		rst->value.bytesVal = NULL;
	}else if(PREFIX_TYPE_LIST == type){
		//锟街斤拷锟斤拷锟斤拷
		if(!bb_get_s16(b,&len)) {
			goto error;
		}

		if(len == 0) {
			rst->value.bytesVal = NULL;
		} else {
			uint8_t *arr = (uint8_t*)os_zalloc(len);
			//arr[0] = len;//锟斤拷锟介长锟斤拷
			if(!bb_get_bytes(b,arr,len)) {
				//锟斤拷锟斤拷锟斤拷失锟斤拷
				if(arr) os_free(arr);
				goto error;
			}
			rst->neddFreeBytes = true;
			rst->value.bytesVal = arr;
		}
	}else if(type == PREFIX_TYPE_INT){
		sint32_t v;
		if(!bb_get_s32(b,&v)) {
			goto error;
		} else {
			rst->value.s32Val = v;
		}
	}else if(PREFIX_TYPE_BYTE == type){
		sint8_t v;
		if(!bb_get_s8(b,&v)) {
			goto error;
		} else {
			rst->value.s8Val = v;
		}
	}else if(PREFIX_TYPE_SHORTT == type){
		sint16_t v;
		if(!bb_get_s16(b,&v)) {
			goto error;
		} else {
			rst->value.s16Val = v;
		}
	}else if(PREFIX_TYPE_LONG == type){
		sint64_t v;
		if(!bb_get_s64(b,&v)) {
			goto error;
		} else {
			rst->value.s64Val = v;
		}
	}else if(PREFIX_TYPE_FLOAT == type){
		/*sint32_t v;
		if(!bb_get_s32(b,&v)) {
			return NULL;
		} else {
			sint32_t* vptr = (sint32_t*)os_zalloc(sizeof(sint32_t));
			if(vptr == NULL) {
				return NULL;
			}
			*vptr = v;
			val = vptr;
		}*/
	}else if(PREFIX_TYPE_DOUBLE == type){
		/*sint32_t v;
		if(!bb_get_s32(b,&v)) {
			return NULL;
		} else {
			sint32_t* vptr = (sint32_t*)os_zalloc(sizeof(sint32_t));
			if(vptr == NULL) {
				return NULL;
			}
			*vptr = v;
			val = vptr;
		}*/
	}else if(PREFIX_TYPE_BOOLEAN == type){
		BOOL v;
		if(!bb_get_bool(b,&v)) {
			goto error;
		} else {
			rst->value.boolVal = v;
		}
	}else if(PREFIX_TYPE_CHAR == type){
		char v;
		if(!bb_get_char(b,&v)) {
			goto error;
		} else {
			rst->value.charVal = v;
		}
	}else if(PREFIX_TYPE_STRINGG == type){

		sint8_t slen;
		if(!bb_get_s8(b,&slen)) {
			goto error;
		}
		len = slen;

		rst->neddFreeBytes = true;

		if(len == -1) {
			//锟秸达拷
			rst->value.bytesVal = NULL;
		}else if(len == 0) {
			char* vptr = (char*)os_zalloc(sizeof(char));
			*vptr = "";
			rst->value.bytesVal = vptr;
		}else {

			sint32_t ilen = len;

			if(len == 127) {
				//一锟斤拷锟街斤拷锟斤拷锟街碉拷锟斤拷蟹锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟絁ava
				sint16_t slen;
				if(!bb_get_s16(b,&slen)) {
					goto error;
				}
				ilen = slen;

				if(slen == 32767) {
					//锟斤拷锟斤拷锟街斤拷锟斤拷锟街碉拷锟斤拷蟹锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟絁ava
					if(!bb_get_s32(b,&ilen)) {
						goto error;
					}
				}
			}

			uint8_t* vptr = (uint8_t*)os_zalloc(ilen + 1);
			if(vptr == NULL) {
				goto error;
			}

			if(!bb_get_bytes(b,vptr,(uint16_t)ilen)) {
				goto error;
			}

			vptr[ilen] = '\0';//锟街凤拷锟斤拷锟斤拷锟斤拷锟斤拷志
			rst->value.bytesVal = vptr;

		}
	}


	//rst->key = k;
	rst->type = type;
	//rst->value = val;
	rst->next = NULL;
	rst->len = len;

	return rst;

	error:
		if(rst) os_free(rst);
		return NULL;

}

ICACHE_FLASH_ATTR static BOOL extra_encodeVal(msg_extra_data_t *extras, byte_buffer_t *b) {
	sint8_t type = extras->type;

	if (PREFIX_TYPE_LIST == type) {
		uint8_t *ptr = extras->value.bytesVal;
		if(extras->len <= 0) {
			//锟斤拷锟斤拷锟斤拷
			bb_put_u16(b, 0);
			return false ;
		}

		//写锟斤拷锟介长锟斤拷
		if (!bb_put_u16(b, extras->len)) {
			return false ;
		}
		if (!bb_put_bytes(b, ptr, extras->len)) {
			return false;
		}
		return true;
	} else if(type == PREFIX_TYPE_INT) {
		if (!bb_put_s32(b, extras->value.s32Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_BYTE == type) {
		if (!bb_put_s8(b, extras->value.s8Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_SHORTT == type) {
		if (!bb_put_s16(b, extras->value.s16Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_LONG == type) {
		if (!bb_put_s64(b, extras->value.s64Val)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_FLOAT == type) {

	} else if (PREFIX_TYPE_DOUBLE == type) {

	} else if (PREFIX_TYPE_BOOLEAN == type) {
		if (!bb_put_bool(b, extras->value.boolVal)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_CHAR == type) {
		if (!bb_put_char(b, extras->value.charVal)) {
			return false ;
		}
		return true;
	} else if (PREFIX_TYPE_STRINGG == type) {
		sint8_t len = extras->len;
		if(len < MAX_BYTE_VALUE) {
			bb_put_s8(b,len);
		}else if(len < MAX_SHORT_VALUE) {
			//0X7F=01111111=127 byte
			//0X0100=00000001 00000000=128 short
			bb_put_s8(b,MAX_BYTE_VALUE);
			bb_put_s16(b,len);
		}else if(len < MAX_INT_VALUE) {
			bb_put_s8(b,MAX_BYTE_VALUE);
			bb_put_s16(b,MAX_SHORT_VALUE);
			bb_put_s32(b,len);
		} else {
			//锟街凤拷锟斤拷锟斤拷锟饺筹拷锟斤拷锟斤拷锟斤拷
			return false;
		}

		if(len > 0) {
			if(!bb_put_chars(b,extras->value.bytesVal,len)) {
				return false;
			}
		}
	}

	return true;
}

/**
 * len  extra锟斤拷锟捷筹拷锟斤拷
 * 锟斤拷锟斤拷晒锟斤拷锟斤拷锟絫rue,失锟杰凤拷锟斤拷false
 */
ICACHE_FLASH_ATTR msg_extra_data_t *extra_decode(byte_buffer_t *b){

	if(b == NULL || bb_is_empty(b)) return NULL;

	//int elen = b.readUnsignedShort();
	uint8_t eleLen;//元素的个数,最多可以存放255个元素
	if(!bb_get_u8(b, &eleLen)) {
		INFO("ERROR: read extra data length fail\r\n");
		return NULL;
	}

	if(eleLen == 0) return NULL;//无元素

	/*byte_buffer_t *wrapBuf = bb_create(0);
	if(wrapBuf == NULL) return NULL;*/

	//byte_buffer_t *wrapBuf = bb_buffer_wrap(b,eleLen,true);

	msg_extra_data_t *ed = NULL;
	//os_zalloc(sizeof(struct _msg_extra_data))

	uint8_t keyType;
	if(!bb_get_u8(b, &keyType)) {
		INFO("ERROR: read extra data keyType fail\r\n");
		return NULL;
	}

	while(eleLen > 0) {
		eleLen--;

		char *p = NULL;
		sint8_t k = 0;

		if(keyType == EXTRA_KEY_TYPE_STRING) {//字符串
			uint8_t flag;
			p = bb_readString(b,&flag);
			if(flag != JM_SUCCESS){
				INFO("_c_pubsubItemParseBin fail to read keyType\n");
				return NULL;
			}
			//v->strKey = k;
		} else {
			sint8_t k;
			if(!bb_get_s8(b,&k)) {
				INFO("ERROR:extra_decode read key error\n");
				return NULL;
			}
			//v->key = k;
		}

		msg_extra_data_t *v = extra_decodeVal(b);
		if(v == NULL) {
			continue;
		}

		if(keyType == EXTRA_KEY_TYPE_STRING) {//字符串
			v->strKey = p;
		} else {
			v->key = k;
		}

		if(ed == NULL) {
			ed = v;
		} else {
			v->next = ed;
			ed = v;
		}
	}
	//bb_release(wrapBuf);
	return ed;

}

/**
 *wl锟芥储锟斤拷锟斤拷锟絙yte锟斤拷锟街斤拷锟斤拷
 *锟斤拷锟斤拷晒锟斤拷锟斤拷锟絫rue,失锟杰凤拷锟斤拷false
 */
ICACHE_FLASH_ATTR BOOL extra_encode(msg_extra_data_t *extras, byte_buffer_t *b, uint16_t *wl, uint8_t keyType){

	*wl = 0;//默锟斤拷写锟斤拷锟捷筹拷锟饺碉拷锟斤拷0

	//锟斤拷锟街碉拷前写写位锟矫ｏ拷
	uint16_t wpos = b->wpos;

	int eleCnt = 0;
	msg_extra_data_t *te = extras;
	while(te != NULL) {
		eleCnt++;
		te = te->next;
	}

	bb_put_u8(b,eleCnt);//写入元素个数

	if(eleCnt == 0) {
		*wl = 1;
		return true;//锟睫革拷锟斤拷锟斤拷锟斤拷
	}

	if(!bb_put_u8(b, keyType)) {
		INFO("extra_encode fail to write keyType: %d", keyType);
		return false;
	}

	BOOL strKey = keyType == EXTRA_KEY_TYPE_STRING;
	while(extras != NULL) {
		if(strKey) {
			if(!bb_writeString(b, extras->strKey,os_strlen(extras->strKey))) {
				INFO("extra_encode fail to write string key: %s", extras->strKey);
				return false;
			}
		}else {
			if(!bb_put_s8(b, extras->key)) {
				INFO("extra_encode fail to write key: %d", extras->key);
				return false;
			}
		}

		if((PREFIX_TYPE_STRINGG == extras->type || PREFIX_TYPE_LIST == extras->type) &&
				extras->value.bytesVal == NULL) {
			if(!bb_put_s8(b, PREFIX_TYPE_NULL)) {
				INFO("extra_encode fail to write PREFIX_TYPE_NULL: %d", PREFIX_TYPE_NULL);
				return false;
			} else {
				extras = extras->next;
				continue;
			}
		}

		if(!bb_put_s8(b, extras->type)) {
			INFO("extra_encode write extra type fail: %d", extras->type);
			return false;
		}

		if(!extra_encodeVal(extras,b)) {
			return false;
		}

		extras = extras->next;
	}

	//锟杰癸拷写锟斤拷锟街斤拷锟斤拷锟斤拷也锟斤拷锟角革拷锟斤拷锟斤拷息锟侥筹拷锟斤拷
	uint16_t wlen;
	if(b->wpos >= wpos) {
		wlen = b->wpos - wpos;
	} else {
		wlen = b->capacity- (wpos - b->wpos);
	}

	//锟斤拷锟斤拷锟斤拷锟斤拷锟杰筹拷锟斤拷
	*wl = wlen;

	return true;
}

//锟较诧拷锟斤拷锟斤拷from 锟节碉拷全锟斤拷锟斤拷锟捷合碉拷 to锟斤拷
ICACHE_FLASH_ATTR msg_extra_data_t * extra_pullAll(msg_extra_data_t *from, msg_extra_data_t *to){
	if(from == NULL) {
		INFO("from is NULL");
		return to;
	}

	msg_extra_data_t *f = from;
	while(f->next != NULL) f = f->next;//查找最后一个元素
	f->next = to;//插入链表头部
	return from;
}

ICACHE_FLASH_ATTR  sint16_t extra_getS16(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s16Val;
}

ICACHE_FLASH_ATTR  sint8_t extra_getS8(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s8Val;
}

ICACHE_FLASH_ATTR  sint64_t extra_getS64(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s64Val;
}

ICACHE_FLASH_ATTR  sint32_t extra_getS32(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? 0 :ed->value.s32Val;
}

ICACHE_FLASH_ATTR  char extra_getChar(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? 0 :ed->value.charVal;
}

ICACHE_FLASH_ATTR  BOOL extra_getBool(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? false : ed->value.boolVal;
}

ICACHE_FLASH_ATTR  char* extra_getChars(msg_extra_data_t *e, sint8_t key){
	msg_extra_data_t* ed = extra_get(e,key);
	return ed == NULL ? NULL : ed->value.bytesVal;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putByte(msg_extra_data_t *e, sint8_t key, sint8_t val){
	msg_extra_data_t *eem = extra_put(e, key, PREFIX_TYPE_BYTE);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s8Val = val;
	return eem;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putShort(msg_extra_data_t *e, sint8_t key, sint16_t val){
	msg_extra_data_t *eem = extra_put(e, key,PREFIX_TYPE_SHORTT);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s16Val = val;
	return eem;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putInt(msg_extra_data_t *e, sint8_t key, sint32_t val){
	msg_extra_data_t *eem = extra_put(e, key,PREFIX_TYPE_INT);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s32Val = val;
	return eem;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putLong(msg_extra_data_t *e, sint8_t key, sint64_t val){
	msg_extra_data_t *eem = extra_put(e, key,PREFIX_TYPE_LONG);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.s64Val = val;
	return eem;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putChar(msg_extra_data_t *e, sint8_t key, char val){
	msg_extra_data_t *eem = extra_put(e, key, PREFIX_TYPE_CHAR);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.charVal = val;
	return eem;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putBool(msg_extra_data_t *e, sint8_t key, BOOL val){
	msg_extra_data_t *eem = extra_put(e, key,PREFIX_TYPE_BOOLEAN);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.boolVal = val;
	return eem;
}

ICACHE_FLASH_ATTR msg_extra_data_t* extra_putChars(msg_extra_data_t *e, sint8_t key, const char* val, uint16_t len){
	msg_extra_data_t *eem = extra_put(e, key,PREFIX_TYPE_STRINGG);
	if(eem == NULL) {
		return NULL;
	}
	eem->value.bytesVal = val;
	eem->len = len;
	return eem;
}

/**************************************EXTRA DATA OPERATION END**********************************/

ICACHE_FLASH_ATTR void msg_release(jm_msg_t *msg) {
	if(msg == NULL) return;

	bb_release(msg->payload);

	extra_release(msg->extraMap);

	cache_back(CACHE_MESSAGE,msg);
}

ICACHE_FLASH_ATTR jm_msg_t* msg_create() {
	return cache_get(CACHE_MESSAGE,true);
}


ICACHE_FLASH_ATTR jm_msg_t* msg_create_msg(sint8_t type, byte_buffer_t *payload) {

	//, sint8_t up, sint8_t dp
	jm_msg_t *msg = msg_create();
	if(!msg) {
		INFO("msg_create_msg out of memory");
		return NULL;
	}

	msg->extraMap = NULL;
	msg->extrFlag = 0;
	msg->flag = 0;
	msg->payload = payload;
	msg->startTime = 0;
	msg->type = type;
	msg->msgId = ++msgId;

	/*
	ApiRequestJRso req = new ApiRequestJRso();
	req.setReqId(reqId);
	req.setArgs(args);

	if(loginKey != null) {
		req.getParams().put(Constants.LOGIN_KEY, loginKey);
	}
	*/

	//Message msg = new Message();
	//msg_setType(msg, MSG_TYPE_REQ_JRPC);

	//msg_setMsgId(msg, req.getReqId());

	msg_setUpProtocol(msg, PROTOCOL_JSON);
	msg_setDownProtocol(msg, PROTOCOL_JSON);

	//msg_setRpcMk(msg, true);
	//msg_setSmKeyCode(msg, mcode);
	//msg_putIntExtra(msg, EXTRA_KEY_SM_CODE, mcode);

	msg_setDumpDownStream(msg, false);
	msg_setDumpUpStream(msg, false);
	msg_setRespType(msg, MSG_TYPE_PINGPONG);

	msg_setOuterMessage(msg, true);
	msg_setMonitorable(msg, false);
	msg_setDebugMode(msg, false);

	//全锟斤拷锟届步锟斤拷锟截ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟届步锟斤拷锟截ｏ拷也锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷
	msg_setUpSsl(msg, false);
	//msg_setInsId(msg, 0);
	//extra_put(msg->extraMap, EXTRA_KEY_INSID, 0, PREFIX_TYPE_INT);

	msg_setForce2Json(msg, true);
	msg_setFromApiGateway(msg, true);

	/*ByteBuffer data = null;
	if(Message.PROTOCOL_BIN == up) {
		data = req.encode();
	} else {
		String json = JsonUtils.getIns().toJson(req);
		try {
			data = ByteBuffer.wrap(json.getBytes(Constants.CHARSET)) ;
		} catch (UnsupportedEncodingException e) {
			throw new CommonException(json,e);
		}
	}*/

	return msg;
}

ICACHE_FLASH_ATTR jm_msg_t* msg_create_rpc_msg(sint32_t mcode, byte_buffer_t *payload) {
	jm_msg_t *msg = msg_create_msg(MSG_TYPE_REQ_JRPC,payload);
	if(!msg) {
		return NULL;
	}

	msg_setRpcMk(msg, true);
	//msg_setSmKeyCode(msg, mcode);
	msg->extraMap = extra_putInt(msg->extraMap, EXTRA_KEY_SM_CODE, mcode);
	return msg;
}

ICACHE_FLASH_ATTR jm_msg_t* msg_create_ps_msg(byte_buffer_t *payload) {
	return msg_create_msg(MSG_TYPE_PUBSUB,payload);
}


ICACHE_FLASH_ATTR static BOOL msg_is_s16(sint16_t flag, sint16_t mask) {
	return (flag & mask) != 0;
}

ICACHE_FLASH_ATTR static BOOL msg_is_s32(sint32_t flag, sint32_t mask) {
	return (flag & mask) != 0;
}

ICACHE_FLASH_ATTR static sint32_t msg_set_s32(BOOL isTrue,sint32_t f,sint32_t mask) {
	return isTrue ?(f |= mask) : (f &= ~mask);
}

ICACHE_FLASH_ATTR static uint16_t msg_set_s16(BOOL isTrue, sint16_t f,sint16_t mask) {
	return isTrue ?(f |= mask) : (f &= ~mask);
}

// 锟角凤拷锟斤拷要锟斤拷锟叫伙拷extra锟斤拷锟斤拷
ICACHE_FLASH_ATTR static BOOL msg_isWriteExtra(jm_msg_t *msg) {
	return msg->extraMap != NULL;
}

//锟角凤拷锟斤拷要锟斤拷取extra锟斤拷锟斤拷
ICACHE_FLASH_ATTR static BOOL msg_isReadExtra(jm_msg_t *msg) {
	return msg_is_s16(msg->flag,FLAG_EXTRA);
}

ICACHE_FLASH_ATTR static void msg_setExtraFlag(jm_msg_t *msg, BOOL f) {
	msg->flag = msg_set_s16(f,msg->flag,FLAG_EXTRA);
}

ICACHE_FLASH_ATTR BOOL msg_isUpSsl(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_UP_SSL);
}

ICACHE_FLASH_ATTR void msg_setUpSsl(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_UP_SSL);
}

ICACHE_FLASH_ATTR BOOL msg_isDownSsl(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_DOWN_SSL);
}

ICACHE_FLASH_ATTR  void msg_setDownSsl(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f,msg->extrFlag,EXTRA_FLAG_DOWN_SSL);
}

ICACHE_FLASH_ATTR  BOOL msg_isFromApiGateway(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_FROM_APIGATEWAY);
}

ICACHE_FLASH_ATTR  void msg_setFromApiGateway(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_FROM_APIGATEWAY);
}

ICACHE_FLASH_ATTR  BOOL msg_isRsaEnc(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_ENC_TYPE);
}

ICACHE_FLASH_ATTR  void msg_setEncType(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f, msg->extrFlag,EXTRA_FLAG_ENC_TYPE);
}

ICACHE_FLASH_ATTR  BOOL msg_isSecretVersion(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_SECTET_VERSION);
}

ICACHE_FLASH_ATTR  void msg_setSecretVersion(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_SECTET_VERSION);
}

ICACHE_FLASH_ATTR  BOOL msg_isSign(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_IS_SIGN);
}

ICACHE_FLASH_ATTR  void msg_setSign(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_IS_SIGN);
}

ICACHE_FLASH_ATTR  BOOL msg_isSec(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_IS_SEC);
}

ICACHE_FLASH_ATTR  void msg_setSec(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f,msg->extrFlag, EXTRA_FLAG_IS_SEC);
}

ICACHE_FLASH_ATTR  BOOL msg_isRpcMk(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_RPC_MCODE);
}

ICACHE_FLASH_ATTR  void msg_setRpcMk(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_RPC_MCODE);
}

ICACHE_FLASH_ATTR  BOOL msg_isDumpUpStream(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag, EXTRA_FLAG_DUMP_UP);
}

ICACHE_FLASH_ATTR  void msg_setDumpUpStream(jm_msg_t *msg, BOOL f) {
	//flag0 |= f ? FLAG0_DUMP_UP : 0 ;
	msg->extrFlag = msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_DUMP_UP);
}

ICACHE_FLASH_ATTR  BOOL msg_isDumpDownStream(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag,EXTRA_FLAG_DUMP_DOWN);
}

ICACHE_FLASH_ATTR  void msg_setDumpDownStream(jm_msg_t *msg, BOOL f) {
	msg->extrFlag = msg_set_s32(f, msg->extrFlag, EXTRA_FLAG_DUMP_DOWN);
}

ICACHE_FLASH_ATTR  bool msg_isLoggable(jm_msg_t *msg) {
	return msg_getLogLevel(msg) > 0 ? true : false;
}

ICACHE_FLASH_ATTR  bool msg_isDebugMode(jm_msg_t *msg) {
	return msg_is_s32(msg->extrFlag,EXTRA_FLAG_DEBUG_MODE);
}

ICACHE_FLASH_ATTR  void msg_setDebugMode(jm_msg_t *msg, bool f) {
	msg->extrFlag = msg_set_s32(f,msg->extrFlag,EXTRA_FLAG_DEBUG_MODE);
}

ICACHE_FLASH_ATTR  BOOL msg_isMonitorable(jm_msg_t *msg) {
	return msg_is_s16(msg->flag,FLAG_MONITORABLE);
}

ICACHE_FLASH_ATTR  void msg_setMonitorable(jm_msg_t *msg, BOOL f) {
	msg->flag = msg_set_s16(f,msg->flag,FLAG_MONITORABLE);
}

ICACHE_FLASH_ATTR  BOOL msg_isError(jm_msg_t *msg) {
	return msg_is_s16(msg->flag,FLAG_ERROR);
}

ICACHE_FLASH_ATTR  void msg_setError(jm_msg_t *msg, BOOL f) {
	msg->flag = msg_set_s16(f,msg->flag,FLAG_ERROR);
}

ICACHE_FLASH_ATTR  BOOL msg_isOuterMessage(jm_msg_t *msg) {
	return msg_is_s16(msg->flag,FLAG_OUT_MESSAGE);
}

ICACHE_FLASH_ATTR  void msg_setOuterMessage(jm_msg_t *msg, BOOL f) {
	msg->flag = msg_set_s16(f,msg->flag,FLAG_OUT_MESSAGE);
}

ICACHE_FLASH_ATTR  BOOL msg_isForce2Json(jm_msg_t *msg) {
	return msg_is_s16(msg->flag, FLAG_FORCE_RESP_JSON);
}

ICACHE_FLASH_ATTR  void msg_setForce2Json(jm_msg_t *msg, BOOL f) {
	msg->flag = msg_set_s16(f,msg->flag,FLAG_FORCE_RESP_JSON);
}

ICACHE_FLASH_ATTR  BOOL msg_isNeedResponse(jm_msg_t *msg) {
	sint8_t rt = msg_getRespType(msg);
	return rt != MSG_TYPE_NO_RESP;
}

ICACHE_FLASH_ATTR  BOOL msg_isPubsubMessage(jm_msg_t *msg) {
	sint8_t rt = msg_getRespType(msg);
	return rt == MSG_TYPE_MANY_RESP;
}

ICACHE_FLASH_ATTR  BOOL msg_isPingPong(jm_msg_t *msg) {
	sint8_t rt = msg_getRespType(msg);
	return rt != MSG_TYPE_PINGPONG;
}

/**
 * @param f true 锟斤拷示锟斤拷锟斤拷锟斤拷false锟斤拷示锟斤拷锟斤拷锟斤拷
 */
ICACHE_FLASH_ATTR  void msg_setLengthType(jm_msg_t *msg, BOOL f) {
	//flag |= f ? FLAG_LENGTH_INT : 0 ;
	msg->flag = msg_set_s16(f, msg->flag, FLAG_LENGTH_INT);
}

ICACHE_FLASH_ATTR BOOL msg_isLengthInt(jm_msg_t *msg) {
	return msg_is_s16(msg->flag,FLAG_LENGTH_INT);
}

ICACHE_FLASH_ATTR  sint8_t msg_getPriority(jm_msg_t *msg) {
	return (sint8_t)((msg->extrFlag >> EXTRA_FLAG_PRIORITY) & 0x03);
}

ICACHE_FLASH_ATTR  BOOL msg_setPriority(jm_msg_t *msg, sint8_t l) {
	if(l > PRIORITY_3 || l < PRIORITY_0) {
		return false;
	}
	msg->extrFlag = (l << EXTRA_FLAG_PRIORITY) | msg->extrFlag;
	return true;
}

ICACHE_FLASH_ATTR  sint8_t msg_getLogLevel(jm_msg_t *msg) {
	sint8_t v = (sint8_t)((msg->flag >> FLAG_LOG_LEVEL) & 0x07);
	return v;
}

//000 001 010 011 100 101 110 111
ICACHE_FLASH_ATTR  BOOL msg_setLogLevel(jm_msg_t *msg, sint8_t v) {
	if(v < 0 || v > 6) {
		 return false;
	}
	msg->flag = (uint16_t)((v << FLAG_LOG_LEVEL) | msg->flag);
	return true;
}

ICACHE_FLASH_ATTR sint8_t msg_getRespType(jm_msg_t *msg) {
	sint8_t v =  (sint8_t)((msg->flag >> FLAG_RESP_TYPE) & 0x03);
	return v;
}

ICACHE_FLASH_ATTR  BOOL msg_setRespType(jm_msg_t *msg, sint16_t v) {
	if(v < 0 || v > 3) {
		 return false;
	}
	msg->flag = (v << FLAG_RESP_TYPE) | msg->flag;
	return true;
}

ICACHE_FLASH_ATTR  sint8_t msg_getUpProtocol(jm_msg_t *msg ) {
	return msg_is_s16(msg->flag, FLAG_UP_PROTOCOL);
}

ICACHE_FLASH_ATTR  void msg_setUpProtocol(jm_msg_t *msg, sint8_t protocol) {
	//flag |= protocol == PROTOCOL_JSON ? FLAG_UP_PROTOCOL : 0 ;
	msg->flag = msg_set_s16(protocol == PROTOCOL_JSON, msg->flag, FLAG_UP_PROTOCOL);
}

ICACHE_FLASH_ATTR  sint8_t msg_getDownProtocol(jm_msg_t *msg) {
	return msg_is_s16(msg->flag, FLAG_DOWN_PROTOCOL);
}

ICACHE_FLASH_ATTR  void msg_setDownProtocol(jm_msg_t *msg, sint8_t protocol) {
	//flag |= protocol == PROTOCOL_JSON ? FLAG_DOWN_PROTOCOL : 0 ;
	msg->flag = msg_set_s16(protocol == PROTOCOL_JSON, msg->flag, FLAG_DOWN_PROTOCOL);
}

ICACHE_FLASH_ATTR static void freeMem(byte_buffer_t *buf){
	if(buf) bb_release(buf);
}

ICACHE_FLASH_ATTR jm_msg_t *msg_decode(byte_buffer_t *b) {

	jm_msg_t *msg = msg_create();
	if(msg == NULL) {
		INFO("ERROR: decode os_zalloc fail\r\n");
		goto error;
	}

	//锟斤拷0,1锟斤拷锟街斤拷
	//uint16_t flag;
	if(!bb_get_s16(b,&(msg->flag))) {
		INFO("ERROR: get rpc package flag fail\r\n");
		goto error;
	}

	//ByteBuffer b = ByteBuffer.wrap(data);
	sint32_t len = 0;
	if(msg_isLengthInt(msg)) {
		//len = b.readInt();
		if(!bb_get_s32(b, &len)) {
			INFO("ERROR: get pck int length fail\r\n");
			goto error;
		}
	} else {
		//len = b.readUnsignedShort(); // len = 锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷
		sint16_t slen = 0;
		if(!bb_get_s16(b, &slen)) {
			INFO("ERROR: get pck length fail\r\n");
			goto error;
		}
		len = slen;
	}

	if(bb_readable_len(b) < len){
		//throw new CommonException("Message len not valid");
		INFO("ERROR: bb_readable_len(b) < len \r\n");
		goto error;
	}

	//锟斤拷3锟斤拷锟街斤拷
	//msg.setVersion(b.readByte());

	//read type
	//锟斤拷4锟斤拷锟街斤拷

	//msg.setType(b.readByte());
	if(!bb_get_u8(b, &msg->type)) {
		INFO("ERROR: read msg type fail\r\n");
		goto error;
	}

	//msg.setMsgId(b.readLong());
	if(!bb_get_s64(b, &msg->msgId)) {
		INFO("ERROR: read msgId type fail\r\n");
		goto error;
	}

	if(msg_isReadExtra(msg)) {
		uint16_t curLen = bb_readable_len(b);

		msg->extraMap = extra_decode(b);
		if(msg->extraMap == NULL) {
			INFO("ERROR: read extra data fail\r\n");
			goto error;
		}

		len = len - (curLen - bb_readable_len(b));
		//msg->len = len;//锟斤拷效锟斤拷锟截的筹拷锟斤拷

		msg_extra_data_t *extraFlag = extra_get(msg->extraMap, EXTRA_KEY_FLAG);

		if(extraFlag) {
			msg->extrFlag = extraFlag->value.s32Val;
		}
	} else {
		//msg->len = len;//锟斤拷效锟斤拷锟截的筹拷锟斤拷
	}

	if(len > 0){
		byte_buffer_t *pl = bb_create(len);
		if(!bb_get_buf(b,pl,len)) {
			INFO("ERROR: buffer fail\r\n");
			os_free(pl);
			goto error;
		}
		msg->payload = pl;
		//msg.setPayload(ByteBuffer.wrap(payload));
	} else {
		msg->payload = NULL;
	}

	return msg;

	error:
		if(msg) {
			os_free(msg);
		}
		return NULL;
}

ICACHE_FLASH_ATTR BOOL msg_encode(jm_msg_t *msg, byte_buffer_t *buf) {

	byte_buffer_t *data = msg->payload;

	int len = 0;//锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷,锟斤拷锟斤拷锟斤拷头锟斤拷锟斤拷锟斤拷
	if(data != NULL){
		len = bb_readable_len(data);
	}

	if(msg->extrFlag != 0) {
		//锟斤拷锟斤拷展锟斤拷志位锟脚碉拷头锟斤拷锟斤拷锟斤拷锟斤拷
		msg->extraMap = extra_putInt(msg->extraMap, EXTRA_KEY_FLAG, msg->extrFlag);
	}

	byte_buffer_t *extBuf = NULL;
	uint16_t extraLen = 0;
	if(msg_isWriteExtra(msg)) {
		extBuf = bb_create(512);//头锟斤拷锟斤拷锟斤拷锟斤拷锟�512锟斤拷锟街斤拷
		if(!extra_encode(msg->extraMap, extBuf, &extraLen,EXTRA_KEY_TYPE_BYTE)) {
			//写锟斤拷锟斤拷锟斤拷息失锟斤拷
			goto doerror;
		}

		/*if(extraLen > 4092) {
			return false;
		}*/

		len += extraLen;
		msg_setExtraFlag(msg,true);
	}

	//锟斤拷1锟斤拷2锟斤拷锟街斤拷 ,len = 锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷模式时锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷
	if(len <= MAX_SHORT_VALUE) {
		msg_setLengthType(msg,false);
	} else if(len < MAX_INT_VALUE){
		msg_setLengthType(msg,true);
	} else {
		//throw new CommonException("Data length too long than :"+MAX_INT_VALUE+", but value "+len);
		goto doerror;
	}

	//锟斤拷0,1,2,3锟斤拷锟街节ｏ拷锟斤拷志头
	//b.put(this.flag);
	//b.writeShort(this.flag);
	//msg_writeUnsignedShort(buf, this.flag);
	if(!bb_put_s16(buf,msg->flag)){
		goto doerror;
	}

	if(len <= MAX_SHORT_VALUE) {
		//锟斤拷2锟斤拷3锟斤拷锟街斤拷 ,len = 锟斤拷锟捷筹拷锟斤拷 + 锟斤拷锟斤拷模式时锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷
		if(!bb_put_s16(buf,len)){
			goto doerror;
		}
		//b.writeUnsignedShort(len);
	}else if(len < MAX_INT_VALUE){
		//锟斤拷息锟斤拷锟斤拷锟斤拷锟斤拷蟪ざ锟轿狹AX_VALUE 2,3,4,5
		//b.writeInt(len);
		if(!bb_put_s32(buf,len)){
			goto doerror;
		}
	} else {
		goto doerror;
		//throw new CommonException("Max int value is :"+ Integer.MAX_VALUE+", but value "+len);
	}

	//b.putShort((short)0);

	//锟斤拷3锟斤拷锟街斤拷
	//b.put(this.version);
	//b.writeByte(this.method);

	//锟斤拷4锟斤拷锟街斤拷
	//writeUnsignedShort(b, this.type);
	//b.put(this.type);
	//b.writeByte(this.type);
	if(!bb_put_s8(buf,msg->type)){
		goto doerror;
	}

	//b.writeLong(this.msgId);
	if(!bb_put_s64(buf,msg->msgId)){
		goto doerror;
	}

	if(msg_isWriteExtra(msg)) {
		//b.writeInt(this.extrFlag);
		/*if(!bb_put_u16(buf,extraLen)){
			goto doerror;
		}*/

		if(!bb_put_buf(buf,extBuf)){
			goto doerror;
		}

		freeMem(extBuf);
		/*b.writeUnsignedShort(this.extra.remaining());
		b.write(this.extra);*/
	}

	if(data != NULL){
		//b.put(data);
		/*b.write(data);
		data.reset();*/
		bb_put_buf(buf,data);
		//freeMem(data);
	}
	return true;

	doerror:
		freeMem(extBuf);
		//freeMem(data);
		return false;
}

ICACHE_FLASH_ATTR jm_msg_t *msg_readMessage(byte_buffer_t *buf){

	//锟斤拷锟斤拷锟斤拷锟斤拷锟轿伙拷锟�
	uint16_t rpos = buf->rpos;

	//锟斤拷锟斤拷锟杰筹拷锟角凤拷晒锟斤拷锟揭伙拷锟斤拷锟斤拷锟斤拷锟叫★拷锟斤拷锟�
	uint16_t totalLen = bb_readable_len(buf);
	INFO("msg_readMessage totalLen %d\n",totalLen);

	if(totalLen < HEADER_LEN) {
		//锟缴讹拷锟斤拷锟斤拷锟捷筹拷锟斤拷小锟斤拷锟斤拷锟斤拷头锟斤拷锟斤拷锟斤拷
		INFO("msg_readMessage totalLen not egonf %d\n",totalLen);
		return NULL;
	}

	//取锟斤拷一锟斤拷锟街节憋拷志位
	sint16_t f ;

	if(!bb_get_s16(buf, &f)) {
		INFO("msg_readMessage read flag fail \n");
		return NULL;
	}

	uint32 len = 0;
	uint32 headerLen = HEADER_LEN;
	//取锟节讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟街斤拷 锟斤拷锟捷筹拷锟斤拷
	if(msg_is_s16(f,FLAG_LENGTH_INT)) {
		//32位锟斤拷锟斤拷
		if(!bb_get_s32(buf, &len)) {
			INFO("msg_readMessage int len fail \n");
			return NULL;
		}

		//锟斤拷原锟斤拷锟斤拷锟捷癸拷位锟斤拷
		if(!bb_set_rpos(buf,rpos)) {
			INFO("msg_readMessage int len pos fail \n");
			return NULL;
		}

		//len = Message.readUnsignedShort(cache);
		//锟斤拷原锟斤拷锟斤拷锟捷癸拷位锟斤拷
		//cache.position(pos);
		headerLen += 2;  //int锟酵憋拷默锟斤拷short锟斤拷2锟街斤拷
		if(totalLen < len + headerLen){
			INFO("msg_readMessage int total not egon totalLen:%d, need len: %d\n",totalLen,(len + headerLen));
			//锟斤拷锟斤拷锟杰癸拷锟斤拷一锟斤拷锟姐够锟斤拷锟饺碉拷锟斤拷锟捷帮拷
			return NULL;
		}
	} else {
		//16位锟斤拷锟斤拷
		//锟斤拷锟捷筹拷锟饺诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值
		//len = cache.getInt();
		//len = cache.getInt();
		if(!bb_get_s16(buf, &len)) {
			INFO("msg_readMessage read short len fail \n");
			return NULL;
		}

		//锟斤拷原锟斤拷锟斤拷锟捷癸拷位锟斤拷
		//cache.position(pos);
		if(!bb_set_rpos(buf,rpos)) {
			INFO("msg_readMessage recover pos fail \n");
			return NULL;
		}
		/*if(len > (Integer.MAX_VALUE-10000) || len < 0) {
			throw new CommonException("Got invalid message len: " + len + ",flag: " + f+",buf: " + cache.toString());
		}*/
		if(totalLen < len + headerLen){
			//锟斤拷锟斤拷锟杰癸拷锟斤拷一锟斤拷锟姐够锟斤拷锟饺碉拷锟斤拷锟捷帮拷
			INFO("msg_readMessage short total not egon totalLen:%d, need len: %d\n",totalLen,(len + headerLen));
			return NULL;
		}
	}

	//byte[] data = new byte[len + headerLen];
	//锟接伙拷锟斤拷锟叫讹拷一锟斤拷锟斤拷,cache锟斤拷position锟斤拷前锟斤拷
	//cache.get(data, 0, len + headerLen);
	//return Message.decode(new JDataInput(ByteBuffer.wrap(data)));

	jm_msg_t *msg = NULL;
	byte_buffer_t *cache = bb_create(len + headerLen);
	if(!cache) {
		INFO("msg_readMessage mof\n");
		return NULL;
	}
	if(bb_get_buf(buf,cache,len + headerLen)) {
		msg = msg_decode(cache);
		bb_release(cache);
	} else {
		bb_release(cache);
		INFO("msg_readMessage read data error\n");
		return NULL;
	}

	INFO("msg_readMessage success read one msg %d\n",msg->msgId);
	//byte_buffer_t *cache = bb_buffer_wrap(buf, ,true);
	//bb_rmove_forward(buf,len + headerLen);//前锟斤拷一锟斤拷锟斤拷息锟斤拷锟斤拷
	return msg;

}

#endif /* JMICRO_MQTT_JM_MSG_H_ */
