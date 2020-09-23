/**********************************************************************************
�ļ�����  : ʵ��ģ���(module board)MCUͨ�Žӿڡ�
            ���������շ�����������ʵ�֣��������������
            1) �Ĵ�����д
            2) ��ȡ������Ϣ
            3) ��ȡ�汾��Ϣ
            4) ��ȡuid
����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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
/* ����ͨ������ */
#define CMD_REG_READ      "read %d\n"         /*  �������Ĵ��� read <reg>h\n */ 
#define CMD_REG_READ_MUL  "read %d-%02xh\n"   /*  ������Ĵ��� read <reg>-<width>h\n */ 
#define CMD_REG_READ_BIT  "read %d.%d\n"      /*  ���Ĵ���λ  read <reg>.<bit>\n */ 
#define CMD_REG_WRITE     "write %d=%02xh\n"  /*  д�Ĵ���    write <reg>=<val>h\n  */
#define CMD_REG_WRITE_BIT "write %d.%d=%d\n"  /*  д�Ĵ���λ  write <reg>.<bit>=<val>\n */
#define CMD_HELP    "help\n"    /*  ��ȡ���������Ϣ */
#define CMD_VER     "ver\n"     /*  ��ȡ�汾��Ϣ���豸���ƣ������Ӳ���汾�ţ�\n */
#define CMD_UID     "uid\n"     /*  ��ȡuid��Ϣ\n */

#define CMD_STR_MAX_LEN         24  /* �����ַ�����󳤶�, ��λ byte */
#define CMD_RET_STR_MAX_LEN     512  /* ִ�����������ص��ַ�����󳤶�, ��λ byte */

#define HWIF_IO_SYNC_TIMEOUT    2000 /* Ӳ���ӿ� IO ͬ��������ʱ����λ ms */

/* ������2����������д��������1��֮���������ܻᱻ����
 * ������Ҫ������ִ����� */
#define CMD_EXEC_TIME           80  /* ����ִ��ʱ�䣬��λ ms */
#define REG_R_EXEC_TIME         15  /* ���Ĵ���ִ��ʱ�䣬��λ ms */
#define REG_W_EXEC_TIME         15  /* д�Ĵ���ִ��ʱ�䣬��λ ms */

/* �ж����ַ����Ƿ���� */
#define STR_MATCH(str1, str2)   (0 == strncmp(str1, str2, strlen(str2)))

/* Log  ��� */
#define HWIF_PRT   BSP_PRT                          

struct hwif_io_pkt 
{
    char *data;         /* ָ�룬ָ��io�������ݻ����� */
    int  len;           /* data ָ�����ݳ��ȣ���λ byte. */
    int  actual_len;    /* ʵ��io�������ݳ��ȣ���λ byte */
    int  timeout;       /* io������ʱ, ��λ ms */
};

/*************************************************
  �������� : Ӳ���ӿ�ͬ����������
  ������� : hwif -- Ӳ���ӿھ��
             data -- ָ�룬ָ��Ҫ���͵�����
             len -- Ҫ���͵����ݳ���
             timeout -- ��ʱ����λms, timeout ms ��δ�����򷵻�;
                        =0, ���ȴ���
  ������� : actual_len -- ʵ�ʷ��ͳ��ȣ���λ:byte.
  �������� :  0 -- ���ͳɹ�
              1 -- ���ͳ�ʱ
             -1 -- ����ʧ��
  ��ע��timeout Ŀǰ��������
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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
    /* Ŀǰû�г�ʱ���˹���Ԥ��*/
}

