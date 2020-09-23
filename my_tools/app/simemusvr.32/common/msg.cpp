/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：msg.c
* 文件说明：网络交互报文格式接口实现文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
#include "msg.h"
#include "zprint.h"





/**************************************************************************** 
* 函数名称 : makeReqPackage
* 功能描述 : 请求包打包函数
* 参    数 : unsigned short type		: 网络包类型
* 参    数 : unsigned short server_type	: 网络包发起服务器类型
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : unsigned short data_len	: 负载数据长度
* 参    数 : char *buff					: 报文内容缓冲，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeReqPackage(unsigned short cmd, char *data, unsigned short data_len, char *buff)
{
	if (buff == NULL)
	{
		return -1;
	}
	unsigned short *pbuff = (unsigned short *)buff;
	pbuff[0] = htons(VERSION);
	pbuff[1] = htons(cmd);
	pbuff[2] = htons(data_len);
	pbuff[3] = htons(0x0000);
	pbuff[4] = htons(0x0000);
	if (data != NULL)
	{
		memcpy(buff+PACKAGE_HEADER_LEN, data, data_len);
	}
	unsigned short crc = checksum((unsigned short *)buff, PACKAGE_HEADER_LEN+data_len);
	((unsigned short *)(buff+PACKAGE_HEADER_LEN+data_len))[0] = htons(crc);
	return PACKAGE_HEADER_LEN+data_len+2;
}

/**************************************************************************** 
* 函数名称 : makeRspPackage
* 功能描述 : 响应包打包函数
* 参    数 : char *buff_req				: 请求包缓冲，里面存储请求内容
* 参    数 : char *buff_rsp				: 响应包缓冲，用于输出
* 参    数 : char *data					: 负载数据
* 参    数 : unsigned short data_len	: 负载数据长度
* 参    数 : unsigned short result		: 报文成功标志
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeRspPackage(char *buff_req, char *buff_rsp, char *data, unsigned short data_len, unsigned short result)
{
	if (buff_req == NULL || buff_rsp == NULL)
	{
		return -1;
	}
	package_header_t * header_req = (package_header_t *)buff_req;
	package_header_t * header_rsp = (package_header_t *)buff_rsp;

	header_rsp->version = header_req->version;
	header_rsp->cmd = htons(ntohs(header_req->cmd)+0x1000);
	header_rsp->len = htons(data_len);
	header_rsp->result = htons(result);
	header_rsp->reserve = htons(0x00);;
	if (data != NULL)
	{
		memcpy(buff_rsp+PACKAGE_HEADER_LEN, data, data_len);
	}
	unsigned short crc = checksum((unsigned short *)buff_rsp, PACKAGE_HEADER_LEN+data_len);
	((unsigned short *)(buff_rsp+PACKAGE_HEADER_LEN+data_len))[0] = htons(crc);
	return PACKAGE_HEADER_LEN+data_len+2;
}

int makeSimDataRspPackage(char *buff_rsp, char *data, unsigned short data_len, unsigned short result)
{
	if (buff_rsp == NULL)
	{
		return -1;
	}
	package_header_t * header_rsp = (package_header_t *)buff_rsp;
	
	header_rsp->version = htons(VERSION);
	header_rsp->cmd = htons(SIMDATA_RSP);
	header_rsp->len = htons(data_len);
	header_rsp->result = htons(result);
	header_rsp->reserve = htons(0x00);;
	if (data != NULL)
	{
		memcpy(buff_rsp+PACKAGE_HEADER_LEN, data, data_len);
	}
	unsigned short crc = checksum((unsigned short *)buff_rsp, PACKAGE_HEADER_LEN+data_len);
	((unsigned short *)(buff_rsp+PACKAGE_HEADER_LEN+data_len))[0] = htons(crc);
	return PACKAGE_HEADER_LEN+data_len+2;
}

int makeSimPullPlugReqPackage(char *buff_rsp, char *data, unsigned short data_len, unsigned short result)
{
	if (buff_rsp == NULL)
	{
		return -1;
	}
	package_header_t * header_rsp = (package_header_t *)buff_rsp;
	
	header_rsp->version = htons(VERSION);
	header_rsp->cmd = htons(SIM_PULLPLUG_NOTICE_REQ);
	header_rsp->len = htons(data_len);
	header_rsp->result = htons(result);
	header_rsp->reserve = htons(0x00);;
	if (data != NULL)
	{
		memcpy(buff_rsp+PACKAGE_HEADER_LEN, data, data_len);
	}
	unsigned short crc = checksum((unsigned short *)buff_rsp, PACKAGE_HEADER_LEN+data_len);
	((unsigned short *)(buff_rsp+PACKAGE_HEADER_LEN+data_len))[0] = htons(crc);
	return PACKAGE_HEADER_LEN+data_len+2;
}


/**************************************************************************** 
* 函数名称 : checkPackage
* 功能描述 : 报文校验检查函数
* 参    数 : char *buff					: 报文缓冲
* 参    数 : int len					: 报文长度
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int checkPackage(char *buff, int len)
{
	if (buff == NULL || len < 0)
	{
		return -1;
	}
	unsigned short crc = 0;
	unsigned short crc_ = 0;

	memcpy((char *)&crc_, buff+len-2, 2);
	crc_ = ntohs(crc_);
	crc = checksum((unsigned short *)buff, len-2);
	if (crc != crc_)
	{
		return -1;
	}
	return 0;
}

/**************************************************************************** 
* 函数名称 : getVersion
* 功能描述 : 获取报文版本函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文版本号
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getVersion(char *buff)
{
	return ntohs(((unsigned short *)buff)[0]);
}

/**************************************************************************** 
* 函数名称 : getCmd
* 功能描述 : 获取报文类型函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文类型号
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getCmd(char *buff)
{
	return ntohs(((unsigned short *)buff)[1]);
}

/**************************************************************************** 
* 函数名称 : getLen
* 功能描述 : 获取报文负载长度函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文负载长度
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getLen(char *buff)
{
	return ntohs(((unsigned short *)buff)[2]);
}

/**************************************************************************** 
* 函数名称 : getResult
* 功能描述 : 获取报文结果码函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文结果码
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getResult(char *buff)
{
	return ntohs(((unsigned short *)buff)[3]);
}

/**************************************************************************** 
* 函数名称 : getServerType
* 功能描述 : 获取报文请求发起服务器类型
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文请求发起服务器类型
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getServerType(char *buff)
{
	return ntohs(((unsigned short *)buff)[5]);
}

/**************************************************************************** 
* 函数名称 : getSerialNo
* 功能描述 : 获取报文结果码函数
* 参    数 : char *buff					: 报文缓冲
* 参    数 : char *seri					: 报文Emu序列号
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int getSerialNo(char *buff, char *seri)
{
	memcpy(seri, buff+(6*sizeof(unsigned short)), 10);
	return 0;
}

/**************************************************************************** 
* 函数名称 : getReserve
* 功能描述 : 获取报文预置函数
* 参    数 : char *buff					: 报文缓冲
* 返 回 值 : 报文预置位
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short getReserve(char *buff)
{
	return ntohs(((unsigned short *)buff)[4]);
}

/**************************************************************************** 
* 函数名称 : checksum
* 功能描述 : 校验函数
* 参    数 : unsigned short *buffer		: 报文缓冲
* 参    数 : int size					: 报文长度
* 返 回 值 : 校验码1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
unsigned short checksum(unsigned short *buffer, int size)
{
	unsigned long cksum=0;
	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(unsigned short);
	}
	if (size)
	{
		cksum += *(unsigned char *)buffer;
	}
	while (cksum >> 16)
		cksum = (cksum >> 16) + (cksum & 0xffff);
	return (unsigned short)(~cksum);
}

/**************************************************************************** 
* 函数名称 : makeRegisterPack
* 功能描述 : 注册包打包函数
* 参    数 : char *buff_req				: 注册请求包缓冲，用于输出
* 参    数 : unsigned short server_type	: 请求发起的服务器类型
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : int data_len				: 负载数据长度
* 参    数 : int len					: 注册请求包缓冲长度，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeRegisterPack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(REGISTER_REQ, data, data_len, buff);
	return *len;
}

/**************************************************************************** 
* 函数名称 : makeUnRegisterPack
* 功能描述 : 注销包打包函数
* 参    数 : char *buff_req				: 注销请求包缓冲，用于输出
* 参    数 : unsigned short server_type	: 请求发起的服务器类型
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : int data_len				: 负载数据长度
* 参    数 : int len					: 注销请求包缓冲长度，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeUnRegisterPack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(UNREGISTER_REQ, data, data_len, buff);
	return *len;
}

/**************************************************************************** 
* 函数名称 : makeHeartbeatPack
* 功能描述 : 心跳包打包函数
* 参    数 : char *buff_req				: 心跳请求包缓冲，用于输出
* 参    数 : unsigned short server_type	: 请求发起的服务器类型
* 参    数 : char *seri					: SimEmuSvr序列号
* 参    数 : char *data					: 负载数据
* 参    数 : int data_len				: 负载数据长度
* 参    数 : int len					: 心跳请求包缓冲长度，用于输出
* 返 回 值 : 成功返回0，失败返回-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int makeHeartbeatPack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(HEARTBEAT_REQ, data, data_len, buff);
	return *len;
}


int makeReportPackage(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(LINKINFO_REPORT_REQ, data, data_len, buff);
	return *len;
}

int makeLinkBreakNoticePack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(LINK_BREAK_NOTICE_REQ, data, data_len, buff);
	return *len;
}

int makeSimPullPlugNoticePack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(SIM_PULLPLUG_NOTICE_REQ, data, data_len, buff);
	return *len;
}

int makeSimRegisterStatNoticePack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(AST_CHAN_STAT_NOTICE_REQ, data, data_len, buff);
	return *len;
}

int makeSMSRSPPackage(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(SIMSMS_RSP, data, data_len, buff);
	return *len;
}
int makeRcvSMSEvnetPackage(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(EVENT_SMS_RECEIVE, data, data_len, buff);
	return *len;
}
int makeEventModuleLimitPackage(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(EVENT_MODULE_LIMIT, data, data_len, buff);
	return *len;
}
int makeEventModuleInterPackage(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(EVENT_MODULE_INTERNET, data, data_len, buff);
	return *len;
}


#if 0 // no checksum
int recvNetPackage(CSocketEx *csock, char *buff, int *buff_len)
{
	int ret = 0;
	int len = 0;
	
	if (csock == NULL || buff == NULL || buff_len == NULL)
	{
		return -1;
	}
	
	// read package header
	ret = csock->ReadData(csock->getSocket(), buff, PACKAGE_HEADER_LEN);
	if (ret != PACKAGE_HEADER_LEN)
	{
		zprintf("recvNetPackage: recv package header error(%d:%s), maybe connection been closed", errno, strerror(errno));
		return -1;
	}
	else
	{
		// read package reload
		len = getLen(buff);
		ret = csock->ReadData(csock->getSocket(), buff+PACKAGE_HEADER_LEN, len);
		if (ret != len+2)
		{
			zprintf("recvNetPackage: recv package reload error(%d:%s)", errno, strerror(errno));
			return -1;
		}
		else
		{
			*buff_len = PACKAGE_HEADER_LEN + len;
			return 0;
		}
	}
}
#else
int recvNetPackage(CSocketEx *csock, char *buff, int *buff_len)
{
	int ret = 0;
	int len = 0;
	
	if (csock == NULL || buff == NULL || buff_len == NULL)
	{
		return -1;
	}
	
	// read package header
	ret = csock->ReadData(csock->getSocket(), buff, PACKAGE_HEADER_LEN);
	if (ret != PACKAGE_HEADER_LEN)
	{
		//zprintf("recvNetPackage: recv package header error(%d:%s), maybe connection been closed", errno, strerror(errno));
		return -1;
	}
	else
	{
		// read package reload
		len = getLen(buff);
		ret = csock->ReadData(csock->getSocket(), buff+PACKAGE_HEADER_LEN, len+2);
		if (ret != len+2)
		{
			//zprintf("recvNetPackage: recv package reload error(%d:%s)", errno, strerror(errno));
			return -1;
		}
		else
		{
			// checksum
			ret = checkPackage(buff, PACKAGE_HEADER_LEN+len+2);
			if (ret != 0)
			{
				//zprintf("recvNetPackage: package checksum fail");
				return -2;
			}
			*buff_len = PACKAGE_HEADER_LEN + len + 2;
			return 0;
		}
	}
}
#endif



int printHex(unsigned char *buff, unsigned short buff_len)
{
	unsigned short i = 0;
	while (i < buff_len)
	{
		printf("%02x ", buff[i]);
		i++;
	}
	printf("\n");
	return 0;
}
int printSimDataToHex(unsigned char *buff, unsigned short buff_len, unsigned short usb_nbr, unsigned short bank_nbr, unsigned short sim_nbr)
{
	unsigned short i = 0;
	printf("[%d-%d-%d]", usb_nbr, bank_nbr, sim_nbr);
	while (i < buff_len)
	{
		printf("%02x ", buff[i]);
		i++;
	}
	printf("\n");
	return 0;
}


int hex2char(unsigned char *buff_hex, unsigned short len_hex, char *buff_char, unsigned short *len_char)
{
	unsigned short i = 0;
	if (buff_hex == NULL || buff_char == NULL)
	{
		return -1;
	}

	*len_char = 0;

	while (i < len_hex)
	{
		sprintf(buff_char+*len_char, "0x%02x ", buff_hex[i]);
		*len_char += 5;
		i++;
	}
	return 0;
}



char * ip_num_to_char(unsigned long num, char *ip)
{
	sprintf(ip, "%0ld.%0ld.%0ld.%0ld", (num>>24)&0xFF, (num>>16)&0xFF, (num>>8)&0xFF, num&0xFF);
	return ip;
}

