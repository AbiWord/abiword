/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "ut_std_string.h"
#include "ut_bytebuf.h"
#include "ut_timer.h"
#include "ut_types.h"
#include "xav_View.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_TOCLayout.h"
#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
#include "fl_SectionLayout.h"
#include "fl_AutoNum.h"
#include "fp_Page.h"
#include "fp_PageSize.h"
#include "fp_Column.h"
#include "fp_TableContainer.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
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
#include "ap_Strings.h"
#include "fd_Field.h"
#include "fv_ViewDoubleBuffering.h"

#ifdef ENABLE_SPELL
#include "spell_manager.h"
#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif
#endif

#include "ut_rand.h"
#include "fl_FootnoteLayout.h"
#include "pp_Revision.h"
#include "gr_Painter.h"

#include "fv_View.h"

// NB -- irrespective of this size, the piecetable will store
// at max BOOKMARK_NAME_LIMIT of chars as defined in pf_Frag_Bookmark.h
#define BOOKMARK_NAME_SIZE 30
#define CHECK_WINDOW_SIZE if(getWindowHeight() < 20) return;
// returns true iff the character BEFORE pos is a space.
// Special cases:
// -returns true if pos is at the beginning of the document
// -returns false if pos is not within the document
bool FV_View::_isSpaceBefore(PT_DocPosition pos) const
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
			return (UT_UCS4_isspace(*reinterpret_cast<UT_UCSChar *>(buffer.getPointer(offset - 1))));
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
	UT_ASSERT(curPos != m_Selection.getSelectionAnchor());
	bool bForwardSelection = (m_Selection.getSelectionAnchor() < curPos);

	if (bForward != bForwardSelection)
	{
		swapSelectionOrientation();
	}

	_clearSelection();

	return;
}

void FV_View::_eraseSelection(void)
{
	_fixInsertionPointCoords();
	if (!m_Selection.isSelected())
	{
		_resetSelection();
		return;
	}

	UT_uint32 iPos1, iPos2;

	if (m_Selection.getSelectionAnchor() < getPoint())
	{
		iPos1 = m_Selection.getSelectionAnchor();
		iPos2 = getPoint();
	}
	else
	{
		iPos1 = getPoint();
		iPos2 = m_Selection.getSelectionAnchor();
	}
	m_iLowDrawPoint = 0;
	m_iHighDrawPoint = 0;

	_clearBetweenPositions(iPos1, iPos2, true);
}

void FV_View::_clearSelection(bool bRedraw)
{
	if( isSelectionEmpty() )
	{
		return;
	}
	if (m_pG)
		m_pG->allCarets()->enable();

	_fixInsertionPointCoords();
	if (!m_Selection.isSelected())
	{
		_resetSelection();
		return;
	}

	UT_uint32 iPos1, iPos2;
	if(m_Selection.getSelectionMode() < FV_SelectionMode_Multiple)
	{
		if (m_Selection.getSelectionAnchor() < getPoint())
		{
			iPos1 = m_Selection.getSelectionAnchor();
			iPos2 = getPoint();
		}
		else
		{
			iPos1 = getPoint();
			iPos2 = m_Selection.getSelectionAnchor();
		}
		bool bres = _clearBetweenPositions(iPos1, iPos2, true);
		if(!bres)
			return;
	
		_resetSelection();
		m_iLowDrawPoint = 0;
		m_iHighDrawPoint = 0;

		if (bRedraw)
			_drawBetweenPositions(iPos1, iPos2);
	}
	else
	{
		UT_sint32 i = 0;
		UT_GenericVector<PD_DocumentRange *> vecRanges;

		for(i=0; i<m_Selection.getNumSelections();i++)
		{
			PD_DocumentRange * pTmp =m_Selection.getNthSelection(i);
			PD_DocumentRange * pTmp2 = new PD_DocumentRange(m_pDoc,pTmp->m_pos1,pTmp->m_pos2);
			vecRanges.addItem(pTmp2);
		}
		for(i=0; i< vecRanges.getItemCount();i++)
		{
			PD_DocumentRange * pDocR = vecRanges.getNthItem(i);
			if(pDocR)
			{
				iPos1 = pDocR->m_pos1;
				iPos2 = pDocR->m_pos2;
				if(iPos1 == iPos2)
				{
					iPos2++;
				}

				if (bRedraw)
					/*bool bres =*/ _clearBetweenPositions(iPos1, iPos2, true);
			}
		}
		_resetSelection();
		for(i=0; i< vecRanges.getItemCount();i++)
		{
			PD_DocumentRange * pDocR = vecRanges.getNthItem(i);
			if(pDocR)
			{
				iPos1 = pDocR->m_pos1;
				iPos2 = pDocR->m_pos2;
				if(iPos1 == iPos2)
				{
					iPos2++;
				}
				if (bRedraw)
					_drawBetweenPositions(iPos1, iPos2);
			}
		}
		UT_VECTOR_PURGEALL(PD_DocumentRange *,vecRanges);
	}
	_resetSelection();
	m_iLowDrawPoint = 0;
	m_iHighDrawPoint = 0;
}

void FV_View::_drawSelection()
{
	UT_return_if_fail(!isSelectionEmpty());
//	CHECK_WINDOW_SIZE
	UT_DEBUGMSG(("_drawSelection getPoint() %d m_Selection.getSelectionAnchor() %d \n",getPoint(),m_Selection.getSelectionAnchor()));
	if(m_Selection.getSelectionMode() < FV_SelectionMode_Multiple)
	{
		if (m_Selection.getSelectionAnchor() < getPoint())
		{
			_drawBetweenPositions(m_Selection.getSelectionAnchor(), getPoint());
		}
		else
		{
			_drawBetweenPositions(getPoint(), m_Selection.getSelectionAnchor());
		}
		m_iLowDrawPoint = UT_MIN(m_Selection.getSelectionAnchor(),getPoint());
		m_iHighDrawPoint = UT_MAX(m_Selection.getSelectionAnchor(),getPoint());
	}
	else
	{
		UT_sint32 i = 0;
		for(i=0; i<m_Selection.getNumSelections();i++)
		{
			PD_DocumentRange * pDocR = m_Selection.getNthSelection(i);
			UT_DEBUGMSG(("Drawing between %d and %d \n",pDocR->m_pos1,pDocR->m_pos2));
			if(pDocR)
			{
				PT_DocPosition iPos1 = pDocR->m_pos1;
				PT_DocPosition iPos2 = pDocR->m_pos2;
				if(iPos1 == iPos2)
				{
					iPos2++;
				}
				_drawBetweenPositions(iPos1, iPos2);
			}
		}
		m_iLowDrawPoint = 0;
		m_iHighDrawPoint = 0;
	}

}

// Note that isClearSelection() might change its tune in one of two ways.
// Way #1 is by calling one of the next few methods.
//   BUT! this never happens because m_Selection.getSelectionAnchor() == getPoint by def.
// Way #2 is if the Selection is set and the point is changed so that it
//          no longer equals m_Selection.getSelectionAnchor().
void FV_View::_resetSelection(void)
{
	m_Selection.clearSelection();
	m_Selection.setSelectionAnchor(getPoint());
	m_Selection.setSelectionLeftAnchor(getPoint());
	m_Selection.setSelectionRightAnchor(getPoint());
	m_iGrabCell = 0;
}

void FV_View::_setSelectionAnchor(void)
{
	m_Selection.setMode(FV_SelectionMode_Single);
	m_Selection.setSelectionAnchor(getPoint());
}

void FV_View::_deleteSelection(PP_AttrProp *p_AttrProp_Before, bool bNoUpdate,
							   bool bCaretLeft)
{
	// delete the current selection.
	// NOTE: this must clear the selection.

	UT_ASSERT(!isSelectionEmpty());

	PT_DocPosition iPoint = getPoint();

	UT_uint32 iRealDeleteCount;

	UT_uint32 iSelAnchor = m_Selection.getSelectionAnchor();
	if(iSelAnchor < 2)
	{
		if(!m_pDoc->isTableAtPos(iSelAnchor))
			iSelAnchor = 2;
	}
	if(m_FrameEdit.isActive())
	{
	        deleteFrame();
		return;
	}
	UT_ASSERT(iPoint != iSelAnchor);

	PT_DocPosition iLow = UT_MIN(iPoint,iSelAnchor);
	PT_DocPosition iHigh = UT_MAX(iPoint,iSelAnchor);

	// deal with character clusters, such as base char + vowel + tone mark in Thai
	UT_uint32 iLen = iHigh - iLow;
	_adjustDeletePosition(iLow, iLen); // modifies both iLow and iLen
	iHigh = iLow + iLen;
//
// OK adjust for deletetions that cross footnote/endnote boundaries.
//
	fl_FootnoteLayout * pFHigh = NULL;
	fl_FootnoteLayout * pFLow = NULL;
	fl_EndnoteLayout * pEHigh = NULL;
	fl_EndnoteLayout * pELow = NULL;
	if(isInFootnote(iHigh))
	{
		pFHigh = getClosestFootnote(iHigh);
		PT_DocPosition j = pFHigh->getPosition()+1; // Leave reference
		if(j > iLow)
		{
			iLow = j;
		}
	}
	else if(isInFootnote(iLow))
	{

// Here if we're not in footnote at iHigh

		pFLow = getClosestFootnote(iLow);
		iHigh = pFLow->getPosition(true) + pFLow->getLength() -1;
	}
	else if(isInEndnote(iHigh))
	{
		pEHigh = getClosestEndnote(iHigh);
		PT_DocPosition j = pEHigh->getPosition()+1; // Leave reference
		if(j > iLow)
		{
			iLow = j;
		}
	}
	else if(isInEndnote(iLow))
	{

// Here if we're not in Endnote at iHigh

		pELow = getClosestEndnote(iLow);
		iHigh = pELow->getPosition(true) + pELow->getLength() -1;
	}
//
// Don't delete the block right before a TOC!
//
	fl_BlockLayout * pBL = _findBlockAtPosition(iLow);
	if(pBL && pBL->getPrev() && pBL->getPrev()->getContainerType() == FL_CONTAINER_TOC)
	{
		if(pBL->getPosition(true) == iLow)
		{
			iLow++;
		}
	}
	// fl_BlockLayout::getLength() *includes* the length of the block strux, hence we
	// should use getPosition(true) here; however, since the block is found at iLow, this
	// condition can never be true !!! -- Anybody knows what this is about?
	else if(pBL && (pBL->getPosition(true) + pBL->getLength() < iLow))
	{
		iLow++;
	}
	// Check if everything is selected from the beginning of the section to a table.
	// In that case, we want to delete also the block.
	if (pBL && !pBL->getPrev() && (pBL->myContainingLayout()->getContainerType() == FL_CONTAINER_DOCSECTION) &&
		isInTable(iHigh) && (pBL->getPosition(false) == iLow))
	{
		iLow--;
	}
	//
	// Handle end effects of table selection.
	//
	bool bDeleteTables = !isInTable(iLow) && !isInTable(iHigh);
	PT_DocPosition iLowTable = 0;
	PT_DocPosition iHighTable = 0;
	if(!bDeleteTables && isInTable(iLow))
	{
		if(m_pDoc->isTableAtPos(iLow))
		{
				iLowTable = iLow;
		}
		else if((iLow > 0) && m_pDoc->isTableAtPos(iLow-1))
		{
				iLowTable = iLow-1;
		}		
		else if((iLow > 1) && m_pDoc->isTableAtPos(iLow-2))
		{
				iLowTable = iLow -2;
		} 
		else if((iLow > 2) &&  m_pDoc->isTableAtPos(iLow-3))
		{
				iLowTable = iLow -3;
		}
		if(iLowTable > 0)
			iLow = iLowTable;
	}
	if(!bDeleteTables && isInTable(iHigh))
	{
			if(m_pDoc->isEndTableAtPos(iHigh))
			{
					iHighTable = iHigh+1;
			}
			if(m_pDoc->isEndTableAtPos(iHigh+1))
			{
					iHighTable = iHigh+2;
			}
			if(iHighTable > 0)
				iHigh = iHighTable;
	}
	if(!bDeleteTables && (iLowTable > 0) && (iHighTable > 0))
	{
			iHigh = iHighTable;
			iLow = iLowTable;
			bDeleteTables = true;
	}
	else if(!bDeleteTables && !isInTable(iLow) && (iHighTable > 0))
	{
			iHigh = iHighTable;
			bDeleteTables = true;
	}
	else if(!bDeleteTables && !isInTable(iHigh) && (iLowTable > 0))
	{
			iLow = iLowTable;
			bDeleteTables = true;
	}
	if(!isInFrame(iLow) && isInFrame(iHigh))
	{
		fl_FrameLayout * pFL = getFrameLayout(iHigh);
		iHigh =pFL->getPosition(true);
	}
	if(isInFrame(iLow) && !isInFrame(iHigh))
	{
		fl_FrameLayout * pFL = getFrameLayout(iLow);
		iHigh =pFL->getPosition(true) + pFL->getLength() -1;
	}
	if(m_pDoc->isFrameAtPos(iLow) && m_pDoc->isEndFrameAtPos(iHigh))
	{
	        iHigh++;
	}
	_resetSelection();

	if(!bNoUpdate)
		_clearBetweenPositions(iLow, iHigh, true);

	bool bOldDelete = m_pDoc->isDontImmediateLayout();
	if(bDeleteTables || bNoUpdate)
	{
		m_pDoc->setDontImmediatelyLayout(true);
	}
	m_pDoc->beginUserAtomicGlob();
	m_pDoc->deleteSpan(iLow, iHigh, p_AttrProp_Before, iRealDeleteCount, bDeleteTables);
	//
	// Handle case of no valid block because of hidden text
	//
	pBL = getCurrentBlock();
	if(pBL && (pBL->getNextBlockInDocument() == NULL) && (pBL->getPrevBlockInDocument() == NULL))
	{
		 if(pBL->isHidden() == FP_HIDDEN_TEXT)
		 {
			  const PP_PropertyVector props = {
				  "display", ""
			  };
			  PT_DocPosition pos = pBL->getPosition();
			  PT_DocPosition posEnd = pos + pBL->getLength() -1;
			  m_pDoc->changeStruxFmt(PTC_RemoveFmt, pos, posEnd, PP_NOPROPS, props, PTX_Block);
			  m_pDoc->changeSpanFmt(PTC_RemoveFmt, pos, posEnd, PP_NOPROPS, props);
		 }
	}
//
// Stop any lists remaining if we've deleted their list fields
//
	PT_DocPosition origPos = getPoint();
	pBL = getCurrentBlock();
	if(!pBL)
	{
		// the user delete the entire document; we need to insert a new block
		// at origPos() (note that with revisions enabled / document history, this
		// position could be > 2).
		m_pDoc->insertStrux(origPos, PTX_Block);
	}
	else if(pBL->getPosition() == iLow)
	{
		pf_Frag_Strux* sdh = getCurrentBlock()->getStruxDocHandle();
		while(pBL->isListItem())
		{
			m_pDoc->StopList(sdh);
		}
	}
	if(bDeleteTables || bNoUpdate)
	{
		m_pDoc->setDontImmediatelyLayout(bOldDelete);
	}
//
// Can't leave list-tab on a line
//
	if(origPos != getPoint())
	{
		setPoint(origPos);
	}
	if(isTabListAheadPoint() == true)
	{
		UT_uint32 iRealDeleteCount2;
		m_pDoc->deleteSpan(getPoint(), getPoint()+2, p_AttrProp_Before, iRealDeleteCount2);
		iRealDeleteCount += iRealDeleteCount2;
	}
	m_pDoc->endUserAtomicGlob();

	//special handling is required for delete in revisions mode
	//where we have to move the insertion point
	if(isMarkRevisions())
	{
		UT_ASSERT( iRealDeleteCount <= iHigh - iLow + 1 );

		// if we are explicitely told to lef the caret on the left side of the
		// selection we do so, otherwise, if the point was on the left of the
		// original selection, we must adjust the point so that it is on the
		// left edge of the text to the right of what we deleted
		if(!bCaretLeft && iPoint == iLow)
			_charMotion(true,iHigh - iLow - iRealDeleteCount);
	}
//
// Make sure the insertion point is in a legal position
//
	PT_DocPosition posEnd = 0;
	getEditableBounds(true, posEnd);
	bool bOK = true;
	while(bOK && !isPointLegal() && getPoint() < posEnd)
	{
		bOK = _charMotion(true,1);
	}
	//
	// We could have gone too far!
	//
	if(getPoint() > posEnd)
	{
		setPoint(posEnd);
		PT_DocPosition posBeg = 0;
		getEditableBounds(false, posBeg);
		while(bOK && !isPointLegal() && getPoint()>=posBeg)
		{
			bOK = _charMotion(false,1);
		}
	}

	m_pG->allCarets()->enable();
}

/*!
 * Do the merge between cells.
 * If bBefore is true the contents of source will be prepended into destination otherwise
 * will e appended to the end
 */ 
bool FV_View::_MergeCells( PT_DocPosition posDestination,PT_DocPosition posSource, bool /*bBefore*/)
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
	pf_Frag_Strux* sourceSDH,*endSourceSDH,*destinationSDH,*endDestSDH;
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
	PT_DocPosition posEndDestCell = m_pDoc->getStruxPosition(endDestSDH);
//	if(!bBefore)
//	{
//		posDestination = posEndDestCell;
//	}
	m_pDoc->beginUserAtomicGlob();
	if(posEndCell > posSource)
	{
//
// OK got the doc range for the source. Set it and copy it.
//
		dr_source.set(m_pDoc,posSource,posEndCell+1);
//
// Copy to and from clipboard to populate the destination cell
//
		UT_DEBUGMSG(("SEVIOR: Copy to clipboard merging cells \n"));
		m_pApp->copyToClipboard(&dr_source);
	}
//
// Now delete the source cell. We can use the old source position since it
// just needs to point inside the table. 
//
	_deleteCellAt(posSource,sTop,sLeft);
	if(posEndCell > posSource)
	{
//
// Now paste in the text from the source cell.
//
		PD_DocumentRange dr_dest(m_pDoc,posEndDestCell,posEndDestCell);
		UT_DEBUGMSG(("SEVIOR: Pasting from clipboard merging cells \n"));
		m_pApp->pasteFromClipboard(&dr_dest,true,true);
	}
//
// Expand the destination cell into the source cell
//
	_changeCellTo(posDestination,dTop,dLeft,fLeft,fRight,fTop,fBot);
	m_pDoc->endUserAtomicGlob();

//
// We're done!
//
	return true;
}

/*!
 * This method is used to change a parameter of the table to trigger a table
 * rebuild. It also restores all the nice needed for single step undo's
 */
bool FV_View::_restoreCellParams(PT_DocPosition posTable, pf_Frag_Strux * tableSDH)
{
	//
	// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
	// with the property table-wait-index decremented by 1 and removed if it is equal to 0.
	//
	fl_TableLayout * pTL = static_cast<fl_TableLayout*>(m_pDoc->getNthFmtHandle(tableSDH,m_pLayout->getLID()));
	PP_PropertyVector propsTable = {
		"table-wait-index", ""
	};
	if (pTL->getTableWaitIndex() == 1)
	{
		m_pDoc->changeStruxFmt(PTC_RemoveFmt, posTable, posTable, PP_NOPROPS, propsTable, PTX_SectionTable);
	}
	else
	{
		propsTable[1] =	UT_std_string_sprintf("%d", pTL->getTableWaitIndex() - 1);
		m_pDoc->changeStruxFmt(PTC_AddFmt, posTable, posTable, PP_NOPROPS, propsTable, PTX_SectionTable);
	}

//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->setDontImmediatelyLayout(false);
	m_pDoc->allowChangeInsPoint();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();
	m_pDoc->endUserAtomicGlob();
	return true;
}

/*!
 *  Change the parameters of the table.
 * Return the line type of the table. We'll restore this later.
 */
bool FV_View::_changeCellParams(PT_DocPosition posTable, pf_Frag_Strux* tableSDH)
{
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(true);
	m_pDoc->setDontChangeInsPoint();
	//
	// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
	// with property table-wait-index incremented by 1 so that it is set to a value different from 0.
	//
	fl_TableLayout * pTL = static_cast<fl_TableLayout*>(m_pDoc->getNthFmtHandle(tableSDH,m_pLayout->getLID()));
	PP_PropertyVector propsTable = {
		"table-wait-index", UT_std_string_sprintf("%d", pTL->getTableWaitIndex() + 1)
	};
	m_pDoc->changeStruxFmt(PTC_AddFmt, posTable, posTable, PP_NOPROPS, propsTable, PTX_SectionTable);

	return true;
}

/*!
 * This method deletes the cell at (row,col) in the table specified by posTable
 */
