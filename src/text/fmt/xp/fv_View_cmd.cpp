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
#include "ut_Language.h"

#include "xav_View.h"
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
#include "fp_TableContainer.h"
#include "fl_FootnoteLayout.h"
#include "fl_ContainerLayout.h"
#include "fl_TOCLayout.h"
#include "pp_Revision.h"

#include "ap_Dialog_SplitCells.h"

#include "fv_View.h"

#if 1
// todo: work around to remove the INPUTWORDLEN restriction for pspell
#include "ispell_def.h"
#endif

// NB -- irrespective of this size, the piecetable will store
// at max BOOKMARK_NAME_LIMIT of chars as defined in pf_Frag_Bookmark.h
#define BOOKMARK_NAME_SIZE 30
#define CHECK_WINDOW_SIZE if(getWindowHeight() < 20) return;

/****************************************************************/

void FV_View::cmdUnselectSelection(void)
{
	_clearSelection();
}


/*!
  Move point a number of character positions
  \param bForward True if moving forward
  \param count Number of char positions to move

  \note Cursor movement while there's a selection has the effect of
		clearing the selection. And only that. See bug 993.
*/
void FV_View::cmdCharMotion(bool bForward, UT_uint32 count)
{
	if (!isSelectionEmpty())
	{
		_moveToSelectionEnd(bForward);
		_fixInsertionPointCoords();
		_ensureInsertionPointOnScreen();
		notifyListeners(AV_CHG_MOTION);
		return;
	}

	PT_DocPosition iPoint = getPoint();
	if (!_charMotion(bForward, count))
	{
		if(bForward)
		{
//
// Reached end of document.
//
			UT_DEBUGMSG(("SEVIOR: Reached end of document \n"));
			m_bPointEOL = true;
		}
		else
		{
			_setPoint(iPoint);
		}
	}
	else
	{
		PT_DocPosition iPoint1 = getPoint();
		if ( iPoint1 == iPoint )
		{
			if(!_charMotion(bForward, count))
			{
				_setPoint(iPoint);
				_fixInsertionPointCoords();
				_ensureInsertionPointOnScreen();
				notifyListeners(AV_CHG_MOTION);
				return;
			}
		}
	}
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	notifyListeners(AV_CHG_MOTION);
}


/*!
 * Split the merged cells located at the current point in the way specified
 * by iSplitType
  */
bool FV_View::cmdSplitCells(AP_CellSplitType iSplitType)
{
	PL_StruxDocHandle cellSDH,tableSDH,curSDH,endTableSDH;
	PL_StruxDocHandle prevCellSDH1,prevCellSDH2;
	PT_DocPosition posTable,posCell,posFirstInsert,posEndTable;
	posFirstInsert = 0;
	UT_sint32 iLeft,iRight,iTop,iBot;
	UT_sint32 jLeft,jRight,jTop,jBot;
	PT_DocPosition posCol = getPoint();
	if(!isInTable(posCol))
	{
		return false;
	}
	getCellParams(posCol, &iLeft, &iRight,&iTop,&iBot);
	UT_String sCellProps;
	getCellFormat(posCol,sCellProps);
//
// Find the Row and column of the cell at the current point. The strategy
// will be insert a new cell with the same (row/col depending on the split)
// and to adjust the rest of the table past this point.
//
	UT_sint32 rowSpan = iBot - iTop;
	UT_sint32 colSpan = iRight - iLeft;
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posCol,PTX_SectionCell,&cellSDH);
	bRes = m_pDoc->getStruxOfTypeFromPosition(posCol,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);

	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;
	posCell = m_pDoc->getStruxPosition(cellSDH);
	endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
	posEndTable = m_pDoc->getStruxPosition(endTableSDH);
//
// Got all we need, now set things up to do the insert nicely
//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	m_pDoc->setDontImmediatelyLayout(true);
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with a bogus line-type property. We'll restore it later.
//
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	const char * szListTag = NULL;
	UT_String sListTag;
	UT_sint32 iListTag;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szListTag);
	if(szListTag == NULL || *szListTag == '\0')
	{
		iListTag = 0;
	}
	else
	{
		iListTag = atoi(szListTag);
		iListTag -= 1;
	}
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
	UT_sint32 splitLeft,splitRight,splitTop,splitBot;
	UT_sint32 newLeft,newRight,newTop,newBot;	
	UT_sint32 numRows = 0;
	UT_sint32 numCols = 0;
	bool bDoSplitSolidHori = false;
	bool bDoSplitSolidVert = false;
//
// OK now insert the cell and do the update
//
	m_pDoc-> getRowsColsFromTableSDH(tableSDH, &numRows, &numCols);

	if(iSplitType <= hori_right)
	{
//
// This is similar to "insert column"
//
		if(iSplitType ==  hori_left)
		{
			splitLeft = iLeft;
			splitRight = iLeft+1;
		}
		else if(iSplitType ==  hori_mid)
		{
			splitLeft = iLeft;
			if(colSpan == 1)
			{
				bDoSplitSolidHori = true;
				splitRight = iLeft+1;
			}
			else
			{
				splitRight = iLeft + colSpan/2;
			}
		}
		else if(iSplitType ==  hori_right)
		{
			splitLeft = iLeft;
			splitRight = iRight -1;
		}
		splitTop = iTop;
		splitBot = iBot;
		newTop = iTop;
		newBot = iBot;
		if(!bDoSplitSolidHori)
		{
			newLeft = splitRight;
			newRight = iRight;
		}
		else
		{
			newLeft = splitRight;
			newRight = newLeft+1;
		}

	}
	else
	{
		if(iSplitType ==  vert_above)
		{
			newTop = iTop;
			newBot = iTop +1;
		}
		else if(iSplitType ==  vert_mid)
		{
			newTop = iTop;
			if(rowSpan == 1)
			{
				bDoSplitSolidVert = true;
				newBot = newTop+1;
			}
			else
			{
				newBot = iTop + rowSpan/2;
			}
		}
		else if(iSplitType ==  vert_below)
		{
			newTop = iTop;
			newBot = iBot -1;
		}
//
// we need to get the location of where to place this cell.
//
		splitLeft = iLeft;
		splitRight = iRight;
		newLeft = iLeft;
		newRight = iRight;
		if(!bDoSplitSolidVert)
		{
			splitTop = newBot;
			splitBot = iBot;
		}
		else
		{
			newTop = iTop;
			newBot = newTop+1;
			splitTop = newBot;
			splitBot = splitTop+1;
		}
//
// OK now we have to find the place to insert this. It should be the cell
// immediately after (splitTop,splitLeft)
//
//(except if it's a splitSolidVert in which case it should be placed as the
// next next on the row in the first cell before (splitleft,splittop)
//
		if(bDoSplitSolidVert)
		{
//
// OK start with cellSDH and scan until we either reach the end of the table
// or we find a cell past where splitcell should be. Then we insert the cell
// at where cell just after splitcell is.
//
			bool bStop = false;
			curSDH = cellSDH;
			while(!bStop)
			{
				posCell = m_pDoc->getStruxPosition(curSDH)+1;
				bRes = getCellParams(posCell,&jLeft,&jRight,&jTop,&jBot);
				UT_ASSERT(bRes);
				if(jTop >= splitTop)
				{
//
// Found it!
//
					bStop = true;
					posCell = m_pDoc->getStruxPosition(curSDH);
					break;
				}
				bRes = m_pDoc->getNextStruxOfType(curSDH,PTX_SectionCell,&curSDH);
				if(!bRes)
				{
					bStop = true;
					posCell = m_pDoc->getStruxPosition(endTableSDH);
					break;
				}
				posCell = m_pDoc->getStruxPosition(curSDH);
				if(posCell > posEndTable)
				{
					bStop = true;
					posCell = m_pDoc->getStruxPosition(endTableSDH);
					break;
				}
			}
		}
		else
		{
			jTop = splitTop;
			jBot = jTop+1;
			jLeft = splitRight;
			if(jLeft >= numCols)
			{
				jLeft = 0;
				jTop += 1;
				jBot +=1;
			}	
			jRight = jLeft+1;
			if(jTop >= numRows)
			{
//
// Place right before endTable Strux
//
				if(endTableSDH == NULL)
				{
					//
					// Disaster! the table structure in the piecetable is screwed.
					// we're totally stuffed now.
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					return false;
				}
				posCell = m_pDoc->getStruxPosition(endTableSDH);
			}
			else
			{
//
// now we have to loop until we find a cell precisely at jLeft,jTop
//
				bool bFound = false;
				while(!bFound)
				{
					curSDH = m_pDoc-> getCellSDHFromRowCol(tableSDH,jTop,jLeft);
					if(curSDH == NULL)
					{
						endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
						if(endTableSDH == NULL)
						{
							//
							// Disaster! the table structure in the piecetable is screwed.
							// we're totally stuffed now.
							UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
							return false;
						}
						posCell = m_pDoc->getStruxPosition(endTableSDH);
						bFound = true;
					}
					UT_sint32 kLeft,kRight,kTop,kBot;
					PT_DocPosition posTmp = m_pDoc->getStruxPosition(curSDH)+1;
					getCellParams(posTmp,&kLeft,&kRight,&kTop,&kBot);
					if((kLeft == jLeft) && (kTop == jTop))
					{
						bFound = true;
						posCell = m_pDoc->getStruxPosition(curSDH);
					}
					else
					{
						jLeft++;
						jRight++;
						if(jLeft >= numCols)
						{
							jLeft = 0;
							jTop++;
							jRight =1;
							jBot++;
						}
						if(jTop >= numRows)
						{
							endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
							if(endTableSDH == NULL)
							{
								//
								// Disaster! the table structure in the piecetable is screwed.
								// we're totally stuffed now.
								UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
								return false;
							}
							posCell = m_pDoc->getStruxPosition(endTableSDH);
							bFound = true;
						}
					}
				}
			}
		}
	}
//
// OK build the table properties
//
	UT_String sRowTop = "top-attach";
	UT_String sRowBot = "bot-attach";
	UT_String sColLeft = "left-attach";
	UT_String sColRight = "right-attach";
	UT_String sTop,sBot,sLeft,sRight;
	UT_String_sprintf(sTop,"%d",splitTop);
	UT_String_sprintf(sBot,"%d",splitBot);
	UT_String_sprintf(sLeft,"%d",splitLeft);
	UT_String_sprintf(sRight,"%d",splitRight);
	UT_String_setProperty(sCellProps,sRowTop,sTop);
	UT_String_setProperty(sCellProps,sRowBot,sBot);
	UT_String_setProperty(sCellProps,sColLeft,sLeft);
	UT_String_setProperty(sCellProps,sColRight,sRight);
	UT_DEBUGMSG(("Cells props for new cell:\n  %s \n",sCellProps.c_str()));
//
// Insert the cell
//
	const XML_Char * atts[4] = {"props",NULL,NULL,NULL};
	atts[1] = sCellProps.c_str();
	bRes = m_pDoc->insertStrux(posCell,PTX_SectionCell,atts,NULL);
	bRes = m_pDoc->insertStrux(posCell+1,PTX_Block);
	posFirstInsert = posCell + 2;
//
// Save the cell SDH for later..
//
	m_pDoc->getStruxOfTypeFromPosition(posCell+1,PTX_SectionCell,&prevCellSDH1);	
	
	bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);

// Changes the props of the new cell
	UT_String_sprintf(sTop,"%d",newTop);
	UT_String_sprintf(sBot,"%d",newBot);
	UT_String_sprintf(sLeft,"%d",newLeft);
	UT_String_sprintf(sRight,"%d",newRight);
	UT_String_setProperty(sCellProps,sRowTop,sTop);
	UT_String_setProperty(sCellProps,sRowBot,sBot);
	UT_String_setProperty(sCellProps,sColLeft,sLeft);
	UT_String_setProperty(sCellProps,sColRight,sRight);
	posCell = m_pDoc->getStruxPosition(cellSDH)+1;
	UT_DEBUGMSG(("New Cells props for old cell:\n  %s \n",sCellProps.c_str()));
	atts[1] = sCellProps.c_str();
	bool bres = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell,posCell,atts,NULL,PTX_SectionCell);
	m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&prevCellSDH2);		if(bDoSplitSolidHori)
	{
//
// OK now we have to adjust all the cells with left or right >= splitleft
// If left of a given cell is < splitLeft it is not adjusted but the right
// of the cell if it's > splitleft is incremented. In this way our new cell
// span two columns
//
		UT_sint32 myleft,myright,mytop,mybot;
//
// start at the first cell and scan through the table adjusting each cell.
//
		bres = m_pDoc->getStruxOfTypeFromPosition(posTable+1,PTX_SectionCell,&cellSDH);
		UT_ASSERT(bres);
		bool bStop= false;
		while(!bStop)
		{
			posCell = m_pDoc->getStruxPosition(cellSDH)+1;
			bres = getCellParams(posCell,&myleft,&myright,&mytop,&mybot);
			UT_ASSERT(bres);
			bool bChange =false;
			if((cellSDH == prevCellSDH1) || (cellSDH == prevCellSDH2))
			{
//
// Igore me!
//
				bChange = false;
			}
			else 
			{
				if(myright> splitLeft)
				{
					myright++;
					bChange = true;
				}
				if(myleft>splitLeft)
				{
					myleft++;
					bChange = true;
				}
				if(bChange)
				{
// Changes the props of the cell
					UT_String_sprintf(sTop,"%d",mytop);
					UT_String_sprintf(sBot,"%d",mybot);
					UT_String_sprintf(sLeft,"%d",myleft);
					UT_String_sprintf(sRight,"%d",myright);
					UT_String_setProperty(sCellProps,sRowTop,sTop);
					UT_String_setProperty(sCellProps,sRowBot,sBot);
					UT_String_setProperty(sCellProps,sColLeft,sLeft);
					UT_String_setProperty(sCellProps,sColRight,sRight);
					atts[1] = sCellProps.c_str();
					m_pDoc->changeStruxFmt(PTC_AddFmt,posCell,posCell,atts,NULL,PTX_SectionCell);
				}
			}
			bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
			if(!bRes)
			{
				bStop = true;
				break;
			}
			posCell = m_pDoc->getStruxPosition(cellSDH);
			if(posCell > posEndTable)
			{
				bStop = true;
				break;
			}
		}
	}
	if(bDoSplitSolidVert)
	{
//
// OK now we have to adjust all the cells with top or bot >= newTop
// If top of a given cell is < newTop it is not adjusted but the bot
// of the cell if it's > newTop is incremented. In this way our new cell
// spans two rows
//
		UT_sint32 myleft,myright,mytop,mybot;
//
// start at the first cell and scan through the table adjusting each cell.
//
		m_pDoc->getStruxOfTypeFromPosition(posTable+1,PTX_SectionCell,&cellSDH);
		bool bStop= false;
		while(!bStop)
		{
			posCell = m_pDoc->getStruxPosition(cellSDH) +1;
			getCellParams(posCell,&myleft,&myright,&mytop,&mybot);
			bool bChange =false;
			if((cellSDH == prevCellSDH1) || (cellSDH == prevCellSDH2))
			{
//
// Igore me!
//
				bChange = false;
			}
			else 
			{
				if(mytop>newTop)
				{
					mytop++;
					bChange = true;
				}
				if(mybot>newTop)
				{
					mybot++;
					bChange = true;
				}
				if(bChange)
				{
// Changes the props of the cell
					UT_String_sprintf(sTop,"%d",mytop);
					UT_String_sprintf(sBot,"%d",mybot);
					UT_String_sprintf(sLeft,"%d",myleft);
					UT_String_sprintf(sRight,"%d",myright);
					UT_String_setProperty(sCellProps,sRowTop,sTop);
					UT_String_setProperty(sCellProps,sRowBot,sBot);
					UT_String_setProperty(sCellProps,sColLeft,sLeft);
					UT_String_setProperty(sCellProps,sColRight,sRight);
					atts[1] = sCellProps.c_str();
					m_pDoc->changeStruxFmt(PTC_AddFmt,posCell,posCell,atts,NULL,PTX_SectionCell);
				}
			}
			bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
			if(!bRes)
			{
				bStop = true;
				break;
			}
			posCell = m_pDoc->getStruxPosition(cellSDH);
			if(posCell > posEndTable)
			{
				bStop = true;
				break;
			}
		}
	}

