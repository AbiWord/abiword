/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fv_Selection.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "ut_units.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "fl_TableLayout.h"
#include "pd_Document.h"
#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "fl_TOCLayout.h"
#include "ie_imp.h"
#include "ie_imp_RTF.h"

#include "ut_bytebuf.h"
#include "fv_View.h"

FV_Selection::FV_Selection (FV_View * pView)
	: m_pView (pView), 
	  m_iSelectionMode(FV_SelectionMode_NONE),
	  m_iPrevSelectionMode(FV_SelectionMode_NONE),
	  m_iSelectAnchor(0),
	  m_iSelectLeftAnchor(0),
	  m_iSelectRightAnchor(0),
	  m_pTableOfSelectedColumn(NULL),
	  m_pSelectedTOC(NULL),
	  m_bSelectAll(false),
	  m_iLeftTableRect(-1),
	  m_iRightTableRect(-1),
	  m_iTopTableRect(-1),
	  m_iBottomTableRect(-1)
{
	UT_ASSERT (pView);
	m_vecSelRanges.clear();
	m_vecSelRTFBuffers.clear();
}

FV_Selection::~FV_Selection()
{
	m_pTableOfSelectedColumn = NULL;
	m_pSelectedTOC = NULL;
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
	UT_VECTOR_PURGEALL(UT_ByteBuf  *,m_vecSelRTFBuffers);
	UT_VECTOR_PURGEALL(FV_SelectionCellProps *,m_vecSelCellProps);
}

void  FV_Selection::checkSelectAll(void)
{
	 fl_SectionLayout * pSL = m_pView->m_pLayout->getLastSection();
	 if(pSL == NULL)
	   return;
	 if(m_pView->m_pDoc->isPieceTableChanging())
	 {
	      return;
	 }
	 if(m_pView->m_pLayout->isLayoutFilling())
	 {
	      return;
	 }
	 PT_DocPosition posLow = m_iSelectAnchor;
	 PT_DocPosition posHigh = m_pView->getPoint();
	 if(posHigh < posLow)
	 {
	      posHigh = m_iSelectAnchor;
	      posLow = m_pView->getPoint();
	 }
	 PT_DocPosition posBeg,posEnd=0;
	 m_pView->getEditableBounds(false,posBeg);
	 m_pView->getEditableBounds(true,posEnd);
	 xxx_UT_DEBUGMSG(("posLow %d posBeg %d posHigh %d posEnd %d\n",posLow,posBeg,posHigh,posEnd));
	 bool bSelAll = ((posBeg >= posLow) && (posEnd == posHigh));
	 setSelectAll(bSelAll);
}

void  FV_Selection::setSelectAll(bool bSelectAll)
{
        xxx_UT_DEBUGMSG(("Select All = %d \n",bSelectAll));
	m_bSelectAll = bSelectAll;	      
}
void FV_Selection::setMode(FV_SelectionMode iSelMode)
{
    // WE SHOULD NO LONGER USE THE ROW AND COL MODES, ALL SHOULD BE IN_TABLE
    if( iSelMode == FV_SelectionMode_TableColumn || iSelMode == FV_SelectionMode_TableRow )
    {
        UT_DEBUGMSG(("USING A DEPRACATED SELECTION MODE!!! FIX THIS!!!"));
    }
    
    // shouldn't this be '&&' ? - dzan
	if( (m_iSelectionMode != FV_SelectionMode_NONE) || (iSelMode !=  FV_SelectionMode_NONE))
	{
		m_iPrevSelectionMode = m_iSelectionMode;
	}
	if((m_iSelectionMode == FV_SelectionMode_TOC) && (m_iSelectionMode != iSelMode))
	{
		if(m_pSelectedTOC)
		{
			m_pSelectedTOC->setSelected(false);
		}
		m_pSelectedTOC = NULL;
	}
	m_iSelectionMode = iSelMode;
	
	if(m_iSelectionMode == FV_SelectionMode_NONE)
	{
		m_pTableOfSelectedColumn = NULL;
		UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
		UT_VECTOR_PURGEALL(UT_ByteBuf  *,m_vecSelRTFBuffers);
		UT_VECTOR_PURGEALL(FV_SelectionCellProps *,m_vecSelCellProps);
		m_vecSelRanges.clear();
		m_vecSelRTFBuffers.clear();
		m_vecSelCellProps.clear();
		m_iLeftTableRect = -1;
	    m_iRightTableRect = -1;
	    m_iTopTableRect = -1;
	    m_iBottomTableRect = -1;
	}
	setSelectAll(false);
}

