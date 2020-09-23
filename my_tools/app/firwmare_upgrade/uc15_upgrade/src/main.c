#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "openvox_status_api.h"
#include "uc15_upgrade.h"

#define TX_DATA_LEN (1024)
#define TX_BUF_LEN (2048)
#define ADDR_LEN (4)
#define CRC_LEN (2)
#define CMD_LEN (1)
//包含头部和尾部长度，为2
#define HEAD_LEN (2)

#define FRAME_HEAD 0x7E
//0x7D 和 0x7E为需要转换的字符
#define ESCAPE_1 0x7D
#define ESCAPE_2 0x7E
//0x7D,0x5D位 0x7D替换字符，0x7D,0x5E为0x7E替换字符
#define ESCAPE_3 0x5D
#define ESCAPE_4 0x5E 

#define TRY_COUNT (50)


//文件数量
#define FIRMWARE_FILE_NUM (6)
#define PORT_NAME_PREFIX "/dev/ttyUSB"

#include "load_nprg.h"
#include "quectel_crc.h"
#include "detect_port.h"

//#define DOWNLOAD_CMD 0x07
//#define DOWNLOAD_CMD_RSP 0x08

 typedef enum ERROR_CODE{
	SUCCESS=0,
	SYNC_ERR=-1,
	BEGIN_ERR=-2,
	LOAD_ERR=-3,
	DOWNLOAD_ERR=-4,
    END_ERR=-5, 
    RUN_ERR=-6, 
    INIT_ERR=-7,
    RESET_ERR=-8,
    PIPE_ERR=-9,
    COM_ERR=-10,
    MODULE_ERR=-11,
    FLASH_ERR=-12,
    UNKOWN =-20,
}ERROR_CODE_T;


enum MODULE_CMD{
	
	DOWNLOAD_CMD = 0x07,
	DOWNLOAD_RSP = 0x08,
	PATHION_HANDLE_CLOSE = 0x15,
	PATHION_HANDLE_CLOSE_RSP = 0x16,
	PATHION_HANDLE_OPEN = 0x1b,
	PATHION_HANDLE_OPEN_RSP = 0x1c,
};
	
enum PATION_FLAG{
	AMSS_FLAG = 0x05,
	DBL_FLAG = 0x0F,
	OSBL_FLAG = 0x10,
	FSBL_FLAG = 0x11,
};

enum MODULE_STATE{
	NPRG_MODE = 0x1D,
	UNKOWN_MODE,
};
/*
[0] NPRG6270.hex
[1] partition.mbn
[2] dbl.mbn
[3] fsbl.mbn
[4] osbl.mbn
[5] amss.mbn
*/
//struct firmware_file firmware[FIRMWARE_FILE_NUM];
struct firmware_file nprg;
struct firmware_file pation;
struct firmware_file dbl;
struct firmware_file fsbl;
struct firmware_file osbl;
struct firmware_file amss;

#define NPRG_FILE_NAME "NPRG6270.hex"
#define PATION_FILE_NAME "partition.mbn"
#define DBL_FILE_NAME    "dbl.mbn"
#define FSBL_FILE_NAME   "fsbl.mbn"
#define OSBL_FILE_NAME   "osbl.mbn"
#define AMSS_FILE_NAME   "amss.mbn"


int g_default_port = 0;
int g_hCom = 0;
short nprg_start_addr = 0;
/*
*功能：获取文件大小
*输入参数：filename --文件名
*返回值: =0 失败
*        >0 文件大小
*/

unsigned long get_file_size(const char* filename)
{
	unsigned long size;
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		printf("Open file %s failed.\n", filename);
		return 0;
	}
	fseek(fp, SEEK_SET, SEEK_END);
	size = ftell(fp);
	fclose(fp);
	return size;
}
/*
*功能：读取文件内容
*输入参数： file --firmware_file结构体指针
*返回值: =0 成功
*        <0 失败
*/

int get_file_content(struct firmware_file *file){
	if(file == NULL)
		return -1;
	file->size = get_file_size(file->filename);
	if(file->size == 0)
		return -1;
	file->data = (unsigned char *)malloc(file->size+1);
	
	FILE *handle = fopen(file->filename, "rb");
	if(handle == NULL){
		printf("open file failed!\n");
		return -1;
	}
	int real_size = fread(file->data, 1, file->size, handle);
	if(file->size != real_size){
		printf("read file content erorr, total %d bytes, actual read %d bytes\n", file->size, real_size);
	}
	printf("load %s success, length is %d\n", file->filename, file->size);
	fclose(handle);
	return 0;
}

