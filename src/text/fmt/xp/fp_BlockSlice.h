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



#ifndef BLOCKSLICE_H
#define BLOCKSLICE_H

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class fl_BlockLayout;
class fp_Column;
class fp_Line;
class DG_Graphics;
struct dg_DrawArgs;

/*
	A fp_BlockSlice is a slice of a fl_BlockLayout.  
	A fp_BlockSlice exists on only one fp_Column.
*/
struct fp_LineInfo
{
	fp_LineInfo(fp_Line*, UT_sint32, UT_sint32, UT_sint32);
	fp_Line*	pLine;
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

class fp_BlockSlice
{
	/* 
		A fp_BlockSlice grows by requesting a sliver of space from its column.
		Each time it needs more room for a line, it requests an fp_Sliver.
		The shape of the fp_BlockSlice is defined by its slivers.
		A fp_BlockSlice does not know its actual Y position, or the actual
		Y position of any of its slivers.  Its size and shape are stored
		by simply remembering each sliver it has been granted by the fp_Column.
		For each sliver, it only knows the height and width of that sliver,
		as well as the X coord, for drawing purposes.  The Y coord, relative
		to the upper-left corner of the fp_BlockSlice, can be calculated on 
		the fly by adding up heights.

		The fp_Column knows the Y coordinate of the first sliver in any 
		fp_BlockSlice.  If the fp_BlockSlice needs to move up or down, the 
		fp_Column can iterate over all the slivers and verify that the new 
		place is wide enough for them all.

		In fact, a fp_BlockSlice could move down such that not all of its 
		slivers fit any more.  This will force the fl_BlockLayout to be 
		reformatted.

		When a fp_Column realizes that a fp_BlockSlice needs to move, then 
		it iterates over all of its slivers to see if they still fit.  For 
		each sliver, it calculates the current Y position, relative to the 
		column, for that sliver, and checks the width of the column at that 
		point, to see if the sliver is still the correct size even after it 
		has been moved.  It also checks to make sure that slivers have not
		bumped off the end.

		If any slivers don't fit anymore, then we need to re-layout the block.  
		This happens even if the block was already split.  We cannot 
		arbitrarily move slivers from one blockslice to the next, since the 
		fl_BlockLayout is responsible for managing widow/orphan control, not 
		the column.
	*/
public:
	fp_BlockSlice(fl_BlockLayout*);
	~fp_BlockSlice();

	void 				setColumn(fp_Column*, void*);
	fp_Column*			getColumn();

	fl_BlockLayout* 	getBlock();
	UT_Bool 			isFirstSliceInBlock(void);
	UT_Bool 			isLastSliceInBlock(void);
	UT_uint32 			getHeight();

	int 				countSlivers();
	fp_Sliver* 			getNthSliver(int n);
	fp_Sliver* 			addSliver(UT_uint32, UT_uint32, UT_uint32);

	UT_uint32			countLines();
	fp_Line*			getNthLine(UT_uint32);
	void				verifyColumnFit();
	void				returnExtraSpace();

	UT_uint32			requestLineSpace(UT_uint32 iHeight);	// TODO should be called only by fl_BlockLayout.  (friend)
	int					addLine(fp_Line*);// TODO should be called only by fl_BlockLayout.  (friend)

	void				removeLine(fp_Line* pLine, void* p);
	void 				deleteLines();

	void				mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void		 		getOffsets(fp_Line* pLine, void* p, UT_sint32& xoff, UT_sint32& yoff);
	void		 		getScreenOffsets(fp_Line* pLine, void* p, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);

	void				align();
	void				alignOneLine(fp_Line* pLine, void* p);
	void 				alignOneLine(fp_LineInfo* pLI);

	void				draw(DG_Graphics*);
	void				draw(dg_DrawArgs*);
	void				clearScreen(DG_Graphics*);
	void				remove();

	void 				dump();

protected:
	UT_Vector			m_vecSlivers;
	UT_Vector			m_vecLineInfos;
	fp_Column*			m_pColumn;
	void*				m_pColumnData;
	fl_BlockLayout*		m_pBlock;
	UT_uint32 			m_iHeight;
	UT_uint32 			m_iTotalLineHeight;
};

#endif /* BLOCKSLICE_H */
