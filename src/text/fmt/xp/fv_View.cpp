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
#include "fl_AutoNum.h"
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
#include "fd_Field.h"
#include "sp_spell.h"

#include "xap_EncodingManager.h"

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
	m_iInsPoint(0),
	m_xPoint(0),
	m_yPoint(0),
	m_iPointHeight(0),
	m_oldxPoint(0),
	m_oldyPoint(0),
	m_oldiPointHeight(0),
	m_xPointSticky(0),
	m_bPointVisible(UT_FALSE),
	m_bPointEOL(UT_FALSE),
	m_bDontChangeInsPoint(UT_FALSE),
	m_pLayout(pLayout),
	m_pDoc(pLayout->getDocument()),
	m_pG(m_pLayout->getGraphics()),
	m_pParentData(pParentData),
	m_iSelectionAnchor(0),
	m_iSelectionLeftAnchor(0),
	m_iSelectionRightAnchor(0),
	m_bSelection(UT_FALSE),
	m_pAutoScrollTimer(0),
	m_xLastMouse(0),
	m_yLastMouse(0),
	m_pAutoCursorTimer(0),
	m_bCursorIsOn(UT_FALSE),
	m_bEraseSaysStopBlinking(UT_FALSE),
	m_wrappedEnd(UT_FALSE),
	m_startPosition(0),
	m_doneFind(UT_FALSE),
	_m_matchCase(UT_FALSE),
	_m_findNextString(0),
	m_bShowPara(UT_FALSE)
{
//	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	// initialize prefs cache
	pApp->getPrefsValueBool(AP_PREF_KEY_CursorBlink, &m_bCursorBlink);

	// initialize prefs listener
	pApp->getPrefs()->addListener( _prefsListener, this );

	// initialize change cache
	m_chg.bUndo = UT_FALSE;
	m_chg.bRedo = UT_FALSE;
	m_chg.bDirty = UT_FALSE;
	m_chg.bSelection = UT_FALSE;
	m_chg.iColumn = 0;                       // current column number
	m_chg.propsChar = NULL;
	m_chg.propsBlock = NULL;
	m_chg.propsSection = NULL;

	pLayout->setView(this);

	pLayout->formatAll();
	//Test_Dump();
	moveInsPtTo(FV_DOCPOS_BOD);
	m_iSelectionAnchor = getPoint();
	_resetSelection();
	_clearOldPoint();
	_fixInsertionPointCoords();
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
		m_pApp->rememberFocussedFrame( m_pParentData);
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
	case AV_FOCUS_MODELESS:
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
			_saveCurrentPoint();
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
		
		fp_Run * pRun = NULL;
		UT_sint32 xCaret, yCaret;
		UT_uint32 heightCaret;

		_findPositionCoords(getPoint(), m_bPointEOL, xCaret, yCaret, heightCaret, NULL, &pRun);
		//
		// Handle Headers/Footers This is a kludge for now
		//
		fp_Container * pContainer = NULL;
		fl_BlockLayout * pBlock = pRun->getBlock();
		if(pBlock->getSectionLayout()->getType() ==  FL_SECTION_HDRFTR)
		{
			pContainer = pBlock->getSectionLayout()->getFirstContainer();
		}
		else
		{
			pContainer = pRun->getLine()->getContainer();
		}

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
	_fixInsertionPointCoords();
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
	//		after clearing the selection.

	UT_ASSERT(!isSelectionEmpty());
	
	PT_DocPosition curPos = getPoint();
	_fixInsertionPointCoords();
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
	_fixInsertionPointCoords();
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
	_fixInsertionPointCoords();
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

void FV_View::_deleteSelection(PP_AttrProp *p_AttrProp_Before)
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
		m_pDoc->deleteSpan(iPoint, iSelAnchor, p_AttrProp_Before);
	}
	else
	{
		m_pDoc->deleteSpan(iSelAnchor, iPoint, p_AttrProp_Before);
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

			UT_Bool bInWord = !UT_isWordDelimiter(pSpan[bKeepLooking ? offset-1 : offset], UCS_UNKPUNK);

			for (offset--; offset > 0; offset--)
			{
				if (UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK))
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

			UT_Bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK);
			
			// Needed so ctrl-right arrow will work
			// This is the code that was causing bug 10
			// There is still some weird behavior that should be investigated

			for (; offset < pgb.getLength(); offset++)
			{
				UT_UCSChar followChar;
				followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
				if (!UT_isWordDelimiter(pSpan[offset], followChar))
				break;
			}
			
			for (; offset < pgb.getLength(); offset++)
			{
				UT_UCSChar followChar;
				followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
				if (!UT_isWordDelimiter(pSpan[offset], followChar))
				{
					if (bBetween)
					{
						break;
					}
				}
				else if (pSpan[offset] != ' ')
				{
					break;
				}
				else
				{
					bBetween = UT_TRUE;
				}
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

			UT_Bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK);
			
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
				UT_UCSChar followChar;
				followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
				if (UT_isWordDelimiter(pSpan[offset], followChar))
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

	_setPoint(dp, /* (dp == FV_DOCPOS_EOL) */ UT_FALSE);	// is this bool correct?
	/*TF CHANGE: Why do we automatically scroll?  We should only scroll the window
                 if the point to be displayed is not already on the screen.  If it 
				 is already on the screen then we should just leave it in place and
                 not do any scrolling.  Instead of the code below, we use the code which
  				 is already in the _ensureThatInsertionPointIsOnScreen() function.
	_fixInsertionPointCoords();
	cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32) (m_yPoint + m_iPointHeight/2 - m_iWindowHeight/2));
	cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32) (m_xPoint - m_iWindowWidth/2));
	notifyListeners(AV_CHG_MOTION);
	*/
	_ensureThatInsertionPointIsOnScreen();
	
}


void FV_View::cmdCharMotion(UT_Bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
		// Note: _moveToSelectionEnd() clears the selection
		//		but does not redraw the insertion point.
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
				_eraseInsertionPoint();
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

UT_uint32 FV_View::getCurrentPageNumber(void)
{ 
	UT_sint32 iPageNum = 0;
	PT_DocPosition pos = getPoint();
	fl_BlockLayout * pBlock = _findBlockAtPosition(pos);
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, iPointHeight);
	fp_Line * pLine = pRun->getLine();
	if (pLine && pLine->getContainer() && pLine->getContainer()->getPage())
	{
		fp_Page* pPage = pLine->getContainer()->getPage();
		FL_DocLayout* pDL = pPage->getDocLayout();

		UT_uint32 iNumPages = pDL->countPages();
		for (UT_uint32 i=0; i<iNumPages; i++)
		{
			fp_Page* pPg = pDL->getNthPage(i);

			if (pPg == pPage)
			{
				iPageNum = i + 1;
				break;
			}
		}
	}
	else
	{
		iPageNum = 0;
	}
	return iPageNum;
}

UT_Bool FV_View::cmdCharInsert(UT_UCSChar * text, UT_uint32 count, UT_Bool bForce)
{
	UT_Bool bResult;

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		PP_AttrProp AttrProp_Before;
		_deleteSelection(&AttrProp_Before);
		bResult = m_pDoc->insertSpan(getPoint(), text, count, &AttrProp_Before);
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
		UT_Bool doInsert = UT_TRUE;
		if(text[0] == UCS_TAB && count == 1)
		{
			//
			// Were inserting a TAB. Handle special case of TAB
			// right after a list-label combo
			//
			if((isTabListBehindPoint() == UT_TRUE || isTabListAheadPoint() == UT_TRUE) && getCurrentBlock()->isFirstInList() == UT_FALSE)
			{
				//
				// OK now start a sublist of the same type as the
				// current list if the list type is of numbered type
				fl_BlockLayout * pBlock = getCurrentBlock();
				List_Type curType = pBlock->getListType();
				if(IS_NUMBERED_LIST_TYPE(curType) == UT_TRUE)
				{
					UT_uint32 curlevel = pBlock->getLevel();
					UT_uint32 currID = pBlock->getAutoNum()->getID();
					curlevel++;
					fl_AutoNum * pAuto = pBlock->getAutoNum();
					const XML_Char * pszAlign = pBlock->getProperty((XML_Char*)"margin-left",UT_TRUE);
					const XML_Char * pszIndent = pBlock->getProperty((XML_Char*)"text-indent",UT_TRUE);
					float fAlign = (float)atof(pszAlign);
					float fIndent = (float)atof(pszIndent);
					fAlign += (float) LIST_DEFAULT_INDENT;
					pBlock->StartList(curType,pAuto->getStartValue32(),pAuto->getDelim(),pAuto->getDecimal(),NULL,fAlign,fIndent, currID,curlevel);
					doInsert = UT_FALSE;
				}
			}
		}
		if (doInsert == UT_TRUE)
		{
			bResult = m_pDoc->insertSpan(getPoint(), text, count);
		}

		if (bOverwrite)
		{
			m_pDoc->endUserAtomicGlob();
		}
	}

	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();


	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bResult;
}


