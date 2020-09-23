/****************************************************************************
* ��Ȩ��Ϣ��
* ϵͳ���ƣ�SimServer
* �ļ����ƣ�msg.c
* �ļ�˵�������罻�����ĸ�ʽ�ӿ�ʵ���ļ�
* ��    �ߣ�hlzheng 
* �汾��Ϣ��v1.0 
* ������ڣ�
* �޸ļ�¼��
* ��    ��		��    ��		�޸��� 		�޸�ժҪ  
****************************************************************************/
#include "msg.h"
#include "zprint.h"





/**************************************************************************** 
* �������� : makeReqPackage
* �������� : ������������
* ��    �� : unsigned short type		: ���������
* ��    �� : unsigned short server_type	: ������������������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : unsigned short data_len	: �������ݳ���
* ��    �� : char *buff					: �������ݻ��壬�������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : makeRspPackage
* �������� : ��Ӧ���������
* ��    �� : char *buff_req				: ��������壬����洢��������
* ��    �� : char *buff_rsp				: ��Ӧ�����壬�������
* ��    �� : char *data					: ��������
* ��    �� : unsigned short data_len	: �������ݳ���
* ��    �� : unsigned short result		: ���ĳɹ���־
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : checkPackage
* �������� : ����У���麯��
* ��    �� : char *buff					: ���Ļ���
* ��    �� : int len					: ���ĳ���
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : getVersion
* �������� : ��ȡ���İ汾����
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ���İ汾��
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getVersion(char *buff)
{
	return ntohs(((unsigned short *)buff)[0]);
}

/**************************************************************************** 
* �������� : getCmd
* �������� : ��ȡ�������ͺ���
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : �������ͺ�
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getCmd(char *buff)
{
	return ntohs(((unsigned short *)buff)[1]);
}

/**************************************************************************** 
* �������� : getLen
* �������� : ��ȡ���ĸ��س��Ⱥ���
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ���ĸ��س���
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getLen(char *buff)
{
	return ntohs(((unsigned short *)buff)[2]);
}

/**************************************************************************** 
* �������� : getResult
* �������� : ��ȡ���Ľ���뺯��
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ���Ľ����
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getResult(char *buff)
{
	return ntohs(((unsigned short *)buff)[3]);
}

/**************************************************************************** 
* �������� : getServerType
* �������� : ��ȡ�������������������
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : �������������������
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getServerType(char *buff)
{
	return ntohs(((unsigned short *)buff)[5]);
}

/**************************************************************************** 
* �������� : getSerialNo
* �������� : ��ȡ���Ľ���뺯��
* ��    �� : char *buff					: ���Ļ���
* ��    �� : char *seri					: ����Emu���к�
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int getSerialNo(char *buff, char *seri)
{
	memcpy(seri, buff+(6*sizeof(unsigned short)), 10);
	return 0;
}

/**************************************************************************** 
* �������� : getReserve
* �������� : ��ȡ����Ԥ�ú���
* ��    �� : char *buff					: ���Ļ���
* �� �� ֵ : ����Ԥ��λ
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
unsigned short getReserve(char *buff)
{
	return ntohs(((unsigned short *)buff)[4]);
}

/**************************************************************************** 
* �������� : checksum
* �������� : У�麯��
* ��    �� : unsigned short *buffer		: ���Ļ���
* ��    �� : int size					: ���ĳ���
* �� �� ֵ : У����1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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
* �������� : makeRegisterPack
* �������� : ע����������
* ��    �� : char *buff_req				: ע����������壬�������
* ��    �� : unsigned short server_type	: ������ķ���������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : int data_len				: �������ݳ���
* ��    �� : int len					: ע����������峤�ȣ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeRegisterPack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(REGISTER_REQ, data, data_len, buff);
	return *len;
}

/**************************************************************************** 
* �������� : makeUnRegisterPack
* �������� : ע�����������
* ��    �� : char *buff_req				: ע����������壬�������
* ��    �� : unsigned short server_type	: ������ķ���������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : int data_len				: �������ݳ���
* ��    �� : int len					: ע����������峤�ȣ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
 *****************************************************************************/
int makeUnRegisterPack(char *buff, char *data, int data_len, int *len)
{
	*len = makeReqPackage(UNREGISTER_REQ, data, data_len, buff);
	return *len;
}

/**************************************************************************** 
* �������� : makeHeartbeatPack
* �������� : �������������
* ��    �� : char *buff_req				: ������������壬�������
* ��    �� : unsigned short server_type	: ������ķ���������
* ��    �� : char *seri					: SimEmuSvr���к�
* ��    �� : char *data					: ��������
* ��    �� : int data_len				: �������ݳ���
* ��    �� : int len					: ������������峤�ȣ��������
* �� �� ֵ : �ɹ�����0��ʧ�ܷ���-1
* ��    �� : hlzheng 
* ������� : 
* �޸�����		  �޸���		   �޸�����  
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

