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

#include <string.h>
#include <stdio.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ap_TopRuler.h"
#include "gr_Graphics.h"
#include "ap_Ruler.h"
#include "ap_Prefs.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "ap_FrameData.h"
#include "ap_StatusBar.h"
#include "ap_Strings.h"

// HACK: private copy of constants from fl_BlockLayout.h
// TODO: find a better way of passing iType for tabs?
// NOTE: this ordering is convenient for cycling m_iDefaultTabType 

#define FL_TAB_LEFT				1
#define FL_TAB_CENTER			2
#define FL_TAB_RIGHT			3
#define FL_TAB_DECIMAL			4
#define FL_TAB_BAR				5

#define	tr_TABINDEX_NEW			-1
#define	tr_TABINDEX_NONE		-2


/*****************************************************************/

AP_TopRuler::AP_TopRuler(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_pView = NULL;
	m_pScrollObj = NULL;
	m_pG = NULL;
	//m_iHeight = 0;
	m_iWidth = 0;
	m_iLeftRulerWidth = 0;
	m_xScrollOffset = 0;
	m_xScrollLimit = 0;
	m_bValidMouseClick = UT_FALSE;
	m_draggingWhat = DW_NOTHING;
	m_iDefaultTabType = FL_TAB_LEFT;

	m_bGuide = UT_FALSE;
	m_xGuide = 0;
	
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

	// set the default to be the fixed size
	m_iHeight = s_iFixedHeight;

	// install top_ruler_prefs_listener as this lister for this func
	pFrame->getApp()->getPrefs()->addListener( AP_TopRuler::_prefsListener, (void *)this );

}

AP_TopRuler::~AP_TopRuler(void)
{
	// don't receive anymore scroll messages
	m_pView->removeScrollListener(m_pScrollObj);

	// no more view messages
	m_pView->removeListener(m_lidTopRuler);
	
	// no more prefs 
	m_pFrame->getApp()->getPrefs()->removeListener( AP_TopRuler::_prefsListener, (void *)this );

	//UT_DEBUGMSG(("AP_TopRuler::~AP_TopRuler (this=%p scroll=%p)\n", this, m_pScrollObj));

	DELETEP(m_pScrollObj);
}

/*****************************************************************/

void AP_TopRuler::setView(AV_View* pView, UT_uint32 iZoom)
{
	this->setView(pView);

	UT_ASSERT(m_pG);
	m_pG->setZoomPercentage(iZoom);

	m_minColumnWidth = m_pG->convertDimension("0.5in");//TODO should this dimension be hard coded.

}

void AP_TopRuler::setView(AV_View * pView)
{
	if (m_pView && (m_pView != pView))
	{
		// view is changing.  since this TopRuler class is bound to
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

	// Register the TopRuler as a ViewListeners on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

	m_pView->addListener(static_cast<AV_Listener *>(this),&m_lidTopRuler);

	return;
}

void AP_TopRuler::setOffsetLeftRuler(UT_uint32 iLeftRulerWidth)
{
	// we assume that the TopRuler spans the LeftRuler and
	// the DocumentWindow.  The width of the LeftRuler gives
	// us the right edge of the fixed region -- the portion
	// that should not be scrolled on an horizontal scroll.
	// We allow for the LeftRuler to be zero (if/when we
	// support a UI to turn it on and off.

	m_iLeftRulerWidth = iLeftRulerWidth;
}

void AP_TopRuler::setHeight(UT_uint32 iHeight)
{
	m_iHeight = iHeight;
}

UT_uint32 AP_TopRuler::getHeight(void) const
{
	//return s_iFixedHeight;
	return m_iHeight;
}

void AP_TopRuler::setWidth(UT_uint32 iWidth)
{
	m_iWidth = iWidth;
}

UT_uint32 AP_TopRuler::getWidth(void) const
{
	return m_iWidth;
}

/*****************************************************************/

UT_Bool AP_TopRuler::notify(AV_View * pView, const AV_ChangeMask mask)
{
	// Handle AV_Listener events on the view.

	UT_ASSERT(pView==m_pView);
	//UT_DEBUGMSG(("AP_TopRuler::notify [view %p][mask %p]\n",pView,mask));

	// if the column containing the caret has changed or any
	// properties on the section (like the number of columns
	// or the margins) or on the block (like the paragraph
	// indents), then we redraw the ruler.
	
	if (mask & (AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK))
		draw(NULL);
	
	return UT_TRUE;
}

/*****************************************************************/

void AP_TopRuler::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit)
{
	// static callback referenced by an AV_ScrollObj() for the ruler
	UT_ASSERT(pData);

	AP_TopRuler * pTopRuler = (AP_TopRuler *)(pData);

	// let non-static member function do all the work.
	
	pTopRuler->scrollRuler(xoff,xlimit);
	return;
}

void AP_TopRuler::_scrollFuncY(void * /*pData*/, UT_sint32 /*yoff*/, UT_sint32 /*ylimit*/)
{
	// static callback referenced by an AV_ScrollObj() for the ruler
	// we don't care about vertical scrolling.
	return;
}

/*****************************************************************/

void AP_TopRuler::scrollRuler(UT_sint32 xoff, UT_sint32 xlimit)
{
	// scroll the window while excluding the portion
	// lining up with the LeftRuler.

	if (xlimit > 0)
		m_xScrollLimit = xlimit;

	if (xoff > m_xScrollLimit)
		xoff = m_xScrollLimit;
	
	UT_sint32 dx = xoff - m_xScrollOffset;
	if (!dx)
		return;

	UT_sint32 xFixed = MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 width = m_iWidth - xFixed;
	UT_sint32 height = s_iFixedHeight;
	UT_sint32 y_dest = 0;
	UT_sint32 y_src = 0;
	UT_sint32 x_dest = xFixed;
	UT_sint32 x_src = xFixed;

	UT_Rect rClip;
	rClip.top = y_src;
	rClip.height = height;
	
	if (dx > 0)
	{
		x_src += dx;
		width += -dx;
		rClip.left = x_dest + width;
		rClip.width = dx;
	}
	else if (dx < 0)
	{
		x_dest += -dx;
		width += dx;
		rClip.left = x_src;
		rClip.width = -dx;
	}
	
	m_pG->scroll(x_dest,y_dest,x_src,y_src,width,height);
	m_xScrollOffset = xoff;
	draw(&rClip);
}

/*****************************************************************/

void AP_TopRuler::draw(const UT_Rect * pClipRect, AP_TopRulerInfo * pUseInfo)
{
	if (!m_pG)
		return;
	
	if (pClipRect)
		m_pG->setClipRect(pClipRect);

	// draw the background

	m_pG->fillRect(GR_Graphics::CLR3D_Background,0,0,m_iWidth,m_iHeight);

	// draw the foreground
	
	_draw(pClipRect,pUseInfo);
	
	if (pClipRect)
		m_pG->setClipRect(NULL);
}

void AP_TopRuler::_drawBar(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
						   GR_Graphics::GR_Color3D clr3d, UT_sint32 x, UT_sint32 w)
{
	// Draw ruler bar (white or dark-gray) over [x,x+w)
	// where x is in page-relative coordinates.  we need
	// to compensate for fixed portion, the page-view margin,
	// and the scroll.
	
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);

	// convert page-relative coordinates into absolute coordinates.
	
	UT_sint32 xAbsLeft = xFixed + pInfo->m_xPageViewMargin + x - m_xScrollOffset;
	UT_sint32 xAbsRight = xAbsLeft + w;

	// we need to do our own clipping for the fixed area
	
	if (xAbsLeft < xFixed)			// need to shorten what we draw
		xAbsLeft = xFixed;

	// draw whatever is left

	if (xAbsRight > xAbsLeft)
	{
		UT_Rect r(xAbsLeft,yTop,(xAbsRight-xAbsLeft),yBar);
		if (!pClipRect || r.intersectsRect(pClipRect))
			m_pG->fillRect(clr3d,r);
	}
	
	return;
}

void AP_TopRuler::_drawTickMark(const UT_Rect * pClipRect,
								AP_TopRulerInfo * /* pInfo */, ap_RulerTicks &tick,
								GR_Graphics::GR_Color3D clr3d, GR_Font * pFont,
								UT_sint32 k, UT_sint32 xTick)
{
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;

	if (pClipRect)
	{
		// do quick and crude check for visibility.
		// we know that everything that we draw will
		// be vertically within the bar.  horizontally
		// it will either be a single line or a small
		// font -- let's assume that a 3 digit centered
		// string will be less than 100 pixels.
		
		UT_Rect r(xTick-50,yTop,100,yBar);
		if (!r.intersectsRect(pClipRect))
			return;
	}

	if (k % tick.tickLabel)
	{
		// draw the ticks
		UT_uint32 h = ((k % tick.tickLong) ? 2 : 6);
		UT_uint32 y = yTop + (yBar-h)/2;
		m_pG->setColor3D(clr3d);
		m_pG->drawLine(xTick,y,xTick,y+h);
	}
	else if (pFont)
	{
		// draw the number
		m_pG->setColor3D(clr3d);
		m_pG->setFont(pFont);
		UT_uint32 iFontHeight = m_pG->getFontHeight();
		UT_uint32 n = k / tick.tickLabel * tick.tickScale;

		if (n == 0)						// we never draw the zero on the
			return;						// origin
		
		char buf[6];
		UT_UCSChar span[6];
		UT_uint16 charWidths[6];
		UT_ASSERT(n < 10000);

		sprintf(buf, "%d", n);
		UT_UCS_strcpy_char(span, buf);
		UT_uint32 len = strlen(buf);

		UT_uint32 w = m_pG->measureString(span, 0, len, charWidths);
		UT_uint32 y = yTop + (yBar-iFontHeight)/2;

		m_pG->drawChars(span, 0, len, xTick - w/2, y);
	}
}
		
