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
#include "fl_SectionLayout.h"

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
class UT_StringPtrMap;
class PP_AttrProp;
class fl_AutoNum;
class fp_PageSize;

typedef enum _FVDocPos
{
	FV_DOCPOS_BOB, FV_DOCPOS_EOB,	// block
	FV_DOCPOS_BOD, FV_DOCPOS_EOD,	// document
	FV_DOCPOS_BOP, FV_DOCPOS_EOP,	// page
	FV_DOCPOS_BOL, FV_DOCPOS_EOL,	// line
	FV_DOCPOS_BOS, FV_DOCPOS_EOS,	// sentence
	FV_DOCPOS_BOW, FV_DOCPOS_EOW_MOVE, FV_DOCPOS_EOW_SELECT	// word
} FV_DocPos;

typedef enum _ToggleCase
{
  CASE_SENTENCE,
  CASE_LOWER,
  CASE_UPPER,
  CASE_TITLE,
  CASE_TOGGLE
} ToggleCase;

typedef enum
{
	BreakSectionContinuous,
	BreakSectionNextPage,
	BreakSectionEvenPage,
	BreakSectionOddPage
} BreakSectionType;

typedef enum
{
  VIEW_PRINT,
  VIEW_NORMAL,
  VIEW_WEB,
  VIEW_PREVIEW
} ViewMode;

typedef enum
{
  PREVIEW_NONE,
  PREVIEW_ZOOMED,
  PREVIEW_ADJUSTED_PAGE,
  PREVIEW_CLIPPED,
  PREVIEW_ZOOMED_SCROLL,
  PREVIEW_ADJUSTED_PAGE_SCROLL,
  PREVIEW_CLIPPED_SCROLL
} PreViewMode;

typedef enum _AP_JumpTarget
{
	AP_JUMPTARGET_PAGE,				// beginning of page
	AP_JUMPTARGET_LINE,
	AP_JUMPTARGET_PICTURE // TODO
} AP_JumpTarget;
		
struct fv_ChangeState
{
	bool				bUndo;
	bool				bRedo;
	bool				bDirty;
	bool				bSelection;
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
	virtual ~FV_View();

	inline GR_Graphics*		getGraphics(void) const { return m_pG; }
	inline UT_uint32		getPoint(void) const { return m_iInsPoint; }
	inline UT_uint32		getSelectionAnchor(void) const { return m_bSelection? m_iSelectionAnchor : m_iInsPoint; }

	virtual void focusChange(AV_Focus focus);

	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual void	draw(const UT_Rect* pRect=(UT_Rect*) NULL);

	virtual bool	notifyListeners(const AV_ChangeMask hint);

	virtual bool	canDo(bool bUndo) const;
	virtual void	cmdUndo(UT_uint32 count);
	virtual void	cmdRedo(UT_uint32 count);
	virtual UT_Error	cmdSave(void);
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft);
	virtual UT_Error        cmdSaveAs(const char * szFilename, int ieft, bool cpy);

	UT_Error		cmdInsertField(const char* szName);
	UT_Error        cmdInsertField(const char* szName, const XML_Char ** extra_attrs);

	UT_Error		cmdInsertGraphic(FG_Graphic*, const char*);

	virtual void    toggleCase(ToggleCase c);
	virtual void    setPaperColor(const XML_Char * clr);

	virtual void	cmdCopy(void);
	virtual void	cmdCut(void);
	virtual void	cmdPaste(bool bHonorFormatting = true);
	virtual void	cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos);

	virtual void	getTopRulerInfo(AP_TopRulerInfo * pInfo);
	virtual void	getLeftRulerInfo(AP_LeftRulerInfo * pInfo);

	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos);
	virtual EV_EditMouseContext getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos);

	virtual bool		isSelectionEmpty(void) const;
	virtual void		cmdUnselectSelection(void);
	void				getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr);
	
