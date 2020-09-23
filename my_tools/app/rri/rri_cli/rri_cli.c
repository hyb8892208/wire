

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "public.h"
#include "rri_api.h"
#include "cli.h"

typedef struct 
{ 
	unsigned short usb_rx_full;      /* usb 接收溢出   */ 
	unsigned short usb_to_spi_full;  /* 语音下行buffer溢出次数 */ 
	unsigned short usb_to_spi_empty; /* 语音下行buffer缺包次数 */ 
	unsigned short spi_to_usb_full;  /* 语音上行buffer溢出次数 */ 
	unsigned short spi_to_usb_empty; /* 语音上行buffer缺包次数 */ 
	unsigned short usb_to_uart_full; /* AT下行buffer溢出字节数 */ 
	unsigned short uart_to_usb_full; /* AT上行buffer溢出字节数 */ 
	unsigned short garbage_pkt;      /* 从usb_rx_fifo取出但无法识别的数据包 */ 
	unsigned short is_voice_start;   /* 已经voice start ？ */ 
	unsigned short usb_rx_pkt;       /* usb 接收中断次数 */ 
	unsigned short voice_up_pkt;     /* voice上行包数 */ 
	unsigned short voice_down_pkt;   /* voice下行包数 */ 
	unsigned short at_up_pkt;        /* AT上行包数 */ 
	unsigned short at_down_pkt;      /* AT下行包数 */ 
	unsigned short spi_rx_cnt;       /* spi dma rx中断次数 */ 
	unsigned short spi_tx_cnt;       /* spi dma tx中断次数 */ 
	unsigned short uart_rx_cnt;      /* uart rx中断次数 */ 
	unsigned short uart_tx_cnt;      /* uart rx中断次数 */ 
}FIFO_REPORT;

#if T_DESC("tools")
/**********************************************************
函数描述 : 16进制字符转数值
输入参数 : c -- 字符，不作参数检查
输出参数 : 
返 回 值 : 数值，
作   者  : zhongwei.peng
时   间  : 2016.11.25
************************************************************/
unsigned char char_to_val(char c)
{
    if ( (c >= '0') && (c <= '9') )
        return c - '0';
    else if ( (c >= 'A') && (c <= 'Z') )
        return c - 'A' + 10;
    else if ( (c >= 'a') && (c <= 'z') )
        return c - 'a' + 10;
    else 
        return 0;
}

