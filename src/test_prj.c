/*
 ============================================================================
 Name        : test_prj.c
 Author      : yyl
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>

extern void test_hashmap();
extern void test_byte_buffer();
extern int testcJson(void);
extern int testMsg(void);
extern int test_socket();
extern int test_jm_client();
extern int test_jm_client_rpc();
void test_jm_client_pubsub(void* Param);

extern int test_stdio_method();

extern void testGetCacheItem();
extern void test_udp_server();

int test_udp_jm_client_pubsub();

int test_ctrl_jm();

void create_thread();

void start_jm();

int main(char* argv, int argc)
{
	setbuf(stdout,NULL);
	//test_hashmap();
	//test_byte_buffer();
	//testcJson();
	//testParsecJson();
	//testMsg();
	//test_socket();
	//test_jm_client();
	//test_jm_client_rpc();
	//test_jm_client_pubsub();
	//test_stdio_method();
	//testGetCacheItem();

	//test_udp_server();

	//test_udp_jm_client_pubsub();
	//test_ctrl_jm();
	//create_thread();

	start_jm();
}
