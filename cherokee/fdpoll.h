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

#ifndef CHEROKEE_FDPOLL_H
#define CHEROKEE_FDPOLL_H

#include <cherokee/common.h>


CHEROKEE_BEGIN_DECLS

typedef enum {
	cherokee_poll_epoll,
	cherokee_poll_kqueue,
	cherokee_poll_port,
	cherokee_poll_poll,
	cherokee_poll_select,
	cherokee_poll_NUM
} cherokee_poll_type_t;

#define FDPOLL(x) ((cherokee_fdpoll_t *)(x))

typedef struct cherokee_fdpoll cherokee_fdpoll_t;

ret_t cherokee_fdpoll_new        (cherokee_fdpoll_t **fdp, cherokee_poll_type_t type, int sys_limit, int limit);
ret_t cherokee_fdpoll_best_new   (cherokee_fdpoll_t **fdp, int sys_limit, int limit);
ret_t cherokee_fdpoll_free       (cherokee_fdpoll_t  *fdp);

ret_t cherokee_fdpoll_has_method     (cherokee_fdpoll_t *fdp, cherokee_poll_type_t  type);
ret_t cherokee_fdpoll_get_method     (cherokee_fdpoll_t *fdp, cherokee_poll_type_t *type);
ret_t cherokee_fdpoll_get_method_str (cherokee_fdpoll_t *fdp, char **name);

ret_t cherokee_fdpoll_add        (cherokee_fdpoll_t *fdp, int fd, int rw);
ret_t cherokee_fdpoll_del        (cherokee_fdpoll_t *fdp, int fd);
ret_t cherokee_fdpoll_reset      (cherokee_fdpoll_t *fdp, int fd);
void  cherokee_fdpoll_set_mode   (cherokee_fdpoll_t *fdp, int fd, int rw);
int   cherokee_fdpoll_check      (cherokee_fdpoll_t *fdp, int fd, int rw);
int   cherokee_fdpoll_watch      (cherokee_fdpoll_t *fdp, int timeout_msecs);
ret_t cherokee_fdpoll_is_full    (cherokee_fdpoll_t *fdp);

CHEROKEE_END_DECLS

#endif /* CHEROKEE_FDPOLL_H */
