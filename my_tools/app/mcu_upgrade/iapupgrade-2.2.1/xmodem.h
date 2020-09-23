#ifndef __IAP_XMODEM__H__
#define __IAP_XMODEM__H__

#include <stdio.h>

#define HEAD		3
#define CSUM		2
#define PKT_LEN 	32
#define CRCXMODEM	0x0000

#define X_SOH	0x01   // 32 Byte Xmodem
#define X_STX	0x02  // 1k-Xmodem
#define X_ACK	0x06
#define X_NAK	0x15
#define X_EOF	0x04
#define X_CAN	0x18


int xmodem_calculate_crc(unsigned int crc, unsigned char data);
void xmodem_configure_serial(int fd, int speed, char * mode);
int calc_file_packet(FILE *fp, int pkt_len);
unsigned int calc_crc16(FILE *fp);

#endif

