/**********************************************************************************
文件描述  : 实现模块板(module board)MCU通信接口。
            包括数据收发、交互命令实现，交互命令包括：
            1) 寄存器读写
            2) 获取帮助信息
            3) 获取版本信息
            4) 获取uid
作者/时间 : junyu.yang@openvox.cn/2018.01.25
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "mod_brd_hw_intf.h"
#include "bsp_tools.h"
#include "mod_brd_regs.h"
/* 定义通信命令 */
#define CMD_REG_READ      "read %d\n"         /*  读单个寄存器 read <reg>h\n */ 
#define CMD_REG_READ_MUL  "read %d-%02xh\n"   /*  读多个寄存器 read <reg>-<width>h\n */ 
#define CMD_REG_READ_BIT  "read %d.%d\n"      /*  读寄存器位  read <reg>.<bit>\n */ 
#define CMD_REG_WRITE     "write %d=%02xh\n"  /*  写寄存器    write <reg>=<val>h\n  */
#define CMD_REG_WRITE_BIT "write %d.%d=%d\n"  /*  写寄存器位  write <reg>.<bit>=<val>\n */
#define CMD_HELP    "help\n"    /*  获取命令帮助信息 */
#define CMD_VER     "ver\n"     /*  获取版本信息（设备名称，软件、硬件版本号）\n */
#define CMD_UID     "uid\n"     /*  获取uid信息\n */

#define CMD_STR_MAX_LEN         24  /* 命令字符串最大长度, 单位 byte */
#define CMD_RET_STR_MAX_LEN     512  /* 执行命令后，命令返回的字符串最大长度, 单位 byte */

#define HWIF_IO_SYNC_TIMEOUT    2000 /* 硬件接口 IO 同步操作超时，单位 ms */

/* 连续发2条或以上条写命名，第1条之后的命令可能会被忽略
 * 所以需要等命令执行完成 */
#define CMD_EXEC_TIME           80  /* 命令执行时间，单位 ms */
#define REG_R_EXEC_TIME         15  /* 读寄存器执行时间，单位 ms */
#define REG_W_EXEC_TIME         15  /* 写寄存器执行时间，单位 ms */

/* 判断两字符串是否相等 */
#define STR_MATCH(str1, str2)   (0 == strncmp(str1, str2, strlen(str2)))

/* Log  输出 */
#define HWIF_PRT   BSP_PRT                          

struct hwif_io_pkt 
{
    char *data;         /* 指针，指向io操作数据缓冲区 */
    int  len;           /* data 指向数据长度，单位 byte. */
    int  actual_len;    /* 实际io操作数据长度，单位 byte */
    int  timeout;       /* io操作超时, 单位 ms */
};

/*************************************************
  函数描述 : 硬件接口同步发送数据
  输入参数 : hwif -- 硬件接口句柄
             data -- 指针，指向要发送的数据
             len -- 要发送的数据长度
             timeout -- 超时，单位ms, timeout ms 内未发送则返回;
                        =0, 不等待。
  输出参数 : actual_len -- 实际发送长度，单位:byte.
  函数返回 :  0 -- 发送成功
              1 -- 发送超时
             -1 -- 发送失败
  备注：timeout 目前不起作用
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
static inline int hwif_sync_tx(struct hw_intf *hwif, char *data, int len, int *actual_len, int timeout)
{
    int res;

    res = write(hwif->fd, data, len);
    if(res < 0)
    {
        return -1;
    }
    else {
        *actual_len = res;
        return 0;
    }
    /* 目前没有超时，此功能预留*/
}

