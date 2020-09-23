/*********************************
	数据打包，以及串口接口
**********************************/
#include "platform_def.h"
#include "openvox_process_bar.h"
#include <stdlib.h>

#include "serialif.h"
#include "download.h"
#include "file.h"
#include <stdio.h>

#ifdef TARGET_OS_WINDOWS
#include "os_windows.h"
#endif

#if defined(TARGET_OS_LINUX) ||defined(TARGET_OS_ANDROID)
#include <string.h>
#include "os_linux.h"
#include <assert.h>
#endif 

HANDLE hCom; //串口句柄

#define ASYNC_HDLC_FLAG 0x7E
#define ASYNC_HDLC_ESC 0x7D
#define ASYNC_HDLC_ESC_MASK 0x20
#define MAX_RECEIVE_BUFFER_SIZE 2048   //一个数据包最大SIZE
#define MAX_SEND_BUFFER_SIZE 2048

byte  dloadbuf[250]; //临时缓存的一个buffer
byte g_Receive_Buffer[MAX_RECEIVE_BUFFER_SIZE];     //收到的buf
int g_Receive_Bytes;							//收到buffer的大小

byte g_Transmit_Buffer[MAX_SEND_BUFFER_SIZE];	//发送的代码
int g_Transmit_Length;							//发送的buffer 大小

//CRC 校验部分代码

#define CRC_16_L_SEED           0xFFFF

#define CRC_TAB_SIZE    256             /* 2^CRC_TAB_BITS      */

const uint16 crc_16_l_table[ CRC_TAB_SIZE ] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#define CRC_16_L_POLYNOMIAL     0x8408

uint16 crc_16_l_calc
(
 byte *buf_ptr,
 /* Pointer to bytes containing the data to CRC.  The bit stream starts
 ** in the LS bit of the first byte.
 */

 uint16 len
 /* Number of data bits to calculate the CRC over */
 )
{
	uint16 data, crc_16;

	/* Generate a CRC-16 by looking up the transformation in a table and
	** XOR-ing it into the CRC, one byte at a time.
	*/
	for (crc_16 = CRC_16_L_SEED ; len >= 8; len -= 8, buf_ptr++) {
		crc_16 = crc_16_l_table[ (crc_16 ^ *buf_ptr) & 0x00ff ] ^ (crc_16 >> 8);
	}

	/* Finish calculating the CRC over the trailing data bits
	**
	** XOR the MS bit of data with the MS bit of the CRC.
	** Shift the CRC and data left 1 bit.
	** If the XOR result is 1, XOR the generating polynomial in with the CRC.
	*/
	if (len != 0) {

		data = ((uint16) (*buf_ptr)) << (16-8); /* Align data MSB with CRC MSB */

		while (len-- != 0) {
			if ( ((crc_16 ^ data) & 0x01) != 0 ){   /* Is LSB of XOR a 1 */

				crc_16 >>= 1;                   /* Right shift CRC         */
				crc_16 ^= CRC_16_L_POLYNOMIAL;  /* XOR polynomial into CRC */

			} else {

				crc_16 >>= 1;                   /* Right shift CRC         */

			}

			data >>= 1;                       /* Right shift data        */
		}
	}

	return( ~crc_16 );            /* return the 1's complement of the CRC */

} /* end of crc_16_l_calc */

void  compute_reply_crc ()
{
	uint16 crc = crc_16_l_calc (g_Transmit_Buffer, g_Transmit_Length * 8);
	g_Transmit_Buffer[g_Transmit_Length] = crc & 0xFF;
	g_Transmit_Buffer[g_Transmit_Length + 1] = crc >> 8;
	g_Transmit_Length += 2;
}

static unsigned char send_tmp[2500];
static int send_tmp_length = 0;

int qdl_atoi(const byte *num_str, int len)
{
	int number = 0;
	int i = len;

	assert(len == 1 || len == 2 || len == 4);
	
	for(; i > 0; i--)
		number = (number << (i * 8) | num_str[i - 1]); 
	
	return number; 
}

