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
#include "fv_View.h"					// TODO remove this.  we need to
										// TODO add the various pView->...()
										// TODO methods that we use to the
										// TODO the xav_View base class.

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
	m_xScrollLimit = 0;
	m_bValidMouseClick = UT_FALSE;
	m_draggingWhat = DW_NOTHING;
	
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

	UT_RGBColor clrLiteGray(192,192,192);
	m_pG->fillRect(clrLiteGray,0,0,m_iWidth,m_iHeight);

	// draw the foreground
	
	_draw(pClipRect,pUseInfo);
	
	if (pClipRect)
		m_pG->setClipRect(NULL);
}

void AP_TopRuler::_drawBar(const UT_Rect * pClipRect, AP_TopRulerInfo * pInfo,
						   UT_RGBColor &clr, UT_sint32 x, UT_sint32 w)
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
			m_pG->fillRect(clr,r);
	}
	
	return;
}

void AP_TopRuler::_drawTickMark(const UT_Rect * pClipRect,
								AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
								UT_RGBColor &clr, GR_Font * pFont,
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
		
		UT_Rect r(xTick-50,yTop,50,yBar);
		if (!r.intersectsRect(pClipRect))
			return;
	}

	if (k % tick.tickLabel)
	{
		// draw the ticks
		UT_uint32 h = ((k % tick.tickLong) ? 2 : 6);
		UT_uint32 y = yTop + (yBar-h)/2;
		m_pG->setColor(clr);
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
		
void AP_TopRuler::_drawTicks(const UT_Rect * pClipRect,
							 AP_TopRulerInfo * pInfo, ap_RulerTicks &tick,
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
				_drawTickMark(pClipRect,pInfo,tick,clr,pFont,k,xTick);
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
				_drawTickMark(pClipRect,pInfo,tick,clr,pFont,k,xTick);
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

void AP_TopRuler::_getParagraphMarkerRects(AP_TopRulerInfo * pInfo,
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
	UT_sint32 hs = 3;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	if (prLeftIndent)
		prLeftIndent->set(leftCenter - hs, yBottom, fs, fs);
	if (prFirstLineIndent)
		prFirstLineIndent->set(firstLineCenter - hs, yBottom-fs, fs, fs);
	if (prRightIndent)
		prRightIndent->set(rightCenter - hs, yBottom, fs, fs);
}

void AP_TopRuler::_drawParagraphProperties(const UT_Rect * pClipRect,
										   AP_TopRulerInfo * pInfo,
										   UT_RGBColor &clrDark, UT_RGBColor &clrLight,
										   UT_Bool bDrawAll)
{
	UT_sint32 leftCenter, rightCenter, firstLineCenter;
	UT_Rect rLeftIndent, rRightIndent, rFirstLineIndent;

	_getParagraphMarkerXCenters(pInfo,&leftCenter,&rightCenter,&firstLineCenter);
	_getParagraphMarkerRects(pInfo,
							 leftCenter, rightCenter, firstLineCenter,
							 &rLeftIndent, &rRightIndent, &rFirstLineIndent);

	if (m_draggingWhat == DW_LEFTINDENT)
	{
		_drawHollowRect(clrDark,clrLight, rLeftIndent);
		_drawSculptedRect(m_draggingRect);
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rLeftIndent.intersectsRect(pClipRect))
			_drawSculptedRect(rLeftIndent);
	}

	if (m_draggingWhat == DW_RIGHTINDENT)
	{
		_drawHollowRect(clrDark,clrLight, rRightIndent);
		_drawSculptedRect(m_draggingRect);
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rRightIndent.intersectsRect(pClipRect))
			_drawSculptedRect(rRightIndent);
	}
	
	if (m_draggingWhat == DW_FIRSTLINEINDENT)
	{
		_drawHollowRect(clrDark,clrLight, rFirstLineIndent);
		_drawSculptedRect(m_draggingRect);
	}
	else if (bDrawAll)
	{
		if (!pClipRect || rFirstLineIndent.intersectsRect(pClipRect))
			_drawSculptedRect(rFirstLineIndent);
	}
}

/*****************************************************************/

UT_sint32 AP_TopRuler::_getColumnMarkerXCenter(AP_TopRulerInfo * pInfo, UT_uint32 kCol)
{
	// return the right edge of the gap following this column
	// (this is equal to the left edge of the start of the
	// next column)
	
	return _getFirstPixelInColumn(pInfo,kCol+1);
}

void AP_TopRuler::_getColumnMarkerRect(AP_TopRulerInfo * pInfo, UT_uint32 kCol,
									   UT_sint32 xCenter, UT_Rect * prCol)
{
	UT_uint32 yTop = s_iFixedHeight/4;
	UT_sint32 hs = 3;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	prCol->set(xCenter -hs, yTop-hs, fs, fs);
}

