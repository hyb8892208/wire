

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <czmq_library.h>
#include "public.h"
#include "bsp_api.h"
#include "int_api.h"
#include "cli.h"
	
typedef enum chan_mod_led_name_e{
	CHAN_MOD_LED_SIG = 0,
	CHAN_MOD_LED_WORK,
	CHAN_MOD_LED_NUM,
};

/* 单板通道个数与bsp_server的g_brd_chn_num一致 */
unsigned int g_board_chn_num = 32;
#define BSP_PRINT(format, ...) do{\
	printf(format, ##__VA_ARGS__); \
	}while(0)
	
#define BSP_PRINT_ERROR(format, ...) do{ \
	zsys_error("[%s:%s:%d]"format, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
	}while(0)
	
#define BSP_PRINT_WARNING(format, ...) do{\
	zsys_warning("[%s:%s:%d]"format, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
	}while(0)
	
#define BSP_PRINT_NOTICE(format, ...) do{\
	zsys_notice("[%s:%s:%d]"format, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
	}while(0)
	
#define BSP_PRINT_INFO(format, ...) do{\
	zsys_info("[%s:%s:%d]"format, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
	}while(0)
	
#define BSP_PRINT_DEBUG(format, ...) do{\
	zsys_debug("[%s:%s:%d]"format, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
	while(0)
	
#if T_DESC("tools")
/**********************************************************
函数描述 : 16进制字符转数值
输入参数 : c -- 字符，不作参数检查
输出参数 : 
返 回 值 : 数值，
作   者  : zhongwei.peng
时   间  : 2016.11.25
************************************************************/
unsigned char char_to_val(char c)
{
    if ( (c >= '0') && (c <= '9') )
        return c - '0';
    else if ( (c >= 'A') && (c <= 'Z') )
        return c - 'A' + 10;
    else if ( (c >= 'a') && (c <= 'z') )
        return c - 'a' + 10;
    else 
        return 0;
}

/**********************************************************
函数描述 : 字符串转数值
输入参数 : str -- 字符串
输出参数 : 
返 回 值 : 数值，
作   者  : zhongwei.peng
时   间  : 2016.11.25
************************************************************/
int str_to_int(char *str)
{
    int flag;
    int value = 0;
    char *p = str;

    if ( NULL == str )
        return 0;

    /* 16进制 */
    if ( (str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')) )
    {
        value = 0;
        p = &str[2];
        while ( *p )
        {
            value = value * 16 + char_to_val(*p);
            p++;
        }
        return value;
    }

    /* 10进制 */
    if ( str[0] == '-' )
        flag = -1;
    else
        flag = 1;

    value = 0;
    p = str;
    while ( *p ) 
        value = value * 10 + char_to_val(*p++);

    value = value * flag;

    return value;
}

/**********************************************************
函数描述 : 打印一段数据
输入参数 : 
输出参数 : 
返 回 值 : 
作   者  : zhongwei.peng
时   间  : 2016.11.07
************************************************************/
void dump_data(char *title, unsigned char *data, unsigned int len)
{
    unsigned int i;

    BSP_PRINT("%s\r\n", title);
    for ( i = 0; i < len; i++ )
    {
        if ( (i != 0) && (i % 4 == 0) )
        {
            if ( i % 8 == 0 )
                BSP_PRINT("\r\n");
            else
                BSP_PRINT("   ");
        }

        BSP_PRINT("%02x ", data[i] & 0xFF);
    }

    BSP_PRINT("\r\n");
}

#endif

#if T_DESC("sim card")

/*
    命令行使用方法说明
*/
void cli_usage_sim(void)
{
    BSP_PRINT("sim [enable|disable] [chn] -- set sim card enable|disable\n");
    BSP_PRINT("sim_state enable [chn] -- get sim card enable state\n");
    BSP_PRINT("sim_state insert1t4 [chn] [card]-- get sim card insert1t4 state. If card is set, will get card state\n");
    BSP_PRINT("sim_state insert [chn] -- get sim card insert state\n");
    BSP_PRINT("sim event [chn] -- get sim card insert|remove event\n");
    BSP_PRINT("sim sel [chn] [slot] -- set sim solt\n");
}

/*
    sim card 使能相关
*/
int cmd_sim(int argc, char **argv)
{
    int ret;
    int chn;

    if ( (argc == 3) && (strcmp(argv[1], "enable") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = simcard_enable(chn)) == RET_OK )
            BSP_PRINT("sim enable chn[%d] ok\r\n", chn);
        else
            BSP_PRINT("sim enable chn[%d] fail: ret = %d\r\n", chn, ret);
    }
    else if ( (argc == 3) && (strcmp(argv[1], "disable") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = simcard_disable(chn)) == RET_OK )
            BSP_PRINT("sim disable chn[%d] ok\r\n", chn);
        else
            BSP_PRINT("sim disable chn[%d] fail: ret = %d\r\n", chn, ret);
    }
    else
    {
        cli_usage_sim();
    }

    return 0;
}

void print_all_sim_state(int chan,int state[][4]){
    int i = 0, j = 0;  
    BSP_PRINT("get sim card insert1t4 ok. \n");
	if((chan & 0xFFFF) != 0xFFFF ){
		BSP_PRINT("chan[%d] sim card state:",chan);
		for(j = 0; j < 4; j++){
			BSP_PRINT("sim[%d](%s),", j+1, state[0][j] == 0 ? "INSERT":"REMOVE");
		}
		BSP_PRINT("\n");
	}
	else{
		for(i = 0; i < g_board_chn_num; i++){
			BSP_PRINT("chn[%d] sim card state:", i+1); 
			for(j = 0; j < 4; j++){
				BSP_PRINT("sim[%d](%s),", j+1 , state[i][j] == 0 ? "INSERT":"REMOVE");
			}   
			BSP_PRINT("\n");
		} 
	}
}
void print_one_sim_state(int chan,int card,int *state){
    int i = 0; 
    BSP_PRINT("get sim card insert1t4 ok. \n");
	if((chan & 0xFFFF) != 0xFFFF ){
		BSP_PRINT("sim card in  %s  state\n",  state[0] == 0 ? "INSERT":"REMOVE");
	}
	else{
		for(i = 0; i < g_board_chn_num; i++){
			BSP_PRINT("chn[%d] sim card[%d] in %s state.\n", i+1, card,state[i] == 0 ? "INSERT":"REMOVE"); 
		} 
	}
}
/*
    sim card 状态
*/
int cmd_sim_state(int argc, char **argv)
{
    int ret;
    int chn;
    unsigned char states[128] = {0};

    if ( (argc == 3) && (strcmp(argv[1], "enable") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = simcard_enable_state_get(chn, states)) == RET_OK )
        {
            if ( chn == -1 )
                dump_data("enable states:", states, g_board_chn_num);
            else
                BSP_PRINT("sim card in  %s  state\r\n", states[0] ? "ENABLE":"DISABLE");
        }
        else
        {
            BSP_PRINT("chn[%d] get sim enable state fail: ret = %d\r\n", chn, ret);
        }
    }
    else if ( (argc == 3) && (strcmp(argv[1], "insert") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = simcard_insert_state_get(chn, states)) == RET_OK )
        {
            if ( chn == -1 )
                dump_data("insert states:", states, g_board_chn_num);
            else
                BSP_PRINT("sim card in  %s  state\r\n", states[0] ? "REMOVE":"INSERTED");
        }
        else
        {
            BSP_PRINT("chn[%d] get sim insert state fail: ret = %d\r\n", chn, ret);
        }
    }
    else if(argc == 3 && strcmp(argv[1], "insert1t4") == 0){
		chn = str_to_int(argv[2]);
		int status[32][4];
		if((ret = get_chan_all_sim_state(chn, status) )== RET_OK) {
			print_all_sim_state(chn,status);
		}else{
			BSP_PRINT("get sim state insert1t4 failed .\n"); 
		}
    }else if(argc == 4 && strcmp(argv[1], "insert1t4") == 0){
    		int status[64] = {0};
		chn = str_to_int(argv[2]);
		int card = str_to_int(argv[3]);
		if((ret = get_chan_one_sim_state(chn, card,status) )== RET_OK) {
			print_one_sim_state(chn, card, status);
		}else{
			BSP_PRINT("get sim state insert1t4 failed .\n"); 
		}
	}
    else
    {
        cli_usage_sim();
    }

    return 0;
}

