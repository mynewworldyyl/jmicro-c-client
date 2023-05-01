#ifndef JMICRO_JM_MSG_MSG_H_
#define JMICRO_JM_MSG_MSG_H_

#include "c_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

ICACHE_FLASH_ATTR double jm_strtod(const char * string, char **endPtr);

ICACHE_FLASH_ATTR char *jm_strcpy(char * dest,const char *src);
ICACHE_FLASH_ATTR char *jm_strcat(char * dest, const char * src);
ICACHE_FLASH_ATTR unsigned int jm_strlen(const char * s);
ICACHE_FLASH_ATTR void * jm_memset(void * s,int c,unsigned int count);
ICACHE_FLASH_ATTR void * jm_memcpy(void * dest,const void *src,unsigned int count);
ICACHE_FLASH_ATTR int jm_sprintf(char * str, const char *fmt, ...);
ICACHE_FLASH_ATTR void jm_itoa(unsigned int n, char * buf);
ICACHE_FLASH_ATTR int jm_atoi(char* pstr);
ICACHE_FLASH_ATTR void jm_xtoa(unsigned int n, char * buf);
ICACHE_FLASH_ATTR int jm_isDigit(unsigned char c);
ICACHE_FLASH_ATTR int jm_isLetter(unsigned char c);

ICACHE_FLASH_ATTR int jm_scan(const char *str, const char *fmt, ...);
ICACHE_FLASH_ATTR int jm_print(char *str, size_t len, const char *fmt, ...);

ICACHE_FLASH_ATTR uint32_t jm_hash32(char* data, size_t len);
ICACHE_FLASH_ATTR uint64_t jm_hash64(char* k, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* JMICRO_JM_MSG_MSG_H_ */
