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
#include <pango/pango.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

typedef guint32					GOColor;
typedef struct _GOEditor                GOEditor;
typedef struct _GOFont			GOFont;
typedef struct _GOFontMetrics	GOFontMetrics;
typedef struct _GOPattern		GOPattern;
typedef struct _GOMarker		GOMarker;
typedef struct _GOFormat		GOFormat;
typedef struct _GODateConventions	GODateConventions;
typedef struct _GOImage			GOImage;
typedef struct _GOPath GOPath;
typedef struct _GOString GOString;
typedef struct _GOStyle			GOStyle;
typedef struct _GOStyledObject	GOStyledObject;

/* rename this */
typedef struct _GOMemChunk		GOMemChunk;

typedef const char *(*GOTranslateFunc)(char const *path, gpointer func_data);

typedef enum {
	GO_LINE_NONE,
	GO_LINE_SOLID,
	GO_LINE_S_DOT,
	GO_LINE_S_DASH_DOT,
	GO_LINE_S_DASH_DOT_DOT,
	GO_LINE_DASH_DOT_DOT_DOT,
	GO_LINE_DOT,
	GO_LINE_S_DASH,
	GO_LINE_DASH,
	GO_LINE_LONG_DASH,
	GO_LINE_DASH_DOT,
	GO_LINE_DASH_DOT_DOT,
	GO_LINE_MAX
} GOLineDashType;

typedef enum {
	GO_LINE_INTERPOLATION_LINEAR,
	GO_LINE_INTERPOLATION_SPLINE,
	GO_LINE_INTERPOLATION_CLOSED_SPLINE,
	GO_LINE_INTERPOLATION_CUBIC_SPLINE,
	GO_LINE_INTERPOLATION_PARABOLIC_CUBIC_SPLINE,
	GO_LINE_INTERPOLATION_CUBIC_CUBIC_SPLINE,
	GO_LINE_INTERPOLATION_CLAMPED_CUBIC_SPLINE,
	GO_LINE_INTERPOLATION_STEP_START,
	GO_LINE_INTERPOLATION_STEP_END,
	GO_LINE_INTERPOLATION_STEP_CENTER_X,
	GO_LINE_INTERPOLATION_STEP_CENTER_Y,
	GO_LINE_INTERPOLATION_MAX
} GOLineInterpolation;

typedef enum
{
  GO_ANCHOR_CENTER,
  GO_ANCHOR_NORTH,
  GO_ANCHOR_NORTH_WEST,
  GO_ANCHOR_NORTH_EAST,
  GO_ANCHOR_SOUTH,
  GO_ANCHOR_SOUTH_WEST,
  GO_ANCHOR_SOUTH_EAST,
  GO_ANCHOR_WEST,
  GO_ANCHOR_EAST,
  GO_ANCHOR_N		= GO_ANCHOR_NORTH,
  GO_ANCHOR_NW		= GO_ANCHOR_NORTH_WEST,
  GO_ANCHOR_NE		= GO_ANCHOR_NORTH_EAST,
  GO_ANCHOR_S		= GO_ANCHOR_SOUTH,
  GO_ANCHOR_SW		= GO_ANCHOR_SOUTH_WEST,
  GO_ANCHOR_SE		= GO_ANCHOR_SOUTH_EAST,
  GO_ANCHOR_W		= GO_ANCHOR_WEST,
  GO_ANCHOR_E		= GO_ANCHOR_EAST
} GOAnchorType;

G_END_DECLS

#include <goffice/goffice.h>

#include <goffice/utils/go-color.h>
#include <goffice/utils/go-file.h>
//#include <goffice/utils/go-format.h>
#include <goffice/utils/go-image.h>

#endif /* GOFFICE_UTILS_H */