/**********************************************************
函数描述 : 字符串转数值
输入参数 : str -- 字符串
输出参数 : 
返 回 值 : 数值，
作   者  : zhongwei.peng
时   间  : 2016.11.25
************************************************************/
int str_to_int(char *str)
{
    int flag;
    int value = 0;
    char *p = str;

    if ( NULL == str )
        return 0;

    /* 16???? */
    if ( (str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')) )
    {
        value = 0;
        p = &str[2];
        while ( *p )
        {
            value = value * 16 + char_to_val(*p);
            p++;
        }
        return value;
    }

    /* 10???? */
    if ( str[0] == '-' )
        flag = -1;
    else
        flag = 1;

    value = 0;
    p = str;
    while ( *p ) 
        value = value * 10 + char_to_val(*p++);

    value = value * flag;

    return value;
}

/**********************************************************
函数描述 : 打印一段数据
输入参数 : 
输出参数 : 
返 回 值 : 
作   者  : zhongwei.peng
时   间  : 2016.11.07
************************************************************/
void dump_data(char *title, unsigned char *data, unsigned int len)
{
    unsigned int i;

    printf("%s\r\n", title);

    for ( i = 0; i < len; i++ )
    {
        if ( (i != 0) && (i % 4 == 0) )
        {
            if ( i % 8 == 0 )
                printf("\r\n");
            else
                printf("   ");
        }

        printf("%02x ", data[i] & 0xFF);
    }

    printf("\r\n");
}

#endif

#if T_DESC("rri")

/*
    命令行使用方法说明
*/
void cli_usage_rri(void)
{
    printf("server ver -- show server version\n");
    printf("server debug [val] -- set server debug switch\n");
    printf("chn_num get -- get channel number\n");
    printf("chn info [chn] -- get channel information\n");
    printf("chn_at debug [chn] [val] -- set channel debug value\n");
    printf("chn_snd debug [chn] [val] -- set channel debug value\n");
    printf("chn_snd speed set [chn] [speed] --set channel speed [speed]*8bytes/[speed]ms\n");
    printf("chn_snd speed get [chn]  --get channel speed \n");
    printf("chn_snd bufsize set [chn] [speed] --set channel snd bufsize\n");
    printf("chn_snd bufsize get [chn]  --get channel snd bufsize \n");
    printf("chn_snd delay set [chn] [delay] --set channel snd start delay\n");
    printf("chn_snd delay get [chn]  --get channel snd start delay \n");
    printf("chn com reopen [chn] -- reopen channel comm\n");
    printf("audio format [chn] -- get channel audio format\n");
    printf("audio transmit [chn] [start|stop] -- start|stop audio stream\n");
    printf("at_port info [chn] -- get at port information\n");
    printf("debug_port info [chn] -- get debug port information\n");
    printf("upgrade_port info [chn] -- get upgrade port information\n");
    printf("conn state [chn] -- get channel connection state\n");
    printf("gsoap test [cnt] -- test gsoap capability\n");
    printf("at [chn] [command] --test at commond\n");
    printf("module version [chn]--get module version\n");
    printf("module track [chn] ---get module trk\n");
    printf("mcu cmd [chn]--mcu cmd test\n");
    printf("upgrade_chn [chn] [value] --set upgrade flag\n");
    printf("pipe_test [chn] [value] --write message to asterisk\n");
    printf("mcu reset [chn] -- reset mcu");
}

int server_version_get(int argc, char **argv)
{
//    char server_name[64] = {0};
    struct rri_server_version_t sv;

    memset(&sv, 0, sizeof(sv));
    //sv.nName = server_name;

    if ( GetServerVersion(&sv) == 0 )
    {
        printf("server name: %s\r\n", sv.nName);
        printf("server majorVersion: %d\r\n", sv.majorVersion);
        printf("server minorVersion: %d\r\n", sv.minorVersion);
        printf("server bugfixNumber: %d\r\n", sv.bugfixNumber);
        printf("server buildNumber: %d\r\n", sv.buildNumber);
    }
    else
    {
        printf("get server version fail!\r\n");
    }
    
    return 0;
}

int server_debug_set(int argc, char **argv)
{
    if ( argc != 3 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }
    int level = 0;
    if(strstr(argv[2], "error")){
	level |= 1 << 0;
   }
    if(strstr(argv[2], "warning")){
	level |= 1 << 1;
   }
    if(strstr(argv[2], "debug")){
	level |= 1 << 2;
   }
    if(strstr(argv[2], "info")){
	level |= 1 << 3;
   }
    if(strstr(argv[2], "notice")){
	level |= 1 << 4;
   }
    if ( SetServerDebug(level) == 0 )
        printf("server set debug ok\r\n, level = %d\n", level);
    else
        printf("server set debug fail\r\n");
    
    return 0;
}

int channel_number_get(int argc, char **argv)
{
    int nChannels = 0;

    if ( GetChannelCount(&nChannels) == 0 )
    {
        printf("GetChannelCount: %d\r\n", nChannels);
    }
    else
    {
        printf("get channel count fail!\r\n");
    }

    return 0;
}

int gsoap_capability_test(int argc, char **argv)
{
    int nChannels = 0;
    int cnt = 0;
    int i;
    int ms;
    struct timeval tstart, tend;

    if ( argc != 3 )
        return 0;

    cnt = str_to_int(argv[2]);
    gettimeofday(&tstart, NULL);

    for ( i = 0; i < cnt; i++ )
        if ( GetChannelCount(&nChannels) != 0 )
            break;

    gettimeofday(&tend, NULL);

    if ( i < cnt )
    {
        printf("test return error, abort!\r\n");
        return 0;
    }

    ms = ((long long)(tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec)) / 1000;

    printf("test complete. cnt = %d\r\n", cnt);
    printf("spend time: %d.%d s\r\n", ms/1000, ms % 1000);

    return 0;
}

int channel_info_get(int argc, char **argv)
{
    struct rri_channel_info_t info;

    if ( argc != 3 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }

    if ( GetChannelInfo(str_to_int(argv[2]), &info) == 0 )
    {
        printf("model_name: %s\r\n", info.rf_module_model_name);
        printf("hw_version: %s\r\n", info.rf_module_hw_version);
        printf("fw_version: %s\r\n", info.rf_module_fw_version);
        printf("manufacturer: %s\r\n", info.rf_module_manufacturer);
        printf("haveDebugPort: %d\r\n", info.haveDebugPort);
        printf("haveUpgradePort: %d\r\n", info.haveUpgradePort);
        printf("audioEndpointName_r: %s\r\n", info.audioEndpointName_r);
        printf("audioEndpointName_w: %s\r\n", info.audioEndpointName_w);
        printf("atEndpointName_r: %s\r\n", info.atEndpointName_r);
        printf("atEndpointName_w: %s\r\n", info.atEndpointName_w);
        printf("DebugEndpointName_r: %s\r\n", info.DebugEndpointName_r);
        printf("DebugEndpointName_w: %s\r\n", info.DebugEndpointName_w);
        printf("UpgradeEndpointName_r: %s\r\n", info.UpgradeEndpointName_r);
        printf("UpgradeEndpointName_w: %s\r\n", info.UpgradeEndpointName_w);
    }
    else
    {
        printf("get chn[%d] info fail!\r\n", str_to_int(argv[2]));
    }

    return 0;
}

int channel_debug_set(int argc, char **argv)
{
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }

    if ( SetChannelDebug(str_to_int(argv[2]), str_to_int(argv[3])) == 0 )
        printf("chn[%d] set at debug[%d] ok\r\n", str_to_int(argv[2]), str_to_int(argv[3]));
    else
        printf("chn[%d] set at debug[%d] fail\r\n", str_to_int(argv[2]), str_to_int(argv[3]));
    
    return 0;
}

int channel_debug_snd_set(int argc, char **argv)
{
    int result;
    if ( argc < 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }
    if(strcmp(argv[1], "debug") == 0){
        if ( SetChannelSndDebug(str_to_int(argv[2]), str_to_int(argv[3])) == 0 )
            printf("chn[%d] set snd debug[%d] ok\r\n", str_to_int(argv[2]), str_to_int(argv[3]));
        else
            printf("chn[%d] set snd debug[%d] fail\r\n", str_to_int(argv[2]), str_to_int(argv[3]));
    }else if(strcmp(argv[1], "speed") == 0){
        if(strcmp(argv[2], "set") == 0){
            if ( SetChannelTxSndSpeed(str_to_int(argv[3]), str_to_int(argv[4])) == 0 )
                printf("chn[%d] set snd speed[%d] ok\r\n", str_to_int(argv[3]), str_to_int(argv[4]));
            else
                printf("chn[%d] set snd spped[%d] fail\r\n", str_to_int(argv[3]), str_to_int(argv[4]));
        }else{
            if ( GetChannelTxSndSpeed(str_to_int(argv[3]), &result) == 0 )
                printf("chn[%d] get snd speed ok, speed is %dbytes/%dms\r\n", str_to_int(argv[3]), result << 3, result);
            else
                printf("chn[%d] get snd spped fail\r\n", str_to_int(argv[3]));
        } 
    }else if(strcmp(argv[1], "bufsize") == 0){
        if(strcmp(argv[2], "set") == 0){
            if ( SetChannelTxSndBufSize(str_to_int(argv[3]), str_to_int(argv[4])) == 0 )
                printf("chn[%d] set snd bufsize[%d] ok\r\n", str_to_int(argv[3]), str_to_int(argv[4]));
            else
                printf("chn[%d] set snd bufsize[%d] fail\r\n", str_to_int(argv[3]), str_to_int(argv[4]));
        }else{
            if ( GetChannelTxSndBufSize(str_to_int(argv[3]), &result) == 0 )
                printf("chn[%d] get snd bufsize ok, speed is %dbytes\r\n", str_to_int(argv[3]), result);
            else
                printf("chn[%d] get snd bufsize fail\r\n", str_to_int(argv[3]));
        } 
    }else if(strcmp(argv[1], "delay") == 0){
        if(strcmp(argv[2], "set") == 0){
            if ( SetChannelTxSndDelay(str_to_int(argv[3]), str_to_int(argv[4])) == 0 )
                printf("chn[%d] set snd tx delay[%d] ok\r\n", str_to_int(argv[3]), str_to_int(argv[4]));
            else
                printf("chn[%d] set snd tx delay[%d] fail\r\n", str_to_int(argv[3]), str_to_int(argv[4]));
        }else{
            if ( GetChannelTxSndDelay(str_to_int(argv[3]), &result) == 0 )
                printf("chn[%d] get snd tx delay ok, delay is %dms\r\n", str_to_int(argv[3]), result);
            else
                printf("chn[%d] get snd tx delay fail\r\n", str_to_int(argv[3]));
        } 
    }
    return 0;
}

int channel_com_reopen(int argc, char **argv)
{
    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }

    if ( ReopenSerial(str_to_int(argv[3])) == 0 )
        printf("channel serial reopen ok\r\n");
    else
        printf("channel serial reopen fail\r\n");
    
    return 0;
}