void AP_TopRuler::_drawTicks(const UT_Rect * pClipRect,
							 AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
							 GR_Graphics::GR_Color3D clr3d, GR_Font * pFont,
							 UT_sint32 xOrigin, UT_sint32 xFrom, UT_sint32 xTo)
{
	// draw tick marks over the ruler bar.
	// xOrigin gives the page-relative x-coordinate of zero.
	// xFrom gives the page-relative x-coordinate of where we should begin drawing.
	// xTo gives the page-relative x-coordinate of where we should end drawing.
	// if xTo is less than xFrom we draw with values increasing to the left.

	UT_ASSERT(xFrom != xTo);
	UT_ASSERT(xFrom >= 0);
	UT_ASSERT(xTo >= 0);
	
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);

	// convert page-relative coordinates into absolute coordinates.
	
	UT_sint32 xAbsOrigin = xFixed + pInfo->m_xPageViewMargin + xOrigin - m_xScrollOffset;
	UT_sint32 xAbsFrom   = xFixed + pInfo->m_xPageViewMargin + xFrom   - m_xScrollOffset;
	UT_sint32 xAbsTo     = xFixed + pInfo->m_xPageViewMargin + xTo     - m_xScrollOffset;

	// we need to do our own clipping for the fixed area
	
	if (xAbsFrom < xFixed)
		xAbsFrom = xFixed;
	if (xAbsTo < xFixed)
		xAbsTo = xFixed;
	if (xAbsFrom == xAbsTo)
		return;							// everything clipped

	if (xAbsTo > xAbsFrom)
	{
		// draw increasing numbers to the right
		
		UT_sint32 k=0; 
		while (1)
		{
			UT_sint32 xTick = xAbsOrigin + k*tick.tickUnit/tick.tickUnitScale;
			if (xTick > xAbsTo)
				break;
			if (xTick >= xAbsFrom)
				_drawTickMark(pClipRect,pInfo,tick,clr3d,pFont,k,xTick);
			k++;
		}
	}
	else
	{
		// draw increasing numbers to the left

		UT_sint32 k=0; 
		while (1)
		{
			UT_sint32 xTick = xAbsOrigin - k*tick.tickUnit/tick.tickUnitScale;
			if (xTick < xAbsTo)
				break;
			if (xTick <= xAbsFrom)
				_drawTickMark(pClipRect,pInfo,tick,clr3d,pFont,k,xTick);
			k++;
		}
	}
}

/*****************************************************************/

void AP_TopRuler::_getParagraphMarkerXCenters(AP_TopRulerInfo * pInfo,
											  UT_sint32 * pLeft, UT_sint32 * pRight, UT_sint32 * pFirstLine)
{
	UT_sint32 xAbsLeft = _getFirstPixelInColumn(pInfo,pInfo->m_iCurrentColumn);
	UT_sint32 xAbsRight = xAbsLeft + pInfo->u.c.m_xColumnWidth;

	if (pLeft)
		*pLeft = xAbsLeft + pInfo->m_xrLeftIndent;
	if (pRight)
		*pRight = xAbsRight - pInfo->m_xrRightIndent;
	if (pFirstLine)
		*pFirstLine = xAbsLeft + pInfo->m_xrLeftIndent + pInfo->m_xrFirstLineIndent;
	return;
}

UT_Bool AP_TopRuler::_isInBottomBoxOfLeftIndent(UT_uint32 y)
{
	// return true if in the lower box of the left-indent pair
	
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;
	UT_uint32 yBottom = yTop + yBar;

	return (y > yBottom);
}

void AP_TopRuler::_getParagraphMarkerRects(AP_TopRulerInfo * /* pInfo */,
										   UT_sint32 leftCenter,
										   UT_sint32 rightCenter,
										   UT_sint32 firstLineCenter,
										   UT_Rect * prLeftIndent,
										   UT_Rect * prRightIndent,
										   UT_Rect * prFirstLineIndent)
{
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;
	UT_uint32 yBottom = yTop + yBar;
	UT_sint32 hs = 5;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	if (prLeftIndent)
		prLeftIndent->set(leftCenter - hs, yBottom - 9, fs, 15);
	if (prFirstLineIndent)
		prFirstLineIndent->set(firstLineCenter - hs, yTop - 3, fs, 9);
	if (prRightIndent)
		prRightIndent->set(rightCenter - hs, yBottom - 9, fs, 9);
}

void AP_TopRuler::_drawParagraphProperties(const UT_Rect * pClipRect,
										   AP_TopRulerInfo * pInfo,
										   UT_Bool bDrawAll)
{
	UT_sint32 leftCenter, rightCenter, firstLineCenter;
	UT_Rect rLeftIndent, rRightIndent, rFirstLineIndent;

	_getParagraphMarkerXCenters(pInfo,&leftCenter,&rightCenter,&firstLineCenter);
	_getParagraphMarkerRects(pInfo,
							 leftCenter, rightCenter, firstLineCenter,
							 &rLeftIndent, &rRightIndent, &rFirstLineIndent);

	if (m_draggingWhat == DW_LEFTINDENTWITHFIRST)
	{
		_drawLeftIndentMarker(rLeftIndent, UT_FALSE); // draw hollow version at old location
		_drawFirstLineIndentMarker(rFirstLineIndent, UT_FALSE);
		_drawLeftIndentMarker(m_draggingRect, UT_TRUE);	// draw sculpted version at mouse
		_drawFirstLineIndentMarker(m_dragging2Rect, UT_TRUE);
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rLeftIndent.intersectsRect(pClipRect))
			_drawLeftIndentMarker(rLeftIndent, UT_TRUE); // draw sculpted version at current location		
		if (!pClipRect || rFirstLineIndent.intersectsRect(pClipRect))
			_drawFirstLineIndentMarker(rFirstLineIndent, UT_TRUE);
	}
	
	if (m_draggingWhat == DW_LEFTINDENT)
	{
		_drawLeftIndentMarker(rLeftIndent, UT_FALSE); // draw hollow version at old location
		_drawLeftIndentMarker(m_draggingRect, UT_TRUE);	// draw sculpted version at mouse
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rLeftIndent.intersectsRect(pClipRect))
			_drawLeftIndentMarker(rLeftIndent, UT_TRUE); // draw sculpted version at current location
	}

	if (m_draggingWhat == DW_RIGHTINDENT)
	{
		_drawRightIndentMarker(rRightIndent, UT_FALSE);
		_drawRightIndentMarker(m_draggingRect, UT_TRUE);
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rRightIndent.intersectsRect(pClipRect))
			_drawRightIndentMarker(rRightIndent, UT_TRUE);
	}
	
	if (m_draggingWhat == DW_FIRSTLINEINDENT)
	{
		_drawFirstLineIndentMarker(rFirstLineIndent, UT_FALSE);
		_drawFirstLineIndentMarker(m_draggingRect, UT_TRUE);
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rFirstLineIndent.intersectsRect(pClipRect))
			_drawFirstLineIndentMarker(rFirstLineIndent, UT_TRUE);
	}
}


/*****************************************************************/

void AP_TopRuler::_getTabToggleRect(UT_Rect * prToggle)
{
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);

	UT_sint32 t = (s_iFixedHeight - 17)/2;
	UT_sint32 l = (xFixed - 17)/2;

	if (prToggle)
		prToggle->set(t, l, 17, 17);
}

/*****************************************************************/

void AP_TopRuler::_getTabStopXAnchor(AP_TopRulerInfo * pInfo,
										UT_sint32 k,
										UT_sint32 * pTab,
										unsigned char & iType)
{
	UT_sint32 xAbsLeft = _getFirstPixelInColumn(pInfo,pInfo->m_iCurrentColumn);

	UT_sint32 iPosition;
	UT_uint32 iOffset;

	if (k == tr_TABINDEX_NEW)
	{
		// this is a new tab
		iPosition = m_dragStart;
		iType = m_draggingTabType;
	}
	else
	{
		// look it up in the document
		UT_ASSERT(k<pInfo->m_iTabStops);

		UT_Bool bRes = pInfo->m_pfnEnumTabStops(pInfo->m_pVoidEnumTabStopsData, 
												k, iPosition, iType, iOffset);
		UT_ASSERT(bRes);
	}

	if (pTab)
		*pTab = xAbsLeft + iPosition;

	return;
}

void AP_TopRuler::_getTabStopRect(AP_TopRulerInfo * /* pInfo */,
									 UT_sint32 anchor,
									 UT_Rect * pRect)
{
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;
	UT_uint32 yBottom = yTop + yBar;
	UT_sint32 hs = 4;					// halfSize
	UT_sint32 fs = hs * 2 + 2;			// fullSize

	if (pRect)
		pRect->set(anchor - hs, yBottom - 6, fs, 6);
}

void AP_TopRuler::_drawTabProperties(const UT_Rect * pClipRect,
										   AP_TopRulerInfo * pInfo,
										   UT_Bool bDrawAll)
{
	UT_sint32 anchor;
	UT_Rect rect;
	unsigned char iType;

	if (m_draggingWhat == DW_TABSTOP)
	{
		// just deal with the tab being moved

		_getTabStopXAnchor(pInfo, m_draggingTab, &anchor, iType);
		_getTabStopRect(pInfo, anchor, &rect);

		_drawTabStop(rect, m_draggingTabType, UT_FALSE);
		_drawTabStop(m_draggingRect, m_draggingTabType, UT_TRUE);
	}
	
	/*
		NOTE: even during tab drags, we might need to draw other tabs
		that got revealed after being obscured by the dragged tab
	*/
	
	if (bDrawAll)
	{
		// loop over all explicit tabs

		UT_sint32 xAbsLeft = _getFirstPixelInColumn(pInfo,pInfo->m_iCurrentColumn);
		UT_sint32 left = xAbsLeft + pInfo->m_xrLeftIndent;

		for (UT_sint32 i = 0; i < pInfo->m_iTabStops; i++)
		{
			if ((m_draggingWhat == DW_TABSTOP) &&
				(m_draggingTab == (UT_sint32) i))
				continue;

			_getTabStopXAnchor(pInfo, i, &anchor, iType);
			_getTabStopRect(pInfo, anchor, &rect);

			if (left < anchor)
				left = anchor;

			if (!pClipRect || rect.intersectsRect(pClipRect))
				_drawTabStop(rect, iType, UT_TRUE);
		}

		if (m_draggingWhat != DW_TABSTOP)
		{
			// draw trailing default tabs 

			UT_sint32 xAbsRight = xAbsLeft + pInfo->u.c.m_xColumnWidth;
			UT_uint32 yTop = s_iFixedHeight/4;
			UT_uint32 yBar = s_iFixedHeight/2;
			UT_uint32 yBottom = yTop + yBar;
		
			m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);

			UT_ASSERT(pInfo->m_iDefaultTabInterval > 0);
			if (pInfo->m_iDefaultTabInterval > 0)			// prevent infinite loop -- just in case
			{
				UT_sint32 iPos = xAbsLeft;
				for (;iPos < xAbsRight; iPos += pInfo->m_iDefaultTabInterval)
				{
					if (iPos <= left)
						continue;
					
					m_pG->drawLine(iPos, yBottom + 1, iPos, yBottom + 4);				
				}
			}
		}
	}
}

