#include "mod_brd_regs.h"
#include "mod_brd_hw_intf.h"
#include "sim_device.h"
#include "swg_device.h"
#include "../common/bsp_tools.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX_CHAN 16
#define MAX_SIM 4

#define CLEAR2BITS(value, bit) (value&(~(0x03<<bit)))
#define SET2BITS(value, bit_value, bit) (value|((bit_value) << (bit<<1)))

//get the bit value, bit rainge 1~8
#define GETBIT(value, bit) ((value>>(bit-1))&0x1)
//#define SET2BITS (num, bit) 
/*
typedef struct sim_device{
    struct hw_intf               hwif;
   unsigned char                sim_slot_reg_map[MAX_CHAN/4];
   pthread_mutex_t           sim_lock;
    SIM_CHANNEL_S           sim_chn[MAX_CHAN];
}SIM_DEVICE_S;

//SIM_DEVICE_S sim_dev[MAX_CHAN/4];

SIM_DEVICE_S  sim_dev;
*/
int sim_dev_init(struct swg_device *dev){
	int i = 0, j = 0, k = 0;
	char info_buf[256] = {0};
	char ver_buf[64]={0};
	const char *swg_simswitch="SWG_SIM_SWITCH_1_4";
	const char *hw_head="HwVer : V";
	const char *sw_head="SwVer : V";
	int len = 0;
	char *p = NULL, *endptr=NULL;
	int pos = 0;
	unsigned char major = 0, minor = 0, bugfix = 0;
//	char sim_device[256] = {0};
	for(k = 0; k < SIM_SWITCH_NUM; k++){
		struct hw_intf *hwif = &dev->sim_switch[k].hwif;
		struct sim_device *sim_dev = &dev->sim_switch[k] ;
		sim_dev->base_chan_no = dev->mod_brd[k].base_chan_no;
		sim_dev->channels = dev->mod_brd[k].channels;
		for(i = 0; i < MAX_CHAN/4; i++){
			sim_dev->sim_slot_reg_map[i] = 0x00;
		}
		for(i = 0; i < sim_dev->channels; i++){
			for(j = 0; j < 4; j++){
				//sim_dev.sim_chn[i].sim_slot[j].sim_evt = 1;
				dev->sim_chan[i+sim_dev->base_chan_no].sim_slot[j].sim_evt = 1;
			}
			dev->sim_chan[i+sim_dev->base_chan_no].cur_slot = 0;
			dev->sim_chan[i+sim_dev->base_chan_no].sim = sim_dev;
			//sim_dev.sim_chn[i].cur_slot = 0;//not insert
		}

		sprintf(hwif->name, "%s%d", SIM_SWITCH_PATH, k);
		if(access(hwif->name, F_OK) != 0){
			sim_dev->state = SIM_SWTICH_STATUS_NOT_INSERT;
			continue;
		}
		sim_dev->state = SIM_SWTICH_STATUS_INSERT;
		hwif->fd = open_serial(hwif->name, 115200, 0, 8, 1, 0);
		if(hwif->fd < 0){
			sim_dev->state = SIM_SWTICH_STATUS_INIT_FAILED;
			BSP_PRT(ERR, "open %s failed.\n", hwif->name);
			continue;
		}

		if(hwif_get_verison_info(hwif, info_buf, sizeof(info_buf), &len) != 0)
		{
			BSP_PRT(ERR, "Sim Switch %d get version info fail.\n", i+1);
			continue;
		}
		/*
		SWG_SIM_SWITCH_1_4
		HwVer : V1.1
		SwVer : V1.1
		Aug 29 2019
		12:18:10
		*/
		if(strstr(info_buf, swg_simswitch) != NULL){
			BSP_PRT(ERR, "Sim Switch[%d]: SWG_SIM_SWITCH_1_4 board.\n", i+1);		
		}

		if(NULL != (p = strstr(info_buf, hw_head))){
			p += strlen(hw_head);
			pos = 0;
			while ( *p && (pos < sizeof(ver_buf)) && (*p != '\n') && (*p != '\r') )
				ver_buf[pos++] = *p++;
			ver_buf[i] = '\0';
			major = strtol(ver_buf, &endptr, 10) & 0xFF;
			p = ++endptr;
			minor = strtol(p, &endptr, 10) & 0xFF;
			bugfix = 0;
			sim_dev->hw_ver =  VERSION_NUMBER(major, minor, bugfix);
		}

		if(NULL != (p = strstr(info_buf, sw_head))){
			p += strlen(sw_head);
			pos = 0;
			while ( *p && (pos < sizeof(ver_buf)) && (*p != '\n') && (*p != '\r') )
				ver_buf[pos++] = *p++;
			ver_buf[pos] = '\0';
			major = strtol(ver_buf, &endptr, 10) & 0xFF;
			p = ++endptr;
			minor = strtol(p, &endptr, 10) & 0xFF;
			bugfix = 0;
			sim_dev->sw_ver =  VERSION_NUMBER(major, minor, bugfix);
		}

		dev->sys_type = SYS_TYPE_1CHAN4SIMS;
		pthread_mutex_init(&sim_dev->sim_lock, NULL);
		sim_dev->state = SIM_SWTICH_STATUS_INITED;
	}
	set_all_sim_slot(dev, 1);
	//hwif->chans = 4;
	return 0;
}

