/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ap_LeftRuler.h"
#include "xap_App.h"
#include "xav_View.h"
#include "gr_Graphics.h"
#include "ap_FrameData.h"
#include "ap_StatusBar.h"
#include "ap_Strings.h"
#include "ap_TopRuler.h"
#include "xap_Frame.h"
#include "ap_Ruler.h"
#include "ap_Prefs.h"
#include "fv_View.h"
#include "fp_TableContainer.h"
#include "fp_FrameContainer.h"
#include "pd_Document.h"

#include "pp_AttrProp.h"
#include "gr_Painter.h"

/*****************************************************************/

AP_LeftRuler::AP_LeftRuler(XAP_Frame * pFrame)
	: XAP_CustomWidgetLU()
#if XAP_DONTUSE_XOR
	, m_guideCache(NULL)
#endif
{
	m_pFrame = pFrame;
	m_pView = NULL;
	m_pScrollObj = NULL;
	m_pG = NULL;
	m_iHeight = 0;
	m_iWidth = 0;
	m_oldY = 0;
	m_yScrollOffset = 0;
	m_yScrollLimit = 0;
	m_bValidMouseClick = false;
	m_draggingWhat = DW_NOTHING;

	m_bGuide = false;
	m_yGuide = 0;
	
	const gchar * szRulerUnits;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;

	// i wanted these to be "static const x = 32;" in the
	// class declaration, but MSVC5 can't handle it....
	// (GCC can :-)
	// These are in DEVICE units!!!
	
	s_iFixedHeight = 32;
	s_iFixedWidth = 32;
	m_lfi = NULL;
	m_draggingDocPos = 0;
	m_bIsHidden = false;
	// install top_ruler_prefs_listener as this lister for this func
	XAP_App::getApp()->getPrefs()->addListener( AP_LeftRuler::_prefsListener, static_cast<void *>(this) );
	m_lidLeftRuler = 9999999;
	UT_DEBUGMSG(("Created LeftRuler %p lid is %d \n",this,m_lidLeftRuler));
}

AP_LeftRuler::~AP_LeftRuler(void)
{
	if(m_pView) 
	{
		// don't receive anymore scroll messages
	  UT_DEBUGMSG(("Remove scroll listener %p \n",m_pScrollObj));
		m_pView->removeScrollListener(m_pScrollObj);

		// no more view messages
		if(m_lidLeftRuler != 9999999)
		{
			UT_ASSERT(m_lidLeftRuler != 0);
			m_pView->removeListener(m_lidLeftRuler);
		}
		static_cast<FV_View *>(m_pView)->setLeftRuler(NULL);
		m_pView = NULL;
	}
	// no more prefs 
	XAP_App::getApp()->getPrefs()->removeListener( AP_LeftRuler::_prefsListener, static_cast<void *>(this) );
	UT_DEBUGMSG(("Deleted LeftRuler %p \n",this));
	m_lidLeftRuler = 0;
	UT_DEBUGMSG(("AP_LeftRuler::~AP_LeftRuler (this=%p scroll=%p)\n", this, m_pScrollObj));

	DELETEP(m_pScrollObj);
	DELETEP(m_lfi);
}

/*****************************************************************/

void AP_LeftRuler::setView(AV_View* pView, UT_uint32 iZoom)
{
	this->setView(pView);

	UT_ASSERT(m_pG);
	m_pG->setZoomPercentage(iZoom);

    // TODO this dimension shouldn't be hard coded.
	// in fact, it shouldn't need to be recomputed at all anymore.
	m_minPageLength = UT_convertToLogicalUnits("0.5in");
	static_cast<FV_View *>(pView)->setLeftRuler(this);
}

void AP_LeftRuler::setZoom(UT_uint32 iZoom)
{
        m_pG->clearFont();
	m_pG->setZoomPercentage(iZoom);
    // TODO this dimension shouldn't be hard coded.
	// in fact, it shouldn't need to be recomputed at all anymore.
	m_minPageLength = UT_convertToLogicalUnits("0.5in");

}

void AP_LeftRuler::setView(AV_View * pView)
{
	if (m_pView && (m_pView != pView))
	{
		// view is changing.  since this LeftRuler class is bound to
		// the actual on-screen widgets, we reuse it as documents
		// change in the frame rather than recreating it with each
		// view (as we do with some of the other objects).
		DELETEP(m_pScrollObj);
		if(m_lidLeftRuler !=  9999999)
		{
			m_pView->removeListener(m_lidLeftRuler);
		}
	}
	
	m_pView = pView;
	
	// create an AV_ScrollObj to receive send*ScrollEvents()
	if (m_pScrollObj == NULL) 
	{
		m_pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
		m_pView->addScrollListener(m_pScrollObj);

	// Register the LeftRuler as a ViewListeners on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

		m_pView->addListener(static_cast<AV_Listener *>(this),&m_lidLeftRuler);
	}
	UT_ASSERT(m_pScrollObj);

	return;
}

void AP_LeftRuler::setViewHidden(AV_View * pView)
{
	m_pView = pView;
	m_bIsHidden = true;
}

void AP_LeftRuler::_refreshView(void)
{
	if(m_pView != NULL)
		setView(m_pView);
}

/*! parameter in device units */
void AP_LeftRuler::setHeight(UT_uint32 iHeight)
{
	m_iHeight = iHeight;
}

/*! return value in logical units */
UT_uint32 AP_LeftRuler::getHeight(void) const
{
	if (m_pG == NULL) {
		return 0;
	}
	return m_pG->tlu(m_iHeight);
}

/*! parameter in device units */
void AP_LeftRuler::setWidth(UT_uint32 iWidth)
{
	if (m_iWidth != iWidth)
	{
		m_iWidth = iWidth;
		AP_FrameData * pFrameData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());
		if (pFrameData && pFrameData->m_pTopRuler)
			pFrameData->m_pTopRuler->setOffsetLeftRuler(iWidth);
	}
}

/*! return value in logical units */
UT_uint32 AP_LeftRuler::getWidth(void) const
{
	if (m_pG == NULL) {
		return 0;
	}
	return m_pG->tlu(m_iWidth);
}

/*****************************************************************/

void AP_LeftRuler::mousePress(EV_EditModifierState /* ems */, EV_EditMouseButton /* emb */, UT_uint32 x, UT_uint32 y)
{
	// by the way, we expect x, y in layout units

	// get the complete state of what should be on the ruler at the
	// time of the grab.  we assume that nothing in the document can
	// change during our grab unless we change it.

	if(m_pView == NULL || m_pView->getPoint() == 0)
		return;
	FV_View * pView = static_cast<FV_View *>(m_pView);
    if(pView->getDocument()->isPieceTableChanging())
	{
		return;
	}
	xxx_UT_DEBUGMSG(("LeftRuler: In mouse press x %d y %d\n",x,y));
	m_bValidMouseClick = false;
	m_draggingWhat = DW_NOTHING;
	m_bEventIgnored = false;

	GR_Graphics * pG = pView->getGraphics();
	pView->getLeftRulerInfo(&m_infoCache);
	UT_ASSERT(m_infoCache.m_yTopMargin >= 0);
	UT_sint32 yAbsTop = m_infoCache.m_yPageStart - m_yScrollOffset;
    UT_sint32 yrel = static_cast<UT_sint32>(y) - yAbsTop;
    ap_RulerTicks tick(pG,m_dim);
    UT_sint32 ygrid = tick.snapPixelToGrid(yrel);
    m_draggingCenter = yAbsTop + ygrid;
	xxx_UT_DEBUGMSG(("MousePress: draggingCenter %d \n",m_draggingCenter));
	m_oldY = ygrid; // used to determine if delta is zero on a mouse release

  	// only check page margins
	
  	UT_Rect rTopMargin, rBottomMargin;
  	_getMarginMarkerRects(&m_infoCache,rTopMargin,rBottomMargin);
	rTopMargin.width = getWidth();
	rBottomMargin.width = getWidth();
 	if (rTopMargin.containsPoint(x,y))
 	{
 		m_bValidMouseClick = true;
 		m_draggingWhat = DW_TOPMARGIN;
 		m_bBeforeFirstMotion = true;
		if(m_pG)
		{
			m_pG->setCursor(GR_Graphics::GR_CURSOR_GRAB);
		}
 		return;
 	}

 	if (rBottomMargin.containsPoint(x,y))
 	{

 		m_bValidMouseClick = true;
 		m_draggingWhat = DW_BOTTOMMARGIN;
 		m_bBeforeFirstMotion = true;
		if(m_pG)
		{
			m_pG->setCursor(GR_Graphics::GR_CURSOR_GRAB);
		}
 		return;
 	}
	if (m_infoCache.m_mode ==  AP_LeftRulerInfo::TRI_MODE_TABLE)
	{
		UT_sint32 i = 0;
		bool bFound = false;
		for(i=0; (i<= m_infoCache.m_iNumRows) && !bFound; i++)
		{
			UT_Rect rCell;
			_getCellMarkerRects(&m_infoCache, i,rCell);
			if(rCell.containsPoint(x,y))
			{
				m_bValidMouseClick = true;
				m_draggingWhat = DW_CELLMARK;
				m_bBeforeFirstMotion = true;
				m_draggingCell = i;
				if(m_pG)
				{
					m_pG->setCursor(GR_Graphics::GR_CURSOR_GRAB);
				}
				xxx_UT_DEBUGMSG(("Grabbed Cell %d \n",i));
				return;
			}
		}
	}
}

