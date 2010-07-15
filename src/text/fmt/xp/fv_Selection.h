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

#ifndef FV_SELECTION_H
#define FV_SELECTION_H

#include "pt_Types.h"
#include "ut_vector.h"
#include "ut_string_class.h"
#include "fl_TableLayout.h"
#include "fp_TableContainer.h"
#include <vector>

typedef enum _FV_SelectionMode
{
	FV_SelectionMode_NONE,
	FV_SelectionMode_TOC,
	// DEPRECATED ALL SHOULD BE InTable now!! Should be removed
	    FV_SelectionMode_TableRow,  
	FV_SelectionMode_InTable,
	FV_SelectionMode_Single,
	FV_SelectionMode_Multiple,
	// DEPRECATED ALL SHOULD BE InTable now!! Should be removed
	    FV_SelectionMode_TableColumn
} FV_SelectionMode;

class FL_DocLayout;
class PD_Document;
class FV_View;
class fl_CellLayout;
class fl_TOCLayout;


class ABI_EXPORT FV_Selection
{
	friend class fv_View;

	public:
	class ABI_EXPORT FV_SelectionCellProps
	{
	public:
		FV_SelectionCellProps(void):m_iLeft(0),
									m_iRight(0),
									m_iTop(0),
									m_iBot(0),
									m_sProps("")
			{}
		UT_sint32 m_iLeft;
		UT_sint32 m_iRight;
		UT_sint32 m_iTop;
		UT_sint32 m_iBot;
		UT_String m_sProps;
	};

		
// Constructor & Destructor
////////////////////////////////////////////////////////////////////////////////////////
	FV_Selection(FV_View * pView);
	~FV_Selection();


// Global getters & setters
////////////////////////////////////////////////////////////////////////////////////////
	void                  setMode(FV_SelectionMode iSelMode);
	void                  setTOCSelected(fl_TOCLayout * pTOCL);

	PD_Document *         getDoc(void) const;
	FL_DocLayout *        getLayout(void) const;
	FV_SelectionMode      getSelectionMode(void) const 			{ return m_iSelectionMode;}
	FV_SelectionMode      getPrevSelectionMode(void) const 		{ return m_iPrevSelectionMode;}
	fl_TOCLayout *        getSelectedTOC(void)					{ return m_pSelectedTOC;}

	UT_sint32             getNumSelections(void) const;
	PD_DocumentRange *    getNthSelection(UT_sint32 i) const;
	void                  addSelectedRange(PT_DocPosition posLow, PT_DocPosition posHigh, bool bAddData);
	bool                  isPosSelected(PT_DocPosition pos) const;
	bool                  isSelected(void) const;
	void                  clearSelection(void);
	void                  checkSelectAll(void);
	bool                  isSelectAll(void) const	{ return m_bSelectAll;}


// Anchor stuff
/////////////////////////////////////////////////////////////////////////////////////////
	PT_DocPosition        getSelectionAnchor(void) const;
	PT_DocPosition        getSelectionLeftAnchor(void) const;
	PT_DocPosition        getSelectionRightAnchor(void) const;

	void                  setSelectionAnchor(PT_DocPosition pos);
	void                  setSelectionLeftAnchor(PT_DocPosition pos);
	void                  setSelectionRightAnchor(PT_DocPosition pos);


// Table stuff
/////////////////////////////////////////////////////////////////////////////////////////
	bool                  getRectTableSel(UT_sint32* left, UT_sint32* right, UT_sint32* top, UT_sint32* bottom);
	void                  setRectTableSel(UT_sint32 left, UT_sint32 right, UT_sint32 top, UT_sint32 bottom);	
	bool                  isSingleTableRowSelected(void) const;
	bool                  isSingleTableColumnSelected(void) const;
	void                  addCellToSelection(fl_CellLayout * pCell);
	bool                  removeCellFromSelection(fl_CellLayout* pCell);
	bool 				  getTableSelAsRangesVector(std::vector<PD_DocumentRange> &ranges);


private:
	void				  _checkSelectAll(PT_DocPosition low, PT_DocPosition high);
	void                  _setSelectAll(bool bSelectAll);

	// pointer to the view the selections belongs too
	FV_View *             m_pView;

	// selection modes
	FV_SelectionMode      m_iSelectionMode;
	FV_SelectionMode      m_iPrevSelectionMode;

	// anchors used
	PT_DocPosition        m_iSelectAnchor;
	PT_DocPosition        m_iSelectLeftAnchor;
	PT_DocPosition        m_iSelectRightAnchor;

	// if a table of content is selected, pointer to it
	fl_TOCLayout  *       m_pSelectedTOC;

	// vector with all ranges selected
	UT_GenericVector<PD_DocumentRange *> 		m_vecSelRanges;

	// is whole document selected?
	bool                  m_bSelectAll;

	// table rectangle selection
	UT_sint32             m_iLeftTableRect;
	UT_sint32             m_iRightTableRect;
	UT_sint32             m_iTopTableRect;
	UT_sint32             m_iBottomTableRect;
};

#endif /* FV_Selection_H */
