
#ifndef __HW_INTF_H_
#define __HW_INTF_H_

#include <linux/limits.h> /* for PATH_MAX */

struct hw_intf 
{
    int fd;             /* 接口文件描述符 */
    char name[PATH_MAX];/* 接口名称 */
    int chans;          /* 接口所有通道数（包括插入，未插入）*/
};

/*************************************************
  函数描述 : 读1个或多个连续的寄存器
  输入参数 : hwif -- 硬件接口文件描述符
             reg -- 起始寄存器号
             num -- 寄存器数量, 大于等于1.
  输出参数 : vals -- 指针，用于存放寄存器值，空间不能少于'num'。
  函数返回 : 0 -- 发送成功
             1 -- 发送超时
             -1 -- 发送失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_read_mul(struct hw_intf *hwif, int reg, int num, int *vals);

/*************************************************
  函数描述 : 写寄存器
  输入参数 : hwif -- 硬件接口文件描述符
             reg -- 寄存器号
             val -- 寄存器值.
  函数返回 :  0 -- 写成功
             -1 -- 写失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_write(struct hw_intf *hwif, int reg, int val) ;

/*************************************************
  函数描述 : 写寄存器指定位
  输入参数 : hwif -- 硬件接口文件描述符
             reg -- 寄存器号
             bit -- 指定寄存器位 
             val -- 寄存器值.
  函数返回 :  0 -- 写成功
             -1 -- 写失败
  备注     ：由于数据传输慢(串口通信)，使用读-改-写
             方式修改寄存器位效率低，因此单片机支持寄存器位操作
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_reg_write_bit(struct hw_intf *hwif, int reg, int bit, int val) ;

/*************************************************
  函数描述 : 获取接口帮助信息。
  输入参数 : hwif -- 硬件接口文件描述符
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- 帮助信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_help_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len);

/*************************************************
  函数描述 : 获取接口版本信息。
  输入参数 : hwif -- 硬件接口文件描述符
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- 版本信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_verison_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len);

/*************************************************
  函数描述 : 获取接口UID信息。
  输入参数 : hwif -- 硬件接口文件描述符
             len -- 期望返回信息长度，即info_buf的长度，单位 byte, 
  输出参数 : info_buf -- UID信息返回缓冲区
             actual_len -- 实际返回信息长度，单位 byte
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  作者/时间 : junyu.yang@openvox.cn/2018.01.25
*************************************************/
int hwif_get_uid_info(struct hw_intf *hwif, char *info_buf, int len, int *actual_len);

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
int hwif_get_key_status(struct hw_intf *hwif, int *status);

/*************************************************
  函数描述 : 设置按键状态保持时间
  输入参数 : hwif -- 硬件接口句柄
  输出参数 : time_s --  状态保持时间，单位秒
  函数返回 :  0 -- 获取成功
             -1 -- 获取失败
  备注     ：有按键按下后，过指定时间，单片机自动将状态置为0.
  作者/时间 : junyu.yang@openvox.cn/2018.03.13
*************************************************/
int hwif_set_key_status_hold_time(struct hw_intf *hwif, int time_s);


int hwif_get_solt_num(struct hw_intf *hwif, int *solt_num);


/*************************************************
  函数描述 : 设置debug_uart开关
  输入参数 : hwif -- 硬件接口句柄
			 enable 1--打开调试开关
			        0--关闭调试开关
  函数返回 :  0 -- 设置成功
             -1 -- 设置失败
  作者/时间 : junyu.yang@openvox.cn/2018.06.04
*************************************************/
int hwif_set_debug_uart(struct hw_intf *hwif, int enable);

int hwif_set_led_lamp(struct hw_intf *hwif, int cts1, int rts1, int dtr1, int cts2, int rts2, int dtr2, int high_flag);

int hwif_set_all_led_lamp(struct hw_intf *hwif, int on_flag);

int mod_brd_set_baud(struct hw_intf * hwif, int baud_flag);

int mod_brd_set_sys_led_lamp(struct hw_intf *hwif, int status );

int sim_chn_set_slot(struct hw_intf *hwif, int chan, int value);
#endif /* __HW_INTF_H_ */

