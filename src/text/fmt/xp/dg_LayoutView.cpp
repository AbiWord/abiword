
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#include <stdio.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "dg_LayoutView.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Page.h"
#include "fp_SectionSlice.h"
#include "fp_Column.h"
#include "fp_BlockSlice.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "dg_DocBuffer.h"
#include "dg_Graphics.h"
#include "dg_DrawArgs.h"

DG_LayoutView::DG_LayoutView(FL_DocLayout* pLayout)
{
	m_pLayout = pLayout;
	m_pBuffer = pLayout->getBuffer();
	m_pG = m_pLayout->getGraphics();
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
	m_xScrollOffset = 0;
	m_yScrollOffset = 0;
	m_iWindowHeight = 0;
	m_iWindowWidth = 0;
	m_iPointHeight = 0;
	m_bPointVisible = UT_FALSE;
	m_bSelectionVisible = UT_FALSE;
	m_iSelectionAnchor = 0;
	m_bSelection = UT_FALSE;

	pLayout->setLayoutView(this);
		
	moveInsPtToBOD();
}

void DG_LayoutView::_swapSelectionOrientation(void)
{
	// reverse the direction of the current selection
	// without changing the screen.

	UT_ASSERT(!_isSelectionEmpty());
	UT_uint32 curPos = _getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	_setPoint(m_iSelectionAnchor);
	m_iSelectionAnchor = curPos;
}
	
void DG_LayoutView::_moveToSelectionEnd(UT_Bool bForward)
{
	// move to the requested end of the current selection.
	// NOTE: this must clear the selection.
	
	UT_ASSERT(!_isSelectionEmpty());
	
	UT_uint32 curPos = _getPoint();
	
	UT_ASSERT(curPos != m_iSelectionAnchor);
	UT_Bool bForwardSelection = (m_iSelectionAnchor < curPos);
	
	// TODO could we just moveAbsolute on this?
	// TODO or consider calling _swapSelectionOrientation
	
	if (bForward != bForwardSelection)
	{
		UT_uint32 countChars = _getDataCount(curPos,m_iSelectionAnchor);
		_charMotion(bForward,countChars);
	}

	_clearSelection();

	return;
}

void DG_LayoutView::_clearSelection(void)
{
	_eraseSelection();

	_resetSelection();
}

void DG_LayoutView::_resetSelection(void)
{
	m_bSelection = UT_FALSE;
	m_iSelectionAnchor = 0;
}

void DG_LayoutView::_eraseSelectionOrInsertionPoint()
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

void DG_LayoutView::_xorSelection()
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

void DG_LayoutView::_eraseSelection(void)
{
	UT_ASSERT(!_isSelectionEmpty());

	if (!m_bSelectionVisible)
	{
		return;
	}

	_xorSelection();
}

void DG_LayoutView::_setSelectionAnchor(void)
{
	m_bSelection = UT_TRUE;
	m_iSelectionAnchor = _getPoint();
}

void DG_LayoutView::_deleteSelection(void)
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

	UT_uint32 iPoint = _getPoint();
	UT_ASSERT(iPoint != m_iSelectionAnchor);
	
	UT_uint32 iCountChars = _getDataCount(iPoint,m_iSelectionAnchor);
	UT_Bool bForward = (iPoint < m_iSelectionAnchor);

	FL_BlockLayout* pBlock1;
	FL_BlockLayout* pBlock2;

	if (bForward)
	{
		pBlock1 = _findBlockWithBufferPosition(iPoint);
		pBlock2 = _findBlockWithBufferPosition(m_iSelectionAnchor);
	}
	else
	{
		pBlock1 = _findBlockWithBufferPosition(m_iSelectionAnchor);
		pBlock2 = _findBlockWithBufferPosition(iPoint);
	}

	m_pBuffer->charDelete(bForward, iCountChars);

	UT_uint32 iMerge = 0;
	FL_BlockLayout* pCurBlock = pBlock1;
	for (;;)
	{
		if (pCurBlock == pBlock2)
		{
			break;
		}
		else
		{
			iMerge++;
			pCurBlock = pCurBlock->getNext();
		}
	}

	for (UT_uint32 i=0; i<iMerge; i++)
	{
		pBlock1->mergeWithNextBlock();
	}
	
	pBlock1->format();
	pBlock1->draw(m_pG);
	
	m_pLayout->reformat();

	_resetSelection();

	return;
}