bool FV_View::_deleteCellAt(PT_DocPosition posTable, UT_sint32 row, UT_sint32 col)
{
	pf_Frag_Strux* cellSDH,*endCellSDH;
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

	// if in revisions mode, we might need to move the insertion point if it was within
	// the cell that we are deleting (since the cell stays physically in the document) but
	// the positions within it are now hidden from the user)
	if(isMarkRevisions() && m_iInsPoint > posCell && m_iInsPoint < posEndCell)
	{
		_setPoint(posEndCell);
	}
	
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
	const PP_PropertyVector props = {
		"left-attach", UT_std_string_sprintf("%d", left),
		"right-attach", UT_std_string_sprintf("%d", right),
		"top-attach", UT_std_string_sprintf("%d", top),
		"bot-attach", UT_std_string_sprintf("%d", bot)
	};

//
// Here we trust that the calling routine will do all the begin/end globbing and other
// stuff so that this will go smoothly and undo's in a single step.
// Here we just change
//

	bool bres = m_pDoc->changeStruxFmt(PTC_AddFmt, posCell, posCell, PP_NOPROPS, props, PTX_SectionCell);

	return bres;
}


/*!
 * This method inserts a cell at PT_DocPosition with the given left, right, top and bottom attach.
 */
bool FV_View::_insertCellAt(PT_DocPosition posCell, UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bot,
							const PP_PropertyVector & attrsBlock, const PP_PropertyVector & propsBlock)
{
	const PP_PropertyVector props = {
		"left-attach", UT_std_string_sprintf("%d", left),
		"right-attach", UT_std_string_sprintf("%d", right),
		"top-attach", UT_std_string_sprintf("%d", top),
		"bot-attach", UT_std_string_sprintf("%d", bot)
	};

	bool bres = true;
	bres = m_pDoc->insertStrux(posCell, PTX_SectionCell, PP_NOPROPS, props);
	UT_return_val_if_fail(bres,false);
	bres = m_pDoc->insertStrux(posCell+1,PTX_Block,attrsBlock,propsBlock);
	UT_return_val_if_fail(bres,false);
	bres = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
	return bres;
}


/*!
  This method changes the attaches of the cell at position posCell
 */
bool FV_View::_changeCellAttach(PT_DocPosition posCell, UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bot)
{
	const PP_PropertyVector props = {
		"left-attach", UT_std_string_sprintf("%d", left),
		"right-attach", UT_std_string_sprintf("%d", right),
		"top-attach", UT_std_string_sprintf("%d", top),
		"bot-attach", UT_std_string_sprintf("%d", bot)
	};

	bool bres = m_pDoc->changeStruxFmt(PTC_AddFmt, posCell, posCell, PP_NOPROPS, props, PTX_SectionCell);
	return bres;
}


PT_DocPosition FV_View::_getDocPos(FV_DocPos dp, bool bKeepLooking) const
{
	return _getDocPosFromPoint(getPoint(),dp,bKeepLooking);
}

PT_DocPosition FV_View::_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, bool bKeepLooking) const
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
		if(!bRes) 
		{
			UT_WARNINGMSG(("getEditableBounds() failed in %s:%d",
						   __FILE__,__LINE__));
		}
		fl_DocSectionLayout * pDSL = m_pLayout->getFirstSection();
		if(pDSL)
		{
			fl_ContainerLayout * pCL = pDSL->getFirstLayout();
			if(pCL->getContainerType() == FL_CONTAINER_TABLE)
			{
				iPos = pCL->getPosition(true);
			}
		}
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
			pLastRun = pLastRun->getPrevRun();
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
		if(!bRes) 
		{
			UT_WARNINGMSG(("getEditableBounds() failed in %s:%d",
						   __FILE__, __LINE__));
		}
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
		if (bKeepLooking && iPos == pBlock->getPosition())
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
			fl_BlockLayout * pPrev = pBlock;
			// BOB for next block
			pBlock = pBlock->getNextBlockInDocument();
			if((pBlock->myContainingLayout()->getContainerType() == FL_CONTAINER_FRAME) && 
			   (pPrev->myContainingLayout()->getContainerType() != FL_CONTAINER_FRAME))
			{
				fl_FrameLayout * pFL = static_cast<fl_FrameLayout *>(pBlock->myContainingLayout());
				iPos = pFL->getPosition(true) -1;
			}
			else
			{
				iPos = pBlock->getPosition();
			}
		}
		else
		{
			// EOD
			bool bRes = getEditableBounds(true, iPos);
			UT_ASSERT(bRes);
			if(!bRes) 
			{
				UT_WARNINGMSG(("getEditableBounds() failed in %s:%d",
							   __FILE__, __LINE__));
			}
		}
	}
	break;

	case FV_DOCPOS_BOW:
	{
		UT_GrowBuf pgb(1024);

		UT_DebugOnly<bool> bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);

		const UT_UCSChar* pSpan = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));

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

			pSpan = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));
			offset = pgb.getLength();

			if (offset == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		if(!pSpan)
		{
			// empty block; this should not happen and will trigger one of the asserts
			// above if(offset == 0)
			return iPoint;
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

		UT_DebugOnly<bool> bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);

		const UT_UCSChar* pSpan = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));

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

			pSpan = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));
			offset = 0;

			if (pgb.getLength() == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		if(!pSpan)
		{
			// empty block; this should not happen and will trigger one of the asserts
			// above if(offset == 0)
			return iPoint;
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

		UT_DebugOnly<bool> bRes = pBlock->getBlockBuf(&pgb);
		UT_ASSERT(bRes);

		const UT_UCSChar* pSpan = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));

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

			pSpan = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(0));
			offset = 0;

			if (pgb.getLength() == 0)
			{
				iPos = pBlock->getPosition();
				break;
			}
		}

		if(!pSpan)
		{
			// empty block; this should not happen and will trigger one of the asserts
			// above if(offset == 0)
			return iPoint;
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
		UT_ASSERT_HARMLESS(UT_TODO);
		break;

	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
	if(m_bEditHdrFtr && (m_pEditShadow != NULL) && (m_FrameEdit.getFrameEditMode() == FV_FrameEdit_NOT_ACTIVE))
	{
		pBL = static_cast<fl_BlockLayout *>(m_pEditShadow->findBlockAtPosition(pos));
		if(pBL != NULL)
		{		
			UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_BLOCK);
			return pBL;
		}
	}
	pBL = m_pLayout->findBlockAtPosition(pos);
	// This assert makes debugging virtually impossible; this is a hack to make it assert only once ...
	//UT_ASSERT(pBL);
#ifdef DEBUG
	static bool bAss = false;
	if(!pBL && !bAss)
	{
		UT_ASSERT_HARMLESS( pBL );
		bAss = true;
	}
#endif
	if(!pBL)
		return NULL;
	UT_ASSERT(pBL->getContainerType() == FL_CONTAINER_BLOCK);

//
// Sevior should remove this after a while..
//
#if(1)
	if(pBL->isHdrFtr())
	{
//		  fl_HdrFtrSectionLayout * pSSL = static_cast<fl_HdrFtrSectionLayout *>(pBL->getSectionLayout());
//		  if(pSSL && pSSL->getFirstShadow())
//		  {
//			  pBL = pSSL->getFirstShadow()->findMatchingBlock(pBL);
			  if(!isLayoutFilling())
			  {
				  UT_DEBUGMSG(("SEVIOR: in view \n"));
				  //UT_ASSERT(0);
			  }
//		  }
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
// Check and adjust insPoint if it's not valid after the selection delete
//
	bool bLooked = false;
	fl_BlockLayout * pBL= getCurrentBlock();
	while(pBL && pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		bLooked = true;
		pBL = pBL->getPrevBlockInDocument();
	}
	if(pBL == NULL)
	{
		pBL= getCurrentBlock();
		while(pBL && pBL->myContainingLayout()->getContainerType() != FL_CONTAINER_DOCSECTION)
		{
			pBL = pBL->getNextBlockInDocument();
		}
	}
	if(bLooked && (pBL != NULL))
	{
		setPoint(pBL->getPosition());
	}
	else if(bLooked)
	{
		setPoint(2); // Start of document 
	}
	//
	// Get preview DocSectionLayout so we know what header/footers we have
		// to insert here.
	//
	fl_DocSectionLayout * pPrevDSL = static_cast<fl_DocSectionLayout *>(getCurrentBlock()->getDocSectionLayout());

	// insert a new paragraph with the same attributes/properties
	// as the previous (or none if the first paragraph in the section).
	// before inserting a section break, we insert a block break
	UT_uint32 iPoint = getPoint();

	m_pDoc->insertStrux(iPoint, PTX_Block);
	m_pDoc->insertStrux(iPoint, PTX_Section);

	_generalUpdate();
	_ensureInsertionPointOnScreen();
	UT_uint32 oldPoint = getPoint();
	fl_DocSectionLayout * pCurDSL = static_cast<fl_DocSectionLayout *>(getCurrentBlock()->getDocSectionLayout());
	//
	// Duplicate previous header/footers for this section.
	//
	UT_GenericVector<fl_HdrFtrSectionLayout *> vecPrevHdrFtr;
	pPrevDSL->getVecOfHdrFtrs( &vecPrevHdrFtr);
	UT_sint32 i =0;
	const PP_PropertyVector block_props = {
		"text-align", "left"
	};
	HdrFtrType hfType;
	fl_HdrFtrSectionLayout * pHdrFtrSrc = NULL;
	fl_HdrFtrSectionLayout * pHdrFtrDest = NULL;
	for(i=0; i< vecPrevHdrFtr.getItemCount(); i++)
	{
		  pHdrFtrSrc = vecPrevHdrFtr.getNthItem(i);
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
  Calculate the position of the top left corner of a page relative to the application frame.
 */
void FV_View::_getPageXandYOffset(const fp_Page* pThePage, UT_sint32& xoff, UT_sint32& yoff, bool bYOnly) const
{
	UT_sint32 iPageNumber = m_pLayout->findPage(const_cast<fp_Page*>(pThePage));
	if(iPageNumber < 0)
	{
		if (!bYOnly)
		{
			xoff = 0;
		}
		yoff = 0;
		return;
	}

	UT_sint32 y = getPageViewTopMargin();
	if ((getViewMode() == VIEW_PRINT) || (getViewMode() == VIEW_PREVIEW))
	{
		UT_sint32 iRow = iPageNumber/getNumHorizPages();
		y += iRow*(getMaxHeight(0) + getPageViewSep());
	}
	else
	{
		fl_DocSectionLayout * pDSL = getLayout()->getFirstSection();
		UT_sint32 num = iPageNumber;
		while(pDSL && num > 0)
		{
			fp_Page * pPage = pDSL->getFirstOwnedPage();
			UT_sint32 height = pPage->getHeight() - pDSL->getTopMargin() - pDSL->getBottomMargin() +getPageViewSep();
			if (num > pDSL->getPageCount())
			{
				y += pDSL->getPageCount() * height;
				num -= pDSL->getPageCount();
			}
			else
			{
				y += num * height;
				break;
			}
			pDSL = static_cast<fl_DocSectionLayout*>(pDSL->getNext());
		}
	}

	yoff = y;
	if (!bYOnly)
	{
		xoff = getWidthPrevPagesInRow(iPageNumber) + getPageViewLeftMargin();
	}
}

/*!
 * Return the next line in the document.
 */
fp_Line * FV_View::_getNextLineInDoc(fp_Container * pCon) const
{
	fp_ContainerObject * pNext = NULL;
	fl_ContainerLayout * pCL = NULL;
	fl_BlockLayout * pNextB = NULL;
	if(pCon->getContainerType() == FP_CONTAINER_CELL)
	{
		pCon = static_cast<fp_CellContainer *>(pCon)->getFirstContainer();
		if(pCon->getContainerType() == FP_CONTAINER_TABLE)
		{
			pCon = static_cast<fp_TableContainer *>(pCon)->getFirstContainer();
			return _getNextLineInDoc(pCon);
		}
		return static_cast<fp_Line *>(pCon);
	}
	if(pCon->getContainerType() != FP_CONTAINER_LINE )
	{
		pCL = static_cast<fl_ContainerLayout *>(pCon->getSectionLayout());
		pCL = pCL->getNext();
		if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
		{
			pNextB = static_cast<fl_BlockLayout *>(pCL);
		}
		else if(pCL)
	    {
			pNextB = pCL->getNextBlockInDocument();
		}
		if(pNextB)
			pNext = pNextB->getFirstContainer();
	}
	else
	{
		pNext = pCon->getNext();
		if(pNext == NULL)
		{
			pNextB = static_cast<fp_Line *>(pCon)->getBlock();
			pNextB = pNextB->getNextBlockInDocument();
			if(pNextB)
				pNext = pNextB->getFirstContainer();
		}
	}
	while(pNext && pNext->getContainerType() !=FP_CONTAINER_LINE)
	{
		pCL = static_cast<fl_ContainerLayout *>(pNext->getSectionLayout());
		pNextB = pCL->getNextBlockInDocument();
		if(pNextB)
			pNext = pNextB->getFirstContainer();
	}
	if(!pNext)
	{
		//
		// end of document
		//
		return NULL;
	}
	fp_Line * pNextLine = static_cast<fp_Line *>(pNext);
	return pNextLine;
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
	fp_VerticalContainer* pOldContainer = static_cast<fp_VerticalContainer *>(pOldLine->getContainer());
	fp_Column * pOldLeader = NULL;
	fp_Page* pOldPage = pOldLine->getPage();
	bool bDocSection = pOldSL->getType() == FL_SECTION_DOC;
	bool bEndNoteSection =	pOldSL->getType() == FL_SECTION_ENDNOTE;
	bool bFootnoteSection = pOldSL->getType() == FL_SECTION_FOOTNOTE;
	bool bCellSection = (pOldSL->getContainerType() == FL_CONTAINER_CELL);
	UT_sint32 iHoriz = getNumHorizPages();
	bool bChangedPage = false;

	if(bDocSection || bEndNoteSection || bFootnoteSection || (bCellSection && !isHdrFtrEdit()))
	{
		pOldLeader = (static_cast<fp_Column*>(pOldLine->getColumn()))->getLeader();
	}

	UT_sint32 iPageOffset;
	getPageYOffset(pOldPage, iPageOffset);

	UT_sint32 iLineX = 0;
	UT_sint32 iLineY = 0;
	UT_sint32 xOldpage = 0;
	UT_sint32 xNewpage = 0;

	pOldContainer->getOffsets(static_cast<fp_Container *>(pOldLine), iLineX, iLineY);
	yPoint = iLineY;

	iLineHeight = pOldLine->getHeight();

	bool bNOOP = false;
	bool bEOL = false, bBOL = false;
	fp_Line * pNextLine = NULL; 
	fp_Page* pPage = NULL;
	xxx_UT_DEBUGMSG(("fv_View::_moveInsPtNextPrevLine: old line 0x%x\n", pOldLine));

	if (bNext)
	{
		if (pOldLine != static_cast<fp_Line *>(pOldContainer->getLastContainer()))
		{
			UT_sint32 iAfter = m_pG->tlu(1);
			yPoint += (iLineHeight + iAfter);
		}
		else if (bDocSection)
		{
			UT_sint32 count = static_cast<UT_sint32>(pOldPage->countColumnLeaders());
			UT_sint32 i = 0;
			for(i =0; i < count ;i++)
			{
				if( static_cast<fp_Column *>(pOldPage->getNthColumnLeader(i)) == pOldLeader)
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
				pPage = pOldPage->getNext();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = 0;
					if(iHoriz > 1)
					{
						xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
						xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
						 m_xPointSticky =  m_xPointSticky - (xOldpage-xNewpage);
						 bChangedPage = true;
					}
				}
				else
				{
					bNOOP = true;
					if (_getDocPosFromPoint(iOldPoint, FV_DOCPOS_EOL) != iOldPoint)
					  bEOL = true;
				}
			}
		}
		else if(bCellSection)
		{
			if (iHoriz <= 1)
			{
				UT_sint32 iAfter = m_pG->tlu(1);
				fp_CellContainer * pCell =  static_cast<fp_CellContainer*> (pOldLine->getContainer());
				fp_TableContainer * pTab = static_cast<fp_TableContainer*> (pCell->getContainer());
				if (pCell->getLastContainer() == pOldLine)
				{
					yPoint += pTab->getYOfRow(pCell->getBottomAttach()) - pTab->getYOfRow(pCell->getTopAttach());
					yPoint += iAfter - pOldLine->getY();
				}
				else
				{
					yPoint += (iLineHeight + iAfter);
				}
			}
			else
			{
				fp_CellContainer * pCell =  static_cast<fp_CellContainer*> (pOldLine->getContainer());
			    if(pCell->getLastContainer() == pOldLine)
				{
					fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCell->getContainer());
					UT_sint32 iBotPrev = pCell->getBottomAttach();
					if(iBotPrev > pTab->getNumRows())
					{
						pNextLine = _getNextLineInDoc(pTab);
					}
					else
				    {
						pCell = pTab->getCellAtRowColumn(iBotPrev,pCell->getLeftAttach());
						if(!pCell)
						{
							pNextLine = _getNextLineInDoc(pTab);
						}
						else
						{
							pNextLine = _getNextLineInDoc(pCell);
						}
					}
				}
				else
				{
					pNextLine =  _getNextLineInDoc(pOldLine);
				}
				if(!pNextLine)
					return;
				pPage = pNextLine->getPage();
				if(pPage && (pPage != pOldPage))
				{
					xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
					xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
					m_xPointSticky =  m_xPointSticky - ( xOldpage -xNewpage);
					if(pPage != pOldPage)
					{
						getPageYOffset(pPage, iPageOffset);
						yPoint = 0;
					}
					bChangedPage = true;
				}
			}
		}
		else if(bEndNoteSection)
		{
			UT_sint32 iAfter = m_pG->tlu(1);
			yPoint += (iLineHeight + iAfter);
			if(pOldPage->getBottom() < yPoint)
			{
				// move to next page
				pPage = pOldPage->getNext();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = 0;
					if(iHoriz > 1)
					{
						xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
						xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
						 m_xPointSticky =  m_xPointSticky - (xOldpage-xNewpage);
						 bChangedPage = true;
					}
				}
			}
		}
		else if(bFootnoteSection)
		{
			UT_sint32 iAfter = m_pG->tlu(1);
			yPoint += (iLineHeight + iAfter);
			if(pOldPage->getBottom() < yPoint)
			{
				// move to next page
				pPage = pOldPage->getNext();
				if (pPage)
				{
					xxx_UT_DEBUGMSG(("Move to next page old IpageOffset %d ypoint %d \n",iPageOffset,yPoint));
					getPageYOffset(pPage, iPageOffset);
					xxx_UT_DEBUGMSG(("Move to next page new IpageOffset %d \n",iPageOffset));
					yPoint = 0;
					if(iHoriz > 1)
					{
						xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
						xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
						 m_xPointSticky =  m_xPointSticky - (xOldpage-xNewpage);
						 bChangedPage = true;
					}
				}
			}
		}
		else
		{
			bNOOP = true;
		}
	}
	else
	{
		if (pOldLine != static_cast<fp_Line *>(pOldContainer->getFirstContainer()))
		{
			// just move off this line
			yPoint -= (pOldLine->getMarginBefore() + 1);
		}
		else if (bDocSection)
		{
			UT_sint32 count = static_cast<UT_sint32>(pOldPage->countColumnLeaders());
			UT_sint32 i = 0;
			for(i =0; i < count ;i++)
			{
				if( static_cast<fp_Column *>(pOldPage->getNthColumnLeader(i)) == pOldLeader)
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
				pPage = pOldPage->getPrev();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = pPage->getBottom();
					if(getViewMode() != VIEW_PRINT)
					{
						fl_DocSectionLayout * pDSL = pPage->getOwningSection();
						yPoint = yPoint - pDSL->getTopMargin() -2;
					}
					if(iHoriz > 1)
					{
						xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
						xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
						 m_xPointSticky =  m_xPointSticky - (xOldpage-xNewpage);
						 bChangedPage = true;
					}
				}
				else
				{
					bNOOP = true;
					if (_getDocPosFromPoint(iOldPoint, FV_DOCPOS_BOL) != iOldPoint)
					  bBOL = true;
				}
			}
		}
		else if(bCellSection)
		{
			UT_sint32 iAfter = m_pG->tlu(2);
			yPoint -= iAfter;
			if(yPoint < 0)
			{
				// move to prev page
				pPage = pOldPage->getPrev();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = pPage->getBottom();
					if(getViewMode() != VIEW_PRINT)
					{
						fl_DocSectionLayout * pDSL = pPage->getOwningSection();
						yPoint = yPoint - pDSL->getTopMargin() -2;
					}
					if(iHoriz > 1)
					{
						xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
						xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
						 m_xPointSticky =  m_xPointSticky - (xOldpage-xNewpage);
						 bChangedPage = true;
					}
				}
			}

		}
		else if(bEndNoteSection || bFootnoteSection)
		{
			UT_sint32 iAfter = m_pG->tlu(2);
			yPoint -= iAfter;

			// change to screen coordinates
			UT_sint32 xP = m_xPointSticky - m_xScrollOffset + getPageViewLeftMargin();
			UT_sint32 yP = yPoint + iPageOffset - m_yScrollOffset;
			
			// hit-test to figure out where that puts us
			UT_sint32 xC, yC;

			PT_DocPosition iNewP;
			fp_Page* pPage2 = _getPageForXY(xP, yP, xC, yC);
			bool isTOC = false;
			pPage2->mapXYToPosition(xC, yC, iNewP, bBOL, bEOL,isTOC);
			UT_sint32 ii =0;
			while((iNewP == iOldPoint) && (ii < 100) && (yPoint > 0))
			{
				yPoint -= iAfter;
				yP = yPoint + iPageOffset - m_yScrollOffset;
				pPage2 = _getPageForXY(xP, yP, xC, yC);
				pPage2->mapXYToPosition(xC, yC, iNewP, bBOL, bEOL,isTOC);
				ii++;
			}
			if(yPoint < 0)
			{
				// move to prev page
				pPage = pOldPage->getPrev();
				if (pPage)
				{
					getPageYOffset(pPage, iPageOffset);
					yPoint = pPage->getBottom();
					if(getViewMode() != VIEW_PRINT)
					{
						fl_DocSectionLayout * pDSL = pPage->getOwningSection();
						yPoint = yPoint - pDSL->getTopMargin() -2;
					}
					if(iHoriz > 1)
					{
						xOldpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pOldPage)));
						xNewpage = static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage)));
						 m_xPointSticky =  m_xPointSticky - (xOldpage-xNewpage);
						 bChangedPage = true;
					}
				}
			}
		}
		else
		{
			bNOOP = true;
		}
	}

	if (bNOOP)
	{
		if (bBOL)
			moveInsPtTo(FV_DOCPOS_BOL, false);
		else if (bEOL)
			moveInsPtTo(FV_DOCPOS_EOL, false);
		// cannot move.  should we beep?
		return;
	}
	// hit-test to figure out where that puts us
	UT_sint32 xClick, yClick;
	if(!pNextLine)
	{
		// change to screen coordinates
		xPoint = m_xPointSticky - m_xScrollOffset + getPageViewLeftMargin();
		yPoint += iPageOffset - m_yScrollOffset;

		pPage = _getPageForXY(xPoint, yPoint, xClick, yClick);
	}
	else
	{
		UT_Rect * pOldRec = pOldLine->getScreenRect();
		UT_Rect * pNewRec = pNextLine->getScreenRect();
		yPoint = pNewRec->top + pNewRec->height/2;
		xPoint = xPoint + (pNewRec->left - pOldRec->left);
		pPage = _getPageForXY(xPoint, yPoint, xClick, yClick);
		delete pOldRec;
		delete pNewRec;
	}
	PT_DocPosition iNewPoint = 0;
	bBOL = false;
	bEOL = false;
	fl_HdrFtrShadow * pShadow=NULL;
