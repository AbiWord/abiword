/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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
#include <locale.h>

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
#include "fl_Squiggles.h"
#include "fl_SectionLayout.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_PageSize.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
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
#include "xap_Frame.h"
#include "xap_Clipboard.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_Prefs.h"
#include "fd_Field.h"
#include "spell_manager.h"
#include "ut_rand.h"

#include "pp_Revision.h"
#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif

// NB -- irrespective of this size, the piecetable will store
// at max BOOKMARK_NAME_LIMIT of chars as defined in pf_Frag_Bookmark.h
#define BOOKMARK_NAME_SIZE 30
#define CHECK_WINDOW_SIZE if(getWindowHeight() < 20) return;
// returns true iff the character BEFORE pos is a space.
// Special cases:
// -returns true if pos is at the beginning of the document
// -returns false if pos is not within the document
bool FV_View::_isSpaceBefore(PT_DocPosition pos)
{
	UT_GrowBuf buffer;

	fl_BlockLayout * block = m_pLayout->findBlockAtPosition(pos);
	if (block)
	{

		PT_DocPosition offset = pos - block->getPosition(false);
		// Just look at the previous character in this block, if there is one...
		if (offset > 0)
		{
			block->getBlockBuf(&buffer);
			return (UT_UCS4_isspace(*(UT_UCSChar *)buffer.getPointer(offset - 1)));
		}
		else
		{
			return true;
		}
	}
	else
		return false;
}

/*!
  Reverse the direction of the current selection
  Does so without changing the screen.
*/
void FV_View::_swapSelectionOrientation(void)
{
	UT_ASSERT(!isSelectionEmpty());
	_fixInsertionPointCoords();
	PT_DocPosition curPos = getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	_setPoint(m_iSelectionAnchor);
	m_iSelectionAnchor = curPos;
}

/*!
  Move point to requested end of selection and clear selection
  \param bForward True if point should be moved to the forward position

  \note Do not draw the insertion point after clearing the
		selection.
  \fixme BIDI broken?
*/
void FV_View::_moveToSelectionEnd(bool bForward)
{
	UT_ASSERT(!isSelectionEmpty());

	PT_DocPosition curPos = getPoint();
	UT_ASSERT(curPos != m_iSelectionAnchor);
	bool bForwardSelection = (m_iSelectionAnchor < curPos);

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

	_clearBetweenPositions(iPos1, iPos2, true);
}

void FV_View::_clearSelection(void)
{
	if( isSelectionEmpty() )
	{
		return;
	}
	m_pG->getCaret()->enable();

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

	bool bres = _clearBetweenPositions(iPos1, iPos2, true);
	if(!bres)
		return;
	_resetSelection();

	_drawBetweenPositions(iPos1, iPos2);
}

void FV_View::_drawSelection()
{
	UT_return_if_fail(!isSelectionEmpty());
//	CHECK_WINDOW_SIZE
	if (m_iSelectionAnchor < getPoint())
	{
		_drawBetweenPositions(m_iSelectionAnchor, getPoint());
	}
	else
	{
		_drawBetweenPositions(getPoint(), m_iSelectionAnchor);
	}
}

// Note that isClearSelection() might change its tune in one of two ways.
// Way #1 is by calling one of the next few methods.
//   BUT! this never happens because m_iSelectionAnchor == getPoint by def.
// Way #2 is if m_bSelection is set and the point is changed so that it
//          no longer equals m_iSelectionAnchor.
void FV_View::_resetSelection(void)
{
	m_bSelection = false;
	m_iSelectionAnchor = getPoint();
}

void FV_View::_setSelectionAnchor(void)
{
	m_bSelection = true;
	m_iSelectionAnchor = getPoint();
}

void FV_View::_deleteSelection(PP_AttrProp *p_AttrProp_Before)
{
	// delete the current selection.
	// NOTE: this must clear the selection.

	UT_ASSERT(!isSelectionEmpty());

	PT_DocPosition iPoint = getPoint();

	UT_uint32 iRealDeleteCount;

	UT_uint32 iSelAnchor = m_iSelectionAnchor;
	if(iSelAnchor < 2)
	{
		iSelAnchor = 2;
	}

	UT_ASSERT(iPoint != iSelAnchor);

	UT_uint32 iLow = UT_MIN(iPoint,iSelAnchor);
	UT_uint32 iHigh = UT_MAX(iPoint,iSelAnchor);
	bool bDeleteTables = !isInTable(iLow) && !isInTable(iHigh);
	_eraseSelection();
	_resetSelection();
	bool bOldDelete = m_pDoc->isDontImmediateLayout();
	if(bDeleteTables)
	{
		m_pDoc->setDontImmediatelyLayout(true);
	}
	m_pDoc->deleteSpan(iLow, iHigh, p_AttrProp_Before, iRealDeleteCount, bDeleteTables);
	if(bDeleteTables)
	{
		m_pDoc->setDontImmediatelyLayout(bOldDelete);
	}
//
// Can't leave list-tab on a line
//
	if(isTabListAheadPoint() == true)
	{
		UT_uint32 iRealDeleteCount2;

		m_pDoc->deleteSpan(getPoint(), getPoint()+2, p_AttrProp_Before, iRealDeleteCount2);
		iRealDeleteCount += iRealDeleteCount2;
	}

	//special handling is required for delete in revisions mode
	//where we have to move the insertion point
	if(isMarkRevisions())
	{
		UT_ASSERT( iRealDeleteCount <= iHigh - iLow + 1 );
		_charMotion(true,iHigh - iLow - iRealDeleteCount);
	}

	m_pG->getCaret()->enable();
}

/*!
 * Do the merge between cells.
 * If bBefore is true the contents of source will be prepended into destination otherwise
 * will e appended to the end
 */ 
bool FV_View::_MergeCells( PT_DocPosition posDestination,PT_DocPosition posSource, bool bBefore)
{
//
// get coordinates of source and destination cells
//
	UT_sint32 sLeft,sRight,sTop,sBot;
	UT_sint32 dLeft,dRight,dTop,dBot;
	UT_sint32 fLeft,fRight,fTop,fBot;
	getCellParams(posSource,&sLeft,&sRight,&sTop,&sBot);
	getCellParams(posDestination,&dLeft,&dRight,&dTop,&dBot);
//
	fLeft = UT_MIN(sLeft,dLeft);
	fRight = UT_MAX(sRight,dRight);
	fTop = UT_MIN(sTop,dTop);
	fBot = UT_MAX(sBot,dBot);

	PD_DocumentRange dr_source;
	PL_StruxDocHandle sourceSDH,endSourceSDH,destinationSDH,endDestSDH;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posSource,PTX_SectionCell,&sourceSDH);
	if(!bres)
	{
		return false;
	}
	endSourceSDH =  m_pDoc->getEndCellStruxFromCellSDH(sourceSDH);
	PT_DocPosition posEndCell = m_pDoc->getStruxPosition(endSourceSDH)-1;
	posSource = m_pDoc->getStruxPosition(sourceSDH)+1;
	bres = m_pDoc->getStruxOfTypeFromPosition(posDestination,PTX_SectionCell,&destinationSDH);
	if(!bres)
	{
		return false;
	}
	endDestSDH = m_pDoc->getEndCellStruxFromCellSDH(destinationSDH);
	PT_DocPosition posEndDestCell = m_pDoc->getStruxPosition(endSourceSDH)-1;
	if(!bBefore)
	{
		posDestination = posEndDestCell;
	}
	if(posEndCell > posSource)
	{
//
// OK got the doc range for the source. Set it and copy it.
//
		dr_source.set(m_pDoc,posSource,posEndCell);
//
// Copy to and from clipboard to populate the destination cell
//
		UT_DEBUGMSG(("SEVIOR: Copy to clipboard merging cells \n"));
		m_pApp->copyToClipboard(&dr_source);
		PD_DocumentRange dr_dest(m_pDoc,posDestination,posDestination);
		UT_DEBUGMSG(("SEVIOR: Pasting from clipboard merging cells \n"));
		m_pApp->pasteFromClipboard(&dr_dest,true,true);
	}
//
// Now delete the source cell
//
	_deleteCellAt(posSource,sTop,sLeft);
//
// Expand the destination cell into the source cell
//
	_changeCellTo(posDestination,dTop,dLeft,fLeft,fRight,fTop,fBot);
//
// We're done!
//
	return true;
}

/*!
 * This method is used to change a parameter of the table to trigger a table
 * rebuild. It also restores all the nice needed for single step undo's
 */
bool FV_View::_restoreCellParams(PT_DocPosition posTable, UT_sint32 iLineType)
{
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	UT_String sLineType;
	UT_String_sprintf(sLineType,"%d",iLineType);
	pszTable[1] = sLineType.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	m_pDoc->allowChangeInsPoint();
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	return true;
}

/*!
 *  Change the parameters of the table.
 * Return the line type of the table. We'll restore this later.
 */
 UT_sint32 FV_View::_changeCellParams(PT_DocPosition posTable, PL_StruxDocHandle tableSDH)
{
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		PP_AttrProp AttrProp_Before;
		_deleteSelection(&AttrProp_Before);
		m_pDoc->endUserAtomicGlob();
	}
	m_pDoc->setDontImmediatelyLayout(true);
	m_pDoc->setDontChangeInsPoint();
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with a bogus line-type property. We'll restore it later.
//
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	const char * szLineType = NULL;
	UT_String sLineType;
	UT_sint32 iLineType;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szLineType);
	if(szLineType == NULL || *szLineType == '\0')
	{
		iLineType = 0;
	}
	else
	{
		iLineType = atoi(szLineType);
		iLineType -= 1;
	}
	UT_String_sprintf(sLineType,"%d",iLineType);
	pszTable[1] = sLineType.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
	return iLineType;
}

/*!
 * This method deletes the cell at (row,col) in the table specified by posTable
 */
bool FV_View::_deleteCellAt(PT_DocPosition posTable, UT_sint32 row, UT_sint32 col)
{
	PL_StruxDocHandle cellSDH,endCellSDH;
	PT_DocPosition posCell = findCellPosAt(posTable,row,col);
	if(posCell == 0)
	{
		return false;
	}
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell+1,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
	if(!endCellSDH)
	{
		return false;
	}
	PT_DocPosition posEndCell = m_pDoc->getStruxPosition(endCellSDH) +1;
	if(posEndCell == 0)
	{
		return false;
	}
//
// check that this
//

//
// We trust that calling routine does all the things needed to make sure this goes
// smoothly and undo's in a single step.
// Here we just delete.
//

//
// OK do the delete
//
	UT_uint32 iRealDeleteCount;

	m_pDoc->deleteSpan( posCell, posEndCell, NULL,iRealDeleteCount,true);
	return true;
}


/*!
 * This method changes the coordinates of the cell at (row,col) in the table specified by
 * posTable to the cordinates specified.
 */
bool FV_View::_changeCellTo(PT_DocPosition posTable, UT_sint32 rowold, UT_sint32 colold,
						  UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bot)
{
	PT_DocPosition posCell = findCellPosAt(posTable,rowold,colold) + 1;
	if(posCell == 0)
	{
		return false;
	}
	const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	UT_String sLeft,sRight,sTop,sBot;
	props[0] = "left-attach";
	UT_String_sprintf(sLeft,"%d",left);
	props[1] = sLeft.c_str();
	props[2] = "right-attach";
	UT_String_sprintf(sRight,"%d",right);
	props[3] = sRight.c_str();
	props[4] = "top-attach";
	UT_String_sprintf(sTop,"%d",top);
	props[5] = sTop.c_str();
	props[6] = "bot-attach";
	UT_String_sprintf(sBot,"%d",bot);
	props[7] = sBot.c_str();

//
// Here we trust that the calling routine will do all the begin/end globbing and other
// stuff so that this will go smoothly and undo's in a single step.
// Here we just change
//

	bool bres = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell,posCell,NULL,props,PTX_SectionCell);

	return bres;
}


/*!
 * This method inserts the cell before the coordinates of the cell at (row,col) in the table
 * specified by
 * posTable at the cordinates specified.
 */
bool FV_View::_insertCellBefore(PT_DocPosition posTable, UT_sint32 rowold, UT_sint32 colold,
						  UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bot)
{
	PT_DocPosition posCell = findCellPosAt(posTable,rowold,colold);
	if(posCell == 0)
	{
		return false;
	}
	posCell--;
	const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	UT_String sLeft,sRight,sTop,sBot;
	props[0] = "left-attach";
	UT_String_sprintf(sLeft,"%d",left);
	props[1] = sLeft.c_str();
	props[2] = "right-attach";
	UT_String_sprintf(sRight,"%d",right);
	props[3] = sRight.c_str();
	props[4] = "top-attach";
	UT_String_sprintf(sTop,"%d",top);
	props[5] = sTop.c_str();
	props[6] = "bot-attach";
	UT_String_sprintf(sBot,"%d",bot);
	props[7] = sBot.c_str();

//
// Here we trust that the calling routine will do all the begin/end globbing and other
// stuff so that this will go smoothly and undo's in a single step.
// Here we just insert
//

	bool bres = m_pDoc->insertStrux(posCell,PTX_SectionCell,NULL,props);
	if(!bres)
	{
		return false;
	}
//
// Insert a block for content
//
	bres = m_pDoc->insertStrux(posCell+1,PTX_Block);
	if(!bres)
	{
		return false;
	}
//
// Insert an endCell
//
	bres = m_pDoc->insertStrux(posCell+1,PTX_EndCell);
	if(!bres)
	{
		return false;
	}
	return bres;
}