/*
*功能：加载固件内容到缓存
*输入参数：path--固件路径
*返回值: =0 成功
*        <0 失败
*/

int uc15_firmware_init(char *path){
	if(path == NULL){
		return -1;
	}
	char tmp_path[FILE_NAME_LEN] =  {0};

	if(path[strlen(path) - 1] != '/'){
		snprintf(tmp_path,FILE_NAME_LEN, "%s%s", path, "/");
	}else{
		strcpy(tmp_path, path);
	}
	
	snprintf(nprg.filename, FILE_NAME_LEN, "%s%s", tmp_path, NPRG_FILE_NAME);
	if( load_nprg_hex(&nprg, &nprg_start_addr) < 0){
		return -1;
	}
	
	snprintf(pation.filename, FILE_NAME_LEN, "%s%s", tmp_path, PATION_FILE_NAME);
	if(get_file_content(&pation) < 0){
		return -1;
	}
	
	snprintf(dbl.filename, FILE_NAME_LEN, "%s%s", tmp_path, DBL_FILE_NAME);
	if(get_file_content(&dbl) < 0){
		return -1;
	}
	
	snprintf(fsbl.filename, FILE_NAME_LEN, "%s%s", tmp_path, FSBL_FILE_NAME);
	if(get_file_content(&fsbl) < 0){
		return -1;
	}
	
	snprintf(osbl.filename, FILE_NAME_LEN, "%s%s", tmp_path, OSBL_FILE_NAME);
	if(get_file_content(&osbl) < 0){
		return -1;
	}
	
	snprintf(amss.filename, FILE_NAME_LEN, "%s%s", tmp_path, AMSS_FILE_NAME);
	if(get_file_content(&amss) < 0){
		return -1;
	}
	
	//设置固件大小，便于显示进度条
	set_total_size(nprg.size + pation.size + dbl.size + osbl.size + fsbl.size + amss.size);
}
/*
*功能：释放所有存储固件的动态内存
*返回值: =0 成功
*        <0 失败
*/
void uc15_firmware_deinit(){
	if(nprg.data != NULL){
		free(nprg.data);
		nprg.data = NULL;
	}
	if(pation.data != NULL){
		free(pation.data);
		pation.data = NULL;
	}
	if(dbl.data != NULL){
		free(dbl.data);
		dbl.data = NULL;
	}
	if(fsbl.data != NULL){
		free(fsbl.data);
		fsbl.data = NULL;
	}
	if(osbl.data != NULL){
		free(osbl.data);
		osbl.data = NULL;
	}
	if(amss.data != NULL){
		free(amss.data);
		amss.data = NULL;
	}
}


/*
*功能：设置uart数据
*输入参数:fd--句柄
*返回值: =0 成功
*        <0 失败
*/

static int config_uart(int fd, int ioflush)
{

	struct termios tio;
    struct termios settings;
    int retval;
    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;
    cfsetospeed(&tio,B115200);            // 115200 baud
    cfsetispeed(&tio,B115200);            // 115200 baud
    tcsetattr(fd, TCSANOW, &tio);
	retval = tcgetattr (fd, &settings);
	if(-1 == retval)
	{
		return 1;
	}    
	cfmakeraw (&settings);
	settings.c_cflag |= CREAD | CLOCAL;
	if(ioflush)
	{
		tcflush(fd, TCIOFLUSH);
	}
	retval = tcsetattr (fd, TCSANOW, &settings);
	if(-1 == retval)
	{
		return 1;
	}
    return 0;
}
/*
*功能：打开端口
*返回值: =0 成功
*        <0 失败
*/

int openport(int ioflush)
{
    int tmp_port;
    tmp_port = g_default_port;
    int retry = 6;
    char pc_comport[32]; 
    
	//first close it if it opened
    if(g_hCom != 0)
    {
    	close(g_hCom);
    	g_hCom = 0;
    }
start_probe_port:
    memset(pc_comport,0,sizeof(pc_comport));
    sprintf(pc_comport, "%s%d", PORT_NAME_PREFIX, tmp_port);
    if(access(pc_comport, F_OK))
    {
        tmp_port++;
        retry--;
        if(retry > 0)
            goto start_probe_port;
        else
            return 1;
    }
    printf("Start to open com port: %s\n", pc_comport);
//g_hCom =  open(pc_comport, O_RDWR | O_NOCTTY);
    g_hCom = open (pc_comport, O_RDWR | O_SYNC|O_NONBLOCK);
    if(g_hCom < 0)
    {
        g_hCom = 0;
        return -1;
    }
    else
    {
    	config_uart((int)g_hCom, ioflush) ;
    }
    return 0;
}
/*
*功能：关闭端口
*返回值: =0 成功
*        <0 失败
*/

