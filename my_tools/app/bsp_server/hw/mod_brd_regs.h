#ifndef __MOD_BRD_REGS_H_
#define __MOD_BRD_REGS_H_

/**********************************************************************************
文件描述  : 定义模块板(module board)寄存器
作者/时间 : junyu.yang@openvox.cn/2018.01.26
***********************************************************************************/

/******    控制寄存器                    *********************/
#define     MB_REG_SIM_EN_L        0     /* sim卡使能寄存器(低)，0=disable, 1=enable */
#define     MB_REG_SIM_EN_H        1     /* 同上，高位地址 */
#define     MB_REG_MODULE_VBAT_L   2     /* 模块供电控制，0=供电，1=断电 */
#define     MB_REG_MODULE_VBAT_H   3     /* 同上，高位地址 */
#define     MB_REG_EMERG_OFF_L     4     /* m35: 拉高20ms紧急关机; sim6320c: 拉高50-200ms重启 */
#define     MB_REG_EMERG_OFF_H     5     /* 同上，高位地址 */
#define     MB_REG_PWRKEY_L        6     /* m35:拉低0.6-1秒关机，拉低2s以上开机; sim6320c: 拉低1-5s关机，拉低30ms以上开机 */
#define     MB_REG_PWRKEY_H        7     /* 同上，高位地址 */
#define     MB_REG_SIM_INSERT_H    8     /* SIM卡插入状态，0=在位，1=不在位 */
#define     MB_REG_SIM_INSERT_L    9     /* 同上，低位地址 */
#define     MB_REG_MODULE_EXIST    10    /* 子板在位状态，0=在位，1=不在位 */
#define     MB_REG_MODULE_ON_OFF_H 11    /* 模块开关机状态，0=关机，1=开机 */
#define     MB_REG_MODULE_ON_OFF_L 12    /* 同上，低位地址 */
#define     MB_REG_CH444_CTRL_L    13    /* usb模拟开关选择 */
#define     MB_REG_CH444_CTRL_H    14    /* usb模拟开关选择 */

/******    通用寄存器                    *********************/
//寄存器 15 - 60 保留用于扩展级联hc595&lv165，或其他功能
#define     MB_REG_LED_LAMP_MOD34	15	/* 控制模块3 / 4 的LED灯 , 0:灭, 1:亮. */
#define     MB_REG_LED_LAMP_MOD12	16	/* 控制模块1 / 2 的LED灯 , 0:灭, 1:亮. */
#define     MB_REG_BAUD            57
#define     MB_REG_SLOT_NUM        58
#define     MB_REG_LED_LAMP_SYS    59
#define     MB_REG_UART_SW         60    /* UART调试寄存器，VS_USB ec20f和uc15模块需要关闭调试模式才能正常工作*/
#define     MB_REG_KEY_FLAG        61    /* 按键标志 */
#define     MB_REG_TEST            62    /* 保存翻转值，例如写0xAA，则保存为0x55，用于读写测试 */
//寄存器 63 - 126 为reserveD寄存器，ram掉电丢失

//寄存器 127 - 252 为EEPROM寄存器 ,  eeprom寄存器，非易失
#define     MB_REG_KEY_TIME_THRD   253   /* 按键时间阈值 */
#define     MB_REG_KEY_TIME_OUT    254   /* 按键标志超时 */

#define     MB_REG_WIDE            8     /* 寄存器位宽，8bits */

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
