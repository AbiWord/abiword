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
#include "fp_TableContainer.h"
#include "xap_EncodingManager.h"

#include "pp_Revision.h"
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
				notifyListeners(AV_CHG_MOTION);
				return;
			}
		}
	}
	notifyListeners(AV_CHG_MOTION);

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
// Now find the number of rows and columns inthis table. This is easiest to
// get from the table container
//
	fl_BlockLayout * pBL =	m_pLayout->findBlockAtPosition(posSource);
	fp_Run * pRun;
	UT_sint32 xPoint,yPoint,xPoint2,yPoint2,iPointHeight;
	bool bDirection;
	pRun = pBL->findPointCoords(posSource, false, xPoint,
							    yPoint, xPoint2, yPoint2,
							    iPointHeight, bDirection);

	UT_return_val_if_fail(pRun, false);

	fp_Line * pLine = pRun->getLine();
	UT_return_val_if_fail(pLine, false);

	fp_Container * pCon = pLine->getContainer();
	UT_return_val_if_fail(pCon, false);

	fp_TableContainer * pTab = (fp_TableContainer *) pCon->getContainer();
	UT_return_val_if_fail(pTab, false);

	UT_sint32 numRows = pTab->getNumRows();
	UT_sint32 numCols = pTab->getNumCols();
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
	PT_DocPosition posTable,posCell,posEndCell,posPrevCell;
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

	fp_TableContainer * pTab = (fp_TableContainer *) pCon->getContainer();
	UT_return_val_if_fail(pTab, false);

	UT_sint32 numRows = pTab->getNumRows();
	UT_sint32 numCols = pTab->getNumCols();
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
	pszTable[0] = "table-line-type";
	const char * szLineType = NULL;
	UT_String sLineType;
	UT_sint32 iLineType;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szLineType);
	if(szLineType == NULL || *szLineType == NULL)
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
//
// OK loop through all the rows in this column and insert the entries in the specified
// column if the cell spans just the width of the column..
//
	UT_sint32 i =0;
	UT_sint32 oldTop = -1;
	for(i=0; i <numRows; i++)
	{
		posCell = findCellPosAt(posTable,i,iLeft);
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
				if(iLeft > 0)
				{
					UT_String_sprintf(sLeft,"%d",iLeft-1);
					UT_String_sprintf(sRight,"%d",iLeft);
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
				bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
			}
			else
			{
				endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
				posEndCell = m_pDoc->getStruxPosition(endCellSDH);
				UT_String_sprintf(sLeft,"%d",iLeft+1);
				UT_String_sprintf(sRight,"%d",iLeft+2);
				props[4] = sColLeft.c_str();
				props[5] = sLeft.c_str();
				props[6] = sColRight.c_str();
				props[7] = sRight.c_str();
				UT_DEBUGMSG(("SEVIOR: Inserting cell strux %s %s %s %s %s %s %s %s \n",props[0],props[1],props[2],props[3],props[4],props[5],props[6],props[7]));
				bRes = m_pDoc->insertStrux(posEndCell+1,PTX_SectionCell,NULL,props);
				bRes = m_pDoc->insertStrux(posEndCell+2,PTX_Block);
				bRes = m_pDoc->insertStrux(posEndCell+3,PTX_EndCell);

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
// OK now loop backwards until we find a cell on this row.
//
				UT_sint32 j =0;
				for(j=iLeft; (j>= 0) && !bBBefore; j--)
				{
					posCell = findCellPosAt(posCol,i,j);
					getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
					if(jTop == i)
					{ 
						bBBefore = true;
						if(iLeft > 0)
						{
							UT_String_sprintf(sLeft,"%d",iLeft-1);
							UT_String_sprintf(sRight,"%d",iLeft);
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
						bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
					}
				}
//
// No cell before on this row. Now look after this cell.
//
				bool bAfter = false;
				if(!bBBefore)
				{
					for(j=iLeft+1; (j< numCols) && !bAfter; j++)
					{
						posCell = findCellPosAt(posCol,i,j);
						getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
						if(jTop == i)
						{
							bAfter = true;
							endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
							posEndCell = m_pDoc->getStruxPosition(endCellSDH);
							UT_String_sprintf(sLeft,"%d",iLeft+1);
							UT_String_sprintf(sRight,"%d",iLeft+2);
							props[4] = sColLeft.c_str();
							props[5] = sLeft.c_str();
							props[6] = sColRight.c_str();
							props[7] = sRight.c_str();
							bRes = m_pDoc->insertStrux(posEndCell+1,PTX_SectionCell,NULL,props);
							bRes = m_pDoc->insertStrux(posEndCell+2,PTX_Block);
							bRes = m_pDoc->insertStrux(posEndCell+3,PTX_EndCell);
						}
					}
				}
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
				for(j=iLeft+1; (j< numCols) && !bAfter; j++)
				{
					posCell = findCellPosAt(posCol,i,j);
					getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
					if(jTop == i)
					{
						bAfter = true;
						endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
						posEndCell = m_pDoc->getStruxPosition(endCellSDH);
						UT_String_sprintf(sLeft,"%d",iLeft+1);
						UT_String_sprintf(sRight,"%d",iLeft+2);
						props[4] = sColLeft.c_str();
						props[5] = sLeft.c_str();
						props[6] = sColRight.c_str();
						props[7] = sRight.c_str();
						bRes = m_pDoc->insertStrux(posEndCell+1,PTX_SectionCell,NULL,props);
						bRes = m_pDoc->insertStrux(posEndCell+2,PTX_Block);
						bRes = m_pDoc->insertStrux(posEndCell+3,PTX_EndCell);
					}
				}
//
// OK now loop backwards until we find a cell on this row.
//
				if(!bAfter)
				{
					UT_sint32 j =0;
					for(j=iLeft; (j>= 0) && !bBBefore; j--)
					{
						posCell = findCellPosAt(posCol,i,j);
						getCellParams(posCell+1,&jLeft,&jRight,&jTop,&jBot);
						if(jTop == i)
						{ 
							bBBefore = true;
							if(iLeft > 0)
							{
								UT_String_sprintf(sLeft,"%d",iLeft-1);
								UT_String_sprintf(sRight,"%d",iLeft);
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
							bRes = m_pDoc->insertStrux(posCell+2,PTX_EndCell);
						}
					}
				}
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
	PT_DocPosition posEndTable = m_pDoc->getStruxPosition(endTableSDH);
	bool bEnd = false;
	UT_sint32 iCurLeft,iCurRight,iCurTop,iCurBot,iNewLeft,iNewRight;
	UT_sint32 iPrevLeft,iPrevRight,iPrevTop,iPrevBot;
	cellSDH = tableSDH;
	bool bFirst = true;
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
		if(posEndCell+1 >= posEndTable)
		{
			bEnd = true;
		}
		posCell =  m_pDoc->getStruxPosition(cellSDH);
		getCellParams(posCell+1, &iCurLeft, &iCurRight,&iCurTop,&iCurBot);
//
// OK after inserting there will be two struxes with the same left-attach. We want to fix up 
// the second of these.
//
		bool bChange = false;
		iNewLeft = iCurLeft;
		iNewRight = iCurRight;
		if(!bFirst)
		{
			bRes = m_pDoc->getPrevStruxOfType(cellSDH,PTX_SectionCell,&prevCellSDH);
			posPrevCell =  m_pDoc->getStruxPosition(prevCellSDH);
			getCellParams(posPrevCell+1, &iPrevLeft, &iPrevRight,&iPrevTop,&iPrevBot);
			if(iPrevLeft == iCurLeft)
			{
				bChange = true;
				iNewLeft++;
				iNewRight++;
			}
		}
		UT_DEBUGMSG(("SEVIOR: Looking at cell left %d right %d top %d bot %d \n",iCurLeft,iCurRight,iCurTop,iCurBot));
		if(!bChange)
		{
			if(iCurLeft > iLeft+1)
			{
				bChange = true;
				iNewLeft++;
			}
			if(iCurRight > iLeft+2)
			{
				bChange = true;
				iNewRight++;
			}
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
			UT_DEBUGMSG(("SEVIOR: Changing cell strux %s %s %s %s %s %s %s %s \n",props[0],props[1],props[2],props[3],props[4],props[5],props[6],props[7]));
			bRes = m_pDoc->changeStruxFmt(PTC_AddFmt,posCell+1,posCell+1,NULL,props,PTX_SectionCell);
		}
		bFirst= false;
	}
//
// Now trigger a rebuild of the whole table by sending a changeStrux to the table strux
// with the restored line-type property it has before.
//
	iLineType += 1;
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
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_ensureInsertionPointOnScreen();
	return true;
}


bool FV_View::cmdInsertRow(PT_DocPosition posRow, bool bBefore)
{
	PL_StruxDocHandle cellSDH,tableSDH,endTableSDH,endCellSDH;
	PT_DocPosition posTable,posCell,posEndCell;
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
  
	fp_TableContainer * pTab = (fp_TableContainer *) pCon->getContainer();
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
	pszTable[0] = "table-line-type";
	const char * szLineType = NULL;
	UT_String sLineType;
	UT_sint32 iLineType;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szLineType);
	if(szLineType == NULL || *szLineType == NULL)
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
		posCell =  findCellPosAt(posTable,Top,0);
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
				prevTop = Bot;
			}
		}