int closeport()
{
    close(g_hCom);
    g_hCom = 0;
    usleep(1000 * 100);
    return 0;
}
/*
*功能：用16进制数据打印数据
*输入参数：str--提示字符串
*          buf--打印的内容
*          size--打印的长度
*返回值: =0 成功
*        <0 失败
*/

void dump_data(char *str, unsigned char *buf, int size){
	printf("%s:",str);
	int i = 0; 
	while(i < size){
		printf("%02x ", buf[i]);
		i++;
		if(i %32 == 0)
			printf("\n");
	}
	printf("\n");
}

/*
*功能：写数据到串口
*输入参数:fd--串口句柄
*         buf--写入串口的数据
*         size--大小
*返回值: =0 成功
*        <0 失败
*/

int write_cmd(int fd, unsigned char *buf, int size){

	char tmp_buf[1024];
	//先读空buff数据
	while(read(fd, buf, 1024) > 0);
	return write(fd, buf, size);
}

/*
*功能：打开分区，为写分区数据做准备
*输入参数：pation_name--分区编号
*返回值: =0 成功
*        <0 失败
*/

int open_pation_handle(enum PATION_FLAG pation_name){

	unsigned char tx_buf[6] = {0};
	unsigned char result[6] = {0};
	char pation_name_str[32];
	sprintf(pation_name_str, "pation[%x]", pation_name);
	int tx_cmd_length = 6, rx_rsp_length = 6;
	tx_buf[0] = FRAME_HEAD;
	tx_buf[1] = PATHION_HANDLE_OPEN;
	tx_buf[2] = pation_name;
	short crc = crc_16_l_calc(&tx_buf[1], 2*8);
	tx_buf[3] = crc & 0xFF;
	tx_buf[4] = (crc >> 8)& 0xFF;
	tx_buf[5] = FRAME_HEAD;

	write_cmd(g_hCom,tx_buf, tx_cmd_length);

	if(get_cmd_respose(g_hCom, result, rx_rsp_length) < 0){
		dump_data(pation_name_str, result, rx_rsp_length);
		printf("get open pation[%x] handle rsp failed\n", pation_name);
		return -1;
	}else if(result[1] != PATHION_HANDLE_OPEN_RSP || result[2] != 0x00){
		printf("check open %s rsp error\n", pation_name_str);
		dump_data(pation_name_str, result, rx_rsp_length);
		return -1;
	}
	return 0;
};
/*
*功能：写完分区数据后，关闭分区
*输入参数：pation_name--分区编号
*返回值: =0 成功
*        <0 失败
*/
int close_pation_handle(enum PATION_FLAG pation_name){
	unsigned char tx_buf[5] = {0};
	unsigned char result[5] = {0};
	char pation_name_str[32];
	sprintf(pation_name_str, "pation[%x]", pation_name);
	int tx_cmd_length = 5, rx_rsp_length = 5;
	tx_buf[0] = FRAME_HEAD;
	tx_buf[1] = PATHION_HANDLE_CLOSE;
	short crc = crc_16_l_calc(&tx_buf[1], 1*8);
	tx_buf[2] = crc & 0xFF;
	tx_buf[3] = (crc >> 8)& 0xFF;
	tx_buf[4] = FRAME_HEAD;
	write_cmd(g_hCom,tx_buf, tx_cmd_length);

	if(get_cmd_respose(g_hCom, result, rx_rsp_length) < 0){
		dump_data(pation_name_str, result, rx_rsp_length);
		printf("get close %s handle rsp failed\n", pation_name_str);
		return -1;
	}else if(result[1] != PATHION_HANDLE_CLOSE_RSP ){
		printf("check close %s rsp error\n", pation_name_str);
		dump_data(pation_name_str, result, rx_rsp_length);
		return -1;
	}
	return 0;
}
/*
*功能：读取响应
*输入参数：fd--串口句柄
*          size-期望读取的大小
*输出参数：buf--返回读到的rsp数据
*返回值: =0 成功
*        <0 失败
*/
int get_cmd_respose(int fd, unsigned char *buf, int size){
	int count = 0;
	int read_size;
	int try_count = 10;
	while(count < size && try_count > 0){
		read_size = read(fd, &buf[count], size);
		if(read_size > 0){
			count += read_size;
			if(count > 1 && buf[count-1] == 0x7E)
				return 0;
		}
		try_count --;
		usleep(10*1000);
	}
	if(count > 0){
		return 0;
	}
	return -1;
}

