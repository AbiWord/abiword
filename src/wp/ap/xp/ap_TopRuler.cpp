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
#include "ap_TopRuler.h"
#include "xav_View.h"
#include "gr_Graphics.h"
#include "ap_Ruler.h"
class XAP_Frame;
#include "fv_View.h"					// TODO remove this

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))
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
	m_xScrollOffset = 0;
	m_bValidMouseClick = UT_FALSE;
	
	// i wanted these to be "static const x = 32;" in the
	// class declaration, but MSVC5 can't handle it....
	// (GCC can :-)
	
	s_iFixedHeight = 32;
	s_iFixedWidth = 32;
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

	// if the column containing the caret has changed or any
	// properties on the section (like the number of columns
	// or the margins) or on the block (like the paragraph
	// indents), then we redraw the ruler.
	
	if (mask & (AV_CHG_COLUMN | AV_CHG_FMTSECTION | AV_CHG_FMTBLOCK))
		draw(NULL);
	
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
	if (!m_pG)
		return;
	
	if (pClipRect)
		m_pG->setClipRect(pClipRect);

	// draw the background

	UT_RGBColor clrLiteGray(192,192,192);
	m_pG->fillRect(clrLiteGray,0,0,m_iWidth,m_iHeight);

	// draw the foreground
	
	_draw();
	
	if (pClipRect)
		m_pG->setClipRect(NULL);
}

void AP_TopRuler::_drawBar(AP_TopRulerInfo &info, UT_RGBColor &clr, UT_sint32 x, UT_sint32 w)
{
	// Draw ruler bar (white or dark-gray) over [x,x+w)
	// where x is in page-relative coordinates.  we need
	// to compensate for fixed portion, the page-view margin,
	// and the scroll.
	
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);

	// convert page-relative coordinates into absolute coordinates.
	
	UT_sint32 xAbsLeft = xFixed + info.m_xPageViewMargin + x - m_xScrollOffset;
	UT_sint32 xAbsRight = xAbsLeft + w;

	// we need to do our own clipping for the fixed area
	
	if (xAbsLeft < xFixed)			// need to shorten what we draw
		xAbsLeft = xFixed;

	// draw whatever is left

	if (xAbsRight > xAbsLeft)
		m_pG->fillRect(clr,xAbsLeft,yTop,(xAbsRight-xAbsLeft),yBar);

	return;
}

void AP_TopRuler::_drawTickMark(AP_TopRulerInfo &info, ap_RulerTicks &tick,
								UT_RGBColor &clr, GR_Font * pFont,
								UT_sint32 k, UT_sint32 xTick)
{
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;

	if (k % tick.tickLabel)
	{
		// draw the ticks
		UT_uint32 h = ((k % tick.tickLong) ? 2 : 6);
		UT_uint32 y = yTop + (yBar-h)/2;
		m_pG->drawLine(xTick,y,xTick,y+h);
	}
	else if (pFont)
	{
		// draw the number

		m_pG->setFont(pFont);
		UT_uint32 iFontHeight = m_pG->getFontHeight();
		UT_uint32 n = k / tick.tickLabel * tick.tickScale;

		if (n == 0)						// we never draw the zero on the
			return;						// origin
		
		char buf[6];
		UT_UCSChar span[6];
		UT_uint16 charWidths[6];
		UT_ASSERT(n < 10000);

		sprintf(buf, "%ld", n);
		UT_UCS_strcpy_char(span, buf);
		UT_uint32 len = strlen(buf);

		UT_uint32 w = m_pG->measureString(span, 0, len, charWidths);
		UT_uint32 y = yTop + (yBar-iFontHeight)/2;

		m_pG->drawChars(span, 0, len, xTick - w/2, y);
	}
}
		