static int send_packet(int flag)
{
	int i;
	int ch;
	int bytes_sent = 0;

	memset(send_tmp, 0,  2500);
	send_tmp_length = 0;
	
	/* Since we don't know how long it's been. */
	if(flag == 1){
		//send_transmit (0x7e);
		send_tmp[send_tmp_length++] = 0x7e;
	}
	
	for (i = 0; i < g_Transmit_Length; i++)
	{
		
		ch = g_Transmit_Buffer[i];
		
		if (ch == 0x7E || ch == 0x7D)
		{
			//send_transmit (0x7D);
			//send_transmit (0x20 ^ ch);        /*lint !e734 */
			send_tmp[send_tmp_length++] = 0x7d;
			send_tmp[send_tmp_length++] = 0x20^ ch;
			
			//printf("write data get 0x7E[%x]\r\n", (byte) ch);
		}
		else
		{
			//send_transmit (ch);       /*lint !e734 */
			send_tmp[send_tmp_length++] = ch;
		}
	}
	
	//send_transmit (0x7E);
	send_tmp[send_tmp_length++] = 0x7e;

	bytes_sent=WriteABuffer(hCom, send_tmp, send_tmp_length);

	if(bytes_sent == send_tmp_length)
		return TRUE;
	else
	{
		QdlContext->text_cb("send_pck, want :%d, sent :%d", send_tmp_length, bytes_sent);
		return FALSE;
	}

}

static void clean_buffer(void)
{
	memset(g_Receive_Buffer,0,sizeof(g_Receive_Buffer));
	g_Receive_Bytes=0;
}

#define MAX_RETRY_CNT_FOR_RECV_PKT   (5)

int receive_packet(void)
{
	int retry_cnt = MAX_RETRY_CNT_FOR_RECV_PKT;

	int result=false;
	int BeReceiveBytes;
	byte * ReceiveByte=g_Receive_Buffer;
	byte escape_state = 0;

	enum{
		TAG_UNDEFINED,
		TAG_START,
		TAG_DATA,
		TAG_END,
	}resp_tag;
	
	clean_buffer();	

	int j = 0;
	int i = 0;
	resp_tag = TAG_UNDEFINED;

start_recv_resp:
	
	BeReceiveBytes=ReadABuffer(hCom, ReceiveByte, MAX_RECEIVE_BUFFER_SIZE);
	for(i=0; i < BeReceiveBytes; i++)
	{
		if(((ReceiveByte[i] == ASYNC_HDLC_FLAG) && (resp_tag == TAG_DATA)) ||
			((ReceiveByte[i] == ASYNC_HDLC_FLAG) && (resp_tag == TAG_START)))
		{
			resp_tag = TAG_END;
			result = true;
			goto ret_data_recv;
		}	
		else if((ReceiveByte[i] == ASYNC_HDLC_FLAG) && (resp_tag != TAG_DATA))
		{
			resp_tag = TAG_START;
			continue;
		}
		else
		{
			resp_tag = TAG_DATA;
	  		switch (ReceiveByte[i])
	  		{
	  			//deal with some change data come from target
	  		case 0x7D:
	  			escape_state = 1;
	  			break;
	  
	  		default:
	  			if (escape_state)
	  			{
	  				g_Receive_Buffer[j] = (ReceiveByte[i]^= 0x20);
	  				escape_state = 0;
	  			}
				else
					g_Receive_Buffer[j] = ReceiveByte[i];

				j++;
			     //   QdlContext->text_cb("recv[%d,%d,%x]", g_Receive_Buffer[0], j,ReceiveByte[i]);
	  			break;
	  		}
		}
	}
	
ret_data_recv:
	if(resp_tag != TAG_END)
	{
		if(BeReceiveBytes <= 0) 
		{
	      		retry_cnt--;
	      		if(retry_cnt)
	      		{
	      			goto start_recv_resp;
	      		}
		}
		else
		{
			retry_cnt = MAX_RETRY_CNT_FOR_RECV_PKT;
			ReceiveByte += j;
			goto start_recv_resp;
		}
	}

	if(retry_cnt ==0 )
	{
		QdlContext->text_cb("timeout[%d]", BeReceiveBytes);
	}

	//g_Receive_Bytes=(uint32)ReceiveByte-(uint32)g_Receive_Buffer;	
	g_Receive_Bytes = j;
	
	return result;
}

