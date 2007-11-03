/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* Cherokee
 *
 * Authors:
 *      Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Copyright (C) 2001-2007 Alvaro Lopez Ortega
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

#ifndef CHEROKEE_SERVER_PROTECTED_H
#define CHEROKEE_SERVER_PROTECTED_H

#include "common-internal.h"

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#else 
# include <time.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#include "server.h"
#include "list.h"
#include "fdpoll.h"
#include "dirs_table.h"
#include "virtual_server.h"
#include "encoder_table.h"
#include "thread.h"
#include "plugin_loader.h"
#include "icons.h"
#include "iocache.h"
#include "regex.h"
#include "nonce.h"
#include "mime.h"
#include "config_node.h"
#include "version.h"


struct cherokee_server {
	/* Current time
	 */
	time_t                       start_time;
	time_t                       bogo_now;
	struct tm                    bogo_now_tmloc;
	struct tm                    bogo_now_tmgmt;
	int                          bogo_now_tzloc_sign;
	cuint_t                      bogo_now_tzloc_offset;
	cherokee_buffer_t            bogo_now_strgmt;
	CHEROKEE_RWLOCK_T           (bogo_now_mutex);

	/* Exit related
	 */
	cherokee_buffer_t            panic_action;
	cherokee_boolean_t           wanna_exit;
	cherokee_boolean_t           wanna_reinit;
	cherokee_server_reinit_cb_t  reinit_callback;
	
	/* Virtual servers
	 */
	cherokee_list_t            vservers;
	cherokee_virtual_server_t *vserver_default;
	
	/* Threads
	 */
	cherokee_thread_t         *main_thread;
	cint_t                     thread_num;
	cherokee_list_t            thread_list;
	cint_t                     thread_policy;

	/* Modules
	 */
	cherokee_plugin_loader_t   loader;
	cherokee_encoder_table_t   encoders;

	/* Tables: icons, iocache
	 */
	cherokee_icons_t          *icons;
	cherokee_regex_table_t    *regexs;
	cherokee_nonce_table_t    *nonces;
	cherokee_iocache_t        *iocache;
	time_t                     iocache_clean_next;

	/* Logging
	 */
	int                        log_flush_elapse;
	time_t                     log_flush_next;

	/* Main socket
	 */
	cherokee_socket_t          socket;
	cherokee_socket_t          socket_tls;

	CHEROKEE_MUTEX_T          (accept_mutex);
#ifdef HAVE_TLS
	CHEROKEE_MUTEX_T          (accept_tls_mutex);
#endif

	/* System related
	 */
 	int                        ncpus;
	int                        max_fds;
	int                        fds_per_thread;
	uint32_t                   system_fd_limit;
	cherokee_poll_type_t       fdpoll_method;

	/* Connection related
	 */
	cint_t                     conns_max;
	cint_t                     conns_reuse_max;
	cint_t                     conns_keepalive_max;
	cuint_t                    conns_num_bogo;

	/* Networking config
	 */
	cherokee_boolean_t         ipv6;
	cherokee_buffer_t          listen_to;
	int                        fdwatch_msecs;
	int                        listen_queue;

	unsigned short             port;
	unsigned short             port_tls;
	cherokee_boolean_t         tls_enabled;
	cherokee_buffer_t          unix_socket;

	/* Server name
	 */
	cherokee_server_token_t    server_token;

	cherokee_buffer_t          server_string;
	cherokee_buffer_t          ext_server_string;
	cherokee_buffer_t          ext_server_w_port_string;
	cherokee_buffer_t          ext_server_w_port_tls_string;

	/* User/group and chroot
	 */
	uid_t                      user;
	uid_t                      user_orig;
	gid_t                      group;
	gid_t                      group_orig;

	cherokee_buffer_t          chroot;
	cherokee_boolean_t         chrooted;

	/* Mime
	 */
	cherokee_mime_t           *mime;

	/* Time
	 */
	int                        timeout;
	cherokee_buffer_t          timeout_header;

	cherokee_boolean_t         keepalive;
	uint32_t                   keepalive_max;

	struct {
		off_t min;
		off_t max;
	} sendfile;

	/* Another config files
	 */
	cherokee_config_node_t     config;
	char                      *icons_file;

	/* Performance
	 */

	/* PID
	 */
	cherokee_buffer_t          pidfile;
};


ret_t cherokee_server_del_connection (cherokee_server_t *srv, char *begin);
ret_t cherokee_server_get_vserver    (cherokee_server_t *srv, cherokee_buffer_t *name, cherokee_virtual_server_t **vsrv);

#endif /* CHEROKEE_SERVER_PROTECTED_H */
