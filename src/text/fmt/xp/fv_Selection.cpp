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

////////////////////////////////////////////////////////////////////////////////
/////		CONSTRUCTOR AND DESTRUCTOR					     				////
////////////////////////////////////////////////////////////////////////////////

FV_Selection::FV_Selection (FV_View * pView)
	: m_pView (pView), 
	  m_iSelectionMode(FV_SelectionMode_NONE),
	  m_iPrevSelectionMode(FV_SelectionMode_NONE),
	  m_iSelectAnchor(0),
	  m_iSelectLeftAnchor(0),
	  m_iSelectRightAnchor(0),
	  m_pSelectedTOC(NULL),
	  m_bSelectAll(false),
	  m_iLeftTableRect(-1),
	  m_iRightTableRect(-1),
	  m_iTopTableRect(-1),
	  m_iBottomTableRect(-1)
{
	UT_ASSERT (pView);
	m_vecSelRanges.clear();
}

FV_Selection::~FV_Selection()
{
	m_pSelectedTOC = NULL;
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
}




////////////////////////////////////////////////////////////////////////////////
/////		ANCHOR METHODS          					     				////
////////////////////////////////////////////////////////////////////////////////

/*!
 *	Sets the selection anchor.
 */
void FV_Selection::setSelectionAnchor(PT_DocPosition pos)
{
	 m_iSelectAnchor = pos;
	 checkSelectAll();
}


/*
 *	Sets the left selection anchor to the given DocPosition
 */
void FV_Selection::setSelectionLeftAnchor(PT_DocPosition pos)
{
    if(pos == 0) return;
	m_iSelectLeftAnchor = pos;
	_checkSelectAll(m_iSelectLeftAnchor, m_iSelectRightAnchor);
}


/*!
 *	Sets the right selection anchor to the given DocPosition
 */
void FV_Selection::setSelectionRightAnchor(PT_DocPosition pos)
{
    if(pos == 0) return;
	m_iSelectRightAnchor = pos;
	_checkSelectAll(m_iSelectLeftAnchor, m_iSelectRightAnchor);
}


/*!
 *	Returns the selection anchor.
 */
PT_DocPosition FV_Selection::getSelectionAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) ||  (m_vecSelRanges.getItemCount() == 0))
		return m_iSelectAnchor;
	
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(0);
	return pDocRange->m_pos1;
}


/*!
 *	Return the most left selection anchor.
 */
PT_DocPosition FV_Selection::getSelectionLeftAnchor(void) const
{
	// Only one range in the selection
	if((m_iSelectionMode < FV_SelectionMode_Multiple) || (m_vecSelRanges.getItemCount() == 0))
		return m_iSelectLeftAnchor;

	// Multiple ranges selected
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(0);
	return pDocRange->m_pos1;
}


/*!
 *	Return the most right selection anchor.
 */
PT_DocPosition FV_Selection::getSelectionRightAnchor(void) const
{
	if((m_iSelectionMode < FV_SelectionMode_Multiple) || (m_vecSelRanges.getItemCount() == 0) )
		return m_iSelectRightAnchor;
	
	PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(0);
	return pDocRange->m_pos2;
}




////////////////////////////////////////////////////////////////////////////////
/////		TABLE METHODS           					     				////
////////////////////////////////////////////////////////////////////////////////


/*! 
 *	 Return the current attach values of the table selection rectangle 
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


/*! 
 * 	Set the attach values of the table selection rectangle
 *  This won't add cells to a selection properly, one should use
 *	addCellToSelection(..) for that.
 */
void FV_Selection::setRectTableSel 
(UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom)
{
    m_iLeftTableRect = left;
	m_iRightTableRect = right;
	m_iTopTableRect = top;
	m_iBottomTableRect = bottom;
}


/*! 
 *	Return true if only a single row is selected 
 */
bool FV_Selection::isSingleTableRowSelected(void) const
{
    return ( m_iTopTableRect == m_iBottomTableRect-1);
}


/*! 
 *	Return true if only a single column is selected 
 */
bool FV_Selection::isSingleTableColumnSelected(void) const
{
    return ( m_iLeftTableRect == m_iRightTableRect-1);
}


