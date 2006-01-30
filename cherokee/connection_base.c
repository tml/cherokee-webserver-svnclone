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

#include "common-internal.h"
#include "connection_base.h"

#include <unistd.h>


ret_t 
cherokee_connection_base_init (cherokee_connection_base_t *conn, cherokee_connectio_types_t type)
{
	INIT_LIST_HEAD(&conn->list_entry);

	conn->type             = type;
	conn->timeout          = -1;
	conn->extra_polling_fd = -1;

	return ret_ok;
}


ret_t 
cherokee_connection_base_clean (cherokee_connection_base_t *conn)
{
	if (conn->extra_polling_fd != -1) {
		close (conn->extra_polling_fd);
		conn->extra_polling_fd = -1;
	}

	conn->timeout = -1;

	return ret_ok;
}