UT_sint32 AP_TopRuler::_findTabStop(AP_TopRulerInfo * pInfo, 
									UT_uint32 x, UT_uint32 y, unsigned char & iType)
{
	// hit-test all the existing tabs
	// return the index of the one found

	UT_sint32 anchor;
	UT_Rect rect;

	for (UT_sint32 i = 0; i < pInfo->m_iTabStops; i++)
	{
		_getTabStopXAnchor(pInfo, i, &anchor, iType);
		_getTabStopRect(pInfo, anchor, &rect);

		if (rect.containsPoint(x,y))
			return i;
	}

	return tr_TABINDEX_NONE;
}

void AP_TopRuler::_getTabZoneRect(AP_TopRulerInfo * pInfo, UT_Rect &rZone)
{
	// this is the zone where clicking will get you a new tab
	// basically the bottom half of the ruler, inside the current column

	UT_uint32 yBar = s_iFixedHeight/2;
	UT_sint32 xAbsLeft = _getFirstPixelInColumn(pInfo,0);
	UT_sint32 xAbsRight = xAbsLeft + pInfo->u.c.m_xColumnWidth;

	rZone.set(xAbsLeft,  s_iFixedHeight - yBar, xAbsRight-xAbsLeft, yBar);
}

const char * AP_TopRuler::_getTabStopString(AP_TopRulerInfo * pInfo, UT_sint32 k)
{
	// return pointer to static buffer -- use it quickly.

	UT_sint32 iPosition;
	UT_uint32 iOffset;
	unsigned char iType;

	UT_Bool bRes = pInfo->m_pfnEnumTabStops(pInfo->m_pVoidEnumTabStopsData, 
											k, iPosition, iType, iOffset);
	UT_ASSERT(bRes);

	const char* pStart = &pInfo->m_pszTabStops[iOffset];
	const char* pEnd = pStart;
	while (*pEnd && (*pEnd != ','))
	{
		pEnd++;
	}

	UT_uint32 iLen = pEnd - pStart;
	UT_ASSERT(iLen<20);

	static char buf[20];

	strncpy(buf, pStart, iLen);
	buf[iLen]=0;

	return buf;
}				

/*****************************************************************/

UT_sint32 AP_TopRuler::_getColumnMarkerXRightEnd(AP_TopRulerInfo * pInfo, UT_uint32 kCol)
{
	// return the right edge of the gap following this column
	// (this is equal to the left edge of the start of the
	// next column)
	
	return _getFirstPixelInColumn(pInfo,kCol+1);
}

void AP_TopRuler::_getColumnMarkerRect(AP_TopRulerInfo * pInfo, UT_uint32 /* kCol */,
									   UT_sint32 xRight, UT_Rect * prCol)
{
	UT_uint32 yTop = s_iFixedHeight/4;

	UT_sint32 xAbsLeft = _getFirstPixelInColumn(pInfo,0);
	UT_sint32 xAbsRight = xAbsLeft + pInfo->u.c.m_xColumnWidth;
	UT_sint32 xAbsRightGap = xAbsRight + pInfo->u.c.m_xColumnGap;
	UT_sint32 xdelta = xRight - xAbsRightGap;
	prCol->set(xAbsRight-xdelta, yTop-5, pInfo->u.c.m_xColumnGap + 2*xdelta + 1, 11);
}

void AP_TopRuler::_drawColumnProperties(const UT_Rect * pClipRect,
										AP_TopRulerInfo * pInfo,
										UT_uint32 kCol)
{
	UT_Rect rCol;
	
	_getColumnMarkerRect(pInfo,kCol,_getColumnMarkerXRightEnd(pInfo,kCol),&rCol);

	if ( (m_draggingWhat == DW_COLUMNGAP) || (m_draggingWhat == DW_COLUMNGAPLEFTSIDE) )
	{
		_drawColumnGapMarker(m_draggingRect);
	}
	else
	{
		if (!pClipRect || rCol.intersectsRect(pClipRect))
			_drawColumnGapMarker(rCol);
	}

}

/*****************************************************************/

void AP_TopRuler::_getMarginMarkerRects(AP_TopRulerInfo * pInfo, UT_Rect &rLeft, UT_Rect &rRight)
{
	// we play some games with where the right margin is
	// drawn.  this compensates for an odd pixel roundoff
	// due to our (current) restriction that all columns
	// and gaps are the same size.  rather than compute
	// from the right of the paper, we compute the right
	// edge of the last column.

	UT_sint32 xAbsLeft = _getFirstPixelInColumn(pInfo,0);
	UT_sint32 xAbsRight = _getFirstPixelInColumn(pInfo, pInfo->m_iNumColumns - 1) + pInfo->u.c.m_xColumnWidth;

	UT_uint32 yTop = s_iFixedHeight / 4;
	UT_sint32 hs = 3;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	rLeft.set( xAbsLeft  -hs, yTop - fs, fs, fs);
	rRight.set(xAbsRight -hs, yTop - fs, fs, fs);
}

void AP_TopRuler::_drawMarginProperties(const UT_Rect * /* pClipRect */,
										AP_TopRulerInfo * pInfo, GR_Graphics::GR_Color3D clr)
{
	UT_Rect rLeft, rRight;

	_getMarginMarkerRects(pInfo,rLeft,rRight);
	m_pG->fillRect(GR_Graphics::CLR3D_Background, rLeft);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	m_pG->drawLine( rLeft.left,  rLeft.top, rLeft.left + rLeft.width, rLeft.top);
	m_pG->drawLine( rLeft.left + rLeft.width,  rLeft.top, rLeft.left + rLeft.width, rLeft.top + rLeft.height);
	m_pG->drawLine( rLeft.left + rLeft.width,  rLeft.top + rLeft.height, rLeft.left, rLeft.top + rLeft.height);
	m_pG->drawLine( rLeft.left,  rLeft.top + rLeft.height, rLeft.left, rLeft.top);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	m_pG->drawLine( rLeft.left + 1,  rLeft.top + 1, rLeft.left + rLeft.width - 2, rLeft.top + 1);
	m_pG->drawLine( rLeft.left + 1,  rLeft.top + rLeft.height - 2, rLeft.left + 1, rLeft.top + 1);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	m_pG->drawLine( rLeft.left + rLeft.width - 2,  rLeft.top + 1, rLeft.left + rLeft.width - 2, rLeft.top + rLeft.height - 2);
	m_pG->drawLine( rLeft.left + rLeft.width - 2,  rLeft.top + rLeft.height - 2, rLeft.left + 1, rLeft.top + rLeft.height - 2);

	m_pG->fillRect(GR_Graphics::CLR3D_Background, rRight);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	m_pG->drawLine( rRight.left,  rRight.top, rRight.left + rRight.width, rRight.top);
	m_pG->drawLine( rRight.left + rRight.width,  rRight.top, rRight.left + rRight.width, rRight.top + rRight.height);
	m_pG->drawLine( rRight.left + rRight.width,  rRight.top + rRight.height, rRight.left, rRight.top + rRight.height);
	m_pG->drawLine( rRight.left,  rRight.top + rRight.height, rRight.left, rRight.top);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	m_pG->drawLine( rRight.left + 1,  rRight.top + 1, rRight.left + rRight.width - 2, rRight.top + 1);
	m_pG->drawLine( rRight.left + 1,  rRight.top + rRight.height - 2, rRight.left + 1, rRight.top + 1);
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	m_pG->drawLine( rRight.left + rRight.width - 2,  rRight.top + 1, rRight.left + rRight.width - 2, rRight.top + rRight.height - 2);
	m_pG->drawLine( rRight.left + rRight.width - 2,  rRight.top + rRight.height - 2, rRight.left + 1, rRight.top + rRight.height - 2);


}

/*****************************************************************/

