/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef fl_ColumnSetLayout_H
#define fl_ColumnSetLayout_H

#include "ut_types.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class fl_ColumnLayout;
class fl_SectionLayout;
class PP_AttrProp;

class fl_ColumnSetLayout : public fl_Layout
{
public:
	fl_ColumnSetLayout(fl_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh);
	~fl_ColumnSetLayout();

	fl_SectionLayout *	getSectionLayout(void) const;
	fl_ColumnLayout *	getFirstColumnLayout(void) const;
	void				appendColumnLayout(fl_ColumnLayout * pCL);
	
	void				setColumnSetLayout(fl_ColumnSetLayout * pcsl);
	fl_ColumnSetLayout*	getColumnSetLayout(void) const;

protected:
	fl_SectionLayout *	m_pSectionLayout;
	fl_ColumnLayout *	m_pFirstColumnLayout;
	fl_ColumnLayout *	m_pLastColumnLayout;
};

#endif /* fl_ColumnSetLayout_H */
