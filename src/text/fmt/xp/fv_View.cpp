/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 



#include <stdio.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Page.h"
#include "fp_SectionSlice.h"
#include "fp_Column.h"
#include "fp_BlockSlice.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pd_Document.h"
#include "dg_Graphics.h"
#include "dg_DrawArgs.h"
#include "ie_types.h"


FV_View::FV_View(FL_DocLayout* pLayout)
{
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pG = m_pLayout->getGraphics();
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
	m_xScrollOffset = 0;
	m_yScrollOffset = 0;
	m_iWindowHeight = 0;
	m_iWindowWidth = 0;
	m_iPointHeight = 0;
	m_bPointVisible = UT_FALSE;
	m_bPointEOL = UT_FALSE;
	m_bSelectionVisible = UT_FALSE;
	m_iSelectionAnchor = 0;
	m_bSelection = UT_FALSE;
	m_bPointAP = UT_FALSE;

	pLayout->setView(this);
		
	moveInsPtTo(FV_DOCPOS_BOD);
}

void FV_View::_swapSelectionOrientation(void)
{
	// reverse the direction of the current selection
	// without changing the screen.

	UT_ASSERT(!_isSelectionEmpty());
	PT_DocPosition curPos = _getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	_setPoint(m_iSelectionAnchor);
	m_iSelectionAnchor = curPos;
}
	
void FV_View::_moveToSelectionEnd(UT_Bool bForward)
{
	// move to the requested end of the current selection.
	// NOTE: this must clear the selection.
	
	UT_ASSERT(!_isSelectionEmpty());
	
	PT_DocPosition curPos = _getPoint();
	
	UT_ASSERT(curPos != m_iSelectionAnchor);
	UT_Bool bForwardSelection = (m_iSelectionAnchor < curPos);
	
	if (bForward != bForwardSelection)
	{
		_swapSelectionOrientation();
	}

	_clearSelection();

	return;
}

void FV_View::_clearSelection(void)
{
	_eraseSelection();

	_resetSelection();
}

void FV_View::_resetSelection(void)
{
	m_bSelection = UT_FALSE;
	m_iSelectionAnchor = 0;
}

void FV_View::_eraseSelectionOrInsertionPoint()
{
	if (_isSelectionEmpty())
	{
		_eraseInsertionPoint();
	}
	else
	{
		_eraseSelection();
	}
}

void FV_View::_xorSelection()
{
	UT_ASSERT(!_isSelectionEmpty());

	m_bSelectionVisible = !m_bSelectionVisible;
	
	if (m_iSelectionAnchor < _getPoint())
	{
		invertBetweenPositions(m_iSelectionAnchor, _getPoint());
	}
	else
	{
		invertBetweenPositions(_getPoint(), m_iSelectionAnchor);
	}
}

void FV_View::_eraseSelection(void)
{
	UT_ASSERT(!_isSelectionEmpty());

	if (!m_bSelectionVisible)
	{
		return;
	}

	_xorSelection();
}

void FV_View::_setSelectionAnchor(void)
{
	m_bSelection = UT_TRUE;
	m_iSelectionAnchor = _getPoint();
}

void FV_View::_deleteSelection(void)
{
	// delete the current selection.
	// NOTE: this must clear the selection.

	/*
	  This is a particularly heavy-handed approach to deleting the
	  selection.  But, it seems to work.  We can find a more optimized
	  way later.
	*/
	
	UT_ASSERT(!_isSelectionEmpty());

	_eraseSelection();

	PT_DocPosition iPoint = _getPoint();
	UT_ASSERT(iPoint != m_iSelectionAnchor);
	
	UT_Bool bForward = (iPoint < m_iSelectionAnchor);

	if (bForward)
		m_pDoc->deleteSpan(iPoint, m_iSelectionAnchor);
	else
		m_pDoc->deleteSpan(m_iSelectionAnchor, iPoint);

	_resetSelection();

	return;
}

UT_Bool FV_View::_isSelectionEmpty()
{
	if (!m_bSelection)
	{
		return UT_TRUE;
	}
	
	PT_DocPosition curPos = _getPoint();
	if (curPos == m_iSelectionAnchor)
	{
		return UT_TRUE;
	}

	return UT_FALSE;
}