/////////////////


int read_buildId(void)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 124;
	g_Transmit_Length= 1;
	compute_reply_crc();
	send_packet(0);
	do{
		if(receive_packet() == 1)
		{
			memset(dloadbuf,0,sizeof(dloadbuf));
			int i=0,index = 0;
			for(i;i<=g_Receive_Bytes;i++)
			{
				if(g_Receive_Buffer[i] != '\0') 
				{
					dloadbuf[index++] =g_Receive_Buffer[i] ;
				}
			}
				/*memset(g_Receive_Buffer,0,sizeof(g_Receive_Buffer));
				memcpy(g_Receive_Buffer,dloadbuf,index);
				printf("Version:%s\r\n",&g_Receive_Buffer[1]);
				sprintf(dload_status,"Version:%s\r\n",&g_Receive_Buffer[1]);
				save_log(dload_status);*/
				break;
			}
		}while(1);
	
	return 1;
}
/*******获取模块当前模式**********/
target_current_state send_sync(void)		
{
	const static byte noppkt[]={0x7e,0x06,0x4e,0x95,0x7e};
	const static byte fpkt[]={0x15,0x06,0x58,0x81,0x7e};
	const static byte tpkt[]={0x7e,0x02,0x6a,0xd3,0x7e};
	const static byte ipkt[]={0x7e,0x0e};
	target_current_state result;
	DWORD dwRead;	
	DWORD dwToRead;	
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	unsigned char pBuf[100];
	int timeout=10;
	HANDLE file;
	BOOL rc;
	OVERLAPPED overlap={0};

	qdl_flush_fifo(hCom, 1, 1);
	
	WriteABuffer(hCom, (unsigned char *)noppkt, sizeof(noppkt));

	rc=ReadABuffer(hCom, pBuf,sizeof(noppkt));
		QdlContext->text_cb( "send_sync ReadABuffer 0x%x,0x%x,0x%x,0x%x",pBuf[0],pBuf[1],pBuf[2],pBuf[3]);

	if(!memcmp(pBuf, tpkt, sizeof(tpkt)))
	{
		result=STATE_DLOAD_MODE;
		QdlContext->text_cb( "in dload mode");
	}	
	else if(!memcmp(pBuf, fpkt, sizeof(fpkt)))
	{
		result=STATE_NORMAL_MODE;
		QdlContext->text_cb( "in normal mode");
	}	
	else if(!memcmp(pBuf, ipkt, sizeof(ipkt)))
	{
		result=STATE_GOING_MODE;
		QdlContext->text_cb( "in going mode");
	}		
	else{
		  result=STATE_UNSPECIFIED;
  		QdlContext->text_cb( "in STATE_UNSPECIFIED");
      result=STATE_DLOAD_MODE;
	  }

	return result;	
}

int read_version(void)
{
	int retry_cnt = 5;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x0;
	g_Transmit_Length = 1;
	compute_reply_crc();

start_read_version:
	
	qdl_flush_fifo(hCom, 1, 1);
		
	send_packet(0);

	if(receive_packet() == 1){
		switch(g_Receive_Buffer[0])
		{
		case 0:
			return 1;
		default:
			goto retry_again;
		}
	}
	else {
			
retry_again:
		retry_cnt--;

		if(retry_cnt > 0)
		{
			QdlContext->text_cb("send hello error, try again[%d]", retry_cnt);
			goto start_read_version;
		}
		else
		{
			QdlContext->text_cb("send hello failed");
			return 0;
		}
	}
			
	return 1;
}

