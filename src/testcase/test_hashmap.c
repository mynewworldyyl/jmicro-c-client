
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../jm_hashmap.h"

#define KEY_MAX_LENGTH (256)
#define KEY_PREFIX ("somekey")
#define KEY_COUNT (1024*1024)


typedef struct data_struct_s
{
    char *key_string;
    int number;
} data_struct_t;

int test_hashmap() {

    int error;
    map_t mymap;
    char *key = "key0";

    data_struct_t* value = (data_struct_t*)malloc(sizeof(struct data_struct_s));
    value->key_string = key;
    value->number = 1;

    mymap = hashmap_new();

    error = hashmap_put(mymap, key, value);
    assert(error==MAP_OK);

    data_struct_t* value1;

    error = hashmap_get(mymap, key, &value1);
    assert(error==MAP_OK);
    assert(value1->number==1);

    hashmap_free(mymap);

    return 1;

}
