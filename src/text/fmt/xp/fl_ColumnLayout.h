 
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

#ifndef FL_COLUMNLAYOUT_H
#define FL_COLUMNLAYOUT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_ColumnSetLayout;
class FP_Column;
class PP_AttrProp;

class FL_ColumnLayout : public fl_Layout
{
public:
	FL_ColumnLayout(FL_ColumnSetLayout * pCSL, PL_StruxDocHandle sdh);
	~FL_ColumnLayout();

	FL_ColumnLayout *		setNext(FL_ColumnLayout * pCL);
	FL_ColumnLayout *		setPrev(FL_ColumnLayout * pCL);
	FL_ColumnLayout *		getNext(void) const;
	FL_ColumnLayout *		getPrev(void) const;

	void					setColumnSetLayout(FL_ColumnSetLayout * pcsl);
	FL_ColumnSetLayout *	getColumnSetLayout(void) const;
	UT_Bool					getNewColumn(UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
										 FP_Column ** ppCol,
										 UT_sint32 * piXoff, UT_sint32 * piYoff) const;
	
protected:
	FL_ColumnLayout *		m_prev;
	FL_ColumnLayout *		m_next;
	FL_ColumnSetLayout *	m_pColumnSetLayout;
};

#endif /* FL_COLUMNLAYOUT_H */
