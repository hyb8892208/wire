
/***************************************************
文件描述  : 实现控制、检测uc15状态接口, 包括：
            1) set SIM card enable/disable
            2) get SIM card enable/disable status
            3) uc15模块上断/掉电(VBAT ON/OFF)
            4) uc15模块紧急开机/关机(EMERGENCY ON/OFF)
            5) uc15模块按键开机/关机(POWR_KEY ON/OFF)
            6) SIM card insert status
            7) 通道模块板(包含2块板的模块）插拔状态
            8) uc15模块开机状态
修改/时间 : wengang.mu@openvox.cn/2018.05.31
*****************************************************/
#include <unistd.h>

#include "mod_brd_regs.h"
#include "mod_brd_hw_intf.h"
#include "bsp_tools.h"
#include "swg_device.h"

#define MAX_CHECK_STATUS_CNT    6

/*************************************************
  函数描述 : 重启uc15模块
  输入参数 : brd -- 模块板结构体
             chan -- 指定紧急关机的通道, =0xFF表示所有通道
  函数返回 :  0 -- 成功
             <0 -- 失败
  备注     ： 函数退出不确保模块已经重启完成
  修改/时间 : wengang.mu@openvox.cn/2018.05.31
*************************************************/
static int uc15_reboot(struct swg_mod_brd *brd, int chan)
{
    int reg, bit, res, i;
    unsigned int pwr_bit_map = 0;
	pwr_bit_map = brd->power_status_h | brd->power_status_h;
	
    /* 检查当前状态是否为关机状态, 如果是，则退出 */
    /* 状态位为'0' */
    if(0xFF == chan)    /* 操作所有通道 */
    {
        if((pwr_bit_map & ((1 << brd->channels) - 1)) == 0)
            return 0;
    }
    else  /* 操作指定通道 */
    {
        if(!(pwr_bit_map  & (1 << chan)))
            return 0;
    }

    /* 检查当前状态不是需要的状态, 设置 */

    /* 初始化起始通道 */
    if(0xFF == chan)    /* 操作所有通道 */
        i = 0;
    else 
        i = chan;       /* 只操作指定通道 */


    for( ; i < brd->channels; i++)
    {
        if(!(pwr_bit_map & (1 << i)))
        {
            /* 已经关机，不能再关机，否则出问题 */

            if(0xFF != chan) /* 操作指定通道完成 */
                break; 
            else
                continue;
        }

        reg = MB_REG_EMERG_OFF_L + i / MB_REG_WIDE;
        bit = i % MB_REG_WIDE;

        /* 50-100ms的脉冲可使模块重启 */
        /* 先拉低 100 ms */
        res = hwif_reg_write_bit(&brd->hwif, reg, bit, 0); 
        usleep(50 * 1000);
        res = hwif_reg_write_bit(&brd->hwif, reg, bit, 1); 
        usleep(100 * 1000);
        res = hwif_reg_write_bit(&brd->hwif, reg, bit, 0); 

        /* 如果操作指定通道, 只执行一次循环 */
        if(0xFF != chan)
        {
            break;
        }
    }

    return 0;
}

int uc15_translate_vbat_v010000(struct swg_mod_brd *mb, int mb_idx, int *reg_l, int *reg_h){
	
	int *reg;
	unsigned char *vbat_state;
	int bit;
	
	if(mb_idx / MB_REG_WIDE){
		vbat_state = &mb->vbat_status_h;
		reg = reg_h;
	}else{
		vbat_state = &mb->vbat_status_l;
		reg = reg_l;
	}

	bit = mb_idx%MB_REG_WIDE;
	
	//处理事件
	if(mb->vbat_event[mb_idx] == CHAN_MOD_VBAT_ON){
		*vbat_state |= 1 << bit;
		mb->vbat_event[mb_idx] = CHAN_MOD_VBAT_UNKOWN;
		BSP_PRT(DBG, "set chan[%d] vbat to on ok, clear vbat event.\n", mb_idx+mb->base_chan_no + 1);
	}else if(mb->vbat_event[mb_idx] == CHAN_MOD_VBAT_OFF){
		*vbat_state &= ~(1 << bit);
		mb->vbat_event[mb_idx] = CHAN_MOD_VBAT_UNKOWN;
		BSP_PRT(DBG, "set chan[%d] vbat to off ok, clear vbat event.\n", mb_idx+mb->base_chan_no + 1);
	}

	//设置寄存器的值
	if(((*vbat_state >> bit)& 0x1) == CHAN_MOD_VBAT_ON){
		*reg &= ~(1 << bit);
	}else{
		*reg |= (1 << bit);
	}
	return 0;

}

