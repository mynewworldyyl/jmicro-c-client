#ifndef JM_CTRL_H_
#define JM_CTRL_H_

#include "jm_client.h"

//方法名称KEY
#define FUNC_NAME "funName"

typedef msg_extra_data_t* (*ctrl_fn)(msg_extra_data_t *ps);

typedef struct {
	char *funName; //unique function name
	sint8_t type; //PS item type
	ctrl_fn fn; //function
} ctrl_item;

//注册一个控制方法
//ICACHE_FLASH_ATTR BOOL ctrl_regist(ctrl_item *fn);
ICACHE_FLASH_ATTR BOOL ctrl_registFun(char *funName, ctrl_fn fun);

//删除一个控制方法
ICACHE_FLASH_ATTR BOOL ctrl_unregist(char *funName);

//是否已经注册了指定名称的方法
ICACHE_FLASH_ATTR BOOL ctrl_exists(char *funName);


#endif /* USER_CONFIG_H_ */
