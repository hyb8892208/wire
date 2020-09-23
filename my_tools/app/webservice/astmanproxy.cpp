/*
  *  Filename: astmanproxy.cpp  Version: 0.1 
  *  
  *  Copyright (C) OPVX QMD
  *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
  *  Reference Stephen F. Booth
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU Lesser General Public
  *  License as published by the Free Software Foundation; either
  *  version 3 of the License, or (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  *  Lesser General Public License for more details.
  *
  *  You should have received a copy of the GNU Lesser General Public
  *  License along with this library; if not, write to the Free Software
  *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
  */

#include "common.h"

int amp_get_header(char *src, char *key, char *value, int vallen)
{
	char *start = src, *end = src, *find = NULL;
	char tmp[256];

	if(src == NULL || key == NULL || value == NULL || vallen <= 0){
		return -1;
	}

	while((end = strstr(start, "\r\n")) != NULL){
		if(strncmp(start, key, strlen(key)) == 0){
			memset(tmp, 0, sizeof(tmp));
			snprintf(tmp, sizeof(tmp), "%s:%%%d[^\r\n]", key, vallen);
			if(sscanf(start, tmp, value) == 1){
				trim(value);
				return 0;
			}
		}
		start = end + sizeof("\r\n") - 1;		
	}
	
	return -1;
}

int amp_check_package(char *pack, char *server, char *actionid)
{
	char value[256];

	dlog(DEBUG_LEVEL5, "server[%s] actionid[%s]\n", server, actionid);	
	if(pack == NULL){
		return -1;
	}

	if(server != NULL && *server != '\0' ){
		memset(value, 0, sizeof(value));
		if(amp_get_header(pack, ASTMANPROXY_HEADER_SERVER, value, sizeof(value)) != 0
			|| strcmp(server, value) != 0
		){
			return -1;
		}
	}

	if(actionid != NULL && *actionid != '\0'){
		memset(value, 0, sizeof(value));
		if(amp_get_header(pack, ASTMANPROXY_HEADER_ACTIONID, value, sizeof(value)) != 0
			|| strcmp(actionid, value) != 0
		){
			return -1;
		}		
	}

	return 0;
}

int amp_action(struct service_info *service, struct amp_command *cmd, char *outbuf, int outlen)
{
	time_t begin, now;
	char value[MAX_LEN_NAME];
	char *p = NULL, *start = NULL, *end = NULL;
	int ret = -1, rest = 0, len = 0;

	if(service->amp_fd <= 0 || cmd == NULL){
		return -1;
	}

	ret = send(service->amp_fd, cmd->command, strlen(cmd->command), 0);
	if (ret <= 0) {        // client close
		close(service->amp_fd);
		service->amp_fd = 0;
		return -1;
	}

	if(outbuf == NULL || outlen <= 0){
		return 0;
	}

	time(&begin);
	p = outbuf;
	rest = outlen;
	while(1){
		ret = recv(service->amp_fd, p, rest-1, 0);
		//dlog(DEBUG_LEVEL3, "recv ret[%d] rest[%d] outbuf[%s]\n", ret, rest-1, outbuf);
		if(ret == 0){
			dlog(DEBUG_LEVEL1, "socket closed ret[%d] outbuf[%s], close\n", ret, outbuf);
			close(service->amp_fd);
			service->amp_fd = 0;
			return -1;
		}else if(ret < 0){
			if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
				dlog(DEBUG_LEVEL1, "recv again ret[%d] [%s]\n", ret, outbuf);
			}else{
				/* client socket error , then close socket */
				dlog(DEBUG_LEVEL1, "socket error ret[%d] outbuf[%s], close\n", ret, outbuf);
				close(service->amp_fd);
				service->amp_fd = 0;
				return -1;
			}
		}
		
		p += ret;
		rest -= ret;
		start = outbuf;

		/* parse recv string */
		while((end = strstr(start, "\r\n\r\n")) != NULL){
			dlog(DEBUG_LEVEL3, "\n");
			/* find one AMI package */
			*(end + sizeof("\r\n") - 1) = '\0';
			dlog(DEBUG_LEVEL3, "recv package[%s]\n", start);
			if(amp_check_package(outbuf, cmd->server, cmd->actionid) == 0){
				return 0;
			}
			start = end + sizeof("\r\n\r\n") - 1;
		}

		if(start != outbuf){
			len = strlen(start);
			memmove(outbuf, start, len);
			memset(outbuf + len, 0, outlen - len);
			p = outbuf + len;
			rest = outlen - len;
		}
		
		if(service->amp_timeout > 0){
			time(&now);
			if((now - begin) > service->amp_timeout) {
				dlog(DEBUG_LEVEL1, "recv timeout [%s]\n", outbuf);
				return -1;
			}
		}
	}
	dlog(DEBUG_LEVEL1, "recv ok [%s]\n", outbuf);

	return 0;
}

