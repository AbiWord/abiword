/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef FP_LINE_H
#define FP_LINE_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_BlockLayout.h"
#include "fp_Column.h"
#include "fp_ContainerObject.h"
//#include "fmt_Types.h"

class fp_Run;
class GR_Graphics;
class fp_Container;
class fp_VerticalContainer;

struct dg_DrawArgs;

// ----------------------------------------------------------------
/*
	fp_Line represents a single line.  A fp_Line is a collection of
	Runs.
*/
//these are the initial sizes of the temp buffers
#define TEMP_LINE_BUFFER_SIZE 512

// the memory requirement connected with the following constant
// is 9 bytes per 1; 100 is reasonable: if loading documents with long paragraphs
// this will have to be reallocated, since initially each paragraph is loaded
// on a single line, but for creating documents from a scratch this is
// more than adequate.
#define RUNS_MAP_SIZE 100
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

enum FL_WORKING_DIRECTION {WORK_FORWARD = 1,WORK_BACKWARD = -1,WORK_CENTER = 0};
enum FL_WHICH_TABSTOP {USE_PREV_TABSTOP,USE_NEXT_TABSTOP,USE_FIXED_TABWIDTH};



class ABI_EXPORT fp_Line : public fp_Container
{
	friend class fp_Run; // this is to allow access to
						 // _createMapOfRuns() which I do not want to
						 // make public
public:
	fp_Line(fl_SectionLayout * pSectionLayout);
	~fp_Line();

	inline fl_BlockLayout*		getBlock(void) const 		{ return m_pBlock; }
	//! Return height of line as it will appear on screen
	virtual UT_sint32	getHeight(void) const;

	virtual UT_sint32	getX(void) const;
	virtual UT_sint32	getY(void) const;

	UT_sint32		getMaxWidth(void) const ;
	UT_sint32		getAscent(void) const;
	UT_sint32		getDescent(void) const;
	UT_sint32               getNumRunsInLine(void) const {return m_vecRuns.getItemCount();}
	UT_sint32			        getColumnGap(void);
	void				        setAssignedScreenHeight(UT_sint32);
	bool                        assertLineListIntegrity(void);
	void				        setMaxWidth(UT_sint32);
	virtual void				setX(UT_sint32 i, bool bDontClearIfNeeded = false);
	virtual void				setY(UT_sint32);

	virtual void				setContainer(fp_Container*);
	void		        setBlock(fl_BlockLayout * pBlock);

	fp_Container *              getColumn(void); // FIXME see if we can make it const
	fp_Page *                   getPage(void);  // FIXME see if we can make it const

	virtual void        setWidth(UT_sint32 ){}
	virtual void        setHeight(UT_sint32 i);
	virtual UT_sint32   getWidth(void) const { return m_iWidth;}
	virtual UT_sint32   getDrawingWidth(void) const;
	UT_sint32           getWidthToRun(fp_Run * pLastRun);
	UT_sint32           getFilledWidth(void);
    virtual bool        isVBreakable(void) { return false;}
    virtual bool        isHBreakable(void) {return true;}
	virtual UT_sint32   wantVBreakAt(UT_sint32) { return 0;}
	virtual UT_sint32   wantHBreakAt(UT_sint32) { return 0;}
    virtual fp_ContainerObject * VBreakAt(UT_sint32) { return NULL;}
    virtual fp_ContainerObject * HBreakAt(UT_sint32) {return NULL;}
    virtual UT_uint32 distanceFromPoint(UT_sint32, UT_sint32) {return 0;}
	virtual fp_Container*	getNextContainerInSection(void) const;
	virtual fp_Container*	getPrevContainerInSection(void) const;
	fp_Run *    getRunFromIndex( UT_uint32 runIndex);
	bool		containsForcedColumnBreak(void) const;
	bool		containsForcedPageBreak(void) const;
	bool        containsFootnoteReference(void);
	bool        getFootnoteContainers(UT_GenericVector<fp_FootnoteContainer *>* pvecFoots);
	bool        containsAnnotations(void);
	bool        getAnnotationContainers(UT_GenericVector<fp_AnnotationContainer *>* pvecAnnotations);
	void 		addRun(fp_Run*);
	void		insertRunAfter(fp_Run* pRun1, fp_Run* pRun2);
	void		insertRunBefore(fp_Run* pNewRun, fp_Run* pBefore);
	void        insertRun(fp_Run*);
    bool     removeRun(fp_Run*, bool bTellTheRunAboutIt=true);

