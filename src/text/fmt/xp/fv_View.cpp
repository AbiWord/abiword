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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
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
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "ie_types.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_Prefs.h"

#include "sp_spell.h"

/****************************************************************/

class _fmtPair
{
public:
	_fmtPair(const XML_Char * p, 
			 const PP_AttrProp * c, const PP_AttrProp * b, const PP_AttrProp * s, 
			 PD_Document * pDoc, UT_Bool bExpandStyles)
	{
		m_prop = p;
		m_val  = PP_evalProperty(p,c,b,s, pDoc, bExpandStyles);
	}

	const XML_Char *	m_prop;
	const XML_Char *	m_val;
};

/****************************************************************/


FV_View::FV_View(XAP_App * pApp, void* pParentData, FL_DocLayout* pLayout)
:	AV_View(pApp, pParentData),
	m_bCursorIsOn(UT_FALSE)
{
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pG = m_pLayout->getGraphics();
//	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	m_iInsPoint = 0;
	m_iPointHeight = 0;
	m_bPointEOL = UT_FALSE;
	m_bSelection = UT_FALSE;
//	m_bPointAP = UT_FALSE;
	m_pAutoScrollTimer 	= NULL;
	m_pAutoCursorTimer  = NULL;
	
	// initialize prefs cache
	pApp->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &m_bCursorBlink);

	// initialize prefs listener
	pApp->getPrefs()->addListener( _prefsListener, this );
	
	// initialize change cache
	m_chg.bUndo = UT_FALSE;
	m_chg.bRedo = UT_FALSE;
	m_chg.bDirty = UT_FALSE;
	m_chg.bSelection = UT_FALSE;
	m_chg.iColumn = 0;					// current column number
	m_chg.propsChar = NULL;
	m_chg.propsBlock = NULL;
	m_chg.propsSection = NULL;

	pLayout->setView(this);
		
	pLayout->formatAll();
	
	moveInsPtTo(FV_DOCPOS_BOD);
	m_iSelectionAnchor = getPoint();
	_resetSelection();
	_fixInsertionPointCoords();

	_m_findNextString = NULL;

	m_wrappedEnd = UT_FALSE;
	m_startPosition = 0;
}

FV_View::~FV_View()
{
	// remove prefs listener
	m_pApp->getPrefs()->removeListener( _prefsListener, this );

	DELETEP(m_pAutoScrollTimer);
	DELETEP(m_pAutoCursorTimer);
	
	FREEP(_m_findNextString);
	
	FREEP(m_chg.propsChar);
	FREEP(m_chg.propsBlock);
	FREEP(m_chg.propsSection);
}
	
void FV_View::focusChange(AV_Focus focus)
{
	m_focus=focus;
	switch(focus)
	{
	case AV_FOCUS_HERE:
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		else
		{
			_drawSelection();
		}
		break;
	case AV_FOCUS_NEARBY:
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
		else
		{
			_drawSelection();
		}
		break;
	case AV_FOCUS_NONE:
		if (isSelectionEmpty())
		{
			_eraseInsertionPoint();
		}
		else
		{
			if (!m_bSelection)
			{
				_resetSelection();
				break;
			}

			UT_uint32 iPos1, iPos2;

			if (m_iSelectionAnchor < getPoint())
			{
				iPos1 = m_iSelectionAnchor;
				iPos2 = getPoint();
			}
			else
			{
				iPos1 = getPoint();
				iPos2 = m_iSelectionAnchor;
			}

			_clearBetweenPositions(iPos1, iPos2, UT_TRUE);
			_drawBetweenPositions(iPos1, iPos2);
		}
	}
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

	if (mask & AV_CHG_FMTSECTION)
	{
		/*
		  The following brute-force solution works, but is atrociously 
		  expensive, so we should avoid using it whenever feasible.  
		*/
		const XML_Char ** propsSection = NULL;
		getSectionFormat(&propsSection);

		UT_Bool bMatch = UT_FALSE;

		if (propsSection && m_chg.propsSection)
		{
			bMatch = UT_TRUE;

			int i=0;

			while (bMatch)
			{
				if (!propsSection[i] || !m_chg.propsSection[i])
				{
					bMatch = (propsSection[i] == m_chg.propsSection[i]);
					break;
				}

				if (UT_stricmp(propsSection[i], m_chg.propsSection[i]))
				{
					bMatch = UT_FALSE;
					break;
				}

				i++;
			}
		}

		if (!bMatch)
		{
			FREEP(m_chg.propsSection);
			m_chg.propsSection = propsSection;
		}
		else
		{
			FREEP(propsSection);
			mask ^= AV_CHG_FMTSECTION;
		}
	}

	if (mask & AV_CHG_FMTSTYLE)
	{
		// NOTE: we don't attempt to filter this
		// TODO: we probably should
	}

	if (mask & AV_CHG_PAGECOUNT)
	{
		// NOTE: we don't attempt to filter this
	}

	if (mask & AV_CHG_COLUMN)
	{
		// computing which column the cursor is in is rather expensive,
		// i'm not sure it's worth the effort here...
		
		fp_Run * pRun;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;

		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, heightCaret, NULL, &pRun);

		fp_Container * pContainer = pRun->getLine()->getContainer();

		if (pContainer->getType() == FP_CONTAINER_COLUMN)
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			
			UT_uint32 nCol=0;
			fp_Column * pNthColumn = pColumn->getLeader();
			while (pNthColumn && (pNthColumn != pColumn))
			{
				nCol++;
				pNthColumn = pNthColumn->getFollower();
			}

			if (nCol != m_chg.iColumn)
			{
				m_chg.iColumn = nCol;
			}
			else
			{
				mask ^= AV_CHG_COLUMN;
			}
		}
	}
	
	// base class does the rest
	return AV_View::notifyListeners(mask);
}

void FV_View::_swapSelectionOrientation(void)
{
	// reverse the direction of the current selection
	// without changing the screen.

	UT_ASSERT(!isSelectionEmpty());
	PT_DocPosition curPos = getPoint();
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
	
	PT_DocPosition curPos = getPoint();
	
	UT_ASSERT(curPos != m_iSelectionAnchor);
	UT_Bool bForwardSelection = (m_iSelectionAnchor < curPos);
	
	if (bForward != bForwardSelection)
	{
		_swapSelectionOrientation();
	}

	_clearSelection();

	return;
}