/*
*功能：切换nprg 模式
*返回值: =0 成功
*        <0 失败
*/
int switch_nprg_mode(){

	unsigned char cmd_satus[5] = {0x7e, 0x0c, 0x14, 0x3a, 0x7e};

	unsigned char nprg_cmd[4] = {0x3a, 0xa1, 0x6e, 0x7e};
	
	unsigned char result[32] = {0};

	if(write_cmd(g_hCom, cmd_satus, 5) != 5){
		printf("write cmd failed!\n");
		return -1;
	}
	if(get_cmd_respose(g_hCom, result, 5) < 0){
		dump_data("status RX:",result, 5);
		printf("get response failed!\n");
		return -1;
	}else{
		if(result[0] == 0x0d){
			printf("module is nprg mode!\n");
		}else{
			printf("module is not nprg mode, switch to nprg mode...\n");
			write_cmd(g_hCom, nprg_cmd, 4);
			if(get_cmd_respose(g_hCom, result, 4) < 0){
				printf("switch to nprg mode failed!\n");
				return -1;
			}else if(result[0] == 0x3a){
				printf("switch to nprg mode success!\n");
				return 0;
			}else{
				dump_data("switch to nprg rsp:", result, 4);
				printf("switch to nprg mode failed!\n");
				return -1;
			}
		}
	}

	return 0;
}

/*描述：0x7D => 0x7D,0x5D  0x7E=>0x7D,0x5E 第一字节和最后一字节不做转换
*输入参数：src--输入缓冲区
*          src_len --输入缓冲区大小
*          drc--输出缓冲区
*          drc_len--输出缓冲区大小
*/
int format_conversion(unsigned char *src, int src_len, unsigned char *drc, int drc_len){
    if(src == NULL || drc == NULL ){
        return 0;   
    }   
    int i = 1, new_len = 1;
	//最后一个0x7e不转换
	int end = src_len - 1;
	//跳过第一个0x7e;
	*drc++ = *src++;
    while(i < end){
        if(*src == 0x7D){
            *drc++ = *src++;
            *drc++ = 0x5D;
            new_len+=2;
        }else if(*src == 0x7e){
			*drc++ = 0x7D;
			*drc++ = 0x5E;
			src++;
            new_len+=2;
		}else{
            *drc++ = *src++;
            new_len+=1;
        }   
        i++;
    }
	//拷贝最后一个0x7e
	*drc++ = *src++;
	
    return new_len+1;
}

/*0x7D,0x5D=>0x7D, 0x7D,0x5E=>0x7E*/
int format_addr(unsigned char *result, int len){
	if(len < 9){
		return -1;
	}
	int i = 2, j = 4;
	int recive = 0;
	for(j = 0; j < 4 && i < len; j++){
		if(result[i] == 0x7D){
			if(result[i+1] == 0x5D)
				recive |= 0x7D << (j*8);
			else
				recive |= 0x7E << (j*8);
			i = i + 2;
		}else{
			recive |= result[i] << (j*8);
			i = i+1;
		}
	}
//	printf("recive = %x\n", recive);
	return recive;
}

//#define CRC_DEBUG
int escape_conversion(unsigned char *src, int src_len, unsigned char *drc){
	if(src == NULL || drc == NULL ){
        return 0;   
    }   
    int i = 1, new_len = 1;
	//最后一个7e不转换
	int end = src_len - 1;
	//跳过第一个7e;
	*drc++ = *src++;
    while(i < end){
        if(*src == 0x7D){
            *drc++ = *src++;
            *drc++ = 0x5D;
            new_len+=2;
        }else if(*src == 0x7e){
			*drc++ = 0x7D;
			*drc++ = 0x5E;
			src++;
            new_len+=2;
		}else{
            *drc++ = *src++;
            new_len+=1;
        }   
        i++;
    }
	//拷贝最后一个0x7e
	*drc++ = *src++;
	
    return new_len+1;
}
/*
*功能：发送数据到模块
*返回值: =0 成功
*        <0 失败
*/