/*****************************************************************/

void AP_LeftRuler::mouseRelease(EV_EditModifierState /*ems*/, EV_EditMouseButton /*emb*/, UT_sint32 x, UT_sint32 y)
{
	if(m_pView == NULL)
	{
		return;
	}
	if(m_pView->getPoint() == 0)
	{
		return;
	}
	FV_View * pView1 = static_cast<FV_View *>(m_pView);
	GR_Graphics * pG = pView1->getGraphics();
	if(pG == NULL)
	{
		return;
	}
	if(pView1->getDocument() == NULL)
	{
		return;
	}
	if(pView1->getDocument()->isPieceTableChanging())
	{
		return;
	}

	if (!m_bValidMouseClick || m_bEventIgnored)
	{
		m_draggingWhat = DW_NOTHING;
		m_bValidMouseClick = false;
		if(m_pG)
		{
			m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
		}
		return;
	}
	m_bValidMouseClick = false;

	// if they drag horizontally off the ruler, we ignore the whole thing.

	if ((x < 0) || (x > static_cast<UT_sint32>(getWidth())))
	{
		_ignoreEvent(true);
		m_draggingWhat = DW_NOTHING;
		if(m_pG)
		{
			m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
		}
		return;
	}

	// mouse up was in the ruler portion of the window, or horizontally
	// off - we cannot ignore it.
	// i'd like to assert that we can just use the data computed in the
	// last mouseMotion() since the Release must be at the same spot or
	// we'd have received another Motion before the release.  therefore,
	// we use the last value of m_draggingCenter that we computed.

	// also, we do not do any drawing here.  we assume that whatever change
	// that we make to the document will cause a notify event to come back
	// to us and cause a full draw.
	
    // UT_DEBUGMSG(("mouseRelease: [ems 0x%08lx][emb 0x%08lx][x %ld][y %ld]\n",ems,emb,x,y));

	ap_RulerTicks tick(pG,m_dim);
	UT_sint32 yAbsTop = m_infoCache.m_yPageStart - m_yScrollOffset;
	UT_sint32 ygrid = tick.snapPixelToGrid(static_cast<UT_sint32>(y)-yAbsTop);
	
	_xorGuide (true);
	
	if (ygrid == m_oldY) // Not moved - clicked and released
	{
		m_draggingWhat = DW_NOTHING;
		if(m_pG)
		{
			m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
		}
		return;
	}

	bool hdrftr = pView1->isHdrFtrEdit();

	fl_HdrFtrShadow * pShadow = pView1->getEditShadow();

	bool hdr = (hdrftr && 
				pShadow->getHdrFtrSectionLayout()->getHFType() < FL_HDRFTR_FOOTER);
	HdrFtrType hfType = FL_HDRFTR_HEADER;
	if(pShadow)
	{
		hfType = pShadow->getHdrFtrSectionLayout()->getHFType();
	}
    PT_DocPosition insPos = pView1->getPoint();
	UT_sint32 iPage = -1;
	fl_DocSectionLayout * pDSL = NULL;
	if(pShadow)
	{
		pDSL = pShadow->getHdrFtrSectionLayout()->getDocSectionLayout();
		iPage = pDSL->getDocLayout()->findPage(pShadow->getPage());
	}

	double dyrel = 0.0;
	UT_sint32 yOrigin = m_infoCache.m_yPageStart + 
		m_infoCache.m_yTopMargin - m_yScrollOffset;
	UT_sint32 yEnd = yOrigin - m_infoCache.m_yTopMargin + 
		m_infoCache.m_yPageSize;
	
	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_TOPMARGIN:
		{
			UT_DEBUGMSG(("Release TOP Margin 1 \n"));
			UT_String sHeights;
			if(!(m_infoCache.m_mode == AP_LeftRulerInfo::TRI_MODE_FRAME) && !pView1->isInFrame(pView1->getPoint()))
			{
				dyrel = tick.scalePixelDistanceToUnits(m_draggingCenter - yAbsTop);
				UT_DEBUGMSG(("Release TOP Margin 1 Column \n"));

				PP_PropertyVector properties(2);
				if (!hdrftr)
					properties[0] = "page-margin-top";
				else
				{
					if (hdr)
						properties[0] = "page-margin-header";
					else
					{
						properties[0] = "page-margin-bottom";

						dyrel = tick.scalePixelDistanceToUnits(yEnd - m_draggingCenter);
					}
				}
				sHeights = pG->invertDimension(tick.dimType,dyrel);
				properties[1] = sHeights.c_str();
				pView1->setSectionFormat(properties);
			}
			else
			{
				if(m_pView == NULL)
				{
					return;
				}
				FV_View * pView = static_cast<FV_View *>(m_pView);
				fl_FrameLayout * pFrame = pView->getFrameLayout();
				if(pFrame)
				{
					const PP_AttrProp* pSectionAP = NULL;
					pFrame->getAP(pSectionAP);
					const gchar * pszYpos = NULL;
					UT_sint32 iYpos;
					const gchar * pszHeight = NULL;
					UT_sint32 iHeight;
					if(!pSectionAP || !pSectionAP->getProperty("ypos",pszYpos))
					{
						UT_DEBUGMSG(("No ypos defined for Frame !\n"));
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						return;
					}
					else
					{
						iYpos = UT_convertToLogicalUnits(pszYpos);
					}
					if(!pSectionAP || !pSectionAP->getProperty("frame-height",pszHeight))
					{
						UT_DEBUGMSG(("No Height defined for Frame !\n"));
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						return;
					}
					else
					{
						iHeight = UT_convertToLogicalUnits(pszHeight);
					}
					UT_sint32 diff = ygrid - m_oldY;
					iHeight -= diff;
					iYpos += diff;
					if(iHeight < 0)
					{
						iHeight = -iHeight;
					}
					UT_String sYpos("");
					double dYpos = static_cast<double>(iYpos)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					sYpos = UT_formatDimensionedValue(dYpos,"in", NULL);
					UT_String sHeight("");
					double dHeight = static_cast<double>(iHeight)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					sHeight = UT_formatDimensionedValue(dHeight,"in", NULL);
					const PP_PropertyVector props = {
						"ypos", sYpos.c_str(),
						"frame-height",sHeight.c_str()
					};
					pView->setFrameFormat(props);
				}
				else
				{
					return;
				}
			}
			_xorGuide(true);
			m_draggingWhat = DW_NOTHING;
			notify(pView1, AV_CHG_HDRFTR);
			pView1->setPoint(insPos);
			pView1->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR );
			pView1->setPoint(insPos);
			pView1->ensureInsertionPointOnScreen();
//
// Put the point at the right point in the header/footer on the right page.
//
			if(iPage >= 0)
			{
				fp_Page * pPage = pDSL->getDocLayout()->getNthPage(iPage);
				if(pPage)
				{
					fp_ShadowContainer* pShadowC = pPage->getHdrFtrP(hfType);
					UT_return_if_fail(pShadowC);
					pShadow = pShadowC->getShadow();
					UT_return_if_fail(pShadow);
					pView1->setHdrFtrEdit(pShadow);
				}
				pView1->setPoint(insPos);
				pView1->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR );
				pView1->setPoint(insPos);
				pView1->ensureInsertionPointOnScreen();
			}

			return;
		}
		return;

	case DW_BOTTOMMARGIN:
		{
			if(!(m_infoCache.m_mode == AP_LeftRulerInfo::TRI_MODE_FRAME) && !pView1->isInFrame(pView1->getPoint()))
			{

				PP_PropertyVector properties(2);
				dyrel = tick.scalePixelDistanceToUnits(yEnd - m_draggingCenter);
				if (!hdrftr)
					properties[0] = "page-margin-bottom";
				else
				{
					if (hdr)
					{
						properties[0] = "page-margin-top";
						dyrel = tick.scalePixelDistanceToUnits
							(m_draggingCenter - yAbsTop);
					}
					else
					{
						properties[0] = "page-margin-footer";
					}
				}
				UT_String sHeights = pG->invertDimension(tick.dimType,dyrel);
				properties[1] = sHeights.c_str();
				pView1->setSectionFormat(properties);
			}
			else
			{
				if(m_pView == NULL)
				{
					return;
				}
				FV_View * pView = static_cast<FV_View *>(m_pView);
				fl_FrameLayout * pFrame = pView->getFrameLayout();
				if(pFrame)
				{
					const PP_AttrProp* pSectionAP = NULL;
					pFrame->getAP(pSectionAP);
					const gchar * pszHeight = NULL;
					UT_sint32 iHeight;
					if(!pSectionAP || !pSectionAP->getProperty("frame-height",pszHeight))
					{
						UT_DEBUGMSG(("No Height defined for Frame !\n"));
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						return;
					}
					else
					{
						iHeight = UT_convertToLogicalUnits(pszHeight);
					}
					UT_sint32 diff = ygrid - m_oldY;
					iHeight += diff;
					if(iHeight < 0)
					{
						iHeight = -iHeight;
					}
					UT_String sHeight("");
					double dHeight = static_cast<double>(iHeight)/static_cast<double>(UT_LAYOUT_RESOLUTION);
					sHeight = UT_formatDimensionedValue(dHeight,"in", NULL);
					const PP_PropertyVector props = {
						"frame-height",sHeight.c_str()
					};
					pView->setFrameFormat(props);
				}
				else
				{
					return;
				}
			}
			_xorGuide(true);
			m_draggingWhat = DW_NOTHING;
			notify(pView1, AV_CHG_HDRFTR);
			pView1->setPoint(insPos);
			pView1->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR );
			pView1->setPoint(insPos);
			pView1->ensureInsertionPointOnScreen();
