#include <stdio.h>
#include <assert.h>

#include "../jm_mem.h"
#include "../jm_msg.h"

void testGetCacheItem(){

	char *cn1 = "cache1";
	char *cn2 = "cache2";
	char *cn3 = "cache3";

	cache_init(cn1, sizeof(struct _jm_msg));
	cache_init(cn2, sizeof(struct _jm_msg));
	cache_init(cn3, sizeof(struct _jm_msg));

	jm_msg_t *mt1 = cache_get(cn1,true);
	assert(mt1);
	printf("mt1 address: %d\n",mt1);
	printf("mt1.msgId: %d\n",mt1->msgId);

	cache_back(cn1,mt1);

	jm_msg_t *mt11 = cache_get(cn1,true);
	assert(mt11);
	printf("mt11 address: %d\n",mt11);
	printf("mt11.msgId: %d\n",mt11->msgId);

	jm_msg_t *mt12 = cache_get(cn1,true);
	assert(mt12);
	printf("mt12 address: %d\n",mt12);
	printf("mt12.msgId: %d\n",mt12->msgId);

	jm_msg_t *mt2 = cache_get(cn2,true);
	assert(mt2);
	mt2->msgId = 3;
	printf("mt2 address: %d\n",mt2);
	printf("mt2.msgId: %d\n",mt2->msgId);

	cache_back(cn1,mt11);
	cache_back(cn1,mt12);

	cache_back(cn2,mt2);

}
