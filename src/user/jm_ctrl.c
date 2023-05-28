#ifndef WIN32
#include <stdlib.h>
#include "mem.h"
#include "c_types.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "gpio.h"
#include "osapi.h"
#include "user_interface.h"
#endif

#ifdef WIN32
#include "eagle_soc.h"
#include "ets_sys.h"
#include "gpio.h"
#include "jm_test_8266.h"
#include "user_interface.h"
#include "test.h"
#endif

#include "jm_device.h"
#include "debug.h"
#include "jm_buffer.h"
#include "jm_mem.h"
#include "jm_msg.h"
#include "jm_constants.h"
#include "jm_cfg.h"
#include "jm_client.h"
#include "jm_ctrl.h"

#define RPC_CB_CODE_VERS 1
#define RPC_CB_CODE_SAVE_RST 2

#define CTRL_ITEM_TYPE PS_TYPE_CODE(-128)

#define FUNC_SIZE 20

static jm_hash_map_t *functionItems;

static ctrl_arg bindDeviceIdArgs[] = {
	{"op", "0", PREFIX_TYPE_BYTE, 0, RESP_OP_CODE_DESC},
	{"deviceId", "", PREFIX_TYPE_STRINGG, 0, RESP_OP_DEVICEID_DESC},
	{"actId", "", PREFIX_TYPE_INT, 0, RESP_OP_ACTID_DESC},
};

ICACHE_FLASH_ATTR static void  _ctrl_onRpcResult(msg_extra_data_t *rst, sint32_t code, char *errMsg, void *args);

//注册一个控制方法
ICACHE_FLASH_ATTR BOOL ctrl_registFun(char *funName, ctrl_fn fun, ctrl_arg* args, char *desc, uint8_t ver) {
	if(ctrl_exists(funName)) {
		INFO("ctrl_registFun %s name exist",funName);
		return false;
	}

	ctrl_item *fn = os_zalloc(sizeof(ctrl_item));
	os_memset(fn, 0, sizeof(ctrl_item));

	uint8_t fnlen = os_strlen(funName);
	fn->funName = os_zalloc(fnlen+1);
	os_memcpy(fn->funName,funName,fnlen);
	fn->funName[fnlen] = '\0';

	if(desc) {
		fnlen = os_strlen(desc);
		fn->funDesc = os_zalloc(fnlen+1);
		os_memcpy(fn->funDesc,desc,fnlen);
		fn->funDesc[fnlen] = '\0';
	}

	fn->fn = fun;
	fn->type = CTRL_ITEM_TYPE;
	fn->ver=ver;
	fn->args = args;

	BOOL rst = hashmap_put(functionItems, fn->funName, fn);

	//ctrl_item *fnh = hashmap_get(functionItems,fn->funName);
	//INFO("ctrl_registFun funcName: %s\n",fnh->funName);

	return rst;
}

//注册一个控制方法
/*ICACHE_FLASH_ATTR BOOL ctrl_regist(ctrl_item *fn) {
	if(ctrl_exists(fn->funName)) {
		INFO("ctrl_regist %s name exist",fn->funName);
		return false;
	}
	return hashmap_put(functionItems,fn->funName,fn);
}*/

//删除一个控制方法
ICACHE_FLASH_ATTR BOOL ctrl_unregist(char *funName){
	ctrl_item *fn = hashmap_get(functionItems,funName);
	if(fn == NULL) return true;

	os_free(fn->funName);
	os_free(fn);

	return hashmap_remove(functionItems,funName);
}

