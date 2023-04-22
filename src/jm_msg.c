/*
 * jm_msg.h
 *
 *  Created on: 2023年4月10日
 *      Author: yeyulei
 */

#ifndef JMICRO_MQTT_JM_MSG_H_
#define JMICRO_MQTT_JM_MSG_H_

//#include "jm_req.h"
#include "jm_buffer.h"
#include "jm_msg.h"
#include "jm_constants.h"
#include "testcase/test.h"

static sint64_t msgId = 0;

ICACHE_FLASH_ATTR void msg_release(jm_msg_t *msg) {

	if(msg == NULL) return;

	if(msg->payload) {
		bb_free(msg->payload);
		msg->payload = NULL;
	}

	msg_extra_release(msg->extraMap);

	os_free(msg);

	/*
	if(msg->extraMap) {
		msg_extra_data_t * em = msg->extraMap->next;
		while(msg->extraMap) {

			if((PREFIX_TYPE_STRING == msg->extraMap->type || PREFIX_TYPE_LIST == msg->extraMap->type) &&
					msg->extraMap->value.bytesVal != NULL) {
				os_free(msg->extraMap->value.bytesVal);
			}
			os_free(msg->extraMap);
			msg->extraMap = em;
			if(em) {
				em = em->next;
			}
		}
	}
	*/
}

ICACHE_FLASH_ATTR void msg_extra_release(msg_extra_data_t *extra) {
	if(!extra) return;
	msg_extra_data_t *next;
	msg_extra_data_t *em = extra;
	while(em) {
		next = em->next;
		em->next = NULL;
		if((PREFIX_TYPE_STRING == em->type || PREFIX_TYPE_LIST == em->type) &&
				em->value.bytesVal != NULL) {
			//os_free(em->value.bytesVal);
			em->value.bytesVal = NULL;
		}
		os_free(em);
		em = next;
	}
}

