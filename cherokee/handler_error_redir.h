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

#ifndef CHEROKEE_HANDLER_ERROR_REDIR_H
#define CHEROKEE_HANDLER_ERROR_REDIR_H

#include "common-internal.h"
#include "handler.h"
#include "connection.h"

typedef struct {
	   cherokee_handler_t handler;
	   cherokee_buffer_t *content;
} cherokee_handler_error_redir_t;

#define ERREDIR_HANDLER(x)  ((cherokee_handler_error_redir_t *)(x))


/* Library init function
 */
void  error_redir_init ();
ret_t cherokee_handler_error_redir_new (cherokee_handler_t **hdl, cherokee_connection_t *cnt, cherokee_table_t *properties);

#endif /* CHEROKEE_HANDLER_ERROR_REDIR_H */
