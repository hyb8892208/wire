/**************************************************************************
 * Copyright (c) 2009-2017 OpenVox Communication Co.,Ltd.
 * All Rights Reserved.	
 * Author: Santmoriz.Chiu <Santmoriz.Chiu@openvox.cn>
 * 
 * This file is part of xmodem.
 * 
 * xmodem is free software: you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * xmodem is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with xmodem. If not, see http://www.gnu.org/licenses/.
 *
 ***************************************************************************/
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include "xmodem.h"

int xmodem_calculate_crc(unsigned int crc, unsigned char data)
{
	crc  = (unsigned char)(crc >> 8) | (crc << 8);
	crc ^= data;
	crc ^= (unsigned char)(crc & 0xff) >> 4;
	crc ^= (crc << 8) << 4;
	crc ^= ((crc & 0xff) << 4) << 1;
	return crc;
}


void xmodem_configure_serial(int fd, int speed, char * mode)
{
	struct termios options;

	tcgetattr(fd, &options);
	options.c_cflag |= (CLOCAL | CREAD);

	switch(speed) {
		case 1200:
			cfsetispeed(&options, B1200);
			break;
		case 1800:
			cfsetispeed(&options, B1800);
			break;
		case 2400:
			cfsetispeed(&options, B2400);
			break;
		case 4800:
			cfsetispeed(&options, B4800);
			break;
		case 9600:
			cfsetispeed(&options, B9600);
			break;
		case 19200:
			cfsetispeed(&options, B19200);
			break;
		case 38400:
			cfsetispeed(&options, B38400);
			break;
		case 57600:
			cfsetispeed(&options, B57600);
			break;
		case 230400:
			cfsetispeed(&options, B230400);
			break;
		case 115200:
		default:
			cfsetispeed(&options, B115200);
			break;
	}

	options.c_cflag &= ~CSIZE;
	switch(mode[0]) {
		case '5':
			options.c_cflag |= CS5;
			break;
		case '6':
			options.c_cflag |= CS6;
			break;
		case '7':
			options.c_cflag |= CS7;
			break;
		case '8':
		default:
			options.c_cflag |= CS8;
		break;
	}

	switch(mode[1]) {
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			break;
		case 'O':
			options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;
			break;
		case 'N':
			options.c_cflag &= ~PARENB;
		default:
			break;
	}

	switch(mode[2]) {
		case '1':
			options.c_cflag &= ~CSTOPB;
			break;
		case '2':
		default:
			options.c_cflag |= CSTOPB;
			break;
	}
	
	 /* make sure in raw mode, 2017-05-25 21:16 */
        options.c_cflag &= ~CLOCAL;

        /* Input */
        options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);

        /*Change output mode in raw mode */
        options.c_oflag  &= ~OPOST;

        /* close software control */
        options.c_cflag &= ~IXON;


	tcsetattr(fd, TCSANOW, &options);
}


int calc_file_packet(FILE *fp, int pkt_len)
{
	int nb = 0;
	int pkt = 0;
	
	if(!fp){
		return pkt;
	}

	fseek(fp, 0, SEEK_END);
	nb = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	pkt = nb/PKT_LEN + ((nb % PKT_LEN) ? 1 : 0); 

	return pkt;
}


