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



#ifndef PAGE_H
#define PAGE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class FL_DocLayout;
class fl_SectionLayout;
class fp_SectionSlice;
class FV_View;
class DG_Graphics;
struct dg_DrawArgs;

// ----------------------------------------------------------------
/*
	fp_Page is a concrete class used to represent a page.  A fp_Page manages
	a group of children, each of which must be a fp_SectionSlice.
*/
struct fp_SectionSliceInfo
{
	fp_SectionSliceInfo(fp_SectionSlice*, UT_uint32, UT_uint32);
	fp_SectionSlice*		pSlice;
	UT_uint32 xoff;
	UT_uint32 yoff;
};

class fp_Page
{
 public:
	fp_Page(FL_DocLayout*,
			FV_View*,
			UT_uint32 iWidth,
			UT_uint32 iHeight,
			UT_uint32 iLeft,
			UT_uint32 iTop,
			UT_uint32 iRight,
			UT_uint32 iBottom);
	~fp_Page();

	UT_Bool 		requestSpace(fl_SectionLayout*, fp_SectionSlice** si);

	int				getWidth();
	int				getHeight();
	fp_Page*		getNext();
	void			setNext(fp_Page*);
	FL_DocLayout*	getLayout();
	void            setView(FV_View*);

	void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void			getOffsets(fp_SectionSlice*, void*, UT_sint32& xoff, UT_sint32& yoff);
	void			getScreenOffsets(fp_SectionSlice*, void*, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);

	void			draw(dg_DrawArgs*);

#ifdef FMT_TEST
	void			__dump(FILE * fp) const;
#endif
	
protected:
	FL_DocLayout*		m_pLayout;
	FV_View*			m_pView;
	fp_Page*			m_pNext;

	UT_sint32			m_iWidth;
	UT_sint32			m_iHeight;
	UT_sint32			m_iLeft;
	UT_sint32			m_iTop;
	UT_sint32			m_iRight;
	UT_sint32			m_iBottom;

	UT_Vector			m_vecSliceInfos;
};

#endif /* PAGE_H */
