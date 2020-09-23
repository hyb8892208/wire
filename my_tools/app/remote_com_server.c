#include <netinet/in.h>// for sockaddr_in
#include <sys/types.h>// for socket
#include <sys/socket.h>// for socket
#include <stdio.h>// for printf
#include <stdlib.h>// for exit
#include <string.h>// for bzero
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

// Usage remote_com_server

/*======================================================================================*/
int g_nStep = 0;
#define dprintf(format,args...) \
do {\
	printf("%d:",++g_nStep);\
	printf(format, ## args);\
} while(0)

#define GSM_SUM 8
#define IP_PORT 5000
#define SERVER_PORT 6666
#define LISTEN_QUEUE 1   //Must connect client counts 
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
#define GSM_NODE  "/dev/ttyGSM"
#define GET_PORTS "GetPorts"
#define POWER_ON_GSM_MODULE "PoweronGSMModule"
#define POWER_OFF_GSM_MODULE "PoweroffGSMModule"
#define SOCAT_FILE  "/my_tools/socat"
#define POWER_ON_OFF_FILE  "/proc/gsm_module_power_key-0"
#define MKDEV(ma,mi) (((ma) << 8) | ((mi) & 0xff))

/*======================================================================================*/
void kill_allpids(pid_t* pProcIds, int bDebug)
{
	int i = 0;
	int nStatus = 0;

	if(NULL == pProcIds)
		return;

	for(i = 0; i < GSM_SUM; i++) 
	{
		if(pProcIds[i] > 0) 
		{
			if(bDebug) dprintf("kill pid :%d .\n", pProcIds[i]);

			kill(pProcIds[i], SIGQUIT);
			waitpid(pProcIds[i], &nStatus, 0);
			pProcIds[i] = 0;
		}
	}
}

/*======================================================================================*/
int port_isfree(int nPort)
{  	 
	int dsSock = -1;  
	int nRet = 0;  
	int nOpt = 0; 
	struct sockaddr_in stSockAddr; 

	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(nPort);
	  
	dsSock = socket(AF_INET, SOCK_STREAM, 0);  
	if (dsSock == -1)  
	{
		return 0;  
	}

	nRet = setsockopt(dsSock, SOL_SOCKET, SO_REUSEADDR, &nOpt, sizeof(nOpt));  
	nRet = bind(dsSock, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr));  
	close(dsSock);

	return (nRet == 0) ? 1 : 0;  
}

/*======================================================================================*/
int get_vaild_port(int nPort, int bDebug)
{
	while(!port_isfree(nPort)) 
	{
		if(bDebug) dprintf("Port %d is invalid .\n", nPort);
		
		if(nPort++ >= (IP_PORT + 1000)) 
		{
			return 0;
		}
	}

	return nPort;
}

/*======================================================================================*/
int get_ports(pid_t* pProcIds, char* pszPort, int nBufLen, int bDebug)
{
	int i = 0;
	int nPort = 0;
	int nCount = 0;
	int nIpPorts[GSM_SUM] = {0};
	int nIpPort = IP_PORT;
	
	if(!pProcIds || !pszPort || 0 == nBufLen)
	{
		return -1;
	}
	
	for(i = 0; i < GSM_SUM; i++) 
	{
		pid_t pid = 0;
		char szArg1[128] = {0};
		char szArg2[128] = {0};
		char szNodeName[64] = {0};

		nPort = get_vaild_port(nIpPort, bDebug);
		if(nPort == 0) 
		{
			kill_allpids(pProcIds, bDebug);
			if(bDebug) dprintf("socat not had valid port\n");
				
			return -1;
		}

		nIpPort = nPort + 1;
		
		pid = vfork();  //Don't use fork(), vfork() will wait execl success or failed
		if(pid == 0) 
		{
			snprintf(szNodeName, sizeof(szNodeName), "%s%d", GSM_NODE, i);
			if(access(szNodeName, F_OK) == -1) 
			{
				dev_t dev = MKDEV(240,i);
				//mknod(szNodeName, S_IFCHR|0777, dev);
			}
			snprintf(szArg1, sizeof(szArg1), "tcp-listen:%d", nPort);
			sprintf(szNodeName, "/dev/ttyUSB%d,b115200", i);
			snprintf(szArg2, sizeof(szArg2), "%s", szNodeName);
			if(bDebug) dprintf("socat %s %s\n", szArg1, szArg2);

			if(-1 == execl(SOCAT_FILE, "socat", szArg1, szArg2, NULL)) 
			{
				if(bDebug) dprintf("execl socat %s %s failed! errno: %s .\n", szArg1, szArg2, strerror(errno));

				exit(1);
			}
			else
			{
				exit(0);
			}
		} 
		else if(pid < 0) 
		{
			if(bDebug) dprintf("Fork failed !\n");
			
			exit(1);
		}
		else 
		{
			if(bDebug) dprintf("vfork return pid: %d .\n", pid);
			
			pProcIds[i] = pid;
			nIpPorts[i] = nPort;
		}
		
		nCount = strlen(pszPort);
		snprintf(pszPort + nCount, nBufLen - nCount, "%d=%d ", i, nIpPorts[i]);
	}
	
	return 0;
}

