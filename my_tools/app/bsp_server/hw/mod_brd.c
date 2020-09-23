
#include "mod_brd_hw_intf.h"
#include "mod_brd_regs.h"
#include "swg_device.h"
#include "bsp_tools.h"
/*************************************************
  函数描述 : 选择模块板需要升级无线模块固件
  输入参数 : mb -- 模块板
             chan -- 指定升级的通道 =0xFF 所有通道都 不 选
  函数返回 :  0 -- 成功
              1 -- 不支持选择升级通道功能
             <0 -- 失败
*************************************************/
int mod_brd_select_upgrade_chan(struct swg_mod_brd *mb, int chan)
{
    int  res;
    int ch444g_idx, chan_idx, ctrl_l_val, ctrl_h_val;
    int tmp_h_val;


    if((mb->name == MB_SWG_1032_BASE) 
       && (mb->hw_ver == VERSION_NUMBER(2,0,0)))
    {
        res = hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, 0x1F); //失能所有CH444G, 0使能，1失能
        if(0xFF == chan) /* 一个通道也 不 选择 */
        {
            /* 失能所有第一层和第二层CH444G芯片 */ 
            res = hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, 0x1F);

            if(0 == res)
                return 0;
            else
                return -1;
        }
        ch444g_idx = chan / 4;  /* 1个CH444G管理4个通道 */
        chan_idx = chan % 4;    /* 通道在CH444G上的下标 */


        ctrl_l_val = (chan_idx << (ch444g_idx*2));  /* 选择通道 */
        ctrl_h_val = (ch444g_idx << 5);         /* 选择最低层4个CH444G中的一个 */

        /* 1, 写选择 */
        res |= hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_L, ctrl_l_val);   /* 选择通道 */
        res |= hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, ctrl_h_val);    /* 选择最底层4个CH444G中的一个 */ 
        /*2, 使能 */ 
        //tmp_h_val = (1 << 4) | (1 << ch444g_idx); /* 使能相应CH444G */
        //tmp_h_val = (~tmp_h_val) & 0x1F; /* 0使能，1失能 */
        //ctrl_h_val |= tmp_h_val; /* 使能相应CH444G, 选择或等是省去读改写步骤 */
        //res |= hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, ctrl_h_val);    /* 使能 */ 

        if(0 == res)
            return 0;
        else
            return -1;
    }else if(mb->name == MB_SWG_VS_USB_BASE){
    
         res = hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, 0x01);
        if(0xFF== chan){
            res = hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, 0x01);
            if(0 == res)
                return 0;
            else
                return -1;
        }
        ctrl_l_val = chan % 4;
        ctrl_h_val = 0x00;
        res |= hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_L, ctrl_l_val); 
        res |= hwif_reg_write(&mb->hwif, MB_REG_CH444_CTRL_H, ctrl_h_val); 
		usleep(5*1000);//睡眠5ms，避免查询升级通道是否已经被选中失败
        if(0 == res)
            return 0;
        else
            return -1;
    }

    return 1;
}

/*************************************************
  函数描述 : 检测模块板升级通道是否已经被选中。
  输入参数 : mb -- 模块板
             chan -- 指定检测的通道
  函数返回 :  0 -- 通道没有被选择为升级通道
              1 -- 通道被选择为升级通道
              2 -- 通道不支持选择升级功能
             <0 出错
*************************************************/
int mod_brd_upgrade_chan_is_selected(struct swg_mod_brd *mb, int chan)
{
    int res, r_vals[2];
    //const int h_vals[] = {0x0e, 0x2d, 0x4b, 0x67};
    const int h_vals[] = {0x00, 0x20, 0x40, 0x60};//我们在选择升级通道时低五位全部设置成0(全部使能)，因此这里需要把低五位也全部设置成0
    const int l_vals[] = {0x00, 0x01, 0x02, 0x03};
    const int vs_h_vals = 0x00;
    const int vs_l_vals[] = {0x00, 0x01, 0x02, 0x03};

    if((mb->name == MB_SWG_1032_BASE) 
            && (mb->hw_ver == VERSION_NUMBER(2,0,0)))
    {
        if((res = hwif_reg_read_mul(&mb->hwif,  MB_REG_CH444_CTRL_L, 2, r_vals)) < 0)
            return -1;

        if(!((r_vals[1] == h_vals[chan / 4])
                    && (r_vals[0] == (l_vals[chan % 4] << ((chan / 4)*2)))))
        {
            return 0; /* not selected */
        }
        else
        {
            return 1; /* is selected */
        }
    }else if(mb->name == MB_SWG_VS_USB_BASE){
        if((res = hwif_reg_read_mul(&mb->hwif,  MB_REG_CH444_CTRL_L, 2, r_vals)) < 0)
            return -1; 
        if(!((r_vals[1] == vs_h_vals)
                    && (r_vals[0] == vs_l_vals[chan%4])))
        {
            return 0; /* not selected */
        }
        else
        {
            return 1; /* is selected */
        }
    }
    else
    {
        return 2; /* not supported */
    }

    return 0;


}


