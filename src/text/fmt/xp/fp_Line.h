/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef FP_LINE_H
#define FP_LINE_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_BlockLayout.h"
#include "fp_Column.h"
//#include "fmt_Types.h"

class fp_Run;
class GR_Graphics;
class fp_Container;

struct dg_DrawArgs;

// ----------------------------------------------------------------
/*
	fp_Line represents a single line.  A fp_Line is a collection of 
	Runs.
*/
#ifdef BIDI_ENABLED
//these are the initial sizes of the temp buffers
#define TEMP_LINE_BUFFER_SIZE 512
#define RUNS_MAP_SIZE         80
/* the following define determines whether the map used to convert logical to visual position and
   vice versa is shared by all lines, or whether each line has map of its own. In the former case
   only small encrease in memory requirenments is needed, but we have to recalculate the map
   more often; in the latter case we only need to recalculate the map when a run
   is added or deleted, but the increase in memory requirenments is significant:
   sizeof(UT_uint32) * run_count * no_of_lines

   there is a mechanism in place that makes sure that the static map gets calculated only when it
   is really necessary and from my debugging experience this is not much more often that in the
   case of the non-static map, so this appears to be definitely the better option;
   PLEASE NOTE that if you change the option you will have to rebuild.
*/
#define USE_STATIC_MAP

//#define FP_LINE_DIRECTION_USED_NONE 0X00
//#define FP_LINE_DIRECTION_USED_LTR	0X01
//#define FP_LINE_DIRECTION_USED_RTL  0x02
//#define FP_LINE_DIRECTION_USED_BOTH 0x04

#endif //BIDI_ENABLED


class fp_Line
{
public:
	fp_Line();
	~fp_Line();

	inline fp_Container* 		getContainer(void) const	{ return m_pContainer; }
	inline fl_BlockLayout*		getBlock(void) const 		{ return m_pBlock; }
	inline UT_sint32 			getHeight(void) const 		{ return m_iHeight; }
	inline UT_sint32 			getHeightInLayoutUnits(void) const 		{ return m_iHeightLayoutUnits; }
	
	inline UT_sint32			getX(void) const 			{ return m_iX; }
	inline UT_sint32			getXInLayoutUnits(void) const 	{ return m_iXLayoutUnits; }
	inline UT_sint32			getY(void) const 			{ return m_iY; }
	
	inline fp_Line*				getNext(void) const 		{ return m_pNext; }
	inline fp_Line*    			getPrev(void) const 		{ return m_pPrev; }

//	inline UT_sint32 			getWidth(void) const	 	{ return m_iWidth; }
	inline UT_sint32			getMaxWidth(void) const 	{ return m_iMaxWidth; }
	inline UT_sint32			getMaxWidthInLayoutUnits(void) const 	{ UT_ASSERT(m_iMaxWidthLayoutUnits); return m_iMaxWidthLayoutUnits; }
	inline UT_sint32			getAscent(void) const 		{ return m_iAscent; }
	inline UT_sint32			getDescent(void) const 		{ return m_iDescent; }

	inline UT_sint32			getColumnGap(void) const	{ return m_pContainer->getColumnGap(); }
	
	void				setMaxWidth(UT_sint32);
	void				setMaxWidthInLayoutUnits(UT_sint32);
	void				setX(UT_sint32);
	void				setXInLayoutUnits(UT_sint32);
	void				setY(UT_sint32);
	void				setYInLayoutUnits(UT_sint32);
	inline	void		setNext(fp_Line * p)				{ m_pNext = p; }
	inline	void        setPrev(fp_Line * p)				{ m_pPrev = p; }
	void				setContainer(fp_Container*);
	inline	void		setBlock(fl_BlockLayout * pBlock)	{ m_pBlock = pBlock; }

	fp_Line*	getNextLineInSection(void) const;
	fp_Line*	getPrevLineInSection(void) const;

	bool		containsForcedColumnBreak(void) const;
	bool		containsForcedPageBreak(void) const;
	
