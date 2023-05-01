
#ifndef WIN32
#include <osapi.h>
#include "mem.h"
#endif

#ifdef WIN32
#include "stdio.h"
#include "string.h"
#include "./testcase/test.h"
#endif

#include "jm_mem.h"
#include <stddef.h>
#include "debug.h"

#include "jm_stdcimpl.h"

#define SIZE 11 //初始容量

//缓存Map
static jm_hash_map_t* cacheMap;

/*******************************Hash Map 定义结束***************************************/

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_getItem(jm_hash_map_t *map, char *cacheName, uint32_t idx) {
	if(map->arr[idx] != NULL) {
		jm_hash_map_item_t* it = map->arr[idx];
		while(it != NULL) {
			if(os_strcmp(cacheName,it->key) == 0) {
				return it;//命中元素
			}
			it = it->next;
		}
	}
	return NULL;
}

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_newItem(jm_hash_map_t *map, char *cacheName, uint32_t idx) {
	jm_hash_map_item_t* it = _hashmap_getItem(map,cacheName,idx);
	if(it != NULL) return it;//直接返回已经存在的项

	//出现Hash冲突，需要创建新的元素
	jm_hash_map_item_t* nit = os_zalloc(sizeof(struct _jm_hash_map_item));
	os_memset(nit,0,sizeof(struct _jm_hash_map_item));

	size_t s = os_strlen(cacheName);

	char* keyArr = os_zalloc(s+1); ;
	os_memcpy(keyArr, cacheName, s);
	keyArr[s] = '\0';

	nit->next = map->arr[idx];
	map->arr[idx] = nit;//相当于表头插入
	nit->key = keyArr;

	map->size++;

	return nit;
}

ICACHE_FLASH_ATTR static uint32_t _hashmap_idx(jm_hash_map_t *map, char *cacheName) {
	uint32_t h = jm_hash32(cacheName, os_strlen(cacheName));
	return h % map->cap;
}

ICACHE_FLASH_ATTR static void _hashmap_release_item(jm_hash_map_item_t* it) {
	if(it->key) {
		os_free(it->key);
		it->key = NULL;
	}
	os_free(it);
}

//释放回缓存
ICACHE_FLASH_ATTR BOOL hashmap_put(jm_hash_map_t *map, char *cacheName, void *extra){
	uint32_t idx = _hashmap_idx(map,cacheName);
	jm_hash_map_item_t* it = _hashmap_getItem(map,cacheName, idx);
	if(it == NULL) {
		it = _hashmap_newItem(map,cacheName, idx);
		if(it == NULL) {
			//内存溢出
			INFO("cache_put mof create cache: %s", cacheName);
			return false;
		}
	}
	//存入值
	it->val = extra;
	return true;
}

ICACHE_FLASH_ATTR BOOL hashmap_remove(jm_hash_map_t *map, char *cacheName){
	uint32_t idx = _hashmap_idx(map,cacheName);
	if(map->arr[idx] == NULL) return false;//删除不存在的元素
	if(map->arr[idx]->next == NULL) {
		if(os_strcmp(cacheName, map->arr[idx]->key) == 0) {
			//删除最后一个元素
			_hashmap_release_item(map->arr[idx]);
			map->arr[idx] = NULL;
			map->size--;
			return true;
		}
		return false;
	}

	//hash冲突
	jm_hash_map_item_t* it = map->arr+idx;
	jm_hash_map_item_t* pre = NULL;//当前元素的前一个元素

	while(it != NULL) {
		if(os_strcmp(cacheName, it->key) == 0) {
			if(pre == NULL) {
				//第一个元素
				map->arr[idx] = it->next;
			} else {
				pre->next = it->next;
			}

			it->next = NULL;
			_hashmap_release_item(it);
			map->size--;
			return true;
		} else {
			pre = it;
			it =  it->next;
		}
	}

	return false;
}

//从缓存中取
ICACHE_FLASH_ATTR void* hashmap_get(jm_hash_map_t *map, char *cacheName){
	uint32_t idx = _hashmap_idx(map,cacheName);
	jm_hash_map_item_t* it = _hashmap_getItem(map, cacheName, idx);
	if(it != NULL) {
		return it->val;
	}
	return NULL;
}

//元素KEY是否存在
ICACHE_FLASH_ATTR BOOL hashmap_exist(jm_hash_map_t *map, char *cacheName){
	uint32_t idx = _hashmap_idx(map, cacheName);
	return _hashmap_getItem(map, cacheName, idx) != NULL;
}

