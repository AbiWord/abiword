 
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
