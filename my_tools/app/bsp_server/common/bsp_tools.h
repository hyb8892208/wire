/**********************************************************************************
文件描述  : 公用的代码
作者/时间 : zhongwei.peng@openvox.cn/2017.12.8
***********************************************************************************/
#ifndef __BSP_TOOLS_H_
#define __BSP_TOOLS_H_
#include <stdio.h> /* for printf */

extern unsigned int bsp_prt_unmask;

#ifndef BIT
#define BIT(i)      (1UL << (i))
#endif

#define BSP_ERR      BIT(0)            //error
#define BSP_WARN     BIT(1)            //warming
#define BSP_INFO     BIT(2)            //information
#define BSP_DBG      BIT(3)            //debug
#define BSP_TRC      BIT(4)            //track
#define BSP_VERB     BIT(5)            //verbose
#define BSP_TST      BIT(6)            //test
#define BSP_PREFIX   "BSP "            //Output prefix

extern FILE *bsp_log_fd; 
/* bsp print function */
#ifndef BSP_PRT  
#define BSP_PRT(bits, fmt, args...)                             \
    ((void)((bsp_prt_unmask & (BSP_ ## bits)) &&                \
    (bsp_print(BSP_##bits, fmt, ##args))))
//        (fprintf(bsp_log_fd, BSP_PREFIX #bits " : " fmt, ## args))))
 
#endif


/**********************************************************
函数描述 : 16进制字符转数值
输入参数 : c -- 字符，不作参数检查
输出参数 : 
返 回 值 : 数值，
作   者  : zhongwei.peng
时   间  : 2016.11.25
************************************************************/
unsigned char char_to_val(unsigned char c);

/**********************************************************
函数描述 : 字符串转数值
输入参数 : str -- 字符串
输出参数 : 
返 回 值 : 数值，
作   者  : zhongwei.peng
时   间  : 2016.11.25
************************************************************/
int str_to_int(unsigned char *str);

/* 串口打开与关闭接口 */
int open_serial
(
    char *serial_file,           /* 串口设备文件绝对路径 */ 
    unsigned int rate,           /* 波特率 */
    unsigned char parity,        /* 是否奇偶校验 */
    unsigned char databits,      /* 数据位 */
    unsigned char stopbits,      /* 停止位 */
    unsigned char streamcontrol  /* 是否流控 */
);
void close_serial(unsigned int serial_fd);

/************************** 定时器相关 *********************************/

/* 被调用任务函数格式声明 */
typedef void (*BS_TIMER_FUNC)(void);

void bs_timer_init(void);
void bs_timer_destroy(void);
int bs_timer_add(BS_TIMER_FUNC func, unsigned int interval);
void bs_timer_del(BS_TIMER_FUNC func);
void bs_timer_run(void);
int bsp_print(int level, const char *formt, ... );
/************************** 定时器相关(end) ****************************/














#endif