void FV_View::insertSectionBreak(BreakSectionType type)
{
	// if Type = 0 "continuous" section break
	// if Type = 1 "next page" section break
	// if Type = 2 "even page" section break
	// if Type = 3 "odd page" section break

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	UT_UCSChar c = UCS_FF;
	UT_uint32 iPageNum = 0;
	switch(type)
	{
	case BreakSectionContinuous:
		_insertSectionBreak();
		break;
	case BreakSectionNextPage:
		m_pDoc->beginUserAtomicGlob();
		cmdCharInsert(&c,1);
		_insertSectionBreak();
		m_pDoc->endUserAtomicGlob();
		break;
	case BreakSectionEvenPage:
		m_pDoc->beginUserAtomicGlob();
		cmdCharInsert(&c,1);
		iPageNum = getCurrentPageNumber();
		if( (iPageNum & 1) == 1)
		{
			cmdCharInsert(&c,1);
			_insertSectionBreak();
		}
		else
		{
			_insertSectionBreak();
		}
		m_pDoc->endUserAtomicGlob();
		break;
	case BreakSectionOddPage:
		m_pDoc->beginUserAtomicGlob();
		cmdCharInsert(&c,1);
		iPageNum = getCurrentPageNumber();
		if( (iPageNum & 1) == 0)
		{
			cmdCharInsert(&c,1);
			_insertSectionBreak();
		}
		else
		{
			_insertSectionBreak();
		}
		m_pDoc->endUserAtomicGlob();
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

}

void FV_View::insertSectionBreak(void)
{
	m_pDoc->beginUserAtomicGlob();

	_insertSectionBreak();
	m_pDoc->endUserAtomicGlob();
}


void FV_View::_insertSectionBreak(void)
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
	// before inserting a section break, we insert a block break
	UT_uint32 iPoint = getPoint();
	
	m_pDoc->insertStrux(iPoint, PTX_Block);
	m_pDoc->insertStrux(iPoint, PTX_Section);

	_generalUpdate();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}
}

UT_Bool FV_View::isCurrentListBlockEmpty(void)
{
	// 
	// If the current block is a list and is otherwise empty return true
	//
	fl_BlockLayout * pBlock = getCurrentBlock(); 
	fl_BlockLayout * nBlock = pBlock->getNext();
	UT_Bool bEmpty = UT_TRUE;
	if(pBlock->isListItem() == UT_FALSE || (nBlock!= NULL && nBlock->isListItem()==UT_TRUE))
	{
		return UT_FALSE;
	}
	
		//
		// Now look to see if the current block is otherwise empty
		//
	fp_Run * pRun = pBlock->getFirstRun();
	UT_uint32 ifield =0;
	while((bEmpty == UT_TRUE) && (pRun != NULL))
	{
		FP_RUN_TYPE runtype = (FP_RUN_TYPE) pRun->getType();	
		if((runtype == FPRUN_TAB) || 
			( runtype == FPRUN_FIELD)  ||
			(runtype == FPRUN_FMTMARK))
		{
			if(runtype == FPRUN_FIELD)
			{
				ifield++;
				if(ifield > 1) 
				{
					bEmpty = UT_FALSE;
					break;
				}
			}
			pRun = pRun->getNext();
		}
		else
		{
			bEmpty = UT_FALSE;
		}
	}
	return bEmpty;
}

UT_Bool FV_View::isPointBeforeListLabel(void)
{
	//
	// If the current point is in a list block and the point is before the list label
	// return true
	//
	fl_BlockLayout * pBlock = getCurrentBlock(); 
	UT_Bool bBefore = UT_TRUE;
	if(pBlock->isListItem() == UT_FALSE)
	{
		return UT_FALSE;
	}
	
	//
	// Now look to see if the point is before the list label
	//
	PT_DocPosition pos = getPoint();
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	fp_Run* pRun = pBlock->findPointCoords(pos, m_bPointEOL, xPoint, yPoint, iPointHeight);
	pRun = pRun->getPrev();
	while(pRun != NULL && bBefore == UT_TRUE)
	{
		if(pRun->getType()== FPRUN_FIELD)
		{
			fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
			if (pFRun->getFieldType() == FPFIELD_list_label)
			{
				bBefore = UT_FALSE;
			}
		}
		pRun = pRun->getPrev();
	}
	return bBefore;
}

void FV_View::processSelectedBlocks(List_Type listType)
{
	//
	// Update Lists in the selected region
	//

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	UT_Vector vBlock;
	getListBlocksInSelection( &vBlock);
	UT_uint32 i;
	m_pDoc->disableListUpdates();

	m_pDoc->beginUserAtomicGlob();

	for(i=0; i< vBlock.getItemCount(); i++)
	{
		fl_BlockLayout * pBlock =  (fl_BlockLayout *) vBlock.getNthItem(i);
		PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
		if(pBlock->isListItem() == UT_TRUE)
		{
			m_pDoc->StopList(sdh);
		}
		else
		{
			fl_BlockLayout * pPrev = pBlock->getPrev();
			if(pBlock->isListItem()== NULL && pPrev != NULL && pPrev->isListItem()== UT_TRUE)
			{
				pBlock->resumeList(pPrev);
			}
			else if(pBlock->isListItem()== NULL)
			{
				XML_Char* cType = pBlock->getListStyleString(listType);
				pBlock->StartList(cType);
			} 
		}
	}
	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

}


void FV_View::getListBlocksInSelection( UT_Vector * vBlock)
{
	PT_DocPosition startpos = getPoint();
	PT_DocPosition endpos = startpos;
	if(isSelectionEmpty())
	{
		vBlock->addItem(getCurrentBlock());
		return;
	}
	if (m_iSelectionAnchor > startpos)
	{
		endpos = m_iSelectionAnchor;
	}
	else
	{
		startpos =  m_iSelectionAnchor;
	}
	fl_BlockLayout * pBlock = _findBlockAtPosition(startpos);
	while( pBlock != NULL && pBlock->getPosition() <= endpos)
	{
		vBlock->addItem(pBlock);
		pBlock = pBlock->getNext();
	}
	return;
}