int download_data_to_module(struct firmware_file *file){
	//0x7e,0x07,addr,data,crc,0x7e 
	char tx_buf_old[HEAD_LEN + CMD_LEN+ADDR_LEN+TX_DATA_LEN+CRC_LEN] = {0};
	//发送buf，设置为
	char tx_buf_new[TX_BUF_LEN] = {0};
	
	//已发送数据的大小
	int count = 0;
	
	//每次发送的净数据长度
	int realsize = TX_DATA_LEN;
	char result[32] = {0};
	
	//失败尝试次数
	int try_count;
	
	//最终写入串口的数据大小
	int new_len;
	
	//文件内容指针
	char *data = file->data;
	
	//需要发送的数据长度
	int data_length = file->size;
	
	//包头和cmd固定位置，固定值
	tx_buf_old[0] = FRAME_HEAD;
	tx_buf_old[1] = DOWNLOAD_CMD;
	while(count < data_length){
		realsize = realsize < data_length - count ? realsize:(data_length-count);
		//设置已发送的数据大小
		tx_buf_old[2] = count &0xFF;
		tx_buf_old[3] = (count >> 8)&0xFF;
		tx_buf_old[4] = (count >> 16)&0xFF;
		tx_buf_old[5] = (count >> 24)&0xFF;
		unsigned short crc = 0;		
		memcpy(&tx_buf_old[6], data + count, realsize );
		crc = crc_16_l_calc(&tx_buf_old[1], (realsize+5) * 8);
		//设置crc校验值
		tx_buf_old[6+realsize] = crc & 0xFF;
		tx_buf_old[6+realsize+1] = (crc >> 8) & 0xFF;
		//设置包尾
		tx_buf_old[6+realsize+2] = FRAME_HEAD;
		
		new_len = format_conversion(tx_buf_old, realsize + 9, tx_buf_new, TX_BUF_LEN);
		
		try_count = TRY_COUNT;
write_again:
		write_cmd(g_hCom, tx_buf_new, new_len);

		if(get_cmd_respose(g_hCom, result, 11) < 0){
			//没有响应，直接退出
			return -1;
		}else if(result[1]!= DOWNLOAD_RSP){
			dump_data("check respons err,try again...", result, 11);
			if(try_count > 0){
				try_count--;
				goto write_again;
			}
		}else{
			int receive = 0;
			receive = format_addr(result, 11);
			if(count != receive && try_count > 0){
				dump_data("check respons addr err,try again...", result, 11);
				try_count--;
				goto write_again;
			}
		}
		if(try_count < 0){
			return -1;
		}
	
		count = count+realsize;
		//更新进度条
		set_write_size(realsize);
		process_par(NULL);
	}
	return 0;
}

/*
*功能：发送NPRG6270.hex到模块
*返回值: =0 成功
*        <0 失败
*/

int transmite_nprg_data(){
	int start_addr;
	char diag_port_buf[FILE_NAME_LEN] = {0};
	char *diag_port = diag_port_buf;
	
	short addr = nprg_start_addr;
	
	start_addr = (addr & 0xFFFF) << 16 ;
	//发送开始信号 7e,07,c7,84,73
	unsigned char begin_cmd[5] = {0x7e,0x07,0xc7,0x84,0x7e};
	closeport();
	sleep(5);
	
	//检测升级串口
	int res = detect_diag_port(&diag_port); 
	if(res != 0){
		set_upgrade_state(COM_ERR);
		process_par("err:com error");
		printf("no detect dialog port!\n");
		return -1;
	}else{
		g_default_port = atoi(diag_port+6);
	}
	
	openport(0);
	int try_count = 10;
	int count = 0;
	unsigned char result[32] = {0};
	unsigned char tx_buf_old[1028] = {0};
	unsigned char tx_buf_new[2048] = {0};
	while(try_count--){
		write_cmd(g_hCom, begin_cmd, 5);

		if(get_cmd_respose(g_hCom, result, 11) < 0){
			printf("get response failed!\n");
		}else{
			if(result[0] == 0x08 && result[1] == 0x06){
				dump_data("nprg cmd", result, 11);	
				break;
			}
		}
		if(try_count == 0){
			printf("get nprg response failed!\n");
			return -1;
		}
	}
	unsigned char *data = nprg.data;
	int size = nprg.size;
	
	//real_size = 0x03f9
	int real_size = 0x03f9;
//	int try_count;
	if(result[0] == 0x08 && result[1] == 0x06){
		while(count < size){
			real_size = (real_size < size - count ? real_size : size-count);
			tx_buf_old[0] = 0x7e;
			tx_buf_old[1] = 0x0f;
			//bit2-bit5,已发送数据长度
			tx_buf_old[2] = (start_addr >> 24) & 0xFF;
			tx_buf_old[3] = (start_addr >> 16) & 0xFF;
			tx_buf_old[4] = (start_addr >> 8) & 0xFF;
			tx_buf_old[5] = start_addr & 0xFF;
			//bit6-bit7,当前净数据长度
			tx_buf_old[6] = (real_size >> 8) & 0xFF;
			tx_buf_old[7] = (real_size) & 0xFF;
			memcpy(&tx_buf_old[8], data+count, real_size);
			short crc = crc_16_l_calc(&tx_buf_old[1], (real_size+7) * 8);
			tx_buf_old[8+real_size] = crc&0xFF;
			tx_buf_old[8+real_size+1] = (crc >> 8)&0xFF;
			tx_buf_old[8+real_size+2] = 0x7e;
			count += real_size;
			start_addr = start_addr+real_size;
			int new_len = format_conversion(tx_buf_old, real_size+11, tx_buf_new, 2048);
			try_count = TRY_COUNT;
write_again:
			write_cmd(g_hCom, tx_buf_new, new_len);
			if(get_cmd_respose(g_hCom, result, 32) < 0){
				printf("get response failed!\n");
				return -1;
			}
			if(result[1] != 0x02){
				dump_data("NPRG RX", result, 32);
				if(try_count > 0){
					try_count--;
					goto write_again;
				}
			}
			if(try_count <= 0){
				return -1;
			}
			set_write_size(real_size);
			process_par(NULL);
		}
	}else{
		return -1;
	}
	printf("transmit nprg data finished!\n");
	return 0;
}
/*
*功能：切换到fastdownlod 模式
*返回值: =0 成功
*        <0 失败
*/