//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with the restored line-type property it has before.
//
	iListTag += 1;
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
// Put the insertion point in a legal position
//
	setPoint(posFirstInsert);
	notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return true;
}

void  FV_View::cmdSelectTOC(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 xClick=0,yClick = 0;
	fp_Page* pPage = _getPageForXY(x,y,xClick,yClick);
	fl_TOCLayout * pTOCL = pPage->getLastMappedTOC();
	m_Selection.setTOCSelected(pTOCL);
	PT_DocPosition pos = pTOCL-> getPosition();
	m_iInsPoint = pos+1;
	if(m_pG->getCaret())
	{
		m_pG->getCaret()->disable();
	}
	m_countDisable++;
}

/*!
 * Select the column of the table  identified by the document position 
 * posOfColumn
 */
bool FV_View::cmdSelectColumn(PT_DocPosition posOfColumn)
{
	PL_StruxDocHandle cellSDH,tableSDH;
	PT_DocPosition posTable,posCell;
	UT_sint32 iLeft,iRight,iTop,iBot;
	UT_sint32 Left,Right,Top,Bot;
	bool bEOL = false; // added this stop compiler warning. Tomas
	if(!isInTable(posOfColumn))
	{
		return false;
	}
	getCellParams(posOfColumn, &iLeft, &iRight,&iTop,&iBot);
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posOfColumn,PTX_SectionCell,&cellSDH);
	bRes = m_pDoc->getStruxOfTypeFromPosition(posOfColumn,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);

	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;
	posCell = m_pDoc->getStruxPosition(cellSDH);

//
// Now find the number of rows and columns inthis table. 
//
	UT_sint32 numRows = 0;
	UT_sint32 numCols = 0;
	m_pDoc->getRowsColsFromTableSDH(tableSDH, &numRows, &numCols);
//
// Ok set the selection type to that of a column
//
	m_Selection.setMode(FV_SelectionMode_TableColumn);

	fl_BlockLayout * pBlock = NULL;
	fp_Run * pRun = NULL;
	UT_sint32 xCaret, yCaret;
	UT_uint32 heightCaret;
	UT_sint32 xCaret2, yCaret2;
	bool bDirection;
	_findPositionCoords(posOfColumn, bEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);
	UT_return_val_if_fail(pBlock,false);
	fl_ContainerLayout * pCL = pBlock->myContainingLayout();
	UT_return_val_if_fail(pCL,false);
	pCL = pCL->myContainingLayout();
	UT_return_val_if_fail(pCL,false);
	UT_return_val_if_fail((pCL->getContainerType() == FL_CONTAINER_TABLE),false);
	fl_TableLayout * pTL = static_cast<fl_TableLayout *>(pCL);
	m_Selection.setTableLayout(pTL);
//
// Now loop through the column and collect all the cells.
//
	UT_sint32 j = 0;
	UT_sint32 jPrev = -1;
	for(j=0; j<numRows; j++)
	{
		PT_DocPosition posWork = findCellPosAt(posTable,j,iLeft) +1;
		getCellParams(posWork,&Left,&Right,&Top,&Bot);
		UT_DEBUGMSG(("Adding cell at left %d right %d top %d bot %d posWork %d \n",Left,Right,Top,Bot,posWork));
		if(Top == jPrev)
		{
			continue;
		}
		_findPositionCoords(posWork+1, bEOL, xCaret, yCaret, xCaret2, yCaret2, heightCaret, bDirection, &pBlock, &pRun);
		UT_return_val_if_fail(pBlock,false);
		UT_DEBUGMSG(("Block pos = %d \n",pBlock->getPosition(false)));
		fl_ContainerLayout * pCL = pBlock->myContainingLayout();
		UT_return_val_if_fail((pCL->getContainerType() == FL_CONTAINER_CELL),false);
		fl_CellLayout * pCell = static_cast<fl_CellLayout *>(pCL);
		m_Selection.addCellToSelection(pCell);
		jPrev = j;
	}
	PD_DocumentRange * pRange = getNthSelection(getNumSelections()-1);
	_setPoint(pRange->m_pos2);
	_drawSelection();
	notifyListeners(AV_CHG_MOTION);
	return true;
}


/*!
 * Merge the cells located at posSource with posDestination by copying the data from 
 * source to destination. Then deleting source and expanding destination into it's location
 * in the table.
 */
bool FV_View::cmdMergeCells(PT_DocPosition posSource, PT_DocPosition posDestination)
{
	UT_sint32 sLeft,sRight,sTop,sBot;
	UT_sint32 dLeft,dRight,dTop,dBot;
	UT_sint32 Left,Right,Top,Bot; // need these for working variables.
	getCellParams(posSource,&sLeft,&sRight,&sTop,&sBot);
	getCellParams(posDestination,&dLeft,&dRight,&dTop,&dBot);

	PT_DocPosition posTable,posWork;
	PL_StruxDocHandle tableSDH;
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posSource,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);
	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;

//
// Now find the number of rows and columns inthis table. 
//
	UT_sint32 numRows = 0;
	UT_sint32 numCols = 0;
	m_pDoc->getRowsColsFromTableSDH(tableSDH, &numRows, &numCols);
	bool bChanged = false;
	UT_sint32 iLineType = 0;

//
// Got all we need, now set things up to do the merge nicely
//
//
// OK that's done, now do the merge
//
// Have to worry about merging cell spanning multiple rows and columns. We do this
// by matching the  the widths and heights of the source and destination cells.	
//
	if((sLeft == dLeft) && (sTop == dTop))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	if(sLeft == dLeft)
	{
//
// Merge vertically
//
//  might have  d |  |  |  |
//                ----------
//             s  |        |
// To solve this merge all top cells horizontally then merge this combined set vertically
//
// First check that top row ends at bottom row boundary. We reject attempts to merge cells
// like this
//            d  |  |  |    |
//               ------------
//            s  |        |
		if(sRight >= dRight)
		{
			if(sRight < numCols -1)
			{
				posWork = findCellPosAt(posTable,dTop,sRight) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
				if(Left != sRight)
				{
					// 
					// Right column of src cell doesn't match the row above it. Bail out.
					//
					// fixme: Put in a dialog to explain this to the user
					//
					return false;
				}
			}
			//
			// OK now merge all the cells in the destination cell together.
			//
			Left = dRight;
			while(Left < sRight)
			{
				posWork = findCellPosAt(posTable,dTop,Left) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
//
// Do the merge without all the undo/update trimings
// append onto destination
				if(!bChanged)
				{
					iLineType = _changeCellParams(posTable, tableSDH);
				}
				bChanged = true;
				_MergeCells(posDestination,posWork,false);
				Left = Right;
			}
//
// Now merge the merged destination into the source
//
			if(!bChanged)
			{
				iLineType = _changeCellParams(posTable, tableSDH);
			}
			bChanged = true;
			posSource = findCellPosAt(posTable,sTop,sLeft) +1;
			posDestination = findCellPosAt(posTable,dTop,dLeft) +1;
			_MergeCells(posDestination,posSource,true);
		}
		else
		{
//
// Here we have this scenario:
//
//             d |          |
//               ------------
//            s  |  |  |    |
//
// ie destination is narrower than the source
//
//			check that the source column lines up with the destination column.
//
			if(dRight < numCols -1)
			{
				posWork = findCellPosAt(posTable,sTop,dRight) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
				if(Left != dRight)
				{
					// 
					// Right column of src cell doesn't match the row above it. Bail out.
					// fixme: Put in a dialog to explain this to the user
					//
					return false;
				}
			}
			//
			// OK now merge all the cells in the src cell together.
			//
			Left = sRight;
			while(Left < dRight)
			{
				posWork = findCellPosAt(posTable,sTop,Left) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
//
// Do the merge without all the undo/update trimings
//
				if(!bChanged)
				{
					iLineType = _changeCellParams(posTable, tableSDH);
				}
				bChanged = true;
				_MergeCells(posSource,posWork,false);
				Left = Right;
			}
//
// Now merge the merged destination into the source
//
			if(!bChanged)
			{
				iLineType = _changeCellParams(posTable, tableSDH);
			}
			bChanged = true;
			posSource = findCellPosAt(posTable,sTop,sLeft) +1;
			posDestination = findCellPosAt(posTable,dTop,dLeft) +1;
			_MergeCells(posDestination,posSource,true);
		}
	}
	else if(sTop == dTop)
	{
//
// Merge horizontally
//
//  might have  rows spanning several columns
//      d    s
//     ---------
//     |   |   |
//     |   -----
//     |   |   |
//     |   -----
//     |   |   |
//     ---------
// To solve this merge all cells vertically then merge this combined set horizontally
//
// First check that left column ends at right column boundary. 
// We reject attempts to merge cells that don't have this condition.
// ie this:
//      d    s
//     ---------
//     |   |   |
//     |   -----
//     |   |   |
//     |   -----
//     |   |   |
//     |   -----
//     |---|   |
//     |   -----
//
		if(dBot >= sBot)
		{
			if(dBot < numRows -1)
			{
				posWork = findCellPosAt(posTable,dBot,sLeft) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
				if(Top != dBot)
				{
					// 
					// Bot col of src cell doesn't match the column before. Bail out.
					//
					// fixme: Put in a dialog to explain this to the user
					//
					return false;
				}
			}
			//
			// OK now merge all the cells right of the src cell together.
			//
			Bot	 = sBot;
			Top = sBot;
			while(Top < dBot)
			{
				posWork = findCellPosAt(posTable,Top,sLeft) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
//
// Do the merge without all the undo/update trimings
//
				if(!bChanged)
				{
					iLineType = _changeCellParams(posTable, tableSDH);
				}
				bChanged = true;
				_MergeCells(posSource,posWork,false);
				UT_ASSERT(Bot > Top);
				if(Bot <= Top)
				{
					break;
				}
				Top = Bot;
			}
//
// Now merge the merged destination into the source
//
			if(!bChanged)
			{
				iLineType = _changeCellParams(posTable, tableSDH);
			}
			bChanged = true;
			posSource = findCellPosAt(posTable,sTop,sLeft) +1;
			posDestination = findCellPosAt(posTable,dTop,dLeft) +1;
			_MergeCells(posDestination,posSource,true);
		}
		else
		{
//
// Here we have this scenario:
//
//
//  might have  rows spanning several columns
//      d    s
//     ---------
//     |   |   |
//     -----   |
//     |   |   |
//     -----   |
//     |   |   |
//     ---------
// To solve this merge all cells vertically then merge this combined set horizontally
//
// First check that left column ends at right column boundary. 
// We reject attempts to merge cells that don't have this condition.
// ie this:
//      d    s
//     ---------
//     |   |   |
//     ----|   |
//     |   |   |
//     ----|   |
//     |   |   |
//     |   |   |
//     |---|   |
//     |   -----
//

			if(sBot < numRows -1)
			{
				posWork = findCellPosAt(posTable,sBot,dLeft) +1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
				if(Top != sBot)
				{
					// 
					// Right column of src cell doesn't match the row above it. Bail out.
					//
					// fixme: Put in a dialog to explain this to the user
					//
					return false;
				}
			}
			//
			// OK now merge all the cells above the src cell together.
			//
			Top = dBot;
			while(Top < sBot)
			{
				posWork = findCellPosAt(posTable,Top,dLeft) + 1;
				getCellParams(posWork,&Left,&Right,&Top,&Bot);
//
// Do the merge without all the undo/update trimings
//
				if(!bChanged)
				{
					iLineType = _changeCellParams(posTable, tableSDH);
				}
				bChanged = true;
				_MergeCells(posDestination,posWork,false);
				Top = Bot;
			}
//
// Now merge the source into the merged destination
//
			if(!bChanged)
			{
				iLineType = _changeCellParams(posTable, tableSDH);
			}
			bChanged = true;
			posSource = findCellPosAt(posTable,sTop,sLeft) +1;
			posDestination = findCellPosAt(posTable,dTop,dLeft) +1;
			_MergeCells(posDestination,posSource,true);
		}
	}
	else
	{
//
// Neight left or top align of the cells to be merged.
// bali out

		return false;
	}
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with the restored line-type property it has before.
//
	iLineType += 1;
	_restoreCellParams(posTable,iLineType);
	posDestination = findCellPosAt(posTable,dTop,dLeft) +2;
	setPoint(posDestination);