/*!
 * This method inserts the cell after the coordinates of the cell at (row,col) in the table
 * specified by
 * posTable at the cordinates specified.
 */
bool FV_View::_insertCellAfter(PT_DocPosition posTable, UT_sint32 rowold, UT_sint32 colold,
						  UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bot)
{
	PL_StruxDocHandle cellSDH,endCellSDH;
	PT_DocPosition posCell = findCellPosAt(posTable,rowold,colold);
	if(posCell == 0)
	{
		return false;
	}
	bool bres = m_pDoc->getStruxOfTypeFromPosition(posCell+1,PTX_SectionCell,&cellSDH);
	if(!bres)
	{
		return false;
	}
	endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
	if(!endCellSDH)
	{
		return false;
	}
	PT_DocPosition posEndCell = m_pDoc->getStruxPosition(endCellSDH);
	if(posEndCell == 0)
	{
		return false;
	}
	posEndCell++;
	const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	UT_String sLeft,sRight,sTop,sBot;
	props[0] = "left-attach";
	UT_String_sprintf(sLeft,"%d",left);
	props[1] = sLeft.c_str();
	props[2] = "right-attach";
	UT_String_sprintf(sRight,"%d",right);
	props[3] = sRight.c_str();
	props[4] = "top-attach";
	UT_String_sprintf(sTop,"%d",top);
	props[5] = sTop.c_str();
	props[6] = "bot-attach";
	UT_String_sprintf(sBot,"%d",bot);
	props[7] = sBot.c_str();

//
// Here we trust that the calling routine will do all the begin/end globbing and other
// stuff so that this will go smoothly and undo's in a single step.
// Here we just insert
//

	bres = m_pDoc->insertStrux(posCell,PTX_SectionCell,NULL,props);
	if(!bres)
	{
		return false;
	}
//
// Insert a block for content
//
	bres = m_pDoc->insertStrux(posCell+1,PTX_Block);
	if(!bres)
	{
		return false;
	}
//
// Insert an endCell
//
	bres = m_pDoc->insertStrux(posCell+1,PTX_EndCell);
	if(!bres)
	{
		return false;
	}
	return bres;
}


PT_DocPosition FV_View::_getDocPos(FV_DocPos dp, bool bKeepLooking)
{
	return _getDocPosFromPoint(getPoint(),dp,bKeepLooking);
}

PT_DocPosition FV_View::_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, bool bKeepLooking)
{
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight;
	bool bDirection;

	PT_DocPosition iPos;

	// this gets called from ctor, so get out quick
	if (dp == FV_DOCPOS_BOD)
	{
		bool bRes = getEditableBounds(false, iPos);
		UT_ASSERT(bRes);

		return iPos;
	}

	// TODO: could cache these to save a lookup if point doesn't change
	fl_BlockLayout* pBlock = NULL;
	fp_Run* pRun = NULL;
	_findPositionCoords(iPoint, m_bPointEOL, xPoint,
						yPoint, xPoint2, yPoint2,
						iPointHeight, bDirection,
						&pBlock, &pRun);

	UT_return_val_if_fail ( pBlock, 0 );
	if ( !pRun )
	  return pBlock->getPosition() ;

	fp_Line* pLine = pRun->getLine();
	UT_return_val_if_fail ( pLine, pBlock->getPosition() );	

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
		// Ignore forced breaks and EOP when finding EOL.
		fp_Run* pLastRun = pLine->getLastRun();
		while (!pLastRun->isFirstRunOnLine()
			   && (pLastRun->isForcedBreak()
				   || (FPRUN_ENDOFPARAGRAPH == pLastRun->getType())))
		{
			pLastRun = pLastRun->getPrev();
		}

		if (pLastRun->isForcedBreak()
			|| (FPRUN_ENDOFPARAGRAPH == pLastRun->getType()))
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
		bool bRes = getEditableBounds(true, iPos);
		UT_ASSERT(bRes);
	}
	break;

	case FV_DOCPOS_BOB:
	{
#if 1

// DOM: This used to be an #if 0. I changed it to #if 1
// DOM: because after enabling this code, I can no
// DOM: longer reproduce bug 403 (the bug caused by this
// DOM: code being if 0'd) or bug 92 (the bug that if 0'ing
// DOM: this code supposedly fixes)

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
			bool bRes = getEditableBounds(true, iPos);
			UT_ASSERT(bRes);
		}
	}
	break;

	case FV_DOCPOS_BOW:
	{
		UT_GrowBuf pgb(1024);

		bool bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);

		const UT_UCSChar* pSpan = (UT_UCSChar*)pgb.getPointer(0);

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

			pSpan = (UT_UCSChar*)pgb.getPointer(0);
			offset = pgb.getLength();

			if (offset == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		UT_uint32 iUseOffset = bKeepLooking ? offset-1 : offset;

		bool bInWord = !UT_isWordDelimiter(pSpan[iUseOffset], UCS_UNKPUNK, iUseOffset > 0 ? pSpan[iUseOffset - 1] : UCS_UNKPUNK);

		for (offset--; offset > 0; offset--)
		{
			if (UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK,pSpan[offset-1]))
			{
				if (bInWord)
					break;
			}
			else
				bInWord = true;
		}

		if ((offset > 0) && (offset < pgb.getLength()))
			offset++;

		iPos = offset + pBlock->getPosition();
	}
	break;

	case FV_DOCPOS_EOW_MOVE:
	{
		UT_GrowBuf pgb(1024);

		bool bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);

		const UT_UCSChar* pSpan = (UT_UCSChar*)pgb.getPointer(0);

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

			pSpan = (UT_UCSChar*)pgb.getPointer(0);
			offset = 0;

			if (pgb.getLength() == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK, offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK);

		// Needed so ctrl-right arrow will work
		// This is the code that was causing bug 10
		// There is still some weird behavior that should be investigated

		for (; offset < pgb.getLength(); offset++)
		{
			UT_UCSChar followChar, prevChar;

			followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
			prevChar = offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK;

			if (!UT_isWordDelimiter(pSpan[offset], followChar, prevChar))
				break;
		}

		for (; offset < pgb.getLength(); offset++)
		{
			UT_UCSChar followChar, prevChar;

			followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
			prevChar = offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK;

			if (!UT_isWordDelimiter(pSpan[offset], followChar, prevChar))
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
				bBetween = true;
			}
		}

		iPos = offset + pBlock->getPosition();
	}
	break;

	case FV_DOCPOS_EOW_SELECT:
	{
		UT_GrowBuf pgb(1024);

		bool bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);

		const UT_UCSChar* pSpan = (UT_UCSChar*)pgb.getPointer(0);

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

			pSpan = (UT_UCSChar*)pgb.getPointer(0);
			offset = 0;

			if (pgb.getLength() == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		bool bBetween = UT_isWordDelimiter(pSpan[offset], UCS_UNKPUNK, offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK);

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
			UT_UCSChar followChar, prevChar;

			followChar = ((offset + 1) < pgb.getLength()) ? pSpan[offset+1] : UCS_UNKPUNK;
			prevChar = offset > 0 ? pSpan[offset - 1] : UCS_UNKPUNK;

			if (UT_isWordDelimiter(pSpan[offset], followChar, prevChar))
			{
				if (bBetween)
					break;
			}
			else if (pSpan[offset] == ' ')
				break;
			else
				bBetween = true;
		}

		iPos = offset + pBlock->getPosition();
	}
	break;


	case FV_DOCPOS_BOP:
	{
		fp_Container* pContainer = pLine->getColumn();
		fp_Page* pPage = pContainer->getPage();

		iPos = pPage->getFirstLastPos(true);
	}
	break;

	case FV_DOCPOS_EOP:
	{
		fp_Container* pContainer = pLine->getColumn();
		fp_Page* pPage = pContainer->getPage();

		iPos = pPage->getFirstLastPos(false);
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


/*!
  Find block at document position. This version is looks outside the
  header region if we get a null block.
  \param pos Document position
  \return Block at specified posistion, or the first block to the
		  rigth of that position. May return NULL.
  \see m_pLayout->findBlockAtPosition
*/
fl_BlockLayout* FV_View::_findBlockAtPosition(PT_DocPosition pos) const
{
	fl_BlockLayout * pBL=NULL;
	if(m_bEditHdrFtr && m_pEditShadow != NULL)
	{
		pBL = (fl_BlockLayout *) m_pEditShadow->findBlockAtPosition(pos);
		if(pBL != NULL)
			return pBL;
	}
	pBL = m_pLayout->findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	if(!pBL)
		return NULL;

//
// Sevior should remove this after a while..
//
#if(1)
	if(pBL->isHdrFtr())
	{
//		  fl_HdrFtrSectionLayout * pSSL = (fl_HdrFtrSectionLayout *) pBL->getSectionLayout();
//		  pBL = pSSL->getFirstShadow()->findMatchingBlock(pBL);
		  UT_DEBUGMSG(("<<<<SEVIOR>>>: getfirstshadow in view \n"));
		  UT_ASSERT(0);
	}
#endif
	return pBL;
}


void FV_View::_insertSectionBreak(void)
{
	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
	//
	// Get preview DocSectionLayout so we know what header/footers we have
		// to insert here.
	//
	fl_DocSectionLayout * pPrevDSL = (fl_DocSectionLayout *) getCurrentBlock()->getSectionLayout();

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	// before inserting a section break, we insert a block break
	UT_uint32 iPoint = getPoint();

	m_pDoc->insertStrux(iPoint, PTX_Block);
	m_pDoc->insertStrux(iPoint, PTX_Section);

	_generalUpdate();
	_ensureInsertionPointOnScreen();
	UT_uint32 oldPoint = getPoint();
	fl_DocSectionLayout * pCurDSL = (fl_DocSectionLayout *) getCurrentBlock()->getSectionLayout();
	//
	// Duplicate previous header/footers for this section.
	//
	UT_Vector vecPrevHdrFtr;
	pPrevDSL->getVecOfHdrFtrs( &vecPrevHdrFtr);
	UT_uint32 i =0;
	const XML_Char* block_props[] = {
		"text-align", "left",
		NULL, NULL
	};
	HdrFtrType hfType;
	fl_HdrFtrSectionLayout * pHdrFtrSrc = NULL;
	fl_HdrFtrSectionLayout * pHdrFtrDest = NULL;
	for(i=0; i< vecPrevHdrFtr.getItemCount(); i++)
	{
		  pHdrFtrSrc = (fl_HdrFtrSectionLayout *) vecPrevHdrFtr.getNthItem(i);
		  hfType = pHdrFtrSrc->getHFType();
		  insertHeaderFooter(block_props, hfType, pCurDSL); // cursor is now in the header/footer
		  if(hfType == FL_HDRFTR_HEADER)
		  {
			  pHdrFtrDest = pCurDSL->getHeader();
		  }
		  else if(hfType == FL_HDRFTR_FOOTER)
		  {
			  pHdrFtrDest = pCurDSL->getFooter();
		  }
		  else if(hfType == FL_HDRFTR_HEADER_FIRST)
		  {
			  pHdrFtrDest = pCurDSL->getHeaderFirst();
		  }
		  else if( hfType == FL_HDRFTR_HEADER_EVEN)
		  {
			  pHdrFtrDest = pCurDSL->getHeaderEven();
		  }
		  else if( hfType == FL_HDRFTR_HEADER_LAST)
		  {
			  pHdrFtrDest = pCurDSL->getHeaderLast();
		  }
		  else if(hfType == FL_HDRFTR_FOOTER_FIRST)
		  {
			  pHdrFtrDest = pCurDSL->getFooterFirst();
		  }
		  else if( hfType == FL_HDRFTR_FOOTER_EVEN)
		  {
			  pHdrFtrDest = pCurDSL->getFooterEven();
		  }
		  else if( hfType == FL_HDRFTR_FOOTER_LAST)
		  {
			  pHdrFtrDest = pCurDSL->getFooterLast();
		  }
		  _populateThisHdrFtr(pHdrFtrSrc,pHdrFtrDest);
	}

	_setPoint(oldPoint);
	_generalUpdate();

	_ensureInsertionPointOnScreen();
}


/*!
  Move insertion point to previous or next line
  \param bNext True if moving to next line

  This function moves the IP up or down one line, attempting to get as
  close as possible to the prior "sticky" x position.  The notion of
  "next" is strictly physical, not logical.

  For example, instead of always moving from the last line of one
  block to the first line of the next, you might wind up skipping over
  a bunch of blocks to wind up in the first line of the second column.
*/
void FV_View::_moveInsPtNextPrevLine(bool bNext)
{
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight, iLineHeight;
	bool bDirection;

//
// No need to to do background updates for 1 seconds.
//
 	m_pLayout->setSkipUpdates(2);
	UT_sint32 xOldSticky = m_xPointSticky;

	// first, find the line we are on now
	PT_DocPosition iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock;
	fp_Run* pOldRun;
	_findPositionCoords(iOldPoint, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pOldBlock, &pOldRun);
	if(pOldRun == NULL)
	{
		PT_DocPosition posEOD;
		getEditableBounds(true,posEOD);
		if(posEOD <= iOldPoint)
		{
			setPoint(posEOD);
			return;
		}
	}
	UT_return_if_fail(pOldRun);

	fl_SectionLayout* pOldSL = pOldBlock->getSectionLayout();
	fp_Line* pOldLine = pOldRun->getLine();
	fp_VerticalContainer* pOldContainer = (fp_VerticalContainer *) pOldLine->getContainer();
	fp_Column * pOldColumn = NULL;
	fp_Column * pOldLeader = NULL;
	fp_Page* pOldPage = pOldContainer->getPage();
	bool bDocSection = (pOldSL->getType() == FL_SECTION_DOC) ||
		(pOldSL->getType() == FL_SECTION_ENDNOTE);
	bool bCellSection = (pOldSL->getContainerType() == FL_CONTAINER_CELL);



	if (bDocSection)
	{
		pOldLeader = ((fp_Column*) (pOldContainer))->getLeader();
	}
	if(bDocSection)
	{
		pOldColumn = (fp_Column *) pOldContainer;
	}
	if(bCellSection)
	{
		pOldColumn = (fp_Column *) pOldContainer->getColumn();
	}
	UT_sint32 iPageOffset;
	getPageYOffset(pOldPage, iPageOffset);

	UT_sint32 iLineX = 0;
	UT_sint32 iLineY = 0;

	pOldContainer->getOffsets((fp_Container *) pOldLine, iLineX, iLineY);
	yPoint = iLineY;

	iLineHeight = pOldLine->getHeight();

	bool bNOOP = false;

	xxx_UT_DEBUGMSG(("fv_View::_moveInsPtNextPrevLine: old line 0x%x\n", pOldLine));

	if (bNext)
	{
		if (pOldLine != (fp_Line *) pOldContainer->getLastContainer())
		{
			UT_sint32 iAfter = 1;
			yPoint += (iLineHeight + iAfter);
		}
		else if (bDocSection)
		{
			UT_sint32 count = (UT_sint32) pOldPage->countColumnLeaders();
			UT_sint32 i = 0;
			for(i =0; i < count ;i++)
			{
				if( (fp_Column *) pOldPage->getNthColumnLeader(i) == pOldLeader)
				{
					break;
				}
			}
			if((i + 1) < count)
			{
				// Move to next container
				yPoint = pOldPage->getNthColumnLeader(i+1)->getY();
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
					bNOOP = true;
				}
			}
		}
		else if(bCellSection)
		{
			UT_sint32 iAfter = 1;
			yPoint += (iLineHeight + iAfter);
		}
		else
		{
			bNOOP = true;
		}
	}
	else
	{
		if (pOldLine != (fp_Line *) pOldContainer->getFirstContainer())
		{
			// just move off this line
			yPoint -= (pOldLine->getMarginBefore() + 1);
		}
		else if (bDocSection)
		{
			UT_sint32 count = (UT_sint32) pOldPage->countColumnLeaders();
			UT_sint32 i = 0;
			for(i =0; i < count ;i++)
			{
				if( (fp_Column *) pOldPage->getNthColumnLeader(i) == pOldLeader)
				{
					break;
				}
			}
			if( (i> 0) && (i < count))
			{
				// Move to prev container
				yPoint = pOldPage->getNthColumnLeader(i-1)->getLastContainer()->getY();
				yPoint +=  pOldPage->getNthColumnLeader(i-1)->getY()+2;
			}
			else
			{
				// move to prev page
				fp_Page* pPage = pOldPage->getPrev();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = pPage->getBottom();
					if(getViewMode() != VIEW_PRINT)
					{
						fl_DocSectionLayout * pDSL = pPage->getOwningSection();
						yPoint = yPoint - pDSL->getTopMargin() -2;
					}
				}
				else
				{
					bNOOP = true;
				}
			}
		}
		else if(bCellSection)
		{
			UT_sint32 iAfter = 2;
			yPoint -= iAfter;
		}
		else
		{
			bNOOP = true;
		}
	}

	if (bNOOP)
	{
		// cannot move.  should we beep?
		return;
	}

	// change to screen coordinates
	xPoint = m_xPointSticky - m_xScrollOffset + getPageViewLeftMargin();
	yPoint += iPageOffset - m_yScrollOffset;

	// hit-test to figure out where that puts us
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPoint, yPoint, xClick, yClick);

	PT_DocPosition iNewPoint;
	bool bBOL = false;
	bool bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
