/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
 * Copyright (C) 2003 Francis James Franklin <fjf@alinameridon.com>
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

#ifndef IE_TABLES
#define IE_TABLES
#include "pd_Document.h"
#include "pt_Types.h"
#include "ut_wctomb.h"
#include "ut_stack.h"

class PD_Document;
class UT_Stack;
class PX_ChangeRecord_Object;
class PP_AttrProp;
class ie_imp_table;


class ABI_EXPORT ie_PartTable
{
 public:
	ie_PartTable(PD_Document * pDoc);
	virtual ~ie_PartTable(void);
	void             setDoc(PD_Document * pDoc);
	void             setTableApi(PL_StruxDocHandle sdh,PT_AttrPropIndex iApi);
	void             setCellApi(PT_AttrPropIndex iApi);
	UT_sint32        getLeft(void);
	UT_sint32        getRight(void);
	UT_sint32        getTop(void);
	UT_sint32        getBot(void);
	const char *     getTableProp(const char * pPropName);
	const char *     getCellProp(const char * pPropName);
	PT_AttrPropIndex getTableAPI(void) const {return m_apiTable;}
	PT_AttrPropIndex getCellAPI(void) const { return m_apiCell;}
	UT_sint32        getNumRows(void);
	UT_sint32        getNumCols(void);
	PL_StruxDocHandle getTableSDH(void)
		{ return m_TableSDH;}
	void             setCellJustOpenned(bool b);
	bool             isCellJustOpenned(void);
 private:
	void                  _setRowsCols(void);
	void                  _clearAll(void);
	void                  _clearAllCell(void);
	PD_Document *         m_pDoc;
	PT_AttrPropIndex      m_apiTable;			
	PT_AttrPropIndex      m_apiCell;
	const PP_AttrProp *   m_TableAttProp;
	const PP_AttrProp *   m_CellAttProp;
	UT_sint32             m_iNumRows;
	UT_sint32             m_iNumCols;
	UT_sint32             m_iLeft;
	UT_sint32             m_iRight;
	UT_sint32             m_iTop;
	UT_sint32             m_iBot;
	PL_StruxDocHandle     m_TableSDH;
	bool                  m_bIsCellJustOpenned;
};			


class ABI_EXPORT ie_Table
{
 public:
	ie_Table(PD_Document * pDoc);
	ie_Table(void);
	virtual ~ie_Table(void);
	void             setDoc(PD_Document * pDoc);
	void             OpenTable(PL_StruxDocHandle tableSDH, PT_AttrPropIndex iApi);
	void             OpenCell(PT_AttrPropIndex iApi);
	void             CloseTable(void);
	void             CloseCell(void);
	UT_sint32        getLeft(void);
	UT_sint32        getRight(void);
	UT_sint32        getTop(void);
	UT_sint32        getBot(void);
	UT_sint32        getNumRows(void);
	UT_sint32        getNumCols(void);
	const char *     getTableProp(const char * pPropName);
	const char *     getCellProp(const char * pPropName);
	UT_sint32        getNestDepth(void);
	void             setCellRowCol(UT_sint32 row, UT_sint32 col);
	PL_StruxDocHandle getTableSDH(void);
	void             setCellJustOpenned(bool b);
	bool             isCellJustOpenned(void);
	PT_AttrPropIndex getTableAPI(void);
	PT_AttrPropIndex getCellAPI(void);
 private:
	PD_Document *     m_pDoc;
	UT_Stack          m_sLastTable;
};			