//
// If we're not in a Header/Footer we can't get off the page with the click
// version of mapXYToPosition
//
	bool isTOC = false;
	if(isHdrFtrEdit())
	{
		bool bGotIt = false;
		UT_sint32 iLoop = 0;
		while(!bGotIt && (iLoop < 50))
		{
			pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL,isTOC, true, &pShadow);
			if(iNewPoint != iOldPoint)
			{
				bGotIt = true;
			}
			else
			{
				iLoop++;
				if(bNext)
				{
					yClick += m_pG->tlu(1);
				}
				else
				{
					yClick -= m_pG->tlu(1);
				}
			}
		}
	}
	else
	{
		pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL,isTOC);
		if (bNext)
		{
			int delta = iLineHeight;
//
// Whoops! foornotes/endnotes can screw this up since they placed on the page
// out of order to their location in the PieceTable
//
			while (iNewPoint <= getPoint() && (getCurrentPage() == pPage) )
			{
				if (yClick+delta > pPage->getHeight())
				{
					pPage = pPage->getNext();
					if (!pPage)
					{
						return;
					}
					delta = -yClick;
				}
				pPage->mapXYToPosition(xClick, yClick+delta, 
									   iNewPoint, bBOL, bEOL,isTOC);
				delta += iLineHeight;
			}
		}
		else
		{
			int delta = iLineHeight;
//
// Whoops! foornotes/endnotes can screw this up since they placed on the page
// out of order to their location in the PieceTable
//
			while (iNewPoint >= getPoint() && (getCurrentPage() == pPage) )
			{
				if (yClick-delta < 0)
				{
					pPage = pPage->getPrev();
					if (!pPage)
					{
						return;
					}
					delta = yClick - pPage->getBottom();
				}
				pPage->mapXYToPosition(xClick, yClick-delta, 
									   iNewPoint, bBOL, bEOL,isTOC);
				delta += iLineHeight;
			}
		}

		while(pPage && (iNewPoint == iOldPoint) && (yClick < m_pLayout->getHeight()) && (yClick > 0))
		{
			if (bNext)
				yClick += iLineHeight/2;
			else
				yClick -= m_pG->tlu(2);

			if(yClick > pPage->getHeight())
			{
				pPage = pPage->getNext();
				if (!pPage) break;
				yClick -= pPage->getHeight();
			}

			if(yClick < 0)
			{
				pPage = pPage->getPrev();
				if (!pPage) break;
				yClick += pPage->getHeight();
			}

			if(pPage)
				pPage->mapXYToPosition(xClick, yClick, iNewPoint, bBOL, bEOL,isTOC);
		}
		xxx_UT_DEBUGMSG((" pPage %x iNewPoint %d iOldPoint %d yClick %d m_pLayout->getHeight() \n",pPage,iNewPoint,yClick,m_pLayout->getHeight()));
	}
//
// Check we're not moving out of allowed region.
//
	PT_DocPosition posBOD,posEOD;
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);

	xxx_UT_DEBUGMSG(("iNewPoint=%d, iOldPoint=%d, xClick=%d, yClick=%d\n",iNewPoint, iOldPoint, xClick, yClick));
	UT_ASSERT(iNewPoint != iOldPoint);
	if((iNewPoint >= posBOD) && (iNewPoint <= posEOD))
	{
		_setPoint(iNewPoint, bEOL);
	}

	_ensureInsertionPointOnScreen();

	// this is the only place where we override changes to m_xPointSticky
	if(!bChangedPage || (pPage == pOldPage))
	   m_xPointSticky = xOldSticky;
}

/*! Scrolls the screen to make sure that the IP is on-screen.
 * \return true iff scrolling took place
 * Q: should this get called if there is a selection?  Does it do
 * harm?  It may, because point may be the beginning of the selection.
 */
bool FV_View::_ensureInsertionPointOnScreen()
{
	xxx_UT_DEBUGMSG(("FV_View::_ensureInsertionPointOnScreen called windowHeight %d Point %d \n",getWindowHeight(),getPoint()));

	// Some short circuit tests to avoid doing bad things.
	if (getWindowHeight() <= 0)
		return false;

    // If == 0 no layout information is present. Don't scroll.
	if(getPoint() == 0)
		return false;

	xxx_UT_DEBUGMSG(("_ensure: [xp %ld][yp %ld][ph %ld] [w %ld][h %ld]\n",m_xPoint,m_yPoint,m_iPointHeight,getWindowWidth(),getWindowHeight()));

	bool bRet = false;
	if (m_yPoint < 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEUP, (UT_uint32) (-(m_yPoint)));
		bRet = true;
	}
	else if ((static_cast<UT_uint32>(m_yPoint + m_iPointHeight)) >= (static_cast<UT_uint32>(getWindowHeight())))
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_uint32>(m_yPoint + m_iPointHeight - getWindowHeight()));
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
	else if ((static_cast<UT_uint32>(m_xPoint)) >= (static_cast<UT_uint32>(getWindowWidth())))
	{
		cmdScroll(AV_SCROLLCMD_LINERIGHT, static_cast<UT_uint32>(m_xPoint - getWindowWidth() + getPageViewLeftMargin()/2));
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
	fp_Page* pPage = 0;

	if (pOldPage)
	{
		pPage = (bNext ? pOldPage->getNext() : pOldPage->getPrev());
	}

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

void FV_View::_moveInsPtNextPrevScreen(bool bMovingDown, bool bClearSelection)
{
	fl_BlockLayout * pBlock;
	fp_Run * pRun;
	UT_sint32 x,y,x2,y2;
	UT_uint32 iHeight;
	bool bDirection;
	bool bBOL,bEOL,isTOC;
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

    // get Screen coordinates of the top of the page and add the y location to this.
	getPageScreenOffsets(pPage, xoff,yoff);
	yoff = y - yoff;

	UT_sint32 iDir = bMovingDown ? 1 : -1;

	iYnext = yoff + getWindowHeight() * iDir;
	iYscroll = m_yScrollOffset + (getWindowHeight() * iDir);
	if (iYscroll < 0)
	{
		// We're trying to scroll past beginning/end of document
		// Move insertion pointer to BOD/EOD instead
		if (iDir == 1)
		{
			moveInsPtTo(FV_DOCPOS_EOD, bClearSelection);
		}
		else
		{
			moveInsPtTo(FV_DOCPOS_BOD, bClearSelection);
		}
		return;
	}

	xxx_UT_DEBUGMSG(("SEVIOR:!!!!!! Yoff %d iYnext %d page %x \n",yoff,iYnext,pPage));

	while (pPage && (bMovingDown ? (iYnext > pPage->getHeight())
					             : (iYnext < 0)))
	{
		iYnext -= (pPage->getHeight() + getPageViewSep()) * iDir;
		pPage = bMovingDown ? pPage->getNext() : pPage->getPrev();
	}

	xxx_UT_DEBUGMSG(("SEVIOR:!!!!!! Set to iYnext %d page %x \n",iYnext,pPage));

	if (pPage == NULL) pPage = pLine->getPage ();
	if (iYnext < 0) iYnext = 0;
	// convert the iYnext back into a point position, namely iNewPoint.
	pPage->mapXYToPosition(x, iYnext, iNewPoint, bBOL, bEOL,isTOC);
	if (bMovingDown)
	{
		int delta = pPage->getHeight()/4;
		while (iNewPoint <= getPoint() && pPage)
		{
			if (iYnext+delta > pPage->getHeight())
			{
				delta -= pPage->getHeight();
				pPage = pPage->getNext();
			}
			if (pPage)
			{
				pPage->mapXYToPosition(x, iYnext+delta, 
									   iNewPoint, bBOL, bEOL,isTOC);
				delta += pPage->getHeight()/4;
			}
		}
	}
	else
	{
		int delta = pPage->getHeight()/4;
		while (iNewPoint >= getPoint() && pPage)
		{
			if (iYnext+delta < 0)
			{
				delta += pPage->getHeight();
				pPage = pPage->getPrev();
			}
			if (pPage)
			{
				pPage->mapXYToPosition(x, iYnext-delta, 
									   iNewPoint, bBOL, bEOL,isTOC);
				delta += pPage->getHeight()/4;
			}
		}
	}

	UT_sint32 newX,newY;
	UT_uint32 newHeight;
	
	_findPositionCoords(iNewPoint,false,newX,newY,x2,y2,newHeight,bDirection,&pBlock,&pRun);
	if(!pRun)
	{
		_moveInsPtNextPrevLine(bMovingDown);
		return;
	}

	fp_Line * pNewLine = static_cast<fp_Line *>(pRun->getLine());

	if(pNewLine == NULL ||
	   ((pNewLine->getContainer() == pLine->getContainer()) && 
	   (bMovingDown ? (pNewLine->getY() < pLine->getY())
		: (pNewLine->getY() > pLine->getY()))))
	{
		_moveInsPtNextPrevLine(bMovingDown);
		return;
	}

    // Couldn't advance! Try scanning x across the page at this new iYnext.
	if(pLine == pNewLine && pPage)
	{
		UT_sint32 step = pPage->getWidth()/20 + 1;

		for (x=0; x < pPage->getWidth(); x += step)
		{
			pPage->mapXYToPosition(x, iYnext, iNewPoint, bBOL, bEOL,isTOC);
			_findPositionCoords(iNewPoint,false,newX,newY,x2,y2,newHeight,bDirection,&pBlock,&pRun);
			pNewLine = static_cast<fp_Line *>(pRun->getLine());
			if(pLine != pNewLine)
				break;
		}

		if (pLine == pNewLine)
		{
			_moveInsPtNextPrevLine(bMovingDown);
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


fp_Page *FV_View::_getCurrentPage(void) const
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
	UT_return_val_if_fail ( pOldRun, 0 );
	fp_Line* pOldLine = pOldRun->getLine();
	fp_Page* pOldPage = pOldLine->getPage();
	return pOldPage;
}

void FV_View::_moveInsPtNthPage(UT_sint32 n)
{
	fp_Page *page = m_pLayout->getFirstPage();

	if (n > m_pLayout->countPages ())
		n = m_pLayout->countPages ();

	for (UT_sint32 i = 1; i < n; i++)
	{
		page = page->getNext ();
	}

	_moveInsPtToPage(page);
}

void FV_View::_moveInsPtToPage(fp_Page *page)
{
	UT_return_if_fail(page);

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
		cmdScroll(AV_SCROLLCMD_LINEUP, static_cast<UT_uint32>(-iPageOffset));
		bVScroll = true;
	}
	else if (iPageOffset > 0)
	{
		cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_uint32>(iPageOffset));
		bVScroll = true;
	}

	// also allow implicit horizontal scroll, if needed
	if (!_ensureInsertionPointOnScreen() && !bVScroll)
	{
		_fixInsertionPointCoords();
	}
}

static bool bScrollRunning = false;
static UT_Worker * s_pScroll = NULL;

void FV_View::_actuallyScroll(UT_Worker * pWorker)
{

	FV_View * pView = static_cast<FV_View *>(pWorker->getInstanceData());
	UT_return_if_fail(pView);
	if(pView->getLayout()->getDocument()->isPieceTableChanging())
	{
		return;
	}

	PT_DocPosition iOldPoint = pView->getPoint();
	UT_DEBUGMSG(("Doing autoscroll \n"));
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

		if ((xPos < 0 || xPos > pView->getWindowWidth()) ||
			(yPos < 0 || yPos > pView->getWindowHeight()))
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
			//			 xPos,yPos,pView->getWindowWidth(),pView->getWindowHeight()));
			//
			// Sevior: Is This what you wanted? Uncomment these lines when
			// needed.
			//
			//XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
			//UT_ASSERT((pFrame));

			if (yPos < 0)
			{
				pView->cmdScroll(AV_SCROLLCMD_LINEUP, static_cast<UT_sint32>(-yPos));
			}
			else if ((static_cast<UT_uint32>(yPos)) >= (static_cast<UT_uint32>(pView->getWindowHeight())))
			{
				pView->cmdScroll(AV_SCROLLCMD_LINEDOWN, static_cast<UT_sint32>(yPos - pView->getWindowHeight()));
			}

			if (xPos < 0)
			{
				pView->cmdScroll(AV_SCROLLCMD_LINELEFT, (UT_uint32) (-(xPos)));
			}
			else if ((static_cast<UT_uint32>(xPos)) >= (static_cast<UT_uint32>(pView->getWindowWidth())))
			{
				pView->cmdScroll(AV_SCROLLCMD_LINERIGHT, static_cast<UT_uint32>(xPos - pView->getWindowWidth()));
			}
		}
	}
	s_pScroll->stop();
	delete s_pScroll;
	s_pScroll = NULL;
	bScrollRunning = false;
}

void FV_View::_autoScroll(UT_Worker * pWorker)
{
	UT_return_if_fail(pWorker);
	if(bScrollRunning)
		{
			UT_DEBUGMSG(("Dropping autoscroll !!!!!!! \n"));
			return;
		}
	// this is a static callback method and does not have a 'this' pointer.

	FV_View * pView = static_cast<FV_View *>(pWorker->getInstanceData());
	UT_return_if_fail(pView);
	if(pView->getLayout()->getDocument()->isPieceTableChanging())
	{
		return;
	}

	int inMode = UT_WorkerFactory::IDLE | UT_WorkerFactory::TIMER;
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	s_pScroll = UT_WorkerFactory::static_constructor (_actuallyScroll,pView, inMode, outMode);

	// If the worker is working on a timer instead of in the idle
	// time, set the frequency of the checks.
	if ( UT_WorkerFactory::TIMER == outMode )
	{
		// this is really a timer, so it's safe to static_cast it
		static_cast<UT_Timer*>(s_pScroll)->set(1);
	}
	bScrollRunning = true;
	s_pScroll->start();
}

/*! Returns the page the user's mouse pointer is in.
 * 
 */
