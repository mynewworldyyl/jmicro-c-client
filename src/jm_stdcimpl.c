#include "jm_stdcimpl.h"

#define FNV_64_INIT 0xcbf29ce484222325L
#define FNV_64_PRIME 0x100000001b3L

#define FNV_32_INIT 0x811c9dc5
#define FNV_32_PRIME 0x01000193

#ifndef WIN32
#include "user_interface.h"
#include "osapi.h"
#include "ctype.h"
#endif

#ifdef WIN32
#include "stdio.h"
#include "string.h"
#include "./testcase/test.h"
#endif

#include "mem.h"
#include "debug.h"

typedef char *va_list;

//#include <stdarg.h>
//#include <string.h>

#define va_start(ap,p) (ap = (char *) (&(p)+1))
#define va_arg(ap, type) ((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)

ICACHE_FLASH_ATTR uint32_t jm_hash32(char* data, size_t len) {
	uint32_t rv = FNV_32_INIT;
	for (int i = 0; i < len; i++) {
		rv = rv ^ data[i];
		rv = rv * FNV_32_PRIME;
	}
	return rv;
}

ICACHE_FLASH_ATTR uint64_t jm_hash64(char* k, size_t len) {
	uint64_t rv = FNV_64_INIT;
	for (int i = 0; i < len; i++) {
		rv ^= k[i];
		rv *= FNV_64_PRIME;
	}
	return rv;
}

/*���ܣ����ַ��� ��ʽ����ӡһ���ַ���
 *��������ʽ�����ַ���
 *ע�⣺����Ǽ��װ汾 (%02x ���)
 * %-3s���У� %fҲ���У� %X����
 */
ICACHE_FLASH_ATTR int jm_sprintf(char * str, const char *fmt, ...) {
	int count = 0;
	char c;
	char *s;
	int n;

	int index = 0;
	int ret = 2;

	char buf[65];
	char digit[16];
	int num = 0;
	int len = 0;

	jm_memset(buf, 0, sizeof(buf));
	jm_memset(digit, 0, sizeof(digit));

	va_list ap;

	va_start(ap, fmt);

	while (*fmt != '\0') {
		//printf("*fmt=[%c]\n", *fmt);
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
			case 'd': /*����*/
			{
				n = va_arg(ap, int);
				if (n < 0) {
					*str = '-';
					str++;
					n = -n;
				}
				//printf("case d n=[%d]\n", n);
				jm_itoa(n, buf);
				//printf("case d buf=[%s]\n", buf);
				os_memcpy(str, buf, jm_strlen(buf));
				str += jm_strlen(buf);
				break;
			}
			case 'c': /*�ַ���*/
			{
				c = va_arg(ap, int);
				*str = c;
				str++;

				break;
			}
			case 'x': /*16����*/
			{
				n = va_arg(ap, int);
				jm_xtoa(n, buf);
				os_memcpy(str, buf, jm_strlen(buf));
				str += jm_strlen(buf);
				break;
			}
			case 's': /*�ַ���*/
			{
				s = va_arg(ap, char *);
				os_memcpy(str, s, jm_strlen(s));
				str += jm_strlen(s);
				break;
			}
			case '%': /*���%*/
			{
				*str = '%';
				str++;

				break;
			}
			case '0': /*λ�������0*/
			{
				index = 0;
				num = 0;
				jm_memset(digit, 0, sizeof(digit));

				while (1) {
					fmt++;
					ret = jm_isDigit(*fmt);
					if (ret == 1) //������
							{
						digit[index] = *fmt;
						index++;
					} else {
						num = jm_atoi(digit);
						break;
					}
				}
				switch (*fmt) {
				case 'd': /*����*/
				{
					n = va_arg(ap, int);
					if (n < 0) {
						*str = '-';
						str++;
						n = -n;
					}
					jm_itoa(n, buf);
					len = jm_strlen(buf);
					if (len >= num) {
						os_memcpy(str, buf, jm_strlen(buf));
						str += jm_strlen(buf);
					} else {
						jm_memset(str, '0', num - len);
						str += num - len;
						memcpy(str, buf, jm_strlen(buf));
						str += jm_strlen(buf);
					}
					break;
				}
				case 'x': /*16����*/
				{
					n = va_arg(ap, int);
					jm_xtoa(n, buf);
					len = jm_strlen(buf);
					if (len >= num) {
						memcpy(str, buf, len);
						str += len;
					} else {
						jm_memset(str, '0', num - len);
						str += num - len;
						memcpy(str, buf, len);
						str += len;
					}
					break;
				}
				case 's': /*�ַ���*/
				{
					s = va_arg(ap, char *);
					len = jm_strlen(s);
					if (len >= num) {
						memcpy(str, s, jm_strlen(s));
						str += jm_strlen(s);
					} else {
						jm_memset(str, '0', num - len);
						str += num - len;
						memcpy(str, s, jm_strlen(s));
						str += jm_strlen(s);
					}
					break;
				}
				default:
					break;
				}
			}
			break;
			default:
				break;
			}
		} else {
			*str = *fmt;
			str++;

			if (*fmt == '\n') {

			}
		}
		fmt++;
	}

	va_end(ap);

	return count;
}

