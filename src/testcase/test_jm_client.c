#include <stdio.h>
#include <winsock.h>
#include <assert.h>

#include "../jm_constants.h"
#include "../jm_msg.h"
#include "../jm_buffer.h"

#define PORT 9092
#define IP "192.168.3.217"
#define USERAGENT "ApOgEE MinGW Socket Client 1.0"

byte_buffer_t *create_message();
jm_msg_t *decode_message(byte_buffer_t *);

int test_jm_client()
{
	int client_socket;
	char *host;
	struct sockaddr_in *remote;
	int tmpres;
	//char *get;
	char buf[BUFSIZ+1];

	printf("Socket client example\n");

	// create tcp socket
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

	client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket < 0) {
		printf("client_socket = %d\n",client_socket);
		perror("Can't create TCP socket\n");
		exit(1);
	}

	//char *ip ="172.67.188.132";
	char *ip = IP;
	printf("The IP: %s\n", ip);

	// setup remote socket
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
	printf("s_addr:%d\n",remote->sin_addr.s_addr);
	remote->sin_addr.s_addr = inet_addr(ip);
	remote->sin_port = htons(PORT);
	printf("s_addr:%d\n",remote->sin_addr.s_addr);

	// connect socket
	if(connect(client_socket, (struct sockaddr *)remote, sizeof(struct sockaddr)) == SOCKET_ERROR){
		closesocket(client_socket);
		perror("Could not connect");
		WSACleanup();
		exit(1);
	}

	// prepare query
	byte_buffer_t *get = create_message();
	printf("query: \n%d\n",bb_readable_len(get));

	//Send the query to the server
	int len = bb_readable_len(get);
	int sent = 0;
	char* p = get->data + get->rpos;
	while(sent < len) {
		tmpres = send(client_socket, p + sent, len - sent, 0);
		if(tmpres == -1) {
			perror("Can't send query");
			exit(1);
		}
		sent += tmpres;
	}

	//now it is time to receive the page
	byte_buffer_t *readBuf = bb_allocate(BUFSIZ);
	while((tmpres = recv(client_socket, buf, BUFSIZ, 0)) > 0) {
		bb_put_chars(readBuf,buf,tmpres);
		jm_msg_t *msg = decode_message(readBuf);
		assert(msg != NULL);
		break;
	}

	free(get);
	free(remote);
	closesocket(client_socket);
	WSACleanup();

	printf("\nProgram end");

	return 0;
}

byte_buffer_t *create_message() {
	char *pro = "{\"args\":[\"netty\"],\"params\":{\"NCR\":\"\"}}";
	size_t pdlen = strlen(pro);

	byte_buffer_t *payload = bb_allocate(pdlen);

	assert(bb_put_chars(payload, pro, pdlen));

	//, "fnvHash1a", PROTOCOL_JSON, PROTOCOL_JSON
	jm_msg_t *msg = msg_create_rpc_msg(-655376287, payload);
	assert(msg != NULL);

	byte_buffer_t *buf = bb_allocate(1024);
	assert(buf != NULL);

	assert(msg_encode(msg,buf));

	//jm_msg_t *dmsg = decode_message(buf);

	msg_release(msg);

	//bb_free(buf);
	//msg_release(dmsg);
	//msg_release(msg);
	return buf;

}

jm_msg_t *decode_message(byte_buffer_t *buf) {

	//jm_msg_t *dmsg = msg_decode(buf);
	jm_msg_t *dmsg = msg_readMessage(buf);
	if(dmsg == NULL) {
		return NULL;
	}
	assert(dmsg != NULL);
	//assert(dmsg->payload != NULL);

	//bb_free(buf);
	//msg_release(dmsg);
	//msg_release(dmsg);
	return dmsg;
}