//
// Put the point at the right point in the header/footer on the right page.
//
			if(iPage >= 0)
			{
				fp_Page * pPage = pDSL->getDocLayout()->getNthPage(iPage);
				if(pPage)
				{
					fp_ShadowContainer* pShadowC = pPage->getHdrFtrP(hfType);
					pShadow = pShadowC->getShadow();
					pView1->setHdrFtrEdit(pShadow);
				}
				pView1->setPoint(insPos);
				pView1->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR );
				pView1->setPoint(insPos);
				pView1->ensureInsertionPointOnScreen();
			}
			return;
		}
		return;
	case DW_CELLMARK:
	    {
			xxx_UT_DEBUGMSG(("Handling CellMark \n"));
/*
   These will be heights applied to all rows.
 
   The format of the string of Heights is:

   table-row-heights:1.2in/3.0in/1.3in/;

   So we read back in pszRowHeights
   1.2in/3.0in/1.3in/

   The "/" characters will be used to delineate different row entries.
   if there are not enough heights defined for the entire table then the 
   rows after the last defined height do not a fixed height.
*/
//
// OK, strategy is to read in all the current row heights and only change
// The height of the cell before the current marker unless the marker is the 
// zeroth in which case we change first cell too..
//
// So new cell i height = mark of (i) - mark of (i-1)
//
			if(m_infoCache.m_vecTableRowInfo == NULL)
			{
				pView1->getLeftRulerInfo(m_draggingDocPos,&m_infoCache);
				UT_ASSERT(m_infoCache.m_yTopMargin >= 0);

				if(m_infoCache.m_vecTableRowInfo == NULL)
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					m_draggingDocPos =0;
					return;
				}
			}
			UT_String sHeights;
			double dHeight  =0;			
			double dNewHeight  =0;
			UT_sint32 iHeight = 0;
			UT_sint32 posPrev =0;
			UT_Rect rCell;
			if(m_draggingCell == 0)
			{
				posPrev = m_draggingCenter -2;
				_getCellMarkerRects(&m_infoCache, 0,rCell);
				iHeight = rCell.top + rCell.height - posPrev;
				dNewHeight  = tick.scalePixelDistanceToUnits(iHeight);
			}
			else
			{
				_getCellMarkerRects(&m_infoCache, m_draggingCell-1,rCell);
				posPrev = rCell.top + 2;
				iHeight = m_draggingCenter - posPrev;
				dNewHeight  = tick.scalePixelDistanceToUnits(iHeight);
			}
//
// Fill the heights string Now
//
			UT_sint32 i =0;
			xxx_UT_DEBUGMSG(("Cell height set to %f for row %d  number item %d \n",dNewHeight,m_draggingCell,m_infoCache.m_vecTableRowInfo->getItemCount()));
			const AP_LeftRulerTableInfo * pTInfo =  NULL;
			pTInfo = m_infoCache.m_vecTableRowInfo->getNthItem(0);
			fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pTInfo->m_pCell->getContainer());
			fp_CellContainer * pCell = NULL;
			posPrev =pTab->getYOfRow(0);
			for(i=1;i<=m_infoCache.m_vecTableRowInfo->getItemCount();i++)
			{
				bool bLast = (m_infoCache.m_vecTableRowInfo->getItemCount() == i);
				UT_sint32 iCurPos = 0;
				if(!bLast)
				{
					pTInfo = m_infoCache.m_vecTableRowInfo->getNthItem(i);
					pCell = pTInfo->m_pCell;
					iCurPos = pTab->getYOfRow(pCell->getTopAttach());
				}
				else
				{
					pTInfo = m_infoCache.m_vecTableRowInfo->getNthItem(i-1);
					pCell = pTInfo->m_pCell;
					iCurPos = pTab->getYOfRow(pCell->getBottomAttach());
				}
				if(((m_draggingCell == 0) || (m_draggingCell == 1)) && i==1)
				{
					sHeights += pG->invertDimension(tick.dimType,dNewHeight);
					sHeights += "/";
					posPrev = iCurPos;
					xxx_UT_DEBUGMSG(("new cell (2) height is %f set at i = %d \n",dNewHeight,i));
				}
				else if(m_draggingCell == static_cast<UT_sint32>(i) )
				{
					sHeights += pG->invertDimension(tick.dimType,dNewHeight);
					sHeights += "/";
					posPrev = iCurPos;
					xxx_UT_DEBUGMSG(("new cell (3) height is %f set at i = %d \n",dNewHeight,i));
				}
				else
				{
					iHeight = iCurPos - posPrev;
					posPrev = iCurPos;
					dHeight  = tick.scalePixelDistanceToUnits(iHeight);
					sHeights += pG->invertDimension(tick.dimType,dHeight);
					sHeights += "/";
				}
			}
			xxx_UT_DEBUGMSG(("cell marker string is %s \n",sHeights.c_str()));
			const PP_PropertyVector props = {
				"table-row-heights", sHeights.c_str()
			};
			if(!pView1->getDragTableLine())
			{
				pView1->setTableFormat(props);
			}
			else
			{
				fl_SectionLayout * pSL = pCell->getSectionLayout();
				fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
				PT_DocPosition pos = pBL->getPosition();
				if(!pView1->isInTable())
				{
					pView1->setPoint(pos);
				}
				pView1->setTableFormat(pos,props);
			}
			m_draggingDocPos =0;
			return;
		}
	}