UT_Bool DG_LayoutView::_isSelectionEmpty()
{
	if (!m_bSelection)
	{
		return UT_TRUE;
	}
	
	UT_uint32 curPos = _getPoint();
	if (curPos == m_iSelectionAnchor)
	{
		return UT_TRUE;
	}

	// TODO because markers are transparent (in a sense), we
	// TODO may need to update this to walk forward and backward
	// TODO as long as we have contiguous markers and see if any
	// TODO any of the marker positions match the current anchor
	// TODO position.

	return UT_FALSE;
}

void DG_LayoutView::moveInsPtToBOD()
{
	UT_uint32 posCur = 0;

	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	
	for (;;)
	{
	    UT_UCSChar ch;
	    DG_DocMarkerId dmid;
	    DG_DocMarker* pMarker = NULL;
		
		UT_Bool bMarker = (m_pBuffer->getOneItem(posCur, &ch, &dmid) == DG_DBPI_MARKER);
		if (bMarker)
		{
			m_pBuffer->inc(posCur);
		}
		else
		{
			_setPoint(posCur);
			break;
		}
	}
}

void DG_LayoutView::moveInsPtToBOL()
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	
	UT_uint32 iPoint = _getPoint();
	FL_BlockLayout* pBlock = _findBlockWithBufferPosition(iPoint);
	FP_Run* pRun = pBlock->findPointCoords(_getPoint(), UT_TRUE,
										   xPoint, yPoint, iPointHeight);
	FP_Line* pLine = pRun->getLine();
	UT_uint32 iPos = pRun->getFirstPosition() + pBlock->getBufferAddress();
	
	_setPoint(iPos);
	m_bInsPointRight = UT_TRUE;
	_updateInsertionPoint();
}

void DG_LayoutView::moveInsPtToEOL()
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}
	
	UT_uint32 iPoint = _getPoint();
	FL_BlockLayout* pBlock = _findBlockWithBufferPosition(iPoint);
	FP_Run* pRun = pBlock->findPointCoords(_getPoint(), UT_TRUE,
										   xPoint, yPoint, iPointHeight);
	FP_Line* pLine = pRun->getLine();

	FP_Run* pLastRun = pLine->getLastRun();

	UT_uint32 iPos = pLastRun->getFirstPosition() + pLastRun->getLength() -	1 + pBlock->getBufferAddress();

	_setPoint(iPos);
	m_bInsPointRight = UT_FALSE;
	_updateInsertionPoint();
}

void DG_LayoutView::cmdCharMotion(UT_Bool bForward, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
	}

	UT_uint32 iPoint = _getPoint();
	if (!_charMotion(bForward, count))
	{
		_setPoint(iPoint);
	}
	else
	{
		m_bInsPointRight = !bForward;
		_updateInsertionPoint();
	}
}

FL_BlockLayout* DG_LayoutView::_findBlockWithBufferPosition(UT_uint32 pos)
{
	UT_uint32 posMarker;
	DG_DocMarkerId idMarker;
	DG_DocMarker* pMarker = NULL;
	DG_DocMarker* pBlockMarker = NULL;

	posMarker = pos;
	for (;;)
	{
		if (m_pBuffer->findMarker(UT_FALSE, posMarker, &posMarker, &idMarker, &pMarker))
		{
			DG_DocMarkerType dmt = pMarker->getType();
			if ((dmt & DG_MT_BLOCK) && !(dmt & DG_MT_END))
			{
				pBlockMarker = pMarker;
				break;
			}
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			
			break;
		}
	}

	if (pBlockMarker)
	{
#if MARKER
		return pBlockMarker->getBlock();
#endif
	}
	else
	{
		return NULL;
	}
}

UT_Bool DG_LayoutView::cmdCharInsert(UT_UCSChar * text, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	FL_BlockLayout* pBlock = _findBlockWithBufferPosition(_getPoint());

	UT_Bool bResult = pBlock->insertData(text, count);

	m_pLayout->reformat();
	
	_drawSelectionOrInsertionPoint();

	return bResult;
}

void DG_LayoutView::insertParagraphBreak()
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}

	FL_BlockLayout* pBlock = _findBlockWithBufferPosition(_getPoint());

	pBlock->insertParagraphBreak();
	m_bInsPointRight = UT_TRUE;

	m_pLayout->reformat();
	
	_drawSelectionOrInsertionPoint();
}