// ----------------------
	FL_DocLayout* 	getLayout() const;
	UT_uint32		getCurrentPageNumForStatusBar(void) const;
	fp_Page*		getCurrentPage(void) const;
	fl_BlockLayout*	getCurrentBlock(void);

	void draw(int page, dg_DrawArgs* da);

	// TODO some of these functions should move into protected
	
	void	getPageScreenOffsets(fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff);
	void	getPageYOffset(fp_Page* pPage, UT_sint32& yoff);
	virtual	UT_sint32 getPageViewLeftMargin(void) const;
	virtual	UT_sint32 getPageViewTopMargin(void) const;
	virtual	UT_sint32 getPageViewSep(void) const;
	
	bool	setSectionFormat(const XML_Char * properties[]);
	bool	getSectionFormat(const XML_Char *** properties);

	bool	isCursorOn(void);
	void	eraseInsertionPoint(void);
	void	drawInsertionPoint(void);

	bool	setBlockIndents(bool doLists, double indentChange, double page_size);
	bool	setBlockFormat(const XML_Char * properties[]);
	bool	getBlockFormat(const XML_Char *** properties,bool bExpandStyles=true);

	bool    processPageNumber(HdrFtrType hfType, const XML_Char ** atts);

	bool	isTabListBehindPoint(void);
	bool	isTabListAheadPoint(void);
	void	processSelectedBlocks(List_Type listType);
	void	getBlocksInSelection( UT_Vector * vBlock);
	void	getAllBlocksInList( UT_Vector * vBlock);
	bool	isPointBeforeListLabel(void);
	bool	isCurrentListBlockEmpty(void);
	bool	cmdStartList(const XML_Char * style);
	bool	cmdStopList(void);
	void	changeListStyle(fl_AutoNum* pAuto,
							List_Type lType,
							UT_uint32 startv,
							const XML_Char* pszDelim,
							const XML_Char* pszDecimal,
							const XML_Char* pszFormat,
							float Aligm,
							float Indent);

	void	setDontChangeInsPoint(void);
	void	allowChangeInsPoint(void);
	bool	isDontChangeInsPoint(void);


	bool	setCharFormat(const XML_Char * properties[]);
	bool	getCharFormat(const XML_Char *** properties,bool bExpandStyles=true);

	bool	setStyle(const XML_Char * style, bool bDontGeneralUpdate=false);
	bool	getStyle(const XML_Char ** style);
	bool appendStyle(const XML_Char ** style);

	UT_uint32		getCurrentPageNumber(void);

	bool    getEditableBounds(bool bEnd, PT_DocPosition & docPos, bool bOverride=false);

	void	insertParagraphBreak(void);
	void	insertParagraphBreaknoListUpdate(void);
	void	insertSectionBreak( BreakSectionType type);
	void	insertSectionBreak(void);
	void	insertSymbol(UT_UCSChar c, XML_Char * symfont);

	// ----------------------
	bool			isLeftMargin(UT_sint32 xPos, UT_sint32 yPos);
	void			cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd);
	void                    cmdSelect(PT_DocPosition dpBeg, PT_DocPosition dpEnd);
	void			cmdCharMotion(bool bForward, UT_uint32 count);
	bool			cmdCharInsert(UT_UCSChar * text, UT_uint32 count, bool bForce = false);
	void			cmdCharDelete(bool bForward, UT_uint32 count);
	void			delTo(FV_DocPos dp);
	UT_UCSChar * 	getSelectionText(void);
		
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos, bool bClick);
	void			moveInsPtTo(FV_DocPos dp);
	void 			moveInsPtTo(PT_DocPosition dp);
	void			warpInsPtNextPrevPage(bool bNext);
	void			warpInsPtNextPrevLine(bool bNext);
	void			extSelHorizontal(bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos, bool bDrag);
	void			extSelToXYword(UT_sint32 xPos, UT_sint32 yPos, bool bDrag);
	void			extSelTo(FV_DocPos dp);
	void			extSelNextPrevLine(bool bNext);
	void			endDrag(UT_sint32 xPos, UT_sint32 yPos);

	void			updateScreen(bool bDirtyRunsOnly=true);


// ----------------------

	bool			isPosSelected(PT_DocPosition pos) const;
	bool			isXYSelected(UT_sint32 xPos, UT_sint32 yPos) const;

	UT_UCSChar *	getContextSuggest(UT_uint32 ndx);
	void			cmdContextSuggest(UT_uint32 ndx, fl_BlockLayout * ppBL = NULL, fl_PartOfBlock * ppPOB = NULL);
	void			cmdContextIgnoreAll(void);
	void			cmdContextAdd(void);
// ----------------------
// Stuff for edittable Headers/Footers
//
	void                setHdrFtrEdit(fl_HdrFtrShadow * pShadow);
	void                clearHdrFtrEdit(void);
	bool                isHdrFtrEdit(void);
	fl_HdrFtrShadow *   getEditShadow(void);
    void                rememberCurrentPosition(void);
	PT_DocPosition      getSavedPosition(void);
	void                clearSavedPosition(void);
	void                markSavedPositionAsNeeded(void);
	bool                needSavedPosition(void);
	void                insertHeaderFooter(HdrFtrType hfType);
	bool				insertHeaderFooter(const XML_Char ** props, HdrFtrType hfType);

	void                cmdEditHeader(void);
	void                cmdEditFooter(void);

