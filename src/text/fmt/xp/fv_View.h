/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "xav_View.h"
#include "pt_Types.h"
#include "gr_DrawArgs.h"
#include "ev_EditBits.h"
#include "ie_types.h"
#include "xap_Prefs.h"
#include "ap_Dialog_Goto.h"
#include "fl_AutoLists.h"

// number of milliseconds between cursor blinks
const int AUTO_DRAW_POINT = 600;

class FL_DocLayout;
class fl_DocListener;
class fl_BlockLayout;
class fl_PartOfBlock;
class fp_Page;
class fp_Run;
class FG_Graphic;
class PD_Document;
class GR_Graphics;
class UT_Timer;
class AP_TopRulerInfo;
class AP_LeftRulerInfo;
class XAP_App;
class XAP_Prefs;
class UT_AlphaHashTable;
class PP_AttrProp;
class fl_AutoNum;

typedef enum _FVDocPos
{
	FV_DOCPOS_BOB, FV_DOCPOS_EOB,	// block
	FV_DOCPOS_BOD, FV_DOCPOS_EOD,	// document
	FV_DOCPOS_BOP, FV_DOCPOS_EOP,	// page
	FV_DOCPOS_BOL, FV_DOCPOS_EOL,	// line
	FV_DOCPOS_BOS, FV_DOCPOS_EOS,	// sentence
	FV_DOCPOS_BOW, FV_DOCPOS_EOW_MOVE, FV_DOCPOS_EOW_SELECT	// word
} FV_DocPos;

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

struct FV_DocCount
{
	UT_uint32 word;  
	UT_uint32 para; 
	UT_uint32 ch_no;
	UT_uint32 ch_sp;
	UT_uint32 line;
	UT_uint32 page;
};

class FV_View : public AV_View
{
	friend class fl_DocListener;
	friend class fl_BlockLayout;
	friend class fl_DocSectionLayout;
	
public:
	FV_View(XAP_App*, void*, FL_DocLayout*);
	~FV_View();

	inline GR_Graphics*		getGraphics(void) const { return m_pG; }
	inline UT_uint32		getPoint(void) const { return m_iInsPoint; }
	inline UT_uint32		getSelectionAnchor(void) const { return m_bSelection? m_iSelectionAnchor : m_iInsPoint; }

	virtual void focusChange(AV_Focus focus);

	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual void	draw(const UT_Rect* pRect=(UT_Rect*) NULL);

	virtual UT_Bool	notifyListeners(const AV_ChangeMask hint);

	virtual UT_Bool	canDo(UT_Bool bUndo) const;
	virtual void	cmdUndo(UT_uint32 count);
	virtual void	cmdRedo(UT_uint32 count);
	virtual UT_Error	cmdSave(void);
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft);

	UT_Error		cmdInsertField(const char* szName);
	UT_Error		cmdInsertGraphic(FG_Graphic*, const char*);
	
	virtual void	cmdCopy(void);
	virtual void	cmdCut(void);
	virtual void	cmdPaste(void);
	virtual void	cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos);

	virtual void	getTopRulerInfo(AP_TopRulerInfo * pInfo);
	virtual void	getLeftRulerInfo(AP_LeftRulerInfo * pInfo);

	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos);
	virtual EV_EditMouseContext getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos);

	virtual UT_Bool		isSelectionEmpty(void) const;
	virtual void		cmdUnselectSelection(void);
	void				getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr);
	
