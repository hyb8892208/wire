/*******************************************************************************
 * astapi - library for using Asterisk Manager API.
 * Copyright (C) 2010 Baligh GUESMI
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc.,
 ******************************************************************************/
/*******************************************************************************
 *  @file astman.c
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h> /* send/recv */
#include <netinet/in.h>  /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntoa function */
#include <netdb.h>  /* gethostbyname */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <stdarg.h>  /* vsnprintf */
#include <string.h>
#include <errno.h>
#include <time.h>   /* time */
#include <pthread.h>
#include "astman.h"
#include "astevent.h"
#include "astlog.h"
/*******************************************************************************
 *  \def ASTMAN_DEFAULT_MANAGER_PORT
 *  \brief  Default port used to connect to the AMI Asterisk
 ******************************************************************************/
#define ASTMAN_DEFAULT_MANAGER_PORT 5038
/*******************************************************************************
 *  \def ASTMAN_DEFAULT_EVENT
 *  \brief  Default Event to register over
 ******************************************************************************/
#define ASTMAN_DEFAULT_EVENT "DEFAULT"
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_add_param(char *buf, int buflen, char *header, const char *value) 
{
	if (!astman_strlen_zero(value))
		return snprintf(buf+strlen(buf), buflen-strlen(buf)-1, "%s: %s\r\n", header, value);
	return 0;
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
void astman_dump_message(struct message *m) 
{
	int x;
	astlog(ASTLOG_INFO, "< Received:");
	for (x=0;x<m->hdrcount;x++) {
		astlog(ASTLOG_INFO, "< %s", m->headers[x]);
	}
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
static void astman_dump_out_message(char *message) 
{
	char *s;
	astlog(ASTLOG_INFO, "> Transmitted:");
	//astlog(ASTLOG_INFO, "> Action: %s", action);
	while (*message) {
		s = message;
		
		while (*s && (*s != '\r')) 
			s++;
		
		if (!*s)
			break;
		
		*s = '\0';
		astlog(ASTLOG_INFO, "> %s", message);
		
		s+=2;
		message = s;
	}
}

/*******************************************************************************
 *  \fn 
 *  \brief  
 *  \param  
 *  \param  
 *  \return 
 ******************************************************************************/
int astman_open(struct mansession *s) 
{
	int so;

	so = socket(AF_INET, SOCK_STREAM, 0);
	if (so < 0) {
		perror("socket");
		return -1;
	}
	s->fd = so;

	return 0;
}


void astman_close(struct mansession *s) 
{
	if (s->fd) {
		close(s->fd);
		s->fd = 0;
	}
}

/*******************************************************************************
 *  \fn 
 *  \brief  
 *  \param  
 *  \param  
 *  \return 
 ******************************************************************************/
int astman_connect(struct mansession *s, char *hostname, int port) 
{
	struct hostent *hp;

	s->fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( s->fd < 0 ) {
		perror("socket");
		return -1;
	}

	s->sin.sin_family = AF_INET;
	if (port > 0){
		s->sin.sin_port = htons(port);
	}else{
		s->sin.sin_port = htons(ASTMAN_DEFAULT_MANAGER_PORT);
	}
	if(NULL != hostname){
		hp = gethostbyname(hostname);
		if (!hp) {
			astlog(ASTLOG_ERROR, "No such address: %s", hostname);
			return -1;
		}
		memcpy(&(s->sin.sin_addr), hp->h_addr, sizeof(s->sin.sin_addr));		
	}else{
		s->sin.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	if ( connect(s->fd, (struct sockaddr *) &(s->sin), sizeof(s->sin)) < 0 ) {
		perror("connect");
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * @fn astman_add_param(char *buf, int buflen, char *header, char *value)
 * @brief  Add a new parameter to the Command
 * @param  s:
 * @param  output
 * @return
 * @warning output must have at least sizeof(s->inbuf) space
 ******************************************************************************/
static int astman_get_input(struct mansession *s, char *output) 
{
	/* output must have at least sizeof(s->inbuf) space */
	int res = 0;
	unsigned int x = 0;
	//Modify qmd 2014-04-02 16:39
	//struct timeval tv = {0,0};
	struct timeval tv = {3,0};
	fd_set fds;
	
	for (x=1;x<s->inlen;x++) {
		if ((s->inbuf[x] == '\n')) {
			/* Copy output data up to and including \r\n */
			memcpy(output, s->inbuf, x + 1);
			/* Add trailing \0 */
			output[x+1] = '\0';
			/* Move remaining data back to the front */
			memmove(s->inbuf, s->inbuf + x + 1, s->inlen - x);
			s->inlen -= (x + 1);
			return 1;
		}
	}

	if (s->inlen >= sizeof(s->inbuf) - 1) {
		astlog(ASTLOG_ERROR, "Dumping long line with no return from %s: [%d/%u] %s", 
			inet_ntoa(s->sin.sin_addr), 
			s->inlen, 
			sizeof(s->inbuf) - 1,
			s->inbuf);
		s->inlen = 0;
	}
	
	FD_ZERO(&fds);
	FD_SET(s->fd, &fds);
	res = select(s->fd + 1, &fds, NULL, NULL, &tv);
	if (res < 0) {
		astlog(ASTLOG_ERROR, "Select returned error: %s", strerror(errno));
	} else if (res > 0) {
		res = recv(s->fd, s->inbuf + s->inlen, sizeof(s->inbuf) - 1 - s->inlen, 0);

		if (res < 0){
			dlog(s->debug & DEBUG_LEVEL4, "recv [%s] res=[%d]\n", s->inbuf + s->inlen, res);
			return ASTMAN_FAILURE;
		}else if(res == ASTMAN_SOCKCLOSED){
			return ASTMAN_SOCKCLOSED;
		}
		s->inlen += res;
		s->inbuf[s->inlen] = '\0';
	} else {
		return ASTMAN_TIMEOUT;
	}
	
	return 3;
}
/*******************************************************************************
 * @fn char *astman_get_header(struct message *m, const char *var)
 * @brief
 * @warning
 * @return
 ******************************************************************************/
char *astman_get_header(struct message *m, const char *var) 
{
	char cmp[80];
	int x;
	snprintf(cmp, sizeof(cmp), "%s: ", var);
	for (x=0;x<m->hdrcount;x++)
		if (!strncasecmp(cmp, m->headers[x], strlen(cmp)))
			return m->headers[x] + strlen(cmp);

	return "";
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
static int astman_process_message(struct mansession *s, struct message *m) 
{
	int x;
	int res;
	char event[80];

	strncpy(event, astman_get_header(m, "Event"), sizeof(event));
	if (!strlen(event)) {
		astlog(ASTLOG_ERROR, "Missing event in request");
		return 0;
	}
	if (s->debug & DEBUG_LEVEL4) {
		astlog(ASTLOG_DEBUG, "Got event packet: %s", event);
		for (x=0;x<m->hdrcount;x++) {
			astlog(ASTLOG_DEBUG, "Header: %s", m->headers[x]);
		}
	}
	for (x=0; x < s->eventcount; x++) {
		if (s->events[x].event && !strcasecmp(event, s->events[x].event)) {
			res = s->events[x].func(s, m);
			if (res < 0) {
				return -1;
			} else if (res > 0) {
				return res;
			}
			break;
		}
		/* Execute system event handler */
		if (s->events[x].event && !strcasecmp(ASTMAN_DEFAULT_EVENT, s->events[x].event)) {
			res = s->events[x].func(s, m);
			if (res < 0) {
				return -1;
			} else if (res > 0) {
				return res;
			}
			break;
		}
	}
	
	if ((s->debug & DEBUG_LEVEL4) && x >= s->eventcount)
		astlog(ASTLOG_DEBUG, "Ignoring unknown event '%s'", event);

	return 0;
}
/*******************************************************************************
 *  \fn int astman_wait_for_response(struct mansession *s, struct message *msg, char *actionid, time_t timeout)
 *  \brief
 *  \return
 ******************************************************************************/
int astman_wait_for_response(struct mansession *s, struct message *msg, char *actionid, time_t timeout) 
{
	int res = 0;
	int proc_ev;
	char *sp;
	time_t begin, end;
	int ret = -1;
	struct message m;

	m.hdrcount = 0;
	m.gettingdata = 0;

	time(&begin);

	for (;;) {
		res = 0;
		res = astman_get_input(s, m.headers[m.hdrcount]);
		dlog((s->debug & DEBUG_LEVEL4) && res != ASTMAN_TIMEOUT, "res=[%d] hearders=[%s] gettingdata=[%d] hdrcount=[%d]\n", res, 
			m.headers[m.hdrcount], 
			m.gettingdata,
			m.hdrcount);
		if (res == 1) { /* get single line */
			if (!m.gettingdata) {
				/* Strip trailing \r\n */
				if (strlen(m.headers[m.hdrcount]) < 2)
					continue;
				m.headers[m.hdrcount][strlen(m.headers[m.hdrcount]) - 2] = '\0';
				/* finish line */
				if (strlen(m.headers[m.hdrcount]) == 0) {
					if (s->debug & DEBUG_LEVEL4)
						astman_dump_message(&m);
					/* Response packet */
					if (strlen(astman_get_header(&m, "Response"))) {
						//astlog(ASTLOG_INFO, "response=%s", astman_get_header(&m, "Response"));
						if ((!strncasecmp(astman_get_header(&m, "Response"), "Success", sizeof("Success")-1))) {
							if((actionid != NULL && !strncasecmp(astman_get_header(&m, "ActionId"), actionid, strlen(actionid)))
								|| actionid == NULL){
								memcpy(msg, &m, sizeof(struct message));
								memset(&m, 0, sizeof(struct message));
								ret = ASTMAN_SUCCESS;
								goto Exit;
							}else{
								continue;
							}
						}else if (!strncasecmp(astman_get_header(&m, "Response"), "Follows", sizeof("Follows")-1)) {
							if((actionid != NULL && !strncasecmp(astman_get_header(&m, "ActionId"), actionid, strlen(actionid)))
								|| actionid == NULL){
								memcpy(msg, &m, sizeof(struct message));
								memset(&m, 0, sizeof(struct message));
								ret = ASTMAN_SUCCESS;
								goto Exit;
							}else{
								continue;
							}
						} else {
							memcpy(msg, &m, sizeof(struct message));
							memset(&m, 0, sizeof(struct message));
							ret = ASTMAN_FAILURE;
							goto Exit;
						}
					}
					/* Event packet */
					if ((proc_ev = astman_process_message(s, &m)) < 0) {
						/* Error */
						break;
					/* Complete */
					} else if ( proc_ev > 0 ) {
						memcpy(msg, &m, sizeof(m));
						memset(&m, 0, sizeof(m));
						ret = ASTMAN_SUCCESS;
						goto Exit;
					}
					memset(&m, 0, sizeof(m));
				} else if (m.hdrcount < MAX_HEADERS - 1) {
					/* headers in packet */

					/* Response: Follows */
					if (!strncasecmp(m.headers[m.hdrcount], "Response: Follows", strlen("Response: Follows"))) {
						m.gettingdata = 1;
					}
					m.hdrcount++;
				}
			} else {
				/* Get raw data from command (m.gettingdata = 1) */
				if ((sp = strstr(m.headers[m.hdrcount], "--END COMMAND--"))) {
					/* End of getting data */
					*sp = '\0';
					m.gettingdata = 0;
				}
				if (m.hdrcount < MAX_HEADERS - 1)
					m.hdrcount++;
			}
		} else if (res < 0) {
			ret =  ASTMAN_FAILURE;
			goto Exit;

		} else if (res == ASTMAN_SOCKCLOSED) {
			ret =  ASTMAN_SOCKCLOSED;
			goto Exit;
		} else if (res == ASTMAN_TIMEOUT) {
			if ( timeout > 0 ) {
				time(&end);
				if ( (end - begin) > timeout ) {
					ret = ASTMAN_TIMEOUT;
					printf("[%d %s]timeout\n", __LINE__, __FUNCTION__);
					goto Exit;
				}
			}
		}
	} /* end loop */
Exit:
	return ret;
}
/*******************************************************************************
 * @fn int astman_manager_action(struct mansession *s, char *action, char *fmt, ...)
 * @brief
 * @return
 ******************************************************************************/
int astman_manager_action(struct mansession *s, char *action, char *fmt, ...) 
{
	char tmp[4096];
	va_list ap;
	
	snprintf(tmp, sizeof(tmp), "Action: %s\r\n", action);
	va_start(ap, fmt);
	vsnprintf(tmp+strlen(tmp), sizeof(tmp)-strlen(tmp), fmt, ap);
	va_end(ap);
	strcpy(tmp+strlen(tmp), "\r\n");

	send(s->fd, tmp, strlen(tmp), 0);

	if (s->debug & DEBUG_LEVEL3)
		astman_dump_out_message(tmp);
	
	return 0;
}
/*******************************************************************************
 *  \fn int astman_manager_action_params(struct mansession *s, char *action, char *params)
 *  \brief
 *  \return
 ******************************************************************************/
int astman_manager_action_params(struct mansession *s, char *action, char *params) 
{
	return astman_manager_action(s, action, "%s", params);
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_add_event_handler(struct mansession *s, char *event, ASTMAN_EVENT_CALLBACK callback ) 
{
	int x;
	int ret = 1;
	
	if (s->eventcount >= MAX_EVENTS) {
		return -1;
	}

	for (x=0; x < s->eventcount; x++) {
		if (s->events[x].event && !strcasecmp(event, s->events[x].event)) {
			if (!callback) {
				/* Remove event handler */
				if (s->events[x].event) 
					free(s->events[x].event);
				for (; x+1 < s->eventcount; x++) {
					s->events[x] = s->events[x+1];
				}
				s->eventcount--;
				ret = 0;
				goto Exit;
			} else {
				astlog(ASTLOG_INFO, "%s handler is already defined, not over-writing.", event);
				ret = -1;
				goto Exit;
			}
		}
	}

	/* Add event handler */
	s->events[s->eventcount].event = strdup(event);
	s->events[s->eventcount].func = callback;
	s->eventcount++;
Exit:
	
	return ret;
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_add_event_handler_system(struct mansession *s, ASTMAN_EVENT_CALLBACK callback ) 
{
	return astman_add_event_handler(s, ASTMAN_DEFAULT_EVENT, callback );
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_login(struct mansession *s, char *hostname, int port, char *username, char *secret) 
{
	struct message m;
	int res;
	int ret = ASTMAN_FAILURE;
	struct hostent *hp;

	if (astman_strlen_zero(username) || astman_strlen_zero(secret))
		return ASTMAN_FAILURE;

	s->fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( s->fd < 0 ) {
		perror("socket");
		return -1;
	}

	s->sin.sin_family = AF_INET;
	if (port > 0){
		s->sin.sin_port = htons(port);
	}else{
		s->sin.sin_port = htons(ASTMAN_DEFAULT_MANAGER_PORT);
	}
	if(NULL != hostname){
		hp = gethostbyname(hostname);
		if (!hp) {
			astlog(ASTLOG_ERROR, "No such address: %s", hostname);
			return -1;
		}
		memcpy(&(s->sin.sin_addr), hp->h_addr, sizeof(s->sin.sin_addr));		
	}else{
		s->sin.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	if ( connect(s->fd, (struct sockaddr *) &(s->sin), sizeof(s->sin)) < 0 ) {
		perror("connect");
		close(s->fd);
		s->fd = 0;
		return -1;
	}

	astman_manager_action(s, "Login",
		"Username: %s" CRLF
		"Secret: %s" CRLF
		"Events: on" CRLF,
		username,
		secret);
	
	res = astman_wait_for_response(s, &m, NULL, 10000);
	if (res > 0 && !strcasecmp(astman_get_header(&m, "Response"), "Success")){
		ret = ASTMAN_SUCCESS;
	}else{
		ret = ASTMAN_FAILURE;
	}

	return ret;
}
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_logoff(struct mansession *s) 
{
	if (s->fd > 0) {
		astman_manager_action(s, "Logoff", "");
		//astman_wait_for_response(s, &m, NULL, 0);
		close(s->fd);
		s->fd = 0;
	}
	
	return ASTMAN_SUCCESS;
}
