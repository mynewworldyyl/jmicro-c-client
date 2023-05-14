#include <stdio.h>
#include <winsock2.h>

#include "../jm_msg.h"
#include "test.h"

static int client_socket = 0;
static struct sockaddr_in *remote;

//char *get;

static int READ_BUF_SIZE = BUFSIZ*1;

static char buf[BUFSIZ];

static byte_buffer_t * readBuf;

client_send_msg_result_t udp_client_ws_send_msg(byte_buffer_t *buf, char* host, uint16_t port) {

	//Send the query to the server
	int len = bb_readable_len(buf);
	if(len <= 0) {
		return NO_DATA_TO_SEND;
	}

	char *p = (char*)os_zalloc(1+sizeof(char)*len);
	if(p == NULL) {
		return MEMORY_OUTOF_RANGE;
	}

	if(!bb_get_chars(buf,p,len)) {
		os_free(p);
		return NO_DATA_TO_SEND;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(host);

	int iResult = sendto(client_socket, p, len, 0,(SOCKADDR *)&addr,sizeof(addr));

	if (iResult == SOCKET_ERROR) {
		printf("sendto failed with error: %d\n", WSAGetLastError());
		closesocket(client_socket);
		WSACleanup();
		os_free(p);
		return NO_DATA_TO_SEND;
	}


	os_free(p);
	//bb_move_forward(buf,len);

	return JM_SUCCESS;
}

BOOL udp_client_recv_one_loop() {

	//接收数据
	//char buf[1024];
	struct sockaddr_in clientAddr;
	int nLen = sizeof(clientAddr);

	int tmpres = recvfrom(client_socket, buf, READ_BUF_SIZE, 0, (struct sockaddr*)&clientAddr, &nLen);
	if (tmpres < 0){
		sleep(1);
		return FALSE;
	}

	INFO("receive from: %s, port:%d", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);

	//printChars(buf,tmpres);
	if(!bb_put_chars(readBuf,buf,tmpres)){
		INFO("byte_buffer writeable len: %d, read len:%d",bb_writeable_len(readBuf),tmpres);
		bb_clear(readBuf);
		return FALSE;
	}

	while(TRUE) {

		jm_msg_t *msg = msg_readMessage(readBuf);

		if(!msg) {
			return TRUE;
		}

		//远程UDP客户端和端口，用于响应信息
		msg->extraMap = extra_putInt(msg->extraMap, EXTRA_KEY_UDP_PORT, clientAddr.sin_port);
		char *p = inet_ntoa(clientAddr.sin_addr);
		msg->extraMap = extra_putChars(msg->extraMap, EXTRA_KEY_UDP_HOST, p, strlen(p));
		msg->extraMap = extra_putBool(msg->extraMap,EXTRA_KEY_UDP_ACK,TRUE);

		msg_setUdp(msg,TRUE);

		sint8_t c = client_onMessage(msg);
		//int c = 0;
		if(c != JM_SUCCESS) {
			printf("handle msg result code: %d\n",c);
		}

		if(msg->payload) {
			bb_release(msg->payload);
			msg->payload = NULL;
		}
		os_free(msg);
	}

	return TRUE;

}

int udp_client_ws_init() {

	printf("client_ws_init udp Socket client init\n");

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// create tcp socket
	client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client_socket == INVALID_SOCKET)
	{
		printf("Failed socke():%d\n",client_socket);
		return 0;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9092);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	//绑定套接字到一个本地地址
	if (bind(client_socket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR){
		printf("Failed bind()");
		return 0;
	}

	if(!client_registP2PMessageSender(udp_client_ws_send_msg)) {
		perror("Regist p2p sender fail");
		exit(1);
	}

	readBuf = bb_create(READ_BUF_SIZE);
}