void DG_LayoutView::insertCharacterFormatting(const XML_Char * properties[])
{
	_eraseSelectionOrInsertionPoint();

	if (_insertFormatPair("C",properties))
	{
		UT_uint32 posCur = _getPoint();
		UT_uint32 posStart = posCur;
		UT_uint32 posEnd = posCur;
		if (!_isSelectionEmpty())
		{
			if (m_iSelectionAnchor < posCur)
				posStart = m_iSelectionAnchor;
			else
				posEnd = m_iSelectionAnchor;

			FL_BlockLayout * pBlockStart = _findBlockWithBufferPosition(posStart);
			FL_BlockLayout * pBlockEnd = _findBlockWithBufferPosition(posEnd);
			FL_BlockLayout * pBlock;

			for (pBlock=pBlockStart; (pBlock); pBlock=pBlock->getNext())
			{
				pBlock->format();				// TODO do something less expensive here
				pBlock->draw(m_pLayout->getGraphics());
				if (pBlock == pBlockEnd)
					break;
			}
			
			m_pLayout->reformat();
		}
	}

	_drawSelectionOrInsertionPoint();
}

UT_Bool DG_LayoutView::_insertFormatPair(const XML_Char * szName, const XML_Char * properties[])
{
	// insert an inline formatting pair.
	
	// Find our current position, the block (paragraph) that we are in, and the
	// our immediate container (in the case of an inline marker).
	
	UT_uint32 posCur = _getPoint();
	FL_BlockLayout* pBlock = _findBlockWithBufferPosition(posCur);
	UT_ASSERT(pBlock);

	UT_uint32 dmidCont;
	UT_uint32 posCont;

	UT_Bool bResult = m_pBuffer->findContainerMarker(posCur,&posCont,&dmidCont);
	UT_ASSERT(bResult);
	DG_DocMarker * pdmCont = m_pBuffer->fetchDocMarker(dmidCont);
	UT_ASSERT(pdmCont);
	UT_ASSERT((pdmCont->getType() & (DG_MT_BLOCK|DG_MT_OTHER)));

	if (_isSelectionEmpty())
	{
		// we do not have a selection.  our job is easy.  stick a begin-
		// and end-marker at the current insertion point and then move
		// the insertion point between them.  the idea is that it should
		// look like nothing changed and any new characters typed will have
		// the properties we are given.  (if the user moves away, before
		// typing anything, they won't be able to get the insertion point
		// back in between these.  but that's ok.)

		UT_DEBUGMSG(("LayoutView: before insert\n"));
		pBlock->dump();
		
		DG_DocMarkerId dmid_1;
		if (!pBlock->insertInlineMarker(szName,UT_TRUE,dmidCont,UT_FALSE,&dmid_1))
			return UT_FALSE;
		DG_DocMarker * pdmStart = m_pBuffer->fetchDocMarker(dmid_1);
		UT_ASSERT(pdmStart);
		pdmStart->addProperties(properties);
		
		DG_DocMarkerId dmid_2;
		if (!pBlock->insertInlineMarker(szName,UT_FALSE,dmid_1,UT_TRUE,&dmid_2))
		{
			// oops, we added the start but not the end.
			// TODO we should do a roll-back and delete the start.
			UT_ASSERT(0);
			return UT_FALSE;
		}

		m_pBuffer->warpToMarker(dmid_1,UT_FALSE);

		UT_DEBUGMSG(("LayoutView: after insert\n"));
		pBlock->dump();

		return UT_TRUE;
	}
	else
	{
		// we have a selection

		UT_uint32 posStart, posEnd;
		UT_uint32 posCur = _getPoint();
		UT_Bool bForwardSelection = (m_iSelectionAnchor < posCur);
		if (bForwardSelection)
		{
			posStart = m_iSelectionAnchor;
			posEnd = posCur;
		}
		else
		{
			posStart = posCur;
			posEnd = m_iSelectionAnchor;
		}
		FL_BlockLayout * pBlockStart = _findBlockWithBufferPosition(posStart);
		FL_BlockLayout * pBlockEnd = _findBlockWithBufferPosition(posEnd);
		
		// if the selection is foward-looking [anchor-->point] we need to warp
		// back to the anchor -- because we need to insert the start-marker
		// before we insert the end-marker -- because the end-marker is back-linked
		// to it.
		
		if (bForwardSelection)
			_swapSelectionOrientation();

		UT_uint32 posRememberStart = posStart;
		UT_uint32 posRememberEnd = posEnd;
		pBlock = pBlockStart;
		while (1)
		{
			// insert the start-marker at the current position
			// with dmidCont as the id of the container.
			// remember the id of the start-marker in dmid_1
		
			DG_DocMarkerId dmid_1;
			if (!pBlock->insertInlineMarker(szName,UT_TRUE,dmidCont,UT_FALSE,&dmid_1))
			{
				// TODO oops, we could not insert a marker and we may have swapped
				// TODO the selection, should we put it back ??  (odds are we're
				// TODO about to go down anyway)
				UT_ASSERT(0);
				return UT_FALSE;
			}

			// apply the marker properties to each start-marker that we create.

			DG_DocMarker * pdmStart = m_pBuffer->fetchDocMarker(dmid_1);
			UT_ASSERT(pdmStart);
			pdmStart->addProperties(properties);

			// we currently have [point-->anchor] and we just inserted a marker at
			// point, so we need to increment anchor by the size of a marker so that
			// we will have the same number of characters in the selection.

			posRememberEnd += m_pBuffer->getMarkerSize();

			// to keep proper nesting, we need to stop and restart this marker-pair
			// around other tags already in place.
			// search forward for any marker between here and the end of the selection.
			// if we don't find any before we reach the end of the selection, insert
			// an end-marker and we're done.  otherwise, we need to juggle things a bit.

			UT_uint32 posFound;
			DG_DocMarkerId dmidFound;
			DG_DocMarker * pdmFound;
			UT_Bool bFound = m_pBuffer->findMarker(UT_TRUE,_getPoint(),&posFound,&dmidFound,&pdmFound);
			if ((!bFound) || posFound > posRememberEnd)
			{
				// no more markers (a bogus case) or the next marker is past the end of the selection.
				
				_setPoint(posRememberEnd);
				DG_DocMarkerId dmid_2;
				if (!pBlock->insertInlineMarker(szName,UT_FALSE,dmid_1,UT_FALSE,&dmid_2))
				{
					// oops, we added the start but not the end.
					// TODO we should do a roll-back and delete the start.
					UT_ASSERT(0);
					return UT_FALSE;
				}
				m_iSelectionAnchor = posRememberStart;

				// we now have [anchor-->point].  swap back to [point-->anchor] if
				// we need to.
				if (!bForwardSelection)
					_swapSelectionOrientation();
				return UT_TRUE;
			}
			else
			{
				// there is another marker between here and the end of the selection.
				// insert an end-marker here (to keep nesting intact).
				
				UT_ASSERT(pdmFound);
				_setPoint(posFound);
				DG_DocMarkerId dmid_2;
				if (!pBlock->insertInlineMarker(szName,UT_FALSE,dmid_1,UT_FALSE,&dmid_2))
				{
					// oops, we added the start but not the end.
					// TODO we should do a roll-back and delete the start.
					UT_ASSERT(0);
					return UT_FALSE;
				}

				// adjust end-of-selection again because we inserted a marker.
				
				posRememberEnd += m_pBuffer->getMarkerSize();

				// now we need to skip over the marker that we found and then possibly
				// insert another start/end pair.  actually, we need to skip over any
				// contiguous run of markers. we update the container marker id and
				// let the main loop run again.  if the marker sequence runs into another
				// block (paragraph), we need to advance the block pointer.

				UT_UCSChar ch;
				DG_DocMarkerId dmidXX;
				DG_DocMarkerId dmidXXLast = 0;
				UT_uint32 point = _getPoint();
				while (m_pBuffer->getOneItem(point,&ch,&dmidXX)==DG_DBPI_MARKER)
				{
					dmidXXLast = dmidXX;
					m_pBuffer->inc(point);
				}
				_setPoint(point);
				FL_BlockLayout * pBlockXX = _findBlockWithBufferPosition(point);
				UT_ASSERT(pBlockXX);
				if (pBlockXX != pBlock)
					pBlock = pBlockXX;
				if (point > posRememberEnd)
				{
					// this case may not be possible.  if the sequence of contiguous
					// markers extends beyond our current selection end, then we don't
					// need to start another pair.  we just backup to the actual end
					// of the selection and finish up.
					
					_setPoint(posRememberEnd);
					m_iSelectionAnchor = posRememberStart;

					// we now have [anchor-->point].  swap back to [point-->anchor] if
					// we need to.
					if (!bForwardSelection)
						_swapSelectionOrientation();
					return UT_TRUE;
				}

				// take the last marker in the contiguous sequence.
				// if it is a start marker, we remember it as the current
				// container.  if it is an end marker, we get parent of the
				// corresponding start marker as the current container.
				
				UT_ASSERT(dmidXXLast);
				DG_DocMarker * pdmXX = m_pBuffer->fetchDocMarker(dmidXXLast);
				UT_ASSERT(pdmXX);
				if (pdmXX->getType() & DG_MT_END)
					pdmXX = pdmXX->getParent(&dmidCont);
				else
					dmidCont = dmidXXLast;
			}
		}
		UT_ASSERT(0);					// not reached
		return UT_FALSE;
	}
}