//
// We insert at poscell position
//
		if(prevTop > numRows -1)
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
			posCell =  findCellPosAt(posTable,prevTop,0);
		}
		UT_DEBUGMSG(("SEVIOR: Inserting after row %d \n",prevTop));
	}		
//
// Got position and top row pos
// Now insert a row.
// 
	PT_DocPosition posInsert = 0;
	UT_sint32 i =0;
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
		if(posEndCell+1 >= posEndTable)
		{
			bEnd = true;
		}
		posCell =  m_pDoc->getStruxPosition(cellSDH);
		getCellParams(posCell+1, &iCurLeft, &iCurRight,&iCurTop,&iCurBot);
		iCurTop++;
		iCurBot++;
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
    iLineType += 1;
    UT_String_sprintf(sLineType,"%d",iLineType);
    pszTable[1] = sLineType.c_str();
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

	fp_TableContainer * pTab = (fp_TableContainer *) pCon->getContainer();
	UT_return_val_if_fail(pTab, false);

	UT_sint32 numRows = pTab->getNumRows();
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
	pszTable[0] = "table-line-type";
	const char * szLineType = NULL;
	UT_String sLineType;
	UT_sint32 iLineType;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szLineType);
	if(szLineType == NULL || *szLineType == NULL)
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
	iLineType += 1;
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
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

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

	fp_TableContainer * pTab = (fp_TableContainer *) pCon->getContainer();
	UT_return_val_if_fail(pTab, false);

	UT_sint32 numCols = pTab->getNumCols();
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
	pszTable[0] = "table-line-type";
	const char * szLineType = NULL;
	UT_String sLineType;
	UT_sint32 iLineType;
	m_pDoc->getPropertyFromSDH(tableSDH,pszTable[0],&szLineType);
	if(szLineType == NULL || *szLineType == NULL)
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
	iLineType += 1;
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
	_generalUpdate();


	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

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
	UT_Error e;

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
	setPoint(pointBreak);
	e |= (UT_sint32)(m_pDoc->insertStrux(getPoint(),PTX_SectionTable,NULL,pPropsArray));
