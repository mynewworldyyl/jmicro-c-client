/*
 * jm_client_winsocket.c
 *
 *  Created on: 2023��4��17��
 *      Author: yeyulei
 */
#include <stdio.h>
#include <winsock.h>
#include <assert.h>

#include "test.h"

#include "../jm_constants.h"
#include "../jm_msg.h"
#include "../jm_buffer.h"
#include "../jm_client.h"

#define PORT 9092
#define IP "192.168.3.22"
#define USERAGENT "ApOgEE MinGW Socket Client 1.0"

static int client_socket = 0;
static struct sockaddr_in *remote;

//char *get;

static int READ_BUF_SIZE = BUFSIZ*1;

static char buf[BUFSIZ];

static byte_buffer_t * readBuf;

client_send_msg_result_t client_ws_send_msg(byte_buffer_t *buf) {

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

	int tmpres;
	int sent = 0;
	while(sent < len) {
		tmpres = send(client_socket, p + sent, len - sent, 0);
		if(tmpres == -1) {
			os_free(p);
			return SEND_DATA_ERROR;
		}
		sent += tmpres;
	}

	os_free(p);
	//bb_move_forward(buf,len);

	return JM_SUCCESS;
}

void printChars(char *buf, int len){
	char *p = buf;
	for(int i = 0; i < len; i++) {
		printf("%c",*(p+i));
	}
	printf("\n");

}

BOOL client_recv_one_loop() {

	int tmpres = recv(client_socket, buf, BUFSIZ, 0);
	if(tmpres <= 0) {
		sleep(1);
		return false;
	}

	//printChars(buf,tmpres);
	if(!bb_put_chars(readBuf,buf,tmpres)){
		INFO("byte_buffer writeable len: %d, read len:%d",bb_writeable_len(readBuf),tmpres);
		bb_clear(readBuf);
		return false;
	}

	while(true) {

		jm_msg_t *msg = msg_readMessage(readBuf);

		if(!msg) {
			return true;
		}
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

	return true;

}

void client_ws_init() {

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
	printf("s_addr:%l\n", remote->sin_addr.s_addr);
	remote->sin_addr.s_addr = inet_addr(ip);
	remote->sin_port = htons(PORT);
	printf("s_addr:%d\n", remote->sin_addr.s_addr);

	//connect socket
	if(connect(client_socket, (struct sockaddr *)remote, sizeof(struct sockaddr)) == SOCKET_ERROR){
		closesocket(client_socket);
		printf("Could not connect to��%s\n", IP);
		WSACleanup();
		exit(1);
	}

	if(!client_registMessageSender(client_ws_send_msg)) {
		perror("Regist sender fail");
		exit(1);
	}

	readBuf = bb_create(READ_BUF_SIZE);
}