/*
    sim card 插拔事件
*/
int cmd_sim_event(int argc, char **argv)
{
    int ret;
    int chn;
    unsigned char states[128] = {0};

    if ( (argc == 3) && (strcmp(argv[1], "event") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = simcard_insert_event_get(chn, states)) == RET_OK )
        {
            if ( chn == -1 )
            {
                dump_data("insert events:", states, g_board_chn_num);
            }
            else
            {
                if ( states[0] == 1 )
                    BSP_PRINT("chn[%d]  INSERT  event occur\r\n", chn);
                else if ( states[0] == 2 )
                    BSP_PRINT("chn[%d]  REMOVE  event occur\r\n", chn);
                else
                    BSP_PRINT("chn[%d]  NO  event occur\r\n", chn);
            }
        }
        else
        {
            BSP_PRINT("chn[%d] get sim event fail: ret = %d\r\n", chn, ret);
        }
    }
    else
    {
        cli_usage_sim();
    }

    return 0;
}

int cmd_sim_sel(int argc, char **argv){
	if(argc < 4){
		BSP_PRINT("usage:sim sel [chn] [card]");
		return -1;
	}
	int res = select_simcard_from_chan(atoi(argv[2]), atoi(argv[3]));
	if(res < 0)
		BSP_PRINT("sel sim[%d] card[%d] failed!\n", atoi(argv[2]), atoi(argv[3]));
	else
		BSP_PRINT("sel sim[%d] card[%d] success!\n", atoi(argv[2]), atoi(argv[3]));
	return 0;
}

