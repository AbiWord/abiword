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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <string.h>
#include <stdio.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ap_LeftRuler.h"
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

#include "pd_Document.h"

/*****************************************************************/

AP_LeftRuler::AP_LeftRuler(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_pView = NULL;
	m_pScrollObj = NULL;
	m_pG = NULL;
	m_iHeight = 0;
	m_iWidth = 0;
	m_yScrollOffset = 0;
	m_yScrollLimit = 0;
	m_bValidMouseClick = false;
	m_draggingWhat = DW_NOTHING;

	m_bGuide = false;
	m_yGuide = 0;
	
	const XML_Char * szRulerUnits;
	if (pFrame->getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		m_dim = UT_determineDimension(szRulerUnits);
	else
		m_dim = DIM_IN;

	// i wanted these to be "static const x = 32;" in the
	// class declaration, but MSVC5 can't handle it....
	// (GCC can :-)
		
	s_iFixedHeight = 32;
	s_iFixedWidth = 32;

	memset(&m_lfi,0,sizeof(m_lfi));
	
	// install top_ruler_prefs_listener as this lister for this func
	pFrame->getApp()->getPrefs()->addListener( AP_LeftRuler::_prefsListener, (void *)this );
}

AP_LeftRuler::~AP_LeftRuler(void)
{
	// don't receive anymore scroll messages
	m_pView->removeScrollListener(m_pScrollObj);

	// no more view messages
	m_pView->removeListener(m_lidLeftRuler);
	
	// no more prefs 
	m_pFrame->getApp()->getPrefs()->removeListener( AP_LeftRuler::_prefsListener, (void *)this );

	//UT_DEBUGMSG(("AP_LeftRuler::~AP_LeftRuler (this=%p scroll=%p)\n", this, m_pScrollObj));

	DELETEP(m_pScrollObj);
}

/*****************************************************************/

void AP_LeftRuler::setView(AV_View* pView, UT_uint32 iZoom)
{
	this->setView(pView);

	UT_ASSERT(m_pG);
	m_pG->setZoomPercentage(iZoom);

    // TODO this dimension shouldn't be hard coded.
	m_minPageLength = m_pG->convertDimension("0.5in");
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
	}
	
	m_pView = pView;
	
	// create an AV_ScrollObj to receive send*ScrollEvents()
	
	m_pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	UT_ASSERT(m_pScrollObj);
	m_pView->addScrollListener(m_pScrollObj);

	// Register the LeftRuler as a ViewListeners on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

	m_pView->addListener(static_cast<AV_Listener *>(this),&m_lidLeftRuler);

	return;
}

void AP_LeftRuler::_refreshView(void)
{
	setView(m_pView);
}

void AP_LeftRuler::setHeight(UT_uint32 iHeight)
{
	m_iHeight = iHeight;
}

UT_uint32 AP_LeftRuler::getHeight(void) const
{
	return m_iHeight;
}

void AP_LeftRuler::setWidth(UT_uint32 iWidth)
{
	m_iWidth = iWidth;
	AP_FrameData * pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	if (pFrameData && pFrameData->m_pTopRuler)
		pFrameData->m_pTopRuler->setOffsetLeftRuler(m_iWidth);
}

UT_uint32 AP_LeftRuler::getWidth(void) const
{
	// Well, someone did this to ap_TopRuler.cpp, so I assume that this must be
	// correct here too.  Otherwise setWidth never gets called, and that just sucks,
	// because m_iWidth does get used. -PL
	//     return s_iFixedWidth;
	return m_iWidth;
}

/*****************************************************************/

