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
 *  @file action.c
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "astman.h"
#include "astevent.h"
#include "astlog.h"
#include "action.h"
/*******************************************************************************
 *
 ******************************************************************************/
//*** MSGBUF macros ***
struct msgbuf {
  struct message *msg;
  int count;
  int len;
};
/*******************************************************************************
 *
 ******************************************************************************/
#define MSGBUF_INIT(L)	\
	struct msgbuf _buf; \
	_buf.msg = (struct message *)calloc(L, sizeof(struct message)); \
	_buf.count = 0; \
	_buf.len = L; \
	if (!_buf.msg) return ASTMAN_FAILURE;
/*******************************************************************************
 *
 ******************************************************************************/
#define MSGBUF_ADD(M)	\
	memcpy(_buf.msg+_buf.count, M, sizeof(struct message)); \
	_buf.count++; \
	if (_buf.count >= _buf.len) { \
		_buf.msg = (struct message *)realloc(_buf.msg, _buf.len * 2 * sizeof(struct message)); \
		memset(_buf.msg+_buf.len, 0, _buf.len * sizeof(struct message)); \
		_buf.len *= 2; \
	}
/*******************************************************************************
 *
 ******************************************************************************/
#define MSGBUF_MSG _buf.msg;
/*******************************************************************************
 *
 ******************************************************************************/
#define MSGBUF_NB _buf.count;

#define AMIMAN_ACTIONID_MAX 32
static void get_actionid(char *actionid, int len)
{
	char str[64] = "00123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; 
	char s[2];
	int i;       
	srand((unsigned int)time((time_t *)NULL));
	for(i = 1; i <= len; i++){
		sprintf(s,"%c",str[(rand()%62)+1]);
		strcat(actionid,s);
	}
}

/*******************************************************************************
 * @fn astman_originate(struct mansession *s, struct message *m,
 *		     char *channel,
 *		     char *exten, char *context, int priority,
 *		     char *application, char *data,
 *		     int timeout,
 *		     char *callerid,
 *		     char *variable,
 *		     char *account,
 *		     int async,
 *		     char *actionid)
 * @brief ASTERISK AMI ACTION Originate
 * @param Channel: Channel name to call(required)
 * @param Exten: Extension to use (requires 'Context' and 'Priority')
 * @param Context: Context to use (requires 'Exten' and 'Priority')
 * @param Priority: Priority to use (requires 'Exten' and 'Context')
 * @param Application: Application to use
 * @param Data: Data to use (requires 'Application')
 * @param Timeout: How long to wait for call to be answered (in ms)
 * @param CallerID: Caller ID to be set on the outgoing channel
 * @param Variable: Channel variable to set, multiple Variable: headers are allowed
 * @param Account: Account code
 * @param Async: Set to 'true' for fast origination
 *
 * Generates an outgoing call to a Extension/Context/Priority or Application/Data
 ******************************************************************************/