//	_charMotion(true,1);
	notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return true;
}

/*!
 * Make a table columns autosizing by removing all the column properties.
 */
bool FV_View::cmdAutoSizeCols(void)
{
//
// Got all we need, now set things up to do the delete nicely
//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "table-column-props";
	pszTable[1] = "1";
	m_pDoc->changeStruxFmt(PTC_RemoveFmt,getPoint(),getPoint(),NULL,pszTable,PTX_SectionTable);
	pszTable[0] = "table-column-leftpos";
	m_pDoc->changeStruxFmt(PTC_RemoveFmt,getPoint(),getPoint(),NULL,pszTable,PTX_SectionTable);
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return true;
}


/*!
 * Make a table Rows autosizing by removing all the row properties.
 */
bool FV_View::cmdAutoSizeRows(void)
{
//
// Got all we need, now set things up to do the delete nicely
//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "table-row-heights";
	pszTable[1] = "1";
	pszTable[2] = NULL;
	m_pDoc->changeStruxFmt(PTC_RemoveFmt,getPoint(),getPoint(),NULL,pszTable,PTX_SectionTable);
	pszTable[0] = "table-column-leftpos";
	m_pDoc->changeStruxFmt(PTC_RemoveFmt,getPoint(),getPoint(),NULL,pszTable,PTX_SectionTable);
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return true;
}


/*!
 * Insert a column containing the position posCol, insert the column before the
 * current column.
 */
bool FV_View::cmdInsertCol(PT_DocPosition posCol, bool bBefore)
{
	PL_StruxDocHandle cellSDH,tableSDH,endTableSDH,endCellSDH,prevCellSDH;
	PT_DocPosition posTable,posCell,posEndCell,posPrevCell,posFirstInsert;
	UT_sint32 numColsForInsertion = getNumColumnsInSelection();
	if(numColsForInsertion == 0)
	{
		return false;
	}
	posFirstInsert = 0;
	UT_sint32 iLeft,iRight,iTop,iBot;
	getCellParams(posCol, &iLeft, &iRight,&iTop,&iBot);
	UT_sint32 iColInsertAt =0;

	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posCol,PTX_SectionCell,&cellSDH);
	bRes = m_pDoc->getStruxOfTypeFromPosition(posCol,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);

	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;
//
// Now find the number of rows and columns in this table. 
//
	UT_sint32 numRows = 0;
	UT_sint32 numCols = 0;
	UT_sint32 i = 0;
	m_pDoc-> getRowsColsFromTableSDH(tableSDH, &numRows, &numCols);
	if(!bBefore)
	{
		for(i=0;i<numRows;i++)
		{
			posCell = findCellPosAt(posTable,i,iLeft+ numColsForInsertion-1);
			UT_sint32 jLeft,jRight,jTop,jBot;
			getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
			if((jRight -1) > iColInsertAt)
			{
				iColInsertAt = jRight -1;
			}
		}
	}
	else
	{
		iColInsertAt = 99999999;
		for(i=0;i<numRows;i++)
		{
			posCell = findCellPosAt(posTable,i,iLeft);
			UT_sint32 jLeft,jRight,jTop,jBot;
			getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
			if(jLeft < iColInsertAt)
			{
				iColInsertAt = jLeft;
			}
		}
	}

//
// Got all we need, now set things up to do the insert nicely
//
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// Turn off list updates

	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	if (!isSelectionEmpty())
	{
		_clearSelection();
	}
	m_pDoc->setDontImmediatelyLayout(true);
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with a bogus line-type property. We'll restore it later.
//
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	const char * szListTag = NULL;
	UT_String sListTag;
	UT_sint32 iListTag;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szListTag);
	if(szListTag == NULL || *szListTag == '\0')
	{
		iListTag = 0;
	}
	else
	{
		iListTag = atoi(szListTag);
		iListTag -= 1;
	}
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG((" Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK loop through all the rows in this column and insert the entries in the specified
// column if the cell spans just the width of the column..
//
	UT_sint32 oldTop = -1;
	UT_sint32 ii = 0;
	for(ii=0; ii<numColsForInsertion;ii++)
	{
		oldTop = -1;
		for(i=0; i <numRows; i++)
		{
			if(iColInsertAt >= numCols)
			{
				posCell = findCellPosAt(posTable,i,numCols-1);
			}
			else
			{
				posCell = findCellPosAt(posTable,i,iColInsertAt);
			}
			UT_ASSERT(posCell > 0);
			bRes = m_pDoc->getStruxOfTypeFromPosition(posCell+1,PTX_SectionCell,&cellSDH);
			UT_sint32 Left,Right,Top,Bot;
			getCellParams(posCell+1,&Left,&Right,&Top,&Bot);
			UT_DEBUGMSG(("SEVIOR: Before Insert column left %d right %d top %d bot %d \n",Left,Right,Top,Bot));
			const XML_Char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
			UT_String sRowTop = "top-attach";
			UT_String sRowBot = "bot-attach";
			UT_String sColLeft = "left-attach";
			UT_String sColRight = "right-attach";
			UT_String sTop,sBot,sLeft,sRight;
			UT_String_sprintf(sTop,"%d",i);
			UT_String_sprintf(sBot,"%d",i+1);
			props[0] = sRowTop.c_str();
			props[1] = sTop.c_str();
			props[2] = sRowBot.c_str();
			props[3] = sBot.c_str();
			bool bBBefore = false;
			bool bAfter = false;
			if(((Bot - Top) > 1) && (oldTop != Top))
			{
				oldTop = Top;
			}
			if((Bot - Top) == 1)
			{
				if(bBefore)
				{
					if(iColInsertAt > 0)
					{
						PT_DocPosition posCellLeft = findCellPosAt(posTable,i,iColInsertAt-1);
						UT_sint32 jLeft,jRight,jTop,jBot;
						getCellParams(posCellLeft+1,&jLeft,&jRight,&jTop,&jBot);
						UT_String_sprintf(sLeft,"%d",jRight);
						UT_String_sprintf(sRight,"%d",jRight+1);
					}
					else
					{
						UT_String_sprintf(sLeft,"%d",0);
						UT_String_sprintf(sRight,"%d",1);
					}
					props[4] = sColLeft.c_str();
					props[5] = sLeft.c_str();
					props[6] = sColRight.c_str();
					props[7] = sRight.c_str();
					bRes = m_pDoc->insertStrux(posCell,PTX_SectionCell,NULL,props);
					bRes = m_pDoc->insertStrux(posCell+1,PTX_Block);
					if(i == 0)
					{
						posFirstInsert = posCell + 2;
					}
					bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
				}
				else
				{
//
// iColInsertAt gives the cell just to the right of the requested cell.
// We need to insert just before this point - unless this is the last column
// on the row in which case we need to insert after the endCell strux.
//
					PT_DocPosition posCellLeft = 0;
					posCellLeft = findCellPosAt(posTable,i,iColInsertAt);
					bRes = m_pDoc->getStruxOfTypeFromPosition(posCellLeft+1,PTX_SectionCell,&cellSDH);
					endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
					posEndCell = m_pDoc->getStruxPosition(endCellSDH);
					posCellLeft = posEndCell+1;
					UT_sint32 jLeft,jRight,jTop,jBot;
					getCellParams(posCellLeft+1,&jLeft,&jRight,&jTop,&jBot);
					UT_String_sprintf(sLeft,"%d",iColInsertAt+1);
					UT_String_sprintf(sRight,"%d",iColInsertAt+2);
					props[4] = sColLeft.c_str();
					props[5] = sLeft.c_str();
					props[6] = sColRight.c_str();
					props[7] = sRight.c_str();
					UT_DEBUGMSG(("SEVIOR: Inserting cell strux %s %s %s %s %s %s %s %s \n",props[0],props[1],props[2],props[3],props[4],props[5],props[6],props[7]));
					bRes = m_pDoc->insertStrux(posCellLeft,PTX_SectionCell,NULL,props);
					bRes = m_pDoc->insertStrux(posCellLeft+1,PTX_Block);
					if(i == 0)
					{
						posFirstInsert = posCellLeft + 2;
					}
					
					bRes = m_pDoc->insertStrux(posCellLeft+2,PTX_EndCell);
					
				}
			}
			else
			{
//
// OK we have to work to find the right place to insert the cell.
//
				UT_sint32 jLeft,jRight,jTop,jBot;
				if(bBefore)
				{
//
// find first cell on this row with left coordinate == or greater than 
// iInsertColAt.
//
					UT_sint32 j =0;
					UT_sint32 k = i;
					for(k=i; k < numRows && !bBBefore; k++)
					{
						if( k == i)
						{
							j = iColInsertAt;
						}
						else
						{
							j = 0;
						}
						while(j < numCols && !bBBefore)
						{
							posCell = findCellPosAt(posCol,k,j);
							getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
							if(jTop == i)
							{ 
								bBBefore = true;
								UT_String_sprintf(sLeft,"%d",iColInsertAt);
								UT_String_sprintf(sRight,"%d",iColInsertAt+1);
								props[4] = sColLeft.c_str();
								props[5] = sLeft.c_str();
								props[6] = sColRight.c_str();
								props[7] = sRight.c_str();
//
// Insert at Previous cell it will push that cell to one position later.
//
								bRes = m_pDoc->insertStrux(posCell,PTX_SectionCell,NULL,props);
								bRes = m_pDoc->insertStrux(posCell+1,PTX_Block);
								if(i == 0)
								{
									posFirstInsert = posCell + 2;
								}
								bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
							}
							j++;
						}
					}
					UT_ASSERT(bBBefore);
				}
//
// Insert column after this column
//
				else
				{
//
// OK we have to work to find the right place to insert the cell.
//
					UT_sint32 jLeft,jRight,jTop,jBot;
//
// Now look after this cell for a column to place this.
//
					bAfter = false;
					bBBefore = false;
					UT_sint32 j=0;
					UT_sint32 k =i;
					for(k=i; k < numRows && !bAfter; k++)
					{
						if(k == i)
						{
							j = iColInsertAt;
						}
						else
						{
							j = 0;
						}
						while((j< numCols) && !bAfter)
						{
							posCell = findCellPosAt(posCol,k,j);
							getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
							if(jTop == i)
							{
								bAfter = true;
//
// iColInsertAt gives the cell just to the right of the requested cell.
// We need to insert just before this point - unless this is the last column
// on the row in which case we need to insert after the endCell strux.
//
								PT_DocPosition posCellLeft = 0;
								posCellLeft = findCellPosAt(posTable,k,j);
								bRes = m_pDoc->getStruxOfTypeFromPosition(posCellLeft+1,PTX_SectionCell,&cellSDH);
								if(jLeft <= iColInsertAt)
								{
									endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
									posEndCell = m_pDoc->getStruxPosition(endCellSDH);
									posCellLeft = posEndCell+1;
								}
								endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
								posEndCell = m_pDoc->getStruxPosition(endCellSDH);
								UT_String_sprintf(sLeft,"%d",iColInsertAt+1);
								UT_String_sprintf(sRight,"%d",iColInsertAt+2);
								props[4] = sColLeft.c_str();
								props[5] = sLeft.c_str();
								props[6] = sColRight.c_str();
								props[7] = sRight.c_str();
								bRes = m_pDoc->insertStrux(posCellLeft,PTX_SectionCell,NULL,props);
								bRes = m_pDoc->insertStrux(posCellLeft+1,PTX_Block);
								if(i == 0)
								{
									posFirstInsert = posCellLeft + 2;
								}
								bRes = m_pDoc->insertStrux(posCellLeft+2,PTX_EndCell);
							}
							j++;
						}
					}
					UT_ASSERT(bAfter);
				}
			}
		}
//
// OK now increment the cell container attach points for the cells after the inserted cells
//
		bRes = m_pDoc->getNextStruxOfType(tableSDH,PTX_EndTable,&endTableSDH);
		if(!bRes)
		{
			//
		// Disaster! the table structure in the piecetable is screwed.
		// we're totally stuffed now.
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		UT_sint32 iCurInsertAt = iColInsertAt;
		PT_DocPosition posEndTable = m_pDoc->getStruxPosition(endTableSDH);
		bool bEnd = false;
		UT_sint32 iCurLeft,iCurRight,iCurTop,iCurBot,iNewLeft,iNewRight;
		UT_sint32 iPrevLeft,iPrevRight,iPrevTop,iPrevBot;
		cellSDH = tableSDH;
		bool bFirst = true;
		PT_DocPosition iLastChangedPos = 0;
		if(bBefore && iCurInsertAt > 0)
		{
			iCurInsertAt--;
		}
		while(!bEnd)
		{
			bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
			if(!bRes)
			{
				bEnd = true;
				break;
			}
			endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
			posEndCell =  m_pDoc->getStruxPosition(endCellSDH);
			if(posEndCell+1 > posEndTable)
			{
				bEnd = true;
				break;
			}
			if(posEndCell+1 == posEndTable)
			{
				bEnd = true;
			}
			posCell =  m_pDoc->getStruxPosition(cellSDH);
			getCellParams(posCell+1, &iCurLeft, &iCurRight,&iCurTop,&iCurBot);
//
// OK after inserting, we simply increment all left/right attaches greater 
// than the iCurInsertAt by the number of columns we insert. 
// Or if there are two cells on the row with 
// same left attach, increment the second one.
//
//
			bool bChange = false;
			iNewLeft = iCurLeft;
			iNewRight = iCurRight;
			if(!bFirst)
			{
				bRes = m_pDoc->getPrevStruxOfType(cellSDH,PTX_SectionCell,&prevCellSDH);
				posPrevCell =  m_pDoc->getStruxPosition(prevCellSDH);
				getCellParams(posPrevCell+1, &iPrevLeft, &iPrevRight,&iPrevTop,&iPrevBot);
				if(iPrevLeft == iCurLeft && (iPrevTop == iCurTop) && (iLastChangedPos != posCell))
				{
					iLastChangedPos = posCell;
					bChange = true;
					iNewLeft+= 1;
					iNewRight+= 1;
				}
			}
			if(!bChange)
			{
				if((iCurLeft > iCurInsertAt +1) && (iLastChangedPos != posCell))
				{
					iLastChangedPos = posCell;
					bChange = true;
					iNewLeft+=1;
					iNewRight+=1;
				}
			}
			UT_DEBUGMSG(("SEVIOR: Looking at cell left %d right %d top %d bot %d bChange %d \n",iCurLeft,iCurRight,iCurTop,iCurBot,bChange));
			if(bChange)
			{
				UT_DEBUGMSG(("SEVIOR: changing cell to left %d right %d top %d bot %d \n",iNewLeft,iNewRight,iCurTop,iCurBot));
				const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
				UT_String sLeft,sRight,sTop,sBot;
				props[0] = "left-attach";
				UT_String_sprintf(sLeft,"%d",iNewLeft);
				props[1] = sLeft.c_str();
				props[2] = "right-attach";
				UT_String_sprintf(sRight,"%d",iNewRight);
				props[3] = sRight.c_str();
				props[4] = "top-attach";
				UT_String_sprintf(sTop,"%d",iCurTop);
				props[5] = sTop.c_str();
				props[6] = "bot-attach";
				UT_String_sprintf(sBot,"%d",iCurBot);
				props[7] = sBot.c_str();
				UT_DEBUGMSG(("SEVIOR: Changing cell strux %s %s %s %s %s %s %s %s \n",props[0],props[1],props[2],props[3],props[4],props[5],props[6],props[7]));
				bRes = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,props,PTX_SectionCell);
			}
			bFirst= false;
		}
//
// End of multiple column insert.
//
	}
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with the restored line-type property it has before.
//
	iListTag += 1;
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
// Put the insertion point in a legal position
//
	setPoint(posFirstInsert);
	notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return true;
}

/*!
 * posRow is the position of the start of the selection in the table.
 * insert rows into the table.
 */
bool FV_View::cmdInsertRow(PT_DocPosition posRow, bool bBefore)
{
	PL_StruxDocHandle cellSDH,tableSDH,endTableSDH,endCellSDH;
	PT_DocPosition posTable,posCell,posEndCell;
	UT_sint32 numRowsForInsertion = getNumRowsInSelection();
	if(numRowsForInsertion == 0)
	{
		return false;
	}

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	UT_sint32 iLeft,iRight,iTop,iBot;
	getCellParams(posRow, &iLeft, &iRight,&iTop,&iBot);
	
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posRow,PTX_SectionCell,&cellSDH);
	bRes = m_pDoc->getStruxOfTypeFromPosition(posRow,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);
  
	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;
  
	//
	// Now find the number of rows and columns inthis table. This is easiest to
	// get from the table container
	//
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(posRow);
	fp_Run * pRun;
	UT_sint32 xPoint,yPoint,xPoint2,yPoint2,iPointHeight;
	bool bDirection;
	pRun = pBL->findPointCoords(posRow, false, xPoint,
								yPoint, xPoint2, yPoint2,
								iPointHeight, bDirection);
  
	UT_return_val_if_fail(pRun, false);
	
	fp_Line * pLine = pRun->getLine();
	UT_return_val_if_fail(pLine, false);
	
	fp_Container * pCon = pLine->getContainer();
	UT_return_val_if_fail(pCon, false);
  
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCon->getContainer());
	UT_return_val_if_fail(pTab, false);
  
	UT_sint32 numCols = pTab->getNumCols();
	UT_sint32 numRows = pTab->getNumRows();
  
	//
	// Got all we need, now set things up to do the delete nicely
	//
	
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
  
	// Turn off list updates
	m_pDoc->disableListUpdates();
	m_pDoc->beginUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(true);
  
	//
	// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
	// with a bogus line-type property. We'll restore it later.
	//
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	const char * szListTag = NULL;
	UT_String sListTag;
	UT_sint32 iListTag;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szListTag);
	if(szListTag == NULL || *szListTag == '\0')
	{
		iListTag = 0;
	}
	else
	{
		iListTag = atoi(szListTag);
		iListTag -= 1;
	}
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("Sevior: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
  
	//
	// OK find the position to insert the row.
	// Have to handle the case of a cell spanning multiple rows.
	//
	// scan the piecetable and find either the first cell that has an attach above the current
    // row (bBefore or the cell witht he largest bottom attach that has a top attach on the row
    // for before
	//
	UT_sint32 prevTop = 100000;
	if(bBefore)
	{
		UT_sint32 Left,Right,Top,Bot;
		for(UT_sint32 i = 0; i < numCols; i++)
		{
			posCell = findCellPosAt(posTable,iTop,i);
			bRes = m_pDoc->getStruxOfTypeFromPosition(posCell+1,PTX_SectionCell,&cellSDH);
			getCellParams(posCell+1,&Left,&Right,&Top,&Bot);
			UT_DEBUGMSG(("Sevior: Cell position left %d right %d top %d bot %d \n",Left,Right,Top,Bot));
			if(Top < prevTop)
			{
				prevTop = Top;
			}
		}
//
// We insert at this position
//
		posCell =  findCellPosAt(posTable,prevTop,0);
	}
	else
	{
		UT_sint32 prevBot = -1;
		for(UT_sint32 i = 0; i < numCols; i++)
		{
			posCell = findCellPosAt(posTable,iTop,i);
			bRes = m_pDoc->getStruxOfTypeFromPosition(posCell+1,PTX_SectionCell,&cellSDH);
			UT_sint32 Left,Right,Top,Bot;
			getCellParams(posCell+1,&Left,&Right,&Top,&Bot);
			UT_DEBUGMSG(("Sevior: Cell position left %d right %d top %d bot %d \n",Left,Right,Top,Bot));
			if(Bot > prevBot)
			{
				prevBot = Bot;
			}
		}
//
// We insert at poscell position
//
		if(prevBot > numRows -1)
		{
//
// After Last row so just before the end of table
//
			prevTop = numRows;
			endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
			if(!endTableSDH)
			{
				//
				// Disaster! the table structure in the piecetable is screwed.
				// we're totally stuffed now.
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
			posCell = m_pDoc->getStruxPosition(endTableSDH);
		}
		else
		{
			prevTop = prevBot;
			posCell =  findCellPosAt(posTable,prevBot,0);
		}
		UT_DEBUGMSG(("SEVIOR: Inserting after row %d \n",prevTop));
	}		
//
// Got position and top row pos
// Now insert a row.
// 
	PT_DocPosition posInsert = 0;
	UT_sint32 i =0;
	UT_sint32 j = 0;
	for(j=0; j< numRowsForInsertion; j++)
	{
		for(i=0; i < numCols; i++)
		{
			const XML_Char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
			UT_String sRowTop = "top-attach";
			UT_String sRowBot = "bot-attach";
			UT_String sColLeft = "left-attach";
			UT_String sColRight = "right-attach";
			UT_String sTop,sBot,sLeft,sRight;
			UT_String_sprintf(sTop,"%d",prevTop);
			UT_String_sprintf(sBot,"%d",prevTop+1);
			UT_String_sprintf(sLeft,"%d",i);
			UT_String_sprintf(sRight,"%d",i+1);
			props[0] = sRowTop.c_str();
			props[1] = sTop.c_str();
			props[2] = sRowBot.c_str();
			props[3] = sBot.c_str();
			props[4] = sColLeft.c_str();
			props[5] = sLeft.c_str();
			props[6] = sColRight.c_str();
			props[7] = sRight.c_str();
			bRes = m_pDoc->insertStrux(posCell,PTX_SectionCell,NULL,props);
			bRes = m_pDoc->insertStrux(posCell+1,PTX_Block);
			if(i == 0)
			{
				posInsert = posCell+2;
			}
			bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
			posCell += 3;
		}
		prevTop++;
	}
//
// OK! now starting immediately after this insert loop through the table adding 1 to top and bottom
//
	endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
	if(!bRes)
	{
		//
		// Disaster! the table structure in the piecetable is screwed.
		// we're totally stuffed now.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	PT_DocPosition posEndTable = m_pDoc->getStruxPosition(endTableSDH);
	bool bEnd = false;
	UT_sint32 iCurLeft,iCurRight,iCurTop,iCurBot;
//
// -2 gives the last cell inserted.
//
	bRes = m_pDoc->getStruxOfTypeFromPosition(posCell-2,PTX_SectionCell,&cellSDH);
    while(!bEnd)
	{
		bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
		if(!bRes)
		{
			bEnd = true;
			break;
		}
		endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
		posEndCell =  m_pDoc->getStruxPosition(endCellSDH);
		if(posEndCell+1 > posEndTable)
		{
			bEnd = true;
			break;
		}
		if(posEndCell+1 == posEndTable)
		{
			bEnd = true;
		}
		posCell =  m_pDoc->getStruxPosition(cellSDH);
		getCellParams(posCell+1, &iCurLeft, &iCurRight,&iCurTop,&iCurBot);
		iCurTop+=numRowsForInsertion;
		iCurBot+=numRowsForInsertion;
		UT_DEBUGMSG(("SEVIOR: changing cell to left %d right %d top %d bot %d \n",iCurLeft,iCurRight,iCurTop,iCurBot));
		const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
		UT_String sLeft,sRight,sTop,sBot;
		props[0] = "left-attach";
		UT_String_sprintf(sLeft,"%d",iCurLeft);
		props[1] = sLeft.c_str();
		props[2] = "right-attach";
		UT_String_sprintf(sRight,"%d",iCurRight);
		props[3] = sRight.c_str();
		props[4] = "top-attach";
		UT_String_sprintf(sTop,"%d",iCurTop);
		props[5] = sTop.c_str();
		props[6] = "bot-attach";
		UT_String_sprintf(sBot,"%d",iCurBot);
		props[7] = sBot.c_str();
		UT_DEBUGMSG(("SEVIOR: Changing cell strux %s %s %s %s %s %s %s %s \n",props[0],props[1],props[2],props[3],props[4],props[5],props[6],props[7]));
		bRes = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,props,PTX_SectionCell);
	}
    //
    // Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
    // with the restored line-type property it has before.
    //
    iListTag += 1;
    UT_String_sprintf(sListTag,"%d",iListTag);
    pszTable[1] = sListTag.c_str();
    UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
    m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);

    //
    // OK finish everything off with the various parameters which allow the formatter to
    // be updated.
    //
	setPoint(posInsert);
    m_pDoc->endUserAtomicGlob();
    m_pDoc->setDontImmediatelyLayout(false);
    _generalUpdate();
    
    // restore updates and clean up dirty lists
    m_pDoc->enableListUpdates();
    m_pDoc->updateDirtyLists();
    
    // Signal PieceTable Changes have finished
    _restorePieceTableState();
    notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
    _ensureInsertionPointOnScreen();
    return true;
}

