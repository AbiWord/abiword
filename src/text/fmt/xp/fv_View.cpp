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


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_timer.h"

#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_SectionLayout.h"
#include "fp_Page.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "ie_types.h"
#include "xap_App.h"
#include "xap_Clipboard.h"

// TODO why do we define these multiply, in different files? --EWS
#define DELETEP(p)	do { if (p) delete p; } while (0)
#define FREEP(p)	do { if (p) free(p); } while (0)

/****************************************************************/

class _fmtPair
{
public:
	_fmtPair(const XML_Char * p, const PP_AttrProp * c, const PP_AttrProp * b, const PP_AttrProp * s)
	{
		m_prop = p;
		m_val  = PP_evalProperty(p,c,b,s);
	}

	const XML_Char *	m_prop;
	const XML_Char *	m_val;
};

/****************************************************************/


FV_View::FV_View(void* pParentData, FL_DocLayout* pLayout)
	: AV_View(pParentData)
{
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pG = m_pLayout->getGraphics();
//	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
	m_iPointHeight = 0;
	m_bPointEOL = UT_FALSE;
	m_bSelection = UT_FALSE;
	m_bPointAP = UT_FALSE;
	m_pAutoScrollTimer = NULL;

	// initialize change cache
	m_chg.bUndo = UT_FALSE;
	m_chg.bRedo = UT_FALSE;
	m_chg.bDirty = UT_FALSE;
	m_chg.bSelection = UT_FALSE;
	m_chg.propsChar = NULL;
	m_chg.propsBlock = NULL;

	pLayout->setView(this);
		
	moveInsPtTo(FV_DOCPOS_BOD);
	m_iSelectionAnchor = _getPoint();
	_resetSelection();
	_fixInsertionPointCoords();

	_m_findNextString = NULL;

	m_wrappedEnd = UT_FALSE;
	m_startPosition = 0;
}

FV_View::~FV_View()
{
	DELETEP(m_pAutoScrollTimer);

	FREEP(_m_findNextString);
	
	FREEP(m_chg.propsChar);
	FREEP(m_chg.propsBlock);
}
	
FL_DocLayout* FV_View::getLayout() const
{
	return m_pLayout;
}

UT_Bool FV_View::notifyListeners(const AV_ChangeMask hint)
{
	/*
		IDEA: The view caches its change state as of the last notification, 
		to minimize noise from duplicate notifications.  
	*/
	UT_ASSERT(hint != AV_CHG_NONE);
	AV_ChangeMask mask = hint;
	
	if (mask & AV_CHG_DO)
	{
		UT_Bool bUndo = canDo(UT_TRUE);
		UT_Bool bRedo = canDo(UT_FALSE);

		if ((m_chg.bUndo == bUndo) && (m_chg.bRedo == bRedo))
		{
			mask ^= AV_CHG_DO;
		}
		else
		{
			if (m_chg.bUndo != bUndo)
				m_chg.bUndo = bUndo;
			if (m_chg.bRedo != bRedo)
				m_chg.bRedo = bRedo;
		}
	}
	
	if (mask & AV_CHG_DIRTY)
	{
		UT_Bool bDirty = m_pDoc->isDirty();

		if (m_chg.bDirty != bDirty)
		{
			m_chg.bDirty = bDirty;
		}
		else
		{
			mask ^= AV_CHG_DIRTY;
		}
	}

	if (mask & AV_CHG_EMPTYSEL)
	{
		UT_Bool bSel = !isSelectionEmpty();

		if (m_chg.bSelection != bSel)
			m_chg.bSelection = bSel;
		else
			mask ^= AV_CHG_EMPTYSEL;
	}

	if (mask & AV_CHG_FILENAME)
	{
		// NOTE: we don't attempt to filter this
	}

	if (mask & AV_CHG_FMTBLOCK)
	{
		/*
			The following brute-force solution works, but is atrociously 
			expensive, so we should avoid using it whenever feasible.  
		*/
		const XML_Char ** propsBlock = NULL;
		getBlockFormat(&propsBlock);

		UT_Bool bMatch = UT_FALSE;

		if (propsBlock && m_chg.propsBlock)
		{
			bMatch = UT_TRUE;

			int i=0;

			while (bMatch)
			{
				if (!propsBlock[i] || !m_chg.propsBlock[i])
				{
					bMatch = (propsBlock[i] == m_chg.propsBlock[i]);
					break;
				}

				if (UT_stricmp(propsBlock[i], m_chg.propsBlock[i]))
				{
					bMatch = UT_FALSE;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			FREEP(m_chg.propsBlock);
			m_chg.propsBlock = propsBlock;
		}
		else
		{
			FREEP(propsBlock);
			mask ^= AV_CHG_FMTBLOCK;
		}
	}

	if (mask & AV_CHG_FMTCHAR)
	{
		/*
			The following brute-force solution works, but is atrociously 
			expensive, so we should avoid using it whenever feasible.  

			TODO: devise special case logic for (at minimum) char motion
		*/
		const XML_Char ** propsChar = NULL;
		getCharFormat(&propsChar);

		UT_Bool bMatch = UT_FALSE;

		if (propsChar && m_chg.propsChar)
		{
			bMatch = UT_TRUE;

			int i=0;

			while (bMatch)
			{
				if (!propsChar[i] || !m_chg.propsChar[i])
				{
					bMatch = (propsChar[i] == m_chg.propsChar[i]);
					break;
				}

				if (UT_stricmp(propsChar[i], m_chg.propsChar[i]))
				{
					bMatch = UT_FALSE;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			FREEP(m_chg.propsChar);
			m_chg.propsChar = propsChar;
		}
		else
		{
			FREEP(propsChar);
			mask ^= AV_CHG_FMTCHAR;
		}
	}

	if (mask & AV_CHG_PAGECOUNT)
	{
		// NOTE: we don't attempt to filter this
	}
		
	// base class does the rest
	return AV_View::notifyListeners(mask);
}

void FV_View::_swapSelectionOrientation(void)
{
	// reverse the direction of the current selection
	// without changing the screen.

	UT_ASSERT(!isSelectionEmpty());
	PT_DocPosition curPos = _getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	_setPoint(m_iSelectionAnchor);
	m_iSelectionAnchor = curPos;
}
	
void FV_View::_moveToSelectionEnd(UT_Bool bForward)
{
	// move to the requested end of the current selection.
	// NOTE: this must clear the selection.
	// NOTE: we do not draw the insertion point
	//       after clearing the selection.

	UT_ASSERT(!isSelectionEmpty());
	
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
	UT_uint32 iPos1, iPos2;
	UT_Bool bRedrawOldSelection;

	if (m_bSelection)
	{
		if (m_iSelectionAnchor < _getPoint())
		{
			iPos1 = m_iSelectionAnchor;
			iPos2 = _getPoint();
		}
		else
		{
			iPos1 = _getPoint();
			iPos2 = m_iSelectionAnchor;
		}
		bRedrawOldSelection = UT_TRUE;
	}
	else
	{
		bRedrawOldSelection = UT_FALSE;
	}

	_resetSelection();

	if (bRedrawOldSelection)
	{
		_drawBetweenPositions(iPos1, iPos2);
	}
}

void FV_View::_resetSelection(void)
{
	m_bSelection = UT_FALSE;
	m_iSelectionAnchor = 0;
}

void FV_View::_drawSelection()
{
	UT_ASSERT(!isSelectionEmpty());

	if (m_iSelectionAnchor < _getPoint())
	{
		_drawBetweenPositions(m_iSelectionAnchor, _getPoint());
	}
	else
	{
		_drawBetweenPositions(_getPoint(), m_iSelectionAnchor);
	}
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

	UT_ASSERT(!isSelectionEmpty());

	PT_DocPosition iPoint = _getPoint();
	UT_ASSERT(iPoint != m_iSelectionAnchor);

	// TODO fix this
	
	UT_Bool bForward = (iPoint < m_iSelectionAnchor);

	if (bForward)
	{
		m_pDoc->deleteSpan(iPoint, m_iSelectionAnchor);
	}
	else
	{
		m_pDoc->deleteSpan(m_iSelectionAnchor, iPoint);
	}

	_resetSelection();
}

UT_Bool FV_View::isSelectionEmpty()
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
				if (!pBlock->getPrev(UT_TRUE))
					break;

				// yep.  look there instead
				pBlock = pBlock->getPrev(UT_TRUE);
			}

			iPos = pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_EOB:
		{
			if (pBlock->getNext(UT_TRUE))
			{
				// BOB for next block
				pBlock = pBlock->getNext(UT_TRUE);
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
			UT_GrowBuf pgb(1024);

			UT_Bool bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);

			const UT_UCSChar* pSpan = pgb.getPointer(0);

			UT_ASSERT(iPos >= pBlock->getPosition());
			UT_uint32 offset = iPos - pBlock->getPosition();
			UT_ASSERT(offset <= pgb.getLength());

			if (offset == 0)
			{
				if (!bKeepLooking)
					break;

				// is there a prior block?
				pBlock = pBlock->getPrev(UT_TRUE);

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
			UT_GrowBuf pgb(1024);

			UT_Bool bRes = pBlock->getBlockBuf(&pgb);
			UT_ASSERT(bRes);

			const UT_UCSChar* pSpan = pgb.getPointer(0);

			UT_ASSERT(iPos >= pBlock->getPosition());
			UT_uint32 offset = iPos - pBlock->getPosition();
			UT_ASSERT(offset <= pgb.getLength());

			if (offset == pgb.getLength())
			{
				if (!bKeepLooking)
					break;

				// is there a next block?
				pBlock = pBlock->getNext(UT_TRUE);

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
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}
	
	PT_DocPosition iPos = _getDocPos(dp);

	if (iPos != _getPoint())
	{
		_clearPointAP(UT_TRUE);
	}

	_setPoint(iPos, (dp == FV_DOCPOS_EOL));

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::moveInsPtTo(PT_DocPosition dp)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}
	
	if (dp != _getPoint())
	{
		_clearPointAP(UT_TRUE);
	}

	_setPoint(dp, /* (dp == FV_DOCPOS_EOL) */ UT_FALSE);  // is this bool correct?

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
}


void FV_View::cmdCharMotion(UT_Bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
		// Note: _moveToSelectionEnd() clears the selection
		//       but does not redraw the insertion point.
		_drawInsertionPoint();
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

	notifyListeners(AV_CHG_MOTION);
}

fl_BlockLayout* FV_View::_findBlockAtPosition(PT_DocPosition pos)
{
	return m_pLayout->findBlockAtPosition(pos);
}

UT_Bool FV_View::cmdCharInsert(UT_UCSChar * text, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	UT_Bool bResult = m_pDoc->insertSpan(_getPoint(), text, count);
	m_pLayout->deleteEmptyColumnsAndPages();
		
	_updateScreen();
	
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bResult;
}

void FV_View::insertParagraphBreak(void)
{
	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).

	// ==>: we *don't* call _clearPointAP() in this case 

	m_pDoc->insertStrux(_getPoint(), PTX_Block);
	
	m_pLayout->deleteEmptyColumnsAndPages();
		
	_updateScreen();
	
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

UT_Bool FV_View::setCharFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
	}

	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,NULL,properties);

	m_pLayout->deleteEmptyColumnsAndPages();
	_updateScreen();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	else
	{
		_drawSelection();
	}

	return bRet;
}