//
// Redraw the left ruler.
//
	return;
}



/*****************************************************************/


UT_sint32 AP_LeftRuler::setTableLineDrag(PT_DocPosition pos, UT_sint32 & iFixed, UT_sint32 y)
{
	xxx_UT_DEBUGMSG(("setTableLineDrag: y %d \n",y));
	m_bValidMouseClick = false;
	m_draggingWhat = DW_NOTHING;
	m_bEventIgnored = false;
	FV_View * pView = (static_cast<FV_View *>(m_pView));
	GR_Graphics * pG = pView->getGraphics();
	iFixed = pG->tlu(s_iFixedWidth);
	if(m_pView == NULL)
	{
		return 0;
	}
	if(m_pView->getPoint() == 0)
	{
		return 0;
	}
	if(pView->getDocument() == NULL)
	{
		return 0;
	}
	if(pView->getDocument()->isPieceTableChanging())
	{
		return 0;
	}
	pView->getLeftRulerInfo(pos,&m_infoCache);
	UT_ASSERT(m_infoCache.m_yTopMargin >= 0);

	queueDraw();

	iFixed = static_cast<UT_sint32>(UT_MAX(pG->tlu(m_iWidth),pG->tlu(s_iFixedWidth)));

	if(pView->getViewMode() != VIEW_PRINT)
	{
		iFixed = pG->tlu(s_iFixedWidth);
	}

	xxx_UT_DEBUGMSG(("setTableLineDrag:LeftRuler y = %d \n",y));

	if (m_infoCache.m_mode ==  AP_LeftRulerInfo::TRI_MODE_TABLE)
	{
		UT_sint32 i = 0;
		bool bFound = false;
		for(i=0; (i<= m_infoCache.m_iNumRows) && !bFound; i++)
		{
			UT_Rect rCell;
			_getCellMarkerRects(&m_infoCache, i,rCell);
			xxx_UT_DEBUGMSG(("cell %d left %d top %d width %d height %d iFixed/2 %d  y %d \n",i,rCell.left,rCell.top,rCell.width,rCell.height,iFixed/2,y));
			if(rCell.containsPoint(iFixed/2,y))
			{
				m_bValidMouseClick = true;
				m_draggingWhat = DW_CELLMARK;
				m_bBeforeFirstMotion = true;
				m_draggingCell = i;
				if(m_pG)
				{
					m_pG->setCursor(GR_Graphics::GR_CURSOR_GRAB);
				}
				m_draggingCenter = rCell.top + pG->tlu(2);
				m_draggingDocPos = pos;
				xxx_UT_DEBUGMSG(("leftRuler: Drag cell %d draggingCenter %d \n",i,m_draggingCenter));
				return (UT_sint32)(m_iWidth/2);
			}
		}
	}
	pView->getGraphics()->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
	return 0;
}



/*****************************************************************/

