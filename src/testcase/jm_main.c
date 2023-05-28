/*
 * jm_main.c
 *
 *  Created on: 2023年5月20日
 *      Author: yeyulei
 */

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "jm_client.h"
#include "jm_mem.h"
#include "jm_cfg.h"

#include "test.h"

#include "time.h"

#include<pthread.h>
//#include<Windows.h>
#pragma comment(lib, "pthreadVC2.lib")  //必须加上这句

/***************TEST METHOD************************/
extern void kv_testAddDeviceData(sint32_t code, char *msg, char *loginKey, sint32_t actId);

/***************TEST METHOD************************/
//extern SYSCFG sysCfg;

//udp读线程
extern int upd_doLoop();
extern void client_jmConnCheck();
extern void jm_checkLocalServer();
extern BOOL client_recv_one_loop();

void start_timer_wrapper() {
	while(1){
		client_main_timer();
		sleep(3);//每三秒运行一次
	}
}

BOOL jm_checkNet() {
	//假设PC上网络永完可用
	return true;
}

BOOL jm_checkLoginStatus() {
	client_login(sysCfg.actId, sysCfg.device_id);
	return true;
}

sint64_t jm_getSystemTime() {
	clock_t t = clock();
	return t;
}

void start_jm() {

    INFO("start_jm starting\n");

    //sysCfg.jm_host = "192.168.3.22";
    char *host = "192.168.3.22";
    os_memcpy(sysCfg.jm_host,host,os_strlen(host));
    sysCfg.jm_host[os_strlen(host)] = '\0';

    sysCfg.jm_port = 9092;

    //sysCfg.device_id = "AAABiAXJ4vo=";
    char *did = "AAABiAXJ4vo=";
    os_memcpy(sysCfg.device_id,did,os_strlen(did));
    sysCfg.device_id[os_strlen(did)] = '\0';

    sysCfg.actId=809;

    sysCfg.use_udp = true;

    client_setJmInfo(sysCfg.jm_host, sysCfg.jm_port,sysCfg.use_udp);
    client_setSysTimeFn(jm_getSystemTime);

    client_getCheck()->jm_checkNet = jm_checkNet;

    client_getCheck()->jm_checkConCheck = client_jmConnCheck;

    client_getCheck()->jm_checkLocalServer = jm_checkLocalServer;
    client_getCheck()->jm_checkLoginStatus = jm_checkLoginStatus;

	client_init(sysCfg.actId, sysCfg.device_id);

	ctrl_init();

	udp_client_ws_init();//创建UDP服务器

	pthread_t checkerThreadId;
	pthread_create(&checkerThreadId, NULL, start_timer_wrapper,NULL);//模拟定时器

	//client_ws_init();
	//client_init("test00","1",true);

	//test KV CRUD
	//client_registLoginListener(kv_testAddDeviceData);
	//client_registLoginListener(test_jmLoginListener2);
	//client_registLoginListener(test_jmLoginListener3);

	//TCP 读线程
	if(!sysCfg.use_udp) {
		pthread_t udpReaderThreadId;
		pthread_create(&udpReaderThreadId, NULL, client_recv_one_loop,NULL);//UDP读数据线程
	}

	//testGetDeviceData();

	while(1) {
		//sleep(1);
		//printf("One loop: \n");
		if(!upd_doLoop()) {
			printf("start_jm JM tcp client_recv_one_loop:fail\n");
			sleep(2);
		}
	}

	INFO("start_jm start_jm stop\n");
}
