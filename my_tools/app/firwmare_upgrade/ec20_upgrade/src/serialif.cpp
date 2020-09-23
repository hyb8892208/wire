
#include "platform_def.h"
#include "os_linux.h"
#include "os_linux.h"
#include "download.h"
#include "quectel_common.h"
#include "quectel_log.h"
#include "serialif.h"
#include "file.h"
#include "quectel_crc.h"
#include<sys/time.h>


extern download_context *QdlContext;


int g_hCom; 
timeval start_my,end_my;

#define ASYNC_HDLC_FLAG 0x7E
#define ASYNC_HDLC_ESC 0x7D
#define ASYNC_HDLC_ESC_MASK 0x20
#define MAX_RECEIVE_BUFFER_SIZE 2048   
#define MAX_SEND_BUFFER_SIZE 2048

unsigned char  dloadbuf[250]; 
unsigned char g_Receive_Buffer[MAX_RECEIVE_BUFFER_SIZE];     
int g_Receive_Bytes;                            

unsigned char g_Transmit_Buffer[MAX_SEND_BUFFER_SIZE];    
int g_Transmit_Length;                            

static unsigned char send_tmp[2500];
static int send_tmp_length = 0;


void  compute_reply_crc ()
{
    uint16 crc = crc_16_l_calc (g_Transmit_Buffer, g_Transmit_Length * 8);
    g_Transmit_Buffer[g_Transmit_Length] = crc & 0xFF;
    g_Transmit_Buffer[g_Transmit_Length + 1] = crc >> 8;
    g_Transmit_Length += 2;
}

static void compose_packet(unsigned char cmd, unsigned char *parameter, uint32 parameter_len, unsigned char *data, uint32 data_len) {
    int i;

    g_Transmit_Buffer[0] = cmd;
    if (parameter == NULL) parameter_len = 0;
    if (data == NULL) data_len = 0;
    for (i = 0; i < parameter_len; i++) {
        g_Transmit_Buffer[1 + i] = parameter[i];
    }
    for (i = 0; i < data_len; i++) {
        g_Transmit_Buffer[1 + parameter_len + i] = data[i];
    }
    g_Transmit_Length = 1 + parameter_len + data_len;
    g_Transmit_Buffer[g_Transmit_Length] = 0;
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
        send_tmp[send_tmp_length++] = 0x7e;
    }
    
    for (i = 0; i < g_Transmit_Length; i++)
    {
        
        ch = g_Transmit_Buffer[i];
        
        if (ch == 0x7E || ch == 0x7D)
        {
            send_tmp[send_tmp_length++] = 0x7d;
            send_tmp[send_tmp_length++] = 0x20^ ch;          
        }
        else
        {
            send_tmp[send_tmp_length++] = ch;
        }
    }
    send_tmp[send_tmp_length++] = 0x7e;

    bytes_sent = WriteABuffer(g_hCom, send_tmp, send_tmp_length);
    if(bytes_sent == send_tmp_length)
    {
    	return 0;
    }
    else
    {        
        return 1;
    }

}

static void clean_buffer(void)
{
    memset(g_Receive_Buffer,0,sizeof(g_Receive_Buffer));
    g_Receive_Bytes=0;
}
#ifdef FEATURE_FAST_DOWNLOAD
#define MAX_TIMEOUT_FOR_RECV_PKT     (3000)
#define MAX_RETRY_CNT_FOR_RECV_PKT    (50)
#endif


unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int receive_packet(void)
{
    int timeout = 30000;
    int result = false;
    BOOL bBool;
    uint32 BeReceiveBytes;
    unsigned char * ReceiveByte = g_Receive_Buffer;
    unsigned char escape_state = 0;
    clean_buffer();


    BeReceiveBytes = ReadABuffer(g_hCom, ReceiveByte, 1);

    if (BeReceiveBytes == 1 && (*ReceiveByte != ASYNC_HDLC_FLAG)) {

        ReceiveByte += BeReceiveBytes;
    }
    unsigned long t_read_s, t_read_e;
    t_read_s = t_read_e = GetTickCount();
    while (timeout > 0) {
        t_read_e = GetTickCount();
        if ((t_read_e - t_read_s) > 5000) {
            return 0;
        }
        BeReceiveBytes = ReadABuffer(g_hCom, ReceiveByte, 1);

        if ((int) BeReceiveBytes <= 0) {
            timeout--;
            continue;
        }
        if (BeReceiveBytes == 1) {
            if (*ReceiveByte == ASYNC_HDLC_FLAG) {
                //timeout=0;
                result = true;
                break;
            }

            switch (*ReceiveByte) {
            //deal with some change data come from target
            case 0x7D:
                escape_state = 1;
                break;

            default:
                if (escape_state) {
                    (*ReceiveByte) ^= 0x20;
                    escape_state = 0;
                }
                ReceiveByte += BeReceiveBytes;
                //break;
            }
        } else {
            return false;
        }

        timeout--;
    }

    if (timeout == 0) {
        return 0;
    }

    g_Receive_Bytes = ReceiveByte - g_Receive_Buffer;
    if (g_Receive_Buffer[0] == ASYNC_HDLC_FLAG) {
        return 0;
    }
    return result;
}


