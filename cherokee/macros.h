/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* Cherokee
 *
 * Authors:
 *      Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Copyright (C) 2001, 2002, 2003, 2004, 2005 Alvaro Lopez Ortega
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#if !defined (CHEROKEE_INSIDE_CHEROKEE_H) && !defined (CHEROKEE_COMPILATION)
# error "Only <cherokee/cherokee.h> can be included directly, this file may disappear or change contents."
#endif


#ifndef CHEROKEE_MACROS_H
#define CHEROKEE_MACROS_H

#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_SYS_VARARGS
# include <sys/varargs.h>
#endif

#ifdef  __cplusplus
#  define CHEROKEE_BEGIN_DECLS  extern "C" {
#  define CHEROKEE_END_DECLS    }
#else
#  define CHEROKEE_BEGIN_DECLS
#  define CHEROKEE_END_DECLS
#endif

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

#ifndef NULL
#  ifdef __cplusplus
#    define NULL        (0L)
#  else /* !__cplusplus */
#    define NULL        ((void*) 0)
#  endif /* !__cplusplus */
#endif

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define CHEROKEE_FUNCTION_NAME __func__
#elif defined(__GNUC__)
#define CHEROKEE_FUNCTION_NAME __FUNCTION__
#else
#define CHEROKEE_FUNCTION_NAME "unknown function"
#endif

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
# define likely(x)   __builtin_expect((x),1)
# define unlikely(x) __builtin_expect((x),0)
# define not_ok(x)   __builtin_expect(x != ret_ok, 0)
# define lt_ok(x)    __builtin_expect(x <  ret_ok, 0)
#else
# define likely(x)   (x) 
# define unlikely(x) (x)
# define not_ok(x)   (x != ret_ok)
# define lt_ok(x)    (x <  ret_ok)
#endif

#define DEFAULT_RECV_SIZE             1024
#define DEFAULT_READ_SIZE             8192
#define MAX_HEADER_LEN                4096
#define MAX_KEEPALIVE                 500
#define MAX_NEW_CONNECTIONS_PER_STEP  50
#define DEFAULT_CONN_REUSE            20
#define TERMINAL_WIDTH                80
#define DEFAULT_TRAFFIC_UPDATE        10
#define CGI_TIMEOUT                   65
#define MSECONS_TO_LINGER             2000

#define IOCACHE_MAX_FILE_SIZE            50000
#define IOCACHE_DEFAULT_CLEAN_ELAPSE     20
#define IOCACHE_BASIC_SIZE               50

#define EXIT_CANT_CREATE_SERVER_SOCKET4 10
#define EXIT_SERVER_CLEAN               30
#define EXIT_SERVER_READ_CONFIG         31
#define EXIT_SERVER_INIT                32

#define CRLF "\r\n"

