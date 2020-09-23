/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：config.c
* 文件说明：配置文件操作接口实现文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<ctype.h>
 
#include "config.h"
 
#define KEY_SIZE        128 // key缓冲区大小
#define VALUE_SIZE      128 // value缓冲区大小
 
#define LINE_BUF_SIZE   256 // 读取配置文件中每一行的缓冲区大小
 
// 属性节点数据结构
typedef struct Properties
{
    char *key;					// 属性key
    char *value;				// 属性key值
    struct Properties *pNext;	// 后指针
}Properties;
 
// 属性链表数据结构
typedef struct PROPS_HANDLE
{
    Properties *pHead;			// 属性链表头节点
    char *filepath;				// 属性文件路径
}PROPS_HANDLE;
 
/**************************************************************************** 
* 函数名称 : createPropsNode
* 功能描述 : 创建节点函数
* 参    数 : Properties **props			: 属性节点
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
static int createPropsNode(Properties **props);

/**************************************************************************** 
* 函数名称 : trimeSpace
* 功能描述 : 去空格函数
* 参    数 : const char *src			: 输入缓冲
* 参    数 : char *dest					: 输出缓冲

* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
static int trimeSpace(const char *src, char *dest);

/**************************************************************************** 
* 函数名称 : saveConfig
* 功能描述 : 保存到属性文件函数
* 参    数 : const char *filepath		: 属性文件路径
* 参    数 : Properties *head			: 属性链表
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
static int saveConfig(const char *filepath, Properties *head);   // 将修改或保存后的配置项保存到文件
 
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
int conf_init(const char *filepath, void **handle)
{
    int ret = 0;
    FILE *fp = NULL;
    Properties *pHead = NULL,*pCurrent = NULL, *pMalloc = NULL;
    PROPS_HANDLE *ph = NULL;
    char line[LINE_BUF_SIZE];               // 存放读取每一行的缓冲区
    char keybuff[KEY_SIZE] = { 0 };         // 存放key的缓冲区
    char valuebuff[VALUE_SIZE] = { 0 };     // 存放value的缓冲区
    char *pLine = NULL;                     // 每行缓冲区数据的指针
     
    if (filepath == NULL || handle == NULL)
    {
        ret = -1;
        printf("fun conf_init error:%d from (filepath == NULL || handler == NULL)\n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)malloc(sizeof(PROPS_HANDLE));
    if (ph == NULL)
	{
        ret = -2;
        printf("fun conf_init malloc handle error:%d",ret);
        return ret;
    }
    memset(ph, 0, sizeof(PROPS_HANDLE));
     
    // 打开文件
    fp = fopen(filepath, "r");
    if (!fp)
	{
        ret = -3;
		free(ph);
        printf("fun conf_init open file error:%d from %s\n", ret, filepath);
        return ret;
    }
     
    // 创建头节点
	pHead = (Properties *)malloc(sizeof(Properties));
	if(pHead == NULL)
	{
		free(ph);
		fclose(fp);
        ret = -4;
        printf("fun conf_init create head node error:%d\n", ret);
        return ret;
	}
    pHead->key = NULL;
	pHead->value = NULL;
	pHead->pNext = NULL;
     
    // 保存链表头节点和文件路径到handle中
    ph->pHead = pHead;
    ph->filepath = (char *)malloc(strlen(filepath) + 1);
    strcpy(ph->filepath, filepath);
     
    pCurrent = pHead;
 
    // 读取配置文件中的所有数据
    while (!feof(fp))
	{
        if (fgets(line, LINE_BUF_SIZE, fp) == NULL)
        {
            break;
        }
         
        // 找等号
        if ((pLine = strstr(line, "=")) == NULL)
		{   // 没有等号，继续读取下一行
            continue;
        }
         
        // 循环创建节点
        ret = createPropsNode(&pMalloc);
        if (ret != 0)
		{
            fclose(fp);  // 关闭文件
            conf_release((void **)&ph);  // 创建节点失败，释放所有资源
            printf("create new node error:%d\n", ret);
            return ret;
        }
 
        // 设置Key
        memcpy(keybuff, line, pLine-line);
        trimeSpace(keybuff, pMalloc->key);    // 将keybuff去空格后放到pMallock.key中
     
        // 设置Value
        pLine += 1;
        trimeSpace(pLine, valuebuff);
        strcpy(pMalloc->value, valuebuff);
         
        // 将新节点入链表
        pMalloc->pNext = NULL;
        pCurrent->pNext = pMalloc;
        pCurrent = pMalloc; // 当前节点下移
         
        // 重置key,value
        memset(keybuff, 0, KEY_SIZE);
        memset(valuebuff, 0, VALUE_SIZE);
    }
     
    // 设置环境句柄给调用者
    *handle = ph;
     
    // 关闭文件
    fclose(fp);
     
    return ret;
}
 
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
int conf_getCount(void *handle, int *count)
{
    int ret = 0,cn = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL;
    if (handle == NULL || count == NULL)
	{
        ret = -1;
        printf("fun conf_getCount error:%d from (handle == NULL || count == NULL)\n", ret);
        return ret;
    }
    ph = (PROPS_HANDLE *)handle;
    pCurrent = ph->pHead->pNext;
    while (pCurrent != NULL)
	{
        cn++;
        pCurrent = pCurrent->pNext;
    }
     
    *count = cn;
     
    return ret;
}
 
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
int conf_getValue(void *handle, const char *key, char *value)
{
    int ret = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL;
    if (handle == NULL || key == NULL || value == NULL)
	{
        ret = -1;
        printf("conf_getValue error:%d from (handle == NULL || key == NULL || value == NULL)\n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)handle;
    pCurrent = ph->pHead->pNext;
    while (pCurrent != NULL)
	{
        if (strcmp(pCurrent->key,key) == 0)
		{
            break;
        }
        pCurrent = pCurrent->pNext;
    }
     
    if (pCurrent == NULL)
	{
        ret = -2;
        printf("fun conf_getValue warning: not found the key:%s\n", key);
        return ret;
    }
     
    strcpy(value, pCurrent->value);
     
    return ret;
}
 
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
int conf_setValue(void *handle, const char *key, const char *value)
{
    int ret = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL;
    if (handle == NULL || key == NULL || value == NULL)
	{
        ret = -1;
        printf("fun conf_setValue error:%d from (handle == NULL || key == NULL || value == NULL)\n", ret);
        return ret;
    }
     
    // 获得环境句柄
    ph = (PROPS_HANDLE *)handle;
     
    // 从环境句柄中获取头节点
    pCurrent = ph->pHead->pNext;
    while (pCurrent != NULL)
	{
        if (strcmp(pCurrent->key, key) == 0)
		{  // 找到
            break;
        }
        pCurrent = pCurrent->pNext;
    }
     
    if (pCurrent == NULL)
	{ // 未找到key
        ret = -2;
        printf("fun conf_setValue warning: not found the key:%s\n", key);
        return ret;
    }
     
    // 修改key的value
    strcpy(pCurrent->value, value);
    if (strchr(value, '\n') == NULL)
	{  // 加一个换行符
        strcat(pCurrent->value, "\n");
    }
     
    // 将修改的配置项写入到文件
    ret = saveConfig(ph->filepath, ph->pHead);
   
    return ret;
}
 
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
int conf_add(void *handle, const char *key, const char *value)
{
    int ret = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL;
    if (handle == NULL || key == NULL || value == NULL)
	{
        ret = -1;
        printf("fun conf_add error:%d from (handle == NULL || key == NULL || value == NULL)\n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)handle;
     
    //-----------如果key存在链表中，则直接修改，否则添加到链表中-----------//
    pCurrent = ph->pHead;
    while (pCurrent->pNext != NULL)
	{
        if (strcmp(pCurrent->pNext->key, key) == 0)
		{
            break;
        }
        pCurrent = pCurrent->pNext;
    }
     
    if (pCurrent->pNext != NULL)
	{
        return conf_setValue(handle, key, value);
    }
     
    //-----------key不存在，创建一个新的配置项，添加到链表中-----------//
    Properties *pMalloc;
    ret = createPropsNode(&pMalloc);
    if (ret != 0)
	{
        printf("fun conf_add error:%d from malloc new node.", ret);
        return ret;
    }
     
    strcpy(pMalloc->key, key);
    if (strchr(pCurrent->value,'\n') == NULL)
	{
        strcat(pCurrent->value, "\n");
    }
    strcpy(pMalloc->value, value);
    if (strchr(value, '\n') == NULL)
	{  // 加一个换行符
        strcat(pMalloc->value, "\n");
    }
    pCurrent->pNext = pMalloc;  // 新配置项入链表
     
    // 将新配置项写入到文件
    ret = saveConfig(ph->filepath, ph->pHead);
     
    return ret;
}
 
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
int conf_del(void *handle, const char *key)
{
    int ret = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL, *pPrev = NULL;
    if (handle == NULL || key == NULL)
	{
        ret = -1;
        printf("fun conf_del error:%d from (handle == NULL || key == NULL)\n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)handle;
    pPrev = ph->pHead;
    pCurrent = ph->pHead->pNext;
     
    while (pCurrent != NULL)
	{
        if (strcmp(pCurrent->key, key) == 0)
		{
            break;
        }
        pPrev = pCurrent;           // 上一个节点下移
        pCurrent = pCurrent->pNext; // 当前节点下移
    }
     
    if (pCurrent == NULL)
	{ // 没有找到
        ret = -2;
        printf("fun conf_del warning:not found the key:%s\n", key);
        return  ret;
    }
     
    pPrev->pNext = pCurrent->pNext; // 从链表中删除
    free(pCurrent); // 释放内存
    pCurrent = NULL;
     
    // 保存到文件
    ret = saveConfig(ph->filepath, ph->pHead);
     
    return ret;
}
 
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
int conf_getKeys(void *handle, char ***keys, int *keyscount)
{
    int ret = 0, count = 0, index = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL;
    char **pKeys = NULL;
    if (handle == NULL || keys == NULL || keyscount == NULL)
	{
        ret = -1;
        printf("fun conf_getKeys error:%d from (handle == NULL || keys == NULL || keyscount == NULL) \n", ret);
        return ret;
    }
     
    // 获取配置项数量
    ret = conf_getCount(handle, &count);
    if (ret != 0)
	{
        printf("fun conf_getKeys error:%d from conf_getCount \n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)handle;
    pCurrent = ph->pHead->pNext;
     
    // 根据链表长度，申请内存空间
    pKeys = (char **)malloc(sizeof(char *) * count);
    if (pKeys == NULL)
	{
        ret = -2;
        printf("fun conf_getKeys error:%d from malloc keys\n", ret);
        return ret;
    }
     
    pCurrent = ph->pHead->pNext;
    while (pCurrent != NULL)
	{
        pKeys[index] = pCurrent->key;
        pCurrent = pCurrent->pNext;
        index++;
    }
     
    *keys = pKeys;
    *keyscount = count;
     
    return ret;
}
 
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
int conf_freeKeys(char ***keys,int *keyscount)
{
    int ret = 0;
    if (keys == NULL || keyscount == NULL)
	{
        ret = -1;
        printf("fun conf_freeKeys error:%d from (keys == NULL || keyscount == NULL) \n", ret);
        return ret;
    }
     
    free(*keys);
    *keys = NULL;
    *keyscount = 0;
     
    return ret;
}
 
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
int conf_getValues(void *handle, char ***values, int *valuescount)
{
    int ret = 0, count = 0, index = 0;
    PROPS_HANDLE *ph = NULL;
    Properties *pCurrent = NULL;
    char **pValues = NULL;
    if (handle == NULL || values == NULL || valuescount == NULL)
	{
        ret = -1;
        printf("fun conf_getValues error:%d from (handle == NULL || values == NULL || valuescount == NULL)\n", ret);
        return ret;
    }
     
    // 获取配置项数量
    ret = conf_getCount(handle, &count);
    if (ret != 0)
	{
        printf("fun conf_getValues error:%d from conf_getCount \n", ret);
        return ret;
    }
     
    // 申请内存空间，存放所有的value
    pValues = (char **)malloc(sizeof(char *) * count);
    if (pValues == NULL)
	{
        ret = -2;
        printf("fun conf_getValues error:%d from malloc values\n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)handle;
    pCurrent = ph->pHead->pNext;
    while (pCurrent != NULL)
	{
        pValues[index] = pCurrent->value;
        pCurrent = pCurrent->pNext;
        index++;
    }
     
    *values = pValues;
    *valuescount = count;
     
    return ret;
}
 
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
int conf_freeValues(char ***values, int *valuescount)
{
    int ret = 0;
    if (values == NULL || valuescount == NULL)
	{
        ret = -1;
        printf("fun conf_freeValues error:%d from (values == NULL || valuescount == NULL) \n", ret);
        return ret;
    }
     
    free(*values);
    *values = NULL;
    *valuescount = 0;
     
    return ret;
}
 
/**************************************************************************** 
* 函数名称 : conf_release
* 功能描述 : 释放环境资源函数
* 参    数 : void **handle				: 配置文件句柄
* 返 回 值 : 成功返回0，失败返回非0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int conf_release(void **handle)
{
    int ret = 0;
    PROPS_HANDLE *ph = NULL;
    if(handle == NULL)
    {
        ret = -1;
        printf("conf_release error:%d from (handler == NULL)\n", ret);
        return ret;
    }
     
    ph = (PROPS_HANDLE *)*handle;
     
    // 释放链表内存资源
    Properties *pCurr = ph->pHead;
    Properties *pTemp = NULL;
     
    while (pCurr != NULL)
	{
        if (pCurr->key != NULL)
		{
            free(pCurr->key);
            pCurr->key = NULL;
        }
         
        if (pCurr->value != NULL)
		{
            free(pCurr->value);
            pCurr->value = NULL;
        }
         
        pTemp = pCurr->pNext;
         
        free(pCurr);
         
        pCurr = pTemp;
    }
     
    // 释放存放配置文件路径分配的内存空间
    if(ph->filepath != NULL)
    {
        free(ph->filepath);
        ph->filepath = NULL;
    }
     
    // 释放环境句柄本身
    free(ph);
    *handle = NULL;    // 避免野指针
         
    return ret;
}
 
/**************************************************************************** 
* 函数名称 : trimeSpace
* 功能描述 : 去空格函数
* 参    数 : const char *src			: 输入缓冲
* 参    数 : char *dest					: 输出缓冲

* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
static int trimeSpace(const char *src, char *dest)
{
    int ret = 0;
    if (src == NULL || dest == NULL)
	{
        ret = -1;
        printf("trimeSpace error:%d from (src == NULL || dest == NULL)\n", ret);
        return ret;
    }
     
    const char *psrc = src;
    unsigned long i = 0,j = strlen(psrc) - 1,len;
    while (psrc[i] == ' ')
    {
        i++;
    }
     
    while (psrc[j] == ' ')
	{
        j--;
    }
     
    len = j - i + 1;
     
    memcpy(dest, psrc+i, len);
    *(dest+len) = '\0';
     
    return ret;
}
 
/**************************************************************************** 
* 函数名称 : createPropsNode
* 功能描述 : 创建节点函数
* 参    数 : Properties **props			: 属性节点
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
static int createPropsNode(Properties **props)
{
    int ret = 0;
    Properties *p = NULL;
    if (props == NULL)
	{
        ret = -100;
        printf("createProps error:%d from (props == NULL)\n", ret);
        return ret;
    }
     
    p = (Properties *)malloc(sizeof(Properties));
    if (p == NULL)
	{
        ret = -200;
        printf("createProps malloc %u bytes error:%d\n", sizeof(Properties), ret);
        return ret;
    }
    p->key = (char *)malloc(KEY_SIZE);
    p->value = (char *)malloc(VALUE_SIZE);
    p->pNext = NULL;
     
    *props = p;
     
    return ret;
}
 
/**************************************************************************** 
* 函数名称 : saveConfig
* 功能描述 : 保存到属性文件函数
* 参    数 : const char *filepath		: 属性文件路径
* 参    数 : Properties *head			: 属性链表
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
static int saveConfig(const char *filepath, Properties *head)
{
    int ret = 0,writeLen = 0;
    FILE *fp = NULL;
    Properties *pCurrent = NULL;
    if (filepath == NULL || head == NULL)
	{
        ret = -100;
        printf("fun saveConfig error:%d from (filepath == NULL || head == NULL)\n", ret);
        return ret;
    }
     
    fp = fopen(filepath,"w");
    if (fp == NULL)
	{
        ret = -200;
        printf("fun saveConfig:open file error:%d from %s\n", ret, filepath);
        return ret;
    }
     
    pCurrent = head->pNext;
    while (pCurrent != NULL)
	{
        writeLen = fprintf(fp, "%s=%s",pCurrent->key,pCurrent->value);    // 返回写入的字节数，出现错误返回一个负值
        if (writeLen < 0)
		{  //TODO 如果写入失败，如何将写入的数据回退？？？
            ret = -300;
            printf("fun saveConfig err:%d from (%s=%s)\n", ret, pCurrent->key, pCurrent->value);
            break;
        }
        pCurrent = pCurrent->pNext;
    }
 
    fclose(fp); // 关闭文件
     
    return ret;
}


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
char * l_trim(char * szOutput, const char *szInput)
{
	for (NULL; *szInput != '\0' && isspace(*szInput); ++szInput)
	{
		;
	}
	return strcpy(szOutput, szInput);
}

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
char * r_trim(char *szOutput, const char *szInput)
{
	char *p = NULL;
	strcpy(szOutput, szInput);
	for (p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p)
	{
		;
	}
	*(++p) = '\0';
	return szOutput;
}

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
char * a_trim(char * szOutput, const char * szInput)
{
	char *p = NULL;
	l_trim(szOutput, szInput);
	for (p = szOutput + strlen(szOutput) - 1;p >= szOutput && isspace(*p); --p)
	{
		;
	}
	*(++p) = '\0';
	return szOutput;
}

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
char * cr_trim(char *szOutput, const char *szInput)
{
	//char *p = NULL;
	strcpy(szOutput, szInput);

	if (szOutput[strlen(szOutput)-1] == 0x0a && szOutput[strlen(szOutput)-2] == 0x0d)
	{
		szOutput[strlen(szOutput)-2] = 0x00;
	}
	else if (szOutput[strlen(szOutput)-1] == 0x0a)
	{
		szOutput[strlen(szOutput)-1] = 0x00;
	}
#if 0
#ifdef WIN32
	if (szOutput[strlen(szOutput)-1] == 0x0a)
	{
		szOutput[strlen(szOutput)-1] = 0x00;
	}
#else
	if (szOutput[strlen(szOutput)-1] == 0x0a && szOutput[strlen(szOutput)-2] == 0x0d)
	{
		szOutput[strlen(szOutput)-2] = 0x00;
	}
#endif
#endif
	//*(++p) = '\0';
	return szOutput;
}

/****************************************************************************
* 函数名称 : cr_trim
* 功能描述 : 删除指定字符函数
* 参    数 : char * szOutput            : 输出缓冲
* 参    数 : const char *szInput        : 输入缓冲
* 返 回 值 : 处理后的内容缓冲
* 作    者 : yzl
* 设计日期 : 20171206
* 修改日期        修改人           修改内容
 *****************************************************************************/
char * chr_trim(char *szOutput,const char *szInput,const char del_char)
{
	while('\0' != *szInput)
	{
		if(del_char != *szInput)
		{
			*szOutput = *szInput;
			szOutput ++;
		}
		szInput++;
	}
	return szOutput;
}