//
// If we're not in a Header/Footer we can't get off the page with the click
// version of mapXYToPosition
//
	if(isHdrFtrEdit())
	{
		pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL, true, &pShadow);
	}
	else
	{
		pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);
		while(pPage && (iNewPoint == iOldPoint) && (yClick < m_pLayout->getHeight()) && (yClick > 0))
		{
			if (bNext)
			{
				yClick += iLineHeight/2;
			}
			else
			{
				yClick -= 2;
			}
			if(yClick > pPage->getHeight())
			{
				pPage = pPage->getNext();
				yClick -= pPage->getHeight();
			}
			if(yClick < 0)
			{
				pPage = pPage->getPrev();
				yClick += pPage->getHeight();
			}
			if(pPage)
			{
				pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL);
			}
		}
	}
//
// Check we're not moving out of allowed region.
//
	PT_DocPosition posBOD,posEOD;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);

	xxx_UT_DEBUGMSG(("iNewPoint=%d, iOldPoint=%d, xClick=%d, yClick=%d\n",iNewPoint, iOldPoint, xClick, yClick));
	UT_ASSERT(iNewPoint != iOldPoint);
	if((iNewPoint >= posBOD) && (iNewPoint <= posEOD) &&
	   ((bNext && (iNewPoint >= iOldPoint))
		|| (!bNext && (iNewPoint <= iOldPoint))))
	{
		_setPoint(iNewPoint, bEOL);
	}

	_ensureInsertionPointOnScreen();

	// this is the only place where we override changes to m_xPointSticky
	m_xPointSticky = xOldSticky;
}

/*! Scrolls the screen to make sure that the IP is on-screen.
 * \return true iff scrolling took place
 * Q: should this get called if there is a selection?  Does it do
 * harm?  It may, because point may be the beginning of the selection.
 */
bool FV_View::_ensureInsertionPointOnScreen()
{
	xxx_UT_DEBUGMSG(("FV_View::_ensureInsertionPointOnScreen called\n"));

	// Some short circuit tests to avoid doing bad things.
	if (m_iWindowHeight <= 0)
		return false;

    // If == 0 no layout information is present. Don't scroll.
	if(m_iPointHeight == 0)
		return false;

	xxx_UT_DEBUGMSG(("_ensure: [xp %ld][yp %ld][ph %ld] [w %ld][h %ld]\n",m_xPoint,m_yPoint,m_iPointHeight,m_iWindowWidth,m_iWindowHeight));

	bool bRet = false;
	if (m_yPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(m_yPoint)));
		bRet = true;
	}
	else if (((UT_uint32) (m_yPoint + m_iPointHeight)) >= ((UT_uint32) m_iWindowHeight))
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(m_yPoint + m_iPointHeight - m_iWindowHeight));
		bRet = true;
	}

	/*
	  TODO: we really ought to try to do better than this.
	*/
	if (m_xPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(m_xPoint) + getPageViewLeftMargin()/2));
		bRet = true;
	}
	else if (((UT_uint32) (m_xPoint)) >= ((UT_uint32) m_iWindowWidth))
	{
		cmdScroll(AV_SCROLLCMD_LINERIGHT, (UT_uint32)(m_xPoint - m_iWindowWidth + getPageViewLeftMargin()/2));
		bRet = true;
	}

	// This used to say 'if !bRet', but I think it's cleaner to always fix,
	// if possibly slower.  If we scroll, perhaps the scroll has already
	// fixed, and we end up fixing twice.
	_fixInsertionPointCoords();

	return bRet;
}

/* It's unclear to me why this is used rather than ensure. -PL */
void FV_View::_updateInsertionPoint()
{
	if (isSelectionEmpty())
 	{
		_ensureInsertionPointOnScreen();
	}
}

void FV_View::_moveInsPtNextPrevPage(bool bNext)
{
#if 0
	UT_sint32 xPoint;
	UT_sint32 yPoint;
	UT_sint32 iPointHeight;
#endif

	fp_Page* pOldPage = _getCurrentPage();

	// TODO when moving to the prev page, we should move to its end, not begining
	// try to locate next/prev page
	fp_Page* pPage = (bNext ? pOldPage->getNext() : pOldPage->getPrev());

	// if couldn't move, go to top of this page if we are looking for the previous page
	// or the end of this page if we are looking for the next page
	if (!pPage)
	{
		if(!bNext)
		{
			pPage = pOldPage;
		}
		else
		{
			moveInsPtTo(FV_DOCPOS_EOD,false);
			return;
		}
	}

	_moveInsPtToPage(pPage);
}

void FV_View::_moveInsPtNextPrevScreen(bool bNext)
{
	fl_BlockLayout * pBlock;
	fp_Run * pRun;
	UT_sint32 x,y,x2,y2;
	UT_uint32 iHeight;
	bool bDirection;
	bool bBOL,bEOL;
	UT_sint32 iYnext,iYscroll;
	PT_DocPosition iNewPoint;
	_findPositionCoords(getPoint(),false,x,y,x2,y2,iHeight,bDirection,&pBlock,&pRun);
	if(!pRun)
		return;

	fp_Line * pLine = pRun->getLine();
	UT_return_if_fail(pLine);
	fp_Page * pPage = pLine->getPage();
	UT_return_if_fail(pPage);
	if(isHdrFtrEdit())
	{
		clearHdrFtrEdit();
		warpInsPtToXY(0,0,false);
	}
	UT_sint32 xoff,yoff;
//
// get Screen coordinates of the top of the page and add the y location to this.
//
	getPageScreenOffsets(pPage, xoff,yoff);
	yoff = y - yoff;
	bool bSuccess = true;
	if(bNext)
	{
		iYnext = yoff + m_iWindowHeight;
		iYscroll = m_yScrollOffset + m_iWindowHeight;
		xxx_UT_DEBUGMSG(("SEVIOR:!!!!!! Yoff %d iYnext %d page %x \n",yoff,iYnext,pPage));
		while(pPage && (iYnext > pPage->getHeight()))
		{
			iYnext -= pPage->getHeight();
			iYnext -= getPageViewSep();
			pPage = pPage->getNext();
		}
		xxx_UT_DEBUGMSG(("SEVIOR:!!!!!! Set to iYnext %d page %x \n",iYnext,pPage));
		if(pPage == NULL)
		{
			return;
		}
		if(iYnext < 0)
		{
			iYnext = 0;
		}
		UT_sint32 newX,newY;
		UT_uint32 newHeight;
		pPage->mapXYToPosition(x, iYnext, iNewPoint, bBOL, bEOL);
		_findPositionCoords(iNewPoint,false,newX,newY,x2,y2,newHeight,bDirection,&pBlock,&pRun);
		if(!pRun)
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
		fp_Line * pNewLine = (fp_Line *) pRun->getLine();
		if(pNewLine == NULL)
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
		if((pNewLine->getPage() == pLine->getPage()) && (pNewLine->getY() < pLine->getY()))
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}

//
// Couldn't advance! Try scanning x across the page at this new iYnext.
//
		if(pLine == pNewLine)
		{
			bSuccess = false;
			UT_sint32 width = pPage->getWidth();
			UT_sint32 step = width/20 + 1;
			for(x=0; (x < width) && !bSuccess; x += step)
			{
				pPage->mapXYToPosition(x, iYnext, iNewPoint, bBOL, bEOL);
				_findPositionCoords(iNewPoint,false,newX,newY,x2,y2,newHeight,bDirection,&pBlock,&pRun);
				pNewLine = (fp_Line *) pRun->getLine();
				if(pLine != pNewLine)
				{
					bSuccess = true;
				}
			}
		}
		if(!bSuccess)
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
	}
	else
	{
		iYnext = yoff  - m_iWindowHeight;
		iYscroll = m_yScrollOffset - m_iWindowHeight;
		if(iYscroll < 0)
		{
			return;
		}
		while(pPage && (iYnext < 0))
		{
			iYnext += pPage->getHeight();
			iYnext += getPageViewSep();
			pPage = pPage->getPrev();
		}
		if(pPage == NULL)
		{
			return;
		}
		pPage->mapXYToPosition(x, iYnext, iNewPoint, bBOL, bEOL);
		UT_sint32 newX,newY;
		UT_uint32 newHeight;
		_findPositionCoords(iNewPoint,false,newX,newY,x2,y2,newHeight,bDirection,&pBlock,&pRun);
		if(!pRun)
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
		fp_Line * pNewLine = (fp_Line *) pRun->getLine();
		if(pNewLine == NULL)
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
		if((pNewLine->getPage() == pLine->getPage()) && (pNewLine->getY() > pLine->getY()))
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
//
// Couldn't advance! Try scanning x across the page at this new iYnext.
//
		if(pNewLine == pLine)
		{
			bSuccess = false;
			UT_sint32 width = pPage->getWidth();
			UT_sint32 step = width/20 + 1;
			for(x=0; (x < width) && !bSuccess; x += step)
			{
				pPage->mapXYToPosition(x, iYnext, iNewPoint, bBOL, bEOL);
				_findPositionCoords(iNewPoint,false,newX,newY,x2,y2,newHeight,bDirection,&pBlock,&pRun);
				fp_Line * pNewLine = (fp_Line *) pRun->getLine();
				if(pLine != pNewLine)
				{
					bSuccess = true;
				}
			}
		}
		if(!bSuccess)
		{
			_moveInsPtNextPrevLine(bNext);
			return;
		}
	}
	_setPoint(iNewPoint);
	sendVerticalScrollEvent(iYscroll);
	if (!_ensureInsertionPointOnScreen())
	{
		_fixInsertionPointCoords();
	}
}