/*
 *���ܣ�����(int) ת���� �ַ���(char)
 *ע�⣺���� % / ���ŵĻ���ֻ����ȷ��ӡ:0...9�����ֶ�Ӧ���ַ�'0'...'9'
 */
ICACHE_FLASH_ATTR void jm_itoa(unsigned int n, char * buf) {
	int i;

	if (n < 10) {
		buf[0] = n + '0';
		buf[1] = '\0';
		return;
	}
	jm_itoa(n / 10, buf);

	for (i = 0; buf[i] != '\0'; i++)
		;

	buf[i] = (n % 10) + '0';

	buf[i + 1] = '\0';
}

ICACHE_FLASH_ATTR long jm_atoi(char* pstr) {
	long int_ret = 0;
	long int_sign = 1;

	if (pstr == NULL)
	{
		return -1;
	}
	while (((*pstr) == ' ') || ((*pstr) == '\n') || ((*pstr) == '\t')
			|| ((*pstr) == '\b')) {
		pstr++;
	}

	if (*pstr == '-') {
		int_sign = -1;
	}
	if (*pstr == '-' || *pstr == '+') {
		pstr++;
	}

	while (*pstr >= '0' && *pstr <= '9')
	{
		int_ret = int_ret * 10 + *pstr - '0';
		pstr++;
	}
	int_ret = int_sign * int_ret;

	return int_ret;
}

ICACHE_FLASH_ATTR void jm_xtoa(unsigned int n, char * buf) {
	int i;

	if (n < 16) {
		if (n < 10) {
			buf[0] = n + '0';
		} else {
			buf[0] = n - 10 + 'a';
		}
		buf[1] = '\0';
		return;
	}
	jm_xtoa(n / 16, buf);

	for (i = 0; buf[i] != '\0'; i++)
		;

	if ((n % 16) < 10) {
		buf[i] = (n % 16) + '0';
	} else {
		buf[i] = (n % 16) - 10 + 'a';
	}
	buf[i + 1] = '\0';
}

/*
 * �ж�һ���ַ��Ƿ�����
 */
ICACHE_FLASH_ATTR int jm_isDigit(unsigned char c) {
	if (c >= '0' && c <= '9')
		return 1;
	else
		return 0;
}

/*
 * �ж�һ���ַ��Ƿ�Ӣ����ĸ
 */
ICACHE_FLASH_ATTR int jm_isLetter(unsigned char c) {
	if (c >= 'a' && c <= 'z')
		return 1;
	else if (c >= 'A' && c <= 'Z')
		return 1;
	else
		return 0;
}

