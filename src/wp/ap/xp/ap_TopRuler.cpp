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

#define DELETEP(p)		do { if (p) delete p; p = NULL; } while (0)

/*****************************************************************/

AP_TopRuler::AP_TopRuler(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_pView = NULL;
	m_pScrollObj = NULL;
	m_pG = NULL;
	m_iHeight = 0;
	m_iLeftRulerWidth = 0;
	m_iPageViewLeftMargin = 0;
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
	return m_iHeight;
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

	// dispatch to platform-dependent scroller
	
	AP_TopRuler * pTopRuler = (AP_TopRuler *)(pData);
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

void AP_TopRuler::draw(const UT_Rect * pClipRect)
{
	if (pClipRect)
		m_pG->setClipRect(pClipRect);

	// TODO remove test draw code...
	
	UT_uint32 pageWidth = 8;			// TODO get this from the document
	
	UT_uint32 dotsPerInch = m_pG->getResolution();
	UT_uint32 xOrigin = m_iLeftRulerWidth + m_iPageViewLeftMargin;
	UT_uint32 yTop = m_iHeight/3;
	UT_uint32 yBar = m_iHeight/3;
	
	UT_RGBColor clrBlack(0,0,0);
	UT_RGBColor clrWhite(255,255,255);

	m_pG->fillRect(clrWhite,xOrigin,yTop,pageWidth*dotsPerInch,yBar);
	m_pG->setColor(clrBlack);
	for (UT_uint32 k=0; k<=pageWidth; k++)
	{
		UT_uint32 x = xOrigin + (k*dotsPerInch);
		m_pG->drawLine(x,yTop,x,yTop+yBar);
	}
	
	m_pG->setColor(clrBlack);
	m_pG->drawLine(0,0,m_iHeight,m_iHeight);
	m_pG->drawLine(m_iHeight-1,m_iHeight-1,200,m_iHeight-1);
	m_pG->drawLine(0,m_iHeight-1,m_iHeight,-1);
	
	if (pClipRect)
		m_pG->setClipRect(NULL);
}

