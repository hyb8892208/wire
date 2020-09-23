#if 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#include <limits.h>
#include <ctype.h>
#include <linux/ethtool.h>


#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/socket.h>
//#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

int get_netlink_status(const char * const if_name);

// if_name like "ath0", "eth0". Notice: call this function
// need root privilege.
// return value:
// -1 -- error , details can check errno
// 1 -- interface link up
// 0 -- interface link down.int get_netlink_status(const char * const if_name)
int get_netlink_status(const char * const if_name)
{
    int skfd;
    struct ifreq ifr;
    struct ethtool_value edata;

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_data = (char *) &edata;

    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
        return -1;

    if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1)
    {
        close(skfd);
        return -1;
    }

    close(skfd);
    return edata.data;
}
#endif


unsigned char g_eth_name[16];
unsigned char g_macaddr[16];
unsigned int g_subnetmask;
unsigned int g_ipaddr;
unsigned int g_broadcast_ipaddr;

#if 0
typedef unsigned int __u32;


#define ETHTOOL_GLINK		0x0000000a /* Get link status (ethtool_value) */

/* for passing single values */
struct ethtool_value {
	__u32	cmd;
	__u32	data;
};
#endif


void get_net_info(const char* ifname)
{
	int i ;
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

#if 0
	switch(get_netlink_status(ifname)) {		
	case 1:
		printf("Link\n");
		break;
	case 0:
		printf("Dislink\n");
		break;
	default:
		printf("Unknow\n");
		break;
	}
#endif 

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock == -1) {
		perror("socket");
		exit(-1);
	}

	strcpy(ifr.ifr_name, ifname);


	//获取Link or no link
	struct ethtool_value edata;
	edata.cmd = ETHTOOL_GLINK;
	edata.data = 0;
	ifr.ifr_data = (char *)&edata;
	if (ioctl(sock, SIOCETHTOOL, &ifr) < 0) {
		//perror("ioctl");
		//exit(1);
		printf("Unknow\n");
	} else {
		if(edata.data) {
			printf("Cable inserted\n");
		} else {
			printf("Cable not inserted\n");
		}
	}

	// Up or Down
	if ((ioctl(sock, SIOCGIFFLAGS, (char *) &ifr)) < 0 ) {
		//perror("ioctl");
		printf("\n");
	} else {
		if (ifr.ifr_flags & IFF_UP) {
			printf("Enable\n");
		} else {
			printf("Disable\n");
		}
	}


	//获取并打印网卡地址
	if(ioctl(sock,SIOCGIFHWADDR,&ifr)<0) {
		//perror("ioctl");
		printf("\n");
	} else {
		memcpy(g_macaddr,ifr.ifr_hwaddr.sa_data,6);
		for(i=0;i<5;i++)
			printf("%.2X:",g_macaddr[i]);
		printf("%.2X\n",g_macaddr[i]);
	}

	//获取并打印IP地址
	if(ioctl(sock,SIOCGIFADDR,&ifr)<0) {
		//perror("ioctl");
		printf("\n");
	} else {
		memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
		g_ipaddr=sin.sin_addr.s_addr;
		printf("%s\n",inet_ntoa(sin.sin_addr));
	}


	//获取并打印广播地址
	if(ioctl(sock,SIOCGIFBRDADDR,&ifr)<0) {
		//perror("ioctl");
		printf("\n");
	} else {
		memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
		g_broadcast_ipaddr=sin.sin_addr.s_addr;
		printf("%s\n",inet_ntoa(sin.sin_addr));
	}

	//获取并打印子网掩码
	if(ioctl(sock,SIOCGIFNETMASK,&ifr)<0) {
		//perror("ioctl");
		printf("\n");
	} else {
		memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
		g_subnetmask=sin.sin_addr.s_addr;
		printf("%s\n",inet_ntoa(sin.sin_addr));
	}

	close(sock);
}