static void handle_sahara_protocol(unsigned char *rx_buff, target_current_state* state_)
{
	sahara_header* header = (sahara_header*)rx_buff;
	boot_sahara_cmd_id id = (boot_sahara_cmd_id)header->Command;
	switch(id)
	{
		case SAHARA_HELLO_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_HELLO_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_READ_DATA_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_READ_DATA_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_DONE_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_HELLO_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_END_IMAGE_TX_ID:
			{
				end_of_image_transfer_packet* ptr = (end_of_image_transfer_packet*)rx_buff;
				QFLASH_LOGD("Sahara Command = SAHARA_END_IMAGE_TX_ID\n");
				QFLASH_LOGD("end_of_image_transfer_packet->Command = %d\n", ptr->Command);
				QFLASH_LOGD("end_of_image_transfer_packet->Length = %d\n", ptr->Length);
				QFLASH_LOGD("end_of_image_transfer_packet->ImageID = %08x\n", ptr->ImageID);
				QFLASH_LOGD("end_of_image_transfer_packet->Status=%08x\n", ptr->Status);
				*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_RESET_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_RESET_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_MEMORY_DEBUG_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_MEMORY_DEBUG_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_MEMORY_READ_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_MEMORY_READ_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_CMD_READY_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_CMD_READY_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_CMD_SWITCH_MODE_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_CMD_SWITCH_MODE_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_CMD_EXEC_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_CMD_EXEC_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_CMD_EXEC_DATA_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_CMD_EXEC_DATA_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_64_BITS_MEMORY_DEBUG_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_64_BITS_MEMORY_DEBUG_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_64_BITS_MEMORY_READ_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_64_BITS_MEMORY_READ_ID\n");*state_ = STATE_SAHARA_MODE;
			}
			break;
		case SAHARA_64_BITS_READ_DATA_ID:
			{
				QFLASH_LOGD("Sahara Command = SAHARA_64_BITS_READ_DATA_ID\n");
				*state_ = STATE_SAHARA_MODE;
			}
			break;
		default:
			{
				QFLASH_LOGD("Sahara Command = Unknown\n");
				*state_ = STATE_UNSPECIFIED;
			}
			break;
	}
}
int ignore_dirty_data()
{
	int ret;
	unsigned char rxbuff[512];
	ret = ReadABuffer(g_hCom, rxbuff,sizeof(rxbuff));
	if(ret > 0)
	{
		printf("clear dirty data in rx buffer\n");
		return 0;
	}
	return 1;
}


target_current_state send_sync(void)        
{
    const static unsigned char noppkt[] = {0x7e,0x06,0x4e,0x95,0x7e};
    const static unsigned char tpkt[] = {0x7e,0x02,0x6a,0xd3,0x7e};

    const static unsigned char ipkt[] = {0x7e,0x0e};
    const static unsigned char fpkt[] = {0x13,0x06,0x88,0xd5,0x7e};

    const static unsigned char fpkt2[] = {0x7e, 0x0d};

    target_current_state result = STATE_UNSPECIFIED;
    unsigned char pBuf[100] = {0};
    int rl = 0, wl = 0;
    qdl_flush_fifo(g_hCom, 1, 1,0); 
    
    wl = WriteABuffer(g_hCom, (unsigned char *)noppkt, sizeof(noppkt));
    if(wl == 0)
    {
        printf("send_sync write failed\n");
        return result;
    }
    qdl_sleep(500);
   
    rl = ReadABuffer(g_hCom, pBuf,sizeof(pBuf));
    if(rl < 0)
    {
        printf("send_sync read failed\n");
        return result;
    }
    if(rl == 0)
    {
    	printf("send_sync read nothing\n");
    	return STATE_UNSPECIFIED;
    }

    if(!memcmp(pBuf, tpkt, sizeof(tpkt)))
    {
        result = STATE_DLOAD_MODE;
        printf("The module in download mode\n");
    }    
    else if(!memcmp(pBuf, fpkt, sizeof(fpkt)))
    {
        result = STATE_NORMAL_MODE;
        printf("The module in normal mode\n");
    }    
    else if(!memcmp(pBuf, ipkt, sizeof(ipkt)) ||
    		!memcmp(pBuf, fpkt2, sizeof(fpkt2)))
    {
        result = STATE_GOING_MODE;
        printf("The module in go mode\n");
    }        
    else
    {
    	//result = STATE_UNSPECIFIED;
    	handle_sahara_protocol(pBuf, &result);  
    	if(result == STATE_SAHARA_MODE)
		{
			printf("The module in sahara mode\n");
		}
    	
    }
    return result;    
}