void AP_LeftRuler::mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y)
{
	UT_DEBUG_ONLY_ARG(ems);

	// The X and Y that are passed to this function are x and y on the application, not on the ruler.
	xxx_UT_DEBUGMSG(("In Left mouseMotion \n"));
	FV_View * pView1 = static_cast<FV_View *>(m_pView);
	if(pView1 == NULL)
	{
		return;
	}
	GR_Graphics * pG = pView1->getGraphics();
	if(m_pG && pView1->isLayoutFilling())
	{
		if(m_pG)
		{
			m_pG->setCursor( GR_Graphics::GR_CURSOR_WAIT);
		}
		return;
	}
	if(pView1->getDocument() == NULL)
	{
		return;
	}
	if(pView1->getDocument()->isPieceTableChanging())
	{
		return;
	}
	if(!m_bValidMouseClick)
	{
		pView1->getLeftRulerInfo(&m_infoCache);
	}
	UT_ASSERT(m_infoCache.m_yTopMargin >= 0);

	// if they drag vertically off the ruler, we ignore the whole thing.
	xxx_UT_DEBUGMSG(("In Left mouseMotion x %d y %d width %d \n",x,y,getWidth()));

	if ((x < 0) || (x > static_cast<UT_sint32>(getWidth())))
	{
		if(!m_bEventIgnored)
		{
			_ignoreEvent(false);
			m_bEventIgnored = true;
		}
		if(m_pG)
		{
			m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
		}
		return;
	}
	
	if (!m_bValidMouseClick)
	{
	
		UT_Rect rTopMargin, rBottomMargin;
		_getMarginMarkerRects(&m_infoCache,rTopMargin,rBottomMargin);
		rTopMargin.width = getWidth();
		rBottomMargin.width = getWidth();
		if (rTopMargin.containsPoint(x,y))
		{
			UT_DEBUGMSG(("In Top Margin box \n"));
			if(m_pG)
			{
				m_pG->setCursor( GR_Graphics::GR_CURSOR_UPDOWN);
			}
		}
		else if (rBottomMargin.containsPoint(x,y))
		{
			UT_DEBUGMSG(("In Bottom Margin box \n"));
			if(m_pG)
			{
				m_pG->setCursor( GR_Graphics::GR_CURSOR_UPDOWN);
			}
		}
		else if (m_infoCache.m_mode ==  AP_LeftRulerInfo::TRI_MODE_TABLE)
		{
			UT_sint32 i = 0;
			bool bFound = false;
			for(i=0; (i<= m_infoCache.m_iNumRows) && !bFound; i++)
			{
				UT_Rect rCell;
				_getCellMarkerRects(&m_infoCache, i,rCell);
				if(rCell.containsPoint(x,y))
				{
					if(m_pG)
					{
						m_pG->setCursor( GR_Graphics::GR_CURSOR_UPDOWN);
					}
					bFound = true;
				}
			}
			if(!bFound)
			{
				if(m_pG)
				{
					m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
				}
			}
		}
		else
		{
			if(m_pG)
			{
				m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
			}
		}
		return;
	}		
	m_bEventIgnored = false;

	UT_DEBUGMSG(("mouseMotion: [ems 0x%08x][x %d][y %d]\n",(int)ems,x,y));
	ap_RulerTicks tick(pG,m_dim);

	// if they drag vertically off the ruler, we ignore the whole thing.

	if ((x < 0) || (x > static_cast<UT_sint32>(getWidth())))
	{
		if(!m_bEventIgnored)
		{
			_ignoreEvent(false);
			m_bEventIgnored = true;
		}
		if(m_pG)
		{
			m_pG->setCursor( GR_Graphics::GR_CURSOR_DEFAULT);
		}
		return;
	}

	// lots of stuff omitted here; we should see about including it.
	// it has to do with autoscroll.

	// if we are this far along, the mouse motion is significant
	// we cannot ignore it.
	if(m_pG)
	{
		m_pG->setCursor( GR_Graphics::GR_CURSOR_GRAB);
	}	
	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_TOPMARGIN:
	case DW_BOTTOMMARGIN:
		{
		FV_View *pView = static_cast<FV_View *>(m_pView);
		bool hdrftr = pView->isHdrFtrEdit();

		fl_HdrFtrShadow * pShadow = pView->getEditShadow();

		bool hdr = (hdrftr && 
					pShadow->getHdrFtrSectionLayout()->getHFType() < FL_HDRFTR_FOOTER);

		UT_sint32 oldDragCenter = m_draggingCenter;

		UT_sint32 yAbsTop = m_infoCache.m_yPageStart - m_yScrollOffset;

		m_draggingCenter = tick.snapPixelToGrid(y);

		// bounds checking for end-of-page

		if (m_draggingCenter < yAbsTop)
			m_draggingCenter = yAbsTop;

		if (m_draggingCenter > (UT_sint32)(yAbsTop + m_infoCache.m_yPageSize))
			m_draggingCenter = yAbsTop + m_infoCache.m_yPageSize;

		UT_sint32 effectiveSize;
		UT_sint32 yOrigin = m_infoCache.m_yPageStart + m_infoCache.m_yTopMargin;
		UT_sint32 yEnd = yOrigin - m_infoCache.m_yTopMargin 
			+ m_infoCache.m_yPageSize - m_infoCache.m_yBottomMargin;
		if (m_draggingWhat == DW_TOPMARGIN)
		{
			UT_sint32 rel = m_draggingCenter + m_yScrollOffset;
			effectiveSize = yEnd - rel;
		}
		else
		{
			UT_sint32 rel = m_draggingCenter + m_yScrollOffset;
			effectiveSize = rel - yOrigin;
		}

//  		UT_DEBUGMSG(("effective size %d, limit %d\n", effectiveSize, m_minPageLength));

		if (effectiveSize < m_minPageLength)
			m_draggingCenter = oldDragCenter;
		if(m_pG)
		{
			m_pG->setCursor(GR_Graphics::GR_CURSOR_GRAB);
		}
		if(m_draggingCenter == oldDragCenter)
		{
			// Position not changing so finish here.

			return;
		}

		if (m_draggingWhat == DW_TOPMARGIN)
		{
			m_infoCache.m_yTopMargin += m_draggingCenter - oldDragCenter;
		}
		if (m_draggingWhat == DW_BOTTOMMARGIN)
		{
			m_infoCache.m_yBottomMargin -= m_draggingCenter - oldDragCenter;
			UT_DEBUGMSG(("Dragging bottom margin new value %d \n",	m_infoCache.m_yBottomMargin));
		}

		queueDraw();
		_xorGuide();
		m_bBeforeFirstMotion = false;

		// Display in margin in status bar.

		if (m_draggingWhat == DW_TOPMARGIN)
		{
			double dyrel = tick.scalePixelDistanceToUnits(m_draggingCenter - yAbsTop);

			if (hdrftr)
			{
				if (hdr)
					_displayStatusMessage(AP_STRING_ID_HeaderStatus, tick, dyrel);
				else
				{
					dyrel = tick.scalePixelDistanceToUnits
						(pShadow->getHdrFtrSectionLayout()->
						          getDocSectionLayout()->getBottomMargin() + 
						 (m_draggingCenter + m_yScrollOffset -
						(m_infoCache.m_yPageStart + m_infoCache.m_yPageSize)));

					_displayStatusMessage(AP_STRING_ID_FooterStatus, tick, dyrel);
				}
			}
			else
				_displayStatusMessage(AP_STRING_ID_TopMarginStatus, tick, dyrel);
		}
		else /* BOTTOM_MARGIN */
		{
			double dyrel = tick.scalePixelDistanceToUnits(yEnd + m_infoCache.m_yBottomMargin - m_draggingCenter - m_yScrollOffset);

			if (hdrftr && hdr)
			{
				dyrel = tick.scalePixelDistanceToUnits
						(m_draggingCenter - yAbsTop);

				_displayStatusMessage(AP_STRING_ID_TopMarginStatus, tick, dyrel); 
			}
			else
				_displayStatusMessage(AP_STRING_ID_BottomMarginStatus, tick, dyrel);
		}
		}
		return;
	case DW_CELLMARK:
		{
			UT_DEBUGMSG(("leftruler: dragging cell dragging center %d \n",m_draggingCenter));
			UT_sint32 oldDragCenter = m_draggingCenter;
			
			UT_sint32 yAbsTop = m_infoCache.m_yPageStart - m_yScrollOffset;

			m_draggingCenter = tick.snapPixelToGrid(y);

		// bounds checking for end-of-page

			if (m_draggingCenter < yAbsTop)
				m_draggingCenter = yAbsTop;

			if (m_draggingCenter > (UT_sint32)(yAbsTop + m_infoCache.m_yPageSize))
				m_draggingCenter = yAbsTop + m_infoCache.m_yPageSize;
			_xorGuide();
			if(m_pG)
			{
				m_pG->setCursor(GR_Graphics::GR_CURSOR_GRAB);
			}
			m_bBeforeFirstMotion = false;
			UT_sint32 lFixedHeight = pG->tlu(s_iFixedHeight);
			UT_uint32 xLeft = pG->tlu(s_iFixedHeight) / 4;
			xxx_UT_DEBUGMSG(("xLeft %d \n",xLeft));
			UT_Rect rCell;
			rCell.set(xLeft, m_draggingCenter-pG->tlu(2), xLeft * 2, pG->tlu(4));

			UT_Rect clip;
			if( m_draggingCenter > oldDragCenter)
			{
				clip.set(xLeft, oldDragCenter-pG->tlu(4),lFixedHeight,m_draggingCenter - oldDragCenter +lFixedHeight );
			}
			else
			{
				clip.set(xLeft, m_draggingCenter-pG->tlu(4),lFixedHeight, oldDragCenter - m_draggingCenter+ lFixedHeight);
			}
			queueDrawLU(&clip);
//
// FIXME need to clear the old cell mark
//
			_drawCellMark(&rCell,true);

			return;
		}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

/*****************************************************************/

void AP_LeftRuler::_ignoreEvent(bool /*bDone*/)
{
	// user released the mouse off of the ruler.  we need to treat
	// this as a cancel.  so we need to put everything back the
	// way it was on the ruler.

	// clear the guide line

	_xorGuide(true);

	// Clear messages from status bar.
#ifdef ENABLE_STATUSBAR
	AP_FrameData * pFrameData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());
	if(m_pFrame->getFrameMode() == XAP_NormalFrame)
	{
	    pFrameData->m_pStatusBar->setStatusMessage("");
	}
#endif
	// erase the widget that we are dragging.   remember what we
	// are dragging, clear it, and then restore it at the bottom.
	
	DraggingWhat dw = m_draggingWhat;
	m_draggingWhat = DW_NOTHING;

	if (!m_bBeforeFirstMotion)
	{
		m_bBeforeFirstMotion = true;
	}

	// redraw the widget we are dragging at its original location
	
	switch (dw)
	{
	case DW_TOPMARGIN:
	case DW_BOTTOMMARGIN:
		queueDraw();
		break;

	case DW_NOTHING:
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	m_draggingWhat = dw;
	return;
}

/*****************************************************************/

static bool s_IsOnDifferentPage(const AP_LeftRulerInfo * p1, const AP_LeftRulerInfo * p2)
{
	if(p2 == NULL)
	{
		return true;
	}
	return (   (p1->m_yPageStart    != p2->m_yPageStart)
			|| (p1->m_yPageSize     != p2->m_yPageSize)
			|| (p1->m_yTopMargin    != p2->m_yTopMargin)
			   || (p1->m_yBottomMargin != p2->m_yBottomMargin));
}
	
bool AP_LeftRuler::notify(AV_View * pView, const AV_ChangeMask mask)
{
	// Handle AV_Listener events on the view.
	UT_DEBUG_ONLY_ARG(pView);

	UT_ASSERT(pView==m_pView);
	FV_View * pVView = static_cast<FV_View *>(m_pView);

	if(pVView->getDocument() == NULL)
	{
		return false;
	}

	// If the caret has moved to a different page or any of the properties
	// on the page (such as the margins) have changed, we force a redraw.

	if (mask & (/*AV_CHG_MOTION |*/ AV_CHG_FMTSECTION | AV_CHG_HDRFTR | AV_CHG_CELL))
	{
		queueDraw();
	}
	
	return true;
}

/*****************************************************************/

void AP_LeftRuler::_scrollFuncX(void * /* pData */, UT_sint32 /* xoff */, UT_sint32 /* xlimit */)
{
	// static callback referenced by an AV_ScrollObj() for the ruler
	// we don't care about horizontal scrolling.
	return;
}

void AP_LeftRuler::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit)
{
	// static callback referenced by an AV_ScrollObj() for the ruler
	UT_ASSERT(pData);

	AP_LeftRuler * pLeftRuler = (AP_LeftRuler *)(pData);

	// let non-static member function do all the work.

	pLeftRuler->scrollRuler(yoff,ylimit);
	return;
}

/*****************************************************************/