void AP_TopRuler::_drawTicks(AP_TopRulerInfo &info, ap_RulerTicks &tick,
							 UT_RGBColor &clr, GR_Font * pFont,
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
	
	UT_sint32 xAbsOrigin = xFixed + info.m_xPageViewMargin + xOrigin - m_xScrollOffset;
	UT_sint32 xAbsFrom   = xFixed + info.m_xPageViewMargin + xFrom   - m_xScrollOffset;
	UT_sint32 xAbsTo     = xFixed + info.m_xPageViewMargin + xTo     - m_xScrollOffset;

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
				_drawTickMark(info,tick,clr,pFont,k,xTick);
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
				_drawTickMark(info,tick,clr,pFont,k,xTick);
			k++;
		}
	}
}

/*****************************************************************/
	
void AP_TopRuler::_getParagraphMarkerRects(AP_TopRulerInfo &info, UT_sint32 xOrigin,
										   UT_Rect &rLeftIndent, UT_Rect &rRightIndent, UT_Rect &rFirstLineIndent)
{
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_uint32 yBar = s_iFixedHeight/2;
	UT_uint32 yBottom = yTop + yBar;

	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xAbsLeft = xFixed + info.m_xPageViewMargin + xOrigin - m_xScrollOffset;
	UT_sint32 xAbsRight = xAbsLeft + info.u.c.m_xColumnWidth;

	UT_sint32 hs = 2;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	rLeftIndent.set(xAbsLeft + info.m_xrLeftIndent - hs, yBottom-fs, fs, fs);
	rFirstLineIndent.set(xAbsLeft + info.m_xrLeftIndent + info.m_xrFirstLineIndent - hs, yBottom-2*fs-1, fs, fs);
	rRightIndent.set(xAbsRight - info.m_xrRightIndent - hs, yBottom-fs, fs, fs);
}

void AP_TopRuler::_drawParagraphProperties(AP_TopRulerInfo &info, UT_RGBColor &clr,
										   UT_sint32 xOrigin)
{
	UT_Rect rLeftIndent;
	UT_Rect rRightIndent;
	UT_Rect rFirstLineIndent;

	_getParagraphMarkerRects(info, xOrigin, rLeftIndent, rRightIndent, rFirstLineIndent);
	m_pG->fillRect(clr, rLeftIndent);
	m_pG->fillRect(clr, rRightIndent);
	m_pG->fillRect(clr, rFirstLineIndent);
}

/*****************************************************************/

void AP_TopRuler::_getColumnMarkerRect(AP_TopRulerInfo &info, UT_uint32 kCol, UT_Rect &rCol)
{
	UT_uint32 yTop = s_iFixedHeight/4;

	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xrCol = info.u.c.m_xaLeftMargin
		+ (kCol+1) * (info.u.c.m_xColumnWidth + info.u.c.m_xColumnGap);
	UT_sint32 xAbsLeft = xFixed + info.m_xPageViewMargin + xrCol - m_xScrollOffset;

	UT_sint32 hs = 2;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	rCol.set(xAbsLeft -hs, yTop-fs, fs, fs);
}

void AP_TopRuler::_drawColumnProperties(AP_TopRulerInfo &info, UT_RGBColor &clr, UT_uint32 kCol)
{
	UT_Rect rCol;
	
	_getColumnMarkerRect(info,kCol,rCol);
	m_pG->fillRect(clr,rCol);
}

/*****************************************************************/

