/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz, William Lachance and others
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

#include "ut_types.h"
#include "ap_Frame.h"

#if defined(ANY_UNIX) || (defined(__APPLE__) && defined(__MACH__))
#include "ap_FrameData.h"
#include "fv_View.h"
#include "xad_Document.h"
#include "pd_Document.h"
#include "xap_ViewListener.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_StatusBar.h"

AP_Frame::~AP_Frame()
{
}

void AP_Frame::_replaceView(GR_Graphics * pG, FL_DocLayout *pDocLayout,
			    AV_View *pView, AV_ScrollObj * pScrollObj,
			    ap_ViewListener *pViewListener, AD_Document *pOldDoc,
			    ap_Scrollbar_ViewListener *pScrollbarViewListener,
			    AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener,
			    UT_uint32 iZoom)
{
	// switch to new view, cleaning up previous settings
	if (((AP_FrameData*)m_pData)->m_pDocLayout)
	{
		pOldDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
	}

	REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);
	REPLACEP(((AP_FrameData*)m_pData)->m_pDocLayout, pDocLayout);

	if (pOldDoc != m_pDoc)
	{
		UNREFP(pOldDoc);
	}

	REPLACEP(m_pView, pView);
        if(getApp()->getViewSelection())
	       getApp()->setViewSelection(pView);
	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener, pScrollbarViewListener);
	m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets we do
	// not destroy and recreate the TopRuler, LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler  here; that is done as the
	// frame is created.
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler )
	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
			((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
			((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
	}

	if ( ((AP_FrameData*)m_pData)->m_pStatusBar && (getFrameMode() != XAP_NoMenusWindowLess))
		((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);
	((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);

	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);
	m_pView->setWindowSize(_getDocumentAreaWidth(), _getDocumentAreaHeight());

	updateTitle();
	//pDocLayout->setView((FV_View *) m_pView);
	pDocLayout->fillLayouts();   
	
	_resetInsertionPoint();
}

// _resetInsertionPoint: sets the current insertion point in the document to be the end
// of the editable bound in the current view, if and only if the upper editable bound of 
// pView is less than the current insertion point. Resets it to the current position 
// otherwise. 
void AP_Frame::_resetInsertionPoint()
{
	UT_uint32 point = 0;

	if (m_pView != NULL) {
		point = (static_cast<FV_View *>(m_pView))->getPoint();
		PT_DocPosition posEOD;
		static_cast<FV_View *>(m_pView)->getEditableBounds(true, posEOD, false);
		if(point > posEOD)
			(static_cast<FV_View *>(m_pView))->moveInsPtTo(posEOD);
	}		
}


#else
AP_Frame::~AP_Frame()
{
}
#endif