void FV_View::_eraseSelection(void)
{
	if (!m_bSelection)
	{
		_resetSelection();
		return;
	}

	UT_uint32 iPos1, iPos2;

	if (m_iSelectionAnchor < getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	_clearBetweenPositions(iPos1, iPos2, UT_TRUE);
}

void FV_View::_clearSelection(void)
{
	if (!m_bSelection)
	{
		_resetSelection();
		return;
	}

	UT_uint32 iPos1, iPos2;

	if (m_iSelectionAnchor < getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	_clearBetweenPositions(iPos1, iPos2, UT_TRUE);
	
	_resetSelection();

	_drawBetweenPositions(iPos1, iPos2);
}

void FV_View::_resetSelection(void)
{
	m_bSelection = UT_FALSE;
	m_iSelectionAnchor = getPoint();
}

void FV_View::cmdUnselectSelection(void)
{
	_clearSelection();
}

void FV_View::_drawSelection()
{
	UT_ASSERT(!isSelectionEmpty());

	if (m_iSelectionAnchor < getPoint())
	{
		_drawBetweenPositions(m_iSelectionAnchor, getPoint());
	}
	else
	{
		_drawBetweenPositions(getPoint(), m_iSelectionAnchor);
	}
}

void FV_View::_setSelectionAnchor(void)
{
	m_bSelection = UT_TRUE;
	m_iSelectionAnchor = getPoint();
}

void FV_View::_deleteSelection(void)
{
	// delete the current selection.
	// NOTE: this must clear the selection.

	UT_ASSERT(!isSelectionEmpty());

	PT_DocPosition iPoint = getPoint();
	UT_ASSERT(iPoint != m_iSelectionAnchor);

	UT_uint32 iSelAnchor = m_iSelectionAnchor;
	
	_eraseSelection();
	_resetSelection();

	UT_Bool bForward = (iPoint < iSelAnchor);
	if (bForward)
	{
		m_pDoc->deleteSpan(iPoint, iSelAnchor);
	}
	else
	{
		m_pDoc->deleteSpan(iSelAnchor, iPoint);
	}
}

UT_Bool FV_View::isSelectionEmpty(void) const
{
	if (!m_bSelection)
	{
		return UT_TRUE;
	}
	
	PT_DocPosition curPos = getPoint();
	if (curPos == m_iSelectionAnchor)
	{
		return UT_TRUE;
	}

	return UT_FALSE;
}

PT_DocPosition FV_View::_getDocPos(FV_DocPos dp, UT_Bool bKeepLooking)
{
	return _getDocPosFromPoint(getPoint(),dp,bKeepLooking);
}

PT_DocPosition FV_View::_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, UT_Bool bKeepLooking)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;

	PT_DocPosition iPos;

	// this gets called from ctor, so get out quick
	if (dp == FV_DOCPOS_BOD)
	{
		UT_Bool bRes = m_pDoc->getBounds(UT_FALSE, iPos);
		UT_ASSERT(bRes);

		return iPos;
	}

	// TODO: could cache these to save a lookup if point doesn't change
	fl_BlockLayout* pBlock = _findBlockAtPosition(iPoint);
	fp_Run* pRun = pBlock->findPointCoords(iPoint, m_bPointEOL,
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

			while (!pLastRun->isFirstRunOnLine() && pLastRun->isForcedBreak())
			{
				pLastRun = pLastRun->getPrev();
			}

			if(pLastRun->isForcedBreak())
			{
				iPos = pBlock->getPosition() + pLastRun->getBlockOffset();
			}
			else
			{
				iPos = pBlock->getPosition() + pLastRun->getBlockOffset() + pLastRun->getLength();
			}
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
#if 0
// TODO this piece of code attempts to go back
// TODO to the previous block if we are on the
// TODO edge.  this causes bug #92 (double clicking
// TODO on the first line of a paragraph selects
// TODO current paragraph and the previous paragraph).
// TODO i'm not sure why it is here.
// TODO
// TODO it's here because it makes control-up-arrow
// TODO when at the beginning of paragraph work. this
// TODO problem is logged as bug #403.
// TODO
			// are we already there?
			if (iPos == pBlock->getPosition())
			{
				// yep.  is there a prior block?
				if (!pBlock->getPrevBlockInDocument())
					break;

				// yep.  look there instead
				pBlock = pBlock->getPrevBlockInDocument();
			}
#endif /* 0 */
			
			iPos = pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_EOB:
		{
			if (pBlock->getNextBlockInDocument())
			{
				// BOB for next block
				pBlock = pBlock->getNextBlockInDocument();
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
				pBlock = pBlock->getPrevBlockInDocument();

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

	case FV_DOCPOS_EOW_MOVE:
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
				pBlock = pBlock->getNextBlockInDocument();

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
			
			// Needed so ctrl-right arrow will work
			// This is the code that was causing bug 10
			// There is still some weird behavior that should be investigated

			for (; offset < pgb.getLength(); offset++)
			{
		       	    if (!UT_isWordDelimiter(pSpan[offset]))
				break;
			}
			
			for (; offset < pgb.getLength(); offset++)
			{
				if (!UT_isWordDelimiter(pSpan[offset]))
				{				  
				  if (bBetween)
				    break;
				}
				else if (pSpan[offset] != ' ')
				    break;
				else
					bBetween = UT_TRUE;
			}

	     		iPos = offset + pBlock->getPosition();
		}
		break;

	case FV_DOCPOS_EOW_SELECT:
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
				pBlock = pBlock->getNextBlockInDocument();

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
			
			// Needed so ctrl-right arrow will work
			// This is the code that was causing bug 10
			// There is still some weird behavior that should be investigated
			/*
			for (; offset < pgb.getLength(); offset++)
			{
		       	    if (!UT_isWordDelimiter(pSpan[offset]))
				break;
			}
			*/
			for (; offset < pgb.getLength(); offset++)
			{
				if (UT_isWordDelimiter(pSpan[offset]))
				{				  
				  if (bBetween)
				    break;
				}
				else if (pSpan[offset] == ' ')
				    break;
				else
					bBetween = UT_TRUE;
			}

	     		iPos = offset + pBlock->getPosition();
		}
		break;


	case FV_DOCPOS_BOP: 
		{
			fp_Container* pContainer = pLine->getContainer();
			fp_Page* pPage = pContainer->getPage();

			iPos = pPage->getFirstLastPos(UT_TRUE);
		}
		break;

	case FV_DOCPOS_EOP:
		{
			fp_Container* pContainer = pLine->getContainer();
			fp_Page* pPage = pContainer->getPage();

			iPos = pPage->getFirstLastPos(UT_FALSE);
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
		_clearSelection();
	else
		_eraseInsertionPoint();
	
	PT_DocPosition iPos = _getDocPos(dp);

	if (iPos != getPoint())
	{
		UT_Bool bPointIsValid = (getPoint() >= _getDocPos(FV_DOCPOS_BOD));
		if (bPointIsValid)
			_clearIfAtFmtMark(getPoint());
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
	if (dp != getPoint())
		_clearIfAtFmtMark(getPoint());

	_setPoint(dp, /* (dp == FV_DOCPOS_EOL) */ UT_FALSE);  // is this bool correct?
	
	_fixInsertionPointCoords();
	cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32) (m_yPoint + m_iPointHeight/2 - m_iWindowHeight/2));
	cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32) (m_xPoint - m_iWindowWidth/2));
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

	PT_DocPosition iPoint = getPoint();
	if (!_charMotion(bForward, count))
	{
	 	_setPoint(iPoint);
	}
	else
	{
		PT_DocPosition iPoint1 = getPoint();
		if ( iPoint1 == iPoint )
		  {
		    if(!_charMotion(bForward, count))   
		      {
			_setPoint(iPoint);
			notifyListeners(AV_CHG_MOTION);
			return;
		      }
		  }
		_updateInsertionPoint();
	}
	notifyListeners(AV_CHG_MOTION);
}

fl_BlockLayout* FV_View::_findBlockAtPosition(PT_DocPosition pos) const
{
	return m_pLayout->findBlockAtPosition(pos);
}

UT_Bool FV_View::cmdCharInsert(UT_UCSChar * text, UT_uint32 count, UT_Bool bForce)
{
	UT_Bool bResult;

	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
		bResult = m_pDoc->insertSpan(getPoint(), text, count);
		m_pDoc->endUserAtomicGlob();
	} 
	else 
	{
		_eraseInsertionPoint();

		UT_Bool bOverwrite = (!m_bInsertMode && !bForce);

		if (bOverwrite)
		{
			// we need to glob when overwriting
			m_pDoc->beginUserAtomicGlob();
			cmdCharDelete(UT_TRUE,count);
		}

		bResult = m_pDoc->insertSpan(getPoint(), text, count);

		if (bOverwrite)
		{
			m_pDoc->endUserAtomicGlob();
		}
	}

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bResult;
}

void FV_View::insertSectionBreak(void)
{
	m_pDoc->beginUserAtomicGlob();

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
	// before inserting a section break, we insert a block break
	UT_uint32 iPoint = getPoint();
	
	m_pDoc->insertStrux(iPoint, PTX_Block);
	m_pDoc->insertStrux(iPoint, PTX_Section);

	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

void FV_View::insertParagraphBreak(void)
{
	UT_Bool bDidGlob = UT_FALSE;

	if (!isSelectionEmpty())
	{
		bDidGlob = UT_TRUE;
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).

	m_pDoc->insertStrux(getPoint(), PTX_Block);

	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

UT_Bool FV_View::setStyle(const XML_Char * style)
{
	UT_Bool bRet;

	PT_DocPosition posStart = getPoint();
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

	// lookup the current style
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(style, &pStyle);
	if (!pStyle)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
	}

	UT_Bool bCharStyle = pStyle->isCharStyle();

	const XML_Char * attribs[] = { PT_STYLE_ATTRIBUTE_NAME, 0, 0 };
	attribs[1] = style;

	if (bCharStyle)
	{
		// set character-level style
		if (isSelectionEmpty())
		{
			_eraseInsertionPoint();
		}

		_clearIfAtFmtMark(getPoint());	// TODO is this correct ??
		_eraseSelection();
                
		bRet = m_pDoc->changeSpanFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL);
	}
	else
	{
		// set block-level style
		_eraseInsertionPoint();

		_clearIfAtFmtMark(getPoint());	// TODO is this correct ??

		// NB: clear explicit props at both block and char levels
		bRet = m_pDoc->changeStruxFmt(PTC_AddStyle,posStart,posEnd,attribs,NULL,PTX_Block);
	}

	_generalUpdate();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	return bRet;
}


static const XML_Char * x_getStyle(const PP_AttrProp * pAP, UT_Bool bBlock)
{
	UT_ASSERT(pAP);
	const XML_Char* sz = NULL;

	pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, sz);

	// TODO: should we have an explicit default for char styles? 
	if (!sz && bBlock)
		sz = "Normal";

	return sz;
}