/*!
 * Delete the column containing the position posCol
 */
bool FV_View::cmdDeleteCol(PT_DocPosition posCol)
{
	PL_StruxDocHandle cellSDH,tableSDH,endTableSDH,endCellSDH;
	PT_DocPosition posTable,posCell;
	UT_sint32 iLeft,iRight,iTop,iBot;
	getCellParams(posCol, &iLeft, &iRight,&iTop,&iBot);

	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posCol,PTX_SectionCell,&cellSDH);
	bRes = m_pDoc->getStruxOfTypeFromPosition(posCol,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);

	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;
//
// Now find the number of rows and columns inthis table. This is easiest to
// get from the table container
//
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(posCol);
	fp_Run * pRun;
	UT_sint32 xPoint,yPoint,xPoint2,yPoint2,iPointHeight;
	bool bDirection;
	pRun = pBL->findPointCoords(posCol, false, xPoint,
							    yPoint, xPoint2, yPoint2,
							    iPointHeight, bDirection);

	UT_return_val_if_fail(pRun, false);

	fp_Line * pLine = pRun->getLine();
	UT_return_val_if_fail(pLine, false);

	fp_Container * pCon = pLine->getContainer();
	UT_return_val_if_fail(pCon, false);

	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCon->getContainer());
	UT_return_val_if_fail(pTab, false);

	UT_sint32 numRows = pTab->getNumRows();
//
// If we delete the last column we're actually deleteing the table. So let's
// do it.
//
	if(pTab->getNumCols() ==1)
	{
		cmdDeleteTable(posCol);
		return true;
	}
//
// Got all we need, now set things up to do the delete nicely
//
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
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with a bogus line-type property. We'll restore it later.
//
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	const char * szListTag = NULL;
	UT_String sListTag;
	UT_sint32 iListTag;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szListTag);
	if(szListTag == NULL || *szListTag == '\0')
	{
		iListTag = 0;
	}
	else
	{
		iListTag = atoi(szListTag);
		iListTag -= 1;
	}
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK loop through all the rows in this column and delete the entries in the specified
// column if the cell spans just the width of the column..
//
	UT_sint32 i =0;
	for(i=0; i <numRows; i++)
	{
		PT_DocPosition posCell = findCellPosAt(posTable,i,iLeft);
		UT_sint32 Left,Right,Top,Bot;
		getCellParams(posCell+1,&Left,&Right,&Top,&Bot);
		UT_DEBUGMSG(("SEVIOR: Before delete left %d right %d top %d bot %d \n",Left,Right,Top,Bot));
		if((Right - Left) == 1)
		{
			_deleteCellAt(posTable,i, iLeft);
		}
	}
