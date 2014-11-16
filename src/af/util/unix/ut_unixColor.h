/* AbiSource Program Utilities
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
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



#ifndef UTUNIXCOLOR_H
#define UTUNIXCOLOR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TOOLKIT_GTK_ALL
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifndef UTCOLOR_H
#include "ut_color.h"
#endif

UT_RGBColor* UT_UnixGdkColorToRGBColor(const GdkRGBA &color);
GdkRGBA* UT_UnixRGBColorToGdkRGBA(const UT_RGBColor &rgb);

#endif

#endif /* UTUNIXCOLOR_H */
