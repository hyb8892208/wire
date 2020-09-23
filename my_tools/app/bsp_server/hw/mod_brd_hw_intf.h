
#ifndef __HW_INTF_H_
#define __HW_INTF_H_

#include <linux/limits.h> /* for PATH_MAX */

struct hw_intf 
{
    int fd;             /* �ӿ��ļ������� */
    char name[PATH_MAX];/* �ӿ����� */
    int chans;          /* �ӿ�����ͨ�������������룬δ���룩*/
};

/*************************************************
  �������� : ��1�����������ļĴ���
  ������� : hwif -- Ӳ���ӿ��ļ�������
             reg -- ��ʼ�Ĵ�����
             num -- �Ĵ�������, ���ڵ���1.
  ������� : vals -- ָ�룬���ڴ�żĴ���ֵ���ռ䲻������'num'��
  �������� : 0 -- ���ͳɹ�
             1 -- ���ͳ�ʱ
             -1 -- ����ʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_read_mul(struct hw_intf *hwif, int reg, int num, int *vals);

/*************************************************
  �������� : д�Ĵ���
  ������� : hwif -- Ӳ���ӿ��ļ�������
             reg -- �Ĵ�����
             val -- �Ĵ���ֵ.
  �������� :  0 -- д�ɹ�
             -1 -- дʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_write(struct hw_intf *hwif, int reg, int val) ;

/*************************************************
  �������� : д�Ĵ���ָ��λ
  ������� : hwif -- Ӳ���ӿ��ļ�������
             reg -- �Ĵ�����
             bit -- ָ���Ĵ���λ 
             val -- �Ĵ���ֵ.
  �������� :  0 -- д�ɹ�
             -1 -- дʧ��
  ��ע     ���������ݴ�����(����ͨ��)��ʹ�ö�-��-д
             ��ʽ�޸ļĴ���λЧ�ʵͣ���˵�Ƭ��֧�ּĴ���λ����
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_write_bit(struct hw_intf *hwif, int reg, int bit, int val) ;

/*************************************************
  �������� : ��ȡ�ӿڰ�����Ϣ��
  ������� : hwif -- Ӳ���ӿ��ļ�������
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- ������Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_help_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len);

/*************************************************
  �������� : ��ȡ�ӿڰ汾��Ϣ��
  ������� : hwif -- Ӳ���ӿ��ļ�������
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- �汾��Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_verison_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len);

/*************************************************
  �������� : ��ȡ�ӿ�UID��Ϣ��
  ������� : hwif -- Ӳ���ӿ��ļ�������
             len -- ����������Ϣ���ȣ���info_buf�ĳ��ȣ���λ byte, 
  ������� : info_buf -- UID��Ϣ���ػ�����
             actual_len -- ʵ�ʷ�����Ϣ���ȣ���λ byte
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_uid_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len);

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
int hwif_get_key_status(struct hw_intf *hwif, int *status);

/*************************************************
  �������� : ���ð���״̬����ʱ��
  ������� : hwif -- Ӳ���ӿھ��
  ������� : time_s --  ״̬����ʱ�䣬��λ��
  �������� :  0 -- ��ȡ�ɹ�
             -1 -- ��ȡʧ��
  ��ע     ���а������º󣬹�ָ��ʱ�䣬��Ƭ���Զ���״̬��Ϊ0.
  ����/ʱ�� : junyu.yang@openvox.cn/2018.03.13
*************************************************/
int hwif_set_key_status_hold_time(struct hw_intf *hwif, int time_s);


int hwif_get_solt_num(struct hw_intf *hwif, int *solt_num);


/*************************************************
  �������� : ����debug_uart����
  ������� : hwif -- Ӳ���ӿھ��
			 enable 1--�򿪵��Կ���
			        0--�رյ��Կ���
  �������� :  0 -- ���óɹ�
             -1 -- ����ʧ��
  ����/ʱ�� : junyu.yang@openvox.cn/2018.06.04
*************************************************/
int hwif_set_debug_uart(struct hw_intf *hwif, int enable);

int hwif_set_led_lamp(struct hw_intf *hwif, int cts1, int rts1, int dtr1, int cts2, int rts2, int dtr2, int high_flag);

int hwif_set_all_led_lamp(struct hw_intf *hwif, int on_flag);

int mod_brd_set_baud(struct hw_intf * hwif, int baud_flag);

int mod_brd_set_sys_led_lamp(struct hw_intf *hwif, int status );

int sim_chn_set_slot(struct hw_intf *hwif, int chan, int value);
#endif /* __HW_INTF_H_ */

