#ifndef __REDIS_INTERFACE_H
#define __REDIS_INTERFACE_H
int conn_redis();

int get_phonenumber(int chan, char *phonenumber);

void redis_disconnect(void);
#endif
