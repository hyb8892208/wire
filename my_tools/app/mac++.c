#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <assert.h>

#define MAC_BUF_LEN  20

// mac µØÖ·×ÔÔö
// macaddr = 00:11:22:33:44:55:66
// step = 1
void getnextmac_by_step(char *macaddr, int step)
{
        unsigned long long mac = 0;
        unsigned long long tmpmac;
        unsigned char *pmac =(unsigned char *) &mac;
        unsigned char i, swap;
        char tmpstr[20], *token;
        int tokencount = 0;

        token = strtok(macaddr, ":");
        while (token) {
                sprintf(tmpstr, "0x%s", token);
                pmac[tokencount++] = strtol(tmpstr, NULL, 16);
                token = strtok(NULL, ":");
        }
        assert(tokencount == 6);
        for (i = 0; i < 3; i++) {
                swap = pmac[i];
                pmac[i] = pmac[6 - i - 1];
                pmac[6 - i - 1] = swap;
        }

        mac += step;
        sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[5], pmac[4], pmac[3], pmac[2], pmac[1], pmac[0]);
        return;
}

int main(int argc, char *argv[])
{
    int fd;
    int ret;
    char *file_path;
    char mac[MAC_BUF_LEN];

    if ( argc < 2 )
    {
        printf("Usage:\r\nmac++ filename\r\n");
        return -1;
    }

    file_path = argv[1];

    fd = open(argv[1], O_RDWR);
    if ( fd < 0 )
    {
        printf("open %s fail!\r\n", file_path);
        return -1;
    }

    memset(mac, 0, MAC_BUF_LEN);
    ret = read(fd, mac, MAC_BUF_LEN);
    if ( ret <= 0 )
    {
        printf("read %s fail!\r\n", file_path);
        return -1;
    }

    getnextmac_by_step(mac, 1);

    lseek(fd, 0, SEEK_SET);

    (void)write(fd, mac, strlen(mac));

    close(fd);

    return 0;
}













