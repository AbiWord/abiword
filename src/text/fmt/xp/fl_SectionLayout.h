 
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

class FL_DocLayout;
class FB_LineBreaker;
class FP_Column;
class PD_Document;

/*
	A section keeps track of all of its columns, as well as all of its
	section slices.
*/
class FL_SectionLayout
{
public:
	FL_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh);
	~FL_SectionLayout();

	FL_DocLayout* getLayout();
	FP_Column* getNewColumn();
	int format();
	UT_Bool reformat();

protected:
	void				_purgeLayout();

	PL_StruxDocHandle	m_sdh;

	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	FB_LineBreaker*		m_pLB;
	
	UT_Vector			m_vecSlices;
	UT_Vector			m_vecColumns;
};

#endif /* SECTIONLAYOUT_H */