int cmd_sim_version(int argc, char **argv)
{
	int idx = 0, ret = -1;
	char buf[128] = {0};
	if ( (argc == 3) && (strcmp(argv[1], "ver") == 0) )
	{
		idx = str_to_int(argv[2]);

		if ( (ret = get_sim_version(idx, buf)) == RET_OK )
			BSP_PRINT("[sim %d]:\n%s\n", idx, buf);
		else
			BSP_PRINT("[sim %d]:\nget ver info fail: ret = %d\n", idx, ret);
	}

	return 0;
}

/*
    注册命令行 -- simcard 相关
*/
void cli_reg_simcard(void)
{
    cb_func_reg("sim", cmd_sim);
    cb_func_reg("sim_state", cmd_sim_state);
    cb_func_reg("sim event", cmd_sim_event);
    cb_func_reg("sim sel", cmd_sim_sel);
    cb_func_reg("sim ver", cmd_sim_version);
}

#endif

#if T_DESC("tel module")

/*
    命令行使用方法说明
*/
void cli_usage_module(void)
{
    BSP_PRINT("module num --get module number\n");
    BSP_PRINT("module uid [index] --get module uid\n");
    BSP_PRINT("module reset_key [index] --get reset key status\n");
    BSP_PRINT("module reset [chn] --reset module\n");
    BSP_PRINT("module turn [on|off] [chn] -- module turn on|off\n");
    BSP_PRINT("module_state turn [chn] -- get module turn state\n");
    BSP_PRINT("module power [on|off] [chn] -- module power on|off\n");
    BSP_PRINT("module_state power [chn] -- get module power state\n");
    BSP_PRINT("module powerkey [chn] [level]-- set powerkey level\n");
    BSP_PRINT("module uart [index] [value]-- set uart switch\n");
    BSP_PRINT("module timer on [chn] [timer]-- module turn off after (timer) seconds\n");
}