int audio_format_get(int argc, char **argv)
{
    struct rri_voice_attri_t va;
    
    if ( argc != 3 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }

    if ( GetChannelAudioFromat(str_to_int(argv[2]), &va) == RET_OK )
    {
        printf("sampleRate = %d\r\n", va.sampleRate);
        printf("samples_per_block = %d\r\n", va.samples_per_block);
    }
    else
    {
        printf("get audio format error\r\n");
    }
    
    return 0;
}

int audio_transmit_set(int argc, char **argv)
{
    int newState = 0;

    if ( argc != 4 )
    {
        printf("param error: argc = %d\r\n", argc);
        return 0;
    }

    if ( strcmp(argv[3], "start") == 0 )
        newState = 1;
    else if ( strcmp(argv[3], "stop") == 0 )
        newState = 0;
    else
        return 0;

    if ( SetAudioTransmitState(str_to_int(argv[2]), newState) == 0 )
        printf("audio[%s] transmit %s ok\r\n", argv[2], argv[3]);
    else
        printf("audio[%s] transmit %s fail\r\n", argv[2], argv[3]);
    
    return 0;
}

int at_port_info_get(int argc, char **argv)
{
	struct rri_at_attri_t attri;
	if(argc != 3){
		printf("param error!\n");
		return -1;
	}
	if(GetAtPortInfo(str_to_int(argv[2]), &attri) == 0){
		if(attri.result == 0){
			printf("get at port info ok:\n");
			printf("baudrate: %d\n", attri.baudrate);
			printf("xon_xoff: %d\n", attri.XON_XOFF);
		}else{
			printf("[%d] is not exist!\n", str_to_int(argv[2]));
			return -1;
		}
	}
	else{
		printf("get at port info failed!\n");
	}
    return 0;
}

