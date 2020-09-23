/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：config.h 
* 文件说明：配置文件操作接口头文件
* 作    者：hlzheng
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__



/**************************** 函数声明和定义 ****************************/


/**************************************************************************** 
* 函数名称 : conf_init
* 功能描述 : 初始化配置环境函数
* 参    数 : const char *filepath		: 配置文件路径
* 参    数 : void **handle				: 配置文件句柄
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_init(const char *filepath, void **handle);

/**************************************************************************** 
* 函数名称 : conf_getValue
* 功能描述 : 根据KEY获取值函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : const char *key			: 配置文件键值
* 参    数 : char *value				: 配置文件键值的内容
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_getValue(void *handle, const char *key, char *value);

/**************************************************************************** 
* 函数名称 : conf_setValue
* 功能描述 : 修改key对应的属性值函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : const char *key			: 配置文件键值
* 参    数 : const char *value			: 配置文件键值的内容
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_setValue(void *handle, const char *key, const char *value);

/**************************************************************************** 
* 函数名称 : conf_add
* 功能描述 : 添加属性函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : const char *key			: 配置文件键值
* 参    数 : const char *value			: 配置文件键值的内容
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_add(void *handle, const char *key, const char *value);

/**************************************************************************** 
* 函数名称 : conf_del
* 功能描述 : 删除属性函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : const char *key			: 配置文件键值
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_del(void *handle, const char *key);

/**************************************************************************** 
* 函数名称 : conf_getKeys
* 功能描述 : 获取配置文件中所有的key函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : char ***keys				: key链表
* 参    数 : int *keyscount				: key数量
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_getKeys(void *handle, char ***keys, int *keyscount);

/**************************************************************************** 
* 函数名称 : conf_freeKeys
* 功能描述 : 释放所有key的内存空间函数
* 参    数 : char ***keys				: key链表
* 参    数 : int *keyscount				: key数量
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_freeKeys(char ***keys,int *keyscount);

/**************************************************************************** 
* 函数名称 : conf_getValues
* 功能描述 : 获取属性文件中所有的值函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : char ***values				: key值链表
* 参    数 : int *valuescount			: key值数量
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_getValues(void *handle, char ***values, int *valuescount);

/**************************************************************************** 
* 函数名称 : conf_freeValues
* 功能描述 : 释放所有value的内存空间函数
* 参    数 : char ***values				: key值链表
* 参    数 : int *valuescount			: key值数量
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_freeValues(char ***values, int *valuescount);

/**************************************************************************** 
* 函数名称 : conf_getCount
* 功能描述 : 获取属性数量函数
* 参    数 : void *handle				: 配置文件句柄
* 参    数 : int *count					: 属性数量
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_getCount(void *handle, int *count);

/**************************************************************************** 
* 函数名称 : conf_release
* 功能描述 : 释放环境资源函数
* 参    数 : void **handle				: 配置文件句柄
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_release(void **handle);

/**************************************************************************** 
* 函数名称 : l_trim
* 功能描述 : 删除左边的空格函数
* 参    数 : char * szOutput			: 输出缓冲
* 参    数 : const char *szInput		: 输入缓冲
* 返 回 值 : 处理后的内容缓冲
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
char * l_trim(char * szOutput, const char *szInput);

/**************************************************************************** 
* 函数名称 : r_trim
* 功能描述 : 删除右边的空格函数
* 参    数 : char * szOutput			: 输出缓冲
* 参    数 : const char *szInput		: 输入缓冲
* 返 回 值 : 处理后的内容缓冲
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
char * r_trim(char *szOutput, const char *szInput);

/**************************************************************************** 
* 函数名称 : a_trim
* 功能描述 : 删除两边的空格函数
* 参    数 : char * szOutput			: 输出缓冲
* 参    数 : const char *szInput		: 输入缓冲
* 返 回 值 : 处理后的内容缓冲
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
char * a_trim(char * szOutput, const char * szInput);

/**************************************************************************** 
* 函数名称 : cr_trim
* 功能描述 : 删除换行符函数
* 参    数 : char * szOutput			: 输出缓冲
* 参    数 : const char *szInput		: 输入缓冲
* 返 回 值 : 处理后的内容缓冲
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
char * cr_trim(char *szOutput, const char *szInput);

char * chr_trim(char *szOutput,const char *szInput,const char del_char);


#endif