//是否已经注册了指定名称的方法
ICACHE_FLASH_ATTR BOOL ctrl_exists(char *funName){
	return hashmap_exist(functionItems,funName);
}

ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_getUdpExtra(jm_pubsub_item_t *it){
	msg_extra_data_t* isUdphost = extra_sget(it->cxt, EXTRA_SKEY_UDP_ACK);
	//msg_extra_data_t *ex = NULL;
	if(isUdphost == NULL || isUdphost->value.s8Val == false) {
		return NULL;
	}

	msg_extra_data_t* ehost = extra_sget(it->cxt, EXTRA_SKEY_UDP_HOST);
	msg_extra_data_t* pport = extra_sget(it->cxt, EXTRA_SKEY_UDP_PORT);

	char *host = ehost->value.bytesVal;
	sint32_t port = pport == NULL? "":pport->value.s32Val;
	sint8_t isUdp = isUdphost == NULL? "":isUdphost->value.s8Val;

	INFO("_ctrl_getUdpExtra remote ip: %d.%d.%d.%d \r\n",host[0], host[1], host[2], host[3]);// windows 输出错误，ESP输出正常
	INFO("_ctrl_getUdpExtra remote port: %d \r\n",port);

	msg_extra_data_t *nex = extra_putChars(NULL, EXTRA_KEY_UDP_HOST, host, ehost->len);
	nex = extra_putInt(nex, EXTRA_KEY_UDP_PORT,port);
	nex = extra_putByte(nex, EXTRA_KEY_UDP_ACK,1);
	return nex;

}

ICACHE_FLASH_ATTR static client_send_msg_result_t _ctrl_respExtra(jm_pubsub_item_t *it, msg_extra_data_t *payload){

	jm_pubsub_item_t *item = cache_get(CACHE_PUBSUB_ITEM, true);
	if(item == NULL) {
		INFO("_ctrl_respExtra create PS item fail\n");
		return false;
	}

	item->id = it->id;//应答报文

	client_initPubsubItem(item, FLAG_DATA_EXTRA);

	item->callback = it->callback;
	item->type = it->type;
	item->topic = it->topic;
	item->fr = it->to;
	item->to = it->fr;

	byte_buffer_t *buf = bb_create(WRITE_BUF_SIZE);
	uint16_t wl;

	item->data = payload;
	client_send_msg_result_t sendResult = client_publishPubsubItem(item, _ctrl_getUdpExtra(it));
	INFO("_ctrl_respExtra send result %d topic: %s, %u\n",sendResult, item->topic, item->id);

	bb_release(buf);
	extra_release(item->cxt);
	cache_back(CACHE_PUBSUB_ITEM,item);

	return sendResult;
}

ICACHE_FLASH_ATTR static client_send_msg_result_t _ctrl_respError(jm_pubsub_item_t *it, sint8_t code,char *msg){
	msg_extra_data_t *ex = extra_sputS16(NULL, "code", code);
	extra_sputStr(ex, "msg", msg, os_strlen(msg));
	return _ctrl_respExtra(it,ex);
}


ICACHE_FLASH_ATTR static void _ctrl_invokeFunc(jm_pubsub_item_t *it){

	msg_extra_data_t *ps = (msg_extra_data_t *)it->data;
	if(ps == NULL) {
		_ctrl_respError(it,1,"_ctrl_invokeFunc invalid argument\n");
		return;
	}

	msg_extra_data_t *funcNameEx = extra_sget(ps,FUNC_NAME);
	if(funcNameEx == NULL) {
		_ctrl_respError(it,2,"_ctrl_invokeFunc null method name\n");
		return;
	}

	ctrl_item *fn = hashmap_get(functionItems, funcNameEx->value.bytesVal);
	if(fn == NULL) {
		char str[50];
		os_sprintf(str,"%s method not found\n",funcNameEx->value.bytesVal);
		_ctrl_respError(it,3,str);
		return;
	}

	INFO("_ctrl_invokeFunc %s begin\n",funcNameEx->value.bytesVal);
	msg_extra_data_t *rst = fn->fn(ps);
	if(rst != NULL) {
		_ctrl_respExtra(it, rst);
	}
	INFO("_ctrl_invokeFunc %s end\n",funcNameEx->value.bytesVal);

}


ICACHE_FLASH_ATTR static void _ctrl_onPubsubItemTypeListener(jm_pubsub_item_t *item) {
	INFO("_ctrl_onPubsubItemTypeListener got ps item type: %d\n",item->type);
	switch(item->type) {
	case PS_TYPE_CODE(0):
		//0号类型用于测试ESP8266络网连接是否正常
		//test_onTest(item);
		INFO("_ctrl_onPubsubItemTypeListener test device ready OK!\n");
		break;
	case PS_TYPE_CODE(1):
		//反转15引脚电平信息
		INFO("_ctrl_onPubsubItemTypeListener node_ledTurn begin\n");
	    uint32  gpioNo = atoi((char*)item->data);
#ifndef WIN32
	    jm_key_ledTurn(gpioNo);
#endif
		INFO("_ctrl_onPubsubItemTypeListener node_ledTurn end\n");
		break;

#ifdef JM_AT
	case PS_TYPE_CODE(2):
		//AT指令
		INFO("_ctrl_onPubsubItemTypeListener process AT cmd %s\n",item->data);
		at_fake_uart_rx(item->data,os_strlen(item->data));
		INFO("_ctrl_onPubsubItemTypeListener process AT cmd end\n");
		break;
#endif

	case CTRL_ITEM_TYPE:
		//动态方法调用
		_ctrl_invokeFunc(item);
		break;
	default:
		os_printf("Not support type code:\n",item->type);
		break;
	}
}