static int to_num(char h, char l)
{
	int ret;
	switch(h) {
	case '0': ret = 16*0; break;
	case '1': ret = 16*1; break;
	case '2': ret = 16*2; break;
	case '3': ret = 16*3; break;
	case '4': ret = 16*4; break;
	case '5': ret = 16*5; break;
	case '6': ret = 16*6; break;
	case '7': ret = 16*7; break;
	case '8': ret = 16*8; break;
	case '9': ret = 16*9; break;
	case 'a': case 'A': ret = 16*10; break;
	case 'b': case 'B': ret = 16*11; break;
	case 'c': case 'C': ret = 16*12; break;
	case 'd': case 'D': ret = 16*13; break;
	case 'e': case 'E': ret = 16*14; break;
	case 'f': case 'F': ret = 16*15; break;
	default:
		return -1;									
	}

	switch(l) {
	case '0': ret = ret+0; break;
	case '1': ret = ret+1; break;
	case '2': ret = ret+2; break;
	case '3': ret = ret+3; break;
	case '4': ret = ret+4; break;
	case '5': ret = ret+5; break;
	case '6': ret = ret+6; break;
	case '7': ret = ret+7; break;
	case '8': ret = ret+8; break;
	case '9': ret = ret+9; break;
	case 'a': case 'A': ret = ret+10; break;
	case 'b': case 'B': ret = ret+11; break;
	case 'c': case 'C': ret = ret+12; break;
	case 'd': case 'D': ret = ret+13; break;
	case 'e': case 'E': ret = ret+14; break;
	case 'f': case 'F': ret = ret+15; break;
	default:
		return -1;									
	}

	return ret;
}

/*Parse default gateway from "/proc/net/route" */
int parse_defgw(const char* ifname, char* buf, int buf_len)
{
/*
Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT
eth0    0000FEA9        00000000        0001    0       0       1002    0000FFFF        0       0       0
eth0    000010AC        00000000        0001    0       0       0       0000FFFF        0       0       0
eth0    00000000        010010AC        0003    0       0       0       00000000        0       0       0
*/

	#define SPLIT_CHAR '\t'

	const char* route_file="/proc/net/route";
	FILE * fp;

	char line[256];
	int len;
	int i;
	char* find;
	char* pif;
	char* pdes;
	char* pgw;
	int dot1,dot2,dot3,dot4;

	fp=fopen(route_file,"r");
	if(fp == NULL) {
		return 0;
	} else {
		while(!feof(fp) && fgets(line,256,fp)) {
			pif=line;
			if((len = (find=strchr(pif,SPLIT_CHAR)) - pif) > 0) {
				if(memcmp(ifname,pif,len) == 0) {// Interface name
					while(*find == SPLIT_CHAR) find++;

					pdes=find;
					if((len = (find=strchr(pdes,SPLIT_CHAR)) - pdes) > 0) {
						if(len == 8 && memcmp("00000000",pdes,8) ==0 ) {// Default
							while(*find == SPLIT_CHAR) find++;

							pgw=find;
							if((len = (find=strchr(pgw,SPLIT_CHAR)) - pgw) > 0) {
								if(len == 8) {
									if(  (dot1 = to_num(pgw[0],pgw[1])) != -1 &&
										 (dot2 = to_num(pgw[2],pgw[3])) != -1 &&
										 (dot3 = to_num(pgw[4],pgw[5])) != -1 &&
										 (dot4 = to_num(pgw[6],pgw[7])) != -1
										 ) {
										snprintf(buf,buf_len,"%d.%d.%d.%d",dot4,dot3,dot2,dot1);
										return 1;
									}
								}
							}
						}
					}
				}
			}
		}

		fclose(fp);
	}

	#undef SPLIT_CHAR

	return 0;
}


/*Parse default gateway from " /proc/net/dev" */
/*
* print string:
* receive bytes 
* receive packets 
* transmit bytes 
* transmit packets 
*/
int parse_package(const char* ifname)
{
/*
	Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo:   11876      91    0    0    0     0          0         0    11876      91    0    0    0     0       0          0
  eth0:66063224  250586    0    0    0     0          0         0 153006220  185486    0    0    0     0       0          0
 */

	#define SPLIT_CHAR ' '

	const char* route_file="/proc/net/dev";
	FILE * fp;

	char rbytes[256];
	char rpackets[256];

	char tbytes[256];
	char tpackets[256];

	char line[1024];
	int len;
	int i;
	char* find;
	char* p;
	char* pif;
	char* start;
	char* prb;
	char* prp;
	char* ptb;
	char* ptp;
	int ret = 4;

	fp=fopen(route_file,"r");
	if(fp == NULL) {
		ret = 4;
		goto out;
	}

	while(!feof(fp) && fgets(line,1024,fp)) {
		start=line;
		while(*start==SPLIT_CHAR) start++;

		if((len = (find=strchr(start,':')) - start) > 0) {
			pif=start;
			if(memcmp(ifname,pif,len) == 0) {// Interface name

				find++; //Skip ':'

				while(*find == SPLIT_CHAR) find++;

				prb=find;
				if((len = (find=strchr(prb,SPLIT_CHAR)) - prb) > 0 && len < 256) {

					memcpy(rbytes,prb,len);
					rbytes[len] = '\0';
					printf("%s\n",rbytes);

					while(*find == SPLIT_CHAR) find++;

					prp=find;
					if((len = (find=strchr(prp,SPLIT_CHAR)) - prp) > 0  && len < 256) {

						memcpy(rpackets,prp,len);
						rpackets[len] = '\0';
						printf("%s\n",rpackets);

						//skip "errs drop fifo frame compressed multicast"
						for(i=0; i<6;i++) {
							while(*find == SPLIT_CHAR) find++;
							p=find;
							if((len = (find=strchr(p,SPLIT_CHAR)) - p) <= 0) {
								ret = 2;
								goto out;
							}
						}

						while(*find == SPLIT_CHAR) find++;
						ptb=find;
						if((len = (find=strchr(ptb,SPLIT_CHAR)) - ptb) > 0 && len < 256) {
							memcpy(tbytes,ptb,len);
							tbytes[len] = '\0';
							printf("%s\n",tbytes);

							while(*find == SPLIT_CHAR) find++;
							ptp=find;
							if((len = (find=strchr(ptp,SPLIT_CHAR)) - ptp) > 0  && len < 256 ) {

								memcpy(tpackets,ptp,len);
								tpackets[len] = '\0';
								printf("%s\n",tpackets);

								ret = 0;
								goto out;
							} else {
								ret = 1;
								goto out;
							}
						} else {
							ret = 2;
							goto out;
						}
					} else {
						ret = 3;
						goto out;
					}
				}
			}
		}
	}

out:
	for(i=0; i<ret; i++) {
		printf("\n");
	}
	fclose(fp);

	return ret;

	#undef SPLIT_CHAR
}