void AP_TopRuler::_draw(const UT_Rect * pClipRect, AP_TopRulerInfo * pUseInfo)
{
	UT_sint32 sum;
	UT_uint32 k;

	// ask the view for paper/margin/column/table/caret
	// details at the current insertion point.

	AP_TopRulerInfo infoLocal;
	AP_TopRulerInfo * pInfo;
	if (pUseInfo)
	{
		// if we are given an info arg, use it
		pInfo = pUseInfo;
	}
	else
	{
		// otherwise we calculate our own.
		pInfo = &infoLocal;
		(static_cast<FV_View *>(m_pView))->getTopRulerInfo(pInfo);
	}
	
	// draw the tab toggle inside the fixed area in the left-hand corner

	_drawTabToggle(pClipRect, UT_FALSE);


	// TODO for now assume we are in column display mode.
	UT_ASSERT(pInfo->m_mode==AP_TopRulerInfo::TRI_MODE_COLUMNS);

	// draw the dark-gray and white bar across the
	// width of the paper.  we adjust the x coords
	// by 1 to keep a light-gray bar between the
	// dark-gray bars (margins & gaps) and the white
	// bars (columns).
	
	// draw a dark-gray bar over the left margin

	_drawBar(pClipRect,pInfo,GR_Graphics::CLR3D_BevelDown,0+1,pInfo->u.c.m_xaLeftMargin-1);
	sum=pInfo->u.c.m_xaLeftMargin;

	for (k=0; k<pInfo->m_iNumColumns; k++)
	{
		// draw white bar over this column
		
		_drawBar(pClipRect,pInfo, GR_Graphics::CLR3D_Highlight, sum+1, pInfo->u.c.m_xColumnWidth-1);
		sum += pInfo->u.c.m_xColumnWidth;

		// if another column after this one, draw dark gray-gap
		
		if (k+1 < pInfo->m_iNumColumns)
		{
			_drawBar(pClipRect,pInfo, GR_Graphics::CLR3D_BevelDown, sum+1, pInfo->u.c.m_xColumnGap-1);
			sum += pInfo->u.c.m_xColumnGap;
		}
	}

	// draw dark-gray right margin
	
	_drawBar(pClipRect,pInfo, GR_Graphics::CLR3D_BevelDown, sum+1,pInfo->u.c.m_xaRightMargin-1);

	// now draw tick marks on the bar, using the selected system of units.

	ap_RulerTicks tick(m_pG,m_dim);
	GR_Font * pFont = m_pG->getGUIFont();

	// find the origin for the tick marks.  this is the left-edge of the
	// column that we are in.  everything to the left of this x-value will
	// be drawn on a negative scale to the left relative to here.  everything
	// to the right of this x-value will be drawn on a positive scale to the
	// right.

	UT_sint32 xTickOrigin = pInfo->u.c.m_xaLeftMargin;
	if (pInfo->m_iCurrentColumn > 0)
		xTickOrigin += pInfo->m_iCurrentColumn * (pInfo->u.c.m_xColumnWidth + pInfo->u.c.m_xColumnGap);

	sum = 0;

	// draw negative ticks over left margin.  

	if(pInfo->u.c.m_xaLeftMargin)
	{
		_drawTicks(pClipRect,pInfo,tick,GR_Graphics::CLR3D_Foreground,pFont,xTickOrigin, pInfo->u.c.m_xaLeftMargin,sum);
	}
	sum += pInfo->u.c.m_xaLeftMargin;
	
	for (k=0; k<pInfo->m_iNumColumns; k++)
	{
		// draw positive or negative ticks on this column.
		
		if (k < pInfo->m_iCurrentColumn)
			_drawTicks(pClipRect,pInfo,tick,GR_Graphics::CLR3D_Foreground,pFont,xTickOrigin, sum+pInfo->u.c.m_xColumnWidth, sum);
		else
			_drawTicks(pClipRect,pInfo,tick,GR_Graphics::CLR3D_Foreground,pFont,xTickOrigin, sum, sum+pInfo->u.c.m_xColumnWidth);

		sum += pInfo->u.c.m_xColumnWidth;

		// if another column after this one, skip over the gap
		// (we don't draw ticks on the gap itself).

		if (k+1 < pInfo->m_iNumColumns)
			sum += pInfo->u.c.m_xColumnGap;
	}

	// draw ticks over the right margin

	if(pInfo->u.c.m_xaRightMargin)
		{
		_drawTicks(pClipRect,pInfo,tick,GR_Graphics::CLR3D_Foreground,pFont,xTickOrigin, sum, sum+pInfo->u.c.m_xaRightMargin);
		}

	// draw the various widgets for the:
	// 
	// current section properties {left-margin, right-margin};
	// the current column properties {column-gap};
	// and the current paragraph properties {left-indent, right-indent, first-left-indent}.

	_drawMarginProperties(pClipRect, pInfo, GR_Graphics::CLR3D_Foreground);
	if (pInfo->m_iNumColumns > 1)
		_drawColumnProperties(pClipRect,pInfo,0);
	_drawParagraphProperties(pClipRect,pInfo,UT_TRUE);
	_drawTabProperties(pClipRect,pInfo,UT_TRUE);
	
	return;
}

/*****************************************************************/

void AP_TopRuler::_xorGuide(UT_Bool bClear)
{
	UT_uint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 x = m_draggingCenter - xFixed;

	// when dragging the column gap, we draw lines on both
	// sides of the gap.
	
	UT_sint32 xOther = m_draggingRect.left - xFixed;

	GR_Graphics * pG = (static_cast<FV_View *>(m_pView))->getGraphics();
	UT_ASSERT(pG);

	// TODO we need to query the document window to see what the actual
	// TODO background color is so that we can compose the proper color so
	// TODO that we can XOR on it and be guaranteed that it will show up.

	UT_RGBColor clrWhite(255,255,255);
	pG->setColor(clrWhite);

	UT_sint32 h = m_pView->getWindowHeight();
	
	if (m_bGuide)
	{
		if (!bClear && (x == m_xGuide))
			return;		// avoid flicker

		// erase old guide
		pG->xorLine(m_xGuide, 0, m_xGuide, h);
		if ( (m_draggingWhat == DW_COLUMNGAP) || (m_draggingWhat == DW_COLUMNGAPLEFTSIDE) )
			pG->xorLine(m_xOtherGuide, 0, m_xOtherGuide, h);
		m_bGuide = UT_FALSE;
	}

	if (!bClear)
	{
		UT_ASSERT(m_bValidMouseClick);
		pG->xorLine(x, 0, x, h);
		if ( (m_draggingWhat == DW_COLUMNGAP) || (m_draggingWhat == DW_COLUMNGAPLEFTSIDE) )
			pG->xorLine(xOther, 0, xOther, h);

		// remember this for next time
		m_xGuide = x;
		m_xOtherGuide = xOther;
		m_bGuide = UT_TRUE;
	}
}

/*****************************************************************/

