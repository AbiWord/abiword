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



#ifndef COLUMN_H
#define COLUMN_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class fl_SectionLayout;
class fp_Line;
class fp_Page;
class PP_AttrProp;
class DG_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

// ----------------------------------------------------------------
/*
	TODO should columns have borders, margins, and padding, plus bg colors?
*/

class fp_Column
{
public:
	fp_Column(fl_SectionLayout* pSectionLayout);
	~fp_Column();

	void				setLeader(fp_Column*);
	void				setFollower(fp_Column*);
	void 				setNext(fp_Column*);
	void 				setPrev(fp_Column*);
	void				setPage(fp_Page*);
	
	fl_SectionLayout* 	getSectionLayout(void) const;
	
	fp_Column*			getLeader(void) const;
	fp_Column*			getFollower(void) const;
	fp_Column* 			getNext(void) const;
	fp_Column* 			getPrev(void) const;
	fp_Page*			getPage(void) const;

	UT_Bool				isEmpty(void) const;
	
	fp_Line*			getFirstLine(void) const;
	fp_Line*			getLastLine(void) const;
	
	UT_uint32			getWidth(void) const;
	void				setWidth(UT_uint32);
	
	UT_uint32			getHeight(void) const;
	UT_uint32			getMaxHeight(void) const;
	void				setMaxHeight(UT_uint32);
	void				setHeight(UT_uint32);

	void				checkForWidowsAndOrphans(void);
	UT_Bool				insertLineAfter(fp_Line* pNewLine, fp_Line*	pAfterLine, UT_sint32 iHeight);
	void				removeLine(fp_Line*);
	void				lineHeightChanged(fp_Line* pLine, DG_Graphics* pG, UT_sint32 iOldHeight, UT_sint32 iNewHeight);
	void				updateLayout(void);
	void 				moveLineToNextColumn(UT_uint32 iBump);
	void				moveLineToNextColumn(fp_Line* pLine);
	void				moveLineFromNextColumn(fp_Line* pLine);
	UT_sint32			getSpaceAtBottom(void) const;

	UT_uint32		 	getTopOffset(UT_uint32 iLineHeight);
	UT_Bool 			containsPoint(UT_sint32 x, UT_sint32 y);
	UT_uint32 			distanceFromPoint(UT_sint32 x, UT_sint32 y);

	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void		 		getOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff);
	void		 		getScreenOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);

	void				draw(dg_DrawArgs*);
	void 				dump();

	UT_sint32			getX(void) const;
	UT_sint32			getY(void) const;
	void				setX(UT_sint32);
	void				setY(UT_sint32);

protected:
	UT_sint32				_getMarginBeforeLine(fp_Line* pLine);
	UT_uint32 				_getBottomOfLastLine(void) const;
	void					_setNeedsLayoutUpdate(UT_Bool);

	UT_Bool					m_bNeedsLayout;

	fp_Page*				m_pPage;

	UT_uint32 				m_iWidth;
	UT_uint32 				m_iHeight;
	UT_uint32				m_iMaxHeight;

	UT_sint32				m_iX;
	UT_sint32				m_iY;

	UT_Vector				m_vecLines;
	
	fp_Column*				m_pNext;
	fp_Column*				m_pPrev;

	fp_Column*				m_pLeader;
	fp_Column*				m_pNextFollower;
	
	fl_SectionLayout*		m_pSectionLayout;

	DG_Graphics*			m_pG;
};

#endif /* COLUMN_H */
