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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_string.h"

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
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "ie_types.h"


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
{
	m_pParentData = pParentData;
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pG = m_pLayout->getGraphics();
//	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
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

	// initialize change cache
	m_chg.bUndo = UT_FALSE;
	m_chg.bRedo = UT_FALSE;
	m_chg.bDirty = UT_FALSE;
	m_chg.bSelection = UT_FALSE;
	m_chg.propsChar = NULL;
	m_chg.propsBlock = NULL;
}

FV_View::~FV_View()
{
	FREEP(m_chg.propsChar);
	FREEP(m_chg.propsBlock);
}
	
void* FV_View::getParentData() const
{
	return m_pParentData;
}

FL_DocLayout* FV_View::getLayout() const
{
	return m_pLayout;
}

UT_Bool FV_View::addListener(FV_Listener * pListener, 
							 FV_ListenerId * pListenerId)
{
	UT_uint32 kLimit = m_vecListeners.getItemCount();
	UT_uint32 k;

	// see if we can recycle a cell in the vector.
	
	for (k=0; k<kLimit; k++)
		if (m_vecListeners.getNthItem(k) == 0)
		{
			(void)m_vecListeners.setNthItem(k,pListener,NULL);
			goto ClaimThisK;
		}

	// otherwise, extend the vector for it.
	
	if (m_vecListeners.addItem(pListener,&k) != 0)
		return UT_FALSE;				// could not add item to vector

  ClaimThisK:

	// give our vector index back to the caller as a "Listener Id".
	
	*pListenerId = k;
	return UT_TRUE;
}

UT_Bool FV_View::removeListener(FV_ListenerId listenerId)
{
	return (m_vecListeners.setNthItem(listenerId,NULL,NULL) == 0);
}

UT_Bool FV_View::notifyListeners(const FV_ChangeMask hint)
{
	/*
		IDEA: The view caches its change state as of the last notification, 
		to minimize noise from duplicate notifications.  
	*/
	UT_ASSERT(hint != FV_CHG_NONE);
	FV_ChangeMask mask = hint;
	
	if (mask & FV_CHG_DO)
	{
		UT_Bool bUndo = canDo(UT_TRUE);
		UT_Bool bRedo = canDo(UT_FALSE);

		if ((m_chg.bUndo == bUndo) && (m_chg.bRedo == bRedo))
		{
			mask ^= FV_CHG_DO;
		}
		else
		{
			if (m_chg.bUndo != bUndo)
				m_chg.bUndo = bUndo;
			if (m_chg.bRedo != bRedo)
				m_chg.bRedo = bRedo;
		}
	}
	
	if (mask & FV_CHG_DIRTY)
	{
		UT_Bool bDirty = m_pDoc->isDirty();

		if (m_chg.bDirty != bDirty)
			m_chg.bDirty = bDirty;
		else
			mask ^= FV_CHG_DIRTY;
	}

	if (mask & FV_CHG_EMPTYSEL)
	{
		UT_Bool bSel = !isSelectionEmpty();

		if (m_chg.bSelection != bSel)
			m_chg.bSelection = bSel;
		else
			mask ^= FV_CHG_EMPTYSEL;
	}

	if (mask & FV_CHG_FILENAME)
	{
		// NOTE: we don't attempt to filter this
	}

	if (mask & FV_CHG_FMTBLOCK)
	{
		// TODO: do something smart here, (ie, easier than getBlockFormat)
		// HYP: if FMTCHAR logic works, just clone it
	}

	if (mask & FV_CHG_FMTCHAR)
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
			mask ^= FV_CHG_FMTCHAR;
		}
	}
		
	// make sure there's something left

	if (mask == FV_CHG_NONE)
		return UT_FALSE;
	
	// notify listeners of a change.
		
	FV_ListenerId lid;
	FV_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).
	
	for (lid=0; lid<lidCount; lid++)
	{
		FV_Listener * pListener = (FV_Listener *)m_vecListeners.getNthItem(lid);
		if (pListener)
			pListener->notify(this,mask);
	}

	return UT_TRUE;
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
	if (isSelectionEmpty())
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
	UT_ASSERT(!isSelectionEmpty());

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
	UT_ASSERT(!isSelectionEmpty());

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
	
	UT_ASSERT(!isSelectionEmpty());

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

			UT_ASSERT(iPos >= pBlock->getPosition());
			UT_uint32 offset = iPos - pBlock->getPosition();
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

			UT_ASSERT(iPos >= pBlock->getPosition());
			UT_uint32 offset = iPos - pBlock->getPosition();
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
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	
	PT_DocPosition iPos = _getDocPos(dp);

	_setPoint(iPos, (dp == FV_DOCPOS_EOL));
	_updateInsertionPoint();
}

void FV_View::cmdCharMotion(UT_Bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
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

	notifyListeners(FV_CHG_MOTION);
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

	_drawSelectionOrInsertionPoint();

	return bResult;
}

void FV_View::insertParagraphBreak()
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

	m_pDoc->insertStrux(_getPoint(), PTX_Block);
	
	_drawSelectionOrInsertionPoint();
}

