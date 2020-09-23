
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
#include <poll.h>


#include <string>
#include <vector>
using namespace std;

#include "config.h"
#include "RawEthFrame.hpp"
#include "RawEthSocket.hpp"

char* ifName = "eth0";

string g_Cmd;
char* CMD_PING = "list";
EtherMac dstMac;
int debug = 0;

unsigned char bcastmac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

bool checkmac(char* input, EtherMac& mac)
{
    int i, j, k;
    char buf[12];

    i = strlen(input);
    if( (i!=12) && (i!=17) )
        return false;

    if(i==17) {
        for(j=0, k=0; j<17; j++) {
            if( j!=2 && j!=5 && j!=8 && j!=11 && j!= 14)  {
                buf[k] = input[j];
                k++;
            }
        }
    } else 
        memcpy(buf, input, 12);

    for(j=0; j<12; j++)
        if( isxdigit(buf[i] == 0) )
            break;
    if(j!=12)
        return false;

	//Freedom Add 2013-10-16 14:00
	/////////////////////////////////////////////////////////////
	unsigned char hexmac[6];
	unsigned char t1,t2;

	for(j=0; j<12; j+=2) {
		if(buf[j] >= '0' && buf[j] <= '9') {
			t1 = buf[j] - '0';
		} else if(buf[j] >= 'a' && buf[j] <= 'z') {
			t1 = buf[j] - 'a' + 0xa;
		} else if(buf[j] >= 'A' && buf[j] <= 'Z') {
			t1 = buf[j] - 'A' + 0xa;
		} else {
			return false;
		}

		if(buf[j+1] >= '0' && buf[j+1] <= '9') {
			t2 = buf[j+1] - '0';
		} else if(buf[j+1] >= 'a' && buf[j+1] <= 'z') {
			t2 = buf[j+1] - 'a' + 0xa;
		} else if(buf[j+1] >= 'A' && buf[j+1] <= 'Z') {
			t2 = buf[j+1] - 'A' + 0xa;
		} else {
			return false;
		}

		hexmac[j/2] = t1 * 16 + t2;
	}
	mac.SetMac(hexmac);
	/////////////////////////////////////////////////////////////
	//mac.SetMac((unsigned char*)buf);

    return true;
}

int checkparam(int argc, char* argv[])
{
    char* mac = NULL;

    if( (argc != 3) && (argc != 4) ) {
        printf("Usage:\n");
        printf("\tgwping [ethx] [cmd] <dstmac> \n");
        printf("Where: \n\t[ethx] is the name of ethernet interface used.\n");
        printf("\t[cmd] is the name of command with param\n");
        printf("\t\t if [cmd] is \'list\', it will list all the server in the lan\n");
        printf("\t\t other [cmd] is user defined\n");
        printf("\t<dstmac> is mac of target gateway, must be available whe cmd is other than list\n");
        printf("Sample:\n\tgwping eth0 list\n\t--expact list all the server in the lan\n");
        printf("Sample:\n\tgwping eth0 \"usercmd userparm\" 8e354d098701\n\t--expact exexute usercmd userparam on mac 8e354d098701\n");
        return -1;
    }

    if(argc == 3) {   // must be list cmd
        if( strcmp(argv[2], CMD_PING) != 0) {
            if(debug)
                printf("invalid cmd, 2nd parm must be %s or <dstmac> missed\n", CMD_PING);
            return -2;
        }
        g_Cmd = argv[2];
    } else if(argc==4) {
        g_Cmd = argv[2];
        mac = argv[3];
    }
    
    if(mac) {
        if(checkmac(mac, dstMac))
            return 0;
        else
            return -2;
    } else
        return 0;
}

RxEtherSocket  rxSocket;
uuid_t uu;
static volatile int rxThreadReady = 0;
static int ThreadExit = 0;
typedef vector<RawEtherFrame*> TFrameList;
TFrameList frameList;

int isValidPacket(RawEtherFrame& frame)
{
    struct RawSvcHead head;
    int size;

    size = frame.GetFrameSize();
    if( size < (ETH_HLEN + sizeof(struct RawSvcHead)) ) {
        if(debug)
            printf("Packet size(%d) < minimal size(%d)\n", size, ETH_HLEN + sizeof(RawSvcHead));
        return -1;
    }

    EthBufDecoder d(frame);
    d.DecodeBuf( (unsigned char*)&head, sizeof(head) );
    // first 4 bytes after ethernet head must be 'OPVX'
    char *f = _ETH_RECV_HEAD_FLAG;
    char *p = head.flag;
    if( !(p[0]==f[0] && p[1]==f[1] && p[2]==f[2] && p[3]==f[3]) ) {
        if(debug)
            printf("Tag field \'%c%c%c%c\' != %s\n", p[0], p[1], p[2], p[3], _ETH_RECV_HEAD_FLAG);
        return -2;
    }

    /*size = ntohs(head.size);
    if(size != (frame.GetDataSize() - sizeof(head))) {
        if(debug)
            printf("Size field %d != actual size %d\n", size, frame.GetFrameSize() - sizeof(head) );
        return -3;
    }*/

    return 0;
}

struct TServerReturn 
{
public:
    EtherMac    _serverMac;
    uuid_t      _uu;
    string      _result;
    bool        _duplicate;
};
typedef vector<TServerReturn> TServerReturnList;
TServerReturnList srList;

