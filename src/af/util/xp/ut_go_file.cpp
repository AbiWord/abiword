 /* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-file.c :
 *
 * Copyright (C) 2009, 2013-2014 Hubert Figuiere <hub@figuiere.net>.
 *     Whose contributions are under GPLv2+
 * Copyright (C) 2004 Morten Welinder (terra@gnome.org)
 * Copyright (C) 2003, Red Hat, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if 0
#include <goffice/goffice-config.h>
#include <glib/gi18n-lib.h>
#endif
#include "ut_go_file.h"
#include <gsf/gsf-impl-utils.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-input-impl.h>
#include <gsf/gsf-output-impl.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-output-stdio.h>
#include <glib/gstdio.h>
#include <libxml/encoding.h>

#if TOOLKIT_COCOA
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef WITH_GSF_INPUT_HTTP
#include <gsf/gsf-input-http.h>
#endif
#include <stdio.h>

#include <gsf/gsf-input-gio.h>
#include <gsf/gsf-output-gio.h>

#ifdef TOOLKIT_GTK_ALL
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif

#ifdef TOOLKIT_QT
#include <QUrl>
#include <QDesktopServices>
#endif

#if defined G_OS_WIN32
#include <windows.h>
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>
#endif

#include <string>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
  #ifndef S_ISDIR
  #define S_ISDIR(m) (((m) & _S_IFMT ) == _S_IFDIR)
  #endif
#else
#include <unistd.h>
#endif

#include "ut_types.h"

#ifndef _
#define _(X) X
#endif

/* ------------------------------------------------------------------------- */

/* TODO: push this up into libgsf proper */

GsfInput *
gsf_input_memory_new_from_file (FILE * input)
{
	GsfOutput *memory_output;
	GsfInput  *memory_input = NULL;

	g_return_val_if_fail (input != NULL, NULL);

	memory_output = gsf_output_memory_new ();
	while (TRUE) {
		guint8 buf[1024];
		size_t nread;
		gboolean res;

		nread = fread (buf, 1, sizeof(buf), input);
		res = gsf_output_write (memory_output, nread, buf);

		if (ferror (input) || !res) {
			/* trouble reading from @input or trouble writing to @memory_output */
			g_object_unref (G_OBJECT (memory_output));
			return NULL;
		}
		else if ((nread < sizeof(buf)) && feof (input)) /* hit eof */
			break;
	}

	if (gsf_output_close (memory_output))
		memory_input = gsf_input_memory_new_clone (gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (memory_output)),
							   gsf_output_size (memory_output));

	g_object_unref (G_OBJECT (memory_output));

	return memory_input;
}

/* ------------------------------------------------------------------------- */

/* TODO: push this up into libgsf proper */

#define GSF_OUTPUT_PROXY_TYPE	(gsf_output_proxy_get_type ())
#define GSF_OUTPUT_PROXY(o)	(G_TYPE_CHECK_INSTANCE_CAST ((o), GSF_OUTPUT_PROXY_TYPE, GsfOutputProxy))
#define GSF_IS_OUTPUT_PROXY(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), GSF_OUTPUT_PROXY_TYPE))

typedef struct _GsfOutputProxy GsfOutputProxy;

GType gsf_output_proxy_get_type      (void) G_GNUC_CONST;
void  gsf_output_proxy_register_type (GTypeModule *module);

GsfOutput *gsf_output_proxy_new      (GsfOutput * sink);

enum {
	PROP_0,
	PROP_SINK
};

static GsfOutputClass *parent_class;

struct _GsfOutputProxy {
	GsfOutput output;
	GsfOutput *memory_output;
	GsfOutput *sink;
};

typedef struct {
	GsfOutputClass output_class;
} GsfOutputProxyClass;

/**
 * gsf_output_proxy_new :
 *
 * Returns a new file or NULL.
 **/
GsfOutput *
gsf_output_proxy_new (GsfOutput * sink)
{
	g_return_val_if_fail (sink != NULL, NULL);
	g_return_val_if_fail (GSF_IS_OUTPUT (sink), NULL);

	return (GsfOutput *)g_object_new (GSF_OUTPUT_PROXY_TYPE, "sink", sink, (void *)NULL);	
}

static gboolean
gsf_output_proxy_close (GsfOutput *object)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;

	if(gsf_output_close (proxy->memory_output))
		{
			const guint8 *bytes;
			size_t num_bytes;

			bytes = gsf_output_memory_get_bytes (GSF_OUTPUT_MEMORY (proxy->memory_output));
			num_bytes = gsf_output_size (proxy->memory_output);

			if (gsf_output_write (proxy->sink, num_bytes, bytes))
				return gsf_output_close (proxy->sink);
		}

	return FALSE;
}

static void
gsf_output_proxy_finalize (GObject *object)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;
	
	g_object_unref (proxy->memory_output);
	g_object_unref (proxy->sink);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gsf_output_proxy_seek (GsfOutput *object,
		       gsf_off_t offset,
		       GSeekType whence)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;

	return gsf_output_seek (proxy->memory_output, offset, whence);
}


static gboolean
gsf_output_proxy_write (GsfOutput *object,
			size_t num_bytes,
			guint8 const *buffer)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;
	
	return gsf_output_write (proxy->memory_output, num_bytes, buffer);
}