// ----------------------
// Stuff for edittable endnotes
//
	bool	insertEndnote();
	bool	insertEndnoteSection(const XML_Char * enpid);
	bool    insertEndnoteSection(const XML_Char ** blkprops, const XML_Char ** blkattrs);

// ----------------------

	bool 		gotoTarget(AP_JumpTarget type, UT_UCSChar * data);

	void			changeNumColumns(UT_uint32 iNumColumns);
	
// ----------------------

	// find and replace
	
	// aid the edit method for the simple non-dialog findAgain()
	bool 		findSetNextString(UT_UCSChar * string, bool matchCase);
	bool			findAgain(void);

	void 			findSetStartAtInsPoint(void);

	// finds the next "find" and selects it, filling bool when done the entire document
	bool			findNext(const UT_UCSChar * find, bool matchCase = true, bool * bDoneEntireDocument = NULL);
	bool			_findNext(const UT_UCSChar * find, bool matchCase = true, bool * bDoneEntireDocument = NULL);
	bool			_findNext(const UT_UCSChar * find, UT_uint32 *prefix, bool matchCase = true, bool * bDoneEntireDocument = NULL);
	// replaces the selection of "find" with "replace" and selects the next, filling
	// bool when done the entire document
	bool			_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
								 bool matchCase = false, bool * bDoneEntireDocument = NULL);
	bool			_findReplace(const UT_UCSChar * find, const UT_UCSChar * replace, UT_uint32 *prefix,
								 bool matchCase = false, bool * bDoneEntireDocument = NULL);
	bool			findReplace(const UT_UCSChar * find, const UT_UCSChar * replace,
								bool matchCase = false, bool * bDoneEntireDocument = NULL);
	// replaces every occurance of "find" with "replace" without stopping for anything
	UT_uint32		findReplaceAll(const UT_UCSChar * find, const UT_UCSChar * replace,
								   bool matchCase = false);
		
// ----------------------

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	void			Test_Dump(void);
#endif

// ----------------------

	FV_DocCount			countWords(void);

// -----------------------

	bool				insertPageNum(const XML_Char ** props, HdrFtrType hfType);
	void				setPoint(PT_DocPosition pt);

// -----------------------

	void				setShowPara(bool);
	inline bool		getShowPara(void) const { return m_bShowPara; };
	
	const fp_PageSize&	getPageSize(void) const;
	UT_uint32			calculateZoomPercentForPageWidth();
	UT_uint32			calculateZoomPercentForPageHeight();
	UT_uint32			calculateZoomPercentForWholePage();
	inline void             setViewMode (ViewMode vm) {m_viewMode = vm;}
	inline ViewMode         getViewMode (void) const  {return m_viewMode;}
	bool                isPreview(void) const {return VIEW_PREVIEW == m_viewMode;}
	void                setPreviewMode(PreViewMode pre) {m_previewMode = pre;}
	PreViewMode         getPreviewMode(void) { return m_previewMode;}

	inline PD_Document * getDocument (void) const {return m_pDoc;}