fp_Page *FV_View::_getCurrentPage(void)
{
	UT_sint32 xPoint, yPoint, xPoint2, yPoint2;
	UT_uint32 iPointHeight;
	bool bDirection;
	/*
	  This function moves the IP to the beginning of the previous or
	  next page (ie not this one).
	*/

	// first, find the page we are on now
	UT_uint32 iOldPoint = getPoint();

	fl_BlockLayout* pOldBlock;
	fp_Run* pOldRun;
	_findPositionCoords(iOldPoint, m_bPointEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection, &pOldBlock, &pOldRun);
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
	PT_DocPosition iNewPoint = page->getFirstLastPos(true);
	_setPoint(iNewPoint, false);

	// explicit vertical scroll to top of page
	UT_sint32 iPageOffset;
	getPageYOffset(page, iPageOffset);

	iPageOffset -= getPageViewSep() /2;
	iPageOffset -= m_yScrollOffset;

	bool bVScroll = false;
	if (iPageOffset < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-iPageOffset));
		bVScroll = true;
	}
	else if (iPageOffset > 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, (UT_uint32)(iPageOffset));
		bVScroll = true;
	}

	// also allow implicit horizontal scroll, if needed
	if (!_ensureInsertionPointOnScreen() && !bVScroll)
	{
		_fixInsertionPointCoords();
	}
}

void FV_View::_autoScroll(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);

	// this is a static callback method and does not have a 'this' pointer.

	FV_View * pView = (FV_View *) pWorker->getInstanceData();
	UT_ASSERT(pView);

	if(pView->getLayout()->getDocument()->isPieceTableChanging())
	{
		return;
	}

	PT_DocPosition iOldPoint = pView->getPoint();

	/*
	  NOTE: We update the selection here, so that the timer can keep
	  triggering autoscrolls even if the mouse doesn't move.
	*/
	pView->extSelToXY(pView->m_xLastMouse, pView->m_yLastMouse, false);

	if (pView->getPoint() != iOldPoint)
	{
		// do the autoscroll
		pView->_ensureInsertionPointOnScreen();
	}
	else
	{
		// not far enough to change the selection ... do we still need to scroll?
		UT_sint32 xPos = pView->m_xLastMouse;
		UT_sint32 yPos = pView->m_yLastMouse;

		// TODO: clamp xPos, yPos to viewable area??

		bool bOnScreen = true;

		if ((xPos < 0 || xPos > pView->m_iWindowWidth) ||
			(yPos < 0 || yPos > pView->m_iWindowHeight))
			bOnScreen = false;

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
			//
			// Sevior: Is This what you wanted? Uncomment these lines when
			// needed.
			//
			//XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
			//UT_ASSERT((pFrame));

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
	xClick = xPos + m_xScrollOffset - getPageViewLeftMargin();
	yClick = yPos + m_yScrollOffset - getPageViewTopMargin();
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageHeight = pPage->getHeight();
		if(getViewMode() != VIEW_PRINT)
		{
			iPageHeight = iPageHeight - pPage->getOwningSection()->getTopMargin() -
				pPage->getOwningSection()->getBottomMargin();
		}
		if (yClick < iPageHeight)
		{
			// found it
			break;
		}
		else
		{
			yClick -= iPageHeight + getPageViewSep();
		}
		pPage = pPage->getNext();
	}

	if (!pPage)
	{
		// we're below the last page
		pPage = m_pLayout->getLastPage();

		UT_sint32 iPageHeight = pPage->getHeight();
		yClick += iPageHeight + getPageViewSep();
	}

	return pPage;
}

/*!
 Compute prefix function for search
 \param pFind String to find
 \param bMatchCase True to match case, false to ignore case
*/
UT_uint32*
FV_View::_computeFindPrefix(const UT_UCSChar* pFind, bool bMatchCase)
{
	UT_uint32 m = UT_UCS4_strlen(pFind);
	UT_uint32 k = 0, q = 1;
	UT_uint32 *pPrefix = (UT_uint32*) UT_calloc(m, sizeof(UT_uint32));
	UT_ASSERT(pPrefix);

	pPrefix[0] = 0; // Must be this regardless of the string

	if (bMatchCase)
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0 && pFind[k] != pFind[q])
				k = pPrefix[k - 1];
			if(pFind[k] == pFind[q])
				k++;
			pPrefix[q] = k;
		}
	}
	else
	{
		for (q = 1; q < m; q++)
		{
			while (k > 0
				   && UT_UCS4_tolower(pFind[k]) != UT_UCS4_tolower(pFind[q]))
				k = pPrefix[k - 1];
			if(UT_UCS4_tolower(pFind[k]) == UT_UCS4_tolower(pFind[q]))
				k++;
			pPrefix[q] = k;
		}
	}

	return pPrefix;
}

/*!
 Find next occurrence of string
 \param pFind String to find
 \param True to match case, false to ignore case
 \result bDoneEntireDocument True if entire document searched,
		 false otherwise
 \return True if string was found, false otherwise

 \fixme The conversion of UCS_RQUOTE should happen in some generic
		function - it is presently done lot's of places in the code.
*/
bool
FV_View::_findNext(const UT_UCSChar* pFind, UT_uint32* pPrefix,
				   bool bMatchCase, bool& bDoneEntireDocument)
{
	UT_ASSERT(pFind);

	fl_BlockLayout* block = _findGetCurrentBlock();
	PT_DocPosition offset = _findGetCurrentOffset();
	UT_UCSChar* buffer = NULL;
	UT_uint32 m = UT_UCS4_strlen(pFind);

	// Clone the search string, converting it to lowercase is search
	// should ignore case.
	UT_UCSChar* pFindStr = (UT_UCSChar*) UT_calloc(m, sizeof(UT_UCSChar));
	UT_ASSERT(pFindStr);
	if (!pFindStr)
		return false;
	UT_uint32 j;
	if (bMatchCase)
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = pFind[j];
	}
	else
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = UT_UCS4_tolower(pFind[j]);
	}

	// Now we use the prefix function (stored as an array) to search
	// through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		UT_sint32 foundAt = -1;
		UT_uint32 i = 0, t = 0;

		if (bMatchCase)
		{
			UT_UCSChar currentChar;

			while ((currentChar = buffer[i]) /*|| foundAt == -1*/)
			{
				// Convert smart quote apostrophe to ASCII single quote to
				// match seach input
				if (currentChar == UCS_RQUOTE) currentChar = '\'';

				while (t > 0 && pFindStr[t] != currentChar)
					t = pPrefix[t-1];
				if (pFindStr[t] == currentChar)
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}
		else
		{
			UT_UCSChar currentChar;

			while ((currentChar = buffer[i]) /*|| foundAt == -1*/)
			{
				// Convert smart quote apostrophe to ASCII single quote to
				// match seach input
				if (currentChar == UCS_RQUOTE) currentChar = '\'';

				currentChar = UT_UCS4_tolower(currentChar);

				while (t > 0 && pFindStr[t] != currentChar)
					t = pPrefix[t-1];
				if (pFindStr[t] == currentChar)
					t++;
				i++;
				if (t == m)
				{
					foundAt = i - m;
					break;
				}
			}
		}


		// Select region of matching string if found
		if (foundAt != -1)
		{
			_setPoint(block->getPosition(false) + offset + foundAt);
			_setSelectionAnchor();
			_charMotion(true, m);

			m_doneFind = true;

			FREEP(pFindStr);
			FREEP(buffer);
			return true;
		}

		// Didn't find anything, so set the offset to the end of the
		// current area
		offset += UT_UCS4_strlen(buffer);

		// Must clean up buffer returned for search
		FREEP(buffer);
	}

	bDoneEntireDocument = true;

	// Reset wrap for next time
	m_wrappedEnd = false;

	FREEP(pFindStr);

	return false;
}


PT_DocPosition
FV_View::_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset)
{
	UT_ASSERT(block);
	return block->getPosition(false) + offset;
}

UT_UCSChar*
FV_View::_findGetNextBlockBuffer(fl_BlockLayout** pBlock,
								 PT_DocPosition* pOffset)
{
	UT_ASSERT(m_pLayout);

	// This assert doesn't work, since the startPosition CAN
	// legitimately be zero
	// The beginning of the first block in any document
	UT_ASSERT(m_startPosition >= 2);

	UT_ASSERT(pBlock);
	UT_ASSERT(*pBlock);

	UT_ASSERT(pOffset);

	fl_BlockLayout* newBlock = NULL;
	PT_DocPosition newOffset = 0;

	UT_uint32 bufferLength = 0;

	UT_GrowBuf pBuffer;

	// Check early for completion, from where we left off last, and
	// bail if we are now at or past the start position
	if (m_wrappedEnd
		&& _BlockOffsetToPos(*pBlock, *pOffset) >= m_startPosition)
	{
		// We're done
		return NULL;
	}

	if (!(*pBlock)->getBlockBuf(&pBuffer))
	{
		UT_DEBUGMSG(("Block %p has no associated buffer.\n", *pBlock));
		UT_ASSERT(0);
	}

	// Have we already searched all the text in this buffer?
	if (*pOffset >= pBuffer.getLength())
	{
		// Then return a fresh new block's buffer
		newBlock = (*pBlock)->getNextBlockInDocument();

		// Are we at the end of the document?
		if (!newBlock)
		{
			// Then wrap (fetch the first block in the doc)
			PT_DocPosition startOfDoc;
			getEditableBounds(false, startOfDoc);

			newBlock = m_pLayout->findBlockAtPosition(startOfDoc);

			m_wrappedEnd = true;

			UT_ASSERT(newBlock);
		}

		// Re-assign the buffer contents for our new block
		pBuffer.truncate(0);
		// The offset starts at 0 for a fresh buffer
		newOffset = 0;

		if (!newBlock->getBlockBuf(&pBuffer))
		{
			UT_DEBUGMSG(("Block %p (a ->next block) has no buffer.\n",
						 newBlock));
			UT_ASSERT(0);
		}

		// Good to go with a full buffer for our new block
	}
	else
	{
		// We have some left to go in this buffer.	Buffer is still
		// valid, just copy pointers
		newBlock = *pBlock;
		newOffset = *pOffset;
	}

	// Are we going to run into the start position in this buffer?	If
	// so, we need to size our length accordingly
	if (m_wrappedEnd && _BlockOffsetToPos(newBlock, newOffset) + pBuffer.getLength() >= m_startPosition)
	{
		bufferLength = (m_startPosition - (newBlock)->getPosition(false)) - newOffset;
	}
	else
	{
		bufferLength = pBuffer.getLength() - newOffset;
	}

	// clone a buffer (this could get really slow on large buffers!)
	UT_UCSChar* bufferSegment = NULL;

	// remember, the caller gets to free this memory
	bufferSegment = (UT_UCSChar*)UT_calloc(bufferLength + 1, sizeof(UT_UCSChar));
	UT_ASSERT(bufferSegment);

	memmove(bufferSegment, pBuffer.getPointer(newOffset),
			(bufferLength) * sizeof(UT_UCSChar));

	// before we bail, hold up our block stuff for next round
	*pBlock = newBlock;
	*pOffset = newOffset;

	return bufferSegment;
}

