
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


//======================================================================================
// class TxEtherSocket
//======================================================================================
TxEtherSocket::TxEtherSocket(void)
 : _ifName(""), _myMac(), _sockfd(0), _isOpen(false), _ifIndex(0), _debug(0)
{
}

int TxEtherSocket::Open(unsigned char *ifName)
{
    int retval = 0;
    int fd = 0;
    struct ifreq if_idx;        /* index of the eth */
	struct ifreq if_mac;        /* mac address of eth */
    char mac[ETH_ALEN];

    assert(ifName);

    if ((fd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        if(_debug)
            esvc_log(ESVC_ERR "Can not create raw socket\n");
        retval = -1;
        goto open_exit;
	}

   	/* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, (const char*)ifName, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFINDEX, &if_idx) < 0) {
        if(_debug)
            esvc_log(ESVC_ERR "SIOCGIFINDEX\n");
        retval = -2;
        goto open_exit;
    }
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, (const char*)ifName, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &if_mac) < 0) {
        if(_debug)
            esvc_log(ESVC_ERR "open_eth_socket: SIOCGIFHWADDR");
        retval=-3;
        goto open_exit;
    }

    _ifName = (const char*)ifName;
    _isOpen = true;
    _sockfd = fd;
    mac[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	mac[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	mac[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	mac[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	mac[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	mac[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
    _myMac.SetMac((const unsigned char*)mac);
    _ifIndex = if_idx.ifr_ifindex;

open_exit:
    return retval;
}

void TxEtherSocket::Close(void)
{
    if(_isOpen){
        _myMac = EtherMac();
        _ifName = "";
        close(_sockfd);
        _sockfd = 0;
        _isOpen = false;
        _ifIndex = 0;
    }
}

int TxEtherSocket::SendTo(EtherMac& dstMac, RawEtherFrame& frame)
{
    int retval = 0;
   	int tx_len = 0;
	struct sockaddr_ll socket_address;
    unsigned char* p;

    frame.SetDestMac(dstMac);
    frame.SetSrcMac(_myMac);

    /* Index of the network device */
	socket_address.sll_ifindex = _ifIndex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
    p = dstMac.GetMac();
    socket_address.sll_addr[0] = p[0];
	socket_address.sll_addr[1] = p[1];
	socket_address.sll_addr[2] = p[2];
	socket_address.sll_addr[3] = p[3];
	socket_address.sll_addr[4] = p[4];
	socket_address.sll_addr[5] = p[5];

	/* Send packet */
    if (sendto(_sockfd, frame.RawBuf(), frame.GetFrameSize(), 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
	    if(_debug)
            esvc_log(ESVC_ERR "sendto failed \n");
        retval = -1;
    }

    return retval;
}

int TxEtherSocket::Send(RawEtherFrame& frame)
{
    int retval = 0;
   	int tx_len = 0;
	struct sockaddr_ll socket_address;
    unsigned char* p;

    frame.SetSrcMac(_myMac);

    /* Index of the network device */
	socket_address.sll_ifindex = _ifIndex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
    p = frame.GetDestMac().GetMac();
    socket_address.sll_addr[0] = p[0];
	socket_address.sll_addr[1] = p[1];
	socket_address.sll_addr[2] = p[2];
	socket_address.sll_addr[3] = p[3];
	socket_address.sll_addr[4] = p[4];
	socket_address.sll_addr[5] = p[5];

	/* Send packet */
    if (sendto(_sockfd, frame.RawBuf(), frame.GetFrameSize(), 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
	    if(_debug)
            esvc_log(ESVC_ERR "sendto failed \n");
        retval = -1;
    }

    return retval;
}

//======================================================================================
// class RxEtherSocket
//======================================================================================
RxEtherSocket::RxEtherSocket(void)
 : _ifName(""), _myMac(), _sockfd(0), _isOpen(false), _ethType(0), _debug(0)
{
}

int RxEtherSocket::Open(unsigned char* ifName, unsigned short eth_p)
{
   int retval = 0;
    struct ifreq ifopts;	/* set promiscuous mode */
    int sockopt;
    int sockfd;

    /* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(eth_p))) == -1) {
		if(_debug)
            esvc_log(ESVC_ERR "open socket error\n");	
		retval=-1;
        goto open_rx_exit;
	}

	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, (const char*)ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		if(_debug)
            esvc_log(ESVC_ERR "setsockopt\n");
		close(sockfd);
		retval = -2;
	} else 
        retval = sockfd;

    /* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		if(_debug)
            esvc_log(ESVC_ERR "SO_BINDTODEVICE\n");
		close(sockfd);
		retval = -3;
	}

    _ethType = eth_p;
    _sockfd = sockfd;
    _ifName = (const char*)ifName;
    _isOpen = true;
 
open_rx_exit:
    return retval;
}

void RxEtherSocket::Close(void)
{
    if(_isOpen){
        _myMac = EtherMac();
        _ifName = "";
        close(_sockfd);
        _sockfd = 0;
        _isOpen = false;
        _ethType = 0;
    }
}

int RxEtherSocket::Recv(RawEtherFrame& frame)
{
    int numbytes = 0;
    
    if(!_isOpen)
        return 0;

    /* wait recv , block caller */
    numbytes = recvfrom(_sockfd, frame.RawBuf(), frame.GetMaxFrameSize(), 0, NULL, NULL);
    if(numbytes==-1) {
        if(_debug)
            esvc_log(ESVC_ERR "recvfrom error -1\n");
        return -1;
    }
    if(_debug)
        esvc_log(ESVC_INFO "%d bytes received\n", numbytes);

    frame.SetFrameSize(numbytes);
    return numbytes;
}