UT_Bool FV_View::setCharFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	_eraseSelectionOrInsertionPoint();

	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,NULL,properties);

	_drawSelectionOrInsertionPoint();

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

		/*
			Likewise, findPointCoords will return the run to the right 
			of the specified position, so we need to stop looking one 
			position to the left. 
		*/
		posEnd--;
	}

	pBlock->getSpanAttrProp(posStart - pBlock->getPosition(),&pSpanAP);
	pBlock->getAttrProp(&pBlockAP);

	v.addItem(new _fmtPair("font-family",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-size",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-weight",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-style",pSpanAP,pBlockAP,pSectionAP));
	v.addItem(new _fmtPair("font-xlfd",pSpanAP,pBlockAP,pSectionAP));
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
				pBlock = pBlock->getNext();

				if (!pBlock)
				{
					// TODO: go to first block of next section
					// for now, just bail
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
			UT_ASSERT((pRun->getLength()>0) || (pRun->getBlockOffset()>0));
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

	UT_VECTOR_PURGEALL(_fmtPair,v);

	*pProps = props;

	return UT_TRUE;
}

UT_Bool FV_View::setBlockFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	_eraseSelectionOrInsertionPoint();

	PT_DocPosition posStart = _getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Block);

	_drawSelectionOrInsertionPoint();

	return bRet;
}

UT_Bool FV_View::getBlockFormat(const XML_Char *** pProps)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
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

			pBlock = pBlock->getNext();

			if (!pBlock)
			{
				// TODO: go to first block of next section
				// for now, just bail
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

	UT_VECTOR_PURGEALL(_fmtPair,v);

	*pProps = props;

	return UT_TRUE;
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
	if (!isSelectionEmpty())
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

	notifyListeners(FV_CHG_MOTION);
}

void FV_View::warpInsPtNextPrevLine(UT_Bool bNext)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
	}

	_moveInsPtNextPrevLine(bNext);

	_updateInsertionPoint();

	notifyListeners(FV_CHG_MOTION);
}

void FV_View::extSelNextPrevLine(UT_Bool bNext)
{
	if (isSelectionEmpty())
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

		if (isSelectionEmpty())
		{
			_resetSelection();
		}
	}

	notifyListeners(FV_CHG_MOTION);
}

void FV_View::extSelHorizontal(UT_Bool bForward, UT_uint32 count)
{
	if (isSelectionEmpty())
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

		if (isSelectionEmpty())
		{
			_resetSelection();
		}
	}

	notifyListeners(FV_CHG_MOTION);
}

void FV_View::extSelTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	_extSelToPos(iPos);

	notifyListeners(FV_CHG_MOTION);
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
	
	notifyListeners(FV_CHG_MOTION);
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
	
	if (isSelectionEmpty())
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

	if (!isSelectionEmpty())
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
	if (isSelectionEmpty())
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
	UT_ASSERT(isSelectionEmpty());
	
	_eraseInsertionPoint();

	_findPositionCoords(_getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_iPointHeight, NULL, NULL);
	
	_xorInsertionPoint();
}

void FV_View::_xorInsertionPoint()
{
	UT_ASSERT(isSelectionEmpty());
	
	UT_ASSERT(m_iPointHeight > 0);
	m_bPointVisible = !m_bPointVisible;

	UT_RGBColor clr(255,255,255);
	m_pG->setColor(clr);
	m_pG->xorLine(m_xPoint, m_yPoint, m_xPoint, m_yPoint + m_iPointHeight);
}

void FV_View::_eraseInsertionPoint()
{
	UT_ASSERT(isSelectionEmpty());
	
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

#if 0
	char buf[100];
	sprintf(buf,"dump.buffer.%d",x);
	FILE * fpDump = fopen(buf,"w");
	m_pDoc->__dump(fpDump);
	fclose(fpDump);
#endif
	
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

void FV_View::addScrollListener(FV_ScrollObj* pObj)
{
	m_scrollListeners.addItem((void *)pObj);
}

void FV_View::removeScrollListener(FV_ScrollObj* pObj)
{
	UT_sint32 count = m_scrollListeners.getItemCount();

	for (UT_sint32 i = 0; i < count; i++)
	{
		FV_ScrollObj* obj = (FV_ScrollObj*) m_scrollListeners.getNthItem(i);

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

		pObj->m_pfn(pObj->m_pData, xoff, yoff);
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

	if (!isSelectionEmpty())
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

	_drawSelectionOrInsertionPoint();
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

	_drawSelectionOrInsertionPoint();
}

void FV_View::cmdSave(void)
{
	m_pDoc->save(IEFT_AbiWord_1);
	notifyListeners(FV_CHG_SAVE);
}

void FV_View::cmdSaveAs(const char * szFilename)
{
	m_pDoc->saveAs(szFilename, IEFT_AbiWord_1);
	notifyListeners(FV_CHG_SAVE);
}

UT_Bool FV_View::pasteBlock(UT_UCSChar * text, UT_uint32 count)
{
	if (!isSelectionEmpty())
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