	inline	bool		isEmpty(void) const				{ return ((m_vecRuns.getItemCount()) == 0); }
	inline	int 		countRuns(void) const			{ return m_vecRuns.getItemCount(); }
	inline	fp_Run*     getFirstRun(void) const			{ if(countRuns() > 0)  return ((fp_Run*) m_vecRuns.getFirstItem()); else return NULL; }
	fp_Run*     getLastRun(void) const ;
	fp_Run*     getLastTextRun(void) const ;

	fp_Run*	calculateWidthOfRun(UT_sint32 &iX,
								UT_uint32 iIndxVisual,
								FL_WORKING_DIRECTION eWorkingDirection,
								FL_WHICH_TABSTOP eUseTabStop);

	void		getWorkingDirectionAndTabstops(FL_WORKING_DIRECTION &eWorkingDirection, FL_WHICH_TABSTOP &eUseTabStop) const;

	inline	bool 	isFirstLineInBlock(void) const;
	bool 	isLastLineInBlock(void) const;

    virtual UT_Rect *       getScreenRect();
    virtual void            markDirtyOverlappingRuns(UT_Rect & recScreen);

	void		remove(void);
	UT_sint32	getMarginBefore(void) const;
	UT_sint32	getMarginAfter(void) const;
	virtual void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool& isTOC);
	void		getOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff);
	void		getScreenOffsets(fp_Run* pRun, UT_sint32& xoff, UT_sint32& yoff);
	virtual void  clearScreen(void);
	void		clearScreenFromRunToEnd(UT_uint32 runIndex);
	void		clearScreenFromRunToEnd(fp_Run * pRun);
	bool        containsOffset(PT_DocPosition blockOffset);
	void         setScreenCleared(bool bisCleared)
		{   m_bIsCleared = bisCleared;}
	bool         isScreenCleared(void) const { return m_bIsCleared;}
	void         setAdditionalMargin(UT_sint32 iAccum)
	  { m_iAdditionalMarginAfter = iAccum;}
	virtual void		draw(dg_DrawArgs*);
	virtual void        draw(GR_Graphics*);
	void		align(void);
	void		layout(void);
	bool		recalculateFields(UT_uint32 iUpdateCount);
	void		recalcHeight(fp_Run * pLast = NULL);
	void		recalcMaxWidth(bool bDontClearIfNeeded = false);
	void            setReformat(void);
	void		coalesceRuns(void);

	UT_sint32	calculateWidthOfLine(void);
	UT_sint32	calculateWidthOfTrailingSpaces(void);
	void		resetJustification(bool bPermanent);
	void		justify(UT_sint32 iAmount);
	UT_uint32	countJustificationPoints(void);

	bool		isLastCharacter(UT_UCSChar Character) const;

	bool		findNextTabStop(UT_sint32 iStartX, UT_sint32& iPosition, eTabType& iType, eTabLeader& iLeader );
	bool		findPrevTabStop(UT_sint32 iStartX, UT_sint32& iPosition, eTabType& iType, eTabLeader& iLeader );
	void		setNeedsRedraw(void);
	//void		setRedoLayout(void){ m_bRedoLayout = true; }
	bool		needsRedraw(void) { return m_bNeedsRedraw; }
	bool		redrawUpdate(void);
	fp_Run *	getLastVisRun();
	fp_Run *	getFirstVisRun();
	UT_uint32	getVisIndx(fp_Run* pRun);
	fp_Run *	getRunAtVisPos(UT_sint32 i);
	void		setMapOfRunsDirty(){m_bMapDirty = true;};
	void		addDirectionUsed(UT_BidiCharType dir, bool bRefreshMap = true);
	void		removeDirectionUsed(UT_BidiCharType dir, bool bRefreshMap = true);
	void		changeDirectionUsed(UT_BidiCharType oldDir, UT_BidiCharType newDir, bool bRefreshMap = true);
    UT_sint32   getBreakTick(void) const
		{ return  m_iBreakTick;}
	void        setBreakTick(UT_sint32 iTick)
		{ m_iBreakTick = iTick;}
	void        setWrapped(bool bWrapped)
		{ m_bIsWrapped = bWrapped;}
	bool        isWrapped(void) const
		{ return m_bIsWrapped;}
	void        setSameYAsPrevious(bool bSameAsPrevious);
	bool        isSameYAsPrevious(void) const
		{ return m_bIsSameYAsPrevious;}
	void        setAlongTopBorder(bool bAlongTopBorder)
		{ m_bIsAlongTopBorder = bAlongTopBorder;}
	void        setAlongBotBorder(bool bAlongBotBorder)
		{ m_bIsAlongBotBorder = bAlongBotBorder;}
	bool        isAlongTopBorder(void) const
		{ return m_bIsAlongTopBorder;}
	bool        isAlongBotBorder(void) const
		{ return m_bIsAlongBotBorder;}
	void        genOverlapRects(UT_Rect & recLeft, UT_Rect & recRight);

	bool        canContainPoint() const;
	UT_sint32   calcLeftBorderThick(void);
	UT_sint32   calcRightBorderThick(void);
	UT_sint32   calcTopBorderThick(void);
	UT_sint32   calcBotBorderThick(void);
	void        calcBorderThickness(void);
	UT_sint32   getLeftThick(void) const;
	UT_sint32   getRightThick(void) const;
	UT_sint32   getTopThick(void) const;
	UT_sint32   getBotThick(void) const;
	UT_sint32   getAvailableWidth(void) const;
	const fp_Line *   getFirstInContainer(void) const;
	const fp_Line *   getLastInContainer(void) const;
	bool        canDrawTopBorder(void) const;
	bool        canDrawBotBorder(void) const;
	void        drawBorders(GR_Graphics * pG);
	UT_sint32              getLeftEdge(void) const;
	UT_sint32              getRightEdge(void) const;
	bool        getAbsLeftRight(UT_sint32& left ,UT_sint32& right); // FIXME try to make it const
	bool        hasBordersOrShading(void) const;
