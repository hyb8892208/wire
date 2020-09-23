//gcc -Wall https_test.c -lssl -o https_test

//./https_test https://s1.bichara.com.br:8181/chkporta.php?user=832700&pwd=sfhawz826&tn=8388166902

//arm-openwrt-linux-gcc https_test.c -lssl -o https_test -I ./sdk-comcerto-openwrt-7.0/staging_dir/toolchain-arm_gcc-3.4.6_uClibc-0.9.28/include/ -L ./sdk-comcerto-openwrt-7.0/staging_dir/toolchain-arm_gcc-3.4.6_uClibc-0.9.28/lib/ -lcrypto


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>


static int gdebug = 0;

/*Code from Unix network programming*/
int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)
{
	int flags, n, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0) {
		fprintf(stderr, "fcntl:%s\a\n", strerror(errno));
		return -1;
	}
	fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);

	error = 0;

	if ((n=connect(sockfd, saptr, salen)) < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}

	/* Do whatever we want while the connect is taking place. */
	if (n == 0) {
		goto done;	/*connect completed immediately*/
	}

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ((n=select(sockfd+1,&rset,&wset,NULL,nsec ? &tval : NULL)) == 0) {
		close(sockfd);		/* timeout */
		errno = ETIMEDOUT;
		return -1;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
			return -1;	/* Solaris pending error */
		}
	} else {
		fprintf(stderr, "select error: sockfd not set\n");
		return -1;
	}

done:
	fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
	if (error) {
		errno = error;
		return -1;
	}

	return 0;
}


//Get remote Mobile Number Portability
int get_mnp(const char* server, int port, const char* user, const char* password, const char* num, int timeo, char* savebuf, int savebuflen)
{
	int sockfd;
    char buffer[1024+1];
	char allbuf[1024+1];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int nbytes;
    char host_addr[256];
    char request[1024];
    int send, totalsend;

    SSL *ssl;
    SSL_CTX *ctx;

	savebuf[0] = '\0';
	allbuf[0] = '\0';

	if ((host = gethostbyname(server)) == NULL) {
		fprintf(stderr, "Gethostname %s error, %s\n", server, strerror(errno));
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
		return -1;
	}


/*	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0) {
		fprintf(stderr, "fcntl:%s\a\n", strerror(errno));
		return -1;
	}
	fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);*/

	struct timeval timeout = {timeo, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);

//	if (connect(sockfd, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) == -1) {
	if (connect_nonb(sockfd, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr), timeo) == -1) {
		fprintf(stderr, "Connect %s:%d Error:%s\a\n", server, port, strerror(errno));

		close(sockfd);
		return -1;
	}

	/* SSL初始化 */
	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
		ERR_print_errors_fp(stderr);

		close(sockfd);
		ERR_free_strings();
		return -1;
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		ERR_print_errors_fp(stderr);

		close(sockfd);
		SSL_CTX_free(ctx);
		ERR_free_strings();
		return -1;
	}

	/* 把socket和SSL关联 */
	if (SSL_set_fd(ssl, sockfd) == 0) {
		ERR_print_errors_fp(stderr);

		close(sockfd);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ERR_free_strings();
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0) {
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	if (SSL_connect(ssl) != 1) {
		ERR_print_errors_fp(stderr);

		close(sockfd);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ERR_free_strings();
		return -1;
	}

	snprintf(request,sizeof(request),
			"GET /chkporta.php?user=%s&pwd=%s&tn=%s HTTP/1.1\r\n" \
			"Accept: */*\r\n" \
			"Accept-Language: en-us\r\n" \
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n" \
			"Host: %s:%d\r\n" \
			"Connection: Close\r\n\r\n", 
			user,password,num,
			server,port);


	/*发送https请求request */
	send = 0;
	totalsend = 0;
	nbytes = strlen(request);
	while (totalsend < nbytes) {
		send = SSL_write(ssl, request + totalsend, nbytes - totalsend);
		if (send == -1) {
			ERR_print_errors_fp(stderr);

			close(sockfd);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
			ERR_free_strings();
			return -1;
		}
		totalsend += send;
	}

	/* 连接成功了，接收https响应，response */
	while ((nbytes = SSL_read(ssl, buffer, sizeof(buffer)-1)) > 0) {
		buffer[nbytes] = '\0';
		strncat(allbuf,buffer,sizeof(allbuf)-strlen(allbuf)-1);
	}

	/* 结束通讯 */
	if (SSL_shutdown(ssl) != 1) {
		ERR_print_errors_fp(stderr);

		close(sockfd);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ERR_free_strings();
		return -1;
	}

	close(sockfd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();

	if(gdebug) {
		printf("get:%s\n",allbuf);
	}

	if (allbuf[0] != '\0') {
		if (strstr(allbuf,"200 OK") > 0 ) {
			char *p;
			int len;
			char *find;
			p = allbuf;
			len = 0;
			while((find=strstr(p,"\r\n"))>0) {
				if(sscanf(p,"%x\r\n%1024[+0-9.]\r\n0\r\n",&len,buffer)>0) {
					if(strlen(buffer) == len) {
						strncpy(savebuf,buffer,savebuflen);
						return 0;
					}
				}
				p = find + sizeof("\r\n") - 1;
			}
		}
	}
	return -1;
}


//Get remote Mobile Number Portability
int get_mnp2(const char* server, int port, const char* user, const char* password, const char* num, int timeo, char* savebuf, int savebuflen)
{
	int sockfd;
    char buffer[1024+1];
	char allbuf[1024+1];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int nbytes;
    char host_addr[256];
    char request[1024];
    int sendi, totalsend;

	savebuf[0] = '\0';
	allbuf[0] = '\0';

	if ((host = gethostbyname(server)) == NULL) {
		fprintf(stderr, "Gethostname %s error, %s\n", server, strerror(errno));
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
		return -1;
	}

	struct timeval timeout = {timeo, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);

//	if (connect(sockfd, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) == -1) {
	if (connect_nonb(sockfd, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr), timeo) == -1) {
		fprintf(stderr, "Connect %s:%d Error:%s\a\n", server, port, strerror(errno));

		close(sockfd);
		return -1;
	}

	//http://direction.webupdate.com.br/Portabilidade.asp?usuario=alex&senha=Teste@123&telefone=8388166902&retorno=1
	snprintf(request,sizeof(request),
			"GET /Portabilidade.asp?usuario=%s&senha=%s&telefone=%s&retorno=1 HTTP/1.1\r\n" \
			"Accept: */*\r\n" \
			"Accept-Language: en-us\r\n" \
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n" \
			"Host: %s:%d\r\n" \
			"Connection: Close\r\n\r\n", 
			user,password,num,
			server,port);

	/*发送https请求request */
	sendi = 0;
	totalsend = 0;
	nbytes = strlen(request);
	while (totalsend < nbytes) {
		sendi = send(sockfd, request + totalsend, nbytes - totalsend, 0);
		if (sendi == -1) {
			close(sockfd);
			return -1;
		}
		totalsend += sendi;
	}


	/* 连接成功了，接收https响应，response */
	while ((nbytes = recv(sockfd, buffer, sizeof(buffer)-1, 0)) > 0) {
		buffer[nbytes] = '\0';
		strncat(allbuf,buffer,sizeof(allbuf)-strlen(allbuf)-1);
	}

	/* 结束通讯 */
	close(sockfd);

	if(gdebug) {
		printf("get:%s\n",allbuf);
/*
HTTP/1.1 200 OK
Connection: close
Date: Wed, 22 Jan 2014 07:11:45 GMT
Content-Type: text/html
Server: Microsoft-IIS/7.5
Cache-Control: private
Set-Cookie: ASPSESSIONIDSARSRSDQ=NBMNLKKDIFFPEFPODHPHIFBD; path=/
X-Powered-By: ASP.NET

0318388166902
*/
	}

	if (allbuf[0] != '\0') {
		if (strstr(allbuf,"200 OK") > 0 ) {
			char *p;
			int len;
			char *find;
			p = allbuf;
			len = 0;
			while((find=strstr(p,"\r\n"))>0) {
				buffer[0] = '\0';
				if(sscanf(p,"\r\n\r\n%1024[+0-9.]\r\n0\r\n",buffer)>0) {
					if (buffer[0] != '\0') {
						strncpy(savebuf,buffer,savebuflen);
						return 0;
					}
				}
				p = find + sizeof("\r\n") - 1;
			}
		}
	}
	return -1;
}


