#ifndef ASTLOG_H_INCLUDED
#define ASTLOG_H_INCLUDED

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
 *  @file astlog.h
 *  @brief
 *  @author Baligh.GUESMI
 *  @date 20100524
 ******************************************************************************/
 #define MODULE_NAME    "AST_API"
/*******************************************************************************
 *
 ******************************************************************************/
enum astlog_level {
    ASTLOG_INFO,
    ASTLOG_WARNING,
    ASTLOG_ERROR,
    ASTLOG_DEBUG,
};
#define AST_API_DEBUG
#ifdef AST_API_DEBUG
/*******************************************************************************
 *
 ******************************************************************************/
static const char * const astlog_map[] = {
    "INFO",
    "WARNING",
    "ERROR",
    "DEBUG",
};
/*******************************************************************************
 *
 ******************************************************************************/
#define astlog(log_level,format,...) \
    do { \
        fprintf(stdout, "[%s]-[%s]-[%s]:[%d] "format"\n", \
                        MODULE_NAME, \
                        astlog_map[log_level], \
                        __FUNCTION__, \
                        __LINE__, \
                        ##__VA_ARGS__); \
    }while(0)

#define astlog_init() \
    do { \
        fprintf(stdout, "[%s]-[%s]-[%s] Entring\n", \
                        MODULE_NAME, \
                        astlog_map[ASTLOG_DEBUG], \
                        __FUNCTION__); \
    } while(0)

#define astlog_end() \
    do { \
        fprintf(stdout, "[%s]-[%s]-[%s] End\n", \
                        MODULE_NAME, \
                        astlog_map[ASTLOG_DEBUG], \
                        __FUNCTION__); \
    } while(0)

#else
#define astlog(log_level,format,...)
#define astlog_init()
#define astlog_end()
#endif //AST_API_DEBUG
#endif // ASTLOG_H_INCLUDED
