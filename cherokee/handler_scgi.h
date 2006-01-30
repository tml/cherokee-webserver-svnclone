/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* Cherokee
 *
 * Authors:
 *      Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Copyright (C) 2001-2006 Alvaro Lopez Ortega
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

#ifndef CHEROKEE_HANDLER_SCGI_H
#define CHEROKEE_HANDLER_SCGI_H

#include "common-internal.h"

#include "handler.h"
#include "buffer.h"
#include "module_loader.h"
#include "socket.h"
#include "handler_cgi_base.h"


typedef struct {
	cherokee_handler_cgi_base_t base;

	cherokee_buffer_t   header;
	cherokee_socket_t  *socket;

	list_t             *scgi_env_ref;
	list_t             *server_list;	
} cherokee_handler_scgi_t;

#define HANDLER_SCGI(x)  ((cherokee_handler_scgi_t *)(x))

 
/* Library init function
 */
void  scgi_init (cherokee_module_loader_t *loader);

/* Methods
 */
ret_t cherokee_handler_scgi_new         (cherokee_handler_t     **hdl, void *cnt, cherokee_table_t *properties);
ret_t cherokee_handler_scgi_free        (cherokee_handler_scgi_t *hdl);
ret_t cherokee_handler_scgi_init        (cherokee_handler_scgi_t *hdl);
ret_t cherokee_handler_scgi_add_headers (cherokee_handler_scgi_t *hdl, cherokee_buffer_t *buffer);
ret_t cherokee_handler_scgi_step        (cherokee_handler_scgi_t *hdl, cherokee_buffer_t *buffer);

#endif /* CHEROKEE_HANDLER_CGI_H */