//
// stuff for cell insertion.
//
	UT_sint32 i,j;
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
			e |= (UT_sint32)(m_pDoc->insertStrux(getPoint(),PTX_SectionCell,NULL,props));
			pointBreak = getPoint();
			e |= (UT_sint32)(m_pDoc->insertStrux(getPoint(),PTX_Block));
			UT_DEBUGMSG(("SEVIOR: 4  cur point %d \n",getPoint()));
			if(getPoint() == pointBreak)
			{
				setPoint(pointBreak+1);
			}
			if(i == 0 && j==0)
			{
				pointTable = getPoint();
			}
			e |= (UT_sint32)(m_pDoc->insertStrux(getPoint(),PTX_EndCell));
		}
	}
	e |= (UT_sint32)(m_pDoc->insertStrux(getPoint(),PTX_EndTable));
	setPoint(pointTable);
	m_pDoc->endUserAtomicGlob();
	m_pDoc->setDontImmediatelyLayout(false);
	_generalUpdate();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

	_ensureInsertionPointOnScreen();
	return e;
}

bool FV_View::cmdCharInsert(UT_UCSChar * text, UT_uint32 count, bool bForce)
{
	bool bResult = true;
	// So this gets rid of the annoying cursor flash at the beginning
	// of the line upon character insertion, but it's the wrong thing to
	// do.  The right thing to do is to either delay calculation, or to
	// not make the wrong number come up; disabling the caret is wrong. -PL
 	// GR_CaretDisabler caretDisabler(m_pG->getCaret());

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

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
		bool bOverwrite = (!m_bInsertMode && !bForce);

		if (bOverwrite)
		{
			// we need to glob when overwriting
			m_pDoc->beginUserAtomicGlob();
			cmdCharDelete(true,count);
		}
		bool doInsert = true;
		if(text[0] == UCS_TAB && count == 1)
		{
			//
			// Were inserting a TAB. Handle special case of TAB
			// right after a list-label combo
			//
			if((isTabListBehindPoint() == true || isTabListAheadPoint() == true) && getCurrentBlock()->isFirstInList() == false)
			{
				//
				// OK now start a sublist of the same type as the
				// current list if the list type is of numbered type
				fl_BlockLayout * pBlock = getCurrentBlock();
				List_Type curType = pBlock->getListType();
//
// Now increase list level for bullet lists too
//
				{
					UT_uint32 curlevel = pBlock->getLevel();
					UT_uint32 currID = pBlock->getAutoNum()->getID();
					curlevel++;
					fl_AutoNum * pAuto = pBlock->getAutoNum();
					const XML_Char * pszAlign = pBlock->getProperty("margin-left",true);
					const XML_Char * pszIndent = pBlock->getProperty("text-indent",true);
					const XML_Char * pszFieldF = pBlock->getProperty("field-font",true);
					float fAlign = (float)atof(pszAlign);
					float fIndent = (float)atof(pszIndent);
//
// Convert pixels to inches.
//
					float maxWidthIN = (float)(((float) pBlock->getFirstContainer()->getContainer()->getWidth())/100. -0.6);
					if(fAlign + (float) LIST_DEFAULT_INDENT < maxWidthIN)
					{
						fAlign += (float) LIST_DEFAULT_INDENT;
					}
					pBlock->StartList(curType,pAuto->getStartValue32(),pAuto->getDelim(),pAuto->getDecimal(),pszFieldF,fAlign,fIndent, currID,curlevel);
					doInsert = false;
				}
			}
		}
		if (doInsert == true)
		{
			bResult = m_pDoc->insertSpan(getPoint(), text, count, NULL);

			if(!bResult)
			{
				const fl_BlockLayout * pBL = getCurrentBlock();
				const PP_AttrProp *pBlockAP = NULL;
				pBL->getAttrProp(&pBlockAP);
				bResult = m_pDoc->insertSpan(getPoint(), text, count,const_cast<PP_AttrProp *>(pBlockAP));
				UT_ASSERT(bResult);
			}
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
	_restorePieceTableState();

	_ensureInsertionPointOnScreen();
	return bResult;
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
		// Code to deal with font boundary problem.
		// TODO: This should really be fixed by someone who understands
		// how this code works! In the meantime save current font to be
		// restored after character is deleted.

		getCharFormat(&props_in);
		currentfont = UT_getAttribute("font-family",props_in);
		properties[1] = currentfont;

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

		_ensureInsertionPointOnScreen();

		//special handling is required for delete in revisions mode
		//where we have to move the insertion point
		if(isMarkRevisions())
		{
			UT_ASSERT( iRealDeleteCount <= count );
			_charMotion(bForward,count - iRealDeleteCount);
		}
	}


	// Signal PieceTable Changes have finished
	_restorePieceTableState();
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
	{
		lineHeight = HACK_LINE_HEIGHT;
	}

	UT_sint32 yoff = m_yScrollOffset;
	UT_sint32 xoff = m_xScrollOffset;

	switch(cmd)
	{
	case AV_SCROLLCMD_PAGEDOWN:
		yoff += m_iWindowHeight - HACK_LINE_HEIGHT;
		bVertical = true;
		break;
	case AV_SCROLLCMD_PAGEUP:
		yoff -= m_iWindowHeight - HACK_LINE_HEIGHT;
		bVertical = true;
		break;
	case AV_SCROLLCMD_PAGELEFT:
		xoff -= m_iWindowWidth;
		bHorizontal = true;
		break;
	case AV_SCROLLCMD_PAGERIGHT:
		xoff += m_iWindowWidth;
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
	{
		yoff = 0;
	}

	bool bRedrawPoint = true;

	if (bVertical && (yoff != m_yScrollOffset))
	{
		sendVerticalScrollEvent(_UD(yoff));
		if ((cmd != AV_SCROLLCMD_PAGEUP
			 && cmd != AV_SCROLLCMD_PAGEDOWN))
		  bRedrawPoint = false;
	}

	if (xoff < 0)
	{
		xoff = 0;
	}

	if (bHorizontal && (xoff != m_xScrollOffset))
	{
		sendHorizontalScrollEvent(_UD(xoff));
		bRedrawPoint = false;
	}

	if (bRedrawPoint)
	{
		_fixInsertionPointCoords();
	}


}


void FV_View::cmdSelect(PT_DocPosition dpBeg, PT_DocPosition dpEnd)
{

	if (!isSelectionEmpty())
	{
		_clearSelection();
	}

	_setPoint(dpBeg);
	_setSelectionAnchor();
	m_iSelectionLeftAnchor = dpBeg;
	m_iSelectionRightAnchor = dpEnd;

	_setPoint (dpEnd);

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
			if(pLine == (fp_Line *) pBlock->getFirstContainer())
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
		pRun= pRun->getNext();

	UT_ASSERT(pRun);
	pRun->getPrev();

	UT_ASSERT(pRun);
#if 0
	if(pRun->getType()== FPRUN_FMTMARK || pRun->getType()== FPRUN_HYPERLINK || pRun->getType()== FPRUN_BOOKMARK)
		pRun  = pRun->getNext();

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

	_updateInsertionPoint();
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

UT_Error FV_View::cmdSaveAs(const char * szFilename, int ieft, bool cpy)
{
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
	// Signal PieceTable Change
	m_pDoc->notifyPieceTableChangeStart();

	//
	// Disable list updates until after we've finished
	//
	m_pDoc->disableListUpdates();
	cmdCopy(true);
	_deleteSelection();

	// restore updates and clean up dirty lists
	m_pDoc->enableListUpdates();
	m_pDoc->updateDirtyLists();


	// Signal PieceTable Changes have finished
	m_pDoc->notifyPieceTableChangeEnd();
	m_iPieceTableState = 0;

	_generalUpdate();


	_fixInsertionPointCoords();
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

	m_pDoc->endUserAtomicGlob();
}

UT_Error FV_View::cmdDeleteBookmark(const char* szName)
{
	PT_DocPosition i,j;
	return _deleteBookmark(szName,true,i,j);
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
	pPage->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL);

	// now get the run at the position and the hyperlink run
	fp_HyperlinkRun * pH1 = 0;

	fl_BlockLayout *pBlock = _findBlockAtPosition(pos);
	PT_DocPosition curPos = pos - pBlock->getPosition(false);

	fp_Run * pRun = pBlock->getFirstRun();

	//find the run at pos1
	while(pRun && pRun->getBlockOffset() <= curPos)
		pRun = pRun->getNext();

	// this sometimes happens, not sure why
	//UT_ASSERT(pRun);
	if(!pRun)
		return false;

	// now we have the run immediately after the run in question, so
	// we step back
	pRun = pRun->getPrev();
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
	PT_DocPosition iAnchorOrig = m_iSelectionAnchor;

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
	else
	{
		//No selection
		XAP_Frame * pFrame = (XAP_Frame *) getParentData();
		UT_ASSERT((pFrame));

		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_HyperlinkNoSelection, m_pApp->getDefaultEncoding()).c_str();

		UT_ASSERT(pMsg1);

		pFrame->showMessageBox(pMsg1, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
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
		XAP_Frame * pFrame = (XAP_Frame *) getParentData();
		UT_ASSERT((pFrame));

		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_HyperlinkNoBookmark, m_pApp->getDefaultEncoding()).c_str();
		UT_ASSERT(pMsg1);

		char * szMsg = new char[strlen(pMsg1)+strlen(szName)+1];
		UT_ASSERT(szMsg);
		sprintf(szMsg,pMsg1,szName);

		pFrame->showMessageBox(szMsg, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		delete[] szMsg;
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
		XAP_Frame * pFrame = (XAP_Frame *) getParentData();
		UT_ASSERT((pFrame));

		const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
		const char *pMsg1 = pSS->getValue(AP_STRING_ID_MSG_HyperlinkCrossesBoundaries, m_pApp->getDefaultEncoding()).c_str();
		UT_ASSERT(pMsg1);

		pFrame->showMessageBox(pMsg1, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);

		return false;
	}

	// Silently fail (TODO: pop up message) if we try to nest hyperlinks.
	if (_getHyperlinkInRange(posStart, posEnd) != NULL)
		return false;

	XML_Char * pAttr[4];
	const XML_Char ** pAt = (const XML_Char **)&pAttr[0];

	UT_uint32 target_len = UT_XML_strlen(szName);
	XML_Char * target  = new XML_Char[ target_len+ 2];

	if(UT_isUrl(szName) || relLink)
	{
		UT_XML_strncpy(target, target_len + 1, (XML_Char*)szName);
	}
	else
	{
		target[0] =  '#';
		UT_XML_strncpy(target + 1, target_len + 1, (XML_Char*)szName);
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
		bRet = m_pDoc->insertObject(posStart, PTO_Hyperlink, pAt, NULL);
	}

	if(bRet)
	{
		// because we have inserted two objects around the selection
		// boundaries the original insetion point and selection anchor
		// are now shifted, so we need to fix them
		setPoint(iPointOrig+1);
		m_iSelectionAnchor = iAnchorOrig + 1;
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
	PT_DocPosition pos1 = 0xFFFFFFFF,pos2 = 0xFFFFFFFF;

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

	posEnd++;

	if(!m_pDoc->isBookmarkUnique((XML_Char*)szName))
	{
		//bookmark already exists -- remove it and then reinsert
		UT_DEBUGMSG(("fv_View::cmdInsertBookmark: bookmark \"%s\" exists - removing\n", szName));
		_deleteBookmark((XML_Char*)szName, false,pos1,pos2);
	}


	// if the bookmark we just deleted was before the current insertion
	// position we have to adjust our positions correspondingly
	if(posStart > pos1)
		posStart--;
	if(posStart > pos2)
		posStart--;
	if(posEnd > pos1)
		posEnd--;
	if(posEnd > pos2)
		posEnd--;

	XML_Char * pAttr[6];
	const XML_Char ** pAt = (const XML_Char **)&pAttr[0];

	XML_Char name_l [] = "name";
	XML_Char type_l [] = "type";
	XML_Char name[BOOKMARK_NAME_SIZE + 1];
	UT_XML_strncpy(name, BOOKMARK_NAME_SIZE, (XML_Char*)szName);
	name[BOOKMARK_NAME_SIZE] = 0;

	XML_Char type[] = "start";
	pAttr [0] = &name_l[0];
	pAttr [1] = &name[0];
	pAttr [2] = &type_l[0];
	pAttr [3] = &type[0];
	pAttr [4] = 0;
	pAttr [5] = 0;

	UT_DEBUGMSG(("fv_View::cmdInsertBookmark: szName \"%s\"\n", szName));

	bRet = m_pDoc->insertObject(posStart, PTO_Bookmark, pAt, NULL);


	if(bRet)
	{
		UT_XML_strncpy(type, 3,(XML_Char*)"end");
		type[3] = 0;
		bRet = m_pDoc->insertObject(posEnd, PTO_Bookmark, pAt, NULL);
	}

	_generalUpdate();

	// Signal piceTable is stable again
	_restorePieceTableState();

	return bRet;

}


/*****************************************************************/
UT_Error FV_View::cmdInsertField(const char* szName, const XML_Char ** extra_attrs, const XML_Char ** extra_props)
{
	bool bResult;

	int attrCount = 0;
	while (extra_attrs && extra_attrs[attrCount] != NULL)
	{
		attrCount++;
	}

	const XML_Char ** attributes = new const XML_Char*[attrCount+4];

	int i = 0;
	while (extra_attrs && extra_attrs[i] != NULL)
	{
		attributes[i] = extra_attrs[i];
		i++;
	}
	attributes[i++] = "type";
	attributes[i++] = szName;
	attributes[i++] = NULL;
	attributes[i++] = NULL;

/*
  currently unused
  fl_BlockLayout* pBL = _findBlockAtPosition(getPoint());
*/

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();

	fd_Field * pField = NULL;
	if (!isSelectionEmpty())
	{
		m_pDoc->beginUserAtomicGlob();
		_deleteSelection();
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, extra_props,&pField);
		if(pField != NULL)
		{
			pField->update();
		}
		m_pDoc->endUserAtomicGlob();
	}
	else
	{
		bResult = m_pDoc->insertObject(getPoint(), PTO_Field, attributes, extra_props, &pField);
		if(pField != NULL)
		{
			pField->update();
		}
	}

	delete [] attributes;

	_generalUpdate();

	// Signal PieceTable Changes have finished
	_restorePieceTableState();

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

	moveInsPtTo((PT_DocPosition) (pBL->getPosition() + pPOB->getOffset()));
	extSelHorizontal(true, pPOB->getLength());
	cmdCharInsert(replace, UT_UCS4_strlen(replace));

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

	const UT_UCSChar * pBuf = (UT_UCSChar*)pgb.getPointer(pPOB->getOffset());

	// make the change
	if (m_pDoc->appendIgnore(pBuf, pPOB->getLength()))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = (fl_BlockLayout *) pSL->getFirstLayout();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
				b = (fl_BlockLayout *) b->getNext();
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
	fl_PartOfBlock* pPOB = pBL->getSquiggles()->get(pos - pBL->getPosition());
	UT_ASSERT(pPOB);

	// grab a copy of the word
	UT_GrowBuf pgb(1024);
	bool bRes = pBL->getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar * pBuf = (UT_UCSChar*)pgb.getPointer(pPOB->getOffset());

	// make the change
	if (m_pApp->addWordToDict(pBuf, pPOB->getLength()))
	{
		// remove the squiggles, too
		fl_DocSectionLayout * pSL = m_pLayout->getFirstSection();
		while (pSL)
		{
			fl_BlockLayout* b = (fl_BlockLayout *) pSL->getFirstLayout();
			while (b)
			{
				// TODO: just check and remove matching squiggles
				// for now, destructively recheck the whole thing
				if(b->getContainerType() == FL_CONTAINER_BLOCK)
				{
					m_pLayout->queueBlockForBackgroundCheck(FL_DocLayout::bgcrSpelling, b);
				}
				b = (fl_BlockLayout *) b->getNext();
			}
			pSL = (fl_DocSectionLayout *) pSL->getNext();
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
	fl_DocSectionLayout * pDSL = pHdrFtr->getDocSectionLayout();
//
// Repeat this code 4 times to remove all the DocSection Layouts.
//
	setCursorWait();
	if(isHeader)
	{
		_removeThisHdrFtr(pDSL->getHeaderFirst());
		_removeThisHdrFtr(pDSL->getHeaderLast());
		_removeThisHdrFtr(pDSL->getHeaderEven());
		_removeThisHdrFtr(pDSL->getHeader());
	}
	else
	{
		_removeThisHdrFtr(pDSL->getFooterFirst());
		_removeThisHdrFtr(pDSL->getFooterLast());
		_removeThisHdrFtr(pDSL->getFooterEven());
		_removeThisHdrFtr(pDSL->getFooter());
	}
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
}

/*!
 * Start edit footer mode. If there is no footer one will be inserted.
 * otherwise start editing the footer on the current page.
 */
void FV_View::cmdEditFooter(void)
{
	_cmdEditHdrFtr(FL_HDRFTR_FOOTER);
}

void FV_View::cmdAcceptRejectRevision(bool bReject, UT_sint32 xPos, UT_sint32 yPos)
{
	UT_DEBUGMSG(( "FV_View::cmdAcceptRejectRevision [bReject=%d]\n",bReject ));

	PT_DocPosition iStart, iEnd;
	fl_BlockLayout * pBlock = NULL;
	fp_Run *pRun = NULL;

	// Signal PieceTable Change
	_saveAndNotifyPieceTableChange();
	//m_pDoc->beginUserAtomicGlob();

	if(isSelectionEmpty())
	{
		if(xPos || yPos) // if given 0,0 use current position
		{
			warpInsPtToXY(xPos, yPos,true);
		}

		pBlock = getCurrentBlock();
		PT_DocPosition iRelPos = getPoint() - pBlock->getPosition(false);

		pRun = pBlock->getFirstRun();
		while (pRun && pRun->getNext() && pRun->getBlockOffset()+ pRun->getLength() <= iRelPos)
			pRun= pRun->getNext();

		UT_ASSERT(pRun);
		//fp_Run * pOrigRun = pRun;

		const PP_RevisionAttr * pRevAttr = pRun->getRevisions();
		iStart = pBlock->getPosition(false) + pRun->getBlockOffset();
		iEnd = pBlock->getPosition(false) + pRun->getBlockOffset() + pRun->getLength();

		_acceptRejectRevision(bReject,iStart,iEnd,pRevAttr);

#if 0
		// now we have the run we clicked on, next we need to work back
		// through the runs to find where the revision starts
		// this does not work -- the PT gets out of sync with the
		// layout; we can only do one run at at time
		while(pRun && pRun->containsRevisions())
		{
			// do any necessary processing to accept/reject the
		// revision
			const PP_RevisionAttr * pRevAttr = pRun->getRevisions();
			iStart = pBlock->getPosition(false) + pRun->getBlockOffset();
			iEnd = pBlock->getPosition(false) + pRun->getBlockOffset() + pRun->getLength();

			_acceptRejectRevision(bReject,iStart,iEnd,pRevAttr);

			pRun = pRun->getPrev();
		}

		// we have already processed the original run, so lets start
		// with the one after it
		pRun = pOrigRun->getNext();

		while(pRun && pRun->containsRevisions())
		{
			// do any necessary processing to accept/reject the revision
			const PP_RevisionAttr * pRevAttr = pRun->getRevisions();
			iStart = pBlock->getPosition(false) + pRun->getBlockOffset();
			iEnd = pBlock->getPosition(false) + pRun->getBlockOffset() + pRun->getLength();

			_acceptRejectRevision(bReject,iStart,iEnd,pRevAttr);

			pRun = pRun->getNext();
		}
#endif
	}
	else
	{
		// selection
		iStart = UT_MIN(getPoint(),m_iSelectionAnchor);
		iEnd   = UT_MAX(getPoint(),m_iSelectionAnchor);


	}

	// Signal PieceTable Changes have finished
	//m_pDoc->endUserAtomicGlob();
	_generalUpdate();
	_restorePieceTableState();
}