int astman_originate(struct mansession *s, struct message *m,
		     char *channel,
		     char *exten, char *context, int priority,
		     char *application, char *data,
		     int timeout,
		     char *callerid,
		     char *variable,
		     char *account,
		     int async,
		     char *actionid) {
  int res;
  char params[MAX_LEN*12] = "";
  char strbuf[12];

	if (astman_strlen_zero(channel))
		return ASTMAN_FAILURE;

	if (!astman_strlen_zero(exten) && !astman_strlen_zero(context) && priority > 0 ) {
		astman_add_param(params, sizeof(params), "Exten", exten);
		astman_add_param(params, sizeof(params), "Context", context);
		snprintf(strbuf, sizeof(strbuf), "%d", priority);
		astman_add_param(params, sizeof(params), "Priority", strbuf);
	} else if (astman_add_param(params, sizeof(params), "Application", application) > 0) {
		astman_add_param(params, sizeof(params), "Data", data);
	} else {
		return ASTMAN_FAILURE;
	}

	if (timeout > 0) {
		snprintf(strbuf, sizeof(strbuf), "%d", timeout);
		astman_add_param(params, sizeof(params), "Timeout", strbuf);
	}
	astman_add_param(params, sizeof(params), "Channel", channel);
	astman_add_param(params, sizeof(params), "CallerId", callerid);
	astman_add_param(params, sizeof(params), "Variable", variable);
	astman_add_param(params, sizeof(params), "Account", account);
	astman_add_param(params, sizeof(params), "Async", (async)?"true":"false");
	astman_add_param(params, sizeof(params), "ActionId", actionid);
	astman_manager_action_params(s, "Originate", params);

	res = astman_wait_for_response(s, m, NULL, 0);
	if ( res > 0 && response_is(m, "Success")) {
		return res;
	}
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @fn astman_ping(struct mansession *s, struct message *m, char *actionid)
 * @brief Ping
 *      A 'Ping' action will ellicit a 'Pong' response.  Used to keep
 *      the manager connection open.
 * @param m:
 * @param actionid:
 * @param s:
 ******************************************************************************/
int astman_ping(struct mansession *s, struct message *m, char *actionid) 
{
	int res;
	char params[MAX_LEN] = "";
	char action_id[AMIMAN_ACTIONID_MAX];

	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
	}

	astman_manager_action_params(s, "Ping", params);
	
	res = astman_wait_for_response(s, m, actionid?actionid:action_id, 1);
	if ( res > 0 && have_header(m, "Ping", "Pong")) {
		return res;
	}else if( res == ASTMAN_SOCKCLOSED){
		return ASTMAN_SOCKCLOSED;
	}
	
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Command
 *        Execute CLI Command
 ******************************************************************************/
//int astman_command(struct mansession *s, struct message *m,
//		   char *command,
//		   char *actionid) {
int astman_command(struct mansession *s, struct message *m, char *command, char *actionid, int timeout) 
{
	int res;
	char params[MAX_LEN] = "";
	char action_id[AMIMAN_ACTIONID_MAX];

	if (astman_strlen_zero(command))
		return ASTMAN_FAILURE;

	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
		
	}

	astman_manager_action(s, "Command", "Command: %s\r\n%s", command, params);
	
	res = astman_wait_for_response(s, m, actionid?actionid:action_id, timeout>0?timeout:1);
	dlog(s->debug & DEBUG_LEVEL3, "res=[%d]\n", res);
	if ( res == ASTMAN_SUCCESS && response_is(m, "Follows")) {
		return res;
	}else if( res == ASTMAN_SOCKCLOSED ){
		return ASTMAN_SOCKCLOSED;
	}else if( res == ASTMAN_TIMEOUT ){
		return ASTMAN_TIMEOUT;
	}
	
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @fn
 * @brief QueueStatus
 * Queue, queue member and queued calls status
 ******************************************************************************/
int astman_queuestatus(struct mansession *s, struct message **m, char *actionid) 
{
	int res;
	char params[MAX_LEN] = "";
	char event[80];
	struct message msg;

	MSGBUF_INIT(1);

	astman_add_event_handler_system(s, astman_queues_callback);

	astman_add_param(params, sizeof(params), "ActionId", actionid);
	astman_manager_action_params(s, "QueueStatus", params);
	
	res = astman_wait_for_response(s, &msg, NULL, 0);
	if ( res > 0 && response_is(&msg, "Success")) {
		for(;;) {
			res = astman_wait_for_response(s, &msg, NULL, 0);
			if ( res > 0 ) {
				strncpy(event, astman_get_header(&msg, "Event"), sizeof(event));
				if (!strcasecmp(event, "QueueParams") ||
					!strcasecmp(event, "QueueMember") ||
					!strcasecmp(event, "QueueEnter")) {
					MSGBUF_ADD(&msg);
				} else if (!strcasecmp(event, "QueueStatusComplete")) {
					*m = MSGBUF_MSG;
					astman_add_event_handler_system(s, NULL);
					return ASTMAN_SUCCESS;
				}
			} else {
				goto out;
			}
		} /* infinity loop */
	}
out:
	astman_add_event_handler_system(s, NULL);
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @fn astman_absolutetimeout(struct mansession *s, struct message *m,
 *                          char *channel,
 *                          int timeout,
 *                          char *actionid)
 * @brief AbsoluteTimeout
 *        Hangup a channel after a certain time.
 * @param IN Channel: Channel name to hangup
 * @param IN Timeout: Maximum duration of the call (sec)
 ******************************************************************************/
int astman_absolutetimeout(struct mansession *s, struct message *m,
                           char *channel,
                           int timeout,
                           char *actionid) {
  int res;
  char params[MAX_LEN] = "";
  char strtimeout[12] = "";

  if (astman_strlen_zero(channel))
    return ASTMAN_FAILURE;

  if (timeout>0) {
    snprintf(strtimeout, sizeof(strtimeout), "%d", timeout);
  } else {
    return ASTMAN_FAILURE;
  }

  astman_add_param(params, sizeof(params), "Channel", channel);
  astman_add_param(params, sizeof(params), "Timeout", strtimeout);
  astman_add_param(params, sizeof(params), "ActionId", actionid);

  astman_manager_action_params(s, "AbsoluteTimeout", params);
  res = astman_wait_for_response(s, m, NULL, 0);
  if ( res > 0 && response_is(m, "Success")) {
    return res;
  }
  return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Status
 *        Lists channel status
 ******************************************************************************/
int astman_status(struct mansession *s, struct message **m, char *actionid) 
{
	int res;
	char params[MAX_LEN] = "";
	char event[80];
	struct message msg;

	MSGBUF_INIT(1);

	astman_add_event_handler_system(s, astman_status_callback);

	astman_add_param(params, sizeof(params), "ActionId", actionid);

	astman_manager_action_params(s, "Status", params);
	res = astman_wait_for_response(s, &msg, NULL, 0);

	if ( res > 0 && response_is(&msg, "Success")) {
		for(;;) {
			res = astman_wait_for_response(s, &msg, NULL, 0);
			if ( res > 0 ) {
				strncpy(event, astman_get_header(&msg, "Event"), sizeof(event));

				if (!strcasecmp(event, "Status")) {
					MSGBUF_ADD(&msg);
				} else if (!strcasecmp(event, "StatusComplete")) {
					*m = MSGBUF_MSG;
					astman_add_event_handler_system(s, NULL);
					return ASTMAN_SUCCESS;
				}
			} else {
				goto out;
			}
		} /* infinity loop */
	}

out:
	astman_add_event_handler_system(s, NULL);
	
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Events
 *        Enable/Disable sending of events to this manager
 * @param EventMask:
 *           system
 *           call
 *           log
 *           verbose
 *           command
 *           agent
 *           user
 *           all   if all events should be sent.
 *           none  if no events should be sent.
 ******************************************************************************/
int astman_events(struct mansession *s, struct message *m, char *eventmask, char *actionid, int timeout) 
{
	int res;
	char params[MAX_LEN] = "";
	char action_id[AMIMAN_ACTIONID_MAX];

	if (astman_strlen_zero(eventmask))
		return ASTMAN_FAILURE;
	
	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
	}

	astman_add_param(params, sizeof(params), "EventMask", eventmask);
	astman_add_param(params, sizeof(params), "ActionId", actionid);
	astman_manager_action_params(s, "Events", params);

	res = astman_wait_for_response(s, m, actionid?actionid:action_id, timeout>0?timeout:1);
	dlog(s->debug & DEBUG_LEVEL3, "res=[%d]\n", res);
	if ( res == ASTMAN_SUCCESS && response_is(m, "Success")) {
		return ASTMAN_SUCCESS;
	}else if( res == ASTMAN_SOCKCLOSED ){
		return ASTMAN_SOCKCLOSED;
	}else if( res == ASTMAN_TIMEOUT ){
		return ASTMAN_TIMEOUT;
	}else{
		return ASTMAN_FAILURE;
	}
}

/*******************************************************************************
 * @brief GetVar
 *        Get the value of a global or local channel variable.
 *
 *  @param Channel: Channel to read variable from
 *  @param Variable: Variable name
 ******************************************************************************/
int astman_getvar(struct mansession *s, struct message *m,
		  char *channel, char *variable,
		  char *actionid) 
{
	int res;
	char params[MAX_LEN] = "";

	if (astman_strlen_zero(variable))
		return ASTMAN_FAILURE;

	astman_add_param(params, sizeof(params), "Channel", channel);
	astman_add_param(params, sizeof(params), "Variable", variable);
	astman_add_param(params, sizeof(params), "ActionId", actionid);

	astman_manager_action_params(s, "GetVar", params);
	res = astman_wait_for_response(s, m, NULL, 0);
	if ( res > 0 && response_is(m, "Success")) {
		return res;
	}
	
	return ASTMAN_FAILURE;
}

/*******************************************************************************
 * @brief SetVar
 *        Set a global or local channel variable.
 *
 * @param Channel: Channel to set variable for
 * @param Variable: Variable name
 * @param Value: Value
 ******************************************************************************/
int astman_setvar(struct mansession *s, struct message *m,
                  char *channel, char *variable, char *value,
                  char *actionid) 
{
	int res;
	char params[MAX_LEN] = "";

	if (astman_strlen_zero(variable))
		return ASTMAN_FAILURE;

	astman_add_param(params, sizeof(params), "Channel", channel);
	astman_add_param(params, sizeof(params), "Variable", variable);
	astman_add_param(params, sizeof(params), "Value", value);
	astman_add_param(params, sizeof(params), "ActionId", actionid);

	astman_manager_action_params(s, "SetVar", params);
	res = astman_wait_for_response(s, m, NULL, 0);
	if ( res > 0 && response_is(m, "Success")) {
		return res;
	}
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief ListCommands
 *         List available manager commands
 ******************************************************************************/
int astman_list_commands(struct mansession *s, struct message *m, char *actionid)
{
	int res = 0;
	char params[MAX_LEN] = "";
	char action_id[AMIMAN_ACTIONID_MAX];

	astman_add_param(params, sizeof(params), "Parameters", "ActionId");
	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
	}

	astman_manager_action_params(s, "ListCommands", params);

	res = astman_wait_for_response(s, m, actionid?actionid:action_id, 1);
	if ( res > 0 && have_header(m, "Response", "Success")) {
		return res;
	}else if( res == ASTMAN_SOCKCLOSED){
		return ASTMAN_SOCKCLOSED;
	}

	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Action: ListCategories
 *        Synopsis: List categories in configuration file
 *        Privilege: config,all
 *        Description: A 'ListCategories' action will dump the categories in
 *                     a given file.
 * @param filename: Configuration filename (e.g. foo.conf)
 ******************************************************************************/
int astman_list_categories(struct mansession *s, struct message *m, char *filename, char *actionid)
{
	char params[MAX_LEN] = "";
	int res = 0;
	char action_id[AMIMAN_ACTIONID_MAX];

	astman_add_param(params, sizeof(params), "Filename", filename);
	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
	}

	astman_manager_action(s, "ListCategories", params);

	res = astman_wait_for_response(s, m, actionid?actionid:action_id, 1);
	if ( res > 0 && have_header(m, "Response", "Success")) {
		return res;
	}else if( res == ASTMAN_SOCKCLOSED){
		return ASTMAN_SOCKCLOSED;
	}
	
	return ASTMAN_FAILURE;
}
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
                      char *filename, char * category, char *actionid)
{
	int res = 0;
	char params[MAX_LEN] = "";
	char action_id[AMIMAN_ACTIONID_MAX];

	astman_add_param(params, sizeof(params), "Filename", filename);
	astman_add_param(params, sizeof(params), "Category", category);
	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
	}

	astman_manager_action_params(s, "GetConfig", params);

	res = astman_wait_for_response(s, m, actionid?actionid:action_id, 1);
	if ( res > 0 && have_header(m, "Response", "Success")) {
		return res;
	}else if( res == ASTMAN_SOCKCLOSED){
		return ASTMAN_SOCKCLOSED;
	}

	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Action: SIPpeers
 *        Synopsis: List SIP peers (text format)
 *        Privilege: system,reporting,all
 *        Description: Lists SIP peers in text format with details on current status.
 *        Peerlist will follow as separate events, followed by a final event called
 *        PeerlistComplete.
 *
 * @param ActionID: <id>	Action ID for this transaction. Will be returned.
 ******************************************************************************/
int astman_sip_peers(struct mansession *s, struct message **m, char *actionid)
{
	int res = 0;
	struct message msg;
	char params[MAX_LEN] = "";

	MSGBUF_INIT(1);

	astman_add_event_handler_system(s, &astman_sippeers_callback);

	astman_add_param(params, sizeof(params), "ActionId", actionid);

	astman_manager_action(s, "SIPpeers", params);

	res = astman_wait_for_response(s, &msg, NULL, 0);
	if ( res > 0 && response_is(&msg, "Success")) {
		if(strlen(astman_get_header(&msg, "Eventlist"))) {
			while(astman_wait_for_response(s, &msg, NULL, 0)==ASTMAN_SUCCESS) {
				if(strncasecmp(astman_get_header(&msg, ASTMAN_HEADER_EVENT), 
					ASTMAN_EVENT_PEER_LIST_COMPLETE,
					strlen(ASTMAN_EVENT_PEER_LIST_COMPLETE))){
					MSGBUF_ADD(&msg);
				} else {
					*m = MSGBUF_MSG;
					astman_add_event_handler_system(s, NULL);
					return MSGBUF_NB;
				}
			}
		}
	}
	astman_add_event_handler_system(s, NULL);
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Action: SIPshowpeer
 *         Synopsis: Show SIP peer (text format)
 *        Privilege: system,reporting,all
 *        Description: Show one SIP peer with details on current status.
 *
 *        Peer: <name>           The peer name you want to check.
 *        ActionID: <id>	  Optional action ID for this AMI transaction.
 *
 ******************************************************************************/
int astman_sip_show_peer(struct mansession *s, struct message *m, char *peer, char *actionid)
{
	int res = 0;
	char params[MAX_LEN] = "";
	char action_id[AMIMAN_ACTIONID_MAX];
	
	if(astman_strlen_zero(peer)) {
		return ASTMAN_FAILURE;
	}

	astman_add_param(params, sizeof(params), "Peer", peer);
	if(actionid){
		astman_add_param(params, sizeof(params), "ActionId", actionid);
	}else{
		memset(action_id, 0, sizeof(action_id));
		get_actionid(action_id, sizeof(action_id)-1);
		astman_add_param(params, sizeof(params), "ActionId", action_id);
	}

	astman_manager_action(s, "SIPShowpeer", params);
	res = astman_wait_for_response(s, m, actionid?actionid:action_id, 1);
	if ( res > 0 && have_header(m, "Response", "Success")) {
		return res;
	}else if( res == ASTMAN_SOCKCLOSED){
		return ASTMAN_SOCKCLOSED;
	}
	
	return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Action: SIPqualifypeer
 *        Synopsis: Show SIP peer (text format)
 *         Privilege: system,reporting,all
 *         Description: Show one SIP peer with details on current status.
 *
 * Peer: <name>           The peer name you want to check.
 * ActionID: <id>          Optional action ID for this AMI transaction.
 *
 ******************************************************************************/
int astman_sip_qualify_peer(struct mansession *s, struct message *m,
                         char *peer, char *actionid)
{
    int res = 0;
    char params[MAX_LEN] = "";
    if(astman_strlen_zero(peer)) {
        return ASTMAN_FAILURE;
    }

    astman_add_param(params, sizeof(params), "Peer", peer);
    astman_add_param(params, sizeof(params), "ActionId", actionid);

    astman_manager_action(s, "SIPqualifypeer", params);
    res = astman_wait_for_response(s, m, NULL, 1);

    if ( res > 0 && response_is(m, "Success")) {
            return ASTMAN_SUCCESS;
    }
    return ASTMAN_FAILURE;
}
/*******************************************************************************
 * @brief Action: SIPshowregistry
Synopsis: Show SIP registrations (text format)
Privilege: system,reporting,all
Description: Lists all registration requests and status
Registrations will follow as separate events. followed by a final event called
RegistrationsComplete.
Variables:
  ActionID: <id>       Action ID for this transaction. Will be returned.
 * @param ActionID: <id>	Action ID for this transaction. Will be returned.
 ******************************************************************************/
int astman_sip_show_registry(struct mansession *s, struct message **m,
                     char *actionid)
{
    int res = 0;
    struct message msg;
    char params[MAX_LEN] = "";

    MSGBUF_INIT(1);

    astman_add_event_handler_system(s, &astman_sipshowregistry_callback);

    astman_add_param(params, sizeof(params), "ActionId", actionid);

    astman_manager_action(s, "SIPshowregistry", params);

    res = astman_wait_for_response(s, &msg, NULL, 0);
    if ( res > 0 && response_is(&msg, "Success")) {
        if(strlen(astman_get_header(&msg, "Eventlist"))) {
            while(astman_wait_for_response(s, &msg, NULL, 0)==ASTMAN_SUCCESS) {
                if(strncasecmp(astman_get_header(&msg, ASTMAN_HEADER_EVENT),
                                ASTMAN_EVENT_REGISTRATIONS_COMPLETE,
                                strlen(ASTMAN_EVENT_REGISTRATIONS_COMPLETE)))
                {
                    MSGBUF_ADD(&msg);
                } else {
                    *m = MSGBUF_MSG;
                    astman_add_event_handler_system(s, NULL);
                    return MSGBUF_NB;
                }
            }
        }
    }
    astman_add_event_handler_system(s, NULL);
    return ASTMAN_FAILURE;
}
