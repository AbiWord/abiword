/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_vector.h"

#include "xap_App.h"
#include "xap_Frame.h"

#include "ap_QtToolbar_ZoomCombo.h"
#include "ap_Toolbar_Id.h"

/*****************************************************************/
EV_Toolbar_Control * AP_QtToolbar_ZoomCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_QtToolbar_ZoomCombo * p = new AP_QtToolbar_ZoomCombo(pToolbar,id);
	return p;
}

AP_QtToolbar_ZoomCombo::AP_QtToolbar_ZoomCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_DEBUG_ONLY_ARG(id);
	UT_ASSERT(id==AP_TOOLBAR_ID_ZOOM);

	m_nPixels = 80;		// TODO: do a better calculation
	m_nLimit = 9;
}

AP_QtToolbar_ZoomCombo::~AP_QtToolbar_ZoomCombo(void)
{
	// nothing to purge.  contents are static strings
}

/*****************************************************************/

bool AP_QtToolbar_ZoomCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector
	m_vecContents.addItem("200%");
	m_vecContents.addItem("150%");
	m_vecContents.addItem("100%");
	m_vecContents.addItem("75%");
	m_vecContents.addItem("50%");
	m_vecContents.addItem("25%");

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	m_vecContents.addItem(pSS->getValue(XAP_STRING_ID_TB_Zoom_PageWidth));
	m_vecContents.addItem(pSS->getValue(XAP_STRING_ID_TB_Zoom_WholePage));
	m_vecContents.addItem(pSS->getValue(XAP_STRING_ID_TB_Zoom_Percent));

	return true;
}