/**
 * RPC函数参数都是一个LIST，LIST每个元素表示函数的一个参数
 * 每个功能元素用一个Map表示，
 * 功能的参数args是一个列表，内嵌一个列表，列表每个元素是一个对像
 * 所以RPC参数结构如下
 * --LIST  PREFIX_TYPE_LIST  extra
 * 	[  --Map PREFIX_TYPE_MAP  extra
 * 		{
	 * 		ver:
	 * 		type:
	 * 		funcDesc:
	 * 		funName:
	 * 		args:[ --LIST PREFIX_TYPE_LIST
	 * 			-- Map  PREFIX_TYPE_MAP
	 * 			{
	 * 				name:
	 * 				desc:
	 * 				defVal:
	 * 				maxLen:
	 * 				type:
	 * 			}
	 * 		]
 * 		}
 * 	]
 *
 *	list extra
 *		bytesVal -- Map extra 1
 *						bytesVal -- Map fields list
 *				 -- Map extra 2
 *				 		bytesVal -- Map fields list
 *				 -- Map extra N
 *				 		bytesVal -- Map fields list
 */
ICACHE_FLASH_ATTR static void  _ctrl_updateOneFun(ctrl_item *fi){
	INFO("_ctrl_updateOneFun %s\n",fi->funName);

	msg_extra_data_t *rpcPsList = extra_sput(NULL, "rpcPsList", PREFIX_TYPE_LIST);

	msg_extra_data_t *psObj1 = extra_sput(NULL, "psObj1", PREFIX_TYPE_MAP);
	rpcPsList->value.bytesVal = psObj1;

	//代表一个功能
	msg_extra_data_t *efi = extra_sputStr(NULL, "funName", fi->funName, -1);
	psObj1->value.bytesVal = efi;

	extra_sputS8(efi, "ver", fi->ver);
	extra_sputS8(efi, "type", fi->type);
	extra_sputStr(efi, "funDesc", fi->funDesc, -1);

	if(fi->args != NULL && sizeof(fi->args) > 0) {
		//功能关联参数
		msg_extra_data_t *arg = extra_sput(efi, "args", PREFIX_TYPE_LIST);

		msg_extra_data_t *argList = NULL;

		uint8_t argNum = sizeof(fi->args)/sizeof(ctrl_item);

		INFO("_ctrl_updateOneFun argNum: %d\n",argNum);

		for(int idx = 0; idx < argNum; idx++ ) {
			ctrl_arg *ar = fi->args + idx;
			INFO("_ctrl_updateOneFun arg: %s \n", ar->name);

			msg_extra_data_t *e = extra_sputStr(argList, "name", ar->name, -1);
			if(argList == NULL) {
				argList = e;
			}
			extra_sputStr(argList, "desc", ar->desc);
			extra_sputStr(argList, "defVal", ar->defVal);
			extra_sputS8(argList, "maxLen", ar->maxLen);
			extra_sputS8(argList, "type", ar->type);
		}

		arg->len = 4;
		arg->neddFreeBytes = true;
		arg->value.bytesVal = argList;

		INFO("_ctrl_updateOneFun arg end: %s \n", fi->funName);
	}

	INFO("_ctrl_updateOneFun  invoke RPC:  %s \n", fi->funName);

	/*msg_extra_data_t *ps = extra_sput(NULL, "ps", PREFIX_TYPE_MAP);
	ps->value.bytesVal = efi;
	ps->neddFreeBytes = true;

	msg_extra_data_t *plist = extra_sput(NULL, "list", PREFIX_TYPE_LIST);
	ps->value.bytesVal = ps;
	ps->neddFreeBytes = true;*/

	client_invokeRpc(916042094, rpcPsList, _ctrl_onRpcResult, (void*)RPC_CB_CODE_SAVE_RST);

	extra_release(rpcPsList);

	INFO("_ctrl_updateOneFun end %s\n",fi->funName);
}