	void 		addRun(fp_Run*);
	void		insertRunAfter(fp_Run* pRun1, fp_Run* pRun2);
	void		insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore);
	void        insertRun(fp_Run*);
    bool     removeRun(fp_Run*, bool bTellTheRunAboutIt=false);
	
	inline	bool		isEmpty(void) const				{ return ((m_vecRuns.getItemCount()) == 0); }
	inline	int 		countRuns(void) const			{ return m_vecRuns.getItemCount(); }
	inline	fp_Run*     getFirstRun(void) const			{ return ((fp_Run*) m_vecRuns.getFirstItem()); }
        fp_Run*     getLastRun(void) const ;

	inline	bool 	isFirstLineInBlock(void) const	{ return (m_pBlock->getFirstLine() == this); }
	inline	bool 	isLastLineInBlock(void) const	{ return (m_pBlock->getLastLine() == this); }
	
	void		remove(void);
	UT_sint32	getMarginBefore(void) const;
	UT_sint32	getMarginAfter(void) const;
	UT_sint32	getMarginAfterInLayoutUnits(void) const;

	void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	void		getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff);
	void		getScreenOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff);

	void		clearScreen(void);
	void		clearScreenFromRunToEnd(UT_uint32 runIndex);
	void		draw(dg_DrawArgs*);
	void        draw(GR_Graphics*);
	void		align(void);
	void		layout(void);
	bool		recalculateFields(void);
	void		recalcHeight();
	void		recalcMaxWidth();
	void		coalesceRuns(void);

	UT_sint32	calculateWidthOfLine(void);
	UT_sint32	calculateWidthOfLineInLayoutUnits(void);
	UT_sint32	calculateWidthOfTrailingSpaces(void);
	UT_sint32	calculateWidthOfTrailingSpacesInLayoutUnits(void);
	void		resetJustification();
	void		distributeJustificationAmongstSpaces(UT_sint32 iAmount);
	UT_uint32	countJustificationPoints(void) const;
	void		splitRunsAtSpaces(void);

	bool		isLastCharacter(UT_UCSChar Character) const;

	bool		findNextTabStop(UT_sint32 iStartX, UT_sint32& iPosition, eTabType& iType, eTabLeader& iLeader );
	bool		findNextTabStopInLayoutUnits(UT_sint32 iStartX, UT_sint32& iPosition, eTabType& iType, eTabLeader& iLeader);
	
	void		setNeedsRedraw(void) { m_bNeedsRedraw = true; m_pBlock->setNeedsRedraw();}
	bool		needsRedraw(void) { return m_bNeedsRedraw; }
	void		redrawUpdate(void);
#ifdef BIDI_ENABLED
	fp_Run *	getLastVisRun();
	fp_Run *	getFirstVisRun();
	void		setMapOfRunsDirty(){m_bMapDirty = true;};
	void		addDirectionUsed(UT_uint32 dir);
	
	/* the following variable is needed for handling of BiDi support; static because we need
	 * only one instance for all the instances of the class; public becuase it must be initialized
	 * before the constructor is called so that the constructor can decide whether any allocation
	 * is necessary or not; */
	static UT_uint32       s_iClassInstanceCounter;	
#endif
protected:
	fl_BlockLayout*	m_pBlock;
	fp_Container*	m_pContainer;
	
	UT_sint32	 	m_iWidth;
	UT_sint32	 	m_iWidthLayoutUnits;
	UT_sint32	 	m_iMaxWidth;
	UT_sint32		m_iMaxWidthLayoutUnits;
	UT_sint32 		m_iHeight;
	UT_sint32 		m_iHeightLayoutUnits;
	UT_sint32 		m_iAscent;
	UT_sint32		m_iDescent;

	UT_sint32		m_iX;
	UT_sint32		m_iXLayoutUnits;
	UT_sint32		m_iY;
	UT_sint32		m_iYLayoutUnits;
	
	UT_Vector		m_vecRuns;

	fp_Line*		m_pNext;
	fp_Line*        m_pPrev;

	bool			m_bNeedsRedraw;
#ifdef BIDI_ENABLED
	UT_uint32       _getRunVisIndx(UT_uint32 indx);
	UT_uint32       _getRunLogIndx(UT_uint32 indx);
	UT_sint32       _createMapOfRuns();	
	
#ifdef USE_STATIC_MAP
	static UT_sint32 * s_pMapOfRuns;
	static UT_uint32   s_iMapOfRunsSize;
	static fp_Line   * s_pMapOwner;
	bool            m_bMapDirty;
#else	
	UT_sint32  *    m_pMapOfRuns;
	UT_uint32       m_iMapOfRunsSize;
#endif
	UT_uint32		m_iRunsRTLcount;
	UT_uint32		m_iRunsLTRcount;
#endif //BIDI_ENABLED
};

#endif /* FP_LINE_H */