/*
    通讯模块相关
*/
int cmd_module(int argc, char **argv)
{
    int ret;
    int chn;
    char buf[64];
    int status;
    if ( (argc == 4) && (strcmp(argv[1], "turn") == 0) && (strcmp(argv[2], "on") == 0) )
    {
        chn = str_to_int(argv[3]);

        if ( (ret = module_turn_on(chn)) == RET_OK )
            BSP_PRINT("module chn[%d] trun on ok\r\n", chn);
        else
            BSP_PRINT("module chn[%d] trun on fail: ret = %d\r\n", chn, ret);
    }
    else if ( (argc == 4) && (strcmp(argv[1], "turn") == 0) && (strcmp(argv[2], "off") == 0) )
    {
        chn = str_to_int(argv[3]);

        if ( (ret = module_turn_off(chn)) == RET_OK )
            BSP_PRINT("module chn[%d] trun off ok\r\n", chn);
        else
            BSP_PRINT("module chn[%d] trun off fail: ret = %d\r\n", chn, ret);
    }
	else if((argc == 5) &&  (strcmp(argv[1], "timer") == 0) && (strcmp(argv[2], "on") == 0 )){
		chn = str_to_int(argv[3]);
		if((ret = module_turn_on_timer(chn, str_to_int(argv[4])))== RET_OK){
			BSP_PRINT("module chn[%d] timer on ok\r\n", chn);
		}else
			BSP_PRINT("module chn[%d] timer on failed\r\n", chn);
	}
	else if ( (argc == 3) && (strcmp(argv[1], "reset") == 0))
    {
        chn = str_to_int(argv[2]);

        if ( (ret = module_reset(chn)) == RET_OK )
            BSP_PRINT("module chn[%d] reset ok\r\n", chn);
        else
            BSP_PRINT("module chn[%d] reset fail: ret = %d\r\n", chn, ret);
    }
    else if ( (argc == 4) && (strcmp(argv[1], "power") == 0) && (strcmp(argv[2], "on") == 0) )
    {
        chn = str_to_int(argv[3]);

        if ( (ret = module_power_on(chn)) == RET_OK )
            BSP_PRINT("module chn[%d] power on ok\r\n", chn);
        else
            BSP_PRINT("module chn[%d] power on fail: ret = %d\r\n", chn, ret);
    }
    else if ( (argc == 4) && (strcmp(argv[1], "power") == 0) && (strcmp(argv[2], "off") == 0) )
    {
        chn = str_to_int(argv[3]);

        if ( (ret = module_power_off(chn)) == RET_OK )
            BSP_PRINT("module chn[%d] power off ok\r\n", chn);
        else
            BSP_PRINT("module chn[%d] power off fail: ret = %d\r\n", chn, ret);
    }else if((argc == 2) && (strcmp(argv[1], "num") == 0)){
		ret = module_num_get();	
		if(ret >= 0)
			BSP_PRINT("module number get ok, total: %d\r\n",ret);
		else
			BSP_PRINT("module number get failed: ret = %d\r\n", ret);
    }
	else if((argc == 3) && (strcmp(argv[1], "uid") == 0))
    {
    		chn = str_to_int(argv[2]);
		ret = module_uid_get(chn, buf, 64);
		if(ret == RET_OK){
			BSP_PRINT("module uid get ok, uid: %s\r\n", buf);
		}else{
			BSP_PRINT("module uid get failed, ret = %d\r\n", ret);
		}
    }
	else if((argc == 3) && (strcmp(argv[1], "reset_key") == 0))
    {
    	chn = str_to_int(argv[2]);
		ret = module_reset_status_get(chn, &status);
		if(ret == RET_OK){
			if(status == 0)
				BSP_PRINT("module reset key is not press!\r\n");
			else
				BSP_PRINT("module reset key is press!\r\n");
		}else{
			BSP_PRINT("module reset key get failed, ret = %d\r\n", ret);
		}
    }
    else if((argc == 4) && strcmp(argv[1], "powerkey") == 0)
    {
	chn = str_to_int(argv[2]);
	int level = str_to_int(argv[3]);
	if((ret = module_powerkey_hign_low(chn, level, "upgrade")) == RET_OK){
		BSP_PRINT("chn[%d] set %s level success!\n", chn, level ? "hign":"low");
	}else{
		BSP_PRINT("chn[%d] set %s level failed!\n", chn, level ? "hign":"low");
	}
    }else if((argc == 4) && strcmp(argv[1], "uart") == 0){
        int sw =-1;
        if(strcmp(argv[2] , "on") == 0){
            sw = 1;
        }else{
            sw = 0;
        }
       int index = str_to_int(argv[3]);
       ret = module_debug_uart_set(index, sw);
	if( ret == RET_OK){
		BSP_PRINT("mod_brd[%d] set debug uart %s  success!\n", index, sw ? "on":"off");
	}else{
		BSP_PRINT("mod_brd[%d] set debug uart %s failed!\n", index, sw ? "on":"off");
	}
    }
    else
    {
        cli_usage_module();
    }

    return 0;
}

/*
    模块 状态
*/
int cmd_module_state(int argc, char **argv)
{
    int ret;
    int chn;
    unsigned char states[128] = {0};

    if ( (argc == 3) && (strcmp(argv[1], "turn") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = module_turn_on_state_get(chn, states)) == RET_OK )
        {
            if ( chn == -1 )
                dump_data("module turn states:", states, g_board_chn_num);
            else{
                
				if(states[0] == CHAN_MOD_POWER_OFF){
					BSP_PRINT("module[%d] turn state is %s\r\n", chn, "OFF");
				}else if(states[0] == CHAN_MOD_POWER_ON){
					BSP_PRINT("module[%d] turn state is  %s\r\n", chn, "ON");
				}else if(states[0] == CHAN_MOD_IS_POWER_ON){
					BSP_PRINT("module[%d] turn state is turning on\r\n", chn);
				}else if(states[0] == CHAN_MOD_IS_POWER_OFF){
					BSP_PRINT("module[%d] turn state is turning off\r\n", chn);
				}else{
					BSP_PRINT("module[%d] turn state is unkown\r\n", chn);
				}
            }
        }
        else
        {
            BSP_PRINT("chn[%d] get module turn on state fail: ret = %d\r\n", chn, ret);
        }
    }
    else if ( (argc == 3) && (strcmp(argv[1], "power") == 0) )
    {
        chn = str_to_int(argv[2]);

        if ( (ret = module_power_state_get(chn, states)) == RET_OK )
        {
            if ( chn == -1 )
                dump_data("module power states:", states, g_board_chn_num);
            else{
                BSP_PRINT("module[%d] is power  %s\r\n", chn, states[0] ? "ON":"OFF");
            }
		}
        else
        {
            BSP_PRINT("module[%d] get power state fail: ret = %d\r\n", chn, ret);
        }
    }
    else
    {
        cli_usage_module();
    }

    return 0;
}

