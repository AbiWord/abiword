/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "ut_types.h"
#include "ut_assert.h"
#include "ap_UnixToolbar_Icons.h"

AP_UnixToolbar_Icons::AP_UnixToolbar_Icons(void)
{
}

AP_UnixToolbar_Icons::~AP_UnixToolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}

UT_Bool AP_UnixToolbar_Icons::getPixmapForIcon(GdkWindow * window, GdkColor * background,
											   const char * szIconName, GtkWidget ** pwPixmap)
{
	UT_ASSERT(window);
	UT_ASSERT(background);
	UT_ASSERT(szIconName && *szIconName);
	UT_ASSERT(pwPixmap);
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	UT_Bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return UT_FALSE;

	GdkBitmap * mask;
	GdkColormap * colormap = NULL;
	GdkPixmap * pixmap
		= gdk_pixmap_colormap_create_from_xpm_d(window,colormap,&mask,
												background, (char **)pIconData);
	if (!pixmap)
		return UT_FALSE;

	GtkWidget * wpixmap = gtk_pixmap_new(pixmap,mask);
	if (!wpixmap)
		return UT_FALSE;
	
	*pwPixmap = wpixmap;
	return UT_TRUE;
}

