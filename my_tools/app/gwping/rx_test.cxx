
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>       
using namespace std;

#include "config.h"
#include "RawEthFrame.hpp"
#include "RawEthSocket.hpp"

char* ifName = "eth0";

int main(int argc, char *argv[])
{
    int ret;
    short eth_p = _ETHER_TYPE_OPVX;

    RxEtherSocket  rxSocket;
    RawEtherFrame frame;

    if(argc > 1)
        ifName = strdup(argv[1]);

    ret = rxSocket.Open((unsigned char*)ifName, eth_p);
    if(ret<0) {
        printf("Open rxSocket on ifName %s, type 0x%04x failed(%d)\n", ifName, eth_p, ret);
        exit(-1);
    }
    
    ret = rxSocket.Recv(frame);

    if(ret>0)
        printf("%d bytes received\n");
    else {
        printf("rxSocket.Recv error %d\n", ret);
        exit(-2);
    }

}