static gsf_off_t gsf_output_proxy_vprintf (GsfOutput *object,
					  char const *format, va_list args) G_GNUC_PRINTF (2, 0);

static gsf_off_t
gsf_output_proxy_vprintf (GsfOutput *object, char const *format, va_list args)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;

	return gsf_output_vprintf (proxy->memory_output, format, args);
}

static void
gsf_output_proxy_get_property (GObject     *object,
			       guint        property_id,
			       GValue      *value,
			       GParamSpec  *pspec)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;

	switch (property_id) {
	case PROP_SINK:
		g_value_set_object (value, proxy->sink);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
gsf_output_proxy_set_sink (GsfOutputProxy *proxy, GsfOutput *sink)
{
	g_return_if_fail (GSF_IS_OUTPUT (sink));
	g_object_ref (sink);
	if (proxy->sink)
		g_object_unref (proxy->sink);
	proxy->sink = sink;
}

static void
gsf_output_proxy_set_property (GObject      *object,
			       guint         property_id,
			       GValue const *value,
			       GParamSpec   *pspec)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;

	switch (property_id) {
	case PROP_SINK:
		gsf_output_proxy_set_sink (proxy, (GsfOutput *)g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
gsf_output_proxy_init (GObject *object)
{
	GsfOutputProxy *proxy = (GsfOutputProxy *)object;

	proxy->memory_output = gsf_output_memory_new ();
	proxy->sink = NULL;
}

static void
gsf_output_proxy_class_init (GObjectClass *gobject_class)
{
	GsfOutputClass *output_class = GSF_OUTPUT_CLASS (gobject_class);
	
	gobject_class->finalize = gsf_output_proxy_finalize;
	gobject_class->set_property = gsf_output_proxy_set_property;
	gobject_class->get_property = gsf_output_proxy_get_property;
	output_class->Close     = gsf_output_proxy_close;
	output_class->Seek      = gsf_output_proxy_seek;
	output_class->Write     = gsf_output_proxy_write;
	output_class->Vprintf   = gsf_output_proxy_vprintf;

	g_object_class_install_property
		(gobject_class,
		 PROP_SINK,
		 g_param_spec_object ("sink", "Sink",
				      "Where the converted data is written.",
				      GSF_OUTPUT_TYPE,
				      (GParamFlags)(GSF_PARAM_STATIC |
						    G_PARAM_READWRITE |
						    G_PARAM_CONSTRUCT_ONLY)));

	parent_class = GSF_OUTPUT_CLASS (g_type_class_peek_parent (gobject_class));
}

/* GSF_DYNAMIC_CLASS once we move this back into libgsf */
GSF_CLASS (GsfOutputProxy, gsf_output_proxy,
	   gsf_output_proxy_class_init, gsf_output_proxy_init,
	   GSF_OUTPUT_TYPE)

/* ------------------------------------------------------------------------- */

static gboolean
is_fd_uri (const char *uri, int *fd);

/* ------------------------------------------------------------------------- */

/*
 * Return TRUE if @path represents a URI, false if not
 */
gboolean 
UT_go_path_is_uri (const char * path)
{
	// hack until i come up with a better test
	if (g_str_has_prefix (path, "mailto:"))
		return TRUE;
	else
		return (strstr (path, "://") != NULL);
}

gboolean UT_go_path_is_path (const char * path)
{
	return (strstr (path, G_DIR_SEPARATOR_S) != NULL);
}

/*
 * Convert an escaped URI into a filename.
 */
char *
UT_go_filename_from_uri (const char *uri)
{
	return g_filename_from_uri (uri, NULL, NULL);
}

/*
 * Convert a filename into an escaped URI.
 */
char *
UT_go_filename_to_uri (const char *filename)
{
	char *simp, *uri;

	g_return_val_if_fail (filename != NULL, NULL);

	simp = UT_go_filename_simplify (filename, UT_GO_DOTDOT_TEST, TRUE);

	uri = g_filename_to_uri (simp, NULL, NULL);
	g_free (simp);
	return uri;
}

char *
UT_go_filename_simplify (const char *filename, UT_GODotDot dotdot,
		      gboolean make_absolute)
{
	char *simp, *p, *q;

	g_return_val_if_fail (filename != NULL, NULL);

	if (make_absolute && !g_path_is_absolute (filename)) {
		/*
		 * FIXME: this probably does not work for "c:foo" on
		 * Win32.
		 */
		char *current_dir = g_get_current_dir ();
		simp = g_build_filename (current_dir, filename, NULL);
		g_free (current_dir);
	} else
		simp = g_strdup (filename);

	for (p = q = simp; *p;) {
		if (p != simp &&
		    G_IS_DIR_SEPARATOR (p[0]) &&
		    G_IS_DIR_SEPARATOR (p[1])) {
			/* "//" --> "/", except initially.  */
			p++;
			continue;
		}

		if (G_IS_DIR_SEPARATOR (p[0]) &&
		    p[1] == '.' &&
		    G_IS_DIR_SEPARATOR (p[2])) {
			/* "/./" -> "/".  */
			p += 2;
			continue;
		}

		if (G_IS_DIR_SEPARATOR (p[0]) &&
		    p[1] == '.' &&
		    p[2] == '.' &&
		    G_IS_DIR_SEPARATOR (p[3])) {
			if (p == simp) {
				/* "/../" --> "/" initially.  */
				p += 3;
				continue;
			} else if (p == simp + 1) {
				/* Nothing, leave "//../" initially alone.  */
			} else {
				/*
				 * "prefix/dir/../" --> "prefix/" if
				 * "dir" is an existing directory (not
				 * a symlink).
				 */
				gboolean isdir;

				switch (dotdot) {
				case UT_GO_DOTDOT_SYNTACTIC:
					isdir = TRUE;
					break;
				case UT_GO_DOTDOT_TEST: {
#if GLIB_CHECK_VERSION(2,26,0) || defined(G_OS_WIN32)
					GStatBuf statbuf;
#else
					struct stat statbuf;
#endif
					char savec = *q;
					/*
					 * Terminate the path so far so we can
					 * it.  Restore because "p" loops over
					 * the same.
					 */
					*q = 0;
					isdir = (g_lstat (simp, &statbuf) == 0) &&
						S_ISDIR (statbuf.st_mode);
					*q = savec;
					break;
				}
				default:
					isdir = FALSE;
					break;
				}

				if (isdir) {
					do {
						g_assert (q != simp);
						q--;
					} while (!G_IS_DIR_SEPARATOR (*q));
					p += 3;
					continue;
				} else {
					/*
					 * Do nothing.
					 *
					 * Maybe the prefix does not
					 * exist, or maybe it is not
					 * a directory (for example
					 * because it is a symlink).
					 */
				}
			}
		}

		*q++ = *p++;
	}
	*q = 0;

	return simp;
}

/*
 * Simplify a potentially non-local path using only slashes.
 */
static char *
simplify_path (const char *uri)
{
	char *simp, *p, *q;

	simp = g_strdup (uri);

	for (p = q = simp; *p;) {
		if (p[0] == '/' && p[1] == '/') {
			/* "//" --> "/".  */
			p++;
			continue;
		}

		if (p[0] == '/' && p[1] == '.' && p[2] == '/') {
			/* "/./" -> "/".  */
			p += 2;
			continue;
		}

		if (p[0] == '/' && p[1] == '.' && p[2] == '.' && p[3] == '/') {
			if (p == simp) {
				/* "/../" --> "/" initially.  */
				p += 3;
				continue;
			} else {
				/* Leave alone */
			}
		}

		*q++ = *p++;
	}
	*q = 0;

	return simp;
}

static char *
simplify_host_path (const char *uri, size_t hstart)
{
	const char *slash = strchr (uri + hstart, '/');
	char *simp, *psimp;
	size_t pstart;

	if (!slash)
		return g_strdup (uri);

	pstart = slash + 1 - uri;
	psimp = simplify_path (slash + 1);
	simp = g_new (char, pstart + strlen (psimp) + 1);
	memcpy (simp, uri, pstart);
	strcpy (simp + pstart, psimp);
	g_free (psimp);
	return simp;
}

char *
UT_go_url_simplify (const char *uri)
{
	char *simp, *p;

	g_return_val_if_fail (uri != NULL, NULL);

	if (g_ascii_strncasecmp (uri, "file:///", 8) == 0) {
		char *filename = UT_go_filename_from_uri (uri);
		simp = filename ? UT_go_filename_to_uri (filename) : NULL;
		g_free (filename);
		return simp;
	}

	if (g_ascii_strncasecmp (uri, "http://", 7) == 0)
		simp = simplify_host_path (uri, 7);
	else if (g_ascii_strncasecmp (uri, "https://", 8) == 0)
		simp = simplify_host_path (uri, 8);
	else if (g_ascii_strncasecmp (uri, "ftp://", 6) == 0)
		simp = simplify_host_path (uri, 6);
	else
		simp = g_strdup (uri);

	/* Lower-case protocol name.  */
	for (p = simp; g_ascii_isalpha (*p); p++)
		*p = g_ascii_tolower (*p);

	return simp;
}

/* code borrowed from gnome-vfs' gnome-vfs-uri.c */

static gboolean
is_uri_relative (const char *uri)
{
        const char *current;

        /* RFC 2396 section 3.1 */
        for (current = uri ;
                *current
		     &&      ((*current >= 'a' && *current <= 'z')
			      || (*current >= 'A' && *current <= 'Z')
			      || (*current >= '0' && *current <= '9')
			      || ('-' == *current)
			      || ('+' == *current)
			      || ('.' == *current)) ;
             current++) {
	}

        return  !(':' == *current);
}

static void
remove_internal_relative_components (char *uri_current)
{
	char *segment_prev, *segment_cur;
	gsize len_prev, len_cur;

	len_prev = len_cur = 0;
	segment_prev = NULL;

	segment_cur = uri_current;

	while (*segment_cur) {
		len_cur = strcspn (segment_cur, "/");

		if (len_cur == 1 && segment_cur[0] == '.') {
			/* Remove "." 's */
			if (segment_cur[1] == '\0') {
				segment_cur[0] = '\0';
				break;
			} else {
				memmove (segment_cur, segment_cur + 2, strlen (segment_cur + 2) + 1);
				continue;
			}
		} else if (len_cur == 2 && segment_cur[0] == '.' && segment_cur[1] == '.' ) {
			/* Remove ".."'s (and the component to the left of it) that aren't at the
			 * beginning or to the right of other ..'s
			 */
			if (segment_prev) {
				if (! (len_prev == 2
				       && segment_prev[0] == '.'
				       && segment_prev[1] == '.')) {
				       	if (segment_cur[2] == '\0') {
						segment_prev[0] = '\0';
						break;
				       	} else {
						memmove (segment_prev, segment_cur + 3, strlen (segment_cur + 3) + 1);

						segment_cur = segment_prev;
						len_cur = len_prev;

						/* now we find the previous segment_prev */
						if (segment_prev == uri_current) {
							segment_prev = NULL;
						} else if (segment_prev - uri_current >= 2) {
							segment_prev -= 2;
							for ( ; segment_prev > uri_current && segment_prev[0] != '/' 
							      ; segment_prev-- ) {
							}
							if (segment_prev[0] == '/') {
								segment_prev++;
							}
						}
						continue;
					}
				}
			}
		}

		/*Forward to next segment */

		if (segment_cur [len_cur] == '\0') {
			break;
		}
		 
		segment_prev = segment_cur;
		len_prev = len_cur;
		segment_cur += len_cur + 1;	
	}
	
}

static char *
make_full_uri_from_relative (const char *base_uri, const char *uri)
{
	char *result = NULL;

	char *mutable_base_uri;
	char *mutable_uri;
	
	char *uri_current;
	gsize base_uri_length;
	char *separator;
	
	/* We may need one extra character
	 * to append a "/" to uri's that have no "/"
	 * (such as help:)
	 */

	mutable_base_uri = (char *)g_malloc(strlen(base_uri)+2);
	strcpy (mutable_base_uri, base_uri);
		
	uri_current = mutable_uri = g_strdup (uri);

	/* Chew off Fragment and Query from the base_url */

	separator = strrchr (mutable_base_uri, '#'); 

	if (separator) {
		*separator = '\0';
	}

	separator = strrchr (mutable_base_uri, '?');

	if (separator) {
		*separator = '\0';
	}

	if ('/' == uri_current[0] && '/' == uri_current [1]) {
		/* Relative URI's beginning with the authority
		 * component inherit only the scheme from their parents
		 */

		separator = strchr (mutable_base_uri, ':');

		if (separator) {
			separator[1] = '\0';
		}			  
	} else if ('/' == uri_current[0]) {
		/* Relative URI's beginning with '/' absolute-path based
		 * at the root of the base uri
		 */

		separator = strchr (mutable_base_uri, ':');

		/* g_assert (separator), really */
		if (separator) {
			/* If we start with //, skip past the authority section */
			if ('/' == separator[1] && '/' == separator[2]) {
				separator = strchr (separator + 3, '/');
				if (separator) {
					separator[0] = '\0';
				}
			} else {
				/* If there's no //, just assume the scheme is the root */
				separator[1] = '\0';
			}
		}
	} else if ('#' != uri_current[0]) {
		/* Handle the ".." convention for relative uri's */

		/* If there's a trailing '/' on base_url, treat base_url
		 * as a directory path.
		 * Otherwise, treat it as a file path, and chop off the filename
		 */

		base_uri_length = strlen (mutable_base_uri);
		if ('/' == mutable_base_uri[base_uri_length-1]) {
			/* Trim off '/' for the operation below */
			mutable_base_uri[base_uri_length-1] = 0;
		} else {
			separator = strrchr (mutable_base_uri, '/');
			if (separator) {
				/* Make sure we don't eat a domain part */
				char *tmp = separator - 1;
				if ((separator != mutable_base_uri) && (*tmp != '/')) {
					*separator = '\0';
				} else {
					/* Maybe there is no domain part and this is a toplevel URI's child */
					char *tmp2 = strstr (mutable_base_uri, ":///");
					if (tmp2 != NULL && tmp2 + 3 == separator) {
						*(separator + 1) = '\0';
					}
				}
			}
		}

		remove_internal_relative_components (uri_current);

		/* handle the "../"'s at the beginning of the relative URI */
		while (0 == strncmp ("../", uri_current, 3)) {
			uri_current += 3;
			separator = strrchr (mutable_base_uri, '/');
			if (separator) {
				*separator = '\0';
			} else {
				/* <shrug> */
				break;
			}
		}

		/* handle a ".." at the end */
		if (uri_current[0] == '.' && uri_current[1] == '.' 
		    && uri_current[2] == '\0') {

			uri_current += 2;
			separator = strrchr (mutable_base_uri, '/');
			if (separator) {
				*separator = '\0';
			}
		}

		/* Re-append the '/' */
		mutable_base_uri [strlen(mutable_base_uri)+1] = '\0';
		mutable_base_uri [strlen(mutable_base_uri)] = '/';
	}

	result = g_strconcat (mutable_base_uri, uri_current, NULL);
	g_free (mutable_base_uri); 
	g_free (mutable_uri); 
	
	return result;
}

/*
 * More or less the same as gnome_vfs_uri_make_full_from_relative.
 */
char *
UT_go_url_resolve_relative (const char *ref_uri, const char *rel_uri)
{
	char *simp, *uri;

	g_return_val_if_fail (rel_uri != NULL, NULL);

	if (is_uri_relative (rel_uri)) {
		g_return_val_if_fail (ref_uri != NULL, NULL);
		uri = make_full_uri_from_relative (ref_uri, 
						   rel_uri);
	} else {
		uri = g_strdup (rel_uri);
	}

	simp = UT_go_url_simplify (uri);
	g_free (uri);
	return simp;
}

static char *
make_rel (const char *uri, const char *ref_uri,
	  const char *uri_host, const char *slash)
{
	const char *p, *q;
	int n;
	GString *res;

	if (!slash)
		return NULL;

	if (uri_host != NULL &&
	    strncmp (uri_host, ref_uri + (uri_host - uri), slash - uri_host))
		return NULL;

	for (p = slash; *p; p++) {
		if (*p != ref_uri[p - uri])
			break;
		else if (*p == '/')
			slash = p;
	}
	/* URI components agree until slash.  */

	/* Find out the number of '/' in uri after slash.  */
	n = 0;
	q = slash;
	while (1) {
		q = strchr (q + 1, '/');
		if (q)
			n++;
		else
			break;
	}

	res = g_string_new (NULL);
	while (n-- > 0)
		g_string_append (res, "../");
	g_string_append (res, slash + 1);
	return g_string_free (res, FALSE);
}

char *
UT_go_url_make_relative (const char *uri, const char *ref_uri)
{
	int i;

	/* Check that protocols are the same.  */
	for (i = 0; 1; i++) {
		char c = uri[i];
		char rc = ref_uri[i];

		if (c == 0)
			return NULL;

		if (c == ':') {
			if (rc == ':')
				break;
			return NULL;
		}

		if (g_ascii_tolower (c) != g_ascii_tolower (rc))
			return NULL;
	}

	if (g_ascii_strncasecmp (uri, "file:///", 8) == 0)
		return make_rel (uri, ref_uri, NULL, uri + 7);  /* Yes, 7.  */

	if (g_ascii_strncasecmp (uri, "http://", 7) == 0)
		return make_rel (uri, ref_uri, uri + 7, strchr (uri + 7, '/'));

	if (g_ascii_strncasecmp (uri, "https://", 8) == 0)
		return make_rel (uri, ref_uri, uri + 8, strchr (uri + 8, '/'));

	if (g_ascii_strncasecmp (uri, "ftp://", 6) == 0)
		return make_rel (uri, ref_uri, uri + 6, strchr (uri + 6, '/'));

	return NULL;
}

/*
 * Convert a shell argv entry (assumed already translated into filename
 * encoding) to an escaped URI.
 */
char *
UT_go_shell_arg_to_uri (const char *arg)
{
	gchar *tmp;

	if (is_fd_uri (arg, NULL))
		return g_strdup (arg);

	if (g_path_is_absolute (arg) || strchr (arg, ':') == NULL)
		return UT_go_filename_to_uri (arg);

	tmp = UT_go_filename_from_uri (arg);
	if (tmp) {
		/*
		 * Do the reverse translation to get a minimum of
		 * canonicalization.
		 */
		char *res = UT_go_filename_to_uri (tmp);
		g_free (tmp);
		return res;
	}

#if !defined(WITH_GSF_INPUT_HTTP)
	{
		GFile *f = g_file_new_for_commandline_arg (arg);
		char *uri = g_file_get_uri (f);
		g_object_unref (G_OBJECT (f));
		if (uri) {
			char *uri2 = UT_go_url_simplify(uri);
			g_free (uri);
			return uri2;
		}
	}
#else
	{
		if (g_ascii_strncasecmp (arg, "http://", strlen ("http://")) == 0) {
			return UT_go_url_simplify (arg);
		}
	}
#endif

	/* Just assume it's a filename.  */
	return UT_go_filename_to_uri (arg);
}

/**
 * UT_go_basename_from_uri:
 * @uri :
 *
 * Decode the final path component.  Returns as UTF-8 encoded suitable
 * for display.
 **/
char *
UT_go_basename_from_uri (const char *uri)
{
	char *res;

	GFile *f = g_file_new_for_uri (uri);
	char *basename = g_file_get_basename (f);
	g_object_unref (G_OBJECT (f));

	res = basename ? g_filename_display_name (basename) : NULL;
	g_free (basename);
	return res;
}

/**
 * UT_go_dirname_from_uri:
 * @uri :
 * @brief: if TRUE, hide "file://" if present.
 *
 * Decode the all but the final path component.  Returns as UTF-8 encoded
 * suitable for display.
 **/
char *
UT_go_dirname_from_uri (const char *uri, gboolean brief)
{
	char *dirname_utf8, *dirname;

	char *uri_dirname = g_path_get_dirname (uri);
	dirname = uri_dirname ? UT_go_filename_from_uri (uri_dirname) : NULL;
	if(uri_dirname) {
		g_free (uri_dirname);
	}
	uri_dirname = dirname ? g_strconcat ("file://", dirname, NULL) : NULL;
	if(dirname) {
		g_free (dirname);
	}
	dirname = uri_dirname;

	if (brief && dirname &&
	    g_ascii_strncasecmp (dirname, "file:///", 8) == 0) {
		char *temp = g_strdup (dirname + 7);
		g_free (dirname);
		dirname = temp;
	}

	dirname_utf8 = dirname ? g_filename_display_name (dirname) : NULL;
	g_free (dirname);
	return dirname_utf8;
}


gboolean 
UT_go_directory_create (char const *uri, int mode, GError **error)
{
	GFile *f = g_file_new_for_uri (uri);
	gboolean res = g_file_make_directory (f, NULL, error);
	g_object_unref (G_OBJECT (f));
	UT_UNUSED(mode);
	return res;
}

/* ------------------------------------------------------------------------- */

static gboolean
is_fd_uri (const char *uri, int *fd)
{
	unsigned long ul;
	char *end;

	if (g_ascii_strncasecmp (uri, "fd://", 5))
		return FALSE;
	uri += 5;
	if (!g_ascii_isdigit (*uri))
		return FALSE;  /* Space, for example.  */

	ul = strtoul (uri, &end, 10);
	if (*end != 0 || ul > INT_MAX)
		return FALSE;

	if (fd != NULL)
		*fd = (int)ul;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static GsfInput *
open_plain_file (const char *path, GError **err)
{
	GsfInput *input = gsf_input_mmap_new (path, NULL);
	if (input != NULL)
		return input;
	/* Only report error if stdio fails too */
	return gsf_input_stdio_new (path, err);
}

static GsfInput *
UT_go_file_open_impl (char const *uri, GError **err)
{
	char *filename;
	int fd;

	if (err != NULL)
		*err = NULL;
	g_return_val_if_fail (uri != NULL, NULL);

	if (uri[0] == G_DIR_SEPARATOR) {
		g_warning ("Got plain filename %s in UT_go_file_open.", uri);
		return open_plain_file (uri, err);
	}

	filename = UT_go_filename_from_uri (uri);
	if (filename) {
		GsfInput *result = open_plain_file (filename, err);
		g_free (filename);
		return result;
	}

	if (is_fd_uri (uri, &fd)) {
#if defined G_OS_WIN32
		setmode (fd, O_BINARY);
#endif
		int fd2 = dup (fd);
		FILE *fil = fd2 != -1 ? fdopen (fd2, "rb") : NULL;
		GsfInput *result;

		if (!fil) {
			g_set_error (err, gsf_output_error_id (), 0,
				     "Unable to read from %s", uri);
			return NULL;
		}

		/* guarantee that file descriptors will be seekable */
		result = gsf_input_memory_new_from_file (fil);
		fclose (fil);

		return result;
	}

	if (!strncmp (uri, "http://", 7) || !strncmp (uri, "https://", 8))
		return gsf_input_http_new (uri, err);

	return gsf_input_gio_new_for_uri (uri, err);
}

/**
 * UT_go_file_open :
 * @uri :
 * @err : #GError
 *
 * Try all available methods to open a file or return an error
 **/
GsfInput *
UT_go_file_open (char const *uri, GError **err)
{
	GsfInput * input;

	input = UT_go_file_open_impl (uri, err);
	if (input != NULL)
	{
		GsfInput * uncompress = gsf_input_uncompress (input);
		gsf_input_set_name (uncompress, uri);
		return uncompress;
	}
	return NULL;
}

static GsfOutput *
gsf_output_proxy_create (GsfOutput *wrapped, char const *uri, GError **err)
{
	if (!wrapped) {
		g_set_error (err, gsf_output_error_id (), 0,
			     "Unable to write to %s", uri);
		return NULL;
	}
	
	/* guarantee that file descriptors will be seekable */
	return gsf_output_proxy_new (wrapped);
}

static GsfOutput *
UT_go_file_create_impl (char const *uri, GError **err)
{
	char *filename;
	int fd;
	g_return_val_if_fail (uri != NULL, NULL);

	std::string path = uri;
        bool is_uri = UT_go_path_is_uri(path.c_str());
        bool is_filename = is_uri ? false : path.find_last_of(G_DIR_SEPARATOR) == std::string::npos;
	bool is_path = !is_uri && !is_filename;

	filename = UT_go_filename_from_uri (uri);
	if (is_path || filename) {
		GsfOutput *result = gsf_output_stdio_new (filename?filename:uri, err);
		if(filename) 
			g_free (filename);
		return result;
	}

	if (is_fd_uri (uri, &fd)) {
#if defined G_OS_WIN32
		setmode (fd, O_BINARY);
#endif
		int fd2 = dup (fd);
		FILE *fil = fd2 != -1 ? fdopen (fd2, "wb") : NULL;
		GsfOutput *result = fil ? gsf_output_stdio_new_FILE (uri, fil, FALSE) : NULL;

		/* guarantee that file descriptors will be seekable */
		return gsf_output_proxy_create(result, uri, err);
	}

	return gsf_output_proxy_create(gsf_output_gio_new_for_uri (uri, err), uri, err);
}

GsfOutput *
UT_go_file_create (char const *uri, GError **err)
{
	GsfOutput * output;

	output = UT_go_file_create_impl (uri, err);
	if (output != NULL)
	{
		gsf_output_set_name (output, uri);
		return output;
	}
	return NULL;
}

gboolean
UT_go_file_remove (char const *uri, GError ** err)
{
	char *filename;

	g_return_val_if_fail (uri != NULL, FALSE);

	filename = UT_go_filename_from_uri (uri);
	if (filename) {
		int result = remove (filename);
		g_free (filename);
		return (result == 0);
	}



	GFile *f = g_file_new_for_uri (uri);
	gboolean res = g_file_delete (f, NULL, err);
	g_object_unref (G_OBJECT (f));

	return res;
}

gboolean
UT_go_file_exists (char const *uri)
{
	GFile *f = g_file_new_for_uri (uri);
	gboolean res = g_file_query_exists (f, NULL);
	g_object_unref (G_OBJECT (f));
	return res;
}

UT_GOFilePermissions *
UT_go_get_file_permissions (char const *uri)
{
	UT_GOFilePermissions * file_permissions = NULL;

#if GLIB_CHECK_VERSION(2,26,0) || defined(G_OS_WIN32)
	GStatBuf file_stat;
#else
	struct stat file_stat;
#endif
	char *filename = UT_go_filename_from_uri (uri);
	int result = filename ? g_stat (filename, &file_stat) : -1;

	g_free (filename);
	if (result == 0) {
		file_permissions = g_new0 (UT_GOFilePermissions, 1);
#if ! defined (G_OS_WIN32)
		/* Owner  Permissions */
		file_permissions->owner_read    = ((file_stat.st_mode & S_IRUSR) != 0);
		file_permissions->owner_write   = ((file_stat.st_mode & S_IWUSR) != 0);
		file_permissions->owner_execute = ((file_stat.st_mode & S_IXUSR) != 0);

		/* Group  Permissions */
		file_permissions->group_read    = ((file_stat.st_mode & S_IRGRP) != 0);
		file_permissions->group_write   = ((file_stat.st_mode & S_IWGRP) != 0);
		file_permissions->group_execute = ((file_stat.st_mode & S_IXGRP) != 0);

		/* Others Permissions */
		file_permissions->others_read    = ((file_stat.st_mode & S_IROTH) != 0);
		file_permissions->others_write   = ((file_stat.st_mode & S_IWOTH) != 0);
		file_permissions->others_execute = ((file_stat.st_mode & S_IXOTH) != 0);
#else
		/* Windows */
		/* Owner  Permissions */
		file_permissions->owner_read    = ((file_stat.st_mode & S_IREAD) != 0);
		file_permissions->owner_write   = ((file_stat.st_mode & S_IWRITE) != 0);
		file_permissions->owner_execute = ((file_stat.st_mode & S_IEXEC) != 0);
#endif
	}
	return file_permissions;
}

void
UT_go_set_file_permissions (char const *uri, UT_GOFilePermissions * file_permissions)
{
#if ! defined (G_OS_WIN32)
	mode_t permissions = 0;
	int result;
	char *filename;

	/* Set owner permissions */
	if (file_permissions->owner_read == TRUE)
		permissions |= S_IRUSR;

	if (file_permissions->owner_write == TRUE)
		permissions |= S_IWUSR;

	if (file_permissions->owner_execute == TRUE)
		permissions |= S_IXUSR;

	/* Set group permissions */
	if (file_permissions->group_read == TRUE)
		permissions |= S_IRGRP;

	if (file_permissions->group_write == TRUE)
		permissions |= S_IWGRP;

	if (file_permissions->group_execute == TRUE)
		permissions |= S_IXGRP;

	/* Set others permissions */
	if (file_permissions->others_read == TRUE)
		permissions |= S_IROTH;

	if (file_permissions->others_write == TRUE)
		permissions |= S_IWOTH;

	if (file_permissions->others_execute == TRUE)
		permissions |= S_IXOTH;

	filename = UT_go_filename_from_uri (uri);
	
#ifdef HAVE_G_CHMOD
	result = g_chmod (filename, permissions);
#else
	result = chmod (filename, permissions);
#endif

	g_free (filename);

	if (result != 0)
		g_warning ("Error setting permissions for %s.", uri);
#endif
}

typedef enum {
	UT_GO_FILE_DATE_TYPE_ACCESSED = 0,
	UT_GO_FILE_DATE_TYPE_MODIFIED,
	UT_GO_FILE_DATE_TYPE_CHANGED
} UT_GOFileDateType;

static time_t
UT_go_file_get_date (char const *uri, UT_GOFileDateType type)
{
	time_t tm = -1;

#if GLIB_CHECK_VERSION(2,26,0) || defined(G_OS_WIN32)
	GStatBuf file_stat;
#else
	struct stat file_stat;
#endif
	char *filename = UT_go_filename_from_uri (uri);
	int result = filename ? g_stat (filename, &file_stat) : -1;

	g_free (filename);
	if (result == 0) {
		switch (type) {
			case UT_GO_FILE_DATE_TYPE_ACCESSED:
				tm = file_stat.st_atime;
				break;
			case UT_GO_FILE_DATE_TYPE_MODIFIED:
				tm = file_stat.st_mtime;
				break;
			case UT_GO_FILE_DATE_TYPE_CHANGED:
				tm = file_stat.st_ctime;
				break;
		}
	}

	return tm;
}

time_t
UT_go_file_get_date_accessed (char const *uri)
{
	return UT_go_file_get_date (uri, UT_GO_FILE_DATE_TYPE_ACCESSED);
}

time_t
UT_go_file_get_date_modified (char const *uri)
{
	return UT_go_file_get_date (uri, UT_GO_FILE_DATE_TYPE_MODIFIED);
}

time_t
UT_go_file_get_date_changed (char const *uri)
{
	return UT_go_file_get_date (uri, UT_GO_FILE_DATE_TYPE_CHANGED);
}

/* ------------------------------------------------------------------------- */
#ifdef TOOLKIT_GTK_ALL
// We need this for systems where gtk_show_uri() is broken
// Don't get me started.
// Note that if gtk_show_uri() fails but returns true, then
// there is nothing that can be done.
static char *
check_program (char const *prog)
{
	if (NULL == prog)
		return NULL;
	if (g_path_is_absolute (prog)) {
		if (!g_file_test (prog, G_FILE_TEST_IS_EXECUTABLE))
			return NULL;
	} else if (!g_find_program_in_path (prog))
		return NULL;
	return g_strdup (prog);
}

static void 
fallback_open_uri(const gchar* url, GError** err)
{
	gchar *browser = NULL;
	gchar *clean_url = NULL;

	/* 1) Check BROWSER env var */
	browser = check_program (getenv ("BROWSER"));

	if (browser == NULL) {
		static char const * const browsers[] = {
			"xdg-open",             /* XDG. you shouldn't need anything else */
			"sensible-browser",	/* debian */
			"epiphany",		/* primary gnome */
			"galeon",		/* secondary gnome */
			"encompass",
			"firefox",
			"mozilla-firebird",
			"mozilla",
			"netscape",
			"konqueror",
			"xterm -e w3m",
			"xterm -e lynx",
			"xterm -e links"
		};
		unsigned i;
		for (i = 0 ; i < G_N_ELEMENTS (browsers) ; i++)
			if (NULL != (browser = check_program (browsers[i])))
				break;
  	}

	if (browser != NULL) {
		gint    argc;
		gchar **argv = NULL;
		char   *cmd_line = g_strconcat (browser, " %1", NULL);

		if (g_shell_parse_argv (cmd_line, &argc, &argv, err)) {
			/* check for '%1' in an argument and substitute the url
			 * otherwise append it */
			gint i;
			char *tmp;

			for (i = 1 ; i < argc ; i++)
				if (NULL != (tmp = strstr (argv[i], "%1"))) {
					*tmp = '\0';
					tmp = g_strconcat (argv[i],
						(clean_url != NULL) ? (char const *)clean_url : url,
						tmp+2, NULL);
					g_free (argv[i]);
					argv[i] = tmp;
					break;
				}

			/* there was actually a %1, drop the one we added */
			if (i != argc-1) {
				g_free (argv[argc-1]);
				argv[argc-1] = NULL;
			}
			g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
				NULL, NULL, NULL, err);
			g_strfreev (argv);
		}
		g_free (cmd_line);
	}
	g_free (browser);
	g_free (clean_url);
}
#endif

#ifdef G_OS_WIN32
#undef _
#include "ut_Win32LocaleString.h"
#endif

GError *
UT_go_url_show (gchar const *url)
{
#ifdef G_OS_WIN32
	UT_Win32LocaleString str;
	str.fromUTF8 (url);
	ShellExecuteW (NULL, L"open", str.c_str(), NULL, NULL, SW_SHOWNORMAL);
	return NULL;
#elif TOOLKIT_COCOA
	CFStringRef urlStr = CFStringCreateWithCString(kCFAllocatorDefault, url, kCFStringEncodingUTF8);
	CFURLRef cfUrl = CFURLCreateWithString(kCFAllocatorDefault, urlStr, NULL);
	OSStatus err = LSOpenCFURLRef(cfUrl, NULL);
	CFRelease(cfUrl);
	CFRelease(urlStr);
	if (err != noErr) {
		;
	}
	return NULL;
#elif TOOLKIT_QT
	if(!QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode))) {
		;
	}
	return NULL;
#else
	GError *err = NULL;
	if(!gtk_show_uri (NULL, url, GDK_CURRENT_TIME, &err)) {
		fallback_open_uri(url, &err);
	}
	return err;
#endif
}

gchar *
UT_go_get_mime_type (gchar const *uri)
{
	gboolean content_type_uncertain = FALSE;
	char *content_type = g_content_type_guess (uri, NULL, 0, &content_type_uncertain);
	if (content_type) {
		char *mime_type = g_content_type_get_mime_type (content_type);
		g_free (content_type);

		if (mime_type)
			return mime_type;
	}

	return g_strdup ("application/octet-stream");
}

/* ------------------------------------------------------------------------- */


gint
UT_go_utf8_collate_casefold (const char *a, const char *b)
{
	char *a2 = g_utf8_casefold (a, -1);
	char *b2 = g_utf8_casefold (b, -1);
	int res = g_utf8_collate (a2, b2);
	g_free (a2);
	g_free (b2);
	return res;
}

