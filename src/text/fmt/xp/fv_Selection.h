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

typedef enum _FV_SelectionMode
{
	FV_SelectionMode_NONE,
	FV_SelectionMode_Single,
	FV_SelectionMode_Multiple,
	FV_SelectionMode_TableColumn,
	FV_SelectionMode_TableRow
} FV_SelectionMode;

class FL_DocLayout;
class PD_Document;
class FV_View;
class fl_TableLayout;
class fl_CellLayout;
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

	FV_Selection(FV_View * pView);
	~FV_Selection();
	PD_Document *         getDoc(void) const;
	FL_DocLayout *        getLayout(void) const;
    void                  setMode(FV_SelectionMode iSelMode);
	FV_SelectionMode      getSelectionMode(void) const 
		{ return m_iSelectionMode;}
	PT_DocPosition        getSelectionAnchor(void) const;
	void                  setSelectionAnchor(PT_DocPosition pos);
	PT_DocPosition        getSelectionLeftAnchor(void) const;
	void                  setSelectionLeftAnchor(PT_DocPosition pos);
	PT_DocPosition        getSelectionRightAnchor(void) const;
	void                  setSelectionRightAnchor(PT_DocPosition pos);
	UT_sint32             getNumSelections(void);
	PD_DocumentRange *    getNthSelection(UT_sint32 i);
	void                  addSelectedRange(PT_DocPosition posLow, PT_DocPosition posHigh, bool bAddData);
	bool                  isPosSelected(PT_DocPosition pos) const;
	bool                  isSelected(void) const;
	void                  clearSelection(void);
	void                  setTableLayout(fl_TableLayout * pFL);
	void                  addCellToSelection(fl_CellLayout * pCell);
private:
	FV_View *             m_pView;
	FV_SelectionMode      m_iSelectionMode;
	PT_DocPosition        m_iSelectAnchor;
	PT_DocPosition        m_iSelectLeftAnchor;
	PT_DocPosition        m_iSelectRightAnchor;
	fl_TableLayout *      m_pTableOfSelectedColumn;
	UT_Vector             m_vecSelRanges;
	UT_Vector             m_vecSelRTFBuffers;
	UT_Vector             m_vecSelCellProps;
};

#endif /* FV_Selection_H */