int set_all_sim_slot(struct swg_device *dev, int slot){
	int i = 0, j = 0; 
	for(j = 0; j < SIM_SWITCH_NUM; j++){
		struct sim_device *sim_dev = &dev->sim_switch[j];
		if(sim_dev->state != SIM_SWTICH_STATUS_INITED)
			continue;
		pthread_mutex_lock(&sim_dev->sim_lock);
		for(i = 0; i < MAX_CHAN; i++){
			sim_chn_set_slot(&sim_dev->hwif, SIM_REG_SELECT_1+i, 0xFF);
		}
		usleep(600000);
		for(i = 0; i < MAX_CHAN; i++){
			sim_chn_set_slot(&sim_dev->hwif, SIM_REG_SELECT_1+i, slot - 1);
			dev->sim_chan[i+sim_dev->base_chan_no].cur_slot= slot -1;
		}
		pthread_mutex_unlock(&sim_dev->sim_lock);
	}
	return 0;
}
static int get_sim_chan_index(struct swg_device *dev, int chan, int *sim_index, int *chan_index){
	int i = 0; 
	chan = chan & 0xFFFF;
	if(chan <= 0)
		return -1;
	if(chan > dev->total_chans)
		return -1;
	chan --;
	for(i = 0; i < SIM_SWITCH_NUM; i++){
		if(dev->sim_switch[i].base_chan_no + dev->sim_switch[i].channels <= chan )
			continue;
		*sim_index = i;
		*chan_index = chan - dev->sim_switch[i].base_chan_no ;
		return 0;
	}
	return -1;
}

int set_sim_slot(struct swg_device *dev, int chan, int solt){
	int reg = 0, bit = 0;
	unsigned int reg_value;
	int sim_index, chan_index;
	if(get_sim_chan_index(dev, chan, &sim_index, &chan_index)!= 0)
		return -1;
	struct sim_device *sim_dev = &dev->sim_switch[sim_index];
	
	pthread_mutex_lock(&sim_dev->sim_lock);
	//at first , disable all sim slot
	sim_chn_set_slot(&sim_dev->hwif, chan_index + SIM_REG_SELECT_1, 0xFF);
	pthread_mutex_unlock(&sim_dev->sim_lock);
	
	usleep(600000);
	
	pthread_mutex_lock(&sim_dev->sim_lock);
	if(sim_chn_set_slot(&sim_dev->hwif, chan_index + SIM_REG_SELECT_1, solt - 1) < 0){
		pthread_mutex_unlock(&sim_dev->sim_lock);
		return -1;
	}
	pthread_mutex_unlock(&sim_dev->sim_lock);
	dev->sim_chan[chan - 1].cur_slot = solt - 1;

	return 0;
}

int get_sim_slot(struct swg_device *dev,int chan, int *slots){
	if((chan & 0xFFFF) != 0xFFFF){
		slots[0] = dev->sim_chan[chan -1].cur_slot;
		return 0;
	}
	int i = 0;
	for(i = 0; i < dev->total_chans;i++){
		slots[i] = dev->sim_chan[i].cur_slot;
	}
	return 0;
}