void AP_TopRuler::_getMarginMarkerRects(AP_TopRulerInfo &info, UT_Rect &rLeft, UT_Rect &rRight)
{
	UT_uint32 yTop = s_iFixedHeight/4;

	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xAbsLeft  = xFixed + info.m_xPageViewMargin
		+ info.u.c.m_xaLeftMargin  - m_xScrollOffset;

	// we play some games with where the right margin is
	// drawn.  this compensates for an odd pixel roundoff
	// due to our (current) restriction that all columns
	// and gaps are the same size.
	//
	// _Actual is where the margin really should be drawn.
	// _Fake is just past the right edge of the last column.
	
	// UT_sint32 xAbsRight_Actual = xFixed + info.m_xPageViewMargin
	// 	+ info.m_xPaperSize - info.u.c.m_xaRightMargin - m_xScrollOffset;

	UT_sint32 xAbsRight_Fake = xFixed + info.m_xPageViewMargin
		+ info.u.c.m_xaLeftMargin
		+ (info.m_iNumColumns * info.u.c.m_xColumnWidth)
		+ ((info.m_iNumColumns-1) * info.u.c.m_xColumnGap)
		- m_xScrollOffset;

	UT_sint32 xAbsRight = xAbsRight_Fake;

	UT_sint32 hs = 2;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	rLeft.set( xAbsLeft  -hs, yTop-fs, fs, fs);
	rRight.set(xAbsRight -hs, yTop-fs, fs, fs);
}

void AP_TopRuler::_drawMarginProperties(AP_TopRulerInfo &info, UT_RGBColor &clr)
{
	UT_Rect rLeft, rRight;

	_getMarginMarkerRects(info,rLeft,rRight);
	m_pG->fillRect(clr,rLeft);
	m_pG->fillRect(clr,rRight);
}

/*****************************************************************/

void AP_TopRuler::_draw(void)
{
	AP_TopRulerInfo info;
	UT_sint32 sum;
	UT_uint32 k;

	// ask the view for paper/margin/column/table/caret
	// details at the current insertion point.

	m_pView->getTopRulerInfo(&info);
	
	UT_RGBColor clrDarkGray(127,127,127);
	UT_RGBColor clrBlack(0,0,0);
	UT_RGBColor clrWhite(255,255,255);

	// TODO for now assume we are in column display mode.
	UT_ASSERT(info.m_mode==AP_TopRulerInfo::TRI_MODE_COLUMNS);

	// draw the dark-gray and white bar across the
	// width of the paper.  we adjust the x coords
	// by 1 to keep a light-gray bar between the
	// dark-gray bars (margins & gaps) and the white
	// bars (columns).
	
	// draw a dark-gray bar over the left margin

	_drawBar(info,clrDarkGray,0+1,info.u.c.m_xaLeftMargin-1);
	sum=info.u.c.m_xaLeftMargin;

	for (k=0; k<info.m_iNumColumns; k++)
	{
		// draw white bar over this column
		
		_drawBar(info,clrWhite, sum+1, info.u.c.m_xColumnWidth-1);
		sum += info.u.c.m_xColumnWidth;

		// if another column after this one, draw dark gray-gap
		
		if (k+1 < info.m_iNumColumns)
		{
			_drawBar(info,clrDarkGray, sum+1, info.u.c.m_xColumnGap-1);
			sum += info.u.c.m_xColumnGap;
		}
	}

	// draw dark-gray right margin
	
	_drawBar(info,clrDarkGray,sum+1,info.u.c.m_xaRightMargin-1);

	// now draw tick marks on the bar, using the selected system of units.

	ap_RulerTicks tick(m_pG);
	GR_Font * pFont = m_pG->getGUIFont();

	// find the origin for the tick marks.  this is the left-edge of the
	// column that we are in.  everything to the left of this x-value will
	// be drawn on a negative scale to the left relative to here.  everything
	// to the right of this x-value will be drawn on a positive scale to the
	// right.

	UT_sint32 xTickOrigin = info.u.c.m_xaLeftMargin;
	if (info.m_iCurrentColumn > 0)
		xTickOrigin += info.m_iCurrentColumn * (info.u.c.m_xColumnWidth + info.u.c.m_xColumnGap);

	sum = 0;

	// draw negative ticks over left margin.  

	_drawTicks(info,tick,clrBlack,pFont,xTickOrigin, info.u.c.m_xaLeftMargin,sum);
	sum += info.u.c.m_xaLeftMargin;
	
	for (k=0; k<info.m_iNumColumns; k++)
	{
		// draw positive or negative ticks on this column.
		
		if (k < info.m_iCurrentColumn)
			_drawTicks(info,tick,clrBlack,pFont,xTickOrigin, sum+info.u.c.m_xColumnWidth, sum);
		else
			_drawTicks(info,tick,clrBlack,pFont,xTickOrigin, sum, sum+info.u.c.m_xColumnWidth);

		sum += info.u.c.m_xColumnWidth;

		// if another column after this one, skip over the gap
		// (we don't draw ticks on the gap itself).

		if (k+1 < info.m_iNumColumns)
			sum += info.u.c.m_xColumnGap;
	}

	// draw ticks over the right margin

	_drawTicks(info,tick,clrBlack,pFont,xTickOrigin, sum, sum+info.u.c.m_xaRightMargin);

	// draw the various widgets for the:
	// 
	// current section properties {left-margin, right-margin};
	// the current column properties {column-gap};
	// and the current paragraph properties {left-indent, right-indent, first-left-indent}.

	_drawMarginProperties(info,clrBlack);
	if (info.m_iNumColumns > 1)
		_drawColumnProperties(info,clrBlack,0);
	_drawParagraphProperties(info,clrBlack, xTickOrigin);
	
	return;
}

