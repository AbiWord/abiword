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



#ifndef SECTIONLAYOUT_H
#define SECTIONLAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_DocLayout;
class fl_ColumnSetLayout;
class fl_BlockLayout;
class fb_LineBreaker;
class fp_Column;
class PD_Document;
class PP_AttrProp;

/*
	A section keeps track of all of its columns, as well as all of its
	section slices.
*/
class fl_SectionLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh);
	~fl_SectionLayout();

	FL_DocLayout *		getLayout();
	fp_Column *			getNewColumn();
	int					format();
	UT_Bool				reformat();

	void				setColumnSetLayout(fl_ColumnSetLayout * pcsl);
	fl_ColumnSetLayout*	getColumnSetLayout(void) const;

	fl_BlockLayout *	getFirstBlock(void) const;
	fl_BlockLayout *	appendBlock(PL_StruxDocHandle sdh);
	fl_BlockLayout *	insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev);
	fl_BlockLayout *	removeBlock(fl_BlockLayout * pBL);

protected:
	void				_purgeLayout();
	fb_LineBreaker *	_getLineBreaker(void);

	FL_DocLayout*		m_pLayout;
	fb_LineBreaker*		m_pLB;
	fl_ColumnSetLayout*	m_pColumnSetLayout;

	fl_BlockLayout*		m_pFirstBlock;
	fl_BlockLayout*		m_pLastBlock;
	
	UT_Vector			m_vecSlices;
	UT_Vector			m_vecColumns;
};

#endif /* SECTIONLAYOUT_H */
