/**
 * @file debug.h
 *
 * @brief Macros para Debug
 *
 * Macros para DEBUG
 *
 * @Author Alvaro Parres (aparres@gmail.com)
 * @date Febrero 2011
 * @version 0.0.1
**/

#include <stdio.h>
#include <stdarg.h>

#ifndef _DEBUG_H
#define _DEBUG_H

extern int debugLevel;

#define debug_print(level, fmt, line,...) \
    fprintf(stderr, "[%i] %s:%d %s(): " fmt "\n",level, __FILE__,line,__FUNCTION__,##__VA_ARGS__)

#define debug(level, _fmt, ...) \
 		if(debugLevel >= level) debug_print(level,_fmt,__LINE__,##__VA_ARGS__)

#define error(errnum, str) \
		if(errnum > 0) { fprintf(stderr,"%s:%d ERROR: %s(%s)\n",__FILE__,__LINE__,str,strerror(errnum)); } \
		else { fprintf(stderr,"%s:%d ERROR: %s\n",__FILE__,__LINE__,str); }

#endif