int read_flash_info(void)
{
	int retry_cnt = 5;
	int err_code = 0; 
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x4B;
	g_Transmit_Buffer[1] = 0x13;
	g_Transmit_Buffer[2] = 0x15;
	g_Transmit_Buffer[3] = 0x00;
	g_Transmit_Length = 4;
	compute_reply_crc();
	
start_read_flash_info:
	
	qdl_flush_fifo(hCom, 1, 1);
		
	send_packet(0);
	
	if(receive_packet() == 1){
	
		err_code = qdl_atoi(&g_Receive_Buffer[4], 4);
	
		if(err_code)   /*error*/
			goto retry_again;
	}
	else {
			
retry_again:
		retry_cnt--;

		if(retry_cnt > 0)
		{
			QdlContext->text_cb("send read_flash_info command error, try again[%d]", retry_cnt);
			goto start_read_flash_info;
		}
		else
		{
			QdlContext->text_cb("send read_flash_info command failed");
			return 0;
		}
	}
			
	return 1;
}

#if 0
int find_char(char *p1,char *p2,int *n)//从p1里查找p2
{
	int flag=0;//标志是否存在
	int loca;//标志存在位序
	int len=0;//记录主串走的位置的
	int len1,len2;//分别记录二个串的长度
	char *t1,*t2;
	int len_2;//标志2走了的位置
	t1=p1;
	t2=p2;
	len1=strlen(t1);
	len2=strlen(t2);
	if(len2>len1)//当查找的串比原串长
	{
		/*printf("\"");
		print(p1);
		printf("\"");
		printf("不包含:");
		printf("\"");
		print(p2);
		printf("\"");
		putchar(10);*/
		return 0;
	}
	else
	{
		while(strlen(t1)>=strlen(t2))
		{
			if(*t1!=*t2)
			{
				t1++;
				len++;
			}
			else
			{
				len_2=0;
				while(*t1==*t2&&*t2)
				{
					t1++;
					t2++;
					len_2++;
				}
				if(len2==len_2)
				{
					flag=1;//能查找到
					loca=len;//记录起始位置
					break;
				}
				else
				{
					len++;
					t1=p1+len;
					t2=p2;
				}
			}
		}
		if(flag==1)//查找到串
		{
			/*printf("\"");
			print(p1);
			printf("\"");
			printf("中包含\"");
			print(p2);
			printf("\"");
			putchar(10);
			printf("位置是%d个开始!\n",len+1);*/
			*n=len;
			return 1;
		}
	}
	return 0;
}


bool module_version(void)
{
	int result = 0;
	int mod_ver = 0;
	read_buildId();

	result = find_char(&dloadbuf[0],"6085",&mod_ver);
	if(result == 1)
	{
		is_amss_6085 = TRUE;
		target_ver = target_ver |0x0001;
	}
	result = find_char(&dloadbuf[0],"6290",&mod_ver);
	if(result == 1)
	{
		is_amss_6290 = TRUE;
		target_ver = target_ver |0x0002;
	}
	result = find_char(&dloadbuf[0],"6270",&mod_ver);
	if(result == 1)
	{
		is_amss_6270 = TRUE;
		target_ver =  target_ver |0x0004;
	}
	memcpy(&dloadbuf[mod_ver + 19] , "\r\n\0",3);
	printf("Build ID:%s\r\n",&dloadbuf[mod_ver]);
	//sprintf(dload_status,"Build ID:%s\r\n",&dloadbuf[mod_ver]);
	save_log(&dloadbuf[mod_ver]);
	if(mod_ver == NULL)
		return FALSE;
	else
		return TRUE;
}
#endif
int handle_erase_efs(void)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x1b;
	g_Transmit_Buffer[1] = 0x09;
	g_Transmit_Length = 2 ;
	compute_reply_crc();
	send_packet(1);
	do{
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x1c:
				return 1;
			default:
				return 0;
			}
		}
	}while(1);
	return 1;

}

int normal_reset(void)
{
	int mode = 2;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x29;
	g_Transmit_Buffer[1] = (mode)&0xff;	   
	g_Transmit_Buffer[2] = (mode>>8)&0xff;
	g_Transmit_Length = 3 ;
	compute_reply_crc();
	send_packet(1);
	//printf("reset to normal mode...\r\n");
	//save_log("reset to normal mode\r\n");
	return 1;

}