/*************************************************
  函数描述 : 硬件接口同步接收数据
  输入参数 : hwif -- 硬件接口句柄
             data -- 指针，指向接收数据缓冲区
             len -- 要发送的数据长度
             timeout -- 超时，单位ms, timeout ms 内未接收则返回,
                        =0, 不等待。
  输出参数 : actual_len -- 实际接收长度，单位:byte.
  函数返回 :  0 -- 接收成功
              1 -- 接收超时
             -1 -- 接收失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
static inline int hwif_sync_rx(struct hw_intf *hwif, char *data, int len, int *actual_len, int timeout)
{
    int res;
    fd_set fds;
    struct timeval to;

    if(0 == timeout)
    {
        res = read(hwif->fd, data, len);
        if(res < 0)
        {
            /* 有时需要清空，所以输出级别用VERB而不是ERR.*/
            HWIF_PRT(VERB,"hwif %s read failed: %s\n", hwif->name, strerror(errno));
            return -1;
        }
        else {
            *actual_len = res;
            return 0;
        }
    }
    else if(timeout > 0)
    {
        to.tv_sec  = timeout / 1000;
        to.tv_usec = (timeout % 1000) * 1000;

        FD_ZERO(&fds);
        FD_SET(hwif->fd, &fds);

        res = select(hwif->fd + 1, &fds, NULL, NULL, &to);
        if ( res < 0 )  /* select on error */
        {
            HWIF_PRT(ERR,"hwif %s select failed: %s\n", hwif->name, strerror(errno));
            return -2;
        }
        else if(res > 0)    /* data avarible */
        {
            res = read(hwif->fd, data, len);
            if(res < 0)
            {
                return -3;
            }
            else
            {
                *actual_len = res;
                return 0;
            }
        }
        else  /* No data within 'timeout' ms */ 
        {
            return 1;
        }
    }

    return 0;
}

/*************************************************
  函数描述 : 硬件接口同步发送、接收数据
  输入参数 : hwif -- 硬件接口句柄
             tx_pkt -- 指针，指向要发送的数据包
             rx_pkt -- 指针，指向要接收的数据包
             prcs_time_ms --  无端处理包所需时间，单位ms
  输出参数 : 无
  函数返回 :  0 -- 发送、接收成功
              1 -- 需要发送长度与实际发送长度不等
             -1 -- 发送超时
             -2 -- 接收失败
  备注：不检测需要接收长度与实际接收长度是否不等
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
static int hwif_sync_io_op(struct hw_intf *hwif, struct hwif_io_pkt *tx_pkt, struct hwif_io_pkt *rx_pkt, int prcs_time_ms)
{
    int res = 0;
    int actual_len;
    char tmp_buf[256] = {0};
    
    /* 清楚缓冲区内容 */
    while(hwif_sync_rx(hwif, tmp_buf, sizeof(tmp_buf), &actual_len, 0) > 0);

    res = hwif_sync_tx(hwif, tx_pkt->data, tx_pkt->len, &tx_pkt->actual_len, tx_pkt->timeout);
    if(res < 0)
    {
        HWIF_PRT(ERR, "hwif %s tx faile, res = %d\n", hwif->name, res);
        return -1;
    }
    if(tx_pkt->len != tx_pkt->actual_len)
    {
        HWIF_PRT(WARN, "hwif %s need to send %d bytes, but actually send %d bytes\n",
                hwif->name, tx_pkt->len, tx_pkt->actual_len);
        return 1;
    }

    /* 等处理完再接收 */
    usleep(prcs_time_ms * 1000);

    res = hwif_sync_rx(hwif, rx_pkt->data, rx_pkt->len, &rx_pkt->actual_len, rx_pkt->timeout);
    if(res != 0)
    {
        HWIF_PRT(ERR, "hwif %s rx faile, res = %d\n", hwif->name, res);
        return -2;
    }

    /*
     *if(rx_pkt->len != rx_pkt->actual_len)
     *{
     *    HWIF_PRT(VERB, "hwif %s need to recv %d bytes, but actually recv %d bytes\n",
     *            hwif->name, rx_pkt->len, rx_pkt->actual_len);
     *    return 2;
     *}
     */

    return 0;
}