UT_Bool FV_View::getCharFormat(const XML_Char *** pProps)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;

	/*
		IDEA: We want to know character-level formatting properties, iff
		they're constant across the entire selection.  To do so, we start 
		at the beginning of the selection, load 'em all into a vector, and 
		then prune any property that collides.
	*/
	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;
	UT_Bool bSelEmpty = isSelectionEmpty();

	if (!bSelEmpty)
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. assemble complete set at insertion point
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight;

	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	fp_Run* pRun = pBlock->findPointCoords(posStart, UT_FALSE,
										   xPoint, yPoint, iPointHeight);

	if (_isPointAP())
	{
		/*
			We have a temporary span format at the insertion point.  
		*/
		UT_ASSERT(bSelEmpty);
		m_pDoc->getAttrProp(_getPointAP(), &pSpanAP);
	}
	else
	{
		if (!bSelEmpty)
		{
			/*
				NOTE: getSpanAttrProp is optimized for insertions, so it 
				essentially returns the properties on the left side of the 
				specified position.  
				
				This is exactly what we want at the insertion point. 

				However, to get properties for a selection, we need to 
				start looking one position to the right.  
			*/
			posStart++;

#if 0
			/*
				Likewise, findPointCoords will return the run to the right 
				of the specified position, so we need to stop looking one 
				position to the left. 
			*/

			/*
			  TODO -- the following line looks a bit suspicious.  Is it is
			  valid to decrement posEnd without checking to see what kind
			  of structure posEnd will then point to?  Specifically, if you
			  load Interview.abw (the new 2-section version), and with the
			  ins pt at the very beginning of the document, hold down the
			  shift key and press the down arrow (to select the line), an
			  assertion fires.  I have traced the root of that problem to
			  the following line, but I'm not quite sure how to change it.
			*/
			posEnd--;
#endif
		}

		pBlock->getSpanAttrProp(posStart - pBlock->getPosition(),&pSpanAP);
	}

	pBlock->getAttrProp(&pBlockAP);

	v.addItem(new _fmtPair("font-family",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-size",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-weight",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-style",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("text-decoration",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("color",pSpanAP,pBlockAP,pSectionAP));

	// 2. prune 'em as they vary across selection
	if (!bSelEmpty)
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		fp_Run* pRunEnd = pBlockEnd->findPointCoords(posEnd, UT_FALSE,
											   xPoint, yPoint, iPointHeight);
	
		while (pRun && (pRun != pRunEnd))
		{
			const PP_AttrProp * pAP;
			UT_Bool bCheck = UT_FALSE;

			pRun = pRun->getNext();

			if (!pRun)
			{
				// go to first run of next block
				pBlock = pBlock->getNext(UT_TRUE);

				if (!pBlock)
				{
					// at EOD, so just bail
					break;
				}

				// did block format change?
				pBlock->getAttrProp(&pAP);
				if (pBlockAP != pAP)
				{
					pBlockAP = pAP;
					bCheck = UT_TRUE;
				}

				pRun = pBlock->getFirstRun();
			}

			// did span format change?

#if 0			
			/*
			  NOTE  I am taking the following assert out for now.
			  It fires a lot on dragging of selections in Jeff's
			  file format document (AbiWord_DocumentFormat.abw), but
			  allowing the app to continue after the assertion
			  seems harmless.  Is the assertion wrong, perhaps?
			*/

			// TODO Why are we getting zero length runs not at the
			// TODO beginning of a block ??  I'd like to keep this
			// TODO in until to catch these...  -- jeff
			
			UT_ASSERT((pRun->getLength()>0) || (pRun->getBlockOffset()>0));
#endif
			pAP = NULL;
			pBlock->getSpanAttrProp(pRun->getBlockOffset()+pRun->getLength(),&pAP);
			if (pSpanAP != pAP)
			{
				pSpanAP = pAP;
				bCheck = UT_TRUE;
			}

			if (bCheck)
			{
				i = v.getItemCount();

				while (i > 0)
				{
					f = (_fmtPair *)v.getNthItem(i-1);

					const XML_Char * value = PP_evalProperty(f->m_prop,pSpanAP,pBlockAP,pSectionAP);
					UT_ASSERT(value);

					// prune anything that doesn't match
					if (UT_stricmp(f->m_val, value))
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}

					i--;
				}

				// when vector is empty, stop looking
				if (0 == v.getItemCount())
				{
					pRun = NULL;
					break;
				}
			}
		}
	}

	// 3. export whatever's left
	UT_uint32 count = v.getItemCount()*2 + 1;

	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** props = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!props)
		return UT_FALSE;

	const XML_Char ** p = props;

	i = v.getItemCount();

	while (i > 0)
	{
		f = (_fmtPair *)v.getNthItem(i-1);
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;

	return UT_TRUE;
}

UT_Bool FV_View::setBlockFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	_clearPointAP(UT_TRUE);
	_eraseInsertionPoint();

	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Block);

	_updateScreen();
	
	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	else
	{
		_drawSelection();
	}

	return bRet;
}

UT_Bool FV_View::getSectionFormat(const XML_Char ***pProps)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;

	/*
		IDEA: We want to know block-level formatting properties, iff
		they're constant across the entire selection.  To do so, we start 
		at the beginning of the selection, load 'em all into a vector, and 
		then prune any property that collides.
	*/
	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. assemble complete set at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	fl_SectionLayout* pSection = pBlock->getSectionLayout();
	pSection->getAttrProp(&pSectionAP);

	v.addItem(new _fmtPair("columns",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("column-gap",NULL,pBlockAP,pSectionAP));

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		fl_SectionLayout *pSectionEnd = pBlockEnd->getSectionLayout();

		while (pSection && (pSection != pSectionEnd))
		{
			const PP_AttrProp * pAP;
			UT_Bool bCheck = UT_FALSE;

			pSection = m_pLayout->getNextSection(pSection);

			if (!pSection)
			{
				// at EOD, so just bail
				break;
			}

			// did block format change?
			pSection->getAttrProp(&pAP);
			if (pSectionAP != pAP)
			{
				pSectionAP = pAP;
				bCheck = UT_TRUE;
			}

			if (bCheck)
			{
				i = v.getItemCount();

				while (i > 0)
				{
					f = (_fmtPair *)v.getNthItem(i-1);

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP);
					UT_ASSERT(value);

					// prune anything that doesn't match
					if (UT_stricmp(f->m_val, value))
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}

					i--;
				}

				// when vector is empty, stop looking
				if (0 == v.getItemCount())
				{
					pSection = NULL;
					break;
				}
			}
		}
	}

	// 3. export whatever's left
	UT_uint32 count = v.getItemCount()*2 + 1;

	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** props = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!props)
		return UT_FALSE;

	const XML_Char ** p = props;

	i = v.getItemCount();

	while (i > 0)
	{
		f = (_fmtPair *)v.getNthItem(i-1);
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;

	return UT_TRUE;
}

