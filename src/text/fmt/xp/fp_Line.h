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



#ifndef LINE_H
#define LINE_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class fp_BlockSlice;
class fp_Run;
class DG_Graphics;
struct dg_DrawArgs;

// ----------------------------------------------------------------
/*
	fp_Line represents a single line.  A fp_Line is a collection of 
	Runs.
*/

struct fp_RunInfo
{
	fp_RunInfo(fp_Run*);

	fp_Run*	pRun;
	UT_uint32 xoff;
	UT_uint32 yoff;
};

class fp_Line
{
public:
	fp_Line(UT_sint32);
	~fp_Line();

	void		setBlockSlice(fp_BlockSlice*, void*);
	fp_BlockSlice* getBlockSlice() const;

	UT_uint32 	getHeight() const;
	UT_uint32 	getWidth() const;
	UT_uint32 	getMaxWidth() const;

	fp_Line*	getNext() const;
	void		setNext(fp_Line*);
	void        setPrev(fp_Line*);
	fp_Line*    getPrev() const;

	void 		addRun(fp_Run*);
	void		splitRunInLine(fp_Run* pRun1, fp_Run* pRun2);
	void        insertRun(fp_Run*, UT_Bool bClear, UT_Bool bNewData = UT_FALSE);
    UT_Bool     removeRun(fp_Run*);
	int 		countRuns() const;
	fp_Run*     getFirstRun() const;
	fp_Run*     getLastRun() const;
	UT_uint32	getNumChars() const;
 	void        runSizeChanged(void*, UT_sint32 oldWidth, UT_sint32 newWidth);
	void		remove();

	void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void		getOffsets(fp_Run* pRun, void* p, UT_sint32& xoff, UT_sint32& yoff);
	void		getScreenOffsets(fp_Run* pRun, void* p, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height, UT_Bool bLineHeight=UT_FALSE);
#if UNUSED
	void		getAbsoluteCoords(UT_sint32& x, UT_sint32& y);
#endif

	void        shrink(UT_sint32);
	void 		expandWidthTo(UT_uint32 iNewWidth);
	void		clearScreen();
	void		draw(dg_DrawArgs*);
	void        draw(DG_Graphics*);
	void		align();
	UT_uint32	getAscent(void) const;
	
	void		dumpRunInfo(const fp_Run* pRun, void* p);

	UT_Bool		isEmpty(void) const;
	
	UT_Bool         m_bDirty;

protected:
	void			_recalcHeight();

	fp_BlockSlice*	m_pBlockSlice;
	void*			m_pBlockSliceData;
	
	UT_uint32	 	m_iWidth;
	UT_uint32	 	m_iMaxWidth;
	UT_uint32 		m_iHeight;
	UT_uint32 		m_iAscent;
	UT_Vector		m_vecRunInfos;

	fp_Line*		m_pNext;
	fp_Line*        m_pPrev;
};

#endif /* LINE_H */


