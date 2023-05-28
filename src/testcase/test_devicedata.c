#include <stdio.h>
#include <assert.h>

#include <test.h>
#include <debug.h>
#include "jm_client.h"

void kv_testaddDeviceDataCb(void *resultMap, sint32_t code, char *errMsg, void *arg){
	INFO("add kv result code=%d, msg: %s\n", code,errMsg);
}

void kv_testgetDeviceDataCb(void *resultMap, sint32_t code, char *errMsg, void *arg){
	INFO("Get kv result code=%d, msg: %s\n", code,errMsg);
}

void kv_testDeleteDeviceDataCb(void *resultMap, sint32_t code, char *errMsg, void *arg){
	INFO("Delete kv result code=%d, msg: %s\n", code,errMsg);
}

void kv_testAddDeviceData(sint32_t code, char *msg, char *loginKey, sint32_t actId){

	if(code != 0) {
		INFO("Login fail code=%d, msg: %s\n", code,msg);
		return;
	}

	/*
	if(kv_add("key01", "Hello", "test value",PREFIX_TYPE_STRINGG , kv_testaddDeviceDataCb)) {
		INFO("Add device data req success\n");
	} else {
		INFO("Add device data req fail\n");
	}*/

	if(kv_update("key01", "World", "test update value",PREFIX_TYPE_STRINGG , kv_testaddDeviceDataCb)) {
		INFO("Update device data req success\n");
	} else {
		INFO("Update device data req fail\n");
	}

	/*if(kv_get("key01", kv_testgetDeviceDataCb)) {
		INFO("Get device data req success\n");
	}else {
		INFO("Get device data req fail\n");
	}*/

	/*if(kv_delete("key01", kv_testDeleteDeviceDataCb)) {
		INFO("Delete device data req success\n");
	}else {
		INFO("Delete device data req fail\n");
	}*/

}
