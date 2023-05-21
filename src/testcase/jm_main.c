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

#include<pthread.h>
//#include<Windows.h>
#pragma comment(lib, "pthreadVC2.lib")  //必须加上这句

//extern SYSCFG sysCfg;

//udp读线程
extern int upd_doLoop();
extern void client_jmConnCheck();
extern void jm_checkLocalServer();

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

void start_jm() {

    INFO("start_jm starting\n");

    client_getCheck()->jm_checkNet = jm_checkNet;
    client_getCheck()->jm_checkConCheck = client_jmConnCheck;
    client_getCheck()->jm_checkLocalServer = jm_checkLocalServer;
    client_getCheck()->jm_checkLoginStatus = jm_checkLoginStatus;

	client_init(sysCfg.actId, sysCfg.device_id);

	jm_ctrl_init();

	udp_client_ws_init();//创建UDP服务器

	pthread_t checkerThreadId;
	pthread_create(&checkerThreadId, NULL, start_timer_wrapper,NULL);//模拟定时器

	pthread_t udpReaderThreadId;
	pthread_create(&udpReaderThreadId, NULL, upd_doLoop,NULL);//UDP读数据线程

	//client_ws_init();
	//client_init("test00","1",true);

	//client_registLoginListener(test_jmLoginListener);
	//client_registLoginListener(test_jmLoginListener2);
	//client_registLoginListener(test_jmLoginListener3);

	//TCP 读线程
	while(1) {
		//sleep(1);
		//printf("One loop: \n");
		if(!client_recv_one_loop()) {
			printf("start_jm JM tcp client_recv_one_loop:fail\n");
			sleep(2);
		}
	}

	INFO("start_jm start_jm stop\n");
}
