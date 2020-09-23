
#ifndef __SWG_DEVICE_H
#define __SWG_DEVICE_H
#include "swg_device.h"

int sim_dev_init(struct swg_device *dev);
int set_sim_slot(struct swg_device *dev,int chan, int solt);
int get_sim_slot(struct swg_device *dev,int chan, int *slots);
int set_all_sim_slot(struct swg_device *dev, int slot);
int get_sim_state_all(struct swg_device *dev,int chan, int state[][4]);
int sim_dev_deinit(struct swg_device *dev);

int get_sim_state(struct swg_device *dev,int chan, int *state );
int get_sim_state_one(struct swg_device *dev,int chan, int card, int *state);
#endif
