#ifndef ASTAPI_H_INCLUDED
#define ASTAPI_H_INCLUDED

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
 *  @file astapi.h
 *  @brief
 *  @author Baligh.GUESMI
 *  @date
 ******************************************************************************/
/*******************************************************************************
 *  @def    MAX_LEN
 *  @brief  One Header Max Len supported
 ******************************************************************************/
#define MAX_NAME_LEN    64
#define MAX_VALUE_LEN   256
#define MAX_LEN         ((MAX_NAME_LEN)+(MAX_VALUE_LEN))
/*******************************************************************************
 *  @def    ASTMAN_SUCCESS
 *  @brief
 ******************************************************************************/
#define ASTMAN_SUCCESS 1
/*******************************************************************************
 *  @def    ASTMAN_FAILURE
 *  @brief
 ******************************************************************************/
#define ASTMAN_FAILURE 0
/*******************************************************************************
 *  @def    MAX_HEADERS
 *  @brief  MAX Header supported in one message command
 ******************************************************************************/
#define MAX_HEADERS 128
/*******************************************************************************
 *  @def    MAX_EVENTS
 *  @brief
 ******************************************************************************/
#define MAX_EVENTS 16

#endif // ASTAPI_H_INCLUDED

