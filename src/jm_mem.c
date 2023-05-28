
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

#define SIZE 11 //锟斤拷始锟斤拷锟斤拷

//锟斤拷锟斤拷Map
static jm_hash_map_t* cacheMap;

/*******************************Hash Map 锟斤拷锟斤拷锟斤拷锟�***************************************/

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_getItem(jm_hash_map_t *map, char *cacheName, uint32_t idx) {
	if(map->arr[idx] != NULL) {
		jm_hash_map_item_t* it = map->arr[idx];
		while(it != NULL) {
			if(os_strcmp(cacheName,it->key) == 0) {
				return it;//锟斤拷锟斤拷元锟斤拷
			}
			it = it->next;
		}
	}
	return NULL;
}

ICACHE_FLASH_ATTR static jm_hash_map_item_t* _hashmap_newItem(jm_hash_map_t *map, char *cacheName, uint32_t idx) {
	jm_hash_map_item_t* it = _hashmap_getItem(map,cacheName,idx);
	if(it != NULL) return it;//直锟接凤拷锟斤拷锟窖撅拷锟斤拷锟节碉拷锟斤拷

	//锟斤拷锟斤拷Hash锟斤拷突锟斤拷锟斤拷要锟斤拷锟斤拷锟铰碉拷元锟斤拷
	jm_hash_map_item_t* nit = os_zalloc(sizeof(struct _jm_hash_map_item));
	os_memset(nit,0,sizeof(struct _jm_hash_map_item));

	size_t s = os_strlen(cacheName);

	char* keyArr = os_zalloc(s+1); ;
	os_memcpy(keyArr, cacheName, s);
	keyArr[s] = '\0';

	nit->next = map->arr[idx];
	map->arr[idx] = nit;//锟洁当锟节憋拷头锟斤拷锟斤拷
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

ICACHE_FLASH_ATTR BOOL hashmap_put(jm_hash_map_t *map, char *cacheName, void *extra){
	uint32_t idx = _hashmap_idx(map,cacheName);
	jm_hash_map_item_t* it = _hashmap_getItem(map,cacheName, idx);
	if(it == NULL) {
		it = _hashmap_newItem(map, cacheName, idx);
		if(it == NULL) {
			//锟节达拷锟斤拷锟�
			INFO("cache_put mof create cache: %s", cacheName);
			return false;
		}
	}
	//锟斤拷锟斤拷值
	it->val = extra;
	return true;
}

ICACHE_FLASH_ATTR BOOL hashmap_remove(jm_hash_map_t *map, char *cacheName){
	uint32_t idx = _hashmap_idx(map, cacheName);
	if(map->arr[idx] == NULL) return false;//删锟斤拷锟斤拷锟斤拷锟节碉拷元锟斤拷
	if(map->arr[idx]->next == NULL) {
		if(os_strcmp(cacheName, map->arr[idx]->key) == 0) {
			//删锟斤拷锟斤拷锟揭伙拷锟皆拷锟�
			_hashmap_release_item(map->arr[idx]);
			map->arr[idx] = NULL;
			map->size--;
			return true;
		}
		return false;
	}

	//hash锟斤拷突
	jm_hash_map_item_t* it = map->arr+idx;
	jm_hash_map_item_t* pre = NULL;//锟斤拷前元锟截碉拷前一锟斤拷元锟斤拷

	while(it != NULL) {
		if(os_strcmp(cacheName, it->key) == 0) {
			if(pre == NULL) {
				//锟斤拷一锟斤拷元锟斤拷
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

//锟接伙拷锟斤拷锟斤拷取
ICACHE_FLASH_ATTR void* hashmap_get(jm_hash_map_t *map, char *cacheName){
	uint32_t idx = _hashmap_idx(map,cacheName);
	jm_hash_map_item_t* it = _hashmap_getItem(map, cacheName, idx);
	if(it != NULL) {
		return it->val;
	}
	return NULL;
}

//元锟斤拷KEY锟角凤拷锟斤拷锟�
ICACHE_FLASH_ATTR BOOL hashmap_exist(jm_hash_map_t *map, char *cacheName){
	uint32_t idx = _hashmap_idx(map, cacheName);
	return _hashmap_getItem(map, cacheName, idx) != NULL;
}

ICACHE_FLASH_ATTR void* hashmap_iteNext(jm_hash_map_iterator_t *ite){
	if(ite == NULL) {
		INFO("hashmap_iterator iterator is NULL!\n");
		return NULL;
	}

	if(ite->cur == NULL || ite->cur->next == NULL) {
		for(ite->idx += 1; ite->idx < ite->map->cap; ite->idx++) {
			jm_hash_map_item_t* it = *(ite->map->arr + ite->idx);
			if(it != NULL) {
				ite->cur = it;
				return ite->cur->val;
			}
		}
		return NULL;
	} else {
		ite->cur = ite->cur->next;
		return ite->cur->val;
	}
}

ICACHE_FLASH_ATTR jm_hash_map_t* hashmap_create(size_t cap){
	jm_hash_map_t *map = os_zalloc(sizeof(struct _jm_hash_map));
	os_memset(map,0,sizeof(struct _jm_hash_map));

	if(cap <= 0) {
		INFO("hashmap_create Invalid cap value %d, use default: %d\n",cap,SIZE);
		cap = SIZE;
	}

	map->arr = os_zalloc(cap * sizeof(struct _jm_hash_map_item*));//锟斤拷锟斤拷指锟斤拷锟斤拷锟斤拷锟阶碉拷址
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

/*******************************Hash Map 锟斤拷锟斤拷锟斤拷锟�***************************************/

/*******************************Cache 锟斤拷锟藉开始************************************/

ICACHE_FLASH_ATTR BOOL cache_init(char *cacheName, size_t itemSize){
	if(cacheMap == NULL) {
		cacheMap = hashmap_create(SIZE);//锟斤拷一锟斤拷Map
		if(cacheMap == NULL) {
			INFO("cache_init global cache fail: %s, %d\n",cacheName);
			return false;
		}
	}

	if(hashmap_exist(cacheMap, cacheName)) {
		INFO("cache_init exist cache: %s\n",cacheName);
		return false;//锟斤拷锟斤拷锟窖撅拷锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷
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
		return false;//锟斤拷锟斤拷锟窖撅拷锟斤拷锟斤拷
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

	c->item = it;

	c->next = ch->cache;
	ch->cache = c;
	return c;
}

/**
 */
ICACHE_FLASH_ATTR void* cache_get(char *cacheName, BOOL created){
	if(cacheMap == NULL) {
		INFO("cache_get fail: %s",cacheName);
		return NULL;
	}

	jm_cache_header_t *ch = hashmap_get(cacheMap, cacheName);

	if(!ch) {
		INFO("cache_get cache not exist: %s\n",cacheName);
		return NULL;
	}

	if(ch->cache) {
		//锟叫伙拷锟芥，锟斤拷锟斤拷一锟斤拷未使锟矫碉拷锟斤拷
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
			os_memset(c->item,0,ch->memSize);//锟斤拷锟斤拷锟节达拷为0
			c->used = true;
			return c->item;
		}
	}

	//INFO("cache_get cache element is NULL: %s\n",cacheName);
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

//锟介还一锟斤拷锟斤拷锟斤拷
ICACHE_FLASH_ATTR BOOL cache_back(char *cacheName, void *it){
	if(cacheMap == NULL) {
		INFO("cache_back cache is NULL: %s",cacheName);
		return false;
	}

	jm_cache_header_t *ch = hashmap_get(cacheMap, cacheName);

	if(!ch) {
		INFO("cache_back cache not exist: %s\n",cacheName);
		return false;//锟斤拷锟芥还锟斤拷锟斤拷锟斤拷
	}

	jm_cache_t* c = ch->cache;
	while(c) {
		if(c->item == it) {
			c->used = false;
			//INFO("cache_back back item success: %s\n",cacheName);
			return true;
		}
		c = c->next;
	}

	INFO("cache_back back item not exist: %s\n",cacheName);
	return false;//锟斤拷锟芥还锟斤拷锟斤拷锟斤拷

}


/*******************************Cache 锟斤拷锟斤拷锟斤拷锟�************************************/
