/*
 * test_mul_thread.c
 *
 *  Created on: 2023年5月19日
 *      Author: yeyulei
 */
#include<pthread.h>
#include<Windows.h>
#pragma comment(lib, "pthreadVC2.lib")  //必须加上这句


void*Function_t(void* Param)
{
     pthread_t myid = pthread_self();
     while(1)
     {
         printf("线程ID=%d \n", myid);
         Sleep(4000);
     }
     return NULL;
}

int create_thread()
{
     pthread_t pid;
     pthread_create(&pid, NULL, Function_t,NULL);
     while (1)
     {
         printf("in fatherprocess!\n");
         Sleep(2000);
     }
     getchar();
     return 1;
}

