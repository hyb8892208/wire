#ifndef __MOD_BRD_REGS_H_
#define __MOD_BRD_REGS_H_

/**********************************************************************************
�ļ�����  : ����ģ���(module board)�Ĵ���
����/ʱ�� : junyu.yang@openvox.cn/2018.01.26
***********************************************************************************/

/******    ���ƼĴ���                    *********************/
#define     MB_REG_SIM_EN_L        0     /* sim��ʹ�ܼĴ���(��)��0=disable, 1=enable */
#define     MB_REG_SIM_EN_H        1     /* ͬ�ϣ���λ��ַ */
#define     MB_REG_MODULE_VBAT_L   2     /* ģ�鹩����ƣ�0=���磬1=�ϵ� */
#define     MB_REG_MODULE_VBAT_H   3     /* ͬ�ϣ���λ��ַ */
#define     MB_REG_EMERG_OFF_L     4     /* m35: ����20ms�����ػ�; sim6320c: ����50-200ms���� */
#define     MB_REG_EMERG_OFF_H     5     /* ͬ�ϣ���λ��ַ */
#define     MB_REG_PWRKEY_L        6     /* m35:����0.6-1��ػ�������2s���Ͽ���; sim6320c: ����1-5s�ػ�������30ms���Ͽ��� */
#define     MB_REG_PWRKEY_H        7     /* ͬ�ϣ���λ��ַ */
#define     MB_REG_SIM_INSERT_H    8     /* SIM������״̬��0=��λ��1=����λ */
#define     MB_REG_SIM_INSERT_L    9     /* ͬ�ϣ���λ��ַ */
#define     MB_REG_MODULE_EXIST    10    /* �Ӱ���λ״̬��0=��λ��1=����λ */
#define     MB_REG_MODULE_ON_OFF_H 11    /* ģ�鿪�ػ�״̬��0=�ػ���1=���� */
#define     MB_REG_MODULE_ON_OFF_L 12    /* ͬ�ϣ���λ��ַ */
#define     MB_REG_CH444_CTRL_L    13    /* usbģ�⿪��ѡ�� */
#define     MB_REG_CH444_CTRL_H    14    /* usbģ�⿪��ѡ�� */

/******    ͨ�üĴ���                    *********************/
//�Ĵ��� 15 - 60 ����������չ����hc595&lv165������������
#define     MB_REG_LED_LAMP_MOD34	15	/* ����ģ��3 / 4 ��LED�� , 0:��, 1:��. */
#define     MB_REG_LED_LAMP_MOD12	16	/* ����ģ��1 / 2 ��LED�� , 0:��, 1:��. */
#define     MB_REG_BAUD            57
#define     MB_REG_SLOT_NUM        58
#define     MB_REG_LED_LAMP_SYS    59
#define     MB_REG_UART_SW         60    /* UART���ԼĴ�����VS_USB ec20f��uc15ģ����Ҫ�رյ���ģʽ������������*/
#define     MB_REG_KEY_FLAG        61    /* ������־ */
#define     MB_REG_TEST            62    /* ���淭תֵ������д0xAA���򱣴�Ϊ0x55�����ڶ�д���� */
//�Ĵ��� 63 - 126 ΪreserveD�Ĵ�����ram���綪ʧ

//�Ĵ��� 127 - 252 ΪEEPROM�Ĵ��� ,  eeprom�Ĵ���������ʧ
#define     MB_REG_KEY_TIME_THRD   253   /* ����ʱ����ֵ */
#define     MB_REG_KEY_TIME_OUT    254   /* ������־��ʱ */

#define     MB_REG_WIDE            8     /* �Ĵ���λ��8bits */

#define     SIM_REG_SELECT_1 0
#define     SIM_REG_SELECT_2 1
#define     SIM_REG_SELECT_3 2
#define     SIM_REG_SELECT_4 3

#define     SIM_REG_STATUS_1 16
#define     SIM_REG_STATUS_2 17
#define     SIM_REG_STATUS_3 18
#define     SIM_REG_STATUS_4 19
#define     SIM_REG_STATUS_5 20
#define     SIM_REG_STATUS_6 21
#define     SIM_REG_STATUS_7 22
#define     SIM_REG_STATUS_8 23

#endif /* __MOD_BRD_REGS_H_ */