PT_DocPosition FV_View::_getDocPos(FV_DocPos dp, UT_Bool bKeepLooking)
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	PT_DocPosition iPos;

	// this gets called from ctor, so get out quick
	if (dp == FV_DOCPOS_BOD)
	{
		UT_Bool bRes = m_pDoc->getBounds(UT_FALSE, iPos);
		UT_ASSERT(bRes);

		return iPos;
	}

	PT_DocPosition iPoint = _getPoint();

	// TODO: could cache these to save a lookup if point doesn't change
	fl_BlockLayout* pBlock = _findBlockAtPosition(iPoint);
	fp_Run* pRun = pBlock->findPointCoords(_getPoint(), m_bPointEOL,
										   xPoint, yPoint, iPointHeight);
	fp_Line* pLine = pRun->getLine();

	// be pessimistic
	iPos = iPoint;

	switch (dp)
	{
	case FV_DOCPOS_BOL:
		{
			fp_Run* pFirstRun = pLine->getFirstRun();

			iPos = pFirstRun->getBlockOffset() + pBlock->getPosition();
		}
		break;
	
	case FV_DOCPOS_EOL:
		{
			fp_Run* pLastRun = pLine->getLastRun();

			iPos = pLastRun->getBlockOffset() + pLastRun->getLength() + pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_EOD:
		{
			UT_Bool bRes = m_pDoc->getBounds(UT_TRUE, iPos);
			UT_ASSERT(bRes);
		}
		break;

	case FV_DOCPOS_BOB:
		{
			// are we already there?
			if (iPos == pBlock->getPosition())
			{
				// yep.  is there a prior block?
				if (!pBlock->getPrev())
					break;

				// yep.  look there instead
				pBlock = pBlock->getPrev();
			}

			iPos = pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_EOB:
		{
			if (pBlock->getNext())
			{
				// BOB for next block
				pBlock = pBlock->getNext();
				iPos = pBlock->getPosition();
			}
			else
			{
				// EOD
				UT_Bool bRes = m_pDoc->getBounds(UT_TRUE, iPos);
				UT_ASSERT(bRes);
			}
		}
		break;

	case FV_DOCPOS_BOW:
		{
			UT_GrowBuf pgb;

			UT_Bool bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);

			const UT_UCSChar* pSpan = pgb.getPointer(0);

			UT_uint32 offset = iPos - pBlock->getPosition();
			UT_ASSERT(offset >= 0);
			UT_ASSERT(offset <= pgb.getLength());

			if (offset == 0)
			{
				if (!bKeepLooking)
					break;

				// is there a prior block?
				pBlock = pBlock->getPrev();

				if (!pBlock)
					break;

				// yep.  look there instead
				pgb.truncate(0);
				bRes = pBlock->getBlockBuf(&pgb);
				UT_ASSERT(bRes);

				pSpan = pgb.getPointer(0);
				offset = pgb.getLength();

				if (offset == 0)
				{
					iPos = pBlock->getPosition();
					break;
				}
			}

			UT_Bool bInWord = !UT_isWordDelimiter(pSpan[bKeepLooking ? offset-1 : offset]);

			for (offset--; offset > 0; offset--)
			{
				if (UT_isWordDelimiter(pSpan[offset]))
				{
					if (bInWord)
						break;
				}
				else
					bInWord = UT_TRUE;
			}

			if ((offset > 0) && (offset < pgb.getLength()))
				offset++;

			iPos = offset + pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_EOW:
		{
			UT_GrowBuf pgb;

			UT_Bool bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);

			const UT_UCSChar* pSpan = pgb.getPointer(0);

			UT_uint32 offset = iPos - pBlock->getPosition();
			UT_ASSERT(offset >= 0);
			UT_ASSERT(offset <= pgb.getLength());

			if (offset == pgb.getLength())
			{
				if (!bKeepLooking)
					break;

				// is there a next block?
				pBlock = pBlock->getNext();

				if (!pBlock)
					break;

				// yep.  look there instead
				pgb.truncate(0);
				bRes = pBlock->getBlockBuf(&pgb);
				UT_ASSERT(bRes);

				pSpan = pgb.getPointer(0);
				offset = 0;

				if (pgb.getLength() == 0)
				{
					iPos = pBlock->getPosition();
					break;
				}
			}

			UT_Bool bBetween = UT_isWordDelimiter(pSpan[offset]);

			for (; offset < pgb.getLength(); offset++)
			{
				if (!UT_isWordDelimiter(pSpan[offset]))
				{
					if (bBetween)
						break;
				}
				else
					bBetween = UT_TRUE;
			}

			iPos = offset + pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_BOS: 
	case FV_DOCPOS_EOS:
		UT_ASSERT(UT_TODO);
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	}

	return iPos;
}

void FV_View::moveInsPtTo(FV_DocPos dp)
{
	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	
	PT_DocPosition iPos = _getDocPos(dp);

	_setPoint(iPos, (dp == FV_DOCPOS_EOL));
	_updateInsertionPoint();
}

void FV_View::cmdCharMotion(UT_Bool bForward, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
	}

	PT_DocPosition iPoint = _getPoint();
	if (!_charMotion(bForward, count))
	{
		_setPoint(iPoint);
	}
	else
	{
		_updateInsertionPoint();
	}
}

