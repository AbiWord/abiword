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
#include <stdio.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "xap_UnixToolbar_Icons.h"
#include "xap_UnixDialogHelper.h"

AP_UnixToolbar_Icons::AP_UnixToolbar_Icons(void)
{
}

AP_UnixToolbar_Icons::~AP_UnixToolbar_Icons(void)
{
}

bool AP_UnixToolbar_Icons::getPixmapForIcon(GdkWindow * window, GdkColor * background,
											const char * szIconName, GtkWidget ** pwPixmap)
{
	UT_return_val_if_fail(window, false);
	UT_return_val_if_fail(background, false);
	UT_return_val_if_fail(szIconName && *szIconName, false);
	UT_return_val_if_fail(pwPixmap, false);
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound || !pIconData || !sizeofIconData)
		return false;

	GdkPixbuf * pixbuf = gdk_pixbuf_new_from_xpm_data (pIconData);
	if (!pixbuf)
		return false;

	*pwPixmap = gtk_image_new_from_pixbuf ( pixbuf ) ;
	g_object_unref (G_OBJECT (pixbuf)); // remove ref - GtkImage retains a ref so we're safe
	return true;
}

