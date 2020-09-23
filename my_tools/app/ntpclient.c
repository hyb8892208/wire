#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
       
#define NTP_SERVER    "time.buptnet.edu.cn"
#define NTP_PORT      123

#define TIMEOUT		10
#define CONTINUE_TIMES	3
#define JAN_1970      0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4 
#define PREC -6

#define NTPFRAC(x) (4294 * (x) + ((1981 * (x))>>11))
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

struct ntptime 
{
	unsigned int coarse;
	unsigned int fine;
};

int send_packet(int fd)
{
	unsigned int data[12];
	struct timeval now;
	int ret;

	memset((char*)data, 0, sizeof(data));
	data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
	data[1] = htonl(1<<16);  /* Root Delay (seconds) */
	data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
	gettimeofday(&now, NULL);
	data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
	ret=send(fd, data, 48, 0);
	return ret;
}

void get_packet_timestamp(int usd, struct ntptime *udp_arrival_ntp)
{
	struct timeval udp_arrival;
	gettimeofday(&udp_arrival, NULL);
	udp_arrival_ntp->coarse = udp_arrival.tv_sec + JAN_1970;
	udp_arrival_ntp->fine   = NTPFRAC(udp_arrival.tv_usec);
}

void rfc1305print(unsigned int *data, struct ntptime *arrival, struct timeval* tv)
{
	int li, vn, mode, stratum, poll, prec;
	int delay, disp, refid;
	struct ntptime reftime, orgtime, rectime, xmttime;
	struct tm *ltm;

#define Data(i) ntohl(((unsigned int *)data)[i])
	li      = Data(0) >> 30 & 0x03;
	vn      = Data(0) >> 27 & 0x07;
	mode    = Data(0) >> 24 & 0x07;
	stratum = Data(0) >> 16 & 0xff;
	poll    = Data(0) >>  8 & 0xff;
	prec    = Data(0)       & 0xff;
	if (prec & 0x80) prec|=0xffffff00;
	delay   = Data(1);
	disp    = Data(2);
	refid   = Data(3);
	reftime.coarse = Data(4);
	reftime.fine   = Data(5);
	orgtime.coarse = Data(6);
	orgtime.fine   = Data(7);
	rectime.coarse = Data(8);
	rectime.fine   = Data(9);
	xmttime.coarse = Data(10);
	xmttime.fine   = Data(11);
#undef Data

	tv->tv_sec = xmttime.coarse - JAN_1970;
	tv->tv_usec = USEC(xmttime.fine);
}

int set_local_time(struct timeval tv)
{
	int ret=-1; 

	/* need root user. */
	if (0 != getuid() && 0 != geteuid()){
		return -1;
	}
	ret=settimeofday(&tv, NULL);
	return ret;
}

static void usage(char *argv0)
{
        fprintf(stderr,
        "Usage: %s [-c count] "
        "-h hostname "
        "[-i interval] "
        "[-s]\n",
        argv0);
}

int main(int argc,char *argv[])
{
	int sock;
	struct sockaddr_in addr_src,addr_dst;
	int addr_len=0;
	struct hostent* host=NULL;
	char ntp_server[256], *psrv;
	int ret=-1;
	int count_timeout=CONTINUE_TIMES;
	int read_timeout=TIMEOUT;
	int numbers=0;
	int c;
	int set_clock=0;
	int debug=0;
	int sucess_flag=1;
	time_t current_time;
	strcpy(ntp_server,NTP_SERVER);
	for (;;) {
                c = getopt( argc, argv, "c:h:i:sd");
                if (c == EOF)
                	break;
                switch (c) {
                        case 'c':
                                count_timeout = atoi(optarg);
                                break;
			case 'd':
				debug=1;
				break;
                        case 'h':
                        	strcpy(ntp_server,optarg);
                        	break;
                        case 'i':
                                read_timeout = atoi(optarg);
                                break;
			case 's':
                                set_clock=1;
                                break;
			default:
                                usage(argv[0]);
                                exit(1);
		}
	}
	/* create socket. */
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	addr_len=sizeof(struct sockaddr_in);
	/* bind local address. */
	memset(&addr_src, 0, addr_len);
	addr_src.sin_family = AF_INET;
	addr_src.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_src.sin_port = htons(0);
	bind(sock, (struct sockaddr*)&addr_src, addr_len);

	/* connect to ntp server. */
	memset(&addr_dst, 0, addr_len);
	addr_dst.sin_family = AF_INET;
	psrv = strtok(ntp_server, ",");
	while (psrv) {
		host= gethostbyname(psrv);
		if(!host){
			if(debug)
				printf("gethostbyname to server %s error\n",psrv);
			sucess_flag=2;
			goto nextsrv;
		}
		//printf("official hostname:%s\n",host->h_name);
		memcpy(&(addr_dst.sin_addr.s_addr), host->h_addr_list[0], 4);
		addr_dst.sin_port = htons(NTP_PORT);
		ret=connect(sock,(struct sockaddr*)&addr_dst, addr_len);
		if(ret){
			if(debug)
				printf("connect to server %s error\n",psrv);
			sucess_flag=3;
			goto nextsrv;
		}
		send_packet(sock);
		while (numbers<count_timeout)
		{
			fd_set fds_read;
			struct timeval timeout;
			int ret;
			unsigned int buf[12];
			int len;

			struct sockaddr server;
			socklen_t svr_len;
			struct ntptime arrival_ntp;
			struct timeval newtime;

			FD_ZERO(&fds_read);
			FD_SET(sock, &fds_read);
			memset(&newtime,0,sizeof(newtime));
			timeout.tv_sec = read_timeout;
			timeout.tv_usec = 0;
			ret = select(sock + 1, &fds_read, NULL, NULL, &timeout);
			if (0 == ret || !FD_ISSET(sock, &fds_read))
			{
				/* send ntp protocol packet. */
				send_packet(sock);
				numbers++;
				continue;
			}

			/* recv ntp server's response. */
			memset(buf,0, sizeof(buf));
			memset(&server, 0 ,sizeof(server));
			memset(&svr_len, 0 ,sizeof(svr_len));
			ret=recvfrom(sock, buf, sizeof(buf), 0, &server, &svr_len);
			if(ret<0){
				if(debug)
					perror("recvfrom");
				send_packet(sock);
				numbers++;
				continue;
			}
			/* get local timestamp. */
			get_packet_timestamp(sock, &arrival_ntp);
			/* get server's time and print it. */
			rfc1305print(buf, &arrival_ntp, &newtime);
			/* set local time to the server's time, if you're a root user. */
			current_time=newtime.tv_sec;
			if(debug) {
				printf("sync ntp time sucess\n");
				printf("%s",ctime(&current_time));
			}
			if(set_clock) {
				ret=set_local_time(newtime);
				if(ret==0){
					sucess_flag=0;
					break;
				}
				else {
					if(debug)
						printf("set local time error\n");
					send_packet(sock);
					numbers++;
					continue;
				}
			} else {
				sucess_flag=0;
				break;
			}
		}
		if(numbers>=count_timeout)
		{
			if(debug)
				printf("sync ntp time fail\n");
			sucess_flag=4;
		}

nextsrv:
		if(debug)
			printf("return flag :%d \n",sucess_flag);
		if (!sucess_flag)
			break;
		else {
			psrv = strtok(NULL, ",");
		}
	}
	close(sock);
	return sucess_flag;
}