void DG_LayoutView::cmdCharDelete(UT_Bool bForward, UT_uint32 count)
{
	if (!_isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();

		UT_uint32 iPoint = _getPoint();
		
		FL_BlockLayout* pBlock = _findBlockWithBufferPosition(iPoint);
		if (pBlock->cmdCharDelete(bForward, count))
		{
			FL_BlockLayout* pBLK = pBlock->getPrev();
			if (pBLK)
			{
				pBLK->mergeWithNextBlock();
				pBLK->format();
				pBLK->draw(m_pLayout->getGraphics());
			}
		}
	}

	_drawSelectionOrInsertionPoint();
}

void DG_LayoutView::_moveInsPtNextPrevLine(UT_Bool bNext)
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	// first, find the line we are on now
	UT_uint32 iOldPoint = _getPoint();
	FL_BlockLayout* pOldBlock = _findBlockWithBufferPosition(iOldPoint);
	FP_Run* pOldRun = pOldBlock->findPointCoords(_getPoint(), UT_TRUE, xPoint, yPoint, iPointHeight);
	FP_Line* pOldLine = pOldRun->getLine();

	FP_Line* pDestLine;
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
		FL_BlockLayout* pNewBlock = pDestLine->getBlockSlice()->getBlock();
		
		if (bNext)
		{
			UT_ASSERT((pOldBlock != pNewBlock) || (pOldLine->getNext() == pDestLine));
		}
		else
		{
			UT_ASSERT((pOldBlock != pNewBlock) || (pDestLine->getNext() == pOldLine));
		}
	
		// how many characters are we from the front of our current line?
		FP_Run* pFirstRunOnOldLine = pOldLine->getFirstRun();
		UT_uint32 iFirstPosOnOldLine = pFirstRunOnOldLine->getFirstPosition() + pOldBlock->getBufferAddress();
		UT_ASSERT(iFirstPosOnOldLine <= iOldPoint);
		UT_sint32 iNumChars = _getDataCount(iFirstPosOnOldLine, iOldPoint);
		
		FP_Run* pFirstRunOnNewLine = pDestLine->getFirstRun();
		UT_uint32 iFirstPosOnNewLine = pFirstRunOnNewLine->getFirstPosition() + pNewBlock->getBufferAddress();
		if (bNext)
		{
			UT_ASSERT(iFirstPosOnNewLine > iOldPoint);
		}
		else
		{
			UT_ASSERT(iFirstPosOnNewLine < iOldPoint);
		}
		_setPoint(iFirstPosOnNewLine);
		UT_uint32 iNumCharsOnNewLine = pDestLine->getNumChars();
		if (iNumChars >= (UT_sint32)iNumCharsOnNewLine)
		{
			iNumChars = iNumCharsOnNewLine - 1;
		}
		
		_setPoint(iFirstPosOnNewLine);
		_charMotion(UT_TRUE, iNumChars);

		// check to see if the run is on the screen, if not bump down/up ...
		FP_Line* pLine = pFirstRunOnNewLine->getLine();
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