/*************************************************
  �������� : Ӳ���ӿ�ͬ����������
  ������� : hwif -- Ӳ���ӿھ��
             data -- ָ�룬ָ��������ݻ�����
             len -- Ҫ���͵����ݳ���
             timeout -- ��ʱ����λms, timeout ms ��δ�����򷵻�,
                        =0, ���ȴ���
  ������� : actual_len -- ʵ�ʽ��ճ��ȣ���λ:byte.
  �������� :  0 -- ���ճɹ�
              1 -- ���ճ�ʱ
             -1 -- ����ʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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
            /* ��ʱ��Ҫ��գ��������������VERB������ERR.*/
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
  �������� : Ӳ���ӿ�ͬ�����͡���������
  ������� : hwif -- Ӳ���ӿھ��
             tx_pkt -- ָ�룬ָ��Ҫ���͵����ݰ�
             rx_pkt -- ָ�룬ָ��Ҫ���յ����ݰ�
             prcs_time_ms --  �޶˴��������ʱ�䣬��λms
  ������� : ��
  �������� :  0 -- ���͡����ճɹ�
              1 -- ��Ҫ���ͳ�����ʵ�ʷ��ͳ��Ȳ���
             -1 -- ���ͳ�ʱ
             -2 -- ����ʧ��
  ��ע���������Ҫ���ճ�����ʵ�ʽ��ճ����Ƿ񲻵�
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
static int hwif_sync_io_op(struct hw_intf *hwif, struct hwif_io_pkt *tx_pkt, struct hwif_io_pkt *rx_pkt, int prcs_time_ms)
{
    int res = 0;
    int actual_len;
    char tmp_buf[256] = {0};
    
    /* ������������� */
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

    /* �ȴ������ٽ��� */
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
  �������� : ��1�����������ļĴ���
  ������� : hwif -- Ӳ���ӿھ��
             reg -- ��ʼ�Ĵ�����
             num -- �Ĵ�������, ���ڵ���1.
  ������� : vals -- ָ�룬���ڴ�żĴ���ֵ���ռ䲻������'num'��
  �������� : 0 -- ���ͳɹ�
             1 -- ���ͳ�ʱ
             -1 -- ����ʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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

    /* �ַ�����ʽ: "FF,FF,00,00,00,00"��3���ַ�1��ֵ */ 
    for (i = 0; i < num; i++)
    {
        vals[i] = (char_to_val(rx_pkt.data[i*3]) << 4) + char_to_val(rx_pkt.data[i*3+1]);
    }

    return 0;
}

/*
 * Comments by junyu.yang@openvox  2018.01.25
 *���������ӿ���������ӿڣ� ����Ƭ��֧�֣����ֻʵ�֣���ʹ��
 */
#if 0 
/*
 *CMD_REG_READ �� CMD_REG_READ_MUL  ������Ч�ʲ�࣬����
 *hwif_reg_read_mul(hwif, reg, 1, val)
 *��Ϊֻ��һ���Ĵ���ֵ����˲��ṩ��������һ���Ĵ����ӿ�
 *(����ϲ��ṩ�����ظ��ӿ�)
 */
int hwif_reg_read(struct hw_intf *hwif, int reg, int *val) 
{
    return  hwif_reg_read_mul(hwif, reg, 1, val);
}

/*
 * ���Ĵ���λ��ͨ�����Ĵ���ֵ��ִ��λ�������ɣ�
 * ���Ĵ���λ����Ĵ���ֵЧ��һ������˲��ṩ���������Ĵ���λ�ӿ�
 *(����ϲ��ṩ�����ظ��ӿ�)
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
  �������� : д�Ĵ���
  ������� : hwif -- Ӳ���ӿھ��
             reg -- �Ĵ�����
             val -- �Ĵ���ֵ.
  �������� :  0 -- д�ɹ�
             -1 -- дʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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

    /* ��ִ�������˳� */
    usleep(REG_W_EXEC_TIME * 1000);

    return 0;
}

/*************************************************
  �������� : д�Ĵ���ָ��λ
  ������� : hwif -- Ӳ���ӿھ��
             reg -- �Ĵ�����
             bit -- ָ���Ĵ���λ 
             val -- �Ĵ���λֵ.
  �������� :  0 -- д�ɹ�
             -1 -- дʧ��
  ��ע     ���������ݴ�����(����ͨ��)��ʹ�ö�-��-д
             ��ʽ�޸ļĴ���λЧ�ʵͣ���˵�Ƭ��֧�ּĴ���λ����
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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

    /* ��ִ�������˳� */
    usleep(REG_W_EXEC_TIME * 1000);

    return 0;
}