int debug_port_info_get(int argc, char **argv)
{
	struct rri_at_attri_t attri;
	if(argc != 3){
		printf("param error!\n");
		return -1;
	}
	if(GetDebugPortInfo(str_to_int(argv[2]), &attri) == 0){
		if(attri.result == 0){
			printf("get debug port info ok:\n");
			printf("baudrate: %d\n", attri.baudrate);
			printf("xon_xoff: %d\n", attri.XON_XOFF);
		}else{
			printf("[%d] is not debug port!\n", str_to_int(argv[2]));
			return -1;
		}
	}else{
		printf("get at port info failed!\n");
	}
	return 0;
}

int upgrade_port_info_get(int argc, char **argv)
{
	struct rri_at_attri_t attri;
	if(argc != 3){
		printf("param error!\n");
		return -1;
	}
	if(GetUpgradePortInfo(str_to_int(argv[2]), &attri) == 0){
		if(attri.result == 0){
			printf("get upgrade port info ok:\n");
			printf("baudrate: %d\n", attri.baudrate);
			printf("xon_xoff: %d\n", attri.XON_XOFF);
		}else{
			printf("[%d] is not upgrade port!\n", str_to_int(argv[2]));
			return -1;
		}
	}else{
		printf("get at port info failed!\n");
	}
	return 0;
}

int connect_state_get(int argc, char **argv)
{
	if(argc != 3){
		printf("param error!\n");
		return -1;
	}
	struct rri_chn_conn_state_t con_state;
	if(GetChannelConnectionState(str_to_int(argv[2]), &con_state) == 0){
		if(con_state.result == 0){
			printf("get debug port info ok:\n");
			//audioStatus==0，表示非通话状态
			printf("chn[%d] is %scalling\n",str_to_int(argv[2]), con_state.audioStatus == 0 ? "not ":"");
			//atStatus==0，表示非短信发送状态
			printf("chn[%d] is %ssending short message\n", str_to_int(argv[2]),con_state.atStatus==0 ? "not ":"");
			//atStatus==0，表示非debug状态
			printf("chn[%d] is %sdebug channel\n",str_to_int(argv[2]), con_state.debugStatus==0 ? "not ":"");
			//upgradeStatus==0，表示非升级状态
			printf("chn[%d] is %supgrade channel\n",str_to_int(argv[2]), con_state.upgradeStatus==0 ? "not ":"");
		}else{
			printf("[%d] is not exist!\n", str_to_int(argv[2]));
			return -1;
		}
	}else{
		printf("get at port info failed!\n");
	}
    return 0;
}

