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



#ifndef COLUMN_H
#define COLUMN_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class fl_SectionLayout;
class fp_SectionSlice;
class fp_BlockSlice;
class PP_AttrProp;
class DG_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

// ----------------------------------------------------------------
/*
	fp_Column is a section of a page into which text may be flowed.
	This class is abstract.
	Subclasses are used to define the shape of the column, which need not 
	actually be rectangular.

	A fp_Column consists of a list of fp_BlockSlice objects.
	fp_Column manages the vertical space between blocks by asking each block 
	for its space-before and space-after requirements.  It handles collapsing 
	vertical margins.

	fp_Column propogates layout changes by letting blocks know when they need 
	to do a re-layout.

	A fp_Column must be the direct child of a fp_SectionSlice.
*/

/*
	TODO should columns have borders, margins, and padding, plus bg colors?
*/

struct fp_BlockSliceInfo
{
	fp_BlockSliceInfo(fp_BlockSlice*);
	
	fp_BlockSlice*			pSlice;
	fp_BlockSliceInfo*		pNext;
	fp_BlockSliceInfo*		pPrev;
	
	UT_uint32				yoff;
};

class fp_Column
{
public:
	fp_Column(fl_SectionLayout*);
	virtual ~fp_Column();

	void				setSectionSlice(fp_SectionSlice*, void*);
	fp_SectionSlice*	getSectionSlice() const;

	void 				setNext(fp_Column*);
	void 				setPrev(fp_Column*);
	fp_Column* 			getNext();
	fp_Column* 			getPrev();
#if UNUSED
	fp_Column* 			getOrCreateNextColumn();
#endif

	int 				insertBlockSliceAfter(fp_BlockSlice* pBS, fp_BlockSlice*	pAfter, int iLineHeight);
	void				removeBlockSlice(fp_BlockSlice* pBS);
	UT_Bool 			requestSliver(fp_BlockSlice* pBS, UT_uint32 iAdditionalHeightNeeded,
									  UT_uint32* pX,
									  UT_uint32* pWidth,
									  UT_uint32* pHeight);
	UT_Bool				verifySliverFit(fp_BlockSlice* pBS, fp_Sliver* pSliver, UT_sint32 iY);
	void 				reportSliceHeightChanged(fp_BlockSlice* pBS, UT_uint32 iNewHeight);

	virtual UT_uint32 	getTopOffset(UT_uint32 iLineHeight) = 0;
	virtual UT_Bool 	containsPoint(UT_sint32 x, UT_sint32 y) = 0;
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y) = 0;

	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void				getOffsets(fp_BlockSlice*, void*, UT_sint32&, UT_sint32&);
	void				getScreenOffsets(fp_BlockSlice*, void*, UT_sint32&, UT_sint32&, UT_sint32&, UT_sint32&);

	void				draw(dg_DrawArgs*);
	void 				dump();

protected:
	virtual UT_uint32 		_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX) = 0;	
	fp_BlockSliceInfo*		_findSlice(fp_BlockSlice* p);
	int 					_repositionSlices();
	UT_uint32				_calcSliceOffset(fp_BlockSliceInfo*, UT_uint32);

	fp_SectionSlice*		m_pSectionSlice;
	void*					m_pSectionSliceData;
	
	fp_BlockSliceInfo*		m_pFirstSlice;

	fp_Column*				m_pNext;
	fp_Column*				m_pPrev;
	fl_SectionLayout*		m_pSectionLayout;

	DG_Graphics*			m_pG;
};

// ----------------------------------------------------------------
/*
	A fp_BoxColumn is a fp_Column which is rectangular in shape.  This is
	the most common type of fp_Column.
*/
class fp_BoxColumn : public fp_Column
{
public:
	fp_BoxColumn(fl_SectionLayout * pSL, const PP_AttrProp * pAP,
				 UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
				 UT_sint32 * piXoff, UT_sint32 * piYoff);
	virtual ~fp_BoxColumn();

	virtual UT_uint32 getTopOffset(UT_uint32 iLineHeight);
	virtual UT_Bool 	containsPoint(UT_sint32 x, UT_sint32 y);
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y);

	static const XML_Char * myTypeName(void);

protected:
	virtual UT_uint32 	_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX);
	
	UT_uint32 m_iWidth;
	UT_uint32 m_iHeight;
};

// ----------------------------------------------------------------
/*
	A fp_CircleColumn is a fp_Column which, surprisingly enough, is shaped like a circle.  
	We implemented this primarily so that we could show off our layout engine's
	ability to handle non-rectangular text flow.
*/
class fp_CircleColumn : public fp_Column
{
public:
	fp_CircleColumn(fl_SectionLayout * pSL, const PP_AttrProp * pAP,
					UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
					UT_sint32 * piXoff, UT_sint32 * piYoff);
	virtual ~fp_CircleColumn();

	virtual UT_uint32 getTopOffset(UT_uint32 iLineHeight);
	virtual UT_Bool 	containsPoint(UT_sint32 x, UT_sint32 y);
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y);

	static const XML_Char * myTypeName(void);
	
protected:
	virtual UT_uint32 	_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX);
	
	UT_uint32 m_iRadius;
};

#endif /* COLUMN_H */