fp_Page* FV_View::_getPageForXY(UT_sint32 xPos, UT_sint32 yPos, UT_sint32& xClick, UT_sint32& yClick) const
{
	xClick = xPos + m_xScrollOffset - getPageViewLeftMargin();
	yClick = yPos + m_yScrollOffset - getPageViewTopMargin();
	fp_Page* pPage = m_pLayout->getFirstPage();
	
	if (xClick <= (signed)getWidthPagesInRow(pPage))  //So we can't select the next row by clicking outside 
	{
		while (pPage) //First, find the row the page is in
		{
			UT_uint32 iNumHorizPages = getNumHorizPages();
			UT_sint32 iPageHeight = pPage->getHeight();
			
			if(getViewMode() != VIEW_PRINT)
			{
				iPageHeight = iPageHeight - pPage->getOwningSection()->getTopMargin() -
					pPage->getOwningSection()->getBottomMargin();
			}
			
			if (yClick < iPageHeight)
			{
				//Found the first page in the row
				break;
			}
			else
			{
				yClick -= iPageHeight + getPageViewSep();
			}
			
			//Loop because we're jumping entire rows
			for (unsigned int i = 0; i < iNumHorizPages; i++)
			{
				if (pPage->getNext())
				{
					pPage = pPage->getNext();
				}
			}
		}
		
		while (pPage) //Now, find the page in the row.
		{
			UT_sint32 iPageWidth = pPage->getWidth();
			
			if (xClick > iPageWidth && !rtlPages()) // Left to right
			{
				xClick -= iPageWidth + getHorizPageSpacing();
			}
			else if (xClick < static_cast<UT_sint32>(getWidthPrevPagesInRow(m_pLayout->findPage(pPage))) && rtlPages()) // Right to left
			{
				//Don't need to do anything.
			}
			else
			{
				if (rtlPages())
				{
					xClick -= getWidthPrevPagesInRow(m_pLayout->findPage(pPage));
				}
				
				//Found the page. Huzzah!
				xxx_UT_DEBUGMSG(("     yClick %d \t     xClick %d\tPage %d\n", yClick, xClick, m_pLayout->findPage(pPage)));
				xxx_UT_DEBUGMSG(("iPageHeight %d \t iPageWidth %d | %d\n", pPage->getHeight(), iPageWidth, getWidthPagesInRow(pPage)));
				break;
			}
			pPage = pPage->getNext();
		}
	}

	if (!pPage)
	{
		// we're below the last page
		pPage = m_pLayout->getLastPage();
		if(pPage == NULL)
	    {
			pPage = m_pLayout->getFirstPage();
		}
		if(pPage == NULL)
		{
			return pPage;
		}
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
FV_View::_computeFindPrefix(const UT_UCSChar* pFind)
{
	UT_uint32 m = UT_UCS4_strlen(pFind);
	UT_uint32 k = 0, q = 1;
	UT_uint32 *pPrefix = (UT_uint32*) UT_calloc(m + 1, sizeof(UT_uint32));
	UT_return_val_if_fail(pPrefix, NULL);

	pPrefix[0] = 0; // Must be this regardless of the string

	if (m_bMatchCase)
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

static inline UT_UCS4Char s_smartQuoteToPlain(UT_UCS4Char currentChar)
{
	switch(currentChar)
	{
		case 0x201A:     //single low 9 quotation
		case 0x201B:     //single reverse comma quotation
		case UCS_LQUOTE:
		case UCS_RQUOTE: return (UT_UCS4Char) '\'';

		case 0x201E:     //double low 9 quotation
		case 0x201F:     //double reverse comma quotation
		case UCS_LDBLQUOTE:
		case UCS_RDBLQUOTE: return (UT_UCS4Char) '\"';

		default: return currentChar;
	}

	return currentChar;
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
FV_View::_findNext(UT_uint32* pPrefix,
				   bool& bDoneEntireDocument)
{
	UT_ASSERT(m_sFind);

	fl_BlockLayout* block = _findGetCurrentBlock();
	PT_DocPosition offset = _findGetCurrentOffset();
	UT_UCSChar* buffer = NULL;
	UT_uint32 m = UT_UCS4_strlen(m_sFind);

	// Clone the search string, converting it to lowercase is search
	// should ignore case.
	UT_UCSChar* pFindStr = (UT_UCSChar*) UT_calloc(m, sizeof(UT_UCSChar));
	UT_return_val_if_fail(pFindStr,false);

	UT_uint32 j;
	if (m_bMatchCase)
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = m_sFind[j];
	}
	else
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = UT_UCS4_tolower(m_sFind[j]);
	}

	// Now we use the prefix function (stored as an array) to search
	// through the document text.
	while ((buffer = _findGetNextBlockBuffer(&block, &offset)))
	{
		UT_sint32 foundAt = -1;
		UT_uint32 i = 0, t = 0;

		UT_UCSChar currentChar;
	
		while ((currentChar = buffer[i]) /*|| foundAt == -1*/)
		{
			// Convert smart quote to plain equivalent
			// for smart quote matching
			UT_UCS4Char cPlainQuote = s_smartQuoteToPlain(currentChar);
			
			if (!m_bMatchCase) currentChar = UT_UCS4_tolower(currentChar);
			
			while (t > 0 && pFindStr[t] != currentChar && pFindStr[t] != cPlainQuote)
				t = pPrefix[t-1];
			if (pFindStr[t] == currentChar || pFindStr[t] == cPlainQuote)
				t++;
			i++;
			if (t == m)
			{
				if (m_bWholeWord) 
				{
					bool start = true;
					if((static_cast<UT_sint32>(i) - static_cast<UT_sint32>(m)) > 0)
						start = UT_isWordDelimiter(buffer[i-m-1], UCS_UNKPUNK, UCS_UNKPUNK);
					bool end = UT_isWordDelimiter(buffer[i], UCS_UNKPUNK, UCS_UNKPUNK);
					if (start && end) 
					{
						foundAt = i - m;
						break;
					}
				}
				else 
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
		offset += UT_MAX(UT_UCS4_strlen(buffer),1);

		// Must clean up buffer returned for search
		FREEP(buffer);
	}

	bDoneEntireDocument = true;

	// Reset wrap for next time
	m_wrappedEnd = false;

	FREEP(pFindStr);

	return false;
}

bool
FV_View::_findPrev(UT_uint32* /*pPrefix*/,
				   bool& bDoneEntireDocument)
{
	UT_ASSERT(m_sFind);

	fl_BlockLayout* block = _findGetCurrentBlock();
	PT_DocPosition offset = _findGetCurrentOffset();
	UT_UCSChar* buffer = NULL;
	UT_uint32 m = UT_UCS4_strlen(m_sFind);

	// Clone the search string, converting it to lowercase is search
	// should ignore case.
	UT_UCSChar* pFindStr = (UT_UCSChar*) UT_calloc(m, sizeof(UT_UCSChar));
	UT_return_val_if_fail(pFindStr,false);

	UT_uint32 j;
	if (m_bMatchCase)
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = m_sFind[j];
	}
	else
	{
		for (j = 0; j < m; j++)
			pFindStr[j] = UT_UCS4_tolower(m_sFind[j]);
	}

	// Now we use the prefix function (stored as an array) to search
	// through the document text.
	UT_sint32 endIndex = 0;
	while ((buffer = _findGetPrevBlockBuffer(&block, &offset, endIndex)))
	{
		UT_sint32 foundAt = -1;
		UT_uint32 i = UT_MIN(offset, UT_UCS4_strlen(buffer));
		if (i > m) 
		{
			i-=m;
		}
		else 
		{
			if (i==0)
				i = UT_UCS4_strlen(buffer);
			else
				i=0;
		}
			
		UT_uint32 t = 0;
		UT_UCSChar currentChar;

		while (i!=UT_uint32(-1))
		{
			t = 0;
			currentChar = buffer[i];
			UT_UCS4Char cPlainQuote = s_smartQuoteToPlain(currentChar);

			if (!m_bMatchCase) currentChar = UT_UCS4_tolower(currentChar);
			while (((m_sFind[t] == currentChar)||(m_sFind[t] == cPlainQuote))&& (t < m))
			{
				t++;
				currentChar = buffer[i + t];
				cPlainQuote = s_smartQuoteToPlain(currentChar);
				if (!m_bMatchCase) currentChar = UT_UCS4_tolower(currentChar);
			}	
				
			if (t == m) {
				if (m_bWholeWord)
				{
					bool start = UT_isWordDelimiter(buffer[i-1], UCS_UNKPUNK, UCS_UNKPUNK);
					bool end = UT_isWordDelimiter(buffer[i+m], UCS_UNKPUNK, UCS_UNKPUNK);
					if (start && end)
					{
						foundAt = i;
						break;
					}
				}
				else
				{
					foundAt = i;
					break;
				}
			}
		
			i--;
		}

		// Select region of matching string if found
		if (foundAt >= 0)
		{

			UT_DEBUGMSG(("Found pos: %d", (block)->getPosition(false)+ foundAt));
			UT_DEBUGMSG((" - len: %d\n", m));
			
			_setPoint(block->getPosition(false) + foundAt + m);
			_setSelectionAnchor();
			_charMotion(false, m);

			m_doneFind = true;

			FREEP(pFindStr);
			FREEP(buffer);
			return true;
		}

		// Didn't find anything, so set the offset to the start of the
		// current area (0)
		offset = 0;

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
FV_View::_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset) const
{
	UT_return_val_if_fail(block, 0);
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
		bool bNeedNewBlock = true;
		
		// if pBlock was inside some kind of an embeded section, we need to make sure we
		// have finished searching the enclosing block
		if((*pBlock)->isEmbeddedType())
		{
			
			fl_ContainerLayout * pCL = (*pBlock)->myContainingLayout();
			UT_ASSERT((pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) || (pCL->getContainerType() == FL_CONTAINER_ENDNOTE) );
			fl_EmbedLayout * pFL = static_cast<fl_EmbedLayout *>(pCL);
			if(pFL->isEndFootnoteIn())
			{
				pf_Frag_Strux* sdhStart = pCL->getStruxDocHandle();
				pf_Frag_Strux* sdhEnd = NULL;
				if(pCL->getContainerType() == FL_CONTAINER_FOOTNOTE)
				{
					getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote, &sdhEnd);
				}
				else
				{
					getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote, &sdhEnd);
				}

				if(sdhEnd)
				{
					PT_DocPosition posStart = getDocument()->getStruxPosition(sdhStart);
					fl_ContainerLayout*  psfh = NULL;
					getDocument()->getStruxOfTypeFromPosition((*pBlock)->getDocLayout()->getLID(),posStart,PTX_Block, &psfh);
					newBlock = static_cast<fl_BlockLayout *>(psfh);

					PT_DocPosition iPos = _BlockOffsetToPos(*pBlock, *pOffset);
					PT_DocPosition iEncBlockPos = newBlock->getPosition(false);

					newOffset = iPos - iEncBlockPos;

					pBuffer.truncate(0);
					
					if (!newBlock->getBlockBuf(&pBuffer))
					{
						UT_DEBUGMSG(("Block %p (a ->next block) has no buffer.\n",
									 newBlock));
						UT_ASSERT(0);
					}

					if(pBuffer.getLength() > newOffset)
					{
						// still stuff left in our containing block
						bNeedNewBlock = false;
					}
					
				}
			}
		}

		if(bNeedNewBlock)
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
		
	}
	else
	{
		// We have some left to go in this buffer.	Buffer is still
		// valid, just copy pointers
		newBlock = *pBlock;
		newOffset = *pOffset;
	}

	if(newBlock == *pBlock
	   && (newBlock->getPosition(false) + pBuffer.getLength()) < m_startPosition)
	{
		// this happens if the document shrinks in the process of replacement
		// we get the same block, but it is shorter than the stored m_startPosition
		return NULL;
	}
			
	// Are we going to run into the start position in this buffer?	If
	// so, we need to size our length accordingly
	if (m_wrappedEnd && _BlockOffsetToPos(newBlock, newOffset) + pBuffer.getLength() >= m_startPosition)
	{
		// sanity check
		if(m_startPosition > (newBlock)->getPosition(false) + newOffset)
			bufferLength = (m_startPosition - (newBlock)->getPosition(false)) - newOffset;
	}
	else if(pBuffer.getLength() > newOffset)
	{
		bufferLength = pBuffer.getLength() - newOffset;
	}

	// clone a buffer (this could get really slow on large buffers!)
	UT_UCSChar* bufferSegment = NULL;

	// remember, the caller gets to g_free this memory
	bufferSegment = static_cast<UT_UCSChar*>(UT_calloc(bufferLength + 1, sizeof(UT_UCSChar)));
	UT_ASSERT(bufferSegment);

	memmove(bufferSegment, pBuffer.getPointer(newOffset),
			(bufferLength) * sizeof(UT_UCSChar));

	// before we bail, hold up our block stuff for next round
	*pBlock = newBlock;
	*pOffset = newOffset;

	return bufferSegment;
}

UT_UCSChar*
FV_View::_findGetPrevBlockBuffer(fl_BlockLayout** pBlock,
								 PT_DocPosition* pOffset,
								 UT_sint32& endIndex)
{
	endIndex = 0;
	UT_return_val_if_fail(m_pLayout && pBlock && *pBlock && pOffset,NULL);

	fl_BlockLayout* newBlock = NULL;
	PT_DocPosition newOffset = 0;
	
	UT_uint32 blockStart = 0;
	UT_uint32 bufferLength = 0;

	UT_GrowBuf pBuffer;

	// Check early for completion, from where we left off last, and
	// bail if we are now at or past the start position
	UT_uint32 blockOffsetToPos = _BlockOffsetToPos(*pBlock, *pOffset);
	UT_DEBUGMSG(("m_wrappedEnd=%d blockOffsetToPos=%d m_startPosition=%d\n",m_wrappedEnd,blockOffsetToPos,m_startPosition));
	if (m_wrappedEnd && (blockOffsetToPos <= m_startPosition))
	{
		// We're done
		return NULL;
	}

	if (!(*pBlock)->getBlockBuf(&pBuffer))
	{
		UT_DEBUGMSG(("Block %p has no associated buffer.\n", *pBlock));
		// I gather we better return ???
		UT_ASSERT_HARMLESS(0);
		return NULL;
	}

	// Have we already searched all the text in this buffer?
	if (_BlockOffsetToPos(*pBlock, *pOffset) <= (*pBlock)->getPosition(false))
	{
		// Then return a fresh new block's buffer
		newBlock = *pBlock;
	    get_new_block:	newBlock = newBlock->getPrevBlockInDocument();
		xxx_UT_DEBUGMSG(("Got prev block %x \n",newBlock));
		// Are we at the end of the document?
		if (!newBlock)
		{
			if(m_wrappedEnd)
				return NULL;
			
			// Then wrap (fetch the first block in the doc)
			PT_DocPosition endOfDoc;
			getEditableBounds(true, endOfDoc);

			newBlock = m_pLayout->findBlockAtPositionReverse(endOfDoc);

			m_wrappedEnd = true;
			UT_DEBUGMSG(("Reached start of doc via getPrevBlockinDocument \n"));
			UT_return_val_if_fail(newBlock, NULL);
		}

		// Re-assign the buffer contents for our new block
		pBuffer.truncate(0);
		// The offset starts at end of block
		blockStart = 0;
		if (!newBlock->getBlockBuf(&pBuffer))
		{
			UT_DEBUGMSG(("Block %p (a ->prev block) has no buffer.\n",
						 newBlock));
			UT_ASSERT_HARMLESS(0);
			return NULL;
		}
		newOffset = pBuffer.getLength();
		if(pBuffer.getLength() == 0)
		{
			goto get_new_block;
		}
		// Good to go with a full buffer for our new block
	}
	else
	{
		// We have some left to go in this buffer.	Buffer is still
		// valid, just copy pointers
		newBlock = *pBlock;
		newOffset = *pOffset;
		blockStart = 0;
	}

	// Are we going to run into the start position in this buffer?	If
	// so, we need to size our length accordingly
	if (m_wrappedEnd && (newBlock->getPosition(false) <= m_startPosition))
	{
		// Check for completion now, if we are now at or past the start position
		blockOffsetToPos = _BlockOffsetToPos(newBlock, newOffset);
		UT_DEBUGMSG(("(2) m_wrappedEnd=%d blockOffsetToPos=%d m_startPosition=%d\n",m_wrappedEnd,blockOffsetToPos,m_startPosition));
		if (blockOffsetToPos <= m_startPosition)
		{
			// We're done
			UT_DEBUGMSG(("(2) PrevSearch completed\n"));
			return NULL;
		}
		else
		{
			endIndex = (m_startPosition - (newBlock->getPosition(false)));
		}		
	}

	if(blockStart >= pBuffer.getLength())
	{
		// we are done, as there is nothing to search .
		UT_DEBUGMSG(("PrevSearch completed \n"));
		return NULL;
	}
		
	bufferLength = pBuffer.getLength() - blockStart;

	// clone a buffer (this could get really slow on large buffers!)
	UT_UCSChar* bufferSegment = NULL;

	// remember, the caller gets to g_free this memory
	bufferSegment = (UT_UCSChar*)UT_calloc(bufferLength + 1, sizeof(UT_UCSChar));
	UT_return_val_if_fail(bufferSegment, NULL);

	memmove(bufferSegment, pBuffer.getPointer(blockStart),
			(bufferLength) * sizeof(UT_UCSChar));

	// before we bail, hold up our block stuff for next round
	*pBlock = newBlock;
	*pOffset = newOffset;
	
	UT_DEBUGMSG(("Block pos: %d ", (newBlock)->getPosition(false)));
	UT_DEBUGMSG((" - len: %d\n", pBuffer.getLength()));
	

	return bufferSegment;
}

bool FV_View::_insertField(const char* szName,
						   const PP_PropertyVector & extra_attrs,
						   const PP_PropertyVector & extra_props)
{
	bool bResult = false;
	if(szName && ((strcmp(szName,"sum_rows") == 0) || (strcmp(szName,"sum_cols") == 0)))
	{
	     if(!isInTable())
	     {
	          return false;
	     }
	}

	PP_PropertyVector attributes = extra_attrs;
	attributes.push_back("type");
	attributes.push_back(szName);


	fd_Field * pField = NULL;
	if (!isSelectionEmpty() && !m_FrameEdit.isActive())
	{
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();

		insertParaBreakIfNeededAtPos(getPoint());
		if(!isPointLegal(getPoint()))
		{
			_makePointLegal();
		}
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, extra_props, &pField);
		if(pField != NULL)
		{
			pField->update();
		}
		m_pDoc->endUserAtomicGlob();
	}
	else if(m_FrameEdit.isActive())
	{
	       m_FrameEdit.setPointInside();
	}
	else
	{
		insertParaBreakIfNeededAtPos(getPoint());
		if(!isPointLegal(getPoint()))
		{
			_makePointLegal();
		}
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, extra_props, &pField);
		if(pField != NULL)
		{
			pField->update();
		}
	}

	return bResult;
}

bool
FV_View::_findReplaceReverse(UT_uint32* pPrefix, bool& bDoneEntireDocument, bool bNoUpdate)
{
	UT_ASSERT(m_sFind && m_sReplace);

	bool bRes = false;

	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();

	// Replace selection if it's due to a find operation
	if (m_doneFind && !isSelectionEmpty())
	{
		bRes = true;

		PP_AttrProp AttrProp_Before;

		if (!isSelectionEmpty() && !m_FrameEdit.isActive())
		{
			_deleteSelection(&AttrProp_Before, bNoUpdate);
		}
		else if(m_FrameEdit.isActive())
		  {
		    m_FrameEdit.setPointInside();
		  }

		// If we have a string with length, do an insert, else let it
		// hang from the delete above
		if (*m_sReplace)
		{
			bRes = m_pDoc->insertSpan(getPoint(), m_sReplace,
									  UT_UCS4_strlen(m_sReplace),
									  &AttrProp_Before);
			
			setPoint( getPoint() - UT_UCS4_strlen(m_sReplace)) ;
		}
		// Do not increase the insertion point index, since the insert
		// span will leave us at the correct place.

		if(!bNoUpdate)
			_generalUpdate();


		// If we've wrapped around once, and we're doing work before
		// we've hit the point at which we started, then we adjust the
		// start position so that we stop at the right spot.
		if (m_wrappedEnd && !bDoneEntireDocument)
		{
			m_startPosition += (long) UT_UCS4_strlen(m_sReplace);
			m_startPosition -= (long) UT_UCS4_strlen(m_sFind);
		}

		UT_ASSERT(m_startPosition >= 2);
	}

	m_pDoc->endUserAtomicGlob();
	_restorePieceTableState();

	// Find next occurrence in document
	_findPrev(pPrefix, bDoneEntireDocument);
	return bRes;
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
FV_View::_findReplace(UT_uint32* pPrefix, bool& bDoneEntireDocument, bool bNoUpdate)
{
	UT_ASSERT(m_sFind && m_sReplace);

	bool bRes = false;

	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();

	// Replace selection if it's due to a find operation
	if (m_doneFind && !isSelectionEmpty())
	{
		bRes = true;

		PP_AttrProp AttrProp_Before;

		if (!isSelectionEmpty() && !m_FrameEdit.isActive())
		{
			_deleteSelection(&AttrProp_Before, bNoUpdate);
		}
		else if(m_FrameEdit.isActive())
		{
		  m_FrameEdit.setPointInside();
		}

		// If we have a string with length, do an insert, else let it
		// hang from the delete above
		if (*m_sReplace)
			bRes = m_pDoc->insertSpan(getPoint(), m_sReplace,
									  UT_UCS4_strlen(m_sReplace),
									  &AttrProp_Before);

		// Do not increase the insertion point index, since the insert
		// span will leave us at the correct place.

		if(!bNoUpdate)
			_generalUpdate();

		// If we've wrapped around once, and we're doing work before
		// we've hit the point at which we started, then we adjust the
		// start position so that we stop at the right spot.
		if (m_wrappedEnd && !bDoneEntireDocument)
		{
			m_startPosition += (long) UT_UCS4_strlen(m_sReplace);
			m_startPosition -= (long) UT_UCS4_strlen(m_sFind);
		}

		UT_ASSERT(m_startPosition >= 2);
	}

	m_pDoc->endUserAtomicGlob();
	_restorePieceTableState();

	// Find next occurrence in document
	_findNext(pPrefix, bDoneEntireDocument);
	return bRes;
}

fl_BlockLayout*
FV_View::_findGetCurrentBlock(void) const
{
	return _findBlockAtPosition(m_iInsPoint);
}

PT_DocPosition
FV_View::_findGetCurrentOffset() const
{
	return (m_iInsPoint - _findGetCurrentBlock()->getPosition(false));
}

// Any takers?
UT_sint32
FV_View::_findBlockSearchRegexp(const UT_UCSChar* /* haystack */,
								const UT_UCSChar* /* needle */)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);

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
//
// If we're in an illegal position move forward till we're safe.
//
	_makePointLegal();
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
	if(!m_pDoc->isDoingPaste())
	{
		notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK | AV_CHG_PAGECOUNT | AV_CHG_FMTSTYLE );
		setCursorToContext();
	}
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
	UT_uint32 iNewPoint = getPoint();
	xxx_UT_DEBUGMSG(("_extSel: iNewPoint %d ioldPoint %d selectionAnchor %d \n",iNewPoint,iOldPoint,m_Selection.getSelectionAnchor()));
	PT_DocPosition posBOD,posEOD,dNewPoint,dOldPoint;
	dNewPoint = static_cast<PT_DocPosition>(iNewPoint);
	dOldPoint = static_cast<PT_DocPosition>(iOldPoint);
	getEditableBounds(false,posBOD);
	getEditableBounds(true,posEOD);
	if(dNewPoint < posBOD || dNewPoint > posEOD || dOldPoint < posBOD
	   || dOldPoint > posEOD)
	{
		return;
	}
	if (iNewPoint == iOldPoint)
	{
		return;
	}
	if(iOldPoint < iNewPoint)
	{
		_drawBetweenPositions(iOldPoint, iNewPoint);
	}
	else
	{
		_drawBetweenPositions(iNewPoint,iOldPoint);
	}
	if(getPoint() > getSelectionAnchor())
	{
		m_Selection.setSelectionLeftAnchor(getSelectionAnchor());
		m_Selection.setSelectionRightAnchor(getPoint());
	}
	else
	{
		m_Selection.setSelectionRightAnchor(m_Selection.getSelectionAnchor());
		m_Selection.setSelectionLeftAnchor(getPoint());
	}
	
}

