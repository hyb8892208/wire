#if SWG_TEST

/***************************************************
  文件描述  : 测试SWG设备对提供的硬件功能接口(swg_intf.h).
              测试函数如下：
              swg_test_get_info();
              swg_reg_rw_test();
              swg_sim_card_enable_test();
              swg_chan_mod_vbat_test();
              swg_chan_mod_power_test();
              swg_select_upgrade_chan_test();
*****************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "swg_intf.h"

/* SWG设备通道号等于SWG_ALL_CHANS表示SWG所有通道 */
#define SWG_ALL_CHANS          (0xFFFF)    

#define MAX_CHAN_NUM        128
#define MB_REG_NUM          15

enum print_color_e{
    DFT = 0,    /* default, 默认不作修改 */
    RED,        /* 红色字体 */ 
};

#define TST_PRT(color, fmt, args...)                                                \
    do                                                                              \
    {                                                                               \
        if(RED == color)                                                            \
            ((void)(printf("\033[31m" "SWG_TEST : " fmt "\033[0m", ## args)));      \
        else /* DFT */                                                              \
            ((void)(printf("SWG_TEST : " fmt , ## args)));                          \
    }while(0)

static int total_mb_num = 0;    /* totoal module board number */
static int total_ch_num = 0;    /* totoal channel number */

/*********************************************************/


#define GET_VERSION_MAJOR(VERSION)          (((VERSION)>> 16) & 0xFF)
#define GET_VERSION_MINOR(VERSION)          (((VERSION)>> 8) & 0xFF)
#define GET_VERSION_BUGFIX(VERSION)         (((VERSION)) & 0xFF)
typedef enum swg_name_e
{
    SWG_DEV_UNKOWN = 0,
    SWG_DEV_1001,       /* 1口无线网关 */
    SWG_DEV_1002,       /* 2口无线网关 */
    SWG_DEV_1004,       /* 4口无线网关 */
    SWG_DEV_1008,       /* 8口无线网关 */
    SWG_DEV_1016,       /* 16口无线网关 */ 
    SWG_DEV_1032,       /* 32口无线网关 */ 
    SWG_DEV_1064,       /* 64口无线网关 */ 
    SWG_DEV_VS_USB_1020,/* 20口可插拔USB模块无线网关 1U机箱*/
    SWG_DEV_VS_USB_1044,/* 44口可插拔USB模块无线网关 2U机箱*/
}SWG_NAME_E;

static inline char *swg_name_to_str(int name)
{
    switch (name)
    {
        case SWG_DEV_UNKOWN:
            return "SWG_DEV_UNKOWN";
            break;

        case SWG_DEV_1001:
            return "SWG_DEV_1001";
            break;

        case SWG_DEV_1002:
            return "SWG_DEV_1002";
            break;
            
        case SWG_DEV_1004:
            return "SWG_DEV_1004";
            break;

        case SWG_DEV_1008:
            return "SWG_DEV_1008";
            break;

        case SWG_DEV_1016:
            return "SWG_DEV_1016";
            break;
        case SWG_DEV_1032:
            return "SWG_DEV_1032";
            break;
        case SWG_DEV_1064:
            return "SWG_DEV_1064";
            break;
        case SWG_DEV_VS_USB_1020:
            return "SWG_DEV_VS_USB_1020";
            break;
        case SWG_DEV_VS_USB_1044:
            return "SWG_DEV_VS_USB_1044";
            break;

        default:
            return "SWG_DEV_UNKOWN";
            break;
    }
}

typedef enum mod_brd_name_e
{
    MB_NAME_UNKOWN = 0,
    MB_SWG_1004_BASE,   /* 4口模块板, 目前无此硬件 */
    MB_SWG_1008_BASE,   /* 8口模块板, 目前无此硬件 */
    MB_SWG_1016_BASE,   /* 16口模块板,实际上目前16口与32口一样, 都是MB_SWG_1032_BASE, 此值预留 */
    MB_SWG_1032_BASE,   /* 32口模块板，虽然命令为1032,但硬件上只有16口，32口由2个此模块叠加而得，目前已有硬件 */ 
    MB_SWG_VS_USB_M35,  /* 可插拔4口USB模块, 目前已有硬件 */
}MOD_BRD_NAME_E;
/* 模块板枚举类型转字符串名称 */
static inline char *mb_name_to_str(int name)
{
    switch (name)
    {
        case MB_NAME_UNKOWN:
            return "HW_NAME_UNKOWN";
            break;

        case MB_SWG_1004_BASE:
            return "SWG_1004_BASE";
            break;

        case MB_SWG_1008_BASE:
            return "SWG_1008_BASE";
            break;
            
        /*
         *[>目前16口与32口一样, 都是HW_SWG_1032_BASE <]
         *case MB_SWG_1016_BASE:
         *    return "SWG_1016_BASE";
         *    break;
         */
        case MB_SWG_1032_BASE:
            return "SWG_1032_BASE";
            break;

        case MB_SWG_VS_USB_M35:
            return "VS_USB_M35";
            break;

        default:
            return "NAME_UNKOWN";
            break;
    }
}

/*************************************************
  函数描述 : 测试获取SWG设备信息接口。获取信息包括
             SWG设备名、版本号、模块板总数、通道总数, 以及
             每块模块板名称、软件本版、硬件版本、UID.
  输入参数 : 无
  函数返回 :  0 -- 成功
             <0 -- 失败，其绝对值为失败次数。
*************************************************/
static int swg_test_get_info(void)
{
    int err, i, res;
    char uid_buf[128];
    struct swg_ver_info swg_ver_info;
    struct mod_brd_ver_info mb_ver_info;
    
    err = 0;

    if((swg_get_dev_ver_info(&swg_ver_info)) < 0)
    {
        TST_PRT(RED, "Get device device version information FAILED\n");
        err--;
    }
    if(swg_ver_info.version <= 0)
        err--;

    if((total_ch_num = swg_get_total_chan_num()) < 0)
    {
        TST_PRT(RED, "Get device total channel number FAILED\n");
        err--;
    }

    if((total_mb_num = swg_get_total_mod_brd_num()) < 0)
    {
        TST_PRT(RED, "Get device total module board number FAILED\n");
        err--;
    }

    TST_PRT(DFT, "===== SWG device information ====\n");

    TST_PRT(DFT, "SWG name   : %s(%d)\n", swg_name_to_str(swg_ver_info.name), swg_ver_info.name);
    TST_PRT(DFT, "version: %ld.%ld.%ld\n",
                           GET_VERSION_MAJOR(swg_ver_info.version),
                           GET_VERSION_MINOR(swg_ver_info.version),
                           GET_VERSION_BUGFIX(swg_ver_info.version));
    TST_PRT(DFT, "total moudle board: %d\n", total_mb_num);
    TST_PRT(DFT, "total channel: %d\n", total_ch_num);

    for(i = 1; i < total_mb_num + 1; i++) /* modulee board 从1开始编号 */
    {
        if((res = swg_get_mod_brd_ver_info(i, &mb_ver_info)) < 0) /* 小于0可能是此模块未插入 */
        {
            /* 默认所有模块都插上 */
            TST_PRT(RED, "Module board %d get version information INVALID\n", i);
            err--;
            continue;
        }

        if(mb_ver_info.hw_ver <= 0
                || mb_ver_info.sw_ver <= 0)
        {
            TST_PRT(RED, "Module board %d version INVALID\n", i);
            err--;
        }

        memset(uid_buf, 0, sizeof(uid_buf));
        if((res = swg_get_mod_brd_uid(i, uid_buf, sizeof(uid_buf))) < 0)
        {
            TST_PRT(RED, "Get module board %d UID FAILED\n", i);
            err--;
        }

        TST_PRT(DFT, "Module board %d\n"
                     "\tname : %s(%d)\n"
                     "\thw ver : %ld.%ld.%ld\n"
                     "\tsw_ver : %ld.%ld.%ld\n"
                     "\t%s\n",
                     i,
                     mb_name_to_str(mb_ver_info.name), mb_ver_info.name,
                     GET_VERSION_MAJOR(mb_ver_info.hw_ver),
                     GET_VERSION_MINOR(mb_ver_info.hw_ver),
                     GET_VERSION_BUGFIX(mb_ver_info.hw_ver),

                     GET_VERSION_MAJOR(mb_ver_info.sw_ver),
                     GET_VERSION_MINOR(mb_ver_info.sw_ver),
                     GET_VERSION_BUGFIX(mb_ver_info.sw_ver),

                     uid_buf);
    }

    if(0 == err)
        TST_PRT(DFT, "Get device information(name, version, uid) testing : PASSED\n");
    else
        TST_PRT(RED, "Get device information(name, version, uid) testing : FAILED\n");

    TST_PRT(DFT, "===================================\n\n");

    return err;
}


/*************************************************
  函数描述 : 测试对模块板寄存器读写功能
             往每个模块板0-8号寄存器写数据0xaa,读出检测0-8寄存器值
             是否为0， 最后将所有寄存器设置为0x0(默认值)
  输入参数 : 无
  函数返回 :  0 -- 成功
             <0 -- 失败，其绝对值为失败次数。
*************************************************/
static int swg_reg_rw_test(void)
{
    int i, j, k;
    int w_val, r_vals[MB_REG_NUM], r_len;
    int  err = 0, res;

    const int test_reg_num = 8; /* 只测试前8个可读可写寄存器 */

    w_val = 0xAA;
    r_len  = 2;

    TST_PRT(DFT, "==== SWG module board register read/write test ====\n");
    TST_PRT(DFT, "Set all module board reg 0-%d to 0x%x\n", test_reg_num, w_val);
    for(i = 1; i < total_mb_num + 1; i++)
    {
        for(j = 0; j < test_reg_num; j++)
        {
            if((res = swg_write_mod_brd_reg(i, j, w_val)) < 0)
            {
                TST_PRT(RED, "Module board %d write reg %d value 0x%x FAILED, res = %d\n",
                        i, j, w_val, res);
            }
        }
    }

    TST_PRT(DFT, "Get all module board 0-%d register value and test whether is 0x%x\n", test_reg_num, w_val);
    for(i = 1; i < total_mb_num + 1; i++)
    {
        for(j = 0; j < test_reg_num; j += r_len)
        {
            if(j >= test_reg_num/2) //后半部分逐个读取
                r_len = 1;

            if((res = swg_read_mod_brd_reg(i, j,  r_len, r_vals)) < 0)
            {
                TST_PRT(RED, "Module board %d read reg %d-%d FAILED, res = %d\n",
                        i, j, j + r_len - 1, res);
            }

            for(k = 0; k < r_len; k++)
            {
                if(w_val != r_vals[k])
                {
                    err--;
                    TST_PRT(RED, "Module board %d read reg %d value is 0x%x, not 0x%x\n",
                            i, j, r_vals[k], w_val);
                }
            }
        }
    }

    /* 复位寄存器 */
    w_val = 0x0;
    TST_PRT(DFT, "Set all module board reg 0-%d to 0x%x\n", test_reg_num, w_val);
    for(i = 1; i <  total_mb_num + 1; i++)
    {
        for(j = 0; j < test_reg_num; j++)
        {
            if((res = swg_write_mod_brd_reg(i, j, w_val)) < 0)
            {
                TST_PRT(RED, "Module board %d write reg %d value 0x%x FAILED, res = %d\n",
                        i, j, w_val, res);
            }
        }
    }
    sleep(3);

    if(0 == err)
        TST_PRT(DFT, "SWG all module board register read/write test : PASSED\n");
    else
        TST_PRT(RED, "SWG all module board  register read/write test : FAILED\n");

    TST_PRT(DFT, "===================================\n\n");
    return err;
}

/*************************************************
  函数描述 : 测试使能/失能sim卡功能
             使能sim卡后从读sim卡状态，判断是否已使能
             失能sim卡后从读sim卡状态，判断是否已失能
             逐个通道和同时操作所有通道进行以上测试。
  输入参数 : 无
  函数返回 :  0 -- 成功
             <0 -- 失败，其绝对值为失败次数。
*************************************************/
static int swg_sim_card_enable_test(void)
{
    int err, res, i;
    int status[MAX_CHAN_NUM] = {0};

    err = 0;
    TST_PRT(DFT, "==== SWG sim card enable/disable test ====\n");

    /***** 1 ******/
    TST_PRT(DFT, "Enable all sim cards and get the status one by one\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_sim_card_enable(i, 1)) < 0)
        {
            TST_PRT(RED, "Enable channel %d sim card FAILED, res = %d\n", i, res);
            err--;
        }

        if((res = swg_get_sim_card_enable_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel %d sim card status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] <= 0)
        {
            TST_PRT(RED, "Enable channel %d sim card FAILED\n",  i);
            err--;
        }
    }

    /***** 2 ******/
    TST_PRT(DFT, "Disable all sim cards and get the status one by one\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_sim_card_enable(i, 0)) < 0)
        {
            TST_PRT(RED, "Disable channel %d sim card FAILED, res = %d\n", i, res);
            err--;
        }

        if((res = swg_get_sim_card_enable_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel %d sim card status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] != 0)
        {
            TST_PRT(RED, "Disable channel %d sim card  FAILED\n", i);
            err--;
        }
    }

    /***** 3 ******/
    TST_PRT(DFT, "Enable all sim cards and get the status at once\n");
    if((res = swg_sim_card_enable(SWG_ALL_CHANS , 1)) < 0)
    {
        TST_PRT(RED, "Enable all channel sim card at once FAILED, res = %d\n", res);
        err--;
    }

    if((res = swg_get_sim_card_enable_status(SWG_ALL_CHANS, status)) < 0)
    {
        TST_PRT(RED, "Get all channel sim card status at once FAILED, res = %d\n", res);
        err--;
    }
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if(status[i -1] <= 0)
        {
            TST_PRT(RED, "Enable sim card %d FAILED\n", i);
            err--;
        }
    }

    /***** 4 ******/
    TST_PRT(DFT, "Disable all sim cards and get the status at once\n");
    if((res = swg_sim_card_enable(SWG_ALL_CHANS , 0)) < 0)
    {
        TST_PRT(RED, "Disable all channel sim card at once FAILED, res = %d\n", res);
        err--;
    }

    if((res = swg_get_sim_card_enable_status(SWG_ALL_CHANS, status)) < 0)
    {
        TST_PRT(RED, "Get all channel sim card status at once FAILED, res = %d\n", res);
        err--;
    }
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if(status[i -1] != 0)
        {
            TST_PRT(RED, "Disable sim card %d FAILED\n", i);
            err--;
        }
    }

    if(0 == err)
        TST_PRT(DFT, "SWG sim card enable/disable test : PASSED\n");
    else
        TST_PRT(RED, "SWG sim card enable/disable test : FAILED\n");

    TST_PRT(DFT, "===================================\n\n");

    return err;
}

/*************************************************
  函数描述 : 测试通过power key 开机通道模块(m35/sim6320)功能
             开机模块然后检测是否已开机
             关机模块然后检测是否已关机
             逐个通道和同时操作所有通道进行以上测试。
  输入参数 : 无
  函数返回 :  0 -- 成功
             <0 -- 失败，其绝对值为失败次数。
  备注     ：测试时此函数先提供vbat供电,结束后关闭vbat借电.
*************************************************/
static int swg_chan_mod_power_test(void)
{
    int res, i, err;
    int status[MAX_CHAN_NUM] = {0};

    err = 0;
    TST_PRT(DFT, "==== SWG channel module power test ====\n");


    /* First of all supply channel module vbat */
    TST_PRT(DFT, "Supply all channel modules vbat and get the status one by one for power test\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_chan_mod_vbat_supply(i, 1)) < 0)
        {
            TST_PRT(RED, "Supple channel module %d vbat FAILED, res = %d\n", i, res);
            err--;
        }

        if((res = swg_get_chan_mod_vbat_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel module %d vbat status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] <= 0)
        {
            TST_PRT(RED, "Supple channel module %d vbat FAILED\n",  i);
            err--;
        }
    }

    /***** 1 ******/
    TST_PRT(DFT, "Power on all channel modules and get the status one by one\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_chan_mod_power_on_off(i, 1)) < 0)
        {
            TST_PRT(RED, "Power on channel module %d FAILED, res = %d\n", i, res);
            err--;
        }

        sleep(2);
        if((res = swg_get_chan_mod_power_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel module %d power status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] <= 0)
        {
            TST_PRT(RED, "Power on channel module %d FAILED\n",  i);
            err--;
        }
    }

    /***** 2 ******/
    TST_PRT(DFT, "Power off all channel modules and get the status one by one\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_chan_mod_power_on_off(i, 0)) < 0)
        {
            TST_PRT(RED, "Power off channel module %d FAILED, res = %d\n", i, res);
            err--;
        }

        sleep(2);

        if((res = swg_get_chan_mod_power_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel module %d power status FAILED, res = %d\n", i, res);
            err--;
        }

        if(status[0] != 0)
        {
            TST_PRT(RED, "Power off channel module %d FAILED\n",  i);
            err--;
        }

    }

    /***** 3 ******/
    TST_PRT(DFT, "Power on all channel modules and get the status at once\n");
    if((res = swg_chan_mod_power_on_off(SWG_ALL_CHANS , 1)) < 0)
    {
        TST_PRT(RED, "Power on all channel modules at once FAILED, res = %d\n", res);
        err--;
    }

    sleep(3);
    if((res = swg_get_chan_mod_power_status(SWG_ALL_CHANS, status)) < 0)
    {
        TST_PRT(RED, "Get all channel module power status at once FAILED, res = %d\n", res);
        err--;
    }

    for(i = 1; i < total_ch_num + 1; i++)
    {
        if(status[i -1] <= 0)
        {
            TST_PRT(RED, "Power on channel module %d FAILED\n",  i);
            err--;
        }
    }

    /***** 4 ******/
    TST_PRT(DFT, "Power off all channel modules and get the status at once\n");
    if((res = swg_chan_mod_power_on_off(SWG_ALL_CHANS , 0)) < 0)
    {
        TST_PRT(RED, "Power off all channel modules at once FAILED, res = %d\n", res);
        err--;
    }

    sleep(5);
    if((res = swg_get_chan_mod_power_status(SWG_ALL_CHANS, status)) < 0)
    {
        TST_PRT(RED, "Get all channel module power status at once FAILED, res = %d\n", res);
        err--;
    }
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if(status[i -1] != 0)
        {
            TST_PRT(RED, "Power off channel module %d FAILED\n",  i);
            err--;
        }
    }

    /* The last unsupply channel module vbat */
    TST_PRT(DFT, "Unsupply all channel modules vbat and get the status one by one when finished power test\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_chan_mod_vbat_supply(i, 0)) < 0)
        {
            TST_PRT(RED, "Unsupply channel module %d vbat FAILED, res = %d\n", i, res);
            err--;
        }

        if((res = swg_get_chan_mod_vbat_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel module %d vbat status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] != 0)
        {
            TST_PRT(RED, "Unsupply channel module %d vbat FAILED\n",  i);
            err--;
        }
    }

    if(0 == err)
        TST_PRT(DFT, "SWG channel module power test : PASSED\n");
    else
        TST_PRT(RED, "SWG channel module power test : FAILED\n");

    TST_PRT(DFT, "===================================\n\n");
    return err;
}

/*************************************************
  函数描述 : 测试功能提供，断开vbat功能
             提供vbat并检测vbat是否已经提供
             断开vbat并检测vbat是否已经断开
             分别逐个通道和同时操作所有通道进行以上测试。
  输入参数 : 无
  函数返回 :  0 -- 成功
             <0 -- 失败，其绝对值为失败次数。
*************************************************/
static int swg_chan_mod_vbat_test(void)
{
    int res, i, err;
    int status[MAX_CHAN_NUM] = {0};

    err = 0;
    TST_PRT(DFT, "==== SWG channel module vbat test ====\n");

    /***** 1 ******/
    TST_PRT(DFT, "Supply all channel modules vbat and get the status one by one\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_chan_mod_vbat_supply(i, 1)) < 0)
        {
            TST_PRT(RED, "Supple channel module %d vbat FAILED, res = %d\n", i, res);
            err--;
        }

        if((res = swg_get_chan_mod_vbat_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel module %d vbat status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] <= 0)
        {
            TST_PRT(RED, "Supple channel module %d vbat FAILED\n",  i);
            err--;
        }
    }

    /***** 2 ******/
    TST_PRT(DFT, "Unsupply all channel modules vbat and get the status one by one\n");
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if((res = swg_chan_mod_vbat_supply(i, 0)) < 0)
        {
            TST_PRT(RED, "Unsupply channel module %d vbat FAILED, res = %d\n", i, res);
            err--;
        }

        if((res = swg_get_chan_mod_vbat_status(i, &status[0])) < 0)
        {
            TST_PRT(RED, "Get channel module %d vbat status FAILED, res = %d\n", i, res);
            err--;
        }
        if(status[0] != 0)
        {
            TST_PRT(RED, "Unsupply channel module %d vbat FAILED\n",  i);
            err--;
        }
    }

    /***** 3 ******/
    TST_PRT(DFT, "Supply all channel modules vbat and get the status at once\n");
    if((res = swg_chan_mod_vbat_supply(SWG_ALL_CHANS , 1)) < 0)
    {
        TST_PRT(RED, "Supply all channel modules vbat at once FAILED, res = %d\n", res);
        err--;
    }

    if((res = swg_get_chan_mod_vbat_status(SWG_ALL_CHANS, status)) < 0)
    {
        TST_PRT(RED, "Get all channel module vbat status at once FAILED, res = %d\n", res);
        err--;
    }

    for(i = 1; i < total_ch_num + 1; i++)
    {
        if(status[i -1] <= 0)
        {
            TST_PRT(RED, "Supply channel module %d vbat FAILED\n",  i);
            err--;
        }
    }

    /***** 4 ******/
    TST_PRT(DFT, "Unsupply all channel modules vbat and get the status at once\n");
    if((res = swg_chan_mod_vbat_supply(SWG_ALL_CHANS , 0)) < 0)
    {
        TST_PRT(RED, "Unsupply all channel modules vbat at once FAILED, res = %d\n", res);
        err--;
    }

    if((res = swg_get_chan_mod_vbat_status(SWG_ALL_CHANS, status)) < 0)
    {
        TST_PRT(RED, "Get all channel module vbat status at once FAILED, res = %d\n", res);
        err--;
    }
    for(i = 1; i < total_ch_num + 1; i++)
    {
        if(status[i -1] != 0)
        {
            TST_PRT(RED, "Unsupply channel module %d vbat FAILED\n",  i);
            err--;
        }
    }

    if(0 == err)
        TST_PRT(DFT, "SWG channel module vbat test : PASSED\n");
    else
        TST_PRT(RED, "SWG channel module vbat test : FAILED\n");

    TST_PRT(DFT, "===================================\n\n");

    return err;
}

/*************************************************
  函数描述 : 测试选择升级通道功能
             升级指定升级通道，然后检测该通道是否已被选中
  输入参数 : 无
  函数返回 :  0 -- 成功
             <0 -- 失败，其绝对值为失败次数。
*************************************************/
static int swg_select_upgrade_chan_test(void)
{
    int res, err, i;

    err = 0;
    TST_PRT(DFT, "=== SWG select upgrade channel test ====\n");

    for(i = 1; i < total_ch_num + 1; i++)
    {
        res = swg_select_upgrade_chan(i); /*  只测第1个模块板所有通道 */
        if(0 == res)
        {
            if( 1 != swg_upgrade_chan_is_selected(i))
            {
                TST_PRT(RED, "Select chan %d upgrade FAILED\n", i);
                err--;
            }
        }
        else if(1 == res)
        {
            TST_PRT(DFT, "Channel %d not support select upgrade\n", i);
        }
        else
        {
            TST_PRT(RED, "Select channel %d upgrade FAILED, res = %d\n", i, res);
            err--;
        }
    }

    if(0 == err)
        TST_PRT(DFT, "SWG select upgrade channel test : PASSED\n");
    else
        TST_PRT(RED, "SWG select upgrade channel test : FAILED\n");

    TST_PRT(DFT, "===================================\n\n");

    return 0;
}


int swg_device_test(void)
{
    int err =  0;

    err = swg_test_get_info();
    err += swg_reg_rw_test();
    err += swg_sim_card_enable_test();
    err += swg_chan_mod_vbat_test();
    err += swg_chan_mod_power_test();
    err += swg_select_upgrade_chan_test();

    if(0 == err)
    {
        TST_PRT(DFT, "##############################\n");
        TST_PRT(DFT, "### SWG device test PASSED ###\n");
        TST_PRT(DFT, "##############################\n");
    }
    else
    {
        TST_PRT(RED, "##############################\n");
        TST_PRT(RED, "### SWG device test FAILED ###\n");
        TST_PRT(RED, "### Total failures : %3d   ###\n", (-err));
        TST_PRT(RED, "##############################\n");
    }

    return 0;
}

#endif //SWG_TEST

