#ifndef ASTEVENT_H_INCLUDED
#define ASTEVENT_H_INCLUDED

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
 *  @file astevent.h
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
#define ASTMAN_HEADER_EVENT                 "Event"

#define ASTMAN_EVENT_PEER_ENTRY             "PeerEntry"
#define ASTMAN_EVENT_PEER_LIST_COMPLETE     "PeerlistComplete"

#define ASTMAN_EVENT_STATUS                 "Status"
#define ASTMAN_EVENT_STATUS_COMPLETE        "StatusComplete"

#define ASTMAN_EVENT_QUEUE_PARAMS           "QueueParams"
#define ASTMAN_EVENT_QUEUE_MEMBER           "QueueMember"
#define ASTMAN_EVENT_QUEUE_ENTER            "QueueEnter"
#define ASTMAN_EVENT_QUEUE_STATUS_COMPLETE  "QueueStatusComplete"

#define ASTMAN_EVENT_REGISTRATIONS_COMPLETE "RegistrationsComplete"
#define ASTMAN_EVENT_REGISTRY_ENTRY         "RegistryEntry"

/*******************************************************************************
 * @typedef (*ASTMAN_EVENT_CALLBACK)
 * @brief   CallBack Event proto-type
 ******************************************************************************/
typedef int (*ASTMAN_EVENT_CALLBACK)(struct mansession *, struct message *);

/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_add_event_handler_system(struct mansession *s, ASTMAN_EVENT_CALLBACK callback );

/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_sippeers_callback(struct mansession *s, struct message *m);

/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_queues_callback(struct mansession *s, struct message *m);

/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_status_callback(struct mansession *s, struct message *m);
/*******************************************************************************
 *  \fn astman_add_param(char *buf, int buflen, char *header, char *value)
 *  \brief  Add a new parameter to the Command
 *  \param  buf
 *  \param  buflen
 *  \param  header
 *  \param  value
 *  \return Number of wrote characters into the buf
 ******************************************************************************/
int astman_sipshowregistry_callback(struct mansession *s, struct message *m);
#endif // ASTEVENT_H_INCLUDED