void FV_View::insertParagraphBreak(void)
{
	UT_Bool bDidGlob = UT_FALSE;
	UT_Bool bBefore = UT_FALSE;
	m_pDoc->beginUserAtomicGlob();

	// Prevent access to Piecetable for things like spellchecks until
	// paragraphs have stablized
	//
	m_pDoc->notifyPieceTableChangeStart();

	if (!isSelectionEmpty())
	{
		bDidGlob = UT_TRUE;
		//	m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	//
	// But first check to see if we're in a list and the current block is
	// otherwise blank.
	//
	m_pDoc->disableListUpdates();
	fl_BlockLayout * pBlock = getCurrentBlock();
	PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
	if(isCurrentListBlockEmpty() == UT_TRUE)
	{
		m_pDoc->StopList(sdh);
	}
	else if(isPointBeforeListLabel() == UT_TRUE)
	{
	//
	// Now deal with the case of entering a line before a list label
	// We flag were entering a new line and delete the current list label. After the we
	// insert the line break (which automatically write a new list label) we stop the list
	// in preceding block.
	//
		bBefore = UT_TRUE;
		pBlock->deleteListLabel();
	}

	m_pDoc->insertStrux(getPoint(), PTX_Block);
//	_generalUpdate();
//	sdh = getCurrentBlock()->getStruxDocHandle();
//	getCurrentBlock()->format();
//	m_pDoc->listUpdate(sdh);
	if(bBefore == UT_TRUE)
	{
		fl_BlockLayout * pPrev = getCurrentBlock()->getPrev();
		sdh = pPrev->getStruxDocHandle();
		m_pDoc->StopList(sdh);
		_setPoint(getCurrentBlock()->getPosition());
	}
	m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	if (!_ensureThatInsertionPointIsOnScreen())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

	m_pLayout->considerPendingSmartQuoteCandidate();
	_checkPendingWordForSpell();
}


void FV_View::insertParagraphBreaknoListUpdate(void)
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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

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
	m_pDoc->getStyle((char*)style, &pStyle);
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


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();


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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

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

	// Signal piceTable is stable again
	m_pDoc->notifyPieceTableChangeEnd();

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

	v.addItem(new _fmtPair((XML_Char*)"font-family",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"font-size",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"font-weight",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"font-style",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"text-decoration",pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"text-position",	pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"color",			pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"bgcolor",		pSpanAP,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));

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

UT_Bool FV_View::setListIndents(double indentChange, double page_size)
{
	//
	// indentChange is the increment to the current alignment.
	//
	UT_Vector v;
	XML_Char pszAlign[20];
	UT_Bool bRet;
	UT_Dimension dim;
	double fAlign;
	fl_BlockLayout * pBlock;
	UT_uint32 i;
	fl_AutoNum * pAuto = getCurrentBlock()->getAutoNum();
	UT_ASSERT(pAuto);
	PL_StruxDocHandle pFirstSdh = pAuto->getFirstItem();
	PL_StruxDocHandle pLastSdh = pAuto->getNthBlock(pAuto->getNumLabels()-1);
	fl_SectionLayout * pSl = getCurrentBlock()->getSectionLayout();
	pBlock = pSl->getFirstBlock();
	UT_Bool foundLast = UT_FALSE;
	UT_Bool foundFirst = UT_FALSE;

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();
	m_pDoc->beginUserAtomicGlob();

	//
	// Now collect all all the blocks between the first and last list elements
	// in a vector
	//
	while (pBlock != NULL && foundLast == UT_FALSE)
	{
		if(pBlock->getStruxDocHandle() == pFirstSdh)
		{
			foundFirst = UT_TRUE;
		}
		if(foundFirst == UT_TRUE)
			v.addItem(pBlock);
		if(pBlock->getStruxDocHandle() == pLastSdh)
			foundLast = UT_TRUE;
		pBlock = pBlock->getNext();
	}
	//
	// OK now change the alignements of the blocks.
	//
	const XML_Char * props[] = {"margin-left","0.0in",NULL,NULL};
	for(i = 0; i<v.getItemCount();i++)
	{
		pBlock = (fl_BlockLayout *)v.getNthItem(i);
		UT_XML_strncpy((XML_Char *) pszAlign,20,pBlock->getProperty("margin-left"));
		dim = UT_determineDimension( (char *) pszAlign);
		fAlign = UT_convertToInches((char *) pszAlign);
		if(fAlign + indentChange < 0.0)
		{
			fAlign = 0.0;
		}
		else if( fAlign + indentChange > page_size)
		{
			fAlign = page_size;
		}
		else
		{
			fAlign = fAlign + indentChange;
		}
		char * pszNewAlign = UT_strdup (UT_convertInchesToDimensionString (dim, fAlign));
		PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();		
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
		props[1] = (XML_Char *) pszNewAlign;
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, NULL, props, PTX_Block); 
		FREEP(pszNewAlign);	
		//
		// Might be able to move thisoutside the loop
		_generalUpdate();
	}

	m_pDoc->endUserAtomicGlob();
	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	return bRet;
}

UT_Bool FV_View::setBlockFormat(const XML_Char * properties[])
{
	UT_Bool bRet;


	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();


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

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	if (isSelectionEmpty())
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bRet;
}

UT_Bool FV_View::cmdStartList(const XML_Char * style)
{
	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	pBlock->StartList( style);
	m_pDoc->endUserAtomicGlob();

	return UT_TRUE;
}

void FV_View::changeListStyle(	fl_AutoNum* pAuto,
									List_Type lType,
									UT_uint32 startv,
									const XML_Char* pszDelim,
									const XML_Char* pszDecimal,
									const XML_Char* pszFont,
									float Align,
									float Indent)
{
	UT_Bool bRet;
	UT_uint32 i=0;
	XML_Char pszStart[80],pszAlign[20],pszIndent[20];
	UT_Vector va,vp,vb;
	PL_StruxDocHandle sdh = pAuto->getNthBlock(i);

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	m_pDoc->disableListUpdates();

	if(lType == NOT_A_LIST)
	{
		// Stop lists in all elements
		i = 0;
		sdh = pAuto->getNthBlock(i);
		while(sdh != NULL)
		{
			vb.addItem((void *) sdh);
			i++;
			sdh = pAuto->getNthBlock(i);
		}
		for(i=0; i< vb.getItemCount(); ++i)
		{
			PL_StruxDocHandle sdh = (PL_StruxDocHandle) vb.getNthItem(i);
			m_pDoc->listUpdate(sdh);
			m_pDoc->StopList(sdh);
		}

		_generalUpdate();
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		// Signal PieceTable Changes have finished
		m_pDoc->notifyPieceTableChangeEnd();

		return;
	}

	XML_Char * style = getCurrentBlock()->getListStyleString(lType);
	_eraseInsertionPoint();
	va.addItem( (void *) "style");	va.addItem( (void *) style);

	pAuto->setListType(lType);
	sprintf(pszStart, "%i" , startv);
	UT_XML_strncpy(	pszAlign,
			sizeof(pszAlign),
			UT_convertInchesToDimensionString(DIM_IN, Align, 0));

	UT_XML_strncpy(	pszIndent,
			sizeof(pszIndent),
			UT_convertInchesToDimensionString(DIM_IN, Indent, 0));

	vp.addItem( (void *) "start-value");	vp.addItem( (void *) pszStart);
	vp.addItem( (void *) "margin-left");	vp.addItem( (void *) pszAlign);
	vp.addItem( (void *) "text-indent");	vp.addItem( (void *) pszIndent);
	pAuto->setStartValue(startv);
	if(pszDelim != NULL)
	{
		vp.addItem( (void *) "list-delim"); vp.addItem( (void *) pszDelim);
		pAuto->setDelim(pszDelim);
	}
	if(pszDecimal != NULL)
	{
		vp.addItem( (void *) "list-decimal"); vp.addItem( (void *) pszDecimal);
		pAuto->setDecimal(pszDecimal);
	}
	if(pszFont != NULL)
	{
		vp.addItem( (void *) "field-font"); vp.addItem( (void *) pszFont);
	}
	//
	// Assemble the List attributes
	//
	UT_uint32 counta = va.getItemCount() + 1;
	const XML_Char ** attribs = (const XML_Char **) calloc(counta, sizeof(XML_Char *));
	for(i=0; i<va.getItemCount();i++)
	{
		attribs[i] = (XML_Char *) va.getNthItem(i);
	}
	attribs[i] = (XML_Char *) NULL;
	//
	// Now assemble the list properties
	//
	UT_uint32 countp = vp.getItemCount() + 1;
	const XML_Char ** props = (const XML_Char **) calloc(countp, sizeof(XML_Char *));
	for(i=0; i<vp.getItemCount();i++)
	{
		props[i] = (XML_Char *) vp.getNthItem(i);
	}
	props[i] = (XML_Char *) NULL;

 	//const XML_Char * attrib_list[] = {"style", style, 0 };
	_eraseInsertionPoint();
	i = 0;
	sdh = (PL_StruxDocHandle) pAuto->getNthBlock(i);
	while(sdh != NULL)
	{
		PT_DocPosition iPos = m_pDoc->getStruxPosition(sdh)+fl_BLOCK_STRUX_OFFSET;
		bRet = m_pDoc->changeStruxFmt(PTC_AddFmt, iPos, iPos, attribs, props, PTX_Block);
		i++;
		sdh = (PL_StruxDocHandle) pAuto->getNthBlock(i);
		_generalUpdate();
	}

	_generalUpdate();
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	_ensureThatInsertionPointIsOnScreen();
	DELETEP(attribs);
	DELETEP(props);
}