ICACHE_FLASH_ATTR static void  _ctrl_delOneFun(char *funName){
	INFO("_ctrl_delOneFun %s\n", funName);
}

ICACHE_FLASH_ATTR static void  _ctrl_submitFuns(msg_extra_data_t *verMap){

	INFO("_ctrl_submitFuns begin\n");

	jm_hash_map_iterator_t ite = {functionItems, NULL, -1};

	if(verMap == NULL) {
		INFO("_ctrl_submitFuns do ALL add\n");
		//服务器无此设备数据，需做全量更新
		ctrl_item *mi = NULL;
		//INFO("jm_hash_map_iterator_t cur: %u, idx: %d, map: %u\n", ite.cur, ite.idx, ite.map);
		while((mi = hashmap_iteNext(&ite)) != NULL) {
			//INFO("fnaddr: %u, desc: %s, name: %s, type: %d, ver:%d\n", mi->fn, mi->funDesc, mi->funName, mi->type, mi->ver);
			INFO("_ctrl_submitFuns add one %s\n", mi->funName);
			_ctrl_updateOneFun(mi);
		}
	} else {
		INFO("_ctrl_submitFuns do diff update\n");

		msg_extra_data_t *ex = NULL;
		//查找新增
		ctrl_item *mi = NULL;
		while((mi = hashmap_iteNext(&ite)) != NULL) {
			 ex = extra_sget(verMap, mi->funName);
			 if(NULL == ex) {
				 INFO("_ctrl_submitFuns add one %s\n", mi->funName);
				 //服务器没有，新增
				 _ctrl_updateOneFun(mi);
			 } else {
				 if(mi->ver > ex->value.s8Val) {
					 INFO("_ctrl_submitFuns update one %s\n", mi->funName);
					 //版本更新
					 _ctrl_updateOneFun(mi);
				 }
			 }
		}
		INFO("_ctrl_submitFuns do diff update end\n");

		msg_extra_data_iterator_t extraIte = {verMap, NULL};
		//查找删除
		while((ex = extra_iteNext(&extraIte)) != NULL) {
			if(!hashmap_exist(functionItems,ex->strKey)) {
				INFO("_ctrl_submitFuns delete one %s\n", ex->strKey);
				_ctrl_delOneFun(ex->strKey);
			}
		}

		INFO("_ctrl_submitFuns do diff delete end\n");
	}

	INFO("_ctrl_submitFuns end\n");
}

//同步本地命令到JM平台
ICACHE_FLASH_ATTR static void  _ctrl_onRpcResult(msg_extra_data_t *rst, sint32_t code, char *errMsg, void *args){
	if(code != 0) {
		INFO("_ctrl_onRpcResult got error code: %d, errMsg: %s, args:%d\n",code, errMsg, args);
		return;
	}

	INFO("_ctrl_onRpcResult got result cbcode:%d\n",args);

	msg_extra_data_t *verList = extra_sget(rst,"data");

	uint8_t cbcode = (uint8_t) args;
	switch(cbcode) {
	case RPC_CB_CODE_VERS:
		if(verList == NULL) {
			INFO("Server not device fun, need submit all funs\n");
		} else {
			verList = (msg_extra_data_t *)verList->value.bytesVal;
		}
		//服务器功能版本码列表
		_ctrl_submitFuns(verList);
		break;
	case RPC_CB_CODE_SAVE_RST:
		//服务器功能版本码列表
		INFO("Save function result\n");
		break;
	}
	INFO("_ctrl_onRpcResult end cbcode:%d\n",args);
}

ICACHE_FLASH_ATTR static void  _ctrl_jmLoginListener(sint32_t code, char *msg, char *loginKey, sint32_t actId){
	if(!client_isLogin()) {
		os_printf(" Login fail with code: %d, msg:%s\n",code,msg);
		return;
	}

	client_invokeRpc(-970493731, NULL, _ctrl_onRpcResult, (void*)RPC_CB_CODE_VERS);

	if(client_subscribeByType(_ctrl_onPubsubItemTypeListener,0,true)) {
		//订阅本设备全部类型消息,消息经服务器下发，所以需要登录成功后才能下发
		INFO("_ctrl_jmLoginListener remote success\n");
	} else {
		INFO("_ctrl_jmLoginListener remote error\n");
	}

}

