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

#include <windows.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_Xpm2Bmp.h"
#include "xap_Win32Toolbar_Icons.h"

AP_Win32Toolbar_Icons::AP_Win32Toolbar_Icons(void)
{
}

AP_Win32Toolbar_Icons::~AP_Win32Toolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}

bool AP_Win32Toolbar_Icons::getBitmapForIcon(HWND hwnd,
												UT_uint32 maxWidth,
												UT_uint32 maxHeight,
												UT_RGBColor * pColor,
												const char * szIconName,
												HBITMAP * pBitmap)
{
	UT_ASSERT(hwnd);
	UT_ASSERT(szIconName && *szIconName);
	UT_ASSERT(pBitmap);
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return false;

	HDC hdc = GetDC(hwnd);
	bool bCreated = UT_Xpm2Bmp(maxWidth,maxHeight,pIconData,sizeofIconData,hdc,pColor,pBitmap);
	ReleaseDC(hwnd,hdc);

	return bCreated;
}

	
