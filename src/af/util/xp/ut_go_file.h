/*
 * go-file.h :
 *
 * Copyright (C) 2004 Morten Welinder (terra@gnome.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#ifndef UT_GO_FILE_H
#define UT_GO_FILE_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

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

ABI_EXPORT gboolean UT_go_path_is_uri (const char * path);
ABI_EXPORT gboolean UT_go_path_is_path (const char * path);

ABI_EXPORT char *UT_go_filename_simplify (const char *filename, UT_GODotDot dotdot, gboolean make_absolute);
ABI_EXPORT char *UT_go_url_simplify (const char *uri);

ABI_EXPORT char *UT_go_filename_from_uri (const char *uri);
ABI_EXPORT char *UT_go_filename_to_uri (const char *filename);

ABI_EXPORT char *UT_go_url_resolve_relative (const char *ref_uri, const char *rel_uri);
ABI_EXPORT char *UT_go_url_make_relative (const char *uri, const char *ref_uri);

ABI_EXPORT char *UT_go_shell_arg_to_uri (const char *arg);
ABI_EXPORT char *UT_go_basename_from_uri (const char *uri);
ABI_EXPORT char *UT_go_dirname_from_uri (const char *uri, gboolean brief);
ABI_EXPORT gboolean UT_go_directory_create (char const *uri, GError **err);

ABI_EXPORT GsfInput  *UT_go_file_open		(char const *uri, GError **err);
ABI_EXPORT GsfOutput *UT_go_file_create	(char const *uri, GError **err);

ABI_EXPORT gboolean UT_go_file_remove (char const *uri, GError **err);

ABI_EXPORT gboolean UT_go_file_exists (char const *uri);

ABI_EXPORT UT_GOFilePermissions *UT_go_get_file_permissions (char const *uri);
ABI_EXPORT void UT_go_set_file_permissions (char const *uri, UT_GOFilePermissions * file_permissions);

ABI_EXPORT time_t UT_go_file_get_date_accessed (char const *uri);
ABI_EXPORT time_t UT_go_file_get_date_modified (char const *uri);
ABI_EXPORT time_t UT_go_file_get_date_changed  (char const *uri);

ABI_EXPORT GError	*UT_go_url_show		(gchar const *url);
ABI_EXPORT gchar	*UT_go_get_mime_type	(gchar const *uri);

ABI_EXPORT gint UT_go_utf8_collate_casefold (const char *a, const char *b);

G_END_DECLS

#endif /* UT_GO_FILE_H */
//#endif

