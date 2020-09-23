#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SOCKET_PORT 8000

int main(int argc,char *argv[])
{

	int client_sockfd;
	struct sockaddr_in my_addr;
	char sendbuf[1024];
	char message[1024];
	int i,j,msglen;
		
	char str[10];
	
	if(argc != 7){
		fprintf(stderr,"Usage:%s masterip slaveip span phonenumber time message \n",argv[0]);
		return 1;
	}
	
	
	if((client_sockfd = socket(PF_INET,SOCK_STREAM,0))<0) {
		perror("cannot create communication socket");
		return 1;
	}
	
	//填充关于服务器的套接字信息
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
//	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_addr.s_addr = inet_addr(argv[1]);
	my_addr.sin_port = htons(SOCKET_PORT);
	
	//连接指定的服务器
	if(connect(client_sockfd,(struct sockaddr*)&my_addr,sizeof(my_addr))<0){
		 perror("cannot connect to the server");
		 close(client_sockfd);
	     return 1;
	}


	msglen = strlen(argv[6]);
	for (i=0,j=0; i<msglen; i++,j++) {
		if (argv[6][i] == '"') {
			message[j++] = '\\';
			message[j] = '"';
		} else if(argv[6][i] == '\\') {
			message[j++] = '\\';
			message[j] = '\\';
		} else {
			message[j] = argv[6][i];
		}
	}
	message[j] = '\0';

	
	//data format: slavesms:[slaveip]\t[span]\t[phonenumber]\t[time]\t[message]
	memset(sendbuf,0,sizeof(sendbuf));
	snprintf(sendbuf,sizeof(sendbuf),"slavesms:%s\t%s\t%s\t%s\t%s\n",argv[2],argv[3],argv[4],argv[5],message);


    if( send(client_sockfd,sendbuf, strlen(sendbuf), 0) < 0){
		perror("Send failed: \n");
		close(client_sockfd);
		return 1;
    }

    close(client_sockfd);
    return 0;
}

