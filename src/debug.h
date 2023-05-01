#ifndef USER_DEBUG_H_
#define USER_DEBUG_H_

#if defined(GLOBAL_DEBUG_ON)
#define MQTT_DEBUG_ON
#endif
#if defined(MQTT_DEBUG_ON)
#define INFO( format, ... ) printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif
// #ifndef INFO
// #define INFO os_printf
// #endif

#define INFO( format, ... ) printf( format, ## __VA_ARGS__ )

#endif /* USER_DEBUG_H_ */