fl_BlockLayout* FV_View::_findBlockAtPosition(PT_DocPosition pos)
{
	return m_pLayout->findBlockAtPosition(pos);
}

UT_Bool FV_View::cmdCharInsert(UT_UCSChar * text, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	UT_Bool bResult = m_pDoc->insertSpan(_getPoint(), text, count);

	_drawSelectionOrInsertionPoint();

	return bResult;
}

void FV_View::insertParagraphBreak()
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).

	m_pDoc->insertStrux(_getPoint(), PTX_Block);
	
	_drawSelectionOrInsertionPoint();
}

void FV_View::insertCharacterFormatting(const XML_Char * properties[])
{
	_eraseSelectionOrInsertionPoint();

	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!_isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,NULL,properties);

	_drawSelectionOrInsertionPoint();
}

void FV_View::delTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	if (iPos == _getPoint())
		return;

	_extSelToPos(iPos);
	_deleteSelection();
	_drawSelectionOrInsertionPoint();
}

void FV_View::cmdCharDelete(UT_Bool bForward, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();

		UT_uint32 amt = count;
		UT_uint32 posCur = _getPoint();

		if (!bForward)
		{

			if (!_charMotion(bForward,count))
			{
				UT_ASSERT(_getPoint() <= posCur);
				amt = posCur - _getPoint();
			}

			posCur = _getPoint();
		}
		else
		{
			PT_DocPosition posEOD;
			UT_Bool bRes;

			bRes = m_pDoc->getBounds(UT_TRUE, posEOD);
			UT_ASSERT(bRes);
			UT_ASSERT(posCur <= posEOD);

			if (posEOD < (posCur+amt))
			{
				amt = posEOD - posCur;
			}
		}

		if (amt > 0)
			m_pDoc->deleteSpan(posCur, posCur+amt);
	}

	_drawSelectionOrInsertionPoint();
}

void FV_View::_moveInsPtNextPrevLine(UT_Bool bNext)
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	// first, find the line we are on now
	UT_uint32 iOldPoint = _getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(_getPoint(), m_bPointEOL, xPoint, yPoint, iPointHeight);
	fp_Line* pOldLine = pOldRun->getLine();

	fp_Line* pDestLine;
	if (bNext)
	{
		pDestLine = pOldBlock->findNextLineInDocument(pOldLine);
	}
	else
	{
		pDestLine = pOldBlock->findPrevLineInDocument(pOldLine);
	}

	if (pDestLine)
	{
		fl_BlockLayout* pNewBlock = pDestLine->getBlockSlice()->getBlock();
		
		if (bNext)
		{
			UT_ASSERT((pOldBlock != pNewBlock) || (pOldLine->getNext() == pDestLine));
		}
		else
		{
			UT_ASSERT((pOldBlock != pNewBlock) || (pDestLine->getNext() == pOldLine));
		}
	
		// how many characters are we from the front of our current line?
		fp_Run* pFirstRunOnOldLine = pOldLine->getFirstRun();
		PT_DocPosition iFirstPosOnOldLine = pFirstRunOnOldLine->getBlockOffset() + pOldBlock->getPosition();
		UT_ASSERT(iFirstPosOnOldLine <= iOldPoint);
		UT_sint32 iNumChars = _getDataCount(iFirstPosOnOldLine, iOldPoint);
		
		fp_Run* pFirstRunOnNewLine = pDestLine->getFirstRun();
		PT_DocPosition iFirstPosOnNewLine = pFirstRunOnNewLine->getBlockOffset() + pNewBlock->getPosition();
		if (bNext)
		{
			UT_ASSERT((iFirstPosOnNewLine > iOldPoint) || (m_bPointEOL && (iFirstPosOnNewLine == iOldPoint)));
		}
		else
		{
			UT_ASSERT(iFirstPosOnNewLine < iOldPoint);
		}

		UT_uint32 iNumCharsOnNewLine = pDestLine->getNumChars();
		if (iNumChars >= (UT_sint32)iNumCharsOnNewLine)
		{
			iNumChars = iNumCharsOnNewLine;
		}
		
		_setPoint(iFirstPosOnNewLine);
		_charMotion(UT_TRUE, iNumChars);

		// check to see if the run is on the screen, if not bump down/up ...
		fp_Line* pLine = pFirstRunOnNewLine->getLine();
		UT_sint32 xoff, yoff, width, height;
		pLine->getScreenOffsets(pFirstRunOnNewLine,
								pFirstRunOnNewLine->getLineData(), xoff, yoff,
								width, height);
	
		if (yoff < 0)
		{
			yoff *= -1;
			cmdScroll(DG_SCROLLCMD_LINEUP, (UT_uint32) yoff);
		}
		else if (yoff + (UT_sint32)pFirstRunOnNewLine->getHeight() >= height)
			cmdScroll(DG_SCROLLCMD_LINEDOWN, (UT_uint32)(yoff + pFirstRunOnNewLine->getHeight() - height));

	}
	else
	{
		// cannot move.  should we beep?
	}
}

