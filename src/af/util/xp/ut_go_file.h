/*
 * go-file.h : 
 *
 * Copyright (C) 2004 Morten Welinder (terra@gnome.org)
 *
 * This program is g_free software; you can redistribute it and/or
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#ifndef UT_GO_FILE_H
#define UT_GO_FILE_H

#include <glib.h>
#include <gsf/gsf.h>
#include <time.h>

G_BEGIN_DECLS

typedef struct _UT_GOFilePermissions UT_GOFilePermissions;

struct _UT_GOFilePermissions {
	gboolean owner_read;
	gboolean owner_write;
	gboolean owner_execute;

	gboolean group_read;
	gboolean group_write;
	gboolean group_execute;

	gboolean others_read;
	gboolean others_write;
	gboolean others_execute;
};

typedef enum {
	UT_GO_DOTDOT_SYNTACTIC,    /* Assume no symlinks.  */
	UT_GO_DOTDOT_TEST,         /* Check.  */
	UT_GO_DOTDOT_LEAVE         /* Leave alone.  */
} UT_GODotDot;

gboolean UT_go_path_is_uri (const char * path);

char *UT_go_filename_simplify (const char *filename, UT_GODotDot dotdot, gboolean make_absolute);
char *UT_go_url_simplify (const char *uri);

char *UT_go_filename_from_uri (const char *uri);
char *UT_go_filename_to_uri (const char *filename);

char *UT_go_url_resolve_relative (const char *ref_uri, const char *rel_uri);
char *UT_go_url_make_relative (const char *uri, const char *ref_uri);

char *UT_go_shell_arg_to_uri (const char *arg);
char *UT_go_basename_from_uri (const char *uri);
char *UT_go_dirname_from_uri (const char *uri, gboolean brief);
gboolean UT_go_directory_create (char const *uri, int mode, GError **err);
gchar const **UT_go_shell_argv_to_glib_encoding (gint argc, gchar const **argv);
void UT_go_shell_argv_to_glib_encoding_free (void);

GsfInput  *UT_go_file_open		(char const *uri, GError **err);
GsfOutput *UT_go_file_create	(char const *uri, GError **err);
GSList	  *UT_go_file_split_urls	(char const *data);

gboolean   UT_go_file_exists (char const *uri);

UT_GOFilePermissions *UT_go_get_file_permissions (char const *uri);
void UT_go_set_file_permissions (char const *uri, UT_GOFilePermissions * file_permissions);

time_t UT_go_file_get_date_accessed (char const *uri);
time_t UT_go_file_get_date_modified (char const *uri);
time_t UT_go_file_get_date_changed  (char const *uri);

gchar	*UT_go_url_decode		(gchar const *text);
gchar	*UT_go_url_encode		(gchar const *text, int type);
GError	*UT_go_url_show		(gchar const *url);
gboolean UT_go_url_check_extension (gchar const *uri,
				 gchar const *std_ext,
				 gchar **new_uri);
gchar	*UT_go_get_mime_type	(gchar const *uri);
gchar	*UT_go_get_mime_type_for_data	(gconstpointer data, int data_size);
gchar const	*UT_go_mime_type_get_description	(gchar const *mime_type);

const char * UT_go_guess_encoding (const char *raw, size_t len, const char *user_guess, char **utf8_str);
char const * UT_go_get_real_name (void);
gint UT_go_utf8_collate_casefold (const char *a, const char *b);

G_END_DECLS

#endif /* UT_GO_FILE_H */