void AP_LeftRuler::mousePress(EV_EditModifierState /* ems */, EV_EditMouseButton /* emb */, UT_uint32 x, UT_uint32 y)
{
	// get the complete state of what should be on the ruler at the
	// time of the grab.  we assume that nothing in the document can
	// change during our grab unless we change it.

	m_bValidMouseClick = false;
	m_draggingWhat = DW_NOTHING;
	m_bEventIgnored = false;

	(static_cast<FV_View *>(m_pView))->getLeftRulerInfo(&m_infoCache);

	UT_sint32 yAbsTop = m_infoCache.m_yPageStart - m_yScrollOffset;
    UT_sint32 yrel = ((UT_sint32)y) - yAbsTop;
    ap_RulerTicks tick(m_pG,m_dim);
    UT_sint32 ygrid = tick.snapPixelToGrid(yrel);
    m_draggingCenter = yAbsTop + ygrid;

	m_oldY = ygrid; // used to determine if delta is zero on a mouse release

  	// only check page margins
	
  	UT_Rect rTopMargin, rBottomMargin;
  	_getMarginMarkerRects(&m_infoCache,rTopMargin,rBottomMargin);
 	if (rTopMargin.containsPoint(x,y))
 	{
 		m_bValidMouseClick = true;
 		m_draggingWhat = DW_TOPMARGIN;
 		m_bBeforeFirstMotion = true;
 		return;
 	}

 	if (rBottomMargin.containsPoint(x,y))
 	{
		FV_View *pView = static_cast<FV_View *>(m_pView);

		// it makes no sense to click on the bottom margin for the footer.
		if (pView->isHdrFtrEdit())
		{
			fl_HdrFtrShadow * pShadow = pView->getEditShadow();

			if (pShadow->getHdrFtrSectionLayout()->getHFType() 
				                == FL_HDRFTR_FOOTER)
				return;
		}

 		m_bValidMouseClick = true;
 		m_draggingWhat = DW_BOTTOMMARGIN;
 		m_bBeforeFirstMotion = true;
 		return;
 	}
}

/*****************************************************************/

void AP_LeftRuler::mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_sint32 x, UT_sint32 y)
{
	if (!m_bValidMouseClick || m_bEventIgnored)
	{
		m_draggingWhat = DW_NOTHING;
		m_bValidMouseClick = false;
		return;
	}
	
	m_bValidMouseClick = false;

	// if they drag horizontally off the ruler, we ignore the whole thing.

	if ((x < 0) || (x > (UT_sint32)m_iWidth))
	{
		_ignoreEvent(true);
		m_draggingWhat = DW_NOTHING;
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

	ap_RulerTicks tick(m_pG,m_dim);
	UT_sint32 yAbsTop = m_infoCache.m_yPageStart - m_yScrollOffset;
	UT_sint32 ygrid = tick.snapPixelToGrid(((UT_sint32)y)-yAbsTop);
	
	_xorGuide (true);
	
	if (ygrid == m_oldY) // Not moved - clicked and released
	{
		m_draggingWhat = DW_NOTHING;
		return;
	}

	const XML_Char * properties[3];

	FV_View *pView = static_cast<FV_View *>(m_pView);
	bool hdrftr = pView->isHdrFtrEdit();

	fl_HdrFtrShadow * pShadow = pView->getEditShadow();

	bool hdr = (hdrftr && 
				pShadow->getHdrFtrSectionLayout()->getHFType() == FL_HDRFTR_HEADER);

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
			dyrel = tick.scalePixelDistanceToUnits(m_draggingCenter - yAbsTop);
			if (!hdrftr)
				properties[0] = "page-margin-top";
			else
			{
				if (hdr)
					properties[0] = "page-margin-header";
				else
				{
					properties[0] = "page-margin-footer";

					dyrel = tick.scalePixelDistanceToUnits
						(pShadow->getHdrFtrSectionLayout()->
						          getDocSectionLayout()->getBottomMargin() + 
						 (m_draggingCenter + m_yScrollOffset -
						(m_infoCache.m_yPageStart + m_infoCache.m_yPageSize)));
				}
			}
		}
		break;

	case DW_BOTTOMMARGIN:
		{
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
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
		break;
	}

	properties[1] = m_pG->invertDimension(tick.dimType,dyrel);
	properties[2] = 0;

	UT_DEBUGMSG(("LeftRuler: %s [%s]\n",properties[0], properties[1]));

	_xorGuide(true);
	m_draggingWhat = DW_NOTHING;

	if(hdrftr)
	{
		pView->rememberCurrentPosition();

		pView->clearHdrFtrEdit();
  		pView->warpInsPtToXY(0,0,false);
	}

	//
	// Finally we've got it all in place, Make the change!
	//

	pView->setSectionFormat(properties);

    // todo: make this warp back to the last saved editing position
    // todo: instead of just the header/footer in general
	if (hdrftr)
	{
		if (hdr)
			pView->cmdEditHeader();
		else
			pView->cmdEditFooter();

		xxx_UT_DEBUGMSG(("DOM: %d\n", pView->getSavedPosition()));

		pView->setPoint(pView->getSavedPosition());

		{
			pView->moveInsPtTo(pView->getSavedPosition());
			pView->eraseInsertionPoint();
			pView->drawInsertionPoint();
		}
		pView->clearSavedPosition();
	}

	return;
}