/*************************************************
  �������� : ��ȡ����ķ�����Ϣ��
  ������� : hwif -- Ӳ���ӿھ��
             cmd_str -- �����ַ���
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- ��Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
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
  �������� : ��ȡ�ӿڰ�����Ϣ��
  ������� : hwif -- Ӳ���ӿھ��
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- ������Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_help_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len)
{
     return hwif_get_info(hwif,  CMD_HELP, info_buf, len, actual_len);
}

/*************************************************
  �������� : ��ȡ�ӿڰ汾��Ϣ��
  ������� : hwif -- Ӳ���ӿھ��
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- �汾��Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_verison_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len)
{
     return hwif_get_info(hwif,  CMD_VER, info_buf, len, actual_len);
}

/*************************************************
  �������� : ��ȡ�ӿ�UID��Ϣ��
  ������� : hwif -- Ӳ���ӿھ��
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- UID��Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_uid_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len)
{
     return hwif_get_info(hwif,  CMD_UID, info_buf, len, actual_len);
}


/*************************************************
  �������� : ��ȡ����״̬
  ������� : hwif -- Ӳ���ӿھ��
  ������� : status --  ���ص�״̬
                        =1 ��������
                        =0 ������û����
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ��ע     ���а������º󣬹�һ��ʱ�䣬��Ƭ���Զ���״̬��Ϊ0.
             ״̬Ϊֻ���������ֶ����ð���״̬��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.03.13
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
  �������� : ���ð���״̬����ʱ��
  ������� : hwif -- Ӳ���ӿھ��
  ������� : time_s --  ״̬����ʱ�䣬��λ��
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ��ע     ���а������º󣬹�ָ��ʱ�䣬��Ƭ���Զ���״̬��Ϊ0.
  ����/ʱ�� : junyu.yang@openvox.cn/2018.03.13
*************************************************/
int hwif_set_key_status_hold_time(struct hw_intf *hwif, int time_s) 
{
    int res, time;

    time = time_s * 1000 / 100; /* Ӳ���Ĵ�����Ӧ��ʱ�䵥λΪ 100ms */

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
  �������� : ��ȡ��λ��
  ������� : hwif -- Ӳ���ӿھ��
  ������� : solt_num --  ��ȡ���Ĳ�λ��
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : wengang.mu@openvox.cn/2018.05.16
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
  �������� : ����ģ������ģʽ
  ������� : hwif -- Ӳ���ӿھ��
             enable-- 0 �رյ���ģʽ
				   -- 1 �򿪵���ģʽ
  ������� : solt_num --  ��ȡ���Ĳ�λ��
  �������� :  0 -- ���óɹ�
             -1 -- ����ʧ��ʧ��
  ����/ʱ�� : wengang.mu@openvox.cn/2018.05.16
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
  ����˵��:  ����VS_USB_XXϵ�в�Ʒ��LED��.
  �������:  hwif -- Ӳ���ӿھ��;
                           cts -- �źŵƺ��ָʾ, 0��, 1��;
                           rts -- �źŵ��̵�ָʾ, 0��, 1��;
                           dtr -- �������̵�ָʾ, 0��, 1��;
                           high_flag -- ��ͨ����ʾ, 0��ʾ�Ĵ���16, ��ͨ��1��2;  1��ʾ�Ĵ���15, ��ͨ��3��4;
  �������:  ��
  ����ֵ     :   0 -- ���óɹ�
                          -1 -- ����ʧ��
  ����/ʱ��:  gengxin.chen@openvox.cn / 20180725
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
  ����˵��:  ����VS_USB_XXϵ�в�Ʒ��LED��
         Ϊ�ر�״̬, ����ϵͳ������ʱ.
  �������:  hwif -- Ӳ���ӿھ��;
  �������:  ��
  ����ֵ     :   0 -- ���óɹ�
                          -1 -- ����ʧ��
  ����/ʱ��:  gengxin.chen@openvox.cn / 20180725
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
  ����˵��:  ���ò�����
  �������:  hwif -- Ӳ���ӿھ��;
             baud_flag, 0--9600
                        1--115200
  �������:  ��
  ����ֵ  :  0 -- ���óɹ�
            -1 -- ����ʧ��
  ����/ʱ��:  wengang.mu@openvox.cn / 20180820
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
  ����˵��:  ����VS_USB_XXϵ�в�Ʒ��system LED��
           0 Ϊ�ر�״̬, ����ϵͳ������ʱ.
  �������:  hwif -- Ӳ���ӿھ��;
  �������:  ��
  ����ֵ     :   0 -- ���óɹ�
                 -1 -- ����ʧ��
  ����/ʱ��:  wengang.mu@openvox.cn / 20180925
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