/*======================================================================================*/
int get_accept_socket(int* psdSocket, struct sockaddr_in stSocketAddr, int* psdNewSocket, int bDebug)
{
	int nOpt = 1;
	
	if(!psdNewSocket || !psdSocket)
	{
		return -1;
	}
	
	//设置一个socket地址结构stServerAddrIn,代表服务器internet地址, 端口
	bzero(&stSocketAddr, sizeof(stSocketAddr)); //把一段内存区的内容全部设置为0
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_addr.s_addr = htons(INADDR_ANY);
	stSocketAddr.sin_port = htons(SERVER_PORT);

	//创建用于internet的流协议(TCP)socket,用sdServerSocket代表服务器socket
	if(bDebug) dprintf("Create socket .\n");

	*psdSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(*psdSocket < 0) 
	{
		if(bDebug) dprintf("Create socket failed !");
		
		return -1;
	}

	setsockopt(*psdSocket, SOL_SOCKET, SO_REUSEADDR, &nOpt, sizeof(int));

	//把socket和socket地址结构联系起来
	if(bDebug) dprintf("Bind socket .\n");

	if(bind(*psdSocket, (struct sockaddr*)&stSocketAddr, sizeof(stSocketAddr))) 
	{
		if(bDebug) dprintf("Server bind port : %d failed !", SERVER_PORT);
		
		return -1;
	}

	//server_socket用于监听 Only connect 1 client.
	if(bDebug) dprintf("Listen socket .\n");
	
	if(listen(*psdSocket, LISTEN_QUEUE)) 
	{
		if(bDebug) dprintf("Server listen failed !"); 
		
		return -1;
	}

	struct sockaddr_in stClientAddr;
	socklen_t nLength = sizeof(stClientAddr);

	//接受一个到server_socket代表的socket的一个连接
	//如果没有连接请求,就等待到有连接请求--这是accept函数的特性
	//accept函数返回一个新的socket,这个socket(sdNewSocket)用于同连接到的客户的通信
	//new_server_socket代表了服务器和客户端之间的一个通信通道
	//accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
	if(bDebug) dprintf("Accept socket .\n");

	*psdNewSocket = accept(*psdSocket, (struct sockaddr*)&stClientAddr, &nLength);
	if(*psdNewSocket < 0) 
	{
		if(bDebug) dprintf("Server sccept failed !\n");
		
		shutdown(*psdSocket, SHUT_RDWR);
		close(*psdSocket);
		return -1;
	}

	return 0;
}

