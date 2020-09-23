/****************************************************************************
 *
 * iapupdate    --The tool used to upgrade STM32 firmware(IAP) by UART(xmodem protocol).
 *
 * Copyright (c) 2009-2017 OpenVox Communication Co.,Ltd.
 * All Rights Reserved.
 * Author: Santmoriz.Chiu <Santmoriz.Chiu@openvox.cn>
 *
 * xmodem is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * @file iapupgrade.c
 * \brief Top level source file for iapupdate  - the IAP upgrade util.
 *
 *  iapupgrade could be used to upgrade STM32 firmware in UC serial devices.
 *
 * Revision:
 *  2017-05-14 16:21 support struct apdu(application header)
 *  2017-05-25 11:30 decrease the struct apdu to 31 byte.
 *  2017-05-25 21:20 output in raw mode
 *  2017-06-07 09:54 Tuned the structure of apdu
 *
 ******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>
#include "xmodem.h"
#include "iapupgrade.h"

#define APP_NAME    "iapupgrade"

static const char iap_version[] = "2.4.0";
static int serial = 0;
static int xmodem_running = 1;

static void get_welcome_info(void) {
	printf(IAP_INFO, iap_version);
}

const char *iap_get_version(void)
{
	return iap_version;
}
const char *iap_show_version(void)
{
	printf("%s: Version %s\n", APP_NAME, iap_get_version());
	return iap_get_version();
}

void iap_interrupt(int sign)
{
	xmodem_running = 0;
	close(serial);

	exit(0);
}

// prints string as hexstatic
void phex(uint8_t* str, char *desc)
{
	printf("%s :\n", desc);
	unsigned char i;
	for(i = 0; i < 32; i++){
		printf("%02x", str[i]);
		if((i & 0x0F) == 0x0F)
			printf("\n");
		else
			printf(" ");
	}
	printf("\n");
}

void usage(void)
{
	printf(
			"Usage:\n"
			"  %s  -h\n"
			"  %s  -p <serial port>\n"
			"  %s  -s <baud speed>\n"
			"  %s  -i <input file>\n"
			"   Specify the STM32 firmware image\n"
			"   Set the the serial attribute\n"
			"\n"
			"Options:\n"
			"  -h      Show help\n"
			"  -p      Specify the serial port\n"
			"  -s      Set the baud rate. 1200 1800 2400 4800 9600 19200 38400 57600 230400 115200. default is 115200\n"
			"  -v      Show version\n"
			"  -i      Speicify the image file.  e.g. stm32-2.0.1.bin\n"
			"  -m      Set work mode.  [5-8] [N|E|O] [1-2] ex: 7E2. default is 8N1\n"
			"\n", APP_NAME, APP_NAME, APP_NAME, APP_NAME);
	exit(0);
}

struct opts *opts_init(struct opts *ops)
{
	struct opts *res = ops;
	res->mode =  "8N1";
	res->input = NULL;
	res->output = NULL;
	res->speed = 115200;

	return res;
}

int opts_param_check(struct opts *ops)
{
	int ret = 0;
	if(!ret && !ops->input) {
		printf("missing input file!\n");
		ret = ERR_INVALIDED_PARAM;
	}

	if((access(ops->input, F_OK)) != 0){
		printf("Are you %s exist?\n", ops->input);
		ret = ERR_FILE_NOT_EXIST;
	}

	if(!ret && !ops->output) {
		printf("missing serial port file\n");
		ret = ERR_INVALIDED_PARAM;
	}

	if((access(ops->output, F_OK)) != 0){
		printf("Are you %s exist?\n", ops->output);
		ret = ERR_FILE_NOT_EXIST;
	}

	return ret;
}


void print_hex(uint8_t *buff )
{
	int i;
	if(!buff)
		return;
	printf("Send data :\r\n");
	for(i = 0; i < 37; i++)
		printf("0x%02x ", *(buff+i));
	printf("\r\n");
}


int main(int argc, char ** argv)
{
	int opt = 0;
	int cpt = 1;
	int retry = 0;
	int oldcpt = 0;
	int pkt = 0;
	int ret = 0;
	unsigned char send_buf[HEAD+PKT_LEN+CSUM];
	unsigned char recv_buf[16];

	struct timespec delta;
	struct opts opts;
	struct opts *op = NULL;

	signal(SIGINT, iap_interrupt);

	memset(&opts, 0, sizeof(struct opts));
	op = opts_init(&opts);

	while((opt = getopt(argc, argv, "hi:s:m:p:v")) != -1) {
		switch (opt) {
			case 'h':
				usage();
				break;
			case 'i':
				op->input = optarg;
				break;
			case 'p':
				op->output = optarg;
				break;
			case 's':
				op->speed = atoi(optarg);
				break;
			case 'm':
				op->mode = optarg;
				break;
			case 'v':
				iap_show_version();
				exit(0);
				break;
			default:
				printf("unknown option '%c'\n", opt);
				usage();
				break;
		}
	}

	/* Check if we're root */
	if (geteuid()) {
		fprintf(stderr, "Must be run as root\n");
		exit(ERR_PERMISSION);
	}

	if(argc < 3) {
		usage();
	}

	ret = opts_param_check(op);
	if(ret){
		usage();
	}

	/* Let us do the next */
	get_welcome_info();

	printf("########################################\n");
	printf("send file %s on %s\n", op->input, op->output);
	printf("########################################\n");

	serial = open(op->output, O_RDWR | O_NOCTTY | O_NDELAY);
	if(serial < 0) {
		printf("error opening '%s'\n", op->output);
		exit(ERR_OPEN_FAILED);
	}

	fcntl(serial, F_SETFL, 0);

	xmodem_configure_serial(serial, op->speed, op->mode);

	FILE * fin = fopen(op->input, "r");
	if(fin == NULL) {
		printf("error opening '%s'\n",op->input);
		close(serial);
		exit(ERR_OPEN_FAILED);
	}

	pkt = calc_file_packet(fin, PKT_LEN);
	if(pkt == 0){
		printf("file packet len is too small(%d)\n", pkt);
		fclose(fin);
		close(serial);
		return ERR_OPEN_FAILED;
	}

	printf("-- %d pkt to be sent\n", pkt);


	memset(send_buf, 0, sizeof(send_buf));
	send_buf[0] = 'u';
	write(serial, send_buf, 1);

	printf("-- wait for synchro\n");
	while(xmodem_running) {
		delta.tv_sec = 0;
		delta.tv_nsec = 1000;
		nanosleep(&delta, NULL);
		int size = read(serial, recv_buf, 1);
		if(size == 1) {
			if(recv_buf[0] == 'C') {
				break;
			}
		}
	}

	printf("-- got 0x%02x: start transfert\n", recv_buf[0]);

	while(!feof(fin)){
		int i;
		union {
			unsigned int checksum;
			unsigned char val[4];
		} csum;
		int size_out = 0;

		delta.tv_sec = 0;
		delta.tv_nsec = 1000;

		nanosleep(&delta, NULL);

		if(oldcpt < cpt) {
			retry = 0;
			memset(send_buf, 0x1A, sizeof(send_buf));

			send_buf[0] = X_SOH;
			send_buf[1] = cpt;
			send_buf[2] = 255 - cpt;

			if(0 == fread(send_buf + 3, 1, PKT_LEN, fin))
				break;

			csum.checksum = CRCXMODEM;
			for(i = 0; i < PKT_LEN; i++) {
				csum.checksum = xmodem_calculate_crc(csum.checksum, send_buf[i + HEAD]);
			}

			send_buf[PKT_LEN+HEAD] = csum.val[1];
			send_buf[PKT_LEN+HEAD+1] = csum.val[0];

			oldcpt++;
		}

		retry++;

		size_out = write(serial, send_buf, sizeof(send_buf));

		if(size_out != sizeof(send_buf)) {
			perror("send data:");
			printf("ERROR: sent %d/%ld bytes\n", size_out,
					sizeof(send_buf));
			exit(-1);
		} else {
			int total = 0;
			while(xmodem_running) {
				int size = read(serial, recv_buf, 1);
				total += size;
				if(total == 1) {
					if(recv_buf[0] == X_NAK) {
						printf("ERROR[%02d] NACK\n", retry);
						break;
					} else if(recv_buf[0] == X_ACK) {
						printf("OK [%4d/%-4d]\r", cpt, pkt);
						//phex(&send_buf[3], "a");
						fflush(stdout);
						cpt++;
						break;
					} else if(recv_buf[0] == X_CAN) {
						printf("Cancel,firmware unidentified!");
						break;
					} else {
						printf("WARN: received 0x%02x ", recv_buf[0]);
						break;
					}
				} else if(size != 0) {
					if(total % 20 == 1) {
						printf("WARN: received ");
					}
					printf("0x%02x ", recv_buf[0]);
					if(total % 20 == 0) {
						printf("\n");
					}
				}
			}
		}
	}
	printf("\n");

	printf("-- end of file\n");
	send_buf[0] = X_EOF;
	ret = write(serial, send_buf, 1);

	printf("-- EOT sent, ret = %d\n", ret);

    int size_ans = read(serial, recv_buf, 1);
    printf("-- EOT reply, size_ans = %d, recv_buf[0] = \n", size_ans, recv_buf[0]);
    
	if(size_ans == 1) {
		if(recv_buf[0] == X_NAK) {
			printf("ERROR NACK\n");
		} else if(recv_buf[0] == X_ACK) {
			printf("OK <EOT> ACKed\n");
		} else {
			printf("WARN: unexpected received 0x%02x ", recv_buf[0]);
		}
	}

	fclose(fin);

	printf("-- done\n");

	printf("########################################\n");
	printf("Transfert complete. Press ^C to exit\n");
	printf("########################################\n");

	xmodem_running = 0;
	sleep(1);

	while(xmodem_running) {
		int size = read(serial, send_buf, sizeof(send_buf));
		if(size > 0) {
			write(1, send_buf, size);
		}
	}

	close(serial);

	return 0;
}