/*
    注册命令行 -- 模块 相关
*/
void cli_reg_module(void)
{
    cb_func_reg("module", cmd_module);
    cb_func_reg("module num", cmd_module);
    cb_func_reg("module uid", cmd_module);
    cb_func_reg("module reset", cmd_module);
    cb_func_reg("module turn", cmd_module);
    cb_func_reg("module power", cmd_module);
    cb_func_reg("module_state", cmd_module);
    cb_func_reg("module_state turn", cmd_module_state);
    cb_func_reg("module_state power", cmd_module_state);
    cb_func_reg("module powerkey", cmd_module);
    cb_func_reg("module uart", cmd_module);
	cb_func_reg("module timer", cmd_module);
	cb_func_reg("module timer on", cmd_module);
}

#endif

#if T_DESC("bmcu")

/*
    命令行使用方法说明
*/
void cli_usage_bmcu(void)
{
    BSP_PRINT("bmcu info -- show board mcu version\n");
    BSP_PRINT("bmcu ver [idx] -- show board mcu version\n");
    BSP_PRINT("bmcu reg read  [brd] [reg] [num] -- read reg\n");
    BSP_PRINT("bmcu reg write [brd] [reg] [val] -- write reg\n");
    BSP_PRINT("bmcu event restore -- get one key restore event\n");
}

int cmd_bmcu(int argc, char **argv)
{
    int i;
    int ret;
    int idx;
    int reg;
    int num;
    int val;
    char version[64]  = {0};
    unsigned char buf[256] = {0};

    if ( (argc == 2) && (strcmp(argv[1], "ver") == 0) )
    {
        if ( (ret = brdmcu_version(buf)) == RET_OK )
            BSP_PRINT("%s\r\n", buf);
        else
            BSP_PRINT("get ver info fail: ret = %d\r\n", ret);
    }
    else if ( (argc == 3) && (strcmp(argv[1], "ver") == 0) )
    {
        idx = str_to_int(argv[2]);

        if ( (ret = brdmcu_int_version(idx, buf)) == RET_OK )
            BSP_PRINT("[mcu %d]:\n%s\n", idx, buf);
        else
            BSP_PRINT("[mcu %d]:\nget ver info fail: ret = %d\n", idx, ret);
    }
    else if ( (argc == 5) && (strcmp(argv[2], "read") == 0) )
    {
        idx = str_to_int(argv[3]);
        reg = str_to_int(argv[4]);

        if ( (ret = brdmcu_reg_read(idx, reg, 1, buf)) == RET_OK)
            BSP_PRINT("mcu[%d] reg[%d] = 0x%02x\r\n", idx, reg, buf[0]);
        else
            BSP_PRINT("read error: mcu[%d] reg[%d], ret = %d\r\n", idx, reg, ret);
    }
    else if ( (argc == 6) && (strcmp(argv[2], "read") == 0) )
    {
        idx = str_to_int(argv[3]);
        reg = str_to_int(argv[4]);
        num = str_to_int(argv[5]);

        if ( (ret = brdmcu_reg_read(idx, reg, num, buf)) != RET_OK)
        {
            BSP_PRINT("read error: mcu[%d] reg[%d], num[%d], ret = %d\r\n", idx, reg, num, ret);
        }
        else
        {
            for ( i = 0; i < num; i++ )
                BSP_PRINT("mcu[%d] reg[%d] = 0x%02x\r\n", idx, reg+i, buf[i]);
        }
    }
    else if ( (argc == 6) && (strcmp(argv[2], "write") == 0) )
    {
        idx = str_to_int(argv[3]);
        reg = str_to_int(argv[4]);
        val = str_to_int(argv[5]);

        if ( (ret = brdmcu_reg_write(idx, reg, (unsigned char)val)) == RET_OK)
            BSP_PRINT("write ok\r\n");
        else
            BSP_PRINT("write error: mcu[%d] reg[%d], val[%d], ret = %d\r\n", idx, reg, val, ret);
    }else if((argc == 2) && (strcmp(argv[1] , "info") == 0)){
		ret = brdinfo_version(version);
		if(ret == RET_OK){
			//printf("get brdinfor version success:");
			BSP_PRINT("%s", version);
		}else{
			BSP_PRINT("get brdinfor version failed, ret = %d\n", ret);
		}
	}
    else
    {
        cli_usage_bmcu();
    }

    return 0;
}

/*
    注册命令行 -- bmcu 相关
*/
void cli_reg_bmcu(void)
{
    cb_func_reg("bmcu", cmd_bmcu);
    cb_func_reg("bmcu info", cmd_bmcu);
    cb_func_reg("bmcu ver", cmd_bmcu);
    cb_func_reg("bmcu reg read", cmd_bmcu);
    cb_func_reg("bmcu reg write", cmd_bmcu);
}

#endif

