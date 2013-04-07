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
#include "ap_Win32Toolbar_StyleCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"
#include "pd_Style.h"
#include "xad_Document.h"
#include "xap_App.h"
#include "ev_Win32Toolbar.h"

/*****************************************************************/

EV_Toolbar_Control * AP_Win32Toolbar_StyleCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_Win32Toolbar_StyleCombo * p = new AP_Win32Toolbar_StyleCombo(pToolbar,id);
	return p;
}

AP_Win32Toolbar_StyleCombo::AP_Win32Toolbar_StyleCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_return_if_fail (id==AP_TOOLBAR_ID_FMT_STYLE);

	m_nPixels = 160;		// TODO: do a better calculation
	m_nLimit = 20;
	m_bSort = true;
	// If 'm_nPixels' is wide enough, the following takes care of the
	// drop-list.
	m_nDroppedWidth = m_nPixels + GetSystemMetrics(SM_CXVSCROLL);

	m_pFrame = static_cast<EV_Win32Toolbar *>(pToolbar)->getFrame();
}

AP_Win32Toolbar_StyleCombo::~AP_Win32Toolbar_StyleCombo(void)
{
//	UT_VECTOR_FREEALL(char *, m_vecContents);
}

/*****************************************************************/

bool AP_Win32Toolbar_StyleCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector

#if 1
	// HACK: for now, just hardwire it
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
	const PD_Style * pStyle;

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	pDoc->enumStyles(pStyles);
	UT_return_val_if_fail( pStyles, false );
	UT_uint32 iStyleCount = getDoc()->getStyleCount();
	
	for (UT_uint32 k=0; k < iStyleCount; k++)
	{
		pStyle = pStyles->getNthItem(k);
		UT_return_val_if_fail( pStyle );
		m_vecContents.addItem(pStyle->getName());
	}

	delete pStyles;
#endif 

	return true;
}


bool AP_Win32Toolbar_StyleCombo::repopulate(void)
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
	const PD_Style * pStyle;

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	m_pDocument->enumStyles(pStyles);
	UT_return_val_if_fail( pStyles, false );
	UT_uint32 iStyleCount = m_pDocument->getStyleCount();

	for (UT_uint32 k=0; k < iStyleCount; k++)
	{
		pStyle = pStyles->getNthItem(k);
		UT_return_val_if_fail( pStyle, false );
		m_vecContents.addItem(pStyle->getName());
	}

	delete pStyles;
	
	return true;
}



