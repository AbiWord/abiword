/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Background.h"
#include "ap_Win32Dialog_Background.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Background::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Background * p = new AP_Win32Dialog_Background(pFactory,id);
	return p;
}

AP_Win32Dialog_Background::AP_Win32Dialog_Background(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Background(pDlgFactory,id)
{
}

AP_Win32Dialog_Background::~AP_Win32Dialog_Background(void)
{
}

void AP_Win32Dialog_Background::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	const XML_Char *  pszC = getColor();
	UT_RGBColor rgbColor(255,255,255);
	if(strcmp(pszC,"transparent") != 0)
	{
		UT_parseColor(pszC,rgbColor);
	}


	CHOOSECOLOR cc;                 // common dialog box structure 
	static COLORREF acrCustClr[16]; // array of custom colors 
	DWORD rgbCurrent;				// initial color selection

	rgbCurrent = RGB( rgbColor.m_red, rgbColor.m_grn, rgbColor.m_blu );

	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = pWin32Frame->getTopLevelWindow();
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
 
	if( ChooseColor(&cc) )
	{
		rgbCurrent = cc.rgbResult;

		UT_setColor( rgbColor, GetRValue(rgbCurrent), GetGValue(rgbCurrent), GetBValue(rgbCurrent) );
		setColor( rgbColor );

		setAnswer( a_OK );
	}
	else
		setAnswer( a_CANCEL );
}

