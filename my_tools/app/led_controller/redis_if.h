#ifndef __REDIS_IF_H
#define __REDIS_IF_H
void led_create_redis_thread(int sys_type);
int get_sim_slot_info(char *out);
int get_total_channel();
int get_sys_type();

#endif
