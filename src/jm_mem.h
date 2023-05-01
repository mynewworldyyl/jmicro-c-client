#ifndef JM_CAHCE_H_
#define JM_CAHCE_H_

#include "c_types.h"

typedef struct _jm_hash_map_item {
	char *key; //KEY
	void* val; //Value
	struct _jm_hash_map_item* next; //hash ��ͻʱ����
} jm_hash_map_item_t;

typedef struct _jm_hash_map {
	uint32_t cap;//��ʼ������Ҳ��������Ĵ�С
	jm_hash_map_item_t **arr;//��һ������
	uint32_t size;//Ԫ�ظ���
} jm_hash_map_t;

typedef struct _jm_cache {
	void* item;
	BOOL used;//ѡ���Ƿ���ʹ����
	struct _jm_cache* next;
} jm_cache_t;

typedef struct _jm_cache_header {
	size_t memSize;//����������ռ�ڴ��С�����������ڴ�
	size_t eleNum;//����Ԫ������
	jm_cache_t* cache;
} jm_cache_header_t;

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_getItem(jm_hash_map_t *map, char *cacheName, uint32_t idx);
//�ͷŻػ���
ICACHE_FLASH_ATTR BOOL hashmap_put(jm_hash_map_t *map, char *cacheName, void *extra);
ICACHE_FLASH_ATTR BOOL hashmap_remove(jm_hash_map_t *map, char *cacheName);
//�ӻ�����ȡ
ICACHE_FLASH_ATTR void* hashmap_get(jm_hash_map_t *map, char *cacheName);
//Ԫ��KEY�Ƿ����
ICACHE_FLASH_ATTR BOOL hashmap_exist(jm_hash_map_t *map, char *cacheName);
ICACHE_FLASH_ATTR jm_hash_map_t* hashmap_create(size_t cap);
ICACHE_FLASH_ATTR void hashmap_release(jm_hash_map_t *map);

/*******************************Hash Map �������***************************************/



/*******************************Cache ���忪ʼ************************************/
//itemSize ��������ռ�ڴ��С�������ڴ���䣬 һ��ͨ�� sizeof���
ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, size_t itemSize);
/**
 * �ӻ�����ȡ��һ�����õ���
 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created);
//�黹һ������
ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it);


/*******************************Cache �������************************************/

#endif