/*!
 Find and replace text unit
 \param pFind String to find
 \param pReplace String to replace it with
 \param pPrefix Search prefix function
 \param bMatchCase True to match case, false to ignore case
 \result bDoneEntireDocument True if entire document searched,
		 false otherwise
 \return True if string was replaced, false otherwise

 This function will replace an existing selection with pReplace. It
 will then do a search for pFind.
*/
bool
FV_View::_findReplace(const UT_UCSChar* pFind, const UT_UCSChar* pReplace,
					  UT_uint32* pPrefix, bool bMatchCase,
					  bool& bDoneEntireDocument)
{
	UT_ASSERT(pFind && pReplace);

	bool bRes = false;

	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();

	// Replace selection if it's due to a find operation
	if (m_doneFind && !isSelectionEmpty())
	{
		bRes = true;

		PP_AttrProp AttrProp_Before;

		if (!isSelectionEmpty())
		{
			_deleteSelection(&AttrProp_Before);
		}

		// If we have a string with length, do an insert, else let it
		// hang from the delete above
		if (*pReplace)
			bRes = m_pDoc->insertSpan(getPoint(), pReplace,
									  UT_UCS4_strlen(pReplace),
									  &AttrProp_Before);

		// Do not increase the insertion point index, since the insert
		// span will leave us at the correct place.

		_generalUpdate();

		// If we've wrapped around once, and we're doing work before
		// we've hit the point at which we started, then we adjust the
		// start position so that we stop at the right spot.
		if (m_wrappedEnd && !bDoneEntireDocument)
		{
			m_startPosition += (long) UT_UCS4_strlen(pReplace);
			m_startPosition -= (long) UT_UCS4_strlen(pFind);
		}

		UT_ASSERT(m_startPosition >= 2);
	}

	m_pDoc->endUserAtomicGlob();
	_restorePieceTableState();

	// Find next occurrence in document
	_findNext(pFind, pPrefix, bMatchCase, bDoneEntireDocument);
	return bRes;
}

fl_BlockLayout*
FV_View::_findGetCurrentBlock(void)
{
	return _findBlockAtPosition(m_iInsPoint);
}

PT_DocPosition
FV_View::_findGetCurrentOffset(void)
{
	return (m_iInsPoint - _findGetCurrentBlock()->getPosition(false));
}

// Any takers?
UT_sint32
FV_View::_findBlockSearchRegexp(const UT_UCSChar* /* haystack */,
								const UT_UCSChar* /* needle */)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return -1;
}

/*
  After most editing commands, it is necessary to call this method,
  _generalUpdate, in order to fix everything.
*/
void FV_View::_generalUpdate(void)
{
	if(!shouldScreenUpdateOnGeneralUpdate())
		return;
	m_pDoc->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);

//
// No need to update other stuff if we're doing a preview
//
	if(isPreview())
		return;
	/*
	  TODO note that we are far too heavy handed with the mask we
	  send here.  I ripped out all the individual calls to notifyListeners
	  which appeared within fl_BlockLayout, and they all now go through
	  here.  For that reason, I made the following mask into the union
	  of all the masks I found.  I assume that this is inefficient, but
	  functionally correct.

	  TODO WRONG! WRONG! WRONG! notifyListener() must be called in
	  TODO WRONG! WRONG! WRONG! fl_BlockLayout in response to a change
	  TODO WRONG! WRONG! WRONG! notification and not here.	this call
	  TODO WRONG! WRONG! WRONG! will only update the current window.
	  TODO WRONG! WRONG! WRONG! having the notification in fl_BlockLayout
	  TODO WRONG! WRONG! WRONG! will get each view on the document.
	*/
//
//	notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK );
	notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK | AV_CHG_PAGECOUNT | AV_CHG_FMTSTYLE );
}


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
	bool bres;
	UT_uint32 iNewPoint = getPoint();

	PT_DocPosition posBOD,posEOD,dNewPoint,dOldPoint;
	dNewPoint = (PT_DocPosition) iNewPoint;
	dOldPoint = (PT_DocPosition) iOldPoint;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);
	if(dNewPoint < posBOD || dNewPoint > posEOD || dOldPoint < posBOD
	   || dNewPoint > posEOD)
	{
		return;
	}
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
				bres = _clearBetweenPositions(m_iSelectionAnchor, iOldPoint, true);
				if(bres)
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

			bres = _clearBetweenPositions(iNewPoint, iOldPoint, true);
			if(bres)
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

			bres =_clearBetweenPositions(iOldPoint, iNewPoint, true);
			if(bres)
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

				bres = _clearBetweenPositions(iOldPoint, m_iSelectionAnchor, true);
				if(bres)
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

	PT_DocPosition posBOD,posEOD,dNewPoint,dOldPoint;
	dNewPoint = (PT_DocPosition) iNewPoint;
	dOldPoint = (PT_DocPosition) iOldPoint;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);
	if(dNewPoint < posBOD || dNewPoint > posEOD || dOldPoint < posBOD
	   || dNewPoint > posEOD)
	{
		return;
	}

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
	}

	notifyListeners(AV_CHG_MOTION);
}


/*
  This method simply iterates over every run between two doc positions
  and draws each one.
*/
void FV_View::_drawBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2)
{
	UT_ASSERT(iPos1 < iPos2);
//	CHECK_WINDOW_SIZE

	fp_Run* pRun1;
	fp_Run* pRun2;
	UT_sint32 xoff;
	UT_sint32 yoff;
	UT_uint32 uheight;
//
// This fixes a bug from insert file, when the view we copy from is selected
// If don't bail out now we get all kinds of crazy dirty on the screen.
//
	if(m_pParentData == NULL)
	{
		return;
	}
	_fixInsertionPointCoords();
	{
		UT_sint32 x;
		UT_sint32 y;
		UT_sint32 x2;
		UT_sint32 y2;
		bool bDirection;
		fl_BlockLayout* pBlock1;
		fl_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, false, x, y, x2, y2, uheight, bDirection, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, false, x, y, x2, y2, uheight, bDirection, &pBlock2, &pRun2);
	}

	bool bDone = false;
	bool bIsDirty = false;
	fp_Run* pCurRun = pRun1;
	bool bShowHidden = getShowPara();

	while ((!bDone || bIsDirty) && pCurRun)
	{

		if (pCurRun == pRun2)
		{
			bDone = true;
		}

		fl_BlockLayout* pBlock = pCurRun->getBlock();
		UT_ASSERT(pBlock);

		FPVisibility eHidden  = pCurRun->isHidden();
		if(!((eHidden == FP_HIDDEN_TEXT && !bShowHidden)
		   || eHidden == FP_HIDDEN_REVISION
		   || eHidden == FP_HIDDEN_REVISION_AND_TEXT))
		{

			fp_Line* pLine = pCurRun->getLine();
			if(pLine == NULL || (pLine->getContainer()->getPage()== NULL))
			{
				return;
			}
			pLine->getScreenOffsets(pCurRun, xoff, yoff);

			dg_DrawArgs da;

			da.pG = m_pG;
			da.xoff = xoff;
			da.yoff = yoff + pLine->getAscent();

			pCurRun->draw(&da);
		}

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
			bIsDirty = false;
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
bool FV_View::_clearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, bool bFullLineHeightRect)
{
	xxx_UT_DEBUGMSG(("FV_View::_clearBetweenPositions called\n"));
	if (iPos1 >= iPos2)
	{
		return true;
	}

	fp_Run* pRun1;
	fp_Run* pRun2;
	UT_uint32 uheight;

	_fixInsertionPointCoords();
	{
		UT_sint32 x;
		UT_sint32 y;
		UT_sint32 x2;
		UT_sint32 y2;
		bool bDirection;
		fl_BlockLayout* pBlock1;
		fl_BlockLayout* pBlock2;

		/*
		  we don't really care about the coords.  We're calling these
		  to get the Run pointer
		*/
		_findPositionCoords(iPos1, false, x, y, x2, y2, uheight, bDirection, &pBlock1, &pRun1);
		_findPositionCoords(iPos2, false, x, y, x2, y2, uheight, bDirection, &pBlock2, &pRun2);
	}

	if (!pRun1 && !pRun2)
	{
		// no formatting info for either block, so just bail
		// this can happen during spell, when we're trying to invalidate
		// a new squiggle before the block has been formatted
		return false;
	}

	// HACK: In certain editing cases only one of these is NULL, which
	//		 makes locating runs to clear more difficult.  For now, I'm
	//		 playing it safe and trying to just handle these cases here.
	//		 The real solution may be to just bail if *either* is NULL,
	//		 but I'm not sure.
	//
	//		 If you're interested in investigating this alternative
	//		 approach, play with the following asserts.

//	UT_ASSERT(pRun1 && pRun2);
	UT_ASSERT(pRun2);

	bool bDone = false;
	fp_Run* pCurRun = (pRun1 ? pRun1 : pRun2);


	while (!bDone)
	{
		if (pCurRun == pRun2)
		{
			bDone = true;
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
			else
				bDone = true;
			// otherwise we get fun
			// infinte loops
		}
	}
	return true;
}

void FV_View::_findPositionCoords(PT_DocPosition pos,
								  bool bEOL,
								  UT_sint32& x,
								  UT_sint32& y,
								  UT_sint32& x2,
								  UT_sint32& y2,
								  UT_uint32& height,
								  bool& bDirection,
								  fl_BlockLayout** ppBlock,
								  fp_Run** ppRun) const
{
	UT_sint32 xPoint = 0;
	UT_sint32 yPoint = 0;
	UT_sint32 xPoint2 = 0;
	UT_sint32 yPoint2 = 0;
	UT_sint32 iPointHeight;
	if(ppRun)
	{
		*ppRun = NULL;
	}
	// Get the previous block in the document. _findBlockAtPosition
	// will iterate forwards until it actually find a block if there
	// isn't one previous to pos.
	// (Removed code duplication. Jesper, 2001.01.25)
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);

	// probably an empty document, return instead of
	// dereferencing NULL.	Dom 11.9.00
	if(!pBlock)
	{
		// Do the assert. Want to know from debug builds when this happens.
		UT_ASSERT(pBlock);

		x = x2 = 0;
		y = y2 = 0;

		height = 0;
		if(ppBlock)
			*ppBlock = 0;
		return;
	}

	// if the block cannot contain point, find the nearest block to
	// the left that can
	while(!pBlock->canContainPoint())
	{
		UT_sint32 pos2 = pBlock->getPosition(true) - 2;
		if(pos2 < 2)
			break;

		pBlock = _findBlockAtPosition(pos2);
	}

	// If block is actually to the right of the requested position
	// (this happens in an empty document), update the pos with the
	// start pos of the block.
	PT_DocPosition iBlockPos = pBlock->getPosition(false);
	if (iBlockPos > pos)
	{
		pos = iBlockPos;
	}

	fp_Run* pRun = pBlock->findPointCoords(pos, bEOL, xPoint, yPoint, xPoint2, yPoint2, iPointHeight, bDirection);

	// NOTE prior call will fail if the block isn't currently formatted,
	// NOTE so we won't be able to figure out more specific geometry

//
// Needed for piecetable fields. We don't have these in 1.0
//
// OK if we're at the end of document at the last run is a field we have to add
// the width of the field run to xPoint, xPoint2.
//
	PT_DocPosition posEOD = 0;
	getEditableBounds(true, posEOD);
	xxx_UT_DEBUGMSG(("SEVIOR: Doing posEOD =%d getPoint %d pRun->isField %d bEOL %d \n",posEOD,getPoint(),pRun->isField(),bEOL));

	if(bEOL && pRun && posEOD == getPoint())
	{
		bool bBack = true;
		while(pRun && pRun->getPrev() && !pRun->isField() && pRun->getWidth() == 0)
		{
			bBack = false;
			pRun = pRun->getPrev();
		}
		if(pRun && pRun->isField() && bBack)
		{
			UT_DEBUGMSG(("SEVIOR: Doing EOD work aorund \n"));
			static_cast<fp_FieldRun *>(pRun)->recalcWidth();
			xPoint += pRun->getWidth();
			xPoint2 += pRun->getWidth();
		}
	}
	else if( (pRun == NULL) && (posEOD == getPoint()))
	{
		pRun = pBlock->getFirstRun();
		while(pRun && (pRun->getNext() != NULL))
		{
			pRun = pRun->getNext();
		}
	}
	if (pRun)
	{
		// we now have coords relative to the page containing the ins pt
			fp_Line * pLine =  pRun->getLine();
			if(!pLine)
		{
			x = x2 = 0;
			y = y2 = 0;

			height = 0;
			if(ppBlock)
			  *ppBlock = 0;
			return;
		}

		fp_Page* pPointPage = pLine->getContainer()->getPage();

		UT_sint32 iPageOffset;
		getPageYOffset(pPointPage, iPageOffset);

		yPoint += iPageOffset;
		xPoint += getPageViewLeftMargin();

		yPoint2 += iPageOffset;
		xPoint2 += getPageViewLeftMargin();

		// now, we have coords absolute, as if all pages were stacked vertically
		xPoint -= m_xScrollOffset;
		yPoint -= m_yScrollOffset;

		xPoint2 -= m_xScrollOffset;
		yPoint2 -= m_yScrollOffset;


		// now, return the results
		x = xPoint;
		y = yPoint;

		x2 = xPoint2;
		y2 = yPoint2;

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
  if ( m_pG->getCaret() != NULL )
    {
	if( getPoint()  )
	{
		_findPositionCoords(getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_xPoint2, m_yPoint2, m_iPointHeight, m_bPointDirection, NULL, NULL);
	}
	fp_Page * pPage = getCurrentPage();
	UT_RGBColor * pClr = NULL;
	if (pPage)
		pClr = pPage->getOwningSection()->getPaperColor();
	
	m_pG->getCaret()->setCoords(m_xPoint, m_yPoint, m_iPointHeight,
					  m_xPoint2, m_yPoint2, m_iPointHeight, 
					  m_bPointDirection, pClr);

	xxx_UT_DEBUGMSG(("SEVIOR: m_yPoint = %d m_iPointHeight = %d \n",m_yPoint,m_iPointHeight));
	// hang onto this for _moveInsPtNextPrevLine()
	m_xPointSticky = m_xPoint + m_xScrollOffset - getPageViewLeftMargin();
    }
}