int handle_switch_target_offline(void)
{
	int mode = 2;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x29;
	g_Transmit_Buffer[1] = (mode)&0xff;	   
	g_Transmit_Buffer[2] = (mode>>8)&0xff;
	g_Transmit_Length = 3 ;
	compute_reply_crc();
	send_packet(1);
	do{
		//printf("switch to offline mode, please wait...\r\n");
		//save_log("switch to offline mode, please wait...\r\n");
		qdl_sleep(5000);
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x29:
			//	printf("target is in offline mode \r\n");
			//	save_log("target is in offline mode\r\n");
				return 1;
			default:
			//	printf("NV: SwitchToOffline failed !!!\r\n");
			//	save_log("NV: SwitchToOffline failed!!!\r\n");
				return -1;
			}
		}
	}while(1);
	return 1;

}

int handle_send_sp_code(void)
{
	int i = 0;
	int loop = 2;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x41;
	for( i = 1; i < 7; i++ )
		g_Transmit_Buffer[i] = 0x30;	

	g_Transmit_Length = i ;
	compute_reply_crc();
try_again:
	send_packet(1);
	loop--;
	do{
		//printf("set sp unlocked, please wait...\r\n");
		//save_log("set sp unlocked, please wait...\r\n");
		qdl_sleep(2000);
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x41:
				if(1 == g_Receive_Buffer[1] )
				{
					//printf("sp unlocked \r\n");
					//save_log("sp unlocked\r\n");
					return 1;
				}
				else
				{
					//printf(" SP is still locked \r\n");
					//save_log(" SP is still locked\r\n");
					if(loop!=0)
						goto try_again;
					
					return 0;//Code was incorrect and SP is still locked
				}
			default:
				return 0;
			}
		}
	}while(1);
	return 1;

}

int switch_to_dload(void)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x3a;
	g_Transmit_Length = 1;
	compute_reply_crc();

	qdl_flush_fifo(hCom, 1, 1);
		
	send_packet(0);
	return 1;
}

/******nop命令*******/
int send_nop(void)
{
	int retry_cnt = 5;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x06;
	g_Transmit_Length = 1;
	compute_reply_crc();

	qdl_flush_fifo(hCom, 1, 1);

start_send_nop:
	
	send_packet(1);
	
	if(receive_packet() == 1){
		switch(g_Receive_Buffer[0])
		{
		case 0x02:
			//QdlContext->text_cb("no is ok\r\n");
			return 1;
		case 0x0d:
			goto try_again;
		default:
			goto try_again;
		}
	}
	else{
try_again:
		retry_cnt--;

		if(retry_cnt > 0)
		{
			QdlContext->text_cb("send nop error, try again[%d]", retry_cnt);
			goto start_send_nop;
		}
		else
		{
			QdlContext->text_cb("send nop failed");
			return 0;
		}
	}

}

