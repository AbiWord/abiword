 
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

#ifndef FL_COLUMNSETLAYOUT_H
#define FL_COLUMNSETLAYOUT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_ColumnLayout;
class FL_SectionLayout;
class PP_AttrProp;

class FL_ColumnSetLayout : public fl_Layout
{
public:
	FL_ColumnSetLayout(FL_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh);
	~FL_ColumnSetLayout();

	FL_SectionLayout *	getSectionLayout(void) const;
	FL_ColumnLayout *	getFirstColumnLayout(void) const;
	void				appendColumnLayout(FL_ColumnLayout * pCL);
	
	void				setColumnSetLayout(FL_ColumnSetLayout * pcsl);
	FL_ColumnSetLayout*	getColumnSetLayout(void) const;

protected:
	FL_SectionLayout *	m_pSectionLayout;
	FL_ColumnLayout *	m_pFirstColumnLayout;
	FL_ColumnLayout *	m_pLastColumnLayout;
};

#endif /* FL_COLUMNSETLAYOUT_H */