UT_Bool FV_View::getStyle(const XML_Char ** style)
{
	UT_Bool bCharStyle = UT_FALSE;
	const XML_Char * szChar = NULL;
	const XML_Char * szBlock = NULL;

	const PP_AttrProp * pBlockAP = NULL;

	/*
		IDEA: We want to know the style, if it's constant across the 
		entire selection.  Usually, this will be a block-level style. 
		However, if the entire span has the same char-level style, 
		we'll report that instead. 
	*/
	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	UT_Bool bSelEmpty = isSelectionEmpty();

	if (!bSelEmpty)
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	// 1. get block style at insertion point
	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	pBlock->getAttrProp(&pBlockAP);

	szBlock = x_getStyle(pBlockAP, UT_TRUE);

	// 2. prune if block style varies across selection
	if (!bSelEmpty)
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock && (pBlock != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			UT_Bool bCheck = UT_FALSE;

			pBlock = pBlock->getNextBlockInDocument();

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
				const XML_Char* sz = x_getStyle(pBlockAP, UT_TRUE);
				
				if (UT_stricmp(sz, szBlock))
				{
					// doesn't match, so stop looking
					szBlock = NULL;
					pBlock = NULL;
					break;
				}
			}
		}
	}

	// NOTE: if block style varies, no need to check char style

	if (szBlock && szBlock[0])
	{
		const PP_AttrProp * pSpanAP = NULL;

		// 3. locate char style at insertion point
		UT_sint32 xPoint;
		UT_sint32 yPoint;
		UT_sint32 iPointHeight;

		fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
		UT_uint32 blockPosition = pBlock->getPosition();
		fp_Run* pRun = pBlock->findPointCoords(posStart, UT_FALSE,
											   xPoint, yPoint, iPointHeight);
		UT_Bool bLeftSide = UT_TRUE;

		// TODO consider adding indexAP from change record to the
		// TODO runs so that we can just use it here.

		if (!bSelEmpty)
		{
			// we want the interior of the selection -- and not the
			// format to the left of the start of the selection.
			bLeftSide = UT_FALSE;
			
			/*
			  Likewise, findPointCoords will return the run to the right 
			  of the specified position, so we need to stop looking one 
			  position to the left. 
			*/
			posEnd--;
		}

		pBlock->getSpanAttrProp( (posStart - blockPosition), bLeftSide, &pSpanAP);

		if (pSpanAP)
		{
			szChar = x_getStyle(pSpanAP, UT_FALSE);
			bCharStyle = (szChar && szChar[0]);
		}

		// 4. prune if char style varies across selection
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
					pBlock = pBlock->getNextBlockInDocument();
					if (!pBlock)		// at EOD, so just bail
						break;
					pRun = pBlock->getFirstRun();
				}

				// did span format change?

				pAP = NULL;
				pBlock->getSpanAttrProp(pRun->getBlockOffset()+pRun->getLength(),UT_TRUE,&pAP);
				if (pAP && (pSpanAP != pAP))
				{
					pSpanAP = pAP;
					bCheck = UT_TRUE;
				}

				if (bCheck)
				{
					const XML_Char* sz = x_getStyle(pSpanAP, UT_TRUE);
					UT_Bool bHere = (sz && sz[0]);

					if ((bCharStyle != bHere) || (UT_stricmp(sz, szChar)))
					{
						// doesn't match, so stop looking
						bCharStyle = UT_FALSE;
						szChar = NULL;
						pRun = NULL;
						break;
					}
				}
			}
		}
	}

	*style = (bCharStyle ? szChar : szBlock);

	return UT_TRUE;
}

UT_Bool FV_View::setCharFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
	}

	PT_DocPosition posStart = getPoint();
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

	_eraseSelection();
	
	bRet = m_pDoc->changeSpanFmt(PTC_AddFmt,posStart,posEnd,NULL,properties);

	_generalUpdate();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bRet;
}