void FV_View::warpInsPtNextPrevLine(UT_Bool bNext)
{
	if (!_isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
	}

	_moveInsPtNextPrevLine(bNext);

	_updateInsertionPoint();
}

void FV_View::extSelNextPrevLine(UT_Bool bNext)
{
	if (_isSelectionEmpty())
	{
		_setSelectionAnchor();
		_moveInsPtNextPrevLine(bNext);
		_drawSelectionOrInsertionPoint();
	}
	else
	{
		PT_DocPosition iOldPoint = _getPoint();
 		_moveInsPtNextPrevLine(bNext);
		PT_DocPosition iNewPoint = _getPoint();

		// top/bottom of doc - nowhere to go
		if (iOldPoint == iNewPoint)
			return;
		
		if (iOldPoint < iNewPoint)
		{
			invertBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			invertBetweenPositions(iNewPoint, iOldPoint);
		}

		if (_isSelectionEmpty())
		{
			_resetSelection();
		}
	}
}

void FV_View::extSelHorizontal(UT_Bool bForward, UT_uint32 count)
{
	if (_isSelectionEmpty())
	{
		_setSelectionAnchor();
		_charMotion(bForward, count);
		_drawSelectionOrInsertionPoint();
	}
	else
	{
		PT_DocPosition iOldPoint = _getPoint();

		if (_charMotion(bForward, count) == UT_FALSE)
		{
			_setPoint(iOldPoint);
			return;
		}
		
		PT_DocPosition iNewPoint = _getPoint();

		if (iOldPoint < iNewPoint)
		{
			invertBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			invertBetweenPositions(iNewPoint, iOldPoint);
		}

		if (_isSelectionEmpty())
		{
			_resetSelection();
		}
	}
}

void FV_View::extSelTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	_extSelToPos(iPos);
}

void FV_View::extSelToXY(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	UT_sint32 yClick = yPos + m_yScrollOffset;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if (yClick < iPageHeight)
		{
			// found it
			break;
		}
		else
		{
			yClick -= iPageHeight;
		}
		pPage = pPage->getNext();
	}

	UT_ASSERT(pPage);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xPos + m_xScrollOffset, yClick, iNewPoint, bBOL, bEOL);

	_extSelToPos(iNewPoint);
}

void FV_View::_extSelToPos(PT_DocPosition iNewPoint)
{
	PT_DocPosition iOldPoint = _getPoint();

	UT_DEBUGMSG(("extSelToPos: iOldPoint=%d  iNewPoint=%d  iSelectionAnchor=%d\n",
				 iOldPoint, iNewPoint, m_iSelectionAnchor));
	
	if (iNewPoint == iOldPoint)
	{
		return;
	}
	
	if (_isSelectionEmpty())
	{
		_setSelectionAnchor();
		m_bSelectionVisible = UT_TRUE;
	}

	/*
	  We need to calculate the differences between the old
	  selection and new one.

	  Anything which was selected, and now is not, should
	  be XORed on screen, back to normal.

	  Anything which was NOT selected, and now is, should
	  be XORed on screen, to show it in selected state.

	  Anything which was selected, and is still selected,
	  should NOT be touched.

	  And, obviously, anything which was not selected, and
	  is still not selected, should not be touched.
	*/

	if (iNewPoint < iOldPoint)
	{
		if (iNewPoint < m_iSelectionAnchor)
		{
			if (iOldPoint < m_iSelectionAnchor)
			{
				/*
				  N O A
				  The selection got bigger.  Both points are
				  left of the anchor.
				*/
				invertBetweenPositions(iNewPoint, iOldPoint);
			}
			else
			{
				/*
				  N A O
				  The selection flipped across the anchor to the left.
				*/
				invertBetweenPositions(iNewPoint, iOldPoint);
			}
		}
		else
		{
			UT_ASSERT(iOldPoint >= m_iSelectionAnchor);

			/*
			  A N O
			  The selection got smaller.  Both points are to the
			  right of the anchor
			*/

			invertBetweenPositions(iNewPoint, iOldPoint);
		}
	}
	else
	{
		UT_ASSERT(iNewPoint > iOldPoint);
			
		if (iNewPoint < m_iSelectionAnchor)
		{
			UT_ASSERT(iOldPoint <= m_iSelectionAnchor);
				
			/*
			  O N A
			  The selection got smaller.  Both points are
			  left of the anchor.
			*/

			invertBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			if (iOldPoint < m_iSelectionAnchor)
			{
				/*
				  O A N
				  The selection flipped across the anchor to the right.
				*/

				invertBetweenPositions(iOldPoint, iNewPoint);
			}
			else
			{
				/*
				  A O N
				  The selection got bigger.  Both points are to the
				  right of the anchor
				*/
				invertBetweenPositions(iOldPoint, iNewPoint);
			}
		}
	}
	
	_setPoint(iNewPoint);
}

