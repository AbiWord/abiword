 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef fl_ColumnLayout_H
#define fl_ColumnLayout_H

#include "ut_types.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class fl_ColumnSetLayout;
class fp_Column;
class PP_AttrProp;

class fl_ColumnLayout : public fl_Layout
{
public:
	fl_ColumnLayout(fl_ColumnSetLayout * pCSL, PL_StruxDocHandle sdh);
	~fl_ColumnLayout();

	fl_ColumnLayout *		setNext(fl_ColumnLayout * pCL);
	fl_ColumnLayout *		setPrev(fl_ColumnLayout * pCL);
	fl_ColumnLayout *		getNext(void) const;
	fl_ColumnLayout *		getPrev(void) const;

	void					setColumnSetLayout(fl_ColumnSetLayout * pcsl);
	fl_ColumnSetLayout *	getColumnSetLayout(void) const;
	UT_Bool					getNewColumn(UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
										 fp_Column ** ppCol,
										 UT_sint32 * piXoff, UT_sint32 * piYoff) const;
	
protected:
	fl_ColumnLayout *		m_prev;
	fl_ColumnLayout *		m_next;
	fl_ColumnSetLayout *	m_pColumnSetLayout;
};

#endif /* fl_ColumnLayout_H */
