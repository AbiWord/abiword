/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 

#include "ap_FrameData.h"
#include "gr_Graphics.h"
#include "fl_DocLayout.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_StatusBar.h"
#include "xap_App.h"

/*****************************************************************/

AP_FrameData::AP_FrameData(XAP_App * pApp)
{
	m_pDocLayout = NULL;
	m_pG = NULL;
	m_pTopRuler = NULL;
	m_pLeftRuler = NULL;
	m_pStatusBar = NULL;

	m_bShowRuler = true;
	m_bShowBar[0] = true;
	m_bShowBar[1] = true;
	m_bShowBar[2] = true;
    m_bShowPara = true;
	m_bInsertMode = true;
	m_bShowStatusBar = true;

	if (pApp)
	{
		bool b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_InsertMode, &b))
			m_bInsertMode = b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_RulerVisible, &b))
			m_bShowRuler = b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_StandardBarVisible, &b))
			m_bShowBar[0] = b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_FormatBarVisible, &b))
			m_bShowBar[1] = b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_ExtraBarVisible, &b))
			m_bShowBar[2] = b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_StatusBarVisible, &b))
			m_bShowStatusBar = b;

		if (pApp->getPrefsValueBool(AP_PREF_KEY_ParaVisible, &b))
			m_bShowPara = b;
	}
}

AP_FrameData::~AP_FrameData()
{
	DELETEP(m_pDocLayout);
	DELETEP(m_pG);
	DELETEP(m_pTopRuler);
	DELETEP(m_pLeftRuler);
	DELETEP(m_pStatusBar);
}