void FV_Selection::setTOCSelected(fl_TOCLayout * pTOCL)
{
	UT_return_if_fail(pTOCL);
	setMode(FV_SelectionMode_TOC);
	m_pSelectedTOC = pTOCL;
	m_iSelectAnchor = pTOCL->getPosition();
	pTOCL->setSelected(true);
	setSelectAll(false);
}

//
// dzan - This seems never to be called?!?
//
void FV_Selection::pasteRowOrCol(void)
{
	PL_StruxDocHandle cellSDH,tableSDH;
	PT_DocPosition pos = m_pView->getPoint();
	if(m_iPrevSelectionMode == FV_SelectionMode_TableColumn)
	{
//
// GLOB stuff together so it undo's in one go.
//
		getDoc()->beginUserAtomicGlob();
//
// Insert a column after the current column
//
		m_pView->cmdInsertCol(m_pView->getPoint(),false);
//
// Now do all the encapsulating stuff for piecetable manipulations.
//
	// Signal PieceTable Change
		m_pView->_saveAndNotifyPieceTableChange();

	// Turn off list updates

		getDoc()->disableListUpdates();
		if (!m_pView->isSelectionEmpty())
		{
			m_pView->_clearSelection();
		}
		getDoc()->setDontImmediatelyLayout(true);
		pos = m_pView->getPoint();
		PT_DocPosition posTable,posCell;
		UT_sint32 iLeft,iRight,iTop,iBot;
		posCell = 0;
		m_pView->getCellParams(pos, &iLeft, &iRight,&iTop,&iBot);
		bool bRes = getDoc()->getStruxOfTypeFromPosition(pos,PTX_SectionCell,&cellSDH);
		bRes = getDoc()->getStruxOfTypeFromPosition(pos,PTX_SectionTable,&tableSDH);
		UT_return_if_fail(bRes);
		posTable = getDoc()->getStruxPosition(tableSDH) + 1;
		UT_sint32 numRows = 0;
		UT_sint32 numCols = 0;
		UT_sint32 i = 0;
		getDoc()-> getRowsColsFromTableSDH(tableSDH, m_pView->isShowRevisions(), m_pView->getRevisionLevel(),
										   &numRows, &numCols);

		PD_DocumentRange DocRange(getDoc(),posCell,posCell);
		for(i=0; i<getNumSelections();i++)
		{
			posCell = m_pView->findCellPosAt(posTable,i,iLeft)+2;
			m_pView->setPoint(posCell);
			PD_DocumentRange * pR = getNthSelection(i);
			if(pR->m_pos1 == pR->m_pos2)
			{
//
// Dont paste empty cells
//
				continue;
			}
			UT_ByteBuf * pBuf = m_vecSelRTFBuffers.getNthItem(i);
			const unsigned char * pData = pBuf->getPointer(0);
			UT_uint32 iLen = pBuf->getLength();
			DocRange.m_pos1 = posCell;
			DocRange.m_pos2 = posCell;
			IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(getDoc());
			pImpRTF->pasteFromBuffer(&DocRange,pData,iLen);
			DELETEP(pImpRTF);
			fl_SectionLayout * pSL = m_pView->getCurrentBlock()->getSectionLayout();
			pSL->checkAndAdjustCellSize();
		}
		getDoc()->endUserAtomicGlob();
		getDoc()->setDontImmediatelyLayout(false);
		m_pView->_generalUpdate();


	// restore updates and clean up dirty lists
		getDoc()->enableListUpdates();
		getDoc()->updateDirtyLists();

	// Signal PieceTable Changes have finished
		m_pView->_restorePieceTableState();
// Put the insertion point in a legal position
//
		m_pView->notifyListeners(AV_CHG_MOTION);
		m_pView->_fixInsertionPointCoords();
		m_pView->_ensureInsertionPointOnScreen();

	}
	else
	{
	}
}

