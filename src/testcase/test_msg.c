#include <stdio.h>
#include <assert.h>
#include "../jm_buffer.h"
#include "../jm_msg.h"
#include "../jm_constants.h"

void testMsg(){

	size_t pdlen = strlen(TRANSPORT_NETTY);

	byte_buffer_t *payload = bb_allocate(pdlen);

	assert(bb_put_chars(payload,TRANSPORT_NETTY,pdlen));

	//, "fnvHash1a", PROTOCOL_JSON, PROTOCOL_JSON
	jm_msg_t *msg = msg_create_rpc_msg(-655376287, payload);
	assert(msg != NULL);

	byte_buffer_t *buf = bb_allocate(1024);
	assert(buf != NULL);

	assert(msg_encode(msg,buf));

	jm_msg_t *dmsg = msg_decode(buf);
	assert(dmsg != NULL);
	assert(dmsg->payload != NULL);

	char c;
	for(int i = 0; i<pdlen; i++) {
		if(bb_get_char(dmsg->payload,&c)) {
			printf("%c",c);
		}
	}

	bb_free(buf);
	//msg_release(dmsg);
	msg_release(msg);
}
