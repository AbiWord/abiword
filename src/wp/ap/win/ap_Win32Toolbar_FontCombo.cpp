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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <windows.h>
#include <stdlib.h>

#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ap_Win32Toolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"
#include "ut_debugmsg.h"
#include "ut_Win32LocaleString.h"

/*****************************************************************/

EV_Toolbar_Control * AP_Win32Toolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_Win32Toolbar_FontCombo * p = new AP_Win32Toolbar_FontCombo(pToolbar,id);
	return p;
}

AP_Win32Toolbar_FontCombo::AP_Win32Toolbar_FontCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_return_if_fail (id==AP_TOOLBAR_ID_FMT_FONT);

	m_nPixels = 160;		// TODO: do a better calculation
	m_nLimit = LF_FACESIZE;
	m_bSort = true;
}

AP_Win32Toolbar_FontCombo::~AP_Win32Toolbar_FontCombo(void)
{
	UT_VECTOR_FREEALL(char *, m_vecContents);
	
}

/*****************************************************************/

#include <set>
static std::set<std::string> seenFonts;

bool AP_Win32Toolbar_FontCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();
	m_vecFontCharSet.clear();

	seenFonts.clear();

	// populate the vector
	HWND hwnd = NULL;
    HDC hdc = GetDC(hwnd) ;
	LOGFONTW lf;
	lf.lfCharSet=DEFAULT_CHARSET;
	*lf.lfFaceName=0;
	lf.lfPitchAndFamily=0;
    EnumFontFamiliesExW(hdc, &lf, (FONTENUMPROCW) AP_Win32Toolbar_FontCombo::_EnumFontsProc, (LONG_PTR) this, 0) ;
    ReleaseDC(hwnd, hdc) ;

	seenFonts.clear();

	return true;
}

int CALLBACK AP_Win32Toolbar_FontCombo::_EnumFontsProc(LPLOGFONTW lplf, 
						       LPTEXTMETRICW /*lptm*/,
													  DWORD dwStyle, 
													  LONG lParam)
{
	AP_Win32Toolbar_FontCombo * ctl = (AP_Win32Toolbar_FontCombo *) lParam;
	UT_return_val_if_fail (ctl, 0);

	/*
	   WARNING: any changes to this function should be closely coordinated
	   with the equivalent logic in Win32Graphics::FindFont()
	*/

	// filter out fonts we don't use
	if (dwStyle & RASTER_FONTTYPE)
		return 1 ;
#if 0
	// This is too restrictive.  Since EnumFontFamilies chooses at random
	// the character set for the chosen font family, we were missing things
	// here.  Perhaps use EnumFontFamiliesEx instead?
	if (lplf->lfCharSet != ANSI_CHARSET)
		return 1 ;
#endif

	// filter out vertical fonts which aren't supported
	if (lplf->lfFaceName[0]=='@')
		return 1;

	UT_Win32LocaleString str;
	str.fromLocale (lplf->lfFaceName);
	char * p = g_strdup((str.utf8_str().utf8_str()));

	if (seenFonts.find(p)!=seenFonts.end()) {
		FREEP(p);
		return 1;
	}

	ctl->m_vecContents.addItem(p);
	ctl->m_vecFontCharSet.addItem((void*)lplf->lfCharSet);

	seenFonts.insert(p);

	return 1;
}

UT_uint32 AP_Win32Toolbar_FontCombo::getDroppedWidth() const
{
	// TODO make better calculation of dropped width
	return getPixelWidth() + 100;
}
