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
#include "ut_types.h"
#include "ut_assert.h"
#include "xap_UnixToolbar_Icons.h"

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
	
	const char ** pIconData = NULL, **used_pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	UT_Bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return UT_FALSE;

	GdkBitmap * mask;
	GdkColormap * colormap = NULL;
	{
	    /*if this is BW icon, use the current theme's text foreground 
	      color instead of black (since background on which the icon
	     is painted can be very dark so that default black icon won't be
             visible at all).
		 Vlad Harchev <hvv@hippo.ru>*/
	    int w,h,nc;
	    if (sscanf(pIconData[0],"%d %d %d",&h,&w,&nc)==3 && (nc == 2 || nc == 3) && 
		    !strcmp(pIconData[2],".	c #000000")) {
		/*it's BW image and 2nd color is black - substitute it.*/
		used_pIconData = (const char**)malloc(sizeof(char*)*(h+nc+1));
		if (!used_pIconData)
		    goto done; /* let's it crash somewhere else */
		memcpy(used_pIconData,pIconData,sizeof(char*)*(h+nc+1));
		
		static GtkWidget* label = NULL;
		static char buf[50];
		if (!label) {
		    label = gtk_label_new("");
		    gtk_widget_ensure_style(label);

		    GdkColor* c = label->style->text + GTK_STATE_NORMAL;
		    sprintf(buf,".\tc #%02x%02x%02x",(unsigned int)c->red>>8,(unsigned int) c->green>>8,(unsigned int) c->blue>>8);
		};
		used_pIconData[2] = buf;
	    };
	    done: ;
	}
	GdkPixmap * pixmap
		= gdk_pixmap_colormap_create_from_xpm_d(window,colormap,&mask,
												background, (char **) (used_pIconData ? used_pIconData : pIconData));
	if (used_pIconData)
	    free(used_pIconData);
	
	    
	if (!pixmap)
		return UT_FALSE;

	GtkWidget * wpixmap = gtk_pixmap_new(pixmap,mask);
	if (!wpixmap)
		return UT_FALSE;
	
	*pwPixmap = wpixmap;
	return UT_TRUE;
}