//
// OK now subtract one from all the column coordinates in the table with iLeft > iLeft
// do this by running through the linked list of SectionCell fragments in the piecetable
//
// We stop when the position of the endCell strux is just before the position of
// the endTable strux. So lets's get that now.
//
	bRes = m_pDoc->getNextStruxOfType(tableSDH,PTX_EndTable,&endTableSDH);
	if(!bRes)
	{
		//
		// Disaster! the table structure in the piecetable is screwed.
		// we're totally stuffed now.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	PT_DocPosition posEndTable = m_pDoc->getStruxPosition(endTableSDH);
	PT_DocPosition posEndCell;
	bool bEnd = false;
	UT_sint32 iCurLeft,iCurRight,iCurTop,iCurBot,iNewLeft,iNewRight;
	cellSDH = tableSDH;
    while(!bEnd)
	{
		bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
		if(!bRes)
		{
			bEnd = true;
			break;
		}
		posCell =  m_pDoc->getStruxPosition(cellSDH);
		getCellParams(posCell+1, &iCurLeft, &iCurRight,&iCurTop,&iCurBot);
		UT_DEBUGMSG(("SEVIOR: Looking at cell left %d right %d top %d bot %d \n",iCurLeft,iCurRight,iCurTop,iCurBot));
		bool bChange = false;
		iNewLeft = iCurLeft;
		iNewRight = iCurRight;
		if(iCurLeft > iLeft)
		{
			bChange = true;
			iNewLeft--;
		}
		if(iCurRight > iLeft)
		{
			bChange = true;
			iNewRight--;
		}
		if(bChange)
		{
			UT_DEBUGMSG(("SEVIOR: changing cell to left %d right %d top %d bot %d \n",iNewLeft,iNewRight,iCurTop,iCurBot));
			const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
			UT_String sLeft,sRight,sTop,sBot;
			props[0] = "left-attach";
			UT_String_sprintf(sLeft,"%d",iNewLeft);
			props[1] = sLeft.c_str();
			props[2] = "right-attach";
			UT_String_sprintf(sRight,"%d",iNewRight);
			props[3] = sRight.c_str();
			props[4] = "top-attach";
			UT_String_sprintf(sTop,"%d",iCurTop);
			props[5] = sTop.c_str();
			props[6] = "bot-attach";
			UT_String_sprintf(sBot,"%d",iCurBot);
			props[7] = sBot.c_str();
			bRes = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,props,PTX_SectionCell);
		}
		endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
		posEndCell =  m_pDoc->getStruxPosition(endCellSDH);
		if(posEndCell+1 >= posEndTable)
		{
			bEnd = true;
		}
	}
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with the restored line-type property it has before.
//
	iListTag += 1;
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
    notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();

	_ensureInsertionPointOnScreen();
	return true;
}


/*!
 * Delete the table containing the position posRow
 */
bool FV_View::cmdDeleteTable(PT_DocPosition posTable)
{
	PL_StruxDocHandle tableSDH,endTableSDH;
	PT_DocPosition posStartTable,posEndTable;
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posTable,PTX_SectionTable,&tableSDH);
	if(!bRes)
	{
		return false;
	}
	posStartTable = m_pDoc->getStruxPosition(tableSDH);
	endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
	posEndTable = m_pDoc->getStruxPosition(endTableSDH)+1;
//
// Got all we need, now set things up to do the delete nicely
//
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
//
// OK do the delete
//
	UT_uint32 iRealDeleteCount;
	m_pDoc->deleteSpan( posStartTable, posEndTable, NULL,iRealDeleteCount,true);
//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
    notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();

	return true;
}

/*!
 * Delete the row containing the position posRow
 */
bool FV_View::cmdDeleteRow(PT_DocPosition posRow)
{
	PL_StruxDocHandle cellSDH,tableSDH,endTableSDH,endCellSDH;
	PT_DocPosition posTable,posCell;
	UT_sint32 iLeft,iRight,iTop,iBot;
	getCellParams(posRow, &iLeft, &iRight,&iTop,&iBot);

	bool bRes = m_pDoc->getStruxOfTypeFromPosition(posRow,PTX_SectionCell,&cellSDH);
	bRes = m_pDoc->getStruxOfTypeFromPosition(posRow,PTX_SectionTable,&tableSDH);
	UT_return_val_if_fail(bRes, false);

	posTable = m_pDoc->getStruxPosition(tableSDH) + 1;
//
// Now find the number of rows and columns inthis table. This is easiest to
// get from the table container
//
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(posRow);
	fp_Run * pRun;
	UT_sint32 xPoint,yPoint,xPoint2,yPoint2,iPointHeight;
	bool bDirection;
	pRun = pBL->findPointCoords(posRow, false, xPoint,
							    yPoint, xPoint2, yPoint2,
							    iPointHeight, bDirection);
	UT_return_val_if_fail(pRun, false);

	fp_Line * pLine = pRun->getLine();
	UT_return_val_if_fail(pLine, false);

	fp_Container * pCon = pLine->getContainer();
	UT_return_val_if_fail(pCon, false);

	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCon->getContainer());
	UT_return_val_if_fail(pTab, false);

	UT_sint32 numCols = pTab->getNumCols();
//
// If we delete the last row we're actually deleting the table, so do that 
// instead.
//
	if(pTab->getNumRows() == 1)
	{
		cmdDeleteTable(posRow);
		return true;
	}
//
// Got all we need, now set things up to do the delete nicely
//
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
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with a bogus line-type property. We'll restore it later.
//
	const char * pszTable[3] = {NULL,NULL,NULL};
	pszTable[0] = "list-tag";
	const char * szListTag = NULL;
	UT_String sListTag;
	UT_sint32 iListTag;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szListTag);
	if(szListTag == NULL || *szListTag == '\0')
	{
		iListTag = 0;
	}
	else
	{
		iListTag = atoi(szListTag);
		iListTag -= 1;
	}
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK loop through all the rows in this column and delete the entries in the specified
// column if the cell spans just the width of the column..
//
	UT_sint32 i =0;
	for(i=0; i <numCols; i++)
	{
		PT_DocPosition posCell = findCellPosAt(posTable,i,iLeft);
		UT_sint32 Left,Right,Top,Bot;
		getCellParams(posCell+1,&Left,&Right,&Top,&Bot);
		UT_DEBUGMSG(("SEVIOR: Before delete left %d right %d top %d bot %d \n",Left,Right,Top,Bot));
		if((Bot - Top) == 1)
		{
			_deleteCellAt(posTable,iTop, i);
		}
	}
//
// OK now subtract one from all the row coordinates in the table with iTop,iBot > iTop
// do this by running through the linked list of SectionCell fragments in the piecetable
//
// We stop when the position of the endCell strux is just before the position of
// the endTable strux. So lets's get that now.
//
	endTableSDH = m_pDoc->getEndTableStruxFromTableSDH(tableSDH);
	if(!bRes)
	{
		//
		// Disaster! the table structure in the piecetable is screwed.
		// we're totally stuffed now.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	PT_DocPosition posEndTable = m_pDoc->getStruxPosition(endTableSDH);
	PT_DocPosition posEndCell;
	bool bEnd = false;
	UT_sint32 iCurLeft,iCurRight,iCurTop,iCurBot,iNewTop,iNewBot;
	cellSDH = tableSDH;
    while(!bEnd)
	{
		bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
		if(!bRes)
		{
			bEnd = true;
			break;
		}
		posCell =  m_pDoc->getStruxPosition(cellSDH);
		getCellParams(posCell+1, &iCurLeft, &iCurRight,&iCurTop,&iCurBot);
		UT_DEBUGMSG(("SEVIOR: Looking at cell left %d right %d top %d bot %d \n",iCurLeft,iCurRight,iCurTop,iCurBot));
		bool bChange = false;
		iNewTop = iCurTop;
		iNewBot = iCurBot;
		if(iCurTop > iTop)
		{
			bChange = true;
			iNewTop--;
		}
		if(iCurBot > iTop)
		{
			bChange = true;
			iNewBot--;
		}
		if(bChange)
		{
			UT_DEBUGMSG(("SEVIOR: changing cell to left %d right %d top %d bot %d \n",iCurLeft,iCurRight,iNewTop,iNewBot));
			const char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
			UT_String sLeft,sRight,sTop,sBot;
			props[0] = "left-attach";
			UT_String_sprintf(sLeft,"%d",iCurLeft);
			props[1] = sLeft.c_str();
			props[2] = "right-attach";
			UT_String_sprintf(sRight,"%d",iCurRight);
			props[3] = sRight.c_str();
			props[4] = "top-attach";
			UT_String_sprintf(sTop,"%d",iNewTop);
			props[5] = sTop.c_str();
			props[6] = "bot-attach";
			UT_String_sprintf(sBot,"%d",iNewBot);
			props[7] = sBot.c_str();
			bRes = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,props,PTX_SectionCell);
		}
		endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
		posEndCell =  m_pDoc->getStruxPosition(endCellSDH);
		if(posEndCell+1 >= posEndTable)
		{
			bEnd = true;
		}
	}
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with the restored line-type property it has before.
//
	iListTag += 1;
	UT_String_sprintf(sListTag,"%d",iListTag);
	pszTable[1] = sListTag.c_str();
	UT_DEBUGMSG(("SEVIOR: Doing Table strux change of %s %s \n",pszTable[0],pszTable[1]));
	m_pDoc->changeStruxFmt(PTC_AddFmt,posTable,posTable,NULL,pszTable,PTX_SectionTable);
//
// OK finish everything off with the various parameters which allow the formatter to
// be updated.
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
    notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();

	_ensureInsertionPointOnScreen();
	return true;
}


/*!
 * Delete the cell at the specified position
 */
bool FV_View::cmdDeleteCell(PT_DocPosition cellPos)
{
#if 1
  UT_ASSERT(UT_NOT_IMPLEMENTED);
  return true ;
#else
	PL_StruxDocHandle cellSDH;
	const char * pszLeftAttach =NULL;
	const char * pszTopAttach = NULL;
	UT_sint32 iLeft =-999;
	UT_sint32 iTop = -999;
	bool bRes = m_pDoc->getStruxOfTypeFromPosition(cellPos,PTX_SectionCell,&cellSDH);
	if(!bRes)
	{
		return false;
	}
	m_pDoc->getPropertyFromSDH(cellSDH,"left-attach",&pszLeftAttach);
	m_pDoc->getPropertyFromSDH(cellSDH,"top-attach",&pszTopAttach);
	if(pszLeftAttach && *pszLeftAttach)
	{
		iLeft = atoi(pszLeftAttach);
	}
	if(pszTopAttach && *pszTopAttach)
	{
		iTop = atoi(pszTopAttach);
	}
//
// Got all we need, now set things up to do the delete nicely
//
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
//
// delete the cell.
//
	_deleteCellAt(cellPos,iTop, iLeft);
//
// OK do all the piecetable finished changing business
//
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
    notifyListeners(AV_CHG_MOTION);
	_fixInsertionPointCoords();

	_ensureInsertionPointOnScreen();
	return true;
#endif
}


/*!
 * Insert a table of the  number of rows and columns given.
 */
UT_Error FV_View::cmdInsertTable(UT_sint32 numRows, UT_sint32 numCols, const XML_Char * pPropsArray[])
{
	// TODO -- why does this function return UT_Error? If bool is
	// sufficient, it should return bool, and if not, than the
	// UT_Error & bool operations below are probably not safe
	UT_Error e = UT_OK;
	if(numRows == 0 || numCols==0)
	{
		return 0; 
	}
	if(isInTable() && isHdrFtrEdit())
	{
		return 0;
	}
//
// Do all the stuff we need to make this go smoothly and to undo in a single step.
//
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
//
// insert a block to terminate the text before this.
//
	PT_DocPosition pointBreak = getPoint();
	PT_DocPosition pointTable = 0;
	e = m_pDoc->insertStrux(getPoint(),PTX_Block);
//
// Insert the table strux at the same spot. This will make the table link correctly in the
// middle of the broken text.
//
// Handle special case of not putting a table immediately after a section break
//
	PL_StruxDocHandle secSDH = NULL;
	bool bres = m_pDoc->getStruxOfTypeFromPosition(pointBreak-1,PTX_Section,&secSDH);
	PT_DocPosition secPos = m_pDoc->getStruxPosition(secSDH);
	UT_DEBUGMSG(("SEVIOR: SecPos %d pointBreak %d \n",secPos,pointBreak));
	secSDH = NULL;
	bres = m_pDoc->getStruxOfTypeFromPosition(pointBreak,PTX_SectionCell,&secSDH);
	if(secSDH != NULL)
	{
		PT_DocPosition secPos = m_pDoc->getStruxPosition(secSDH);
		UT_DEBUGMSG(("SEVIOR: Cell Pos %d pointBreak %d \n",secPos,pointBreak));	}
	setPoint(pointBreak);
	e |= static_cast<UT_sint32>(m_pDoc->insertStrux(getPoint(),PTX_SectionTable,NULL,pPropsArray));
//
// stuff for cell insertion.
//
	UT_sint32 i,j;
	const XML_Char * attrs[3] = {"style","Normal",NULL};
	const XML_Char * props[9] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	UT_String sRowTop = "top-attach";
	UT_String sRowBot = "bot-attach";
	UT_String sColLeft = "left-attach";
	UT_String sColRight = "right-attach";
	UT_String sTop,sBot,sLeft,sRight;
	for(i=0;i<numRows;i++)
	{
		UT_String_sprintf(sTop,"%d",i);
		UT_String_sprintf(sBot,"%d",i+1);
		props[0] = sRowTop.c_str();
		props[1] = sTop.c_str();
		props[2] = sRowBot.c_str();
		props[3] = sBot.c_str();
		for(j=0;j<numCols;j++)
		{
			UT_String_sprintf(sLeft,"%d",j);
			UT_String_sprintf(sRight,"%d",j+1);
			props[4] = sColLeft.c_str();
			props[5] = sLeft.c_str();
			props[6] = sColRight.c_str();
			props[7] = sRight.c_str();
			e |= static_cast<UT_sint32>(m_pDoc->insertStrux(getPoint(),PTX_SectionCell,NULL,props));
			pointBreak = getPoint();
			e |= static_cast<UT_sint32>(m_pDoc->insertStrux(getPoint(),PTX_Block,attrs,NULL));
			UT_DEBUGMSG(("SEVIOR: 4  cur point %d \n",getPoint()));
			if(getPoint() == pointBreak)
			{
				setPoint(pointBreak+1);
			}
			if(i == 0 && j==0)
			{
				pointTable = getPoint();
			}
			e |= static_cast<UT_sint32>(m_pDoc->insertStrux(getPoint(),PTX_EndCell));
		}
	}
	e |= static_cast<UT_sint32>(m_pDoc->insertStrux(getPoint(),PTX_EndTable));

	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	setPoint(pointTable);
	_fixInsertionPointCoords();
	_generalUpdate();
	AV_View::notifyListeners (AV_CHG_ALL);
	m_pG->getCaret()->setBlink(false);
	focusChange(AV_FOCUS_HERE);
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();
	return e;
}