void FV_View::warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	UT_sint32 yClick = yPos + m_yScrollOffset;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if (yClick < iPageHeight)
		{
			// found it
			break;
		}
		else
		{
			yClick -= iPageHeight;
		}
		pPage = pPage->getNext();
	}

	UT_ASSERT(pPage);

	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	
	PT_DocPosition pos;
	UT_Bool bBOL, bEOL;
	
	pPage->mapXYToPosition(xPos + m_xScrollOffset, yClick, pos, bBOL, bEOL);
	
	_setPoint(pos, bEOL);
	_updateInsertionPoint();
}

void FV_View::getPageScreenOffsets(fp_Page* pThePage, UT_sint32& xoff,
										 UT_sint32& yoff, UT_sint32& width,
										 UT_sint32& height)
{
	UT_uint32 y = 0;
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight();

		pPage = pPage->getNext();
	}

	yoff = y - m_yScrollOffset;
	xoff = m_xScrollOffset;
	height = m_iWindowHeight;
	width = m_iWindowWidth;
}

void FV_View::getPageYOffset(fp_Page* pThePage, UT_sint32& yoff)
{
	UT_uint32 y = 0;
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight();

		pPage = pPage->getNext();
	}

	yoff = y;
}

/*
  This functionality has moved into the run code.
*/
void FV_View::invertBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2)
{
	UT_ASSERT(iPos1 < iPos2);
	
	fp_Run* pRun1;
	fp_Run* pRun2;

	{
		UT_uint32 x;
		UT_uint32 y;
		UT_uint32 height;
		fl_BlockLayout* pBlock1;
		fl_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, UT_FALSE, x, y, height, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, UT_FALSE, x, y, height, &pBlock2, &pRun2);
	}

	UT_Bool bDone = UT_FALSE;
	fp_Run* pCurRun = pRun1;

	while (!bDone)
	{
		if (pCurRun == pRun2)
		{
			bDone = UT_TRUE;
		}
		
		fl_BlockLayout* pBlock = pCurRun->getBlock();
		UT_ASSERT(pBlock);
		UT_uint32 iBlockBase = pBlock->getPosition();

		UT_uint32 iStart;
		UT_uint32 iLen;
		if (iPos1 > (iBlockBase + pCurRun->getBlockOffset()))
		{
			iStart = iPos1 - iBlockBase;
		}
		else
		{
			iStart = pCurRun->getBlockOffset();
		}
	
		if (iPos2 < (iBlockBase + pCurRun->getBlockOffset() + pCurRun->getLength()))
		{
			iLen = iPos2 - (iStart + iBlockBase);
		}
		else
		{
			iLen = pCurRun->getLength() - iStart + pCurRun->getBlockOffset();
		}

		if (iLen > 0)
			pCurRun->invert(iStart, iLen);

		pCurRun = pCurRun->getNext();
		if (!pCurRun)
		{
			fl_BlockLayout* pNextBlock;
			
			pNextBlock = pBlock->getNext();
			if (pNextBlock)
			{
				pCurRun = pNextBlock->getFirstRun();
			}
		}
	}
}

void FV_View::_findPositionCoords(PT_DocPosition pos,
										UT_Bool bEOL,
										UT_uint32& x,
										UT_uint32& y,
										UT_uint32& height,
										fl_BlockLayout** ppBlock,
										fp_Run** ppRun)
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	UT_ASSERT(pBlock);
	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, iPointHeight);

	// we now have coords relative to the page containing the ins pt
	fp_Page* pPointPage = pRun->getLine()->getBlockSlice()->getColumn()->getSectionSlice()->getPage();

	UT_sint32 iPageOffset;
	getPageYOffset(pPointPage, iPageOffset);
	yPoint += iPageOffset;

	// now, we have coords absolute, as if all pages were stacked vertically
	xPoint -= m_xScrollOffset;
	yPoint -= m_yScrollOffset;

	// now, return the results
	x = xPoint;
	y = yPoint;
	height = iPointHeight;

	if (ppBlock)
	{
		*ppBlock = pBlock;
	}
	
	if (ppRun)
	{
		*ppRun = pRun;
	}
}

