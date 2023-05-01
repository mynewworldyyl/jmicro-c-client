#ifndef JMICRO_MQTT_JM_test_H_
#define JMICRO_MQTT_JM_test_H_

#define ICACHE_FLASH_ATTR

#ifndef NULL
#define NULL (void *)0
#endif

#define os_zalloc(s) malloc((s))
#define os_free(s) free((s))
#define os_realloc realloc

#define os_memcpy memcpy
#define os_memset memset
#define os_strcpy strcpy
#define os_strncmp strncmp
#define os_strcmp strcmp
#define os_strlen strlen
#define os_strncpy strncpy
#define os_printf printf
#define os_memset memset

#endif

