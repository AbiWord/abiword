/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fv_VisualDragText.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_Graphics.h"
#include "fv_View.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"

FV_VisualDragText::FV_VisualDragText (FV_View * pView)
	: m_pView (pView), 
	  m_iVisualDragMode(FV_VisualDrag_NOT_ACTIVE),
	  m_pDragImage(NULL),
	  m_iLastX(0),
	  m_iLastY(0),
	  m_recCurFrame(0,0,0,0),
	  m_iInitialOffX(0),
	  m_iInitialOffY(0),
	  m_recOrigLeft(0,0,0,0),
	  m_recOrigRight(0,0,0,0)
{
	UT_ASSERT (pView);
}

FV_VisualDragText::~FV_VisualDragText()
{
	DELETEP(m_pDragImage);
}

bool FV_VisualDragText::isActive(void) const
{
	return (FV_VisualDrag_NOT_ACTIVE != m_iVisualDragMode);
}

GR_Graphics * FV_VisualDragText::getGraphics(void) const
{
	return m_pView->getGraphics();
}

void FV_VisualDragText::setMode(FV_VisualDragMode iEditMode)
{
	m_iVisualDragMode = iEditMode;
}

void FV_VisualDragText::mouseDrag(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 dx = 0;
	UT_sint32 dy = 0;
	UT_Rect expX(0,m_recCurFrame.top,0,m_recCurFrame.height);
	UT_Rect expY(m_recCurFrame.left,0,m_recCurFrame.width,0);
	UT_sint32 iext = getGraphics()->tlu(3);
	dx = x - m_iLastX;
	dy = y - m_iLastY;
	m_recCurFrame.left += dx;
	m_recCurFrame.top += dy;
	m_recOrigLeft.left += dx;
	m_recOrigLeft.top += dy;
	m_recOrigRight.left += dx;
	m_recOrigRight.top += dy;
	if(dx < 0)
	{
		expX.left = m_recCurFrame.left+m_recCurFrame.width -iext;
		expX.width = -dx + 2*iext;
		if(dy > 0)
		{
			expX.top -=  iext;
			expX.height += dy + 2*iext;
		}
		else
		{
			expX.top -=  iext;
			expX.height += (-dy + 2*iext);
		}
	}
	else
	{
		expX.left = m_recCurFrame.left - dx - iext;
		expX.width = dx + 2*iext;
		if(dy > 0)
		{
			expX.top -=  iext;
			expX.height += dy + 2*iext;
		}
		else
		{
			expX.top -= iext;
			expX.height += (-dy + 2*iext);
		}
	}
	expY.left -= iext;
	expY.width += 2*iext;
	if(dy < 0)
	{
		expY.top = m_recCurFrame.top + m_recCurFrame.height -iext;
		expY.height = -dy + 2*iext;
	}
	else
	{
		expY.top = m_recCurFrame.top - dy - iext;
		expY.height = dy + 2*iext;
	}

	if(expX.width > 0)
	{
		getGraphics()->setClipRect(&expX);
		m_pView->updateScreen(false);
	}
	if(expY.height > 0)
	{
		getGraphics()->setClipRect(&expY);
		m_pView->updateScreen(false);
	}
	getGraphics()->setClipRect(NULL);
	drawImage();
	if(m_recOrigLeft.width > 0)
	{
		getGraphics()->setClipRect(&m_recOrigLeft);
		m_pView->updateScreen(false);
	}
	if(m_recOrigRight.width > 0)
	{
		getGraphics()->setClipRect(&m_recOrigRight);
		m_pView->updateScreen(false);
	}
	m_iLastX = x;
	m_iLastY = y;
	getGraphics()->setClipRect(NULL);
}

/*!
 * This method creates an image from the current selection. It sets
 * the drag rectangle, the initial offsets and the initial positions 
 * of the cursor.
 */
