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
 *  @file update.c
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
#include "astlog.h"
#include "action.h"

#define NB_PARAMS_IN_ACTION 6
#define MAX_ACTIONS         20
#define MAX_PARAMS_LEN          (((MAX_ACTIONS)*(MAX_LEN)))
#define MAX_PARAM_NAME_LEN  14

static int _nb_action = -1;
static char params[MAX_PARAMS_LEN];
/**
 *
 * @param src_filename
 * @param dst_filename
 * @param reload
 * @return
 */
int astman_update_config_init(char *src_filename,
                              char *dst_filename,
                              int reload)
{
    astman_add_param(params, sizeof(params), "SrcFilename", src_filename);
    astman_add_param(params, sizeof(params), "DstFilename", dst_filename);
    astman_add_param(params, sizeof(params), "Reload", reload?"yes":"no");
    return ASTMAN_SUCCESS;
}
/**
 *
 * @param action
 * @param cat
 * @param var
 * @param val
 * @param match
 * @param line
 * @return
 */
int astman_update_config_add_action(char *action,
                                    char *cat,
                                    char *var, char *val,
                                    char *match,
                                    char *line)
{
    char a_tmp[MAX_PARAM_NAME_LEN];
    if(astman_strlen_zero(action) ||
        astman_strlen_zero(cat)) {
            return ASTMAN_FAILURE;
    }

    _nb_action++;

    snprintf(a_tmp, MAX_PARAM_NAME_LEN, "Action-%06d", _nb_action);
    astman_add_param(params, sizeof(params), a_tmp, action);

    snprintf(a_tmp, MAX_PARAM_NAME_LEN, "Cat-%06d", _nb_action);
    astman_add_param(params, sizeof(params), a_tmp, cat);

    snprintf(a_tmp, MAX_PARAM_NAME_LEN, "Var-%06d", _nb_action);
    astman_add_param(params, sizeof(params), a_tmp, var);

    snprintf(a_tmp, MAX_PARAM_NAME_LEN, "Value-%06d", _nb_action);
    astman_add_param(params, sizeof(params), a_tmp, val);

    if(!astman_strlen_zero(match)) {
        snprintf(a_tmp, MAX_PARAM_NAME_LEN, "Match-%06d", _nb_action);
        astman_add_param(params, sizeof(params), a_tmp, match);
    }

    if(!astman_strlen_zero(line)) {
        snprintf(a_tmp, MAX_PARAM_NAME_LEN, "Line-%06d", _nb_action);
        astman_add_param(params, sizeof(params), a_tmp, line);
    }
    return ASTMAN_SUCCESS;
}
/**
 *
 * @param s
 * @param m
 * @return
 */
int astman_update_config_execute(struct mansession *s, struct message *m)
{
    int res = ASTMAN_FAILURE;
    astlog_init();
    if(_nb_action == -1) {
        res = ASTMAN_FAILURE;
        goto Exit;
    }

    astman_manager_action_params(s, "UpdateConfig", params);

    res = astman_wait_for_response(s, m, NULL, 0);
    if ( res > 0 && response_is(m, "Success")) {
        res = ASTMAN_SUCCESS;
    }
    astlog(ASTLOG_INFO, "UpdateConfig %s", astman_get_header(m, "Response"));
    _nb_action = -1;
    memset(params, 0, MAX_PARAMS_LEN);
Exit:
    astlog_end();
    return res;
}
