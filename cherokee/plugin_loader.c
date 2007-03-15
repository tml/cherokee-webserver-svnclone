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

#include "common-internal.h"
#include "plugin_loader.h"

#include <stdlib.h>
#include <string.h>

#ifndef CHEROKEE_EMBEDDED
# ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
# endif
#endif

#include "table.h"
#include "buffer.h"

typedef cherokee_plugin_loader_entry_t entry_t;


#ifdef CHEROKEE_EMBEDDED

ret_t
cherokee_plugin_loader_init  (cherokee_plugin_loader_t *loader)
{
	return ret_ok;
}

ret_t 
cherokee_plugin_loader_mrproper (cherokee_plugin_loader_t *loader)
{  
	return ret_ok; 
}

ret_t 
cherokee_plugin_loader_load (cherokee_plugin_loader_t *loader, char *modname)
{
	extern void MODULE_INIT(common)   (cherokee_plugin_loader_t *);
	extern void MODULE_INIT(file)     (cherokee_plugin_loader_t *);
	extern void MODULE_INIT(redir)    (cherokee_plugin_loader_t *);
	extern void MODULE_INIT(dirlist)  (cherokee_plugin_loader_t *);
	extern void MODULE_INIT(cgi)      (cherokee_plugin_loader_t *);
	extern void MODULE_INIT(phpcgi)   (cherokee_plugin_loader_t *);
	extern void MODULE_INIT(htdigest) (cherokee_plugin_loader_t *);

	if      (strcmp(modname, "common")   == 0) MODULE_INIT(common) (NULL);
	else if (strcmp(modname, "file")     == 0) MODULE_INIT(file) (NULL);
	else if (strcmp(modname, "redir")    == 0) MODULE_INIT(redir) (NULL);
	else if (strcmp(modname, "dirlist")  == 0) MODULE_INIT(dirlist) (NULL);
	else if (strcmp(modname, "cgi")      == 0) MODULE_INIT(cgi) (NULL);
	else if (strcmp(modname, "phpcgi")   == 0) MODULE_INIT(phpcgi) (NULL);
	else if (strcmp(modname, "htdigest") == 0) MODULE_INIT(htdigest) (NULL);
	else return ret_error;

	return ret_ok;
}

ret_t 
cherokee_plugin_loader_unload (cherokee_plugin_loader_t *loader, char *modname)
{
	return ret_ok;
}

ret_t 
cherokee_plugin_loader_get_info (cherokee_plugin_loader_t *loader, char *modname, cherokee_plugin_info_t **info)
{
	extern cherokee_plugin_info_t cherokee_common_info;
	extern cherokee_plugin_info_t cherokee_file_info;
	extern cherokee_plugin_info_t cherokee_redir_info;
	extern cherokee_plugin_info_t cherokee_dirlist_info;
	extern cherokee_plugin_info_t cherokee_cgi_info;
	extern cherokee_plugin_info_t cherokee_phpcgi_info;
	extern cherokee_plugin_info_t cherokee_htdigest_info;

	if      (strcmp(modname, "common")   == 0) *info = &cherokee_common_info;
	else if (strcmp(modname, "file")     == 0) *info = &cherokee_file_info;
	else if (strcmp(modname, "redir")    == 0) *info = &cherokee_redir_info;
	else if (strcmp(modname, "dirlist")  == 0) *info = &cherokee_dirlist_info;
	else if (strcmp(modname, "cgi")      == 0) *info = &cherokee_cgi_info;
	else if (strcmp(modname, "phpcgi")   == 0) *info = &cherokee_phpcgi_info;
	else if (strcmp(modname, "htdigest") == 0) *info = &cherokee_htdigest_info;
	else return ret_error;

	return ret_ok;
}


#else 


/* This is the non-embedded implementation
 */

#include "loader.autoconf.h"

#ifdef HAVE_RTLDNOW	
# define RTLD_BASE RTLD_NOW
#else
# define RTLD_BASE RTLD_LAZY
#endif


typedef void *func_new_t;


static void
add_static_entry (cherokee_plugin_loader_t *loader, const char *name, void *info)
{
	entry_t *entry;

	entry = malloc (sizeof(entry_t));
	entry->dlopen_ref = dlopen (NULL, RTLD_BASE);
	entry->info       = info; 

	cherokee_table_add (&loader->table, (char *)name, entry);
}


static ret_t
load_static_linked_modules (cherokee_plugin_loader_t *loader)
{
/* I do know that to include code in the middle of a function is hacky
 * and dirty, but this is the best solution that I could figure out.
 * If you have some idea to clean it up, please, don't hesitate and
 * let me know. - alo
 */

#include "loader.autoconf.inc"

	return ret_ok;
}

static void
free_entry (void *item)
{
	entry_t *entry = (entry_t *)item;

	if (entry->dlopen_ref) {
		dlclose (entry->dlopen_ref);
		entry->dlopen_ref = NULL;
	}

	free (item);
}