UT_Bool FV_View::getCharFormat(const XML_Char *** pProps, UT_Bool bExpandStyles)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;

	/*
		IDEA: We want to know character-level formatting properties, if
		they're constant across the entire selection.  To do so, we start 
		at the beginning of the selection, load 'em all into a vector, and 
		then prune any property that collides.
	*/
	PT_DocPosition posStart = getPoint();
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
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;

	fl_BlockLayout* pBlock = _findBlockAtPosition(posStart);
	UT_uint32 blockPosition = pBlock->getPosition();
	fp_Run* pRun = pBlock->findPointCoords(posStart, UT_FALSE,
										   xPoint, yPoint, iPointHeight);
	UT_Bool bLeftSide = UT_TRUE;

	// TODO consider adding indexAP from change record to the
	// TODO runs so that we can just use it here.

	if (!bSelEmpty)
	{
		// we want the interior of the selection -- and not the
		// format to the left of the start of the selection.
		bLeftSide = UT_FALSE;

		/*
		  Likewise, findPointCoords will return the run to the right 
		  of the specified position, so we need to stop looking one 
		  position to the left. 
		*/
		posEnd--;
	}

	pBlock->getSpanAttrProp( (posStart - blockPosition), bLeftSide, &pSpanAP);

	pBlock->getAttrProp(&pBlockAP);

	v.addItem(new _fmtPair("font-family",    pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("font-size",      pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("font-weight",    pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("font-style",     pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("text-decoration",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("text-position",	 pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("color",          pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));

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
				pBlock = pBlock->getNextBlockInDocument();

				if (!pBlock) 			// at EOD, so just bail
					break;

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

			pAP = NULL;
			pBlock->getSpanAttrProp(pRun->getBlockOffset()+pRun->getLength(),UT_TRUE,&pAP);
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

					const XML_Char * value = PP_evalProperty(f->m_prop,pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
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

	_clearIfAtFmtMark(getPoint());

	_eraseInsertionPoint();

	PT_DocPosition posStart = getPoint();
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

	_generalUpdate();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
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
		IDEA: We want to know block-level formatting properties, if
		they're constant across the entire selection.  To do so, we start 
		at the beginning of the selection, load 'em all into a vector, and 
		then prune any property that collides.
	*/
	PT_DocPosition posStart = getPoint();
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

	v.addItem(new _fmtPair("columns",   NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair("column-gap",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair("page-margin-left",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair("page-margin-top",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair("page-margin-right",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair("page-margin-bottom",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);
		fl_SectionLayout *pSectionEnd = pBlockEnd->getSectionLayout();

		while (pSection && (pSection != pSectionEnd))
		{
			const PP_AttrProp * pAP;
			UT_Bool bCheck = UT_FALSE;

			pSection = pSection->getNext();
			if (!pSection)				// at EOD, so just bail
				break;

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

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE);
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

UT_Bool FV_View::getBlockFormat(const XML_Char *** pProps,UT_Bool bExpandStyles)
{
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	UT_Vector v;
	UT_uint32 i;
	_fmtPair * f;

	/*
		IDEA: We want to know block-level formatting properties, if
		they're constant across the entire selection.  To do so, we start 
		at the beginning of the selection, load 'em all into a vector, and 
		then prune any property that collides.
	*/
	PT_DocPosition posStart = getPoint();
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

	v.addItem(new _fmtPair("text-align",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("text-indent",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-left",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-right",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-top",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("margin-bottom",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("line-height",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("tabstops",			  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("default-tab-interval",NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("keep-together",		  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("keep-with-next",	  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("orphans",			  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair("widows",			  NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));

	// 2. prune 'em as they vary across selection
	if (!isSelectionEmpty())
	{
		fl_BlockLayout* pBlockEnd = _findBlockAtPosition(posEnd);

		while (pBlock && (pBlock != pBlockEnd))
		{
			const PP_AttrProp * pAP;
			UT_Bool bCheck = UT_FALSE;

			pBlock = pBlock->getNextBlockInDocument();
			if (!pBlock)				// at EOD, so just bail
				break;

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

					const XML_Char * value = PP_evalProperty(f->m_prop,NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles);
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

	if (iPos == getPoint())
	{
		return;
	}

	_extSelToPos(iPos);
	_deleteSelection();

	_generalUpdate();

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
  const XML_Char * properties[] = { "font-family", NULL, 0};
  const XML_Char ** props_in = NULL;
  const XML_Char * currentfont;
  
	if (!isSelectionEmpty())
	{
		_deleteSelection();

		_generalUpdate();

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
	  /*
	    Code to deal with font boundary problem. 
TODO: This should really be fixed by someone who understands how this code
 works! In the meantime save current font to be restored after character is
deleted.
Blame Martin Sevior (msevior@physics.unimelb.edu.au) if this screws up
something
	  */
        	getCharFormat(&props_in);
		currentfont = UT_getAttribute("font-family",props_in);
		properties[1] = currentfont;

		_eraseInsertionPoint();
		UT_uint32 amt = count;
		UT_uint32 posCur = getPoint();
		UT_uint32 nposCur = getPoint();
		UT_Bool fontFlag = UT_FALSE;

		if (!bForward)
		{

			if (!_charMotion(bForward,count))
			{
				UT_ASSERT(getPoint() <= posCur);
				amt = posCur - getPoint();
			}

			posCur = getPoint();
  /* 
     Code to deal with change of font boundaries: 
  */
			if((posCur == nposCur) && (posCur > 0)) 
			  {
			    fontFlag = UT_TRUE;
			    posCur--;
			  }
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
			if(fontFlag)
			  {
			    setCharFormat(properties);
			  }
	}

		_generalUpdate();
		free(props_in);

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

void FV_View::_moveInsPtNextPrevLine(UT_Bool bNext)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight, iLineHeight;

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
	UT_uint32 iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(getPoint(), m_bPointEOL, xPoint, yPoint, iPointHeight);
	fl_SectionLayout* pOldSL = pOldBlock->getSectionLayout();
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Container* pOldContainer = pOldLine->getContainer();
	fp_Page* pOldPage = pOldContainer->getPage();
	UT_Bool bDocSection = (pOldSL->getType() == FL_SECTION_DOC);

	fp_Column* pOldLeader = NULL;
	if (bDocSection)
	{
		pOldLeader = ((fp_Column*) (pOldContainer))->getLeader();
	}

	UT_sint32 iPageOffset;
	getPageYOffset(pOldPage, iPageOffset);

	UT_sint32 iLineX = 0;
	UT_sint32 iLineY = 0;

	pOldContainer->getOffsets(pOldLine, iLineX, iLineY);
	yPoint = iLineY;

	iLineHeight = pOldLine->getHeight();

	UT_Bool bNOOP = UT_FALSE;

	if (bNext)
	{
		if (pOldLine != pOldContainer->getLastLine())
		{
			// just move off this line
			yPoint += (iLineHeight + pOldLine->getMarginAfter());
		}
		else if (bDocSection && (((fp_Column*) (pOldSL->getLastContainer()))->getLeader() == pOldLeader))
		{
			// move to next section
			fl_SectionLayout* pSL = pOldSL->getNext();
			if (pSL)
			{
				yPoint = pSL->getFirstContainer()->getY();
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
		if (pOldLine != pOldContainer->getFirstLine())
		{
			// just move off this line
			yPoint -= (pOldLine->getMarginBefore() + 1);
		}
		else if (bDocSection && (pOldSL->getFirstContainer() == pOldLeader))
		{
			// move to prev section
			fl_SectionLayout* pSL = pOldSL->getPrev();
			if (pSL)
			{
				fp_Column* pTmpCol = ((fp_Column*) (pSL->getLastContainer()))->getLeader();
				yPoint = pTmpCol->getY();

				UT_sint32 iMostHeight = 0;
				while (pTmpCol)
				{
					iMostHeight = UT_MAX(iMostHeight, pTmpCol->getHeight());

					pTmpCol = pTmpCol->getFollower();
				}

				yPoint += (iMostHeight - 1);
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
	UT_Bool bBOL = UT_FALSE;
	UT_Bool bEOL = UT_FALSE;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	UT_ASSERT(iNewPoint != iOldPoint);

	UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
	UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

	_setPoint(iNewPoint, bEOL);

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

	//UT_DEBUGMSG(("_ensure: [xp %ld][yp %ld][ph %ld] [w %ld][h %ld]\n",m_xPoint,m_yPoint,m_iPointHeight,m_iWindowWidth,m_iWindowHeight));

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
		cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(m_xPoint) + fl_PAGEVIEW_MARGIN_X/2));
		bRet = UT_TRUE;
	}
	else if (((UT_uint32) (m_xPoint)) >= ((UT_uint32) m_iWindowWidth))
	{
		cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(m_xPoint - m_iWindowWidth + fl_PAGEVIEW_MARGIN_X/2));
		bRet = UT_TRUE;
	}

	return bRet;
}

void FV_View::_moveInsPtNextPrevPage(UT_Bool bNext)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;

	/*
		This function moves the IP to the beginning of the previous or 
		next page (ie not this one).
	*/

	// first, find the page we are on now
	UT_uint32 iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock = _findBlockAtPosition(iOldPoint);
	fp_Run* pOldRun = pOldBlock->findPointCoords(getPoint(), m_bPointEOL, xPoint, yPoint, iPointHeight);
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Container* pOldContainer = pOldLine->getContainer();
	fp_Page* pOldPage = pOldContainer->getPage();

	// try to locate next/prev page
	fp_Page* pPage = (bNext ? pOldPage->getNext() : pOldPage->getPrev());

	// if couldn't move, go to top of this page instead
	if (!pPage) 
		pPage = pOldPage;

	// move to the first pos on this page
	PT_DocPosition iNewPoint = pPage->getFirstLastPos(UT_TRUE);
	_setPoint(iNewPoint, UT_FALSE);

	// explicit vertical scroll to top of page
	UT_sint32 iPageOffset;
	getPageYOffset(pPage, iPageOffset);

	iPageOffset -= fl_PAGEVIEW_PAGE_SEP /2;
	iPageOffset -= m_yScrollOffset;
	
	UT_Bool bVScroll = UT_FALSE;
	if (iPageOffset < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(iPageOffset)));
		bVScroll = UT_TRUE;
	}
	else if (iPageOffset > 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(iPageOffset));
		bVScroll = UT_TRUE;
	}

	// also allow implicit horizontal scroll, if needed
	if (!_ensureThatInsertionPointIsOnScreen() && !bVScroll)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

void FV_View::warpInsPtNextPrevPage(UT_Bool bNext)
{
	if (!isSelectionEmpty())
		_moveToSelectionEnd(bNext);
	else
		_eraseInsertionPoint();

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevPage(bNext);
	notifyListeners(AV_CHG_MOTION);
}

void FV_View::warpInsPtNextPrevLine(UT_Bool bNext)
{
	if (!isSelectionEmpty())
		_moveToSelectionEnd(bNext);
	else
		_eraseInsertionPoint();

	_resetSelection();
	_clearIfAtFmtMark(getPoint());
	_moveInsPtNextPrevLine(bNext);
	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelNextPrevLine(UT_Bool bNext)
{
	if (isSelectionEmpty())
	{
		_setSelectionAnchor();
		_clearIfAtFmtMark(getPoint());
		_moveInsPtNextPrevLine(bNext);
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
		PT_DocPosition iOldPoint = getPoint();
 		_moveInsPtNextPrevLine(bNext);
		PT_DocPosition iNewPoint = getPoint();

		// top/bottom of doc - nowhere to go
		if (iOldPoint == iNewPoint)
			return;

		_extSel(iOldPoint);
		
		if (isSelectionEmpty())
		{
			_resetSelection();
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelHorizontal(UT_Bool bForward, UT_uint32 count)
{
	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_setSelectionAnchor();
		_charMotion(bForward, count);
	}
	else
	{
		PT_DocPosition iOldPoint = getPoint();

		if (_charMotion(bForward, count) == UT_FALSE)
		{
			_setPoint(iOldPoint);
			return;
		}
		
		_extSel(iOldPoint);
	}
	
	_ensureThatInsertionPointIsOnScreen();
		
	/*
	  It IS possible for the selection to be empty, even
	  after extending it.  If the charMotion fails, for example,
	  because we are at the end of a document, then the selection
	  will end up empty once again.
	*/
	if (isSelectionEmpty())
	{
		_resetSelection();
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
	else
	{
		_drawSelection();
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::extSelTo(FV_DocPos dp)
{
	PT_DocPosition iPos = _getDocPos(dp);

	_extSelToPos(iPos);

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		if (isSelectionEmpty())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
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

	PT_DocPosition iOldPoint = pView->getPoint();

	/*
		NOTE: We update the selection here, so that the timer can keep 
		triggering autoscrolls even if the mouse doesn't move.  
	*/
	pView->extSelToXY(pView->m_xLastMouse, pView->m_yLastMouse, UT_FALSE);

	if (pView->getPoint() != iOldPoint)
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
 
			// TODO currently we blindly send these auto scroll events without regard
			// TODO to whether the window can scroll any further in that direction.
			// TODO we could optimize this a bit and check the scroll range before we
			// TODO fire them, but that knowledge is only stored in the frame and we
			// TODO don't have a backpointer to it.
			// UT_DEBUGMSG(("_auto: [xp %ld][yp %ld] [w %ld][h %ld]\n",
			//			 xPos,yPos,pView->m_iWindowWidth,pView->m_iWindowHeight));
 
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


fp_Page* FV_View::_getPageForXY(UT_sint32 xPos, UT_sint32 yPos, UT_sint32& xClick, UT_sint32& yClick) const
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
	UT_Bool bBOL = UT_FALSE;
	UT_Bool bEOL = UT_FALSE;
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
				// timer not needed any more, so stop it
				m_pAutoScrollTimer->stop();
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
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this, m_pG);
				if (m_pAutoScrollTimer)
					m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
			}
			else
			{
				m_pAutoScrollTimer->start();
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

void FV_View::extSelToXYword(UT_sint32 xPos, UT_sint32 yPos, UT_Bool bDrag)
{
	// extend the current selection to
	// include the WORD at the given XY.
	// this should behave just like extSelToXY()
	// but with WORD-level granularity.
	
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	//UT_ASSERT(!isSelectionEmpty());

	if (iNewPoint <= m_iSelectionLeftAnchor) {
	    m_iSelectionAnchor = m_iSelectionRightAnchor;
	}
	else {
	    m_iSelectionAnchor = m_iSelectionLeftAnchor;
	}
	    
	PT_DocPosition iNewPointWord;
	if (iNewPoint > m_iSelectionAnchor)
		iNewPointWord = _getDocPosFromPoint(iNewPoint,FV_DOCPOS_EOW_SELECT,UT_FALSE);
	else
		iNewPointWord = _getDocPosFromPoint(iNewPoint,FV_DOCPOS_BOW,UT_FALSE);

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
				// timer not needed any more, so stop it
				m_pAutoScrollTimer->stop();
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
				m_pAutoScrollTimer = UT_Timer::static_constructor(_autoScroll, this, m_pG);
				if (m_pAutoScrollTimer)
					m_pAutoScrollTimer->set(AUTO_SCROLL_MSECS);
			}
			else
			{
				m_pAutoScrollTimer->start();
			}
			
			// postpone selection until timer fires
			bPostpone = UT_TRUE;
		}
	}
	
	if (!bPostpone)
	{
		_extSelToPos(iNewPointWord);
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

	// timer not needed any more, so stop it
	m_pAutoScrollTimer->stop();
}

// ---------------- start goto ---------------

UT_Bool FV_View::gotoTarget(FV_JumpTarget /* type */, UT_UCSChar * /* data */)
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
		_ensureThatInsertionPointIsOnScreen();
		_drawSelection();
	}

	// TODO do we need to do a notifyListeners(AV_CHG_MOTION) ??
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
			foundAt = _findBlockSearchDumbCase(buffer, find);
		}
		else if (matchCase == UT_FALSE)
		{
			// do the case-insensitive search
			foundAt = _findBlockSearchDumbNoCase(buffer, find);
		}


		if (foundAt != -1)
		{
			_setPoint(block->getPosition(UT_FALSE) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(UT_TRUE, UT_UCS_strlen(find));

			m_doneFind = UT_TRUE;
			
			FREEP(buffer);
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

	// this assert doesn't work, since the startPosition CAN legitimately be zero
	UT_ASSERT(m_startPosition >= 2);	// the beginning of the first block in any document
	
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
		newBlock = (*block)->getNextBlockInDocument();

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
	UT_ASSERT(bufferSegment);
	
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
		UT_Bool bRes = findNext(_m_findNextString, _m_matchCase, NULL);
		if (bRes)
		{
			_drawSelection();
		}

		return bRes;
	}
	
	return UT_FALSE;
}

UT_Bool	FV_View::findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							 UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	UT_ASSERT(find && replace);
	
	UT_Bool bRes = _findReplace(find, replace, matchCase, bDoneEntireDocument);

	updateScreen();
	
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
		_ensureThatInsertionPointIsOnScreen();
	}

	return bRes;
}

UT_Bool	FV_View::_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							  UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	UT_ASSERT(find && replace);

	// if we have done a find, and there is a selection, then replace what's in the
	// selection and move on to next find (batch run, the common case)
	if ((m_doneFind == UT_TRUE) && (!isSelectionEmpty()))
	{
		UT_Bool result = UT_TRUE;

		if (!isSelectionEmpty())
		{
			_deleteSelection();
		}
		else
		{
			_eraseInsertionPoint();
		}

		// if we have a string with length, do an insert, else let it hang
		// from the delete above
		if (*replace)
			result = m_pDoc->insertSpan(getPoint(), replace, UT_UCS_strlen(replace));

		_generalUpdate();

			// if we've wrapped around once, and we're doing work before we've
			// hit the point at which we started, then we adjust the start
			// position so that we stop at the right spot.
		if (m_wrappedEnd && !*bDoneEntireDocument)
			m_startPosition += ((long) UT_UCS_strlen(replace) - (long) UT_UCS_strlen(find));

		UT_ASSERT(m_startPosition >= 2);

		// do not increase the insertion point index, since the insert span will
		// leave us at the correct place.
		
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

void FV_View::insertSymbol(UT_UCSChar c, XML_Char * symfont, XML_Char * currentfont)
  /* Move code here to use _generalUpdate */

{
        if(strstr(symfont,currentfont) == NULL) 
	  {
	      /*  Now set the font */
	 
	    const XML_Char * properties[] =	{ "font-family", NULL, 0};
	    properties[1] = symfont ;
	    setCharFormat(properties);

	    /* Now insert the character */
	    
	    cmdCharInsert(&c,1);
	    
	    /* Now change the font back to its original value */

	    properties[1] = currentfont;
	    setCharFormat(properties);
	    _generalUpdate();
	  }
	else
	  {
	      /* Just insert the character */
 
	    cmdCharInsert(&c,1);
	  }
}

/*
  After most editing commands, it is necessary to call this method,
  _generalUpdate, in order to fix everything.
*/
void FV_View::_generalUpdate(void)
{
	m_pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);

	/*
	  TODO note that we are far too heavy handed with the mask we
	  send here.  I ripped out all the individual calls to notifyListeners
	  which appeared within fl_BlockLayout, and they all now go through
	  here.  For that reason, I made the following mask into the union
	  of all the masks I found.  I assume that this is inefficient, but
	  functionally correct.

	  TODO WRONG! WRONG! WRONG! notifyListener() must be called in
	  TODO WRONG! WRONG! WRONG! fl_BlockLayout in response to a change
	  TODO WRONG! WRONG! WRONG! notification and not here.  this call
	  TODO WRONG! WRONG! WRONG! will only update the current window.
	  TODO WRONG! WRONG! WRONG! having the notification in fl_BlockLayout
	  TODO WRONG! WRONG! WRONG! will get each view on the document.
	*/
	notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK);
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
	
	_generalUpdate();
	
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
		_ensureThatInsertionPointIsOnScreen();
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
UT_sint32 FV_View::_findBlockSearchDumbCase(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_strstr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}

/*
  Pierre Sarrazin <ps@cam.org> supplied the Unicode stricmp comparison function,
  which works for Latin-1 at the moment.
*/

UT_sint32 FV_View::_findBlockSearchDumbNoCase(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_stristr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}


/*
  Any takers?
*/

UT_sint32 FV_View::_findBlockSearchRegexp(const UT_UCSChar * /* haystack */, const UT_UCSChar * /* needle */)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	
	return -1;
}

// ---------------- end find and replace ---------------

void FV_View::_extSel(UT_uint32 iOldPoint)
{
	/*
	  We need to calculate the differences between the old
	  selection and new one.

	  Anything which was selected, and now is not, should
	  be fixed on screen, back to normal.

	  Anything which was NOT selected, and now is, should
	  be fixed on screen, to show it in selected state.

	  Anything which was selected, and is still selected,
	  should NOT be touched.

	  And, obviously, anything which was not selected, and
	  is still not selected, should not be touched.
	*/

	UT_uint32 iNewPoint = getPoint();

	if (iNewPoint == iOldPoint)
	{
		return;
	}
	
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
				_clearBetweenPositions(m_iSelectionAnchor, iOldPoint, UT_TRUE);
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

			_clearBetweenPositions(iNewPoint, iOldPoint, UT_TRUE);
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

			_clearBetweenPositions(iOldPoint, iNewPoint, UT_TRUE);
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

				_clearBetweenPositions(iOldPoint, m_iSelectionAnchor, UT_TRUE);
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
}

void FV_View::_extSelToPos(PT_DocPosition iNewPoint)
{
	PT_DocPosition iOldPoint = getPoint();
	if (iNewPoint == iOldPoint)
		return;

	if (isSelectionEmpty())
	{
		_eraseInsertionPoint();
		_clearIfAtFmtMark(getPoint());
		_setSelectionAnchor();
	}

	_setPoint(iNewPoint);
	_extSel(iOldPoint);
	
	if (isSelectionEmpty())
	{
		_resetSelection();
		_drawInsertionPoint();
	}

	notifyListeners(AV_CHG_MOTION);
}

void FV_View::warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos)
{
	/*
	  Figure out which page we clicked on.
	  Pass the click down to that page.
	*/
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	if (!isSelectionEmpty())
		_clearSelection();
	else
		_eraseInsertionPoint();
	
	PT_DocPosition pos;
	UT_Bool bBOL = UT_FALSE;
	UT_Bool bEOL = UT_FALSE;
	
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);

	if (pos != getPoint())
		_clearIfAtFmtMark(getPoint());
	
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
  and draws each one.
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
        UT_Bool bIsDirty = UT_FALSE;
	fp_Run* pCurRun = pRun1;

	while (!bDone || bIsDirty)
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

		pCurRun->draw(&da);
		
		pCurRun = pCurRun->getNext();
		if (!pCurRun)
		{
			fl_BlockLayout* pNextBlock;
			
			pNextBlock = pBlock->getNextBlockInDocument();
			if (pNextBlock)
			{
				pCurRun = pNextBlock->getFirstRun();
			}
		}
		if( !pCurRun)
		{
		        bIsDirty = UT_FALSE;
		}
		else
		{
		        bIsDirty = pCurRun->isDirty();
		}
	}
}

/*
  This method simply iterates over every run between two doc positions
  and draws each one.
*/
void FV_View::_clearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, UT_Bool bFullLineHeightRect)
{
	if (iPos1 >= iPos2)
	{
		return;
	}
	
	fp_Run* pRun1;
	fp_Run* pRun2;
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

	if (!pRun1 && !pRun2)
	{
		// no formatting info for either block, so just bail
		// this can happen during spell, when we're trying to invalidate
		// a new squiggle before the block has been formatted
		return;
	}

    // HACK: In certain editing cases only one of these is NULL, which 
    //       makes locating runs to clear more difficult.  For now, I'm 
    //       playing it safe and trying to just handle these cases here. 
    //       The real solution may be to just bail if *either* is NULL, 
    //       but I'm not sure.  
    //
    //       If you're interested in investigating this alternative 
    //       approach, play with the following asserts.  

//	UT_ASSERT(pRun1 && pRun2);
	UT_ASSERT(pRun2);

	UT_Bool bDone = UT_FALSE;
	fp_Run* pCurRun = (pRun1 ? pRun1 : pRun2);


	while (!bDone)
	{
		if (pCurRun == pRun2)
		{
			bDone = UT_TRUE;
		}
		
		pCurRun->clearScreen(bFullLineHeightRect);
		
		if (pCurRun->getNext())
		{
			pCurRun = pCurRun->getNext();
		}
		else
		{
			fl_BlockLayout* pNextBlock;
			
			fl_BlockLayout* pBlock = pCurRun->getBlock();
			UT_ASSERT(pBlock);

			pNextBlock = pBlock->getNextBlockInDocument();
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
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;

	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	UT_ASSERT(pBlock);
	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, iPointHeight);

	// NOTE prior call will fail if the block isn't currently formatted,
	// NOTE so we won't be able to figure out more specific geometry

	if (pRun)
	{
		// we now have coords relative to the page containing the ins pt
		fp_Page* pPointPage = pRun->getLine()->getContainer()->getPage();

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
	}

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
	_findPositionCoords(getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_iPointHeight, NULL, NULL);

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

		m_pG->setColor(clr);
		m_pG->xorLine(m_xPoint-1, m_yPoint, m_xPoint-1, m_yPoint + m_iPointHeight);
		m_pG->xorLine(m_xPoint, m_yPoint, m_xPoint, m_yPoint + m_iPointHeight);
	}
}

void FV_View::_eraseInsertionPoint()
{
	if (m_pAutoCursorTimer) 
		m_pAutoCursorTimer->stop();
	
	if (!isSelectionEmpty() || !m_bCursorIsOn)
	{
		return;
	}

	_xorInsertionPoint();
}

void FV_View::_drawInsertionPoint()
{
	if(m_focus==AV_FOCUS_NONE)
		return;
	if (m_bCursorBlink && m_focus==AV_FOCUS_HERE)
	{
		if (m_pAutoCursorTimer == NULL) {
			m_pAutoCursorTimer = UT_Timer::static_constructor(_autoDrawPoint, this, m_pG);
			m_pAutoCursorTimer->set(AUTO_DRAW_POINT);
		}

		m_pAutoCursorTimer->start();
	}

	m_bCursorIsOn = UT_TRUE;

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

void FV_View::_autoDrawPoint(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	FV_View * pView = (FV_View *) pTimer->getInstanceData();
	UT_ASSERT(pView);

	if (pView->m_iWindowHeight <= 0)
	{
		return;
	}
	
	if (!pView->isSelectionEmpty())
	{
		return;
	}
	pView->_xorInsertionPoint();
	pView->m_bCursorIsOn = !pView->m_bCursorIsOn;
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

void FV_View::updateScreen(void)
{
	_draw(0,0,m_iWindowWidth,m_iWindowHeight,UT_TRUE,UT_FALSE);
}

void FV_View::_draw(UT_sint32 x, UT_sint32 y,
				   UT_sint32 width, UT_sint32 height,
				   UT_Bool bDirtyRunsOnly, UT_Bool bClip)
{
	xxx_UT_DEBUGMSG(("FV_View::draw_3 [x %ld][y %ld][w %ld][h %ld][bClip %ld]\n"
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
	{
		lineHeight = HACK_LINE_HEIGHT;
	}
	
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

	if (yoff < 0)
	{
		yoff = 0;
	}

	UT_Bool bRedrawPoint = UT_TRUE;
	
	if (bVertical && (yoff != m_yScrollOffset))
	{
		sendVerticalScrollEvent(yoff);
		bRedrawPoint = UT_FALSE;
	}

	if (xoff < 0)
	{
		xoff = 0;
	}
		
	if (bHorizontal && (xoff != m_xScrollOffset))
	{
		sendHorizontalScrollEvent(xoff);
		bRedrawPoint = UT_FALSE;
	}

	if (bRedrawPoint)
	{
		_drawInsertionPoint();
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
	UT_Bool bBOL = UT_FALSE;
	UT_Bool bEOL = UT_FALSE;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
	UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

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
	m_iSelectionLeftAnchor = iPosLeft;
	m_iSelectionRightAnchor = iPosRight;
	
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

void FV_View::_setPoint(PT_DocPosition pt, UT_Bool bEOL)
{
	UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);

	m_iInsPoint = pt;
	m_bPointEOL = bEOL;

	_checkPendingWord();
}

void FV_View::_checkPendingWord(void)
{
	// deal with pending word, if any
	if (m_pLayout->isPendingWord())
	{
		fl_BlockLayout* pBL = _findBlockAtPosition(m_iInsPoint);
		if (pBL)
		{
			UT_uint32 iOffset = m_iInsPoint - pBL->getPosition();

			if (!m_pLayout->touchesPendingWord(pBL, iOffset, 0))
			{
				// no longer there, so check it
				if (m_pLayout->checkPendingWord())
					updateScreen();
			}
		}
	}
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
		{
			_checkPendingWord();
			_clearIfAtFmtMark(posOld);
			notifyListeners(AV_CHG_MOTION);
		}
		
		return UT_FALSE;
	}

	bRes = m_pDoc->getBounds(UT_TRUE, posEOD);
	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint > (UT_sint32) posEOD)
	{
		m_iInsPoint = posEOD;

		if (m_iInsPoint != posOld)
		{
			_checkPendingWord();
			_clearIfAtFmtMark(posOld);
			notifyListeners(AV_CHG_MOTION);
		}
		
		return UT_FALSE;
	}

	if (m_iInsPoint != posOld)
	{
		_checkPendingWord();
		_clearIfAtFmtMark(posOld);
		notifyListeners(AV_CHG_MOTION);
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
		_clearSelection();
	else
		_eraseInsertionPoint();

	m_pDoc->undoCmd(count);

	_generalUpdate();
	
	notifyListeners(AV_CHG_DIRTY);
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

void FV_View::cmdRedo(UT_uint32 count)
{
	if (!isSelectionEmpty())
		_clearSelection();
	else
		_eraseInsertionPoint();

	m_pDoc->redoCmd(count);

	_generalUpdate();
	
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

UT_Error FV_View::cmdSave(void)
{
  UT_Error tmpVar;
  tmpVar = m_pDoc->save();
  if (!tmpVar)
      notifyListeners(AV_CHG_SAVE);
  return tmpVar;
}


UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft)
{
  UT_Error tmpVar;
  tmpVar = m_pDoc->saveAs(szFilename, ieft);
  if (!tmpVar)
      notifyListeners(AV_CHG_SAVE);
  return tmpVar;
}


void FV_View::cmdCut(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	
	cmdCopy();
	_deleteSelection();

	_generalUpdate();
	
	_fixInsertionPointCoords();
	_drawInsertionPoint();
}

void FV_View::getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr)
{
	PT_DocPosition iPos1, iPos2;
	if (m_iSelectionAnchor < getPoint())
	{
		iPos1 = m_iSelectionAnchor;
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_iSelectionAnchor;
	}

	pdr->set(m_pDoc,iPos1,iPos2);
	return;
}

void FV_View::cmdCopy(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	
	PD_DocumentRange dr;
	getDocumentRangeOfCurrentSelection(&dr);
	m_pApp->copyToClipboard(&dr);
	notifyListeners(AV_CHG_CLIPBOARD);
}

void FV_View::cmdPaste(void)
{
	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.
	
	m_pDoc->beginUserAtomicGlob();
	_doPaste(UT_TRUE);
	m_pDoc->endUserAtomicGlob();
}

void FV_View::cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos)
{
	// this is intended for the X11 middle mouse paste trick.
	//
	// if this view has the selection, we need to remember it
	// before we warp to the given (x,y) -- or else there won't
	// be a selection to paste when get there.  this is sort of
	// back door hack and should probably be re-thought.
	
	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.
	
	m_pDoc->beginUserAtomicGlob();
	if (!isSelectionEmpty())
		m_pApp->cacheCurrentSelection(this);
	warpInsPtToXY(xPos,yPos);
	_doPaste(UT_FALSE);
	m_pApp->cacheCurrentSelection(NULL);
	m_pDoc->endUserAtomicGlob();
}

void FV_View::_doPaste(UT_Bool bUseClipboard)
{
	// internal portion of paste operation.
	
	if (!isSelectionEmpty())
		_deleteSelection();
	else
		_eraseInsertionPoint();

	_clearIfAtFmtMark(getPoint());
	PD_DocumentRange dr(m_pDoc,getPoint(),getPoint());
	m_pApp->pasteFromClipboard(&dr,bUseClipboard);

	_generalUpdate();
	
	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

UT_Bool FV_View::setSectionFormat(const XML_Char * properties[])
{
	UT_Bool bRet;

	_eraseInsertionPoint();
	_clearIfAtFmtMark(getPoint());

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;
	}

	bRet = m_pDoc->changeStruxFmt(PTC_AddFmt,posStart,posEnd,NULL,properties,PTX_Section);

	_generalUpdate();
	
	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bRet;
}

/*****************************************************************/
/*****************************************************************/

void FV_View::getTopRulerInfo(AP_TopRulerInfo * pInfo)
{
	memset(pInfo,0,sizeof(*pInfo));

	if (1)		// TODO support tables
	{
		// we are in a column context

		fl_BlockLayout * pBlock;
		fp_Run * pRun;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;

		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, heightCaret, &pBlock, &pRun);

		fp_Container * pContainer = pRun->getLine()->getContainer();
		fl_SectionLayout * pSection = pContainer->getSectionLayout();
		if (pSection->getType() == FL_SECTION_DOC)
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) pSection;
			
			UT_uint32 nCol=0;
			fp_Column * pNthColumn=pColumn->getLeader();
			while (pNthColumn && (pNthColumn != pColumn))
			{
				nCol++;
				pNthColumn = pNthColumn->getFollower();
			}
			pInfo->m_iCurrentColumn = nCol;
			pInfo->m_iNumColumns = pDSL->getNumColumns();

			pInfo->u.c.m_xaLeftMargin = pDSL->getLeftMargin();
			pInfo->u.c.m_xaRightMargin = pDSL->getRightMargin();
			pInfo->u.c.m_xColumnGap = pDSL->getColumnGap();
			pInfo->u.c.m_xColumnWidth = pColumn->getWidth();
		}
		else
		{
			// TODO fill in the same info as above, with whatever is appropriate
		}
		
		// fill in the details
		
		pInfo->m_mode = AP_TopRulerInfo::TRI_MODE_COLUMNS;
		pInfo->m_xPaperSize = m_pG->convertDimension("8.5in"); // TODO eliminate this constant
		pInfo->m_xPageViewMargin = fl_PAGEVIEW_MARGIN_X;

		pInfo->m_xrPoint = xCaret - pContainer->getX();
		pInfo->m_xrLeftIndent = m_pG->convertDimension(pBlock->getProperty("margin-left"));
		pInfo->m_xrRightIndent = m_pG->convertDimension(pBlock->getProperty("margin-right"));
		pInfo->m_xrFirstLineIndent = m_pG->convertDimension(pBlock->getProperty("text-indent"));

		pInfo->m_pfnEnumTabStops = pBlock->s_EnumTabStops;
		pInfo->m_pVoidEnumTabStopsData = (void *)pBlock;
		pInfo->m_iTabStops = (UT_sint32) pBlock->getTabsCount();
		pInfo->m_iDefaultTabInterval = pBlock->getDefaultTabInterval();
		pInfo->m_pszTabStops = pBlock->getProperty("tabstops");

	}
	else
	{
		// TODO support tables
	}

	return;
}

void FV_View::getLeftRulerInfo(AP_LeftRulerInfo * pInfo)
{
	memset(pInfo,0,sizeof(*pInfo));

	if (1)								// TODO support tables
	{
		// we assume that we are in a column context (rather than a table)

		pInfo->m_mode = AP_LeftRulerInfo::TRI_MODE_COLUMNS;

		fl_BlockLayout * pBlock;
		fp_Run * pRun;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;

		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, heightCaret, &pBlock, &pRun);

		UT_ASSERT(pRun);
		UT_ASSERT(pRun->getLine());

		fp_Container * pContainer = pRun->getLine()->getContainer();

		pInfo->m_yPoint = yCaret - pContainer->getY();

		fl_SectionLayout * pSection = pContainer->getSectionLayout();
		if (pSection->getType() == FL_SECTION_DOC)
		{
			fp_Column* pColumn = (fp_Column*) pContainer;
			fl_DocSectionLayout* pDSL = (fl_DocSectionLayout*) pSection;
			fp_Page * pPage = pColumn->getPage();

			UT_sint32 yoff = 0;
			getPageYOffset(pPage, yoff);
			pInfo->m_yPageStart = (UT_uint32)yoff;
			pInfo->m_yPageSize = pPage->getHeight();

			pInfo->m_yTopMargin = pDSL->getTopMargin();
			pInfo->m_yBottomMargin = pDSL->getBottomMargin();
		}
		else
		{
			// TODO fill in the same info as above, with whatever is appropriate
		}
	}
	else
	{
		// TODO support tables
	}

	return;
}

/*****************************************************************/
UT_Error FV_View::cmdInsertField(const char* szName)
{
	UT_Bool bResult;
	const XML_Char*	attributes[] = {
		"type", szName,
		NULL, NULL
	};

	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL);
		m_pDoc->endUserAtomicGlob();
	}
	else
	{
		_eraseInsertionPoint();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL);
	}

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bResult;
}

UT_Error FV_View::_insertGraphic(FG_Graphic* pFG, const char* szName)
{
	UT_ASSERT(pFG);
	UT_ASSERT(szName);

	double fDPI = m_pG->getResolution() * 100 / m_pG->getZoomPercentage(); 
	return pFG->insertIntoDocument(m_pDoc, fDPI, getPoint(), szName);
}

UT_Error FV_View::cmdInsertGraphic(FG_Graphic* pFG, const char* pszName)
{
	UT_Bool bDidGlob = UT_FALSE;

	if (!isSelectionEmpty())
	{
		bDidGlob = UT_TRUE;
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	/*
	  First, find a unique name for the data item.
	*/
	char szName[GR_IMAGE_MAX_NAME_LEN + 10 + 1];
	UT_uint32 ndx = 0;
	for (;;)
	{
		sprintf(szName, "%s_%d", pszName, ndx);
		if (!m_pDoc->getDataItemDataByName(szName, NULL, NULL, NULL))
		{
			break;
		}
		ndx++;
	}

	UT_Error errorCode = _insertGraphic(pFG, szName);

	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return errorCode;
}

UT_Bool FV_View::isPosSelected(PT_DocPosition pos) const
{
	if (!isSelectionEmpty())
	{
		PT_DocPosition posStart = getPoint();
		PT_DocPosition posEnd = posStart;

		if (m_iSelectionAnchor < posStart)
			posStart = m_iSelectionAnchor;
		else
			posEnd = m_iSelectionAnchor;

		return ((pos >= posStart) && (pos <= posEnd));
	}

	return UT_FALSE;
}

UT_Bool FV_View::isXYSelected(UT_sint32 xPos, UT_sint32 yPos) const
{
	if (isSelectionEmpty())
		return UT_FALSE;

	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	if (!pPage)
		return UT_FALSE;

	if (   (yClick < 0)
		|| (xClick < 0)
		|| (xClick > pPage->getWidth()) )
	{
		return UT_FALSE;
	}

	PT_DocPosition pos;
	UT_Bool bBOL, bEOL;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);

	return isPosSelected(pos);
}

EV_EditMouseContext FV_View::getMouseContext(UT_sint32 xPos, UT_sint32 yPos)
{
	UT_sint32 xClick, yClick;
	PT_DocPosition pos;
	UT_Bool bBOL = UT_FALSE;
	UT_Bool bEOL = UT_FALSE;
	UT_sint32 xPoint, yPoint, iPointHeight;

	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);
	if (!pPage)
		return EV_EMC_UNKNOWN;

	if (   (yClick < 0)
		|| (xClick < 0)
		|| (xClick > pPage->getWidth()) )
	{
		return EV_EMC_UNKNOWN;
	}

	if (isLeftMargin(xPos,yPos))
	{
		return EV_EMC_LEFTOFTEXT;
	}

	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	if (!pBlock)
		return EV_EMC_UNKNOWN;
	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, iPointHeight);
	if (!pRun)
		return EV_EMC_UNKNOWN;

	switch (pRun->getType())
	{
	case FPRUN_TEXT:
		if (!isPosSelected(pos))
			if (pBlock->getSquiggle(pos - pBlock->getPosition()))
				return EV_EMC_MISSPELLEDTEXT;

		return EV_EMC_TEXT;
		
	case FPRUN_IMAGE:
		// TODO see if the image is selected and current x,y
		// TODO is over the image border or the border handles.
		// TODO if so, return EV_EMC_IMAGESIZE
		return EV_EMC_IMAGE;
		
	case FPRUN_TAB:
	case FPRUN_FORCEDLINEBREAK:
	case FPRUN_FORCEDCOLUMNBREAK:
	case FPRUN_FORCEDPAGEBREAK:
	case FPRUN_FMTMARK:
		return EV_EMC_TEXT;
		
	case FPRUN_FIELD:
		return EV_EMC_FIELD;
		
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return EV_EMC_UNKNOWN;
	}

	/*NOTREACHED*/
	UT_ASSERT(0);
	return EV_EMC_UNKNOWN;
}

EV_EditMouseContext FV_View::getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos)
{
	// compute an EV_EMC_ context for the position
	// of the current insertion point and return
	// the window coordinates of the insertion point.
	// this is to allow a keyboard binding to raise
	// a context menu.

	EV_EditMouseContext emc = EV_EMC_TEXT;

	// TODO compute the correct context based upon the
	// TODO current insertion point and/or the current
	// TODO selection region.
	
	if (pxPos)
		*pxPos = m_xPoint;
	if (pyPos)
		*pyPos = m_yPoint + m_iPointHeight;
	return emc;
}

fp_Page* FV_View::getCurrentPage(void) const
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_uint32 pos = getPoint();
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	UT_ASSERT(pBlock);
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, iPointHeight);

	// NOTE prior call will fail if the block isn't currently formatted,
	// NOTE so we won't be able to figure out more specific geometry

	if (pRun)
	{
		// we now have coords relative to the page containing the ins pt
		fp_Page* pPointPage = pRun->getLine()->getContainer()->getPage();

		return pPointPage;
	}

	return NULL;
}

UT_uint32 FV_View::getCurrentPageNumForStatusBar(void) const
{
	fp_Page* pCurrentPage = getCurrentPage();
	UT_uint32 ndx = 1;

	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		if (pPage == pCurrentPage)
		{
			return ndx;
		}

		ndx++;
		pPage = pPage->getNext();
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return 0;
}

void FV_View::_clearIfAtFmtMark(PT_DocPosition dpos)
{
	m_pDoc->clearIfAtFmtMark(dpos);
	_generalUpdate();
}

// ndx is one-based, not zero-based
UT_UCSChar * FV_View::getContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab the suggestion
	return _lookupSuggestion(pBL, pPOB, ndx);
}

// NB: returns a UCS string that the caller needs to FREEP
UT_UCSChar * FV_View::_lookupSuggestion(fl_BlockLayout* pBL, fl_PartOfBlock* pPOB, UT_uint32 ndx)
{
	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	UT_Bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pWord = pgb.getPointer(pPOB->iOffset);
	UT_UCSChar * szSuggest = NULL;

	// lookup suggestions
	sp_suggestions sg;
	memset(&sg, 0, sizeof(sg));

	SpellCheckSuggestNWord16(pWord, pPOB->iLength,&sg);

	// we currently return all requested suggestions
	// TODO: prune lower-weighted ones??
	if ((sg.count) && 
		((int) ndx <= sg.count)) 
	{
		UT_UCS_cloneString(&szSuggest, (UT_UCSChar *) sg.word[ndx-1]);
	}

	// clean up
	for (int i = 0; i < sg.count; i++)
		FREEP(sg.word[i]);
	FREEP(sg.word);
	FREEP(sg.score);

	return szSuggest;
}

void FV_View::cmdContextSuggest(UT_uint32 ndx)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab the suggestion
	UT_UCSChar * replace = _lookupSuggestion(pBL, pPOB, ndx);

	// make the change
	UT_ASSERT(isSelectionEmpty());

	moveInsPtTo((PT_DocPosition) (pBL->getPosition() + pPOB->iOffset));
	extSelHorizontal(UT_TRUE, pPOB->iLength);
	cmdCharInsert(replace, UT_UCS_strlen(replace));

	FREEP(replace);
}

void FV_View::cmdContextIgnoreAll(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	UT_Bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = pgb.getPointer(pPOB->iOffset);

	// make the change
	if (m_pDoc->appendIgnore(pBuf, pPOB->iLength))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				m_pLayout->queueBlockForSpell(b, UT_FALSE);
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
}

void FV_View::cmdContextAdd(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggle(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	UT_Bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = pgb.getPointer(pPOB->iOffset);

	// make the change
	if (m_pApp->addWordToDict(pBuf, pPOB->iLength))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = pSL->getFirstBlock();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				m_pLayout->queueBlockForSpell(b, UT_FALSE);
				b = b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
		}
	}
}


