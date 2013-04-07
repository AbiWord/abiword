/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2011 Hubert Figuiere
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

#include "ut_assert.h"
#include "ut_vector.h"
#include "ap_CocoaToolbar_StyleCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"
#include "pd_Style.h"
#include "xad_Document.h"
#include "xap_App.h"
#include "ev_CocoaToolbar.h"
#include "ut_debugmsg.h"
#include "xap_CocoaFrame.h"
#include "ap_CocoaFrame.h"

/*****************************************************************/

EV_Toolbar_Control * AP_CocoaToolbar_StyleCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id tlbrid)
{
	AP_CocoaToolbar_StyleCombo * p = new AP_CocoaToolbar_StyleCombo(pToolbar,tlbrid);
	return p;
}

AP_CocoaToolbar_StyleCombo::AP_CocoaToolbar_StyleCombo(EV_Toolbar * pToolbar,
													   XAP_Toolbar_Id tlbrid)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_DEBUG_ONLY_ARG(tlbrid);

	UT_ASSERT(tlbrid==AP_TOOLBAR_ID_FMT_STYLE);

	m_nPixels = 120;		// TODO: do a better calculation
	m_nLimit = 15;         // 15 styles before the scroll bar??.
	m_pFrame = static_cast<EV_CocoaToolbar *>(pToolbar)->getFrame();
}

AP_CocoaToolbar_StyleCombo::~AP_CocoaToolbar_StyleCombo(void)
{
//	UT_VECTOR_FREEALL(char *, m_vecContents);
}

/*****************************************************************/

bool AP_CocoaToolbar_StyleCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector

#if 1
	// HACK: for now, just hardwire it
	// NB if you change the case of the labels, it will stop working
	// unless you also change all the places where the style appears!
	m_vecContents.addItem("Normal");
	m_vecContents.addItem("Heading 1");
	m_vecContents.addItem("Heading 2");
	m_vecContents.addItem("Heading 3");
	m_vecContents.addItem("Plain Text");
	m_vecContents.addItem("Block Text");
#else
	// TODO: need a view/doc pointer to get this right
	// ALSO: will need to repopulate as new styles added
	// HYP:  only call this method from shared code? 

	UT_GenericVector<const PD_Styles*> pStyles = NULL;
	pDoc->enumStyles(pStyles);
	UT_uint32 nStyles = pStyles->getItemCount();

	for (UT_uint32 k = 0; k < nStyles; k++)
	{
		const PD_Style * pStyle;
		pStyle = pStyles->getNthItem(k);
		if(pStyle) {
			m_vecContents.addItem(pStyle->getName());
		}
	}
	DELETEP(pStyles);
#endif 

	return true;
}


bool AP_CocoaToolbar_StyleCombo::repopulate(void)
{
	// repopulate the vector from the current document
    // If ithere is one present

	AD_Document * pAD_Doc = m_pFrame->getCurrentDoc();
	if(!pAD_Doc)
	{
		return false;
	}

	// clear anything that's already there
	m_vecContents.clear();

	m_pDocument = static_cast<PD_Document *>(pAD_Doc);

	UT_GenericVector<PD_Style*>* pStyles = NULL;
	m_pDocument->enumStyles(pStyles);
	UT_uint32 nStyles = pStyles->getItemCount();

	for (UT_uint32 k = 0; k < nStyles; k++)
	{
		const PD_Style * pStyle;
		pStyle = pStyles->getNthItem(k);
		if(pStyle) {
			m_vecContents.addItem(pStyle->getName());
		}
	}
	DELETEP(pStyles);

	return true;
}