void FV_View::_extSelToPos(PT_DocPosition iNewPoint)
{
	PT_DocPosition iOldPoint = getPoint();
	if (iNewPoint == iOldPoint)
		return;

	PT_DocPosition posBOD,posEOD,dNewPoint,dOldPoint;
	dNewPoint = static_cast<PT_DocPosition>(iNewPoint);
	dOldPoint = static_cast<PT_DocPosition>(iOldPoint);
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
	m_Selection.setMode(FV_SelectionMode_Single);
	_setPoint(iNewPoint);
//
// Look if we should select the initial cell.
//
	_extSel(iOldPoint);
	if(getSelectionAnchor() < getPoint())
	{
		PT_DocPosition posLow = getSelectionAnchor();
		fp_CellContainer * pLowCell = NULL;
		fp_CellContainer * pHighCell = NULL;
		if(isInTable(posLow) )
		{
			pLowCell = getCellAtPos(posLow+1);
			pHighCell =  getCellAtPos(getPoint());
			if((pLowCell != NULL) && (pLowCell != pHighCell))
			{
				fl_CellLayout * pCell = static_cast<fl_CellLayout *>(pLowCell->getSectionLayout());
				PT_DocPosition posCell = pCell->getPosition(true);
				xxx_UT_DEBUGMSG(("posCell %d posLow %d \n",posCell,posLow));
				if((posCell == posLow) && (m_iGrabCell == 0))
				{
					m_iGrabCell++;
					m_Selection.setSelectionAnchor(posCell-1);
					_drawBetweenPositions(posCell-1, getPoint());
				}
				else if(((posCell + 1) == posLow) && (m_iGrabCell == 0))
				{
					m_iGrabCell++;
					m_Selection.setSelectionAnchor(posCell-1);
					_drawBetweenPositions(posCell-1, getPoint());
				}
				else if(((posCell + 2) == posLow) && (m_iGrabCell == 0))
				{
					m_iGrabCell++;
					m_Selection.setSelectionAnchor(posCell-1);
					_drawBetweenPositions(posCell-1, getPoint());
				}
//
// FIXME look to see if we've selected a whole row.
//
			}
		}
	}

	if (isSelectionEmpty())
	{
		_resetSelection();
	}

//	notifyListeners(AV_CHG_MOTION);
}


/*
  This method simply iterates over every run between two doc positions
  and draws each one.
*/
void FV_View::_drawBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2)
{
	_drawOrClearBetweenPositions(iPos1, iPos2, false,false);
}

/*!
  This class was local to FV_View::_drawOrClearBetweenPositions(),
  but you can't use local types for a template instancation.
  HACK: I did move it outside and made everything private to prevent
  anyone else to use it.
 */
class ABI_EXPORT CellLine
{
private:
	CellLine(void):
		m_pCell(NULL),
		m_pBrokenTable(NULL),
		m_pLine(NULL)
		{}
	virtual ~CellLine(void)
		{
			m_pCell = NULL;
			m_pBrokenTable = NULL;
			m_pLine = NULL;
		}
	fp_CellContainer * m_pCell;
	fp_TableContainer * m_pBrokenTable;
	fp_Line * m_pLine;
	friend class FV_View;
	friend bool FV_View::_drawOrClearBetweenPositions(PT_DocPosition iPos1,
													  PT_DocPosition iPos2, 
													  bool bClear, bool bFullLineHeight);
};