void FV_View::_draw(UT_sint32 x, UT_sint32 y,
					UT_sint32 width, UT_sint32 height,
					bool bDirtyRunsOnly, bool bClip)
{
	xxx_UT_DEBUGMSG(("FV_View::draw_3 [x %ld][y %ld][w %ld][h %ld][bClip %ld]\n"
					 "\t\twith [yScrollOffset %ld][windowHeight %ld]\n",
					 x,y,width,height,bClip,
					 m_yScrollOffset,m_iWindowHeight));
	
	// CHECK_WINDOW_SIZE
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

	// TMN: Leave this rect at function scope!
	// gr_Graphics only stores a _pointer_ to it!
	UT_Rect rClip;
	if (bClip)
	{
		rClip.left = x;
		rClip.top = y;
		rClip.width = width;
		rClip.height = height;
		m_pG->setClipRect(&rClip);
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

	UT_RGBColor clrMargin = getColorMargin();

	if (!bDirtyRunsOnly)
	{
		if ((m_xScrollOffset < getPageViewLeftMargin()) && (getViewMode() == VIEW_PRINT))
		{
			// fill left margin
			m_pG->fillRect(clrMargin, 0, 0, getPageViewLeftMargin() - m_xScrollOffset, m_iWindowHeight);
		}

		if (m_yScrollOffset < getPageViewTopMargin() && (getViewMode() == VIEW_PRINT))
		{
			// fill top margin
			m_pG->fillRect(clrMargin, 0, 0, m_iWindowWidth, getPageViewTopMargin() - m_yScrollOffset);
		}
	}

	UT_sint32 curY = getPageViewTopMargin();
	fp_Page* pPage = m_pLayout->getFirstPage();
	while (pPage)
	{
		UT_sint32 iPageWidth		= pPage->getWidth();
		UT_sint32 iPageHeight		= pPage->getHeight();
		UT_sint32 adjustedTop		= curY - m_yScrollOffset;
		fl_DocSectionLayout * pDSL = pPage->getOwningSection();
		if(getViewMode() != VIEW_PRINT)
		{
			iPageHeight = iPageHeight - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}

		UT_sint32 adjustedBottom = adjustedTop + iPageHeight + getPageViewSep();

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
			// don't need to draw them either.	exit loop now.
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
			//TF NOTE: Can we break out here?
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
			da.xoff = getPageViewLeftMargin() - m_xScrollOffset;
			da.yoff = adjustedTop;
			UT_sint32 adjustedLeft	= getPageViewLeftMargin() - m_xScrollOffset;
			UT_sint32 adjustedRight = adjustedLeft + iPageWidth;

			if(pPage->getNext() != NULL)	
			{
				adjustedBottom -= getPageViewSep() - 1;
			}
			else
			{
				adjustedBottom -= getPageViewSep() + getPageViewTopMargin();
			}

			if (!bDirtyRunsOnly || pPage->needsRedraw() && (getViewMode() == VIEW_PRINT))
			{
			  UT_RGBColor * pClr = pPage->getOwningSection()->getPaperColor();
			  m_pG->fillRect(*pClr,adjustedLeft+_UL(1),adjustedTop+_UL(1),iPageWidth-_UL(1),iPageHeight-_UL(1));
//
// Since we're clearing everything we have to draw every run no matter
// what.
//
			  da.bDirtyRunsOnly = false;
			}
			pPage->draw(&da);

			// draw page decorations
			UT_RGBColor clr(0,0,0); 	// black
			m_pG->setColor(clr);

			// one pixel border a
			if(!isPreview() && (getViewMode() == VIEW_PRINT))
			{
				m_pG->setLineProperties(1.0,
										GR_Graphics::JOIN_MITER,
										GR_Graphics::CAP_BUTT,
										GR_Graphics::LINE_SOLID);

				m_pG->drawLine(adjustedLeft, adjustedTop, adjustedRight, adjustedTop);
				m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
				m_pG->drawLine(adjustedRight, adjustedBottom, adjustedLeft, adjustedBottom);
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedLeft, adjustedTop);
			}

//
// Draw page seperator
//
			UT_RGBColor paperColor = *(pPage->getOwningSection()->getPaperColor());

			// only in NORMAL MODE - draw a line across the screen
			// at a page boundary. Not used in online/web and print
			// layout modes
			if(getViewMode() == VIEW_NORMAL)
			{
				UT_RGBColor clrPageSep(192,192,192);		// light gray
				m_pG->setColor(clrPageSep);

				m_pG->setLineProperties(1.0,
										GR_Graphics::JOIN_MITER,
										GR_Graphics::CAP_BUTT,
										GR_Graphics::LINE_SOLID);

				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight, adjustedBottom);
				adjustedBottom += 1;
				m_pG->setColor(clr);
			}
			// fill to right of page
			if (m_iWindowWidth - (adjustedRight + 1) > 0)
			{
				// In normal mode, the right margin is
				// white (since the whole screen is white).
				if(getViewMode() != VIEW_PRINT)
				{
					m_pG->fillRect(paperColor, adjustedRight, adjustedTop, m_iWindowWidth - (adjustedRight), iPageHeight + 1);
				}
				// Otherwise, the right margin is the
				// margin color (gray).
				else
				{
					m_pG->fillRect(clrMargin, adjustedRight + 1, adjustedTop, m_iWindowWidth - (adjustedRight + 1), iPageHeight + 1);
				}
			}

			// fill separator below page
			if ((m_iWindowHeight - (adjustedBottom + 1) > 0) && (VIEW_PRINT == getViewMode()))
			{
			        if(pPage->getNext() != NULL)
				{
					m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, getPageViewSep());
				}
				else // found last page
				{
				        UT_sint32 botfill = getWindowHeight() - adjustedBottom - 1 ;
					m_pG->fillRect(clrMargin, adjustedLeft, adjustedBottom + 1, m_iWindowWidth - adjustedLeft, botfill);
				}
			}

			// two pixel drop shadow

			if(!isPreview() && (getViewMode() == VIEW_PRINT))
			{
				m_pG->setLineProperties(1.0,
										GR_Graphics::JOIN_MITER,
										GR_Graphics::CAP_BUTT,
										GR_Graphics::LINE_SOLID);

				adjustedLeft += 3;
				adjustedBottom += 1;
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight+1, adjustedBottom);

				adjustedBottom += 1;
				m_pG->drawLine(adjustedLeft, adjustedBottom, adjustedRight+1, adjustedBottom);

				adjustedTop += 3;
				adjustedRight += 1;
				m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);

				adjustedRight += 1;
				m_pG->drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
			}
		}

		curY += iPageHeight + getPageViewSep();

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

}


void FV_View::_setPoint(PT_DocPosition pt, bool bEOL)
{
	bool bWasSelectionEmpty = isSelectionEmpty();

	if (!m_pDoc->getAllowChangeInsPoint())
	{
		return;
	}
	m_iInsPoint = pt;
	m_bPointEOL = bEOL;
	_fixInsertionPointCoords();
	if(!m_pDoc->isPieceTableChanging())
	{
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
	}

	// So, if there is a selection now, we should disable the cursor; conversely,
	// if there is no longer a selection, we should enable the cursor.
	if (bWasSelectionEmpty != isSelectionEmpty())
	{
		if (isSelectionEmpty())
			m_pG->getCaret()->enable();
		else
			m_pG->getCaret()->disable();
	}
}


/*!
 Spell-check pending word
 If the IP does not touch the pending word, spell-check it.

 \note This function used to exit if PT was changing - but that
	   prevents proper squiggle behavior during undo, so the check has
	   been removed. This means that the pending word POB must be
	   updated to reflect the PT changes before the IP is moved.
 */
