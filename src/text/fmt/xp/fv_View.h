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



#ifndef FV_VIEW_H
#define FV_VIEW_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xav_View.h"
#include "xav_Listener.h"
#include "pt_Types.h"
#include "gr_DrawArgs.h"

class FL_DocLayout;
class fl_DocListener;
class fl_BlockLayout;
class fp_Page;
class fp_Run;
class PD_Document;
class GR_Graphics;
class GR_Image;
class UT_Timer;

typedef enum _FVDocPos
{
	FV_DOCPOS_BOB, FV_DOCPOS_EOB,	// block
	FV_DOCPOS_BOD, FV_DOCPOS_EOD,	// document
	FV_DOCPOS_BOL, FV_DOCPOS_EOL,	// line
	FV_DOCPOS_BOS, FV_DOCPOS_EOS,	// sentence
	FV_DOCPOS_BOW, FV_DOCPOS_EOW	// word
} FV_DocPos;

typedef enum _FVJumpTarget
{
	FV_JUMPTARGET_PAGE,				// beginning of page
	FV_JUMPTARGET_LINE				// beginning of line
} FV_JumpTarget;
		
struct fv_ChangeState
{
	UT_Bool				bUndo;
	UT_Bool				bRedo;
	UT_Bool				bDirty;
	UT_Bool				bSelection;
	UT_uint32			iColumn;
	const XML_Char **	propsChar;
	const XML_Char **	propsBlock;
	const XML_Char **	propsSection;
};

class FV_View : public AV_View
{
	friend class fl_DocListener;
	friend class fl_BlockLayout;
	
public:
	FV_View(void*, FL_DocLayout*);
	~FV_View();

	inline GR_Graphics*		getGraphics(void) const { return m_pG; }
	inline UT_uint32		getPoint(void) const { return m_iInsPoint; }
	inline UT_uint32		getSelectionAnchor(void) const { return m_bSelection? m_iSelectionAnchor : m_iInsPoint; }
	
	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual void	draw(const UT_Rect* pRect=(UT_Rect*) NULL);

	virtual UT_Bool	notifyListeners(const AV_ChangeMask hint);

	virtual UT_Bool	canDo(UT_Bool bUndo) const;
	virtual void	cmdUndo(UT_uint32 count);
	virtual void	cmdRedo(UT_uint32 count);
	virtual void	cmdSave(void);
	virtual void	cmdSaveAs(const char * szFilename);

	virtual void	cmdCopy(void);
	virtual void	cmdCut(void);
	virtual void	cmdPaste(void);

	virtual void	getTopRulerInfo(AP_TopRulerInfo * pInfo);
	
// ----------------------
	FL_DocLayout* getLayout() const;

	void draw(int page, dg_DrawArgs* da);

	// TODO some of these functions should move into protected
	
	void getPageScreenOffsets(fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff);
	void getPageYOffset(fp_Page* pPage, UT_sint32& yoff);
	virtual UT_uint32 getPageViewLeftMargin(void) const;
	virtual UT_uint32 getPageViewTopMargin(void) const;
	
	UT_Bool setSectionFormat(const XML_Char * properties[]);
	UT_Bool getSectionFormat(const XML_Char *** properties);

	UT_Bool setBlockFormat(const XML_Char * properties[]);
	UT_Bool getBlockFormat(const XML_Char *** properties);

	UT_Bool setCharFormat(const XML_Char * properties[]);
	UT_Bool getCharFormat(const XML_Char *** properties);

	void insertParagraphBreak(void);
	void insertSectionBreak(void);

// ----------------------
	UT_Bool			isLeftMargin(UT_sint32 xPos, UT_sint32 yPos);
	UT_Bool			isSelectionEmpty();
	void			cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd);
	void			cmdCharMotion(UT_Bool bForward, UT_uint32 count);
	UT_Bool			cmdCharInsert(UT_UCSChar * text, UT_uint32 count);
	void			cmdCharDelete(UT_Bool bForward, UT_uint32 count);
	void			delTo(FV_DocPos dp);
	UT_UCSChar * 	getSelectionText(void);
		
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos);
	void			moveInsPtTo(FV_DocPos dp);
	void 			moveInsPtTo(PT_DocPosition dp);
	void			warpInsPtNextPrevLine(UT_Bool bNext);
	void			extSelHorizontal(UT_Bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos, UT_Bool bDrag);
	void			extSelTo(FV_DocPos dp);
	void			extSelNextPrevLine(UT_Bool bNext);
	void			endDrag(UT_sint32 xPos, UT_sint32 yPos);

// ----------------------

	// goto -- this is really not implemented
	UT_Bool 		gotoTarget(FV_JumpTarget type, UT_UCSChar * data);

	void			changeNumColumns(UT_uint32 iNumColumns);
	