UT_Bool FV_View::getBlockFormat(const XML_Char *** pProps)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;

	/*
		IDEA: We want to know block-level formatting properties, iff
		they're constant across the entire selection.  To do so, we start 
		at the beginning of the selection, load 'em all into a vector, and 
		then prune any property that collides.
	*/
	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. assemble complete set at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	pBlock->getAttrProp(&pBlockAP);

	v.addItem(new _fmtPair("text-align",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("text-indent",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("margin-left",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("margin-right",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("margin-top",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("margin-bottom",NULL,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("line-height",NULL,pBlockAP,pSectionAP));

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock && (pBlock != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			UT_Bool bCheck = UT_FALSE;

			pBlock = pBlock->getNext(UT_TRUE);

			if (!pBlock)
			{
				// at EOD, so just bail
				break;
			}

			// did block format change?
			pBlock->getAttrProp(&pAP);
			if (pBlockAP != pAP)
			{
				pBlockAP = pAP;
				bCheck = UT_TRUE;
			}

			if (bCheck)
			{
				i = v.getItemCount();

				while (i > 0)
				{
					f = (_fmtPair *)v.getNthItem(i-1);

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP);
					UT_ASSERT(value);

					// prune anything that doesn't match
					if (UT_stricmp(f->m_val, value))
					{
						DELETEP(f);
						v.deleteNthItem(i-1);
					}

					i--;
				}

				// when vector is empty, stop looking
				if (0 == v.getItemCount())
				{
					pBlock = NULL;
					break;
				}
			}
		}
	}

	// 3. export whatever's left
	UT_uint32 count = v.getItemCount()*2 + 1;

	// NOTE: caller must free this, but not the referenced contents
	const XML_Char ** props = (const XML_Char **) calloc(count, sizeof(XML_Char *));
	if (!props)
		return UT_FALSE;

	const XML_Char ** p = props;

	i = v.getItemCount();

	while (i > 0)
	{
		f = (_fmtPair *)v.getNthItem(i-1);
		i--;

		p[0] = f->m_prop;
		p[1] = f->m_val;
		p += 2;
	}

	UT_VECTOR_PURGEALL(_fmtPair *,v);

	*pProps = props;

	return UT_TRUE;
}

void FV_View::delTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	if (iPos == _getPoint())
	{
		return;
	}

	_extSelToPos(iPos);
	_deleteSelection();
	
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

/*
  This function is somewhat of a compromise.  It will return a new
  range of memory (destroy with free()) full of what's in the selection,
  but it will not cross block boundaries.  This is for convenience
  in implementation, but could probably be accomplished without
  too much more effort.  However, since an edit entry in a dialog
  box (1) only allows a bit of text and (2) has no concept of a
  block break anyway, I don't see a reason to make this behave
  differently.
*/
UT_UCSChar * FV_View::getSelectionText(void)
{
	UT_ASSERT(!isSelectionEmpty());

	UT_GrowBuf buffer;	

	UT_uint32 selLength = labs(m_iInsPoint - m_iSelectionAnchor);

	PT_DocPosition low;
	if (m_iInsPoint > m_iSelectionAnchor)
	{
		low = m_iSelectionAnchor;
	}
	else
	{
		low = m_iInsPoint;
	}
	
	// get the current block the insertion point is in
	fl_BlockLayout * block = m_pLayout->findBlockAtPosition(low);

	if (block)
	{
		block->getBlockBuf(&buffer);

		UT_UCSChar * bufferSegment = NULL;

		PT_DocPosition offset = low - block->getPosition(UT_FALSE);

		// allow no more than the rest of the block
		if (offset + selLength > buffer.getLength())
			selLength = buffer.getLength() - offset;
		
		// give us space for our new chunk of selected text, add 1 so it
		// terminates itself
		bufferSegment = (UT_UCSChar *) calloc(selLength + 1, sizeof(UT_UCSChar));

		// copy it out
		memmove(bufferSegment, buffer.getPointer(offset), selLength * sizeof(UT_UCSChar));

		return bufferSegment;
	}

	return NULL;
}

void FV_View::cmdCharDelete(UT_Bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_deleteSelection();

		m_pLayout->deleteEmptyColumnsAndPages();
		_updateScreen();
		
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
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
		{
			m_pDoc->deleteSpan(posCur, posCur+amt);
		}

		m_pLayout->deleteEmptyColumnsAndPages();
		
		_updateScreen();
	
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

void FV_View::_moveInsPtNextPrevLine(UT_Bool bNext)
{
	UT_uint32 xPoint;
	UT_uint32 yPoint;
	UT_uint32 iPointHeight, iLineHeight;

	/*
		This function moves the IP up or down one line, attempting to get 
		as close as possible to the prior "sticky" x position.  The notion 
		of "next" is strictly physical, not logical. 

		For example, instead of always moving from the last line of one block
		to the first line of the next, you might wind up skipping over a 
		bunch of blocks to wind up in the first line of the second column. 
	*/
	UT_sint32 xOldSticky = m_xPointSticky;

	// first, find the line we are on now
	UT_uint32 iOldPoint = _getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(_getPoint(), m_bPointEOL, xPoint, yPoint, iPointHeight);
	fl_SectionLayout* pOldSL = pOldBlock->getSectionLayout();
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Column* pOldColumn = pOldLine->getColumn();
	fp_Column* pOldLeader = pOldColumn->getLeader();
	fp_Page* pOldPage = pOldColumn->getPage();

	UT_sint32 iPageOffset;
	getPageYOffset(pOldPage, iPageOffset);

	iLineHeight = pOldLine->getHeight();

	UT_Bool bNOOP = UT_FALSE;

	if (bNext)
	{
		if (pOldLine != pOldColumn->getLastLine())
		{
			// just move off this line
			yPoint += iPointHeight + pOldLine->getMarginAfter();
		}
		else if (pOldSL->getLastColumn()->getLeader() == pOldLeader)
		{
			// move to next section
			fl_SectionLayout* pSL = m_pLayout->getNextSection(pOldSL);
			if (pSL)
			{
				yPoint = pSL->getFirstColumn()->getY();
			}
			else
			{
				bNOOP = UT_TRUE;
			}
		}
		else 
		{
			// move to next page
			fp_Page* pPage = pOldPage->getNext();
			if (pPage)
			{
				getPageYOffset(pPage, iPageOffset);
				yPoint = 0;
			}
			else
			{
				bNOOP = UT_TRUE;
			}
		}
	}
	else
	{
		if (pOldLine != pOldColumn->getFirstLine())
		{
			// just move off this line
			yPoint -= (iLineHeight - iPointHeight) + pOldLine->getMarginBefore() + 1;
		}
		else if (pOldSL->getFirstColumn() == pOldLeader)
		{
			// move to prev section
			fl_SectionLayout* pSL = m_pLayout->getPrevSection(pOldSL);
			if (pSL)
			{
				fp_Column* pTmpCol = pSL->getLastColumn()->getLeader();
				yPoint = pTmpCol->getY();

				UT_sint32 iMostHeight = 0;
				while (pTmpCol)
				{
					iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());

					pTmpCol = pTmpCol->getFollower();
				}

				yPoint += iMostHeight;
			}
			else
			{
				bNOOP = UT_TRUE;
			}
		}
		else 
		{
			// move to prev page
			fp_Page* pPage = pOldPage->getPrev();
			if (pPage)
			{
				getPageYOffset(pPage, iPageOffset);
				yPoint = pPage->getBottom();
			}
			else
			{
				bNOOP = UT_TRUE;
			}
		}
	}

	if (bNOOP)
	{
		// cannot move.  should we beep?
		_drawInsertionPoint();
		return;
	}

	// change to screen coordinates
	xPoint = m_xPointSticky - m_xScrollOffset + fl_PAGEVIEW_MARGIN_X;
	yPoint += iPageOffset - m_yScrollOffset;

	// hit-test to figure out where that puts us
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPoint, yPoint, xClick, yClick);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	UT_ASSERT(iNewPoint != iOldPoint);

	_setPoint(iNewPoint);

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	// this is the only place where we override changes to m_xPointSticky 
	m_xPointSticky = xOldSticky;
}

UT_Bool FV_View::_ensureThatInsertionPointIsOnScreen(void)
{
	UT_Bool bRet = UT_FALSE;

	if (m_iWindowHeight <= 0)
	{
		return UT_FALSE;
	}
	
	_fixInsertionPointCoords();

	if (m_yPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(m_yPoint)));
		bRet = UT_TRUE;
	}
	else if (((UT_uint32) (m_yPoint + m_iPointHeight)) >= ((UT_uint32) m_iWindowHeight))
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(m_yPoint + m_iPointHeight - m_iWindowHeight));
		bRet = UT_TRUE;
	}

	/*
		TODO: we really ought to try to do better than this.  
	*/
	if (m_xPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(m_xPoint)));
		bRet = UT_TRUE;
	}
	else if (((UT_uint32) (m_xPoint)) >= ((UT_uint32) m_iWindowWidth))
	{
		cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(m_xPoint - m_iWindowWidth));
		bRet = UT_TRUE;
	}

	return bRet;
}

