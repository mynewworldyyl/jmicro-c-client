#ifndef JM_CAHCE_H_
#define JM_CAHCE_H_

#include "c_types.h"

typedef struct _jm_hash_map_item {
	char *key; //KEY
	void* val; //Value
	struct _jm_hash_map_item* next; //hash 冲突时链表
} jm_hash_map_item_t;

typedef struct _jm_hash_map {
	uint32_t cap;//初始容量，也就是数组的大小
	jm_hash_map_item_t **arr;//第一层数组
	uint32_t size;//元素个数
} jm_hash_map_t;

typedef struct _jm_cache {
	void* item;
	BOOL used;//选项是否在使用中
	struct _jm_cache* next;
} jm_cache_t;

typedef struct _jm_cache_header {
	size_t memSize;//单个缓存项占内存大小，用于申请内存
	size_t eleNum;//缓存元素数量
	jm_cache_t* cache;
} jm_cache_header_t;

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_getItem(jm_hash_map_t *map, char *cacheName, uint32_t idx);
//释放回缓存
ICACHE_FLASH_ATTR BOOL hashmap_put(jm_hash_map_t *map, char *cacheName, void *extra);
ICACHE_FLASH_ATTR BOOL hashmap_remove(jm_hash_map_t *map, char *cacheName);
//从缓存中取
ICACHE_FLASH_ATTR void* hashmap_get(jm_hash_map_t *map, char *cacheName);
//元素KEY是否存在
ICACHE_FLASH_ATTR BOOL hashmap_exist(jm_hash_map_t *map, char *cacheName);
ICACHE_FLASH_ATTR jm_hash_map_t* hashmap_create(size_t cap);
ICACHE_FLASH_ATTR void hashmap_release(jm_hash_map_t *map);

/*******************************Hash Map 定义结束***************************************/



/*******************************Cache 定义开始************************************/
//itemSize 缓存项所占内存大小，用于内存分配， 一般通过 sizeof获得
ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, size_t itemSize);
/**
 * 从缓存中取出一个可用的项
 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created);
//归还一个缓存
ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it);


/*******************************Cache 定义结束************************************/

#endif

