#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

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
 *  @file connection.h
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/

/*******************************************************************************
 *  \fn int astInit()
 *  \brief
 *  \return
 ******************************************************************************/
int astInit();
/*******************************************************************************
 *  \fn int astConnect(char * username, char *secret, char *host, int port)
 *  \brief
 *  \return
 ******************************************************************************/
int astConnect(char * username, char *secret, char *host, int port);
/*******************************************************************************
 *  \fn int astDisconnect()
 *  \brief
 *  \return
 ******************************************************************************/
int astDisconnect();
/*******************************************************************************
 *  \fn int astIsConnected()
 *  \brief
 *  \return
 ******************************************************************************/
int astIsConnected();
/*******************************************************************************
 *  \fn char* astConnectionGetUsername()
 *  \brief
 *  \return
 ******************************************************************************/
char* astConnectionGetUsername();
/*******************************************************************************
 *  \fn char* astConnectionGetUsername()
 *  \brief
 *  \return
 ******************************************************************************/
char* astConnectionGetPassword();
/*******************************************************************************
 *  \fn char* astConnectionGetUsername()
 *  \brief
 *  \return
 ******************************************************************************/
char* astConnectionGetHost();
/*******************************************************************************
 *  \fn char* astConnectionGetUsername()
 *  \brief
 *  \return
 ******************************************************************************/
int astConnectionGetPort();
/*******************************************************************************
 *  \fn char* astConnectionGetUsername()
 *  \brief
 *  \return
 ******************************************************************************/
void* astConnectionGetSession();

#endif // CONNECTION_H_INCLUDED