UT_Bool FV_View::cmdStopList(void)
{


	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	m_pDoc->StopList(pBlock->getStruxDocHandle());
	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

	return UT_TRUE;
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

	v.addItem(new _fmtPair((XML_Char*)"columns", NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair((XML_Char*)"column-line", NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair((XML_Char*)"column-gap",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair((XML_Char*)"page-margin-left",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair((XML_Char*)"page-margin-top",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair((XML_Char*)"page-margin-right",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));
	v.addItem(new _fmtPair((XML_Char*)"page-margin-bottom",NULL,pBlockAP,pSectionAP,m_pDoc,UT_FALSE));

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

	v.addItem(new _fmtPair((XML_Char*)"text-align",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"text-indent",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"margin-left",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"margin-right",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"margin-top",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"margin-bottom",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"line-height",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"tabstops",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"default-tab-interval",	NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"keep-together",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"keep-with-next",			NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"orphans",				NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));
	v.addItem(new _fmtPair((XML_Char*)"widows",					NULL,pBlockAP,pSectionAP,m_pDoc,bExpandStyles));

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


	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	if (iPos == getPoint())
	{
		return;
	}

	_extSelToPos(iPos);
	_deleteSelection();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

UT_Bool FV_View::isTabListBehindPoint(void)
{
	PT_DocPosition cpos = getPoint();
	PT_DocPosition ppos = cpos - 1;
	PT_DocPosition posBOD;
	UT_Bool bRes;
	UT_Bool bEOL = UT_FALSE;
	UT_sint32 xPoint, yPoint, iPointHeight;

	bRes = m_pDoc->getBounds(UT_FALSE, posBOD);
	UT_ASSERT(bRes);
	if (cpos <= posBOD - 1)
	{
		return UT_FALSE;
	}
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(cpos);
	if (!pBlock)
		return UT_FALSE;
	if(pBlock->isListItem() == UT_FALSE)
		return UT_FALSE;
	fl_BlockLayout* ppBlock = _findBlockAtPosition(ppos);
	if (!ppBlock || pBlock != ppBlock)
	{
		return UT_FALSE;
	}

	fp_Run* pRun = pBlock->findPointCoords(ppos, bEOL, xPoint, yPoint, iPointHeight);
	if(pRun->getType() != FPRUN_TAB)
	{
		return UT_FALSE;
	}
	pRun = pRun->getPrev();
	while((pRun != NULL) && (pRun->getType()== FPRUN_FMTMARK))
	{
		pRun = pRun->getPrev();
	}
	if (!pRun || pRun->getType() != FPRUN_FIELD)
	{
		return UT_FALSE;
	}
	else
	{
		fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
		if (pFRun->getFieldType() != FPFIELD_list_label)
		{
			return UT_FALSE;
		}
		return UT_TRUE;
	}
}


UT_Bool FV_View::isTabListAheadPoint(void)
{
	//
	// Return TRUE if the point is immediately ahead of a list label - TAB combination
	//

	PT_DocPosition cpos = getPoint();
	
	fl_BlockLayout* pBlock = _findBlockAtPosition(cpos);
	if (!pBlock || pBlock->isListItem() == UT_FALSE)
	{
		return UT_FALSE;
	}

	UT_Bool bEOL = UT_FALSE;
	UT_sint32 xPoint, yPoint, iPointHeight;

	fp_Run* pRun = pBlock->findPointCoords(cpos, bEOL, xPoint, yPoint, iPointHeight);

	// Find first run that is not an FPRUN_FMTMARK
	while (pRun && (pRun->getType() == FPRUN_FMTMARK))
	{
		pRun = pRun->getNext();
	}

	if (!pRun || pRun->getType() != FPRUN_FIELD)
	{
		return UT_FALSE;
	}

	fp_FieldRun * pFRun = (fp_FieldRun *) pRun;
	if(pFRun->getFieldType() != FPFIELD_list_label)
	{
		return UT_FALSE;
	}

	pRun = pRun->getNext();
	while (pRun && (pRun->getType()== FPRUN_FMTMARK))
	{
		pRun = pRun->getNext();
	}
	if (!pRun || pRun->getType() != FPRUN_TAB)
	{
		return UT_FALSE;
	}

	return UT_TRUE;
}

void FV_View::cmdCharDelete(UT_Bool bForward, UT_uint32 count)
{
	const XML_Char * properties[] = { (XML_Char*)"font-family", NULL, 0};
	const XML_Char ** props_in = NULL;
	const XML_Char * currentfont;
	UT_Bool bisList = UT_FALSE;
	fl_BlockLayout * curBlock = NULL;
	fl_BlockLayout * nBlock = NULL;


	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	if (!isSelectionEmpty())
	{
		m_pDoc->disableListUpdates();

		_deleteSelection();

		_generalUpdate();

		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
	else
	{
		//
		// Look to see if there is a tab - list label deal with these together
		//
		if((bForward == UT_FALSE) && (count == 1))
		{
			if(isTabListBehindPoint() == UT_TRUE)
			{
				curBlock = _findBlockAtPosition(getPoint()); 
				nBlock = _findBlockAtPosition(getPoint()-2);
				if(nBlock == curBlock)
				{
					count = 2;
					bisList = UT_TRUE;
				}
			}
		}
		if((bForward == UT_TRUE) && (count == 1))
		{
			if(isTabListAheadPoint() == UT_TRUE)
			{
				count = 2;
				bisList = UT_TRUE;
			}

		}
	// Code to deal with font boundary problem.
	// TODO: This should really be fixed by someone who understands
	// how this code works! In the meantime save current font to be
	// restored after character is deleted.

		getCharFormat(&props_in);
		currentfont = UT_getAttribute((XML_Char*)"font-family",props_in);
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
			// Code to deal with change of font boundaries: 
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
			m_pDoc->disableListUpdates();

			m_pDoc->deleteSpan(posCur, posCur+amt);
 			nBlock = _findBlockAtPosition(getPoint());
 			if(nBlock->getAutoNum() != NULL && nBlock->isListLabelInBlock() == UT_FALSE)
 			{
 				nBlock->remItemFromList();
 			}
 			
			// restore updates and clean up dirty lists
			m_pDoc->enableListUpdates();
			m_pDoc->updateDirtyLists();

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

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

}

void FV_View::_moveInsPtNextPrevLine(UT_Bool bNext)
{
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
	UT_sint32 iLineHeight;

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
	// Sevior TODO the +2 is a work around. The problem is somewhere else
			yPoint += (iLineHeight + pOldLine->getMarginAfter()+2);
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
	// Sevior TODO the +2 is a work around. The problem is somewhere else
			yPoint -= (pOldLine->getMarginBefore() + 2);
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
	if(bRet == UT_FALSE)
	{
		_fixInsertionPointCoords();
		_drawInsertionPoint();
	}

	return bRet;
}

void FV_View::_moveInsPtNextPrevPage(UT_Bool bNext)
{
#if 0
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
#endif

	fp_Page* pOldPage = _getCurrentPage();

	// try to locate next/prev page
	fp_Page* pPage = (bNext ? pOldPage->getNext() : pOldPage->getPrev());

	// if couldn't move, go to top of this page instead
	if (!pPage) 
		pPage = pOldPage;

	_moveInsPtToPage(pPage);
}

fp_Page *FV_View::_getCurrentPage(void)
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

	return pOldPage;
}

void FV_View::_moveInsPtNthPage(UT_uint32 n)
{
	fp_Page *page = m_pLayout->getFirstPage();

	if (n > m_pLayout->countPages ())
		n = m_pLayout->countPages ();

	for (UT_uint32 i = 1; i < n; i++)
	{
		page = page->getNext ();
	}

	_moveInsPtToPage(page);
}

void FV_View::_moveInsPtToPage(fp_Page *page)
{
	// move to the first pos on this page
	PT_DocPosition iNewPoint = page->getFirstLastPos(UT_TRUE);
	_setPoint(iNewPoint, UT_FALSE);

	// explicit vertical scroll to top of page
	UT_sint32 iPageOffset;
	getPageYOffset(page, iPageOffset);

	iPageOffset -= fl_PAGEVIEW_PAGE_SEP /2;
	iPageOffset -= m_yScrollOffset;
	
	UT_Bool bVScroll = UT_FALSE;
	if (iPageOffset < 0)
	{
		_eraseInsertionPoint();	
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-iPageOffset));
		bVScroll = UT_TRUE;
	}
	else if (iPageOffset > 0)
	{
		_eraseInsertionPoint();	
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
		_eraseInsertionPoint();
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

	// It IS possible for the selection to be empty, even
	// after extending it.  If the charMotion fails, for example,
	// because we are at the end of a document, then the selection
	// will end up empty once again.

	if (isSelectionEmpty())
	{
		_resetSelection();
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

	if(pView->getLayout()->getDocument()->isPieceTableChanging() == UT_TRUE)
	{
		return;
	}

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
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(yPos)));
			}
			else if (((UT_uint32) (yPos)) >= ((UT_uint32) pView->m_iWindowHeight))
			{
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(yPos - pView->m_iWindowHeight));
			}

			if (xPos < 0)
			{
				pView->_eraseInsertionPoint();	
				pView->cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(xPos)));
			}
			else if (((UT_uint32) (xPos)) >= ((UT_uint32) pView->m_iWindowWidth))
			{
				pView->_eraseInsertionPoint();	
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
	// Figure out which page we clicked on.
	// Pass the click down to that page.
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
	
	// Figure out which page we clicked on.
	// Pass the click down to that page.
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition iNewPoint;
	UT_Bool bBOL, bEOL;

	bBOL = bEOL = UT_FALSE;
	pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);

	//UT_ASSERT(!isSelectionEmpty());

	if (iNewPoint <= m_iSelectionLeftAnchor)
	{
		m_iSelectionAnchor = m_iSelectionRightAnchor;
	}
	else
	{
		m_iSelectionAnchor = m_iSelectionLeftAnchor;
	}

	const FV_DocPos argDocPos =
		iNewPoint > m_iSelectionAnchor ? FV_DOCPOS_EOW_SELECT : FV_DOCPOS_BOW;
	const PT_DocPosition iNewPointWord = _getDocPosFromPoint(iNewPoint, argDocPos,UT_FALSE);

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

UT_Bool FV_View::gotoTarget(AP_JumpTarget type, UT_UCSChar *data)
{
	UT_ASSERT(m_pLayout);
	UT_Bool inc = UT_FALSE;
	UT_Bool dec = UT_FALSE;

	char * numberString = (char *) calloc(UT_UCS_strlen(data) + 1, sizeof(char));
	UT_ASSERT(numberString);
	
	UT_UCS_strcpy_to_char(numberString, data);
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}

	switch (numberString[0])
	{
	case '+':
		inc = UT_TRUE;
		numberString++;
		break;
	case '-':
		dec = UT_TRUE;
		numberString++;
		break;
	}

	UT_uint32 number = atol(numberString);

	if (dec || inc)
		numberString--;
	FREEP(numberString);

	// check for range
	//	if (number < 0 || number > (UT_uint32) m_pLayout->countPages())
	//		return UT_FALSE;
	
	switch (type)
	{
	case AP_JUMPTARGET_PAGE:
	{
		if (!inc && !dec)
			_moveInsPtNthPage (number);
		else
		{
			fp_Page* pOldPage = _getCurrentPage();
			fp_Page* pPage = pOldPage;
			fp_Page* pTmpPage = pOldPage;

			if (inc) // TODO:  What if number passes the number of pages?
				for (UT_uint32 i = 0; i < number; i++)
				{
					if ((pTmpPage = pPage->getNext ()) != NULL)
						pPage = pTmpPage;
					else
						break;
				}
			else
				for (UT_uint32 i = 0; i < number; i++)
				{
					if ((pTmpPage = pPage->getPrev ()) != NULL)
						pPage = pTmpPage;
					else
						break;
				}

			if (!pPage) 
				pPage = pOldPage;

			_moveInsPtToPage (pPage);
		}

		notifyListeners(AV_CHG_MOTION);

		break;
	}
	case AP_JUMPTARGET_LINE:
		if (inc || dec)
		{
			UT_Bool bNext;
			bNext = inc;

			for (UT_uint32 i = 0; i < number; i++)
			{
				_moveInsPtNextPrevLine (bNext);	// HACK: A like the quick hacks... :)
			}
		}
		else
		{
			//UT_uint32 line = 0;
			fl_SectionLayout * pSL = m_pLayout->getFirstSection();
			fl_BlockLayout * pBL = pSL->getFirstBlock();
			fp_Line* pLine = pBL->getFirstLine();
			
			for (UT_uint32 i = 1; i < number; i++)
			{
				fp_Line* pOldLine = pLine;
				
				if ((pLine = pLine->getNext ()) == NULL)
				{
					if ((pBL = pBL->getNext ()) == NULL)
					{
						if ((pSL = pSL->getNext ()) == NULL)
						{
							pLine = pOldLine;
							break;
						}
						else
							pBL = pSL->getFirstBlock ();
					}
					else
					{
						pLine = pBL->getFirstLine ();
					}
				}
			}

			fp_Run* frun = pLine->getFirstRun ();
			fl_BlockLayout* fblock = pLine->getBlock ();
			PT_DocPosition dp = frun->getBlockOffset () + fblock->getPosition ();
			moveInsPtTo (dp);
		}
	
		notifyListeners(AV_CHG_MOTION);
		
		break;
	case AP_JUMPTARGET_PICTURE:
		// TODO
		break;
	default:
		// TODO
		;
	}

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

	fl_BlockLayout* block = NULL;
	PT_DocPosition offset = 0;
	
	block = _findGetCurrentBlock();
	offset = _findGetCurrentOffset();

	UT_UCSChar * buffer = NULL;

	//Now we compute the static prefix function
	//Which can be done based soley on the find string

	UT_uint32	m = UT_UCS_strlen(find);
	UT_uint32	*prefix;
	UT_uint32	k = 0;
	UT_uint32	q = 1;

	prefix = (UT_uint32*) calloc (m, sizeof(UT_uint32));

	prefix[0] = 0; //Must be this reguardless of the string

	if (matchCase)
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && find[k] != find[q])
				k = prefix[k - 1];
			if(find[k] == find[q])
				k++;
			prefix[q] = k;
		}
	}
	else //!matchCase
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && UT_UCS_tolower(find[k]) != UT_UCS_tolower(find[q]))
				k = prefix[k - 1];
			if(UT_UCS_tolower(find[k]) == UT_UCS_tolower(find[q]))
				k++;
			prefix[q] = k;
		}
	}

	//Now we use this prefix function (stored as an array)
	//to search through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		
		// magic number; range of UT_sint32 falls short of extremely large docs
		UT_sint32	foundAt = -1;
		UT_uint32	i = 0;
		UT_uint32	t = 0;

		if (matchCase)
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && find[t] != buffer[i])
					t = prefix[t-1];
				if (find[t] == buffer[i])
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}
		else //!matchCase
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && UT_UCS_tolower(find[t]) != UT_UCS_tolower(buffer[i]))
					t = prefix[t-1];
				if (UT_UCS_tolower(find[t]) == UT_UCS_tolower(buffer[i]))
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}


		if (foundAt != -1)
		{
			_setPoint(block->getPosition(UT_FALSE) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(UT_TRUE, UT_UCS_strlen(find));

			m_doneFind = UT_TRUE;

			FREEP(buffer);
			FREEP(prefix);
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

	FREEP(prefix);
	
	return UT_FALSE;
}

//Does exactly the same as the previous function except that the prefix
//function is passed in as an agrument rather than computed within
//the function body.
UT_Bool FV_View::_findNext(const UT_UCSChar * find, UT_uint32 *prefix,
						   UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	UT_ASSERT(find);

	fl_BlockLayout*	block = NULL;
	PT_DocPosition	offset = 0;
	
	block = _findGetCurrentBlock();
	offset = _findGetCurrentOffset();

	UT_UCSChar * buffer = NULL;
	UT_uint32	m = UT_UCS_strlen(find);

	

	//Now we use the prefix function (stored as an array)
	//to search through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		
		// magic number; range of UT_sint32 falls short of extremely large docs
		UT_sint32	foundAt = -1;
		UT_uint32	i = 0;
		UT_uint32	t = 0;

		if (matchCase)
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && find[t] != buffer[i])
					t = prefix[t-1];
				if (find[t] == buffer[i])
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}
		else //!matchCase
		{
			while (buffer[i] /*|| foundAt == -1*/)
			{
				while (t > 0 && UT_UCS_tolower(find[t]) != UT_UCS_tolower(buffer[i]))
					t = prefix[t-1];
				if (UT_UCS_tolower(find[t]) == UT_UCS_tolower(buffer[i]))
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
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
	bufferSegment = (UT_UCSChar*)calloc(bufferLength + 1, sizeof(UT_UCSChar));
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

		PP_AttrProp AttrProp_Before;

		if (!isSelectionEmpty())
		{
			_eraseInsertionPoint();
			_deleteSelection(&AttrProp_Before);
		}
		else
		{
			_eraseInsertionPoint();
		}

		// if we have a string with length, do an insert, else let it hang
		// from the delete above
		if (*replace)
			result = m_pDoc->insertSpan(getPoint(), 
											replace, 
											UT_UCS_strlen(replace), 
											&AttrProp_Before);

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


UT_Bool	FV_View::_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
							  UT_uint32 *prefix, UT_Bool matchCase, UT_Bool * bDoneEntireDocument)
{
	UT_ASSERT(find && replace);

	// if we have done a find, and there is a selection, then replace what's in the
	// selection and move on to next find (batch run, the common case)
	if ((m_doneFind == UT_TRUE) && (!isSelectionEmpty()))
	{
		UT_Bool result = UT_TRUE;

		PP_AttrProp AttrProp_Before;

		if (!isSelectionEmpty())
		{
			_eraseInsertionPoint();
			_deleteSelection(&AttrProp_Before);
		}
		else
		{
			_eraseInsertionPoint();
		}

		// if we have a string with length, do an insert, else let it hang
		// from the delete above
		if (*replace)
			result = m_pDoc->insertSpan(getPoint(), 
											replace, 
											UT_UCS_strlen(replace), 
											&AttrProp_Before);

		_generalUpdate();

			// if we've wrapped around once, and we're doing work before we've
			// hit the point at which we started, then we adjust the start
			// position so that we stop at the right spot.
		if (m_wrappedEnd && !*bDoneEntireDocument)
			m_startPosition += ((long) UT_UCS_strlen(replace) - (long) UT_UCS_strlen(find));

		UT_ASSERT(m_startPosition >= 2);

		// do not increase the insertion point index, since the insert span will
		// leave us at the correct place.
		
		_findNext(find, prefix, matchCase, bDoneEntireDocument);
		return result;
	}

	// if we have done a find, but there is no selection, do a find for them
	// but no replace
	if (m_doneFind == UT_TRUE && isSelectionEmpty() == UT_TRUE)
	{
		_findNext(find, prefix, matchCase, bDoneEntireDocument);
		return UT_FALSE;
	}
	
	// if we haven't done a find yet, do a find for them
	if (m_doneFind == UT_FALSE)
	{
		_findNext(find, prefix, matchCase, bDoneEntireDocument);
		return UT_FALSE;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}

void FV_View::insertSymbol(UT_UCSChar c, XML_Char * symfont)
{

	// First check to see if there is a selection already.
	// if so delete it then get the current font
	m_pDoc->beginUserAtomicGlob();

	if (!isSelectionEmpty())
	{
		_deleteSelection();
		_generalUpdate();
	} 
	// We have to determine the current font so we can put it back after
	// Inserting the Symbol

	const XML_Char ** props_in = NULL;
	const XML_Char * currentfont;
	getCharFormat(&props_in);
	currentfont = UT_getAttribute((XML_Char*)"font-family",props_in);
	free(props_in);

	if(strstr(symfont,currentfont) == NULL) 
	{
		// Set the font 
		const XML_Char* properties[] = { (XML_Char*)"font-family", 0, 0 };
		properties[1] = symfont ;
		setCharFormat(properties);

		// Insert the character 
		cmdCharInsert(&c,1);

		// Change the font back to its original value 
		properties[1] = currentfont;
		setCharFormat(properties);
		_generalUpdate();
	}
	else
	{
		// Just insert the character 

		cmdCharInsert(&c,1);
	}
	m_pDoc->endUserAtomicGlob();
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

	//Now we compute the static prefix function
	//Which can be done based soley on the find string
	//we compute this here rather than inside the _findNext
	//function so that it is not needlessy re-computed
	//this will save a lot on large documents with many
	//hits of the find word.

	UT_uint32	m = UT_UCS_strlen(find);
	UT_uint32	*prefix;
	UT_uint32	k = 0;
	UT_uint32	q = 1;

	prefix = (UT_uint32*) calloc (m, sizeof(UT_uint32));

	prefix[0] = 0; //Must be this reguardless of the string

	if (matchCase)
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && find[k] != find[q])
				k = prefix[k - 1];
			if(find[k] == find[q])
				k++;
			prefix[q] = k;
		}
	}
	else //!matchCase
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && UT_UCS_tolower(find[k]) != UT_UCS_tolower(find[q]))
				k = prefix[k - 1];
			if(UT_UCS_tolower(find[k]) == UT_UCS_tolower(find[q]))
				k++;
			prefix[q] = k;
		}
	}
	
	// prime it with a find
	if (!_findNext(find, prefix, matchCase, &bDoneEntireDocument))
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
		if (!_findReplace(find, replace, prefix, matchCase, &bDoneEntireDocument))
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

	//Clean up the prefix function array
	FREEP(prefix);

	return numReplaced;
}