// ----------------------
	FL_DocLayout* 	getLayout() const;
	UT_uint32		getCurrentPageNumForStatusBar(void) const;
	fp_Page*		getCurrentPage(void) const;
	fl_BlockLayout *        getCurrentBlock(void);

	void draw(int page, dg_DrawArgs* da);

	// TODO some of these functions should move into protected
	
	void getPageScreenOffsets(fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff);
	void getPageYOffset(fp_Page* pPage, UT_sint32& yoff);
	virtual UT_uint32 getPageViewLeftMargin(void) const;
	virtual UT_uint32 getPageViewTopMargin(void) const;
	
	UT_Bool setSectionFormat(const XML_Char * properties[]);
	UT_Bool getSectionFormat(const XML_Char *** properties);

        UT_Bool isCursorOn(void);
        void    eraseInsertionPoint(void);
        void    drawInsertionPoint(void);

	UT_Bool setBlockFormat(const XML_Char * properties[]);
	UT_Bool getBlockFormat(const XML_Char *** properties,UT_Bool bExpandStyles=UT_TRUE);


	UT_Bool isTabListBehindPoint(void);
	UT_Bool isTabListAheadPoint(void);
	void    processSelectedBlocks(List_Type listType);
	void    getListBlocksInSelection( UT_Vector * vBlock);
	UT_Bool isPointBeforeListLabel(void);
	UT_Bool isCurrentListBlockEmpty(void);
	UT_Bool cmdStartList(const XML_Char * style);
	UT_Bool cmdStopList(void);
	void    changeListStyle( fl_AutoNum * pAuto,  List_Type lType, UT_uint32 startv, XML_Char * pszDelim, XML_Char * pszDecimal, XML_Char * pszFormat, float Aligm, float Indent);

	UT_Bool setCharFormat(const XML_Char * properties[]);
	UT_Bool getCharFormat(const XML_Char *** properties,UT_Bool bExpandStyles=UT_TRUE);

	UT_Bool setStyle(const XML_Char * style);
	UT_Bool getStyle(const XML_Char ** style);

	UT_Bool dontSpellCheckRightNow(void);

	void insertParagraphBreak(void);
	void insertParagraphBreaknoListUpdate(void);
	void insertSectionBreak(void);
	void insertSymbol(UT_UCSChar c, XML_Char * symfont);
// ----------------------
	UT_Bool			isLeftMargin(UT_sint32 xPos, UT_sint32 yPos);
	void			cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd);
	void			cmdCharMotion(UT_Bool bForward, UT_uint32 count);
    UT_Bool         cmdCharInsert(UT_UCSChar * text, UT_uint32 count, UT_Bool bForce = UT_FALSE);
	void			cmdCharDelete(UT_Bool bForward, UT_uint32 count);
	void			delTo(FV_DocPos dp);
	UT_UCSChar * 	getSelectionText(void);
		
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos);
	void			moveInsPtTo(FV_DocPos dp);
	void 			moveInsPtTo(PT_DocPosition dp);
	void			warpInsPtNextPrevPage(UT_Bool bNext);
	void			warpInsPtNextPrevLine(UT_Bool bNext);
	void			extSelHorizontal(UT_Bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos, UT_Bool bDrag);
	void			extSelToXYword(UT_sint32 xPos, UT_sint32 yPos, UT_Bool bDrag);
	void			extSelTo(FV_DocPos dp);
	void			extSelNextPrevLine(UT_Bool bNext);
	void			endDrag(UT_sint32 xPos, UT_sint32 yPos);

	void			updateScreen(void);


// ----------------------

	UT_Bool			isPosSelected(PT_DocPosition pos) const;
	UT_Bool			isXYSelected(UT_sint32 xPos, UT_sint32 yPos) const;

	UT_UCSChar *	getContextSuggest(UT_uint32 ndx);
	void			cmdContextSuggest(UT_uint32 ndx);
	void			cmdContextIgnoreAll(void);
	void			cmdContextAdd(void);
	
// ----------------------

	UT_Bool 		gotoTarget(AP_JumpTarget type, UT_UCSChar * data);

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
	UT_Bool			_findNext(const UT_UCSChar * find, UT_uint32 *prefix, UT_Bool matchCase = UT_TRUE, UT_Bool * bDoneEntireDocument = NULL);
	// replaces the selection of "find" with "replace" and selects the next, filling
	// bool when done the entire document
	UT_Bool			_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
								 UT_Bool matchCase = UT_FALSE, UT_Bool * bDoneEntireDocument = NULL);
	UT_Bool			_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace, UT_uint32 *prefix,
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

	FV_DocCount                     countWords(void);

// -----------------------

	UT_Bool                         insertPageNum(const XML_Char ** props, UT_Bool ftr);

// -----------------------

    void				setShowPara(UT_Bool);    
    inline UT_Bool		getShowPara(void) const { return m_bShowPara; };
	
	const fp_PageSize&	getPageSize(void) const;
	UT_uint32			calculateZoomPercentForPageWidth();
	UT_uint32			calculateZoomPercentForWholePage();

