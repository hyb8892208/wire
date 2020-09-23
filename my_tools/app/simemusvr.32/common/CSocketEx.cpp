/****************************************************************************
* 版权信息：
* 系统名称：SimServer
* 文件名称：CSocketEx.c
* 文件说明：socket套接字通讯接口实现文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/
//#include "stdafx.h"
//#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <netinet/tcp.h>
#include "CSocketEx.h"
#include "zprint.h"

#ifndef WIN32
#include <errno.h>
#endif


/**************************************************************************** 
* 函数名称 : CSocketEx
* 功能描述 : 网络套接口类构造函数
* 参    数 : 
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
CSocketEx::CSocketEx()
{
#ifdef WIN32
	WSAData wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);
#endif

	memset(m_localIp, 0, sizeof(m_localIp));
	memset(m_peerIp,  0, sizeof(m_peerIp));
	m_localPort = 0;
	m_peerPort  = 0;
	m_sockfd	= -1;
	m_timeout   = TIMEOUT*1000;
	m_mode      = PROTO_TCP;
	m_acceptFd  = -1;
	
	memset(&m_peerAddr, 0, sizeof(m_peerAddr));
	m_sockLen = sizeof(m_peerAddr);
}

/**************************************************************************** 
* 函数名称 : CSocketEx
* 功能描述 : 网络套接口类构造函数
* 参    数 : int mode					: 运行模式, 取值范围：PROTO_TCP, PROTO_UDP, PROTO_UNIX
* 参    数 : char *localip				: 本地ip地址
* 参    数 : unsigned short localport	: 本地端口
* 参    数 : char *peerip				: 远程ip地址
* 参    数 : unsigned short peerport	: 远程端口
* 参    数 : int timeout				: 数据发送超时
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
CSocketEx::CSocketEx(int mode, char *localip, unsigned short localport, char *peerip, unsigned short peerport, int timeout)
{
	//printf("CSocketEx::CSocketEx\n");
#ifdef WIN32
	WSAData wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);
#endif
	memset(m_localIp, 0, sizeof(m_localIp));
	memset(m_peerIp,  0, sizeof(m_peerIp));
	
// 	if (MODE_UDP == mode)
// 	{
// 		m_mode = PROTO_UDP;
// 	}
// 	else
// 	{
// 		m_mode = PROTO_TCP;
// 	}
	m_mode = mode;
	strcpy(m_localIp, localip);
	m_localPort = localport;
	strcpy(m_peerIp, peerip);
	m_peerPort = peerport;
	m_timeout = timeout*1000;
	m_acceptFd  = -1;
	
	memset(&m_peerAddr, 0, sizeof(m_peerAddr));
	
	m_peerAddr.sin_family = AF_INET;
	m_peerAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	//m_peerAddr.sin_addr.S_un.S_addr = inet_addr(m_peerIp);
	m_peerAddr.sin_port = htons(m_peerPort);

	m_sockLen = sizeof(m_peerAddr);
}

/**************************************************************************** 
* 函数名称 : ~CSocketEx
* 功能描述 : 网络套接口类析构函数
* 参    数 : 
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
CSocketEx::~CSocketEx()
{
	//printf("CSocketEx::~CSocketEx\n");
	CloseSocket();
#ifdef WIN32
	WSACleanup();
#endif
}

/**************************************************************************** 
* 函数名称 : Init
* 功能描述 : 网络套接口类构造函数
* 参    数 : int mode					: 运行模式
* 参    数 : char *localip				: 本地ip地址
* 参    数 : unsigned short localport	: 本地端口
* 参    数 : char *peerip				: 远程ip地址
* 参    数 : unsigned short peerport	: 远程端口
* 参    数 : int timeout				: 数据发送超时
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void CSocketEx::Init(int mode, char *localip, unsigned short localport, char *peerip, unsigned short peerport, int timeout)
{
// 	if (NULL != strstr(mode, "udp") || NULL != strstr(mode, "UDP"))
// 	{
// 		m_mode = PROTO_UDP;
// 	}
// 	else
// 	{
// 		m_mode = PROTO_TCP;
// 	}
	m_mode = mode;
	if (localip != NULL)
	{
		strcpy(m_localIp, localip);
	} 
	m_localPort = localport;
	if (peerip != NULL)
	{
		strcpy(m_peerIp, peerip);
	}
	m_peerPort = peerport;
	m_timeout = timeout;
	
	m_peerAddr.sin_family = AF_INET;
	m_peerAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	//m_peerAddr.sin_addr.S_un.S_addr = inet_addr(m_peerIp);
	m_peerAddr.sin_port = htons(m_peerPort);
}

void CSocketEx::Init(int mode, char *localip, unsigned short localport, unsigned int peerip, unsigned short peerport, int timeout)
{
	// 	if (NULL != strstr(mode, "udp") || NULL != strstr(mode, "UDP"))
	// 	{
	// 		m_mode = PROTO_UDP;
	// 	}
	// 	else
	// 	{
	// 		m_mode = PROTO_TCP;
	// 	}
	m_mode = mode;
	if (localip != NULL)
	{
		strcpy(m_localIp, localip);
	} 
	m_localPort = localport;
	m_peerPort = peerport;
	m_timeout = timeout;
	
	m_peerAddr.sin_family = AF_INET;
	m_peerAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	//m_peerAddr.sin_addr.S_un.S_addr = inet_addr(m_peerIp);
	m_peerAddr.sin_port = htons(m_peerPort);
	
	struct in_addr iaddr;
	iaddr.s_addr = peerip;
	sprintf(m_peerIp, "%s", inet_ntoa(iaddr));
}

/**************************************************************************** 
* 函数名称 : CreateSocket
* 功能描述 : 创建网络套接口函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::CreateSocket()
{
	//first close the existing socket
	CloseSocket();

	if (PROTO_TCP == m_mode)
	{
		m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	}
	else
	{
		m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	}
	if (m_sockfd < 0)
	{
		return false;
	}

	setTimeout();
	setBlock();
	setReuseAddr();
	setLinger();
	setNoDelay();
	return true;
}

/**************************************************************************** 
* 函数名称 : CloseSocket
* 功能描述 : 关闭网络套接口函数
* 参    数 : 
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void CSocketEx::CloseSocket()
{
	CloseSocket(m_sockfd);
	m_sockfd = -1;
}

/**************************************************************************** 
* 函数名称 : CloseSocket
* 功能描述 : 关闭网络套接口函数
* 参    数 : int sockfd : 套接字
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void CSocketEx::CloseSocket(int sockfd)
{
	if (0 < sockfd)
	{
#ifdef WIN32
		closesocket(sockfd);
#else
		close(sockfd);
#endif
	}
}

/**************************************************************************** 
* 函数名称 : setSocket
* 功能描述 : 设置网络套接口函数
* 参    数 : int sockfd : 套接字
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void CSocketEx::setSocket(int sockfd)
{
	m_sockfd = sockfd;
}

/**************************************************************************** 
* 函数名称 : getSocket
* 功能描述 : 获取网络套接口函数
* 参    数 : 
* 返 回 值 : 套接字
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::getSocket()
{
	return m_sockfd;
}

/**************************************************************************** 
* 函数名称 : checkSocketReady
* 功能描述 : 检查套接字是否就位函数
* 参    数 : int mode : 模式
* 返 回 值 : 就位:true; 未就位:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::checkSocketReady(int mode)
{
	return checkSocketReady(m_sockfd, mode);
}

/**************************************************************************** 
* 函数名称 : checkSocketReady
* 功能描述 : 检查套接字是否就位函数
* 参    数 : int sockfd : 套接字
* 参    数 : int mode : 模式
* 返 回 值 : 就位:true; 未就位:false
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::checkSocketReady(int sockfd, int mode)
{
	timeval tv;
	fd_set fset;

	if (sockfd < 0)
	{
		return false;
	}

	FD_ZERO(&fset);
	FD_SET((unsigned)sockfd, &fset);
	tv.tv_sec = m_timeout;
	tv.tv_usec = 0;
	if (READ_WAIT == mode)
	{
		select(sockfd+1, &fset, NULL, NULL, &tv);
	}
	else 
	{
		select(sockfd+1, NULL, &fset, NULL, &tv);
	}
	if (FD_ISSET(sockfd, &fset))
	{
		return true;
	}
	return false;

}
bool CSocketEx::checkSocketReady(int sockfd, int mode, int timeout)
{
	timeval tv;
	fd_set fset;
	
	if (sockfd < 0)
	{
		return false;
	}
	
	FD_ZERO(&fset);
	FD_SET((unsigned)sockfd, &fset);
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	if (READ_WAIT == mode)
	{
		select(sockfd+1, &fset, NULL, NULL, &tv);
	}
	else 
	{
		select(sockfd+1, NULL, &fset, NULL, &tv);
	}
	if (FD_ISSET(sockfd, &fset))
	{
		return true;
	}
	return false;
	
}

bool CSocketEx::checkSocketReady(int sockfd, int mode, int timeout_sec, int timeout_usec)
{
	timeval tv;
	fd_set fset;
	
	if (sockfd < 0)
	{
		return false;
	}
	
	FD_ZERO(&fset);
	FD_SET((unsigned)sockfd, &fset);
	tv.tv_sec = timeout_sec;
	tv.tv_usec = timeout_usec;
	if (READ_WAIT == mode)
	{
		select(sockfd+1, &fset, NULL, NULL, &tv);
	}
	else 
	{
		select(sockfd+1, NULL, &fset, NULL, &tv);
	}
	if (FD_ISSET(sockfd, &fset))
	{
		return true;
	}
	return false;
	
}

/**************************************************************************** 
* 函数名称 : Connect
* 功能描述 : 连接远程服务器函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::Connect()
{
	if(m_localPort > 0)
	{
		if (!Bind())
		{
			return false;
		}
	}
  
	//阻塞方式连接服务器
// 	int bIfNonBlock = 0;
// 	bIfNonBlock = ifNonBlock();
// 	if (0 == bIfNonBlock)
// 	{
// 		//强制设置为非阻塞模式
// 		setNonBlock();
// 	}
	
	//connect
	struct sockaddr_in peerSockAddr;
	memset(&peerSockAddr, 0, sizeof(struct sockaddr_in));

	peerSockAddr.sin_family	     = AF_INET;
	peerSockAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	peerSockAddr.sin_port		 = htons(m_peerPort);
	
	if (connect(m_sockfd, (const sockaddr*)&peerSockAddr, sizeof(struct sockaddr_in)) == -1)
	{
		//printf("connect to %s:%d err(%d)\n", m_peerIp, m_peerPort, errno);
		CloseSocket();
		return false;
	}
	
// 	if (0 == bIfNonBlock)
// 	{
// 		//恢复阻塞模式
// 		setBlock();
// 	}

	return true;
}

/**************************************************************************** 
* 函数名称 : Connect
* 功能描述 : 非阻塞式连接远程服务器函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::ConnectNonB()
{
// 	if(m_localPort > 0)
// 	{
// 		if (!Bind())
// 		{
// 			return false;
// 		}
// 	}
  
	//阻塞方式连接服务器
// 	int bIfNonBlock = 0;
// 	bIfNonBlock = ifNonBlock();
// 	if (0 == bIfNonBlock)
// 	{
// 		//强制设置为非阻塞模式
// 		setNonBlock();
// 	}
	
	//connect
	struct sockaddr_in peerSockAddr;
	memset(&peerSockAddr, 0, sizeof(struct sockaddr_in));

	peerSockAddr.sin_family	     = AF_INET;
	peerSockAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	peerSockAddr.sin_port		 = htons(m_peerPort);
	
	connect(m_sockfd, (const sockaddr*)&peerSockAddr, sizeof(struct sockaddr_in));
	
// 	if (0 == bIfNonBlock)
// 	{
// 		//恢复阻塞模式
// 		setBlock();
// 	}

	return true;
}

#if 1
bool CSocketEx::ConnectNonB(int sockfd, int timeout)
{
	struct timeval tv;
	fd_set rset;
	fd_set wset;
	int ret = 0;
#ifdef WIN32
	int socklen = 0;
#else
	socklen_t socklen;
#endif
	int error = 0;

	if (sockfd < 0)
	{
		return false;
	}

	
	//非阻塞方式连接服务器
 	setNonBlock(sockfd);
	
	//connect
	struct sockaddr_in peerSockAddr;
	memset(&peerSockAddr, 0, sizeof(struct sockaddr_in));
	
	peerSockAddr.sin_family	     = AF_INET;
	peerSockAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	peerSockAddr.sin_port		 = htons(m_peerPort);
	
	ret = connect(sockfd, (const sockaddr*)&peerSockAddr, sizeof(struct sockaddr_in));
#ifndef WIN32
	if (ret < 0)
	{
#ifdef WIN32
		if (errno != WSAEINPROGRESS)
#else
		if (errno != EINPROGRESS)
#endif
		{
			return false;
		}
	}
#endif
	if (ret == 0)
	{ // connect completed immediately
		goto DONE;
	}

	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET((unsigned int)sockfd, &rset);
	FD_SET((unsigned int)sockfd, &wset);
	ret = select(sockfd+1, &rset, &wset, NULL, &tv);
	if (ret == 0)
	{
		//errno = ETIMEOUT;
		return false;
	}
	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
	{
		socklen = sizeof(error);
#ifdef WIN32
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, &socklen) < 0)
#else
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &socklen) < 0)
#endif
		{
			return false; // Solaris pending error
		}
		//setBlock(sockfd);
		//return true;
	}
	else
	{
		return false;
	}
DONE:
	setBlock(sockfd);
	if (error)
	{
		errno = error;
		return false;
	}
	return true;
}
#else
bool CSocketEx::ConnectNonB(int sockfd, int timeout)
{
	struct timeval tv;
	fd_set rset;
	fd_set wset;
	//非阻塞方式连接服务器
 	setNonBlock(sockfd);
	
	//connect
	struct sockaddr_in peerSockAddr;
	memset(&peerSockAddr, 0, sizeof(struct sockaddr_in));
	
	peerSockAddr.sin_family	     = AF_INET;
	peerSockAddr.sin_addr.s_addr = inet_addr(m_peerIp);
	peerSockAddr.sin_port		 = htons(m_peerPort);
	
	connect(sockfd, (const sockaddr*)&peerSockAddr, sizeof(struct sockaddr_in));

	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET((unsigned int)sockfd, &rset);
	FD_SET((unsigned int)sockfd, &wset);
	select(sockfd+1, &rset, &wset, NULL, &tv);
	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
	{
		setBlock(sockfd);
		return true;
	}
	
	// 	if (0 == bIfNonBlock)
	// 	{
	// 		//恢复阻塞模式
	// 		setBlock();
	// 	}
	setBlock(sockfd);
	return false;
}
#endif

/**************************************************************************** 
* 函数名称 : setBlock
* 功能描述 : 设置套接字阻塞模式函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setBlock()
{
	return setBlock(m_sockfd);
}

/**************************************************************************** 
* 函数名称 : setBlock
* 功能描述 : 设置套接字阻塞模式函数
* 参    数 : int sockfd : 套接字
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setBlock(int sockfd)
{
	int ret = 0;
#ifdef WIN32
	u_long mode = 0;
#endif
	
	if (0 < sockfd)
	{
#ifdef WIN32
		ret = ioctlsocket(sockfd, FIONBIO, &mode);
		if (ret == NO_ERROR)
		{
			return true;
		}
#else
		ret = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_SETFL) & (~O_NONBLOCK));
		if (ret > 0)
		{
			return true;
		}
#endif
	}
	return false;
}

/**************************************************************************** 
* 函数名称 : Bind
* 功能描述 : 绑定套接字函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::Bind()
{
	return Bind(m_sockfd, m_localIp, m_localPort);
}

/**************************************************************************** 
* 函数名称 : Bind
* 功能描述 : 绑定套接字函数
* 参    数 : int sockfd				: 套接字
* 参    数 : char *ip				: ip地址
* 参    数 : unsigned short port	: 端口
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::Bind(int sockfd, char *ip, unsigned short port)
{
	int iBindResult = -1;
	struct sockaddr_in localSockAddr;

	memset(&localSockAddr, 0, sizeof(struct sockaddr_in));
	localSockAddr.sin_family = AF_INET;
	if (NULL == ip)
	{
		localSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		localSockAddr.sin_addr.s_addr = inet_addr(ip);
	}
	localSockAddr.sin_port = htons(port);
	
	//bind	
	iBindResult = bind(sockfd, (const sockaddr*)&localSockAddr, sizeof(struct sockaddr_in));
	if (iBindResult < 0)
	{
		CloseSocket(sockfd);
		return false;
	}
	return true;
}


/**************************************************************************** 
* 函数名称 : setNoDelay
* 功能描述 : 设置套接字禁止发送合并的Nagle算法
* 参    数 : 
* 返 回 值 : void
* 作    者 : lyz 
* 设计日期 : 2017/08/16
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void CSocketEx::setNoDelay()
{
	int enable = 1;
	setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}

void CSocketEx::setNoDelay(int sockfd)
{
	int enable = 1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}

/**************************************************************************** 
* 函数名称 : setNonBlock
* 功能描述 : 设置套接字非阻塞模式函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setNonBlock()
{
	return setNonBlock(m_sockfd);
}

/**************************************************************************** 
* 函数名称 : setNonBlock
* 功能描述 : 设置套接字非阻塞模式函数
* 参    数 : int sockfd	: 套接字
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setNonBlock(int sockfd)
{
	int ret = 0;
#ifdef WIN32
	u_long mode = 1;
#endif
	
	if (0 < sockfd)
	{
#ifdef WIN32
		ret = ioctlsocket(sockfd, FIONBIO, &mode);
		if (ret == NO_ERROR)
		{
			return true;
		}
#else
		ret = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_SETFL) | O_NONBLOCK);
		if (ret > 0)
		{
			return true;
		}
#endif
	}
	return false;
}

/**************************************************************************** 
* 函数名称 : setReuseAddr
* 功能描述 : 设置地址复用函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setReuseAddr()
{
	return setReuseAddr(m_sockfd);
}

/**************************************************************************** 
* 函数名称 : setReuseAddr
* 功能描述 : 设置地址复用函数
* 参    数 : int sockfd	: 套接字
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setReuseAddr(int sockfd)
{
	int iReuseAddrFlag=1;
	int ret = -1;

	if (0 > sockfd)
	{
		return false;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iReuseAddrFlag, sizeof(iReuseAddrFlag));
	if (-1 == ret)
	{
		return false;
	}
	return true;
}

/**************************************************************************** 
* 函数名称 : setKeepAlive
* 功能描述 : 保活套接字函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setKeepAlive()
{
	return setKeepAlive(m_sockfd);
}

/**************************************************************************** 
* 函数名称 : setKeepAlive
* 功能描述 : 保活套接字函数
* 参    数 : int sockfd	: 套接字
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setKeepAlive(int sockfd)
{
	int iKeepAliveFlag = 1;
	int iKeepAliveSize = sizeof iKeepAliveFlag;

	if (0 > sockfd)
	{
		return false;
	}
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&iKeepAliveFlag, iKeepAliveSize);
	if (-1 == ret)
	{
		return false;
	}

	return true;
}

/**************************************************************************** 
* 函数名称 : getSocketState
* 功能描述 : 获取套接字状态函数
* 参    数 : 
* 返 回 值 : 可用:1; 不可用:0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::getSocketState()
{
	if (m_sockfd < 0)
	{
		return 0;
	}

	return 1;
}

int CSocketEx::getEthernetMac(char *device,unsigned char *mac_addr)
{
    struct ifreq ifreq;
    int sock;

    if(!device || !mac_addr)
    {
        return 1;
    }
    if((sock=socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        return 2;
    }
    strcpy(ifreq.ifr_name,device);
    if(ioctl(sock,SIOCGIFHWADDR,&ifreq)<0)
    {
		close(sock);
        return 3;
    }
	close(sock);
	memcpy((char *)mac_addr,ifreq.ifr_hwaddr.sa_data,6);
	return 0;
}

/**************************************************************************** 
* 函数名称 : setTimeout
* 功能描述 : 设置套接字数据收发超时函数
* 参    数 : 
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
void CSocketEx::setTimeout()
{
	if (m_timeout <= 0 || m_sockfd < 0)
	{
		return;
	}
	struct timeval tv;  
	tv.tv_sec = m_timeout;  
	tv.tv_usec = 0;
	setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
	setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
}

/*bool CSocketEx::setNoDelay(void)
{
	return setNoDelay(m_sockfd);
}*/
/*bool CSocketEx::setNoDelay(int sockfd)
{
	int optval = 1;
	return setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &optval, sizeof(optval));
}*/