int transmite_protol_host(){
	char tmp_buf[64] = {0};
	char *dialg_port = tmp_buf;
	unsigned char result[1024] = {0};

	unsigned char cmd[9] = {0x7e, 0x05,0x00, 0x10,0x00, 0x00,0xb6, 0x6c, 0x7e};
	write_cmd(g_hCom, cmd, 9);
	if(get_cmd_respose(g_hCom, result, 6) < 0){
		printf("set fast download mode failed!\n");
		return -1;
	}
	if(result[1] != 0x02){
		dump_data("host", result, 6);
		printf("protol_host set cmd failed!\n");
		return -1;
	}
	closeport();
	sleep(5);
	if(detect_diag_port(&dialg_port)!= 0){
		printf("dectect dialg_port failed!\n");
		return -1;
	}else{
		printf("dialg_port name is %s\n", dialg_port);
		g_default_port = atoi(dialg_port+6);
	}
	openport(0);

	//"QCOM fast download protol host"
	unsigned char buf[40] = {0x7e,0x01,0x51,0x43,0x4f,0x4d,0x20,0x66,
							  0x61,0x73,0x74,0x20,0x64,0x6f,0x77,0x6e,
							  0x6c,0x6f,0x61,0x64,0x20,0x70,0x72,0x6f,
							  0x74,0x6f,0x63,0x6f,0x6c,0x20,0x68,0x6f,
							  0x73,0x74,0x02,0x02,0x01,0x8a,0xfd,0x7e};

	write_cmd(g_hCom, buf, 40);


	if(get_cmd_respose(g_hCom, result, 576) < 0){
		printf("set fast download mode failed!\n");
		return -1;
	}
	if(strncmp(&result[2], "QCOM fast download protocol targ", strlen("QCOM fast download protocol targ")) == 0){
		printf("set fast download mode success!\n");
		return 0;
	}
	printf("set fast download mode failed!\n");
	return -1;
}
/*
*功能：发送pation.mbn到模块
*返回值: =0 成功
*        <0 失败
*/