#define return_if_fail(expr,ret) \
	if (!(expr)) {                                                      \
		fprintf (stderr,                                            \
       		         "file %s: line %d (%s): assertion `%s' failed\n",  \
                          __FILE__,                                         \
                          __LINE__,                                         \
                          CHEROKEE_FUNCTION_NAME,                           \
                          #expr);                                           \
	        return (ret);                                               \
	}


#define SHOULDNT_HAPPEN \
	do { fprintf (stderr, "file %s:%d (%s): this shouldn't happend\n",  \
		      __FILE__, __LINE__, CHEROKEE_FUNCTION_NAME);          \
	} while (0)

#define RET_UNKNOWN(ret) \
	do { fprintf (stderr, "file %s:%d (%s): ret code unknown ret=%d\n", \
		      __FILE__, __LINE__, CHEROKEE_FUNCTION_NAME, ret);     \
	} while (0)


#define CHEROKEE_NEW_STRUCT(obj,type) \
	cherokee_ ## type ## _t * obj = (cherokee_ ## type ## _t *) malloc (sizeof(cherokee_ ## type ## _t)); \
	return_if_fail (obj != NULL, ret_nomem)

#define CHEROKEE_NEW(obj,type)                   \
	cherokee_ ## type ## _t * obj;           \
	cherokee_ ## type ## _new (& obj );      \
	return_if_fail (obj != NULL, ret_nomem)

#define CHEROKEE_NEW2(obj1,obj2,type)             \
	cherokee_ ## type ## _t * obj1, *obj2;    \
	cherokee_ ## type ## _new (& obj1 );      \
	cherokee_ ## type ## _new (& obj2 );      \
	return_if_fail (obj1 != NULL, ret_nomem); \
	return_if_fail (obj2 != NULL, ret_nomem)

#define CHEROKEE_NEW_TYPE(obj,type) \
	type * obj = (type *) malloc(sizeof(type)); \
	return_if_fail (obj != NULL, ret_nomem)	

#define CHEROKEE_TEMP(obj, size)                 \
        const unsigned int obj ## _size = size;  \
	char obj[size]

#ifndef MIN
# define MIN(x,y) ((x<y) ? x : y)
#endif

#ifndef MAX
# define MAX(x,y) ((x>y) ? x : y)
#endif

/* Printing macros
 */
#ifdef __GNUC__
# define PRINT_ERROR(fmt,arg...) fprintf(stderr, "%s/%d: "fmt, __FILE__, __LINE__, ##arg)
# define PRINT_MSG(fmt,arg...)   fprintf(stderr, fmt, ##arg)
#else
# define PRINT_ERROR(fmt,...)    fprintf(stderr, "%s/%d: "fmt, __FILE__, __LINE__, __VA_ARGS__)
# define PRINT_MSG(fmt,...)      fprintf(stderr, fmt, __VA_ARGS__)
#endif

#ifdef DEBUG
# ifdef __GNUC__
#  define PRINT_DEBUG(fmt,arg...) do { fprintf(stdout, "%s/%d: " fmt,__FILE__,__LINE__,##arg); fflush(stdout); } while (0)
# else
#  define PRINT_DEBUG(fmt,...) do { fprintf(stdout, "%s/%d: " fmt,__FILE__,__LINE__,__VA_ARGS__); fflush(stdout); } while (0)
# endif
#else 
# ifdef __GNUC__
#  define PRINT_DEBUG(fmt,arg...) do { } while(0)
# else
#  define PRINT_DEBUG(fmt,...) do { } while(0)
# endif
#endif

#define PRINT_ERROR_S(str) PRINT_ERROR("%s",str)
#define PRINT_MSG_S(str)   PRINT_MSG("%s",str)

 
/* Conversions
 */
#define POINTER_TO_INT(pointer) ((long)(pointer))
#define INT_TO_POINTER(integer) ((void*)((long)(integer)))

/* IMPORTANT:
 * Cross compilers should define BYTE_ORDER in CFLAGS 
 */
#ifndef BYTE_ORDER

/* Definitions for byte order, according to byte significance from low
 * address to high.
 */
# define LITTLE_ENDIAN  1234    /* LSB first: i386, vax */
# define    BIG_ENDIAN  4321    /* MSB first: 68000, ibm, net */
# define    PDP_ENDIAN  3412    /* LSB first in word, MSW first in long */

/* assume autoconf's AC_C_BIGENDIAN has been ran. If it hasn't, we 
 * assume (maybe falsely) the order is LITTLE ENDIAN
 */
# ifdef WORDS_BIGENDIAN
#   define BYTE_ORDER  BIG_ENDIAN
# else
#   define BYTE_ORDER  LITTLE_ENDIAN
# endif

#endif /* BYTE_ORDER */


/* Format string for off_t
 */
#if _FILE_OFFSET_BITS == 64
# define FMT_OFFSET "%llu"
# define CST_OFFSET unsigned long long
#else
# define FMT_OFFSET "%lu"
# define CST_OFFSET unsigned long
#endif


#ifdef O_NOATIME
# define CHE_O_READ O_RDONLY | O_NOATIME
#else
# define CHE_O_READ O_RDONLY
#endif


#ifndef va_copy
# ifdef __va_copy
#  define va_copy(DEST,SRC) __va_copy((DEST),(SRC))
# else
#  ifdef HAVE_VA_LIST_AS_ARRAY
#   define va_copy(DEST,SRC) (*(DEST) = *(SRC))
#  else
#   define va_copy(DEST,SRC) ((DEST) = (SRC))
#  endif
# endif
#endif


#endif /* CHEROKEE_MACROS_H */
