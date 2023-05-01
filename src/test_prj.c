/*
 ============================================================================
 Name        : test_prj.c
 Author      : yyl
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

extern void test_hashmap();
extern void test_byte_buffer();
extern int testcJson(void);
extern int testMsg(void);
extern int test_socket();
extern int test_jm_client();
extern int test_jm_client_rpc();
extern int test_jm_client_pubsub();

extern int test_stdio_method();

extern void testGetCacheItem();

int main(char* argv, int argc)
{
	//test_hashmap();
	//test_byte_buffer();
	//testcJson();
	//testParsecJson();
	testMsg();
	//test_socket();
	//test_jm_client();
	//test_jm_client_rpc();
	//test_jm_client_pubsub();
	//test_stdio_method();
	//testGetCacheItem();
}