void FV_View::warpInsPtNextPrevLine(UT_Bool bNext)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
	}
	else
	{
		_eraseInsertionPoint();
	}

	_resetSelection();

	_moveInsPtNextPrevLine(bNext);

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelNextPrevLine(UT_Bool bNext)
{
	if (isSelectionEmpty())
	{
		_setSelectionAnchor();
		_moveInsPtNextPrevLine(bNext);
		_drawSelection();
	}
	else
	{
		PT_DocPosition iOldPoint = _getPoint();
 		_moveInsPtNextPrevLine(bNext);
		PT_DocPosition iNewPoint = _getPoint();

		// top/bottom of doc - nowhere to go
		if (iOldPoint == iNewPoint)
		{
			return;
		}
		
		if (iOldPoint < iNewPoint)
		{
			_drawBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			_drawBetweenPositions(iNewPoint, iOldPoint);
		}

		if (isSelectionEmpty())
		{
			_resetSelection();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelHorizontal(UT_Bool bForward, UT_uint32 count)
{
	if (isSelectionEmpty())
	{
		_setSelectionAnchor();
		_charMotion(bForward, count);

		/*
		  It IS possible for the selection to be empty, even
		  after extending it.  If the charMotion fails, for example,
		  because we are at the end of a document, then the selection
		  will end up empty once again.
		*/
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		else
		{
			_drawSelection();
		}
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
			_drawBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			_drawBetweenPositions(iNewPoint, iOldPoint);
		}

		if (isSelectionEmpty())
		{
			_resetSelection();
		}
	}
	
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
//		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	_extSelToPos(iPos);

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
//		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
}

#define AUTO_SCROLL_MSECS	100

void FV_View::_autoScroll(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	FV_View * pView = (FV_View *) pTimer->getInstanceData();
	UT_ASSERT(pView);

	PT_DocPosition iOldPoint = pView->_getPoint();

	/*
		NOTE: We update the selection here, so that the timer can keep 
		triggering autoscrolls even if the mouse doesn't move.  
	*/
	pView->extSelToXY(pView->m_xLastMouse, pView->m_yLastMouse, UT_FALSE);

	if (pView->_getPoint() != iOldPoint)
	{
		// do the autoscroll
		if (!pView->_ensureThatInsertionPointIsOnScreen())
		{
			pView->_fixInsertionPointCoords();
//			pView->_drawInsertionPoint();
		}
	}
	else
	{
		// not far enough to change the selection ... do we still need to scroll?
		UT_sint32 xPos = pView->m_xLastMouse;
		UT_sint32 yPos = pView->m_yLastMouse;

		// TODO: clamp xPos, yPos to viewable area??

		UT_Bool bOnScreen = UT_TRUE;

		if ((xPos < 0 || xPos > pView->m_iWindowWidth) || 
			(yPos < 0 || yPos > pView->m_iWindowHeight))
			bOnScreen = UT_FALSE;
		
		if (!bOnScreen) 
		{
			// yep, do it manually 
			if (yPos < 0)
			{
				pView->cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(yPos)));
			}
			else if (((UT_uint32) (yPos)) >= ((UT_uint32) pView->m_iWindowHeight))
			{
				pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(yPos - pView->m_iWindowHeight));
			}

			if (xPos < 0)
			{
				pView->cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(xPos)));
			}
			else if (((UT_uint32) (xPos)) >= ((UT_uint32) pView->m_iWindowWidth))
			{
				pView->cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(xPos - pView->m_iWindowWidth));
			}
		}
	}
}


fp_Page* FV_View::_getPageForXY(UT_sint32 xPos, UT_sint32 yPos, UT_sint32& xClick, UT_sint32& yClick)
{
	xClick = xPos + m_xScrollOffset - fl_PAGEVIEW_MARGIN_X;
	yClick = yPos + m_yScrollOffset - fl_PAGEVIEW_MARGIN_Y;
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
			yClick -= iPageHeight + fl_PAGEVIEW_PAGE_SEP;
		}
		pPage = pPage->getNext();
	}

	if (!pPage)
	{
		// we're below the last page
		pPage = m_pLayout->getLastPage();

		UT_sint32 iPageHeight = pPage->getHeight();
		yClick += iPageHeight + fl_PAGEVIEW_PAGE_SEP;
	}

	return pPage;
}

void FV_View::extSelToXY(UT_sint32 xPos, UT_sint32 yPos, UT_Bool bDrag)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	UT_Bool bPostpone = UT_FALSE;

	if (bDrag)
	{
		// figure out whether we're still on screen 
		UT_Bool bOnScreen = UT_TRUE;

		if ((xPos < 0 || xPos > m_iWindowWidth) || 
			(yPos < 0 || yPos > m_iWindowHeight))
			bOnScreen = UT_FALSE;
		
		// is autoscroll timer set properly?
		if (bOnScreen) 
		{
			if (m_pAutoScrollTimer)
			{
				// timer not needed any more, so clear it
				DELETEP(m_pAutoScrollTimer);
				m_pAutoScrollTimer = NULL;
			}
		}
		else
		{
			// remember where mouse is
			m_xLastMouse = xPos;
			m_yLastMouse = yPos;

			// offscreen ==> make sure it's set
			if (!m_pAutoScrollTimer)
			{
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this);

				if (m_pAutoScrollTimer)
					m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
			}

			// postpone selection until timer fires
			bPostpone = UT_TRUE;
		}
	}
	
	if (!bPostpone)
	{
		_extSelToPos(iNewPoint);
		notifyListeners(AV_CHG_MOTION);
	}
}

void FV_View::endDrag(UT_sint32 xPos, UT_sint32 yPos)
{
	if (!m_pAutoScrollTimer)
		return;

	// figure out whether we're still on screen 
	UT_Bool bOnScreen = UT_TRUE;

	if ((xPos < 0 || xPos > m_iWindowWidth) || 
		(yPos < 0 || yPos > m_iWindowHeight))
		bOnScreen = UT_FALSE;
	
	if (!bOnScreen) 
	{
		// remember where mouse is
		m_xLastMouse = xPos;
		m_yLastMouse = yPos;

		// finish pending autoscroll
		m_pAutoScrollTimer->fire();
	}

	// timer not needed any more, so clear it
	DELETEP(m_pAutoScrollTimer);
	m_pAutoScrollTimer = NULL;
}
// ---------------- start goto ---------------

UT_Bool FV_View::gotoTarget(FV_JumpTarget type, UT_UCSChar * data)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	UT_ASSERT(m_pLayout);

	// TODO:  We need a Unicode atol/strtol.

	/*
	char * numberString = (char *) calloc(UT_UCS_strlen(m_targetData) + 1, sizeof(char));
	UT_ASSERT(numberString);
	
	UT_UCS_strcpy_to_char(numberString, m_targetData);
	
	UT_uint32 pageNumber = atol(numberString);
	FREEP(numberString);
	*/

	// check for range
//	if (pageNumber < 0 || pageNumber > (UT_uint32) m_pLayout->countPages())
//		return UT_FALSE;

	// get the right page
//	fp_Page * page = m_pLayout->getNthPage(pageNumber);
//	UT_ASSERT(page);

	// peek inside the page
	// ...

	return UT_FALSE;
}

// ---------------- start find and replace ---------------
	
UT_Bool FV_View::findNext(const UT_UCSChar * find, UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	UT_Bool bRes = _findNext(find, matchCase, bDoneEntireDocument);

	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_drawSelection();
		}
	}

	return bRes;
}

UT_Bool FV_View::_findNext(const UT_UCSChar * find, UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	UT_ASSERT(find);

	fl_BlockLayout * block = NULL;
	PT_DocPosition   offset = 0;
	
	block = _findGetCurrentBlock();
	offset = _findGetCurrentOffset();

	UT_UCSChar * buffer = NULL;
	
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		// magic number; range of UT_sint32 falls short of extremely large docs
		UT_sint32 foundAt = -1;

		// Change the ordering of searches to accomodate new searches (like
		// regular expressions, case-sensitive, or reverse searches).
		// Right now we just work off case searches.
		if (matchCase == UT_TRUE)
		{
			// this search will do case-sensitive work
			foundAt = _findBlockSearchDumb(buffer, find);
		}
		else if (matchCase == UT_FALSE)
		{
			// TODO call the right function
			UT_ASSERT(UT_NOT_IMPLEMENTED);
		}


		if (foundAt != -1)
		{
			_setPoint(block->getPosition(UT_FALSE) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(UT_TRUE, UT_UCS_strlen(find));

			m_doneFind = UT_TRUE;
			
			return UT_TRUE;
		}

		// didn't find anything, so set the offset to the end
		// of the current area
		offset += UT_UCS_strlen(buffer);

		// must clean up buffer returned for search
		FREEP(buffer);
	}

	if (bDoneEntireDocument)
	{
		*bDoneEntireDocument = UT_TRUE;
	}

	// reset wrap for next time
	m_wrappedEnd = UT_FALSE;
	
	return UT_FALSE;
}

void FV_View::findSetStartAtInsPoint(void)
{
	m_startPosition = m_iInsPoint;
	m_wrappedEnd = UT_FALSE;
	m_doneFind = UT_FALSE;
}

PT_DocPosition FV_View::_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset)
{
	UT_ASSERT(block);
	return block->getPosition(UT_FALSE) + offset;
}