void AP_TopRuler::_drawColumnProperties(const UT_Rect * pClipRect,
										AP_TopRulerInfo * pInfo,
										UT_RGBColor &clrDark, UT_RGBColor &clrLight,
										UT_uint32 kCol)
{
	UT_Rect rCol;
	
	_getColumnMarkerRect(pInfo,kCol,_getColumnMarkerXCenter(pInfo,kCol),&rCol);
	if (m_draggingWhat == DW_COLUMNGAP)
	{
		_drawHollowRect(clrDark,clrLight, rCol);
		_drawSculptedRect(m_draggingRect);
	}
	else
	{
		if (!pClipRect || rCol.intersectsRect(pClipRect))
			_drawSculptedRect(rCol);
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
	UT_sint32 xAbsRight = _getFirstPixelInColumn(pInfo,pInfo->m_iNumColumns) + pInfo->u.c.m_xColumnWidth;

	UT_uint32 yTop = s_iFixedHeight/4;
	UT_sint32 hs = 3;					// halfSize
	UT_sint32 fs = hs * 2 + 1;			// fullSize

	rLeft.set( xAbsLeft  -hs, yTop-fs, fs, fs);
	rRight.set(xAbsRight -hs, yTop-fs, fs, fs);
}

void AP_TopRuler::_drawMarginProperties(const UT_Rect * pClipRect,
										AP_TopRulerInfo * pInfo, UT_RGBColor &clr)
{
#if 0
	UT_Rect rLeft, rRight;

	_getMarginMarkerRects(pInfo,rLeft,rRight);
	m_pG->fillRect(clr,rLeft);
	m_pG->fillRect(clr,rRight);
#endif
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
		m_pView->getTopRulerInfo(pInfo);
	}
	
	UT_RGBColor clrDarkGray(127,127,127);
	UT_RGBColor clrBlack(0,0,0);
	UT_RGBColor clrWhite(255,255,255);

	// TODO for now assume we are in column display mode.
	UT_ASSERT(pInfo->m_mode==AP_TopRulerInfo::TRI_MODE_COLUMNS);

	// draw the dark-gray and white bar across the
	// width of the paper.  we adjust the x coords
	// by 1 to keep a light-gray bar between the
	// dark-gray bars (margins & gaps) and the white
	// bars (columns).
	
	// draw a dark-gray bar over the left margin

	_drawBar(pClipRect,pInfo,clrDarkGray,0+1,pInfo->u.c.m_xaLeftMargin-1);
	sum=pInfo->u.c.m_xaLeftMargin;

	for (k=0; k<pInfo->m_iNumColumns; k++)
	{
		// draw white bar over this column
		
		_drawBar(pClipRect,pInfo,clrWhite, sum+1, pInfo->u.c.m_xColumnWidth-1);
		sum += pInfo->u.c.m_xColumnWidth;

		// if another column after this one, draw dark gray-gap
		
		if (k+1 < pInfo->m_iNumColumns)
		{
			_drawBar(pClipRect,pInfo,clrDarkGray, sum+1, pInfo->u.c.m_xColumnGap-1);
			sum += pInfo->u.c.m_xColumnGap;
		}
	}

	// draw dark-gray right margin
	
	_drawBar(pClipRect,pInfo,clrDarkGray,sum+1,pInfo->u.c.m_xaRightMargin-1);

	// now draw tick marks on the bar, using the selected system of units.

	ap_RulerTicks tick(m_pG);
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

	_drawTicks(pClipRect,pInfo,tick,clrBlack,pFont,xTickOrigin, pInfo->u.c.m_xaLeftMargin,sum);
	sum += pInfo->u.c.m_xaLeftMargin;
	
	for (k=0; k<pInfo->m_iNumColumns; k++)
	{
		// draw positive or negative ticks on this column.
		
		if (k < pInfo->m_iCurrentColumn)
			_drawTicks(pClipRect,pInfo,tick,clrBlack,pFont,xTickOrigin, sum+pInfo->u.c.m_xColumnWidth, sum);
		else
			_drawTicks(pClipRect,pInfo,tick,clrBlack,pFont,xTickOrigin, sum, sum+pInfo->u.c.m_xColumnWidth);

		sum += pInfo->u.c.m_xColumnWidth;

		// if another column after this one, skip over the gap
		// (we don't draw ticks on the gap itself).

		if (k+1 < pInfo->m_iNumColumns)
			sum += pInfo->u.c.m_xColumnGap;
	}

	// draw ticks over the right margin

	_drawTicks(pClipRect,pInfo,tick,clrBlack,pFont,xTickOrigin, sum, sum+pInfo->u.c.m_xaRightMargin);

	// draw the various widgets for the:
	// 
	// current section properties {left-margin, right-margin};
	// the current column properties {column-gap};
	// and the current paragraph properties {left-indent, right-indent, first-left-indent}.

	_drawMarginProperties(pClipRect,pInfo,clrBlack);
	if (pInfo->m_iNumColumns > 1)
		_drawColumnProperties(pClipRect,pInfo,clrBlack,clrWhite,0);
	_drawParagraphProperties(pClipRect,pInfo,clrBlack,clrWhite,UT_TRUE);
	
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

#if 0
	UT_sint32 leftMarginCenter, rightMarginCenter;
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
		UT_DEBUGMSG(("hit right margin block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_RIGHTMARGIN;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}
#endif

	if (m_infoCache.m_iNumColumns > 1)
	{
		UT_Rect rCol;
		_getColumnMarkerRect(&m_infoCache,0,_getColumnMarkerXCenter(&m_infoCache,0),&rCol);
		if (rCol.containsPoint(x,y))
		{
			UT_DEBUGMSG(("hit in column gap block\n"));
			m_bValidMouseClick = UT_TRUE;
			m_draggingWhat = DW_COLUMNGAP;
			m_bBeforeFirstMotion = UT_TRUE;
			return;
		}
	}

	UT_sint32 leftIndentCenter, rightIndentCenter, firstLineIndentCenter;
	UT_Rect rLeftIndent, rRightIndent, rFirstLineIndent;
	_getParagraphMarkerXCenters(&m_infoCache,&leftIndentCenter,&rightIndentCenter,&firstLineIndentCenter);
	_getParagraphMarkerRects(&m_infoCache,
							 leftIndentCenter, rightIndentCenter, firstLineIndentCenter,
							 &rLeftIndent, &rRightIndent, &rFirstLineIndent);
	if (rLeftIndent.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit left indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_LEFTINDENT;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}
	if (rRightIndent.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit right indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_RIGHTINDENT;
		m_bBeforeFirstMotion = UT_TRUE;
		return;
	}
	if (rFirstLineIndent.containsPoint(x,y))
	{
		UT_DEBUGMSG(("hit first-line-indent block\n"));
		m_bValidMouseClick = UT_TRUE;
		m_draggingWhat = DW_FIRSTLINEINDENT;
		m_bBeforeFirstMotion = UT_TRUE;
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
		m_draggingWhat = DW_NOTHING;
		return;
	}

	// if they drag horizontall off the ruler, we probably want to ignore
	// the whole thing.  but this may interact with the current scroll.

	UT_uint32 xFixed = (UT_sint32)MyMax(m_iLeftRulerWidth,s_iFixedWidth);
	if ((x < xFixed) || (x > m_iWidth))
	{
		_ignoreEvent();
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
	
	UT_DEBUGMSG(("mouseRelease: [ems 0x%08lx][emb 0x%08lx][x %ld][y %ld]\n",ems,emb,x,y));

	ap_RulerTicks tick(m_pG);

	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_LEFTMARGIN:
	case DW_RIGHTMARGIN:
		m_draggingWhat = DW_NOTHING;
		return;

	case DW_COLUMNGAP:
		{
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xAbsRight = xAbsLeft + m_infoCache.u.c.m_xColumnWidth;
			double dxrel = _scalePixelDistanceToUnits(m_draggingCenter - xAbsRight,tick);

			const XML_Char * properties[3];
			properties[0] = "column-gap";
			properties[1] = m_pG->invertDimension(tick.dimType,dxrel);
			properties[2] = 0;
			UT_DEBUGMSG(("TopRuler: ColumnGap [%s]\n",properties[1]));

			m_draggingWhat = DW_NOTHING;
			(static_cast<FV_View *>(m_pView))->setSectionFormat(properties);
		}
		return;
		
	case DW_LEFTINDENT:
		{
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
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
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
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
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
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

	UT_RGBColor clrBlack(0,0,0);
	UT_RGBColor clrWhite(255,255,255);

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

	// mouse motion was in the ruler portion of the window, we cannot ignore it.
	
	ap_RulerTicks tick(m_pG);

	switch (m_draggingWhat)
	{
	case DW_NOTHING:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
		
	case DW_LEFTMARGIN:
	case DW_RIGHTMARGIN:
		m_bBeforeFirstMotion = UT_FALSE;
		return;

	case DW_COLUMNGAP:
		{
			// TODO what upper/lower bound should we place on this ?
			
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xAbsRight = xAbsLeft + m_infoCache.u.c.m_xColumnWidth;
			UT_sint32 xrel = ((UT_sint32)x) - xAbsRight;
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingColumnGap: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsRight + xgrid;
			_getColumnMarkerRect(&m_infoCache,0,m_draggingCenter,&m_draggingRect);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawColumnProperties(NULL,&m_infoCache,clrBlack,clrWhite,0);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;
		
	case DW_LEFTINDENT:
		{
			// TODO what upper/lower bound should we place on this ?
			
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xrel = ((UT_sint32)x) - xAbsLeft;
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingLeftIndent: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsLeft + xgrid;
			_getParagraphMarkerRects(&m_infoCache,m_draggingCenter,0,0,&m_draggingRect,NULL,NULL);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawParagraphProperties(NULL,&m_infoCache,clrBlack,clrWhite,UT_FALSE);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;
		
	case DW_RIGHTINDENT:
		{
			// TODO what upper/lower bound should we place on this ?
			
			UT_sint32 xAbsLeft = _getFirstPixelInColumn(&m_infoCache,m_infoCache.m_iCurrentColumn);
			UT_sint32 xAbsRight = xAbsLeft + m_infoCache.u.c.m_xColumnWidth;
			UT_sint32 xrel = xAbsRight - ((UT_sint32)x);
			UT_sint32 xgrid = _snapPixelToGrid(xrel,tick);
			double dgrid = _scalePixelDistanceToUnits(xrel,tick);
			UT_DEBUGMSG(("SettingRightIndent: %s\n",m_pG->invertDimension(tick.dimType,dgrid)));
			UT_sint32 oldDraggingCenter = m_draggingCenter;
			UT_Rect oldDraggingRect = m_draggingRect;
			m_draggingCenter = xAbsRight - xgrid;
			_getParagraphMarkerRects(&m_infoCache,0,m_draggingCenter,0,NULL,&m_draggingRect,NULL);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawParagraphProperties(NULL,&m_infoCache,clrBlack,clrWhite,UT_FALSE);
		}
		m_bBeforeFirstMotion = UT_FALSE;
		return;

	case DW_FIRSTLINEINDENT:
		{
			// TODO what upper/lower bound should we place on this ?
			
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
			_getParagraphMarkerRects(&m_infoCache,0,0,m_draggingCenter,NULL,NULL,&m_draggingRect);
			if (!m_bBeforeFirstMotion && (m_draggingCenter != oldDraggingCenter))
				draw(&oldDraggingRect,&m_infoCache);
			_drawParagraphProperties(NULL,&m_infoCache,clrBlack,clrWhite,UT_FALSE);
			UT_DEBUGMSG(("FirstLineIndent: r [%ld %ld %ld %ld]]n",
						 m_draggingRect.left,m_draggingRect.top,m_draggingRect.width,m_draggingRect.height));
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
		xrel =   (( xrel + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta / tick.tickUnitScale;
	else
		xrel = -(((-xrel + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta / tick.tickUnitScale);

	return xrel;
}

double AP_TopRuler::_scalePixelDistanceToUnits(UT_sint32 xDist, ap_RulerTicks & tick)
{
	// convert pixel distance to actual dimensioned units

	UT_sint32 xrel = xDist * tick.tickUnitScale;
	if (xrel > 0)
		xrel =   (( xrel + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta;
	else
		xrel = -(((-xrel + (tick.dragDelta/2) - 1) / tick.dragDelta) * tick.dragDelta);

	double dxrel = ((double)xrel) / ((double)tick.tickUnitScale);
	return dxrel;
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

void AP_TopRuler::_ignoreEvent(void)
{
}

void AP_TopRuler::_drawHollowRect(UT_RGBColor &clrDark, UT_RGBColor &clrLight, UT_Rect &r)
{
	m_pG->fillRect(clrLight, r.left+1, r.top+1, r.width-2, r.height-2);

	UT_Point p[5] = { { r.left,				r.top				},
					  { r.left,				r.top+r.height-1	},
					  { r.left+r.width-1,	r.top+r.height-1	},
					  { r.left+r.width-1,	r.top				},
					  { r.left,				r.top				} };

	m_pG->setLineWidth(1);
	m_pG->setColor(clrDark);
	m_pG->polyLine(p,5);
}

void AP_TopRuler::_drawSculptedRect(UT_Rect &r)
{
	UT_RGBColor clrLiteGray(192,192,192);
	UT_RGBColor clrDarkGray(127,127,127);
	UT_RGBColor clrBlack(0,0,0);
	UT_RGBColor clrWhite(255,255,255);

	_drawHollowRect(clrBlack,clrLiteGray,r);
	m_pG->setColor(clrWhite);
	m_pG->drawLine(r.left+1, r.top+r.height-2, r.left+1, r.top+1);
	m_pG->drawLine(r.left+1, r.top+1, r.left+r.width-2, r.top+1);
	m_pG->setColor(clrDarkGray);
	m_pG->drawLine(r.left+2, r.top+r.height-2, r.left+r.width-2, r.top+r.height-2);
	m_pG->drawLine(r.left+r.width-2, r.top+r.height-2, r.left+r.width-2, r.top+2);
}