void cli_usage_upgrade(void){
	BSP_PRINT("upgrade sel [chn] --select upgrade chn\n");
	BSP_PRINT("upgrade state [chn]  --get upgrade chn status\n");
	BSP_PRINT("upgrade flag [chn]  --set upgrade flag chn\n");
}
int cmd_upgrade(int argc, char **argv){
	int chn, ret;
	int status = 0;
	if((argc == 3) && strcmp(argv[1], "sel") == 0){
		chn = str_to_int(argv[2]);
		ret = upgrade_chan_select(chn);
		if(ret == RET_OK){
			if(chn == 0xFFFF ||chn == -1 ){
				BSP_PRINT("clear upgrade channel success!\n");
			}else if(0 == upgrade_chan_status(chn, &status)){
				if(status == 1 ){
					BSP_PRINT("upgrade select [%d] success!\n", chn);
				}else if(status == 0){
					BSP_PRINT("[%d] is not support upgrade!\n", chn);
				}else{
					BSP_PRINT("upgrade select failed, upgrade status = %d\n", status);
				}
			}else{
				BSP_PRINT("upgrade select failed, upgrade status = %d\n", ret);
			}
			
		}else if(ret == 1){
			BSP_PRINT("[%d] is not support upgrade!\n", chn);
		}else{
			BSP_PRINT("upgrade select failed, ret = %d, line = %d\n", ret, __LINE__);
		}
	}else if((argc == 3) && strcmp(argv[1], "state") == 0){
		chn = str_to_int(argv[2]);
		ret = upgrade_chan_status(chn , &status);
		if(ret == RET_OK){
			if(status == 0){
				BSP_PRINT("[%d] is not upgrade chn!\n", chn);
			}else if(status == 1){
				BSP_PRINT("[%d] is upgrade chn!\n", chn);
			}else{
				BSP_PRINT("[%d] is not support upgrade!\n", chn);
			}
		}else{
			BSP_PRINT("get upgrade state failed, ret = %d\n", ret);
		}
	}else if((argc == 4) && strcmp(argv[1], "flag") == 0){
		chn = str_to_int(argv[2]);
		int flag = str_to_int(argv[3]);
		ret = chan_upgrade_status_set(chn,  flag, "upgrade", &status);
		if(status == 0 && flag == 0){
			BSP_PRINT("unset [%d] upgrade flag success!\n", chn);
		}else if(status == 0 && flag == 1){
			BSP_PRINT("set [%d] upgrade flag success!\n", chn);
		}else{
			BSP_PRINT("set [%d] upgrade flag failed!\n", chn);
		}
	}else{
		cli_usage_upgrade();
	}
	return 0;
}
void cli_reg_upgrade(void)
{
	cb_func_reg("upgrade", cmd_upgrade);
	cb_func_reg("upgrade sel", cmd_upgrade);
	cb_func_reg("upgrade state", cmd_upgrade);
	cb_func_reg("upgrade flag", cmd_upgrade);
}