UT_UCSChar * FV_View::_findGetNextBlockBuffer(fl_BlockLayout ** block, PT_DocPosition * offset)
{
	UT_ASSERT(m_pLayout);
#if 0
	// this assert doesn't work, since the startPosition CAN legitimately be zero
	UT_ASSERT(m_startPosition);
#endif	
	
	UT_ASSERT(block);
	UT_ASSERT(*block);

	UT_ASSERT(offset);
	
	fl_BlockLayout * newBlock = NULL;
	PT_DocPosition newOffset = 0;

	UT_uint32 bufferLength = 0;
	
	UT_GrowBuf buffer;

	// check early for completion, from where we left off last, and bail
	// if we are now at or past the start position
	if (m_wrappedEnd && _BlockOffsetToPos(*block, *offset) >= m_startPosition)
	{
		// we're done
		return NULL;
	}

	if (!(*block)->getBlockBuf(&buffer))
	{
		UT_DEBUGMSG(("Block %p has no associated buffer.\n", *block));
		UT_ASSERT(0);
	}
	
	// have we already searched all the text in this buffer?
	if (*offset >= buffer.getLength())
	{
		// then return a fresh new block's buffer
		newBlock = (*block)->getNext(UT_TRUE);

		// are we at the end of the document?
		if (!newBlock)
		{
			// then wrap (fetch the first block in the doc)
			PT_DocPosition startOfDoc;
			m_pDoc->getBounds(UT_FALSE, startOfDoc);
			
			newBlock = m_pLayout->findBlockAtPosition(startOfDoc);

			m_wrappedEnd = UT_TRUE;
			
			UT_ASSERT(newBlock);
		}

		// re-assign the buffer contents for our new block
		buffer.truncate(0);
		// the offset starts at 0 for a fresh buffer
		newOffset = 0;
		
		if (!newBlock->getBlockBuf(&buffer))
		{
			UT_DEBUGMSG(("Block %p (a ->next block) has no buffer.\n", newBlock));
			UT_ASSERT(0);
		}

		// good to go with a full buffer for our new block
	}
	else	// we have some left to go in this buffer
	{
		// buffer is still valid, just copy pointers
		newBlock = *block;
		newOffset = *offset;
	}

	// are we going to run into the start position in this buffer?
	// if so, we need to size our length accordingly
	if (m_wrappedEnd && _BlockOffsetToPos(newBlock, newOffset) + buffer.getLength() >= m_startPosition)
	{
		bufferLength = (m_startPosition - (newBlock)->getPosition(UT_FALSE)) - newOffset;
	}
	else
	{
		bufferLength = buffer.getLength() - newOffset;
	}
	
	// clone a buffer (this could get really slow on large buffers!)
	UT_UCSChar * bufferSegment = NULL;

	// remember, the caller gets to free this memory
	bufferSegment = (UT_UCSChar *) calloc(bufferLength + 1,
										  sizeof(UT_UCSChar));

	memmove(bufferSegment, buffer.getPointer(newOffset),
			(bufferLength) * sizeof(UT_UCSChar));

	// before we bail, hold up our block stuff for next round
	*block = newBlock;
	*offset = newOffset;
		
	return bufferSegment;
}

UT_Bool FV_View::findSetNextString(UT_UCSChar * string, UT_Bool matchCase)
{
	UT_ASSERT(string);

	// update case matching
	_m_matchCase = matchCase;

	// update string
	FREEP(_m_findNextString);
	return UT_UCS_cloneString(&_m_findNextString, string);
}

UT_Bool FV_View::findAgain()
{
	if (_m_findNextString && *_m_findNextString)
	{
		return findNext(_m_findNextString, _m_matchCase, NULL);
	}
	
	return UT_FALSE;
}

UT_Bool	FV_View::findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							 UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	UT_Bool bRes = _findReplace(find, replace, matchCase, bDoneEntireDocument);

	_updateScreen();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_drawSelection();
		}
	}

	return bRes;
}

UT_Bool	FV_View::_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							  UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	// if we have done a find, and there is a selection, then replace what's in the
	// selection and move on to next find (batch run, the common case)
	if ((m_doneFind == UT_TRUE) && (!isSelectionEmpty()))
	{
		// if we've wrapped around once, and we're doing work before we've
		// hit the point at which we started, then we adjust the start
		// position so that we stop at the right spot.
		m_startPosition += ((long) UT_UCS_strlen(replace) - (long) UT_UCS_strlen(find));

		UT_Bool result = UT_TRUE;

		if (*replace)
		{
			if (!isSelectionEmpty())
			{
				_deleteSelection();
			}
			else
			{
				_eraseInsertionPoint();
			}

			result = m_pDoc->insertSpan(_getPoint(), replace, UT_UCS_strlen(replace));
			m_pLayout->deleteEmptyColumnsAndPages();
		}

		// we must move the find cursor past the insertion
		// we just did, so that we don't get caught in a loop while doing
		// a replace, but account for the 1 that the find advanced.
		m_iInsPoint ++;

		_findNext(find, matchCase, bDoneEntireDocument);
		return result;
	}

	// if we have done a find, but there is no selection, do a find for them
	// but no replace
	if (m_doneFind == UT_TRUE && isSelectionEmpty() == UT_TRUE)
	{
		_findNext(find, matchCase, bDoneEntireDocument);
		return UT_FALSE;
	}
	
	// if we haven't done a find yet, do a find for them
	if (m_doneFind == UT_FALSE)
	{
		_findNext(find, matchCase, bDoneEntireDocument);
		return UT_FALSE;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}

UT_uint32 FV_View::findReplaceAll(const UT_UCSChar * find, const UT_UCSChar * replace,
								  UT_Bool matchCase)
{
	UT_uint32 numReplaced = 0;
	m_pDoc->beginUserAtomicGlob();

	UT_Bool bDoneEntireDocument = UT_FALSE;
	
	// prime it with a find
	if (!_findNext(find, matchCase, &bDoneEntireDocument))
	{
		// can't find a single thing, we're done
		m_pDoc->endUserAtomicGlob();
		return numReplaced;
	}
	
	// while we've still got buffer
	while (bDoneEntireDocument == UT_FALSE)
	{
		// if it returns false, it found nothing more before
		// it hit the end of the document
		if (!_findReplace(find, replace, matchCase, &bDoneEntireDocument))
		{
			m_pDoc->endUserAtomicGlob();
			return numReplaced;
		}
		numReplaced++;
	}

	m_pDoc->endUserAtomicGlob();
	
	_updateScreen();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_drawSelection();
		}
	}

	return numReplaced;
}

fl_BlockLayout * FV_View::_findGetCurrentBlock(void)
{
	return m_pLayout->findBlockAtPosition(m_iInsPoint);
}

PT_DocPosition FV_View::_findGetCurrentOffset(void)
{
	return (m_iInsPoint - _findGetCurrentBlock()->getPosition(UT_FALSE));
}

/*
  A simple strstr search of the buffer.
*/
UT_sint32 FV_View::_findBlockSearchDumb(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_strstr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}

/*
  Any takers?
*/
UT_sint32 FV_View::_findBlockSearchRegexp(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return -1;
}

// ---------------- end find and replace ---------------

void FV_View::_extSelToPos(PT_DocPosition iNewPoint)
{
	PT_DocPosition iOldPoint = _getPoint();

	xxx_UT_DEBUGMSG(("extSelToPos: iOldPoint=%d  iNewPoint=%d  iSelectionAnchor=%d\n",
				 iOldPoint, iNewPoint, m_iSelectionAnchor));
	
	if (iNewPoint == iOldPoint)
	{
		return;
	}

	_clearPointAP(UT_TRUE);

	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_setSelectionAnchor();
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

	_setPoint(iNewPoint);
	
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
				_drawBetweenPositions(iNewPoint, iOldPoint);
			}
			else
			{
				/*
				  N A O
				  The selection flipped across the anchor to the left.
				*/
				_drawBetweenPositions(iNewPoint, iOldPoint);
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

			_drawBetweenPositions(iNewPoint, iOldPoint);
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

			_drawBetweenPositions(iOldPoint, iNewPoint);
		}
		else
		{
			if (iOldPoint < m_iSelectionAnchor)
			{
				/*
				  O A N
				  The selection flipped across the anchor to the right.
				*/

				_drawBetweenPositions(iOldPoint, iNewPoint);
			}
			else
			{
				/*
				  A O N
				  The selection got bigger.  Both points are to the
				  right of the anchor
				*/
				_drawBetweenPositions(iOldPoint, iNewPoint);
			}
		}
	}

	if (isSelectionEmpty())
	{
		_resetSelection();
		_drawInsertionPoint();
	}
}

void FV_View::warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	_clearPointAP(UT_TRUE);

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}
	
	PT_DocPosition pos;
	UT_Bool bBOL, bEOL;
	
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);
	
	_setPoint(pos, bEOL);
	_fixInsertionPointCoords();
	_drawInsertionPoint();

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::getPageScreenOffsets(fp_Page* pThePage, UT_sint32& xoff,
										 UT_sint32& yoff)
{
	UT_uint32 y = fl_PAGEVIEW_MARGIN_Y;
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + fl_PAGEVIEW_PAGE_SEP;

		pPage = pPage->getNext();
	}

	yoff = y - m_yScrollOffset;
	xoff = fl_PAGEVIEW_MARGIN_Y - m_xScrollOffset;
}