fl_TableLayout * FV_Selection::getTableLayout(void) const
{
	return m_pTableOfSelectedColumn;
}

PD_Document * FV_Selection::getDoc(void) const
{
	return m_pView->getDocument();
}

FL_DocLayout * FV_Selection::getLayout(void) const
{
	return m_pView->getLayout();
}

PT_DocPosition FV_Selection::getSelectionAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) ||  (m_vecSelRanges.getItemCount() == 0))
	{
		return m_iSelectAnchor;
	}
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(0);
	return pDocRange->m_pos1;
}

void FV_Selection::setSelectionAnchor(PT_DocPosition pos)
{
	 m_iSelectAnchor = pos;
	 fl_SectionLayout * pSL = m_pView->m_pLayout->getLastSection();
	 if(pSL == NULL)
	   return;
	 PT_DocPosition posLow = m_iSelectAnchor;
	 PT_DocPosition posHigh = m_pView->getPoint();
	 if(posHigh < posLow)
	 {
	      posHigh = m_iSelectAnchor;
	      posLow = m_pView->getPoint();
	 }
	 PT_DocPosition posBeg,posEnd=0;
	 m_pView->getEditableBounds(false,posBeg);
	 m_pView->getEditableBounds(true,posEnd);
	 xxx_UT_DEBUGMSG(("posLow %d posBeg %d posHigh %d posEnd %d\n",posLow,posBeg,posHigh,posEnd));
	 bool bSelAll = ((posBeg >= posLow) && (posEnd <= posHigh));
	 setSelectAll(bSelAll);
}


PT_DocPosition FV_Selection::getSelectionLeftAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) || (m_vecSelRanges.getItemCount() == 0))
	{
		return m_iSelectLeftAnchor;
	}
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(0);
	return pDocRange->m_pos1;
}

void FV_Selection::setSelectionLeftAnchor(PT_DocPosition pos)
{
        if(pos == 0)
	  return;
	m_iSelectLeftAnchor = pos;
	PT_DocPosition posBeg,posEnd=0;
	m_pView->getEditableBounds(false,posBeg);
	m_pView->getEditableBounds(true,posEnd);
	bool bSelAll = ((posBeg >= m_iSelectLeftAnchor) && (posEnd <= m_iSelectRightAnchor));
	 xxx_UT_DEBUGMSG(("setleft posBeg %d left %d posEnd %d right %d\n",posBeg,m_iSelectLeftAnchor,posEnd,m_iSelectRightAnchor));
	setSelectAll(bSelAll);
}


PT_DocPosition FV_Selection::getSelectionRightAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) || (m_vecSelRanges.getItemCount() == 0) )
	{
		return m_iSelectRightAnchor;
	}
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(0);
	return pDocRange->m_pos2;
}

void FV_Selection::setSelectionRightAnchor(PT_DocPosition pos)
{
        if(pos == 0)
	  return;
	m_iSelectRightAnchor = pos;
	PT_DocPosition posBeg,posEnd=0;
	m_pView->getEditableBounds(false,posBeg);
	m_pView->getEditableBounds(true,posEnd);
	bool bSelAll = ((posBeg >= m_iSelectLeftAnchor) && (posEnd <= m_iSelectRightAnchor));
	 xxx_UT_DEBUGMSG(("setRight posBeg %d left %d posEnd %d right %d\n",posBeg,m_iSelectLeftAnchor,posEnd,m_iSelectRightAnchor));
	setSelectAll(bSelAll);
}

