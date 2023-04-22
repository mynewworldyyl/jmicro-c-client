#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "../jm_client.h"
#include "../jm_buffer.h"
#include "../debug.h"

extern BOOL client_recv_one_loop();
extern void client_ws_init();


int test_jm_client_pubsub()
{
	INFO("test_jm_client_pubsub starting");
	INFO("test_jm_client_pubsub starting");

	setbuf(stdout,NULL);

	client_ws_init();//����TCP����
	client_init("test00","1");//��ʼ���ͻ���,��¼������ƽ̨

	while(1) {
		sleep(1);
		printf("One loop: ");
		if(!client_recv_one_loop()) {
			printf("client_recv_one_loop:fail\n");
		}
	}
}