void FV_VisualDragText::getImageFromSelection(UT_sint32 x, UT_sint32 y)
{
//
// OK first work out the locations in the document of the anchor and point
//	
	PT_DocPosition posLow = m_pView->getSelectionAnchor();
	PT_DocPosition posHigh = 0;
	if(posLow < m_pView->getPoint())
	{
		posHigh = m_pView->getPoint();
	}
	else
	{
		posLow = m_pView->getPoint();
		posHigh = m_pView->getSelectionAnchor();
	}

	fp_Run * pRunLow = NULL;
	UT_sint32 xLow, yLow;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection,bEOL;

	m_pView->_findPositionCoords(posLow, bEOL, xLow, yLow, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunLow);
	fp_Line * pLineLow = pRunLow->getLine();
	fp_Run * pRunHigh = NULL;
	UT_sint32 xHigh, yHigh;
	m_pView->_findPositionCoords(posHigh, bEOL, xHigh, yHigh, xCaret2, yCaret2, heightCaret, bDirection, NULL, &pRunHigh);
	fp_Line * pLineHigh = pRunHigh->getLine();
//
// OK deal with the nice case of the selection just on the single line
//
	if(pLineLow == pLineHigh)
	{
		UT_sint32 xx,yy;
		pLineLow->getScreenOffsets(pRunLow,xx,yy);
		m_recCurFrame.left = xLow;
		m_recCurFrame.width = xHigh - xLow;
		m_recCurFrame.top = yy;
		m_recCurFrame.height = pLineLow->getHeight();
		m_recOrigLeft.width = 0;
		m_recOrigLeft.height = 0;
		m_recOrigLeft.left = 0;
		m_recOrigLeft.top = 0;
		m_recOrigRight.width = 0;
		m_recOrigRight.height = 0;
		m_recOrigRight.left = 0;
		m_recOrigRight.top = 0;
	}
	else
	{
//
// low and high are on different rows. First get top, left
//
		UT_sint32 xx,yy;
		fp_Run * pRun = pLineLow->getFirstRun();
		pLineLow->getScreenOffsets(pRun,xx,yy);
		xx -= pRun->getX();
		xx -= pLineLow->getX();
		m_recOrigLeft.left = xx;
		m_recOrigLeft.width = xLow - xx;
		m_recOrigLeft.top = yy;
		m_recOrigLeft.height = pLineLow->getHeight();
		m_recCurFrame.left = xx;
		m_recCurFrame.top = yy;
		fp_Line * pNext = pLineLow;
		UT_sint32 width = 0;
		while(pNext && (pNext != pLineHigh))
		{
			pRun = pNext->getFirstRun();
			pNext->getScreenOffsets(pRun,xx,yy);
			xx += pNext->getWidth();
			if(xx > width)
			{
				width = xx;
			}
			fp_ContainerObject * pCon = pNext->getNext();
			if(pCon)
			{
				pNext = static_cast<fp_Line *>(pCon);
			}
			else
			{
				fl_BlockLayout * pBL = pNext->getBlock();
				pBL = pBL->getNextBlockInDocument();
				if(pBL)
				{
					pNext = static_cast<fp_Line *>(pBL->getFirstContainer());
				}
				else
				{ 
					pNext = NULL;
				}
			}
		}
		if(pNext == NULL)
		{
			UT_DEBUGMSG(("Last line of selection not found! \n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		pRun = pLineHigh->getFirstRun();
		pLineHigh->getScreenOffsets(pRun,xx,yy);
		yy += pLineHigh->getHeight();
		m_recCurFrame.width = width - m_recCurFrame.left;
		m_recCurFrame.height = yy - m_recCurFrame.top;
		if(m_recCurFrame.top + m_recCurFrame.height > m_pView->getWindowHeight())
		{
			m_recCurFrame.height = m_pView->getWindowHeight() - m_recCurFrame.top;
		}
		m_recOrigRight.left = xHigh;
		m_recOrigRight.width = m_recCurFrame.left + m_recCurFrame.width - xHigh;
		m_recOrigRight.top = yy - pLineHigh->getHeight();
		m_recOrigRight.height = pLineHigh->getHeight();
	}
	m_iLastX = x;
	m_iLastY = y;
	m_iInitialOffX = x - m_recCurFrame.left;
	m_iInitialOffY = y - m_recCurFrame.top;

	DELETEP(m_pDragImage);
	if (m_recCurFrame.width > 0 && m_recCurFrame.height > 0) {
		m_pDragImage = getGraphics()->genImageFromRectangle(m_recCurFrame);
		UT_ASSERT(m_pDragImage);
	}
	else {
		UT_DEBUGMSG(("Dragging Text - different rows have different widths or starting x values. Not yet implemented."));
		UT_ASSERT(UT_TODO);
	}
}

void FV_VisualDragText::mouseCut(UT_sint32 x, UT_sint32 y)
{
	getImageFromSelection(x,y);
	m_pView->cmdCut();
	m_pView->updateScreen(false);
	drawImage();
}

void FV_VisualDragText::mouseCopy(UT_sint32 x, UT_sint32 y)
{
	getImageFromSelection(x,y);
	m_pView->cmdCopy();
	m_pView->updateScreen(false);
	drawImage();
}

/*!
 * x and y is the location in the document windows of the mouse in logical
 * units.
 */
void FV_VisualDragText::mouseRelease(UT_sint32 x, UT_sint32 y)
{
//
// Convert this to a document position and paste!
//
	x -= m_iInitialOffX;
	y -= m_iInitialOffY;
	y += getGraphics()->tlu(6); //Otherwise it's too easy to hit the line above
	x += m_recOrigLeft.width; // Add in offset 
	PT_DocPosition posAtXY = m_pView->getDocPositionFromXY(x,y,false);
	m_pView->setPoint(posAtXY);
	getGraphics()->setClipRect(&m_recCurFrame);
	m_pView->updateScreen(false);
	getGraphics()->setClipRect(NULL);
	m_iVisualDragMode = FV_VisualDrag_NOT_ACTIVE;
	m_pView->getMouseContext(x- m_iInitialOffX,y - m_iInitialOffY);
	m_iInitialOffX = 0;
	m_iInitialOffY = 0;
	m_pView->cmdPaste();
	DELETEP(m_pDragImage);
}

void FV_VisualDragText::drawImage(void)
{
	if(m_pDragImage != NULL)
	{
		getGraphics()->drawImage(m_pDragImage,m_recCurFrame.left,m_recCurFrame.top);
	}
	// or else we already asserted when we failed to grab the image for whatever reason.
	// no need to be redundant here
}