void cli_usage_led(void){
	printf("led sig off [channel]\n");
	printf("led sig red [channel]\n");
	printf("led sig red_flash [channel]\n");
	printf("led sig green [channel]\n");
	printf("led sig green_flash [channel]\n");
	printf("led sig yellow [channel]\n");
	printf("led sig yellow_flash [channel]\n");
	printf("led work off [channel]\n");
	printf("led work green [channel]\n");
	printf("led work green_flash [channel]\n");
	printf("led all on\n");
	printf("led all off\n");
	printf("led sys on [mod_brd]\n");
	printf("led sys off [mod_brd]\n");
	printf("led sys green_flash [mod_brd]\n");
}
int cmd_led(int argc, char **argv){
	if(argc < 3){
		cli_usage_led();
		return -1;
	}
//	int led_name = 0;
	int status = 0;
	if(strcmp(argv[1], "sig") == 0 && argc == 4){
//		led_name = CHAN_MOD_LED_SIG ;
		int chan = atoi(argv[3]);
		if(strcmp(argv[2], "red") == 0){
			status = CHAN_LAMP_STATUS_RED_ON;
		}else if(strcmp(argv[2], "red_flash") == 0){
			status = CHAN_LAMP_STATUS_RED_FLASH;
		}else if(strcmp(argv[2], "green") == 0){
			status = CHAN_LAMP_STATUS_GREEN_ON;
		}else if(strcmp(argv[2], "green_flash") == 0){
			status = CHAN_LAMP_STATUS_GREEN_FLASH;
		}else if(strcmp(argv[2], "yellow") == 0){
			status = CHNA_LAMP_STATUS_YELLOW_ON;
		}else if(strcmp(argv[2], "yellow_flash") == 0){
			status = CHNA_LAMP_STATUS_YELLOW_FLASH;
		}else if(strcmp(argv[2], "off") == 0){
			status = CHAN_LAMP_STATUS_OFF;
		}else{
			cli_usage_led();
			return -1;
		}
		if(chan_led_set_status_sig(chan, status) < 0){
			printf("set chan[%s] sig led %s failed!\n", argv[3], argv[2]);
		}else{
			printf("set chan[%s] sig led %s success!\n", argv[3], argv[2]);
		}
	}else if(strcmp(argv[1], "work") == 0 && argc == 4){
		int chan = atoi(argv[3]);
//		led_name = CHAN_MOD_LED_WORK;
		if(strcmp(argv[2], "green") == 0){
			status = CHAN_LAMP_STATUS_GREEN_ON;
		}else if(strcmp(argv[2], "green_flash") == 0){
			status = CHAN_LAMP_STATUS_GREEN_FLASH;
		}else if(strcmp(argv[2], "off") == 0){
			status = CHAN_LAMP_STATUS_OFF;
		}else{
			cli_usage_led();
			return -1;
		}
		if(chan_led_set_status_work(chan, status) < 0){
			printf("set chan[%s] work led %s failed!\n", argv[3], argv[2]);
		}else{
			printf("set chan[%s] work led %s success!\n", argv[3], argv[2]);
		}
	}else if(strcmp(argv[1], "all") == 0 && argc == 3){
		if(strcmp(argv[2], "on") == 0){
			status = 1;
		}else if(strcmp(argv[2], "off") == 0){
			status = 0;
		}else{
			cli_usage_led();
			return -1;
		}
		if(chan_led_set_all(status) < 0){
			printf("set all led %s failed!\n", argv[2]);
		}else{
			printf("set all led %s success!\n", argv[2]);
		}
	}else if(strcmp(argv[1], "sys") == 0 && argc == 4){//led sys on 1
		int index = atoi(argv[3]);
		if(strcmp(argv[2], "on") == 0){
			status = 1;
		}else if(strcmp(argv[2], "off") == 0){
			status = 0;
		}else if(strcmp(argv[2], "green_flash") == 0){
			status = 2;
		}
		else{
			cli_usage_led();
			return -1;
		}
		if(mod_brd_led_set_sys(index, status) < 0){
			printf("set sys led %s failed!\n", argv[2]);
		}else{
			printf("set sys led %s success!\n", argv[2]);
		}
	}else{
			cli_usage_led();
			return -1;
	}
	
	
	return 0;
}
void cli_reg_led(void){
	cb_func_reg("led", cmd_led);
	cb_func_reg("led sig green", cmd_led);
	cb_func_reg("led sig green_flash", cmd_led);
	cb_func_reg("led sig red", cmd_led);
	cb_func_reg("led sig red_flash", cmd_led);
	cb_func_reg("led sig yellow", cmd_led);
	cb_func_reg("led sig yellow_flash", cmd_led);
	cb_func_reg("led sig off", cmd_led);
	cb_func_reg("led work off", cmd_led);
	cb_func_reg("led work green", cmd_led);
	cb_func_reg("led work green_flash", cmd_led);
	cb_func_reg("led all off", cmd_led);
	cb_func_reg("led all on", cmd_led);
	cb_func_reg("led sys on", cmd_led);
	cb_func_reg("led sys off", cmd_led);
	cb_func_reg("led sys green_flash", cmd_led);
}

int cmd_sys(int argc, char **argv){
	SYS_TYPE_E systype;
	systype = bsp_get_sys_type();
//	if(systype < 0){
//		printf("get sys_type failed!\n");
//		return -1;
//	}
	switch (systype){
		case SYS_TYPE_UNKOWN:
			printf("Unknown systype\n");
			break;
		case SYS_TYPE_SWG20XX:
			printf("systype is swg20xx\n");
			break;
		case SYS_TYPE_1CHAN4SIMS:
			printf("systype is 1chan4sims\n");
			break;
		case SYS_TYPE_VS_USB:
			printf("systype is vs_usb\n");
			break;
		case SYS_TYPE_VS2_X8:
			printf("systype is vs2_8x\n");
			break;
		default:
			printf("get sys_type failed!\n");
			break;
	}	
	return 0;
}

void cli_reg_sys(){
	cb_func_reg("sys type", cmd_sys);
}

void cli_usage_sys(void){
	printf("sys type -- get systype");
}
void cli_usage_server_debug(void){
	printf("server_debug [error|warning|info|debug|notice] \n");
}