int cli_at_commond_test(int argc, char **argv){
	if(argc < 3){
		printf("at [chn] [command]\n");
		return -1;
	}
	char read_pipe_name[1024] = {0};
	char write_pipe_name[1024] = {0};
//	int cnt;
	sprintf(write_pipe_name, "/tmp/module_pipe/at-%s-r.pipe", argv[1]);
	sprintf(read_pipe_name, "/tmp/module_pipe/at-%s-w.pipe", argv[1]);
	char buf[1024] = {0};
	//打开读管道
	int read_fd = open(read_pipe_name, O_RDONLY|O_NONBLOCK);
	if(read_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}
	
	//先读出里面已经存在的内容，丢掉
	read(read_fd, buf, 1024);
	
	memset(buf, 0, 1024);
	//打开写管道
	int write_fd = open(write_pipe_name, O_WRONLY);
	if(write_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}

	sprintf(buf, "%s\r\n", argv[2]);

	if(write(write_fd, buf, strlen(buf)) < 0){
		printf("write error\n");
		return -1;
	}
	close(write_fd);
	
	memset(buf, 0, 1024);

	//读取管道中返回的内容，尝试读取10次，每次间隔30ms
	int i = 160; 
	while(i > 0){
		if(read(read_fd, buf, sizeof(buf)) > 0){
			break;
		}
		usleep(30000);
		--i;
	}
	if(i > 0){
		printf("\n%s\n", buf);
		return 0;
	}else{
		printf("get result error\n");
		return -1;
	}
	close(read_fd);
	return 0;
}

int cli_module_version(int argc, char **argv){
	if(argc < 3){
		printf("module version [chn]\n");
              printf("module track [chn]\n");
              printf("module upgrade [chn]\n");
		return -1;
	}
	char read_pipe_name[1024] = {0};
	char write_pipe_name[1024] = {0};
//	int cnt;
	sprintf(write_pipe_name, "/tmp/module_pipe/mcu-%s-r.pipe", argv[2]);
	sprintf(read_pipe_name, "/tmp/module_pipe/mcu-%s-w.pipe", argv[2]);
	char buf[1024] = {0};
	//打开读管道
	int read_fd = open(read_pipe_name, O_RDONLY|O_NONBLOCK);
	if(read_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}
	
	//先读出里面已经存在的内容，丢掉
	read(read_fd, buf, 1024);
	
	memset(buf, 0, 1024);
	//打开写管道
	int write_fd = open(write_pipe_name, O_WRONLY);
	if(write_fd < 0){
		printf("open pipe error!\n");
		return -1;
	}
	if(strcmp(argv[1], "version") == 0){
		sprintf(buf, "ver\n");
	}else if(strcmp(argv[1], "track") == 0){ 
	      sprintf(buf, "trk\n");
	}else if(strcmp(argv[1], "upgrade")==0){
            sprintf(buf, "upgrade\n");
        }else{
		printf("input error!");
		close(read_fd);
		close(write_fd);
		return -1;
	}
	if(write(write_fd, buf, strlen(buf)) < 0){
		printf("write error\n");
		return -1;
	}
	close(write_fd);
	
	memset(buf, 0, 1024);

       int ret;
	//读取管道中返回的内容，尝试读取10次，每次间隔30ms
	int i = 10; 
	while(i > 0){
		if((ret = read(read_fd, buf, sizeof(buf))) > 0){
			break;
		}
		usleep(30000);
		--i;
	}
	if(i > 0){
              if(strcmp(argv[1], "track") == 0 ){
                   FIFO_REPORT *repot = (FIFO_REPORT *)buf;
                   printf("received %d bytes!\n", ret);
                   printf("\tusb_rx_full=%d\n"
                            "\tusb_to_spi_full=%d\n"
                            "\tusb_to_spi_empty=%d\n"
                            "\tspi_to_usb_full=%d\n"
                            "\tspi_to_usb_empty=%d\n"
                            "\tusb_to_uart_full=%d\n"
                            "\tuart_to_usb_full=%d\n"
			    "\tgarbage_pkt=%d\n"
			    "\tis_voice_start=%d\n"
			    "\tusb_rx_pkt=%d\n"
			    "\tvoice_up_pkt=%d\n"
			    "\tvoice_down_pkt=%d\n"
			    "\tat_up_pkt=%d\n"
			    "\tat_down_pkt=%d\n"
			    "\tspi_rx_cnt=%d\n"
			    "\tspi_tx_cnt=%d\n"
			    "\tuart_rx_cnt=%d\n"
			    "\tuart_tx_cnt=%d\n",
                            repot->usb_rx_full,
                            repot->usb_to_spi_full,
                            repot->usb_to_spi_empty,
                            repot->spi_to_usb_full,
                            repot->spi_to_usb_empty,
                            repot->usb_to_uart_full,
                            repot->uart_to_usb_full,
			    repot->garbage_pkt,
			    repot->is_voice_start,
			    repot->usb_rx_pkt,
			    repot->voice_up_pkt,
			    repot->voice_down_pkt,
			    repot->at_up_pkt,
			    repot->at_down_pkt,
			    repot->spi_rx_cnt,
			    repot->spi_tx_cnt,
			    repot->uart_rx_cnt,
			    repot->uart_tx_cnt);
              }else
                  printf("\n%s\n", buf);
		return 0;
	}else{
		printf("get result error\n");
		return -1;
	}
	close(read_fd);
	return 0;
}