void AP_LeftRuler::scrollRuler(UT_sint32 yoff, UT_sint32 ylimit)
{
	UT_Rect rClip;
	UT_Rect * prClip;
	FV_View * pView = static_cast<FV_View *>(m_pView);
	if(pView->getDocument() == NULL)
	{
		return;
	}
	if (ylimit > 0)
		m_yScrollLimit = ylimit;

	if (yoff > m_yScrollLimit)
		yoff = m_yScrollLimit;
	
	UT_sint32 dy = yoff - m_yScrollOffset;

	if (!dy)
		return;
	AP_LeftRulerInfo lfi;
	(static_cast<FV_View *>(m_pView))->getLeftRulerInfo(&lfi);
	UT_ASSERT(lfi.m_yTopMargin >= 0);


	if (s_IsOnDifferentPage(&lfi, m_lfi))
	{
		// if the current page has changed we override the clipping
		// and redraw everything.

		prClip = NULL;
	}
	else
	{
		// the current page is the same as the last call to queueDraw().
		// all we need to draw is the area exposed by the scroll.
		
		rClip.left = 0;
		rClip.width = m_pG->tlu(s_iFixedWidth);

		if (dy > 0)
		{
			// fudge factor for redrawing
			rClip.top = getHeight() - dy - m_pG->tlu(10);
			rClip.height = dy + m_pG->tlu(10);
		}
		else
		{
			rClip.top = 0;
			rClip.height = -dy + m_pG->tlu(10);
		}

		prClip = &rClip;
	}

	// now scroll and draw what we need to.
	
	m_pG->scroll(0,dy);
	m_yScrollOffset = yoff;
	queueDrawLU(prClip);
}

/*****************************************************************/
 
void AP_LeftRuler::_getMarginMarkerRects(const AP_LeftRulerInfo * pInfo, UT_Rect &rTop, UT_Rect &rBottom)
{
	UT_sint32 yStart = pInfo->m_yPageStart + pInfo->m_yTopMargin - m_yScrollOffset;
	UT_sint32 yEnd = pInfo->m_yPageStart + pInfo->m_yPageSize - pInfo->m_yBottomMargin - m_yScrollOffset;
	FV_View * pView = static_cast<FV_View *>(m_pView);
	if(pView == NULL)
	{
		return;
	}
	GR_Graphics * pG = pView->getGraphics();
	UT_uint32 xLeft = pG->tlu(s_iFixedHeight) / 4;
	UT_sint32 hs = pG->tlu(3);	// halfSize
	UT_sint32 fs = hs * 2;			// fullSize

	rTop.set(xLeft - fs, yStart  - hs, fs, fs- pG->tlu(1));
	rBottom.set(xLeft - fs, yEnd - hs, fs, fs);
}

void AP_LeftRuler::_drawMarginProperties(const UT_Rect * /* pClipRect */,
										const AP_LeftRulerInfo * pInfo, GR_Graphics::GR_Color3D /*clr*/)
{
	//FV_View *pView = static_cast<FV_View *>(m_pView);
	//bool hdrftr = pView->isHdrFtrEdit();
	if(m_pG == NULL)
	{
		return;
	}
	//fl_HdrFtrShadow * pShadow = pView->getEditShadow();

	//bool hdr = (hdrftr && 
	//			pShadow->getHdrFtrSectionLayout()->getHFType() < FL_HDRFTR_FOOTER);


	UT_Rect rTop, rBottom;
	UT_uint32 onePX = m_pG->tlu(1);

	_getMarginMarkerRects(pInfo,rTop,rBottom);
	
	GR_Painter painter(m_pG);

	painter.fillRect(GR_Graphics::CLR3D_Background, rTop);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	painter.drawLine( rTop.left,  rTop.top, rTop.left + rTop.width, rTop.top);
	painter.drawLine( rTop.left + rTop.width,  rTop.top, rTop.left + rTop.width, rTop.top + rTop.height);
	painter.drawLine( rTop.left + rTop.width,  rTop.top + rTop.height, rTop.left, rTop.top + rTop.height);
	painter.drawLine( rTop.left,  rTop.top + rTop.height, rTop.left, rTop.top);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	painter.drawLine( rTop.left + onePX,  rTop.top + onePX, rTop.left + rTop.width - onePX, rTop.top + onePX);
	painter.drawLine( rTop.left + onePX,  rTop.top + rTop.height - m_pG->tlu(2), rTop.left + onePX, rTop.top + onePX);
	
	// TODO: this isn't the right place for this logic. But it works.
//	if (hdrftr && !hdr)
//		return;

	painter.fillRect(GR_Graphics::CLR3D_Background, rBottom);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	painter.drawLine( rBottom.left,  rBottom.top, rBottom.left + rBottom.width, rBottom.top);
	painter.drawLine( rBottom.left + rBottom.width,  rBottom.top, rBottom.left + rBottom.width, rBottom.top + rBottom.height);
	painter.drawLine( rBottom.left + rBottom.width,  rBottom.top + rBottom.height, rBottom.left, rBottom.top + rBottom.height);
	painter.drawLine( rBottom.left,  rBottom.top + rBottom.height, rBottom.left, rBottom.top);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	painter.drawLine( rBottom.left + onePX,  rBottom.top + onePX, rBottom.left + rBottom.width - onePX, rBottom.top + onePX);
	painter.drawLine( rBottom.left + onePX,  rBottom.top + rBottom.height - m_pG->tlu(2), rBottom.left + onePX, rBottom.top + onePX);
#if 0
    m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	painter.drawLine( rBottom.left + rBottom.width - onePX,  rBottom.top + onePX, rBottom.left + rBottom.width - onePX, rBottom.top + rBottom.height - onePX);
	painter.drawLine( rBottom.left + rBottom.width - onePX,  rBottom.top + rBottom.height - onePX, rBottom.left + onePX, rBottom.top + rBottom.height - onePX);
#endif
}

 
void AP_LeftRuler::_getCellMarkerRects(const AP_LeftRulerInfo * pInfo, UT_sint32 iCell, 
									   UT_Rect &rCell, fp_TableContainer * pBroke)
{
	if(pInfo->m_mode !=  AP_LeftRulerInfo::TRI_MODE_TABLE)
	{
		rCell.set(0,0,0,0);
		return;
	}
	FV_View * pView = static_cast<FV_View *>(m_pView);
	if(pView == NULL)
	{
		rCell.set(0,0,0,0);
		return;
	}
	GR_Graphics * pG = pView->getGraphics();
	AP_LeftRulerTableInfo * pLInfo = NULL;
	if(pInfo->m_iNumRows == 0)
	{
		rCell.set(0,0,0,0);
		return;
	}

	if(iCell < pInfo->m_iNumRows)
	{
		pLInfo = pInfo->m_vecTableRowInfo->getNthItem(iCell);
	}
	else
	{
		pLInfo = pInfo->m_vecTableRowInfo->getNthItem(pInfo->m_iNumRows -1);
	}

//	UT_sint32 yOrigin = pInfo->m_yPageStart + pInfo->m_yTopMargin - m_yScrollOffset;
	UT_sint32 yOrigin = pInfo->m_yPageStart - m_yScrollOffset;
	UT_sint32 pos =0;
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pLInfo->m_pCell->getContainer());
	UT_return_if_fail(pTab);
	fp_Page * pPage = NULL;
	if(pBroke == NULL)
	{
		pBroke = pTab->getFirstBrokenTable();
		fp_Page * pCurPage =  static_cast<FV_View *>(m_pView)->getCurrentPage();
		pPage = NULL;
		while(pBroke && (pPage == NULL))
		{
			if(pBroke->getPage() != pCurPage)
			{
				pBroke = static_cast<fp_TableContainer *>(pBroke->getNext());
			}
			else
			{
				pPage = pBroke->getPage();
			}
		}
	}
	else
	{
		pPage = pBroke->getPage();
	}
	if(pPage == NULL)
	{
//
// This cell is off the page
//
		rCell.set(0,0,0,0);
		return;
	}
	if(!pView->isInFrame(pView->getPoint()))
	{
		fp_Column * pCol = static_cast<fp_Column *>(pBroke->getColumn());
		UT_sint32 iColOffset = pCol->getY();
		yOrigin += iColOffset;
	}
	else
	{
		fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(pView->getFrameLayout()->getFirstContainer());
		yOrigin += pFC->getY();
	}
	UT_sint32 yoff = pBroke->getYBreak();
	UT_sint32 yTab = 0;
	if(pBroke->getYBreak() == 0)
	{
		yTab = pTab->getY();
	}
	UT_sint32 yEnd = yOrigin - pInfo->m_yBottomMargin - pInfo->m_yTopMargin + pInfo->m_yPageSize;
	if(iCell != pInfo->m_iNumRows)
	{
		pos = yOrigin + yTab + pLInfo->m_iTopCellPos - yoff;
	}
	else
	{
		pos = yOrigin + yTab + pLInfo->m_iBotCellPos - yoff;
	}

	if((pos < yOrigin) || (pos > yEnd))
	{
//
// This cell is off the page
//
		rCell.set(0,0,0,0);
		return;
	}
	/*
	UT_sint32 bottomSpacing;
	UT_sint32 topSpacing;
	
	if (iCell == 0)
	{
		bottomSpacing = 0;
	} 
	else
	{
		UT_sint32 imax = pInfo->m_vecTableRowInfo->getItemCount();
		AP_LeftRulerTableInfo * pKInfo = NULL;
		if(iCell - 1 < imax)
		{
			pKInfo = pInfo->m_vecTableRowInfo->getNthItem(iCell-1);
		}
		else
		{
			pKInfo = pInfo->m_vecTableRowInfo->getNthItem(imax-1);
		}
		bottomSpacing = pKInfo->m_iBotSpacing;
	}

	if (iCell < pInfo->m_iNumRows)
	{
		topSpacing = pLInfo->m_iTopSpacing;
	} else
	{
		topSpacing = 0;
	}
	*/
	UT_uint32 xLeft = pG->tlu(s_iFixedHeight) / 4;
