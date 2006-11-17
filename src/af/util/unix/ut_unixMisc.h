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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#ifndef UTUNIXMISC_H
#define UTUNIXMISC_H

#ifdef XP_UNIX_TARGET_GTK
#include <gdk/gdk.h>
#include <gtk/gtk.h>
typedef GtkWidget AbiNativeWidget;

#ifndef UTMISC_H
#include "ut_misc.h"
#endif

UT_RGBColor* UT_UnixGdkColorToRGBColor(const GdkColor &color);
GdkColor* UT_UnixRGBColorToGdkColor(const UT_RGBColor &rgb);
#endif

#endif /* UTUNIXMISC_H */