int upgrade_chn_set(int argc, char **argv){
	if(argc != 3){
		printf("upgrade_chn [chn] [value]");
		return -1;
	}
	int nCh;
	int value ;
	nCh = str_to_int(argv[1]);
	value = str_to_int(argv[2]);

	if(SetChannelUpgrade(nCh, value) < 0)
	{
		printf("set channel Upgrade failed!\n");
		return -1;
	}
	else
	{	
		if(value == 1)
			printf("set channel Upgrade module success!\n");
		else
			printf("set channel Non-pgrade module success!\n");
	}
	return 0;
}
int cli_pipe_test(int argc, char **argv){
	int i = argc;
	char buf[1024] = {0};
	char filename[256]={0};
	int handle = 0;
	int result = 0;
	sprintf(filename, "/tmp/module_pipe/at-%s-w.pipe", argv[1]);

	if(argc < 3){
		printf("pipe_test channel message\n");
		return -1;
	}
	for(i = 2; i < argc; i++){
		strncat(buf, argv[i],1024);
		strncat(buf, " ", 1024);
	}
	strncat(buf, "\r\n", 1024);

	handle = open(filename, O_WRONLY);

	if(handle < 0){
		printf("open pipe error!\n");
		return -1;
	}

	result = write(handle, buf, strlen(buf));

	if(result < 0){
		printf("write to pipe failed!\n");
		close(handle);
		return -1;
	}
	close(handle);
    return 0;
}

int cli_mcu_test(int argc, char **argv)
{
	char filename[256] = {0};
	int handle = 0;
	int result = 0;
	char cmd[256];
	char gen_cmd[256] = {0};
	if(argc < 3){//mcu reset 1
		printf("param error\n");
		return -1;
	}
	sprintf(filename, "/tmp/module_pipe/mcu-%s-r.pipe", argv[2]);
	if(strcmp(argv[1], "reset") == 0){
		strcpy(cmd, "mcu_reset");
	}else{
		printf("mcu resuet [chn] fail\n");
		return -1;
	}

	handle = open(filename, O_WRONLY);

	if(handle < 0){
		printf("open pipe error!\n");
		return -1;
	}

	result = write(handle, cmd, strlen(cmd));

	if(result < 0){
		printf("write to pipe failed!\n");
		close(handle);
		return -1;
	}
	close(handle);
	sleep(4);
	sprintf(gen_cmd, "/my_tools/gen_mcu_device_link.sh %s", argv[2]);
	system(gen_cmd);
	if ( ReopenSerial(str_to_int(argv[2])) == 0 )
		printf("channel serial reopen ok\r\n");
	else
		printf("channel serial reopen fail\r\n");
}
/*
    注册命令行 -- bmcu 相关
*/
void cli_reg_rri(void)
{
    cb_func_reg("server ver", server_version_get);
    cb_func_reg("server debug", server_debug_set);
    cb_func_reg("chn_num get", channel_number_get);
    cb_func_reg("chn info", channel_info_get);
    cb_func_reg("chn_at debug", channel_debug_set);
    cb_func_reg("chn_snd debug", channel_debug_snd_set);
    cb_func_reg("chn_snd speed", channel_debug_snd_set);
    cb_func_reg("chn_snd speed set", channel_debug_snd_set);
    cb_func_reg("chn_snd speed get", channel_debug_snd_set);
    cb_func_reg("chn_snd bufsize", channel_debug_snd_set);
    cb_func_reg("chn_snd bufsize set", channel_debug_snd_set);
    cb_func_reg("chn_snd bufsize get", channel_debug_snd_set);
    cb_func_reg("chn_snd delay set", channel_debug_snd_set);
    cb_func_reg("chn_snd delay get", channel_debug_snd_set);
    cb_func_reg("chn com reopen", channel_com_reopen);
    cb_func_reg("audio format", audio_format_get);
    cb_func_reg("audio transmit", audio_transmit_set);
    cb_func_reg("at_port info", at_port_info_get);
    cb_func_reg("debug_port info", debug_port_info_get);
    cb_func_reg("upgrade_port info", upgrade_port_info_get);
    cb_func_reg("conn state", connect_state_get);
    cb_func_reg("gsoap test", gsoap_capability_test);
    cb_func_reg("at", cli_at_commond_test);
    cb_func_reg("upgrade_chn", upgrade_chn_set);
    cb_func_reg("module", cli_module_version);
    cb_func_reg("module version", cli_module_version);
    cb_func_reg("pipe_test", cli_pipe_test);
    cb_func_reg("mcu reset", cli_mcu_test);
}

