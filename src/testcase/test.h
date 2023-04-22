#ifndef JMICRO_MQTT_JM_test_H_
#define JMICRO_MQTT_JM_test_H_

#define ICACHE_FLASH_ATTR

#ifndef NULL
#define NULL (void *)0
#endif

#ifndef os_zalloc
#define os_zalloc(s) malloc((s))
#endif

#ifndef os_free
#define os_free(s) free((s))
#endif

#endif