protected:
	void				_generalUpdate(void);
	
	void 				_draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32, bool bDirtyRunsOnly, bool bClip=false);
	
	void				_drawBetweenPositions(PT_DocPosition left, PT_DocPosition right);
	bool				_clearBetweenPositions(PT_DocPosition left, PT_DocPosition right, bool bFullLineHeightRect);
	
	bool				_ensureThatInsertionPointIsOnScreen(bool bDrawIP = true);
	void				_moveInsPtNextPrevPage(bool bNext);
	void				_moveInsPtNextPrevLine(bool bNext);
	fp_Page *			_getCurrentPage(void);
	void				_moveInsPtNthPage(UT_uint32 n);
	void				_moveInsPtToPage(fp_Page *page);
	void				_insertSectionBreak(void);

	PT_DocPosition		_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, bool bKeepLooking=true);
	PT_DocPosition		_getDocPos(FV_DocPos dp, bool bKeepLooking=true);
	void 				_findPositionCoords(PT_DocPosition pos,
											bool b,
											UT_sint32& x,
											UT_sint32& y,
											UT_sint32& x2, //these are needed for BiDi split carret
	  										UT_sint32& y2,

											UT_uint32& height,
											bool& bDirection,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun);
	
	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos) const;

	fp_Page*			_getPageForXY(UT_sint32 xPos,
									  UT_sint32 yPos,
									  UT_sint32& xClick,
									  UT_sint32& yClick) const;

	void				_moveToSelectionEnd(bool bForward);
	void				_eraseSelection(void);
	void				_clearSelection(void);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(PP_AttrProp *p_AttrProp_Before = NULL);
	bool				_insertFormatPair(const XML_Char * szName, const XML_Char * properties[]);
	void 				_eraseInsertionPoint();
	void				_drawInsertionPoint();
	void 				_updateInsertionPoint();
	void				_fixInsertionPointCoords();
	void 				_xorInsertionPoint();
	bool				_hasPointMoved(void);
	void				_saveCurrentPoint(void); 
	void				_clearOldPoint(void); 
	void				_drawSelection();
	void				_swapSelectionOrientation(void);
	void				_extSel(UT_uint32 iOldPoint);
	void				_extSelToPos(PT_DocPosition pos);
	UT_Error			_insertGraphic(FG_Graphic*, const char*);

	UT_UCSChar *		_lookupSuggestion(fl_BlockLayout* pBL, fl_PartOfBlock* pPOB, UT_uint32 ndx);

	static void			_autoScroll(UT_Timer * pTimer);
	static void			_autoDrawPoint(UT_Timer * pTimer);

	// localize handling of insertion point logic
	void				_setPoint(PT_DocPosition pt, bool bEOL = false);
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2);
	bool				_charMotion(bool bForward,UT_uint32 countChars);
	void				_doPaste(bool bUseClipboard, bool bHonorFormatting = true);
	void				_clearIfAtFmtMark(PT_DocPosition dpos);

	void				_checkPendingWordForSpell(void);

        bool                            _isSpaceBefore(PT_DocPosition pos);

	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	//the followingare BiDi specific, but need to be in place because of the
	//change to the signature of findPointCoords
	UT_sint32			m_xPoint2;
	UT_sint32			m_yPoint2;
	UT_sint32			m_oldxPoint2;
	UT_sint32			m_oldyPoint2;
	bool             m_bPointDirection;

#ifdef BIDI_ENABLED
	bool				m_bDefaultDirectionRtl;
#endif
	UT_uint32			m_iPointHeight;
	UT_sint32			m_oldxPoint;
	UT_sint32			m_oldyPoint;
	UT_uint32			m_oldiPointHeight;
	UT_sint32			m_xPointSticky;		// used only for _moveInsPtNextPrevLine()

	bool				m_bPointVisible;
	bool				m_bPointEOL;
	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	GR_Graphics*		m_pG;
	void*				m_pParentData;

	PT_DocPosition		m_iSelectionAnchor;
	PT_DocPosition		m_iSelectionLeftAnchor;
	PT_DocPosition		m_iSelectionRightAnchor;
	bool				m_bSelection;

	// autoscroll stuff
	UT_Timer *			m_pAutoScrollTimer;
	UT_sint32			m_xLastMouse;
	UT_sint32			m_yLastMouse;

	UT_Timer *			m_pAutoCursorTimer;
	bool				m_bCursorIsOn;
	bool				m_bEraseSaysStopBlinking;
	bool				m_bCursorBlink;

	bool				m_bdontSpellCheckRightNow;
	fv_ChangeState		m_chg;

	// find and replace stuff
	bool				m_wrappedEnd;
	PT_DocPosition		m_startPosition;

	bool				m_doneFind;

	bool                m_bEditHdrFtr;
	fl_HdrFtrShadow *   m_pEditShadow;
	PT_DocPosition      m_iSavedPosition;
	bool                m_bNeedSavedPosition;
	PT_DocPosition 		_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset);
	
	fl_BlockLayout * 	_findGetCurrentBlock(void);
	PT_DocPosition	 	_findGetCurrentOffset(void);	
	UT_UCSChar * 		_findGetNextBlockBuffer(fl_BlockLayout ** block, PT_DocPosition *offset);

	bool				_m_matchCase;
	UT_UCSChar * 		_m_findNextString;

	// search routines (these return values will fall short of an
	// extremely large document - fix them)
	UT_sint32 			_findBlockSearchDumbCase(const UT_UCSChar * haystack, const UT_UCSChar * needle);
	UT_sint32 			_findBlockSearchDumbNoCase(const UT_UCSChar * haystack, const UT_UCSChar * needle);	
	UT_sint32			_findBlockSearchRegexp(const UT_UCSChar * haystack, const UT_UCSChar * needle);

	// prefs listener - to change cursor blink on/off (and possibly others)
	static void _prefsListener( XAP_App *, XAP_Prefs *, UT_StringPtrMap *, void *);

	bool		m_bShowPara;
	ViewMode        m_viewMode;
	PreViewMode m_previewMode;
};

#endif /* FV_VIEW_H */