int uc15_translate_vbat_v020000(struct swg_mod_brd *mb, int mb_idx, int *reg_l, int *reg_h){
	int *reg;
	unsigned char *vbat_state;
	int bit;
	
	if(mb_idx / MB_REG_WIDE){
		vbat_state = &mb->vbat_status_h;
		reg = reg_h;
	}else{
		vbat_state = &mb->vbat_status_l;
		reg = reg_l;
	}

	bit = mb_idx%MB_REG_WIDE;
	
	//处理事件
	if(mb->vbat_event[mb_idx] == CHAN_MOD_VBAT_ON){
		*vbat_state |= 1 << bit;
		mb->vbat_event[mb_idx] = CHAN_MOD_VBAT_UNKOWN;
		BSP_PRT(DBG, "set chan[%d] vbat to on ok, clear vbat event.\n", mb_idx+mb->base_chan_no + 1);
	}else if(mb->vbat_event[mb_idx] == CHAN_MOD_VBAT_OFF){
		*vbat_state &= ~(1 << bit);
		mb->vbat_event[mb_idx] = CHAN_MOD_VBAT_UNKOWN;
		BSP_PRT(DBG, "set chan[%d] vbat to off ok, clear vbat event.\n", mb_idx+mb->base_chan_no + 1);
	}

	//设置寄存器的值
	if(((*vbat_state >> bit)& 0x1) == CHAN_MOD_VBAT_ON){
		*reg |= (1 << bit);
	}else{
		*reg &= ~(1 << bit);
	}

	return 0;
}

int uc15_translate_vbat_status_v010000(struct swg_mod_brd *mb, int md_idx, int *reg_value){
	unsigned int vbat_bit_map;
	unsigned int bit_value;
	int bit;
    vbat_bit_map = ~(((reg_value[1] & 0xFF) << 8) | (reg_value[0] & 0xFF)); //0 供电，1断电
	bit_value = (vbat_bit_map >> md_idx) & 0x1;
	bit = md_idx % MB_REG_WIDE;
	if(md_idx/MB_REG_WIDE){
		mb->vbat_status_h &= ~(1<<bit);//清空vbat_lamp中对应bit位
    	mb->vbat_status_h |= (bit_value << bit);//
	}else{
		mb->vbat_status_l &= ~(1<<bit);//清空vbat_lamp中对应bit位
    	mb->vbat_status_l |= (bit_value << bit);//
	}
	return 0;
}
int uc15_translate_vbat_status_v020000(struct swg_mod_brd *mb, int md_idx, int *reg_value){
	unsigned int vbat_bit_map;
	unsigned int bit_value;
	int bit;
    vbat_bit_map = ((reg_value[1] & 0xFF) << 8) | (reg_value[0] & 0xFF); //1 供电，0断电
	bit_value = (vbat_bit_map >> md_idx) & 0x1;
	bit = md_idx % MB_REG_WIDE;
	
	if(md_idx/MB_REG_WIDE){
		mb->vbat_status_h &= ~(1<<bit);//清空vbat_lamp中对应bit位
    	mb->vbat_status_h |= (bit_value << bit);//
	}else{
		mb->vbat_status_l &= ~(1<<bit);//清空vbat_lamp中对应bit位
    	mb->vbat_status_l |= (bit_value << bit);//
	}
	return 0;
}

int uc15_translate_power_status(struct swg_mod_brd *mb, int mb_idx, int *reg_value){
	unsigned int power_bit_map;
	unsigned int bit_value;
	int bit;
    /* 获取所有通道状态值 */
    /* MB_REG_SIM_INSERT_L地址在MB_REG_SIM_INSERT_L之后 */
    power_bit_map = ((reg_value[0] & 0xFF) << 8) | (reg_value[1] & 0xFF);  /* 寄存器值1开机，0表示关机 */

	bit_value = (power_bit_map>>mb_idx)&0x1;
	
	bit = mb_idx % MB_REG_WIDE;
	
	if(mb_idx / MB_REG_WIDE){
		mb->power_status_h &= ~(1<<bit);
		mb->power_status_h |= (bit_value << bit);
	}else{
		mb->power_status_l &= ~(1<<bit);
		mb->power_status_l |= (bit_value << bit);
	}
	BSP_PRT(DBG, "get chan[%d] power state from mod_brd ok, state is power %s.\n", mb_idx+mb->base_chan_no + 1, bit_value?"on":"off");
	return 0;

}

struct sim_card_ops sim_uc15_ops_v010000 =
{
    /* uc15 没有紧急关机，与之对应的是重启 */
    .emergency_power_off            = uc15_reboot, 
    .translate_vbat                  = uc15_translate_vbat_v010000,
    .translate_vbat_status           = uc15_translate_vbat_status_v010000,
    .translate_power_status          = uc15_translate_power_status,
};

struct sim_card_ops sim_uc15_ops_v020000 =
{
    /* uc15 没有紧急关机，与之对应的是重启 */
    .emergency_power_off            = uc15_reboot,
    .translate_vbat                  = uc15_translate_vbat_v020000,
    .translate_vbat_status           = uc15_translate_vbat_status_v020000,
    .translate_power_status          = uc15_translate_power_status,
};