class ABI_EXPORT ie_imp_cell
{
 public:
	ie_imp_cell(ie_imp_table * pImpTable, PD_Document * pDoc, 
				ie_imp_cell * pImpCell, UT_sint32 iRow);
	virtual          ~ie_imp_cell(void);
	void             setCellX(UT_sint32 cellx);
	UT_sint32        getCellX(void);
	void             setCellLeft(ie_imp_cell * pImpCell);
	void             setLeft(UT_sint32 iLeft);
	UT_sint32        getLeft(void);
	void             setRight(UT_sint32 iRight);
	UT_sint32        getRight(void);
	void             setTop(UT_sint32 iTop);
	UT_sint32        getTop(void);
	void             setBot(UT_sint32 iBot);
	UT_sint32        getBot(void);
	PL_StruxDocHandle getCellSDH(void);
	void             setCellSDH(PL_StruxDocHandle cellSDH);
	void             writeCellPropsInDoc(void);
	ie_imp_cell *    getCellAbove(void);
	ie_imp_cell *    getCellBelow(void);
	ie_imp_cell *    getCellLeft(void);
	ie_imp_cell *    getCellRight(void);
	void             addPropString(const UT_String & sPropString);
	void             setProp(const UT_String & psProp, const UT_String & psVal);
	UT_String        getPropVal(const UT_String & psProp);
	void             setProp(const char * szProp, const char * szVal);
	UT_String        getPropVal(const char * szProp);
	UT_sint32        getRow(void) { return m_iRow;}
	void             setMergeAbove(bool bAbove) { m_bMergeAbove = bAbove;}
	void             setMergeRight(bool bRight) {m_bMergeRight = bRight;}
	void             setMergeLeft(bool bLeft) {m_bMergeLeft = bLeft;}
	void             setFirstHorizontalMerge(bool bHori) {m_bFirstHori = bHori;}
	void             setFirstVerticalMerge( bool bVert) {m_bFirstVertical = bVert;}
	bool             isMergedAbove(void) const {return m_bMergeAbove;}
	bool             isMergedRight(void) const {return m_bMergeRight;}
	bool             isMergedLeft(void) const {return m_bMergeLeft;}
	bool             isFirstVerticalMerged(void) const {return m_bFirstVertical;}
	bool             isFirstHorizontalMerged(void) const {return m_bFirstHori;}
	void             copyCell(ie_imp_cell * pCell);
	void             setImpTable(ie_imp_table * pTable) { m_pImpTable = pTable;}
	void             setRow(UT_sint32 row) { m_iRow = row;}
 private:
	PD_Document *         m_pDoc;
	UT_sint32             m_iCellX;
	UT_sint32             m_iLeft;
	UT_sint32             m_iRight;
	UT_sint32             m_iTop;
	UT_sint32             m_iBot;
	PL_StruxDocHandle     m_cellSDH;
	ie_imp_table   *      m_pImpTable;
    ie_imp_cell *         m_pCellLeft;
	UT_sint32             m_iRow;
	bool                  m_bMergeAbove;
	bool                  m_bMergeRight;
	bool                  m_bMergeLeft;
	bool                  m_bFirstVertical;
	bool                  m_bFirstHori;
	UT_String             m_sCellProps;
};			


