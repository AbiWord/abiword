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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef FV_SELECTION_H
#define FV_SELECTION_H

#include "pt_Types.h"
#include "ut_vector.h"
#include "ut_string_class.h"

typedef enum _FV_SelectionMode
{
	FV_SelectionMode_NONE,
	FV_SelectionMode_TOC,
	FV_SelectionMode_TableRow,
	FV_SelectionMode_Single,
	FV_SelectionMode_Multiple,
	FV_SelectionMode_TableColumn
} FV_SelectionMode;

class UT_ByteBuf;
class FL_DocLayout;
class PD_Document;
class FV_View;
class fl_TableLayout;
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

	FV_Selection(FV_View * pView);
	~FV_Selection();
	PD_Document *         getDoc(void) const;
	FL_DocLayout *        getLayout(void) const;
    void                  setMode(FV_SelectionMode iSelMode);
	FV_SelectionMode      getSelectionMode(void) const
		{ return m_iSelectionMode;}
	FV_SelectionMode      getPrevSelectionMode(void) const
		{ return m_iPrevSelectionMode;}
	void                  setTOCSelected(fl_TOCLayout * pTOCL);
	fl_TOCLayout *        getSelectedTOC(void)
		{ return m_pSelectedTOC;}
	PT_DocPosition        getSelectionAnchor(void) const;
	void                  setSelectionAnchor(PT_DocPosition pos);
	PT_DocPosition        getSelectionLeftAnchor(void) const;
	void                  setSelectionLeftAnchor(PT_DocPosition pos);
	PT_DocPosition        getSelectionRightAnchor(void) const;
	void                  setSelectionRightAnchor(PT_DocPosition pos);
	UT_sint32             getNumSelections(void) const;
	PD_DocumentRange *    getNthSelection(UT_sint32 i) const;
	void                  addSelectedRange(PT_DocPosition posLow, PT_DocPosition posHigh, bool bAddData);
	bool                  isPosSelected(PT_DocPosition pos) const;
	bool                  isSelected(void) const;
	void                  clearSelection(void);
	void                  setTableLayout(fl_TableLayout * pFL);
	fl_TableLayout *      getTableLayout(void) const;
	void                  addCellToSelection(fl_CellLayout * pCell);
	void                  pasteRowOrCol(void);
	void                  checkSelectAll(void);
	void                  setSelectAll(bool bSelectAll);
	bool                  isSelectAll(void) const
	{ return m_bSelectAll;}
private:
	FV_View *             m_pView;
	FV_SelectionMode      m_iSelectionMode;
	FV_SelectionMode      m_iPrevSelectionMode;
	PT_DocPosition        m_iSelectAnchor;
	PT_DocPosition        m_iSelectLeftAnchor;
	PT_DocPosition        m_iSelectRightAnchor;
	fl_TableLayout *      m_pTableOfSelectedColumn;
	fl_TOCLayout  *       m_pSelectedTOC;
	UT_GenericVector<PD_DocumentRange *> m_vecSelRanges;
	UT_GenericVector<UT_ByteBuf*> m_vecSelRTFBuffers;
	UT_GenericVector<FV_SelectionCellProps*> m_vecSelCellProps;
	bool                  m_bSelectAll;
};

#endif /* FV_Selection_H */