/*! Return the current attach values of the table selection rectangle 
 *  \return False if none were set yet, else true.
 */
bool FV_Selection::getRectTableSel
(UT_sint32* left, UT_sint32* right, UT_sint32* top, UT_sint32* bottom)
{
    // if one isn't set they all aren't
    if( m_iLeftTableRect < 0 )
        return false;
        
    *left = m_iLeftTableRect;
	*right = m_iRightTableRect;
	*top = m_iTopTableRect;
	*bottom = m_iBottomTableRect;	
	return true;                               
}

/*! Set the attach values of the table selection rectangle
*   This won't add cells to a selection properly, one should use
    addCellToSelection(..) for that.
*/
void FV_Selection::setRectTableSel
(UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom)
{
    m_iLeftTableRect = left;
	m_iRightTableRect = right;
	m_iTopTableRect = top;
	m_iBottomTableRect = bottom;
}

/*! Return true if only a single row is selected */
bool FV_Selection::isSingleTableRowSelected(void) const
{
    return ( m_iTopTableRect == m_iBottomTableRect-1);
}

/*! Return true if only a single column is selected */
bool FV_Selection::isSingleTableColumnSelected(void) const
{
    return ( m_iLeftTableRect == m_iRightTableRect-1);
}
                                   
bool FV_Selection::isPosSelected(PT_DocPosition pos) const
{
	if(m_iSelectionMode == FV_SelectionMode_NONE)
	{
		return false;
	}
	if( m_iSelectionMode == FV_SelectionMode_InTable ){
	    // check if the given pos is in a selected cell
	    /*
	     * This could be done in many ways and probably faster ways too.
	     * Now I iterate over the vector, this could be avoided.
	     * This is temporarily, other options should be discussed.
	     * ^ One other way would be remembering the max of the attach values 
	     *   in AddCellTo and comparing these against the cell at the pos argument.
	     *   
	     * A binary search in the vector woud speed up too in big selections.
	     * An advantage of this would be the user is able to select multiple single cells.
	     *
	     * If this way would be kept we can just place the InTable selection mode
	     * after the Multiple mode and then we don't need this if anymore, the code
	     * in the bottom of this function would have the same effect.
	     */
	    /*UT_sint32 i =0;
	    PD_DocumentRange * pDocRange = NULL;
	    for(i=0; i < m_vecSelRanges.getItemCount(); i++){
	        pDocRange = m_vecSelRanges.getNthItem(i);           
	        if ((pos >= pDocRange->m_pos1) && (pos <= pDocRange->m_pos2+1)) 
	            return true;
	    }
	    // no cell found around the pos
	    return false;*/
	    
	    // due problem mentioned in mail I'm going to use attach values
	    
	    // if one rect border isn't set yet they all aren't
		if( m_iLeftTableRect < 0 || !m_pView->isInTable(pos) )
		    return false;
		  
		// make sure we are in the same table the anchor is in
		if( m_pView->getTableAtPos(getSelectionAnchor()) != 
		    m_pView->getTableAtPos(pos) )
		    return false;
		
	    UT_sint32 iLeft, iRight, iTop, iBot;
	    m_pView->getCellParams(pos,&iLeft,&iRight,&iTop,&iBot);
		
		// see if our cell is in the selection
	    if( iLeft >= m_iLeftTableRect &&
	        iRight <= m_iRightTableRect &&
	        iTop >= m_iTopTableRect &&
	        iBot <= m_iBottomTableRect )
	        return true;
	        
	    else
	        return false;
	}
	else if(m_iSelectionMode < FV_SelectionMode_Multiple)
	{
		if(m_iSelectAnchor == m_pView->getPoint())
		{
			return false;
		}
		xxx_UT_DEBUGMSG(("m_iSelectAnchor %d \n",m_iSelectAnchor));
		PT_DocPosition posLow = m_iSelectAnchor;
		PT_DocPosition posHigh = m_pView->getPoint();
		if(posHigh < posLow)
		{
			posHigh = m_iSelectAnchor;
			posLow = m_pView->getPoint();
		}
		return ((pos >= posLow) && (pos <=posHigh));
	}
	UT_sint32 i =0;
	for(i=0; i < m_vecSelRanges.getItemCount(); i++)
	{
		PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(i);
		xxx_UT_DEBUGMSG(("Looking at pos %d low %d high %d \n",pos, pDocRange->m_pos1,pDocRange->m_pos2 ));
		if ((pos >= pDocRange->m_pos1) && (pos <= pDocRange->m_pos2+1))
		{
			return true;
		}
	}
	return false;
		
}