fl_BlockLayout * FV_View::_findGetCurrentBlock(void)
{
	return m_pLayout->findBlockAtPosition(m_iInsPoint);
}


fl_BlockLayout * FV_View::getCurrentBlock(void)
{
	return _findGetCurrentBlock();
}

PT_DocPosition FV_View::_findGetCurrentOffset(void)
{
	return (m_iInsPoint - _findGetCurrentBlock()->getPosition(UT_FALSE));
}

//
// A simple strstr search of the buffer.
//
UT_sint32 FV_View::_findBlockSearchDumbCase(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_strstr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}

//
// Pierre Sarrazin <ps@cam.org> supplied the Unicode stricmp comparison function,
// which works for Latin-1 at the moment.
//
UT_sint32 FV_View::_findBlockSearchDumbNoCase(const UT_UCSChar * haystack, const UT_UCSChar * needle)
{
	UT_ASSERT(haystack);
	UT_ASSERT(needle);
		
	UT_UCSChar * at = UT_UCS_stristr(haystack, needle);

	return (at) ? (at - haystack) : -1;
}


//
// Any takers?
//
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
		_fixInsertionPointCoords();
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

	_fixInsertionPointCoords();
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
		if (!pCurRun)
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

	_fixInsertionPointCoords();
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

	PT_DocPosition posEOD;
	UT_Bool bRes;


	// The idea of the following code is thus: we need the previous block in 
	// the document.  But at the beginning of the document, sometimes there
	// isn't a previous block.  So we get the next block in the document.  
	// If we don't do this, we end up in big trouble, since we reference
	// that block in about 8 lines.   Sam, 11.9.00

	bRes = m_pDoc->getBounds(UT_TRUE, posEOD);
	UT_ASSERT(bRes);
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);
	while(!pBlock && (pos < posEOD))
	{
		pBlock = _findBlockAtPosition((PT_DocPosition) pos++);
	}
	pos = (PT_DocPosition) pos; 

	//UT_ASSERT(pBlock);

	// probably an empty document, return instead of
	// dereferencing NULL.  Dom 11.9.00
	if(!pBlock)
	{
		x = 0;
		y = 0;
		height = 0;
		*ppBlock = 0;
		return;
	}

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
	_eraseInsertionPoint();
	_findPositionCoords(getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_iPointHeight, NULL, NULL);
	_saveCurrentPoint();
	// hang onto this for _moveInsPtNextPrevLine()
	m_xPointSticky = m_xPoint + m_xScrollOffset - fl_PAGEVIEW_MARGIN_X;
}