class ABI_EXPORT ie_imp_table
{
 public:
	ie_imp_table(PD_Document * pDoc);
	virtual ~ie_imp_table(void);
	UT_sint32           OpenCell(void);
	UT_sint32           NewRow(void);
	void                setCellRowNthCell(UT_sint32 row, UT_sint32 col);
	ie_imp_cell *       getNthCellOnRow(UT_sint32 iCell);
	void                setCellX(UT_sint32 cellx);
	PL_StruxDocHandle   getTableSDH(void);
	void                setTableSDH(PL_StruxDocHandle cellSDH);
	void                writeTablePropsInDoc(void);
	void                writeAllCellPropsInDoc(void);
	void                setProp(const UT_String & psProp, const UT_String & psVal);
	void                setProp(const char *szProp, const char *  szVal);
	UT_String           getPropVal(const UT_String & psProp);
	UT_String           getPropVal(const char * szProp);
	UT_String           getCellPropVal(const UT_String & psProp);
	void                setCellProp(const UT_String & psProp, const UT_String & psVal);
	ie_imp_cell *       getCurCell(void);
	void                setNthCellOnThisRow(UT_sint32 iCell);
	void                buildTableStructure(void);
	void                setAutoFit(bool bVal) {m_bAutoFit = bVal;}
	bool                isAutoFit(void) { return m_bAutoFit;}
	bool                isNewRow(void) { return m_bNewRow;}
	UT_sint32           getColNumber(ie_imp_cell * pImpCell);
	ie_imp_cell *       getCellAtRowColX(UT_sint32 newRow,UT_sint32 cellX);
	void                CloseCell(void);
	bool                wasTableUsed(void) { return m_bTableUsed;}
	void                setCell( ie_imp_cell * pCell) { m_pCurImpCell = pCell;}
	UT_sint32           getRow(void) { return m_iRowCounter;}
	void                removeExtraneousCells(void);
	UT_sint32           getNumRows(void);
	void                setPosOnRow(UT_sint32 posOnRow) { m_iPosOnRow = posOnRow;}
	UT_sint32           getPosOnRow(void) { return m_iPosOnRow;}
	void                setCellXOnRow(UT_sint32 cellxOnRow) { m_iCellXOnRow = cellxOnRow;}
	UT_sint32           getCellXOnRow(void) { return m_iCellXOnRow;}
	void                incPosOnRow(void) { m_iPosOnRow++;}
	void                incCellXOnRow(void) { m_iCellXOnRow++;}
	bool                getVecOfCellsOnRow(UT_sint32 row, UT_Vector * pVec);
	bool                removeRow(UT_sint32 row);
	void                appendRow(UT_Vector * pVecRowOfCells);
	bool                doCellXMatch(UT_sint32 iCellX1, UT_sint32 iCellX2);
 private:
	void                _buildCellXVector(void);
	void                _removeAllStruxes(void);
	PD_Document *       m_pDoc;
	PL_StruxDocHandle   m_tableSDH;
	ie_imp_cell *       m_pCurImpCell;
	UT_sint32           m_iRowCounter;
	UT_String           m_sTableProps;
	bool                m_bAutoFit;
	bool                m_bNewRow;
	bool                m_bTableUsed;
	UT_sint32           m_iPosOnRow;
	UT_sint32           m_iCellXOnRow;
	UT_Vector           m_vecCells;
	UT_Vector           m_vecCellX;
	UT_Vector           m_vecSavedX;
};			

class ABI_EXPORT ie_imp_table_control
{
public:
	ie_imp_table_control(PD_Document * pDoc);
	virtual ~ie_imp_table_control(void);
	UT_sint32           getNestDepth(void);
	void                OpenTable(void);
	UT_sint32           OpenCell(void);
	void                CloseTable(void);
	void                CloseCell(void);
	ie_imp_table *      getTable(void);
	bool                NewRow(void);
	void                SaveRowInfo(void);
	void                RemoveRowInfo(void);
private:
	UT_Stack            m_sLastTable;
	PD_Document *       m_pDoc;
};

// EXPERIMENTAL CODE
/* #define USE_IE_IMP_TABLEHELPER 1 */

#ifdef USE_IE_IMP_TABLEHELPER

class ABI_EXPORT IE_Imp_TableHelper
{
public:
	static bool			Strux (PD_Document * pDocument, PTStruxType pts,
							   const XML_Char ** attributes,
							   pf_Frag * pf,
							   pf_Frag_Strux *& pfs_ret, bool bImport);

private:
	enum TableZone
		{
			/* tz_caption, */
			tz_head,
			tz_foot,
			tz_body
		};
	class ABI_EXPORT CellHelper
	{
	public:
		CellHelper ();

		UT_UTF8String		m_style;

		/* cell frag/strux
		 */
		pf_Frag_Strux *		m_pfsCell;
		
		/* cell-attach points
		 */
		UT_uint32			m_bottom;
		UT_uint32			m_left;
		UT_uint32			m_right;
		UT_uint32			m_top;

		UT_uint32			m_rowspan;
		UT_uint32			m_colspan;

		CellHelper *		m_next;

		bool	isVirtual () const { return (m_next != 0); }
	};

