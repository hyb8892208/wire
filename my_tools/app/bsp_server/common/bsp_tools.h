/**********************************************************************************
�ļ�����  : ���õĴ���
����/ʱ�� : zhongwei.peng@openvox.cn/2017.12.8
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
�������� : 16�����ַ�ת��ֵ
������� : c -- �ַ��������������
������� : 
�� �� ֵ : ��ֵ��
��   ��  : zhongwei.peng
ʱ   ��  : 2016.11.25
************************************************************/
unsigned char char_to_val(unsigned char c);

/**********************************************************
�������� : �ַ���ת��ֵ
������� : str -- �ַ���
������� : 
�� �� ֵ : ��ֵ��
��   ��  : zhongwei.peng
ʱ   ��  : 2016.11.25
************************************************************/
int str_to_int(unsigned char *str);

/* ���ڴ���رսӿ� */
int open_serial
(
    char *serial_file,           /* �����豸�ļ�����·�� */ 
    unsigned int rate,           /* ������ */
    unsigned char parity,        /* �Ƿ���żУ�� */
    unsigned char databits,      /* ����λ */
    unsigned char stopbits,      /* ֹͣλ */
    unsigned char streamcontrol  /* �Ƿ����� */
);
void close_serial(unsigned int serial_fd);

/************************** ��ʱ����� *********************************/

/* ��������������ʽ���� */
typedef void (*BS_TIMER_FUNC)(void);

void bs_timer_init(void);
void bs_timer_destroy(void);
int bs_timer_add(BS_TIMER_FUNC func, unsigned int interval);
void bs_timer_del(BS_TIMER_FUNC func);
void bs_timer_run(void);
int bsp_print(int level, const char *formt, ... );
/************************** ��ʱ�����(end) ****************************/














#endif