/*!
 * Returns a vector of the selected ranges. This will result in a range for each
 * row selected.
 */
 bool FV_Selection::getTableSelAsRangesVector(std::vector<PD_DocumentRange>& ranges){

	
	// check to see if we are in a table
	if( getSelectionMode() != FV_SelectionMode_InTable ) return false;
	if( m_iLeftTableRect < 0 )	return false;
	if( ! m_pView->isInTable(getSelectionAnchor()) ) return false; 
	
	// get table strux
	fl_TableLayout* tableLayout = m_pView->getTableAtPos(getSelectionAnchor());
	fp_TableContainer* table = static_cast<fp_TableContainer *>(tableLayout->getFirstContainer());
	 
	//
	// Run through rows and make 1 PD_DocumentRange for each row
	//
	PL_StruxDocHandle sdh, sdhEnd;
	fl_CellLayout* tmpCell;
	for( int i= m_iTopTableRect; i<m_iBottomTableRect; ++i)
	{	
		// Start point, first cell left
		tmpCell = static_cast<fl_CellLayout *>(table->getCellAtRowColumn(i,m_iLeftTableRect)->getSectionLayout());
		sdh = tmpCell->getStruxDocHandle();
		
		// End point
	    tmpCell = static_cast<fl_CellLayout *>(table->getCellAtRowColumn(i,m_iRightTableRect-1)->getSectionLayout());
		sdh = tmpCell->getStruxDocHandle();
		sdhEnd = NULL;
		getDoc()->getNextStruxOfType(sdh,PTX_EndCell,&sdhEnd);
		
		// adding to vector
		PD_DocumentRange linerange(getDoc(), getDoc()->getStruxPosition(sdh), getDoc()->getStruxPosition(sdhEnd));
		ranges.push_back(linerange);		
	}
	 
	return true;
}


/*!
 * Remove a cell from the selection. This is the inverse of the addCellToSelection.
 * \return True if cell was selected, false if it wasn't.
 */
bool FV_Selection::removeCellFromSelection(fl_CellLayout* pCell){
    
    UT_ASSERT(m_iSelectionMode == FV_SelectionMode_InTable);
			  
    // seeking a DocPoint inside our cell
	PL_StruxDocHandle sdhCell = pCell->getStruxDocHandle();
	PT_DocPosition posInCell = getDoc()->getStruxPosition(sdhCell)+1;
	
	// iterating the ranges vector checking if it was in it
	UT_sint32 i = 0;
	PD_DocumentRange* pDocRange = NULL;
	bool bFound = false;
	
	while( !bFound && i < m_vecSelRanges.getItemCount() ){
	    pDocRange = m_vecSelRanges.getNthItem(i);  
	    if ((posInCell >= pDocRange->m_pos1) && (posInCell <= pDocRange->m_pos2+1))
	        bFound = true;  
	    ++i;
	}
	
	// removing the range if it was in it, otherwise return false
	if( bFound ){
	    m_vecSelRanges.deleteNthItem(i-1); // if found our 'i' will be 1 to large
	    delete pDocRange;
	} else return false;

}


/*!
 * Add a cell to the list of selected regions.
 */
void FV_Selection::addCellToSelection(fl_CellLayout * pCell)
{
	UT_ASSERT(m_iSelectionMode == FV_SelectionMode_InTable);
	
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
}




////////////////////////////////////////////////////////////////////////////////
/////		OTHER METHODS           					     				////
////////////////////////////////////////////////////////////////////////////////

/*!
 *	Checks to see if the whole document is selected. If it is m_bSelectAll is set.
 */
void  FV_Selection::checkSelectAll(void)
{
	 fl_SectionLayout * pSL = m_pView->m_pLayout->getLastSection();
	
	 if(pSL == NULL) 								return;
	 if(m_pView->m_pDoc->isPieceTableChanging()) 	return;
	 if(m_pView->m_pLayout->isLayoutFilling()) 		return;
	
	 PT_DocPosition posLow = m_iSelectAnchor;
	 PT_DocPosition posHigh = m_pView->getPoint();
	 if(posHigh < posLow)
	 {
	      posHigh = m_iSelectAnchor;
	      posLow = m_pView->getPoint();
	 }

	_checkSelectAll(posLow, posHigh);
}


/*!
 *	Checks to see if entire document is selected using the given anchors.
 */
void FV_Selection::_checkSelectAll(PT_DocPosition low, PT_DocPosition high){
	 PT_DocPosition posBeg,posEnd=0;
	 m_pView->getEditableBounds(false,posBeg);
	 m_pView->getEditableBounds(true,posEnd);
	
	 bool bSelAll = ((low <= posBeg) && (high >= posEnd));
	 _setSelectAll(bSelAll);	
}


/*!
 *	Simple wrapper for setting the SelectAll boolean.
 */
void  FV_Selection::_setSelectAll(bool bSelectAll)
{
    xxx_UT_DEBUGMSG(("Select All = %d \n",bSelectAll));
	m_bSelectAll = bSelectAll;	      
}


/*!
 *	We set the SelectionMode to the requested one. 
 *  Dzan - GSoC: Maybe think about doing this automaticly when selection changes.
 *				 Scan where the cursor is, get the strux and check the type...?
 */