int normal_reset(void) {
    unsigned char mode[2] = {2, 0};
    compose_packet(0x29, mode, sizeof(mode), NULL, 0);
    compute_reply_crc();
    send_packet(1);
    return 1;

}


int handle_switch_target_ftm(void) {
    unsigned char mode[2] = {1, 0};
    compose_packet(0x29, mode, sizeof(mode), NULL, 0);
    compute_reply_crc();
    send_packet(1);
    do{
        sleep(5);
        if(receive_packet() == 1){
            switch(g_Receive_Buffer[0])
            {
            case 0x29:
                return 1;
            default:
                return -1;
            }
        }
    }while(1);
    return 1;

}


int switch_to_dload(void) {
    compose_packet(0x3a, NULL, 0, NULL, 0);
    compute_reply_crc();

    qdl_flush_fifo(g_hCom, 1, 1,0);
        
    int re = send_packet(0);
    return re;
}

int preq_cmd(void) {
    compose_packet(0x07, NULL, 0, NULL, 0);
    compute_reply_crc();

    qdl_flush_fifo(g_hCom, 1, 1,0);
    
    send_packet(1);
    do{
        if(receive_packet() == 1){
            switch(g_Receive_Buffer[0])
            {
            case 0x08:
                return 1;
            default:
                return 0;
            }
        }
    }while(1);
}



