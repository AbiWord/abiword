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
#include "ut_Win32OS.h"
#include "gr_Win32CharWidths.h"

//////////////////////////////////////////////////////////////////

void GR_Win32CharWidths::setCharWidthsOfRange(HDC hdc, UT_UCSChar c0, UT_UCSChar c1)
{
#if 0
	UT_uint32 loc0 = (c0 & 0xff);
	UT_uint32 loc1 = (c1 & 0xff);
	UT_uint32 hic0 = ((c0 >> 8) & 0xff);
	UT_uint32 hic1 = ((c1 >> 8) & 0xff);

	if (hic0 == hic1)					// we are contained within one page
	{
		Array256 * pA = NULL;
		if (hic0 == 0)
			pA = &m_aLatin1;
		else if (m_vecHiByte.getItemCount() > hic0)
			pA = (Array256 *)m_vecHiByte.getNthItem(hic0);
		if (pA)
		{
			if (UT_IsWinNT())
				GetCharWidth32(hdc, loc0, loc1, &(pA->aCW[loc0]));
			else
				GetCharWidth(hdc, loc0, loc1, &(pA->aCW[loc0]));
			return;
		}
	}

	// if we fall out of the above, we're either spanning
	// different pages or we are on a page that hasn't
	// be loaded yet.  do it the hard way....
#endif
	UINT k;
	int w;

	if (UT_IsWinNT())
		for (k=c0; k<=c1; k++)
		{
			GetCharWidth32(hdc,k,k,&w);
			setWidth(k,w);
		}
	else
		for (k=c0; k<=c1; k++)
		{
			GetCharWidth(hdc,k,k,&w);
			setWidth(k,w);
		}
}