void FV_View::getPageYOffset(fp_Page* pThePage, UT_sint32& yoff)
{
	UT_uint32 y = fl_PAGEVIEW_MARGIN_Y;
	
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pThePage)
		{
			break;
		}
		y += pPage->getHeight() + fl_PAGEVIEW_PAGE_SEP;

		pPage = pPage->getNext();
	}

	yoff = y;
}

UT_uint32 FV_View::getPageViewLeftMargin(void) const
{
	// return the amount of gray-space we draw to the left
	// of the paper in "Page View".  return zero if not in
	// "Page View".

	return fl_PAGEVIEW_MARGIN_X;
}

UT_uint32 FV_View::getPageViewTopMargin(void) const
{
	// return the amount of gray-space we draw above the top
	// of the paper in "Page View".  return zero if not in
	// "Page View".

	return fl_PAGEVIEW_MARGIN_Y;
}

/*
  This method simply iterates over every run between two doc positions
  and draws each one.  The current selection information is heeded.
*/
void FV_View::_drawBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2)
{
	UT_ASSERT(iPos1 < iPos2);
	
	fp_Run* pRun1;
	fp_Run* pRun2;
	UT_sint32 xoff;
	UT_sint32 yoff;
	UT_uint32 uheight;

	{
		UT_sint32 x;
		UT_sint32 y;
		fl_BlockLayout* pBlock1;
		fl_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, UT_FALSE, x, y, uheight, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, UT_FALSE, x, y, uheight, &pBlock2, &pRun2);
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

		fp_Line* pLine = pCurRun->getLine();

		pLine->getScreenOffsets(pCurRun, xoff, yoff);

		dg_DrawArgs da;
			
		da.pG = m_pG;
		da.xoff = xoff;
		da.yoff = yoff + pLine->getAscent();

		if (m_bSelection)
		{
			if (m_iSelectionAnchor < _getPoint())
			{
				da.iSelPos1 = m_iSelectionAnchor;
				da.iSelPos2 = _getPoint();
			}
			else
			{
				da.iSelPos1 = _getPoint();
				da.iSelPos2 = m_iSelectionAnchor;
			}
		}
		else
		{
			da.iSelPos1 = da.iSelPos2 = 0;
		}

		pCurRun->draw(&da);
		
		pCurRun = pCurRun->getNext();
		if (!pCurRun)
		{
			fl_BlockLayout* pNextBlock;
			
			pNextBlock = pBlock->getNext(UT_TRUE);
			if (pNextBlock)
			{
				pCurRun = pNextBlock->getFirstRun();
			}
		}
	}
}

void FV_View::_findPositionCoords(PT_DocPosition pos,
										UT_Bool bEOL,
										UT_sint32& x,
										UT_sint32& y,
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
	fp_Page* pPointPage = pRun->getLine()->getColumn()->getPage();

	UT_sint32 iPageOffset;
	getPageYOffset(pPointPage, iPageOffset);
	yPoint += iPageOffset;
	xPoint += fl_PAGEVIEW_MARGIN_X;

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

void FV_View::_fixInsertionPointCoords()
{
	_findPositionCoords(_getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_iPointHeight, NULL, NULL);

	// hang onto this for _moveInsPtNextPrevLine()
	m_xPointSticky = m_xPoint + m_xScrollOffset - fl_PAGEVIEW_MARGIN_X;
}

void FV_View::_updateInsertionPoint()
{
	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

void FV_View::_xorInsertionPoint()
{
	if (m_iPointHeight > 0)
	{
		UT_RGBColor clr(255,255,255);

		UT_DEBUGMSG(("_xorInsertionPoint\n"));

		m_pG->setColor(clr);
		m_pG->xorLine(m_xPoint, m_yPoint, m_xPoint, m_yPoint + m_iPointHeight);
	}
}

void FV_View::_eraseInsertionPoint()
{
	if (!isSelectionEmpty())
	{
		return;
	}

	_xorInsertionPoint();
}

void FV_View::_drawInsertionPoint()
{
	if (m_iWindowHeight <= 0)
	{
		return;
	}
	
	if (!isSelectionEmpty())
	{
		return;
	}

	_xorInsertionPoint();
}

void FV_View::setXScrollOffset(UT_sint32 v)
{
	UT_sint32 dx = v - m_xScrollOffset;

	if (dx == 0)
		return;
	
	m_pG->scroll(dx, 0);
	m_xScrollOffset = v;
	
	if (dx > 0)
    {
		if (dx >= m_iWindowWidth)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, UT_FALSE, UT_TRUE);
		}
		else
		{
			_draw(m_iWindowWidth - dx, 0, m_iWindowWidth, m_iWindowHeight, UT_FALSE, UT_TRUE);
		}
    }
	else
    {
		if (dx <= -m_iWindowWidth)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, UT_FALSE, UT_TRUE);
		}
		else
		{
			_draw(0, 0, -dx, m_iWindowHeight, UT_FALSE, UT_TRUE);
		}
    }
}

void FV_View::setYScrollOffset(UT_sint32 v)
{
	UT_sint32 dy = v - m_yScrollOffset;

	if (dy == 0)
		return;
	
	m_pG->scroll(0, dy);
	m_yScrollOffset = v;

	if (dy > 0)
    {
		if (dy >= m_iWindowHeight)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, UT_FALSE, UT_TRUE);
		}
		else
		{
			_draw(0, m_iWindowHeight - dy, m_iWindowWidth, dy, UT_FALSE, UT_TRUE);
		}
    }
	else
    {
		if (dy <= -m_iWindowHeight)
		{
			_draw(0, 0, m_iWindowWidth, m_iWindowHeight, UT_FALSE, UT_TRUE);
		}
		else
		{
			_draw(0, 0, m_iWindowWidth, -dy, UT_FALSE, UT_TRUE);
		}
    }
}

void FV_View::draw(int page, dg_DrawArgs* da)
{
	xxx_UT_DEBUGMSG(("FV_View::draw_1: [page %ld]\n",page));

	da->pG = m_pG;
	fp_Page* pPage = m_pLayout->getNthPage(page);
	if (pPage)
	{
		pPage->draw(da);
	}
}


void FV_View::draw(const UT_Rect* pClipRect)
{
	if (pClipRect)
	{
		_draw(pClipRect->left,pClipRect->top,pClipRect->width,pClipRect->height,UT_FALSE,UT_TRUE);
	}
	else
	{
		_draw(0,0,m_iWindowWidth,m_iWindowHeight,UT_FALSE,UT_FALSE);
	}
}

void FV_View::_updateScreen(void)
{
	_draw(0,0,m_iWindowWidth,m_iWindowHeight,UT_TRUE,UT_FALSE);
}