/**************************************************************************** 
* 函数名称 : setBufferSize
* 功能描述 : 设置套接字数据收发缓冲函数
* 参    数 : int size : 缓冲大小
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setBufferSize(int size)
{
	return setBufferSize(m_sockfd, size);
}

/**************************************************************************** 
* 函数名称 : setBufferSize
* 功能描述 : 设置套接字数据收发缓冲函数
* 参    数 : int sockfd : 套接字
* 参    数 : int size	: 缓冲大小
* 返 回 值 : 
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::setBufferSize(int sockfd, int size)
{
	if (sockfd < 0 || size < 0)
	{
		return false;
	}

	int value=size;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&value, sizeof(value));

	return true;
}

/**************************************************************************** 
* 函数名称 : BuildServer
* 功能描述 : 创建服务器函数
* 参    数 : 
* 返 回 值 : 成功:true; 失败:false
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
bool CSocketEx::BuildServer()
{
	//先不设置非阻塞
/*	if (!setNonBlock())

	{
		LOG4CPLUS_ERROR(logger,"fcntl set failed errno is  " << errno);
		closeSocket();  
		return false;
	}
*/
	if (!setReuseAddr()) // 出错信息在底层打印
	{
		CloseSocket();  
		return false;
	}
/*
	//设置本端地址信息，以便绑定套接字
	struct sockaddr_in  localAddr;
	localAddr.sin_family	  = AF_INET;

    if(m_localIp == "*")
    {
	    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        unsigned iNetIP = inet_addr(m_localIp);
        if(iNetIP == INADDR_NONE)
        {
            return false;
        }

        localAddr.sin_addr.s_addr = iNetIP;
    }
    
	localAddr.sin_port		= htons(m_localPort);

	int iBindResult = bind(m_sockfd, (struct sockaddr *)&localAddr, sizeof(localAddr));

	if (iBindResult < 0)
	{
		printf()
		CloseSocket();
		return false;
	}
	*/
	//if (!Bind(m_sockfd, m_localIp, m_localPort))
	if (!Bind(m_sockfd, NULL, m_localPort))
	{
		return false;
	}

	int iListenResult = listen(m_sockfd, 1024);

	if (iListenResult < 0)
	{
		CloseSocket();
		return false;
	}
	
	return true;
}

