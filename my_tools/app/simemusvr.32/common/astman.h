#ifndef ASTMAN_H_INCLUDED
#define ASTMAN_H_INCLUDED

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
 *  @file astman.h
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
 #include <netinet/in.h>  /* struct sockaddr_in */
 #include "astapi.h"
/*******************************************************************************
 *  @def    CRLF
 *  @brief
 ******************************************************************************/
#define CRLF "\r\n"
/*******************************************************************************
 * @struct  message
 * @brief   The struct representing the message command to send
 ******************************************************************************/
struct message {
  int hdrcount;                         /**!< Header count */
  int gettingdata;                      /**!< data */
  char headers[MAX_HEADERS][MAX_LEN];   /**!< Headers list */
} __attribute__((packed));
/*******************************************************************************
 * @struct  mansession
 * @brief   The struct of an opened AMI session
 ******************************************************************************/
struct mansession {
  int fd;                   /**!< the discriptor to the socket */
  char inbuf[MAX_LEN];      /**!< buffer */
  unsigned int inlen;                /**!< length of the buffer */
  struct sockaddr_in sin;   /**!< address of the socket */
  struct event {
    char *event;    /**!< the event ID */
    int (*func)(struct mansession *s, struct message *m); /**!< callback associated to this event */
  } events[MAX_EVENTS]; /**!< event registred to */
  int eventcount; /**!< the real count of event that the session is registred to */
  int debug:1;    /**!< active/desactivated DEBUG */
} __attribute__((packed));
/*******************************************************************************
 * @fn  astman_strlen_zero(const char *s)
 * @brief inline function to test if the given char is not empty
 * @param IN s
 * @return >0 if the given char is not empty
 ******************************************************************************/
static inline __attribute__((always_inline)) int astman_strlen_zero(const char *s)
{
  return (!s || (*s == '\0'));
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
int astman_add_param(char *buf, int buflen, char *header, const char *value);
/*******************************************************************************
 *  \fn int astman_manager_action_params(struct mansession *s, char *action, char *params)
 *  \brief
 *  \return
 ******************************************************************************/
int astman_manager_action_params(struct mansession *s, char *action, char *params);
/*******************************************************************************
 *  \fn int astman_wait_for_response(struct mansession *s, struct message *msg, time_t timeout)
 *  \brief
 *  \return
 ******************************************************************************/
int astman_wait_for_response(struct mansession *s, struct message *msg, time_t timeout);
/*******************************************************************************
 * @fn char *astman_get_header(struct message *m, const char *var)
 * @brief
 * @warning
 * @return
 ******************************************************************************/
char *astman_get_header(struct message *m, const char *var);
/*******************************************************************************
 * @fn int astman_manager_action(struct mansession *s, char *action, char *fmt, ...)
 * @brief
 * @return
 ******************************************************************************/
int astman_manager_action(struct mansession *s, char *action, char *fmt, ...);
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
struct mansession *astman_open(void);
struct mansession *astman_open_r(struct mansession *);

/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_connect(struct mansession *s, char *hostname, int port);
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_login(struct mansession *s, char *username, char *secret);
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_logoff(struct mansession *s);
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
void astman_disconnect(struct mansession *s);
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
void astman_dump_message(struct message *m);
#endif // ASTMAN_H_INCLUDED