void DG_LayoutView::warpInsPtNextPrevLine(UT_Bool bNext)
{
		if (!_isSelectionEmpty())
		{
			_moveToSelectionEnd(bNext);
		}

		_moveInsPtNextPrevLine(bNext);

		_updateInsertionPoint();
}

void DG_LayoutView::extSelNextPrevLine(UT_Bool bNext)
{
	if (_isSelectionEmpty())
	{
		_setSelectionAnchor();
		_moveInsPtNextPrevLine(bNext);
		_drawSelectionOrInsertionPoint();
	}
	else
	{
		UT_uint32 iOldPoint = _getPoint();
 		_moveInsPtNextPrevLine(bNext);
		UT_uint32 iNewPoint = _getPoint();

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

void DG_LayoutView::extSelHorizontal(UT_Bool bForward, UT_uint32 count)
{
	if (_isSelectionEmpty())
	{
		_setSelectionAnchor();
		_charMotion(bForward, count);
		_drawSelectionOrInsertionPoint();
	}
	else
	{
		UT_uint32 iOldPoint = _getPoint();

		if (_charMotion(bForward, count) == UT_FALSE)
		{
			_setPoint(iOldPoint);
			return;
		}
		
		UT_uint32 iNewPoint = _getPoint();

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

void DG_LayoutView::extSelToXY(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	UT_sint32 yClick = yPos + m_yScrollOffset;
	FP_Page* pPage = m_pLayout->getFirstPage();
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

	UT_uint32 iOldPoint = _getPoint();
	
	UT_uint32 iNewPoint;
	UT_Bool bRight;
	pPage->mapXYToBufferPosition(xPos + m_xScrollOffset, yClick, iNewPoint, bRight);

	UT_DEBUGMSG(("extToXY: iOldPoint=%d  iNewPoint=%d  iSelectionAnchor=%d\n",
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

void DG_LayoutView::warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/

	UT_sint32 yClick = yPos + m_yScrollOffset;
	FP_Page* pPage = m_pLayout->getFirstPage();
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
	
	UT_uint32 pos;
	UT_Bool bRight;
	
	pPage->mapXYToBufferPosition(xPos + m_xScrollOffset, yClick, pos, bRight);
	
	_setPoint(pos);
	m_bInsPointRight = bRight;

	_updateInsertionPoint();
}

void DG_LayoutView::getPageScreenOffsets(FP_Page* pThePage, UT_sint32& xoff,
										 UT_sint32& yoff, UT_sint32& width,
										 UT_sint32& height)
{
	UT_uint32 y = 0;
	
	FP_Page* pPage = m_pLayout->getFirstPage();
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

void DG_LayoutView::getPageYOffset(FP_Page* pThePage, UT_sint32& yoff)
{
	UT_uint32 y = 0;
	
	FP_Page* pPage = m_pLayout->getFirstPage();
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
void DG_LayoutView::invertBetweenPositions(UT_uint32 iPos1, UT_uint32 iPos2)
{
	UT_ASSERT(iPos1 < iPos2);
	
	FP_Run* pRun1;
	FP_Run* pRun2;

	{
		UT_uint32 x;
		UT_uint32 y;
		UT_uint32 height;
		FL_BlockLayout* pBlock1;
		FL_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, UT_TRUE, x, y, height, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, UT_FALSE, x, y, height, &pBlock2, &pRun2);
	}

	UT_Bool bDone = UT_FALSE;
	FP_Run* pCurRun = pRun1;

	while (!bDone)
	{
		if (pCurRun == pRun2)
		{
			bDone = UT_TRUE;
		}
		
		FL_BlockLayout* pBlock = pCurRun->getBlock();
		UT_ASSERT(pBlock);
		UT_uint32 iBlockBase = pBlock->getBufferAddress();

		UT_uint32 iStart;
		UT_uint32 iLen;
		if (iPos1 > (iBlockBase + pCurRun->getFirstPosition()))
		{
			iStart = iPos1 - iBlockBase;
		}
		else
		{
			iStart = pCurRun->getFirstPosition();
		}
	
		if (iPos2 < (iBlockBase + pCurRun->getFirstPosition() + pCurRun->getLength()))
		{
			iLen = iPos2 - (iStart + iBlockBase);
		}
		else
		{
			iLen = pCurRun->getLength() - iStart + pCurRun->getFirstPosition();
		}

		pCurRun->invert(iStart, iLen);

		pCurRun = pCurRun->getNext();
		if (!pCurRun)
		{
			FL_BlockLayout* pNextBlock;
			
			pNextBlock = pBlock->getNext();
			if (pNextBlock)
			{
				pCurRun = pNextBlock->getFirstRun();
			}
		}
	}
}

void DG_LayoutView::_findPositionCoords(UT_uint32 pos,
										UT_Bool bRight,
										UT_uint32& x,
										UT_uint32& y,
										UT_uint32& height,
										FL_BlockLayout** ppBlock,
										FP_Run** ppRun)
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;
	
	FL_BlockLayout* pBlock = _findBlockWithBufferPosition(pos);
	UT_ASSERT(pBlock);
	FP_Run* pRun = pBlock->findPointCoords(pos, bRight, xPoint, yPoint, iPointHeight);

	// we now have coords relative to the page containing the ins pt
	FP_Page* pPointPage = pRun->getLine()->getBlockSlice()->getColumn()->getSectionSlice()->getPage();

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

void DG_LayoutView::_drawSelectionOrInsertionPoint()
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

void DG_LayoutView::_updateInsertionPoint()
{
	UT_ASSERT(_isSelectionEmpty());
	
	_eraseInsertionPoint();

	_findPositionCoords(_getPoint(), m_bInsPointRight, m_xPoint, m_yPoint, m_iPointHeight, NULL, NULL);
	
	_xorInsertionPoint();
}

void DG_LayoutView::_xorInsertionPoint()
{
	UT_ASSERT(_isSelectionEmpty());
	
	UT_ASSERT(m_iPointHeight > 0);
	m_bPointVisible = !m_bPointVisible;

	UT_RGBColor clr(255,255,255);
	m_pG->setColor(clr);
	m_pG->xorLine(m_xPoint, m_yPoint, m_xPoint, m_yPoint + m_iPointHeight);
}

void DG_LayoutView::_eraseInsertionPoint()
{
	UT_ASSERT(_isSelectionEmpty());
	
	if (!m_bPointVisible)
	{
		return;
	}
	_xorInsertionPoint();
}

void DG_LayoutView::setXScrollOffset(UT_sint32 v)
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

void DG_LayoutView::setYScrollOffset(UT_sint32 v)
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

void DG_LayoutView::setWindowSize(UT_sint32 width, UT_sint32 height)
{
	m_iWindowWidth = width;
	m_iWindowHeight = height;
}

void DG_LayoutView::draw()
{
  draw(0, 0, m_iWindowWidth, m_iWindowHeight);
}

void DG_LayoutView::draw(UT_sint32 x, UT_sint32 y, UT_sint32 width,
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
	FP_Page* pPage = m_pLayout->getFirstPage();
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
			UT_uint32 iPoint = _getPoint();

			pPage->draw(&da);
		}
		curY += iPageHeight;

		pPage = pPage->getNext();
	}

	_drawSelectionOrInsertionPoint();
}

// TODO remove this later
#include "rw_DocWriter.h"
#include "ps_Graphics.h"
void DG_LayoutView::Test_Dump(void)
{
	static int x = 0;
	char buf[100];
	sprintf(buf,"dump.buffer.%d",x);
	
	m_pBuffer->dumpBuffer(buf);

	sprintf(buf,"dump.DocWriter.%d",x);

	RW_DocWriter dw(m_pLayout->getDocument());
	dw.writeFile(buf);

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

	x++;
}

void DG_LayoutView::cmdScroll(UT_sint32 iScrollCmd, UT_uint32 iPos)
{
	UT_sint32 lineHeight = iPos;

	_xorInsertionPoint();	

	if (lineHeight == 0)
		lineHeight = 20; // TODO
	
	switch(iScrollCmd)
	{
	case DG_SCROLLCMD_PAGEDOWN:
		sendScrollEvent(m_xScrollOffset, m_yScrollOffset + m_iWindowHeight - 20);
		break;
	case DG_SCROLLCMD_PAGEUP:
		sendScrollEvent(m_xScrollOffset, m_yScrollOffset - m_iWindowHeight + 20);
		break;
	case DG_SCROLLCMD_PAGELEFT:
		sendScrollEvent(m_xScrollOffset - m_iWindowWidth, m_yScrollOffset);
		break;
	case DG_SCROLLCMD_PAGERIGHT:
		sendScrollEvent(m_xScrollOffset + m_iWindowWidth, m_yScrollOffset);
		break;
	case DG_SCROLLCMD_LINEDOWN:
		sendScrollEvent(m_xScrollOffset, m_yScrollOffset + lineHeight);
		break;
	case DG_SCROLLCMD_LINEUP:
		sendScrollEvent(m_xScrollOffset, m_yScrollOffset - lineHeight); 
		break;
	case DG_SCROLLCMD_LINELEFT:
		sendScrollEvent(m_xScrollOffset - lineHeight, m_yScrollOffset);
		break;
	case DG_SCROLLCMD_LINERIGHT:
		sendScrollEvent(m_xScrollOffset + lineHeight, m_yScrollOffset);
		break;
	case DG_SCROLLCMD_TOTOP:
		sendScrollEvent(m_xScrollOffset, 0);
		break;
	case DG_SCROLLCMD_TOBOTTOM:
		FP_Page* pPage = m_pLayout->getFirstPage();
		UT_sint32 iDocHeight = 0;
		while (pPage)
		{
			iDocHeight += pPage->getHeight();
			pPage = pPage->getNext();
		}
		sendScrollEvent(m_xScrollOffset, iDocHeight);
		break;
	}
}

void DG_LayoutView::addScrollListener(void (*pfn)(UT_sint32, UT_sint32))
{
	m_scrollListeners.addItem((void *)pfn);
}

void DG_LayoutView::removeScrollListener(void (*pfn)(UT_sint32, UT_sint32))
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		void (*pfn2)(UT_sint32, UT_sint32)  = (void (*)(UT_sint32,UT_sint32)) m_scrollListeners.getNthItem(i);

		if (pfn2 == pfn)
		{
			m_scrollListeners.deleteNthItem(i);
			break;
		}
	}
}

void DG_LayoutView::sendScrollEvent(UT_sint32 xoff, UT_sint32 yoff)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		void (*pfn)(UT_sint32, UT_sint32)  = (void ((*)(UT_sint32,UT_sint32))) m_scrollListeners.getNthItem(i);

		pfn(xoff, yoff);
	}
}

void DG_LayoutView::cmdSelectWord(UT_sint32 xPos, UT_sint32 yPos)
{
	warpInsPtToXY(xPos, yPos);

	_eraseInsertionPoint();
	
	UT_uint32 iPoint = _getPoint();
	
	UT_UCSChar ch;
	DG_DocMarkerId dmid;
	UT_Bool bDone;

	/*
	  TODO this notion of selecting a word is a bit too English-centric
	*/
	
	bDone = UT_FALSE;
	UT_uint32 iPosLeft = iPoint;
	while (!bDone)
	{
		DG_DB_PosInfo dbpi = m_pBuffer->getOneItem(iPosLeft, &ch, &dmid);
		switch (dbpi)
		{
		case DG_DBPI_DATA:
			if (UT_isWordDelimiter(ch))
			{
				bDone = UT_TRUE;
			}
			else
			{
				iPosLeft--;
			}
			break;
		case DG_DBPI_END:
		case DG_DBPI_ERROR:
			bDone = UT_TRUE;
			break;
		default:
			iPosLeft--;
			break;
		}
	}

	iPosLeft++;		// TODO is this safe?  what if the next char is a marker?
	
	bDone = UT_FALSE;
	UT_uint32 iPosRight = iPoint;
	while (!bDone)
	{
		DG_DB_PosInfo dbpi = m_pBuffer->getOneItem(iPosRight, &ch, &dmid);
		switch (dbpi)
		{
		case DG_DBPI_DATA:
			if (UT_isWordDelimiter(ch))
			{
				bDone = UT_TRUE;
			}
			else
			{
				iPosRight++;
			}
			break;
		case DG_DBPI_END:
		case DG_DBPI_ERROR:
			bDone = UT_TRUE;
			break;
		default:
			iPosRight++;
			break;
		}
	}

	if (!_isSelectionEmpty())
	{
		_clearSelection();
	}

	m_iSelectionAnchor = iPosLeft;
	_setPoint(iPosRight);
	m_bSelection = UT_TRUE;
	
	_xorSelection();
}

void DG_LayoutView::cmdAlignBlock(UT_uint32 iAlignCmd)
{
	UT_uint32 iPoint = _getPoint();
	
	FL_BlockLayout* pBlock1;
	FL_BlockLayout* pBlock2;

	FL_BlockLayout*	pBlockPoint = _findBlockWithBufferPosition(iPoint);
	
	if (_isSelectionEmpty())
	{
		pBlock1 = pBlock2 = pBlockPoint;
	}
	else
	{
		UT_Bool bForward = (iPoint < m_iSelectionAnchor);

		if (bForward)
		{
			pBlock1 = pBlockPoint;
			pBlock2 = _findBlockWithBufferPosition(m_iSelectionAnchor);
		}
		else
		{
			pBlock1 = _findBlockWithBufferPosition(m_iSelectionAnchor);
			pBlock2 = pBlockPoint;
		}
	}

	FL_BlockLayout* pCurBlock = pBlock1;
	for (;;)
	{
		pCurBlock->clearScreen(m_pG);
		pCurBlock->setAlignment(iAlignCmd);
		pCurBlock->draw(m_pG);

		if (pCurBlock == pBlock2)
		{
			break;
		}
		else
		{
			pCurBlock = pCurBlock->getNext();
		}
	}
}

// -------------------------------------------------------------------------
UT_uint32 DG_LayoutView::_getPoint(void)
{
	return m_pBuffer->getPoint();
}

void DG_LayoutView::_setPoint(UT_uint32 pt)
{
	m_pBuffer->moveAbsolute(pt);
}

UT_uint32 DG_LayoutView::_getDataCount(UT_uint32 pt1, UT_uint32 pt2)
{
	return m_pBuffer->getDataCount(pt1,pt2);
}

UT_Bool DG_LayoutView::_charMotion(UT_Bool bForward,UT_uint32 countChars)
{
	return m_pBuffer->charMotion(bForward,countChars);
}
// -------------------------------------------------------------------------
