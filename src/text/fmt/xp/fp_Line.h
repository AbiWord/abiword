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

class fp_Run;
class DG_Graphics;
class fl_BlockLayout;
class fp_Column;

struct dg_DrawArgs;

// ----------------------------------------------------------------
/*
	fp_Line represents a single line.  A fp_Line is a collection of 
	Runs.
*/

class fp_Line
{
public:
	fp_Line();
	~fp_Line();

	inline fp_Column* 			getColumn(void) const		{ return m_pColumn; }
	inline fl_BlockLayout*		getBlock(void) const 		{ return m_pBlock; }
	inline UT_sint32 			getHeight(void) const 		{ return m_iHeight; }
	
	inline UT_sint32			getX(void) const 			{ return m_iX; }
	inline UT_sint32			getY(void) const 			{ return m_iY; }
	
	inline fp_Line*				getNext(void) const 		{ return m_pNext; }
	inline fp_Line*    			getPrev(void) const 		{ return m_pPrev; }

//	inline UT_sint32 			getWidth(void) const	 	{ return m_iWidth; }
	inline UT_sint32			getMaxWidth(void) const 	{ return m_iMaxWidth; }
	inline UT_sint32			getAscent(void) const 		{ return m_iAscent; }
	
	void		setMaxWidth(UT_sint32);
	void		setX(UT_sint32);
	void		setY(UT_sint32);
	void		setNext(fp_Line*);
	void        setPrev(fp_Line*);
	void		setColumn(fp_Column*);
	void		setBlock(fl_BlockLayout*);

	void 		addRun(fp_Run*);
	void		insertRunAfter(fp_Run* pRun1, fp_Run* pRun2);
	void		insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore);
	void        insertRun(fp_Run*);
    UT_Bool     removeRun(fp_Run*);
	
	int 		countRuns(void) const;
	fp_Run*     getFirstRun(void) const;
	fp_Run*     getLastRun(void) const;
	UT_Bool 	isFirstLineInBlock(void) const;
	UT_Bool 	isLastLineInBlock(void) const;
	
	void		remove(void);
	UT_sint32	getMarginBefore(void) const;
	UT_sint32	getMarginAfter(void) const;

	void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void		getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff);
	void		getScreenOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff);

	void		clearScreen(void);
	void		draw(dg_DrawArgs*);
	void        draw(DG_Graphics*);
	void		align(void);
	void		layout(void);
	UT_Bool		recalculateFields(void);
	void		recalcHeight();
	
	UT_Bool		isEmpty(void) const;
	UT_Bool		findNextTabStop(UT_sint32 iStartX, UT_sint32& iPosition, unsigned char& iType);
	
protected:
	fl_BlockLayout*	m_pBlock;
	fp_Column*		m_pColumn;
	
	UT_sint32	 	m_iWidth;
	UT_sint32	 	m_iMaxWidth;
	UT_sint32 		m_iHeight;
	UT_sint32 		m_iAscent;

	UT_sint32		m_iX;
	UT_sint32		m_iY;
	
	UT_Vector		m_vecRuns;

	fp_Line*		m_pNext;
	fp_Line*        m_pPrev;
};

#endif /* LINE_H */