bool FV_View::cmdCharInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce)
{
	// the code below inserts a direction marker before a space if the automatic insertion of such
	// markers is indicated by user's preferences and if the current keyboard language direction is
	// inconsistent with the dominant direction of the paragraph (this makes phone numbers and similar
	// to work in rtl languages more intuitively)

	// we only do this for space ... (certain other chars can be handled in ap_EditMethods.cpp
	// because they do not need knowledge of block direction)
	if(count == 1 && text[0] == UCS_SPACE)
	{
		bool bLang = false, bMarker = false;

		XAP_App::getApp()->getPrefsValueBool(static_cast<const XML_Char *>(XAP_PREF_KEY_ChangeLanguageWithKeyboard),
											 &bLang);

		const UT_LangRecord * pLR = NULL;
	
		if(bLang)
		{
			pLR = XAP_App::getApp()->getKbdLanguage();
		
			XAP_App::getApp()->getPrefsValueBool(static_cast<const XML_Char *>(XAP_PREF_KEY_DirMarkerAfterClosingParenthesis), &bMarker);
		}

		if(bMarker && pLR)
		{
			fl_BlockLayout * pBlock = m_pLayout->findBlockAtPosition(getPoint());

			if(!pBlock)
				goto normal_insert;
			
			{
				UT_BidiCharType iDomDir = pBlock->getDominantDirection();
				
				UT_UCS4Char data[2];
				data[1] = *text;
		
				if(pLR->m_eDir == UTLANG_RTL && iDomDir != UT_BIDI_RTL)
				{
					data[0] = UCS_RLM;
				}
				else if(pLR->m_eDir == UTLANG_LTR  && iDomDir != UT_BIDI_LTR)
				{
					data[0] = UCS_LRM;
				}
				else
				{
					goto normal_insert;
				}

				return _charInsert(&data[0],2,bForce);
			}
		}
	}

   normal_insert:
	return _charInsert(text, count, bForce);
}

bool FV_View::cmdStartList(const XML_Char * style)
{
	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	pBlock->StartList( style);
	m_pDoc->endUserAtomicGlob();

	return true;
}


bool FV_View::cmdStopList(void)
{


	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	m_pDoc->beginUserAtomicGlob();
	fl_BlockLayout * pBlock = getCurrentBlock();
	m_pDoc->StopList(pBlock->getStruxDocHandle());
	m_pDoc->endUserAtomicGlob();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	return true;
}


void FV_View::cmdCharDelete(bool bForward, UT_uint32 count)
{
	const XML_Char * properties[] = { "font-family", NULL, 0};
	const XML_Char ** props_in = NULL;
	const XML_Char * currentfont;
	bool bisList = false;
	fl_BlockLayout * curBlock = NULL;
	fl_BlockLayout * nBlock = NULL;
	UT_uint32 iRealDeleteCount = 0;


	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
	{
		m_pDoc->disableListUpdates();

		_deleteSelection();
		_generalUpdate();

		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();
		_fixInsertionPointCoords();
		_ensureInsertionPointOnScreen();
	}
	else
	{
		//
		// Look to see if there is a tab - list label deal with these together
		//
		if((bForward == false) && (count == 1))
		{
			if(isTabListBehindPoint() == true)
			{
				curBlock = _findBlockAtPosition(getPoint());
				nBlock = _findBlockAtPosition(getPoint()-2);
				if(nBlock == curBlock)
				{
					count = 2;
					bisList = true;
				}
			}
		}
		if((bForward == true) && (count == 1))
		{
			if(isTabListAheadPoint() == true)
			{
//
// Check we're at the start of a block
//
				if(getPoint() == getCurrentBlock()->getPosition())
				{
					bisList = true;
					count = 2;
				}
			}

		}
//
// Code to deal with deleting a footnote reference that embeds a footnote Layout.
//
		if(bForward)
		{
			if(!isInFootnote() && isInFootnote(getPoint() + count))
			{
				fl_FootnoteLayout * pFL = getClosestFootnote(getPoint() + count +1);
				count += pFL->getLength();
			}
			if(!isInEndnote() && isInEndnote(getPoint() + count))
			{
				fl_EndnoteLayout * pEL = getClosestEndnote(getPoint() + count +1);
				count += pEL->getLength();
			}
		}
		else
		{
			if(!isInFootnote(getPoint()) && isInFootnote(getPoint() - count))
			{
				fl_FootnoteLayout * pFL = getClosestFootnote(getPoint());
				count += pFL->getLength();
			}
			else if(!isInEndnote() && isInEndnote(getPoint() - count))
			{
				fl_EndnoteLayout * pEL = getClosestEndnote(getPoint());
				count += pEL->getLength();
			}
			if(isInFootnote() && !isInFootnote(getPoint() - count))
			{
//
// Can't delete a footnote strux. Bail out.
//
				return;
			}
			else if(isInEndnote() && !isInEndnote(getPoint() - count))
			{
//
// Can't delete a footnote strux. Bail out.
//
				return;
			}
		}
		// Code to deal with font boundary problem.
		// TODO: This should really be fixed by someone who understands
		// how this code works! In the meantime save current font to be
		// restored after character is deleted.

		getCharFormat(&props_in);
		currentfont = UT_getAttribute("font-family",props_in);
		properties[1] = currentfont;
		UT_DEBUGMSG(("deleteSpan - 1: Inital pos %d count %d \n",getPoint(),count));

		UT_uint32 amt = count;
		UT_uint32 posCur = getPoint();
		UT_uint32 nposCur = getPoint();
		bool fontFlag = false;

		if (!bForward)
		{

			if (!_charMotion(bForward,count))
			{
				UT_ASSERT(getPoint() <= posCur);
				UT_DEBUGMSG(("SEVIOR: posCur %d getPoint() %d \n",posCur,getPoint()));
				amt = posCur - getPoint();
			}

			posCur = getPoint();
			// Code to deal with change of font boundaries:
			if((posCur == nposCur) && (posCur > 0))
			{
				fontFlag = true;
				posCur--;
			}
		}
		else
		{
			PT_DocPosition posEOD;
			bool bRes;

			bRes = getEditableBounds(true, posEOD);
			UT_ASSERT(bRes);
			UT_ASSERT(posCur <= posEOD);

			if (posEOD < (posCur+amt))
			{
				amt = posEOD - posCur;
			}
		}

		if(!curBlock)
			curBlock = _findBlockAtPosition(getPoint());

		if (amt > 0)
		{
			m_pDoc->disableListUpdates();

			nBlock = _findBlockAtPosition(getPoint());
			fl_AutoNum * pAuto = nBlock->getAutoNum();
			if(pAuto != NULL )
			{
				PL_StruxDocHandle sdh = nBlock->getStruxDocHandle();
				if((bisList == true) && (pAuto->getFirstItem() == sdh || pAuto->getLastItem() == sdh))
				{
					m_pDoc->StopList(sdh);
					PT_DocPosition listPoint,posEOD;
					getEditableBounds(true, posEOD);
					listPoint = getPoint();
					fl_AutoNum * pAuto = nBlock->getAutoNum();
					if(pAuto != NULL)
					{
						if(listPoint + 2 <= posEOD)
							_setPoint(listPoint+2);
						else
							_setPoint(posEOD);
					}
				}
				else if(bisList == true)
				{
					m_pDoc->deleteSpan(posCur, posCur+amt,NULL, iRealDeleteCount);
					nBlock->remItemFromList();
				}
				else
				{
					m_pDoc->deleteSpan(posCur, posCur+amt,NULL, iRealDeleteCount);
				}
			}
			else
			{
				UT_DEBUGMSG(("deleteSpan - 2: posCur %d amt %d \n",posCur,amt));
				m_pDoc->deleteSpan(posCur, posCur+amt,NULL, iRealDeleteCount);
			}

			if(fontFlag)
			{
				setCharFormat(properties);
			}
		}
//
// Dont leave a List field - tab on a line.
//
		if(isTabListAheadPoint())
		{
			UT_uint32 iRealDeleteCount2;

			m_pDoc->deleteSpan(getPoint(), getPoint()+2,NULL,iRealDeleteCount2);
			iRealDeleteCount += iRealDeleteCount2;
		}

		// restore updates and clean up dirty lists
		m_pDoc->enableListUpdates();
		m_pDoc->updateDirtyLists();

		_generalUpdate();
		free(props_in);

		_fixInsertionPointCoords();
		_ensureInsertionPointOnScreen();

		//special handling is required for delete in revisions mode
		//where we have to move the insertion point
		// only if we are deleting forward; if deleting backwards, the
		// code above already moved the insertion point
		// Tomas, Oct 28, 2003
		if(bForward && isMarkRevisions())
		{
			UT_ASSERT( iRealDeleteCount <= count );
			_charMotion(bForward,count - iRealDeleteCount);
		}
	}


	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_setPoint(getPoint());
}


void FV_View::cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos)
{
#define HACK_LINE_HEIGHT				20 // TODO Fix this!!

	UT_sint32 lineHeight = iPos;
	UT_sint32 docHeight = 0;
	bool bVertical = false;
	bool bHorizontal = false;

	docHeight = m_pLayout->getHeight();

	if (lineHeight == 0)
		lineHeight = m_pG->tlu(HACK_LINE_HEIGHT);

	UT_sint32 yoff = m_yScrollOffset;
	UT_sint32 xoff = m_xScrollOffset;

	switch(cmd)
	{
	case AV_SCROLLCMD_PAGEDOWN:
		yoff += getWindowHeight();
		bVertical = true;
		break;
	case AV_SCROLLCMD_PAGEUP:
		yoff -= getWindowHeight();
		bVertical = true;
		break;
	case AV_SCROLLCMD_PAGELEFT:
		xoff -= getWindowWidth();
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_PAGERIGHT:
		xoff += getWindowWidth();
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_LINEDOWN:
		yoff += lineHeight;
		bVertical = true;
		break;
	case AV_SCROLLCMD_LINEUP:
		yoff -= lineHeight;
		bVertical = true;
		break;
	case AV_SCROLLCMD_LINELEFT:
		xoff -= lineHeight;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_LINERIGHT:
		xoff += lineHeight;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_TOTOP:
		yoff = 0;
		bVertical = true;
		break;
	case AV_SCROLLCMD_TOPOSITION:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		break;
	case AV_SCROLLCMD_TOBOTTOM:
		fp_Page* pPage = m_pLayout->getFirstPage();
		UT_sint32 iDocHeight = getPageViewTopMargin();
		while (pPage)
		{
			iDocHeight += pPage->getHeight() + getPageViewSep();
			pPage = pPage->getNext();
		}
		yoff = iDocHeight;
		bVertical = true;
		break;
	}

	if (yoff < 0)
		yoff = 0;

	bool bRedrawPoint = true;

	if (bVertical && (yoff != m_yScrollOffset))
	{
		sendVerticalScrollEvent(yoff);
		if ((cmd != AV_SCROLLCMD_PAGEUP
			 && cmd != AV_SCROLLCMD_PAGEDOWN))
		  bRedrawPoint = false;
//		UT_ASSERT(m_yScrollOffset == m_pG->getPrevYOffset());
	}

	if (xoff < 0)
		xoff = 0;

	if (bHorizontal && (xoff != m_xScrollOffset))
	{
		sendHorizontalScrollEvent(xoff);
		bRedrawPoint = false;
	}
	
	if (bRedrawPoint)
		_fixInsertionPointCoords();

}


void FV_View::cmdSelect(PT_DocPosition dpBeg, PT_DocPosition dpEnd)
{

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	_setPoint(dpBeg);
	_setSelectionAnchor();
	m_Selection.setSelectionLeftAnchor(dpBeg);
	m_Selection.setSelectionRightAnchor(dpEnd);
	_setPoint (dpEnd);
	UT_ASSERT(!isSelectionEmpty());

	if (dpBeg == dpEnd)
	{
		return;
	}

	_drawSelection();

	notifyListeners(AV_CHG_EMPTYSEL);
}

#define IS_SELECTALL(a, b) ((a) == FV_DOCPOS_BOD && (b) == FV_DOCPOS_EOD)

void FV_View::cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd)
{
	UT_DEBUGMSG(("Double click on mouse \n"));

	warpInsPtToXY(xPos, yPos,true);

	PT_DocPosition iPosLeft = _getDocPos(dpBeg, false);
	PT_DocPosition iPosRight = _getDocPos(dpEnd, false);

	if(iPosLeft == iPosRight) return;
//
// Code to select a paragraph break on selectLine if on first line of a Block.
//
	if((dpBeg == FV_DOCPOS_BOL) || (dpBeg == FV_DOCPOS_BOP) || (dpBeg == FV_DOCPOS_BOD))
	{
		fl_BlockLayout * pBlock =  _findBlockAtPosition(iPosLeft);
		UT_sint32 x, y, x2, y2, h;
		bool b;
		fp_Run* pRun = pBlock->findPointCoords(m_iInsPoint, false, x, y, x2, y2, h, b);
		if(pRun)
		{
			fp_Line * pLine = pRun->getLine();
			if(pLine == static_cast<fp_Line *>(pBlock->getFirstContainer()))
			{
				iPosLeft = pBlock->getPosition() -1;
			}
		}
	}
	cmdSelect (iPosLeft, iPosRight);
}

void FV_View::cmdHyperlinkJump(UT_sint32 xPos, UT_sint32 yPos)
{
	_clearSelection();
	warpInsPtToXY(xPos, yPos,true);

	fl_BlockLayout * pBlock = getCurrentBlock();
	PT_DocPosition iRelPos = getPoint() - pBlock->getPosition(false);

	fp_Run *pRun = pBlock->getFirstRun();
	while (pRun && pRun->getBlockOffset()+ pRun->getLength() < iRelPos)
		pRun= pRun->getNextRun();

	UT_ASSERT(pRun);
	pRun->getPrevRun();

	UT_ASSERT(pRun);
#if 0
	if(pRun->getType()== FPRUN_FMTMARK || pRun->getType()== FPRUN_HYPERLINK || pRun->getType()== FPRUN_BOOKMARK)
		pRun  = pRun->getNextRun();

	UT_ASSERT(pRun);
#endif
	fp_HyperlinkRun * pH = pRun->getHyperlink();

	UT_ASSERT(pH);

	const XML_Char * pTarget = pH->getTarget();

	if(*pTarget == '#')
		pTarget++;

	UT_uint32 iTargetLen = UT_XML_strlen(pTarget);
	UT_UCSChar * pTargetU = new UT_UCSChar[iTargetLen+1];

	UT_ASSERT(pTargetU);

	UT_UCSChar * pJump = pTargetU;

	for (UT_uint32 i = 0; i < iTargetLen; i++)
		*pTargetU++ = (UT_UCSChar) *pTarget++;
	*pTargetU = 0;

	gotoTarget(AP_JUMPTARGET_BOOKMARK, pJump);

	delete [] pJump;
}

