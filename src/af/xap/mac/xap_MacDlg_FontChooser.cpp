/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_MacDlg_FontChooser.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"
#include "gr_Graphics.h"

/*****************************************************************/
AP_Dialog * AP_MacDialog_FontChooser::static_constructor(AP_DialogFactory * pFactory,
														  AP_Dialog_Id id)
{
	AP_MacDialog_FontChooser * p = new AP_MacDialog_FontChooser(pFactory,id);
	return p;
}

AP_MacDialog_FontChooser::AP_MacDialog_FontChooser(AP_DialogFactory * pDlgFactory,
													 AP_Dialog_Id id)
	: AP_Dialog_FontChooser(pDlgFactory,id)
{
}

AP_MacDialog_FontChooser::~AP_MacDialog_FontChooser(void)
{
}

/*****************************************************************/

void AP_MacDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	m_pMacFrame = (XAP_MacFrame *)pFrame;
	UT_ASSERT(m_pMacFrame);
	XAP_MacApp * pApp = (XAP_MacApp *)m_pMacFrame->getApp();
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
	   with the equivalent logic in MacGraphics::FindFont()
	*/
	m_pMacFrame = NULL;
}

