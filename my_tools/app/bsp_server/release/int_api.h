/**********************************************************************************
文件描述  : bsp server有部分api不需要对外开放，放在这里
作者/时间 : zhongwei.peng@openvox.cn/2017.12.9
***********************************************************************************/
#ifndef __INT_API_H_
#define __INT_API_H_


/*
desc      : get board mcu version info
param in  : idx -- mcu number
param out : ver_info -- version information buffer
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_int_version(unsigned int idx, unsigned char *ver_info);











#endif

