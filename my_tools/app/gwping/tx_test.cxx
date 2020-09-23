
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
#include <uuid/uuid.h>

#include <string>
using namespace std;

#include "config.h"
#include "RawEthFrame.hpp"
#include "RawEthSocket.hpp"

char* ifName = "eth0";
unsigned char bcastmac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


int main(int argc, char *argv[])
{
    int ret;
    short eth_p = _ETHER_TYPE_OPVX;

    TxEtherSocket  txSocket;
    RawEtherFrame frame;

    if(argc > 1)
        ifName = strdup(argv[1]);

    ret = txSocket.Open((unsigned char*)ifName);
    if(ret<0) {
        printf("Open txSocket on ifName %s, failed(%d)\n", ifName, ret);
        exit(-1);
    }
    
    EthBufCoder cd(frame);
    char* cmd = _ETH_SVC_PING;
    short msgsize = strlen(_ETH_SVC_PING) + 1;
    uuid_t uu;
    uuid_generate(uu);
    EtherMac mac(bcastmac);
    char* p = _ETH_SEND_HEAD_FLAG;
    
    cd.EncodeBuf((unsigned char*)p, strlen(p));
    cd.EncodeWord(msgsize);
    cd.EncodeBuf(uu, sizeof(uuid_t));
    cd.EncodeCStr((unsigned char*)cmd);

    int i = cd.GetDataSize();
    frame.SetDataSize(i);
    frame.SetType(_ETHER_TYPE_OPVX);

    txSocket.SendTo(mac, frame);
}
