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

#ifndef CHEROKEE_HANDLER_ERROR_H
#define CHEROKEE_HANDLER_ERROR_H

#include "common-internal.h"
#include "handler.h"
#include "connection.h"

typedef struct {
	cherokee_handler_t handler;
	cherokee_buffer_t *content;
} cherokee_handler_error_t;

#define ERR_HANDLER(x)  ((cherokee_handler_error_t *)(x))


/* Library init function
 */
void  error_init ();
ret_t cherokee_handler_error_new (cherokee_handler_t **hdl, cherokee_connection_t *cnt, cherokee_table_t *properties);

/* virtual methods implementation
 */
ret_t cherokee_handler_error_init        (cherokee_handler_error_t *hdl);
ret_t cherokee_handler_error_free        (cherokee_handler_error_t *hdl);
ret_t cherokee_handler_error_step        (cherokee_handler_error_t *hdl, cherokee_buffer_t *buffer);
ret_t cherokee_handler_error_add_headers (cherokee_handler_error_t *hdl, cherokee_buffer_t *buffer);

#endif /* CHEROKEE_HANDLER_ERROR_H */