ret_t
cherokee_plugin_loader_init (cherokee_plugin_loader_t *loader)
{
	ret_t ret;
	
	ret = cherokee_table_init (&loader->table);
	if (unlikely(ret < ret_ok)) return ret;
	
	ret = cherokee_buffer_init (&loader->module_dir);
	if (unlikely(ret < ret_ok)) return ret;

	cherokee_buffer_add_str (&loader->module_dir, CHEROKEE_PLUGINDIR);

	ret = load_static_linked_modules (loader);
	if (unlikely(ret < ret_ok)) return ret;

	return ret_ok;
}


ret_t 
cherokee_plugin_loader_mrproper (cherokee_plugin_loader_t *loader)
{
	cherokee_buffer_mrproper (&loader->module_dir);
	cherokee_table_mrproper2 (&loader->table, (cherokee_table_free_item_t)free_entry);
	return ret_ok;
}


static void *
get_sym_from_dlopen_handler (void *dl_handle, const char *sym)
{
	void *re;
	char *error;
	   
	/* Clear the possible error and look for the symbol
	 */
	dlerror();
	re = (void *) dlsym(dl_handle, sym);
	if ((error = dlerror()) != NULL)  {
		PRINT_MSG ("ERROR: %s\n", error);
		return NULL;
	}
	
	return re;
}


static ret_t
dylib_open (cherokee_plugin_loader_t  *loader, 
	    const char                *libname, 
	    int                        extra_flags,
	    void                     **handler_out) 
{
	ret_t             ret;
	void             *lib;
	int               flags;
	cherokee_buffer_t tmp = CHEROKEE_BUF_INIT; 
	
	flags = RTLD_BASE | extra_flags;

	/* Build the path string
	 */
	ret = cherokee_buffer_add_va (&tmp, "%s/libplugin_%s." MOD_SUFFIX, loader->module_dir.buf, libname);
	if (unlikely(ret < ret_ok)) return ret;
	
	/* Open the library	
	 */
	lib = dlopen (tmp.buf, flags);
	if (lib == NULL) {
		PRINT_ERROR ("ERROR: dlopen(%s): %s\n", tmp.buf, dlerror());
		goto error;
	}

	/* Free the memory
	 */
	cherokee_buffer_mrproper (&tmp);

	*handler_out = lib;
	return ret_ok;

error:
	cherokee_buffer_mrproper (&tmp);
	return ret_error;
}


static ret_t
execute_init_func (cherokee_plugin_loader_t *loader, const char *module, entry_t *entry)
{
	ret_t ret;
	void (*init_func) (cherokee_plugin_loader_t *);
	cherokee_buffer_t init_name = CHEROKEE_BUF_INIT;

	/* Build the init function name
	 */
	ret = cherokee_buffer_add_va (&init_name, "cherokee_plugin_%s_init", module);
	if (unlikely(ret < ret_ok)) return ret;

	/* Get the function
	 */
	if (entry->dlopen_ref == NULL) 
		return ret_error;

	init_func = get_sym_from_dlopen_handler (entry->dlopen_ref, init_name.buf);
		
	/* Only try to execute if it exists
	 */
	if (init_func == NULL) {
		PRINT_ERROR ("WARNING: %s doesn't found\n", init_name.buf);

		cherokee_buffer_mrproper (&init_name);
		return ret_not_found;
	}

	/* Free the init function name string
	 */
	cherokee_buffer_mrproper (&init_name);

	/* Execute the init func
	 */
	init_func(loader);
	return ret_ok;
}


static ret_t
get_info (cherokee_plugin_loader_t  *loader, 
	  const char                *module, 
	  int                        flags, 
	  cherokee_plugin_info_t   **info, 
	  void                     **dl_handler)
{
	ret_t             ret;
	cherokee_buffer_t info_name = CHEROKEE_BUF_INIT;

	/* Build the info struct string
	 */
	cherokee_buffer_add_va (&info_name, "cherokee_%s_info", module);

	/* Open it
	 */
	ret = dylib_open (loader, module, flags, dl_handler);
	if (ret != ret_ok) {
		ret = ret_error;
		goto error;
	}
	
	*info = get_sym_from_dlopen_handler (*dl_handler, info_name.buf);
	if (*info == NULL) {
		ret = ret_not_found;
		goto error;
	}

	/* Free the info struct string
	 */
	cherokee_buffer_mrproper (&info_name);
	return ret_ok;

error:
	cherokee_buffer_mrproper (&info_name);
	return ret;	
}


ret_t
check_deps_file (cherokee_plugin_loader_t *loader, char *modname)
{
	FILE             *file;
	char              temp[128];
	cherokee_buffer_t filename = CHEROKEE_BUF_INIT;

	cherokee_buffer_add_va (&filename, "%s/%s.deps", CHEROKEE_DEPSDIR, modname);
	file = fopen (filename.buf, "r");
	if (file == NULL) goto exit;

	while (!feof(file)) {
		int   len;
		char *ret;

		ret = fgets (temp, 127, file);
		if (ret == NULL) break;

		len = strlen (temp); 

		if (len < 2) continue;
		if (temp[0] == '#') continue;
		
		if (temp[len-1] == '\n')
			temp[len-1] = '\0';

		cherokee_plugin_loader_load (loader, temp);
		temp[0] = '\0';
	}

	fclose (file);

exit:
	cherokee_buffer_mrproper (&filename);
	return ret_ok;
}


