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

#include "validator_pam.h"
#include <security/pam_appl.h>

cherokee_module_info_t cherokee_pam_info = {
	cherokee_validator,           /* type     */
	cherokee_validator_pam_new    /* new func */
};

/* PAM service name
 */
#define CHEROKEE_AUTH_SERVICE "cherokee"


ret_t 
cherokee_validator_pam_new (cherokee_validator_pam_t **pam, cherokee_table_t *properties)
{
	CHEROKEE_NEW_STRUCT(n,validator_pam);

	/* Init 
	 */
	cherokee_validator_init_base (VALIDATOR(n));

	MODULE(n)->free     = (module_func_free_t)     cherokee_validator_pam_free;
	VALIDATOR(n)->check = (validator_func_check_t) cherokee_validator_pam_check;

	*pam = n;
	return ret_ok;
}


ret_t 
cherokee_validator_pam_free (cherokee_validator_pam_t *pam)
{
	free (pam);
	return ret_ok;
}



/*
 * auth_pam_talker: supply authentication information to PAM when asked
 *
 * Assumptions:
 *   A password is asked for by requesting input without echoing
 *   A username is asked for by requesting input _with_ echoing
 *
 */
static int 
auth_pam_talker (int                        num_msg,
		 const struct pam_message **msg,
		 struct pam_response      **resp,
		 void                      *appdata_ptr)
{
	unsigned short i = 0;
	cherokee_connection_t *conn = CONN(appdata_ptr);
	struct pam_response *response = 0;

	/* parameter sanity checking 
	 */
	if (!resp || !msg || !conn) {
		return PAM_CONV_ERR;
	}

	/* allocate memory to store response 
	 */
	response = malloc (num_msg * sizeof(struct pam_response));
	if (!response) {
		return PAM_CONV_ERR;
	}

	/* copy values 
	 */
	for (i = 0; i < num_msg; i++) {
		/* initialize to safe values 
		 */
		response[i].resp_retcode = 0;
		response[i].resp = 0;

		/* select response based on requested output style 
		 */
		switch (msg[i]->msg_style) {
		case PAM_PROMPT_ECHO_ON:
			response[i].resp = strdup(conn->user->buf);
			break;
		case PAM_PROMPT_ECHO_OFF:
			response[i].resp = strdup(conn->passwd->buf);
			break;
		default:
			if (response)
				free(response);
			return PAM_CONV_ERR;
		}
	}

	/* everything okay, set PAM response values 
	 */
	*resp = response;
	return PAM_SUCCESS;
}


ret_t 
cherokee_validator_pam_check (cherokee_validator_pam_t  *pam, cherokee_connection_t *conn)
{
	int   ret;
	char *app_data[2];
	static pam_handle_t *pamhandle = NULL;
	struct pam_conv      pamconv   = {&auth_pam_talker, conn};

	extern int _pam_dispatch (pam_handle_t *, int, int);

	app_data[0] = NULL;
	app_data[1] = strdup(conn->passwd->buf);


	ret = pam_start (CHEROKEE_AUTH_SERVICE, conn->user->buf, &pamconv, &pamhandle);
	if (ret != PAM_SUCCESS) {
		conn->error_code = http_internal_error;
		return ret_error;
	}

	/* NOTE:
	 * First of all, it's a really *horrible* hack.
	 * The right way to authenticate a user is call:
	 *
	 * 	ret = pam_authenticate (pamhandle, 0);
	 *
	 * Instead it, the validator calls to:
	 *
	 *	ret = _pam_dispatch (pamhandle, 0, 1);
	 * 
	 * It's because pam_uthenticate do long delay if the user is not
	 * authenticated.  It's a problem if Cherokee is compiled with out
	 * threading support, because the server will be frozen for some
	 * second until pam_authenticate finish the delay.
	 *
	 * The second parameter: 0, is the flags
	 * The last one: 1, is PAM_AUTHENTICATE
	 */

	/* Try to authenticate user:
	 */
	ret = _pam_dispatch (pamhandle, 0, 1);
	if (ret != PAM_SUCCESS) {
		CHEROKEE_NEW(msg, buffer);

		cherokee_buffer_add (msg, "PAM: user '", 11);
		cherokee_buffer_add_buffer (msg, conn->user);
		cherokee_buffer_add_va (msg, "' - not authenticated: %s", pam_strerror(pamhandle, ret));

		cherokee_logger_write_string (CONN_VSRV(conn)->logger, "%s", msg->buf);
		
		cherokee_buffer_free (msg);
		goto unauthorized;
	}

	/* Check that the account is healthy 
	 */
	ret = pam_acct_mgmt (pamhandle, PAM_DISALLOW_NULL_AUTHTOK); 
	if (ret != PAM_SUCCESS) {
		CHEROKEE_NEW(msg, buffer);

		cherokee_buffer_add (msg, "PAM: user '", 11);
		cherokee_buffer_add_buffer (msg, conn->user);
		cherokee_buffer_add_va (msg, "'  - invalid account: %s", pam_strerror(pamhandle, ret));

		cherokee_logger_write_string (CONN_VSRV(conn)->logger, "%s", msg->buf);

		cherokee_buffer_free (msg);
		goto unauthorized;
	}

	pam_end (pamhandle, PAM_SUCCESS);
	return ret_ok;

unauthorized:
	pam_end (pamhandle, PAM_SUCCESS);
	return ret_error;
}