int preq_cmd(void)
{
	int retry_cnt = 5;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x07;
	g_Transmit_Length = 1;
	compute_reply_crc();

start_send_preq:
	
	//qdl_flush_fifo(hCom, 1, 1);
	
	send_packet(1);
	
	if(receive_packet() == 1){
		switch(g_Receive_Buffer[0])
		{
		case 0x08:
			//QdlContext->text_cb("preq_cmd is ok\r\n");
			return 1;
		default:
			goto try_again;
		}
	}
	else{
try_again:
		retry_cnt--;

		if(retry_cnt > 0)
		{
			QdlContext->text_cb("send preq error, try again[%d]", retry_cnt);
			goto start_send_preq;
		}
		else
		{
			QdlContext->text_cb("send preq failed");
			return 0;
		}
	}

	return 0;
}
/******发送hex文件*******/
int write_32bit_cmd(void)
{
	unsigned int base = 0x00800000;
	int i,j;
	unsigned char *pdst = NULL;
	unsigned char *pscr = NULL;
	int packet_length;
	pscr = (unsigned char *)nprghex;
	packet_length = nprghex_length;
	int size = 0x03f9;
	int retry_cnt = 10;

	qdl_flush_fifo(hCom, 1, 1);

	for(i=0;i<packet_length;)
	{
		memset(&g_Transmit_Buffer[0], 0, sizeof(g_Transmit_Buffer));
		g_Transmit_Length = 0;
		
		g_Transmit_Buffer[0] = 0x0f;
		g_Transmit_Buffer[1] = (base>>24)&0xff;
		g_Transmit_Buffer[2] = (base>>16)&0xff;
		g_Transmit_Buffer[3] = (base>>8)&0xff;
		g_Transmit_Buffer[4] = (base)&0xff;

		if((i+size)>packet_length){
			size = packet_length - i;
		}
		g_Transmit_Buffer[5] = (size>>8)&0xff;
		g_Transmit_Buffer[6] = (size)&0xff;
		
		pdst = &g_Transmit_Buffer[7];
		for(j=0; j<size; j++){
			*pdst++ = *pscr++;	
		}
		
		base += size;
		i += size;
		g_Transmit_Length = 7+size;
		compute_reply_crc();

start_send_cmd:
	
		send_packet(1);

		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x02:
//				QdlContext->prog_cb(i, packet_length,0);
				break;
			default:
				goto retry_again;
				break;
			}
		}
          	else{
	retry_again:
          		retry_cnt--;
          
          		if(retry_cnt > 0)
          		{
          			QdlContext->text_cb("send command error, try again[%d]", retry_cnt);
          			goto start_send_cmd;
          		}
          		else
          		{
          			QdlContext->text_cb("send command failed");
          			return 0;
          		}
          	}
	}
	QdlContext->prog_cb(i, packet_length,0);
	QdlContext->text_cb("");   /*"print \r\n"*/
	
	return 1;
}

/******dload模式切换到hex引导模式*******/
int go_cmd(void)
{
	unsigned int base = 0x00800000;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x05;
	g_Transmit_Buffer[1] = (byte)(base>>24)&0xff;
	g_Transmit_Buffer[2] = (byte)(base>>16)&0xff;
	g_Transmit_Buffer[3] = (byte)(base>>8)&0xff;
	g_Transmit_Buffer[4] = (byte)(base)&0xff;	   
	g_Transmit_Length = 5;
	compute_reply_crc();

	qdl_flush_fifo(hCom, 1, 1);

	send_packet(1);

	return 1;
}

int handle_hello(void)
{
	static const byte host_header[] = "QCOM fast download protocol host";
	char string1[64];
	int size;
	int err;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x01;		
	memcpy(&g_Transmit_Buffer[1],host_header,32);
	g_Transmit_Buffer[33] = 3;
	g_Transmit_Buffer[34] = 3;
	g_Transmit_Buffer[35] = 9;	
	g_Transmit_Length = 36;
	
	compute_reply_crc();
	
	int retry_cnt  = 50;
	
start_send_hello:
	
	qdl_flush_fifo(hCom, 1, 1);

	send_packet(1);

start_recv_resp:
	err = receive_packet();
	if(err == 1){
		switch(g_Receive_Buffer[0])
		{
		case 0x02:
			return 1;
		case 0x0d:
			goto start_recv_resp;
		default:
			goto try_again;
		}
	}
	else{
try_again:
		retry_cnt--;

		if(retry_cnt > 0)
		{
			QdlContext->text_cb("send hello error, try again[%d]", retry_cnt);
			goto start_send_hello;
		}
		else
		{
			QdlContext->text_cb("send hello failed");
			return 0;
		}
	}
	
	return 0;
}

int go_cmd_6085(void)
{
	int err;
	unsigned int base = 0x00800000;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x05;
	g_Transmit_Buffer[1] = (byte)(base>>24)&0xff;
	g_Transmit_Buffer[2] = (byte)(base>>16)&0xff;
	g_Transmit_Buffer[3] = (byte)(base>>8)&0xff;
	g_Transmit_Buffer[4] = (byte)(base)&0xff;	   
	g_Transmit_Length = 5;
	compute_reply_crc();
	send_packet(1);
	int timeout = 20;
	do{
		err = receive_packet();
		if(err == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x02:
				return 1;
			default:
				return -1;
			}
		}
		else if(err == -1){
			return 0;
		}
		timeout--;
	}while(timeout);
	return 1;
}