bool FV_Selection::isSelected(void) const
{
	return FV_SelectionMode_NONE != m_iSelectionMode;
}

void FV_Selection::clearSelection(void)
{
	setMode(FV_SelectionMode_NONE);
	setSelectAll(false);
}

void FV_Selection::setTableLayout(fl_TableLayout * pFL)
{
	UT_ASSERT((m_iSelectionMode == 	FV_SelectionMode_TableColumn) 
			  || ( m_iSelectionMode == 	FV_SelectionMode_TableRow) 
			  || ( m_iSelectionMode == 	FV_SelectionMode_InTable));
	m_pTableOfSelectedColumn = pFL;
}

/*!
 * Add a range to the list of selected regions as defined by posLow, posHigh.
 * If bAddData is true also make a copy of the selected text in RTF format.
 */
void FV_Selection::addSelectedRange(PT_DocPosition /*posLow*/, PT_DocPosition /*posHigh*/, bool /*bAddData*/)
{

}

/*!
 * Remove a cell from the selection. This is the inverse of the addCellToSelection.
 * \return True if cell was selected, false if it wasn't.
 */
bool FV_Selection::removeCellFromSelection(fl_CellLayout* pCell){
    
    // this should only be done in the right selection modes!
    // all table selections should use mode InTable so this should be only
    // assert here, for now ( compatibility ) I check others too
    UT_ASSERT((m_iSelectionMode == 	FV_SelectionMode_TableColumn) 
	    || ( m_iSelectionMode == 	FV_SelectionMode_TableRow)
	    || ( m_iSelectionMode == FV_SelectionMode_InTable));
			  
    /*-----------------------------------------------------------------
    *  removing it from the ranges-vector
    *----------------------------------------------------------------*/
    // seeking a DocPoint inside our cell
	PL_StruxDocHandle sdhCell = pCell->getStruxDocHandle();
	PT_DocPosition posInCell = getDoc()->getStruxPosition(sdhCell)+1;
	
	// iterating the ranges vector checking if it was in it
	UT_sint32 i = 0;
	UT_sint32 iItemCount = m_vecSelRanges.getItemCount();
	PD_DocumentRange* pDocRange = NULL;
	bool bFound = false;
	
	while( !bFound && i < iItemCount ){
	    pDocRange = m_vecSelRanges.getNthItem(i);  
	    if ((posInCell >= pDocRange->m_pos1) && (posInCell <= pDocRange->m_pos2+1))
	        bFound = true;  
	    ++i;
	}
	
	// remember! if found our 'i' will be 1 to large
    // could use 'break' in loop too but I prefer this.
	
	// removing the range if it was in it, otherwise return false
	if( bFound ){
	    m_vecSelRanges.deleteNthItem(i-1);
	    // don't know if this is needed but when created 'new' was used and
	    // I couldn't find where the memory get's released otherwise..
	    // and we don't want memory leaks :)
	    delete pDocRange;
	} else return false;
	
	
	/*-----------------------------------------------------------------
    *  removing it from SelCellProps vector
    *----------------------------------------------------------------*/
    // getting the attach values of the cell we want to remove
    UT_sint32 iLeft,iRight,iTop,iBot;
	m_pView->getCellParams(posInCell,&iLeft,&iRight,&iTop,&iBot); 
	
	// again iterating over the vector
	FV_SelectionCellProps* pCellProps = NULL;
	i = 0;
	bFound = false;
	
	while( !bFound && i < iItemCount ){
	    pCellProps = m_vecSelCellProps.getNthItem(i);  
	        
	    // checking if it matches our cell since it's only 1 cell not a selection
	    // checking top and left only should do the trick
	    if( pCellProps->m_iLeft == iLeft && pCellProps->m_iTop == iTop )
	        bFound = true;  
	    ++i;
	}
	
	// remember! if found our 'i' will be 1 to large
    // could use 'break' in loop too but I prefer this.
	
	// removing the range if it was in it, otherwise return false
	if( bFound ){
	    m_vecSelCellProps.deleteNthItem(i-1);
	    delete pCellProps;
	}else return false;  // shouldn't be possible.. but to be sure
	
	
	/*-----------------------------------------------------------------
    *  removing it from RTF copy buffers
    *----------------------------------------------------------------*/
    // needs to be done!
    // don't really know how to find the right one in the vector..
    // need some guidance
}