/*static*/ void FV_View::_prefsListener( XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_AlphaHashTable * /*phChanges*/, void *data )
{
	FV_View *pView = (FV_View *)data;
	UT_Bool b;
	UT_ASSERT(data && pPrefs);
	if ( pPrefs->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &b) && b != pView->m_bCursorBlink )
	{
		UT_DEBUGMSG(("FV_View::_prefsListener m_bCursorBlink=%s m_bCursorIsOn=%s\n",
					 pView->m_bCursorBlink ? "TRUE" : "FALSE",
					 pView->m_bCursorIsOn ? "TRUE" : "FALSE"));

		pView->m_bCursorBlink = b;

		// if currently blinking, turn it off
		if ( pView->m_bCursorBlink == UT_FALSE && pView->m_pAutoCursorTimer )
			pView->m_pAutoCursorTimer->stop();                                           

		// this is an attempt for force the cursors to draw, don't know if it actually helps
		if ( !pView->m_bCursorBlink && pView->m_bCursorIsOn )
			pView->_drawInsertionPoint();

		pView->_updateInsertionPoint();
	}
}

/******************************************************
 *****************************************************/
UT_Bool s_notChar(UT_UCSChar c)
{
	UT_Bool res = UT_FALSE;

	switch (c)
	{
	case UCS_TAB:
	case UCS_LF:
	case UCS_VTAB:
	case UCS_FF:
	case UCS_CR:
		{
			res = UT_TRUE;
			break;
		}
	default:
		break;
	}
	return res;
}

