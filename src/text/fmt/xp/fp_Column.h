 
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


#ifndef COLUMN_H
#define COLUMN_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class FL_SectionLayout;
class FP_SectionSlice;
class FP_BlockSlice;
class PP_AttrProp;
class DG_Graphics;
struct dg_DrawArgs;
struct fp_Sliver;

// ----------------------------------------------------------------
/*
	FP_Column is a section of a page into which text may be flowed.
	This class is abstract.
	Subclasses are used to define the shape of the column, which need not 
	actually be rectangular.

	A FP_Column consists of a list of FP_BlockSlice objects.
	FP_Column manages the vertical space between blocks by asking each block 
	for its space-before and space-after requirements.  It handles collapsing 
	vertical margins.

	FP_Column propogates layout changes by letting blocks know when they need 
	to do a re-layout.

	A FP_Column must be the direct child of a FP_SectionSlice.
*/

/*
	TODO should columns have borders, margins, and padding, plus bg colors?
*/

struct fp_BlockSliceInfo
{
	fp_BlockSliceInfo(FP_BlockSlice*);
	
	FP_BlockSlice*			pSlice;
	fp_BlockSliceInfo*		pNext;
	fp_BlockSliceInfo*		pPrev;
	
	UT_uint32				yoff;
};

class FP_Column
{
public:
	FP_Column(FL_SectionLayout*);
	~FP_Column();

	void				setSectionSlice(FP_SectionSlice*, void*);
	FP_SectionSlice*	getSectionSlice() const;

	void 				setNext(FP_Column*);
	void 				setPrev(FP_Column*);
	FP_Column* 			getNext();
	FP_Column* 			getPrev();
#if UNUSED
	FP_Column* 			getOrCreateNextColumn();
#endif

	int 				insertBlockSliceAfter(FP_BlockSlice* pBS, FP_BlockSlice*	pAfter, int iLineHeight);
	void				removeBlockSlice(FP_BlockSlice* pBS);
	UT_Bool 			requestSliver(FP_BlockSlice* pBS, UT_uint32 iAdditionalHeightNeeded,
									  UT_uint32* pX,
									  UT_uint32* pWidth,
									  UT_uint32* pHeight);
	UT_Bool				verifySliverFit(FP_BlockSlice* pBS, fp_Sliver* pSliver, UT_sint32 iY);
	void 				reportSliceHeightChanged(FP_BlockSlice* pBS, UT_uint32 iNewHeight);

	virtual UT_uint32 	getTopOffset(UT_uint32 iLineHeight) = 0;
	virtual UT_Bool 	containsPoint(UT_sint32 x, UT_sint32 y) = 0;
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y) = 0;

	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bRight);
	void				getOffsets(FP_BlockSlice*, void*, UT_sint32&, UT_sint32&);
	void				getScreenOffsets(FP_BlockSlice*, void*, UT_sint32&, UT_sint32&, UT_sint32&, UT_sint32&);

	void				draw(dg_DrawArgs*);
	void 				dump();

protected:
	virtual UT_uint32 		_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX) = 0;	
	fp_BlockSliceInfo*		_findSlice(FP_BlockSlice* p);
	int 					_repositionSlices();
	UT_uint32				_calcSliceOffset(fp_BlockSliceInfo*, UT_uint32);

	FP_SectionSlice*		m_pSectionSlice;
	void*					m_pSectionSliceData;
	
	fp_BlockSliceInfo*		m_pFirstSlice;

	FP_Column*				m_pNext;
	FP_Column*				m_pPrev;
	FL_SectionLayout*		m_pSectionLayout;

	DG_Graphics*			m_pG;
};

// ----------------------------------------------------------------
/*
	A FP_BoxColumn is a FP_Column which is rectangular in shape.  This is
	the most common type of FP_Column.
*/
class FP_BoxColumn : public FP_Column
{
public:
	FP_BoxColumn(FL_SectionLayout * pSL, const PP_AttrProp * pAP,
				 UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
				 UT_sint32 * piXoff, UT_sint32 * piYoff);

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
	A FP_CircleColumn is a FP_Column which, surprisingly enough, is shaped like a circle.  
	We implemented this primarily so that we could show off our layout engine's
	ability to handle non-rectangular text flow.
*/
class FP_CircleColumn : public FP_Column
{
public:
	FP_CircleColumn(FL_SectionLayout * pSL, const PP_AttrProp * pAP,
					UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
					UT_sint32 * piXoff, UT_sint32 * piYoff);

	virtual UT_uint32 getTopOffset(UT_uint32 iLineHeight);
	virtual UT_Bool 	containsPoint(UT_sint32 x, UT_sint32 y);
	virtual UT_uint32 	distanceFromPoint(UT_sint32 x, UT_sint32 y);

	static const XML_Char * myTypeName(void);
	
protected:
	virtual UT_uint32 	_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX);
	
	UT_uint32 m_iRadius;
};

#endif /* COLUMN_H */
