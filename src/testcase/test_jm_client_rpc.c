#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "../jm_client.h"
#include "../jm_buffer.h"

extern BOOL client_recv_one_loop();
extern void client_ws_init();

uint8_t client_rpc_callback(byte_buffer_t *payload, void *arg){
	printf("RPC Response: ");
	char c=0;
	while(!bb_is_empty(payload)) {
		if(bb_get_char(payload,&c)) {
			printf("%c",c);
		}
	}
	printf("\n");
	return SUCCESS;
}

int test_jm_client_rpc()
{
	setbuf(stdout,NULL);

	client_init();
	client_ws_init();

	char *pro = "{\"args\":[\"netty\"],\"params\":{\"NCR\":\"\"}}";
	//size_t pdlen = strlen(pro);
	//byte_buffer_t *payload = bb_allocate(pdlen);
	//assert(bb_put_chars(payload, pro, pdlen));

	//int msgId = client_invoke_rpc(-655376287, payload, client_rpc_callback);

	while(1) {
		//bb_rmark(payload);
		//client_invoke_rpc(-655376287, payload, (client_rpc_callback_fn*)client_rpc_callback);
		client_invokeRpcWithStrArgs(-655376287, pro, (client_rpc_callback_fn*)client_rpc_callback, NULL);
		//bb_rmark_reset(payload);
		sleep(1);
		printf("One loop");
		if(!client_recv_one_loop()) {
			printf("client_recv_one_loop:fail\n");
		}
	}
}

