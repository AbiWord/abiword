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



#ifndef SECTIONSLICE_H
#define SECTIONSLICE_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class fp_Page;
class fp_Column;
class DG_Graphics;
struct dg_DrawArgs;

struct fp_ColumnInfo
{
	fp_ColumnInfo(fp_Column*, UT_sint32, UT_sint32);
	fp_Column*		pColumn;
	UT_uint32 xoff;
	UT_uint32 yoff;
};

class fp_SectionSlice
{
public:
	fp_SectionSlice(UT_sint32 iWidth, UT_sint32 iHeight);
	~fp_SectionSlice();

	void		setPage(fp_Page*, void*);
	fp_Page*	getPage() const;
	UT_sint32 	getWidth();
	UT_sint32 	getHeight();
	fp_Column* 	getFirstColumn();
	void 		addColumn(fp_Column*, UT_sint32, UT_sint32);

	void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bEOL);
	void 		getOffsets(fp_Column* pCol, void* p, UT_sint32& xoff, UT_sint32& yoff);
	void 		getScreenOffsets(fp_Column* pCol, void* p, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);	

	void		draw(dg_DrawArgs*);
	void		dump();

protected:
	fp_Page*	m_pPage;
	void*		m_pPageData;
	
	UT_sint32	m_iWidth;
	UT_sint32	m_iHeight;
	UT_Vector	m_vecColumnInfos;
};

#endif /* SECTIONSLICE_H */