//	rCell.set(xLeft, pos - bottomSpacing, xLeft * 2, bottomSpacing + topSpacing); //left/top/width/height
	UT_sint32 mywidth = xLeft *2;
	if(mywidth == 0)
	{
		mywidth = s_iFixedWidth;
		if(mywidth == 0)
		{
			mywidth = pos-pG->tlu(8);
		}
	}
	rCell.set(xLeft, pos-pG->tlu(2), mywidth, pG->tlu(4));
}

/*!
 * Draw simple cell markers at each row position.
 */
void AP_LeftRuler::_drawCellProperties(const AP_LeftRulerInfo * pInfo)
{
	if(pInfo->m_mode != AP_LeftRulerInfo::TRI_MODE_TABLE)
	{
		return;
	}
	if(m_pG == NULL)
	{
		return;
	}
	UT_sint32 nrows = pInfo->m_iNumRows;
	UT_sint32 i = 0;
	UT_Rect rCell;
	xxx_UT_DEBUGMSG(("ap_LeftRuler: Draw Cell Marks start \n"));
	fp_Page * pCurPage =  static_cast<FV_View *>(m_pView)->getCurrentPage();
	PT_DocPosition pos = static_cast<FV_View *>(m_pView)->getPoint();
	bool bStop = false;
	fp_TableContainer *pBroke = pCurPage->getContainingTable(pos);
	if(pBroke == NULL)
	{
	  AP_LeftRulerTableInfo * pTInfo =  NULL;
	  if (pInfo->m_vecTableRowInfo->getItemCount() == 0)
	  {
	      return;
	  }
	  pTInfo = pInfo->m_vecTableRowInfo->getNthItem(0);
	  UT_return_if_fail(pTInfo);
	  fp_CellContainer * pCell = pTInfo->m_pCell;
	  fp_Container * pHdr = pCell->getContainer();
	  while(pHdr && !pHdr->isColumnType())
	  {
	    pHdr = pHdr->getContainer();
	  }
	  if(pHdr == NULL || pHdr->getContainerType() == FP_CONTAINER_COLUMN)
	  {
	        return;
	  }
	  pBroke = static_cast<fp_TableContainer *>(pCell->getContainer());
	  if(pBroke == NULL) 
	    return;
	  if(pBroke->getPage() == NULL)
	    return;
	}
	for(i=pInfo->m_iCurrentRow;i <= nrows && !bStop; i++)
	{
		if(m_bValidMouseClick && (m_draggingWhat == DW_CELLMARK) && (i == m_draggingCell ))
		{
			continue;
		}
		_getCellMarkerRects(pInfo,i,rCell,pBroke);
		if(rCell.height > 0)
		{
			_drawCellMark(&rCell,true);
		}
		else
		{
			bStop = true;
		}
	}
	bStop = false;
	for(i=pInfo->m_iCurrentRow;i >= 0 && !bStop; i--)
	{
		if(m_bValidMouseClick && (m_draggingWhat == DW_CELLMARK) && (i == m_draggingCell ))
		{
			continue;
		}
		_getCellMarkerRects(pInfo,i,rCell,pBroke);
		if(rCell.height > 0)
		{
			_drawCellMark(&rCell,true);
		}
		else
		{
			bStop = true;
		}
	}
	xxx_UT_DEBUGMSG(("ap_LeftRuler: Draw Cell Marks end \n"));
}

void AP_LeftRuler::_drawCellMark(UT_Rect *prDrag, bool /*bUp*/)
{
//
// Draw square inside
//
	if(m_pG == NULL)
	{
		return;
	}

	GR_Painter painter(m_pG);

	UT_sint32 left = prDrag->left;
	UT_sint32 right = left + prDrag->width - m_pG->tlu(1);
	UT_sint32 top = prDrag->top;
	UT_sint32 bot = top + prDrag->height - m_pG->tlu(1); // For the clever people: this gives the rect a height of 5 pixels (eg. top:10, bot:14 is 5 pixels)!
	
	painter.fillRect(GR_Graphics::CLR3D_Background, left, top, prDrag->width, prDrag->height);
	
	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	painter.drawLine(left,top,right,top);
	painter.drawLine(left,top,left,bot);
	painter.drawLine(left,bot,right,bot);
	painter.drawLine(right,top,right,bot);
	
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	painter.drawLine( left + m_pG->tlu(1), top + m_pG->tlu(1), right - m_pG->tlu(1), top + m_pG->tlu(1));
	painter.drawLine( left + m_pG->tlu(1), top + m_pG->tlu(1), left + m_pG->tlu(1), bot - m_pG->tlu(1));
}

/*****************************************************************/