/*****************************************************************/

void AP_LeftRuler::mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y)
{
	// The X and Y that are passed to this function are x and y on the screen, not on the ruler.
	
	if (!m_bValidMouseClick)
		return;
		
	m_bEventIgnored = false;

//  	UT_DEBUGMSG(("mouseMotion: [ems 0x%08lx][x %ld][y %ld]\n",ems,x,y));
	ap_RulerTicks tick(m_pG,m_dim);

	// if they drag vertically off the ruler, we ignore the whole thing.

	if ((x < 0) || (x > (UT_sint32)m_iWidth))
	{
		if(!m_bEventIgnored)
		{
			_ignoreEvent(false);
			m_bEventIgnored = true;
		}
		return;
	}

	// lots of stuff omitted here; we should see about including it.
	// it has to do with autoscroll.

	// if we are this far along, the mouse motion is significant
	// we cannot ignore it.
		
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
					pShadow->getHdrFtrSectionLayout()->getHFType() == FL_HDRFTR_HEADER);

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

		if(m_draggingCenter == oldDragCenter)
		{
			// Position not changing so finish here.

			return;
		}

		draw(NULL, m_infoCache);
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

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

/*****************************************************************/

void AP_LeftRuler::_ignoreEvent(bool bDone)
{
	// user released the mouse off of the ruler.  we need to treat
	// this as a cancel.  so we need to put everything back the
	// way it was on the ruler.

	// clear the guide line

	_xorGuide(true);

	// Clear messages from status bar.

	AP_FrameData * pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	pFrameData->m_pStatusBar->setStatusMessage("");

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
		draw(NULL, m_infoCache);
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
	return (   (p1->m_yPageStart    != p2->m_yPageStart)
			|| (p1->m_yPageSize     != p2->m_yPageSize)
			|| (p1->m_yTopMargin    != p2->m_yTopMargin)
			   || (p1->m_yBottomMargin != p2->m_yBottomMargin));
}
	