ICACHE_FLASH_ATTR jm_msg_t* msg_create_msg(sint8_t type, byte_buffer_t *payload) {

	//, sint8_t up, sint8_t dp
	size_t s = sizeof(struct _jm_msg);
	jm_msg_t *msg = (jm_msg_t*)os_zalloc(s);
	if(!msg) {
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

	//全部异步返回，服务器可以异步返回，也可以同步返回
	msg_setUpSsl(msg, false);
	//msg_setInsId(msg, 0);
	//msg_putExtra(msg, EXTRA_KEY_INSID, 0, PREFIX_TYPE_INT);

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
	msg_putIntExtra(msg, EXTRA_KEY_SM_CODE, mcode);
	return msg;
}

ICACHE_FLASH_ATTR jm_msg_t* msg_create_ps_msg(byte_buffer_t *payload) {
	jm_msg_t *msg = msg_create_msg(MSG_TYPE_PUBSUB,payload);
	if(!msg) {
		return NULL;
	}
	return msg;
}


ICACHE_FLASH_ATTR static BOOL msg_is_s16(sint16_t flag, sint16_t mask) {
	return (flag & mask) != 0;
}

ICACHE_FLASH_ATTR static BOOL msg_is_s32(sint32_t flag, sint32_t mask) {
	return (flag & mask) != 0;
}

ICACHE_FLASH_ATTR static uint32_t msg_set_s32(BOOL isTrue,sint32_t f,sint32_t mask) {
	return isTrue ?(f |= mask) : (f &= ~mask);
}

ICACHE_FLASH_ATTR static uint16_t msg_set_s16(BOOL isTrue, sint16_t f,sint16_t mask) {
	return isTrue ?(f |= mask) : (f &= ~mask);
}

// 是否需要序列化extra数据
ICACHE_FLASH_ATTR static BOOL msg_isWriteExtra(jm_msg_t *msg) {
	return msg->extraMap != NULL;
}

//是否需要读取extra数据
ICACHE_FLASH_ATTR static BOOL msg_isReadExtra(jm_msg_t *msg) {
	return msg_is_s16(msg->flag,FLAG_EXTRA);
}

ICACHE_FLASH_ATTR static void msg_setExtra(jm_msg_t *msg, BOOL f) {
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
 * @param f true 表示整数，false表示短整数
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

ICACHE_FLASH_ATTR sint64_t msg_getS64Extra(jm_msg_t *msg, sint8_t key) {
	msg_extra_data_t* ed = msg_getExtra(msg,key);
	return ed == NULL ? 0L : ed->value.s64Val;
}

ICACHE_FLASH_ATTR  sint32_t msg_getS32Extra(jm_msg_t *msg, sint8_t key) {
	msg_extra_data_t* ed = msg_getExtra(msg,key);
	return ed == NULL ? 0 :ed->value.s32Val;
}

ICACHE_FLASH_ATTR  sint16_t msg_getS16Extra(jm_msg_t *msg, sint8_t key) {
	msg_extra_data_t* ed = msg_getExtra(msg,key);
	return ed == NULL ? 0 :ed->value.s16Val;
}

ICACHE_FLASH_ATTR  sint8_t msg_getS8Extra(jm_msg_t *msg, sint8_t key) {
	msg_extra_data_t* ed = msg_getExtra(msg,key);
	return ed == NULL ? 0 :ed->value.s8Val;
}

/*ICACHE_FLASH_ATTR  void msg_setInsId(jm_msg_t *msg, sint32_t *insId) {
	if(insId < 0) {
		insId = abs(insId);
	}
	msg_putExtra(msg, EXTRA_KEY_INSID, insId,PREFIX_TYPE_INT);
}*/

/*ICACHE_FLASH_ATTR  sint32_t msg_getInsId(jm_msg_t *msg) {
	return msg_getS32Extra(msg,EXTRA_KEY_INSID);
}*/

/*ICACHE_FLASH_ATTR  void msg_setSignData(jm_msg_t *msg, char *data) {
	msg_putExtra(msg, EXTRA_KEY_SIGN, data, PREFIX_TYPE_LIST);
}*/

ICACHE_FLASH_ATTR  char* msg_getCharsExtra(jm_msg_t *msg, sint8_t key) {
	msg_extra_data_t* ed = msg_getExtra(msg,key);
	return ed == NULL ? NULL : (char*)(ed->value.bytesVal);
}

/*ICACHE_FLASH_ATTR  char* msg_getSignData(jm_msg_t *msg) {
	return msg_getCharExtra(msg, EXTRA_KEY_SIGN);
}*/

/*ICACHE_FLASH_ATTR  void msg_setLinkId(jm_msg_t *msg, sint64_t *insId) {
	msg_putExtra(msg,EXTRA_KEY_LINKID, insId, PREFIX_TYPE_LONG);
}

ICACHE_FLASH_ATTR  sint64_t msg_getLinkId(jm_msg_t *msg) {
	return msg_getS64Extra(msg, EXTRA_KEY_LINKID);
}

ICACHE_FLASH_ATTR void msg_setSaltData(jm_msg_t *msg, char* data) {
	msg_putExtra(msg, EXTRA_KEY_SALT, data,PREFIX_TYPE_STRINGG);
}

ICACHE_FLASH_ATTR char* msg_getSaltData(jm_msg_t *msg) {
	return msg_getCharExtra(msg, EXTRA_KEY_SALT);
}

ICACHE_FLASH_ATTR  void msg_setSecData(jm_msg_t *msg, char* data) {
	msg_putExtra(msg,EXTRA_KEY_SEC, data,PREFIX_TYPE_STRINGG);
}

ICACHE_FLASH_ATTR char* msg_getSecData(jm_msg_t *msg, char **rst) {
	return msg_getCharExtra(msg, EXTRA_KEY_SEC);
}

ICACHE_FLASH_ATTR  void msg_setSmKeyCode(jm_msg_t *msg, sint32_t *code) {
	msg_putExtra(msg,EXTRA_KEY_SM_CODE, code, PREFIX_TYPE_INT);
}

ICACHE_FLASH_ATTR  sint32_t msg_getSmKeyCode(jm_msg_t *msg) {
	return msg_getS32Extra(msg, EXTRA_KEY_SM_CODE);
}

ICACHE_FLASH_ATTR  void msg_setMethod(jm_msg_t *msg, char* method) {
	msg_putExtra(msg,EXTRA_KEY_SM_NAME, method, PREFIX_TYPE_STRINGG);
}

ICACHE_FLASH_ATTR  char* msg_getMethod(jm_msg_t *msg) {
	return msg_getCharExtra(msg, EXTRA_KEY_SM_NAME);
}

ICACHE_FLASH_ATTR  void msg_setTime(jm_msg_t *msg, sint64_t *time) {
	msg_putExtra(msg, EXTRA_KEY_TIME, time, PREFIX_TYPE_LONG);
}

ICACHE_FLASH_ATTR sint64 msg_getTime(jm_msg_t *msg) {
	return msg_getS64Extra(msg, EXTRA_KEY_TIME);
}*/

ICACHE_FLASH_ATTR static void freeMem(byte_buffer_t *buf){
	if(buf) bb_free(buf);
}


ICACHE_FLASH_ATTR msg_extra_data_t* msg_getExtra(jm_msg_t *msg, sint8_t key) {
	msg_extra_data_t *em = msg->extraMap;
	while(em != NULL) {
		if(em->key == key) {
			return em;
		}
		em = em->next;
	}
	 return NULL;
}

ICACHE_FLASH_ATTR static msg_extra_data_t * msg_decodeVal(byte_buffer_t *b) {

		sint8_t k;
		if(!bb_get_s8(b,&k)) {
			goto error;
		}

		sint8_t type;
		if(!bb_get_s8(b,&type)){
			goto error;
		}

		msg_extra_data_t *rst = os_zalloc(sizeof(struct _msg_extra_data));
		//void *val = NULL;
		sint16_t len = 0;

		if(type == PREFIX_TYPE_NULL) {
			rst->value.bytesVal = NULL;
		}else if(PREFIX_TYPE_LIST == type){
			//字节数组
			if(!bb_get_s16(b,&len)) {
				goto error;
			}

			if(len == 0) {
				rst->value.bytesVal = NULL;
			} else {
				uint8_t *arr = (uint8_t*)os_zalloc(len);
				//arr[0] = len;//数组长度
				if(!bb_get_bytes(b,arr,len)) {
					//读数据失败
					if(arr) os_free(arr);
					goto error;
				}
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
		}else if(PREFIX_TYPE_STRING == type){

			sint8_t slen;
			if(!bb_get_s8(b,&slen)) {
				goto error;
			}
			len = slen;

			if(len == -1) {
				//空串
				rst->value.bytesVal = NULL;
			}else if(len == 0) {
				char* vptr = (char*)os_zalloc(sizeof(char));
				*vptr = "";
				rst->value.bytesVal = vptr;
			}else {

				sint32_t ilen = len;

				if(len == 127) {
					//一个字节最大值，有符号数，兼容Java
					sint16_t slen;
					if(!bb_get_s16(b,&slen)) {
						goto error;
					}
					ilen = slen;

					if(slen == 32767) {
						//两个字节最大值，有符号数，兼容Java
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

				vptr[ilen] = '\0';//字符串结束标志
				rst->value.bytesVal = vptr;

			}
		}


		rst->key = k;
		rst->type = type;
		//rst->value = val;
		rst->next = NULL;
		rst->len = len;

		return rst;

		error:
			if(rst) os_free(rst);
			return NULL;

	}

ICACHE_FLASH_ATTR static BOOL msg_encodeVal(msg_extra_data_t *extras, byte_buffer_t *b) {
	sint8_t type = extras->type;

	if (PREFIX_TYPE_LIST == type) {
		uint8_t *ptr = extras->value.bytesVal;
		if(extras->len <= 0) {
			//空数组
			bb_put_u16(b, 0);
			return false ;
		}

		//写数组长度
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
	} else if (PREFIX_TYPE_STRING == type) {
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
			//字符串长度超过限制
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

ICACHE_FLASH_ATTR msg_extra_data_t *msg_decodeExtra(byte_buffer_t *b, uint16_t len) {
		if(b == NULL || bb_is_empty(b) || len <= 0) return NULL;

		/*byte_buffer_t *wrapBuf = bb_allocate(0);
		if(wrapBuf == NULL) return NULL;*/

		byte_buffer_t *wrapBuf = bb_buffer_wrap(b,len,true);

		msg_extra_data_t *ed = NULL;
		//os_zalloc(sizeof(struct _msg_extra_data))

		while(bb_readable_len(wrapBuf) > 0) {
			msg_extra_data_t *v = msg_decodeVal(wrapBuf);
			if(v == NULL) {
				break;
			}
			if(ed == NULL) {
				ed = v;
			} else {
				v->next = ed;
				ed = v;
			}
		}
		bb_free(wrapBuf);
		return ed;
	}

ICACHE_FLASH_ATTR static BOOL msg_encodeExtra(msg_extra_data_t *extras, byte_buffer_t *b, uint16_t *wl) {
	*wl = 0;//默认写数据长度等于0

	if(extras == NULL) return true;//无附加数据

	//备分当前写写位置，
	uint16_t wpos = b->wpos;

	while(extras != NULL) {
		if(!bb_put_s8(b, extras->key)) {
			return false;
		}

		if((PREFIX_TYPE_STRING == extras->type || PREFIX_TYPE_LIST == extras->type) &&
				extras->value.bytesVal == NULL) {
			if(!bb_put_s8(b, PREFIX_TYPE_NULL)) {
				return false;
			} else {
				extras = extras->next;
				continue;
			}
		}

		if(!bb_put_s8(b, extras->type)) {
			return false;
		}

		if(!msg_encodeVal(extras,b)) {
			return false;
		}

		extras = extras->next;
	}

	//总共写的字节数，也就是附加信息的长度
	uint16_t wlen;
	if(b->wpos >= wpos) {
		wlen = b->wpos - wpos;
	} else {
		wlen = b->capacity- (wpos - b->wpos);
	}

	//附加数据总长度
	*wl = wlen;

	return true;
}

ICACHE_FLASH_ATTR jm_msg_t *msg_decode(byte_buffer_t *b) {

	jm_msg_t *msg = (jm_msg_t*)os_zalloc(sizeof(struct _jm_msg));
	if(msg == NULL) {
		INFO("ERROR: decode os_zalloc fail\r\n");
		goto error;
	}

	//第0,1个字节
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
		//len = b.readUnsignedShort(); // len = 数据长度 + 附加数据长度
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

	//第3个字节
	//msg.setVersion(b.readByte());

	//read type
	//第4个字节

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
		//int elen = b.readUnsignedShort();
		uint16_t elen;
		if(!bb_get_u16(b, &elen)) {
			INFO("ERROR: read extra data length fail\r\n");
			goto error;
		}

		msg_extra_data_t *extdata = msg_decodeExtra(b, elen);
		if(extdata == NULL) {
			INFO("ERROR: read extra data fail\r\n");
			goto error;
		}

		msg->extraMap = extdata;

		len = len - EXT_HEADER_LEN - elen;
		//msg->len = len;//有效负载的长度

		msg_extra_data_t *extraFlag = msg_getExtra(msg, EXTRA_KEY_FLAG);

		if(extraFlag) {
			msg->extrFlag = extraFlag->value.s32Val;
		}
	} else {
		//msg->len = len;//有效负载的长度
	}

	if(len > 0){
		byte_buffer_t *pl = bb_allocate(len);
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

	int len = 0;//数据长度 + 附加数据长度,不包括头部长度
	if(data != NULL){
		len = bb_readable_len(data);
	}

	if(msg->extrFlag != 0) {
		//将扩展标志位放到头部参数中
		msg_putIntExtra(msg, EXTRA_KEY_FLAG, msg->extrFlag);
	}

	byte_buffer_t *extBuf = NULL;
	uint16_t extraLen = 0;
	if(msg_isWriteExtra(msg)) {
		extBuf = bb_allocate(512);//头部长度最大512个字节
		if(!msg_encodeExtra(msg->extraMap, extBuf, &extraLen)) {
			//写附加信息失败
			goto doerror;
		}

		/*if(extraLen > 4092) {
			return false;
		}*/

		len += extraLen + EXT_HEADER_LEN;
		msg_setExtra(msg,true);
	}

	//第1，2个字节 ,len = 数据长度 + 测试模式时附加数据长度
	if(len <= MAX_SHORT_VALUE) {
		msg_setLengthType(msg,false);
	} else if(len < MAX_INT_VALUE){
		msg_setLengthType(msg,true);
	} else {
		//throw new CommonException("Data length too long than :"+MAX_INT_VALUE+", but value "+len);
		goto doerror;
	}

	//第0,1,2,3个字节，标志头
	//b.put(this.flag);
	//b.writeShort(this.flag);
	//msg_writeUnsignedShort(buf, this.flag);
	if(!bb_put_u16(buf,msg->flag)){
		goto doerror;
	}

	if(len <= MAX_SHORT_VALUE) {
		//第2，3个字节 ,len = 数据长度 + 测试模式时附加数据长度
		if(!bb_put_u16(buf,len)){
			goto doerror;
		}
		//b.writeUnsignedShort(len);
	}else if(len < MAX_INT_VALUE){
		//消息内内容最大长度为MAX_VALUE 2,3,4,5
		//b.writeInt(len);
		if(!bb_put_u32(buf,len)){
			goto doerror;
		}
	} else {
		goto doerror;
		//throw new CommonException("Max int value is :"+ Integer.MAX_VALUE+", but value "+len);
	}

	//b.putShort((short)0);

	//第3个字节
	//b.put(this.version);
	//b.writeByte(this.method);

	//第4个字节
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
		if(!bb_put_u16(buf,extraLen)){
			goto doerror;
		}

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

	//保存读数据位置
	uint16_t rpos = buf->rpos;

	//数据总长是否可构建一个包的最小长度
	uint16_t totalLen = bb_readable_len(buf);
	if(totalLen < HEADER_LEN) {
		//可读的数据长度小于最少头部长度
		return NULL;
	}

	//取第一个字节标志位
	uint16_t f ;

	if(!bb_get_u16(buf, &f)) {
		return NULL;
	}

	uint16_t len = 0;
	uint16_t headerLen = HEADER_LEN;
	//取第二，第三个字节 数据长度
	if(msg_is_s16(f,FLAG_LENGTH_INT)) {
		//数据长度不可能起过整数的最大值
		//len = cache.getInt();
		//len = cache.getInt();
		if(!bb_get_u16(buf, &len)) {
			return NULL;
		}

		//还原读数据公位置
		//cache.position(pos);
		if(!bb_set_rpos(buf,rpos)) {
			return NULL;
		}
		/*if(len > (Integer.MAX_VALUE-10000) || len < 0) {
			throw new CommonException("Got invalid message len: " + len + ",flag: " + f+",buf: " + cache.toString());
		}*/

		headerLen += 2;  //int型比默认short多2字节
		if(totalLen < len + headerLen){
			//还不能构成一个足够长度的数据包
			return NULL;
		}
	} else {
		if(!bb_get_u16(buf, &len)) {
			return NULL;
		}

		//还原读数据公位置
		if(!bb_set_rpos(buf,rpos)) {
			return NULL;
		}

		//len = Message.readUnsignedShort(cache);
		//还原读数据公位置
		//cache.position(pos);
		if(totalLen < len + headerLen){
			//还不能构成一个足够长度的数据包
			return NULL;
		}
	}

	//byte[] data = new byte[len + headerLen];
	//从缓存中读一个包,cache的position往前推
	//cache.get(data, 0, len + headerLen);
	//return Message.decode(new JDataInput(ByteBuffer.wrap(data)));

	jm_msg_t *msg = NULL;
	byte_buffer_t *cache = bb_allocate(len + headerLen);
	if(!cache) {
		INFO("msg_readMessage同存溢出");
		return NULL;
	}
	if(bb_get_buf(buf,cache,len + headerLen)) {
		msg = msg_decode(cache);
		bb_free(cache);
	}else {
		bb_free(cache);
		INFO("msg_readMessage读数据错误");
		return NULL;
	}

	//byte_buffer_t *cache = bb_buffer_wrap(buf, ,true);
	//bb_rmove_forward(buf,len + headerLen);//前移一个消息长度
	return msg;

}

ICACHE_FLASH_ATTR static msg_extra_data_t* msg_putExtra(jm_msg_t *msg, sint8_t key, sint8_t type) {

	msg_extra_data_t *eem = msg_getExtra(msg, key);
	if(eem != NULL) {
		return eem;
	}

	msg_extra_data_t *em = (msg_extra_data_t *)os_zalloc(sizeof(struct _msg_extra_data));
	if(em==NULL) return NULL;

	em->key = key;
	//em->value = val;
	em->type = type;
	em->len = 0;

	if(msg->extraMap == NULL) {
		msg->extraMap = em;
		em->next = NULL;
	} else {
		//表头插入
		em->next = msg->extraMap;
		msg->extraMap = em;
		//msg->extraMap->next = NULL;
	}
	return em;
}

ICACHE_FLASH_ATTR BOOL msg_putByteExtra(jm_msg_t *msg, sint8_t key, sint8_t val) {
	msg_extra_data_t *eem = msg_putExtra(msg, key, PREFIX_TYPE_BYTE);
	if(eem == NULL) {
		return false;
	}
	eem->value.s8Val = val;
	return true;
}

ICACHE_FLASH_ATTR BOOL msg_putShortExtra(jm_msg_t *msg, sint8_t key, sint16_t val) {
	msg_extra_data_t *eem = msg_putExtra(msg, key,PREFIX_TYPE_SHORTT);
	if(eem == NULL) {
		return false;
	}
	eem->value.s16Val = val;
	return true;
}

ICACHE_FLASH_ATTR BOOL msg_putIntExtra(jm_msg_t *msg, sint8_t key, sint32_t val) {
	msg_extra_data_t *eem = msg_putExtra(msg, key,PREFIX_TYPE_INT);
	if(eem == NULL) {
		return false;
	}
	eem->value.s32Val = val;
	return true;
}

ICACHE_FLASH_ATTR BOOL msg_putLongExtra(jm_msg_t *msg, sint8_t key, sint64_t val) {
	msg_extra_data_t *eem = msg_putExtra(msg, key,PREFIX_TYPE_LONG);
	if(eem == NULL) {
		return false;
	}
	eem->value.s64Val = val;
	return true;
}

ICACHE_FLASH_ATTR BOOL msg_putCharExtra(jm_msg_t *msg, sint8_t key, char val) {
	msg_extra_data_t *eem = msg_putExtra(msg, key,PREFIX_TYPE_CHAR);
	if(eem == NULL) {
		return false;
	}
	eem->value.charVal = val;
	return true;
}

ICACHE_FLASH_ATTR BOOL msg_putBoolExtra(jm_msg_t *msg, sint8_t key, BOOL val) {
	msg_extra_data_t *eem = msg_putExtra(msg, key,PREFIX_TYPE_BOOLEAN);
	if(eem == NULL) {
		return false;
	}
	eem->value.boolVal = val;
	return true;
}

ICACHE_FLASH_ATTR BOOL msg_putCharsExtra(jm_msg_t *msg, sint8_t key, const char* val, uint16_t len) {
	msg_extra_data_t *eem = msg_putExtra(msg, key,PREFIX_TYPE_STRING);
	if(eem == NULL) {
		return false;
	}
	eem->value.bytesVal = val;
	eem->len = len;
	return true;
}

#endif /* JMICRO_MQTT_JM_MSG_H_ */
