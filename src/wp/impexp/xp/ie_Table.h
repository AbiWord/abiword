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

#ifndef IE_TABLE_H
#define IE_TABLE_H

#include "pd_Document.h"
#include "pt_Types.h"
#include "ut_wctomb.h"
#include "ut_stack.h"

/******************************************************************
** This file is considered useful to all exporters
******************************************************************/

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

 private:
	PD_Document *     m_pDoc;
	UT_Stack          m_sLastTable;
};			

#endif /* IE_TABLE_H */



