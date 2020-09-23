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
 *  @file astevent.c
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "astman.h"
#include "astevent.h"

/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_sippeers_callback(struct mansession *s __attribute__((unused)),
                             struct message *m) {
  char event[80];
  strncpy(event, astman_get_header(m, ASTMAN_HEADER_EVENT), sizeof(event));

  if (!strcasecmp(event, ASTMAN_EVENT_PEER_ENTRY) ||
      !strcasecmp(event, ASTMAN_EVENT_PEER_LIST_COMPLETE)) {
    return 1; /* return wait_for_answer() */
  }
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
int astman_sipshowregistry_callback(struct mansession *s __attribute__((unused)),
                             struct message *m) {
  char event[80];
  strncpy(event, astman_get_header(m, ASTMAN_HEADER_EVENT), sizeof(event));

  if (!strcasecmp(event, ASTMAN_EVENT_REGISTRY_ENTRY) ||
      !strcasecmp(event, ASTMAN_EVENT_REGISTRATIONS_COMPLETE)) {
    return 1; /* return wait_for_answer() */
  }
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
int astman_queues_callback(struct mansession *s __attribute__((unused)),
                           struct message *m) {
  char event[80];
  strncpy(event, astman_get_header(m, ASTMAN_HEADER_EVENT), sizeof(event));

  if (!strcasecmp(event, ASTMAN_EVENT_QUEUE_ENTER) ||
      !strcasecmp(event, ASTMAN_EVENT_QUEUE_MEMBER) ||
      !strcasecmp(event, ASTMAN_EVENT_QUEUE_PARAMS)  ||
      !strcasecmp(event, ASTMAN_EVENT_QUEUE_STATUS_COMPLETE)) {
    return 1; /* return wait_for_answer() */
  }
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
int astman_status_callback(struct mansession *s __attribute__((unused)), struct message *m) 
{
	char event[80];
	
	strncpy(event, astman_get_header(m, ASTMAN_HEADER_EVENT), sizeof(event));

	if (!strcasecmp(event, ASTMAN_EVENT_STATUS)  || !strcasecmp(event, ASTMAN_EVENT_STATUS_COMPLETE)) {
		return 1; /* return wait_for_answer() */
	}
	
	return 0;
}