// ----------------------

	// find and replace
	
	// aid the edit method for the simple non-dialog findAgain()
	UT_Bool 		findSetNextString(UT_UCSChar * string, UT_Bool matchCase);
	UT_Bool			findAgain(void);

	void 			findSetStartAtInsPoint(void);

	// finds the next "find" and selects it, filling bool when done the entire document
	UT_Bool			findNext(const UT_UCSChar * find, UT_Bool matchCase = UT_TRUE, UT_Bool * bDoneEntireDocument = NULL);
	UT_Bool			_findNext(const UT_UCSChar * find, UT_Bool matchCase = UT_TRUE, UT_Bool * bDoneEntireDocument = NULL);
	// replaces the selection of "find" with "replace" and selects the next, filling
	// bool when done the entire document
	UT_Bool			_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
								 UT_Bool matchCase = UT_FALSE, UT_Bool * bDoneEntireDocument = NULL);
	UT_Bool			findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
								UT_Bool matchCase = UT_FALSE, UT_Bool * bDoneEntireDocument = NULL);
	// replaces every occurance of "find" with "replace" without stopping for anything
	UT_uint32		findReplaceAll(const UT_UCSChar * find, const UT_UCSChar * replace,
								   UT_Bool matchCase = UT_FALSE);
		
// ----------------------

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	void			Test_Dump(void);
#endif

// ----------------------

protected:
	void				_generalUpdate(void);
	
	void 				_draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32, UT_Bool bDirtyRunsOnly, UT_Bool bClip=UT_FALSE);
	void				_updateScreen(UT_Bool bToggleIP=UT_FALSE);
	
	void				_drawBetweenPositions(PT_DocPosition left, PT_DocPosition right);
	void				_clearBetweenPositions(PT_DocPosition left, PT_DocPosition right, UT_Bool bFullLineHeightRect);
	
	UT_Bool				_ensureThatInsertionPointIsOnScreen(void);
	void			    _moveInsPtNextPrevLine(UT_Bool bNext);

	PT_DocPosition		_getDocPos(FV_DocPos dp, UT_Bool bKeepLooking=UT_TRUE);
	void 				_findPositionCoords(UT_uint32 pos,
											UT_Bool b,
											UT_sint32& x,
											UT_sint32& y,
											UT_uint32& height,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun);
	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos);

	fp_Page*			_getPageForXY(UT_sint32 xPos, 
									  UT_sint32 yPos, 
									  UT_sint32& xClick, 
									  UT_sint32& yClick);

	void				_moveToSelectionEnd(UT_Bool bForward);
	void				_eraseSelection(void);
	void				_clearSelection(void);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(void);
	UT_Bool				_insertFormatPair(const XML_Char * szName, const XML_Char * properties[]);
	void 				_eraseInsertionPoint();
	void				_drawInsertionPoint();
	void 				_updateInsertionPoint();
	void				_fixInsertionPointCoords();
	void 				_xorInsertionPoint();
	void				_drawSelection();
	void				_swapSelectionOrientation(void);
	void				_extSel(UT_uint32 iOldPoint);
	void				_extSelToPos(PT_DocPosition pos);

	static void			_autoScroll(UT_Timer * pTimer);

	// localize handling of insertion point logic
	void				_setPoint(UT_uint32 pt, UT_Bool bEOL = UT_FALSE);
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2);
	UT_Bool				_charMotion(UT_Bool bForward,UT_uint32 countChars);
	UT_Bool				_isPointAP(void);
	PT_AttrPropIndex	_getPointAP(void);
	void				_setPointAP(PT_AttrPropIndex indexAP);
	UT_Bool				_clearPointAP(UT_Bool bNotify);
	void				_doPaste(void);
	void				_doInsertImage(GR_Image*);
	
	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	UT_uint32			m_iPointHeight;

	UT_sint32			m_xPointSticky;		// used only for _moveInsPtNextPrevLine() 

	UT_Bool				m_bPointVisible;
	UT_Bool				m_bPointEOL;

	UT_Bool				m_bPointAP;
	PT_AttrPropIndex	m_apPoint;
	
	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	GR_Graphics*		m_pG;

	PT_DocPosition		m_iSelectionAnchor;
	UT_Bool				m_bSelection;

	// autoscroll stuff
	UT_Timer *			m_pAutoScrollTimer;
	UT_sint32			m_xLastMouse;
	UT_sint32			m_yLastMouse;

	fv_ChangeState		m_chg;

	// find and replace stuff
	UT_Bool				m_wrappedEnd;
	PT_DocPosition		m_startPosition;

	UT_Bool				m_doneFind;
	
	PT_DocPosition 		_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset);
	
	fl_BlockLayout * 	_findGetCurrentBlock(void);
	PT_DocPosition	 	_findGetCurrentOffset(void);	
	UT_UCSChar * 		_findGetNextBlockBuffer(fl_BlockLayout ** block, PT_DocPosition *offset);

	UT_Bool				_m_matchCase;
	UT_UCSChar * 		_m_findNextString;

	// search routines (these return values will fall short of an
	// extremely large document - fix them)
	UT_sint32 			_findBlockSearchDumb(const UT_UCSChar * haystack, const UT_UCSChar * needle);
	UT_sint32			_findBlockSearchRegexp(const UT_UCSChar * haystack, const UT_UCSChar * needle);
};

#endif /* FV_VIEW_H */
