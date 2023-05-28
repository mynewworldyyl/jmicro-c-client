#ifndef JM_CTRL_H_
#define JM_CTRL_H_

#include "jm_client.h"

//方法名称KEY
#define FUNC_NAME "funName"

typedef msg_extra_data_t* (*ctrl_fn)(msg_extra_data_t *ps);

typedef struct {
	char *name; //参数名称，如op,code,msg等
	char *defVal; //默认值
	sint8_t type; //参数类型，参考jm_msg.h文件  PREFIX_TYPE_BYTE，PREFIX_TYPE_SHORT等
	sint8_t maxLen; //参数最大长度
	char *desc;
} ctrl_arg;

typedef struct {
	char *funDesc; //方法描述，或功能描述
	char *funName; //device unique function name
	sint8_t type; //PS item type, 默认是-128，当前只支持-128
	ctrl_fn fn; //function
	ctrl_arg* args;//方法用到的参数
	sint8_t ver;//版本
} ctrl_item;

//注册一个控制方法
//ICACHE_FLASH_ATTR BOOL ctrl_regist(ctrl_item *fn);
ICACHE_FLASH_ATTR BOOL ctrl_registFun(char *funName, ctrl_fn fun, ctrl_arg* args, char *desc, uint8_t ver);

//删除一个控制方法
ICACHE_FLASH_ATTR BOOL ctrl_unregist(char *funName);

//是否已经注册了指定名称的方法
ICACHE_FLASH_ATTR BOOL ctrl_exists(char *funName);

void ICACHE_FLASH_ATTR ctrl_init(void);

#endif /* USER_CONFIG_H_ */
