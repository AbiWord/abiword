/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
 
#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <string.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "xap_CocoaToolbar_Icons.h"

AP_CocoaToolbar_Icons::AP_CocoaToolbar_Icons(void)
{
}

AP_CocoaToolbar_Icons::~AP_CocoaToolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}


/*!
	returns the pixmap for the named icon
	
	\param szIconName the name of the icon
	\returnvalue pwPixmap the newly allocated NSImage
 */
bool AP_CocoaToolbar_Icons::getPixmapForIcon(const char * szIconName, NSImage ** pwPixmap)
{
	UT_ASSERT(szIconName && *szIconName);
	UT_ASSERT(pwPixmap);
	NSImage *pixmap;
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return false;

	NSData * iconData = [NSData dataWithBytes:(const void *)pIconData length:sizeofIconData];
	pixmap = [NSImage alloc];
	pixmap = [pixmap initWithData:iconData];
	[iconData release];

	UT_ASSERT (pixmap);	
	if (!pixmap)
		return false;

	*pwPixmap = pixmap;
	return true;
}