/******6085的hello要发2次*******/
int handle_hello_6085(void)
{
/*
	int err;
	static const byte host_header[] = "QCOM fast download protocol host";
	int size;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x01;		
	memcpy(&g_Transmit_Buffer[1],host_header,32);
	g_Transmit_Buffer[33] = 3;
	g_Transmit_Buffer[34] = 3;
	g_Transmit_Buffer[35] = 9;	
	g_Transmit_Length = 36;
	compute_reply_crc();
	send_packet(1);
	send_packet(1);
	//send_packet(1);
	int timeout = 20;
	do{
		err = receive_packet();
		if(err == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x02:
				memcpy(string,&g_Receive_Buffer[1],32);
				string[32] = 0;
				size = (g_Receive_Buffer[36]<<8) | g_Receive_Buffer[35];
				size = g_Receive_Buffer[43];
				memcpy(string,&g_Receive_Buffer[44],size);
				string[size] = 0;
				QdlContext->text_cb( "Flash_type:%s\r\n",string);
				return 1;
			case 0x0d:
				continue;
			default:
				return 0;
			}
		}
		else if(err == -1){
			return 0;
		}
		timeout--;
	}while(timeout);
	*/
	return 1;
}

int handle_security_mode(void)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x17;
	g_Transmit_Buffer[1] = 0x01;
	g_Transmit_Length = 2;
	compute_reply_crc();
	send_packet(1);
	do{
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x18:
				return 1;
			default:
				return 0;
			}
		}
	}while(1);
}

int handle_parti_tbl(int erase)
{
	unsigned char *pdst;
	unsigned char *pscr;
	char inChar;
	int i;
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x19;
	g_Transmit_Buffer[1] = erase;
	pscr = (unsigned char *)partition;  
	pdst = (unsigned char *)&g_Transmit_Buffer[2];
	for(i=0;i<partition_length;i++){
		*pdst++ = *pscr++;
	}
	g_Transmit_Length = 2+ partition_length;
	compute_reply_crc();
	send_packet(1);
	do{
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x1a:
				switch( g_Receive_Buffer[1] ){
					case 0x00:
						return 1;   //不擦
					case 0x01:
						return 2; //擦除
					break;
			default:
				return 0;
			}
			default:
				return 0;
			}
		}
	}while(1);
}

/******升级完成reset*******/

int handle_reset(void)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x0b;
	g_Transmit_Length= 1;
	compute_reply_crc();
	send_packet(1);
	do{
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x0c:
				return 1;
			default:
				return 0;
			}
		}
	}while(1);

}

/******打开分区*******/

void pkt_open_multi_image (byte mode, byte *data, uint32 size)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Length = 0;
	g_Transmit_Buffer[g_Transmit_Length++]  = 0x1b;	
	g_Transmit_Buffer[g_Transmit_Length++] = mode;
	for(int i = 0;i<size;i++)
	{
		g_Transmit_Buffer[g_Transmit_Length++]=data[i];
	}
	compute_reply_crc();
}
int handle_openmulti(int mode)
{	
	byte * data;
	uint32 size;

	switch(mode)
	{
	case OPEN_MULTI_MODE_QCSBLHDCFG:
	case OPEN_MULTI_MODE_QCSBL:
	case OPEN_MULTI_MODE_DBL:
	case OPEN_MULTI_MODE_OSBL:
	case OPEN_MULTI_MODE_FSBL:
	case OPEN_MULTI_MODE_LFS:
	case OPEN_MULTI_MODE_CEFS:			
		data=NULL;
		size=0;
		break;			
	case OPEN_MULTI_MODE_OEMSBL:
		data=oemsblhd;
		size=oemsblhd_length;
		break;
	case OPEN_MULTI_MODE_AMSS:
		data=amsshd;
		size=amsshd_length;
		break;

	default:
		data=NULL;
		size=0;
		break;
	}

	qdl_flush_fifo(hCom, TRUE, TRUE);

	pkt_open_multi_image(mode, data, size);
	send_packet(1);	
	do{
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x1c:
				return 1;
			default:
				return 0;
			}
		}
	}while(1);
	
	return 1;	
}