void FV_View::_updateInsertionPoint()
{
	if (isSelectionEmpty())
	{
		if (!_ensureThatInsertionPointIsOnScreen())
		{
			_fixInsertionPointCoords();
			_drawInsertionPoint();
		}
	}
}

UT_Bool FV_View::_hasPointMoved(void)
{
	if( m_xPoint == m_oldxPoint && m_yPoint == m_oldyPoint && m_iPointHeight ==  m_oldiPointHeight)
	{
		return UT_FALSE;
	}

	return UT_TRUE;
}

void  FV_View::_saveCurrentPoint(void)
{
	m_oldxPoint = m_xPoint;
	m_oldyPoint = m_yPoint;
	m_oldiPointHeight = m_iPointHeight;
}

void  FV_View::_clearOldPoint(void)
{
	m_oldxPoint = -1;
	m_oldyPoint = -1;
	m_oldiPointHeight = 0;
}

void FV_View::_xorInsertionPoint()
{
	if (m_iPointHeight > 0 )
	{
		UT_RGBColor clr(255,255,255);

		m_pG->setColor(clr);
		m_pG->xorLine(m_xPoint-1, m_yPoint+1, m_xPoint-1, m_yPoint + m_iPointHeight+1);
		m_pG->xorLine(m_xPoint, m_yPoint+1, m_xPoint, m_yPoint + m_iPointHeight+1);
		m_bCursorIsOn = !m_bCursorIsOn;
	}
	if(_hasPointMoved() == UT_TRUE)
	{
		m_bCursorIsOn = UT_TRUE;
	}
	_saveCurrentPoint();
}

UT_Bool FV_View::isCursorOn(void)
{
	return m_bCursorIsOn;
}

void FV_View::eraseInsertionPoint(void)
{
	_eraseInsertionPoint();
}

void FV_View::_eraseInsertionPoint()
{
	m_bEraseSaysStopBlinking = UT_TRUE;
	if (_hasPointMoved() == UT_TRUE)
	{
		UT_DEBUGMSG(("Insertion Point has moved before erasing \n"));
		if (m_pAutoCursorTimer) 
			m_pAutoCursorTimer->stop();
		m_bCursorIsOn = UT_FALSE;
		_saveCurrentPoint();
		return;
	}

	//	if (m_pAutoCursorTimer) 
	//		m_pAutoCursorTimer->stop();

	
	if (m_bCursorIsOn && isSelectionEmpty())
	{
		_xorInsertionPoint();
	}
	m_bCursorIsOn = UT_FALSE;
}

void FV_View::drawInsertionPoint()
{
	_drawInsertionPoint();
}