int amp_ping(struct service_info *service)
{
	struct timeval tv;
	struct amp_command cmd;
	char buffer[MAX_LEN_BUFFER];
	int ret = -1;

	if(service->amp_fd <= 0){
		return -1;
	}

	/* astmanproxy logoff */
	gettimeofday(&tv, NULL);
	memset(&cmd, 0, sizeof(cmd));
	snprintf(cmd.actionid, sizeof(cmd.actionid), "%ld.%ld", tv.tv_sec, tv.tv_usec);
	snprintf(cmd.command, sizeof(cmd.command), "Action: Ping\r\n"\
		 "ActionID: %s\r\n"\
		 "\r\n",
		 cmd.actionid); // set default first asterisk server
	dlog(DEBUG_LEVEL1, "send command: [%s]\n", cmd.command);
	
	memset(buffer, 0, sizeof(buffer));
	ret = amp_action(service, &cmd, buffer, sizeof(buffer));
	dlog(DEBUG_LEVEL1, "recv buffer [%s]\n", buffer);
	if(ret == 0){
		char tmp[32];
		memset(tmp, 0, sizeof(tmp));
		if(amp_get_header(buffer, ASTMANPROXY_HEADER_PING, tmp, sizeof(tmp)) == 0
			&& strcasecmp(tmp, ASTMANPROXY_HEADER_PONG) == 0
		){
			dlog(DEBUG_LEVEL1, "server is alive. [%s]\n", tmp);
			return 0;
		}
	}
	dlog(DEBUG_LEVEL1, "server is not alive\n");
	
	return -1;
}

int amp_login(struct service_info *service)
{
	struct timeval timeout = {0,100000}, tv;
	struct sockaddr_in my_addr;
	int client_sockfd = -1;

	struct amp_command cmd;
	char buffer[MAX_LEN_BUFFER];
	char tmp[MAX_LEN_NAME];
	
	/*
	  *  create port socket 
	  *
	  */
	dlog(DEBUG_LEVEL1, "socket\n");
	if((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		dlog(DEBUG_LEVEL1, "Cannot create communication socket.\n");
		return -1;
	}

	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(service->ami_cfg.port);

	dlog(DEBUG_LEVEL1, "connect port[%d]\n", service->ami_cfg.port);
	if (connect_nonb(client_sockfd, (struct sockaddr *)(&my_addr), sizeof(struct sockaddr), service->amp_timeout) < 0) {
		dlog(DEBUG_LEVEL1, "connect fail\n");
		close(client_sockfd);
		return -1;
	}

	setsockopt(client_sockfd, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	setsockopt(client_sockfd, SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	service->amp_fd = client_sockfd;
	dlog(DEBUG_LEVEL1, "socket created [%d]\n", service->amp_fd);

	/*    
	  *    astmanproxy login 
	  *
	  */
	gettimeofday(&tv, NULL);
	memset(&cmd, 0, sizeof(cmd)); // login cannot check server and actionid
	snprintf(cmd.command, sizeof(cmd.command), "Action: Login\r\n"\
		 "Username: %s\r\n"\
		 "Secret: %s\r\n"\
		 "ActionID: %ld.%ld\r\n"\
		 "\r\n",
		 service->ami_cfg.username,
		 service->ami_cfg.password,
		 tv.tv_sec, tv.tv_usec);
	dlog(DEBUG_LEVEL1, "send command: [%s]\n", cmd.command);

	/* send comand */
	memset(buffer, 0, sizeof(buffer));
	if(amp_action(service, &cmd, buffer, sizeof(buffer)) != 0){
		return -1;
	}

	dlog(DEBUG_LEVEL1, "recv buffer: [%s]\n", buffer);
	if(strstr(buffer, ASTMANPROXY_KEYWORD_AUTH) != NULL){
		return 0;
	}

	return -1;
}

int amp_logoff(struct service_info *service)
{
	struct timeval tv;
	struct amp_command cmd;

	if(service->amp_fd <= 0){
		return -1;
	}

	/* astmanproxy logoff */
	gettimeofday(&tv, NULL);
	memset(&cmd, 0, sizeof(cmd));
	snprintf(cmd.command, sizeof(cmd.command), "Action: Logoff\r\n"\
		 "ActionID: %ld.%ld\r\n"\
		 "\r\n",
		 tv.tv_sec, tv.tv_usec);
	dlog(DEBUG_LEVEL3, "command: [%s]\n", cmd.command);

	amp_action(service, &cmd, NULL, 0);
	close(service->amp_fd);
	service->amp_fd = 0;
	service->recv_len = 0;
	memset(service->recv_buf, 0, sizeof(service->recv_buf));
	
	return 0;
}