//https s1.bichara.com.br 8181 832700 sfhawz826 8388166902 2
//http direction.webupdate.com.br 80 alex Teste@123 8388166902 2
int main(int argc, char *argv[])
{
	char buf[1024];

	if (argc < 7) {
		return -1;
	}

	int port;
	port = atoi(argv[3]);
	if (port <= 0 ) {
		return -1;
	}

	int timeout;
	timeout = atoi(argv[7]);
	if (timeout <= 0) {
		return -1;
	}

	//s1.bichara.com.br 8181 832700 sfhawz826 8388166902 2
	//
	//./https_test https://s1.bichara.com.br:8181/chkporta.php?user=832700&pwd=sfhawz826&tn=8388166902
	/*if(0 == get_mnp("s1.bichara.com.br", 8181, "832700", "sfhawz826", "8388166902", buf, sizeof(buf))) {
		printf("%s\n",buf);
	}*/

	//http://direction.webupdate.com.br/Portabilidade.asp?usuario=alex&senha=Teste@123&telefone=8388166902&retorno=1

	if (argc >=9 ) {
		if(!strcmp(argv[8],"-d")) {
			gdebug = 1;
		}
	}

	if (!strcmp(argv[1],"http")) {
		if(0 == get_mnp2(argv[2], port, argv[4], argv[5], argv[6], timeout, buf, sizeof(buf))) {
			printf("%s",buf);
		} else {
			//printf("%s",argv[6]);
		}
	} else {
		if(0 == get_mnp(argv[2], port, argv[4], argv[5], argv[6], timeout, buf, sizeof(buf))) {
			printf("%s",buf);
		} else {
			//printf("%s",argv[6]);
		}
	}


	return 0;
}