void FV_View::cmdUndo(UT_uint32 count)
{
	if (!isSelectionEmpty())
		_clearSelection();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();

// Don't update tables until finished

	m_pDoc->setDontImmediatelyLayout(true);

	// Remember the current position, We might need it later.
	rememberCurrentPosition();
	UT_DEBUGMSG(("SEVIOR: undoing %d operations \n",count));
	m_pDoc->undoCmd(count);
	allowChangeInsPoint();
	m_pDoc->setDontImmediatelyLayout(false);

//
// Now do a general update to make everything look good again.
//
	_generalUpdate();

	notifyListeners(AV_CHG_DIRTY);

// Look to see if we need the saved insertion point after the undo
// 	if(needSavedPosition())
// 	{
//
// We do, so restore insertion point to that value.
// 		_setPoint(getSavedPosition());
// 		clearSavedPosition();
// 	}
	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;
	// Move insertion point out of field run if it is in one
	//
	_charMotion(true, 0);
//
// Do a complete update coz who knows what happened in the undo!
//
	notifyListeners(AV_CHG_ALL);
	setCursorToContext();


	_updateInsertionPoint();
}

void FV_View::cmdRedo(UT_uint32 count)
{
	if (!isSelectionEmpty())
		_clearSelection();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	// Turn off list updates
	m_pDoc->disableListUpdates();
	m_pDoc->setDontImmediatelyLayout(true);

	// Remember the current position, We might need it later.
	rememberCurrentPosition();

	m_pDoc->redoCmd(count);
	allowChangeInsPoint();

// Look to see if we need the saved insertion point after the undo
// 	if(needSavedPosition())
// 	{
//
// We do, so restore insertion point to that value.
//
// 		_setPoint(getSavedPosition());
// 		clearSavedPosition();
// 	}
	m_pDoc->setDontImmediatelyLayout(false);

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	_generalUpdate();

//
// Do a complete update coz who knows what happened in the undo!
//
	notifyListeners(AV_CHG_ALL);
	setCursorToContext();

	_updateInsertionPoint();
}

UT_Error FV_View::cmdSave(void)
{
	// transfer any persistent properties into the doc
	const XML_Char ** ppProps = getViewPersistentProps();
	m_pDoc->setProperties(ppProps);
	
	UT_Error tmpVar;
	tmpVar = m_pDoc->save();
	if (!tmpVar)
	{
		notifyListeners(AV_CHG_SAVE);
	}
	return tmpVar;
}

UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft, bool cpy)
{
	// transfer any persistent properties into the doc
	const XML_Char ** ppProps = getViewPersistentProps();
	m_pDoc->setProperties(ppProps);

	UT_Error tmpVar;
	tmpVar = m_pDoc->saveAs(szFilename, ieft, cpy);
	if (!tmpVar && cpy)
	{
		notifyListeners(AV_CHG_SAVE);
	}
	return tmpVar;
}

UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft)
{
  return cmdSaveAs(szFilename, ieft, true);
}


void FV_View::cmdCut(void)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}
	if(m_Selection.getSelectionMode() == FV_SelectionMode_TableColumn)
	{
		PD_DocumentRange * pDR = m_Selection.getNthSelection(0);
		PT_DocPosition pos = pDR->m_pos1 +1;
		_clearSelection();
		cmdDeleteCol(pos);
		return;
	}
	if(m_Selection.getSelectionMode() == FV_SelectionMode_TableRow)
	{
		PD_DocumentRange * pDR = m_Selection.getNthSelection(0);
		PT_DocPosition pos = pDR->m_pos1 +1;
		_clearSelection();
		cmdDeleteRow(pos);
		return;
	}
	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();
	cmdCopy(true);
	_deleteSelection();
	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();


	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;

	_setPoint(getPoint());
}

// bToClipboard is true if you want to copy to the CLIPBOARD
// selection on UNIX, as opposed to some PRIMARY selection
void FV_View::cmdCopy(bool bToClipboard)
{
	if (isSelectionEmpty())
	{
		// clipboard does nothing if there is no selection
		return;
	}

	PD_DocumentRange dr;
	getDocumentRangeOfCurrentSelection(&dr);
	m_pApp->copyToClipboard(&dr, bToClipboard);
	notifyListeners(AV_CHG_CLIPBOARD);
}

void FV_View::cmdPaste(bool bHonorFormatting)
{
//
// Look to see if should paste a table column or row
//
	if((m_Selection.getPrevSelectionMode() == FV_SelectionMode_TableColumn)
	   || (m_Selection.getPrevSelectionMode() == 	FV_SelectionMode_TableRow))
	{
		if(isInTable())
		{
			fl_TableLayout * pTab = getTableAtPos(getPoint());
			if(pTab && pTab == m_Selection.getTableLayout())
			{
				m_Selection.pasteRowOrCol();
				return;
			}
		}
	}

	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.

	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();
	m_pDoc->setDoingPaste();
	setCursorWait();
	_doPaste(true, bHonorFormatting);

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();
	clearCursorWait();

	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;

	m_pDoc->clearDoingPaste();
	m_pDoc->endUserAtomicGlob();
	m_iPieceTableState = 0;
	// Move insertion point out of field run if it is in one
	//
	_charMotion(true, 0);

//
// Do a complete update coz who knows what happened in the paste!
//
	notifyListeners(AV_CHG_ALL);

	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();

}

void FV_View::cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos)
{
	// this is intended for the X11 middle mouse paste trick.
	//
	// if this view has the selection, we need to remember it
	// before we warp to the given (x,y) -- or else there won't
	// be a selection to paste when get there.	this is sort of
	// back door hack and should probably be re-thought.

	// set UAG markers around everything that the actual paste does
	// so that undo/redo will treat it as one step.

	m_pDoc->beginUserAtomicGlob();

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
		m_pApp->cacheCurrentSelection(this);
	cmdCopy(false);
	warpInsPtToXY(xPos,yPos,true);
	_doPaste(false, true);
	m_pApp->cacheCurrentSelection(NULL);

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_fixInsertionPointCoords();
	_ensureInsertionPointOnScreen();

	m_pDoc->endUserAtomicGlob();
}

UT_Error FV_View::cmdDeleteBookmark(const char* szName)
{
	return _deleteBookmark(szName, true);
}

UT_Error FV_View::cmdDeleteHyperlink()
{
	PT_DocPosition pos = getPoint();
	UT_DEBUGMSG(("fv_View::cmdDeleteHyperlink: pos %d\n", pos));
	return _deleteHyperlink(pos,true);
}


UT_Error FV_View::cmdHyperlinkStatusBar(UT_sint32 xPos, UT_sint32 yPos)
{
	UT_sint32 xClick, yClick;
	fp_Page* pPage = _getPageForXY(xPos, yPos, xClick, yClick);

	PT_DocPosition pos;
	bool bBOL = false;
	bool bEOL = false;
	bool isTOC = false;
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL,isTOC);

	// now get the run at the position and the hyperlink run
	fp_HyperlinkRun * pH1 = 0;

	fl_BlockLayout *pBlock = _findBlockAtPosition(pos);
	PT_DocPosition curPos = pos - pBlock->getPosition(false);

	fp_Run * pRun = pBlock->getFirstRun();

	//find the run at pos1
	while(pRun && pRun->getBlockOffset() <= curPos)
		pRun = pRun->getNextRun();

	// this sometimes happens, not sure why
	//UT_ASSERT(pRun);
	if(!pRun)
		return false;

	// now we have the run immediately after the run in question, so
	// we step back
	pRun = pRun->getPrevRun();
	UT_ASSERT(pRun);
	if(!pRun)
		return false;

	xxx_UT_DEBUGMSG(("fv_View::cmdHyperlinkStatusBar: run 0x%x, type %d\n", pRun,pRun->getType()));
	pH1 = pRun->getHyperlink();

	// this happens after a deletion of a hyperlink
	// the mouse processing is in the state of belief
	// that the processing has not finished yet -- this is not specific
	// to hyperlinks, it happens with anything on the context menu, except
	// it goes unobserved since the cursor does not change
	//UT_ASSERT(pH1);
	if(!pH1)
		return false;
	xxx_UT_DEBUGMSG(("fv_View::cmdHyperlinkStatusBar: msg [%s]\n",pH1->getTarget()));
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (getParentData());
	pFrame->setStatusMessage(pH1->getTarget());
	return true;
}


UT_Error FV_View::cmdInsertHyperlink(const char * szName)
{
	bool bRet;

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;
	PT_DocPosition iPointOrig = posStart;
	PT_DocPosition iAnchorOrig = m_Selection.getSelectionAnchor();

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
		{
			posStart = m_Selection.getSelectionAnchor();
		}
		else
		{
			posEnd = m_Selection.getSelectionAnchor();
		}

	}
	else
	{
		//No selection
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
		UT_ASSERT((pFrame));

		pFrame->showMessageBox(AP_STRING_ID_MSG_HyperlinkNoSelection, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return false;
	}

	bool relLink = false;
	if (!UT_isUrl(szName))
		relLink = m_pDoc->isBookmarkRelativeLink(szName);
	// TODO: After strings freeze is lifted, we should
	// TODO: display a message if relLink is true but
	// TODO: szName does not stat.

	if(!UT_isUrl(szName) && m_pDoc->isBookmarkUnique(szName) && !relLink)
	{
		//No bookmark of that name in document, tell user.
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
		UT_ASSERT((pFrame));

		pFrame->showMessageBox(AP_STRING_ID_MSG_HyperlinkNoBookmark, 
				       XAP_Dialog_MessageBox::b_O, 
				       XAP_Dialog_MessageBox::a_OK, 
				       szName);
	}

	// Hack for bug 2940
	if (posStart == 1) posStart++;

	// the selection has to be within a single block
	// we could implement hyperlinks spaning arbitrary part of the document
	// but then we could not use <a href=> </a> in the output and
	// I see no obvious need for hyperlinks to span more than a single block
	fl_BlockLayout * pBl1 = _findBlockAtPosition(posStart);
	fl_BlockLayout * pBl2 = _findBlockAtPosition(posEnd);

	if(pBl1 != pBl2)
	{
		XAP_Frame * pFrame = static_cast<XAP_Frame *>(getParentData());
		UT_ASSERT((pFrame));

		pFrame->showMessageBox(AP_STRING_ID_MSG_HyperlinkCrossesBoundaries, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);

		return false;
	}

	// Silently fail (TODO: pop up message) if we try to nest hyperlinks.
	if (_getHyperlinkInRange(posStart, posEnd) != NULL)
		return false;

	XML_Char * pAttr[4];

	UT_uint32 target_len = UT_XML_strlen(szName);
	XML_Char * target  = new XML_Char[ target_len+ 2];

	if(UT_isUrl(szName) || relLink)
	{
		UT_XML_strncpy(target, target_len + 1, static_cast<const XML_Char*>(szName));
	}
	else
	{
		target[0] =  '#';
		UT_XML_strncpy(target + 1, target_len + 1, static_cast<const XML_Char*>(szName));
	}

	XML_Char target_l[]  = "xlink:href";
	pAttr [0] = &target_l[0];
	pAttr [1] = &target[0];
	pAttr [2] = 0;
	pAttr [3] = 0;

	UT_DEBUGMSG(("fv_View::cmdInsertHyperlink: target \"%s\"\n", target));

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	// we first insert the end run, so that we can use it as a stop
	// after inserting the start run when marking the runs in between
	// as a hyperlink
	bRet = m_pDoc->insertObject(posEnd, PTO_Hyperlink, NULL, NULL);

	if(bRet)
	{
		const XML_Char ** pAttrs = const_cast<const XML_Char **>(pAttr);
		const XML_Char ** pProps = 0;
		bRet = m_pDoc->insertObject(posStart, PTO_Hyperlink, pAttrs, pProps);
	}

	if(bRet)
	{
		// because we have inserted two objects around the selection
		// boundaries the original insetion point and selection anchor
		// are now shifted, so we need to fix them
		setPoint(iPointOrig+1);
		m_Selection.setSelectionAnchor(iAnchorOrig + 1);
	}

	delete [] target;

	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;

}

/******************************************************************/
UT_Error FV_View::cmdInsertBookmark(const char * szName)
{
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	bool bRet;

	PT_DocPosition posStart = getPoint();
	PT_DocPosition posEnd = posStart;

	if (!isSelectionEmpty())
	{
		if (m_Selection.getSelectionAnchor() < posStart)
		{
			posStart = m_Selection.getSelectionAnchor();
		}
		else
		{
			posEnd = m_Selection.getSelectionAnchor();
		}
	}

	posEnd++;

	if(!m_pDoc->isBookmarkUnique(static_cast<const XML_Char*>(szName)))
	{
		//bookmark already exists -- remove it and then reinsert
		UT_DEBUGMSG(("fv_View::cmdInsertBookmark: bookmark \"%s\" exists - removing\n", szName));
		_deleteBookmark(static_cast<const XML_Char*>(szName), false, &posStart, &posEnd);
	}

	XML_Char * pAttr[6];

	XML_Char name_l [] = "name";
	XML_Char type_l [] = "type";
	XML_Char name[BOOKMARK_NAME_SIZE + 1];
	UT_XML_strncpy(name, BOOKMARK_NAME_SIZE, static_cast<const XML_Char*>(szName));
	name[BOOKMARK_NAME_SIZE] = 0;

	XML_Char type[] = "start";
	pAttr [0] = &name_l[0];
	pAttr [1] = &name[0];
	pAttr [2] = &type_l[0];
	pAttr [3] = &type[0];
	pAttr [4] = 0;
	pAttr [5] = 0;

	UT_DEBUGMSG(("fv_View::cmdInsertBookmark: szName \"%s\"\n", szName));

	const XML_Char ** pAttrs = const_cast<const XML_Char **>(pAttr);
	const XML_Char ** pProps = 0;
	bRet = m_pDoc->insertObject(posStart, PTO_Bookmark, pAttrs, pProps);

	if(bRet)
	{
		UT_XML_strncpy(type, 3,static_cast<const XML_Char*>("end"));
		type[3] = 0;
		bRet = m_pDoc->insertObject(posEnd, PTO_Bookmark, pAttrs, pProps);
	}

	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;

}


