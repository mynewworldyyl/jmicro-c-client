#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#include "os_type.h"

#define SYS_SECTOR_NUM 6

#define CFG_LOCATION    0x79    /* Please don't change or if you know what you doing */
#define CFG_HOLDER    0x00FF55A4    /* Change this value to load default configurations */

#define JM_HOST            "192.168.3.22" // the IP address or domain name of your MQTT server or MQTT broker ,such as "mqtt.yourdomain.com"
#define JM_PORT            9092    // the listening port of your MQTT server or MQTT broker
#define JM_DEVICE_ID        "testdevice001"    // the ID of yourself, any string is OK,client would use this ID register itself to MQTT server
#define JM_USER            "test00" // your MQTT login name, if MQTT server allow anonymous login,any string is OK, otherwise, please input valid login name which you had registered
#define JM_PASS            "1" // you MQTT login password, same as above
#define STA_SSID "AP_SSID"    // your AP/router SSID to config your device networking
#define STA_PASS "AP_Passwd" // your AP/router password

#define STA_TYPE AUTH_WPA2_PSK
#define DEFAULT_SECURITY    0      // very important: you must config DEFAULT_SECURITY for SSL/TLS
#define MQTT_KEEPALIVE        120 //两小时


typedef struct{
	uint32_t cfg_holder;

	uint8_t sta_ssid[64];//连接的wifi ssid
	uint8_t sta_pwd[64];//wifi 密码
	uint32_t sta_type;//

	uint8_t jm_host[64];//JM后台网关IP
	uint32_t jm_port;//JM后台网关端口

	uint8_t device_id[32];
	uint32_t actId;//设备绑定的账号ID

	uint32_t use_udp;//是否使用UDP与JM平台通信，1：是，0：否，使用UDP后，不再使用

} SYSCFG;

typedef struct {
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

uint32 ICACHE_FLASH_ATTR cfg_lastUserDataSectorNo(void);

void ICACHE_FLASH_ATTR cfg_save();

void ICACHE_FLASH_ATTR cfg_load();

extern SYSCFG sysCfg; //让其他文件可以引入此变量

#endif /* USER_CONFIG_H_ */