/*****************************************************************/

void AP_TopRuler::mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y)
{
	UT_DEBUGMSG(("mousePress: [ems 0x%08lx][emb 0x%08lx][x %ld][y %ld]\n",ems,emb,x,y));

	// get the complete state of what should be on the ruler at the time of the grab.
	// we assume that nothing in the document can change during our grab unless we
	// change it.

	m_bValidMouseClick = UT_FALSE;
	m_draggingWhat = DW_NOTHING;
	
	m_pView->getTopRulerInfo(&m_infoCache);

	UT_Rect rLeftMargin, rRightMargin, rCol, rLeftIndent, rRightIndent, rFirstLineIndent;

	_getMarginMarkerRects(m_infoCache,rLeftMargin,rRightMargin);
	if (rLeftMargin.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit left margin block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_LEFTMARGIN;
		return;
	}
	if (rRightMargin.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit right margin block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_RIGHTMARGIN;
		return;
	}

	if (m_infoCache.m_iNumColumns > 1)
	{
		_getColumnMarkerRect(m_infoCache,0,rCol);
		if (rCol.containsPoint(x,y))
		{
			UT_DEBUGMSG(("hit in column gap block\n"));
			m_bValidMouseClick = UT_TRUE;
			m_draggingWhat = DW_COLUMNGAP;
			return;
		}
	}

	UT_sint32 xTickOrigin = m_infoCache.u.c.m_xaLeftMargin;
	if (m_infoCache.m_iCurrentColumn > 0)
		xTickOrigin += m_infoCache.m_iCurrentColumn * (m_infoCache.u.c.m_xColumnWidth + m_infoCache.u.c.m_xColumnGap);

	_getParagraphMarkerRects(m_infoCache, xTickOrigin, rLeftIndent, rRightIndent, rFirstLineIndent);
	if (rLeftIndent.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit left indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_LEFTINDENT;
		return;
	}
	if (rRightIndent.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit right indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_RIGHTINDENT;
		return;
	}
	if (rFirstLineIndent.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit first-line-indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_FIRSTLINEINDENT;
		return;
	}

	return;
}

/*****************************************************************/