int transmite_partion_table(){
	
	unsigned char result[64] = {0};
	unsigned char *content = pation.data;
	int size = pation.size;
	unsigned char tx_buf_old[1024] = {0};
	unsigned char tx_buf_new[2048] = {0};

	//关闭其他分区
	unsigned char cmd1[5] = {0x7e, 0x15,0x54, 0xb7, 0x7e};
	
	unsigned char cmd2[6] = {0x7e, 0x17,0x00, 0xde, 0xd7, 0x7e};

	unsigned char cmd3[6] = {0x7e, 0x60, 0x01, 0x9b, 0x7b, 0x7e};

	write_cmd(g_hCom, cmd1, 5);

	if(get_cmd_respose(g_hCom, result, 32) < 0){
			
		printf("get respos failed!\n");
	}
		
	dump_data("pation table RX", result, 32);

	write_cmd(g_hCom, cmd2, 6);

	if(get_cmd_respose(g_hCom, result, 64) < 0){
		
		printf("get respos failed!\n");
	}
	
	dump_data("pation table RX", result, 64);
	if(result[1] != 0x18 ){
		return -1;
	}

	write_cmd(g_hCom, cmd3, 6);
	if(get_cmd_respose(g_hCom, result, 6) < 0){
		
		printf("get respos failed!\n");
	}
	dump_data("pation table RX", result, 6);

	
	if(result[1] != 0x61 && result[2] != 0x00){
		return -1;
	}
	int count = 0;
	int realsize = 1024 - 6;
	while(count < size){
		realsize = realsize < size - count ? realsize:(size-count);
		tx_buf_old[0] = 0x7e;
		tx_buf_old[1] = 0x19;
		tx_buf_old[2] = 0x00;
		short crc = 0;		
		memcpy(&tx_buf_old[3], content + count, realsize );
		crc = crc_16_l_calc(&tx_buf_old[1], (realsize+2) * 8);
		tx_buf_old[3+realsize] = crc & 0xFF;
		tx_buf_old[3+realsize+1] = (crc >> 8) & 0xFF;
		tx_buf_old[3+realsize+2] = 0x7e;
		int new_len = format_conversion(tx_buf_old, realsize + 6, tx_buf_new, TX_DATA_LEN << 1);
		int try_count = TRY_COUNT;
		
write_again:
		write_cmd(g_hCom, tx_buf_new, new_len);

		if(get_cmd_respose(g_hCom, result, 6) < 0){
			dump_data("get pation table rsp failed!\n", result, 6);
			return -1;
		}else if(result[1]!= 0x1a || result[2] != 0x00){
			dump_data("pation table response", result, 6);
			if(try_count > 0){
				try_count--;
				goto write_again;
			}
		}
		if(try_count == 0)
			return -1;
		count = count+realsize;
		set_write_size(realsize);
		process_par(NULL);
	}
	printf("transmit pation table finished!\n");
	return 0;
}


/*
*功能：发送dbl.mbn到模块
*返回值: =0 成功
*        <0 失败
*/

int transmite_dbl(){
	printf("write dbl.msbn to module...\n");
	if(open_pation_handle(DBL_FLAG) < 0)
		return -1;
	if(download_data_to_module(&dbl) < 0){
		printf("write dbl.msbn to module failed!\n");
		return -1;
	}
	if(close_pation_handle(DBL_FLAG) < 0){
		return -1;
	}
	printf("write dbl.msbn finished!\n");
	return 0;
}
/*
*功能：发送fsbl.mbn到模块
*返回值: =0 成功
*        <0 失败
*/

int transmite_fsbl(){
	printf("write fsbl.msbn to module...\n");
	if(open_pation_handle(FSBL_FLAG) < 0)
		return -1;
	if(download_data_to_module(&fsbl) < 0){
		printf("write dbl.msbn to module failed!\n");
		return -1;
	}
	if(close_pation_handle(FSBL_FLAG)<0)
		return -1;
	printf("write fsbl.msbn finished!\n");
	return 0;
}

/*
*功能：发送osbl.mbn到模块
*返回值: =0 成功
*        <0 失败
*/

int transmite_osbl(){
	printf("write osbl.msbn to module...\n");
	if(open_pation_handle(OSBL_FLAG) < 0)
		return -1;
	if(download_data_to_module(&osbl) < 0){
		printf("write osbl.msbn to module failed!\n");
		return -1;
	}
	if(close_pation_handle(OSBL_FLAG) < 0)
		return -1;
	printf("write osbl.msbn finished!\n");
	return 0;
}

/*
*功能：发送amss.mbn到模块
*返回值: =0 成功
*        <0 失败
*/

int transmite_amss(){
	printf("write amss to module...\n");
	if(open_pation_handle(AMSS_FLAG) < 0)
		return -1;
	if(download_data_to_module(&amss) < 0){
		printf("write amss.msbn to module failed!\n");
		return -1;
	}
	if(close_pation_handle(AMSS_FLAG) < 0)
		return -1;
	printf("write amss.msbn finished!\n");
	return 0;
}
/*
*功能：模块重启
*返回值: =0 成功
*        <0 失败
*/
int transmit_reboot(){
	unsigned char result[32] = {0};
	unsigned char tx_buf_old[1024] = {0};
	unsigned char tx_buf_new[2048] = {0};

	unsigned char cmd1[6] = {0x7e, 0x60,0x00, 0x12, 0x6a, 0x7e};
	unsigned char cmd2[5] = {0x7e, 0x0b,0xab, 0x4e, 0x7e};


	write_cmd(g_hCom, cmd1, 6);
	if(get_cmd_respose(g_hCom, result, 6) < 0){
		printf("get reboot cmd1 response failed!\n");
		return -1;
	}else if(result[1] != 0x61 || result[2] != 0x00){
		dump_data("reboot cmd1", result, 6);
		return -1;
	}

	write_cmd(g_hCom, cmd2, 5);
	if(get_cmd_respose(g_hCom, result, 5) < 0){
		printf("get reboot cmd2 response failed!\n");
		return -1;
	}else if(result[1] != 0x0c){
		dump_data("reboot cmd2", result, 6);
		return -1;
	}
	
	printf("reboot module success!\n");
	return 0;
}
/*
*功能：模块升级接口
*输入参数：firmware_path--固件路径
*返回值: =0 成功
*        <0 失败
*/