/*************************************************
  函数描述 : 读1个或多个连续的寄存器
  输入参数 : hwif -- 硬件接口句柄
             reg -- 起始寄存器号
             num -- 寄存器数量, 大于等于1.
  输出参数 : vals -- 指针，用于存放寄存器值，空间不能少于'num'。
  函数返回 : 0 -- 发送成功
             1 -- 发送超时
             -1 -- 发送失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_read_mul(struct hw_intf *hwif, int reg, int num, int *vals) 
{
    int i, res = 0;
    struct hwif_io_pkt tx_pkt, rx_pkt;
    char cmd_str[CMD_STR_MAX_LEN] = {0};
    char cmd_ret_str[CMD_RET_STR_MAX_LEN] = {0};

    tx_pkt.len = snprintf(cmd_str, sizeof(cmd_str), CMD_REG_READ_MUL, reg, num); 
    tx_pkt.data = cmd_str;
    tx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    rx_pkt.data = cmd_ret_str;
    rx_pkt.len = CMD_RET_STR_MAX_LEN;
    rx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    res = hwif_sync_io_op(hwif, &tx_pkt, &rx_pkt, REG_R_EXEC_TIME);
    if(0 != res) 
    {
        HWIF_PRT(ERR, "hwif %s io operation failed, res = %d\n", hwif->name, res);
        return -1;
    }

    /* 字符串格式: "FF,FF,00,00,00,00"，3个字符1个值 */ 
    for (i = 0; i < num; i++)
    {
        vals[i] = (char_to_val(rx_pkt.data[i*3]) << 4) + char_to_val(rx_pkt.data[i*3+1]);
    }

    return 0;
}

/*
 * Comments by junyu.yang@openvox  2018.01.25
 *以下两个接口属于冗余接口， 但单片机支持，因此只实现，不使用
 */
#if 0 
/*
 *CMD_REG_READ 与 CMD_REG_READ_MUL  两命令效率差不多，而且
 *hwif_reg_read_mul(hwif, reg, 1, val)
 *即为只读一个寄存器值，因此不提供功单独读一个寄存器接口
 *(设计上不提供功能重复接口)
 */
int hwif_reg_read(struct hw_intf *hwif, int reg, int *val) 
{
    return  hwif_reg_read_mul(hwif, reg, 1, val);
}

/*
 * 读寄存器位，通过读寄存器值再执行位操作即可，
 * 读寄存器位与读寄存器值效率一样，因此不提供功单独读寄存器位接口
 *(设计上不提供功能重复接口)
 */
int hwif_reg_read_bit(struct hw_intf *hwif, int reg, int bit, int *val) 
{
    int res = 0;
    struct hwif_io_pkt tx_pkt, rx_pkt;
    char cmd_str[CMD_STR_MAX_LEN] = {0};
    char cmd_ret_str[CMD_RET_STR_MAX_LEN] = {0};

    tx_pkt.len = snprintf(cmd_str, sizeof(cmd_str),CMD_REG_READ_BIT, reg, bit); 
    tx_pkt.data = cmd_str;
    tx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    rx_pkt.data = cmd_ret_str;
    rx_pkt.len = CMD_RET_STR_MAX_LEN;
    rx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    res = hwif_sync_io_op(hwif, &tx_pkt, &rx_pkt, REG_R_EXEC_TIME);
    if(0 != res) 
    {
        HWIF_PRT(ERR, "hwif %s io operation failed, res = %d\n", hwif->name, res);
        return -1;
    }

    if (STR_MATCH(rx_pkt.data, "01"))
        *val = 1;
    else
        *val = 0;

    return 0;
}
#endif