void FV_View::_draw(UT_sint32 x, UT_sint32 y,
				   UT_sint32 width, UT_sint32 height,
				   UT_Bool bDirtyRunsOnly, UT_Bool bClip)
{
	UT_DEBUGMSG(("FV_View::draw_3 [x %ld][y %ld][w %ld][h %ld][bClip %ld]\n"
				 "\t\twith [yScrollOffset %ld][windowHeight %ld]\n",
				 x,y,width,height,bClip,
				 m_yScrollOffset,m_iWindowHeight));

	// this can happen when the frame size is decreased and
	// only the toolbars show...
	if ((m_iWindowWidth <= 0) || (m_iWindowHeight <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero drawing area.\n"));
		return;
	}

	if ((width <= 0) || (height <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero width or height expose.\n"));
		return;
	}

	if (bClip)
	{
		UT_Rect r;

		r.left = x;
		r.top = y;
		r.width = width;
		r.height = height;

		m_pG->setClipRect(&r);
	}

	// figure out where pages go, based on current window dimensions
	// TODO: don't calc for every draw
	// HYP:  cache calc results at scroll/size time
	UT_sint32 iDocHeight = m_pLayout->getHeight();

	// TODO: handle positioning within oversized viewport
	// TODO: handle variable-size pages (envelope, landscape, etc.)

	/*
	  In page view mode, so draw outside decorations first, then each 
	  page with its decorations.  
	*/

	UT_RGBColor clrMargin(127,127,127);		// dark gray

	if (!bDirtyRunsOnly)
	{
		if (m_xScrollOffset < fl_PAGEVIEW_MARGIN_X)
		{
			// fill left margin
			m_pG->fillRect(clrMargin, 0, 0, fl_PAGEVIEW_MARGIN_X - m_xScrollOffset, m_iWindowHeight);
		}

		if (m_yScrollOffset < fl_PAGEVIEW_MARGIN_Y)
		{
			// fill top margin
			m_pG->fillRect(clrMargin, 0, 0, m_iWindowWidth, fl_PAGEVIEW_MARGIN_Y - m_yScrollOffset);
		}
	}

	UT_sint32 curY = fl_PAGEVIEW_MARGIN_Y;
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageWidth = pPage->getWidth();
		UT_sint32 iPageHeight = pPage->getHeight();
		UT_sint32 adjustedTop    = curY - m_yScrollOffset;
		UT_sint32 adjustedBottom = adjustedTop + iPageHeight + fl_PAGEVIEW_PAGE_SEP;
		if (adjustedTop > m_iWindowHeight)
		{
			// the start of this page is past the bottom
			// of the window, so we don't need to draw it.

			xxx_UT_DEBUGMSG(("not drawing page A: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight));

			// since all other pages are below this one, we
			// don't need to draw them either.  exit loop now.
			break;
		}
		else if (adjustedBottom < 0)
		{
			// the end of this page is above the top of
			// the window, so we don't need to draw it.

			xxx_UT_DEBUGMSG(("not drawing page B: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight));
		}
		else if (adjustedTop > y + height)
		{
			// the top of this page is beyond the end
			// of the clipping region, so we don't need
			// to draw it.

			xxx_UT_DEBUGMSG(("not drawing page C: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d y=%d h=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight,
							 y,height));
		}
		else if (adjustedBottom < y)
		{
			// the bottom of this page is above the top
			// of the clipping region, so we don't need
			// to draw it.

			xxx_UT_DEBUGMSG(("not drawing page D: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d y=%d h=%d\n",
							 iPageHeight,
							 curY,
							 m_yScrollOffset,
							 m_iWindowHeight,
							 y,height));
		}
		else
		{
			// this page is on screen and intersects the clipping region,
			// so we *DO* draw it.

			xxx_UT_DEBUGMSG(("drawing page E: iPageHeight=%d curY=%d nPos=%d m_iWindowHeight=%d y=%d h=%d\n",
							 iPageHeight,curY,m_yScrollOffset,m_iWindowHeight,y,height));

			dg_DrawArgs da;

			da.bDirtyRunsOnly = bDirtyRunsOnly;
			da.pG = m_pG;
			da.xoff = fl_PAGEVIEW_MARGIN_X - m_xScrollOffset;
			da.yoff = adjustedTop;
			if (m_bSelection)
			{
				if (m_iSelectionAnchor < _getPoint())
				{
					da.iSelPos1 = m_iSelectionAnchor;
					da.iSelPos2 = _getPoint();
				}
				else
				{
					da.iSelPos1 = _getPoint();
					da.iSelPos2 = m_iSelectionAnchor;
				}
			}
			else
			{
				da.iSelPos1 = da.iSelPos2 = 0;
			}

			UT_sint32 adjustedLeft  = fl_PAGEVIEW_MARGIN_X - m_xScrollOffset;
			UT_sint32 adjustedRight = adjustedLeft + iPageWidth;

			adjustedBottom -= fl_PAGEVIEW_PAGE_SEP;

			if (!bDirtyRunsOnly || pPage->needsRedraw())
			{
				UT_RGBColor clrPaper(255,255,255);
				m_pG->fillRect(clrPaper,adjustedLeft+1,adjustedTop+1,iPageWidth-1,iPageHeight-1);
			}

			pPage->draw(&da);

			// draw page decorations
			UT_RGBColor clr(0,0,0);		// black
			m_pG->setColor(clr);

			// one pixel border
			m_pG->drawLine(adjustedLeft, adjustedTop, adjustedRight, adjustedTop);
			m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
			m_pG->drawLine(adjustedRight, adjustedBottom, adjustedLeft, adjustedBottom);
			m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedLeft, adjustedTop);

			// fill to right of page
			m_pG->fillRect(clrMargin, adjustedRight + 1, adjustedTop, m_iWindowWidth - (adjustedRight + 1), iPageHeight + 1);

			// fill separator below page
			m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, fl_PAGEVIEW_PAGE_SEP);

			// two pixel drop shadow
			adjustedLeft += 3;
			adjustedBottom += 1;
			m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight+1, adjustedBottom);
			adjustedBottom += 1;
			m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight+1, adjustedBottom);

			adjustedTop += 3;
			adjustedRight += 1;
			m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom+1);
			adjustedRight += 1;
			m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom+1);
		}

		curY += iPageHeight + fl_PAGEVIEW_PAGE_SEP;

		pPage = pPage->getNext();
	}

	if (curY < iDocHeight)
	{
		// fill below bottom of document
		UT_sint32 y = curY - m_yScrollOffset + 1;
		UT_sint32 h = m_iWindowHeight - y;

		m_pG->fillRect(clrMargin, 0, y, m_iWindowWidth, h);
	}

	if (!bDirtyRunsOnly)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	if (bClip)
	{
		m_pG->setClipRect(NULL);
	}

#if 0
	{
		// Some test code for the graphics interface.
		UT_RGBColor clrRed(255,0,0);
		m_pG->setColor(clrRed);
		m_pG->drawLine(10,10,20,10);
		m_pG->drawLine(20,11,30,11);
		m_pG->fillRect(clrRed,50,10,10,10);
		m_pG->fillRect(clrRed,60,20,10,10);
	}
#endif

}

#if defined(PT_TEST) || defined(FMT_TEST)
void FV_View::Test_Dump(void)
{
	char buf[100];
	static UT_uint32 x = 0;

	x++;
	
#if defined(PT_TEST)
	sprintf(buf,"dump.ptbl.%ld",x);
	FILE * fpDumpPTbl = fopen(buf,"w");
	m_pDoc->__dump(fpDumpPTbl);
	fclose(fpDumpPTbl);
#endif
#if defined(FMT_TEST)
	sprintf(buf,"dump.fmt.%ld",x);
	FILE * fpDumpFmt = fopen(buf,"w");
	m_pLayout->__dump(fpDumpFmt);
	fclose(fpDumpFmt);
#endif
}
#endif

void FV_View::cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos)
{
#define HACK_LINE_HEIGHT				20 // TODO Fix this!!
	
	UT_sint32 lineHeight = iPos;
	UT_sint32 docHeight = 0;
	UT_Bool bVertical = UT_FALSE;
	UT_Bool bHorizontal = UT_FALSE;
	
	_eraseInsertionPoint();	

	docHeight = m_pLayout->getHeight();
	
	if (lineHeight == 0)
		lineHeight = HACK_LINE_HEIGHT;
	
	UT_sint32 yoff = m_yScrollOffset;
	UT_sint32 xoff = m_xScrollOffset;
	
	switch(cmd)
	{
	case AV_SCROLLCMD_PAGEDOWN:
		yoff += m_iWindowHeight - HACK_LINE_HEIGHT;
		bVertical = UT_TRUE;
		break;
	case AV_SCROLLCMD_PAGEUP:
		yoff -= m_iWindowHeight - HACK_LINE_HEIGHT;
		bVertical = UT_TRUE;
		break;
	case AV_SCROLLCMD_PAGELEFT:
		xoff -= m_iWindowWidth;
		bHorizontal = UT_TRUE;
		break;
	case AV_SCROLLCMD_PAGERIGHT:
		xoff += m_iWindowWidth;
		bHorizontal = UT_TRUE;
		break;
	case AV_SCROLLCMD_LINEDOWN:
		yoff += lineHeight;
		bVertical = UT_TRUE;
		break;
	case AV_SCROLLCMD_LINEUP:
		yoff -= lineHeight;
		bVertical = UT_TRUE;
		break;
	case AV_SCROLLCMD_LINELEFT:
		xoff -= lineHeight;
		bHorizontal = UT_TRUE;
		break;
	case AV_SCROLLCMD_LINERIGHT:
		xoff += lineHeight;
		bHorizontal = UT_TRUE;
		break;
	case AV_SCROLLCMD_TOTOP:
		yoff = 0;
		bVertical = UT_TRUE;
		break;
	case AV_SCROLLCMD_TOPOSITION:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		break;
	case AV_SCROLLCMD_TOBOTTOM:
		fp_Page* pPage = m_pLayout->getFirstPage();
		UT_sint32 iDocHeight = fl_PAGEVIEW_MARGIN_Y;
		while (pPage)
		{
			iDocHeight += pPage->getHeight() + fl_PAGEVIEW_PAGE_SEP;
			pPage = pPage->getNext();
		}
		yoff = iDocHeight;
		bVertical = UT_TRUE;
		break;
	}

	if (bVertical)
	{
		if (yoff < 0)
			yoff = 0;
				
		sendVerticalScrollEvent(yoff);
	}

	if (bHorizontal)
	{
		if (xoff < 0)
			xoff = 0;
		sendHorizontalScrollEvent(xoff);
	}
}

UT_Bool FV_View::isLeftMargin(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	return bBOL;
}