/**************************************************************************** 
* 函数名称 : AcceptClient
* 功能描述 : 接收连接函数
* 参    数 : 
* 返 回 值 : 成功:返回连接套接字; 失败:-1
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::AcceptClient()
{
	//struct sockaddr_in pRemoteAddr;
	int iLen = sizeof(struct sockaddr_in);
#ifdef WIN32
	int iAcceptResult = accept(m_sockfd, (struct sockaddr *)&m_peerAddr, &iLen);
#else
	int iAcceptResult = accept(m_sockfd, (struct sockaddr *)&m_peerAddr, (socklen_t *)&iLen);
#endif

	if (iAcceptResult < 0)
	{
		return -1;
	}
#ifdef WIN32
	m_aptip = m_peerAddr.sin_addr.S_un.S_addr;
#else
	m_aptip = m_peerAddr.sin_addr.s_addr;
#endif
	m_peerPort = ntohs(m_peerAddr.sin_port);

	if ( !setKeepAlive(iAcceptResult))
	{
#ifdef WIN32
		closesocket(iAcceptResult);
#else
		close(iAcceptResult);
#endif
		iAcceptResult = -1;
		return iAcceptResult;
	}
	m_acceptFd = iAcceptResult;
	return iAcceptResult;
}

/**************************************************************************** 
* 函数名称 : TCPReadData
* 功能描述 : TCP协议套接字读取数据函数
* 参    数 : int sockfd		: 套接字
* 参    数 : char* buffer	: 缓冲
* 参    数 : int len		: 读取长度
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::TCPReadData(int sockfd, char* buffer, int len)
{
	int leftn = len;
	char *pbuf = buffer;

	int recv_bytes = 0;
	while(leftn > 0)
	{
#ifdef WIN32
		recv_bytes = recv(sockfd, pbuf, leftn, 0);
#else
		recv_bytes = recv(sockfd, pbuf, leftn, MSG_NOSIGNAL);
#endif
		if (recv_bytes <= 0)
			zprintf(ERROR,"[ERRO]TCPReadData: leftn=%d, recv_bytes=%d, error(%d:%s)", leftn, recv_bytes,errno, strerror(errno));
			
		if(recv_bytes < 0)
		{
			switch(errno)
			{
				case EINTR:
				case EAGAIN:
					continue;
				default:
					*pbuf = '\0';
					return len-leftn;
			}
		}
		else if(recv_bytes == 0 ){
			return -1;
		}
		pbuf += recv_bytes;
		leftn -= recv_bytes;
	}
	*pbuf = '\0';
	return len;
}

/**************************************************************************** 
* 函数名称 : TCPReadDataOnce
* 功能描述 : TCP协议套接字读取数据函数，读取一次，不作循环读取
* 参    数 : int sockfd		: 套接字
* 参    数 : char* buffer	: 缓冲
* 参    数 : int len		: 读取长度
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::TCPReadDataOnce(int sockfd, char* buffer, int len)
{
	int recv_bytes = 0;
#ifdef WIN32
		recv_bytes = recv(sockfd, buffer, len, 0);
#else
		recv_bytes = recv(sockfd, buffer, len, MSG_NOSIGNAL);
#endif
	buffer[recv_bytes] = '\0';
	return recv_bytes;
}


/**************************************************************************** 
* 函数名称 : UDPReadData
* 功能描述 : UDP协议套接字读取数据函数
* 参    数 : int sockfd		: 套接字
* 参    数 : char* buffer	: 缓冲
* 参    数 : int len		: 读取长度
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
#if 1
int CSocketEx::UDPReadData(int sockfd, char* buffer, int len)
{
	struct sockaddr_in peerAddr;
	int sockLen = sizeof(struct sockaddr_in);
#ifdef WIN32
	return recvfrom(sockfd, buffer, len, 0, (sockaddr *)&peerAddr, &sockLen);
#else
	return recvfrom(sockfd, buffer, len, 0, (sockaddr *)&peerAddr, (socklen_t *)&sockLen);
#endif
}
#else
int CSocketEx::UDPReadData(int sockfd, char* buffer, int len)
{
	m_sockLen = sizeof(struct sockaddr_in);
#ifdef WIN32
	return recvfrom(sockfd, buffer, len, 0, (sockaddr *)&m_peerAddr, &m_sockLen);
#else
	return recvfrom(sockfd, buffer, len, 0, (sockaddr *)&m_peerAddr, (socklen_t *)&m_sockLen);
#endif
}
#endif

/**************************************************************************** 
* 函数名称 : UDPReadData
* 功能描述 : 读取套接字数据函数
* 参    数 : int sockfd					: 套接字
* 参    数 : char* buffer				: 缓冲
* 参    数 : int len					: 读取长度
* 参    数 : char *peerip				: 远程ip地址
* 参    数 : unsigned short peerport	: 远程端口
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::UDPReadData(int sockfd, char *buffer, int len, char *peerip, unsigned short peerport)
{
	//int ret = 0;
	struct sockaddr_in peerAddr;
	int sockLen = 0;

	memset(&peerAddr, 0, sizeof(peerAddr));
	peerAddr.sin_family = AF_INET;
	peerAddr.sin_addr.s_addr = inet_addr(peerip);
	peerAddr.sin_port = htons(peerport);
#ifdef WIN32
	return recvfrom(sockfd, buffer, len, 0, (sockaddr *)&peerAddr, &sockLen);;
#else
	return recvfrom(sockfd, buffer, len, 0, (sockaddr *)&peerAddr, (socklen_t *)&sockLen);;
#endif
}

/**************************************************************************** 
* 函数名称 : ReadData
* 功能描述 : 读取数据函数
* 参    数 : int sockfd		: 套接字
* 参    数 : char* buffer	: 缓冲
* 参    数 : int len		: 读取长度
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::ReadData(int sockfd, char* buffer, int len)
{
	if (m_mode == PROTO_TCP)
	{
		return TCPReadData(sockfd, buffer, len);
	}
	return UDPReadData(sockfd, buffer, len);
}


/**************************************************************************** 
* 函数名称 : WriteData
* 功能描述 : 套接字写数据函数
* 参    数 : int sockfd		: 套接字
* 参    数 : char* buffer	: 缓冲
* 参    数 : int len		: 读取长度
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 :  
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::WriteData(int sockfd, char* buffer, int len)
{
	int ret = 0;
	int leftBytes = len;
	while(leftBytes > 0)
	{
		if (m_mode == PROTO_TCP)
		{
#ifdef WIN32
			ret = send(sockfd, buffer, leftBytes, 0);
#else
			ret = send(sockfd, buffer, leftBytes, MSG_NOSIGNAL);
#endif
		}
		else
		{
			ret = sendto(sockfd, buffer, leftBytes, 0, (sockaddr *)&m_peerAddr, sizeof(m_peerAddr));
		}
		if(ret <= 0)
		{
			zprintf(ERROR,"[ERRO]WriteData: leftBytes=%d, ret=%d, error(%d:%s)", leftBytes, ret, errno, strerror(errno));

			return len - leftBytes;
		}
		leftBytes -= ret;
		buffer += ret;
	}
	return len-leftBytes;
}

/**************************************************************************** 
* 函数名称 : WriteData
* 功能描述 : 套接字写数据函数
* 参    数 : int sockfd					: 套接字
* 参    数 : char* buffer				: 缓冲
* 参    数 : int len					: 读取长度
* 参    数 : char *peerip				: 远程ip地址
* 参    数 : unsigned short peerport	: 远程端口
* 返 回 值 : 成功:返回服务字节数; 失败:0
* 作    者 : hlzheng 
* 设计日期 : 
* 修改日期		  修改人		   修改内容  
 *****************************************************************************/