/*************************************************
  函数描述 : 写寄存器
  输入参数 : hwif -- 硬件接口句柄
             reg -- 寄存器号
             val -- 寄存器值.
  函数返回 :  0 -- 写成功
             -1 -- 写失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_write(struct hw_intf *hwif, int reg, int val) 
{
    int res = 0;
    struct hwif_io_pkt tx_pkt;
    char cmd_str[CMD_STR_MAX_LEN] = {0};

    tx_pkt.len = snprintf(cmd_str, sizeof(cmd_str), CMD_REG_WRITE, reg, val); 
    tx_pkt.data = cmd_str;
    tx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    res = hwif_sync_tx(hwif, tx_pkt.data, tx_pkt.len, &tx_pkt.actual_len, tx_pkt.timeout);
    if(res < 0)
    {
        HWIF_PRT(ERR, "hwif %s sync tx failed, res = %d\n", hwif->name, res);
        return -1;
    }

    /* 等执行完再退出 */
    usleep(REG_W_EXEC_TIME * 1000);

    return 0;
}

/*************************************************
  函数描述 : 写寄存器指定位
  输入参数 : hwif -- 硬件接口句柄
             reg -- 寄存器号
             bit -- 指定寄存器位 
             val -- 寄存器位值.
  函数返回 :  0 -- 写成功
             -1 -- 写失败
  备注     ：由于数据传输慢(串口通信)，使用读-改-写
             方式修改寄存器位效率低，因此单片机支持寄存器位操作
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_write_bit(struct hw_intf *hwif, int reg, int bit, int val) 
{
    int res = 0;
    struct hwif_io_pkt tx_pkt;
    char cmd_str[CMD_STR_MAX_LEN] = {0};

    tx_pkt.len = snprintf(cmd_str, sizeof(cmd_str),CMD_REG_WRITE_BIT , reg, bit, val > 0 ? 1 : 0); 
    tx_pkt.data = cmd_str;
    tx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    res = hwif_sync_tx(hwif, tx_pkt.data, tx_pkt.len, &tx_pkt.actual_len, tx_pkt.timeout);
    if(res < 0)
    {
        HWIF_PRT(ERR, "hwif %s sync tx failed, res = %d\n", hwif->name, res);
        return -1;
    }

    /* 等执行完再退出 */
    usleep(REG_W_EXEC_TIME * 1000);

    return 0;
}

/*************************************************
  函数描述 : 获取命令的返回信息。
  输入参数 : hwif -- 硬件接口句柄
             cmd_str -- 命令字符串
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- 信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
static int hwif_get_info(struct hw_intf *hwif, char *cmd_str, char *info_buf, int len, int *actual_len)
{
    
    int res = 0;
    struct hwif_io_pkt tx_pkt, rx_pkt;

    tx_pkt.len = strlen(cmd_str); 
    tx_pkt.data = cmd_str;
    tx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    rx_pkt.data = info_buf;
    rx_pkt.len = len;
    rx_pkt.timeout = HWIF_IO_SYNC_TIMEOUT;

    res = hwif_sync_io_op(hwif, &tx_pkt, &rx_pkt, CMD_EXEC_TIME);
    if(0 != res) 
    {
        HWIF_PRT(ERR, "hwif %s io operation failed, res = %d\n", hwif->name, res);
        return -1;
    }

    *actual_len = rx_pkt.actual_len;

    return 0;
}

/*************************************************
  函数描述 : 获取接口帮助信息。
  输入参数 : hwif -- 硬件接口句柄
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- 帮助信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_help_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len)
{
     return hwif_get_info(hwif,  CMD_HELP, info_buf, len, actual_len);
}

/*************************************************
  函数描述 : 获取接口版本信息。
  输入参数 : hwif -- 硬件接口句柄
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- 版本信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_verison_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len)
{
     return hwif_get_info(hwif,  CMD_VER, info_buf, len, actual_len);
}

/*************************************************
  函数描述 : 获取接口UID信息。
  输入参数 : hwif -- 硬件接口句柄
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- UID信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_uid_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len)
{
     return hwif_get_info(hwif,  CMD_UID, info_buf, len, actual_len);
}


/*************************************************
  函数描述 : 获取按键状态
  输入参数 : hwif -- 硬件接口句柄
  输出参数 : status --  返回的状态
                        =1 按键按下
                        =0 按键按没有下
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  备注     ：有按键按下后，过一定时间，单片机自动将状态置为0.
             状态为只读，不能手动设置按键状态。
  作者/时间 : junyu.yang@openvox.cn/2018.03.13
*************************************************/
int hwif_get_key_status(struct hw_intf *hwif, int *status) 
{
    int res;

    res = hwif_reg_read_mul(hwif, MB_REG_KEY_FLAG, 1, status);
    if(res != 0)
    {
        HWIF_PRT(ERR, "hwif %s read reg MB_REG_KEY_FLAG failed, res = %d\n", hwif->name, res);
        return -1;
    }
    else
    {
        return 0;
    }
}

