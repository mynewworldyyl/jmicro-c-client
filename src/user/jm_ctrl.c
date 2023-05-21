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

#define TLED GPIO_ID_PIN(5)
#define TINT_INPUT GPIO_ID_PIN(12)

#define CTRL_ITEM_TYPE PS_TYPE_CODE(-128)

static uint32 intCnt;

#define FUNC_SIZE 20
static jm_hash_map_t *functionItems;

//注册一个控制方法
ICACHE_FLASH_ATTR BOOL ctrl_registFun(char *funName, ctrl_fn fun) {
	if(ctrl_exists(funName)) {
		INFO("ctrl_registFun %s name exist",funName);
		return false;
	}

	uint8_t fnlen = os_strlen(funName);
	char *fname = os_zalloc(fnlen+1);
	os_memcpy(fname,funName,fnlen);
	fname[fnlen] = '\0';

	ctrl_item *fn = os_zalloc(sizeof(ctrl_item));
	fn->fn = fun;
	fn->funName = fname;
	fn->type = CTRL_ITEM_TYPE;

	return hashmap_put(functionItems,fname,fn);
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


/*ICACHE_FLASH_ATTR static void node_initLed()
{
	os_printf("init LED turn\n");
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);

	GPIO_OUTPUT_SET(TLED,1);
}*/

ICACHE_FLASH_ATTR static void node_ledTurn(uint32_t gpioNo)
{
	os_printf("LED turn\n");

	//uint32 curv = GPIO_INPUT_GET(TLED);
	//GPIO_OUTPUT_SET(TLED, ~curv);
#ifndef WIN32
	uint32 curv = ~((gpio_input_get()>>gpioNo) & BIT0);
	gpio_output_set((curv)<<gpioNo, ((~curv) & 0x01)<<gpioNo, 1<<gpioNo,0);
#endif

}

ICACHE_FLASH_ATTR void node_interrupt_cb() {
#ifndef WIN32
	uint32	gpio_status;
	gpio_status	= GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

	os_printf("Interrupt cnt %d\n", intCnt++);
#endif

}

ICACHE_FLASH_ATTR  void node_init_interrupt()
{
	os_printf("init LED turn");

	/*
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	GPIO_DIS_OUTPUT(TINT_INPUT);

	PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDI_U);

	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH((ets_isr_t)node_interrupt_cb,	NULL);
	gpio_pin_intr_state_set(TINT_INPUT,	GPIO_PIN_INTR_NEGEDGE);
	ETS_GPIO_INTR_ENABLE();
	*/
#ifndef WIN32
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U , FUNC_GPIO0);
	GPIO_DIS_OUTPUT(0);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(node_interrupt_cb, NULL);
	gpio_pin_intr_state_set(0, GPIO_PIN_INTR_NEGEDGE);
	ETS_GPIO_INTR_ENABLE();
#endif

}

ICACHE_FLASH_ATTR static void jm_ctrl_onPubsubItemTypeListener(jm_pubsub_item_t *item) {
	INFO("jm_ctrl_onPubsubItemTypeListener got ps item type: %d\n",item->type);
	switch(item->type) {
	case PS_TYPE_CODE(0):
		//0号类型用于测试ESP8266络网连接是否正常
		//test_onTest(item);
		INFO("jm_ctrl_onPubsubItemTypeListener test device ready OK!\n");
		break;
	case PS_TYPE_CODE(1):
		//反转15引脚电平信息
		INFO("jm_ctrl_onPubsubItemTypeListener node_ledTurn begin\n");
	    uint32  gpioNo = atoi((char*)item->data);
		node_ledTurn(gpioNo);
		INFO("jm_ctrl_onPubsubItemTypeListener node_ledTurn end\n");
		break;

#ifdef JM_AT
	case PS_TYPE_CODE(2):
		//AT指令
		INFO("jm_ctrl_onPubsubItemTypeListener process AT cmd %s\n",item->data);
		at_fake_uart_rx(item->data,os_strlen(item->data));
		INFO("jm_ctrl_onPubsubItemTypeListener process AT cmd end\n");
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

ICACHE_FLASH_ATTR static void  jm_ctrl_jmLoginListener(sint32_t code, char *msg, char *loginKey, sint32_t actId){
	if(!client_isLogin()) {
		os_printf("jm_ctrl_jmLoginListener Login fail with code: %d, msg:%s\n",code,msg);
		return;
	}
	if(client_subscribeByType(jm_ctrl_onPubsubItemTypeListener,0,true)) {
		//订阅本设备全部类型消息,消息经服务器下发，所以需要登录成功后才能下发
		INFO("jm_ctrl_onPubsubItemTypeListener remote success\n");
	} else {
		INFO("jm_ctrl_onPubsubItemTypeListener remote error\n");
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
	ps = extra_sputStr(ps,"msg", msg);
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
		extra_sputS32(h,"actId", sysCfg.actId,-1);
		extra_sputS32(h,"isLogin", client_isLogin());
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
			extra_sputStr(h,RESP_MSG, msg);
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
			extra_sputStr(h,RESP_MSG, msg);
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
			extra_sputStr(h,RESP_MSG, "device not bind any account!");
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
			extra_sputStr(h,"msg", msg);
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
			extra_sputStr(h,"msg", msg);
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

/******************************esp 8266 远程方法结束***********************************/

void ICACHE_FLASH_ATTR jm_ctrl_init(void){

	functionItems = hashmap_create(FUNC_SIZE);
	ctrl_registFun("sysInfo",_ctrl_remote_sysInfo);
	ctrl_registFun("about",_ctrl_remote_about);
	ctrl_registFun("bindDeviceId",_ctrl_remote_bindDevice);
	ctrl_registFun("ctrlGpio",_ctrl_remote_ctrlGpio);
	ctrl_registFun("jmInfo",_ctrl_remote_jmInfo);

	//注册手机与设备间的消息处理器
	if(client_subscribe(TOPIC_P2P, (client_PubsubListenerFn)jm_ctrl_onPubsubItemTypeListener, 0, false)) {
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

	/*if(client_subscribeByType(jm_ctrl_onPubsubItemTypeListener,0,true)) {
		//订阅本设备全部类型消息,消息经服务器下发，所以需要登录成功后才能下发
		INFO("jm_ctrl_onPubsubItemTypeListener remote success\n");
	} else {
		INFO("jm_ctrl_onPubsubItemTypeListener remote error\n");
	}*/

	//node_initLed();
	//INFO("jm_ctrl_init regist login listener begin\n");
	client_registLoginListener(jm_ctrl_jmLoginListener);
	//INFO("jm_ctrl_init regist login listener end\n");
	//node_init_interrupt();

}
