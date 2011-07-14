/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * goffice-utils.h: 
 *
 * Copyright (C) 2003-2004 Jody Goldberg (jody@gnome.org)
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GOFFICE_UTILS_H
#define GOFFICE_UTILS_H

#include <glib.h>

G_BEGIN_DECLS

typedef guint32				GOColor;
typedef struct _GOFont			GOFont;
typedef struct _GOFontMetrics		GOFontMetrics;
typedef struct _GOPattern		GOPattern;
typedef struct _GOMarker		GOMarker;
typedef struct _GOFormat		GOFormat;
typedef struct _GOFormatElement		GOFormatElement;
typedef struct _GODateConventions	GODateConventions;
typedef struct _GOImage		GOImage;

/* rename this */
typedef struct _GOMemChunk		GOMemChunk;

typedef const char *(*GOTranslateFunc)(char const *path, gpointer func_data);

G_END_DECLS

#endif /* GOFFICE_UTILS_H */