void FV_View::_drawInsertionPoint()
{
	if(m_focus==AV_FOCUS_NONE)
		return;
	if (m_bCursorBlink && (m_focus==AV_FOCUS_HERE || m_focus==AV_FOCUS_MODELESS || AV_FOCUS_NEARBY))
	{
		if (m_pAutoCursorTimer == NULL) {
			m_pAutoCursorTimer = UT_Timer::static_constructor(_autoDrawPoint, this, m_pG);
			m_pAutoCursorTimer->set(AUTO_DRAW_POINT);
			m_bCursorIsOn = UT_FALSE;
		}
		m_pAutoCursorTimer->stop();
		m_pAutoCursorTimer->start();
	}
	m_bEraseSaysStopBlinking = UT_FALSE;
	if (m_iWindowHeight <= 0)
	{
		return;
	}
	
	if (!isSelectionEmpty())
	{
		return;
	}
	UT_ASSERT(m_bCursorIsOn == UT_FALSE);
	if (m_bCursorIsOn == UT_FALSE)
	{
		_xorInsertionPoint();
	}
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
	if (pView->m_bEraseSaysStopBlinking == UT_FALSE)
	{
		pView->_xorInsertionPoint();
	}
}

void FV_View::setXScrollOffset(UT_sint32 v)
{
	UT_sint32 dx = v - m_xScrollOffset;

	if (dx == 0)
		return;
	_fixInsertionPointCoords();
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
	_fixInsertionPointCoords();
	_drawInsertionPoint();

}

void FV_View::setYScrollOffset(UT_sint32 v)
{
	UT_sint32 dy = v - m_yScrollOffset;

	if (dy == 0)
		return;
	_fixInsertionPointCoords();
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
	_fixInsertionPointCoords();
	_drawInsertionPoint();
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
	_fixInsertionPointCoords();
	if (pClipRect)
	{
		_draw(pClipRect->left,pClipRect->top,pClipRect->width,pClipRect->height,UT_FALSE,UT_TRUE);
	}
	else
	{
		_draw(0,0,m_iWindowWidth,m_iWindowHeight,UT_FALSE,UT_FALSE);
	}
	_fixInsertionPointCoords();
	_drawInsertionPoint();
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
		UT_sint32 iPageWidth		= pPage->getWidth();
		UT_sint32 iPageHeight		= pPage->getHeight();
		UT_sint32 adjustedTop		= curY - m_yScrollOffset;
		UT_sint32 adjustedBottom	= adjustedTop + iPageHeight + fl_PAGEVIEW_PAGE_SEP;
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

			UT_sint32 adjustedLeft	= fl_PAGEVIEW_MARGIN_X - m_xScrollOffset;
			UT_sint32 adjustedRight	= adjustedLeft + iPageWidth;

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
			if (m_iWindowWidth - (adjustedRight + 1) > 0)
			{
				m_pG->fillRect(clrMargin, adjustedRight + 1, adjustedTop, m_iWindowWidth - (adjustedRight + 1), iPageHeight + 1);
			}

			// fill separator below page
			if (m_iWindowHeight - (adjustedBottom + 1) > 0)
			{
				m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, fl_PAGEVIEW_PAGE_SEP);
			}
			
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
		_fixInsertionPointCoords();
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

	if (m_bDontChangeInsPoint == UT_TRUE)
	{
		return;
	}
	m_iInsPoint = pt;
	m_bPointEOL = bEOL;

	m_pLayout->considerPendingSmartQuoteCandidate();
	_checkPendingWordForSpell();
}

void FV_View::setPoint(PT_DocPosition pt)
{
	if (m_bDontChangeInsPoint == UT_TRUE)
	{
		return;
	}
	m_iInsPoint = pt;
	m_pLayout->considerPendingSmartQuoteCandidate();
	_checkPendingWordForSpell();
}

void FV_View::setDontChangeInsPoint(void)
{
	m_bDontChangeInsPoint = UT_TRUE;
}

void FV_View::allowChangeInsPoint(void)
{
	m_bDontChangeInsPoint = UT_FALSE;
}


UT_Bool FV_View::isDontChangeInsPoint(void)
{
	return m_bDontChangeInsPoint;
}

void FV_View::_checkPendingWordForSpell(void)
{
	if(m_pDoc->isPieceTableChanging() == UT_TRUE)
	{
		return;
	}
	// deal with pending word, if any
	if (m_pLayout->isPendingWordForSpell())
	{
		fl_BlockLayout* pBL = _findBlockAtPosition(m_iInsPoint);
		if (pBL)
		{
			UT_uint32 iOffset = m_iInsPoint - pBL->getPosition();

			if (!m_pLayout->touchesPendingWordForSpell(pBL, iOffset, 0))
			{
				// no longer there, so check it
				if (m_pLayout->checkPendingWordForSpell())
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
	fp_Run* pRun = NULL;
	fl_BlockLayout* pBlock = NULL;
	UT_sint32 x;
	UT_sint32 y;
	UT_uint32 uheight;
	m_bPointEOL = UT_FALSE;

	/*
	  we don't really care about the coords.  We're calling these
	  to get the Run pointer
	*/
	PT_DocPosition posBOD;
	PT_DocPosition posEOD;
	UT_Bool bRes;

	bRes = m_pDoc->getBounds(UT_FALSE, posBOD);
	bRes = m_pDoc->getBounds(UT_TRUE, posEOD);
	UT_ASSERT(bRes);

	if (bForward)
	{
		m_iInsPoint += countChars;
		_findPositionCoords(m_iInsPoint-1, UT_FALSE, x, y, uheight, &pBlock, &pRun);
		//		while(pRun != NULL && (pRun->isField() == UT_TRUE || pRun->getType() == FPRUN_FIELD && m_iInsPoint < posEOD))
		while(pRun != NULL && pRun->isField() == UT_TRUE && m_iInsPoint < posEOD)
		{
			m_iInsPoint++;
			_findPositionCoords(m_iInsPoint, UT_FALSE, x, y, uheight, &pBlock, &pRun);
		}
	}
	else
	{
		m_iInsPoint -= countChars;
//		if (m_iInsPoint < posBOD)
//		{
//			m_iInsPoint = posBOD;
//		}
		_findPositionCoords(m_iInsPoint, UT_FALSE, x, y, uheight, &pBlock, &pRun);
		while(pRun != NULL && pRun->isField() == UT_TRUE && m_iInsPoint > posBOD)
		{
			m_iInsPoint--;
			_findPositionCoords(m_iInsPoint-1, UT_FALSE, x, y, uheight, &pBlock, &pRun);
		}
	}

	UT_ASSERT(bRes);

	if ((UT_sint32) m_iInsPoint < (UT_sint32) posBOD)
	{
		m_iInsPoint = posBOD;

		if (m_iInsPoint != posOld)
		{
			m_pLayout->considerPendingSmartQuoteCandidate();
			_checkPendingWordForSpell();
			_clearIfAtFmtMark(posOld);
			notifyListeners(AV_CHG_MOTION);
		}
		
		return UT_FALSE;
	}

	if ((UT_sint32) m_iInsPoint > (UT_sint32) posEOD)
	{
		m_iInsPoint = posEOD;

		if (m_iInsPoint != posOld)
		{
			m_pLayout->considerPendingSmartQuoteCandidate();
			_checkPendingWordForSpell();
			_clearIfAtFmtMark(posOld);
			notifyListeners(AV_CHG_MOTION);
		}

		return UT_FALSE;
	}

	if (m_iInsPoint != posOld)
	{
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

	m_pDoc->undoCmd(count);
	allowChangeInsPoint();
	_generalUpdate();
	
	notifyListeners(AV_CHG_DIRTY);
	// Move insertion point out of field run if it is in one
	//
	_charMotion(UT_Bool UT_TRUE, 0);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

	m_pDoc->redoCmd(count);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	
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
	{
		notifyListeners(AV_CHG_SAVE);
	}
	return tmpVar;
}


UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft)
{
	UT_Error tmpVar;
	tmpVar = m_pDoc->saveAs(szFilename, ieft);
	if (!tmpVar)
	{
		notifyListeners(AV_CHG_SAVE);
	}
	return tmpVar;
}


void FV_View::cmdCut(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();
	cmdCopy();
	_deleteSelection();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	
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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();

	_doPaste(UT_TRUE);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	if (!isSelectionEmpty())
		m_pApp->cacheCurrentSelection(this);
	warpInsPtToXY(xPos,yPos);
	_doPaste(UT_FALSE);
	m_pApp->cacheCurrentSelection(NULL);

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	_eraseInsertionPoint();

	//	_clearIfAtFmtMark(getPoint()); TODO:	This was giving problems 
	//											caused bug 431 when changing
	//											columns.
	
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
	
	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

		fl_BlockLayout * pBlock = NULL;
		fp_Run * pRun = NULL;
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
		pInfo->m_xrLeftIndent = m_pG->convertDimension(pBlock->getProperty((XML_Char*)"margin-left"));
		pInfo->m_xrRightIndent = m_pG->convertDimension(pBlock->getProperty((XML_Char*)"margin-right"));
		pInfo->m_xrFirstLineIndent = m_pG->convertDimension(pBlock->getProperty((XML_Char*)"text-indent"));

		pInfo->m_pfnEnumTabStops = pBlock->s_EnumTabStops;
		pInfo->m_pVoidEnumTabStopsData = (void *)pBlock;
		pInfo->m_iTabStops = (UT_sint32) pBlock->getTabsCount();
		pInfo->m_iDefaultTabInterval = pBlock->getDefaultTabInterval();
		pInfo->m_pszTabStops = pBlock->getProperty((XML_Char*)"tabstops");

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

		fl_BlockLayout * pBlock = NULL;
		fp_Run * pRun = NULL;
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

/*
	currently unused
	fl_BlockLayout* pBL = _findBlockAtPosition(getPoint());
*/

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

	fd_Field * pField = NULL;
	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL,&pField);
		if(pField != NULL)
		{
			pField->update();
		}
		m_pDoc->endUserAtomicGlob();
	}
	else
	{
		_eraseInsertionPoint();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, NULL, &pField);
		if(pField != NULL)
		{
			pField->update();
		}
	}
	_generalUpdate();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();

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

	// Signal PieceTable Change 
	m_pDoc->notifyPieceTableChangeStart();

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
	m_pDoc->notifyPieceTableChangeEnd();

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

	UT_UCSChar theWord[101];
	// convert smart quote apostrophe to ASCII single quote to be compatible with ispell
	for (UT_uint32 ldex=0; ldex<pPOB->iLength && ldex<100; ++ldex)
	{
		UT_UCSChar currentChar;
		currentChar = *(pWord + ldex);
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		theWord[ldex] = currentChar;
	}
				
	SpellCheckSuggestNWord16(theWord, pPOB->iLength, &sg);

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
				m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
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
				m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
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
	if ( pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_CursorBlink, &b) && b != pView->m_bCursorBlink )
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
				UT_UCSChar followChar;
				followChar = (i+1 < len) ? pSpan[i+1] : UCS_UNKPUNK;
				newWord = (delim && !UT_isWordDelimiter(pSpan[i], followChar));
				
				delim = UT_isWordDelimiter(pSpan[i], followChar);
				
				/* 
					CJK-FIXME: this can work incorrectly under CJK locales since it can
					give 'true' for UCS with value >0xff (like quotes, etc).
				 */
				if (newWord || XAP_EncodingManager::instance->is_cjk_letter(pSpan[i]))
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

