
#ifndef __BSP_API_H_
#define __BSP_API_H_


typedef enum chan_mod_lamp_status_e
{
    CHAN_LAMP_STATUS_OFF = 0,       
    CHAN_LAMP_STATUS_GREEN_ON,         
    CHAN_LAMP_STATUS_GREEN_FLASH,     
    CHAN_LAMP_STATUS_RED_ON,
    CHAN_LAMP_STATUS_RED_FLASH,
    CHNA_LAMP_STATUS_YELLOW_ON,
    CHNA_LAMP_STATUS_YELLOW_FLASH,
    CHAN_LAMP_STATUS_MAX,
} CHAN_MOD_LAMP_STATUS_E;

typedef enum _sys_type_e_
{
	SYS_TYPE_UNKOWN = 0,
	SYS_TYPE_SWG20XX,
	SYS_TYPE_1CHAN4SIMS,
	SYS_TYPE_VS_USB,
	SYS_TYPE_VS2_X8,
/*	SYS_TYPE_VS_1008,*/
}SYS_TYPE_E;

typedef enum chan_mod_power_state_e{
	CHAN_MOD_POWER_OFF = 0,//模块处于关机状态
	CHAN_MOD_POWER_ON,     //模块处于开机状态
	CHAN_MOD_IS_POWER_ON,  //模块正在开机
	CHAN_MOD_IS_POWER_OFF, //模块正在关机
	CHAN_MOD_POWER_RESET,  //重启模块
	CHAN_MOD_POWER_UNKOWN,
}CHAN_MOD_POWER_STATE_E;

/**************************************************/
/*                  bsp api                       */
/**************************************************/
/*
desc      : api init, connect to soap server
param in  : host -- soap server ip address
            port -- soap server port
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int bsp_api_init(char *host, unsigned short port);

/*
desc      : api deinit, disconnect from soap server
param in  : 
param out : 
return    : 
history   : create by zhongwei.peng/2017.11.24
*/
void bsp_api_deinit(void);
/**************************************************/
/*                  bsp api(end)                  */
/**************************************************/

/**************************************************/
/*                  simcard                       */
/**************************************************/
/*
desc      : enable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_enable(int chn);

/*
desc      : disable simcard by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_disable(int chn);

/*
desc      : get simcard state(disable/enable) by channel
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- 1 means enable, 0 means disable
                      single channel use states[0], 
                      multi channel use states[chn]
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_enable_state_get(int chn, unsigned char *states);

/*
desc      : get simcard state(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- 0 means insert, 1 means remove
                      single channel use states[0], 
                      multi channel use states[chn]
return    : ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_insert_state_get(int chn, unsigned char *states);

/*
desc      : get simcard event(insert/remove) by channel
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : events -- 0 means no event, 1 means insert, 2 means remove
                      single channel use events[0], 
                      multi channel use events[chn]
return    : ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int simcard_insert_event_get(int chn, unsigned char *events);


/**************************************************/
/*                  simcard(end)                  */
/**************************************************/


/**************************************************/
/*                  module                        */
/**************************************************/
/*
desc      : turn on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_turn_on(int chn);

/*
desc      : turn off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_turn_off(int chn);


/*
desc      : reset specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by wengang.mu/2018.10.26
*/
int module_reset(int chn);


/*
desc      : get module state(turn on/turn off)
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- only use in multi chn mode(chn == -1)
                      states[chn] == 1 means on, 0 means off
return    : single chn: on return 1, off return 0, or return error num(<0)
            multi  chn: ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int module_turn_on_state_get(int chn, unsigned char *states);

/*
desc      : power on specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_power_on(int chn);

/*
desc      : power off specified module by channel
param in  : chn -- channel, if (chn == -1), it means all channels
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int module_power_off(int chn);

/*
desc      : get module state(power on/power off)
param in  : chn -- channel, if (chn == -1), it means all channels(multi channels mode)
param out : states -- only use in multi chn mode(chn == -1)
                      states[chn] == 1 means on, 0 means off
return    : single chn: on return 1, off return 0, or return error num(<0)
            multi  chn: ok return 0, or return error num(<0)
history   : create by zhongwei.peng/2017.11.24
*/
int module_power_state_get(int chn, unsigned char *states);

