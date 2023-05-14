#ifndef JM_CAHCE_H_
#define JM_CAHCE_H_

#include "c_types.h"

#define CACHE_PUBSUB_ITEM "C_PS_ITEM_"
#define CACHE_PUBSUB_ITEM_EXTRA "C_PS_ITEM_EXTRA_"

#define CACHE_MESSAGE "C_MESSAGE_"
#define CACHE_MESSAGE_EXTRA "C_MESSAGE_EXTRA_"

typedef struct _jm_hash_map_item {
	char *key; //KEY
	void* val; //Value
	struct _jm_hash_map_item* next; //hash
} jm_hash_map_item_t;

typedef struct _jm_hash_map {
	uint32_t cap;
	jm_hash_map_item_t **arr;
	uint32_t size;
} jm_hash_map_t;

typedef struct _jm_cache {
	void* item;
	BOOL used;//
	struct _jm_cache* next;
} jm_cache_t;

typedef struct _jm_cache_header {
	size_t memSize;//
	size_t eleNum;//
	jm_cache_t* cache;
} jm_cache_header_t;


#ifdef __cplusplus
extern "C" {
#endif

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_getItem(jm_hash_map_t *map, char *cacheName, uint32_t idx);

ICACHE_FLASH_ATTR BOOL hashmap_put(jm_hash_map_t *map, char *cacheName, void *extra);
ICACHE_FLASH_ATTR BOOL hashmap_remove(jm_hash_map_t *map, char *cacheName);

ICACHE_FLASH_ATTR void* hashmap_get(jm_hash_map_t *map, char *cacheName);

ICACHE_FLASH_ATTR BOOL hashmap_exist(jm_hash_map_t *map, char *cacheName);

ICACHE_FLASH_ATTR jm_hash_map_t* hashmap_create(size_t cap);
ICACHE_FLASH_ATTR void hashmap_release(jm_hash_map_t *map);

/*******************************Hash Map***************************************/



/*******************************Cache************************************/
//itemSize
ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, size_t itemSize);
/**

 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created);
//
ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it);


/*******************************Cache************************************/


#ifdef __cplusplus
}
#endif

#endif