int main(int argc, char *argv[])
{
    int ret;
    short eth_p = _ETHER_TYPE_OPVX;

    TxEtherSocket  txSocket;
    ret = checkparam(argc, argv);
    if(ret)
        exit(-1);

    ifName = argv[1];
    ret = txSocket.Open((unsigned char*)ifName);
    if(ret<0) {
        printf("Open txSocket on ifName %s, failed(%d)\n", ifName, ret);
        exit(-2);
    }
    ret = rxSocket.Open((unsigned char*)ifName, _ETHER_TYPE_OPVX);
    if(ret<0) {
        printf("Open rxSocket on ifName %s, failed(%d)\n", ifName, ret);
        txSocket.Close();
        exit(-3);
    }

    time_t tm = time(NULL);
    int timeout;
    EtherMac bmac(bcastmac);

    if(g_Cmd == "list") {
        if(debug)
            esvc_log(ESVC_INFO "Cmd is %s\n", g_Cmd.c_str());
        timeout = _PING_WAIT_TIME;
        dstMac = bmac;
        g_Cmd = _ETH_SVC_PING;
    }
    else {
        timeout = _CMD_WAIT_TIME;
    }

    RawEtherFrame txFrame;
    EthBufCoder cd(txFrame);
    
    uuid_generate(uu);
    
    char* p = _ETH_SEND_HEAD_FLAG;
    
    cd.EncodeBuf((unsigned char*)p, strlen(p));
    cd.EncodeWord(g_Cmd.length()+1);
    cd.EncodeBuf(uu, sizeof(uuid_t));
    cd.EncodeCStr((unsigned char*)g_Cmd.c_str());

    int i = cd.GetDataSize();
    txFrame.SetDataSize(i);
    txFrame.SetType(_ETHER_TYPE_OPVX);
    txSocket.SendTo(dstMac, txFrame);

    struct pollfd fds[1];
    fds[0].fd = rxSocket.Sockfd();
    fds[0].events = POLLIN;

    while(difftime(time(NULL), tm) < timeout) {
        int poll_ret = poll(fds, 1, 1000);
        if( poll_ret < 0) {
            if(debug)
                printf("Poll rx socket error\n");
            exit(-3);
        } 
        
        if (poll_ret == 0) {
            if(debug)
                printf("Poll timeout\n");
            continue;
        }
        
        RawEtherFrame rxFrame;
        int rxSize = 0;
        if(fds[0].revents)
            rxSize = rxSocket.Recv(rxFrame);
        else
            continue;

        if(debug)
            printf("Got a Packet\n");

        // retrive infromation received.
        i = isValidPacket(rxFrame);
        if(i==0) {
            if( !rxFrame.GetDestMac().isBroadCast()) {
                if(debug)
                    printf("Got a None BroadCast Packet\n");
                TServerReturn sr;
                EthBufDecoder dc(rxFrame);
                RawSvcHead rxhead;
                int rxlen;
                unsigned char buf[ETH_DATA_LEN];
                int bufsize = sizeof(buf);

                dc.DecodeBuf((unsigned char*)&rxhead, sizeof(rxhead));
                if( memcmp(rxhead.unique_id, uu, sizeof(uu))!=0 ) {
                    if(debug)
                        printf("Not my cmd, discard\n");
                    continue;
                }

                sr._serverMac = rxFrame.GetSrcMac();
                memcpy(sr._uu, rxhead.unique_id, sizeof(rxhead.unique_id));
                rxlen = ntohs(rxhead.size);
                
                if(g_Cmd != _ETH_SVC_PING) {
                    memset(buf, 0, sizeof(buf));
                    dc.DecodeCStr(buf, bufsize);
                    printf("%s\n", buf);
                    txSocket.Close();
                    rxSocket.Close();
                    exit(0);
                }
                
                while(1) {
                    i = dc.DecodeCStr(buf, bufsize);
                    if(i) {
                        if(sr._result.length())
                            sr._result += (const char*)"\n";
                        sr._result += (const char*)buf;
                    } else 
                        break;
                }
                
                if(debug)
                    printf("response from %s\n", rxFrame.GetSrcMac().AsString().c_str());
                sr._duplicate = false;
                srList.push_back(sr);
            } else
                if(debug)
                    printf("Get a BroadCast from %s\n", rxFrame.GetSrcMac().AsString().c_str());
        } else
            if(debug)
                printf("Get a Invalid frame from %s\n", rxFrame.GetSrcMac().AsString().c_str());
    } 

    if(debug)
        printf("Start decode result\n");

    TServerReturnList::iterator iter = srList.begin();
    //remove duplicate 
    while(iter!=srList.end()) {
        TServerReturnList::iterator iter1 = iter;
        EtherMac em;
        em = iter->_serverMac;
        iter1++;
        while(iter1!=srList.end()) {
            if(iter1->_serverMac.isEqual(em))
                iter1->_duplicate = true;
            iter1++;
        }
        iter++;
    }

    for(iter = srList.begin(); iter != srList.end(); iter++) {
        if( !iter->_duplicate) {
            printf("$Server %s\n", iter->_serverMac.AsString().c_str());
            printf( "%s\n------\n", iter->_result.c_str());
        }
    }

    rxSocket.Close();
    txSocket.Close();
}