ICACHE_FLASH_ATTR void test_scan_wifi_cb(void *arg, STATUS statu)
{
	if(statu != 0){
		os_printf("Scan fail with status:%d", statu);
		return;
	}

	struct bss_info	*bss_link = (struct	bss_info *)arg;
	while(bss_link != NULL){
		os_printf("Scan SSID: %s", bss_link->ssid);
		os_printf("\n");
		//bss_link = bss_link->next.stqe_next;
	}

}

ICACHE_FLASH_ATTR void ICACHE_FLASH_ATTR test_scan_wifi_aps()
{
#ifndef WIN32
	if(wifi_station_scan(NULL, test_scan_wifi_cb) != true){
		os_printf("Scan wifi ap fail!");
	} else {
		os_printf("Scan wifi on going....");
	}
#endif
}


/******************************esp 8266 远程方法开始***********************************/

ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_invalidOpCode(msg_extra_data_t *ps, char *m, sint8_t op){
	char msg[64];
	os_sprintf(msg, m, op);
	ps = extra_sputStr(ps,"msg", msg,3);
	return ps;
}

ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_remote_sysInfo(msg_extra_data_t *ps){
#ifndef WIN32
	msg_extra_data_t *h = extra_sputStr(NULL,"sdkVersion", system_get_sdk_version(), -1);
	extra_sputS8(h,"flashSizeMap", system_get_flash_size_map());
	extra_sputS8(h,"opmode", wifi_get_opmode());


	struct station_config cfg;
	wifi_station_get_config(&cfg);
	extra_sputStr(h,"ssid", cfg.ssid, -1);
	extra_sputS64(h,"shipId", system_get_chip_id());
	extra_sputStr(h,"deviceHostName", wifi_station_get_hostname(),-1);

	struct	ip_info	info;
	BOOL suc = false;
	uint8_t opmode = wifi_get_opmode();
	if(opmode == 0x01) {
		suc = wifi_get_ip_info(STATION_IF,&info);
	}else if(opmode == 0x02) {
		suc = wifi_get_ip_info(SOFTAP_IF,&info);
	}

	if(suc) {
		extra_sputStr(h,"deviceIP", IP2STR(info.ip.addr),-1);
	}else {
		extra_sputStr(h,"deviceIP", "Inalid IP",-1);
	}

	return h;

#else
	return NULL;
#endif
}