int cmd_server_debug(int argc, char **argv){
	int debug_level = 0;
	if(argc < 2){
		cli_usage_server_debug();
		return -1;
	}
	if(strstr(argv[1], "none")){
		debug_level = 0;
	}
	if(strstr(argv[1], "error")){
		debug_level |= 1 << 0;
	}
	if(strstr(argv[1], "warning")){
		debug_level |= 1 << 1;
	}
	if(strstr(argv[1], "info")){
		debug_level |= 1 << 2;
	}
	if(strstr(argv[1], "debug")){
		debug_level |= 1 << 3;
	}
	if(strstr(argv[1], "notice")){
		debug_level |= 1<<4 | 1<<5 | 1<<6; 
	}
#if 0 
	if(strstr(argv[1], "track")){
		debug_level |= 1 << 4;
	}
	if(strstr(argv[1], "verbose")){
		debug_level |= 1 << 5;
	}
	if(strstr(argv[1], "test")){
		debug_level |= 1 << 6;
	}
#endif	
	int res = bsp_server_debug_level(debug_level);
	if(res < 0){
		printf("set bsp_server debug level error\n");
	}else{
		printf("set bsp_server debug level success, level = %x!\n", debug_level);
	}
	return 0;
}
void cli_reg_server_debug(void){
	cb_func_reg("server_debug", cmd_server_debug);
}
/*
    命令行使用方法说明
*/
int cli_usage_main(int argc, char **argv)
{
    BSP_PRINT("[?|help] -- show this menu\n");
    BSP_PRINT("sim -- sim card\n");
    BSP_PRINT("module -- tel module\n");
    BSP_PRINT("bmcu -- board mcu\n");
    BSP_PRINT("upgrade -- upgrade channel\n");
    BSP_PRINT("server_debug -- set server_debug level\n");
	BSP_PRINT("led -- set channel led statues\n");
	BSP_PRINT("sys -- get sys infomations\n");
    return 0;
}
void cli_test_one_usage(int argc, char **argv){
	BSP_PRINT("%s [option] ...\n\n", argv[0]);
	BSP_PRINT("option: sim, sim_state, module_statue, module, bmcu, upgrade, led, server_debug\n\n");
	cli_usage_sim();
	BSP_PRINT("\n");
	cli_usage_module();
	BSP_PRINT("\n");
	cli_usage_bmcu();
	BSP_PRINT("\n");
	cli_usage_upgrade();
	BSP_PRINT("\n");
	cli_usage_server_debug();
	BSP_PRINT("\n");
	cli_usage_led();
	BSP_PRINT("\n");
	cli_usage_sys();
	BSP_PRINT("\n");
}
int cli_test_one(int argc, char **argv){
	if(strcmp(argv[1], "sim_state")==0){
		cmd_sim_state(argc -1, &argv[1]);
	}else if(strcmp(argv[1], "sim") == 0){
		if(argc < 3){
			cli_test_one_usage(argc, argv);
			return -1;
		}
		if(strcmp(argv[2], "event") == 0){
			cmd_sim_event(argc - 1 , &argv[1]);
		}else if(strcmp(argv[2], "sel") == 0){
			cmd_sim_sel(argc - 1 , &argv[1]);
		}else if(strcmp(argv[2], "ver") == 0){
			cmd_sim_version(argc-1, &argv[1]);
		}else{
			cmd_sim(argc - 1, &argv[1]);
		}
		
	}else if(strcmp(argv[1], "module_state") == 0){
		cmd_module_state(argc - 1, &argv[1]);
	}else if(strcmp(argv[1], "module") == 0){
		cmd_module(argc - 1, argv + 1);
	}else if(strcmp(argv[1], "bmcu") == 0){
		cmd_bmcu(argc - 1, &argv[1]);
	}else if(strcmp(argv[1], "upgrade") == 0){
		cmd_upgrade(argc - 1, &argv[1]);
	}else if(strcmp(argv[1], "server_debug") == 0){
		cmd_server_debug(argc - 1, argv + 1);
	}else if(strcmp(argv[1], "led") == 0){
		cmd_led(argc - 1, argv + 1);
	}else{
		cli_test_one_usage(argc, argv);
	}
	return 0;
}
int main(int argc, char **argv)
{
	if ( bsp_api_init(NULL, 0) != 0 )
	{
	    BSP_PRINT_ERROR("bsp api init fail!\r\n");
	    return -1;
	}
	if(argc == 1){
		
	    g_board_chn_num = brdinfo_chn_num_get();
	    cli_init();
	    cb_func_reg("?", cli_usage_main);
	    cb_func_reg("help", cli_usage_main);
	    cli_reg_bmcu();
	    cli_reg_simcard();
	    cli_reg_module();
	     cli_reg_upgrade();
	    cli_reg_server_debug();
		cli_reg_led();
		cli_reg_sys();
	    run_main((char *)"bsp>");
    	}
	else{
		bsp_api_init(NULL, 0);
		cli_test_one(argc, argv);
	}

	bsp_api_deinit();
    return 0;
}