void AP_LeftRuler::drawLU(const UT_Rect *clip)
{
	FV_View * pView = static_cast<FV_View *>(m_pView);
	if (!pView)
		return;
	if(pView->getPoint() == 0)
	{
		return;
	}

	if(pView->getDocument() == NULL)
	{
		return;
	}
	if(pView->getDocument()->isPieceTableChanging())
	{
		return;
	}

	if (!m_pG)
		return;

	if (!m_lfi)
		m_lfi = new AP_LeftRulerInfo;

	AP_LeftRulerInfo *lfi = m_lfi;
	pView->getLeftRulerInfo(lfi);

	GR_Painter painter(m_pG);
	painter.beginDoubleBuffering();

	UT_ASSERT(lfi->m_yTopMargin >= 0);

	m_pG->setClipRect(clip);

	/* if you get one of these two asserts then you forgot to call setWidth() or setHeight() */
	UT_ASSERT(m_iHeight);
	UT_ASSERT(m_iWidth);
	// draw the background
	
	painter.fillRect(GR_Graphics::CLR3D_Background,0,0,
					 getWidth(),getHeight());

	UT_uint32 xLeft = m_pG->tlu(s_iFixedWidth)/4;
	UT_uint32 xBar  = m_pG->tlu(s_iFixedWidth)/2;

	UT_uint32 docWithinMarginHeight = lfi->m_yPageSize - lfi->m_yTopMargin - lfi->m_yBottomMargin;

	UT_sint32 yOrigin = lfi->m_yPageStart;
	UT_sint32 yScrolledOrigin = yOrigin - m_yScrollOffset;
	UT_sint32 y,h;

	if ((yScrolledOrigin + lfi->m_yTopMargin) > 0)
	{
		// top margin of paper is on-screen.  draw dark-gray bar.
		// we need to clip it ourselves -- since the expose/paint
		// clip rects don't know anything about this distinction.

		y = yScrolledOrigin;
		h = lfi->m_yTopMargin - m_pG->tlu(1);
		painter.fillRect(GR_Graphics::CLR3D_BevelDown,xLeft,y,xBar,h);
	}

	yScrolledOrigin += lfi->m_yTopMargin + m_pG->tlu(1);
	if ((yScrolledOrigin + docWithinMarginHeight) > 0)
	{
		// area within the page margins is on-screen.
		// draw a main white bar over the area.

		y = yScrolledOrigin;
		h = docWithinMarginHeight - m_pG->tlu(1);
		painter.fillRect(GR_Graphics::CLR3D_Highlight,xLeft,y,xBar,h);
	}

	yScrolledOrigin += docWithinMarginHeight + m_pG->tlu(1);
	if ((yScrolledOrigin + lfi->m_yBottomMargin) > 0)
	{
		// bottom margin of paper is on-screen.
		// draw another dark-gray bar, like we
		// did at the top.

		y = yScrolledOrigin;
		h = lfi->m_yBottomMargin - m_pG->tlu(1);
		painter.fillRect(GR_Graphics::CLR3D_BevelDown,xLeft,y,xBar,h);
	}

	// draw 3D frame around top margin + document + bottom margin rects

	// now draw tick marks on the bar, using the selected system of units.

	ap_RulerTicks tick(m_pG,m_dim);

	UT_uint32 iFontHeight = 0;
	UT_sint32 k = 0;

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);

	GR_Font * pFont = m_pG->getGUIFont();
	if (pFont)
	{
		m_pG->setFont(pFont);
		iFontHeight = m_pG->getFontHeight() * 100 / m_pG->getZoomPercentage();
	}

	// first draw the top margin
	for (k=1; ((UT_sint32)(k*tick.tickUnit/tick.tickUnitScale) < lfi->m_yTopMargin); k++)
	{
		y = yOrigin + lfi->m_yTopMargin - k*tick.tickUnit/tick.tickUnitScale - m_yScrollOffset;
		if (y >= 0)
		{
			if (k % tick.tickLabel)
			{
				// draw the ticks
				UT_uint32 w = ((k % tick.tickLong) 
							   ? m_pG->tlu(2) : m_pG->tlu(6));
				UT_uint32 x = xLeft + (xBar-w)/2;
				painter.drawLine(x,y,x+w,y);
			}
			else if (pFont)
			{
				// draw the number
				UT_uint32 n = k / tick.tickLabel * tick.tickScale;

				char buf[6];
				UT_UCSChar span[6];
				UT_ASSERT(n < 10000);

				sprintf(buf, "%d", n);
				UT_UCS4_strcpy_char(span, buf);
				UT_uint32 len = strlen(buf);
				UT_uint32 w = m_pG->measureString(span, 0, len,
								  NULL) *
				    100 / m_pG->getZoomPercentage();

				UT_sint32 x = xLeft;
				
				if(xBar > w)
					x += (xBar-w)/2;
				painter.drawChars(span, 0, len, x, y - iFontHeight/2);
			}
		}
	}

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);	
	
	// draw everything below the top margin
	for (k=1; (static_cast<UT_sint32>(k*tick.tickUnit/tick.tickUnitScale) < (static_cast<UT_sint32>(lfi->m_yPageSize) - static_cast<UT_sint32>(lfi->m_yTopMargin))); k++)
	{
		y = yOrigin + lfi->m_yTopMargin + k*tick.tickUnit/tick.tickUnitScale - m_yScrollOffset;
		if (y >= 0)
		{
			if (k % tick.tickLabel)
			{
				// draw the ticks
				UT_uint32 w = ((k % tick.tickLong) ? 
							   m_pG->tlu(2) : m_pG->tlu(6));
				UT_uint32 x = xLeft + (xBar-w)/2;
				painter.drawLine(x,y,x+w,y);
			}
			else if (pFont)
			{
				// draw the number
				UT_uint32 n = k / tick.tickLabel * tick.tickScale;

				char buf[6];
				UT_UCSChar span[6];
				UT_ASSERT(n < 10000);

				sprintf(buf, "%d", n);
				UT_UCS4_strcpy_char(span, buf);
				UT_uint32 len = strlen(buf);

				UT_uint32 w = m_pG->measureString(span,
								  0,
								  len,
								  NULL)
				    * 100 / m_pG->getZoomPercentage();
				UT_sint32 x = xLeft;

				if(xBar > w)
					x += (xBar-w)/2;
				painter.drawChars(span, 0, len, x, y - iFontHeight/2);
			}
		}
	}

	//
	// draw the various widgets for the left ruler
	// 
	
	// section properties {left-margin, right-margin};
	_drawMarginProperties(clip, lfi, GR_Graphics::CLR3D_Foreground);

	// draw the cell properties for a table
	_drawCellProperties(lfi);
	
	// reset the current clip rect
	if (clip)
	{
		m_pG->setClipRect(NULL);
	}
}

/*****************************************************************/

void AP_LeftRuler::_xorGuide(bool bClear)
{
	UT_sint32 y = m_draggingCenter;
	GR_Graphics * pG = (static_cast<FV_View *>(m_pView))->getGraphics();
	UT_ASSERT(pG);

	GR_Painter painter(pG);

	// TODO we need to query the document window to see what the actual
	// TODO background color is so that we can compose the proper color so
	// TODO that we can XOR on it and be guaranteed that it will show up.

#if XAP_DONTUSE_XOR
	UT_RGBColor clrBlack(0,0,0);
	pG->setColor(clrBlack);
#else
	UT_RGBColor clrWhite(255,255,255);
	pG->setColor(clrWhite);
#endif

	UT_sint32 w = m_pView->getWindowWidth();
	
	if (m_bGuide)
	{
		if (!bClear && (y == m_yGuide))
			return;		// avoid flicker

		// erase old guide
#if XAP_DONTUSE_XOR
		if (m_guideCache) {
			painter.drawImage(m_guideCache, m_guideCacheRect.left, m_guideCacheRect.top);
			DELETEP(m_guideCache);
		}
#else
		painter.xorLine(0, m_yGuide, w, m_yGuide);
#endif
		m_bGuide = false;
	}

	if (!bClear)
	{
		UT_ASSERT(m_bValidMouseClick);

#if XAP_DONTUSE_XOR
		m_guideCacheRect.left = 0;
		m_guideCacheRect.top = y - pG->tlu(1);
		m_guideCacheRect.width = w;
		m_guideCacheRect.height = pG->tlu(3);
		DELETEP(m_guideCache);		// make sure it is deleted. we could leak it here
		m_guideCache = painter.genImageFromRectangle(m_guideCacheRect);

		painter.drawLine(0, y, w, y);
#else
		painter.xorLine(0, y, w, y);
#endif
		// remember this for next time
		m_yGuide = y;
		m_bGuide = true;
	}
}

void AP_LeftRuler::_prefsListener( XAP_Prefs *pPrefs, UT_StringPtrMap * /*phChanges*/, void *data )
{
	AP_LeftRuler *pLeftRuler = static_cast<AP_LeftRuler *>(data);
	UT_ASSERT( data && pPrefs );

	const gchar *pszBuffer;
	pPrefs->getPrefsValue(static_cast<const gchar *>(AP_PREF_KEY_RulerUnits), &pszBuffer );

	// or should I just default to inches or something?
	UT_Dimension dim = UT_determineDimension( pszBuffer, DIM_none );
	UT_ASSERT( dim != DIM_none );

	if ( dim != pLeftRuler->getDimension() )
		pLeftRuler->setDimension( dim );
}

void AP_LeftRuler::setDimension( UT_Dimension newdim )
{
	m_dim = newdim;
	queueDraw();
}

void AP_LeftRuler::_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue)
{
#ifdef ENABLE_STATUSBAR
	const gchar * pText = m_pG->invertDimension(tick.dimType, dValue);
	char temp[100];
	const gchar *pzMessageFormat = XAP_App::getApp()->getStringSet()->getValue(messageID);
	sprintf(temp, pzMessageFormat, pText);

	AP_FrameData * pFrameData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());
	if(m_pFrame->getFrameMode() == XAP_NormalFrame)
	{
	    pFrameData->m_pStatusBar->setStatusMessage(temp);
	}
#endif
}