/**
 * 由op参数确定 1：绑定设备 ，2： 解绑设备， 0：查询设备绑定账号
 */
ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_remote_bindDevice(msg_extra_data_t *ps){

	msg_extra_data_t *h = extra_sputS16(NULL,RESP_CODE, 0);
	sint8_t op = extra_sgetChars(ps,RESP_OP);

	INFO("_ctrl_remote_bindDevice begin op=%d\n",op);
	//1.1.1.1 255.255.255.255
	BOOL up = false;

	switch(op) {
	case RESP_OP_QRY:
		//查询绑定关系
		extra_sputS16(h,"deviceId", sysCfg.device_id,-1);
		extra_sputS32(h,"actId", sysCfg.actId);
		extra_sputBool(h,"isLogin", client_isLogin());
		break;
	case RESP_OP_UP:
		//绑定设备
		/*if(sysCfg.actId > 0) {
			extra_sputStr(h,RESP_MSG, "device have been band!");
			extra_sputS16(h,RESP_CODE, 1);
			return h;
		}*/
		INFO("_ctrl_remote_bindDevice do update device info\n");
		char *deviceId = extra_sgetChars(ps,"deviceId");

		uint8_t hlen = os_strlen(deviceId);
		if( hlen > 6 && hlen < 16) {
			os_memcpy(sysCfg.device_id, deviceId, hlen);
			sysCfg.device_id[hlen] = '\0';
			up = true;
			INFO("_ctrl_remote_bindDevice deviceId=%s\n",sysCfg.device_id);
		} else {
			char msg[32] = {0};
			os_sprintf(msg,"invalid deviceId %d\n",hlen);
			extra_sputStr(h,RESP_MSG, msg,-1);
			extra_sputS16(h,RESP_CODE, 2);
			INFO(msg);
			return h;
		}

		sint32_t actId = extra_sgetS32(ps,"actId");
		if(actId > 0) {
			sysCfg.actId = actId;
			up = true;
			INFO("_ctrl_remote_bindDevice actId=%d\n",sysCfg.actId);
		} else {
			char msg[32];
			os_sprintf(msg,"invalid actId %d\n",actId);
			extra_sputStr(h,RESP_MSG, msg,-1);
			extra_sputS16(h,RESP_CODE, 3);
			INFO(msg);
			return h;
		}
		if(up) {
#ifndef WIN32
			cfg_save();
#endif
			//重新登录
			//client_logout();
			//client_login(sysCfg.actId, sysCfg.device_id);
		}
		break;
	case RESP_OP_DEL:
		INFO("_ctrl_remote_bindDevice unbind device\n");
		//解绑
		if(sysCfg.actId <= 0) {
			extra_sputStr(h,RESP_MSG, "device not bind any account!",-1);
			extra_sputS16(h,RESP_CODE, 1);
			return h;
		}

		//client_logout();
		sysCfg.actId = 0;
		os_memset(sysCfg.device_id,0,32);
#ifndef WIN32
			cfg_save();
#endif
		break;
	default:
		_ctrl_invalidOpCode(h,"invalid op code %d, 0:query, 1:bind, 2: unbind",op);
	}

	INFO("_ctrl_remote_bindDevice end\n");
	return h;
}

ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_remote_jmInfo(msg_extra_data_t *ps){

	_ctrl_remote_bindDevice(ps);

	msg_extra_data_t *h = extra_sputS16(NULL, RESP_CODE, 0);

	//1.1.1.1 255.255.255.255
	bool update;
	sint8_t op = extra_sgetS8(ps,RESP_OP);
	INFO("_ctrl_remote_jmInfo begin op=%d\n",op);

	switch(op) {
	case RESP_OP_QRY:
		//查询
		INFO("_ctrl_remote_jmInfo do query\n");
		extra_sputStr(h,"jmHost", sysCfg.jm_host,-1);
		extra_sputS32(h,"port", sysCfg.jm_port);
		extra_sputS32(h,"isLogin", client_isLogin());
		break;
	case RESP_OP_UP:

		os_printf("_ctrl_remote_jmInfo do update\n", op);

		char *host = extra_sgetChars(ps,"jmHost");

		uint8_t hlen =0;
		if(host) {
			hlen = os_strlen(host);
		}

		if(hlen > 6 && hlen < 16) {
			os_memcpy(sysCfg.jm_host,host,os_strlen(host));
			sysCfg.jm_host[os_strlen(host)] = '\0';
			update = true;
			INFO("Set host: %s \n",sysCfg.jm_host);
		} else {
			char msg[32] = {0};
			os_sprintf(msg,"invalid host %s",host);
			extra_sputStr(h,"msg", msg,-1);
			return h;
		}

		sint32_t port = extra_sgetS32(ps,"jmPort");
		if(port > 2) {
			sysCfg.jm_port = port;
			update = true;
			INFO("Set port: %d\n", sysCfg.jm_port);
		} else {
			char msg[32];
			os_sprintf(msg,"invalid port %d\n",port);
			extra_sputStr(h,"msg", msg,-1);
			return h;
		}

		if(update) {
		//client_login()
#ifndef WIN32
			cfg_save();
#endif

#ifdef WIN32
		INFO("Connected jm server: %s:%d\n",sysCfg.jm_host, sysCfg.jm_port);
		//client_jmConnCheck();
		//client_login(sysCfg.actId, sysCfg.device_id);
#endif
			//h = _ctrl_remote_bindDevice(ps);
		}
		break;

	default:
		_ctrl_invalidOpCode(h,"invalid op code %d, 0:query, 1:bind, 2: unbind\n",op);
	}

	INFO("_ctrl_remote_jmInfo end\n");
	return h;
}