/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
ICACHE_FLASH_ATTR void * jm_memset(void * s, int c, unsigned int count) {
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

/**
 * strcpy - Copy a %NUL terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 */
ICACHE_FLASH_ATTR char * jm_strcpy(char * dest, const char *src) {
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

/**
 * jm_strlen - Find the length of a string
 * @s: The string to be sized
 */
ICACHE_FLASH_ATTR unsigned int jm_strlen(const char * s) {
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/**
 * strcat - Append one %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 */
ICACHE_FLASH_ATTR char * jm_strcat(char * dest, const char * src) {
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

/*
 * Private functions.
 */

ICACHE_FLASH_ATTR static int is_space(char c) {
	return (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r'
			|| c == '\n');
}

ICACHE_FLASH_ATTR static char* skip_spaces(const char *str) {
	while (is_space(*str)) {
		++str;
	}
	return (char*) str;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* dec_to_signed(const char *str, long *out) {
	const char * cur = skip_spaces(str);
	long value = 0;
	int isneg = 0, isempty = 1;
	if (cur[0] == '+') {
		cur += 1;
	} else if (cur[0] == '-') {
		cur += 1;
		isneg = 1;
	}
	while (*cur != '\0' && *cur >= '0' && *cur <= '9') {
		value = (value * 10) + (*cur - '0');
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	if (isneg) {
		*out = -value;
	} else {
		*out = value;
	}
	return (char*) cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* dec_to_unsigned(const char *str,
		unsigned long *out) {
	const char * cur = skip_spaces(str);
	unsigned long value = 0;
	int isempty = 1;
	while (*cur != '\0' && *cur >= '0' && *cur <= '9') {
		value = (value * 10) + (*cur - '0');
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	*out = value;
	return (char*) cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* hex_to_signed(const char *str, long *out) {
	const char * cur = skip_spaces(str);
	long value = 0;
	int isneg = 0, isempty = 1;
	if (cur[0] == '+') {
		cur += 1;
	} else if (cur[0] == '-') {
		cur += 1;
		isneg = 1;
	}
	if (cur[0] == '0' && cur[1] == 'x') {
		cur += 2;
	}
	while (*cur != '\0') {
		if (*cur >= '0' && *cur <= '9') {
			value = (value * 16) + (*cur - '0');
		} else if (*cur >= 'a' && *cur <= 'f') {
			value = (value * 16) + 10 + (*cur - 'a');
		} else if (*cur >= 'A' && *cur <= 'F') {
			value = (value * 16) + 10 + (*cur - 'A');
		} else {
			break;
		}
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	if (isneg) {
		*out = -value;
	} else {
		*out = value;
	}
	return (char*) cur;
}

/* Returns a pointer after the last read char, or 'str' on error. */
ICACHE_FLASH_ATTR static char* hex_to_unsigned(const char *str,
		unsigned long *out) {
	const char * cur = skip_spaces(str);
	unsigned long value = 0;
	int isempty = 1;
	if (cur[0] == '0' && cur[1] == 'x') {
		cur += 2;
	}
	while (*cur != '\0') {
		if (*cur >= '0' && *cur <= '9') {
			value = (value * 16) + (*cur - '0');
		} else if (*cur >= 'a' && *cur <= 'f') {
			value = (value * 16) + 10 + (*cur - 'a');
		} else if (*cur >= 'A' && *cur <= 'F') {
			value = (value * 16) + 10 + (*cur - 'A');
		} else {
			break;
		}
		isempty = 0;
		++cur;
	}
	if (isempty) {
		return (char*) str;
	}
	*out = value;
	return (char*) cur;
}

#define MFMT_DEC_TO_SIGNED(TYPE, NAME)                          \
ICACHE_FLASH_ATTR static char*                                                    \
dec_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        long v;                                                 \
        char *cur = dec_to_signed(str, &v);                     \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

#define MFMT_DEC_TO_UNSIGNED(TYPE, NAME)                        \
ICACHE_FLASH_ATTR static char*                                                    \
dec_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        unsigned long v;                                        \
        char *cur = dec_to_unsigned(str, &v);                   \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

#define MFMT_HEX_TO_SIGNED(TYPE, NAME)                                  \
ICACHE_FLASH_ATTR static char*                                                            \
hex_to_##NAME(const char *str, TYPE *out)                               \
{                                                                       \
        long v;                                                         \
        char *cur = hex_to_signed(str, &v);                             \
        if (cur != str){                                                \
                *out = (TYPE)v;                                         \
        }                                                               \
        return cur;                                                     \
}

#define MFMT_HEX_TO_UNSIGNED(TYPE, NAME)                        \
ICACHE_FLASH_ATTR static char*                                                    \
hex_to_##NAME(const char *str, TYPE *out)                       \
{                                                               \
        unsigned long v;                                        \
        char *cur = hex_to_unsigned(str, &v);                   \
        if (cur != str){                                        \
                *out = (TYPE)v;                                 \
        }                                                       \
        return cur;                                             \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_SIGNED_TO_HEX(TYPE, NAME)                                  \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_hex(TYPE val, int uppercase, char padchar, size_t padlen,     \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t isneg = 0, cnt = 0;                                      \
        if (uppercase){                                                 \
                uppercase = 'A';                                        \
        }else{                                                          \
                uppercase = 'a';                                        \
        }                                                               \
        if (val < 0){                                                   \
                isneg = 1;                                              \
                val = -val;                                             \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 16;                                  \
                val = val / 16;                                         \
        }while (val != 0);                                              \
        if (padlen > isneg + cnt){                                      \
                padlen -= isneg + cnt;                                  \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        if (isneg && len > 0){                                          \
                str[0] = '-';                                           \
                str += 1;                                               \
                len -= 1;                                               \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                if (buf[cnt] < 10){                                     \
                        *str = buf[cnt] + '0';                          \
                }else{                                                  \
                        *str = (buf[cnt] - 10) + uppercase;             \
                }                                                       \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_SIGNED_TO_DEC(TYPE, NAME)                                  \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_dec(TYPE val, char padchar, size_t padlen,                    \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t isneg = 0, cnt = 0;                                      \
        if (val < 0){                                                   \
                isneg = 1;                                              \
                val = -val;                                             \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 10;                                  \
                val = val / 10;                                         \
        }while (val != 0);                                              \
        if (padlen > isneg + cnt){                                      \
                padlen -= isneg + cnt;                                  \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        if (isneg && len > 0){                                          \
                str[0] = '-';                                           \
                str += 1;                                               \
                len -= 1;                                               \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                *str = buf[cnt] + '0';                                  \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_UNSIGNED_TO_HEX(TYPE, NAME)                                \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_hex(TYPE val, int uppercase, char padchar, size_t padlen,     \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t cnt = 0;                                                 \
        if (uppercase){                                                 \
                uppercase = 'A';                                        \
        }else{                                                          \
                uppercase = 'a';                                        \
        }                                                               \
        do{                                                             \
                buf[cnt++] = val % 16;                                  \
                val = val / 16;                                         \
        }while (val != 0);                                              \
        if (padlen > cnt){                                              \
                padlen -= cnt;                                          \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                if (buf[cnt] < 10){                                     \
                        *str = buf[cnt] + '0';                          \
                }else{                                                  \
                        *str = (buf[cnt] - 10) + uppercase;             \
                }                                                       \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

/* Returns a pointer after the last written char, or 'str' on error. */
#define MFMT_UNSIGNED_TO_DEC(TYPE, NAME)                                \
ICACHE_FLASH_ATTR static char*                                                            \
NAME##_to_dec(TYPE val, char padchar, size_t padlen,                    \
              size_t len, char *str)                                    \
{                                                                       \
        char buf[24];                                                   \
        size_t cnt = 0;                                                 \
        do{                                                             \
                buf[cnt++] = val % 10;                                  \
                val = val / 10;                                         \
        }while (val != 0);                                              \
        if (padlen > cnt){                                              \
                padlen -= cnt;                                          \
                padlen = (padlen < len ? padlen : len);                 \
                memset(str, padchar, padlen);                           \
                str += padlen;                                          \
                len -= padlen;                                          \
        }                                                               \
        while (cnt-- > 0 && len-- > 0){                                 \
                *str = buf[cnt] + '0';                                  \
                ++str;                                                  \
        }                                                               \
        return str;                                                     \
}

MFMT_DEC_TO_SIGNED(int, int)
MFMT_HEX_TO_SIGNED(int, int)
MFMT_SIGNED_TO_DEC(int, int)
MFMT_SIGNED_TO_HEX(int, int)

MFMT_DEC_TO_UNSIGNED(unsigned int, uint)
MFMT_HEX_TO_UNSIGNED(unsigned int, uint)
MFMT_UNSIGNED_TO_DEC(unsigned int, uint)
MFMT_UNSIGNED_TO_HEX(unsigned int, uint)
MFMT_UNSIGNED_TO_HEX(size_t, siz)

ICACHE_FLASH_ATTR static const char* jm_parse_arg(const char *fmt,
		const char *str, va_list args) {
	int *intp, intv = 0;
	unsigned int *uintp, uintv = 0, width = 0;
	char *charp;
	const char *cur = str;
	fmt = dec_to_uint(fmt, &width);
	if (*fmt == 'd') {
		cur = dec_to_int(str, &intv);
		if (cur != str) {
			intp = va_arg(args, int*);
			*intp = intv;
		}
	} else if (*fmt == 'u') {
		cur = dec_to_uint(str, &uintv);
		if (cur != str) {
			uintp = va_arg(args, unsigned int*);
			*uintp = uintv;
		}
	} else if (*fmt == 'x' || *fmt == 'X') {
		cur = hex_to_uint(str, &uintv);
		if (cur != str) {
			uintp = va_arg(args, unsigned int*);
			*uintp = uintv;
		}
	} else if (*fmt == 'c') {
		charp = va_arg(args, char*);
		if (width == 0) {
			width = 1;
		}
		while (cur[0] != '\0' && uintv < width) {
			charp[uintv] = cur[0];
			++cur;
			++uintv;
		}
	} else if (*fmt == 's') {
		charp = va_arg(args, char*);
		while (cur[0] != '\0' && !is_space(cur[0])
				&& (width == 0 || uintv < width)) {
			charp[uintv] = cur[0];
			++cur;
			++uintv;
		}
		charp[uintv] = '\0';
	} else if (*fmt == '%' && str[0] == '%') {
		++cur;
	}
	return cur;
}

ICACHE_FLASH_ATTR static char* jm_print_arg(const char *fmt, char *str,
		size_t len, va_list args) {
	unsigned int uintv, width = 0;
	size_t charplen = 0, padlen = 0;
	int intv;
	char *charp, padchar = (*fmt == '0' ? '0' : ' ');
	fmt = dec_to_uint(fmt, &width);
	if (*fmt == 'd' || *fmt == 'i') {
		intv = va_arg(args, int);
		str = int_to_dec(intv, padchar, width, len, str);
	} else if (*fmt == 'u') {
		uintv = va_arg(args, unsigned int);
		str = uint_to_dec(uintv, padchar, width, len, str);
	} else if (*fmt == 'x' || *fmt == 'X') {
		uintv = va_arg(args, unsigned int);
		str = uint_to_hex(uintv, (*fmt == 'X'), padchar, width, len, str);
	} else if (*fmt == 'p') {
		charp = (char*) va_arg(args, void*);
		str = siz_to_hex((size_t) charp, 0, padchar, width, len, str);
	} else if (*fmt == 'c') {
		intv = va_arg(args, int);
		if (width > 1) {
			padlen = (size_t) width - 1;
			padlen = (padlen < len ? padlen : len);
			memset(str, ' ', padlen);
			str += padlen;
			len -= padlen;
		}
		if (len > 0) {
			str[0] = (char) intv;
			str += 1;
			len -= 1;
		}
	} else if (*fmt == 's') {
		charp = va_arg(args, char*);
		charplen = strlen(charp);
		if (width > 0 && (size_t) width > charplen) {
			padlen = (size_t) width - charplen;
			padlen = (padlen < len ? padlen : len);
			memset(str, ' ', padlen);
			str += padlen;
			len -= padlen;
		}
		charplen = (charplen < len ? charplen : len);
		memcpy(str, charp, charplen);
		str += charplen;
		len -= charplen;
	} else if (*fmt == '%') {
		str[0] = '%';
		++str;
	}
	return str;
}

/*
 * Public functions.
 */

ICACHE_FLASH_ATTR int jm_print(char *str, size_t len, const char *fmt, ...) {
	va_list args;
	int cnt = 0;
	char *tmp, *cur = str;
	if (len == 0) {
		return cnt;
	}
	--len;
	va_start(args, fmt);
	while (fmt[0] != '\0' && len > 0) {
		if (fmt[0] == '%') {
			tmp = jm_print_arg(&fmt[1], cur, len, args);
			if (tmp == cur) {
				break;
			}
			len -= (tmp - cur);
			cur = tmp;
			++fmt;
			while (fmt[0] >= '0' && fmt[0] <= '9') {
				++fmt;
			}
			++fmt;
		} else {
			cur[0] = fmt[0];
			--len;
			++cur;
			++fmt;
		}
	} va_end(args);
	cnt = (int) (cur - str);
	str[cnt] = '\0';
	return cnt;
}

ICACHE_FLASH_ATTR int jm_scan(const char *str, const char *fmt, ...) {
	int ret = 0;
	va_list args;
	va_start(args, fmt);
	while (fmt[0] != '\0' && str[0] != '\0') {
		if (fmt[0] == '%') {
			const char * tmp = jm_parse_arg(&fmt[1], str, args);
			if (tmp == str) {
				break;
			}
			if (fmt[1] != '%') {
				++ret;
			}
			++fmt;
			while (fmt[0] >= '0' && fmt[0] <= '9') {
				++fmt;
			}
			++fmt;
			str = tmp;
		} else if (is_space(fmt[0])) {
			++fmt;
			str = skip_spaces(str);
		} else if (fmt[0] == str[0]) {
			++fmt;
			++str;
		} else {
			break;
		}
	}

	va_end(args);
	return ret;
}

static int maxExponent = 511;	/* Largest possible base 10 exponent.  Any
				 * exponent larger than this will already
				 * produce underflow or overflow, so there's
				 * no need to worry about additional digits.
				 */
static double powersOf10[] = {	/* Table giving binary powers of 10.  Entry */
    10.,			/* is 10^2^i.  Used to convert decimal */
    100.,			/* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

/*
 *----------------------------------------------------------------------
 *
 * strtod --
 *
 *	This procedure converts a floating-point number from an ASCII
 *	decimal representation to internal double-precision format.
 *
 * Results:
 *	The return value is the double-precision floating-point
 *	representation of the characters in string.  If endPtr isn't
 *	NULL, then *endPtr is filled in with the address of the
 *	next character after the last one that was part of the
 *	floating-point number.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

ICACHE_FLASH_ATTR double jm_strtod(const char * string, char **endPtr)
    /* const char *string;	A decimal ASCII floating-point number,
				 * optionally preceded by white space.
				 * Must have form "-I.FE-X", where I is the
				 * integer part of the mantissa, F is the
				 * fractional part of the mantissa, and X
				 * is the exponent.  Either of the signs
				 * may be "+", "-", or omitted.  Either I
				 * or F may be omitted, or both.  The decimal
				 * point isn't necessary unless F is present.
				 * The "E" may actually be an "e".  E and X
				 * may both be omitted (but not just one).
				 */
    /* char **endPtr;		If non-NULL, store terminating character's
				 * address here. */
{
	int sign, expSign = FALSE;
	double fraction, dblExp, *d;
	register const char *p;
	register int c;
	int exp = 0; /* Exponent read from "EX" field. */
	int fracExp = 0; /* Exponent that derives from the fractional
	 * part.  Under normal circumstatnces, it is
	 * the negative of the number of digits in F.
	 * However, if I is very long, the last digits
	 * of I get dropped (otherwise a long I with a
	 * large negative exponent could cause an
	 * unnecessary overflow on I alone).  In this
	 * case, fracExp is incremented one for each
	 * dropped digit. */
	int mantSize; /* Number of digits in mantissa. */
	int decPt; /* Number of mantissa digits BEFORE decimal
	 * point. */
	const char *pExp; /* Temporarily holds location of exponent
	 * in string. */

	/*
	 * Strip off leading blanks and check for a sign.
	 */

	p = string;
	while (isspace(*p)) {
		p += 1;
	}
	if (*p == '-') {
		sign = TRUE;
		p += 1;
	} else {
		if (*p == '+') {
			p += 1;
		}
		sign = FALSE;
	}

	/*
	 * Count the number of digits in the mantissa (including the decimal
	 * point), and also locate the decimal point.
	 */

	decPt = -1;
	for (mantSize = 0;; mantSize += 1) {
		c = *p;
		if (!isdigit(c)) {
			if ((c != '.') || (decPt >= 0)) {
				break;
			}
			decPt = mantSize;
		}
		p += 1;
	}

	/*
	 * Now suck up the digits in the mantissa.  Use two integers to
	 * collect 9 digits each (this is faster than using floating-point).
	 * If the mantissa has more than 18 digits, ignore the extras, since
	 * they can't affect the value anyway.
	 */

	pExp = p;
	p -= mantSize;
	if (decPt < 0) {
		decPt = mantSize;
	} else {
		mantSize -= 1; /* One of the digits was the point. */
	}
	if (mantSize > 18) {
		fracExp = decPt - 18;
		mantSize = 18;
	} else {
		fracExp = decPt - mantSize;
	}
	if (mantSize == 0) {
		fraction = 0.0;
		p = string;
		goto done;
	} else {
		int frac1, frac2;
		frac1 = 0;
		for (; mantSize > 9; mantSize -= 1) {
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac1 = 10 * frac1 + (c - '0');
		}
		frac2 = 0;
		for (; mantSize > 0; mantSize -= 1) {
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac2 = 10 * frac2 + (c - '0');
		}
		fraction = (1.0e9 * frac1) + frac2;
	}

	/*
	 * Skim off the exponent.
	 */

	p = pExp;
	if ((*p == 'E') || (*p == 'e')) {
		p += 1;
		if (*p == '-') {
			expSign = TRUE;
			p += 1;
		} else {
			if (*p == '+') {
				p += 1;
			}
			expSign = FALSE;
		}
		while (isdigit(*p)) {
			exp = exp * 10 + (*p - '0');
			p += 1;
		}
	}
	if (expSign) {
		exp = fracExp - exp;
	} else {
		exp = fracExp + exp;
	}

	/*
	 * Generate a floating-point number that represents the exponent.
	 * Do this by processing the exponent one bit at a time to combine
	 * many powers of 2 of 10. Then combine the exponent with the
	 * fraction.
	 */

	if (exp < 0) {
		expSign = TRUE;
		exp = -exp;
	} else {
		expSign = FALSE;
	}
	if (exp > maxExponent) {
		exp = maxExponent;
		//errno = 34;
	}
	dblExp = 1.0;
	for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
		if (exp & 01) {
			dblExp *= *d;
		}
	}
	if (expSign) {
		fraction /= dblExp;
	} else {
		fraction *= dblExp;
	}

	done: if (endPtr != NULL) {
		*endPtr = (char *) p;
	}

	if (sign) {
		return -fraction;
	}
	return fraction;
}