int read_sim_state(struct swg_device *dev){
	int value[MAX_CHAN/2] = {0};
	int j = 0, success_flag = 0;
	int i = 0;
	for(j = 0; j < SIM_SWITCH_NUM; j++){
		struct sim_device *sim_dev = &dev->sim_switch[j];
		if(sim_dev->state != SIM_SWTICH_STATUS_INITED)
			continue;
		pthread_mutex_lock(&sim_dev->sim_lock);
		int res = hwif_reg_read_mul(&sim_dev->hwif, SIM_REG_STATUS_1, MAX_CHAN/2, value);
		pthread_mutex_unlock(&sim_dev->sim_lock);
		if(res >= 0){
			for(i = 0; i < sim_dev->channels; i = i+2){
				//one register contains two channel' sim card state
				int pos = i/2;
				//On the modlule borad,0 means remove, 1 means insert. 
				//On the sim switch, 1 means remove, 0 means insert
				//So, must be set the the value to opposite
				value[pos] = ~value[pos];// 1menas remove, 0 means insert
				int base_no= sim_dev->base_chan_no;
				struct sim_channel *sim_chan1 = &dev->sim_chan[base_no + i];
				struct sim_channel *sim_chan2 = &dev->sim_chan[base_no + i + 1];
				//save last state
				sim_chan1->sim_slot[0].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan1->sim_slot[1].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan1->sim_slot[2].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan1->sim_slot[3].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan2->sim_slot[0].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan2->sim_slot[1].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan2->sim_slot[2].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				sim_chan2->sim_slot[3].sim_prv_evt = sim_chan1->sim_slot[0].sim_evt;
				//save new state
				sim_chan1->sim_slot[0].sim_evt = GETBIT(value[pos], 1);
				sim_chan1->sim_slot[1].sim_evt = GETBIT(value[pos], 2);
				sim_chan1->sim_slot[2].sim_evt = GETBIT(value[pos], 3);
				sim_chan1->sim_slot[3].sim_evt = GETBIT(value[pos], 4);
				sim_chan2->sim_slot[0].sim_evt = GETBIT(value[pos], 5);
				sim_chan2->sim_slot[1].sim_evt = GETBIT(value[pos], 6);
				sim_chan2->sim_slot[2].sim_evt = GETBIT(value[pos], 7);
				sim_chan2->sim_slot[3].sim_evt = GETBIT(value[pos], 8);
			}
		}
		success_flag++;
	}
	BSP_PRT(INFO,"person sim state ok.\n");
	if(success_flag == 0)
		return -1;
	return 0;
}

int get_sim_state_all(struct swg_device *dev,int chan, int state[][4]){

	//int res = hwif_reg_read_mul(&sim_dev->hwif, SIM_REG_STATUS_1, MAX_CHAN/2, value);
	int i = 0, j = 0;
	if((chan & 0xFFFF) != 0xFFFF){//get one channel
		for(j = 0; j < 4; j++){
			state[0][j] = dev->sim_chan[chan - 1].sim_slot[j].sim_evt;
		}
		return 0;
	}
	for(i = 0; i < dev->total_chans; i++){
		for(j = 0; j < 4;j++){
			state[i][j] = dev->sim_chan[i].sim_slot[j].sim_evt;
		}
	}
	BSP_PRT(INFO,"get all sim state ok\n");
	return 0;
}

int get_sim_state_one(struct swg_device *dev,int chan, int card, int *state){

	int i = 0;
	card = card-1;
	if((chan & 0xFFFF) != 0xFFFF){//get one channel
		state[0] = dev->sim_chan[chan - 1].sim_slot[card].sim_evt;
		return 0;
	}
	for(i = 0; i < dev->total_chans; i++){
		state[i] = dev->sim_chan[i].sim_slot[card].sim_evt;
	}
	return 0;
}


int get_sim_state(struct swg_device *dev,int chan, int *state){
	if(chan <= 0 && chan != -1)
		return -1;
	if(state == NULL)
		return -1;

	SIM_CHANNEL_S *sim_chn ;
	//int res = hwif_reg_read_mul(&sim_dev->hwif, SIM_REG_STATUS_1, MAX_CHAN/2, value);
	int res = read_sim_state(dev);
	if(res >= 0){
		int i = 0;
		if((chan & 0xFFFF) != 0xFFFF){
			sim_chn = &dev->sim_chan[i];
			state[0] = sim_chn->sim_slot[sim_chn->cur_slot].sim_evt;
			return 0;
		}
		for(i = 0; i < dev->total_chans; i++){
			sim_chn = &dev->sim_chan[i];
			state[i] = sim_chn->sim_slot[sim_chn->cur_slot].sim_evt;
		}
	}
	BSP_PRT(INFO,"get sim state ok\n");
	return 0;
}

int sim_dev_deinit(struct swg_device *dev){
	int i = 0; 
	for(i = 0; i < SIM_SWITCH_NUM; i++){
		struct sim_device *sim_dev = &dev->sim_switch[i];
		struct hw_intf *hwif = &sim_dev->hwif;
		if(hwif->fd > 0){
			close(hwif->fd);
			hwif->fd = -1;
		}
		sim_dev->state = SIM_SWTICH_STATUS_NOT_INSERT;
		pthread_mutex_destroy(&sim_dev->sim_lock);
	}
	return 0;
}

int get_sim_evet_all(struct swg_device *dev, int **event){

	return 0;
}
