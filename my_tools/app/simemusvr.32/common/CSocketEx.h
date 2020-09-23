/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：CSocketEx.h 
* 文件说明：socket套接字通讯接口头文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __CSOCKETEX_H__
#define __CSOCKETEX_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define Sleep(x) usleep(x*1000)

#endif


#define PROTO_TCP		0
#define PROTO_UDP		1

#define READ_WAIT		1
#define WRITE_WAIT		2

#define MODE_UDP		1
#define MODE_TCP		2
#define MODE_UNIX		3

#define TIMEOUT			2


/**************************** 类声明和定义 ****************************/
class CSocketEx
{
public:
	CSocketEx();
	CSocketEx(int mode, char *localip, unsigned short localport, char *peerip, unsigned short peerport, int timemode = 30);
	virtual ~CSocketEx();
	
	// 1--success, 0--failed
	bool CreateSocket();
	void CloseSocket();
	void CloseSocket(int sockfd);
	
	void Init(int mode, char *localip, unsigned short localport, char *peerip, unsigned short peerport, int timeout);
	void Init(int mode, char *localip, unsigned short localport, unsigned int peerip, unsigned short peerport, int timeout);
	
	bool Connect();
	bool ConnectNonB();
	bool Bind();
	bool Bind(int sockfd, char *ip, unsigned short port);
	bool BuildServer();
	int AcceptClient();
	
	int WriteData(int sockfd, char* buffer, int len);
	int WriteData(int sockfd, char *buffer, int len, char *peerip, unsigned short peerport);
	int TCPReadData(int sockfd, char* buffer, int len);
	int TCPReadDataOnce(int sockfd, char* buffer, int len);
	int UDPReadData(int sockfd, char* buffer, int len);
	int UDPReadData(int sockfd, char *buffer, int len, char *peerip, unsigned short peerport);
	int ReadData(int sockfd, char* buffer, int len);
	
	void setSocket(int sockfd);
	int getSocket();
	
	bool setBlock();
	bool setBlock(int sockfd);
	bool setNonBlock();
	bool setNonBlock(int sockfd);
	bool ConnectNonB(int sockfd, int timeout);
	
	bool setReuseAddr();
	bool setReuseAddr(int sockfd);
	bool setKeepAlive();
	bool setKeepAlive(int sockfd);
	
	//socket状态,1--created, 0--not created
	int getSocketState();	

	
	int getEthernetMac(char *device,unsigned char *mac_addr);
	//设置超时
	void setTimeout();
	bool setBufferSize(int size);
	bool setBufferSize(int sockfd, int size);
	//bool setNoDelay(void);
	//bool setNoDelay(int sockfd);
	
	bool checkSocketReady(int mode);
	bool checkSocketReady(int sockfd, int mode);
	bool checkSocketReady(int sockfd, int mode, int timeout);
	bool checkSocketReady(int sockfd, int mode, int timeout_sec, int timeout_usec);

	bool setLinger();
	
	bool setLinger(int socket);
	void setNoDelay();
	void setNoDelay(int socket);
	
	inline void setLocalIP(char *ip) { strcpy(m_localIp, ip); }
	inline void getLocalIP(char *ip) { strcpy(ip, m_localIp); }
	inline void setLocalPort(unsigned short port) {	m_localPort = port; }
	inline unsigned short getLocalPort() {	return m_localPort; }
	inline void setPeerIP(char *ip) { strcpy(m_peerIp, ip); }
	inline void getPeerIP(char *ip) { strcpy(ip, m_peerIp);}
	inline void setPeerPort(unsigned short port) {m_peerPort = port;}
	inline unsigned short getPeerPort() {return m_peerPort;}
	
	
	char m_localIp[16];
	char m_peerIp[16];
	unsigned short m_localPort;
	unsigned short m_peerPort;
	unsigned int m_aptip; // accept peer ip
	int m_sockfd;
	int m_timeout;
	int m_mode; // 同transMode
	
	struct sockaddr_in m_localAddr;
	struct sockaddr_in m_peerAddr;
	int m_sockLen;
	int m_acceptFd;
};


#endif
