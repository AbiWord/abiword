 
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


#ifndef BLOCKSLICE_H
#define BLOCKSLICE_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class FL_BlockLayout;
class FP_Column;
class FP_Line;
class DG_Graphics;
struct dg_DrawArgs;

/*
	A FP_BlockSlice is a slice of a FL_BlockLayout.  
	A FP_BlockSlice exists on only one FP_Column.
*/
struct fp_LineInfo
{
	fp_LineInfo(FP_Line*, UT_sint32, UT_sint32, UT_sint32);
	FP_Line*	pLine;
	UT_sint32 base_xoff;
	UT_sint32 xoff;
	UT_sint32 yoff;
};

struct fp_Sliver
{
public:
	UT_uint32 iX;
	UT_uint32 iWidth;
	UT_uint32 iHeight;
};

class FP_BlockSlice
{
	/* 
		A FP_BlockSlice grows by requesting a sliver of space from its column.
		Each time it needs more room for a line, it requests an fp_Sliver.
		The shape of the FP_BlockSlice is defined by its slivers.
		A FP_BlockSlice does not know its actual Y position, or the actual
		Y position of any of its slivers.  Its size and shape are stored
		by simply remembering each sliver it has been granted by the FP_Column.
		For each sliver, it only knows the height and width of that sliver,
		as well as the X coord, for drawing purposes.  The Y coord, relative
		to the upper-left corner of the FP_BlockSlice, can be calculated on 
		the fly by adding up heights.

		The FP_Column knows the Y coordinate of the first sliver in any 
		FP_BlockSlice.  If the FP_BlockSlice needs to move up or down, the 
		FP_Column can iterate over all the slivers and verify that the new 
		place is wide enough for them all.

		In fact, a FP_BlockSlice could move down such that not all of its 
		slivers fit any more.  This will force the FL_BlockLayout to be 
		reformatted.

		When a FP_Column realizes that a FP_BlockSlice needs to move, then 
		it iterates over all of its slivers to see if they still fit.  For 
		each sliver, it calculates the current Y position, relative to the 
		column, for that sliver, and checks the width of the column at that 
		point, to see if the sliver is still the correct size even after it 
		has been moved.  It also checks to make sure that slivers have not
		bumped off the end.

		If any slivers don't fit anymore, then we need to re-layout the block.  
		This happens even if the block was already split.  We cannot 
		arbitrarily move slivers from one blockslice to the next, since the 
		FL_BlockLayout is responsible for managing widow/orphan control, not 
		the column.
	*/
public:
	FP_BlockSlice(FL_BlockLayout*);
	~FP_BlockSlice();

	void 				setColumn(FP_Column*, void*);
	FP_Column*			getColumn();

	FL_BlockLayout* 	getBlock();
	UT_Bool 			isFirstSliceInBlock(void);
	UT_Bool 			isLastSliceInBlock(void);
	UT_uint32 			getHeight();

	int 				countSlivers();
	fp_Sliver* 			getNthSliver(int n);
	fp_Sliver* 			addSliver(UT_uint32, UT_uint32, UT_uint32);

	UT_uint32			countLines();
	FP_Line*			getNthLine(UT_uint32);
	void				verifyColumnFit();
	void				returnExtraSpace();

	UT_uint32			requestLineSpace(UT_uint32 iHeight);	// TODO should be called only by FL_BlockLayout.  (friend)
	int					addLine(FP_Line*);// TODO should be called only by FL_BlockLayout.  (friend)

	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bRight);
	void		 		getOffsets(FP_Line* pLine, void* p, UT_sint32& xoff, UT_sint32& yoff);
	void		 		getScreenOffsets(FP_Line* pLine, void* p, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);

	void				align();
	void				alignOneLine(FP_Line* pLine, void* p);
	void 				alignOneLine(fp_LineInfo* pLI);

	void				draw(DG_Graphics*);
	void				draw(dg_DrawArgs*);
	void				clearScreen(DG_Graphics*);
	void				remove();
	void 				dump();
	void 				deleteLines();

protected:
	UT_Vector			m_vecSlivers;
	UT_Vector			m_vecLineInfos;
	FP_Column*			m_pColumn;
	void*				m_pColumnData;
	FL_BlockLayout*		m_pBlock;
	UT_uint32 			m_iHeight;
	UT_uint32 			m_iTotalLineHeight;
};

#endif /* BLOCKSLICE_H */