void AP_TopRuler::mousePress(EV_EditModifierState /* ems */, EV_EditMouseButton /* emb */, UT_uint32 x, UT_uint32 y)
{
	//UT_DEBUGMSG(("mousePress: [ems 0x%08lx][emb 0x%08lx][x %ld][y %ld]\n",ems,emb,x,y));

	// get the complete state of what should be on the ruler at the time of the grab.
	// we assume that nothing in the document can change during our grab unless we
	// change it.

	m_bValidMouseClick = UT_FALSE;
	m_draggingWhat = DW_NOTHING;
	m_bEventIgnored = UT_FALSE;
	
	(static_cast<FV_View *>(m_pView))->getTopRulerInfo(&m_infoCache);

	// Set this in case we never get a mouse motion event
        UT_sint32 xAbsLeft = _getFirstPixelInColumn(    &m_infoCache,
                                                        m_infoCache.m_iCurrentColumn);
        UT_sint32 xrel = ((UT_sint32)x) - xAbsLeft;
        ap_RulerTicks tick(m_pG,m_dim);
        UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
        m_draggingCenter = xAbsLeft + xgrid;

		m_oldX = xgrid; // used to determine if delta is zero on a mouse release

	// first hit-test against the tab toggle control

	UT_Rect rToggle;
	_getTabToggleRect(&rToggle);
	if (rToggle.containsPoint(x,y))
	{
		switch(m_iDefaultTabType)
		{
			case FL_TAB_LEFT:		m_iDefaultTabType = FL_TAB_CENTER;	break;
			case FL_TAB_CENTER:		m_iDefaultTabType = FL_TAB_RIGHT;	break;
			case FL_TAB_RIGHT:		m_iDefaultTabType = FL_TAB_DECIMAL;	break;
			case FL_TAB_DECIMAL:	m_iDefaultTabType = FL_TAB_LEFT;	break;
		}
		_drawTabToggle(NULL, UT_TRUE);
		return;
	}

	// next hit-test against the tabs

	unsigned char iType;
	UT_sint32 iTab = _findTabStop(&m_infoCache, x, y, iType);
	if (iTab >= 0)
	{
		UT_DEBUGMSG(("hit tab %ld\n",iTab));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_TABSTOP;
		m_draggingTab = iTab;
		m_draggingTabType = iType;
		m_dragStart = 0;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}

	// next hit-test against the paragraph widgets
	
	UT_sint32 leftIndentCenter, rightIndentCenter, firstLineIndentCenter;
	UT_Rect rLeftIndent, rRightIndent, rFirstLineIndent;
	_getParagraphMarkerXCenters(&m_infoCache,&leftIndentCenter,&rightIndentCenter,&firstLineIndentCenter);
	_getParagraphMarkerRects(&m_infoCache,
							 leftIndentCenter, rightIndentCenter, firstLineIndentCenter,
							 &rLeftIndent, &rRightIndent, &rFirstLineIndent);
	if (rLeftIndent.containsPoint(x,y))
	{
		//UT_DEBUGMSG(("hit left indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = ((_isInBottomBoxOfLeftIndent(y)) ? DW_LEFTINDENTWITHFIRST : DW_LEFTINDENT);
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}
	if (rRightIndent.containsPoint(x,y))
	{
		//UT_DEBUGMSG(("hit right indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_RIGHTINDENT;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}
	if (rFirstLineIndent.containsPoint(x,y))
	{
		//UT_DEBUGMSG(("hit first-line-indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_FIRSTLINEINDENT;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}

	// next check the column gap
	
	if (m_infoCache.m_iNumColumns > 1)
	{
		UT_Rect rCol;
		_getColumnMarkerRect(&m_infoCache,0,_getColumnMarkerXRightEnd(&m_infoCache,0),&rCol);
		if (rCol.containsPoint(x,y))
		{
			//UT_DEBUGMSG(("hit in column gap block\n"));
			m_bValidMouseClick = UT_TRUE;
			m_draggingWhat = ((((UT_sint32)x) > (rCol.left+(rCol.width/2))) ? DW_COLUMNGAP : DW_COLUMNGAPLEFTSIDE);
			m_bBeforeFirstMotion = UT_TRUE;
			return;
		}
	}

	// next check page margins
	
	UT_Rect rLeftMargin, rRightMargin;
	_getMarginMarkerRects(&m_infoCache,rLeftMargin,rRightMargin);
	if (rLeftMargin.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit left margin block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_LEFTMARGIN;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}
	if (rRightMargin.containsPoint(x,y))
	{
		//UT_DEBUGMSG(("hit right margin block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_RIGHTMARGIN;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}

	// finally, if nothing else, try to create a new tab

	UT_Rect rZone;
	_getTabZoneRect(&m_infoCache,rZone);
	if (rZone.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit new tab\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_TABSTOP;
		m_draggingTab = tr_TABINDEX_NEW;
		m_draggingTabType = m_iDefaultTabType;
		m_bBeforeFirstMotion = UT_TRUE;

		// this is a new widget, so it needs more work to get started
		m_dragStart = xgrid;

		double dgrid = _scalePixelDistanceToUnits(xrel,tick);
		UT_DEBUGMSG(("SettingTabStop: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));

		UT_sint32 oldDraggingCenter = m_draggingCenter;
		UT_Rect oldDraggingRect = m_draggingRect;
		m_draggingCenter = xAbsLeft + xgrid;
		_getTabStopRect(&m_infoCache,m_draggingCenter,&m_draggingRect);
		if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
			draw(&oldDraggingRect,&m_infoCache);
		_drawTabProperties(NULL,&m_infoCache,UT_FALSE);
		_xorGuide();

		m_bBeforeFirstMotion = UT_FALSE;

		// Since this is a new tab, it may be a simple click and not
		// a drag. Set m_oldX to a negative number so that
		// mouseMotion() is fooled.
		m_oldX = -1;
	}
}

/*****************************************************************/

void AP_TopRuler::mouseRelease(EV_EditModifierState /* ems */, EV_EditMouseButton /* emb */, UT_sint32 x, UT_sint32 y)
{
	if (!m_bValidMouseClick || (m_bEventIgnored && m_draggingWhat != DW_TABSTOP))
	{
		m_draggingWhat = DW_NOTHING;
		return;
	}

	m_bValidMouseClick = UT_FALSE;

	// if they drag vertically off the ruler, we ignore the whole thing.

	if ((y < 0) || (y > (UT_sint32)m_iHeight))
	{
		_ignoreEvent(UT_TRUE);
		m_draggingWhat = DW_NOTHING;
		return;
	}

	// if they drag horizontally off the ruler, we probably want to ignore
	// the whole thing.  but this may interact with the current scroll.

	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	if ((x < xFixed) || (x > (UT_sint32)m_iWidth))
	{
		_ignoreEvent(UT_TRUE);
		m_draggingWhat = DW_NOTHING;
		return;
	}

	// mouse up was in the ruler portion of the window, we cannot ignore it.
	// i'd like to assert that we can just use the data computed in the
	// last mouseMotion() since the Release must be at the same spot or
	// we'd have received another Motion before the release.  therefore,
	// we use the last value of m_draggingCenter that we computed.

	// also, we do not do any drawing here.  we assume that whatever change
	// that we make to the document will cause a notify event to come back
	// to us and cause a full draw.
	
	//UT_DEBUGMSG(("mouseRelease: [ems 0x%08lx][emb 0x%08lx][x %ld][y %ld]\n",ems,emb,x,y));

	ap_RulerTicks tick(m_pG,m_dim);
	UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
	UT_sint32 xgrid = _snapPixelToGrid(((UT_sint32)x) - xAbsLeft, tick);
	
	_xorGuide (UT_TRUE);
	
	if (xgrid == m_oldX) // Not moved - clicked and released
	{
		m_draggingWhat = DW_NOTHING;
		return;
	}
	
	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_LEFTMARGIN:
		{
			UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
			UT_sint32 xAbsLeft = xFixed + m_infoCache.m_xPageViewMargin - m_xScrollOffset;
			double dxrel = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft, tick);

			const XML_Char * properties[3];
			properties[0] = "page-margin-left";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: page-margin-left [%s]\n",properties[1]));

			_xorGuide(UT_TRUE);
			m_draggingWhat = DW_NOTHING;
            FV_View *pView = static_cast<FV_View *>(m_pView);
            pView->setSectionFormat(properties);
            pView->draw(NULL);
		}
		return;

	case DW_RIGHTMARGIN:
		{
			UT_sint32 xAbsRight = _getFirstPixelInColumn(&m_infoCache, m_infoCache.m_iNumColumns - 1) + 
								m_infoCache.u.c.m_xColumnWidth + m_infoCache.u.c.m_xaRightMargin;
			double dxrel = _scalePixelDistanceToUnits(xAbsRight - m_draggingCenter, tick);

			const XML_Char * properties[3];
			properties[0] = "page-margin-right";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: page-margin-right [%s]\n",properties[1]));

			_xorGuide(UT_TRUE);
			m_draggingWhat = DW_NOTHING;
            FV_View *pView = static_cast<FV_View *>(m_pView);
            pView->setSectionFormat(properties);
            pView->draw(NULL);
		}
		return;

	case DW_COLUMNGAP:
	case DW_COLUMNGAPLEFTSIDE:
		{
			double dxrel = _scalePixelDistanceToUnits(m_draggingRect.width,tick);

			const XML_Char * properties[3];
			properties[0] = "column-gap";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: ColumnGap [%s]\n",properties[1]));

			m_draggingWhat = DW_NOTHING;
            FV_View *pView = static_cast<FV_View *>(m_pView);
            pView->setSectionFormat(properties);
            pView->draw(NULL);
		}
		return;
		
	case DW_LEFTINDENT:
		{
			// we are dragging only the left-indent and not the first-line.
			// so, when we drop the left-indent, we need to reset the first-line
			// so that the absolute position of the first-line has not changed.
			// first-line is stored in relative terms, so we need to update it.
			
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			double dxrel  = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft,tick);

			UT_sint32 xdelta = (m_draggingCenter-xAbsLeft) - m_infoCache.m_xrLeftIndent;
			double dxrel2 = _scalePixelDistanceToUnits(m_infoCache.m_xrFirstLineIndent - xdelta,tick);

			// invertDimension() returns pointer to static buffer, so
			// we need to copy these for later use.
			
			char buf1[50];
			strcpy(buf1,m_pG->invertDimension(tick.dimType,dxrel));
			char buf2[50];
			strcpy(buf2,m_pG->invertDimension(tick.dimType,dxrel2));
			
			const XML_Char * properties[5];
			properties[0] = "margin-left";
			properties[1] = buf1;
			properties[2] = "text-indent";
			properties[3] = buf2;
			properties[4] = 0;
			UT_DEBUGMSG(("TopRuler: LeftIndent [%s] TextIndent [%s]\n",
						 properties[1],properties[3]));

			m_draggingWhat = DW_NOTHING;
			(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
		}
		return;

	case DW_LEFTINDENTWITHFIRST:
		{
			// we are dragging both the left-indent and first-line in sync
			// so that we do not change the first-line-indent relative to
			// the paragraph.  since first-line-indent is stored in the
			// document in relative coordinates, we don't need to do anything.
			double dxrel = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft,tick);

			const XML_Char * properties[3];
			properties[0] = "margin-left";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: LeftIndent [%s]\n",properties[1]));

			m_draggingWhat = DW_NOTHING;
			(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
		}
		return;
		
	case DW_RIGHTINDENT:
		{
			UT_sint32 xAbsRight = xAbsLeft + m_infoCache.u.c.m_xColumnWidth;
			double dxrel = _scalePixelDistanceToUnits(xAbsRight - m_draggingCenter,tick);

			const XML_Char * properties[3];
			properties[0] = "margin-right";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: RightIndent [%s]\n",properties[1]));

			m_draggingWhat = DW_NOTHING;
			(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
		}
		return;

	case DW_FIRSTLINEINDENT:
		{
			double dxrel = _scalePixelDistanceToUnits(m_draggingCenter-xAbsLeft-m_infoCache.m_xrLeftIndent,tick);

			const XML_Char * properties[3];
			properties[0] = "text-indent";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: FirstLineIndent [%s]\n",properties[1]));
			
			m_draggingWhat = DW_NOTHING;
			(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
		}
		return;

	case DW_TABSTOP:
		{
			unsigned char iType;
			UT_sint32 iTab = _findTabStop(&m_infoCache, x, y, iType);

			if ((iTab >= 0) && (iTab == m_draggingTab))
			{
				// this tabstop is already set here ==> NOOP
				m_draggingWhat = DW_NOTHING;
				return;
			}

			_setTabStops(tick, iTab, UT_FALSE);
		}
		return;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

void AP_TopRuler::_setTabStops(ap_RulerTicks tick, UT_sint32 iTab, UT_Bool bDelete)
{
	UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
	double dxrel = _scalePixelDistanceToUnits(m_draggingCenter-xAbsLeft,tick);
	
	char buf[1024];

	// first add the new tab settings
		
	if (!bDelete)
	{
		char * sz = NULL;
		switch(m_draggingTabType)
		{
			case FL_TAB_LEFT:		sz = "L";	break;
			case FL_TAB_RIGHT:		sz = "R";	break;
			case FL_TAB_CENTER:		sz = "C";	break;
			case FL_TAB_DECIMAL:	sz = "D";	break;
			case FL_TAB_BAR:		sz = "B";	break;
			default:
				sz = "";
		}
		sprintf(buf, "%s/%s", m_pG->invertDimension(tick.dimType,dxrel), sz);
	}
	else
	{
		buf[0] = 0;
	}

	// then append all the remaining tabstops, if any

	for (UT_sint32 i = 0; i < m_infoCache.m_iTabStops; i++)
	{
		if ((i == iTab) || (i == m_draggingTab))
			continue;

		if (*buf)
			strcat(buf, ",");

		strcat(buf, _getTabStopString(&m_infoCache, i));
	}

	const XML_Char * properties[3];
	properties[0] = "tabstops";
	properties[1] = buf;
	properties[2] = 0;
	UT_DEBUGMSG(("TopRuler: Tab Stop [%s]\n",properties[1]));

	m_draggingWhat = DW_NOTHING;
	(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
}

/*****************************************************************/

void AP_TopRuler::mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y)
{
	if (!m_bValidMouseClick)
		return;
		
	UT_DEBUGMSG(("mouseMotion: [ems 0x%08lx][x %ld][y %ld]\n",ems,x,y));

	// if they drag vertically off the ruler, we ignore the whole thing.

	if ((y < 0) || (y > (UT_sint32)m_iHeight))
	{
		if(!m_bEventIgnored)
		{
			_ignoreEvent(UT_FALSE);
			m_bEventIgnored = UT_TRUE;
		}
		return;
	}

	// if they drag horizontally off the ruler, we probably want to ignore
	// the whole thing.  but this may interact with the current scroll.

	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xAbsRight = _getFirstPixelInColumn(&m_infoCache, m_infoCache.m_iNumColumns - 1) + 
		m_infoCache.u.c.m_xColumnWidth + m_infoCache.u.c.m_xaRightMargin;
	ap_RulerTicks tick(m_pG,m_dim);

	if ((x < xFixed + m_infoCache.m_xPageViewMargin)
		|| (x > xAbsRight))
	{
		if(!m_bEventIgnored)
		{
			_ignoreEvent(UT_FALSE);
			m_bEventIgnored = UT_TRUE;
		}
		return;
	}

	m_bEventIgnored = UT_FALSE;

	// mouse motion was in the ruler portion of the window, we cannot ignore it.

	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_LEFTMARGIN:
		{
		UT_sint32 oldDragCenter = m_draggingCenter;

		UT_sint32 xAbsLeft = xFixed + m_infoCache.m_xPageViewMargin - m_xScrollOffset;

		m_draggingCenter = _snapPixelToGrid(x, tick);

        UT_sint32 iAbsLeft = _getFirstPixelInColumn(&m_infoCache,0);
        UT_sint32 iAbsRight = iAbsLeft + m_infoCache.u.c.m_xColumnWidth;
        UT_sint32 iIndentShift = UT_MAX(0,UT_MAX(m_infoCache.m_xrLeftIndent,m_infoCache.m_xrLeftIndent + m_infoCache.m_xrFirstLineIndent));
        UT_sint32 iRightIndentPos = iAbsRight - UT_MAX(0,m_infoCache.m_xrRightIndent) - iIndentShift;
        if(iRightIndentPos - m_draggingCenter < m_minColumnWidth)
		{
            m_draggingCenter = iRightIndentPos - m_minColumnWidth;
		}
        iIndentShift = UT_MIN(0,UT_MIN(m_infoCache.m_xrLeftIndent,m_infoCache.m_xrLeftIndent + m_infoCache.m_xrFirstLineIndent));
        m_draggingCenter = UT_MAX(m_draggingCenter, xAbsLeft - iIndentShift);

		if(m_draggingCenter == oldDragCenter)
		{
			// Position not changing so finish here.

			return;
		}

		UT_sint32 newMargin = m_draggingCenter - xAbsLeft;
		UT_sint32 deltaLeftMargin = newMargin - m_infoCache.u.c.m_xaLeftMargin;
		UT_sint32 newColumnWidth = m_infoCache.u.c.m_xColumnWidth - deltaLeftMargin / (UT_sint32)m_infoCache.m_iNumColumns;
		if(m_infoCache.m_xrFirstLineIndent + m_infoCache.m_xrLeftIndent > m_infoCache.m_xrLeftIndent)
		{
			if(m_infoCache.m_xrFirstLineIndent + m_infoCache.m_xrLeftIndent > 0)
			{
				newColumnWidth -= m_infoCache.m_xrFirstLineIndent + m_infoCache.m_xrLeftIndent;
			}
		}
		else if(m_infoCache.m_xrLeftIndent > 0)
		{
			newColumnWidth -= m_infoCache.m_xrLeftIndent;
		}
		if(newColumnWidth < m_minColumnWidth)
		{
			x -= (m_minColumnWidth - newColumnWidth) * (UT_sint32)m_infoCache.m_iNumColumns;

			m_draggingCenter = _snapPixelToGrid(x, tick);
			newMargin = m_draggingCenter - xAbsLeft;
			deltaLeftMargin = newMargin - m_infoCache.u.c.m_xaLeftMargin;
			
		}
		m_infoCache.u.c.m_xaLeftMargin += deltaLeftMargin;
		m_infoCache.u.c.m_xColumnWidth -= deltaLeftMargin / (UT_sint32)m_infoCache.m_iNumColumns;

		draw(NULL, &m_infoCache);
		m_infoCache.u.c.m_xaLeftMargin -= deltaLeftMargin;
		m_infoCache.u.c.m_xColumnWidth += deltaLeftMargin / (UT_sint32)m_infoCache.m_iNumColumns;
		_xorGuide();
		m_bBeforeFirstMotion = UT_FALSE;

		// Display in margin in status bar.

		double dxrel = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft, tick);

		_displayStatusMessage(AP_STRING_ID_LeftMarginStatus, tick, dxrel);
		}
		return;

	case DW_RIGHTMARGIN:
		{
		UT_sint32 oldDragCenter = m_draggingCenter;
        UT_sint32 iAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
        UT_sint32 iRightShift = UT_MAX(0,m_infoCache.m_xrRightIndent);
        UT_sint32 iLeftShift = UT_MAX(0,UT_MAX(m_infoCache.m_xrLeftIndent,m_infoCache.m_xrLeftIndent + m_infoCache.m_xrFirstLineIndent));
        UT_sint32 newMargin;
        UT_sint32 deltaRightMargin;
        UT_sint32 newColumnWidth;

        x = UT_MIN(x,xAbsRight + UT_MIN(0,m_infoCache.m_xrRightIndent));
        while(1){
            newMargin = xAbsRight - x;
            deltaRightMargin = newMargin - m_infoCache.u.c.m_xaRightMargin;
            newColumnWidth = m_infoCache.u.c.m_xColumnWidth - deltaRightMargin / (UT_sint32)m_infoCache.m_iNumColumns;

            if(newColumnWidth - iRightShift - iLeftShift < m_minColumnWidth){
                x += (m_minColumnWidth - (newColumnWidth - iRightShift - iLeftShift)) * (UT_sint32)m_infoCache.m_iNumColumns;
            } else {
                break;
		}
        }

        m_draggingCenter = _snapPixelToGrid(x,tick);

		if(m_draggingCenter == oldDragCenter)
		{
			// Position not changing so finish here.

			return;
		}

		m_infoCache.u.c.m_xaRightMargin += deltaRightMargin;
		m_infoCache.u.c.m_xColumnWidth -= deltaRightMargin / (UT_sint32)m_infoCache.m_iNumColumns;

		draw(NULL, &m_infoCache);
		m_infoCache.u.c.m_xaRightMargin -= deltaRightMargin;
		m_infoCache.u.c.m_xColumnWidth += deltaRightMargin / (UT_sint32)m_infoCache.m_iNumColumns;
		_xorGuide();
		m_bBeforeFirstMotion = UT_FALSE;

		// Display in margin in status bar.

		double dxrel = _scalePixelDistanceToUnits(xAbsRight - m_draggingCenter, tick);

		_displayStatusMessage(AP_STRING_ID_RightMarginStatus, tick, dxrel);
		}
		return;

	case DW_COLUMNGAP:
	case DW_COLUMNGAPLEFTSIDE:
		{
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,0);
			UT_sint32 xAbsRight2 = xAbsLeft + m_infoCache.u.c.m_xColumnWidth;
			UT_sint32 xAbsRightGap = xAbsRight2 + m_infoCache.u.c.m_xColumnGap;
			UT_sint32 xAbsMidPoint = (xAbsRight2 + xAbsRightGap)/2;
			UT_sint32 xrel;
			
			if(m_draggingWhat == DW_COLUMNGAP)
			{
				UT_sint32 xAbsLowerLimit = xAbsMidPoint + _snapPixelToGrid((UT_sint32)(tick.dragDelta/tick.tickUnitScale),tick);
				if (((UT_sint32)x) < xAbsLowerLimit)
					x = (UT_uint32)xAbsLowerLimit;
				xrel = ((UT_sint32)x) - xAbsRight2;
			}
			else
			{
				UT_sint32 xAbsUpperLimit = xAbsMidPoint - _snapPixelToGrid((UT_sint32)(tick.dragDelta/tick.tickUnitScale),tick);
				if (((UT_sint32)x) > xAbsUpperLimit)
					x = (UT_uint32)xAbsUpperLimit;
				xrel = xAbsRightGap - ((UT_sint32)x);
			}

			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);

			// Check the column width.

			UT_sint32 columnWidth = xAbsRightGap - xgrid - xAbsLeft;
			UT_DEBUGMSG(("[Column width %ld]\n",columnWidth));

			if(columnWidth < m_minColumnWidth)
			{
				xgrid -= m_minColumnWidth - columnWidth;
			}

			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsRight2 + xgrid;
			_getColumnMarkerRect(&m_infoCache,0,m_draggingCenter,&m_draggingRect);


			UT_DEBUGMSG(("Gap: [x %ld][xAbsRight %ld][xrel %ld][xgrid %ld][width %ld]\n",x,xAbsRight2,xrel,xgrid,m_draggingRect.width));


			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(((oldDraggingRect.width > m_draggingRect.width ) ? &oldDraggingRect : &m_draggingRect),
					 &m_infoCache);
			_drawColumnProperties(NULL,&m_infoCache,0);
			_xorGuide();

			double dgrid = _scalePixelDistanceToUnits(m_draggingRect.width,tick);
			_displayStatusMessage(AP_STRING_ID_ColumnGapStatus, tick, dgrid);
			
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;
		
	case DW_LEFTINDENT:
		{
			// we are dragging the left-indent box **without** the
			// first-line-indent.  this keeps the same absolute
			// location for the first-line, but means that we need
			// to update it (since it is stored in relative to the
			// paragraph in the document).
			
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xrel = ((UT_sint32)x) - xAbsLeft;
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingLeftIndent: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsLeft + xgrid;

            UT_sint32 iRightPos = m_infoCache.u.c.m_xColumnWidth + xAbsLeft - m_infoCache.m_xrRightIndent;
            if(iRightPos - m_draggingCenter < m_minColumnWidth)
            {
                m_draggingCenter = iRightPos - m_minColumnWidth;
            }

			_getParagraphMarkerRects(&m_infoCache,m_draggingCenter,0,0,&m_draggingRect,NULL,NULL);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawParagraphProperties(NULL,&m_infoCache,UT_FALSE);
			_xorGuide();

		
			double dxrel  = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft,tick);

			UT_sint32 xdelta = (m_draggingCenter-xAbsLeft) - m_infoCache.m_xrLeftIndent;
			double dxrel2 = _scalePixelDistanceToUnits(m_infoCache.m_xrFirstLineIndent - xdelta,tick);

			_displayStatusMessage(AP_STRING_ID_LeftIndentTextIndentStatus, tick, dxrel, dxrel2);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;
		
	case DW_LEFTINDENTWITHFIRST:
		{
			// we are dragging the left-indent box with the
			// first-line-indent in sync -- this keeps a constant
			// first-line-indent (since it is stored in relative
			// terms.
			
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xrel = ((UT_sint32)x) - xAbsLeft;
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			UT_sint32 xgridTagAlong = xgrid + m_infoCache.m_xrFirstLineIndent;
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingLeftIndent: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_sint32 oldDragging2Center = m_dragging2Center;
			UT_Rect oldDraggingRect = m_draggingRect;
			UT_Rect oldDragging2Rect = m_dragging2Rect;
			m_draggingCenter  = xAbsLeft + xgrid;
			m_dragging2Center = xAbsLeft + xgridTagAlong;

            UT_sint32 iFirstIndentShift = UT_MAX(0,m_infoCache.m_xrFirstLineIndent);
            UT_sint32 iRightIndentPos = xAbsLeft + m_infoCache.u.c.m_xColumnWidth - m_infoCache.m_xrRightIndent - iFirstIndentShift;

			// Prevent the first-line indent from being dragged off the page
			if (m_dragging2Center < xFixed + m_infoCache.m_xPageViewMargin)
			{
				m_dragging2Center = oldDragging2Center;
				m_draggingCenter = oldDragging2Center;
				if(!m_bEventIgnored)
				{
					_ignoreEvent(UT_FALSE);
					m_bEventIgnored = UT_TRUE;
				}
				return;
			}

            if(iRightIndentPos - m_draggingCenter < m_minColumnWidth)
            {
                m_draggingCenter = iRightIndentPos - m_minColumnWidth;
                m_dragging2Center = m_draggingCenter + xgridTagAlong - xgrid;
            }

			_getParagraphMarkerRects(&m_infoCache,
									 m_draggingCenter,0,m_dragging2Center,
									 &m_draggingRect,NULL,&m_dragging2Rect);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
			{
				draw(&oldDraggingRect,&m_infoCache);
				draw(&oldDragging2Rect,&m_infoCache);
			}
			_drawParagraphProperties(NULL,&m_infoCache,UT_FALSE);
			_xorGuide();

			double dxrel  = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft,tick);

			_displayStatusMessage(AP_STRING_ID_LeftIndentStatus, tick, dxrel);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;
		
	case DW_RIGHTINDENT:
		{
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xAbsRight2 = xAbsLeft + m_infoCache.u.c.m_xColumnWidth;
			UT_sint32 xrel = xAbsRight2 - ((UT_sint32)x);
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingRightIndent: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsRight2 - xgrid;

            UT_sint32 iLeftIndentPos = xAbsLeft + UT_MAX(m_infoCache.m_xrLeftIndent,m_infoCache.m_xrLeftIndent + m_infoCache.m_xrFirstLineIndent);
            if(m_draggingCenter - iLeftIndentPos < m_minColumnWidth)
            {
                m_draggingCenter = iLeftIndentPos + m_minColumnWidth;
            }

			_getParagraphMarkerRects(&m_infoCache,0,m_draggingCenter,0,NULL,&m_draggingRect,NULL);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawParagraphProperties(NULL,&m_infoCache,UT_FALSE);
			_xorGuide();

			double dxrel = _scalePixelDistanceToUnits(xAbsRight2 - m_draggingCenter,tick);
			_displayStatusMessage(AP_STRING_ID_RightIndentStatus, tick, dxrel);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;

	case DW_FIRSTLINEINDENT:
		{
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xrel = ((UT_sint32)x) - xAbsLeft;
			// first-line-indent is relative to the left-indent
			// not the left edge of the column.
			UT_sint32 xrel2 = xrel - m_infoCache.m_xrLeftIndent;
			UT_sint32 xgrid = _snapPixelToGrid(xrel2,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel2,tick);
			UT_DEBUGMSG(("SettingFirstLineIndent: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsLeft + m_infoCache.m_xrLeftIndent + xgrid;

            UT_sint32 iRightIndentPos = xAbsLeft + m_infoCache.u.c.m_xColumnWidth - m_infoCache.m_xrRightIndent;
            if(iRightIndentPos - m_draggingCenter  < m_minColumnWidth)
            {
                m_draggingCenter = iRightIndentPos - m_minColumnWidth;
            }

			_getParagraphMarkerRects(&m_infoCache,0,0,m_draggingCenter,NULL,NULL,&m_draggingRect);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawParagraphProperties(NULL,&m_infoCache,UT_FALSE);
			UT_DEBUGMSG(("FirstLineIndent: r [%ld %ld %ld %ld]]n",
						 m_draggingRect.left,m_draggingRect.top,m_draggingRect.width,m_draggingRect.height));
			_xorGuide();

			double dxrel = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft - m_infoCache.m_xrLeftIndent, tick);

			_displayStatusMessage(AP_STRING_ID_FirstLineIndentStatus, tick, dxrel);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;

	case DW_TABSTOP:
		{
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xrel = ((UT_sint32)x) - xAbsLeft;
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingTabStop: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsLeft + xgrid;
			_getTabStopRect(&m_infoCache,m_draggingCenter,&m_draggingRect);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawTabProperties(NULL,&m_infoCache,UT_FALSE);
			_xorGuide();

			double dxrel = _scalePixelDistanceToUnits(m_draggingCenter - xAbsLeft,tick);
			_displayStatusMessage(AP_STRING_ID_TabStopStatus, tick, dxrel);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

/*****************************************************************/

UT_sint32 AP_TopRuler::_snapPixelToGrid(UT_sint32 xDist, ap_RulerTicks & tick)
{
	// snap pixel value to nearest grid line
	
	UT_sint32 xrel = xDist * tick.tickUnitScale;
	if (xrel > 0)
		xrel =              ((  xrel  + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta / tick.tickUnitScale;
	else
		xrel = -(UT_sint32)((((-xrel) + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta / tick.tickUnitScale);

	return xrel;
}

double AP_TopRuler::_scalePixelDistanceToUnits(UT_sint32 xDist, ap_RulerTicks & tick)
{
	// convert pixel distance to actual dimensioned units

	UT_sint32 xrel = xDist * tick.tickUnitScale;
	if (xrel > 0)
		xrel =              ((  xrel  + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta;
	else
		xrel = -(UT_sint32)((((-xrel) + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta);

	double dxrel = ((double)xrel) / ((double)tick.tickUnitScale);
	return dxrel;
}

double AP_TopRuler::_getUnitsFromRulerLeft(UT_sint32 xColRel, ap_RulerTicks & tick)
{
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xAbsLeft = xFixed + m_infoCache.m_xPageViewMargin - m_xScrollOffset;

	return _scalePixelDistanceToUnits(xColRel - xAbsLeft, tick) * tick.tickUnitScale /  tick.tickUnit * tick.dBasicUnit;
}
	


UT_sint32 AP_TopRuler::_getFirstPixelInColumn(AP_TopRulerInfo * pInfo, UT_uint32 kCol)
{
	// return absolute pixel value for the first pixel in this column.

	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xOrigin = pInfo->u.c.m_xaLeftMargin
		+ kCol * (pInfo->u.c.m_xColumnWidth + pInfo->u.c.m_xColumnGap);
	UT_sint32 xAbsLeft = xFixed + pInfo->m_xPageViewMargin + xOrigin - m_xScrollOffset;

	return xAbsLeft;
}

void AP_TopRuler::_ignoreEvent(UT_Bool bDone)
{
	// user released the mouse off of the ruler.  we need to treat
	// this as a cancel.  so we need to put everything back the
	// way it was on the ruler.

	// clear the guide line

	_xorGuide(UT_TRUE);

	// Clear messages from status bar.

	AP_FrameData * pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	pFrameData->m_pStatusBar->setStatusMessage("");


	// erase the widget that we are dragging.   remember what we
	// are dragging, clear it, and then restore it at the bottom.
	
	DraggingWhat dw = m_draggingWhat;
	m_draggingWhat = DW_NOTHING;

	if (!m_bBeforeFirstMotion || (bDone && (dw==DW_TABSTOP)))
	{
		// erase the widget we are dragging by invalidating
		// the region that's under it and letting it repaint.
		// to avoid flashing, we only do this once.
		draw(&m_draggingRect, &m_infoCache);
		if (dw == DW_LEFTINDENTWITHFIRST)
			draw(&m_dragging2Rect, &m_infoCache);
		m_bBeforeFirstMotion = UT_TRUE;
	}

	// redraw the widget we are dragging at its original location
	
	switch (dw)
	{
	case DW_LEFTMARGIN:
	case DW_RIGHTMARGIN:
		draw(NULL, &m_infoCache);
		break;
		
	case DW_COLUMNGAP:
	case DW_COLUMNGAPLEFTSIDE:
		_drawColumnProperties(NULL,&m_infoCache,0);
		break;
		
	case DW_LEFTINDENT:
	case DW_RIGHTINDENT:
	case DW_FIRSTLINEINDENT:
	case DW_LEFTINDENTWITHFIRST:
		_drawParagraphProperties(NULL,&m_infoCache,UT_TRUE);
		break;
		
	case DW_TABSTOP:
		// tabs are different.  instead of restoring the control when 
		// dragged off, we delete it.  
		//
		// during the drag, this is visually indicated by leaving the 
		// ghost in place (since the delete hasn't happened yet).  
		// the actual delete is done on mouseup.  
		if (bDone)
		{
			// delete the tab
			m_draggingWhat = dw;
			ap_RulerTicks tick(m_pG,m_dim);
			_setTabStops(tick, tr_TABINDEX_NONE, UT_TRUE);
		}
		break;

	case DW_NOTHING:
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	m_draggingWhat = dw;
	return;
}

static void _computeEffects(UT_Bool bFilled,
							GR_Graphics::GR_Color3D & clr3dBorder,
							GR_Graphics::GR_Color3D & clr3dBevel)
{
	if (bFilled)
	{
		clr3dBorder = GR_Graphics::CLR3D_Foreground;
		clr3dBevel =  GR_Graphics::CLR3D_BevelUp;
	}
	else
	{
		clr3dBorder = GR_Graphics::CLR3D_BevelDown;
		clr3dBevel  = GR_Graphics::CLR3D_Background;
	}
}

void AP_TopRuler::_drawLeftIndentMarker(UT_Rect & rect, UT_Bool bFilled)
{
	GR_Graphics::GR_Color3D clr3dBorder, clr3dBevel;
	_computeEffects(bFilled,clr3dBorder,clr3dBevel);
	
	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	// fill in the body
	
	m_pG->setColor3D(GR_Graphics::CLR3D_Background);
	m_pG->drawLine( l+1,   t+13, l+10, t+13);
	m_pG->drawLine( l+2,   t+12, l+10, t+12);
	m_pG->drawLine( l+2,   t+11, l+10, t+11);
	m_pG->drawLine( l+2,   t+10, l+10, t+10);
	m_pG->drawLine( l+9,   t+9,  l+10, t+9 );
	m_pG->drawLine( l+1,   t+7,  l+10, t+7 );
	m_pG->drawLine( l+2,   t+6,  l+10, t+6 );
	m_pG->drawLine( l+2,   t+5,  l+10, t+5 );
	m_pG->drawLine( l+3,   t+4,  l+9,  t+4 );
	m_pG->drawLine( l+4,   t+3,  l+8, t+3 );
	m_pG->drawLine( l+5,   t+2,  l+7, t+2 );

	// draw 3d highlights
	
	m_pG->setColor3D(clr3dBevel);
	m_pG->drawLine( l+5,   t+1,  l,    t+6 );
	m_pG->drawLine( l+1,   t+5,  l+1,  t+7 );
	m_pG->drawLine( l+1,   t+9,  l+9,  t+9 );
	m_pG->drawLine( l+1,   t+9,  l+1,  t+13);

	// draw border
	
	m_pG->setColor3D(clr3dBorder);
	m_pG->drawLine(	l+5,   t,    l+11, t+6 );
	m_pG->drawLine(	l+5,   t,    l- 1, t+6 );
	m_pG->drawLine(	l,     t+5,  l,    t+15);
	m_pG->drawLine(	l+10,  t+5,  l+10, t+15);
	m_pG->drawLine(	l,     t+14, l+11, t+14);
	m_pG->drawLine(	l,     t+8,  l+11, t+8 );

}

void AP_TopRuler::_drawRightIndentMarker(UT_Rect & rect, UT_Bool bFilled)
{
	GR_Graphics::GR_Color3D clr3dBorder, clr3dBevel;
	_computeEffects(bFilled,clr3dBorder,clr3dBevel);

	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	// fill in the body
	
	m_pG->setColor3D(GR_Graphics::CLR3D_Background);
	m_pG->drawLine( l+1,   t+7,  l+10, t+7 );
	m_pG->drawLine( l+2,   t+6,  l+10, t+6 );
	m_pG->drawLine( l+2,   t+5,  l+10, t+5 );
	m_pG->drawLine( l+3,   t+4,  l+9,  t+4 );
	m_pG->drawLine( l+4,   t+3,  l+8, t+3 );
	m_pG->drawLine( l+5,   t+2,  l+7, t+2 );

	// draw 3d highlights
	
	m_pG->setColor3D(clr3dBevel);
	m_pG->drawLine( l+5,   t+1,  l,    t+6 );
	m_pG->drawLine( l+1,   t+5,  l+1,  t+7 );

	// draw border
	
	m_pG->setColor3D(clr3dBorder);
	m_pG->drawLine(	l+5,   t,    l+11, t+6 );
	m_pG->drawLine(	l+5,   t,    l- 1, t+6 );
	m_pG->drawLine(	l,     t+5,  l,    t+9 );
	m_pG->drawLine(	l+10,  t+5,  l+10, t+9 );
	m_pG->drawLine(	l,     t+8,  l+11, t+8 );

}

void AP_TopRuler::_drawFirstLineIndentMarker(UT_Rect & rect, UT_Bool bFilled)
{
	GR_Graphics::GR_Color3D clr3dBorder, clr3dBevel;
	_computeEffects(bFilled,clr3dBorder,clr3dBevel);

	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;

	// fill in the body
	
	m_pG->setColor3D(GR_Graphics::CLR3D_Background);
	m_pG->drawLine( l+9,   t+1,  l+10, t+1 );
	m_pG->drawLine( l+2,   t+2,  l+10, t+2 );
	m_pG->drawLine( l+2,   t+3,  l+10, t+3 );
	m_pG->drawLine( l+3,   t+4,  l+9,  t+4 );
	m_pG->drawLine( l+4,   t+5,  l+8,  t+5 );
	m_pG->drawLine( l+5,   t+6,  l+7,  t+6 );

	// draw 3d highlights
	
	m_pG->setColor3D(clr3dBevel);
	m_pG->drawLine( l+1,   t+1,  l+9,  t+1 );
	m_pG->drawLine( l+1,   t+2,  l+1,  t+4 );
	m_pG->drawLine( l+1,   t+3,  l+6,  t+8 );

	// draw border
	
	m_pG->setColor3D(clr3dBorder);
	m_pG->drawLine(	l+10,  t+3,  l+4,  t+9 );
	m_pG->drawLine(	l,     t+3,  l+6,  t+9 );
	m_pG->drawLine(	l,     t,    l,    t+4 );
	m_pG->drawLine(	l+10,  t,    l+10, t+4 );
	m_pG->drawLine(	l,     t,    l+11, t   );

}

void AP_TopRuler::_drawTabToggle(const UT_Rect * pClipRect, UT_Bool bErase)
{
	UT_Rect rect;
	_getTabToggleRect(&rect);

	if (!pClipRect || rect.intersectsRect(pClipRect) || bErase)
	{
		UT_sint32 l = rect.left;
		UT_sint32 t = rect.top;

		// first draw the frame 

		m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
		m_pG->drawLine( l,    t,    l,    t+16);
		m_pG->drawLine( l,    t+16, l+16, t+16);
		m_pG->drawLine( l+16, t+16, l+16, t);
		m_pG->drawLine( l+16, t,    l,    t);

		m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
		m_pG->drawLine( l+1,  t+1,  l+1,  t+16);
		m_pG->drawLine( l+1,  t+1,  l+16, t+1);
		m_pG->drawLine( l,    t+17, l+17, t+17);

		// now draw the default tab style

		rect.set(l+4, t+6, 10, 9);

		// fill first if needed 

		if (bErase)
			m_pG->fillRect(GR_Graphics::CLR3D_Background, rect);

		if		(m_iDefaultTabType == FL_TAB_LEFT)	rect.left -= 2;
		else if (m_iDefaultTabType == FL_TAB_RIGHT)	rect.left += 2;

		_drawTabStop(rect, m_iDefaultTabType, UT_TRUE);
	}
}

void AP_TopRuler::_drawTabStop(UT_Rect & rect, unsigned char iType, UT_Bool bFilled)
{
	GR_Graphics::GR_Color3D clr3d;
	if (bFilled)
		clr3d = GR_Graphics::CLR3D_Foreground;
	else
		clr3d = GR_Graphics::CLR3D_Background;
	
	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;
	UT_sint32 r = rect.left + rect.width;

	// stroke the vertical first
	m_pG->fillRect(clr3d, l+4,   t,    2,    4);

	if (iType == FL_TAB_DECIMAL)
	{
		// add the dot
		m_pG->fillRect(clr3d, l+7,   t+1,    2,   2);
	}

	// figure out the bottom
	switch (iType)
	{
		case FL_TAB_LEFT:
			l += 4;
			break;

		case FL_TAB_BAR:
			l += 4;
			// fall through

		case FL_TAB_RIGHT:
			r -= 4;
			break;

		case FL_TAB_CENTER:
		case FL_TAB_DECIMAL:
			l += 1;
			r -= 1;
			break;

		default:
			UT_ASSERT(UT_TODO);
			break;
	}

	m_pG->fillRect(clr3d, l,     t+4,  r-l,  2);
}

void AP_TopRuler::_drawColumnGapMarker(UT_Rect & rect)
{
	GR_Graphics::GR_Color3D clr3dBorder, clr3dBevel;
	_computeEffects(UT_TRUE,clr3dBorder,clr3dBevel);
	
	UT_sint32 l = rect.left;
	UT_sint32 t = rect.top;
	UT_sint32 w = rect.width;
	UT_sint32 w2 = w/2 - 1;

	// fill in the body
	
	m_pG->setColor3D(GR_Graphics::CLR3D_Background);
	m_pG->drawLine(l+2,   t+1,  l+w-1,   t+1 );
	m_pG->drawLine(l+2,   t+2,  l+w-1,   t+2 );
	m_pG->drawLine(l+2,   t+3,  l+w-1,   t+3 );
	m_pG->drawLine(l+2,   t+4,  l+w-1,   t+4 );
	m_pG->drawLine(l+2,   t+3,  l+2,     t+8 );
	m_pG->drawLine(l+3,   t+3,  l+3,     t+7 );
	m_pG->drawLine(l+4,   t+3,  l+4,     t+6 );
	m_pG->drawLine(l+w-2, t+3,  l+w-2,   t+9 );
	m_pG->drawLine(l+w-3, t+3,  l+w-3,   t+8 );
	m_pG->drawLine(l+w-4, t+3,  l+w-4,   t+7 );
	m_pG->drawLine(l+w-5, t+3,  l+w-5,   t+6 );

	// draw 3d highlights
	
	m_pG->setColor3D(clr3dBevel);
	m_pG->drawLine(l+1,   t+1,  l+w2,    t+1 );
	m_pG->drawLine(l+w2+1,t+1,  l+w-1,   t+1 );
	m_pG->drawLine(l+1,   t+1,  l+1,     t+10);
	m_pG->drawLine(l+w2+1,t+1,  l+w2+1,  t+5 );
	
	// draw border
	
	m_pG->setColor3D(clr3dBorder);
	m_pG->drawLine(l,     t,    l+w,     t   );
	m_pG->drawLine(l,     t,    l,       t+11);
	m_pG->drawLine(l+w-1, t,    l+w-1,   t+11);
	m_pG->drawLine(l,     t+10, l+5,     t+5);
	m_pG->drawLine(l+w-1, t+10, l+w-6,   t+5);
	m_pG->drawLine(l+5,   t+5,  l+w-5,   t+5);
}

/*static*/ void AP_TopRuler::_prefsListener( XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_AlphaHashTable * /*phChanges*/, void *data )
{
	AP_TopRuler *pTopRuler = (AP_TopRuler *)data;
	UT_ASSERT( data && pPrefs );

	//UT_DEBUGMSG(("AP_TopRuler::_prefsListener (this=%p)\n", data));

	const XML_Char *pszBuffer;
	pPrefs->getPrefsValue( AP_PREF_KEY_RulerUnits, &pszBuffer );

	// or should I just default to inches or something?
	UT_Dimension dim = UT_determineDimension( pszBuffer, DIM_none );
	UT_ASSERT( dim != DIM_none );

	if ( dim != pTopRuler->getDimension() )
		pTopRuler->setDimension( dim );
}

void AP_TopRuler::setDimension( UT_Dimension newdim )
{
	m_dim = newdim;
	draw( (const UT_Rect *)0 );
}

void AP_TopRuler::_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue)
{
	const XML_Char * pText = m_pG->invertDimension(tick.dimType, dValue);
	char temp[100];
	const XML_Char *pzMessageFormat = m_pFrame->getApp()->getStringSet()->getValue(messageID);
	sprintf(temp, pzMessageFormat, pText);

	AP_FrameData * pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	pFrameData->m_pStatusBar->setStatusMessage(temp);
}

void AP_TopRuler::_displayStatusMessage(XAP_String_Id messageID, const ap_RulerTicks &tick, double dValue1, double dValue2)
{
	const XML_Char * pText = m_pG->invertDimension(tick.dimType, dValue1);
	char buf1[100];
	strcpy(buf1, pText);
	pText = m_pG->invertDimension(tick.dimType, dValue2);

	char temp[100];
	const XML_Char *pzMessageFormat = m_pFrame->getApp()->getStringSet()->getValue(messageID);
	sprintf(temp, pzMessageFormat, buf1, pText);

	AP_FrameData * pFrameData = (AP_FrameData *)m_pFrame->getFrameData();
	pFrameData->m_pStatusBar->setStatusMessage(temp);
}