int upgrade_process( int port){
	char diag_port_buf[FILE_NAME_LEN] = {0};
	char *diag_port = diag_port_buf;
	//检测升级串口
	int res = detect_diag_port(&diag_port); 
	if(res != 0){
		set_upgrade_state(COM_ERR);
		process_par("err:com error");
		printf("no detect dialog port!\n");
		return -1;
	}else{
		g_default_port = atoi(diag_port+6);
	}
	//打开升级串口
	if(openport(0) < 0){
		set_upgrade_state(COM_ERR);
		process_par("err:com error");
		printf("open port failed!\n");
		return -1;
	}
	
	
	//切换到nprg模式
	if(switch_nprg_mode() < 0){
		process_par("err:sync error");
		set_upgrade_state(SYNC_ERR);
		return -1;
	}

	//发送nprg数据
	if(transmite_nprg_data() < 0){
		set_upgrade_state(SYNC_ERR);
		process_par("err:sync error");
		printf("transmit nprg data failed!\n");
		return -1;
	}
	//切换到fast download模式
	if(transmite_protol_host() < 0){
		set_upgrade_state(SYNC_ERR);
		process_par("err:sync error");
		printf("upgrade failed!\n");
		return -1;
	}
	//发送partition.mbn
	if(transmite_partion_table() < 0){
		set_upgrade_state(DOWNLOAD_ERR);
		process_par("err:download error");
		printf("upgrade failed!\n");
		return -1;
	}
	//发送dbl.mbn
	if(transmite_dbl()<0){
		set_upgrade_state(DOWNLOAD_ERR);
		process_par("err:download error");
		printf("upgrade failed!\n");
		return -1;
	}
	//发送fsbl.mbn
	if(transmite_fsbl() < 0){
		set_upgrade_state(DOWNLOAD_ERR);
		process_par("err:download error");
		printf("upgrade failed!\n");
		return -1;
	}
	//发送osbl.mbn
	if(transmite_osbl() < 0){
		set_upgrade_state(DOWNLOAD_ERR);
		process_par("err:download error");
		printf("upgrade failed!\n");
		return -1;
	}
	//发送amss.mbn
	if(transmite_amss() < 0){
		set_upgrade_state(DOWNLOAD_ERR);
		process_par("err:download error");
		printf("upgrade failed!\n");
		return -1;
	}

	//模块重启
	if(transmit_reboot()){
		set_upgrade_state(RESET_ERR);
		process_par("err:reset error");
		printf("upgrade failed!\n");
		return -1;
	}

	printf("upgrade success!\n");
	process_par("upgrade success");
}
int main(int argc, char **argv){
	int opt = 0;
	char firmware_path[FILE_NAME_LEN];
	int port = 0;
	while( (opt = getopt(argc, argv, "f:c:h")) > 0){
		switch (opt){
			case 'f':
				if(access(optarg, F_OK) == 0)
					strcpy(firmware_path, optarg);
				else
					strcpy(firmware_path, "./");
				break;
			case 'c':
				port = atoi(optarg);
				break;
			case 'h':
				printf("usage:%s [-f dir] [-c channel]\n", argv[0]);
				break;
		}
	}

	
	//设置升级端口
	set_channel(port);
	//读取单片机版本号
	set_module_version(port, NULL);
	//读取模块版本号
	set_old_version();
	//设置开始时间
	set_start_time();

	//加载固件
	if(uc15_firmware_init(firmware_path) < 0){
		set_upgrade_state(LOAD_ERR);
		process_par("err:load firmware error");
		uc15_firmware_deinit();
		return -1;
	}
	
	upgrade_process( port );

	//释放固件占用的内存空间
	uc15_firmware_deinit();
	
	set_new_version();
	
	set_end_time();
	
	record_info_to_file();
	
	return 0;
}