/*************************************************
  函数描述 : 设置按键状态保持时间
  输入参数 : hwif -- 硬件接口句柄
  输出参数 : time_s --  状态保持时间，单位秒
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  备注     ：有按键按下后，过指定时间，单片机自动将状态置为0.
  作者/时间 : junyu.yang@openvox.cn/2018.03.13
*************************************************/
int hwif_set_key_status_hold_time(struct hw_intf *hwif, int time_s) 
{
    int res, time;

    time = time_s * 1000 / 100; /* 硬件寄存器对应的时间单位为 100ms */

    res = hwif_reg_write(hwif, MB_REG_KEY_TIME_THRD, time);
    if(res != 0)
    {
        HWIF_PRT(ERR, "hwif %s write reg  MB_REG_KEY_TIME_THRD failed, res = %d\n", hwif->name, res);
        return -1;
    }
    else
    {
        return 0;
    }
}

/*************************************************
  函数描述 : 获取槽位号
  输入参数 : hwif -- 硬件接口句柄
  输出参数 : solt_num --  获取到的槽位号
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : wengang.mu@openvox.cn/2018.05.16
*************************************************/
int hwif_get_solt_num(struct hw_intf *hwif, int *solt_num){
	int res;

	res = hwif_reg_read_mul(hwif, MB_REG_SLOT_NUM, 1, solt_num);
	if(res < 0){
		HWIF_PRT(ERR, "hwif %s get solt_num failed, res = %d\n", hwif->name, res);
		return -1;
	}
	if((*solt_num) >= 0 && (*solt_num) <= 2){
		(*solt_num) ++;
	}else if((*solt_num) == 3)
		return -1;
	return 0;
}
/*************************************************
  函数描述 : 设置模块板调试模式
  输入参数 : hwif -- 硬件接口句柄
             enable-- 0 关闭调试模式
				   -- 1 打开调试模式
  输出参数 : solt_num --  获取到的槽位号
  函数返回 :  0 -- 设置成功
             -1 -- 设置失败失败
  作者/时间 : wengang.mu@openvox.cn/2018.05.16
*************************************************/
int hwif_set_debug_uart(struct hw_intf *hwif, int enable){
	int res;
	if(enable){
		res = hwif_reg_write(hwif, MB_REG_UART_SW, 0x01);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s set uart switch failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}else{
		res = hwif_reg_write(hwif, MB_REG_UART_SW, 0x00);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s set uart switch failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}
	return 0;
}

