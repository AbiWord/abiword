/* AbiWord
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
#include <stdlib.h>

#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ap_Win32Toolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "ap_Frame.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

/*****************************************************************/

EV_Toolbar_Control * AP_Win32Toolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
														  AP_Toolbar_Id id)
{
	AP_Win32Toolbar_FontCombo * p = new AP_Win32Toolbar_FontCombo(pToolbar,id);
	return p;
}

AP_Win32Toolbar_FontCombo::AP_Win32Toolbar_FontCombo(EV_Toolbar * pToolbar,
													 AP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_FONT);

	m_nPixels = 130;		// TODO: do a better calculation
	m_nLimit = LF_FACESIZE;
	m_bSort = UT_TRUE;
}

AP_Win32Toolbar_FontCombo::~AP_Win32Toolbar_FontCombo(void)
{
	int max = m_vecContents.getItemCount();
	for (int i=max-1; i>=0; i--)
	{
		char * p = (char *) m_vecContents.getNthItem(i);
		UT_ASSERT(p);
		FREEP(p);
	}
}

/*****************************************************************/

UT_Bool AP_Win32Toolbar_FontCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector
	HWND hwnd = NULL;
    HDC hdc = GetDC(hwnd) ;
    EnumFontFamilies(hdc, (LPTSTR) NULL, (FONTENUMPROC) AP_Win32Toolbar_FontCombo::_EnumFontsProc, (LONG) this) ;
    ReleaseDC(hwnd, hdc) ;

	return UT_TRUE;
}

int CALLBACK AP_Win32Toolbar_FontCombo::_EnumFontsProc(LPLOGFONT lplf, 
													  LPTEXTMETRIC lptm,
													  DWORD dwStyle, 
													  LONG lParam)
{
	AP_Win32Toolbar_FontCombo * ctl = (AP_Win32Toolbar_FontCombo *) lParam;
	UT_ASSERT(ctl);

	/*
	   WARNING: any changes to this function should be closely coordinated
	   with the equivalent logic in Win32Graphics::FindFont()
	*/

	// filter out fonts we don't use
	if (dwStyle & RASTER_FONTTYPE)
		return 1 ;

	if (lplf->lfCharSet != ANSI_CHARSET)
		return 1 ;

	char * p;
	UT_cloneString(p, lplf->lfFaceName);

	ctl->m_vecContents.addItem(p);

	return 1;
}
