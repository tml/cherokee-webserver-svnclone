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
#include "source.h"
#include "config_node.h"
#include "resolv_cache.h"
#include "util.h"

#define ENTRIES "source,src"


ret_t
cherokee_source_new (cherokee_source_t **src)
{
	CHEROKEE_NEW_STRUCT (n, source);
	
	*src = n;
	return cherokee_source_init (n);
}


ret_t 
cherokee_source_init (cherokee_source_t *src)
{
	cherokee_buffer_init (&src->original);
	cherokee_buffer_init (&src->unix_socket);
	cherokee_buffer_init (&src->host);

	src->port = -1;

	return ret_ok;
}


ret_t 
cherokee_source_mrproper (cherokee_source_t *src)
{
	cherokee_buffer_mrproper (&src->original);
	cherokee_buffer_mrproper (&src->unix_socket);
	cherokee_buffer_mrproper (&src->host);

	return ret_ok;
}


ret_t 
cherokee_source_connect (cherokee_source_t *src, cherokee_socket_t *sock)
{
	ret_t                    ret;
	cherokee_resolv_cache_t *resolv;

	ret = cherokee_resolv_cache_get_default (&resolv);
        if (unlikely (ret!=ret_ok)) return ret;

	/* UNIX socket
	 */
	if (! cherokee_buffer_is_empty (&src->unix_socket)) {
		ret = cherokee_socket_set_client (sock, AF_UNIX);
		if (ret != ret_ok) return ret;
		
		ret = cherokee_resolv_cache_get_host (resolv, src->unix_socket.buf, sock);
		if (ret != ret_ok) return ret;

		return cherokee_socket_connect (sock);
	}

	/* INET socket
	 */
	ret = cherokee_socket_set_client (sock, AF_INET);
	if (ret != ret_ok) return ret;
	
	ret = cherokee_resolv_cache_get_host (resolv, src->host.buf, sock);
	if (ret != ret_ok) return ret;
	
	SOCKET_ADDR_IPv4(sock)->sin_port = htons(src->port);
 	
	return cherokee_socket_connect (sock);
}


static ret_t
set_host (cherokee_source_t *src, cherokee_buffer_t *host)
{
	char *p;

	if (cherokee_buffer_is_empty (host))
		return ret_error;
	
	TRACE (ENTRIES, "Configuring source, setting host '%s'\n", host->buf);

	/* Original
	 */
	cherokee_buffer_add_buffer (&src->original, host);
	
	/* Unix socket
	 */
	if (host->buf[0] == '/') {
		cherokee_buffer_add_buffer (&src->unix_socket, host);
		return ret_ok;
	} 
	
	/* Host name
	 */
	p = strchr (host->buf, ':');
	if (p == NULL) {
		cherokee_buffer_add_buffer (&src->host, host);
		return ret_ok;
	} 
	
	/* Host name + port
	 */
	*p = '\0';
	src->port = atoi (p+1);
	cherokee_buffer_add (&src->host, host->buf, p - host->buf);
	*p = ':';
	
	return ret_ok;
}


ret_t 
cherokee_source_configure (cherokee_source_t *src, cherokee_config_node_t *conf)
{
	cherokee_list_t        *i;
	cherokee_config_node_t *child;

	cherokee_config_node_foreach (i, conf) {
		child = CONFIG_NODE(i);
		
		if (equal_buf_str (&child->key, "host")) {
			set_host (src, &child->val);
		}
		
		/* Base class: do not display error here
		 */
	}

	return ret_ok;
}
