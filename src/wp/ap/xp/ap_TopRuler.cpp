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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_TopRuler.h"
#include "xav_View.h"
#include "gr_Graphics.h"
class XAP_Frame;

#define MyMax(a,b)		(((a)>(b)) ? (a) : (b))
#define DELETEP(p)		do { if (p) delete p; p = NULL; } while (0)

/*****************************************************************/

AP_TopRuler::AP_TopRuler(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_pView = NULL;
	m_pScrollObj = NULL;
	m_pG = NULL;
	m_iHeight = 0;
	m_iWidth = 0;
	m_iLeftRulerWidth = 0;
	m_iPageViewLeftMargin = 0;
	m_xScrollOffset = 0;
}

AP_TopRuler::~AP_TopRuler(void)
{
	DELETEP(m_pScrollObj);
}

/*****************************************************************/

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
	setOffsetPageViewLeftMargin(pView->getPageViewLeftMargin());
	
	// create an AV_ScrollObj to receive send*ScrollEvents()
	
	m_pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	UT_ASSERT(m_pScrollObj);
	m_pView->addScrollListener(m_pScrollObj);

	// Register the TopRuler as a ViewListeners on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

	AV_ListenerId lidTopRuler;
	m_pView->addListener(static_cast<AV_Listener *>(this),&lidTopRuler);

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

void AP_TopRuler::setOffsetPageViewLeftMargin(UT_uint32 iPageViewLeftMargin)
{
	// This gives us the amount of gray-space that the DocumentWindow
	// draws to the left of the paper in the "Page View".  We set the
	// origin of our ruler at this offset from the width of the left
	// ruler.  For "Normal View" this should be zero.

	m_iPageViewLeftMargin = iPageViewLeftMargin;
}

void AP_TopRuler::setHeight(UT_uint32 iHeight)
{
	m_iHeight = iHeight;
}

UT_uint32 AP_TopRuler::getHeight(void) const
{
	return s_iFixedHeight;
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
	UT_DEBUGMSG(("AP_TopRuler::notify [view %p][mask %p]\n",pView,mask));
	return UT_TRUE;
}

/*****************************************************************/

void AP_TopRuler::_scrollFuncX(void * pData, UT_sint32 xoff)
{
	// static callback referenced by an AV_ScrollObj() for the ruler
	UT_ASSERT(pData);

	AP_TopRuler * pTopRuler = (AP_TopRuler *)(pData);

	// let non-static member function do all the work.
	
	pTopRuler->scrollRuler(xoff);
	return;
}

void AP_TopRuler::_scrollFuncY(void * pData, UT_sint32 yoff)
{
	// static callback referenced by an AV_ScrollObj() for the ruler
	// we don't care about vertical scrolling.
	return;
}

/*****************************************************************/

void AP_TopRuler::scrollRuler(UT_sint32 xoff)
{
	// scroll the window while excluding the portion
	// lining up with the LeftRuler.
	
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

void AP_TopRuler::draw(const UT_Rect * pClipRect)
{
	if (pClipRect)
		m_pG->setClipRect(pClipRect);

	UT_RGBColor clrDarkGray(127,127,127);
	UT_RGBColor clrLiteGray(192,192,192);
	UT_RGBColor clrBlack(0,0,0);
	UT_RGBColor clrWhite(255,255,255);

	// draw the background
	
	m_pG->fillRect(clrLiteGray,0,0,m_iWidth,m_iHeight);

	// draw a dark-gray and white bar lined up with the paper
	
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;

	// TODO get these from the document at the current cursor position.
	
	UT_uint32 pageWidth = m_pG->convertDimension("8.5in");
	UT_uint32 docLeftMarginWidth = m_pG->convertDimension("1.0in");
	UT_uint32 docRightMarginWidth = m_pG->convertDimension("1.0in");
	UT_uint32 docWithinMarginWidth = pageWidth - docLeftMarginWidth - docRightMarginWidth;

	// left edge of the paper depends upon size of the LeftRuler
	// (if present) and the width of the gray-space we draw for
	// PageView (when in PageView mode).
	//
	// when we scroll, we fix the area above the LeftRuler, so it
	// is omitted from the calculations.
	
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xOrigin = (UT_sint32)m_iPageViewLeftMargin;
	UT_sint32 xScrolledOrigin = xOrigin - m_xScrollOffset;
	UT_sint32 x,w;

	if ((xScrolledOrigin + docLeftMarginWidth) > 0)
	{
		// left margin of paper is on-screen (or rather -- not
		// scrolled over the LeftRuler).  draw dark-gray bar.
		// we need to clip it ourselves -- since the expose/paint
		// clip rects don't know anything about this distinction.

		x = xFixed;
		w = docLeftMarginWidth;
		if (xScrolledOrigin < 0)
			w += xScrolledOrigin;
		else
			x += xScrolledOrigin;
		if (w > 0)
			m_pG->fillRect(clrDarkGray,x,yTop,w,yBar);
	}

	xScrolledOrigin += docLeftMarginWidth;
	if ((xScrolledOrigin + docWithinMarginWidth) > 0)
	{
		// area within the page margins is on-screen (not over
		// the LeftRuler).  draw a main white bar over the area.

		x = xFixed;
		w = docWithinMarginWidth;
		if (xScrolledOrigin < 0)
			w += xScrolledOrigin;
		else
			x += xScrolledOrigin;
		if (w > 0)
			m_pG->fillRect(clrWhite,x,yTop,w,yBar);
	}

	xScrolledOrigin += docWithinMarginWidth;
	if ((xScrolledOrigin + docRightMarginWidth) > 0)
	{
		// right margin of paper is on-screen (not over the
		// LeftRuler).  draw another dark-gray bar, like we
		// did on the left side.

		x = xFixed;
		w = docRightMarginWidth;
		if (xScrolledOrigin < 0)
			w += xScrolledOrigin;
		else
			x += xScrolledOrigin;
		if (w > 0)
			m_pG->fillRect(clrDarkGray,x,yTop,w,yBar);
	}

	// now draw tick marks on the bar, using the select
	// system of units.
	//
	// For english, we draw long ticks on the inches and
	// short ticks on the half inches.  (we use 5.0in rather
	// then 0.5in to avoid round-off problems.)
	//
	// TODO for now we assume English units.

	m_pG->setColor(clrBlack);
	UT_uint32 halfUnit = m_pG->convertDimension("5.0in");
	for (UT_uint32 k=0; (k*halfUnit/10 <= pageWidth); k++)
	{
		x = xFixed + xOrigin + k*halfUnit/10 - m_xScrollOffset;
		if (x >= xFixed)
		{
			UT_uint32 y = ((k % 2) ? yBar/2 : yBar);
			m_pG->drawLine(x,yTop,x,yTop+y);
		}
	}
	
	if (pClipRect)
		m_pG->setClipRect(NULL);
}