/*======================================================================================*/
int main(int argc, char **argv)
{
	int fd = 0;
	int nFailed = 0;
	int ch = 0;
	int bDebug = 0;
	ssize_t nRevLen = 0;
	pid_t pids[GSM_SUM] = {0};
	char szBuf[BUFFER_SIZE] = {0};
	char szSendMsg[BUFFER_SIZE] = {0};
	char szAllPorts[1024] = {0};
	struct sockaddr_in stServerAddr;
	int sdSocket = 0;
	int sdNewSocket = 0;
	
	while((ch = getopt(argc, argv, "d")) != -1)
	{
		switch(ch)
		{
		case 'd':
			bDebug = 1;
			break;

		default: // command is error 
			{
				printf("unknown option : %c. \n", (char)ch);
			}
			break;
		}
	}
	bDebug = 1;
	
	if(-1 == get_ports(pids, szAllPorts, sizeof(szAllPorts), bDebug))
	{
		return 1;
	}
	
	if(bDebug) dprintf("All ports:%s.\n", szAllPorts);
	
	if(-1 == get_accept_socket(&sdSocket, stServerAddr, &sdNewSocket, bDebug))
	{
		if(bDebug) dprintf("Get accept socket failed !\n");
		
		return 1;
	}
	
	if(bDebug) dprintf("Send socket.\n");
	
	if(send(sdNewSocket, szAllPorts, strlen(szAllPorts), 0) < 0) 
	{
		if(bDebug) dprintf("Send socket failed !\n");
		
		nFailed = 1;
		goto failed;
	}

	while(1) 
	{
		bzero(szBuf, sizeof(szBuf));
		
		nRevLen = recv(sdNewSocket, szBuf, sizeof(szBuf), 0);
		if(nRevLen < 0) 
		{
			if(bDebug) dprintf("Server recieve data failed !\n");
			
			nFailed = 1;
			break;
		} 
		else if(nRevLen == 0) 
		{
			g_nStep = 0;
			
			if(bDebug) dprintf("Remote client quit !\n");
			
			kill_allpids(pids, bDebug);
			shutdown(sdNewSocket, SHUT_RDWR);
			close(sdNewSocket);
				
			memset(pids, 0, sizeof(pids));
			memset(szAllPorts, 0, sizeof(szAllPorts));
			
			get_ports(pids, szAllPorts, sizeof(szAllPorts), bDebug);
			
			if(bDebug) dprintf("All ports:%s.\n", szAllPorts);				
			
			if(bDebug) dprintf("Listen socket .\n");
			
			if(listen(sdSocket, LISTEN_QUEUE)) 
			{
				if(bDebug) dprintf("Server listen failed !"); 
				
				break;
			}
		
			struct sockaddr_in stClientAddr;
			socklen_t nLength = sizeof(stClientAddr);
		
			if(bDebug) dprintf("Accept socket .\n");
			
			sdNewSocket = accept(sdSocket, (struct sockaddr*)&stClientAddr, &nLength);
			if(sdNewSocket < 0) 
			{
				if(bDebug) dprintf("Server sccept failed !\n");
				
				break;
			}

			if(send(sdNewSocket, szAllPorts, strlen(szAllPorts), 0) < 0) 
			{
				if(bDebug) dprintf("Send socket failed .\n");
				
				nFailed = 1;
				break;
			}
				
			continue;
		}

		if(0 == strncasecmp(szBuf, GET_PORTS, nRevLen)) 
		{
			if(bDebug) dprintf("Get Ports .\n");
			
			if(send(sdNewSocket, szAllPorts, strlen(szAllPorts),0) < 0) 
			{
				if(bDebug) dprintf("Send socket failed !\n");
			}
		} 
		else if(0 == strncasecmp(szBuf, POWER_OFF_GSM_MODULE, nRevLen)) 
		{
			if(bDebug) dprintf("Power Off GSM Module .\n");
			
			fd = open(POWER_ON_OFF_FILE, O_RDWR);
			
			if(fd > 0) 
			{
				write(fd,"0",1);
				close(fd);

				memset(szSendMsg, 0, sizeof(szSendMsg));
				snprintf(szSendMsg, sizeof(szSendMsg), "%s %s", POWER_OFF_GSM_MODULE, "OK");
				if(send(sdNewSocket, szSendMsg, strlen(szSendMsg) + 1, 0) < 0) 
				{
					if(bDebug) dprintf("Send socket failed !\n");
				}
			}
		} 
		else if(0 == strncasecmp(szBuf, POWER_ON_GSM_MODULE, nRevLen)) 
		{
			if(bDebug) dprintf("Power On GSM Module .\n");

			fd = open(POWER_ON_OFF_FILE, O_RDWR);
			
			if(fd > 0) 
			{
				write(fd, "1", 1);
				
				close(fd);
				
				memset(szSendMsg, 0, sizeof(szSendMsg));
				snprintf(szSendMsg, sizeof(szSendMsg), "%s %s", POWER_ON_GSM_MODULE, "OK");
				if(send(sdNewSocket, szSendMsg, strlen(szSendMsg) + 1, 0) < 0) 
				{
					if(bDebug) dprintf("Send socket failed !\n");
				}
			}
		}
	}

failed:
	//关闭与客户端的连接
	if(nFailed)
	{
		shutdown(sdNewSocket, SHUT_RDWR);
		close(sdNewSocket);
	}

	kill_allpids(pids, bDebug);
	shutdown(sdSocket, SHUT_RDWR);
	close(sdSocket);

	return 0;
}

/*======================================================================================*/