void FV_View::_drawSelectionOrInsertionPoint()
{
	if (_isSelectionEmpty())
	{
		_updateInsertionPoint();
	}
	else
	{
		_xorSelection();
	}
}

void FV_View::_updateInsertionPoint()
{
	UT_ASSERT(_isSelectionEmpty());
	
	_eraseInsertionPoint();

	_findPositionCoords(_getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_iPointHeight, NULL, NULL);
	
	_xorInsertionPoint();
}

void FV_View::_xorInsertionPoint()
{
	UT_ASSERT(_isSelectionEmpty());
	
	UT_ASSERT(m_iPointHeight > 0);
	m_bPointVisible = !m_bPointVisible;

	UT_RGBColor clr(255,255,255);
	m_pG->setColor(clr);
	m_pG->xorLine(m_xPoint, m_yPoint, m_xPoint, m_yPoint + m_iPointHeight);
}

void FV_View::_eraseInsertionPoint()
{
	UT_ASSERT(_isSelectionEmpty());
	
	if (!m_bPointVisible)
	{
		return;
	}
	_xorInsertionPoint();
}

void FV_View::setXScrollOffset(UT_sint32 v)
{
	UT_sint32 dx = v - m_xScrollOffset;

	if (dx != 0)
	{
		m_pG->scroll(dx, 0);
	}

	m_xScrollOffset = v;
	
	if (dx > 0)
    {
		if (dx >= m_iWindowWidth)
			draw(0, 0, m_iWindowWidth, m_iWindowHeight);
		else
			draw(m_iWindowWidth - dx, 0, m_iWindowWidth, m_iWindowHeight);
    }
	else
    {
		if (dx <= -m_iWindowWidth)
			draw(0, 0, m_iWindowWidth, m_iWindowHeight);
		else
			draw(0, 0, -dx, m_iWindowHeight);
    }
}

void FV_View::setYScrollOffset(UT_sint32 v)
{
	UT_sint32 dy = v - m_yScrollOffset;
	if (dy != 0)
	{
		m_pG->scroll(0, dy);
	}

	m_yScrollOffset = v;

	if (dy > 0)
    {
		if (dy >= m_iWindowHeight)
			draw(0, 0, m_iWindowWidth, m_iWindowHeight);
		else
			draw(0, m_iWindowHeight - dy, m_iWindowWidth, dy);
    }
	else
    {
		if (dy <= -m_iWindowHeight)
			draw(0, 0, m_iWindowWidth, m_iWindowHeight);
		else
			draw(0, 0, m_iWindowWidth, -dy);
    }
}

void FV_View::setWindowSize(UT_sint32 width, UT_sint32 height)
{
	m_iWindowWidth = width;
	m_iWindowHeight = height;
}

void FV_View::draw(int page, dg_DrawArgs* da)
{
	da->pG = m_pG;
	fp_Page* pPage = m_pLayout->getNthPage(page);
	if (pPage)
		pPage->draw(da);
}

void FV_View::draw()
{
  draw(0, 0, m_iWindowWidth, m_iWindowHeight);
}

void FV_View::draw(UT_sint32 x, UT_sint32 y, UT_sint32 width,
						 UT_sint32 height)
{
	UT_ASSERT(m_iWindowWidth > 0);
	UT_ASSERT(m_iWindowHeight > 0);

	/*
	  We erase the selection before we draw, then we
	  redraw it afterwards.  This causes flicker, but it
	  makes sure that the selection is always drawn in the right
	  place.  What we should actually do is not erase before
	  we draw, then as we scroll things into view, we should
	  draw the selection in the newly shown area, if it's
	  visible there.
	*/
	_eraseSelectionOrInsertionPoint();
	
	UT_sint32 curY = 0;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if ((curY - m_yScrollOffset) > m_iWindowHeight)
		{
#if 0
			UT_DEBUGMSG(("not drawing page A: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d\n",
						 iPageHeight,
						 curY,
						 m_yScrollOffset,
						 m_iWindowHeight));
#endif
		}
		else if ((curY + iPageHeight - m_yScrollOffset) < 0)
		{
#if 0
			UT_DEBUGMSG(("not drawing page B: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d\n",
						 iPageHeight,
						 curY,
						 m_yScrollOffset,
						 m_iWindowHeight));
#endif
		}
		else if ((curY - m_yScrollOffset >= y &&
				  curY - m_yScrollOffset <= y + height) ||
				 (curY - m_yScrollOffset < y &&
				  curY - m_yScrollOffset + iPageHeight > y))

		{
			m_pG->drawLine(0, curY - m_yScrollOffset, m_iWindowWidth, curY - m_yScrollOffset);
			
			dg_DrawArgs da;
			da.pG = m_pG;
			da.xoff = 0;
			da.yoff = curY - m_yScrollOffset;
			da.x = x;
			da.y = y;
			da.width = width;
			da.height = height;

			pPage->draw(&da);
		}
		curY += iPageHeight;

		pPage = pPage->getNext();
	}

	_drawSelectionOrInsertionPoint();
}