/******写分区*******/
void pkt_write_multi_image(uint32 addr, byte*data, uint16 size)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Length = 0;
	g_Transmit_Buffer[g_Transmit_Length++] = 0x07;
	g_Transmit_Buffer[g_Transmit_Length++] = (addr)&0xff;
	g_Transmit_Buffer[g_Transmit_Length++] = (addr>>8)&0xff;
	g_Transmit_Buffer[g_Transmit_Length++] = (addr>>16)&0xff;
	g_Transmit_Buffer[g_Transmit_Length++] = (addr>>24)&0xff;
	for(int i = 0;i<size;i++)
	{
		g_Transmit_Buffer[g_Transmit_Length++]=data[i];
	}
	compute_reply_crc();
}
int handle_write(int mode)
{	
	byte * data;
	uint32 size, total_size;
	uint32 addr=0;
	uint32 writesize;
#ifdef TARGET_OS_WINDOWS
	uint32 buffer_size = 1024;
#elif defined(TARGET_OS_LINUX) ||defined(TARGET_OS_ANDROID)
	uint32 buffer_size = 256;
#endif 
	int retry_cnt = 50;

	switch(mode)
	{
	case OPEN_MULTI_MODE_QCSBLHDCFG:
		data=qcsblhd_cfgdata;
		size=qcsblhd_cfgdata_length;
		break;
	case OPEN_MULTI_MODE_QCSBL:
		data=qcsbl;
		size=qcsbl_length;
		break;
	case OPEN_MULTI_MODE_OEMSBL:
		data=oemsbl;
		size=oemsbl_length;
		break;
		// add secureboot2.0 partition mode
	case OPEN_MULTI_MODE_DBL:
		data=dbl;
		size=dbl_length;
		break;
	case OPEN_MULTI_MODE_FSBL:
		data=fsbl;
		size=fsbl_length;
		break;
	case OPEN_MULTI_MODE_OSBL:
		data=osbl;
		size=osbl_length;
		break;
	case OPEN_MULTI_MODE_AMSS:
		data=amss;
		size=amss_length;
		break;
	default:
		data=NULL;
		size=NULL;
		break;
	}

	total_size = size;
		
	while(size)
	{
		writesize=(size<buffer_size)?size:buffer_size;
		pkt_write_multi_image(addr, data, writesize);
		
start_send_packet:
		send_packet(1);
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x08:
				size-=writesize;
				addr+=writesize;
				data+=writesize;
				UpgradeCount::getInstance()->set_write_size(writesize);
				UpgradeCount::getInstance()->process_bar(NULL);
				retry_cnt = 50;
				break;
			default:
				goto retry_send_packet;
			}
		}
		else
		{
	retry_send_packet:
			retry_cnt--;

			if(retry_cnt  > 0)
			{
				QdlContext->text_cb("send packet error, try again[%d]", retry_cnt);
				qdl_flush_fifo(hCom, 1, 1);
				qdl_sleep(1000);
				goto start_send_packet;
			}
			else
			{
				QdlContext->text_cb("send packet failed");
				return 0;
			}
		}
	}
	QdlContext->prog_cb(addr, total_size,0);//不需要实时更新
	return 1;	
}
/******关闭PARTITION*******/
int handle_close(void)
{
	memset(&g_Transmit_Buffer[0],0,sizeof(g_Transmit_Buffer));
	g_Transmit_Buffer[0] = 0x15;
	g_Transmit_Length = 1;
	compute_reply_crc();

	qdl_flush_fifo(hCom, 1, 1);
	
	send_packet(1);
	do{
		if(receive_packet() == 1){
			switch(g_Receive_Buffer[0])
			{
			case 0x16:
				return 1;
			default:
				return 0;
			}
		}
	}while(1);

}



