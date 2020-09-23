#include <sys/types.h>   
#include <sys/socket.h>   
#include <asm/types.h>   
#include <linux/netlink.h>   
#include <linux/rtnetlink.h>   
#include <stdlib.h>   
#include <stdio.h>   
#include <sys/ioctl.h>   
#include <linux/if.h>   
#include <string.h>   
#include <unistd.h>
#include <signal.h> 
#include <sys/file.h>
#include <syslog.h>
  
#define BUFLEN 20480
#define VERSION "1.0.0"  

static int main_eth_flag=0;
static int extra_eth_flag=0;
int main(int argc, char *argv[])   
{   
	int fd, retval;   
	char buf[BUFLEN] = {0};   
	int len = BUFLEN;   
	struct sockaddr_nl addr;   
	struct nlmsghdr *nh;   
	struct ifinfomsg *ifinfo;   
	struct rtattr *attr;   
	char *main_eth=NULL;
	char *main_up_script=NULL;
	char *main_down_script=NULL;
	char *extra_eth=NULL;
	char *extra_up_script=NULL;
	char *extra_down_script=NULL;
	int daemonize=0;
	int ch;
	
	while ((ch = getopt(argc, argv, "i:u:d:e:m:o:sv")) != -1)
	{
        switch (ch) {
        case 'i':
            main_eth = optarg;
            break;
        case 'u':
            main_up_script = optarg;
            break;
		case 'd':
			main_down_script = optarg;
			break;
		case 'e':
			extra_eth = optarg;
			break;
		case 'm':
			extra_up_script=optarg;
			break;
        case 'o':
            extra_down_script = optarg;
            break;
		case 's':
			daemonize=1;
			break;
		case 'v':
			printf("networl interface minitor version:%s\n",VERSION);
			return 0;
		case '?':
			printf("Unknown option: %c\n",(char)optopt);
			break;
		}
	}
	if(!main_eth||!main_up_script||!main_down_script)
	{
		main_eth=NULL;
		main_up_script=NULL;
		main_down_script=NULL;
	}
	if(!extra_eth||!extra_up_script||!extra_down_script)
	{
		extra_eth=NULL;
		extra_up_script=NULL;
		extra_down_script=NULL;
	}
	if(!extra_eth&&!main_eth)
	{
		printf("mast set one network interface info\n");
		return 0;
	}
	if (daemonize)
	{
		pid_t pid;
		int fdtablesize;
		signal(SIGTTOU, SIG_IGN); 
		signal(SIGTTIN, SIG_IGN); 
		signal(SIGTSTP, SIG_IGN); 
		signal(SIGHUP, SIG_IGN);
		if(pid = fork())
			exit(0);
		else if(pid < 0)
			exit(-1);
		if(setsid() < 0)
			exit(1);
		if(pid = fork())
			exit(0);
		else if(pid < 0)
			exit(-1);
		for (fd = 0, fdtablesize = getdtablesize(); fd < fdtablesize; fd++) 
			close(fd); 
		umask(0);
		signal(SIGCHLD, SIG_IGN);
		syslog(LOG_USER|LOG_INFO, "network interface start!\n"); 
    }
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);   
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));   
	memset(&addr, 0, sizeof(addr));   
	addr.nl_family = AF_NETLINK;   
	addr.nl_groups = RTNLGRP_LINK;   
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));   
	while ((retval = read(fd, buf, BUFLEN)) > 0)   
	{   
		for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))   
		{   
			if (nh->nlmsg_type == NLMSG_DONE)   
				break;   
			else if (nh->nlmsg_type == NLMSG_ERROR)   
				return;   
			else if (nh->nlmsg_type != RTM_NEWLINK)   
				continue;   
			ifinfo = NLMSG_DATA(nh);   
			//printf("%u: %s", ifinfo->ifi_index,(ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );   
			attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));   
			len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));   
			for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))   
			{   
				if (attr->rta_type == IFLA_IFNAME)   
				{   
					if(main_eth&&(strcmp((char*)RTA_DATA(attr),main_eth)==0))
					{
						if((ifinfo->ifi_flags & IFF_LOWER_UP)&&!main_eth_flag)
						{
							char temp[128];
							memset(temp,0,sizeof(temp));
							syslog(LOG_USER|LOG_INFO, "%s up,run %s\n ",(char*)RTA_DATA(attr),main_up_script);
							sprintf(temp,"%s&",main_up_script);
							main_eth_flag=1;
							system(temp);
						}
						else
						{
							char temp[128];
							memset(temp,0,sizeof(temp));
							syslog(LOG_USER|LOG_INFO, "%s down,run %s\n ",(char*)RTA_DATA(attr),main_down_script);
							sprintf(temp,"%s&",main_down_script);
							system(temp);
							main_eth_flag=0;
						}
					}
					else if(extra_eth&&(strcmp((char*)RTA_DATA(attr),extra_eth)==0))
					{
						if((ifinfo->ifi_flags & IFF_LOWER_UP)&&!extra_eth_flag)
						{
							char temp[128];
							memset(temp,0,sizeof(temp));
							syslog(LOG_USER|LOG_INFO, "%s up,run %s\n ",(char*)RTA_DATA(attr),extra_up_script); 
							sprintf(temp,"%s&",extra_up_script);
							extra_eth_flag=1;
							system(temp);
						}
						else
						{
							char temp[128];
							memset(temp,0,sizeof(temp));
							syslog(LOG_USER|LOG_INFO, "%s down,run %s\n ",(char*)RTA_DATA(attr),extra_down_script);
							sprintf(temp,"%s&",extra_down_script);
							system(temp);
							extra_eth_flag=0;
						}  
					}
					else
					{
						syslog(LOG_USER|LOG_INFO, "%s %s\n ",(char*)RTA_DATA(attr),(ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down");
					}
					break;   
				}   
			}
		}   
	}
	return 0;   
}