bool AP_LeftRuler::notify(AV_View * pView, const AV_ChangeMask mask)
{
	// Handle AV_Listener events on the view.

	UT_ASSERT(pView==m_pView);
	//UT_DEBUGMSG(("AP_LeftRuler::notify [view %p][mask %p]\n",pView,mask));

	// If the caret has moved to a different page or any of the properties
	// on the page (such as the margins) have changed, we force a redraw.

	if (mask & (AV_CHG_MOTION | AV_CHG_FMTSECTION | AV_CHG_HDRFTR))
	{
		AP_LeftRulerInfo lfi;
		(static_cast<FV_View *>(m_pView))->getLeftRulerInfo(&lfi);

		if (s_IsOnDifferentPage(&lfi, &m_lfi))
			draw(NULL,lfi);
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
	//UT_DEBUGMSG(("LeftRuler:: scroll [y %d]\n",yoff));
	UT_Rect rClip;
	UT_Rect * prClip;

	if (ylimit > 0)
		m_yScrollLimit = ylimit;

	if (yoff > m_yScrollLimit)
		yoff = m_yScrollLimit;
	
	UT_sint32 dy = yoff - m_yScrollOffset;
	if (!dy)
		return;

	AP_LeftRulerInfo lfi;
	(static_cast<FV_View *>(m_pView))->getLeftRulerInfo(&lfi);

	if (s_IsOnDifferentPage(&lfi, &m_lfi))
	{
		// if the current page has changed we override the clipping
		// and redraw everything.

		prClip = NULL;
	}
	else
	{
		// the current page is the same as the last call to draw().
		// all we need to draw is the area exposed by the scroll.
		
		rClip.left = 0;
		rClip.width = s_iFixedWidth;

		if (dy > 0)
		{
			rClip.top = m_iHeight - dy;
			rClip.height = dy;
		}
		else
		{
			rClip.top = 0;
			rClip.height = -dy;
		}

		prClip = &rClip;
	}

	// now scroll and draw what we need to.
	
	m_pG->scroll(0,dy);
	m_yScrollOffset = yoff;
	draw(prClip);
}

/*****************************************************************/

void AP_LeftRuler::_getMarginMarkerRects(AP_LeftRulerInfo * pInfo, UT_Rect &rTop, UT_Rect &rBottom)
{
	UT_sint32 yOrigin = pInfo->m_yPageStart + pInfo->m_yTopMargin - m_yScrollOffset;
	UT_sint32 yEnd = yOrigin - pInfo->m_yBottomMargin - pInfo->m_yTopMargin + pInfo->m_yPageSize;

	UT_uint32 xLeft = s_iFixedHeight / 4;
	UT_sint32 hs = 3;					// halfSize
	UT_sint32 fs = hs * 2;			// fullSize

	rTop.set(xLeft - fs, yOrigin  - hs, fs, fs-1);
	rBottom.set(xLeft - fs, yEnd - hs, fs, fs);
}

void AP_LeftRuler::_drawMarginProperties(const UT_Rect * /* pClipRect */,
										AP_LeftRulerInfo * pInfo, GR_Graphics::GR_Color3D clr)
{
	FV_View *pView = static_cast<FV_View *>(m_pView);
	bool hdrftr = pView->isHdrFtrEdit();

	fl_HdrFtrShadow * pShadow = pView->getEditShadow();

	bool hdr = (hdrftr && 
				pShadow->getHdrFtrSectionLayout()->getHFType() == FL_HDRFTR_HEADER);

	UT_Rect rTop, rBottom;

	_getMarginMarkerRects(pInfo,rTop,rBottom);
	
	m_pG->fillRect(GR_Graphics::CLR3D_Background, rTop);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	m_pG->drawLine( rTop.left,  rTop.top, rTop.left + rTop.width, rTop.top);
	m_pG->drawLine( rTop.left + rTop.width,  rTop.top, rTop.left + rTop.width, rTop.top + rTop.height);
	m_pG->drawLine( rTop.left + rTop.width,  rTop.top + rTop.height, rTop.left, rTop.top + rTop.height);
	m_pG->drawLine( rTop.left,  rTop.top + rTop.height, rTop.left, rTop.top);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	m_pG->drawLine( rTop.left + 1,  rTop.top + 1, rTop.left + rTop.width - 1, rTop.top + 1);
	m_pG->drawLine( rTop.left + 1,  rTop.top + rTop.height - 2, rTop.left + 1, rTop.top + 1);
	
	// TODO: this isn't the right place for this logic. But it works.
	if (hdrftr && !hdr)
		return;

	m_pG->fillRect(GR_Graphics::CLR3D_Background, rBottom);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	m_pG->drawLine( rBottom.left,  rBottom.top, rBottom.left + rBottom.width, rBottom.top);
	m_pG->drawLine( rBottom.left + rBottom.width,  rBottom.top, rBottom.left + rBottom.width, rBottom.top + rBottom.height);
	m_pG->drawLine( rBottom.left + rBottom.width,  rBottom.top + rBottom.height, rBottom.left, rBottom.top + rBottom.height);
	m_pG->drawLine( rBottom.left,  rBottom.top + rBottom.height, rBottom.left, rBottom.top);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	m_pG->drawLine( rBottom.left + 1,  rBottom.top + 1, rBottom.left + rBottom.width - 1, rBottom.top + 1);
	m_pG->drawLine( rBottom.left + 1,  rBottom.top + rBottom.height - 2, rBottom.left + 1, rBottom.top + 1);
#if 0
    m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	m_pG->drawLine( rBottom.left + rBottom.width - 1,  rBottom.top + 1, rBottom.left + rBottom.width - 1, rBottom.top + rBottom.height - 1);
	m_pG->drawLine( rBottom.left + rBottom.width - 1,  rBottom.top + rBottom.height - 1, rBottom.left + 1, rBottom.top + rBottom.height - 1);
#endif
}

/*****************************************************************/

void AP_LeftRuler::draw(const UT_Rect * pClipRect)
{
	if (!m_pView)
		return;
	
	AP_LeftRulerInfo lfi;
	(static_cast<FV_View *>(m_pView))->getLeftRulerInfo(&lfi);
	
	draw(pClipRect,lfi);
}

void AP_LeftRuler::draw(const UT_Rect * pClipRect, AP_LeftRulerInfo & lfi)
{
	if (!m_pG)
		return;
	
	if (pClipRect)
	{
		//UT_DEBUGMSG(("LeftRuler:: draw [clip %ld %ld %ld %ld]\n",pClipRect->left,pClipRect->top,pClipRect->width,pClipRect->height));
		m_pG->setClipRect(pClipRect);
	}
	else
	{
		//UT_DEBUGMSG(("LeftRuler:: draw [no clip]\n"));
	}
	
	// draw the background
	
	m_pG->fillRect(GR_Graphics::CLR3D_Background,0,0,m_iWidth,m_iHeight);

	// draw a dark-gray and white bar lined up with the paper

	UT_uint32 xLeft = s_iFixedWidth/4;
	UT_uint32 xBar  = s_iFixedWidth/2;

	UT_uint32 docWithinMarginHeight = lfi.m_yPageSize - lfi.m_yTopMargin - lfi.m_yBottomMargin;

	UT_sint32 yOrigin = lfi.m_yPageStart;
	UT_sint32 yScrolledOrigin = yOrigin - m_yScrollOffset;
	UT_sint32 y,h;

	if ((yScrolledOrigin + lfi.m_yTopMargin) > 0)
	{
		// top margin of paper is on-screen.  draw dark-gray bar.
		// we need to clip it ourselves -- since the expose/paint
		// clip rects don't know anything about this distinction.

		y = yScrolledOrigin;
		h = lfi.m_yTopMargin - 1;
		m_pG->fillRect(GR_Graphics::CLR3D_BevelDown,xLeft,y,xBar,h);
	}

	yScrolledOrigin += lfi.m_yTopMargin + 1;
	if ((yScrolledOrigin + docWithinMarginHeight) > 0)
	{
		// area within the page margins is on-screen.
		// draw a main white bar over the area.

		y = yScrolledOrigin;
		h = docWithinMarginHeight - 1;
		m_pG->fillRect(GR_Graphics::CLR3D_Highlight,xLeft,y,xBar,h);
	}

	yScrolledOrigin += docWithinMarginHeight + 1;
	if ((yScrolledOrigin + lfi.m_yBottomMargin) > 0)
	{
		// bottom margin of paper is on-screen.
		// draw another dark-gray bar, like we
		// did at the top.

		y = yScrolledOrigin;
		h = lfi.m_yBottomMargin - 1;
		m_pG->fillRect(GR_Graphics::CLR3D_BevelDown,xLeft,y,xBar,h);
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
		iFontHeight = m_pG->getFontHeight();
	}

	// first draw the top margin
	for (k=1; ((UT_sint32)(k*tick.tickUnit/tick.tickUnitScale) < lfi.m_yTopMargin); k++)
	{
		y = yOrigin + lfi.m_yTopMargin - k*tick.tickUnit/tick.tickUnitScale - m_yScrollOffset;
		if (y >= 0)
		{
			if (k % tick.tickLabel)
			{
				// draw the ticks
				UT_uint32 w = ((k % tick.tickLong) ? 2 : 6);
				UT_uint32 x = xLeft + (xBar-w)/2;
				m_pG->drawLine(x,y,x+w,y);
			}
			else if (pFont)
			{
				// draw the number
				UT_uint32 n = k / tick.tickLabel * tick.tickScale;

				char buf[6];
				UT_UCSChar span[6];
				UT_uint16 charWidths[6];
				UT_ASSERT(n < 10000);

				sprintf(buf, "%d", n);
				UT_UCS_strcpy_char(span, buf);
				UT_uint32 len = strlen(buf);

				UT_uint32 w = m_pG->measureString(span, 0, len, charWidths);
				UT_uint32 x = xLeft + (xBar-w)/2;

				m_pG->drawChars(span, 0, len, x, y - iFontHeight/2);
			}
		}
	}
	
	// then draw everything below
	for (k=1; (k*tick.tickUnit/tick.tickUnitScale < (lfi.m_yPageSize - lfi.m_yTopMargin)); k++)
	{
		y = yOrigin + lfi.m_yTopMargin + k*tick.tickUnit/tick.tickUnitScale - m_yScrollOffset;
		if (y >= 0)
		{
			if (k % tick.tickLabel)
			{
				// draw the ticks
				UT_uint32 w = ((k % tick.tickLong) ? 2 : 6);
				UT_uint32 x = xLeft + (xBar-w)/2;
				m_pG->drawLine(x,y,x+w,y);
			}
			else if (pFont)
			{
				// draw the number
				UT_uint32 n = k / tick.tickLabel * tick.tickScale;

				char buf[6];
				UT_UCSChar span[6];
				UT_uint16 charWidths[6];
				UT_ASSERT(n < 10000);

				sprintf(buf, "%d", n);
				UT_UCS_strcpy_char(span, buf);
				UT_uint32 len = strlen(buf);

				UT_uint32 w = m_pG->measureString(span, 0, len, charWidths);
				UT_uint32 x = xLeft + (xBar-w)/2;

				m_pG->drawChars(span, 0, len, x, y - iFontHeight/2);
			}
		}
	}

	// draw the various widgets for the:
	// 
	// current section properties {left-margin, right-margin};
	_drawMarginProperties(pClipRect, &lfi, GR_Graphics::CLR3D_Foreground);
	
	if (pClipRect)
		m_pG->setClipRect(NULL);

	m_lfi = lfi;
}

/*****************************************************************/

void AP_LeftRuler::_xorGuide(bool bClear)
{
	UT_sint32 y = m_draggingCenter;

	GR_Graphics * pG = (static_cast<FV_View *>(m_pView))->getGraphics();
	UT_ASSERT(pG);

	// TODO we need to query the document window to see what the actual
	// TODO background color is so that we can compose the proper color so
	// TODO that we can XOR on it and be guaranteed that it will show up.

	UT_RGBColor clrWhite(255,255,255);
	pG->setColor(clrWhite);

	UT_sint32 w = m_pView->getWindowWidth();
	
	if (m_bGuide)
	{
		if (!bClear && (y == m_yGuide))
			return;		// avoid flicker

		// erase old guide
		pG->xorLine(0, m_yGuide, w, m_yGuide);
		m_bGuide = false;
	}

	if (!bClear)
	{
		UT_ASSERT(m_bValidMouseClick);
		pG->xorLine(0, y, w, y);

		// remember this for next time
		m_yGuide = y;
		m_bGuide = true;
	}
}

/*static*/ void AP_LeftRuler::_prefsListener( XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_AlphaHashTable * /*phChanges*/, void *data )
{
	AP_LeftRuler *pLeftRuler = (AP_LeftRuler *)data;
	UT_ASSERT( data && pPrefs );

	const XML_Char *pszBuffer;
	pPrefs->getPrefsValue((XML_Char*)AP_PREF_KEY_RulerUnits, &pszBuffer );

	// or should I just default to inches or something?
	UT_Dimension dim = UT_determineDimension( pszBuffer, DIM_none );
	UT_ASSERT( dim != DIM_none );

	if ( dim != pLeftRuler->getDimension() )
		pLeftRuler->setDimension( dim );
}

void AP_LeftRuler::setDimension( UT_Dimension newdim )
{
	m_dim = newdim;
	draw( (const UT_Rect *)0 );
}

void AP_LeftRuler::_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue)
{
	const XML_Char * pText = m_pG->invertDimension(tick.dimType, dValue);
	char temp[100];
	const XML_Char *pzMessageFormat = m_pFrame->getApp()->getStringSet()->getValue(messageID);
	sprintf(temp, pzMessageFormat, pText);

	AP_FrameData * pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	pFrameData->m_pStatusBar->setStatusMessage(temp);
}

