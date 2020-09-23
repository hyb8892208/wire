/*
 * Minimalistic implementation of the XModem/YModem protocol suite, including
 * a compact version of an CRC16 algorithm. The code is just enough to upload
 * an image to a MCU that bootstraps itself over an UART.
 *
 * Copyright (c) 2014 Daniel Mack <github@zonque.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define X_STX 0x02
#define X_ACK 0x06
#define X_NAK 0x15
#define X_EOF 0x04

#define min(a, b)       ((a) < (b) ? (a) : (b))

struct xmodem_chunk {
        uint8_t start;
        uint8_t block;
        uint8_t block_neg;
        uint8_t payload[1024];
        uint16_t crc;
} __attribute__((packed));

#define CRC_POLY 0x1021

static uint16_t crc_update(uint16_t crc_in, int incr)
{
        uint16_t xor = crc_in >> 15;
        uint16_t out = crc_in << 1;

        if (incr)
                out++;

        if (xor)
                out ^= CRC_POLY;

        return out;
}

static uint16_t crc16(const uint8_t *data, uint16_t size)
{
        uint16_t crc, i;

        for (crc = 0; size > 0; size--, data++)
                for (i = 0x80; i; i >>= 1)
                        crc = crc_update(crc, *data & i);

        for (i = 0; i < 16; i++)
                crc = crc_update(crc, 0);

        return crc;
}
static uint16_t swap16(uint16_t in)
{
        return (in >> 8) | ((in & 0xff) << 8);
}

enum {
        PROTOCOL_XMODEM,
        PROTOCOL_YMODEM,
};

static int xymodem_send(int serial_fd, const char *filename, int protocol, int wait)
{
	printf("11111\n");
        size_t len;
        int ret, fd;
        uint8_t answer;
        struct stat stat;
        const uint8_t *buf;
        uint8_t eof = X_EOF;
        struct xmodem_chunk chunk;
        int skip_payload = 0;

        fd = open(filename, O_RDONLY);
        if (fd < 0) {
                perror("open");
                return -errno;
        }

		
	printf("122221\n");
        fstat(fd, &stat);
        len = stat.st_size;
        buf = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        if (!buf) {
                perror("mmap");
                return -errno;
        }

	/*
        if (wait) {
                printf("Waiting for receiver ping ...");
                fflush(stdout);

                do {
                        ret = read(serial_fd, &answer, sizeof(answer));
                        if (ret != sizeof(answer)) {
                                perror("read");
                                return -errno;
                        }
                } while (answer != 'C');

                printf("done.\n");
        }*/

        printf("Sending %s ", filename);

        if (protocol == PROTOCOL_YMODEM) {
                strncpy((char *) chunk.payload, filename, sizeof(chunk.payload));
                chunk.block = 0;
                skip_payload = 1;
        } else {
                chunk.block = 1;
        }

        chunk.start = X_STX;
	printf("len : %ld\n", len);
        while (len) {
                size_t z = 0;
                int next = 0;
                char status;

                if (!skip_payload) {
                        z = min(len, sizeof(chunk.payload));
                        memcpy(chunk.payload, buf, z);
                        memset(chunk.payload + z, 0xff, sizeof(chunk.payload) - z);
                } else {
                        skip_payload = 0;
                }

                chunk.crc = swap16(crc16(chunk.payload, sizeof(chunk.payload)));
                chunk.block_neg = 0xff - chunk.block;
		printf("z = %ld\n", z);
                ret = write(serial_fd, &chunk, sizeof(chunk));
		printf("chunk : %s\n", chunk.payload);
                if (ret != sizeof(chunk))
                        return -errno;
		
                ret = read(serial_fd, &answer, sizeof(answer));		
                if (ret != sizeof(answer))
                        return -errno;
				
		printf("answer :0x%x\n", answer);
		
                switch (answer) {
                case X_NAK:
                        status = 'N';
                        break;
                case X_ACK:
                        status = '.';
                        next = 1;
                        break;
                default:
                        status = '?';
                        break;
                }

                printf("%c", status);
                fflush(stdout);

                if (next) {
                        chunk.block++;
                        len -= z;
                        buf += z;
                }
        }

        ret = write(serial_fd, &eof, sizeof(eof));
        if (ret != sizeof(eof))
                return -errno;

        /* send EOT again for YMODEM */
        if (protocol == PROTOCOL_YMODEM) {
                ret = write(serial_fd, &eof, sizeof(eof));
                if (ret != sizeof(eof))
                        return -errno;
        }

        printf("done.\n");

        return 0;
}

static int open_serial(const char *path, int baud)
{
        int fd;
        struct termios tty;

        fd = open(path, O_RDWR | O_SYNC);
        if (fd < 0) {
                perror("open");
                return -errno;
        }

        memset(&tty, 0, sizeof(tty));
        if (tcgetattr(fd, &tty) != 0) {
                perror("tcgetattr");
                return -errno;
        }

        cfsetospeed(&tty, baud);
        cfsetispeed(&tty, baud);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        tty.c_iflag &= ~IGNBRK;                         // disable break processing
        tty.c_lflag = 0;                                // no signaling chars, no echo,
                                                        // no canonical processing
        tty.c_oflag = 0;                                // no remapping, no delays
        tty.c_cc[VMIN]  = 1;                            // read doesn't block
        tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
                                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
                perror("tcsetattr");
                return -errno;
        }

        return fd;
}

static void dump_serial(int serial_fd)
{
        char in;

        for (;;) {
                read(serial_fd, &in, sizeof(in));
                printf("%c", in);
                fflush(stdout);
        }
}

int main(int argc, char **argv)
{
        int a, ret, serial_fd;

        serial_fd = open_serial("/dev/ttyACM0", 115200);
        if (serial_fd < 0)
                return -errno;
	char str[10] = "1";

	 ret = write(serial_fd, str, sizeof(str));
	uint8_t answer;
/*
	while(1){
		ret = read(serial_fd, &answer, sizeof(answer));
                if (ret != sizeof(answer))
                        return -errno;
		else{
			printf("answer : %c\n",answer);
		}

	}
*/
             

       ret = xymodem_send(serial_fd, argv[1], PROTOCOL_XMODEM, 0);
        if (ret < 0)
                return ret;
	
        /*ret = xymodem_send(serial_fd, argv[2], PROTOCOL_YMODEM, 1);
        if (ret < 0)
                return ret;

        sleep(4);

        ret = xymodem_send(serial_fd, argv[1], PROTOCOL_YMODEM, 0);
        if (ret < 0)
                return ret;

        sleep(3);

        ret = xymodem_send(serial_fd, argv[2], PROTOCOL_YMODEM, 0);
        if (ret < 0)
                return ret;
*/
        dump_serial(serial_fd);

        return 0;
}
