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
#include <stdio.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_Win32Dialog_FontChooser.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"
#include "gr_Graphics.h"

/*****************************************************************/
AP_Dialog * AP_Win32Dialog_FontChooser::static_constructor(AP_DialogFactory * pFactory,
														  AP_Dialog_Id id)
{
	AP_Win32Dialog_FontChooser * p = new AP_Win32Dialog_FontChooser(pFactory,id);
	return p;
}

AP_Win32Dialog_FontChooser::AP_Win32Dialog_FontChooser(AP_DialogFactory * pDlgFactory,
													 AP_Dialog_Id id)
	: AP_Dialog_FontChooser(pDlgFactory,id)
{
}

AP_Win32Dialog_FontChooser::~AP_Win32Dialog_FontChooser(void)
{
}

/*****************************************************************/

void AP_Win32Dialog_FontChooser::runModal(AP_Frame * pFrame)
{
	m_pWin32Frame = (AP_Win32Frame *)pFrame;
	UT_ASSERT(m_pWin32Frame);
	AP_Win32App * pApp = (AP_Win32App *)m_pWin32Frame->getApp();
	UT_ASSERT(pApp);

	UT_DEBUGMSG(("FontChooserStart: Family[%s] Size[%s] Weight[%s] Style[%s] Color[%s] Underline[%d] StrikeOut[%d]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),
				 ((m_pColor) ? m_pColor : "" ),
				 (m_bUnderline),
				 (m_bStrikeOut)));
	
	/*
	   WARNING: any changes to this function should be closely coordinated
	   with the equivalent logic in Win32Graphics::FindFont()
	*/
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	
	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = m_pWin32Frame->getTopLevelWindow();
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
	cf.hInstance = pApp->getInstance();

	if (m_pFontFamily && *m_pFontFamily)
		strcpy(lf.lfFaceName,m_pFontFamily);
	else
		cf.Flags |= CF_NOFACESEL;

	if (m_pFontSize && *m_pFontSize)
	{
		UT_ASSERT(sizeof(char) == sizeof(XML_Char));
		const char * szSize = (const char *)m_pFontSize;
		lf.lfHeight = -(m_pGraphics->convertDimension(szSize));
	}
	else
		cf.Flags |= CF_NOSIZESEL;

	if (m_pFontWeight && *m_pFontWeight)
	{
		if (UT_stricmp(m_pFontWeight,"bold") == 0)
			lf.lfWeight = 700;
		// TODO do we need any others here...
	}
	else
		cf.Flags |= CF_NOSTYLESEL;

	if (m_pFontStyle && *m_pFontStyle)
	{
		if (UT_stricmp(m_pFontStyle,"italic") == 0)
			lf.lfItalic = TRUE;
	}
	else
		cf.Flags |= CF_NOSTYLESEL;

	if (m_pColor && *m_pColor)
	{
		UT_RGBColor c;
		UT_parseColor(m_pColor,c);
		cf.rgbColors = RGB(c.m_red,c.m_grn,c.m_blu);
	}
	
	if (m_bUnderline)
		lf.lfUnderline = TRUE;
	if (m_bStrikeOut)
		lf.lfStrikeOut = TRUE;

	// run the actual dialog...
	
	m_answer = (ChooseFont(&cf) ? a_OK : a_CANCEL);

	if (m_answer == a_OK)
	{
		if (   (!lf.lfFaceName && m_pFontFamily)
			|| (lf.lfFaceName && !m_pFontFamily)
			|| (UT_stricmp(lf.lfFaceName,m_pFontFamily) != 0))
		{
			m_bChangedFontFamily = UT_TRUE;
			setFontFamily(lf.lfFaceName);
		}

		UT_Bool bIsSizeValid = ((cf.Flags & CF_NOSIZESEL) == 0);
		UT_Bool bWasSizeValid = (m_pFontSize && *m_pFontSize);
		char bufSize[10];
		if (bIsSizeValid)
			sprintf(bufSize,"%dpt",(cf.iPointSize/10));
		else
			bufSize[0] = 0;
		if (   (bIsSizeValid && bWasSizeValid && (UT_stricmp(bufSize,m_pFontSize) == 0))
			|| (!bIsSizeValid && !bWasSizeValid))
		{
			/* nothing changed */
		}
		else
		{
			m_bChangedFontSize = UT_TRUE;
			setFontSize(bufSize);
		}

		UT_Bool bIsBold = ((cf.nFontType & BOLD_FONTTYPE) != 0);
		UT_Bool bWasBold = (m_pFontWeight && *m_pFontWeight && (UT_stricmp(m_pFontWeight,"bold") == 0));
		UT_Bool bIsNormal = ((cf.nFontType & REGULAR_FONTTYPE) != 0);
		UT_Bool bWasNormal = (!m_pFontWeight
							  || !*m_pFontWeight
							  || (UT_stricmp(m_pFontWeight,"normal") != 0));
		if ((bIsBold != bWasBold) || (bIsNormal != bWasNormal))
		{
			m_bChangedFontWeight = UT_TRUE;
			setFontWeight((bIsBold) ? "bold" : "normal");
		}

		UT_Bool bIsItalic = ((cf.nFontType & ITALIC_FONTTYPE) != 0);
		UT_Bool bWasItalic = (m_pFontStyle && *m_pFontStyle && (UT_stricmp(m_pFontStyle,"italic") == 0));
		if (bIsItalic != bWasItalic)
		{
			m_bChangedFontStyle = UT_TRUE;
			setFontStyle((bIsItalic) ? "italic" : "normal");
		}

		char bufColor[10];
		sprintf(bufColor,"%02x%02x%02x",GetRValue(cf.rgbColors),
				GetGValue(cf.rgbColors),GetBValue(cf.rgbColors));
		UT_Bool bWasColorValid = (m_pColor && *m_pColor);
		if (   (bWasColorValid && (UT_stricmp(bufColor,m_pColor) != 0))
			|| (!bWasColorValid && (UT_stricmp(bufColor,"000000") != 0)))
		{
			m_bChangedColor = UT_TRUE;
			setColor(bufColor);
		}

		m_bChangedUnderline = (lf.lfUnderline != m_bUnderline);
		m_bChangedStrikeOut = (lf.lfStrikeOut != m_bStrikeOut);
		if (m_bChangedUnderline || m_bChangedStrikeOut)
			setFontDecoration(lf.lfUnderline,lf.lfStrikeOut);
	}
	
	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((m_pColor) ? m_pColor : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeOut),							((m_bChangedStrikeOut) ? "(chg)" : "")));
	
	// the caller can get the answer from getAnswer().

	m_pWin32Frame = NULL;
}

