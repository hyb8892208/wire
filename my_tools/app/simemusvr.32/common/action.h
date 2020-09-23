#ifndef ACTION_H_INCLUDED
#define ACTION_H_INCLUDED

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
 *  @file action.h
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
#include "astman.h"
#include "update.h"
/*******************************************************************************
 * @def esponse_is(M, RES)
 * @brief Get response Code
 ******************************************************************************/
#define response_is(M, RES)  ((strlen(astman_get_header(M, "Response")))&& \
                            (!strcasecmp(astman_get_header(M, "Response"), RES)))
/*******************************************************************************
 * @brief Action: GetConfig
 *        Synopsis: Retrieve configuration
 *        Privilege: system,config,all
 *        Description: A 'GetConfig' action will dump the contents
 *                      of a configuration file by category and contents or
 *                      optionally by specified category only.
 * @warning Variables: (Names marked with * are required)
 * @param   filename: Configuration filename (e.g. foo.conf)
 * @param category: Category in configuration file
 ******************************************************************************/
int astman_get_config(struct mansession *s, struct message *m,
                      char *filename, char * category, char *actionid);
/*******************************************************************************
 * @brief Action: SIPpeers
 *        Synopsis: List SIP peers (text format)
 *        Privilege: system,reporting,all
 *        Description: Lists SIP peers in text format with details on current status.
 *        Peerlist will follow as separate events, followed by a final event called
 *        PeerlistComplete.
 *
 * @param ActionID: <id>        Action ID for this transaction. Will be returned.
 ******************************************************************************/
int astman_sip_peers(struct mansession *s, struct message **m,
                     char *actionid);
/*******************************************************************************
 * @brief Action: SIPshowpeer
 *         Synopsis: Show SIP peer (text format)
 *        Privilege: system,reporting,all
 *        Description: Show one SIP peer with details on current status.
 *
 *        Peer: <name>           The peer name you want to check.
 *        ActionID: <id>          Optional action ID for this AMI transaction.
 *
 ******************************************************************************/
int astman_sip_show_peer(struct mansession *s, struct message *m,
                         char *peer, char *actionid);
/*******************************************************************************
 * @brief Action: SIPshowregistry
Synopsis: Show SIP registrations (text format)
Privilege: system,reporting,all
Description: Lists all registration requests and status
Registrations will follow as separate events. followed by a final event called
RegistrationsComplete.
Variables:
  ActionID: <id>       Action ID for this transaction. Will be returned.
 * @param ActionID: <id>        Action ID for this transaction. Will be returned.
 ******************************************************************************/
int astman_sip_show_registry(struct mansession *s, struct message **m,
                     char *actionid);
#endif // ACTION_H_INCLUDED