// TODO remove this later
#include "ps_Graphics.h"
void FV_View::Test_Dump(void)
{
	static int x = 0;
	char buf[100];

	sprintf(buf,"dump.buffer.%d",x);
	FILE * fpDump = fopen(buf,"w");
	m_pDoc->__dump(fpDump);
	fclose(fpDump);

#ifdef POSTSCRIPT	// Test_Dump
	sprintf(buf,"dump.ps.%d",x);
	PS_Graphics ps(buf,"my_title","AbiWord 0.0");
	FL_DocLayout * pPrintLayout = new FL_DocLayout(m_pLayout->getDocument(),&ps);
	UT_ASSERT(pPrintLayout);
	pPrintLayout->formatAll();
	if (ps.startPrint())
	{
		int width = ps.convertDimension("8.5in");
		int height = ps.convertDimension("11in");
		
		int count = pPrintLayout->countPages();
		for (int k=0; k<count; k++)
			if (ps.startPage("foo",k+1,UT_TRUE,width,height))
			{
				dg_DrawArgs da;
				da.pG = &ps;
				da.width = width;
				da.height = height;
				pPrintLayout->getNthPage(k)->draw(&da);
			}

		UT_Bool bResult = ps.endPrint();
		UT_ASSERT(bResult);
	}

	delete pPrintLayout;
#endif /* POSTSCRIPT */

	x++;
}

void FV_View::cmdScroll(UT_sint32 iScrollCmd, UT_uint32 iPos)
{
	UT_sint32 lineHeight = iPos;
	UT_sint32 docWidth = 0, docHeight = 0;
	
	_xorInsertionPoint();	

	docHeight = m_pLayout->getHeight();
	docWidth = m_pLayout->getWidth();
	
	if (lineHeight == 0)
		lineHeight = 20; // TODO
	
	UT_sint32 yoff = m_yScrollOffset, xoff = m_xScrollOffset;
	
	switch(iScrollCmd)
	{
	case DG_SCROLLCMD_PAGEDOWN:
		yoff += m_iWindowHeight - 20;
		break;
	case DG_SCROLLCMD_PAGEUP:
		yoff -= m_iWindowHeight - 20;
		break;
	case DG_SCROLLCMD_PAGELEFT:
		xoff -= m_iWindowWidth;
		break;
	case DG_SCROLLCMD_PAGERIGHT:
		xoff += m_iWindowWidth;
		break;
	case DG_SCROLLCMD_LINEDOWN:
		yoff += lineHeight;
		break;
	case DG_SCROLLCMD_LINEUP:
		yoff -= lineHeight;
		break;
	case DG_SCROLLCMD_LINELEFT:
		xoff -= lineHeight;
		break;
	case DG_SCROLLCMD_LINERIGHT:
		xoff += lineHeight;
		break;
	case DG_SCROLLCMD_TOTOP:
		yoff = 0;
		break;
	case DG_SCROLLCMD_TOBOTTOM:
		fp_Page* pPage = m_pLayout->getFirstPage();
		UT_sint32 iDocHeight = 0;
		while (pPage)
		{
			iDocHeight += pPage->getHeight();
			pPage = pPage->getNext();
		}
		yoff = iDocHeight;
		break;
	}

	// try and figure out of we really need to scroll
	if (yoff < 0)
	{
		if (m_yScrollOffset == 0) // already at top - forget it
			return;
		
		yoff = 0;
	}

	if (yoff > docHeight)
	{
		if (m_yScrollOffset == docHeight) // all ready at bottom
			return;
		else
			yoff = docHeight;
	}
				
	sendScrollEvent(xoff, yoff);
}

//void FV_View::addScrollListener(void (*pfn)(FV_View*,UT_sint32, UT_sint32))
void FV_View::addScrollListener(FV_ScrollObj* pObj)
{
	m_scrollListeners.addItem((void *)pObj);
}

//void FV_View::removeScrollListener(void (*pfn)(UT_sint32, UT_sint32))
void FV_View::removeScrollListener(FV_ScrollObj* pObj)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		FV_ScrollObj* obj = (FV_ScrollObj*) m_scrollListeners.getNthItem(i);
//		void (*pfn2)(FV_View*,UT_sint32, UT_sint32)  = (void (*)(FV_ScrollObj*,UT_sint32,UT_sint32)) m_scrollListeners.getNthItem(i);

		if (obj == pObj)
		{
			m_scrollListeners.deleteNthItem(i);
			break;
		}
	}
}