void FV_Selection::setMode(FV_SelectionMode iSelMode)
{
    // WE SHOULD NO LONGER USE THE ROW AND COL MODES, ALL SHOULD BE IN_TABLE
    if( iSelMode == FV_SelectionMode_TableColumn || iSelMode == FV_SelectionMode_TableRow )
        UT_DEBUGMSG(("USING A DEPRACATED SELECTION MODE USED!!! FIX THIS!!!"));
    
    // Save the current mode to the previousmode
	if( (m_iSelectionMode != FV_SelectionMode_NONE) && (iSelMode !=  FV_SelectionMode_NONE))
		m_iPrevSelectionMode = m_iSelectionMode;
	
	// Deal with table of content selections
	if((m_iSelectionMode == FV_SelectionMode_TOC) && (m_iSelectionMode != iSelMode))
	{
		if(m_pSelectedTOC) m_pSelectedTOC->setSelected(false);
		m_pSelectedTOC = NULL;
	}

	// If mode set to NONE, no selection is made! All needs to be reset
	if(m_iSelectionMode == FV_SelectionMode_NONE)
	{
		UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecSelRanges);
		m_vecSelRanges.clear();
		m_iLeftTableRect = -1;
	    m_iRightTableRect = -1;
	    m_iTopTableRect = -1;
	    m_iBottomTableRect = -1;
	}

	// SET THE CURRENT SELECTION MODE AND UNSET SELECTALL
	m_iSelectionMode = iSelMode;
	_setSelectAll(false);
}


/*!
 *	Sets some stuff when a Table of Content is selected.
 *  DZan - GSoC: Maybe this doesn't belong here but in the TOC class..?
 */
void FV_Selection::setTOCSelected(fl_TOCLayout * pTOCL)
{
	UT_return_if_fail(pTOCL);
	setMode(FV_SelectionMode_TOC);
	m_pSelectedTOC = pTOCL;
	m_iSelectAnchor = pTOCL->getPosition();
	pTOCL->setSelected(true);
	_setSelectAll(false);
}


/*!
 *	Important method! This is used by the drawing classes to determine if something
 *  should be drawn as being selected or not!
 */
bool FV_Selection::isPosSelected(PT_DocPosition pos) const
{
	if(m_iSelectionMode == FV_SelectionMode_NONE)
	{
		return false;
	}
	if( m_iSelectionMode == FV_SelectionMode_InTable ){
	    //
	    // This is the vector/range-way of testing
		//
		
	    /*UT_sint32 i =0;
	    PD_DocumentRange * pDocRange = NULL;
	    for(i=0; i < m_vecSelRanges.getItemCount(); i++){
	        pDocRange = m_vecSelRanges.getNthItem(i);           
	        if ((pos >= pDocRange->m_pos1) && (pos <= pDocRange->m_pos2+1)) 
	            return true;
	    }
	    // no cell found around the pos
	    return false;*/

		//
		// I'm going to use the attach values for now to check, seems faster
		//
		
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

	//
	// Dzan - Should look into this, all should be replaced by this part:
	//
	/*UT_sint32 i =0;
	for(i=0; i < m_vecSelRanges.getItemCount(); i++)
	{
		PD_DocumentRange * pDocRange = m_vecSelRanges.getNthItem(i);
		xxx_UT_DEBUGMSG(("Looking at pos %d low %d high %d \n",pos, pDocRange->m_pos1,pDocRange->m_pos2 ));
		if ((pos >= pDocRange->m_pos1) && (pos <= pDocRange->m_pos2+1))
		{
			return true;
		}
	}*/
	return false;
		
}


/*! 
 *	Returns true if a selection is made at the current moment.
 */
bool FV_Selection::isSelected(void) const
{
	return FV_SelectionMode_NONE != m_iSelectionMode;
}


/*!
 *	Clears the selections.
 */
void FV_Selection::clearSelection(void)
{
	setMode(FV_SelectionMode_NONE);
	_setSelectAll(false);
}


/*!
 * Add a range to the list of selected regions as defined by posLow, posHigh.
 * If bAddData is true also make a copy of the selected text in RTF format.
 */
void FV_Selection::addSelectedRange(PT_DocPosition /*posLow*/, PT_DocPosition /*posHigh*/, bool /*bAddData*/)
{

}


/*!
 *	Return the ith selection.
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
 *	Return the number of active selections.
 */
UT_sint32 FV_Selection::getNumSelections(void) const
{
	return m_vecSelRanges.getItemCount();
}


/*!
 *	Return the PT_Document to which the selection belongs.
 */
PD_Document * FV_Selection::getDoc(void) const
{
	return m_pView->getDocument();
}


/*!
 *	Return the FL_DocLayout to which the selection belongs.
 */
FL_DocLayout * FV_Selection::getLayout(void) const
{
	return m_pView->getLayout();
}