/*!
  This method simply iterates over every run between two doc positions
  and draws or clears and redraws each one. We clear if bClear is true 
  otherwise we just draw selected.
  If bClear is true then the if bFullLineHeight is true the runs are 
  cleared to their full height.
*/
bool FV_View::_drawOrClearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, bool bClear, bool bFullLineHeight)
{
//
// Handle little class for redrawing lines around cells
//
	
	// make sure iPos1 < iPos2
	if (iPos1 >= iPos2)
	{
		PT_DocPosition tmp = iPos1;
		iPos1 = iPos2;
		iPos2 = tmp;
	}
	
//	CHECK_WINDOW_SIZE
	xxx_UT_DEBUGMSG(("Draw between positions %d to %d \n",iPos1,iPos2));
	fp_Run* pRun1;
	fp_Run* pRun2;
	UT_sint32 xoff2;
	UT_sint32 yoff2;
	UT_uint32 uheight;
	UT_GenericVector<CellLine *> vecTables;
	UT_GenericVector<fp_Page *>vecPages;
//
// This fixes a bug from insert file, when the view we copy from is selected
// If don't bail out now we get all kinds of crazy dirty on the screen.
//
	if(m_pParentData == NULL)
	{
		return true;
	}
//	_fixInsertionPointCoords();
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

	UT_return_val_if_fail(pRun1 && pRun2, false );
	
	bool bDone = false;
	bool bIsDirty = false;
	fp_Run* pCurRun = pRun1;
	//bool bShowHidden = getShowPara();
	fp_CellContainer * pCell = NULL;
	fl_BlockLayout* pBlockEnd = pRun2->getBlock();
	PT_DocPosition posEnd = pBlockEnd->getPosition() + pRun2->getBlockOffset();

	FV_ViewDoubleBuffering dblBufferingObj(this, false, false);
	dblBufferingObj.beginDoubleBuffering();

	while ((!bDone || bIsDirty) && pCurRun)
	{

		fl_BlockLayout* pBlock2 = pCurRun->getBlock();
		fp_Line * pLine = pCurRun->getLine();
		if(pLine == NULL || (pLine->getContainer()->getPage()== NULL))
		{
			UT_VECTOR_PURGEALL(CellLine *, vecTables);
			return true;
		}
		PT_DocPosition curpos = pBlock2->getPosition() + pCurRun->getBlockOffset();
		if ((pCurRun->getLength() > 0 ) && (pCurRun == pRun2 || curpos >= posEnd))
		{
			bDone = true;
		}
		if(curpos > posEnd)
		{
//			break;
		}
		xxx_UT_DEBUGMSG(("draw_between positions pos is %d width is %d \n",curpos,pCurRun->getWidth()));
		UT_return_val_if_fail(pBlock2,false);
//
// Look to see if the Block is in a table.
//
		fl_ContainerLayout * pCL = pBlock2->myContainingLayout();
		bool bCellSelected = false;
		if(pCL->getContainerType() == FL_CONTAINER_CELL)
		{
			fp_Container * pCP = static_cast<fp_Container *>(pLine->getContainer());
			if(pCP)
			{
				pCell = static_cast<fp_CellContainer *>(pCP);
				fp_TableContainer * pTab = pCell->getBrokenTable(pLine);
				if(pTab)
				{
					CellLine * pCellLine = new CellLine();
					pCellLine->m_pCell = pCell;
					pCellLine->m_pLine = pLine;
					pCellLine->m_pBrokenTable = pTab;
					xxx_UT_DEBUGMSG(("cellLine %x cell %x Table %x Line %x \n",pCellLine,pCellLine->m_pCell,pCellLine->m_pBrokenTable,pCellLine->m_pLine));
					vecTables.addItem(pCellLine);
					fp_Page * pPage = pTab->getPage();
					if((pPage != NULL) && (vecPages.findItem(pPage) <0))
					{
						vecPages.addItem(pPage);
					}
				}
			}
			fl_CellLayout * pCellLayout = static_cast<fl_CellLayout *>(pCL);
//
// See if the whole cell is selected. If so draw it.
//
			bCellSelected = pCellLayout->isCellSelected();
			fp_Container * pNextCon = NULL;
			if(bCellSelected)
			{
				pNextCon = pCell->drawSelectedCell(pCurRun->getLine());
				if(pNextCon == NULL)
				{
					fl_BlockLayout * pBlock = pCurRun->getBlock();
					pBlock = pBlock->getNextBlockInDocument();
					if(pBlock)
					{
						pCurRun = pBlock->getFirstRun();
						continue;
					}
					pCurRun = NULL;
					continue;
				}
				if(pNextCon->getContainerType() == FP_CONTAINER_LINE)
				{
					pCurRun = static_cast<fp_Line *>(pNextCon)->getFirstRun();
					continue;
				}
				else
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
					pCurRun = NULL;
					continue;
				}
			}
			else
			{
				if(pCell && pCell->isSelected())
				{
					pCell->clearSelection();
					pCell->clearScreen();
					pCell->draw(pCurRun->getLine());
					if(pCurRun->getLine() != NULL)
					{
						fp_Page * pPage = pCurRun->getLine()->getPage();
						if((pPage != NULL) && (vecPages.findItem(pPage) <0))
						{
							vecPages.addItem(pPage);
						}
					}
				}
			}
		}

		if(!pCurRun->isHidden())
		{
			if(pLine == NULL || (pLine->getContainer()->getPage()== NULL))
			{
				UT_VECTOR_PURGEALL(CellLine *, vecTables);
				return true;
			}
			pLine->getScreenOffsets(pCurRun, xoff2, yoff2);
			dg_DrawArgs da;
			da.bDirtyRunsOnly = false;
			da.pG = m_pG;
			da.xoff = xoff2;
			da.yoff = yoff2 + pLine->getAscent();
			xxx_UT_DEBUGMSG(("Draw Position CurRun %x CurLine %x Yoffset %d \n",pCurRun,pLine,yoff2));
			if(!bClear)
			{
				xxx_UT_DEBUGMSG(("Draw Position Low %d High %d anchor %d point %d xoff %d \n",iPos1,iPos2,getSelectionAnchor(),getPoint(),xoff2));
//				UT_sint32 iLow = getSelectionAnchor();
//				UT_sint32 iHigh = getPoint();
//				if(iHigh < iLow
				pCurRun->setSelectionMode(iPos1-1,iPos2+1);
				pCurRun->draw(&da);
				pCurRun->clearSelectionMode();
			}
			else
			{
				xxx_UT_DEBUGMSG(("Clear Position Low %d High %d anchor %d point %d xoff %d \n",iPos1,iPos2,getSelectionAnchor(),getPoint(),xoff2));
				pCurRun->setSelectionMode(iPos1-4,iPos2+4);
				pCurRun->Run_ClearScreen(bFullLineHeight);
				pCurRun->draw(&da);
				pCurRun->clearSelectionMode();
			}
			fp_Page * pPage = pLine->getPage();
			if((pPage != NULL) && (vecPages.findItem(pPage) <0))
			{
				vecPages.addItem(pPage);
			}
		}

		pCurRun = pCurRun->getNextRun();
		if (!pCurRun)
		{
			fl_BlockLayout* pNextBlock;

			pNextBlock = pBlock2->getNextBlockInDocument();
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
//
// Now redraw the lines in any table encountered.
//
	xxx_UT_DEBUGMSG(("Drawing lines in tables %d \n",vecTables.getItemCount()));
	UT_sint32 i =0;
	for(i=0; i< vecTables.getItemCount(); i++)
	{
 		CellLine * pCellLine = vecTables.getNthItem(i);
		bool bLineDrawn = pCellLine->m_pCell->drawLines(pCellLine->m_pBrokenTable,getGraphics(),true);
		if (bLineDrawn)
		{
			bLineDrawn = pCellLine->m_pCell->drawLines(pCellLine->m_pBrokenTable,getGraphics(),false);
			pCellLine->m_pCell->drawLinesAdjacent();
		}
	}
	for(i=0; i< vecPages.getItemCount(); i++)
	{
		fp_Page * pPage = vecPages.getNthItem(i);
		UT_sint32 xoff,yoff;
		getPageScreenOffsets(pPage,xoff, yoff);
		dg_DrawArgs da;
		da.pG = m_pG;
		da.xoff = xoff;
		da.yoff = yoff;
		da.bDirtyRunsOnly = true;
		pPage->redrawDamagedFrames(&da);
	}
	UT_VECTOR_PURGEALL(CellLine *, vecTables);
	xxx_UT_DEBUGMSG(("Finished Drawing lines in tables \n"));
	m_pG->flush();
	_generalUpdate();
	return true;
}

/*
  This method simply iterates over every run between two doc positions
  and draws each one.
*/
bool FV_View::_clearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, bool bFullLineHeightRect)
{
	return _drawOrClearBetweenPositions(iPos1, iPos2, true,bFullLineHeightRect);
#if 0
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

		fl_BlockLayout* pBlock = pCurRun->getBlock();
		UT_ASSERT(pBlock);
//
// Look to see if the Block is in a table.
//
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		if(pCL->getContainerType() == FL_CONTAINER_CELL)
		{
			fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pCL->getFirstContainer());
			if(pCell->isSelected())
			{
				pCell->clearSelection();
				pCell->clearScreen();
			
				fl_BlockLayout * pBlock = NULL;
				fl_ContainerLayout * pLastCL = pCL->getFirstLayout();
				while(pLastCL->getNext())
				{
					pLastCL = pLastCL->getNext();
				}
				while(pLastCL->getContainerType() != FL_CONTAINER_BLOCK)
				{
					pLastCL = pLastCL->getFirstLayout();
				}
				pBlock = static_cast<fl_BlockLayout *>(pLastCL);
				pBlock = pBlock->getNextBlockInDocument();
				if(pBlock)
				{
					pCurRun = pBlock->getFirstRun();
					continue;
				}
				pCurRun = NULL;
				bDone = true;
				continue;
			}
		}
		pCurRun->clearScreen(bFullLineHeightRect);
		if (pCurRun->getNextRun())
		{
			pCurRun = pCurRun->getNextRun();
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
#endif
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

//
// Have to deal with special case of point being exactly on a footnote/endnote
// boundary
//
	bool onFootnoteBoundary = false;
	if(m_pDoc->isFootnoteAtPos(pos))
	{
		onFootnoteBoundary = true;
		pos--;
	}
	fl_BlockLayout* pBlock = _findBlockAtPosition(pos);

	// this assert makes debugging very hard ...
#ifdef DEBUG
	static bool bDoneTheAssert = false;
	if(!bDoneTheAssert)
	{
		bDoneTheAssert = true;
		UT_ASSERT_HARMLESS(pBlock && pBlock->getContainerType() == FL_CONTAINER_BLOCK);
	}
#endif
	if(!pBlock || pBlock->getContainerType() != FL_CONTAINER_BLOCK)
	{
		x = x2 = 0;
		y = y2 = 0;
		
		height = 0;
		if(ppBlock)
			*ppBlock = 0;
		return;
	}


	if(onFootnoteBoundary)
	{
		pos++;
	}
	// probably an empty document, return instead of
	// dereferencing NULL.	Dom 11.9.00
	if(!pBlock)
	{
		// Do the assert. Want to know from debug builds when this happens.
		// This assert makes debuging virtuall impossible; the hack below makes it assert only once.
		// UT_ASSERT(pBlock);
#ifdef DEBUG
		static bool bAss = false;
		if(!pBlock && !bAss)
		{
			UT_ASSERT_HARMLESS( pBlock );
			bAss = true;
		}
#endif

		x = x2 = 0;
		y = y2 = 0;

		height = 0;
		if(ppBlock)
			*ppBlock = 0;
		return;
	}

	// if the block cannot contain point, find the nearest block to
	// the left that can; or to the right, if none to the left exists
	fl_BlockLayout * pOrigBL = pBlock;
	
	while(pBlock && !pBlock->canContainPoint())
	{
		pBlock = pBlock->getPrevBlockInDocument();
	}

	if(!pBlock)
	{
		pBlock = pOrigBL;
		
		while(pBlock && !pBlock->canContainPoint())
		{
			pBlock = pBlock->getNextBlockInDocument();
		}
	}

	if(!pBlock)
	{
		// no blocks that can take point in this document !!!
		// one of the scenarios in which this happens is when the user did ctrl+a -> del
		// in revisions mode or marked everything hidden while fmt marks are not showing
		// If there is a block and it is not visible, we return that block
		// Note that it is difficult to prevent this from happening on the PT level, since
		// just because text is marked as hidden or deleted does not mean it is not
		// visible in a given view.
		fl_DocSectionLayout * pDSL = m_pLayout->getFirstSection();
		pBlock = pDSL->getFirstBlock();

		if(!pBlock)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			x = x2 = 0;
			y = y2 = 0;

			height = 0;
			if(ppBlock)
				*ppBlock = 0;
			return;
		}
	}
	
	// If block is actually to the right of the requested position
	// (this happens in an empty document), update the pos with the
	// start pos of the block.
	PT_DocPosition iBlockPos = pBlock->getPosition(false);
	if (iBlockPos > pos)
	{
		pos = iBlockPos;
	}
	xxx_UT_DEBUGMSG(("Position to find %d \n",pos));
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
		while(pRun && pRun->getPrevRun() && !pRun->isField() && pRun->getWidth() == 0)
		{
			bBack = false;
			pRun = pRun->getPrevRun();
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
		while(pRun && (pRun->getNextRun() != NULL))
		{
			pRun = pRun->getNextRun();
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
		
		fp_Page* pPointPage = pLine->getPage();

		UT_sint32 iPageOffset;
		getPageYOffset(pPointPage, iPageOffset); // <- look at this later
		
		UT_uint32 iPageNumber = m_pLayout->findPage(pPointPage);
		//UT_uint32 iRow = iPageNumber / getNumHorizPages();
		//UT_uint32 iCol = iPageNumber - (iRow * getNumHorizPages());
		
		yPoint += iPageOffset; //beware wierdness discribed in getPageYOffset(...)
		xPoint += getPageViewLeftMargin() + getWidthPrevPagesInRow(iPageNumber);
		yPoint2 += iPageOffset; //beware wierdness discribed in getPageYOffset(...)
		xPoint2 += getPageViewLeftMargin() + getWidthPrevPagesInRow(iPageNumber);


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
		xxx_UT_DEBUGMSG(("x,y pos in view %d,%d \n",x,y));
		xxx_UT_DEBUGMSG(("x2,y2 pos in view %d,%d \n",x2,y2));

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

void FV_View::_fixAllInsertionPointCoords() const
{
	fv_CaretProps * pCaretProps = NULL;
	UT_sint32 iCount = m_vecCarets.getItemCount();
	UT_sint32 i = 0;
	for(i=0; i<iCount;i++)
	{
			pCaretProps = m_vecCarets.getNthItem(i);
			_fixInsertionPointCoords(pCaretProps);
	}
}

void FV_View::_fixInsertionPointCoords(fv_CaretProps * pCP) const
{
	if ((pCP->m_iInsPoint > 0) && !isLayoutFilling())
	{
		fl_BlockLayout * pBlock = NULL;
		fp_Run * pRun = NULL;
		_findPositionCoords(pCP->m_iInsPoint, pCP->m_bPointEOL, pCP->m_xPoint, 
							pCP->m_yPoint, pCP->m_xPoint2, pCP->m_yPoint2, 
							pCP->m_iPointHeight, pCP->m_bPointDirection, 
							&pBlock, &pRun);
		const fp_Page * pPage = getCurrentPage();
		const UT_RGBColor * pClr = NULL;
		if (pPage)
			pClr = pPage->getFillType().getColor();
		UT_sint32 yoff = 0;
		if(pCP->m_yPoint < 0)
		{
			UT_sint32 negY  = -pCP->m_yPoint;
			yoff = negY + 1;
			if(negY > (UT_sint32)pCP->m_iPointHeight)
			{
				pCP->m_iPointHeight = 0;
				yoff = 0;
			}
		}
		if(pRun && (pRun->getType() == FPRUN_IMAGE))
		{
			UT_DEBUGMSG(("On image run with fixPointcoords \n"));
		}
		xxx_UT_DEBUGMSG(("Xpoint in fixpoint %d \n",m_xPoint));
		pCP->m_pCaret->setCoords(pCP->m_xPoint, pCP->m_yPoint+yoff, 
								 pCP->m_iPointHeight-yoff,
								 pCP->m_xPoint2, 
								 pCP->m_yPoint2+yoff, 
								 pCP->m_iPointHeight-yoff, 
								 pCP->m_bPointDirection, pClr);
	}
	pCP->m_pCaret->setWindowSize(getWindowWidth(), getWindowHeight());

}

void FV_View::_fixInsertionPointCoords(bool bIgnoreAll)
{
	if (m_pG->allCarets()->getBaseCaret() == NULL)
		return;
	if(!bIgnoreAll)
		_fixAllInsertionPointCoords();
	
	fp_Page * pPage = NULL;
	fl_BlockLayout * pBlock = NULL;
	fp_Run * pRun = NULL;
	if(m_bInsertAtTablePending)
	{
		//
		// Position the caret just before the table
		//
		fl_TableLayout * pTL = getTableAtPos(m_iPosAtTable+3);
		if(pTL == NULL)
		{
			m_bInsertAtTablePending = false;
			return;
		}
		pBlock = pTL->getNextBlockInDocument();
		if(pBlock == NULL)
		{
			m_bInsertAtTablePending = false;
			return;
		}
		UT_sint32 height= 0;
		pRun = pBlock->findPointCoords(pBlock->getPosition(false), false, m_xPoint, m_yPoint, m_xPoint2, m_yPoint2, height, m_bPointDirection);
		m_iPointHeight = static_cast<UT_uint32>(height);
		fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pTL->getFirstContainer());
		fp_TableContainer * pBroke = pTab->getFirstBrokenTable();
		fp_CellContainer * pCell = static_cast<fp_CellContainer *>(pTab->getFirstContainer());
		UT_sint32 iLeft,iRight,iTop,iBot,col_y =0;
		bool bDoClear= true;
		fp_Column * pCol = NULL;
		fp_ShadowContainer * pShadow = NULL;
		pCell->getScreenPositions(pBroke,getGraphics(),iLeft,iRight,iTop,iBot,col_y,pCol,pShadow,bDoClear);
		m_xPoint = iLeft - getGraphics()->tlu(2);
		m_xPoint2 = iLeft - getGraphics()->tlu(2);
		m_yPoint = iTop;
		m_yPoint2 = iTop;
		pPage = getCurrentPage();
		const UT_RGBColor * pClr = NULL;
		if (pPage)
			pClr = pPage->getFillType().getColor();
		m_pG->allCarets()->getBaseCaret()->setCoords(m_xPoint, m_yPoint, m_iPointHeight,
									m_xPoint2, m_yPoint2, m_iPointHeight, 
									m_bPointDirection, pClr);
	}
	else if ((getPoint() > 0) && !isLayoutFilling())
	{
		_findPositionCoords(getPoint(), m_bPointEOL, m_xPoint, m_yPoint, m_xPoint2, m_yPoint2, m_iPointHeight, m_bPointDirection, &pBlock, &pRun);
		pPage = getCurrentPage();
		const UT_RGBColor * pClr = NULL;
		if (pPage)
			pClr = pPage->getFillType().getColor();
		UT_sint32 yoff = 0;
		if(m_yPoint < 0)
		{
			UT_sint32 negY  = -m_yPoint;
			yoff = negY + 1;
			if(negY > (UT_sint32)m_iPointHeight)
			{
				m_iPointHeight = 0;
				yoff = 0;
			}
		}
		if(pRun && (pRun->getType() == FPRUN_IMAGE))
		{
			UT_DEBUGMSG(("On image run with fixPointcoords \n"));
		}
		xxx_UT_DEBUGMSG(("Xpoint in fixpoint %d \n",m_xPoint));
		m_pG->allCarets()->getBaseCaret()->setCoords(m_xPoint, m_yPoint+yoff, m_iPointHeight-yoff,
									m_xPoint2, m_yPoint2+yoff, m_iPointHeight-yoff, 
									m_bPointDirection, pClr);
	}

	m_pG->allCarets()->setWindowSize(getWindowWidth(), getWindowHeight());

	xxx_UT_DEBUGMSG(("SEVIOR: m_yPoint = %d m_iPointHeight = %d \n",m_yPoint,m_iPointHeight));
	// hang onto this for _moveInsPtNextPrevLine()
	m_xPointSticky = m_xPoint + m_xScrollOffset - getPageViewLeftMargin();
#ifdef ENABLE_SPELL
	if(pBlock && pBlock->getSpellSquiggles()->get(getPoint() - pBlock->getPosition()))
	{
		if(m_prevMouseContext == EV_EMC_TEXT)
		{
			m_prevMouseContext = EV_EMC_MISSPELLEDTEXT;
		}
	}
	if(pBlock)
	{
		m_pLayout->triggerPendingBlock(pBlock);
	}
#endif
}

// Finds what pages are on screen and draws them
void FV_View::_draw(UT_sint32 x, UT_sint32 y,
					UT_sint32 width, UT_sint32 height,
					bool bDirtyRunsOnly, bool bClip)
{
	xxx_UT_DEBUGMSG(("FV_View::draw_3 [x %d][y %d][w %d][h %d][bClip %d]\n"
					 "\t\twith [yScrollOffset %d][windowHeight %d][bDirtyRunsOnly %d]\n",
					 x,y,width,height,bClip,
					 m_yScrollOffset,getWindowHeight(),bDirtyRunsOnly));

	if(m_pViewDoubleBufferingObject != NULL && m_pViewDoubleBufferingObject->getCallDrawOnlyAtTheEnd())
	{
		// record this call's arguments and return
		if(bClip)
		{
			UT_Rect r(x, y, width, height);
			m_pG->setClipRect(&r);
		}
		m_pViewDoubleBufferingObject->recordViewDrawCall(x, y, width, height, bDirtyRunsOnly, bClip);
		m_pG->setClipRect(NULL);
		return;
	}

	/**************************
	 * STEP 0: Initialization *
	 **************************/

	GR_Painter painter(m_pG);
	
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(getParentData());

	// CHECK_WINDOW_SIZE
	// this can happen when the frame size is decreased and
	// only the toolbars show...
	if ((getWindowWidth() <= 0) || (getWindowHeight() <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero drawing area.\n"));
		return;
	}

	if ((width <= 0) || (height <= 0))
	{
		UT_DEBUGMSG(("fv_View::draw() called with zero width or height expose.\n"));
		return;
	}

	painter.beginDoubleBuffering();

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

//	UT_ASSERT(m_yScrollOffset == m_pG->getPrevYOffset());
	
	// figure out where pages go, based on current window dimensions
	// TODO: don't calc for every draw
	// HYP:  cache calc results at scroll/size time
	calculateNumHorizPages();

	
	/******************************************************************
	 * STEP 1: Find the first page so we can start drawing from there *
	 ******************************************************************/
	
	// EYA: In case you were wondering, everything assumes that all pages have
	// the same dimensions and are ordered in rows and columns as in print view
	// (normal view is just print view with one column).  Abandon hope, all ye
	// who would attempt to change those assumptions: they are very deeply
	// embedded throughout AbiWord.
	
	UT_sint32 iPageWidth = 0, iPageHeight = 0;
	UT_sint32 iFirstVisiblePageNumber = -1;
	fl_DocSectionLayout *pDSL = NULL;

	// we should have at least the first page
	if(getLayout() -> getFirstPage())
	{
		// layout ref
		pDSL = getLayout() -> getFirstPage() -> getOwningSection();

		if(getViewMode() == VIEW_PRINT || getViewMode() == VIEW_PREVIEW)
		{
			// since all pages have the same width / height, set them here
			iPageWidth = getLayout() -> getFirstPage() -> getWidth();
			iPageHeight = getLayout() -> getFirstPage() -> getHeight();
			iFirstVisiblePageNumber = ((getYScrollOffset() - getPageViewTopMargin() + getPageViewSep()) /
									   (iPageHeight + getPageViewSep())) * getNumHorizPages();
		}
		else
		{
			UT_sint32 yRemaining = getYScrollOffset();
			// Since the margins are section properties, each section has different page heights in the two other view modes
			iFirstVisiblePageNumber = 0;
			while (pDSL)
			{
				iPageHeight = pDSL->getFirstOwnedPage()->getHeight() - pDSL -> getTopMargin() - pDSL -> getBottomMargin();
				iPageWidth = pDSL->getFirstOwnedPage()->getWidth();
				if (yRemaining < iPageHeight * pDSL->getPageCount())
				{
					iFirstVisiblePageNumber += yRemaining/iPageHeight;
					break;
				}
				else
				{
					yRemaining -= iPageHeight * pDSL->getPageCount();
					iFirstVisiblePageNumber += pDSL->getPageCount();
				}
				pDSL = static_cast<fl_DocSectionLayout*>(pDSL->getNext());
			}
		}
	}

	/**********************
	 * STEP 2: Draw pages *
	 **********************/

	// enter a double-buffered section

	if( !bDirtyRunsOnly && (getViewMode() == VIEW_PRINT) ) {
		UT_RGBColor clrMargin;
		if (m_pG->getColor3D(GR_Graphics::CLR3D_BevelDown, clrMargin)) {
			painter.fillRect(GR_Graphics::CLR3D_BevelDown, 0, 0,
							 getWindowWidth(), getWindowHeight());
		} else {
			painter.fillRect(getColorMargin(), 0, 0, getWindowWidth(), getWindowHeight());
		}
	}

	// start from the first visible page
	fp_Page *pPage = NULL;
	if(iFirstVisiblePageNumber >= 0)
		pPage = getLayout() -> getNthPage(iFirstVisiblePageNumber);

	while(pPage)
	{
		pDSL = pPage->getOwningSection();
		if(getViewMode() == VIEW_PRINT || getViewMode() == VIEW_PREVIEW)
		{
			iPageHeight = pPage->getHeight();
		}
		else
		{
			iPageHeight = pPage->getHeight() - pDSL->getTopMargin() - pDSL->getBottomMargin();
		}
		iPageWidth = pPage->getWidth();

		UT_sint32 adjustedTop = 0; // Top line of the page that defines the page's top margin,
				       // relative to the top of the screen and in layout units
		UT_sint32 adjustedBottom;
		UT_sint32 adjustedLeft = 0;
		UT_sint32 adjustedRight;

		UT_sint32 iPageXOffset;
		UT_sint32 iPageYOffset;

		dg_DrawArgs da;

		// get X / Y page offsets
		getPageYOffset(pPage, iPageYOffset);
		iPageXOffset = getWidthPrevPagesInRow(pPage->getPageNumber());

		// if this page is not visible, then stop
		if(!((iPageYOffset  <= getYScrollOffset() + getWindowHeight()) &&
		    (iPageYOffset + iPageHeight >= getYScrollOffset())))
			break;

		// Adjust page's boundaries
		switch(getViewMode())
		{
			case VIEW_NORMAL:
			case VIEW_WEB:
			adjustedTop = iPageYOffset - getYScrollOffset() + ( pPage->getPageNumber() * (m_pG->tlu(1) - getPageViewSep()) );
			adjustedLeft = 0;
			break;

			case VIEW_PRINT:
			case VIEW_PREVIEW:
			adjustedTop = iPageYOffset - getYScrollOffset();
			adjustedLeft = iPageXOffset - getXScrollOffset()  + getPageViewLeftMargin();
			break;
		}

		// view independant boundaries
		adjustedBottom = adjustedTop + iPageHeight;
		adjustedRight = adjustedLeft + iPageWidth;

		xxx_UT_DEBUGMSG(("Drawing page adjustedTop = %i, Bottom = %i, Left = %i, Right = %i\n", adjustedTop, adjustedBottom, adjustedLeft, adjustedRight));
		xxx_UT_DEBUGMSG(("--Entered _draw loop:\n  iPageNumber = %i, vecitemcount = %i\n  iRow = %i, iCol = %i\n  iPageWidth = %i, iPageHeight = %i\n  getPageViewTopMargin() = %i, m_yScrollOffset = %i\n", iPageNumber, vecPagesOnScreen.getItemCount(), iRow, iCol, iPageWidth, iPageHeight, getPageViewTopMargin(), m_yScrollOffset));

		xxx_UT_DEBUGMSG(("drawing page E: iPageHeight=%d curY=%d nPos=%d getWindowHeight()=%d y=%d h=%d\n", iPageHeight,curY,m_yScrollOffset,getWindowHeight(),y,height));

		// set drawing args
		da.pG = m_pG;
		da.yoff = adjustedTop;
		da.xoff = adjustedLeft;

		xxx_UT_DEBUGMSG(("Drawing page with da.yoff and da.xoff %i %i\n", da.yoff, da.xoff));

		// Redraw the page background, if necessary
		if(!bDirtyRunsOnly || (pPage->needsRedraw() && (getViewMode() == VIEW_PRINT)))
		{
			const UT_RGBColor * pClr = pPage->getFillType().getColor();
			if(getViewMode() == VIEW_NORMAL || getViewMode() == VIEW_WEB) // Normal/web view pages take up the whole window
				painter.fillRect(*pClr, adjustedRight, adjustedTop, getWindowWidth() - adjustedRight + m_pG->tlu(1), iPageHeight);
			else
				painter.fillRect(*pClr, adjustedLeft + m_pG->tlu(1), adjustedTop + m_pG->tlu(1), iPageWidth - m_pG->tlu(1), iPageHeight - m_pG->tlu(1));
			xxx_UT_DEBUGMSG(("   ---PAINTING PAGE %i---\n", pPage->getPageNumber()));

			//
			// Since we're clearing everything we have to draw every run no matter
			// what.
			//
			da.bDirtyRunsOnly = false;
		}

		// Draw the page and all its subcontainers (i.e., the content)
		pPage->draw(&da);

		// draw page decorations
		UT_RGBColor clr(0,0,0); 	// black
		m_pG->setColor(clr);

		// one pixel border a
		if(!isPreview() && (getViewMode() == VIEW_PRINT))
		{
			m_pG->setLineProperties(m_pG->tluD(1.0),
					GR_Graphics::JOIN_MITER,
					GR_Graphics::CAP_PROJECTING,
					GR_Graphics::LINE_SOLID);

			painter.drawLine(adjustedLeft, adjustedTop, adjustedRight, adjustedTop);
			painter.drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
			painter.drawLine(adjustedLeft, adjustedBottom, adjustedRight + m_pG->tlu(1), adjustedBottom);
			painter.drawLine(adjustedLeft, adjustedTop, adjustedLeft, adjustedBottom);
		}

		// Draw page seperator
		// only in NORMAL MODE - draw a line across the screen
		// at a page boundary. Not used in online/web and print
		// layout modes
		if(getViewMode() == VIEW_NORMAL)
		{
			UT_RGBColor clrPageSep(192,192,192);		// light gray
			m_pG->setColor(clrPageSep);

			m_pG->setLineProperties(m_pG->tluD(1.0),
						GR_Graphics::JOIN_MITER,
						GR_Graphics::CAP_PROJECTING,
						GR_Graphics::LINE_SOLID);

			painter.drawLine(adjustedLeft, adjustedBottom, getWindowWidth() + m_pG->tlu(1), adjustedBottom);
			adjustedBottom += m_pG->tlu(1);
			m_pG->setColor(clr);
		}

		// two pixel drop shadow for pages in print view
		if(!isPreview() && (getViewMode() == VIEW_PRINT) && !pFrame->isMenuScrollHidden() )
		{
			m_pG->setLineProperties(m_pG->tluD(1.0),
					GR_Graphics::JOIN_MITER,
					GR_Graphics::CAP_PROJECTING,
					GR_Graphics::LINE_SOLID);

			adjustedLeft += m_pG->tlu(3);
			adjustedBottom += m_pG->tlu(1);
			painter.drawLine(adjustedLeft, adjustedBottom, adjustedRight + m_pG->tlu(1), adjustedBottom);

			adjustedBottom += m_pG->tlu(1);
			painter.drawLine(adjustedLeft, adjustedBottom, adjustedRight + m_pG->tlu(1), adjustedBottom);

			adjustedTop += m_pG->tlu(3);
			adjustedRight += m_pG->tlu(1);
			painter.drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);

			adjustedRight += m_pG->tlu(1);
			painter.drawLine(adjustedRight, adjustedTop, adjustedRight, adjustedBottom);
		}

		// advance to the next page
		pPage = pPage -> getNext();
	}

	if (bClip)
	{
		m_pG->setClipRect(NULL);
	}

	xxx_UT_DEBUGMSG(("End _draw\n"));
}


void FV_View::_setPoint(fv_CaretProps * pCP,PT_DocPosition pt, UT_sint32 iLen) const
{
	getGraphics()->allCarets()->disable();
	pCP->m_iInsPoint = pt + iLen;
	_fixInsertionPointCoords(pCP);
	getGraphics()->allCarets()->enable();
}


void FV_View::_setPoint(PT_DocPosition pt, bool bEOL)
{
	if (!m_pDoc->getAllowChangeInsPoint())
		return;
	if(!m_pDoc->isPieceTableChanging())
	{
//
// Have to deal with special case of point being exactly on a footnote/endnote
// boundary. Move the point past the footnote so we always have Footnote field
// followed by footnotestrux in the piecetable
//
		fl_FootnoteLayout * pFL = NULL;
		if(m_pDoc->isFootnoteAtPos(pt))
		{
			pFL = getClosestFootnote(pt);
			if(pFL == NULL)
			{
				fl_EndnoteLayout * pEL = getClosestEndnote(pt);
				if(pEL)
				{
					pt += pEL->getLength();
				}
			}
			else
			{
				pt += pFL->getLength();
			}
		}		
	}
	m_iInsPoint = pt;
	m_Selection.checkSelectAll();
	m_bInsertAtTablePending = false;
	m_iPosAtTable = 0;
	xxx_UT_DEBUGMSG(("Point set to %d in View %x \n",pt,this));
	m_bPointEOL = bEOL;
	if(!m_pDoc->isPieceTableChanging())
	{
		_fixInsertionPointCoords(true);
		m_pLayout->considerPendingSmartQuoteCandidate();
#ifdef ENABLE_SPELL
		_checkPendingWordForSpell();
#endif
	// So, if there is a selection now, we should disable the cursor; conversely,
	// if there is no longer a selection, we should enable the cursor.
		if (isSelectionEmpty())
		{	
			while(m_countDisable > 0)
			{
			  if(m_pG)
			    m_pG->allCarets()->enable();
			  m_countDisable--;
			}
			if(m_pG) {
			  m_pG->allCarets()->disable();
			  m_pG->allCarets()->enable();
			}
		}
		else
		{	
//
// We have to remember the number of times we disabled the cursor and wind
// them back to re-enable it because the cursor class keeps a count this to
// handle nested disable calls.
//

		  if(m_pG)
		    m_pG->allCarets()->disable();
		  m_countDisable++;
		}
	}
}


#ifdef ENABLE_SPELL
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
#endif

UT_uint32 FV_View::_getDataCount(UT_uint32 pt1, UT_uint32 pt2) const
{
	UT_ASSERT(pt2>=pt1);
	return pt2 - pt1;
}


bool FV_View::_charMotion(bool bForward,UT_uint32 countChars, bool bSkipCannotContainPoint)
{
	// advance(backup) the current insertion point by count characters.
	// return false if we ran into an end (or had an error).
	bool bInsertAtTable = false;
	PT_DocPosition posTable = 0;
	PT_DocPosition posOld = m_iInsPoint;
	fp_Run* pRun = NULL;
	fl_BlockLayout* pBlock = NULL;
	UT_sint32 x=0;
	UT_sint32 y=0;
	UT_sint32 x2=0;
	UT_sint32 y2=0;
	bool bDirection=false;
	UT_uint32 uheight;
	m_bPointEOL = false;
	UT_sint32 iOldDepth = getEmbedDepth(getPoint());
	xxx_UT_DEBUGMSG(("_charMotion: Old Position is %d embed depth %d \n",posOld,iOldDepth));
	/*
	  we don't really care about the coords.  We're calling these
	  to get the Run pointer
	*/
	PT_DocPosition posBOD=0;
	PT_DocPosition posEOD=0;
	bool bRes;

	bRes = getEditableBounds(false, posBOD);
	bRes = getEditableBounds(true, posEOD);
	UT_ASSERT(bRes);

	// FIXME:jskov want to rewrite this code to use simplified
	// versions of findPositionCoords. I think there's been some bugs
	// due to that function being overloaded to be used from this
	// code.
	UT_sint32 xold,yold,x2old,y2old=0;
	bool bDirectionOld=false;
	xxx_UT_DEBUGMSG(("Count Chars %d \n",countChars));
	_findPositionCoords(m_iInsPoint, false, xold, yold, x2old,y2old,uheight, bDirectionOld, &pBlock, &pRun);
	if (bForward)
	{
		xxx_UT_DEBUGMSG(("Just before First forward _setPoint %d \n",m_iInsPoint));
		_setPoint(m_iInsPoint + countChars);
		xxx_UT_DEBUGMSG(("Just After First forward _setPoint %d \n",m_iInsPoint));
//
// Scan past any strux boundaries (like table controls
//
		while(getPoint() < posEOD && !isPointLegal())
		{
			xxx_UT_DEBUGMSG(("Forward scan past illegal point pos 1 %d \n",m_iInsPoint));
			_setPoint(m_iInsPoint + 1);
			xxx_UT_DEBUGMSG(("Forward scan past illegal point pos 2 %d \n",m_iInsPoint));
		}
		_findPositionCoords(m_iInsPoint-1, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);
//
// If we come to a table boundary we have doc positions with no blocks.
// _findPositionCoords signals this by returning pRun == NULL
//
		// I have added the bDirection == bDirectionOld condition
		// because without it the code did not work on direction
		// boundaries. However, I am not sure whether the whole of the
		// x,y test is at all desirable here; any idea why it is here?
		// Tomas, Jan 16, 2003.

		// Testing the coords is definitely wrong; combining characters often do not advance x,y,
		// but need to be treated as a valid document position. See bug 6987. Tomas, July 27, 2004

		bool bExtra = false;
		while(m_iInsPoint <= posEOD && (pRun == NULL /*|| ((x == xold) && (y == yold) &&
														 (x2 == x2old) && (y2 == y2old) &&
														 (bDirection == bDirectionOld))*/))
		{
			xxx_UT_DEBUGMSG(("fv_View_protected: (2) pRun = %x pos %d\n",pRun,m_iInsPoint));
			_setPoint(m_iInsPoint+1);
			xxx_UT_DEBUGMSG(("fv_View_protected: (3) pRun = %x pos %d \n",pRun,m_iInsPoint));
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
		UT_sint32 iPos = static_cast<UT_sint32>(m_iInsPoint) - static_cast<UT_sint32>(countChars);
		PT_DocPosition realBOD =0;
		if(posBOD == 2)
	    {
			realBOD = 0;
		}
		else
		{
			realBOD = posBOD;
			while(!m_pDoc->isHdrFtrAtPos(realBOD) && (realBOD > 0))
			{
				realBOD--;
			}
		}
		if(iPos > 0 && iPos > static_cast<UT_sint32>(realBOD))
		{
			_setPoint(m_iInsPoint - countChars);
		}
		else
		{
			_setPoint(posBOD);
		}
//
// Scan past any strux boundaries (like table controls
//
// when moving backwards, we need to also skip over the EndOfFootnote struxes
//
		bool bGotTableStrux = false;
		bool bGotOtherStrux = false;
		while(getPoint() > realBOD && !isPointLegal())
		{
			_setPoint(m_iInsPoint - 1);
			xxx_UT_DEBUGMSG(("Backward scan past illegal point pos %d \n",m_iInsPoint));
			if(!bGotOtherStrux && m_pDoc->isTableAtPos(getPoint()))
			{
				bGotTableStrux = true;
				posTable = getPoint();
			}
			else if(!bGotOtherStrux && bGotTableStrux)
			{
				if(m_pDoc->isEndTableAtPos(getPoint()) ||
				   m_pDoc->isCellAtPos(getPoint()) ||
				   m_pDoc->isSectionAtPos(getPoint()) ||
				   m_pDoc->isHdrFtrAtPos(getPoint()) ||
				   m_pDoc->isFrameAtPos(getPoint()))
				{
					  bGotOtherStrux = true;
				}
			}
		}
	if(bGotOtherStrux && bGotTableStrux)
		{
			bInsertAtTable = true;
		}
		if(getPoint() < posBOD)
		{
			_setPoint(posBOD);
		}
		_findPositionCoords(m_iInsPoint, false, x, y, x2,y2,uheight, bDirection, &pBlock, &pRun);

//
// If we come to a table boundary we have doc positions with no blocks.
// _findPositionCoords signals this by returning pRun == NULL
//
		// I have added the bDirection == bDirectionOld condition
		// because without it the code did not work on direction
		// boundaries. However, I am not sure whether the whole of the
		// x,y test is at all desirable here; any idea why it is here?
		// Tomas, Jan 16, 2003.

		// Testing the coords is definitely wrong; combining characters often do not advance x,y,
		// but need to be treated as a valid document position. See bug 6987. Tomas, July 27, 2004

		bool bExtra = false;
		while( m_iInsPoint >= posBOD && (pRun == NULL /*|| ((x == xold) && (y == yold) &&
														 (x2 == x2old) && (y2 == y2old) &&
														 (bDirection == bDirectionOld))*/))
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

	UT_return_val_if_fail(pBlock, false);
	UT_return_val_if_fail(pRun, false);
	
	PT_DocPosition iRunStart = pBlock->getPosition(false) + pRun->getBlockOffset();
	PT_DocPosition iRunEnd = iRunStart + pRun->getLength();

	// containing layout we will work with, ususally section
// 	fl_ContainerLayout * pCL = pBlock->myContainingLayout();

	// the layout immediately above the runs, should be block
// 	fl_ContainerLayout * pBL = pBlock;

	// indicates how many layout layers we had to step up to get valid pCL
// 	UT_uint32 iLayoutDepth = 0;

	if(iRunEnd > posEOD)
		iRunEnd = posEOD;

	if(bForward && ( m_iInsPoint > iRunEnd))
	{
		// the run we have got is the on left of the ins point, we
		// need to find the right one and set the point there; we also
		// need to make sure that we handle correctly any hidden
		// sub layouts (blocks, sections, table cells, tables ...)

		// get the next run that can contain insertion point
		pRun = pRun->getNextRun();
		UT_uint32 iLength = 0;
		while(pRun && ((bSkipCannotContainPoint && !pRun->canContainPoint()) || pRun->getLength() == 0))
		{
			xxx_UT_DEBUGMSG(("_charMotion: Sweep forward through runs %d \n",pRun->getLength()));
			iLength += pRun->getLength();
			pRun = pRun->getNextRun();
		}

		if(pRun)
		{
			_setPoint(m_iInsPoint + iLength);
		}
		else
		{
//
// FIXME: Put in some code here to handle table/cell boundaries. Right
// now you have to press left arrow twice to move form outside to inside
// a table.

		}
	}
	// these two branches ensure that insertion point is not moved just after a run that is not
	// suppossed to take point
	else if(bSkipCannotContainPoint && bForward && m_iInsPoint == iRunStart && pRun->getLength()>0)
	{
		// moving forward, with insertion point ending between two runs;
		// we need to check that the previous run was not one that cannot take point, in which case
		// we need to advance the point by one (otherwise it will appear just after the non-point
		// run
		// this case happens, for example, when the user has a hyperlink at the start of line and
		// presses HOME, RIGHT. The HOME key takes her before the hyperlink, the right, however,
		// should skip over they hyperlink run
		
		pRun = pRun->getPrevRun();
		if(pRun && !pRun->canContainPoint())
			_setPoint(m_iInsPoint + 1);
	}
	else if(bSkipCannotContainPoint && !bForward && m_iInsPoint == iRunStart)
	{
		// moving backwards, with insertion point ending between two runs; we need to scroll
		// through any adjucent runs on the left that cannot contain point
		pRun = pRun->getPrevRun();
		UT_uint32 iLength = 0;
		while(pRun && !pRun->canContainPoint())
		{
			iLength += pRun->getLength();
			pRun = pRun->getPrevRun();
		}

		// do this unconditionally; if !pRun, we are at the start of the block
		_setPoint(m_iInsPoint - iLength);
	}

	// this is much simpler, since the findPointCoords will return the
	// run on the left of the requested position, so we just need to move
	// to its end if the position does not fall into that run
	xxx_UT_DEBUGMSG(("_charMotion: iRunEnd %d \n",iRunEnd));
	if(!bForward && (iRunEnd < m_iInsPoint) && (pRun->getBlockOffset() > 0))
	{
		_setPoint(iRunEnd - 1);
	}

	// now have the get the run that actualy holds this position, and let it do any internal
	// adjustments (as needed for complex scripts, e.g., Thai) not sure whether this should not come
	// after the footnote sweep
	if(bSkipCannotContainPoint)
	{
		
		pBlock = _findBlockAtPosition(getPoint());
		UT_return_val_if_fail( pBlock, false );

		pRun = pBlock->findRunAtOffset(getPoint() - pBlock->getPosition());

		// at the end of document we do not have a run ...
		if(pRun)
		{
			UT_uint32 iAdjustedPos = pRun->adjustCaretPosition(getPoint(), bForward);

			if(iAdjustedPos != getPoint())
			{
				xxx_UT_DEBUGMSG(("FV_View::_charMotion: orig pos %d, adjusted to %d\n", getPoint(), iAdjustedPos));
				_setPoint(iAdjustedPos);
			}
		}
	}
	
//
// OK sweep through footnote sections without stopping
	xxx_UT_DEBUGMSG(("Point is %d inFootnote %d bOldFootnote %d \n",m_iInsPoint,isInFootnote(),iOldDepth));
	if(bForward)
	{
		if(iOldDepth < getEmbedDepth(m_iInsPoint))
		{
			bool bSweep = false;
			while(m_iInsPoint <= posEOD && ( (iOldDepth < getEmbedDepth(m_iInsPoint)) || m_pDoc->isEndFootnoteAtPos(getPoint())))
			{ 
				xxx_UT_DEBUGMSG(("_charMotion: Sweep forward -1 %d \n",m_iInsPoint));
				bSweep = true;
				m_iInsPoint++;
			}
			if(bSweep && (m_iInsPoint <= posEOD))
			{
				_setPoint(m_iInsPoint);
			}
			else if(m_iInsPoint > posEOD)
			{
				_setPoint(posEOD);
			}
		}
		else if((iOldDepth > getEmbedDepth(m_iInsPoint)) )
		{
			bool bSweep = false;
			while( ((m_iInsPoint > posBOD) && (iOldDepth > getEmbedDepth(m_iInsPoint))) || m_pDoc->isFootnoteAtPos(getPoint()) )
			{
				xxx_UT_DEBUGMSG(("_charMotion: Sweep backward -1 %d \n",m_iInsPoint));
				m_iInsPoint--;
				bSweep = true;
			}
			if(bSweep)
			{
				_setPoint(m_iInsPoint);
			}
		}

	}
	else
	{
		if(iOldDepth < getEmbedDepth(m_iInsPoint))
		{
			bool bSweep = false;
			while(((iOldDepth < getEmbedDepth(m_iInsPoint)) || ((m_pDoc->isFootnoteAtPos(getPoint()) ) && (m_iInsPoint >= posBOD))))
			{ 
				xxx_UT_DEBUGMSG(("_charMotion: Sweep backward -2 %d \n",m_iInsPoint));
				bSweep = true;
				m_iInsPoint--;
			}
			if(bSweep && (m_iInsPoint >= posBOD))
			{
				_setPoint(m_iInsPoint);
			}
			else if(m_iInsPoint > posEOD)
			{
				_setPoint(posOld);
			}
		}
	    else
		{
			bool bSweep = false;
			while((m_iInsPoint < posEOD) &&((iOldDepth > getEmbedDepth(m_iInsPoint)) || m_pDoc->isEndFootnoteAtPos(getPoint())))
			{
				xxx_UT_DEBUGMSG(("_charMotion: Sweep forward -2 %d \n",m_iInsPoint));
				m_iInsPoint++;
				bSweep = true;
			}
			if(bSweep)
			{
				_setPoint(m_iInsPoint);
			}
		}
	}
	PT_DocPosition legalBOD = posBOD;
	if(!isHdrFtrEdit())
	{
		fl_DocSectionLayout * pDSL = m_pLayout->getFirstSection();
		if(pDSL == NULL)
		{
			legalBOD =2;
		}
		else
		{
			fl_BlockLayout * pBL = pDSL->getFirstBlock();
			if(pBL != NULL)
			{
					legalBOD = pBL->getPosition(false);
			}
			else
			{
					legalBOD = 2;
			}
		}
	}
	if (static_cast<UT_sint32>(m_iInsPoint) < static_cast<UT_sint32>(legalBOD))
	{
		_setPoint(legalBOD);
		bRes = true;
	}
	else if (static_cast<UT_sint32>(m_iInsPoint) > static_cast<UT_sint32>(posEOD))
	{
		m_bPointEOL = true;
		_setPoint(posEOD);
		bRes = false;
	}
	if(m_iInsPoint < legalBOD)
	{
		_setPoint(legalBOD);
		bRes = true;
	}
	if(bInsertAtTable)
	{
		m_bInsertAtTablePending = true;
		m_iPosAtTable =posTable;
	}
	if (m_iInsPoint != posOld)
	{
		m_pLayout->considerPendingSmartQuoteCandidate();
#ifdef ENABLE_SPELL
		_checkPendingWordForSpell();
#endif
		_clearIfAtFmtMark(posOld);
		if(!m_pDoc->isDoingPaste())
		{
			notifyListeners(AV_CHG_MOTION);
		}
	}
	if(m_FrameEdit.isActive())
	{
		m_FrameEdit.setMode(FV_FrameEdit_NOT_ACTIVE);
	}
	xxx_UT_DEBUGMSG(("SEVIOR: Point = %d \n",getPoint()));
	_fixInsertionPointCoords();
	return (bRes && m_iInsPoint != posOld);
}


void FV_View::_doPaste(bool bUseClipboard, bool bHonorFormatting)
{
	// internal portion of paste operation.

	if (!isSelectionEmpty() && !m_FrameEdit.isActive())
		_deleteSelection();
	else if(m_FrameEdit.isActive())
	{
	       m_FrameEdit.setPointInside();
	}

	_clearIfAtFmtMark(getPoint());
	PD_DocumentRange dr(m_pDoc,getPoint(),getPoint());
	m_pApp->pasteFromClipboard(&dr,bUseClipboard,bHonorFormatting);
	insertParaBreakIfNeededAtPos(getPoint());
	fl_SectionLayout * pSL = getCurrentBlock()->getSectionLayout();
	m_pDoc->setDontImmediatelyLayout(false);
	pSL->checkAndAdjustCellSize();
	_generalUpdate();

	_updateInsertionPoint();
}


UT_Error FV_View::_deleteBookmark(const char* szName, bool bSignal, PT_DocPosition *posStart, PT_DocPosition *posEnd)
{
	if(!m_pDoc->isBookmarkUnique(static_cast<const gchar *>(szName)))
	{
		// even though we will only send out a single explicit deleteSpan
		// call, we need to find out where both of the markers are in the
		// document, so that the caller can adjust any stored doc positions
		// if necessary

		PT_DocPosition pos1, pos2;

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
			pBL = pSL->getNextBlockInDocument();

			while(pBL)
			{
				pRun = pBL->getFirstRun();

				while(pRun)
				{
					if(pRun->getType()== FPRUN_BOOKMARK)
					{
						pB1 = static_cast<fp_BookmarkRun*>(pRun);
						if(!strcmp(static_cast<const gchar *>(szName), pB1->getName()))
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
					pRun = pRun->getNextRun();
				}
				if(bFound)
					break;
				pBL = static_cast<fl_BlockLayout *>(pBL->getNext());
			}
			if(bFound)
				break;
			pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
		}

		UT_ASSERT(pRun && pRun->getType()==FPRUN_BOOKMARK && pBlock[0] && pBlock[1]);
		if(!pRun || pRun->getType()!=FPRUN_BOOKMARK || !pBlock[0] || !pBlock[1])
			return false;

		// Signal PieceTable Change
		if(bSignal)
			_saveAndNotifyPieceTableChange();

		UT_DEBUGMSG(("fv_View::cmdDeleteBookmark: bl pos [%d,%d], bmOffset [%d,%d]\n",
					 pBlock[0]->getPosition(false), pBlock[1]->getPosition(false),bmBlockOffset[0],bmBlockOffset[1]));

		pos1 = pBlock[0]->getPosition(false) + bmBlockOffset[0];
		pos2 = pBlock[1]->getPosition(false) + bmBlockOffset[1];

		if (posStart && *posStart > pos1)
			(*posStart)--;
		if (posStart && *posStart > pos2)
			(*posStart)--;

		if (posEnd && *posEnd > pos1)
			(*posEnd)--;
		if (posEnd && *posEnd > pos1)
			(*posEnd)--;

		UT_uint32 iRealDeleteCount;

		m_pDoc->deleteSpan(pos1,pos1 + 1,NULL,iRealDeleteCount);
		// TODO -- add proper revision handling using iRealDeleteCount

		// Signal PieceTable Changes have finished
		if(bSignal)
		{
			_restorePieceTableState();
			_generalUpdate();
		}
	}
	else {
		UT_DEBUGMSG(("fv_View::cmdDeleteBookmark: bookmark \"%s\" does not exist\n",szName));
	}
	return true;
}



/*! Returns the hyperlink around position pos, if any; assumes
 * posStart, posEnd in same block. */
fp_HyperlinkRun * FV_View::_getHyperlinkInRange(PT_DocPosition &posStart,
												PT_DocPosition &posEnd)
{
	fl_BlockLayout *pBlock = _findBlockAtPosition(posStart);
	PT_DocPosition curPos = posStart - pBlock->getPosition(false);
	if(curPos <2)
		return NULL;
	fp_Run * pRun = pBlock->getFirstRun();

	//find the run at pos
	while(pRun && pRun->getBlockOffset() <= curPos)
		pRun = pRun->getNextRun();

	UT_return_val_if_fail(pRun,NULL);

	// now we have the run immediately after the run in question, so
	// we step back
	pRun = pRun->getPrevRun();
	UT_return_val_if_fail(pRun,NULL);

	if (pRun->getHyperlink() != NULL)
		return pRun->getHyperlink();

	// Now, getHyperlink() looks NULL, so let's step forward till posEnd.

	PT_DocPosition curPosEnd = posEnd - pBlock->getPosition(false);

	// Continue checking for hyperlinks.
	while(pRun && pRun->getBlockOffset() <= curPosEnd)
	{
		pRun = pRun->getNextRun();
		if (pRun && pRun->getPrevRun() && pRun->getPrevRun()->getHyperlink() != NULL)
			return pRun->getPrevRun()->getHyperlink();
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
	UT_return_val_if_fail(pH1,false);
	fp_AnnotationRun  * pAR = NULL;
	UT_uint32 iRunLen = 1;
	if(pH1->getHyperlinkType() ==  HYPERLINK_ANNOTATION)
	{
			pAR = static_cast<fp_AnnotationRun *>(pH1);
			fl_AnnotationLayout * pAL = getLayout()->findAnnotationLayout(pAR->getPID());
			UT_return_val_if_fail(pAL,false);
			iRunLen = pAL->getLength();
			
	}

	if (!isSelectionEmpty())
		_clearSelection();

	pos1 = pH1->getBlock()->getPosition(false) + pH1->getBlockOffset();

	// Signal PieceTable Change
	if(bSignal)
		_saveAndNotifyPieceTableChange();

	UT_DEBUGMSG(("fv_View::cmdDeleteHyperlink() position:%d len:%d\n",
				 pos1, iRunLen ));

	UT_uint32 iRealDeleteCount;
	m_pDoc->beginUserAtomicGlob();
	m_pDoc->deleteSpan(pos1,pos1 + iRunLen,NULL, iRealDeleteCount);

	// TODO -- add proper revision handling using iRealDeleteCount

	// Signal PieceTable Changes have finished
	m_pDoc->endUserAtomicGlob();
	if(bSignal)
	{
		_restorePieceTableState();
		_generalUpdate();
	}
	return true;
}


UT_Error FV_View::_deleteXMLID( const std::string& xmlid, bool bSignal, PT_DocPosition& extPosStart, PT_DocPosition& extPosEnd )
{
    PD_DocumentRDFHandle rdf = m_pDoc->getDocumentRDF();
    std::pair< PT_DocPosition, PT_DocPosition > range = rdf->getIDRange( xmlid );

	UT_DEBUGMSG(("_deleteXMLID() xmlid:%s point:%d\n", xmlid.c_str(), getPoint() ));
	UT_DEBUGMSG(("_deleteXMLID() xmlid:%s start:%d end:%d\n", xmlid.c_str(), range.first, range.second ));

	if( range.first == range.second )
 	{
		return UT_ERROR;
	}
	
	fp_HyperlinkRun* r = _getHyperlinkInRange( range.first, range.first );
	UT_DEBUGMSG(("_deleteXMLID() xmlid:%s r:%p\n", xmlid.c_str(), r ));
	if( !r )
 	{
		return UT_ERROR;
	}
		
	UT_DEBUGMSG(("_deleteXMLID() xmlid:%s type:%d\n", xmlid.c_str(), r->getHyperlinkType() ));
	if( r->getHyperlinkType() ==  HYPERLINK_RDFANCHOR )
	{
		UT_DEBUGMSG(("_deleteXMLID() xmlid:%s len:%d\n", xmlid.c_str(), r->getLength() ));
	}

	if (!isSelectionEmpty())
		_clearSelection();
	
	PT_DocPosition pos1 = r->getBlock()->getPosition(false) + r->getBlockOffset();
	int iRunLen = 1;
	UT_DEBUGMSG(("_deleteXMLID() xmlid:%s pos1:%d\n", xmlid.c_str(), pos1 ));
	
	// Signal PieceTable Change
	if(bSignal)
		_saveAndNotifyPieceTableChange();

	UT_uint32 iRealDeleteCount;
	m_pDoc->beginUserAtomicGlob();
	m_pDoc->deleteSpan(pos1,pos1 + iRunLen,NULL, iRealDeleteCount);
	if( extPosStart > pos1 )
		extPosStart -= 2;
	if( extPosEnd > pos1 )
		extPosEnd   -= 2;
	
	// TODO -- add proper revision handling using iRealDeleteCount

	// Signal PieceTable Changes have finished
	m_pDoc->endUserAtomicGlob();
	if(bSignal)
	{
		_restorePieceTableState();
		_generalUpdate();
	}

	return UT_OK;
}
	

UT_Error FV_View::_deleteXMLID( const std::string& xmlid, bool bSignal )
{
	PT_DocPosition s,e;
	return _deleteXMLID( xmlid, bSignal, s, e );
}



UT_Error FV_View::_insertGraphic(const FG_ConstGraphicPtr& pFG, const char* szName)
{
	UT_return_val_if_fail(pFG,UT_ERROR);
	UT_ASSERT(szName);

	if(!isPointLegal(getPoint()))
	{
		_makePointLegal();
	}
	return pFG->insertIntoDocument(m_pDoc, m_pG->getDeviceResolution(), getPoint(), szName);
}


UT_Error FV_View::_insertGraphic(const FG_ConstGraphicPtr& pFG, const char* szName, PT_DocPosition pos)
{
	UT_return_val_if_fail(pFG,UT_ERROR);
	UT_ASSERT(szName);
	PT_DocPosition posEOD,posBOD;
	bool bRes;
	bRes = getEditableBounds(true, posEOD);
	bRes = getEditableBounds(false, posBOD);
	bRes = false;
	while(!isPointLegal(pos) && (pos <= posEOD))
	{
		pos++;
	}
	if(pos > posEOD)
	{
		while(!isPointLegal(pos) && pos >= posBOD)
		{
			pos--;
		}
		if(pos >= posBOD)
			bRes = true;
	}
	else
	{
		bRes = true;
	}
	if(!bRes)
		return UT_ERROR;

	return pFG->insertIntoDocument(m_pDoc, m_pG->getDeviceResolution(), pos, szName);
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
		//	_generalUpdate();//  Sevior: May be able to live with notify.. always
	}
	else
	{
		notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK);
	}
}


#ifdef ENABLE_SPELL
// NB: returns a UCS string that the caller needs to FREEP
UT_UCSChar * FV_View::_lookupSuggestion(fl_BlockLayout* pBL,
										fl_PartOfBlock* pPOB, UT_sint32 ndx)
{
	// mega caching - are these assumptions valid?
	UT_UCSChar * szSuggest = NULL;

	// TODO these should really be static members, so we can properly
	// clean up
	static fl_BlockLayout * s_pLastBL = 0;
	static fl_PartOfBlock * s_pLastPOB = 0;
	static const UT_GenericVector<UT_UCSChar*>* s_pvCachedSuggestions = 0;

	// can we use the cached suggestions?
	if (pBL != s_pLastBL || pPOB != s_pLastPOB)
	{
		// no we cannot, so we empty and invalidate the cache
		if (s_pvCachedSuggestions)
		{
			// clean up
			for (UT_sint32 i = 0; i < s_pvCachedSuggestions->getItemCount(); i++)
			{
				const UT_UCSChar * sug = s_pvCachedSuggestions->getNthItem(i);
				FREEP(sug);
			}

			s_pLastBL = 0;
			s_pLastPOB = 0;
			DELETEP(s_pvCachedSuggestions);
		}

		// grab a copy of the word
		UT_GrowBuf pgb(1024);
		bool bRes = pBL->getBlockBuf(&pgb);
		UT_ASSERT(bRes);
		if(!bRes) 
		{
			UT_WARNINGMSG(("getBlockBuf() failed in %s:%d",
						   __FILE__, __LINE__));
		}

		UT_UCS4String stMisspelledWord;
		// convert smart quote apostrophe to ASCII single quote to be
		// compatible with ispell
		const UT_UCSChar * pWord;
		UT_sint32 iLength, iPTLength, iBlockPos;

		fl_BlockSpellIterator BSI(pBL, pPOB->getOffset());
		BSI.nextWordForSpellChecking(pWord, iLength, iBlockPos, iPTLength);
		
		UT_uint32 len = iLength;
		for (UT_uint32 ldex=0; ldex < len && ldex < INPUTWORDLEN; ldex++)
		{
			stMisspelledWord += *pWord == UCS_RQUOTE ? '\'' : *pWord;
			++pWord;
		}

		// get language code for misspelled word
		std::string lang;

		PP_PropertyVector props_in;

		if (getCharFormat(props_in))
		{
			lang = PP_getAttribute("lang", props_in);
		}

		// get spellchecker engine for language code
		SpellChecker * checker = NULL;

		if (!lang.empty())
		{
			// we get smart and request the proper dictionary
			checker = SpellManager::instance().requestDictionary(lang.c_str());
		}
		else
		{
			// we just (dumbly) default to the last dictionary
			// TODO this is known to return the wrong dictionary sometimes in multilanguge docs
			checker = SpellManager::instance().lastDictionary();
		}

		// lookup suggestions

		// create an empty vector
		UT_GenericVector<UT_UCSChar*>* pvFreshSuggestions = 0;
		UT_ASSERT(!pvFreshSuggestions);

		pvFreshSuggestions = new UT_GenericVector<UT_UCSChar*>();
		UT_ASSERT(pvFreshSuggestions);

		if (checker && (checker->checkWord(stMisspelledWord.ucs4_str(), iLength) == SpellChecker::LOOKUP_FAILED))
		{
			// get suggestions from spelling engine
			const UT_GenericVector<UT_UCSChar*>* cpvEngineSuggestions;

			cpvEngineSuggestions = checker->suggestWord (stMisspelledWord.ucs4_str(), iLength);

			for (UT_sint32 i = 0; i < cpvEngineSuggestions->getItemCount(); ++i)
			{
				UT_UCSChar *sug = cpvEngineSuggestions->getNthItem(i);
				UT_ASSERT(sug);
				pvFreshSuggestions->addItem(sug);
			}

			// add suggestions from user's AbiWord file
			 m_pApp->suggestWord(pvFreshSuggestions,stMisspelledWord.ucs4_str(), iLength);
		}

		// update static vars for next call
		s_pvCachedSuggestions = pvFreshSuggestions;
		s_pLastBL = pBL;
		s_pLastPOB = pPOB;
	}

	// return the indexed suggestion from the cache
	if ((s_pvCachedSuggestions->getItemCount()) &&
		( ndx <= s_pvCachedSuggestions->getItemCount()))
	{
		UT_UCS4_cloneString(&szSuggest, s_pvCachedSuggestions->getNthItem(ndx-1));
	}

	return szSuggest;
}
#endif

void FV_View::_prefsListener( XAP_Prefs *pPrefs, UT_StringPtrMap * /*phChanges*/, void *data )
{
	FV_View *pView = static_cast<FV_View *>(data);
	bool b;
	UT_ASSERT(data && pPrefs);
	if ( pPrefs->getPrefsValueBool(static_cast<const gchar*>(AP_PREF_KEY_CursorBlink), &b) && b != pView->m_bCursorBlink )
	{
		UT_DEBUGMSG(("FV_View::_prefsListener m_bCursorBlink=%s m_bCursorIsOn=%s\n",
					 pView->m_bCursorBlink ? "TRUE" : "FALSE",
					 pView->m_bCursorIsOn ? "TRUE" : "FALSE"));

		pView->m_bCursorBlink = b;
		pView->m_pG->allCarets()->setBlink(b);
	}


	// Update colors
   	const gchar * pszTmpColor = NULL;
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForShowPara), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorShowPara);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForSquiggle), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorSpellSquiggle);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForGrammarSquiggle), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorGrammarSquiggle);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForMargin), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorMargin);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForFieldOffset), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorFieldOffset);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForImage), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorImage);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForHyperLink), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorHyperLink);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForHdrFtr), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorHdrFtr);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForColumnLine), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorColumnLine);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision1), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[0]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision2), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[1]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision3), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[2]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision4), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[3]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision5), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[4]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision6), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[5]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision7), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[6]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision8), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[7]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision9), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[8]);
	}
	if (pPrefs->getPrefsValue(static_cast<const gchar *>(XAP_PREF_KEY_ColorForRevision10), &pszTmpColor))
	{
		UT_parseColor(pszTmpColor, pView->m_colorRevisions[9]);
	}
	pView->m_bgColorInitted = false; // force refresh/update on next getColorSelBackground () call
	//
	// Set this true to redraw the entire screen during the configure event
	// Other we get lefover crap at the top of the window
	//
	pView->setConfigure(true);

	// FIXME:jskov: is it necessary to do something here to cause a full redraw?
	if (!pView->m_bWarnedThatRestartNeeded &&
		(( (pPrefs->getPrefsValueBool(static_cast<const gchar*>(AP_PREF_KEY_DefaultDirectionRtl), &b) && b != pView->m_bDefaultDirectionRtl))
		 || ((pPrefs->getPrefsValueBool(static_cast<const gchar*>(XAP_PREF_KEY_UseHebrewContextGlyphs), &b) && b != pView->m_bUseHebrewContextGlyphs)))
		)
	{
		/*	It is possible to change this at runtime, but it may impact the
			way the document is displayed in an unexpected way (from the user's
			point of view). It is therefore probably better to apply this change
			only when AW will be restarted or a new document is created and
			notify the user about that.
		*/
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(pView->getParentData());
		UT_return_if_fail((pFrame));

		pFrame->showMessageBox(AP_STRING_ID_MSG_AfterRestartNew, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
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

	UT_return_if_fail(pHdrFtrSrc->getFirstLayout());
	iPos1 = m_pDoc->getStruxPosition(pHdrFtrSrc->getFirstLayout()->getStruxDocHandle());

	fl_BlockLayout * pLast = static_cast<fl_BlockLayout *>(pHdrFtrSrc->getLastLayout());
	iPos2 = pLast->getPosition(false);
//
// This code assumes there is an End of Block run at the end of the Block.
// Thanks to Jesper, there always is!
//
	while(pLast->getNext() != NULL)
	{
		pLast = static_cast<fl_BlockLayout *>(pLast->getNext());
	}
	fp_Run * pRun = pLast->getFirstRun();
	while( pRun->getNextRun() != NULL)
	{
		pRun = pRun->getNextRun();
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
	UT_DEBUGMSG(("view_protected: Removing HdrFtr %p \n",pHdrFtr));
//
// Need this to remove the HdrFtr attributes in the section strux.
//
	const gchar * pszHdrFtrType = NULL;
	UT_ASSERT(pHdrFtr->getContainerType() == FL_CONTAINER_HDRFTR);
	pf_Frag_Strux* sdhHdrFtr = pHdrFtr->getStruxDocHandle();
	m_pDoc->getAttributeFromSDH(sdhHdrFtr,isShowRevisions(),getRevisionLevel(),PT_TYPE_ATTRIBUTE_NAME, &pszHdrFtrType);
//
// Remove the header/footer strux
//
	m_pDoc->deleteHdrFtrStrux(sdhHdrFtr);
}

void FV_View::_cmdEditHdrFtr(HdrFtrType hfType)
{
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
	
	if(isHdrFtrEdit())
		clearHdrFtrEdit();
	pShadow = pHFCon->getShadow();
	UT_return_if_fail(pShadow);
//
// Put the insertion point at the beginning of the header
//
	fl_BlockLayout * pBL = pShadow->getNextBlockInDocument();
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
		m_iPieceTableState =0;
	}
}


void FV_View::_fixInsertionPointAfterRevision()
{
	if(!m_pDoc->isMarkRevisions() && isSelectionEmpty())
	{
		UT_DebugOnly<bool> bRet;

		// Signal PieceTable Change
		_saveAndNotifyPieceTableChange();

		PT_DocPosition posStart = getPoint();
		PT_DocPosition posEnd = posStart;

		PP_PropertyVector attr = {
			"revision", ""
		};

		bRet = m_pDoc->changeSpanFmt(PTC_RemoveFmt, posStart, posEnd, attr, PP_NOPROPS);
		UT_ASSERT(bRet);

		// Signal piceTable is stable again
		_restorePieceTableState();

		// might need to do general update here; leave it off for now
		// _generalUpdate();
		_fixInsertionPointCoords();
	}
}

bool FV_View::_makePointLegal(void)
{
		bool bOK = true;
		while(!isPointLegal() && bOK)
		{
//
// If we're in an illegal position move forward till we're safe.
//
			bOK = _charMotion(true,1);
		}
		PT_DocPosition posEnd = 0;
		getEditableBounds(true, posEnd);
		if(posEnd == getPoint() && !isPointLegal())
		{
			bOK = _charMotion(false,1);
		}
		if(posEnd-1 == getPoint() && !isPointLegal())
		{
			bOK = _charMotion(false,1);
		}
		if(posEnd-1 == getPoint() && m_pDoc->isEndFrameAtPos(getPoint()) && m_pDoc->isFrameAtPos(getPoint()-1))
		{
			bOK = _charMotion(false,1);
		}
		while(bOK && !isPointLegal())
		{
			bOK = _charMotion(false,1);
		}
		return bOK;
}

bool FV_View::_charInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce)
{
	
	// see if prefs specify we should set language based on kbd layout
	UT_return_val_if_fail(m_pApp, false);
	bool bSetLang = false;
	m_pApp->getPrefsValueBool(static_cast<const gchar*>(XAP_PREF_KEY_ChangeLanguageWithKeyboard),
							  &bSetLang);

	const UT_LangRecord * pLR = NULL;

	if(bSetLang)
		pLR = m_pApp->getKbdLanguage();

	
	bool bResult = true;
	// So this gets rid of the annoying cursor flash at the beginning
	// of the line upon character insertion, but it's the wrong thing to
	// do.  The right thing to do is to either delay calculation, or to
	// not make the wrong number come up; disabling the caret is wrong. -PL
	GR_Painter caretDisablerPainter(m_pG); // not an elegant way to disable all carets, but it works beautifully - MARCM
	
	STD_DOUBLE_BUFFERING_FOR_THIS_FUNCTION

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	bool doInsert = true;

	// Turn off list updates
	m_pDoc->disableListUpdates();
	if (!isSelectionEmpty() && !m_FrameEdit.isActive())
	{
		m_pDoc->beginUserAtomicGlob();
		PP_AttrProp AttrProp_Before;
		_deleteSelection(&AttrProp_Before);
		bool bOK = true;
		if(!isPointLegal() && bOK)
		{
//
// If we're in an illegal position move forward till we're safe.
//
			bOK = _charMotion(true,1);
		}

		if(pLR)
			AttrProp_Before.setProperty("lang", pLR->m_szLangCode);
		insertParaBreakIfNeededAtPos(getPoint());
		bResult = m_pDoc->insertSpan(getPoint(), text, count, &AttrProp_Before);
		m_pDoc->endUserAtomicGlob();
	}
	else
	{
		if(m_FrameEdit.isActive())
		{
			m_FrameEdit.setPointInside();
		}
		bool bOK = true;
		if(!isPointLegal() && bOK)
		{
//
// If we're in an illegal position move forward till we're safe.
//
			bOK = _charMotion(true,1);
		}
		PT_DocPosition posEnd = 0;
		getEditableBounds(true, posEnd);
		if(posEnd == getPoint() && !isPointLegal())
		{
			bOK = _charMotion(false,1);
		}
		if(posEnd-1 == getPoint() && !isPointLegal())
		{
			bOK = _charMotion(false,1);
		}
		if(posEnd-1 == getPoint() && m_pDoc->isEndFrameAtPos(getPoint()) && m_pDoc->isFrameAtPos(getPoint()-1))
		{
			bOK = _charMotion(false,1);
		}
		bool bOverwrite = (!m_bInsertMode && !bForce);

		if (bOverwrite)
		{
			// we need to glob when overwriting
			m_pDoc->beginUserAtomicGlob();
			cmdCharDelete(true,count);
		}
		if(text[0] == UCS_TAB && count == 1)
		{
			//
			// Were inserting a TAB. Handle special case of TAB
			// right after a list-label combo
			//
			UT_sint32 iCount=0;
			if((   ((isTabListBehindPoint(iCount) == true) && (iCount == 2)) || isTabListAheadPoint() == true)
			    && getCurrentBlock()->isFirstInList() == false)
			{
				//
				// OK now start a sublist of the same type as the
				// current list if the list type is of numbered type
				fl_BlockLayout * pBlock = getCurrentBlock();
				FL_ListType curType = pBlock->getListType();
//
// Now increase list level for bullet lists too
//
				{
					UT_uint32 curlevel = pBlock->getLevel();
					UT_uint32 currID = pBlock->getAutoNum()->getID();
					curlevel++;
					fl_AutoNum * pAuto = pBlock->getAutoNum();
					const gchar * pszAlign = pBlock->getProperty("margin-left",true);
					const gchar * pszIndent = pBlock->getProperty("text-indent",true);
					const gchar * pszFieldF = pBlock->getProperty("field-font",true);
					float fAlign = static_cast<float>(atof(pszAlign));
					float fIndent = static_cast<float>(atof(pszIndent));
//
// Convert pixels to inches.
//
					float maxWidthIN = static_cast<float>((static_cast<float>(pBlock->getFirstContainer()->getContainer()->getWidth()))/100. -0.6);
					if(fAlign + static_cast<float>(LIST_DEFAULT_INDENT) < maxWidthIN)
					{
						fAlign += static_cast<float>(LIST_DEFAULT_INDENT);
					}
					pBlock->StartList(curType,pAuto->getStartValue32(),pAuto->getDelim(),pAuto->getDecimal(),pszFieldF,fAlign,fIndent, currID,curlevel);
					doInsert = false;
				}
			}
		}
		
		if (doInsert)
		{
			if(pLR)
			{
				PP_AttrProp AP;
				AP.setProperty("lang", pLR->m_szLangCode);
				m_pDoc->insertFmtMark(PTC_AddFmt,getPoint(), &AP);
			}
			insertParaBreakIfNeededAtPos(getPoint());
			const fl_BlockLayout * pBL = getCurrentBlock();
			PP_AttrProp * pSpanAP = const_cast<PP_AttrProp *>(getAttrPropForPoint());
			bResult = m_pDoc->insertSpan(getPoint(), text, count,pSpanAP);

			if(!bResult)
			{
				const PP_AttrProp *pBlockAP = NULL;
				pBL->getAP(pBlockAP);
				bResult = m_pDoc->insertSpan(getPoint(), text, count,
											 const_cast<PP_AttrProp *>(pBlockAP));
				UT_ASSERT(bResult);
			}
		}

		if (bOverwrite)
		{
			m_pDoc->endUserAtomicGlob();
		}
	}
	if(m_FrameEdit.isActive())
	{
		m_FrameEdit.setMode(FV_FrameEdit_NOT_ACTIVE);
	}

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	if(!doInsert)
	{
	  notifyListeners(AV_CHG_ALL);
	}

	_generalUpdate();
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();

	return bResult;
}

