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
#include "xap_QNXToolbar_Icons.h"
#include "ut_Xpm2Bitmap.h"
#include <stdio.h>

AP_QNXToolbar_Icons::AP_QNXToolbar_Icons(void)
{
}

AP_QNXToolbar_Icons::~AP_QNXToolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}

PhImage_t *AP_QNXToolbar_Icons::getPixmapForIcon(const char * szIconName) 
{
	PhImage_t *outimage = NULL;

	if (!szIconName || !*szIconName)
		return UT_FALSE;
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	//printf("Looking for icon [%s] \n", szIconName);
	UT_Bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return UT_FALSE;

	if (!UT_Xpm2Bitmap(pIconData, sizeofIconData, &outimage)) 
		return UT_FALSE;

	return outimage;
}

