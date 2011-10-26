/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-color.h
 *
 * Copyright (C) 1999, 2000 EMC Capital Management, Inc.
 *
 * Developed by Jon Trowbridge <trow@gnu.org> and
 * Havoc Pennington <hp@pobox.com>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#ifndef GO_COLOR_H
#define GO_COLOR_H

#include <goffice/goffice.h>

G_BEGIN_DECLS

typedef struct {
	GOColor		 color;
	char const 	*name;	/* english name - eg. "white" */
} GONamedColor;

/*
  Some convenient macros for drawing into an RGB buffer.
  Beware of side effects, code-bloat, and all of the other classic
  cpp-perils...
*/

#define GO_COLOR_FROM_GDK_RGBA(c)	GO_COLOR_FROM_RGBA((int)((c).red * 255.), (int)((c).green * 255.), (int)((c).blue * 255.), (int)((c).alpha * 255.))

#define GO_COLOR_FROM_RGB(r,g,b)	((((guint)(r&0xff))<<24)|(((guint)(g&0xff))<<16)|((guint)(b&0xff)<<8)|0xff)
#define GO_COLOR_FROM_RGBA(r,g,b,a)	((((guint)(r&0xff))<<24)|(((guint)(g&0xff))<<16)|((guint)(b&0xff)<<8)|(guint)(a&0xff))
#define GO_COLOR_WHITE  GO_COLOR_FROM_RGB(0xff, 0xff, 0xff)
#define GO_COLOR_BLACK   GO_COLOR_FROM_RGB(0x00, 0x00, 0x00)
#define GO_COLOR_RED     GO_COLOR_FROM_RGB(0xff, 0x00, 0x00)
#define GO_COLOR_GREEN   GO_COLOR_FROM_RGB(0x00, 0xff, 0x00)
#define GO_COLOR_BLUE   GO_COLOR_FROM_RGB(0x00, 0x00, 0xff)
#define GO_COLOR_YELLOW  GO_COLOR_FROM_RGB(0xff, 0xff, 0x00)
#define GO_COLOR_VIOLET  GO_COLOR_FROM_RGB(0xff, 0x00, 0xff)
#define GO_COLOR_CYAN    GO_COLOR_FROM_RGB(0x00, 0xff, 0xff)
#define GO_COLOR_GREY(x) GO_COLOR_FROM_RGB(x,x,x)

#define GO_COLOR_UINT_R(x) (((guint)(x))>>24)
#define GO_COLOR_UINT_G(x) ((((guint)(x))>>16)&0xff)
#define GO_COLOR_UINT_B(x) ((((guint)(x))>>8)&0xff)
#define GO_COLOR_UINT_A(x) (((guint)(x))&0xff)
#define GO_COLOR_CHANGE_R(x, r) (((x)&(~(0xff<<24)))|(((r)&0xff)<<24))
#define GO_COLOR_CHANGE_G(x, g) (((x)&(~(0xff<<16)))|(((g)&0xff)<<16))
#define GO_COLOR_CHANGE_B(x, b) (((x)&(~(0xff<<8)))|(((b)&0xff)<<8))
#define GO_COLOR_CHANGE_A(x, a) (((x)&(~0xff))|((a)&0xff))
#define GO_COLOR_TO_RGB(u,r,g,b) \
{ (*(r)) = ((u)>>24)&0xff; (*(g)) = ((u)>>16)&0xff; (*(b)) = ((u)>>8)&0xff; }
#define GO_COLOR_TO_RGBA(u,r,g,b,a) \
{GO_COLOR_TO_RGB((u),r,g,b); (*(a)) = (u)&0xff; }
#define GO_COLOR_MONO_INTERPOLATE(v1, v2, t) ((gint)go_rint((v2)*(t)+(v1)*(1-(t))))
#define GO_COLOR_INTERPOLATE(c1, c2, t) \
  GO_COLOR_FROM_RGBA( GO_COLOR_MONO_INTERPOLATE(GO_COLOR_UINT_R(c1), GO_COLOR_UINT_R(c2), t), \
		GO_COLOR_MONO_INTERPOLATE(GO_COLOR_UINT_G(c1), GO_COLOR_UINT_G(c2), t), \
		GO_COLOR_MONO_INTERPOLATE(GO_COLOR_UINT_B(c1), GO_COLOR_UINT_B(c2), t), \
		GO_COLOR_MONO_INTERPOLATE(GO_COLOR_UINT_A(c1), GO_COLOR_UINT_A(c2), t) )

#define GO_COLOR_DOUBLE_R(x) (GO_COLOR_UINT_R(x)/255.0)
#define GO_COLOR_DOUBLE_G(x) (GO_COLOR_UINT_G(x)/255.0)
#define GO_COLOR_DOUBLE_B(x) (GO_COLOR_UINT_B(x)/255.0)
#define GO_COLOR_DOUBLE_A(x) (GO_COLOR_UINT_A(x)/255.0)

#define GO_COLOR_TO_CAIRO(x) GO_COLOR_DOUBLE_R(x),GO_COLOR_DOUBLE_G(x),GO_COLOR_DOUBLE_B(x),GO_COLOR_DOUBLE_A(x)

gboolean  go_color_from_str (char const *str, GOColor *res);
gchar    *go_color_as_str   (GOColor color);
PangoAttribute *go_color_to_pango (GOColor color, gboolean is_fore);
#ifdef GOFFICE_WITH_GTK
GdkRGBA *go_color_to_gdk_rgba   (GOColor color, GdkRGBA *res);
#endif

G_END_DECLS

#endif /* GO_COLOR_H */