/*
desc      : get module num
param in  : 
param out : 
return    : success return module num, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_num_get(void);
/*
desc      : get module uid
param in  : idx--module index
param out : uidbuf--return uid
                 len--uidbuf length
return    : success 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_uid_get(int idx, char *uidbuf, int len);

/*
desc      : get module reset key status
param in  : idx--module index
param out : status--0:means reset key press, 1:means reset key not press
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_reset_status_get(int index, int *stauts );

/*
desc      : set powerkey level
param in  : chn--channel num
            press --0:means low level, 1:means hign level
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int module_powerkey_hign_low(int chn , int level, char *id);


/*
desc      : set debug switch
param in  : index -- module brd index
            enable --0:means close debug uart , 1:means open debug uart
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.06.04
*/
int module_debug_uart_set(int idx , int enable );
/**************************************************/
/**************************************************/
/*                  module(end)                   */
/**************************************************/

/**************************************************/
/*                  board mcu                     */
/**************************************************/
/*
desc      : get board mcu version info
param in  : 
param out : ver_info
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_version(unsigned char *ver_info);

/*
desc      : get board mcu version info
param in  : idx -- mcu number
param out : ver_info -- version information buffer
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_int_version(unsigned int idx, unsigned char *ver_info);

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            num -- reg num to read
param out : values -- reg value in string
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_reg_read(int brd, int reg, int num, unsigned char *values);

/*
desc      : read board mcu reg
param in  : brd -- board num(0,1)
            reg -- reg addr
            value -- value to write
param out : 
return    : ok return 0, or return error num
history   : create by zhongwei.peng/2017.11.24
*/
int brdmcu_reg_write(int brd, int reg, unsigned char value);


/**************************************************/
/*                 board mcu(end)                 */
/**************************************************/
/**************************************************/
/*                  board info(begin)               */
/**************************************************/
/*
desc      : get board channel number
param in  : 
param out : 
return    : channel number
history   : create by zhongwei.peng/2017.12.20
*/
unsigned char brdinfo_chn_num_get(void);

/*
desc      : get board name and version
param in  : 
param out : version--name and version
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/

int brdinfo_version(char *version);
/**************************************************/
/*                  board info(end)               */
/**************************************************/

/**************************************************/
/*                  upgrade info(begin)               */
/**************************************************/
/*
desc      : select upgrade channel
param in  : chn-channels, if(chn == 0xFFFF), it means all channels unselect
param out :
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/

int upgrade_chan_select(int chn);
/*
desc      : select upgrade channel
param in  : chn-channels,
param out : status--1:chn is upgrade channel, 0:chn is not upgrade channel, 2:channel is not support upgrade
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.3.15
*/
int upgrade_chan_status(int chn, int *status);


int chan_upgrade_status_set(int chn, int flag, char *id, int *status);
/**************************************************/
/*                  upgrade info(end)               */
/**************************************************/

int bsp_server_debug_level(int value);

/*
desc      : set signal led or work led status
param in  : chan -- channel
            status -- led status
                      CHAN_LAMP_STATUS_OFF(0):         set sig led off
                      CHAN_LAMP_STATUS_RED_ON(1):      set sig led red on
                      CHAN_LAMP_STATUS_RED_FLASH(2):   set sig led red flash
                      CHAN_LAMP_STATUS_GREEN_ON(3):       set sig led green
                      CHAN_LAMP_STATUS_GREEN_FLASH(4): set sig led green flash
                      CHAN_LAMP_STATUS_YELLOW_ON(5):       set sig led yellow
                      CHAN_LAMP_STATUS_YELLOW_FLASH(6): set sig led yellow flash
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.8.2
*/
int chan_led_set_status_sig(int chan, int status);

/*
desc      : set signal led or work led status
param in  : chan -- channel
            status -- led status
                      CHAN_LAMP_STATUS_OFF(0):         set work led off
                      CHAN_LAMP_STATUS_GREEN(3):       set work led green
                      CHAN_LAMP_STATUS_GREEN_FLASH(4): set work led green flash
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.8.2
*/
int chan_led_set_status_work(int chan, int status);

/*
desc      : set signal led or work led status
param in  : status -- led status, 0--turn off led
                                  1 -turn on led
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.8.2
*/
int chan_led_set_all(int status);

/*
desc      : set mod brd system led status
param in  : status -- led status, 0--turn off led
                                  1 -turn on led
            index  -- mod brd index
return    : success return 0, or error return(<0)
history   : create by wengang.mu/2018.9.22
*/
int mod_brd_led_set_sys(int index,int status);

/*
desc      : get system type
return    : system type
			0 --SYS_TYPE_UNKOWN
			1 --SYS_TYPE_SWG20XX
			2 --SYS_TYPE_1CHAN4SIMS
			3 --SYS_TYPE_VS_USB
history   : create by wengang.mu/2018.8.3
*/

SYS_TYPE_E bsp_get_sys_type();

#endif

