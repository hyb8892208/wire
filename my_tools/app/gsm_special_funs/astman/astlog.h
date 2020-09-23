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
                        __FILE__, \
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


/*********************************************/

#define DEBUG_LEVEL0 (1<<0)
#define DEBUG_LEVEL1 (1<<1)
#define DEBUG_LEVEL2 (1<<2)
#define DEBUG_LEVEL3 (1<<3)
#define DEBUG_LEVEL4 (1<<4)
#define DEBUG_LEVEL5 (1<<5)

#define dlog(enable, format,...) \
	do { \
		if(enable){ \
			time_t t = time(NULL); \
			char buf[256]; \
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t)); \
			fprintf(stdout, "[%s][ID:%6d][%6d %s %s] "format"", buf, (int)pthread_self(),\
				__LINE__, __FILE__, __FUNCTION__, \
				##__VA_ARGS__); \
		} \
	}while(0)	
/*********************************************/

#endif // ASTLOG_H_INCLUDED