void FV_View::sendScrollEvent(UT_sint32 xoff, UT_sint32 yoff)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		FV_ScrollObj* pObj = (FV_ScrollObj*) m_scrollListeners.getNthItem(i);
//		void (*pfn)(FV_View*,UT_sint32, UT_sint32)  = (void ((*)(FV_ScrollObj*,UT_sint32,UT_sint32))) m_scrollListeners.getNthItem(i);

		pObj->m_pfn(pObj->m_pData, xoff, yoff);
//		pfn(this, xoff, yoff);
	}
}

UT_Bool FV_View::isLeftMargin(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	UT_sint32 yClick = yPos + m_yScrollOffset;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if (yClick < iPageHeight)
		{
			// found it
			break;
		}
		else
		{
			yClick -= iPageHeight;
		}
		pPage = pPage->getNext();
	}

	UT_ASSERT(pPage);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xPos + m_xScrollOffset, yClick, iNewPoint, bBOL, bEOL);

	return bBOL;
}

void FV_View::cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd)
{
	warpInsPtToXY(xPos, yPos);

	_eraseInsertionPoint();

	PT_DocPosition iPosLeft = _getDocPos(dpBeg, UT_FALSE);
	PT_DocPosition iPosRight = _getDocPos(dpEnd, UT_FALSE);

	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}

	m_iSelectionAnchor = iPosLeft;
	_setPoint(iPosRight);

	if (iPosLeft == iPosRight)
	{
		_updateInsertionPoint();
		return;
	}

	m_bSelection = UT_TRUE;
	
	_xorSelection();
}

void FV_View::cmdFormatBlock(const XML_Char * properties[])
{
	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!_isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Block);
}

// -------------------------------------------------------------------------
PT_DocPosition FV_View::_getPoint(void)
{
	return m_iInsPoint;
}

void FV_View::_setPoint(PT_DocPosition pt, UT_Bool bEOL)
{
	m_iInsPoint = pt;
	m_bPointEOL = bEOL;
}

UT_Bool FV_View::_isPointAP(void)
{
	return m_bPointAP;
}

PT_AttrPropIndex FV_View::_getPointAP(void)
{
	UT_ASSERT(m_bPointAP);
	return m_apPoint;
}

void FV_View::_setPointAP(PT_AttrPropIndex indexAP)
{
	m_bPointAP = UT_TRUE;
	m_apPoint = indexAP;
}

UT_Bool FV_View::_clearPointAP(UT_Bool bNotify)
{
	if (_isPointAP())
	{
		m_bPointAP = UT_FALSE;

		// notify document that insertion point format is obsolete
		if (bNotify)
			m_pDoc->clearTemporarySpanFmt();
	}

	return UT_TRUE;
}

UT_uint32 FV_View::_getDataCount(UT_uint32 pt1, UT_uint32 pt2)
{
	UT_ASSERT(pt2>=pt1);
	return pt2 - pt1;
}

UT_Bool FV_View::_charMotion(UT_Bool bForward,UT_uint32 countChars)
{
	// advance(backup) the current insertion point by count characters.
	// return UT_FALSE if we ran into an end (or had an error).

	_clearPointAP(UT_TRUE);
	m_bPointEOL = UT_FALSE;
	
	if (bForward)
		m_iInsPoint += countChars;
	else
		m_iInsPoint -= countChars;

	PT_DocPosition posBOD;
	PT_DocPosition posEOD;
	UT_Bool bRes;

	bRes = m_pDoc->getBounds(UT_FALSE, posBOD);
	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint < (UT_sint32) posBOD)
	{
		m_iInsPoint = posBOD;
		return UT_FALSE;
	}

	bRes = m_pDoc->getBounds(UT_TRUE, posEOD);
	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint > (UT_sint32) posEOD)
	{
		m_iInsPoint = posEOD;
		return UT_FALSE;
	}

	return UT_TRUE;
}
// -------------------------------------------------------------------------

void FV_View::cmdUndo(UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	m_pDoc->undoCmd(count);

	_drawSelectionOrInsertionPoint();
}

void FV_View::cmdRedo(UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	m_pDoc->redoCmd(count);

	_drawSelectionOrInsertionPoint();
}

void FV_View::cmdSave(void)
{
	m_pDoc->save(IEFT_AbiWord_1);
}

UT_Bool FV_View::pasteBlock(UT_UCSChar * text, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}	

/* this comment could get really ugly if pasting pages of text */
#ifdef UT_DEBUG
	UT_DEBUGMSG(("pasteBlock: pasting \"%s\" length %d.\n", text, count));
#endif

	UT_Bool bResult = m_pDoc->insertSpan(_getPoint(), text, count);

	_drawSelectionOrInsertionPoint();

	return bResult;
}