void FV_View::setShowPara(UT_Bool bShowPara)
{
	if (bShowPara != m_bShowPara)
	{
		m_bShowPara = bShowPara;
		draw();
	}
};

UT_Bool FV_View::insertHeaderFooter(const XML_Char ** props, UT_Bool ftr)
{

	/* 
		This inserts a header/footer at the end of the document,
		and leaves the insertion point there.
		This provides NO undo stuff.  Do it yourself.
	*/

	const XML_Char* szString = ftr ? "footer" : "header";

	// TODO: This stuff shouldn't be hardcoded
	// TODO: The fact that it is hardcoded means that only 
	// TODO: one section can have footers at one time, currently

	const XML_Char*	sec_attributes1[] = {
		"type", szString,
		"id", "page_num",
		NULL, NULL
	};

	const XML_Char*	sec_attributes2[] = {
		szString, "page_num",
		NULL, NULL
	};


	const XML_Char*	block_props[] = {
		"text-align", "center",
		NULL, NULL
	};

	if(!props)
		props = block_props; // use the defaults

	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
	else
	{
		_eraseInsertionPoint();
	}
	// change the section to point to the footer which doesn't exist yet.
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), sec_attributes2, NULL, PTX_Section);

	UT_DEBUGMSG(("EOD: %d\n", FV_DOCPOS_EOD));

	// Sevior: I don't think this is needed	UT_uint32 iPoint = FV_DOCPOS_EOD;

	UT_uint32 oldPos = getPoint(); // Save the old position in the document for later

	//
	// 
	//	fl_BlockLayout* pOldBlock = _findBlockAtPosition(getPoint());
	//fl_SectionLayout* pSL = pOldBlock->getSectionLayout();


	moveInsPtTo(FV_DOCPOS_EOD);	// Move to the end, where we will create the page numbers

	// Now create the footer section
	// First Do a block break to finish the last section.
	UT_uint32 iPoint = getPoint();
	m_pDoc->insertStrux(iPoint, PTX_Block);

	//
	// Now Insert the footer section. 
	// Doing things this way will grab the previously intereted block 
	// and put into the footter section.

	m_pDoc->insertStrux(iPoint, PTX_Section);
	m_pDoc->insertStrux(getPoint(), PTX_Block);


	// Make the new section into a footer
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), sec_attributes1, NULL, PTX_Section);
	// Change the formatting of the new footer appropriately (currently just center it)
	m_pDoc->changeStruxFmt(PTC_AddFmt, getPoint(), getPoint(), NULL, props, PTX_Block);

	return UT_TRUE;
}

UT_Bool FV_View::insertPageNum(const XML_Char ** props, UT_Bool ftr)
{
	
	/*
		This code implements some hardcoded hacks to insert a page number.
		It allows you to set the properties, but nothing else.  Use that
		to center, etc.

		Warning: this code assumes that _insertFooter() leaves the insertion
		point in a place where it can write the page_num field.
	*/

	const XML_Char*	f_attributes[] = {
		"type", "page_number",
		NULL, NULL
	};

	m_pDoc->beginUserAtomicGlob(); // Begin the big undo block

	// Signal PieceTable Changes have Started
	m_pDoc->notifyPieceTableChangeStart();

	_eraseInsertionPoint();

	UT_uint32 oldPos = getPoint();	// This ends up being redundant, but it's neccessary
	//UT_Bool bftr = UT_TRUE;
	UT_Bool bResult = insertHeaderFooter(props, ftr);

	//
	// after this call the insertion point is at the position where stuff
	// can be inserted into the header/footer
	//
	if(!bResult) 
		return UT_FALSE;
	
	// Insert the page_number field
	bResult = m_pDoc->insertObject(getPoint(), PTO_Field, f_attributes, NULL);

	moveInsPtTo(oldPos);	// Get back to where you once belonged.

	m_pLayout->updateLayout(); // Update document layout everywhere
	m_pDoc->endUserAtomicGlob(); // End the big undo block
	

	// Signal PieceTable Changes have Ended

	m_pDoc->notifyPieceTableChangeEnd();
	_generalUpdate();
	_fixInsertionPointCoords();
	_drawInsertionPoint();

	return bResult;
}

const fp_PageSize & FV_View::getPageSize(void) const
{
	return m_pLayout->getFirstPage()->getPageSize();
}
	
		
UT_uint32 FV_View::calculateZoomPercentForPageWidth()
{

	const fp_PageSize pageSize = getPageSize();
	double pageWidth = pageSize.Width(fp_PageSize::inch);
	
	// Set graphics zoom to 100 so we can get the display resolution.
	GR_Graphics *pG = getGraphics();
	UT_uint32 temp_zoom = pG->getZoomPercentage();
	pG->setZoomPercentage(100);
	UT_uint32 resolution = pG->getResolution();
	pG->setZoomPercentage(temp_zoom);

	// Verify scale as a positive non-zero number else return old zoom
	if ( ( getWindowWidth() - 2 * fl_PAGEVIEW_MARGIN_X ) <= 0 )
		return temp_zoom;

	double scale = (double)(getWindowWidth() - 2 * fl_PAGEVIEW_MARGIN_X) / 
											(pageWidth * (double)resolution);
	return (UT_uint32)(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForPageHeight()
{

	const fp_PageSize pageSize = getPageSize();
	double pageHeight = pageSize.Height(fp_PageSize::inch);
	
	// Set graphics zoom to 100 so we can get the display resolution.
	GR_Graphics *pG = getGraphics();
	UT_uint32 temp_zoom = pG->getZoomPercentage();
	pG->setZoomPercentage(100);
	UT_uint32 resolution = pG->getResolution();
	pG->setZoomPercentage(temp_zoom);

	// Verify scale as a positive non-zero number else return old zoom
	if ( ( getWindowHeight() - 2 * fl_PAGEVIEW_MARGIN_Y ) <= 0 )
		return temp_zoom;

	double scale = (double)(getWindowHeight() - 2 * fl_PAGEVIEW_MARGIN_Y) /
							(pageHeight * (double)resolution);
	return (UT_uint32)(scale * 100.0);
}

UT_uint32 FV_View::calculateZoomPercentForWholePage()
{
	return MyMin(	calculateZoomPercentForPageWidth(),
					calculateZoomPercentForPageHeight());
}


	

	