/*!
 * Add a cell to the list of selected regions.
 */
void FV_Selection::addCellToSelection(fl_CellLayout * pCell)
{
	UT_ASSERT((m_iSelectionMode == 	FV_SelectionMode_TableColumn) 
			  || ( m_iSelectionMode == 	FV_SelectionMode_TableRow)
			  || ( m_iSelectionMode == FV_SelectionMode_InTable));
	
	// getting beginning and end Document Position of the cell	  
	PL_StruxDocHandle sdhEnd = NULL;
	PL_StruxDocHandle sdhStart = pCell->getStruxDocHandle();
	PT_DocPosition posLow = getDoc()->getStruxPosition(sdhStart) +1; // First block
	bool bres = getDoc()->getNextStruxOfType(sdhStart,PTX_EndCell,&sdhEnd);
	PT_DocPosition posHigh = getDoc()->getStruxPosition(sdhEnd) -1;
	
	// creating and adding a DocumentRange based on the DocPosition's
	UT_ASSERT(bres && sdhEnd);
	PD_DocumentRange * pDocRange = new PD_DocumentRange(getDoc(),posLow,posHigh);
	m_vecSelRanges.addItem(pDocRange);
	
	//
	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
	UT_ByteBuf * pByteBuf = new UT_ByteBuf;
    if (pExpRtf)
    {
		if(posLow < posHigh)
		{
			pDocRange->m_pos1++;
			pDocRange->m_pos2++;
		}
		pExpRtf->copyToBuffer(pDocRange,pByteBuf);
		if(posLow < posHigh)
		{
			pDocRange->m_pos1--;
			pDocRange->m_pos2--;
		}
		DELETEP(pExpRtf);
    }
	m_vecSelRTFBuffers.addItem(pByteBuf);
	
	// adding the cell params to a selected cells list
	FV_SelectionCellProps * pCellProps = new FV_SelectionCellProps;
	UT_sint32 iLeft,iRight,iTop,iBot;
	m_pView->getCellParams(posLow,&iLeft,&iRight,&iTop,&iBot);
	UT_DEBUGMSG(("In Selection left %d right %d top %d bot %d \n",iLeft,iRight,iTop,iBot));
	pCellProps->m_iLeft = iLeft;
	pCellProps->m_iRight = iRight;
	pCellProps->m_iTop = iTop;
	pCellProps->m_iBot = iBot;
	m_vecSelCellProps.addItem(pCellProps);
	setSelectAll(false);
}

/*!
 * Return the ith selection.
 */
PD_DocumentRange * FV_Selection::getNthSelection(UT_sint32 i) const
{
	if(i >= getNumSelections())
	{
		return NULL;
	}
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(i);
	return pDocRange;
}

/*!
 * Return the number of active selections.
 */
UT_sint32 FV_Selection::getNumSelections(void) const
{
	return m_vecSelRanges.getItemCount();
}
