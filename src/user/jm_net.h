
#ifndef JM_NET_H_
#define JM_NET_H_

#define BUFSIZ 512

#define SEND_QUEUE_SIZE 20 //发送队列最大数量

#define MS_FOR_SECONDS 1000

//#define KEEPALIVE 512
//#define JM_RECONNECT_TIMEOUT 3000

#define JM_TASK_PRIO                2
#define JM_TASK_QUEUE_SIZE        1
#define JM_SEND_TIMOUT            5

static int PORT = 9092;
static char *IP = "192.168.3.41";
#define USERAGENT "ApOgEE MinGW Socket Client 1.0"

//因为ESP8266要等前一个消息发送成功后才能发下一个消息，所以发送时将消息存于队列中，在发送成功回调方法检测队列消息发送
typedef struct _jm_tcpclient_send_item {
	char *msg;//将下发送的消息
	uint16_t len;//数据长度
	BOOL use;//选项是否在使用中
	char *targetHost;//UDP目录主机
	uint16_t targetPort;//UDP目录端口
	struct _jm_tcpclient_send_item *next;
} jm_tcpclient_send_item;

typedef enum {
	WIFI_INIT,
	WIFI_CONNECTING, //1
	WIFI_CONNECTING_ERROR,//2
	WIFI_CONNECTED, //3
	DNS_RESOLVE, //4
	TCP_DISCONNECTING, //5
	TCP_DISCONNECTED, //6
	TCP_RECONNECT_DISCONNECTING,//7
	TCP_RECONNECT_REQ, //8
	TCP_RECONNECT,//9
	TCP_CONNECTING,//10
	TCP_CONNECTING_ERROR,//11
	TCP_CONNECTED,//12
	JM_DATA, //13在传输数据中
	JM_IDLE, //14空闲中
	JM_CONNECT_SEND,
	JM_CONNECT_SENDING,
	JM_SUBSCIBE_SEND,
	JM_SUBSCIBE_SENDING,
	JM_KEEPALIVE_SEND,
	JM_PUBLISH_RECV,
	JM_PUBLISHING,
	JM_DELETING,
	JM_DELETED,
} tConnState;



#endif