void
FV_View::_checkPendingWordForSpell(void)
{
	if (!m_pLayout->isPendingWordForSpell()) return;

	// Find block at IP
	fl_BlockLayout* pBL = _findBlockAtPosition(m_iInsPoint);
	if (pBL)
	{
		UT_uint32 iOffset = m_iInsPoint - pBL->getPosition();

		// If it doesn't touch the pending word, spell-check it
		if (!m_pLayout->touchesPendingWordForSpell(pBL, iOffset, 0))
		{
			// no longer there, so check it
			if (m_pLayout->checkPendingWordForSpell())
			{
				// FIXME:jskov Without this updateScreen call, the
				// just squiggled word remains deleted. It's overkill
				// (surely we should have a requestUpdateScreen() that
				// does so after all operations have completed), but
				// works. Unfortunately it causes a small screen
				// artifact when pressing undo, since some runs may be
				// redrawn before they have their correct location
				// recalculated. In other words, make the world a
				// better place by adding requestUpdateScreen or
				// similar.
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


bool FV_View::_charMotion(bool bForward,UT_uint32 countChars)
{
	// advance(backup) the current insertion point by count characters.
	// return false if we ran into an end (or had an error).

	PT_DocPosition posOld = m_iInsPoint;
	fp_Run* pRun = NULL;
	fl_BlockLayout* pBlock = NULL;
	UT_sint32 x;
	UT_sint32 y;
	UT_sint32 x2;
	UT_sint32 y2;
	bool bDirection;
	UT_uint32 uheight;
	m_bPointEOL = false;

	/*
	  we don't really care about the coords.  We're calling these
	  to get the Run pointer
	*/
	PT_DocPosition posBOD;
	PT_DocPosition posEOD;
	bool bRes;

	bRes = getEditableBounds(false, posBOD);
	bRes = getEditableBounds(true, posEOD);
	UT_ASSERT(bRes);

	// FIXME:jskov want to rewrite this code to use simplified
	// versions of findPositionCoords. I think there's been some bugs
	// due to that function being overloaded to be used from this
	// code.
	UT_sint32 xold,yold,x2old,y2old;
	_findPositionCoords(m_iInsPoint, false, xold, yold, x2old,y2old,uheight, bDirection, &pBlock, &pRun);
	if (bForward)
	{
		_setPoint(m_iInsPoint + countChars);
		_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
//
// If we come to a table boundary we have doc positions with no blocks.
// _findPositionCoords signals this by returning pRun == NULL
//
		bool bExtra = false;
		while(m_iInsPoint <= posEOD && (pRun == NULL || ((x == xold) && (y == yold) &&
														 (x2 == x2old) && (y2 == y2old))))
		{
			_setPoint(m_iInsPoint+1);
			_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
			bExtra = true;
		}
		if(bExtra)
		{
			_setPoint(m_iInsPoint-1);
		}
#if 0
		while(pRun != NULL &&  pRun->isField() && m_iInsPoint <= posEOD)
		{
			_setPoint(m_iInsPoint+1);
			if(m_iInsPoint <= posEOD)
			{
				_findPositionCoords(m_iInsPoint, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
			}
		}
#endif
	}
	else
	{
		_setPoint(m_iInsPoint - countChars);
		_findPositionCoords(m_iInsPoint, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
//
// If we come to a table boundary we have doc positions with no blocks.
// _findPositionCoords signals this by returning pRun == NULL
//
		bool bExtra = false;
		while( m_iInsPoint >= posBOD && (pRun == NULL || ((x == xold) && (y == yold) &&
														 (x2 == x2old) && (y2 == y2old))))
		{
			xxx_UT_DEBUGMSG(("_charMotion: Looking at point m_iInsPoint %d \n",m_iInsPoint));
			_setPoint(m_iInsPoint-1);
			_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
			bExtra = true;
		}
		if(bExtra)
		{
			_setPoint(m_iInsPoint-1);
		}
#if 0
// Needed for piecetable fields - we don't have these in 1.0

		while(pRun != NULL && pRun->isField() && m_iInsPoint >= posBOD)
		{
			_setPoint(m_iInsPoint-1);
			_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
		}
#endif
		// if the run which declared itself for our position is end of paragraph run,
		// we need to ensure that the position is just before the run, not after it
		// (fixes bug 1120)
		xxx_UT_DEBUGMSG(("_charMotion: pRun->getBlockOffset() %d getPosition %d insPoint %d \n",pRun->getBlockOffset(), pRun->getBlock()->getPosition(),m_iInsPoint));
		if(pRun && pRun->getType() == FPRUN_ENDOFPARAGRAPH
		   && (pRun->getBlockOffset() + pRun->getBlock()->getPosition()) < m_iInsPoint)
		{
			_setPoint(m_iInsPoint-1);
		}
	}

	UT_ASSERT(bRes);

	bRes = true;

	// we might have skipped over some runs that cannot contain the
	// point, but but have non-zero length, such as any hidden text;
	// if this is the case, we need to adjust the document position accordingly

	PT_DocPosition iRunStart = pBlock->getPosition(false) + pRun->getBlockOffset();
	PT_DocPosition iRunEnd = iRunStart + pRun->getLength();

	// containing layout we will work with, ususally section
// 	fl_ContainerLayout * pCL = pBlock->myContainingLayout();

	// the layout immediately above the runs, should be block
// 	fl_ContainerLayout * pBL = pBlock;

	// indicates how many layout layers we had to step up to get valid pCL
// 	UT_uint32 iLayoutDepth = 0;

	if(bForward && ( m_iInsPoint > iRunEnd))
	{
		// the run we have got is the on left of the ins point, we
		// need to find the right one and set the point there; we also
		// need to make sure that we handle correctly any hidden
		// sub layouts (blocks, sections, table cells, tables ...)

		// get the next run that can contain insertion point
		pRun = pRun->getNext();
		while(pRun && (!pRun->canContainPoint() || pRun->getLength() == 0))
			pRun = pRun->getNext();
		if(pRun)
		{
			_setPoint(1 + pBlock->getPosition(false) + pRun->getBlockOffset());
		}
		else
		{
//
// FIXME: Put in some code here to handle table/cell boundaries. Right
// now you have to press left arrow twice to move form outside to inside
// a table.

		}
	}

	// this is much simpler, since the findPointCoords will return the
	// run on the left of the requested position, so we just need to move
	// to its end if the position does not fall into that run
	xxx_UT_DEBUGMSG(("_charMotion: iRunEnd %d \n",iRunEnd));
	if(!bForward && (iRunEnd <= m_iInsPoint) && (pRun->getBlockOffset() > 0))
	{
		_setPoint(iRunEnd - 1);
	}


	if ((UT_sint32) m_iInsPoint < (UT_sint32) posBOD)
	{
		_setPoint(posBOD);
		bRes = false;
	}
	else if ((UT_sint32) m_iInsPoint >= (UT_sint32) posEOD)
	{
		m_bPointEOL = true;
		_setPoint(posEOD);
		bRes = false;
	}

	if (m_iInsPoint != posOld)
	{
		m_pLayout->considerPendingSmartQuoteCandidate();
		_checkPendingWordForSpell();
		_clearIfAtFmtMark(posOld);
		notifyListeners(AV_CHG_MOTION);
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Point = %d \n",getPoint()));
	_fixInsertionPointCoords();
	return bRes;
}


void FV_View::_doPaste(bool bUseClipboard, bool bHonorFormatting)
{
	// internal portion of paste operation.

	if (!isSelectionEmpty())
		_deleteSelection();

	_clearIfAtFmtMark(getPoint());
	PD_DocumentRange dr(m_pDoc,getPoint(),getPoint());
	m_pApp->pasteFromClipboard(&dr,bUseClipboard,bHonorFormatting);

	_generalUpdate();

	_updateInsertionPoint();
}


UT_Error FV_View::_deleteBookmark(const char* szName, bool bSignal, PT_DocPosition &pos1, PT_DocPosition &pos2)
{
	if(!m_pDoc->isBookmarkUnique((const XML_Char *)szName))
	{
		// even though we will only send out a single explicit deleteSpan
		// call, we need to find out where both of the markers are in the
		// document, so that the caller can adjust any stored doc positions
		// if necessary

		fp_BookmarkRun * pB1;
		UT_uint32 bmBlockOffset[2];
		fl_BlockLayout * pBlock[2];
		UT_uint32 i = 0;

		fl_BlockLayout *pBL;
		fl_SectionLayout *pSL = m_pLayout->getFirstSection();
		fp_Run * pRun = 0;
		bool bFound = false;

		//find the first of the two bookmarks
		while(pSL)
		{
			pBL = (fl_BlockLayout *) pSL->getFirstLayout();

			while(pBL)
			{
				pRun = pBL->getFirstRun();

				while(pRun)
				{
					if(pRun->getType()== FPRUN_BOOKMARK)
					{
						pB1 = static_cast<fp_BookmarkRun*>(pRun);
						if(!UT_XML_strcmp((const XML_Char *)szName, pB1->getName()))
						{
							bmBlockOffset[i] = pRun->getBlockOffset();
							pBlock[i] = pRun->getBlock();
							i++;
							if(i>1)
							{
								bFound = true;
								break;
							}
						}
					}
					if(bFound)
						break;
					pRun = pRun->getNext();
				}
				if(bFound)
					break;
				pBL = (fl_BlockLayout *) pBL->getNext();
			}
			if(bFound)
				break;
			pSL = (fl_SectionLayout *) pSL->getNext();
		}

		UT_ASSERT(pRun && pRun->getType()==FPRUN_BOOKMARK && pBlock || pBlock);
		if(!pRun || pRun->getType()!=FPRUN_BOOKMARK || !pBlock || !pBlock)
			return false;

		// Signal PieceTable Change
		if(bSignal)
			_saveAndNotifyPieceTableChange();

		UT_DEBUGMSG(("fv_View::cmdDeleteBookmark: bl pos [%d,%d], bmOffset [%d,%d]\n",
					 pBlock[0]->getPosition(false), pBlock[1]->getPosition(false),bmBlockOffset[0],bmBlockOffset[1]));

		pos1 = pBlock[0]->getPosition(false) + bmBlockOffset[0];
		pos2 = pBlock[1]->getPosition(false) + bmBlockOffset[1];

		UT_uint32 iRealDeleteCount;

		m_pDoc->deleteSpan(pos1,pos1 + 1,NULL,iRealDeleteCount);
		// TODO -- add proper revision handling using iRealDeleteCount

		// Signal PieceTable Changes have finished
		if(bSignal)
		{
			_generalUpdate();
			_restorePieceTableState();
		}
	}
	else
		UT_DEBUGMSG(("fv_View::cmdDeleteBookmark: bookmark \"%s\" does not exist\n",szName));
	return true;
}


/*! Returns the hyperlink around position pos, if any; assumes
 * posStart, posEnd in same block. */
fp_HyperlinkRun * FV_View::_getHyperlinkInRange(PT_DocPosition &posStart,
												PT_DocPosition &posEnd)
{
	fl_BlockLayout *pBlock = _findBlockAtPosition(posStart);
	PT_DocPosition curPos = posStart - pBlock->getPosition(false);

	fp_Run * pRun = pBlock->getFirstRun();

	//find the run at pos
	while(pRun && pRun->getBlockOffset() <= curPos)
		pRun = pRun->getNext();

	UT_ASSERT(pRun);
	if(!pRun)
		return NULL;

	// now we have the run immediately after the run in question, so
	// we step back
	pRun = pRun->getPrev();
	UT_ASSERT(pRun);
	if(!pRun)
		return NULL;

	if (pRun->getHyperlink() != NULL)
		return pRun->getHyperlink();

	// Now, getHyperlink() looks NULL, so let's step forward till posEnd.

	PT_DocPosition curPosEnd = posEnd - pBlock->getPosition(false);

	// Continue checking for hyperlinks.
	while(pRun && pRun->getBlockOffset() <= curPosEnd)
	{
		pRun = pRun->getNext();
		if (pRun && pRun->getPrev() && pRun->getPrev()->getHyperlink() != NULL)
			return pRun->getPrev()->getHyperlink();
	}

	// OK, we're really safe now.
	return NULL;
}

/*
	NB: this function assumes that the position it is passed is inside a
	hyperlink and will assert if it is not so.
*/

UT_Error FV_View::_deleteHyperlink(PT_DocPosition &pos1, bool bSignal)
{
	fp_HyperlinkRun * pH1 = _getHyperlinkInRange(pos1, pos1);

	UT_ASSERT(pH1);
	if(!pH1)
		return false;

	pos1 = pH1->getBlock()->getPosition(false) + pH1->getBlockOffset();

	// now reset the hyperlink member for the runs that belonged to this
	// hyperlink

	fp_Run * pRun = pRun = pH1->getNext();
	UT_ASSERT(pRun);
	while(pRun && pRun->getHyperlink() != NULL)
	{
		UT_DEBUGMSG(("fv_View::_deleteHyperlink: reseting run 0x%x\n", pRun));
		pRun->setHyperlink(NULL);
		pRun = pRun->getNext();
	}

	UT_ASSERT(pRun);

	// Signal PieceTable Change
	if(bSignal)
		_saveAndNotifyPieceTableChange();

	UT_DEBUGMSG(("fv_View::cmdDeleteHyperlink: position [%d]\n",
				pos1));

	UT_uint32 iRealDeleteCount;

	m_pDoc->deleteSpan(pos1,pos1 + 1,NULL, iRealDeleteCount);
	// TODO -- add proper revision handling using iRealDeleteCount

	// Signal PieceTable Changes have finished
	if(bSignal)
	{
		_generalUpdate();
		_restorePieceTableState();
	}
	return true;
}


UT_Error FV_View::_insertGraphic(FG_Graphic* pFG, const char* szName)
{
	UT_ASSERT(pFG);
	UT_ASSERT(szName);

	if (!pFG)
	  return UT_ERROR;

	double fDPI = m_pG->getResolution() * 100. / m_pG->getZoomPercentage();
	return pFG->insertIntoDocument(m_pDoc, fDPI, getPoint(), szName);
}


void FV_View::_clearIfAtFmtMark(PT_DocPosition dpos)
{
	// Check to see if we're at the beginning of the line. If we
	// aren't, then it's safe to delete the FmtMark. Else we could
	// wipe out the placeholder FmtMark for our attributes.
	// Fix for Bug #863
	xxx_UT_DEBUGMSG(("FV_View::_clearIfAtFmtMark called\n"));
	if ( ( dpos != _getDocPosFromPoint(dpos,FV_DOCPOS_BOL) ))
	{
		m_pDoc->clearIfAtFmtMark(dpos);
		_generalUpdate();//  Sevior: May be able to live with notify.. always
	}
	else
	{
		notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK);
	}
}


// NB: returns a UCS string that the caller needs to FREEP
UT_UCSChar * FV_View::_lookupSuggestion(fl_BlockLayout* pBL,
										fl_PartOfBlock* pPOB, UT_uint32 ndx)
{
	// mega caching - are these assumptions valid?
	UT_UCSChar * szSuggest = NULL;

	// TODO these should really be static members, so we can properly
	// clean up
	static fl_BlockLayout * pLastBL = 0;
	static fl_PartOfBlock * pLastPOB = 0;
	static UT_Vector * pSuggestionCache = 0;

	if (pBL == pLastBL && pLastPOB == pPOB)
	{
		if ((pSuggestionCache->getItemCount()) &&
			( ndx <= pSuggestionCache->getItemCount()))
		{
			UT_UCS4_cloneString(&szSuggest,
							   (UT_UCSChar *) pSuggestionCache->getNthItem(ndx-1));
		}
		return szSuggest;
	}

	if (pSuggestionCache) // got here, so we need to invalidate the cache
	{
		// clean up
		for (UT_uint32 i = 0; i < pSuggestionCache->getItemCount(); i++)
		{
			UT_UCSChar * sug = (UT_UCSChar *)pSuggestionCache->getNthItem(i);
			FREEP(sug);
		}

		pLastBL = 0;
		pLastPOB = 0;
		DELETEP(pSuggestionCache);
	}

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pWord = (UT_UCSChar*)pgb.getPointer(pPOB->getOffset());

	// lookup suggestions
	UT_Vector * sg = 0;

	UT_UCSChar theWord[INPUTWORDLEN + 1];
	// convert smart quote apostrophe to ASCII single quote to be
	// compatible with ispell
	UT_uint32 len = pPOB->getLength();
	for (UT_uint32 ldex=0; ldex < len && ldex < INPUTWORDLEN; ldex++)
	{
		UT_UCSChar currentChar;
		currentChar = *(pWord + ldex);
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		theWord[ldex] = currentChar;
	}

	{
		SpellChecker * checker = NULL;
		const char * szLang = NULL;

		const XML_Char ** props_in = NULL;

		if (getCharFormat(&props_in))
		{
			szLang = UT_getAttribute("lang", props_in);
			FREEP(props_in);
		}

		if (szLang)
		{
			// we get smart and request the proper dictionary
			checker = SpellManager::instance().requestDictionary(szLang);
		}
		else
		{
			// we just (dumbly) default to the last dictionary
			checker = SpellManager::instance().lastDictionary();
		}

		sg = checker->suggestWord (theWord, pPOB->getLength());
		if(sg)
		{
			 m_pApp->suggestWord(sg,theWord, pPOB->getLength());
		}
	}

	if (!sg)
	{
		UT_DEBUGMSG(("DOM: no suggestions returned in main dictionary \n"));
		DELETEP(sg);
		sg = new UT_Vector();
		m_pApp->suggestWord(sg,theWord, pPOB->getLength());
		if(sg->getItemCount() == 0)
		{
			 DELETEP(sg);
			 return 0;
		}

	}

	// we currently return all requested suggestions
	if ((sg->getItemCount()) &&
		( ndx <= sg->getItemCount()))
	{
		UT_UCS4_cloneString(&szSuggest, (UT_UCSChar *) sg->getNthItem(ndx-1));
	}

	pSuggestionCache = sg;
	pLastBL = pBL;
	pLastPOB = pPOB;
	return szSuggest;
}


void FV_View::_prefsListener( XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_StringPtrMap * /*phChanges*/, void *data )
{
	FV_View *pView = (FV_View *)data;
	bool b;
	UT_ASSERT(data && pPrefs);
	if ( pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_CursorBlink, &b) && b != pView->m_bCursorBlink )
	{
		UT_DEBUGMSG(("FV_View::_prefsListener m_bCursorBlink=%s m_bCursorIsOn=%s\n",
					 pView->m_bCursorBlink ? "TRUE" : "FALSE",
					 pView->m_bCursorIsOn ? "TRUE" : "FALSE"));

		pView->m_bCursorBlink = b;
		pView->m_pG->getCaret()->setBlink(b);
	}


	// Update colors
   	const XML_Char * pszTmpColor = NULL;
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForShowPara, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorShowPara);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForSquiggle, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorSquiggle);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForMargin, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorMargin);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForSelBackground, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorSelBackground);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForFieldOffset, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorFieldOffset);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForImage, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorImage);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForHyperLink, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorHyperLink);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForHdrFtr, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorHdrFtr);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForColumnLine, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorColumnLine);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision1, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[0]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision2, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[1]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision3, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[2]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision4, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[3]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision5, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[4]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision6, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[5]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision7, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[6]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision8, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[7]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision9, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[8]);
	}
	if (pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForRevision10, &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[9]);
	}
	// FIXME:jskov: is it necessary to do something here to cause a full redraw?

	if (!pView->m_bWarnedThatRestartNeeded &&
		( pPrefs->getPrefsValueBool((XML_Char*)AP_PREF_KEY_DefaultDirectionRtl, &b) && b != pView->m_bDefaultDirectionRtl)
		 || (pPrefs->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_UseHebrewContextGlyphs, &b) && b != pView->m_bUseHebrewContextGlyphs)
		)
	{
		/*	It is possible to change this at runtime, but it may impact the
			way the document is displayed in an unexpected way (from the user's
			point of view). It is therefore probably better to apply this change
			only when AW will be restarted or a new document is created and
			notify the user about that.
		*/
		XAP_Frame * pFrame = (XAP_Frame *) pView->getParentData();
		UT_ASSERT((pFrame));

		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg2 = pSS->getValue(AP_STRING_ID_MSG_AfterRestartNew, XAP_App::getApp()->getDefaultEncoding()).c_str();

		UT_ASSERT((/*pMsg1 && */pMsg2));

		pFrame->showMessageBox(pMsg2, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		pView->m_bWarnedThatRestartNeeded = true;
	}
}


