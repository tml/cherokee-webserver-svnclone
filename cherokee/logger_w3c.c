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

#include "logger_w3c.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else 
#include <time.h>
#endif

#include "module.h"
#include "connection.h"
#include "server.h"
#include "server-protected.h"

#ifdef HAVE_PTHREAD
pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
#endif


/* Documentation:
 * - http://www.w3.org/TR/WD-logfile
 */

/* Some constants
 */
static char *method[]  = {"GET", "POST", "HEAD", "UNKNOWN", NULL};
static char *month[]   = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};

#define IN_ADDR(c) ((struct in_addr) (c).sin_addr)


cherokee_module_info_t cherokee_w3c_info = {
	cherokee_logger,            /* type     */
	cherokee_logger_w3c_new     /* new func */
};


ret_t
cherokee_logger_w3c_new  (cherokee_logger_t **logger, cherokee_table_t *properties)
{
	CHEROKEE_NEW_STRUCT (n, logger_w3c);
	
	/* Init the base class object
	 */
	cherokee_logger_init_base(LOGGER(n));

	MODULE(n)->init         = (logger_func_init_t) cherokee_logger_w3c_init;
	MODULE(n)->free         = (logger_func_free_t) cherokee_logger_w3c_free;
	LOGGER(n)->flush        = (logger_func_flush_t) cherokee_logger_w3c_flush;
	LOGGER(n)->write_error  = (logger_func_write_error_t) cherokee_logger_w3c_write_error;
	LOGGER(n)->write_access = (logger_func_write_access_t) cherokee_logger_w3c_write_access;
	LOGGER(n)->write_string = (logger_func_write_string_t) cherokee_logger_w3c_write_string;

	*logger = LOGGER(n);
	
	/* Init
	 */
	
	n->header_added = 0;
	n->filename     = NULL;
	n->file         = NULL;
	
	if (properties != NULL) {
		n->filename = cherokee_table_get_val (properties, "LogFile");
	}
	
	return ret_ok;
}


ret_t 
cherokee_logger_w3c_init (cherokee_logger_w3c_t *logger)
{
	/* Syslog
	 */
	if (logger->filename == NULL) {
		openlog ("Cherokee", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
		return ret_ok;
	}


	/* Direct file writting
	 */
	logger->file = fopen (logger->filename, "a+");
	if (logger->file == NULL) {
		PRINT_ERROR("cherokee_logger_w3c: error opening %s for append\n", logger->filename); 
		return ret_error;
	}
	fcntl (fileno (logger->file), F_SETFD, 1);

	return ret_ok;
}


ret_t
cherokee_logger_w3c_free (cherokee_logger_w3c_t *logger)
{
	ret_t ret = ret_ok;

	if (logger->file != NULL) {
		if (fclose(logger->file) != 0) {
			ret = ret_error;
		}
	} else {
		closelog();		
	}
	
	free (logger);
	
	return ret;
}


ret_t
cherokee_logger_w3c_flush (cherokee_logger_w3c_t *logger)
{
	CHEROKEE_MUTEX_LOCK(&buffer_lock);

	if (cherokee_buffer_is_empty(LOGGER_BUFFER(logger))) {
		return ret_ok;
	}
	
	if (logger->file != NULL) {
		size_t wrote;
		
		wrote = fwrite (LOGGER_BUFFER(logger)->buf, 1, LOGGER_BUFFER(logger)->len, logger->file);
		fflush(logger->file);

		return (wrote > 0) ? ret_ok : ret_error;
	}

	syslog (LOG_ERR, "%s", LOGGER_BUFFER(logger)->buf);

	CHEROKEE_MUTEX_UNLOCK(&buffer_lock);

	return ret_ok;
}


ret_t 
cherokee_logger_w3c_write_error  (cherokee_logger_w3c_t *logger, cherokee_connection_t *cnt)
{
	long int z;
	int  len;
	struct tm conn_time;
	CHEROKEE_TEMP (tmp, 200);

	localtime_r (&CONN_THREAD(cnt)->bogo_now, &conn_time);

#ifdef HAVE_INT_TIMEZONE
	z = - (timezone / 60);
#else
#warning TODO
	z = 0;
#endif

	len = snprintf (tmp, tmp_size-1,
			"%02d:%02d:%02d [error] %s %s\n",
			conn_time.tm_hour, 
			conn_time.tm_min, 
			conn_time.tm_sec,
			method[cnt->header->method],
			cnt->request->buf);

	if ((len > tmp_size-1) || (len == -1)) {
		len = tmp_size;
		tmp[tmp_size-1] = '\n';
	}


	CHEROKEE_MUTEX_LOCK(&buffer_lock);
	cherokee_buffer_add (LOGGER_BUFFER(logger), tmp, len);
	CHEROKEE_MUTEX_UNLOCK(&buffer_lock);

	return ret_ok;
}


ret_t 
cherokee_logger_w3c_write_string (cherokee_logger_w3c_t *logger, const char *string)
{
	   syslog (LOG_INFO, "%s", string);
	   return ret_ok;
}


ret_t
cherokee_logger_w3c_write_access (cherokee_logger_w3c_t *logger, cherokee_connection_t *cnt)
{
	long int  z;
	int       len;
	struct tm conn_time;
	CHEROKEE_TEMP (tmp, 200);

	localtime_r (&CONN_THREAD(cnt)->bogo_now, &conn_time);

	if ((logger->header_added == 0) && (logger->file)) {
		len = snprintf (tmp, tmp_size-1, 
				"#Version 1.0\n"
				"#Date: %d02-%s-%4d %02d:%02d:%02d\n"
				"#Fields: time cs-method cs-uri\n",
				conn_time.tm_mday, 
				month[conn_time.tm_mon], 
				1900 + conn_time.tm_year,
				conn_time.tm_hour, 
				conn_time.tm_min, 
				conn_time.tm_sec);
		
		CHEROKEE_MUTEX_LOCK(&buffer_lock);
		cherokee_buffer_add (LOGGER_BUFFER(logger), tmp, len);
		CHEROKEE_MUTEX_UNLOCK(&buffer_lock);

		logger->header_added = 1;
	}
	
#ifdef HAVE_INT_TIMEZONE
	   z = - (timezone / 60);
#else
#warning TODO
	   z = 0;
#endif

	   len = snprintf (tmp, tmp_size-1,
			   "%02d:%02d:%02d %s %s\n",
			   conn_time.tm_hour, 
			   conn_time.tm_min, 
			   conn_time.tm_sec,
			   method[cnt->header->method],
			   cnt->request->buf);

	   
	   if ((len > tmp_size-1) || (len == -1)) {
		   len = tmp_size;
		   tmp[tmp_size-1] = '\n';
	   }


	   CHEROKEE_MUTEX_LOCK(&buffer_lock);
	   cherokee_buffer_add (LOGGER_BUFFER(logger), tmp, len);
	   CHEROKEE_MUTEX_UNLOCK(&buffer_lock);

	   return(ret_ok);
}




/*   Library init function
 */

static int _w3c_is_init = 0;

void
w3c_init ()
{
	/* Init flag
	 */
	if (_w3c_is_init) {
		return;
	}
	_w3c_is_init = 1;
}