FV_DocCount FV_View::countWords(void)
{
	FV_DocCount wCount;
	memset(&wCount,0,sizeof(wCount));

	UT_Bool isPara = UT_FALSE;

	fl_SectionLayout * pSL = m_pLayout->getFirstSection();
	while (pSL)
	{
		fl_BlockLayout * pBL = pSL->getFirstBlock();
		while (pBL)
		{
			UT_GrowBuf gb(1024);
			pBL->getBlockBuf(&gb);
			const UT_UCSChar * pSpan = gb.getPointer(0);
			UT_uint32 len = gb.getLength();
			
			// count words in pSpan[0..len]
			UT_uint32 i;
			UT_Bool newWord = UT_FALSE;
			UT_Bool delim = UT_TRUE;
			for (i = 0; i < len; i++)
			{
				if (!s_notChar(pSpan[i]))
				{
					wCount.ch_sp++;
					isPara = UT_TRUE;

					switch (pSpan[i])
					{
					case UCS_SPACE:
					case UCS_NBSP:
					case UCS_EN_SPACE:
					case UCS_EM_SPACE:
						break;

					default:
						wCount.ch_no++;
					}
				}
				newWord = (delim && !UT_isWordDelimiter(pSpan[i]));
				
				delim = UT_isWordDelimiter(pSpan[i]);
				
				if (newWord)
					wCount.word++;
			
			}

		
			// count lines

			fp_Line * pLine = pBL->getFirstLine();
			while (pLine)
		       	{
				wCount.line++;
				pLine = pLine->getNext();
			}
			
			if (isPara)
		       	{
				wCount.para++;
				isPara = UT_FALSE;
			}
	
			pBL = pBL->getNext();
		}
		pSL = pSL->getNext();

	}
	// count pages
	wCount.page = m_pLayout->countPages();
	
	return (wCount);
}