static ret_t
load_common (cherokee_plugin_loader_t *loader, char *modname, int flags)
{
	ret_t                   ret;
	entry_t                *entry     = NULL;
	cherokee_plugin_info_t *info      = NULL;
	void                   *dl_handle = NULL;

	/* If it is already loaded just return 
	 */
	ret = cherokee_table_get (&loader->table, modname, (void **)&entry);
	if (ret == ret_ok) return ret_ok;
	
	/* Check deps
	 */
	ret = check_deps_file (loader, modname);
	if (ret != ret_ok) return ret;

	/* Get the module info
	 */
	ret = get_info (loader, modname, flags, &info, &dl_handle);
	switch (ret) {
	case ret_ok:
		break;
	case ret_error:
		PRINT_ERROR ("ERROR: Can't open \"%s\" module\n", modname);		
		return ret;
	case ret_not_found:
		PRINT_ERROR ("ERROR: Can't read \"info\" structure from %s\n", modname);
		return ret;
	default:
		SHOULDNT_HAPPEN;
		return ret_error;
	}
	
	/* Add new entry
	 */
	entry = malloc (sizeof(entry_t));
	entry->dlopen_ref = dl_handle;
	entry->info       = info; 
	
	ret = cherokee_table_add (&loader->table, modname, entry);
	if (unlikely(ret != ret_ok)) return ret;

	/* Execute init function
	 */
	ret = execute_init_func (loader, modname, entry);
	if (ret != ret_ok) return ret;

	return ret_ok;
}


ret_t 
cherokee_plugin_loader_load_no_global (cherokee_plugin_loader_t *loader, char *modname)
{
	return load_common (loader, modname, 0);
}


ret_t 
cherokee_plugin_loader_load (cherokee_plugin_loader_t *loader, char *modname)
{
#ifdef HAVE_RTLDGLOBAL
	return load_common (loader, modname, RTLD_GLOBAL);
#else
	return load_common (loader, modname, 0);
#endif
}


ret_t 
cherokee_plugin_loader_unload (cherokee_plugin_loader_t *loader, char *modname)
{
	int      re     = 0;
	ret_t    ret;
	entry_t *entry;

	/* Remove item from the table
	 */
	ret = cherokee_table_del (&loader->table, modname, (void **)&entry);
	if (ret != ret_ok) return ret;

	/* Free the resources
	 */
	if (entry->dlopen_ref != NULL) {
		re = dlclose (entry->dlopen_ref);
	}

	free (entry);

	return (re == 0) ? ret_ok : ret_error;
}


ret_t 
cherokee_plugin_loader_get_info (cherokee_plugin_loader_t *loader, char *modname, cherokee_plugin_info_t **info)
{
	ret_t    ret;
	entry_t *entry;

	ret = cherokee_table_get (&loader->table, modname, (void **)&entry);
	if (ret != ret_ok) return ret;

	*info = entry->info;
	return ret_ok;
}


ret_t
cherokee_plugin_loader_get_sym  (cherokee_plugin_loader_t *loader, char *modname, char *name, void **sym)
{
	ret_t    ret;
	entry_t *entry;
	void    *tmp;

	/* Get the symbol from a dynamic library
	 */
	ret = cherokee_table_get (&loader->table, modname, (void **)&entry);
	if (ret != ret_ok) return ret;

	/* Even if we're trying to look for symbols in the executable,
	 * using dlopen(NULL), the handler pointer should not be nil.
	 */
	if (entry->dlopen_ref == NULL) 
		return ret_not_found;

	tmp = get_sym_from_dlopen_handler (entry->dlopen_ref, name);
	if (tmp == NULL) return ret_not_found;

	*sym = tmp;
	return ret_ok;
}


#endif /* CHEROKEE_EMBEDDED */


ret_t 
cherokee_plugin_loader_get (cherokee_plugin_loader_t *loader, char *modname, cherokee_plugin_info_t **info)
{
	   ret_t ret;

	   ret = cherokee_plugin_loader_load (loader, modname);
	   if (ret != ret_ok) return ret;

	   ret = cherokee_plugin_loader_get_info (loader, modname, info);
	   if (ret != ret_ok) return ret;

	   return ret_ok;
}


ret_t 
cherokee_plugin_loader_set_directory  (cherokee_plugin_loader_t *loader, cherokee_buffer_t *dir)
{
	cherokee_buffer_clean (&loader->module_dir);
	cherokee_buffer_add_buffer (&loader->module_dir, dir);

	return ret_ok;
}
