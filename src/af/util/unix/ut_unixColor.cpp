/* AbiSource Program Utilities
 * Copyright (C) 2003-2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#include "ut_unixColor.h"
#include "ut_assert.h"

#if defined(TOOLKIT_GTK) || defined(TOOLKIT_GTK2)

UT_RGBColor* UT_UnixGdkColorToRGBColor(const GdkRGBA &color)
{
	return new UT_RGBColor(color.red * 255.,
			       color.green * 255.,
			       color.blue * 255.);
}

GdkRGBA* UT_UnixRGBColorToGdkRGBA(const UT_RGBColor &rgb)
{
	GdkRGBA color;
	color.red = (gdouble)(rgb.m_red) / 255.;
	color.green = (gdouble)(rgb.m_grn) / 255.;
        color.blue = (gdouble)(rgb.m_blu) /  255.;
	color.alpha = 1.;
	return gdk_rgba_copy(&color);
}

#endif