#ifdef FMT_TEST
	void		__dump(FILE * fp) const;
#endif

protected:
	void 	_calculateWidthOfRun(UT_sint32 &iX,
								 fp_Run * pRun,
								 UT_uint32 iIndx,
								 UT_uint32 iCountRuns,
								 FL_WORKING_DIRECTION eWorkingDirection,
								 FL_WHICH_TABSTOP eUseTabStop,
								 UT_BidiCharType iDomDirection
								 );

private:
	void		_splitRunsAtSpaces(void);
	void        _doClearScreenFromRunToEnd(UT_sint32 runIndex);


	void  		setAscent(UT_sint32 i) { m_iAscent = i; }
	void  		setDescent(UT_sint32 i) { m_iDescent = i; }
	void        setScreenHeight(UT_sint32 i) {m_iScreenHeight =i;}


	fl_BlockLayout*	m_pBlock;
	fp_Container*	m_pContainer;

	UT_sint32	 	m_iWidth;
	UT_sint32	 	m_iMaxWidth;
	UT_sint32       m_iClearToPos;
	UT_sint32       m_iClearLeftOffset;
	UT_sint32 		m_iHeight;
	//! Height assigned on screen
	//! -1 if undefined
	UT_sint32 		m_iScreenHeight;
	UT_sint32 		m_iAscent;
	UT_sint32		m_iDescent;

	UT_sint32		m_iX;
	UT_sint32		m_iY;
	UT_GenericVector<fp_Run *>	m_vecRuns;

	bool			m_bNeedsRedraw;
	//bool			m_bRedoLayout;
	static UT_sint32 * s_pOldXs;
	static UT_uint32   s_iOldXsSize;
	static UT_uint32   s_iClassInstanceCounter;

	UT_uint32       _getRunVisIndx(UT_sint32 indx);
	UT_uint32       _getRunLogIndx(UT_sint32 indx);
	UT_sint32       _createMapOfRuns();
#ifdef USE_STATIC_MAP
	static UT_Byte     * s_pEmbeddingLevels;
	static UT_uint32   * s_pMapOfRunsL2V;
	static UT_uint32   * s_pMapOfRunsV2L;
	static UT_UCS4Char * s_pPseudoString;


	static UT_sint32   s_iMapOfRunsSize;
	static fp_Line   * s_pMapOwner;
	bool            m_bMapDirty;
#else
	UT_sint32  * 	m_pMapOfDirs;
	UT_sint32  *    m_pMapOfRunsL2V;
	UT_sint32  *    m_pMapOfRunsV2L;

	UT_sint32       m_iMapOfRunsSize;
#endif
	UT_uint32		m_iRunsRTLcount;
	UT_uint32		m_iRunsLTRcount;
	UT_sint32		m_iMaxDirLevel;
	bool            m_bIsCleared;
	bool			m_bContainsFootnoteRef; // updated when runs added/removed.
	void			_updateContainsFootnoteRef(void);
	UT_sint32       m_iBreakTick;
	bool            m_bIsWrapped;
	bool            m_bIsSameYAsPrevious;
	bool            m_bIsAlongTopBorder;
	bool            m_bIsAlongBotBorder;
        UT_sint32       m_iAdditionalMarginAfter;
	UT_sint32       m_iLeftThick;
	UT_sint32       m_iRightThick;
	UT_sint32       m_iTopThick;
	UT_sint32       m_iBotThick;
};

#endif /* FP_LINE_H */