void AP_TopRuler::mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y)
{
	if (!m_bValidMouseClick)
		return;

	m_bValidMouseClick = UT_FALSE;

	// if they drag vertically off the ruler, we ignore the whole thing.

	if ((y < 0) || (y > m_iHeight))
	{
		_ignoreEvent();
		return;
	}

	// if they drag horizontall off the ruler, we probably want to ignore
	// the whole thing.  but this may interact with the current scroll.

	UT_uint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	if ((x < xFixed) || (x > m_iWidth))
	{
		_ignoreEvent();
		return;
	}

	// mouse up was in the ruler portion of the window, we cannot ignore it.
	
	UT_DEBUGMSG(("mouseRelease: [ems 0x%08lx][emb 0x%08lx][x %ld][y %ld]\n",ems,emb,x,y));

	ap_RulerTicks tick(m_pG);

	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_LEFTMARGIN:
	case DW_RIGHTMARGIN:
	case DW_COLUMNGAP:
		return;
		
	case DW_LEFTINDENT:
		{
			UT_sint32 xrel = _mapLeftIndentToColumnRelative(m_infoCache,x);
			if (xrel > 0)
				xrel = ((xrel * tick.tickUnitScale + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta;
			else
				xrel = ((xrel * tick.tickUnitScale - (tick.dragDelta/2) + 1) / tick.dragDelta) * tick.dragDelta;
			double dxrel = ((double)xrel) / ((double)tick.tickUnitScale);
			const XML_Char * properties[3];
			properties[0] = "margin-left";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: LeftIndent [%s]\n",properties[1]));
			(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
		}
		return;
		
	case DW_RIGHTINDENT:
		{
			UT_sint32 xrel = _mapRightIndentToColumnRelative(m_infoCache,x);
			if (xrel > 0)
				xrel = ((xrel * tick.tickUnitScale + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta;
			else
				xrel = ((xrel * tick.tickUnitScale - (tick.dragDelta/2) + 1) / tick.dragDelta) * tick.dragDelta;
			double dxrel = ((double)xrel) / ((double)tick.tickUnitScale);
			const XML_Char * properties[3];
			properties[0] = "margin-right";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: RightIndent [%s]\n",properties[1]));
			(static_cast<FV_View *>(m_pView))->setBlockFormat(properties);
		}
		return;

	case DW_FIRSTLINEINDENT:
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

/*****************************************************************/

void AP_TopRuler::mouseMotion(EV_EditModifierState ems, UT_uint32 x, UT_uint32 y)
{
	if (!m_bValidMouseClick)
		return;
		
	UT_DEBUGMSG(("mouseMotion: [ems 0x%08lx][x %ld][y %ld]\n",ems,x,y));
}

/*****************************************************************/


UT_sint32 AP_TopRuler::_mapLeftIndentToColumnRelative(AP_TopRulerInfo &info, UT_uint32 x)
{
	// compute page-relative coordinate of the left edge of this column
	
	UT_sint32 xTickOrigin = m_infoCache.u.c.m_xaLeftMargin;
	if (m_infoCache.m_iCurrentColumn > 0)
		xTickOrigin += m_infoCache.m_iCurrentColumn * (m_infoCache.u.c.m_xColumnWidth + m_infoCache.u.c.m_xColumnGap);

	// map page-relative to window absolute
	
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xAbsLeft = xFixed + info.m_xPageViewMargin + xTickOrigin - m_xScrollOffset;

	// return distance from left edge of column
	
	return (x - xAbsLeft);
}

UT_sint32 AP_TopRuler::_mapRightIndentToColumnRelative(AP_TopRulerInfo &info, UT_uint32 x)
{
	// compute page-relative coordinate of the right edge of this column
	
	UT_sint32 xTickOrigin = m_infoCache.u.c.m_xaLeftMargin;
	if (m_infoCache.m_iCurrentColumn > 0)
		xTickOrigin += m_infoCache.m_iCurrentColumn * (m_infoCache.u.c.m_xColumnWidth + m_infoCache.u.c.m_xColumnGap);

	// map page-relative to window absolute
	
	UT_sint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	UT_sint32 xAbsLeft = xFixed + info.m_xPageViewMargin + xTickOrigin - m_xScrollOffset;
	UT_sint32 xAbsRight = xAbsLeft + info.u.c.m_xColumnWidth;

	// return distance from right edge of column
	
	return (xAbsRight - x);
}

void AP_TopRuler::_ignoreEvent(void)
{
}
