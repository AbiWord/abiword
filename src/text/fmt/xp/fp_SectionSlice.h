 
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


#ifndef SECTIONSLICE_H
#define SECTIONSLICE_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class FP_Page;
class FP_Column;
class DG_Graphics;
struct dg_DrawArgs;

struct fp_ColumnInfo
{
	fp_ColumnInfo(FP_Column*, UT_sint32, UT_sint32);
	FP_Column*		pColumn;
	UT_uint32 xoff;
	UT_uint32 yoff;
};

class FP_SectionSlice
{
public:
	FP_SectionSlice(UT_sint32 iWidth, UT_sint32 iHeight);
	~FP_SectionSlice();

	void		setPage(FP_Page*, void*);
	FP_Page*	getPage() const;
	UT_sint32 	getWidth();
	UT_sint32 	getHeight();
	FP_Column* 	getFirstColumn();
	void 		addColumn(FP_Column*, UT_sint32, UT_sint32);

	void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bRight);
	void 		getOffsets(FP_Column* pCol, void* p, UT_sint32& xoff, UT_sint32& yoff);
	void 		getScreenOffsets(FP_Column* pCol, void* p, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);	

	void		draw(dg_DrawArgs*);
	void		dump();

protected:
	FP_Page*	m_pPage;
	void*		m_pPageData;
	
	UT_sint32	m_iWidth;
	UT_sint32	m_iHeight;
	UT_Vector	m_vecColumnInfos;
};

#endif /* SECTIONSLICE_H */