/*************************************************
  函数说明:  设置VS_USB_XX系列产品的LED灯.
  输入参数:  hwif -- 硬件接口句柄;
                           cts -- 信号灯红灯指示, 0灭, 1亮;
                           rts -- 信号灯绿灯指示, 0灭, 1亮;
                           dtr -- 工作灯绿灯指示, 0灭, 1亮;
                           high_flag -- 高通道标示, 0表示寄存器16, 低通道1跟2;  1表示寄存器15, 高通道3跟4;
  输出参数:  无
  返回值     :   0 -- 设置成功
                          -1 -- 设置失败
  作者/时间:  gengxin.chen@openvox.cn / 20180725
*************************************************/
int hwif_set_led_lamp(struct hw_intf *hwif, int cts1, int rts1, int dtr1, int cts2, int rts2, int dtr2, int high_flag)
{
	int res;
	int led_val;

	led_val = (cts1&0x01) | ((rts1&0x01) << 1) | ((dtr1&0x01) << 2) | ((cts2&0x01) << 3) | ((rts2&0x01) << 4) | ((dtr2&0x01) << 5);
	if (high_flag) {
		res = hwif_reg_write(hwif, MB_REG_LED_LAMP_MOD34, led_val);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s set led lamp for module 3 and 4 failed, res = %d\n", hwif->name, res);
			return -1;
		}
	} else {
		res = hwif_reg_write(hwif, MB_REG_LED_LAMP_MOD12, led_val);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s set led lamp for module 1 and 2 failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}
	return 0;
}

/*************************************************
  函数说明:  设置VS_USB_XX系列产品的LED灯
         为关闭状态, 用于系统刚启动时.
  输入参数:  hwif -- 硬件接口句柄;
  输出参数:  无
  返回值     :   0 -- 设置成功
                          -1 -- 设置失败
  作者/时间:  gengxin.chen@openvox.cn / 20180725
*************************************************/
int hwif_set_all_led_lamp(struct hw_intf *hwif, int on_flag)
{
	int res;
	int led_val = (on_flag)?0x3f:0;

	res = hwif_reg_write(hwif, MB_REG_LED_LAMP_MOD34, led_val);
	res |= hwif_reg_write(hwif, MB_REG_LED_LAMP_MOD12, led_val);
	if(res < 0){
		HWIF_PRT(ERR, "hwif %s set led lamp off failed, res = %d\n", hwif->name, res);
		return -1;
	}

	return 0;
}

/*************************************************
  函数说明:  设置波特率
  输入参数:  hwif -- 硬件接口句柄;
             baud_flag, 0--9600
                        1--115200
  输出参数:  无
  返回值  :  0 -- 设置成功
            -1 -- 设置失败
  作者/时间:  wengang.mu@openvox.cn / 20180820
*************************************************/
int mod_brd_set_baud(struct hw_intf * hwif, int baud_flag){
	int res;
	if(baud_flag){
		//115200
		res = hwif_reg_write(hwif, MB_REG_BAUD, 0x01);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s set uart baud 115200 failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}else{
		//9600
		res = hwif_reg_write(hwif, MB_REG_BAUD, 0x00);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s set uart baud 9600 failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}
	return 0;
}

/*************************************************
  函数说明:  设置VS_USB_XX系列产品的system LED灯
           0 为关闭状态, 用于系统刚启动时.
  输入参数:  hwif -- 硬件接口句柄;
  输出参数:  无
  返回值     :   0 -- 设置成功
                 -1 -- 设置失败
  作者/时间:  wengang.mu@openvox.cn / 20180925
*************************************************/

int mod_brd_set_sys_led_lamp(struct hw_intf *hwif, int status ){
	int res = 0;
	if(status){
		res = hwif_reg_write(hwif, MB_REG_LED_LAMP_SYS, 0x01);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s turn on system led failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}else{
		res = hwif_reg_write(hwif, MB_REG_LED_LAMP_SYS, 0x00);
		if(res < 0){
			HWIF_PRT(ERR, "hwif %s turn off system led failed, res = %d\n", hwif->name, res);
			return -1;
		}
	}
	return 0;
}


int sim_chn_set_slot(struct hw_intf *hwif, int reg,  int value){
	int res = 0;
	res = hwif_reg_write(hwif, reg, value);
	if(res < 0){
		HWIF_PRT(ERR, "set chan slot failed, res = %d\n", hwif->name, res);
		return -1;
	}
	return 0;
}