/**
 * op: 1 高电平， 2：低电平， 3：反转
 * gpioNo： 引脚编码
 */
ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_remote_ctrlGpio(msg_extra_data_t *ps){

	msg_extra_data_t *h = extra_sputS16(NULL,RESP_CODE, 0);

	sint8_t op = extra_sgetS8(ps,RESP_OP);//取得操作码
	uint32_t gpioNo = (uint32_t)extra_sgetS32(ps,"gpioNo");//引脚编号
#ifndef WIN32

	sint8_t curv = 100;
	if(op == 1) curv = 1;
	else if(op == 2) curv = 0;
	else if(op == 3) curv = ~((gpio_input_get()>>gpioNo) & BIT0);
	else if(op == 0) {
		//查旬
		extra_sputS16(h,"status", gpio_input_get()>>gpioNo);
	} else {
		//无效操作码
		h = extra_sputS16(h,RESP_CODE, 1);
		char msg[30];
		os_sprintf(msg,"Invalid op code: %d",op);
		extra_sputS16(h,RESP_MSG, msg,-1);
	}

	if(curv != 100) {
		gpio_output_set((curv)<<gpioNo, ((~curv) & 0x01)<<gpioNo, 1<<gpioNo,0);
	}
#endif
	return h;

}

ICACHE_FLASH_ATTR static msg_extra_data_t* _ctrl_remote_about(msg_extra_data_t *ps){
	char *str = "JMicro IoT is a system to operate devices deployed any place!";
	msg_extra_data_t *h = extra_sputStr(NULL,"desc", str, -1);
	INFO("_ctrl_remote_about %s=%s, len=%d\n", h->key, h->value.bytesVal, h->len);
	return h;
}

ICACHE_FLASH_ATTR void _ctrl_remote_restartSystem(msg_extra_data_t *ps) {
	INFO("_ctrl_remote_restartSystem restart system\n");
#ifndef WIN32
	system_restart();
#endif
}

/******************************esp 8266 远程方法结束***********************************/

/**
	char *name; //参数名称，如op,code,msg等
	char *defVal; //默认值
	sint8_t type; //参数类型，参考jm_msg.h文件  PREFIX_TYPE_BYTE，PREFIX_TYPE_SHORT等
	sint8_t maxLen; //参数最大长度
	char *desc;
 */
void ICACHE_FLASH_ATTR _ctrl_registDefFuntions(void){

	//ctrl_registFun("sysInfo", _ctrl_remote_sysInfo,NULL);
	//ctrl_registFun("about", _ctrl_remote_about,NULL);

	ctrl_registFun("bindDeviceId", _ctrl_remote_bindDevice, bindDeviceIdArgs,"bind device with ID",2);

	//ctrl_registFun("ctrlGpio", _ctrl_remote_ctrlGpio);

	//ctrl_registFun("jmInfo", _ctrl_remote_jmInfo);

	//ctrl_registFun("restartSystem", _ctrl_remote_restartSystem);

}

void ICACHE_FLASH_ATTR ctrl_init(void){

	functionItems = hashmap_create(FUNC_SIZE);

	_ctrl_registDefFuntions();

	//注册手机与设备间的消息处理器
	if(client_subscribe(TOPIC_P2P, (client_PubsubListenerFn)_ctrl_onPubsubItemTypeListener, 0, false)) {
		//订阅本设备全部类型消息
		INFO("test_onPubsubItemType1Listener TOPIC_P2P regist p2p topic success\n");
	} else {
		INFO("test_onPubsubItemType1Listener TOPIC_P2P  regist p2p topic fail\n");
	}

	/*if(client_subscribeP2PByType(test_onPubsubItemType1Listener,1)) {
		INFO("jm_udpclient_init regist test listener success\n");
	} else {
		INFO("jm_udpclient_init  regist test listener fail\n");
	}*/

	/*if(client_subscribeByType(_ctrl_onPubsubItemTypeListener,0,true)) {
		//订阅本设备全部类型消息,消息经服务器下发，所以需要登录成功后才能下发
		INFO("_ctrl_onPubsubItemTypeListener remote success\n");
	} else {
		INFO("_ctrl_onPubsubItemTypeListener remote error\n");
	}*/

	//node_initLed();
	//INFO("jm_ctrl_init regist login listener begin\n");
	client_registLoginListener(_ctrl_jmLoginListener);
	//INFO("jm_ctrl_init regist login listener end\n");
	//node_init_interrupt();

}
