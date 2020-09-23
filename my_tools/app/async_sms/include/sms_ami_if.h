#ifndef __SMS_AMI_IF_H
#define __SMS_AMI_IF_H
int sms_ami_init(void);
int insert_to_fail_list(int chan_id, char *uuid, char *message, char *phonenumber);
#endif