void FV_View::_adjustDeletePosition(UT_uint32 &iDocPos, UT_uint32 &iCount)
{
	// This code deals with character clusters, such as the Thai base character + vowel + tone
	// mark combinations. Basically, if we are asked to delete the base character, we also have
	// to delete any of the other characters from its cluster. We need to find the runs that
	// contain the start and end of the requested delete segment and have them to adjust the
	// delete offsets.
	//

	//
	// Also use this code to deal with attempts to delete across hdrftr 
	// boundaries
	
	fl_BlockLayout * pBlock = NULL;
	pBlock = _findBlockAtPosition(iDocPos);

	UT_return_if_fail( pBlock );

	if(static_cast<UT_uint32>(pBlock->getLength()) <  iDocPos - pBlock->getPosition())
	{
		return;
	}
	
	fp_Run * pRun = pBlock->findRunAtOffset(iDocPos - pBlock->getPosition());
	UT_return_if_fail( pRun );

	UT_uint32 pos1 = iDocPos;
	UT_uint32 iRunOffset = pBlock->getPosition() + pRun->getBlockOffset();
	UT_uint32 iLen = UT_MIN(iCount, pRun->getLength() - (iDocPos - iRunOffset));
	bool bMoreThanOneRun = (iCount > iLen);
		
	// this call modifies both pos1, and iLen
	pRun->adjustDeletePosition(pos1, iLen);
		
	if(bMoreThanOneRun)
	{
		// the deletion spans more than a single run
		// locate the run that contains the last char to be deleted
			
		UT_uint32 iOrigEndOffset = iDocPos + iCount - 1; // doc offset of the last char to be deleted

		fl_BlockLayout * pEndBlock = _findBlockAtPosition(iOrigEndOffset);
		UT_return_if_fail( pEndBlock );
		if(static_cast<UT_uint32>(pEndBlock->getLength()) <  iOrigEndOffset - pEndBlock->getPosition())
		{
			return;
		}
		
		fp_Run * pEndRun = pEndBlock->findRunAtOffset(iOrigEndOffset - pEndBlock->getPosition());

		if(!pEndRun)
		{
			// this happens when there is no real run in the block (e.g., just a fmt mark
			// and eop
			return;
		}

		UT_uint32 iEndRunOffset = pEndBlock->getPosition() + pEndRun->getBlockOffset();

		// how much of the deleted sequence is in our run?
		iLen = iDocPos + iCount - iEndRunOffset;
		UT_ASSERT_HARMLESS( iLen <= pEndRun->getLength());
			
		pEndRun->adjustDeletePosition(iEndRunOffset, iLen);
		UT_DEBUGMSG(("iCount adjusted from %d to %d \n",iCount,iEndRunOffset + iLen - pos1));
		iCount  = iEndRunOffset + iLen - pos1;
	}
	else
	{
		// adjust count
		iCount  = iLen;
	}

	// adjust point
	iDocPos = pos1;
}

void FV_View::_updateSelectionHandles(void)
{
	if (!getVisualSelectionEnabled()){
		m_SelectionHandles.hide();
	} else if (isSelectionEmpty()) {
		m_SelectionHandles.setCursor(getInsPoint());
	} else {
		m_SelectionHandles.setSelection(getSelectionLeftAnchor(),
										getSelectionRightAnchor());
	}
}
