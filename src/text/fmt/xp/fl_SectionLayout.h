 
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


#ifndef SECTIONLAYOUT_H
#define SECTIONLAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_DocLayout;
class FL_ColumnSetLayout;
class FL_BlockLayout;
class FB_LineBreaker;
class FP_Column;
class PD_Document;
class PP_AttrProp;

/*
	A section keeps track of all of its columns, as well as all of its
	section slices.
*/
class FL_SectionLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	FL_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh);
	~FL_SectionLayout();

	FL_DocLayout *		getLayout();
	FP_Column *			getNewColumn();
	int					format();
	UT_Bool				reformat();

	void				setColumnSetLayout(FL_ColumnSetLayout * pcsl);
	FL_ColumnSetLayout*	getColumnSetLayout(void) const;

	FL_BlockLayout *	getFirstBlock(void) const;
	FL_BlockLayout *	insertBlock(PL_StruxDocHandle sdh, FL_BlockLayout * pPrev);
	FL_BlockLayout *	appendBlock(PL_StruxDocHandle sdh);

protected:
	void				_purgeLayout();
	FB_LineBreaker *	_getLineBreaker(void);

	FL_DocLayout*		m_pLayout;
	FB_LineBreaker*		m_pLB;
	FL_ColumnSetLayout*	m_pColumnSetLayout;

	FL_BlockLayout*		m_pFirstBlock;
	FL_BlockLayout*		m_pLastBlock;
	
	UT_Vector			m_vecSlices;
	UT_Vector			m_vecColumns;
};

#endif /* SECTIONLAYOUT_H */