void FV_View::cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd)
{
	warpInsPtToXY(xPos, yPos);

	_eraseInsertionPoint();

	PT_DocPosition iPosLeft = _getDocPos(dpBeg, UT_FALSE);
	PT_DocPosition iPosRight = _getDocPos(dpEnd, UT_FALSE);

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	m_iSelectionAnchor = iPosLeft;
	_setPoint(iPosRight);

	if (iPosLeft == iPosRight)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
		return;
	}

	m_bSelection = UT_TRUE;
	
	_drawSelection();

	notifyListeners(AV_CHG_EMPTYSEL);
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
		{
			m_pDoc->clearTemporarySpanFmt();
			notifyListeners(AV_CHG_FMTCHAR); // ensure that toolbar doesn't get stale...
		}
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

	PT_DocPosition posOld = m_iInsPoint;

	m_bPointEOL = UT_FALSE;
	
	if (bForward)
	{
		m_iInsPoint += countChars;
	}
	else
	{
		m_iInsPoint -= countChars;
	}

	PT_DocPosition posBOD;
	PT_DocPosition posEOD;
	UT_Bool bRes;

	bRes = m_pDoc->getBounds(UT_FALSE, posBOD);
	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint < (UT_sint32) posBOD)
	{
		m_iInsPoint = posBOD;

		if (m_iInsPoint != posOld)
			_clearPointAP(UT_TRUE);

		return UT_FALSE;
	}

	bRes = m_pDoc->getBounds(UT_TRUE, posEOD);
	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint > (UT_sint32) posEOD)
	{
		m_iInsPoint = posEOD;

		if (m_iInsPoint != posOld)
			_clearPointAP(UT_TRUE);

		return UT_FALSE;
	}

	_clearPointAP(UT_TRUE);
	return UT_TRUE;
}
// -------------------------------------------------------------------------

UT_Bool FV_View::canDo(UT_Bool bUndo) const
{
	return m_pDoc->canDo(bUndo);
}

void FV_View::cmdUndo(UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	m_pDoc->undoCmd(count);

	m_pLayout->deleteEmptyColumnsAndPages();
	
	_updateScreen();
	
	notifyListeners(AV_CHG_DIRTY);
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		_drawSelection();
	}
}

void FV_View::cmdRedo(UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	m_pDoc->redoCmd(count);

	m_pLayout->deleteEmptyColumnsAndPages();
	
	_updateScreen();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		_drawSelection();
	}
}

void FV_View::cmdSave(void)
{
	m_pDoc->save(IEFT_AbiWord_1);
	notifyListeners(AV_CHG_SAVE);
}

void FV_View::cmdSaveAs(const char * szFilename)
{
	m_pDoc->saveAs(szFilename, IEFT_AbiWord_1);
	notifyListeners(AV_CHG_SAVE);
}

void FV_View::cmdCut(void)
{
	cmdCopy();
	_deleteSelection();

	m_pLayout->deleteEmptyColumnsAndPages();
		
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::cmdCopy(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	
	AP_Clipboard* pClip = AP_App::getClipboard();

	UT_uint32 iPos1, iPos2;

	if (m_iSelectionAnchor < _getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = _getPoint();
	}
	else
	{
		iPos1 = _getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	fp_Run* pRunUnused;
	UT_uint32 uheight;
	UT_sint32 x;
	UT_sint32 y;
	fl_BlockLayout* pBlock1;
	fl_BlockLayout* pBlock2;
		
	_findPositionCoords(iPos1, UT_FALSE, x, y, uheight, &pBlock1, &pRunUnused);
	_findPositionCoords(iPos2, UT_FALSE, x, y, uheight, &pBlock2, &pRunUnused);

	UT_uint32 iTotalDataLength = 0;

	fl_BlockLayout* pCurBlock;
	UT_Bool bDone;

	bDone = UT_FALSE;
	pCurBlock = pBlock1;
	while (!bDone)
	{		
		UT_GrowBuf pgb(1024);
		UT_Bool bRes = pCurBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);
		UT_sint32 iBlockOffset = pCurBlock->getPosition();

		// pSpan is unused and bothers my compiler.
		//const UT_UCSChar* pSpan = pgb.getPointer(0);
		UT_uint32 iBlockLength = pgb.getLength();

		UT_uint32 iBlockPos1;
		UT_uint32 iBlockPos2;

		if (iPos1 < (UT_uint32) iBlockOffset)
		{
			iBlockPos1 = 0;
		}
		else
		{
			iBlockPos1 = iPos1 - iBlockOffset;
		}

		UT_Bool bCRLF = UT_FALSE;
		
		if (iPos2 > (iBlockOffset + iBlockLength))
		{
			iBlockPos2 = iBlockLength;
			bCRLF = UT_TRUE;
		}
		else
		{
			iBlockPos2 = iPos2 - iBlockOffset;
		}

		iTotalDataLength += (iBlockPos2 - iBlockPos1);
		if (bCRLF)
		{
			iTotalDataLength += 2;	// for the CR/LF at the end of every para
		}
		
		if (pCurBlock == pBlock2)
		{
			break;
		}
		else
		{
			pCurBlock = pCurBlock->getNext(UT_TRUE);
		}
	}

	iTotalDataLength++;	// for the ending zero
	
	char* pData = new char[iTotalDataLength];
	UT_ASSERT(pData); // TODO check outofmem
	UT_uint32 iDataLen = 0;
	
	bDone = UT_FALSE;
	pCurBlock = pBlock1;
	while (!bDone)
	{		
		UT_GrowBuf pgb(1024);
		UT_Bool bRes = pCurBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);
		UT_sint32 iBlockOffset = pCurBlock->getPosition();
		
		const UT_UCSChar* pSpan = pgb.getPointer(0);
		UT_uint32 iBlockLength = pgb.getLength();

		UT_uint32 iBlockPos1;
		UT_uint32 iBlockPos2;

		if (iPos1 < (UT_uint32) iBlockOffset)
		{
			iBlockPos1 = 0;
		}
		else
		{
			iBlockPos1 = iPos1 - iBlockOffset;
		}

		UT_Bool bCRLF = UT_FALSE;
		
		if (iPos2 > (iBlockOffset + iBlockLength))
		{
			iBlockPos2 = iBlockLength;
			bCRLF = UT_TRUE;
		}
		else
		{
			iBlockPos2 = iPos2 - iBlockOffset;
		}
		
		for (PT_DocPosition iPos = iBlockPos1; iPos < iBlockPos2; iPos++)
		{
			UT_UCSChar ch = pSpan[iPos];

			UT_ASSERT(ch != 0);
		
			// Note that we are stripping the other Unicode byte here.
			pData[iDataLen++] = (char) ch;
		}

		if (bCRLF)
		{
			// now add the CRLF
			pData[iDataLen++] = 13;
			pData[iDataLen++] = 10;
		}
		
		if (pCurBlock == pBlock2)
		{
			break;
		}
		else
		{
			pCurBlock = pCurBlock->getNext(UT_TRUE);
		}
	}
	
	pData[iDataLen++] = 0;

	UT_ASSERT(iDataLen == iTotalDataLength);
	
	if (pClip->open())
	{
		pClip->clear();
		pClip->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT, pData, iTotalDataLength);
		pClip->close();
	}

	delete pData;
	
	// TODO add the text to the clip in RTF as well.
	
	notifyListeners(AV_CHG_CLIPBOARD);
}

void FV_View::cmdPaste(void)
{
	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.
	
	m_pDoc->beginUserAtomicGlob();
	_doPaste();
	m_pDoc->endUserAtomicGlob();
}

void FV_View::_doPaste(void)
{
	// internal portion of paste operation.
	
	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	AP_Clipboard* pClip = AP_App::getClipboard();
	if (pClip->open())
	{
		// TODO support paste of RTF
		
		if (pClip->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		{
			UT_sint32 iLen = pClip->getDataLen(AP_CLIPBOARD_TEXTPLAIN_8BIT);

			unsigned char* pData = new unsigned char[iLen];
			UT_ASSERT(pData); // TODO outofmem

			pClip->getData(AP_CLIPBOARD_TEXTPLAIN_8BIT, (char*) pData);

			UT_UCSChar* pUCSData = new UT_UCSChar[iLen];
			for (int i=0; i<iLen; i++)
			{
				pUCSData[i] = pData[i];
			}
			delete pData;

			UT_UCSChar* pStart = pUCSData;
			UT_UCSChar* pCur = pStart;
			for (;;)
			{
				while (
					(*pCur != 13)
					&& (*pCur != 10)
					&& *pCur
					&& ((pCur - pStart) < iLen)
					)
				{
					pCur++;
				}

				if ((pCur - pStart) > 0)
				{
					m_pDoc->insertSpan(_getPoint(), pStart, pCur - pStart);
				}

				if (
					(*pCur == 13)
					|| (*pCur == 10)
					)
				{
					m_pDoc->insertStrux(_getPoint(), PTX_Block);
	
					if (*pCur == 13)
					{
						pCur++;
						if (*pCur == 10 && ((pCur - pStart) < iLen))
						{
							pCur++;
						}
					}
					else 
					{
						UT_ASSERT(*pCur == 10);
						
						pCur++;
					}
					
					if ((*pCur) && ((pCur - pStart) < iLen))
					{
						pStart = pCur;
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			

			delete pUCSData;
		}
		pClip->close();
	}

	m_pLayout->deleteEmptyColumnsAndPages();
		
	_updateScreen();
	
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

#if 0
void FV_View::_drawRuler(void)
{

}
#endif

UT_Bool FV_View::setSectionFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	_clearPointAP(UT_TRUE);
	_eraseInsertionPoint();

	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
		{
			posStart = m_iSelectionAnchor;
		}
		else
		{
			posEnd = m_iSelectionAnchor;
		}
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Section);

	_updateScreen();
	
	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	else
	{
		_drawSelection();
	}

	return bRet;
}