int main(int argc, char* argv[])
{
	char buf[20];

	if(argc != 2) {
		printf("net_tools <interface name>\n");
		exit(-1);
	}

	get_net_info(argv[1]);

	if(parse_defgw(argv[1], buf, 20))
		printf("%s\n",buf);
	else
		printf("\n");

	parse_package(argv[1]);

	return 0;
}


#endif


#if 0
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
 
#define MAXINTERFACES 16
 
int main(int argc, char **argv)
{
    register int fd, interface, retn = 0;
    struct ifreq buf[MAXINTERFACES];
    struct arpreq arp;
    struct ifconf ifc;
    char mac[32] = "";
 
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *) &ifc)) {
            interface = ifc.ifc_len / sizeof(struct ifreq);
            printf("interface num is interface=%d\n\n", interface);
            while (interface-- > 0) {
                printf("net device %s\n", buf[interface].ifr_name);
 
				/*Jugde whether the net card status is promisc */
                if (!(ioctl(fd, SIOCGIFFLAGS, (char *) &buf[interface]))) {
                    if (buf[interface].ifr_flags & IFF_PROMISC) {
                        printf("the interface is PROMISC");
                        retn ;
                    }
                } else {
                    char str[256] = "";
 
                    sprintf(str, "cpm: ioctl device %s",
                            buf[interface].ifr_name);
                    perror(str);
                }
 
				/*Jugde whether the net card status is up */
                if (buf[interface].ifr_flags & IFF_UP) {
                    printf("the interface status is UP\n");
                } else {
                    printf("the interface status is DOWN\n");
                }
 
/*Get IP of the net card */
                if (!(ioctl(fd, SIOCGIFADDR, (char *) &buf[interface]))) {
                    printf("IP address is:");
                    printf("%s\n",
                           inet_ntoa(((struct sockaddr_in
                                       *) (&buf[interface].ifr_addr))->
                                     sin_addr));
                } else {
                    char str[256] = "";
 
                    sprintf(str, "cpm: ioctl device %s",
                            buf[interface].ifr_name);
                    perror(str);
                }
 
/*Get HW ADDRESS of the net card */
                if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[interface]))) {
                    printf("HW address is:");
 
                    sprintf(mac, "%02x%02x%02x%02x%02x%02x",
                            (unsigned char) buf[interface].ifr_hwaddr.
                            sa_data[0],
                            (unsigned char) buf[interface].ifr_hwaddr.
                            sa_data[1],
                            (unsigned char) buf[interface].ifr_hwaddr.
                            sa_data[2],
                            (unsigned char) buf[interface].ifr_hwaddr.
                            sa_data[3],
                            (unsigned char) buf[interface].ifr_hwaddr.
                            sa_data[4],
                            (unsigned char) buf[interface].ifr_hwaddr.
                            sa_data[5]); // 利用sprintf转换成char *
                    printf("%s\n", mac);
 
                    printf("\n");
                }
 
                else {
                    char str[256];
 
                    sprintf(str, "cpm: ioctl device %s",
                            buf[interface].ifr_name);
                    perror(str);
                }
            }                   //end of while
        } else
            perror("cpm: ioctl");
 
    } else
        perror("cpm: socket");
 
    close(fd);
    return retn;
}
#endif