int rri_test_one(int argc, char **argv){
	if(strcmp(argv[0], "server") == 0){
		if(strcmp(argv[1], "ver") == 0){
			server_version_get(argc, argv);
		}else if(strcmp(argv[1], "debug") == 0){
			server_debug_set(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "chn_num") == 0){
		channel_number_get(argc, argv);
	}else if(strcmp(argv[0], "chn") == 0){
		if(strcmp(argv[1], "info") == 0){
			channel_info_get(argc, argv);
		}else if(strcmp(argv[1], "com") == 0){
			channel_com_reopen(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "chn_at") == 0){
		if(strcmp(argv[1], "debug") == 0){
			channel_debug_set(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "chn_snd") == 0){
		if(strcmp(argv[1], "debug") == 0){
			channel_debug_snd_set(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "audio") == 0){
		if(strcmp(argv[1], "format") == 0){
			audio_format_get(argc, argv);
		}else if(strcmp(argv[1], "transmit") == 0){
			audio_transmit_set(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "at_port") == 0){
		if(strcmp(argv[1], "info") == 0){
			at_port_info_get(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "debug_port") == 0){
		if(strcmp(argv[1], "info") == 0){
			debug_port_info_get(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "at_port") == 0){
		if(strcmp(argv[1], "info") == 0){
			upgrade_port_info_get(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "conn") == 0){
		if(strcmp(argv[1], "state") == 0){
			connect_state_get(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "gsoap") == 0){
		if(strcmp(argv[1], "test") == 0){
			gsoap_capability_test(argc, argv);
		}else{
			cli_usage_rri();
		}
	}else if(strcmp(argv[0], "at") == 0){
		cli_at_commond_test(argc, argv);
	}else if(strcmp(argv[0], "module") == 0){
		cli_module_version(argc, argv);
      }else if(strcmp(argv[0], "upgrade_chn") == 0){
               upgrade_chn_set(argc, argv);
	}else if(strcmp(argv[0], "pipe_test") == 0){
		cli_pipe_test(argc, argv);	
	}else if(strcmp(argv[0], "mcu") == 0){
		cli_mcu_test(argc, argv);
	}else{
		cli_usage_rri();
	}
	return 0;
}

#endif

/*
    命令行使用方法说明
*/
int cli_usage_main(int argc, char **argv)
{
    printf("[?|help] -- show this menu\n");
    cli_usage_rri();
	return 0;
}

int main(int argc, char **argv)
{
    if ( rri_api_init(NULL, 0) != 0 )
    {
        printf("rri api init fail!\r\n");
        return -1;
    }
	if(argc == 1){
	    cli_init();
	    cb_func_reg("?", cli_usage_main);
	    cb_func_reg("help", cli_usage_main);
	    cli_reg_rri();

	    run_main((char *)"rri>");
	}else{
		rri_test_one(argc - 1, argv + 1);
	}

    rri_api_deinit();

    return 0;
}