int CSocketEx::WriteData(int sockfd, char *buffer, int len, char *peerip, unsigned short peerport)
{
	int ret = 0;
	int leftBytes = len;
	struct sockaddr_in peerAddr;

	memset(&peerAddr, 0, sizeof(peerAddr));
	peerAddr.sin_family = AF_INET;
	peerAddr.sin_addr.s_addr = inet_addr(peerip);
	peerAddr.sin_port = htons(peerport);

	while(leftBytes > 0)
	{
		ret = sendto(sockfd, buffer, leftBytes, 0, (sockaddr *)&peerAddr, sizeof(struct sockaddr_in));
		if(ret <= 0)
		{
			return len - leftBytes;
		}
		leftBytes -= ret;
		buffer += ret;
	}
	return len-leftBytes;
}


bool CSocketEx::setLinger()
{
	return setLinger(m_sockfd);
}

bool CSocketEx::setLinger(int socket)
{
	struct linger sndOver;
	memset(&sndOver, 0, sizeof(struct linger));
	int iSndOverLength = 0;
	
	sndOver.l_onoff  = 1;
	sndOver.l_linger = 0;
	iSndOverLength  = sizeof sndOver;
	
	int ret = setsockopt(socket, SOL_SOCKET, SO_LINGER, (char*)&sndOver, iSndOverLength);
	if (ret < 0)
	{
		return false;
	}
	
	return true;
}
