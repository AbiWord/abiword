/* AbiWord
 * Copyright (C) 2002 Martin Sevior
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

/******************************************************************
** This file is considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to actually write
** the contents of the document to the RTF file.
******************************************************************/

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
	UT_sint32        getNumRows(void);
	UT_sint32        getNumCols(void);
	PL_StruxDocHandle getTableSDH(void)
		{ return m_TableSDH;}
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
 private:
	PD_Document *     m_pDoc;
	UT_Stack          m_sLastTable;
};			

#endif /* IE_TABLE */



