/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