ICACHE_FLASH_ATTR jm_hash_map_t* hashmap_create(size_t cap){
	jm_hash_map_t *map = os_zalloc(sizeof(struct _jm_hash_map));
	os_memset(map,0,sizeof(struct _jm_hash_map));

	if(cap <= 0) {
		INFO("hashmap_create Invalid cap value %d, use default: %d\n",cap,SIZE);
		cap = SIZE;
	}

	map->arr = os_zalloc(cap * sizeof(struct _jm_hash_map_item*));//返回指针数组首地址
	os_memset(map->arr,0,cap * sizeof(struct _jm_hash_map_item*));

	map->cap = cap;
	map->size = 0;
	return map;
}

ICACHE_FLASH_ATTR void hashmap_release(jm_hash_map_t *map){
	for(int idx = 0; idx < map->cap; idx++) {
		jm_hash_map_item_t* it = map->arr+idx, *next = NULL;
		while(it != NULL) {
			next = it->next;
			it->next = NULL;
			_hashmap_release_item(it);
			it = next;
		}
		*(map->arr+idx) = NULL;
	}
	os_free(map->arr);
	map->arr = NULL;
	os_free(map);
}

/*******************************Hash Map 定义结束***************************************/

/*******************************Cache 定义开始************************************/

ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, size_t itemSize){
	if(cacheMap == NULL) {
		cacheMap = hashmap_create(SIZE);//第一级Map
		if(cacheMap == NULL) {
			INFO("cache_init global cache fail: %s, %d\n",cacheName);
			return false;
		}
	}

	if(hashmap_exist(cacheMap, cacheName)) {
		INFO("cache_init exist cache: %s\n",cacheName);
		return false;//缓存已经存在同名缓存
	}

	jm_cache_header_t* ch = os_zalloc(sizeof(struct _jm_cache_header));
	if(ch == NULL) {
		INFO("cache_init create header fail: %s\n",cacheName);
		return false;
	}
	os_memset(ch,0,sizeof(struct _jm_cache_header));
	ch->cache = NULL;
	ch->memSize = itemSize;
	ch->eleNum = 0;

	if(!hashmap_put(cacheMap,cacheName,ch)) {
		INFO("cache_init fail to put cache: %s\n",cacheName);
		os_free(ch);
		return false;//缓存已经存在
	}

	return true;
}

ICACHE_FLASH_ATTR static jm_cache_t* _cache_createCacheItem(jm_cache_header_t *ch, char *cn){
	jm_cache_t* c = os_zalloc(sizeof(struct _jm_cache));
	if(c == NULL) {
		INFO("cache_get create cache item fail: %s\n",cn);
		return false;
	}
	os_memset(c,0,sizeof(struct _jm_cache));

	jm_cache_t* it = os_zalloc(ch->memSize);
	if(it == NULL) {
		INFO("cache_get create item fail: %s\n",cn);
		os_free(c);
		return false;
	}
	os_memset(it,0,ch->memSize);

	c->item = it;//业务数据节点

	//采用头部插入
	c->next = ch->cache;
	ch->cache = c;
	return c;
}

/**
 * 从缓存中取出一个可用的项
 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created){
	if(cacheMap == NULL) {
		INFO("cache_get fail: %s",cacheName);
		return NULL;
	}

	jm_cache_header_t *ch = hashmap_get(cacheMap, cacheName);

	if(!ch) {
		INFO("cache_get cache not exist: %s\n",cacheName);
		return NULL;//缓存还不存在
	}

	if(ch->cache) {
		//有缓存，查找一个未使用的项
		jm_cache_t *c = ch->cache, *pre = NULL;
		while(c) {
			if(!c->used) {
				break;
			} else {
				pre = c;
				c = c->next;
			}
		}

		if(c) {
			os_memset(c->item,0,ch->memSize);//重置内存为0
			c->used = true;
			return c->item;
		}
	}

	//第一次创建缓存项目
	INFO("cache_get cache element is NULL: %s\n",cacheName);
	if(!created) {
		INFO("cache_get no cache item to found: %s\n",cacheName);
		return NULL;
	}

	jm_cache_t* c = _cache_createCacheItem(ch,cacheName);
	if(c) {
		c->used = true;
		return c->item;
	} else {
		INFO("cache_get create cache item fail: %s\n",cacheName);
		return NULL;
	}
}

//归还一个缓存
ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it){
	if(cacheMap == NULL) {
		INFO("cache_back cache is NULL: %s",cacheName);
		return false;
	}

	jm_cache_header_t *ch = hashmap_get(cacheMap, cacheName);

	if(!ch) {
		INFO("cache_back cache not exist: %s\n",cacheName);
		return false;//缓存还不存在
	}

	jm_cache_t* c = ch->cache;
	while(c) {
		if(c->item == it) {
			c->used = false;
			INFO("cache_back back item success: %s\n",cacheName);
			return true;
		}
		c = c->next;
	}

	INFO("cache_back back item not exist: %s\n",cacheName);
	return false;//缓存还不存在

}


/*******************************Cache 定义结束************************************/