/*!
 * Copy a header/footer from a pHdrFtrSrc to an empty pHdrFtrDest.
 * into a new type of header/footer in the same section.
 */
void FV_View::_populateThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtrSrc, fl_HdrFtrSectionLayout * pHdrFtrDest)
{
	PD_DocumentRange dr_source;
	PT_DocPosition iPos1,iPos2;
	iPos1 = m_pDoc->getStruxPosition(pHdrFtrSrc->getFirstLayout()->getStruxDocHandle());
	fl_BlockLayout * pLast = (fl_BlockLayout *) pHdrFtrSrc->getLastLayout();
	iPos2 = pLast->getPosition(false);
//
// This code assumes there is an End of Block run at the end of the Block.
// Thanks to Jesper, there always is!
//
	while(pLast->getNext() != NULL)
	{
		pLast = (fl_BlockLayout *) pLast->getNext();
	}
	fp_Run * pRun = pLast->getFirstRun();
	while( pRun->getNext() != NULL)
	{
		pRun = pRun->getNext();
	}
	iPos2 += pRun->getBlockOffset();
//
// OK got the doc range for the source. Set it and copy it.
//
	dr_source.set(m_pDoc,iPos1,iPos2);
//
// Copy to and from clipboard to populate the header/Footer
//
	UT_DEBUGMSG(("SEVIOR: Copy to clipboard making header/footer \n"));
	m_pApp->copyToClipboard(&dr_source);
	PT_DocPosition posDest = 0;
	posDest = pHdrFtrDest->getFirstLayout()->getPosition(true);
	PD_DocumentRange dr_dest(m_pDoc,posDest,posDest);
	UT_DEBUGMSG(("SEVIOR: Pasting to clipboard making header/footer \n"));
	m_pApp->pasteFromClipboard(&dr_dest,true,true);
}


/*!
 * This method removes the HdrFtr pHdrFtr
 */
void FV_View::_removeThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtr)
{
	if(pHdrFtr == NULL)
	{
		return;
	}
//
// Need this to remove the HdrFtr attributes in the section strux.
//
	fl_DocSectionLayout * pDSL = pHdrFtr->getDocSectionLayout();
	const XML_Char * pszHdrFtrType = NULL;
	PL_StruxDocHandle sdhHdrFtr = pHdrFtr->getStruxDocHandle();
	m_pDoc->getAttributeFromSDH(sdhHdrFtr,PT_TYPE_ATTRIBUTE_NAME, &pszHdrFtrType);
	PT_DocPosition	posDSL = m_pDoc->getStruxPosition(pDSL->getStruxDocHandle());
//
// Remove the header/footer strux
//
	m_pDoc->deleteHdrFtrStrux(sdhHdrFtr);
//
// Change the DSL strux to remove the reference to this header/footer
//
	const XML_Char * remFmt[] = {pszHdrFtrType,NULL,NULL,NULL};
	m_pDoc->changeStruxFmt(PTC_RemoveFmt,posDSL,posDSL,(const XML_Char **) remFmt,NULL,PTX_Section);
}

void FV_View::_cmdEditHdrFtr(HdrFtrType hfType)
{
	if(isHdrFtrEdit())
		clearHdrFtrEdit();
	fp_Page * pPage = getCurrentPage();
//
// If there is no header/footer, insert it and start to edit it.
//
	fl_HdrFtrShadow * pShadow = NULL;
	fp_ShadowContainer * pHFCon = NULL;
	pHFCon = pPage->getHdrFtrP(hfType);
	if(pHFCon == NULL)
	{
		insertHeaderFooter(hfType);
		return;
	}
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
//
// Put the insertion point at the beginning of the header
//
	fl_BlockLayout * pBL = (fl_BlockLayout *) pShadow->getFirstLayout();
	if (!isSelectionEmpty())
		_clearSelection();

	_setPoint(pBL->getPosition());
//
// Set Header/footer mode and we're done! Easy :-)
//
	setHdrFtrEdit(pShadow);
	_generalUpdate();
	_updateInsertionPoint();
}


/*	the problem with using bool to store the PT state is that
	when we make two successive calls to _saveAndNotifyPieceTableChange
	all subsequent calls to _restorePieceTableState will end up in the
	else branch, i.e, the PT will remain in state of change. Thus,
	the new implementation uses int instead of bool and actually keeps
	count of the calls to _save...;
*/
void FV_View::_saveAndNotifyPieceTableChange(void)
{
	//UT_DEBUGMSG(("notifying PieceTableChange start [%d]\n", m_iPieceTableState));
	if(m_pDoc->isPieceTableChanging())
		m_iPieceTableState++;
	m_pDoc->notifyPieceTableChangeStart();
}

void FV_View::_restorePieceTableState(void)
{
	if(m_iPieceTableState > 0)
	{
		//UT_DEBUGMSG(("notifying PieceTableChange (restore/start) [%d]\n", m_iPieceTableState));
		m_pDoc->notifyPieceTableChangeStart();
		m_iPieceTableState--;
	}
	else
	{
		//UT_DEBUGMSG(("notifying PieceTableChange (restore/end) [%d]\n", m_iPieceTableState));
		m_pDoc->notifyPieceTableChangeEnd();
	}
}


/*! will process the revision and apply fmt changes/deletes as appropriate;
    NB: this function DOES NOT notify piecetable changes, it is the
    responsibility of the caller to ensure that undo is properly bracketed
*/
void FV_View::_acceptRejectRevision(bool bReject, PT_DocPosition iStart, PT_DocPosition iEnd, const PP_RevisionAttr *pRevA)
{
	PP_RevisionAttr * pRevAttr = const_cast<PP_RevisionAttr*>(pRevA);

	const XML_Char * ppAttr[3];
	const XML_Char rev[] = "revision";
	ppAttr[0] = rev;
	ppAttr[1] = NULL;
	ppAttr[2] = NULL;

	const XML_Char ** ppProps, ** ppAttr2;
	UT_uint32 i;
	UT_uint32 iRealDeleteCount;

	const PP_Revision * pRev = pRevAttr->getGreatestLesserOrEqualRevision(m_iViewRevision);

	if(bReject)
	{
		switch(pRev->getType())
		{
			case PP_REVISION_ADDITION:
			case PP_REVISION_ADDITION_AND_FMT:
				// delete this fragment
				m_pDoc->deleteSpan(iStart,iEnd,NULL,iRealDeleteCount);
				break;

			case PP_REVISION_DELETION:
			case PP_REVISION_FMT_CHANGE:
				// remove the revision attribute
				m_pDoc->changeSpanFmt(PTC_RemoveFmt,iStart,iEnd,ppAttr,NULL);
				break;

			default:
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		}
	}
	else
	{
		switch(pRev->getType())
		{
			case PP_REVISION_ADDITION:
				// simply remove the revision attribute
				m_pDoc->changeSpanFmt(PTC_RemoveFmt,iStart,iEnd,ppAttr,NULL);
				break;

			case PP_REVISION_DELETION:
				// delete this fragment
				m_pDoc->deleteSpan(iStart,iEnd,NULL,iRealDeleteCount);
				break;

			case PP_REVISION_ADDITION_AND_FMT:
				// overlay the formatting and remove the revision attribute
				m_pDoc->changeSpanFmt(PTC_RemoveFmt,iStart,iEnd,ppAttr,NULL);

			case PP_REVISION_FMT_CHANGE:
				// overlay the formatting and remove this revision
				// from the revision attribute
				ppProps = new const XML_Char *[2* pRev->getPropertyCount() + 1];
				ppAttr2 = new const XML_Char *[2* pRev->getAttributeCount() + 3];

				for(i = 0; i < pRev->getPropertyCount(); i++)
				{
					pRev->getNthProperty(i, ppProps[2*i],ppProps[2*i + 1]);
				}

				ppProps[2*i] = NULL;

				for(i = 0; i < pRev->getAttributeCount(); i++)
				{
					pRev->getNthAttribute(i, ppAttr2[2*i],ppAttr2[2*i + 1]);
				}

				if(pRev->getType() == PP_REVISION_ADDITION_AND_FMT)
				{
					ppAttr2[2*i] = NULL;
				}
				else
				{
					// need to set a new revision attribute
					// first remove current revision from pRevAttr
					pRevAttr->removeRevision(pRev);
					delete pRev;

					ppAttr2[2*i] = rev;
					ppAttr2[2*i + 1] = pRevAttr->getXMLstring();
					ppAttr2[2*i + 2] = NULL;

					if(*ppAttr2[2*i + 1] == 0)
					{
						// no revision attribute left, which means we
						// have to remove it by separate call to changeSpanFmt

						// if this is the only attribute, we just set
						// the whole thing to NULL
						if(i == 0)
						{
							delete ppAttr2;
							ppAttr2 = NULL;
						}
						else
						{
							// OK, there are some other attributes
							// left, so we set the rev name to NULL
							// and remove the formatting by a separate
							// call to changeSpanFmt
							ppAttr2[2*i] = NULL;
						}

						// now we use the ppAttr set to remove the
						// revision attribute
						m_pDoc->changeSpanFmt(PTC_RemoveFmt,iStart,iEnd,ppAttr,NULL);
					}
				}


				m_pDoc->changeSpanFmt(PTC_AddFmt,iStart,iEnd,ppAttr2,ppProps);

				delete ppProps;
				delete ppAttr2;

				break;

			default:
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
		}
	}
}