protected:
	void				_generalUpdate(void);
	
	void 				_draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32, UT_Bool bDirtyRunsOnly, UT_Bool bClip=UT_FALSE);
	
	void				_drawBetweenPositions(PT_DocPosition left, PT_DocPosition right);
	void				_clearBetweenPositions(PT_DocPosition left, PT_DocPosition right, UT_Bool bFullLineHeightRect);
	
	UT_Bool				_ensureThatInsertionPointIsOnScreen(void);
	void			    _moveInsPtNextPrevPage(UT_Bool bNext);
	void			    _moveInsPtNextPrevLine(UT_Bool bNext);
	fp_Page *           _getCurrentPage(void);
	void                _moveInsPtNthPage(UT_uint32 n);
	void                _moveInsPtToPage(fp_Page *page);

	PT_DocPosition		_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, UT_Bool bKeepLooking=UT_TRUE);
	PT_DocPosition		_getDocPos(FV_DocPos dp, UT_Bool bKeepLooking=UT_TRUE);
	void 				_findPositionCoords(UT_uint32 pos,
											UT_Bool b,
											UT_sint32& x,
											UT_sint32& y,
											UT_uint32& height,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun);
	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos) const;

	fp_Page*			_getPageForXY(UT_sint32 xPos, 
									  UT_sint32 yPos, 
									  UT_sint32& xClick, 
									  UT_sint32& yClick) const;

	void				_moveToSelectionEnd(UT_Bool bForward);
	void				_eraseSelection(void);
	void				_clearSelection(void);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(PP_AttrProp *p_AttrProp_Before = NULL);
	UT_Bool				_insertFormatPair(const XML_Char * szName, const XML_Char * properties[]);
	void 				_eraseInsertionPoint();
	void				_drawInsertionPoint();
	void 				_updateInsertionPoint();
	void				_fixInsertionPointCoords();
	void 				_xorInsertionPoint();
        UT_Bool                         _hasPointMoved(void); 
	void                            _saveCurrentPoint(void); 
        void                            _clearOldPoint(void); 
	void				_drawSelection();
	void				_swapSelectionOrientation(void);
	void				_extSel(UT_uint32 iOldPoint);
	void				_extSelToPos(PT_DocPosition pos);
	UT_Error   			_insertGraphic(FG_Graphic*, const char*);

	UT_UCSChar *		_lookupSuggestion(fl_BlockLayout* pBL, fl_PartOfBlock* pPOB, UT_uint32 ndx);

	static void			_autoScroll(UT_Timer * pTimer);
	static void			_autoDrawPoint(UT_Timer * pTimer);

	// localize handling of insertion point logic
	void				_setPoint(UT_uint32 pt, UT_Bool bEOL = UT_FALSE);
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2);
	UT_Bool				_charMotion(UT_Bool bForward,UT_uint32 countChars);
	void				_doPaste(UT_Bool bUseClipboard);
	void				_clearIfAtFmtMark(PT_DocPosition dpos);

	void				_checkPendingWordForSpell(void);


	UT_Bool				_insertHeaderFooter(const XML_Char ** props, UT_Bool ftr);


	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	UT_uint32			m_iPointHeight;
	UT_sint32			m_oldxPoint; 
	UT_sint32			m_oldyPoint; 
	UT_uint32			m_oldiPointHeight;
	UT_sint32			m_xPointSticky;		// used only for _moveInsPtNextPrevLine() 

	UT_Bool				m_bPointVisible;
	UT_Bool				m_bPointEOL;

	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	GR_Graphics*		m_pG;
        void *                  m_pParentData;

    PT_DocPosition		m_iSelectionAnchor;
    PT_DocPosition		m_iSelectionLeftAnchor;
    PT_DocPosition		m_iSelectionRightAnchor;
	UT_Bool				m_bSelection;

	// autoscroll stuff
	UT_Timer *			m_pAutoScrollTimer;
	UT_sint32			m_xLastMouse;
	UT_sint32			m_yLastMouse;

	UT_Timer *			m_pAutoCursorTimer;
	UT_Bool				m_bCursorIsOn;
	UT_Bool				m_bEraseSaysStopBlinking;
	UT_Bool				m_bCursorBlink;

	UT_Bool                         m_bdontSpellCheckRightNow;
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
	UT_sint32 			_findBlockSearchDumbCase(const UT_UCSChar * haystack, const UT_UCSChar * needle);
	UT_sint32 			_findBlockSearchDumbNoCase(const UT_UCSChar * haystack, const UT_UCSChar * needle);	
	UT_sint32			_findBlockSearchRegexp(const UT_UCSChar * haystack, const UT_UCSChar * needle);

	// prefs listener - to change cursor blink on/off (and possibly others)
	static void _prefsListener( XAP_App *, XAP_Prefs *, UT_AlphaHashTable *, void *);

    UT_Bool             m_bShowPara;
};

#endif /* FV_VIEW_H */