/*****************************************************************/


UT_Error FV_View::cmdInsertTOC(void)
{
	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	m_pDoc->beginUserAtomicGlob();
	bool bRet = false; // was not initialised; since ret value is
					   // UT_Error, false should correspond to OK. Tomas


	if (!isSelectionEmpty())
	{
		_deleteSelection();
	}
//
// Close off the current block
//
	PT_DocPosition pos = getPoint(); 
	insertParagraphBreak();
//
// insert just before this block to make the TOC gets inserted just BEFORE
// the Block we just created.
//
// we want this sort of structure in the PT
//
// <Strux Block><Frag><frag><frag><TOC></TOC><Strux Block>|
//                                                        |
// Point is here after insert TOC-------------------------|
//
	m_pDoc->insertStrux(pos,PTX_SectionTOC);
	pos++;
	m_pDoc->insertStrux(pos,PTX_EndTOC);
	setPoint(getPoint()+1);
	m_pDoc->endUserAtomicGlob();
	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;

}


/*****************************************************************/
UT_Error FV_View::cmdInsertField(const char* szName, const XML_Char ** extra_attrs, const XML_Char ** extra_props)
{
	bool bResult;

/*
  currently unused
  fl_BlockLayout* pBL = _findBlockAtPosition(getPoint());
*/

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	_insertField(szName,  extra_attrs,extra_props);


	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	_generalUpdate();

	_fixInsertionPointCoords();
	if (!_ensureInsertionPointOnScreen())
	{
//
// Handle End of Paragraph case
//
		PT_DocPosition posEOD;
		getEditableBounds(true, posEOD);
		if(getPoint() == posEOD)
		{
			m_bPointEOL = true;
		}
		_fixInsertionPointCoords();
	}
	return bResult;
}

UT_Error FV_View::cmdInsertGraphic(FG_Graphic* pFG, const char* pszName)
{
	bool bDidGlob = false;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if (!isSelectionEmpty())
	{
		bDidGlob = true;
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
	}

	/*
	  First, find a unique name for the data item.
	*/
	char *szName = new char [strlen (pszName) + 64 + 1];
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

	_restorePieceTableState();
	_updateInsertionPoint();

	delete [] szName;

	return errorCode;
}

/*!
 * This method inserts an image at the strux of type iStruxType at the 
 * point given by ipos.
 * This is useful for speficifying images as backgrounds to pages and cells.
 */
UT_Error FV_View::cmdInsertGraphicAtStrux(FG_Graphic* pFG, const char* pszName, PT_DocPosition iPos, PTStruxType iStruxType)
{
	bool bDidGlob = false;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	/*
	  First, find a unique name for the data item.
	*/
	char *szName = new char [strlen (pszName) + 64 + 1];
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

	UT_Error errorCode = pFG->insertAtStrux(m_pDoc, 
											m_pG->getDeviceResolution(),
											iPos,
											iStruxType,szName);
	if (bDidGlob)
		m_pDoc->endUserAtomicGlob();

	_generalUpdate();

	_restorePieceTableState();
	_updateInsertionPoint();

	delete [] szName;

	return errorCode;
}

void FV_View::cmdContextSuggest(UT_uint32 ndx, fl_BlockLayout * ppBL,
								fl_PartOfBlock * ppPOB)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL;
	fl_PartOfBlock* pPOB;

	if (!ppBL)
		pBL = _findBlockAtPosition(pos);
	else
		pBL = ppBL;
	UT_ASSERT(pBL);

	if (!ppPOB)
		pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	else
		pPOB = ppPOB;
	UT_ASSERT(pPOB);

	// grab the suggestion
	UT_UCSChar * replace = _lookupSuggestion(pBL, pPOB, ndx);

	if (!replace)
		return;

	// make the change
	UT_ASSERT(isSelectionEmpty());

	moveInsPtTo(static_cast<PT_DocPosition>(pBL->getPosition() + pPOB->getOffset()));
	extSelHorizontal(true, pPOB->getLength());

	UT_UCSChar * selection;
	getSelectionText(selection);
	getDictForSelection ()->correctWord (selection, UT_UCS4_strlen (selection),
										 replace, UT_UCS4_strlen (replace));
	cmdCharInsert(replace, UT_UCS4_strlen(replace));
	FREEP(selection);
	FREEP(replace);
}

void FV_View::cmdContextIgnoreAll(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(pPOB->getOffset()));

	// make the change
	getDictForSelection ()->ignoreWord ((const UT_UCSChar *)pBuf, (size_t)pPOB->getLength());
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
				b = static_cast<fl_BlockLayout *>(b->getNext());
			}
			pSL = static_cast<fl_DocSectionLayout *>(pSL->getNext());
		}
	}
}

void FV_View::cmdContextAdd(void)
{
	// locate the squiggle
	PT_DocPosition pos = getPoint();
	fl_BlockLayout* pBL = _findBlockAtPosition(pos);
	UT_ASSERT(pBL);
	fl_PartOfBlock* pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = reinterpret_cast<UT_UCSChar*>(pgb.getPointer(pPOB->getOffset()));

	// make the change
	if (getDictForSelection ()->addToCustomDict (pBuf, pPOB->getLength()))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = static_cast<fl_BlockLayout *>(pSL->getFirstLayout());
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				if(b->getContainerType() == FL_CONTAINER_BLOCK)
				{
					m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
				}
				b = static_cast<fl_BlockLayout *>(b->getNext());
			}
			pSL = static_cast<fl_DocSectionLayout *>(pSL->getNext());
		}
	}
}



/*!
 * Remove all the Headers or footers from the section owning the current Page.
\params bool isHeader remove the header if true, the footer if false.
*/
void FV_View::cmdRemoveHdrFtr( bool isHeader)
{
//
// Branch to Header/Footer sections.
//
	fp_ShadowContainer * pHFCon = NULL;
	fl_HdrFtrShadow * pShadow = NULL;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;

	if(isHeader)
	{
		fp_Page * pPage = getCurrentPage();
		pHFCon = pPage->getHdrFtrP(FL_HDRFTR_HEADER);
		if(pHFCon == NULL)
		{
			return;
		}
//
// Now see if we are in the header to be removed. If so, jump out.
//
		if (!isSelectionEmpty())
			_clearSelection();
		if(isHdrFtrEdit())
		{
			clearHdrFtrEdit();
			_setPoint(pPage->getFirstLastPos(true));
		}
	}
	else
	{
		fp_Page * pPage = getCurrentPage();
		pHFCon = pPage->getHdrFtrP(FL_HDRFTR_FOOTER);
		if(pHFCon == NULL)
		{
			return;
		}
//
// Now see if we are in the Footer to be removed. If so, jump out.
//
		if (!isSelectionEmpty())
			_clearSelection();
		if(isHdrFtrEdit())
		{
			clearHdrFtrEdit();
			_setPoint(pPage->getFirstLastPos(false));
		}
	}
	pShadow = pHFCon->getShadow();
	UT_ASSERT(pShadow);
	if(!pShadow)
		return;

	m_pDoc->beginUserAtomicGlob();

	_saveAndNotifyPieceTableChange();
//
// Save current document position.
//
	PT_DocPosition curPoint = getPoint();
//
// Get the hdrftrSectionLayout
// Get it's position.
// Find the last run in the Section and get it's position.
//
// Need code here to remove all the header/footers.
//
	pHdrFtr = pShadow->getHdrFtrSectionLayout();
//
// Repeat this code 4 times to remove all the DocSection Layouts.
//
	setCursorWait();
	_removeThisHdrFtr(pHdrFtr);
//
// After erarsing the cursor, Restore to the point before all this mess started.
//
	_setPoint(curPoint);

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();
	updateScreen (); // fix 1803, force screen update/redraw

	_updateInsertionPoint();
	m_pDoc->endUserAtomicGlob();
	clearCursorWait();
}

/*!
 * Start edit header mode. If there is no header one will be inserted.
 * otherwise start editing the header on the current page.
 */
void FV_View::cmdEditHeader(void)
{
	_cmdEditHdrFtr(FL_HDRFTR_HEADER);
	notifyListeners (AV_CHG_HDRFTR | AV_CHG_FMTSECTION);
}

/*!
 * Start edit footer mode. If there is no footer one will be inserted.
 * otherwise start editing the footer on the current page.
 */
void FV_View::cmdEditFooter(void)
{
	_cmdEditHdrFtr(FL_HDRFTR_FOOTER);
	notifyListeners (AV_CHG_HDRFTR | AV_CHG_FMTSECTION);
}

void FV_View::cmdAcceptRejectRevision(bool bReject, UT_sint32 xPos, UT_sint32 yPos)
{
	UT_DEBUGMSG(( "FV_View::cmdAcceptRejectRevision [bReject=%d]\n",bReject ));

	PT_DocPosition iStart, iEnd;
	fl_BlockLayout * pBlock = NULL;
	fp_Run *pRun = NULL;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	if(isSelectionEmpty())
	{
		if(xPos || yPos) // if given 0,0 use current position
		{
			warpInsPtToXY(xPos, yPos,true);
		}

		pBlock = getCurrentBlock();
		PT_DocPosition iRelPos = getPoint() - pBlock->getPosition(false);

		pRun = pBlock->getFirstRun();
		while (pRun && pRun->getNextRun() && pRun->getBlockOffset()+ pRun->getLength() <= iRelPos)
			pRun= pRun->getNextRun();

		UT_return_if_fail(pRun);

		iStart = pBlock->getPosition(false) + pRun->getBlockOffset();
		iEnd = pBlock->getPosition(false) + pRun->getBlockOffset() + pRun->getLength();

	}
	else
	{
		iStart = getPoint();
		iEnd   = getSelectionAnchor();
	}

	// remove the selection, since things will get inserted, deleted, etc.
	_clearSelection();
	
	m_pDoc->acceptRejectRevision(bReject,iStart,iEnd,m_iViewRevision);

	_generalUpdate();
	_restorePieceTableState();
}

void FV_View::cmdSetRevisionLevel(UT_uint32 i)
{
	// first set the same level in Doc; we do this unconditionally,
	// this way the doc will always save the level the user last used
	// NB: the doc id and the view id can be differnt if the user
	// changed it in some other view
	m_pDoc->setShowRevisionId(i);

	if(m_iViewRevision != i)
	{
		m_iViewRevision = i;

		// need to rebuild the doc to reflect the new level ...
		m_pLayout->rebuildFromHere(static_cast<fl_DocSectionLayout *>(m_pLayout->getFirstSection()));
	}
}

/*!
    finds the next/previous revision and sets selection to it
    TODO the selection will not cross block boundaries; it probably should

    \param bNext: if true the search is carried out in forward direction
    \return returns true on succes
*/
bool FV_View::cmdFindRevision(bool bNext, UT_sint32 xPos, UT_sint32 yPos)
{
	if(xPos || yPos)
	{
		// this is the case we were called from context menu ...
		warpInsPtToXY(xPos, yPos,true);
	}

	if(!isSelectionEmpty())
	{
		_moveToSelectionEnd(bNext);
	}
	
	fl_BlockLayout * pBL =	getCurrentBlock();

	if(!pBL)
		return false;
	
	fl_DocSectionLayout * pSL = pBL->getDocSectionLayout();

	if(!pSL)
		return false;
	
	fp_Run * pRun;
	UT_sint32 xPoint,yPoint,xPoint2,yPoint2,iPointHeight;
	bool bDirection;

	pRun = pBL->findPointCoords(getPoint(), false, xPoint,
								yPoint, xPoint2, yPoint2,
								iPointHeight, bDirection);

	if(!pRun)
		return false;

	if(bNext)
	{
		pRun = pRun->getNextRun();

		while(pSL)
		{
			while(pBL)
			{
				while(pRun)
				{
					if(pRun->containsRevisions() && !pRun->isHidden())
					{
						goto move_point;
					}

					pRun = pRun->getNextRun();
				}

				pBL = pBL->getNextBlockInDocument();
			}

			pSL = pSL->getNextDocSection();
		}
	}
	else
	{
		pRun = pRun->getPrevRun();

		while(pSL)
		{
			while(pBL)
			{
				while(pRun)
				{
					if(pRun->containsRevisions() && !pRun->isHidden())
					{
						goto move_point;
					}

					pRun = pRun->getPrevRun();
				}

				pBL = pBL->getPrevBlockInDocument();
			}

			pSL = pSL->getPrevDocSection();
		}
	}

	return false;
	
 move_point:
	UT_return_val_if_fail(pRun && pBL, false);

	// we want to span the selection not only over this run, but also
	// all subesequent runs that contain the same revions
	// TODO: probably should do this across block/section boundaries
	fp_Run * pRun2 = bNext ? pRun->getNextRun() : pRun->getPrevRun();
	fp_Run * pOldRun2 = pRun;

	PP_RevisionAttr * pR1 = pRun->getRevisions();
	
	while(pRun2)
	{
		if(pRun2->containsRevisions() && !pRun2->isHidden())
		{
			// test the two runs, if their revions are the same
			// include this one as well
			PP_RevisionAttr * pR2 = pRun2->getRevisions();

			if(!(*pR1 == *pR2))
				break;
		}
		else
		{
			break;
		}
		
		pOldRun2 = pRun2;
		pRun2 = bNext ? pRun2->getNextRun() : pRun2->getPrevRun();
	}

	// backtrack (we want pRun2 to be the last run in the selection
	pRun2 = pOldRun2;
	UT_return_val_if_fail(pRun2, false);
	
	PT_DocPosition dpos1, dpos2;

	if(bNext)
	{
		dpos1 = pBL->getPosition() + pRun->getBlockOffset();
		dpos2 = pRun2->getBlock()->getPosition() + pRun2->getBlockOffset() + pRun2->getLength();
	}
	else
	{
		dpos1 = pRun2->getBlock()->getPosition() + pRun2->getBlockOffset();
		dpos2 = pBL->getPosition() + pRun->getBlockOffset() + pRun->getLength();
	}
	
	cmdSelect(dpos1, dpos2);
	
	return true;
}