	/* 1. Need a section on column definitions, allowing for <col> and <colgroup><col>
	 * 2. <thead> & <tfoot> should come before <tbody>; any repeats or trailing entries
	 *    shall be treated as additional <tbody> sections for the moment
	 */
	
	PD_Document *		m_pDocument;

	UT_UTF8String		m_style_table;
	UT_UTF8String		m_style_tzone; // thead,tfoot,tbody
	UT_UTF8String		m_style; // tr

	/* cell frag/strux
	 */
	pf_Frag_Strux *		m_pfsInsertionPoint;
	pf_Frag_Strux *		m_pfsTableStart;
	pf_Frag_Strux *		m_pfsTableEnd;
		
	UT_uint32			m_rows;
	UT_uint32			m_rows_head;
	UT_uint32			m_rows_head_max;
	UT_uint32			m_rows_foot;
	UT_uint32			m_rows_foot_max;
	UT_uint32			m_rows_body;
	UT_uint32			m_rows_body_max;

	UT_uint32			m_cols;
	UT_uint32			m_cols_max;

	UT_uint32			m_col_next;
	UT_uint32			m_row_next;

	UT_uint32			m_garbage_count;

	CellHelper ***		m_thead;
	CellHelper ***		m_tfoot;
	CellHelper ***		m_tbody;

	CellHelper *		m_current;

	CellHelper *		m_garbage;

	TableZone			m_tzone;

	PD_Document *	getDoc () const { return m_pDocument; }

	pf_Frag_Strux *	getInsertionPoint () const { return m_pfsInsertionPoint; }

	/* garbage collection & recycling
	 */
	CellHelper *	recoverCellHelper (bool create_if_none = true);
	void			discardCellHelper (CellHelper * cell_helper);
	bool			requireCellHelper (UT_uint32 number);

public:
	IE_Imp_TableHelper (PD_Document * pDocument, pf_Frag_Strux * pfsInsertionPoint, const char * style);

	~IE_Imp_TableHelper ();

private:
	bool	tableStart ();
public:
	bool	tableEnd ();

	bool	theadStart (const char * style);
	bool	tfootStart (const char * style);
	bool	tbodyStart (const char * style = 0);

	bool	trStart (const char * style);
private:
	bool	trEnd ();
	void	trClean ();
public:
	bool	tdStart (UT_uint32 rowspan, UT_uint32 colspan, const char * style, bool bounded = false);
private:
	bool	tdEnd ();
	bool	tdPending ();

	/* these confirm capacities of existing arrays, they do not create new arrays
	 */
	bool	grow_cols (UT_uint32 cols);
	bool	grow_rows (UT_uint32 rows);

	/* these add new (virtual and/or pending) cells and increase effective table size
	 */
	bool	append_cols (UT_uint32 cols = 1);
	bool	append_rows (UT_uint32 rows = 1);

	bool	append_col (); // use these via the above
	bool	append_row ();
};

class ABI_EXPORT IE_Imp_TableHelperStack
{
private:
	PD_Document *			m_pDocument;

	UT_uint32				m_count;
	UT_uint32				m_max;

	IE_Imp_TableHelper **	m_stack;

	pf_Frag *				m_pfInsertionPoint;

public:
	IE_Imp_TableHelperStack (PD_Document * pDocument, PT_DocPosition docPos);
	IE_Imp_TableHelperStack (PD_Document * pDocument);

	~IE_Imp_TableHelperStack ();

	void					clear ();

private:
	bool					push (const char * style);
	bool					pop ();
public:
	IE_Imp_TableHelper *	top () const { return m_stack ? m_stack[m_count] : 0; }

	bool					tableStart (const char * style);
	bool					tableEnd ();

	bool					theadStart (const char * style);
	bool					tfootStart (const char * style);
	bool					tbodyStart (const char * style = 0);
	bool					trStart (const char * style);
	bool					tdStart (UT_uint32 rowspan, UT_uint32 colspan, const char * style);
};

#endif /* USE_IE_IMP_TABLEHELPER */

#endif /* IE_TABLE */