int handle_hello(void)
{
    static const unsigned char host_header[] = "QCOM fast download protocol host";
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

    qdl_flush_fifo(g_hCom, 1, 1,0);
    
    send_packet(1);
    int timeout = 2000;
    do{
        err = receive_packet();
        if(err == 1){
            switch(g_Receive_Buffer[0])
            {
            case 0x02:
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
    return 1;
}

int handle_security_mode(unsigned char trusted) {
    compose_packet(0x17, &trusted, 1, NULL, 0);
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
/*
set download flag in module, quectel custom command, 
if flag : reboot, module will enter DM
if not flag: reboot normal
*/
int handle_quectel_download_flag(byte mode) {
	//byte mode = 1;
	compose_packet(0x60, &mode, 1, NULL, 0);
	compute_reply_crc();
	send_packet(1);
	int timeout=5;
	do{
		if(receive_packet() == 1)
		{
			switch(g_Receive_Buffer[0])
			{
				case 0x61:
					switch( g_Receive_Buffer[1] )
						{
							case 0x00:
								return 1;
							default:
								return 0;
						}
					break;
				case 0x0E:
					printf("Invalid command");
					return 2;
				default:
					return 0;
			}
		}
		else
		{
			timeout--;
			if(timeout==0)
			{
				return 0;
			}
		}
	}while(1);
}

int handle_parti_tbl(unsigned char override) {
    uint32 filesize;
    unsigned char *partition=NULL;
	int type=0;
	
	partition= open_file(QdlContext->partition_path, &filesize);
    if (partition == NULL)
        return 0;
    compose_packet(0x19, &override, 1, partition, filesize);
    free_file(partition, filesize);
    compute_reply_crc();
    send_packet(1);
    
    do{
        if(receive_packet() == 1){
            QFLASH_LOGD("handle_parti_tbl command = %02x, status = %02x\n", g_Receive_Buffer[0], g_Receive_Buffer[1]);
            switch(g_Receive_Buffer[0])
            {
            case 0x1a:
                switch( g_Receive_Buffer[1] ){
                    case 0x00: 
                        return 1;  
                    case 0x01: //0x1 this means that the original partition is different from the current partition,try to send partition
						return 0; 
                    case 0x02: //0x2 Partition table format not recognized, does not accept override
                        return 0;
                    case 0x03: //0x3  Erase operation failed
                        return 0;
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

/******�����reset*******/

int handle_reset(void) {
    compose_packet(0x0b, NULL, 0, NULL, 0);
    compute_reply_crc();
    send_packet(1);
    int timeout=5;
    do{
        if(receive_packet() == 1){
            switch(g_Receive_Buffer[0])
            {
            case 0x0c:
                return 1;
            case 0x0d:
                    continue;
            default:
                return 0;
            }
        }
        else
                {

                    timeout--;
                    if(timeout==0)
                    {
                        return 0;
                    }
                }
    }while(1);

}

/******pkt_open_multi_image*******/

void pkt_open_multi_image (unsigned char mode, unsigned char *data, uint32 size) {
    compose_packet(0x1b, &mode, 1, data, size);
    compute_reply_crc();
}
int handle_openmulti(uint32 size,unsigned char* data)
{    
    unsigned char mode=0x0e;
    qdl_flush_fifo(g_hCom, true, true,0);
    pkt_open_multi_image(mode, data, size);
    send_packet(1);
    do {
        if (receive_packet() == 1) {
            switch (g_Receive_Buffer[0]) {
            case 0x1c:
                return 1;
            case 0x0d:
                continue;
            default:
                return 0;
            }
        }
    } while (1);
    return 1;    
}

/******pkt_write_multi_image*******/
void pkt_write_multi_image(uint32 addr, unsigned char*data, uint16 size) {
    unsigned char parameter[4] = {(unsigned char)(addr)&0xff, (unsigned char)(addr>>8)&0xff, (unsigned char)(addr>>16)&0xff, (unsigned char)(addr>>24)&0xff};
    compose_packet(0x07, parameter, 4, data, size);
    compute_reply_crc();
}

int handle_write(FILE *fp,  unsigned char *data, uint32 size)
{    
    uint32 total_size;
    uint32 addr=0;
    uint32 writesize;
    uint32 buffer_size = QdlContext->cache;
    int loop = 1;
    int retry_cnt=3;  //if send failed,send again
    int ret;

    total_size = size;
    while(size)
    {
        writesize=(size < buffer_size) ? size : buffer_size;
        if(fp!=NULL){
            fread((void *)data, 1, writesize, fp);
        }
        pkt_write_multi_image(addr, data, writesize);
start_send_packet:
        ret = send_packet(1);
        if(0 != ret)
        {
        	QFLASH_LOGD("io read/write failed\n");
        	return 0;
        }
        if(receive_packet() == 1){
            switch(g_Receive_Buffer[0])
            {
            case 0x08:
                size -= writesize;
                addr += writesize;
                QdlContext->process_cb(addr, total_size, 0);                
                transfer_statistics::getInstance()->set_write_bytes(writesize);
		        transfer_statistics::getInstance()->process_bar();
                //retry_cnt=5;
                break;
            default:
                goto retry_send_packet;
                return 0;
            }
        }
        else
        {

retry_send_packet:
            retry_cnt--;
            if(retry_cnt>0)
            {                                
                qdl_flush_fifo(g_hCom,1,1,0);
                qdl_sleep(1000);
                goto start_send_packet;
            }
            else
             {
                printf( "value is [0x%02x]",g_Receive_Buffer[0]);
                return 0;
            }
        }
    
    }

    return 1;    
}
/******PARTITION*******/
int handle_close(void) {
    compose_packet(0x15, NULL, 0, NULL, 0);
    compute_reply_crc();

    qdl_flush_fifo(g_hCom, 1, 1,0);
    
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

//sahara protocol
int ReadSAHARABuffer(int file, unsigned char * lpBuf, int dwToRead)
{
	int timeout=2;
	int rSize;
	while(1)
	{ 
		rSize=ReadABuffer(file, lpBuf, dwToRead);
		if(rSize > 0)
			return rSize;
		else
		{
			if (timeout == 0)
			{
                return 0;
			}
			timeout--;
		}
	}
}
int kick_sahara_state_machine()
{
	int ret;
	unsigned char temp[1] = {0};
	ret = WriteABuffer(g_hCom, temp, sizeof(temp));
	if(ret == sizeof(temp))
	{
		return 0;
	}
	return 1;
}
int get_sahara_hello_packet()
{
#if 0
	unsigned char RecvCommand[4] = {0};
	unsigned char RecvLength[4] = {0};
	unsigned char RecvData[MAX_RECEIVE_BUFFER_SIZE] = {0};
	//get command id
	if(ReadSAHARABuffer(g_hCom, RecvCommand, 4)!=4)
	{
		return 0;
	}
	int Command = (RecvCommand[3]<<24)|(RecvCommand[2]<<16)|(RecvCommand[1]<<8)|RecvCommand[0];
	//get length
	if(ReadSAHARABuffer(g_hCom, RecvLength, 4)!=4)
	{
		return 0;
	}
	if(Command != GET_HELLO_PACKET)
	{
		return 0;
	}	
	int length=(RecvLength[3]<<24)|(RecvLength[2]<<16)|(RecvLength[1]<<8)|RecvLength[0];
	//get data
	if(ReadSAHARABuffer(g_hCom, RecvData, length-8)!=(length-8))
	{
		return 0;
	}
	printf( "Command is [0x%02x],length is [0x%02x]",Command,length);
	return 1;
#else
	int ret = 0;
	sahara_header* header = 0;
	boot_sahara_cmd_id id = SAHARA_MAX_CMD_ID;
	unsigned char rx_buff[128];
	ret = ReadABuffer(g_hCom, rx_buff,sizeof(rx_buff));
	if(ret > sizeof(sahara_header))
	{
		header = (sahara_header*)rx_buff;
		id = (boot_sahara_cmd_id)header->Command;
		if(SAHARA_HELLO_ID == id)
		{
			return 0;
		}
	}else
	{
		return 1;
	}
	return 2;
#endif
}
int SendHelloPacketTest(int emergency_mode)
{
	if(!emergency_mode)
	{
		unsigned char zeroBuf[1024] = {0};
		if(WriteABuffer(g_hCom, zeroBuf,1024 ) != 1024)
		{
			QFLASH_LOGD("Send Zero packet error\n");
			return 0;
		}
		sleep(3);
	}
	return SendHelloPacket();
}

int send_sahara_hello_response_packet()
{
	unsigned CommandID=htonl(HELLO_RESPONSE_PACKET);
	unsigned Length=htonl(HELLO_RESPONSE_PACKET_LENGTH);
	unsigned VersionN=htonl(SHARA_PROCTOL_VERSON_NUM);
	unsigned VersionC=htonl(LOW_COMP_VERSON_NUM);
	unsigned State=htonl(SUCESS_OR_ERROR_STATE);
  	if(endian_flag)
    {
        CommandID=0x02000000;
        Length=0x30000000;
        VersionN=0x02000000;
        VersionC=0x01000000;
        State=0x00000000;
    }
	unsigned char SendBuffer[48]={0};
	int index=0;
	SendBuffer[index++]=(CommandID>>24)&0xff;
	SendBuffer[index++]=(CommandID>>16)&0xff;
	SendBuffer[index++]=(CommandID>>8)&0xff;
	SendBuffer[index++]=(CommandID)&0xff;
	
	SendBuffer[index++]=(Length>>24)&0xff;
	SendBuffer[index++]=(Length>>16)&0xff;
	SendBuffer[index++]=(Length>>8)&0xff;
	SendBuffer[index++]=(Length)&0xff;
	
		
	SendBuffer[index++]=(VersionN>>24)&0xff;
	SendBuffer[index++]=(VersionN>>16)&0xff;
	SendBuffer[index++]=(VersionN>>8)&0xff;
	SendBuffer[index++]=(VersionN)&0xff;
	
		
	SendBuffer[index++]=(VersionC>>24)&0xff;
	SendBuffer[index++]=(VersionC>>16)&0xff;
	SendBuffer[index++]=(VersionC>>8)&0xff;
	SendBuffer[index++]=(VersionC)&0xff;
	
		
	SendBuffer[index++]=(State>>24)&0xff;
	SendBuffer[index++]=(State>>16)&0xff;
	SendBuffer[index++]=(State>>8)&0xff;
	SendBuffer[index++]=(State)&0xff;
	qdl_flush_fifo(g_hCom, 1, 1,1);	
    if(WriteABuffer(g_hCom, SendBuffer,48 ) == 48)
	{
		return 0;
	}
	return 1;
}
int SendHelloPacket()
{
	unsigned CommandID=htonl(HELLO_RESPONSE_PACKET);
	unsigned Length=htonl(HELLO_RESPONSE_PACKET_LENGTH);
	unsigned VersionN=htonl(SHARA_PROCTOL_VERSON_NUM);
	unsigned VersionC=htonl(LOW_COMP_VERSON_NUM);
	unsigned State=htonl(SUCESS_OR_ERROR_STATE);
  if(endian_flag)
    {
        CommandID=0x02000000;
        Length=0x30000000;
        VersionN=0x02000000;
        VersionC=0x01000000;
        State=0x00000000;
    }
	unsigned char SendBuffer[48]={0};
	int index=0;
	SendBuffer[index++]=(CommandID>>24)&0xff;
	SendBuffer[index++]=(CommandID>>16)&0xff;
	SendBuffer[index++]=(CommandID>>8)&0xff;
	SendBuffer[index++]=(CommandID)&0xff;
	
	SendBuffer[index++]=(Length>>24)&0xff;
	SendBuffer[index++]=(Length>>16)&0xff;
	SendBuffer[index++]=(Length>>8)&0xff;
	SendBuffer[index++]=(Length)&0xff;
	
		
	SendBuffer[index++]=(VersionN>>24)&0xff;
	SendBuffer[index++]=(VersionN>>16)&0xff;
	SendBuffer[index++]=(VersionN>>8)&0xff;
	SendBuffer[index++]=(VersionN)&0xff;
	
		
	SendBuffer[index++]=(VersionC>>24)&0xff;
	SendBuffer[index++]=(VersionC>>16)&0xff;
	SendBuffer[index++]=(VersionC>>8)&0xff;
	SendBuffer[index++]=(VersionC)&0xff;
	
		
	SendBuffer[index++]=(State>>24)&0xff;
	SendBuffer[index++]=(State>>16)&0xff;
	SendBuffer[index++]=(State>>8)&0xff;
	SendBuffer[index++]=(State)&0xff;
	qdl_flush_fifo(g_hCom, 1, 1,1);	
    if(WriteABuffer(g_hCom, SendBuffer,48 )!=48)
	{
		return 0;
	}
	return 1;
}

int transfer_nrpg_or_enpgr_file()
{
	unsigned char *Data = NULL;
	unsigned char *tmp = NULL;
	unsigned filesize;
	int i = 0;
	int loadfile = 0;
	int need_free_memory = 0;
	int ret = 0;
	while(1)
	{
		
		unsigned char RecvCommand[4]={0};
		int size=ReadSAHARABuffer(g_hCom, RecvCommand, 4);
		unsigned Command=(RecvCommand[3]<<24)|(RecvCommand[2]<<16)|(RecvCommand[1]<<8)|RecvCommand[0];
		if(Command == GET_DATA_PACKET)
		{
			unsigned char RecvData[16]={0};
			unsigned offset;
			unsigned length;
			unsigned type;

			read_data_packet* ptr = (read_data_packet*)(RecvCommand);
			//get data offset & data length
			if(ReadSAHARABuffer(g_hCom, RecvData,16) != 16)
			{
				printf("sahara read bytes invalid\n");
				ret = 1;
				break;
			}
			type=(RecvData[7]<<24)|(RecvData[6]<<16)|(RecvData[5]<<8)|RecvData[4];
			
			if(loadfile == 0)
			{
				if(type==0x00000007)
				{
					Data= open_file(QdlContext->NPRG_path, &filesize);
					QFLASH_LOGD("Sahara send %s\n", QdlContext->NPRG_path);
					need_free_memory = 1;
				}
				else if(type==0x0000000D)
				{
					Data= open_file(QdlContext->ENPRG_path, &filesize);
					QFLASH_LOGD("Sahara send %s\n", QdlContext->ENPRG_path);
					need_free_memory = 1;
				}
				else
				{
					printf("unknown sahara id : %d\n", type);
					ret = 2;
					break;
				}				
				loadfile = 1;
				tmp = Data;
			}
			offset=(RecvData[11]<<24)|(RecvData[10]<<16)|(RecvData[9]<<8)|RecvData[8];
			length=(RecvData[15]<<24)|(RecvData[14]<<16)|(RecvData[13]<<8)|RecvData[12];
			Data = tmp;
			//read file(offset length),and send content
			Data += offset;
			i = offset+length;
			qdl_flush_fifo(g_hCom, 1, 1,0); //flush no finish data
			int size = length;
			if(length > 1024)
			{
				while(size != 0)
				{
					int writesize = (size<1024)?size:1024;
					if(WriteABuffer(g_hCom, Data,writesize )!=writesize)
					{
						printf("write sahara packet failed\n");
						ret = 3;
						break;
					}					
					size -= writesize;
					Data += writesize;
					//usleep(5);
				}
				
			}
			else
			{
				if(WriteABuffer(g_hCom, Data,length ) != length)
				{
					ret = 4;
					break;
				}
				
			}
		}
		else if(Command==END_IMAGE_TRNSER_PACKET)
		{
			unsigned char RecvData[12]={0};
			if(ReadSAHARABuffer(g_hCom, RecvData,12)!=12)
			{
				ret = 5;
				break;
			}
			unsigned state=(RecvData[11]<<24)|(RecvData[10]<<16)|(RecvData[9]<<8)|RecvData[8];
			if(state == 0x00000000)
			{	
				//transfer file success.
				ret = 0;
				break;
			}
			else
			{
				ret = 6;
				break;
			}
		}
		else if(Command==GET_HELLO_PACKET)
		{
			ret = 7;
			break;
		}
		else
		{
			QFLASH_LOGD("Can't read the data\r\n");
			ret = 8;
			break;
		}
			
	}

	if(need_free_memory)
	{
		free(tmp);
	}
	return ret;
}
int GetReadDataPacket(int *emergency_mode)
{
	unsigned char *Data = NULL;
	unsigned char *tmp = NULL;
	unsigned filesize;
	int i = 0;
	int flag = 0;
	int need_free = 0;
	while(1)
	{
		
		//Get read data packet
		unsigned char RecvCommand[4]={0};
		int size=ReadSAHARABuffer(g_hCom, RecvCommand, 4);
		unsigned Command=(RecvCommand[3]<<24)|(RecvCommand[2]<<16)|(RecvCommand[1]<<8)|RecvCommand[0];
		if(Command == GET_DATA_PACKET)
		{
			unsigned char RecvData[16]={0};
			unsigned offset;
			unsigned length;
			unsigned type;

			read_data_packet* ptr = (read_data_packet*)(RecvCommand);
			//get data offset & data length
			if(ReadSAHARABuffer(g_hCom, RecvData,16)!=16)
				return 0;
			type=(RecvData[7]<<24)|(RecvData[6]<<16)|(RecvData[5]<<8)|RecvData[4];
			
			if(flag==0)
			{
				if(type==0x00000007)
				{
					Data= open_file(QdlContext->NPRG_path, &filesize);
					QFLASH_LOGD("Sahara send %s\n", QdlContext->NPRG_path);
					need_free = 1;
				}
				else if(type==0x0000000D)
				{
					Data= open_file(QdlContext->ENPRG_path, &filesize);
					QFLASH_LOGD("Sahara send %s\n", QdlContext->ENPRG_path);
					need_free = 1;
					*emergency_mode = 1;
				}
				else
					return 0;
				
				flag = 1;
				tmp = Data;
			}
			offset=(RecvData[11]<<24)|(RecvData[10]<<16)|(RecvData[9]<<8)|RecvData[8];
			length=(RecvData[15]<<24)|(RecvData[14]<<16)|(RecvData[13]<<8)|RecvData[12];
			Data = tmp;
			//read file(offset length),and send content
			Data += offset;
			i=offset+length;
			qdl_flush_fifo(g_hCom, 1, 1,0); //flush no finish data
			int size=length;
			if(length > 2048)
			{
				while(size != 0)
				{
					int writesize = (size < 2048)?size : 2048;
					if(WriteABuffer(g_hCom, Data,writesize ) != writesize)
					{
						QFLASH_LOGD("-------------error-----------------\n");
						return 0;
					}
					
					size -= writesize;
					Data += writesize;
					//usleep(5);
				}
				
			}
			else
			{
				if(WriteABuffer(g_hCom, Data,length ) != length)
				{
					if(need_free)
					{
						free(tmp);
					}
					return 0;
				}
				
			}
			QdlContext->process_cb(i, filesize,0);
		}
		else if(Command==END_IMAGE_TRNSER_PACKET)
		{
			unsigned char RecvData[12]={0};
			if(ReadSAHARABuffer(g_hCom, RecvData,12)!=12)
			{
				if(need_free)
				{
					free(tmp);
				}
				return 0;
			}
			unsigned state=(RecvData[11]<<24)|(RecvData[10]<<16)|(RecvData[9]<<8)|RecvData[8];
			if(state == 0x00000000)
			{
				break;
			}
			else
			{
				if(need_free)
				{
					free(tmp);
				}
				return 0;
			}
		}
		else if(Command == GET_HELLO_PACKET)
		{
			if(need_free)
			{
				free(tmp);
			}
			return 2;
		}
		else
		{
			QFLASH_LOGD("Can't read the data\r\n");
			if(need_free)
			{
				free(tmp);
			}
			return 0;
		}
			
	}

	if(need_free)
	{
		free(tmp);
	}
	return 1;
}
int send_sahara_do_packet()
{
	unsigned char SendBuffer[8]={0x05,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
	qdl_flush_fifo(g_hCom, 1, 1,0);
	if(WriteABuffer(g_hCom, SendBuffer,8 )!=8)
		return 1;
	unsigned char RecvCommand[4]={0};
	if(ReadSAHARABuffer(g_hCom, RecvCommand, 4)!=4)
		return 2;
	unsigned Command=(RecvCommand[3]<<24)|(RecvCommand[2]<<16)|(RecvCommand[1]<<8)|RecvCommand[0];
	if(Command!=DONE_RESONSE_PACKET)
		return 3;
	unsigned char RecvData[8]={0};
	if(ReadSAHARABuffer(g_hCom, RecvData,8)!=8)
		return 4;
	unsigned state=(RecvData[7]<<24)|(RecvData[6]<<16)|(RecvData[5]<<8)|RecvData[4];
	if(state == 0x00000000)
		return 0;
	return 5;
}
int SendDoPacket()
{
	unsigned char SendBuffer[8]={0x05,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
	qdl_flush_fifo(g_hCom, 1, 1,0);
	if(WriteABuffer(g_hCom, SendBuffer,8 )!=8)
		return 0;
	unsigned char RecvCommand[4]={0};
	if(ReadSAHARABuffer(g_hCom, RecvCommand, 4)!=4)
		return 0;
	unsigned Command=(RecvCommand[3]<<24)|(RecvCommand[2]<<16)|(RecvCommand[1]<<8)|RecvCommand[0];
	if(Command!=DONE_RESONSE_PACKET)
		return 0;
	unsigned char RecvData[8]={0};
	if(ReadSAHARABuffer(g_hCom, RecvData,8)!=8)
		return 0;
	unsigned state=(RecvData[7]<<24)|(RecvData[6]<<16)|(RecvData[5]<<8)|RecvData[4];
	if(state!=0x00000000)
		return 0;
	return 1;
}

int SendResetPacket()
{
	unsigned char SendBuffer[8]={0x07,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
	qdl_flush_fifo(g_hCom, 1, 1,0);
	if(WriteABuffer(g_hCom, SendBuffer,8 )!=8)
		return 0;
	unsigned char RecvCommand[4]={0};
	if(ReadSAHARABuffer(g_hCom, RecvCommand, 4)!=4)
		return 0;
	unsigned Command=(RecvCommand[3]<<24)|(RecvCommand[2]<<16)|(RecvCommand[1]<<8)|RecvCommand[0];
	if(Command!=RESET_PACKET)
		return 0;
	unsigned char RecvData[8]={0};
	if(ReadSAHARABuffer(g_hCom, RecvData,8)!=8)
		return 0;
	unsigned state=(RecvData[7]<<24)|(RecvData[6]<<16)|(RecvData[5]<<8)|RecvData[4];
	if(state!=0x00000000)
		return 0;
	return 1;
}

int sahara_reset()
{
	unsigned char rx_buff[128];
	unsigned char SendBuffer[8]={0x07,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
	qdl_flush_fifo(g_hCom, 1, 1,0);
	if(WriteABuffer(g_hCom, SendBuffer,8 )!=8)
		return 0;
	if(ReadSAHARABuffer(g_hCom, rx_buff, sizeof(rx_buff) > 0))
	{
		
	}
	if(ReadSAHARABuffer(g_hCom, rx_buff, sizeof(rx_buff) > 0))
	{
		
	}
	if(ReadSAHARABuffer(g_hCom, rx_buff, sizeof(rx_buff) > 0))
	{
		
	}
}
int sahara_done()
{
	unsigned char rx_buff[128];
	unsigned char SendBuffer[8]={0x05,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
	qdl_flush_fifo(g_hCom, 1, 1,0);
	if(WriteABuffer(g_hCom, SendBuffer,8 )!=8)
		return 0;
	if(ReadSAHARABuffer(g_hCom, rx_buff, sizeof(rx_buff) > 0))
	{
		
	}
	if(ReadSAHARABuffer(g_hCom, rx_buff, sizeof(rx_buff) > 0))
	{
		
	}
	if(ReadSAHARABuffer(g_hCom, rx_buff, sizeof(rx_buff) > 0))
	{
		
	}
}

int retrieve_soft_revision()
{
	unsigned char req1[] = {0x7E,0x7C, 0x93,0x49, 0x7E};
	size_t len = sizeof(req1) / sizeof(req1[0]);
	size_t read_count = 0;
	unsigned char rx_buff[256] = {0};
	if(WriteABuffer(g_hCom, req1, len) != len)
	{
		return 1;
	}

	if((read_count = ReadSAHARABuffer(g_hCom, rx_buff, 256)) > 0)
	{
		extended_build_id_response_t* ptr = (extended_build_id_response_t*)rx_buff;
		if(ptr->cmd_code != 0x7C && rx_buff[read_count - 1] != 0x7E)
		{
			return 2;
		}else
		{
			QFLASH_LOGD("\n");
			QFLASH_LOGD("Software Revision = %s\n", &ptr->mobile_software_revision[0]);
			return 0;
		}
	}
}


