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
#include "fp_Page.h"

class fl_HdrFtrSectionLayout;
class fl_DocSectionLayout;
class fl_SectionLayout;
class fp_Line;
class fp_Page;
class PP_AttrProp;
class GR_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

#define FP_CONTAINER_COLUMN		1
#define FP_CONTAINER_HDRFTR		2

class fp_Container
{
public:
	fp_Container(UT_uint32 iType, fl_SectionLayout* pSectionLayout);
	~fp_Container();

	inline UT_uint32	getType(void) const { return m_iType; }
	
	void				setPage(fp_Page*);
	void				setWidth(UT_sint32);
	void				setWidthInLayoutUnits(UT_sint32);
	void				setMaxHeight(UT_sint32);
	void				setMaxHeightInLayoutUnits(UT_sint32);
	void				setHeight(UT_sint32);
	void				setX(UT_sint32);
	void				setY(UT_sint32);
	
	inline				UT_sint32			getMaxHeight(void) const 		{ return m_iMaxHeight; }
	inline				UT_sint32			getMaxHeightInLayoutUnits(void) const 	{ return m_iMaxHeightLayoutUnits; }
	inline				UT_sint32			getWidth(void) const 			{ return m_iWidth; }
	inline				UT_sint32			getWidthInLayoutUnits(void) const 	{ UT_ASSERT(m_iWidthLayoutUnits); return m_iWidthLayoutUnits; }
	inline				UT_sint32			getX(void) const 				{ return m_iX; }
	inline				UT_sint32			getY(void) const				{ return m_iY; }
	inline				fp_Page*			getPage(void) const				{ return m_pPage; }
	inline				fl_SectionLayout*	getSectionLayout(void) const	{ return m_pSectionLayout; }
	inline				UT_sint32			getHeight(void) const			{ return m_iHeight; }
	inline				UT_sint32			getHeightInLayoutUnits(void) const	{ return m_iHeightLayoutUnits; }

	inline				UT_sint32			getColumnGap(void) const			{ return m_pPage->getColumnGap(); }


	fp_Line*			getFirstLine(void) const;
	fp_Line*			getLastLine(void) const;
	
	UT_Bool				insertLineAfter(fp_Line* pNewLine, fp_Line*	pAfterLine);
	UT_Bool				insertLine(fp_Line*);
	UT_Bool				addLine(fp_Line*);
	void				removeLine(fp_Line*);
	UT_Bool				isEmpty(void) const;
	
	UT_Bool 			containsPoint(UT_sint32 x, UT_sint32 y);
	UT_uint32 			distanceFromPoint(UT_sint32 x, UT_sint32 y);

	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void		 		getOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff);
	void		 		getScreenOffsets(fp_Line* pLine, UT_sint32& xoff, UT_sint32& yoff);

	void				draw(dg_DrawArgs*);
	void				clearScreen(void);

protected:
	UT_uint32				m_iType;
	
	fp_Page*				m_pPage;

	UT_sint32 				m_iWidth;
	UT_sint32 				m_iWidthLayoutUnits;
	UT_sint32 				m_iHeight;
	UT_sint32				m_iMaxHeight;
	UT_sint32 				m_iHeightLayoutUnits;
	UT_sint32				m_iMaxHeightLayoutUnits;

	UT_sint32				m_iX;
	UT_sint32				m_iY;

	UT_Vector				m_vecLines;
	
	fl_SectionLayout*		m_pSectionLayout;

	GR_Graphics*			m_pG;

    void                    _drawBoundaries(dg_DrawArgs* pDA);
};

class fp_Column : public fp_Container
{
public:
	fp_Column(fl_SectionLayout* pSectionLayout);
	~fp_Column();

	void				setLeader(fp_Column*);
	void				setFollower(fp_Column*);
	void 				setNext(fp_Column*);
	void 				setPrev(fp_Column*);

	fl_DocSectionLayout*	getDocSectionLayout(void) const;
	
	inline				fp_Column*			getLeader(void) const 			{ return m_pLeader; }
	inline				fp_Column*			getFollower(void) const 		{ return m_pNextFollower; }
	inline				fp_Column*			getNext(void) const				{ return m_pNext; }
	inline				fp_Column*			getPrev(void) const				{ return m_pPrev; }

	void				layout(void);
	
	void 				bumpLines(fp_Line* pLastLineToKeep);
	
protected:
	UT_uint32 				_getBottomOfLastLine(void) const;

	fp_Column*				m_pNext;
	fp_Column*				m_pPrev;

	fp_Column*				m_pLeader;
	fp_Column*				m_pNextFollower;
};

class fp_HdrFtrContainer : public fp_Container
{
public:
	fp_HdrFtrContainer(UT_sint32 iX, UT_sint32 iY, 
							UT_sint32 iWidth, UT_sint32 iHeight,
							UT_sint32 iWidthLayout, UT_sint32 iHeightLayout, 
							fl_SectionLayout* pSL);
	~fp_HdrFtrContainer();

	fl_HdrFtrSectionLayout*	getHdrFtrSectionLayout(void) const;
	
	void				layout(void);
	
protected:

};

#endif /* COLUMN_H */
