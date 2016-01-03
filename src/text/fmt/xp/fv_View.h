/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#ifndef FV_VIEW_H
#define FV_VIEW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <vector>

#include "xap_Features.h"
#include "ut_types.h"
#include "xav_View.h"
#include "pt_Types.h"
#include "fp_types.h"
#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
#include "ev_EditBits.h"

// have to include these as they are instantiated in the FV_View
// class definition
#include "fv_FrameEdit.h"
#include "fv_Selection.h"
#include "fv_InlineImage.h"

#ifdef TOOLKIT_GTK_ALL
#include "fv_UnixVisualDrag.h"
#include "fv_UnixFrameEdit.h"
#include "fv_UnixInlineImage.h"
#if !defined(TOOLKIT_GTK2)
#include "fv_UnixSelectionHandles.h"
#endif
#else
#include "fv_VisualDragText.h"
#endif
#include "fv_SelectionHandles.h"

#define AUTO_SCROLL_MSECS	100
#define STD_DOUBLE_BUFFERING_FOR_THIS_FUNCTION FV_ViewDoubleBuffering dblBuffObj(this, true, true); dblBuffObj.beginDoubleBuffering();

class FL_DocLayout;
class FV_Caret_Listener;

class fl_DocSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_DocListener;
class fl_BlockLayout;
class fl_PartOfBlock;
class fl_AutoNum;
class fl_EndnoteLayout;

class fp_PageSize;
class fp_Page;
class fp_Run;
class fp_HyperlinkRun;
class fp_CellContainer;

class FG_Graphic;

class pf_Frag_Strux;
class PD_Document;
class PP_AttrProp;
class PP_RevisionAttr;

class GR_Graphics;
class FV_ViewDoubleBuffering;
struct dg_DrawArgs;

class UT_Worker;
class UT_Timer;
class UT_UTF8String;
class UT_StringPtrMap; // TODO remove.

class AP_TopRulerInfo;
class AP_LeftRulerInfo;
class AP_TopRuler;
class AP_LeftRuler;

class AP_Dialog_Annotation;
class AP_Dialog_SplitCells;

class XAP_App;
class XAP_Prefs;

class SpellChecker;

class CellLine;

typedef enum
{
	hori_left,
	hori_mid,
	hori_right,
	vert_above,
	vert_mid,
	vert_below
} AP_CellSplitType;


typedef enum _AP_JumpTarget
{
	AP_JUMPTARGET_PAGE, 			// beginning of page
	AP_JUMPTARGET_LINE,
	AP_JUMPTARGET_BOOKMARK,
//	AP_JUMPTARGET_PICTURE, TODO
    AP_JUMPTARGET_XMLID,
    AP_JUMPTARGET_ANNOTATION
} AP_JumpTarget;

struct fv_ChangeState
{
	bool				bUndo;
	bool				bRedo;
	bool				bDirty;
	bool				bSelection;
	UT_uint32			iColumn;
	fl_CellLayout *     pCellLayout;
	const gchar **	propsChar;
	const gchar **	propsBlock;
	const gchar **	propsSection;
};

struct FV_DocCount
{
	UT_uint32 word;
	UT_uint32 para;
	UT_uint32 ch_no;
	UT_uint32 ch_sp;
	UT_uint32 line;
	UT_uint32 page;
       // sometimes people want to have a word count without footnotes/endnotes included
	UT_uint32 words_no_notes;
};

class ABI_EXPORT fv_PropCache
{
public:
	fv_PropCache(void);
	~fv_PropCache(void);
	UT_uint32         getTick(void) const;
	void              setTick(UT_uint32 iTick);
	fl_ContainerLayout * getCurrentCL(void) const;
	void              setCurrentCL(fl_ContainerLayout* pCL);
	bool              isValid(void) const;
	const gchar ** getCopyOfProps(void) const;
	void              fillProps(UT_uint32 numProps, const gchar ** props);
	void              clearProps(void);
private:
	UT_uint32         m_iTick;
	UT_uint32         m_iNumProps;
	gchar **       m_pszProps;
	fl_ContainerLayout* m_pCurrentCL;
};

enum FV_BIDI_Order
{
	FV_Order_Visual = 0,
	FV_Order_Logical_LTR = UT_BIDI_LTR,
	FV_Order_Logical_RTL = UT_BIDI_RTL
};

class ABI_EXPORT fv_CaretProps
{
public:
        fv_CaretProps(FV_View * pView, PT_DocPosition InsPoint);
	virtual ~fv_CaretProps(void);
	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	UT_sint32			m_xPoint2;
	UT_sint32			m_yPoint2;
	bool				m_bPointDirection;
	bool				m_bDefaultDirectionRtl;
	bool				m_bUseHebrewContextGlyphs;
	bool				m_bPointEOL;
	UT_uint32			m_iPointHeight;
	UT_RGBColor			m_caretColor;
	FV_Caret_Listener*  m_PropCaretListner;
	GR_Caret*			m_pCaret;
	UT_uint32			m_ListenerID;
	FV_View*			m_pView;
	UT_sint32			m_iAuthorId;
	std::string			m_sCaretID;
};

/**
 * A RAII class for blocking popup bubbles that are used by
 * annotations, RDF and maybe other parts of the system. To get a real
 * one of these use FV_View::getBubbleBlocker() when this object is
 * destroyed the counter is decremented for you. To explicitly
 * decrement, simply assign your FV_View_BubbleBlocker to
 * FV_View_BubbleBlocker().
 *
 * While any real FV_View_BubbleBlocker objects are held, popup
 * bubbles are suspended and not created. This is useful for making
 * dialog windows where you might want to avoid the popup bubbles from
 * overlapping the dialog window.
 *
 * Both AP_Dialog_Modeless and AP_Dialog_Modal obtain and release
 * these BubbleBlocker objects for you automatically, so dialog
 * subclasses of those do not have to worry about this at all.
 */
class ABI_EXPORT FV_View_BubbleBlocker
{
    friend class FV_View;
    FV_View* m_pView;
  public:
    FV_View_BubbleBlocker( FV_View* pView = 0 );
    ~FV_View_BubbleBlocker();
    FV_View_BubbleBlocker& operator=( const FV_View_BubbleBlocker& r );

};

class ABI_EXPORT FV_View : public AV_View
{
	friend class fl_DocListener;
	friend class fl_BlockLayout;
	friend class FL_DocLayout;
	friend class fl_Squiggles;
	friend class fl_DocSectionLayout;
	friend class GR_Caret;
	friend class FV_FrameEdit;
	friend class FV_VisualDragText;
	friend class FV_VisualInlineImage;
	friend class FV_Selection;
	friend class CellLine;
    friend class FV_View_BubbleBlocker;
	friend class FV_ViewDoubleBuffering;
	friend class FV_SelectionHandles;
public:
	FV_View(XAP_App*, void*, FL_DocLayout*);
	virtual ~FV_View();

	virtual inline GR_Graphics*    getGraphics(void) const { return m_pG; }
	void  setGraphics(GR_Graphics *pG);
	void  replaceGraphics(GR_Graphics *pG);

	virtual inline PT_DocPosition   getPoint(void) const { return m_iInsPoint; }
	PT_DocPosition	getSelectionAnchor(void) const;
	PT_DocPosition	getSelectionLeftAnchor(void) const;
	PT_DocPosition	getSelectionRightAnchor(void) const;
	UT_uint32       getSelectionLength(void) const;

	UT_sint32       getFrameMargin(void) const;

	virtual void focusChange(AV_Focus focus);
	virtual bool    isActive(void) const;

	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual void	cmdHyperlinkJump(UT_sint32 xPos, UT_sint32 yPos);
	void	        cmdHyperlinkJump(PT_DocPosition pos);
	void			cmdHyperlinkCopyLocation(PT_DocPosition pos);

	virtual void	draw(const UT_Rect* pRect=static_cast<UT_Rect*>(NULL));
	virtual void 	drawSelectionBox(UT_Rect & box, bool drawHandles);

	void			setVisualSelectionEnabled(bool bActive);
private:
	inline void 	_drawResizeHandle(UT_Rect & box);
    void getCmdInsertRangeVariables( PT_DocPosition& posStart,
                                     PT_DocPosition& posEnd,
                                     fl_BlockLayout*& pBL1,
                                     fl_BlockLayout*& pBL2 );
	void _updateSelectionHandles (void);


public:
	const PP_AttrProp * getAttrPropForPoint() const;

	virtual bool	notifyListeners(const AV_ChangeMask hint);

	virtual bool	canDo(bool bUndo) const;
	virtual UT_uint32 undoCount (bool bUndo) const;
	virtual void	cmdUndo(UT_uint32 count);
	virtual void	cmdRedo(UT_uint32 count);
	virtual UT_Error	cmdSave(void);
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft);
	virtual UT_Error		cmdSaveAs(const char * szFilename, int ieft, bool cpy);

	UT_Error		cmdInsertField(const char* szName, const gchar ** extra_attrs = NULL, const gchar ** extra_props = NULL);
	UT_Error		cmdInsertBookmark(const char* szName);
	UT_Error		cmdDeleteBookmark(const char* szName);
	UT_Error		cmdInsertHyperlink(const char* szName, const char* szTitle = NULL);
	UT_Error		cmdInsertXMLID(const std::string& name);
	UT_Error		cmdDeleteXMLID(const std::string& name);

	fp_Run *        getHyperLinkRun(PT_DocPosition pos);
	UT_Error		cmdDeleteHyperlink();
	bool                    cmdInsertMathML(const char * szFileName,
						PT_DocPosition pos);
	bool	        cmdInsertEmbed(const UT_ByteBuf * pBuf,PT_DocPosition pos,const char * szMime,const char * szProps);
	bool            cmdUpdateEmbed(const UT_ByteBuf * pBuf, const char * szMime, const char * szProps);
	bool	        cmdUpdateEmbed(fp_Run * pRun, const UT_ByteBuf * pBuf, const char * szMime, const char * szProps);
	bool	        cmdDeleteEmbed(fp_Run * pRun);

	bool                    cmdInsertLatexMath(UT_UTF8String & sLatex,
						   UT_UTF8String & sMath, bool compact);

	UT_Error		cmdInsertTOC(void);
	UT_Error		cmdHyperlinkStatusBar(UT_sint32 xPos, UT_sint32 yPos);

	UT_Error		cmdInsertGraphic(FG_Graphic*);
	UT_Error        cmdInsertGraphicAtStrux(FG_Graphic* pFG, PT_DocPosition iPos, PTStruxType iStruxType);
	virtual void	toggleCase(ToggleCase c);
	virtual void	setPaperColor(const gchar * clr);

	virtual bool    isDocumentPresent(void) const;
	virtual void	cmdCopy(bool bToClipboard = true);
	virtual void	cmdCut(void);
	virtual void	cmdPaste(bool bHonorFormatting = true);
	virtual void	cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos);

	void            pasteFromLocalTo(PT_DocPosition pos);
	void            _pasteFromLocalTo(PT_DocPosition pos);
	void            copyToLocal(PT_DocPosition pos1, PT_DocPosition pos2);
	void		copyTextToClipboard(const UT_UCS4String sIncoming, bool useClipboard=true);

	virtual void	getTopRulerInfo(AP_TopRulerInfo * pInfo);
	virtual void	getTopRulerInfo(PT_DocPosition pos, AP_TopRulerInfo * pInfo);
	virtual void	getLeftRulerInfo(AP_LeftRulerInfo * pInfo);
	virtual void	getLeftRulerInfo(PT_DocPosition pos, AP_LeftRulerInfo * pInfo);
        virtual void    setCursorWait(void);
	virtual void    clearCursorWait(void);
	virtual void    setCursorToContext(void);
	EV_EditMouseContext         getLastMouseContext(void);
	void                getMousePos(UT_sint32 * x, UT_sint32 * y);

	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos);
	EV_EditMouseContext _getMouseContext(UT_sint32 xPos, UT_sint32 yPos);
	virtual EV_EditMouseContext getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos);
	void                setPrevMouseContext(EV_EditMouseContext  emc)
	{m_prevMouseContext = emc;}

	virtual void        updateLayout(void);
	virtual void        rebuildLayout(void);
	virtual void        remeasureCharsWithoutRebuild();
	virtual void        fontMetricsChange();
	virtual bool		isSelectionEmpty(void) const;
	bool                isSelectAll(void) const
	{ return m_Selection.isSelectAll();}
	virtual void		cmdUnselectSelection(void);
	void				getDocumentRangeOfCurrentSelection(PD_DocumentRange * pdr) const;
	PT_DocPosition		mapDocPos( FV_DocPos dp );
	PT_DocPosition		mapDocPosSimple( FV_DocPos dp );
	PT_DocPosition saveSelectedImage (const char * toFile );
	PT_DocPosition saveSelectedImage (const UT_ByteBuf ** outByteBuf);
	PT_DocPosition getSelectedImage(const char **dataId) const;
	PT_DocPosition getSelectedImage(const char **dataId,const fp_Run **pImRun) const;
	fp_Run *getSelectedObject(void) const;

	void            getTextInCurrentBlock(UT_GrowBuf & buf) const;
	void            getTextInCurrentSection(UT_GrowBuf & buf) const;
	void            getTextInDocument(UT_GrowBuf & buf) const;
	bool            getLineBounds(PT_DocPosition pos, PT_DocPosition *start, PT_DocPosition *end);
	UT_UCSChar getChar(PT_DocPosition pos, UT_sint32 *x = NULL, UT_sint32 *y = NULL, UT_uint32 *width = NULL, UT_uint32 *height = NULL);

// ----------------------
	FL_DocLayout*	getLayout() const;
	UT_uint32		getCurrentPageNumForStatusBar(void) const;
	fp_Page*		getCurrentPage(void) const;
	fl_BlockLayout* getCurrentBlock(void) const;

	void draw(int page, dg_DrawArgs* da);


	// TODO some of these functions should move into protected

	void	getPageScreenOffsets(const fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff) const;
	void	getPageYOffset(const fp_Page* pPage, UT_sint32& yoff) const;
	virtual UT_sint32 getPageViewLeftMargin(void) const;
	virtual UT_sint32 getPageViewTopMargin(void) const;
	virtual UT_sint32 getPageViewSep(void) const;

	bool	setSectionFormat(const gchar * properties[]);
	bool	getSectionFormat(const gchar *** properties) const;

	bool	setBlockIndents(bool doLists, double indentChange, double page_size);
	bool    setCollapsedRange(PT_DocPosition posLow,PT_DocPosition posHigh, const gchar * properties[]);
	bool	setBlockFormat(const gchar * properties[]);
	bool	getBlockFormat(const gchar *** properties,bool bExpandStyles=true) const;
	bool    removeStruxAttrProps(PT_DocPosition ipos1, PT_DocPosition ipos2, PTStruxType iStrux,const gchar * attributes[] ,const gchar * properties[]);
	bool    isImageAtStrux(PT_DocPosition ipos1, PTStruxType iStrux);

	bool	processPageNumber(HdrFtrType hfType, const gchar ** atts);

#ifdef ENABLE_SPELL
	bool	isTextMisspelled()const ;
#endif
	bool	isTabListBehindPoint(UT_sint32 & iNumToDelete) const;
	bool	isTabListAheadPoint(void) const;
	void	processSelectedBlocks(FL_ListType listType);
	void	getBlocksInSelection(UT_GenericVector<fl_BlockLayout*> * vBlock, bool bAllBlocks = true) const;
	UT_sint32 getNumColumnsInSelection(void) const;
	UT_sint32 getNumRowsInSelection(void) const;
	void	getAllBlocksInList(UT_GenericVector<fl_BlockLayout *> * vBlock) const;
	bool	isPointBeforeListLabel(void) const;
	bool	isCurrentListBlockEmpty(void) const;
	bool	cmdStartList(const gchar * style);
	bool	cmdStopList(void);
	void	changeListStyle(fl_AutoNum* pAuto,
							FL_ListType lType,
							UT_uint32 startv,
							const gchar* pszDelim,
							const gchar* pszDecimal,
							const gchar* pszFormat,
							float Aligm,
							float Indent);

	void	setDontChangeInsPoint(void);
	void	allowChangeInsPoint(void);

	bool    getAttributes(const PP_AttrProp ** ppSpanAP, const PP_AttrProp ** ppBlockAP = NULL, PT_DocPosition posStart = 0) const;

	/* Experimental, for the moment; use with caution. - fjf, 24th Oct. '04
	 */
	// - begin
	bool    getAllAttrProp(const PP_AttrProp *& pSpanAP, const PP_AttrProp *& pBlockAP, const PP_AttrProp *& pSectionAP, const PP_AttrProp *& pDocAP) const;
	bool	queryCharFormat(const gchar * szProperty, UT_UTF8String & szValue, bool & bExplicitlyDefined, bool & bMixedSelection) const;
	bool	queryCharFormat(const gchar * szProperty, UT_UTF8String & szValue, bool & bExplicitlyDefined, PT_DocPosition position) const;
	// - end

	bool	setCharFormat(const gchar * properties[], const gchar * attribs[] = NULL);
	bool	setCharFormat(const std::vector<std::string>& properties);
	bool	resetCharFormat(bool bAll);
	bool	getCharFormat(const gchar *** properties,bool bExpandStyles=true) const;
	bool	getCharFormat(const gchar *** properties,bool bExpandStyles, PT_DocPosition posStart) const;
	fl_BlockLayout * getBlockFromSDH(pf_Frag_Strux* sdh);
	bool	setStyle(const gchar * style, bool bDontGeneralUpdate=false);
	bool	setStyleAtPos(const gchar * style, PT_DocPosition posStart, PT_DocPosition posEnd, bool bDontGeneralUpdate=false);
	bool    isNumberedHeadingHere(fl_BlockLayout * pBlock) const;
	bool	getStyle(const gchar ** style) const;
	bool appendStyle(const gchar ** style);

	UT_uint32		getCurrentPageNumber(void) const;

	bool	getEditableBounds(bool bEnd, PT_DocPosition & docPos, bool bOverride=false)const;

	bool    isParaBreakNeededAtPos(PT_DocPosition pos) const;
	bool    insertParaBreakIfNeededAtPos(PT_DocPosition pos);
	void	insertParagraphBreak(void);
	void	insertParagraphBreaknoListUpdate(void);
	void	insertSectionBreak( BreakSectionType type);
	void	insertSectionBreak(void);
	void	insertSymbol(UT_UCSChar c, const gchar * symfont);

	// ----------------------
	bool			isLeftMargin(UT_sint32 xPos, UT_sint32 yPos) const;
    void            selectRange( PT_DocPosition start, PT_DocPosition end );
    void            selectRange( const std::pair< PT_DocPosition, PT_DocPosition >& range );
	void			cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd);
	void			cmdSelectTOC(UT_sint32 xPos, UT_sint32 yPos);
	bool            isTOCSelected(void) const;
	bool            setTOCProps(PT_DocPosition pos, const char * szProps);

	bool			cmdSelectNoNotify(PT_DocPosition dpBeg, PT_DocPosition dpEnd);
	void			cmdSelect(PT_DocPosition dpBeg, PT_DocPosition dpEnd);
	void			cmdSelect( const std::pair< PT_DocPosition, PT_DocPosition >& range );
	void			cmdCharMotion(bool bForward, UT_uint32 count);
	bool			cmdCharInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce = false);
	bool			cmdCharInsert(const std::string& s, bool bForce = false);
	void			cmdCharDelete(bool bForward, UT_uint32 count);
	void			delTo(FV_DocPos dp);
	void            getSelectionText(UT_UCS4Char *& text) const;

	UT_UCSChar *	getTextBetweenPos(PT_DocPosition pos1, PT_DocPosition pos2) const;
	inline PT_DocPosition  getInsPoint () const { return m_iInsPoint; }
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos, bool bClick);
	void			moveInsPtTo(FV_DocPos dp, bool bClearSelection = true);
	void			moveInsPtTo(PT_DocPosition dp);
	void			warpInsPtNextPrevPage(bool bNext);
	void			warpInsPtNextPrevLine(bool bNext);
	void            warpInsPtNextPrevScreen(bool bNext);
	void			extSelHorizontal(bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos, bool bDrag);
	void			extSelToXYword(UT_sint32 xPos, UT_sint32 yPos, bool bDrag);
	void			extSelTo(FV_DocPos dp);
	void			swapSelectionOrientation(void);

#ifdef ENABLE_SPELL
	SpellChecker * getDictForSelection () const;
#endif
	void			extSelNextPrevLine(bool bNext);
	void            extSelNextPrevPage(bool bNext);
	void            extSelNextPrevScreen(bool bNext);
	void			endDrag(UT_sint32 xPos, UT_sint32 yPos);

	void endDragSelection(UT_sint32 xPos, UT_sint32 yPos);

	PT_DocPosition  getDocPositionFromXY(UT_sint32 xpos, UT_sint32 ypos, bool bNotFrames = false);
	PT_DocPosition  getDocPositionFromLastXY(void);

	fl_BlockLayout* getBlockAtPosition(PT_DocPosition pos) const {return _findBlockAtPosition(pos);};
	virtual void	updateScreen(bool bDirtyRunsOnly=true);
	bool            isInDocSection(PT_DocPosition pos = 0) const;

//---------
//Visual Drag stuff
//
	void            cutVisualText(UT_sint32 x, UT_sint32 y);
	void            copyVisualText(UT_sint32 x, UT_sint32 y);
	void            dragVisualText(UT_sint32 x, UT_sint32 y);
	void            pasteVisualText(UT_sint32 x, UT_sint32 y);
	void            btn0VisualDrag(UT_sint32 x, UT_sint32 y);
	const FV_VisualDragText * getVisualText(void) const
	  { return &m_VisualDragText;}
	FV_VisualDragText * getVisualText(void)
	  { return &m_VisualDragText;}
	const UT_ByteBuf * getLocalBuf(void) const;

//---------
//Visual Inline Image Drag stuff
//
	void            btn0InlineImage(UT_sint32 x, UT_sint32 y);
	void            btn1InlineImage(UT_sint32 x, UT_sint32 y);
	void            btn1CopyImage(UT_sint32 x, UT_sint32 y);
	void            dragInlineImage(UT_sint32 x, UT_sint32 y);
	void            releaseInlineImage(UT_sint32 x, UT_sint32 y);

// -------
// Frame stuff
//
	FV_FrameEdit *  getFrameEdit(void);
	void            btn0Frame(UT_sint32 x, UT_sint32 y);
	void            btn1Frame(UT_sint32 x, UT_sint32 y);
	void            dragFrame(UT_sint32 x, UT_sint32 y);
	void            releaseFrame(UT_sint32 x, UT_sint32 y);
	bool            isInFrame(PT_DocPosition pos) const;
	void            deleteFrame(void);
	void            copyFrame(bool b_keepFrame = true);
	void            selectFrame(void);
	bool            isFrameSelected(void) const;
	void            activateFrame(void);
	fl_FrameLayout * getFrameLayout(PT_DocPosition pos) const;
	fl_FrameLayout * getFrameLayout(void) const;
	void            setFrameFormat(const gchar ** props);
	void            setFrameFormat(const gchar ** attribs, const gchar ** props,
								   fl_BlockLayout * pNewBL = NULL);
	void            setFrameFormat(const gchar ** props,FG_Graphic * pFG, const std::string & dataID,
								   fl_BlockLayout * pNewBL = NULL);
	bool            getFrameStrings_view(UT_sint32 x, UT_sint32 y,fv_FrameStrings & FrameStrings,
										 fl_BlockLayout ** pCloseBL,fp_Page ** ppPage);
	void            convertInLineToPositioned(PT_DocPosition pos,
											const gchar ** attribs);

	bool            convertPositionedToInLine(fl_FrameLayout * pFrame);
	UT_Error        cmdInsertPositionedGraphic(FG_Graphic* pFG,UT_sint32 mouseX, UT_sint32 mouseY);
	UT_Error        cmdInsertPositionedGraphic(FG_Graphic* pFG);

// ----------------------

	bool			isPosSelected(PT_DocPosition pos) const;
	bool			isXYSelected(UT_sint32 xPos, UT_sint32 yPos) const;
	FV_SelectionMode getSelectionMode(void) const;
	FV_SelectionMode getPrevSelectionMode(void) const;
	PD_DocumentRange * getNthSelection(UT_sint32 i) const;
	UT_sint32          getNumSelections(void) const;
	void            setSelectionMode(FV_SelectionMode selMode);
#ifdef ENABLE_SPELL
// ----------------------
// Stuff for spellcheck context menu
//
	UT_UCSChar *	getContextSuggest(UT_uint32 ndx);
	void			cmdContextSuggest(UT_uint32 ndx, fl_BlockLayout * ppBL = NULL, fl_PartOfBlock * ppPOB = NULL);
	void			cmdContextIgnoreAll(void);
	void			cmdContextAdd(void);
#endif
// ----------------------
// Stuff for edittable Headers/Footers
//
	bool                isInHdrFtr(PT_DocPosition pos) const;
	void				setHdrFtrEdit(fl_HdrFtrShadow * pShadow);
	void				clearHdrFtrEdit(void);
	bool				isHdrFtrEdit(void) const;
	fl_HdrFtrShadow *	getEditShadow(void) const;
	void				rememberCurrentPosition(void);
	PT_DocPosition		getSavedPosition(void) const;
	void				clearSavedPosition(void);
	void				markSavedPositionAsNeeded(void);
	bool				needSavedPosition(void) const;
	void				insertHeaderFooter(HdrFtrType hfType);
	bool				insertHeaderFooter(const gchar ** props, HdrFtrType hfType, fl_DocSectionLayout * pDSL=NULL);

	void				cmdEditHeader(void);
	void				cmdEditFooter(void);

	void                cmdRemoveHdrFtr(bool isHeader);
	bool                isFooterOnPage(void) const;
	bool                isHeaderOnPage(void) const;

	void                SetupSavePieceTableState(void);
	void                RestoreSavedPieceTableState(void);
    void                removeThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves = false);
	void                createThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves = false);
	void                populateThisHdrFtr(HdrFtrType hfType, bool bSkipPTSaves = false);
	void                _populateThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtrSrc, fl_HdrFtrSectionLayout * pHdrFtrDest);

//
// ----------------------
// Stuff for edittable Footnote/Endnotes
//
	bool	            insertFootnote(bool bFootnote);
	bool	            insertFootnoteSection(bool bFootnote,const gchar * enpid);
	bool                isInFootnote(PT_DocPosition pos) const;
	bool                isInFootnote(void) const;
	bool                isInEndnote(PT_DocPosition pos) const;
	bool                isInEndnote(void) const;
	bool                isInAnnotation(PT_DocPosition pos) const;
	bool                isInAnnotation(void) const;
	fl_FootnoteLayout * getClosestFootnote(PT_DocPosition pos) const;
	fl_EndnoteLayout *  getClosestEndnote(PT_DocPosition pos) const;
	fl_AnnotationLayout *  getClosestAnnotation(PT_DocPosition pos) const;
	UT_sint32           getEmbedDepth(PT_DocPosition pos) const;
	//
	// ----------------------------------
	// Stuff for Annotaions
	//
	bool				insertAnnotation(UT_sint32 iAnnotation,
										 const std::string & sDescr,
										 const std::string & sAuthor,
										 const std::string & sTitle,
										 bool bReplace);
	bool                getAnnotationText(UT_uint32 iAnnotation, std::string & sText) const;
    std::string         getAnnotationText(UT_uint32 iAnnotation) const;
	bool                setAnnotationText(UT_uint32 iAnnotation, const std::string & sText);
	bool                setAnnotationText(UT_uint32 iAnnotation, const std::string & sText,
                                          const std::string & sAuthor, const std::string & sTitle);
	bool                getAnnotationRichText(UT_uint32 iAnnotation, std::string & sRTF) const;
    bool                setAnnotationRichText(UT_uint32 iAnnotation, const std::string & sRTF);
	// TODO getters and setters to implement/change/add as judged necessary
	bool                getAnnotationTitle(UT_uint32 iAnnotation, std::string & sTitle) const;
    std::string         getAnnotationTitle(UT_uint32 iAnnotation) const;
	bool                setAnnotationTitle(UT_uint32 iAnnotation, const std::string & sTitle);
	bool                getAnnotationAuthor(UT_uint32 iAnnotation, std::string & sAuthor) const;
    std::string         getAnnotationAuthor(UT_uint32 iAnnotation) const;
	bool                setAnnotationAuthor(UT_uint32 iAnnotation, const std::string & sAuthor);

	bool                isAnnotationPreviewActive(void) const { return m_bAnnotationPreviewActive;}
	void                setAnnotationPreviewActive(bool b) { m_bAnnotationPreviewActive = b;}
	UT_uint32			getActivePreviewAnnotationID() const { return m_iAnnPviewID;}
	void				setActivePreviewAnnotationID(UT_uint32 iID) { m_iAnnPviewID = iID;}
	void				killAnnotationPreview();
	bool				cmdEditAnnotationWithDialog(UT_uint32 aID);
	fl_AnnotationLayout * insertAnnotationDescription(UT_uint32 aID, AP_Dialog_Annotation *pDialog);
	fl_AnnotationLayout * getAnnotationLayout(UT_uint32 iAnnotation) const;
	bool                selectAnnotation(fl_AnnotationLayout * pAL);
	UT_uint32           countAnnotations(void) const;

    FV_View_BubbleBlocker getBubbleBlocker();
    bool                  bubblesAreBlocked() const;
// ----------------------

	bool		gotoTarget(AP_JumpTarget type, const UT_UCSChar * data);
	bool		gotoTarget(AP_JumpTarget type, const char *numberString);

	void			changeNumColumns(UT_uint32 iNumColumns);

// ----------------------

	// find and replace

	void			findSetFindString	(const UT_UCSChar* string);
	void			findSetReplaceString(const UT_UCSChar* string);
	void			findSetReverseFind	(bool newValue);
	void			findSetMatchCase	(bool newValue);
	void			findSetWholeWord	(bool newValue);
	UT_UCSChar *	findGetFindString   (void);
	UT_UCSChar *	findGetReplaceString(void);
	bool			findGetReverseFind	();
	bool			findGetMatchCase	();
	bool			findGetWholeWord	();

	bool			findAgain(void);

	void			findSetStartAt(PT_DocPosition pos);
	void			findSetStartAtInsPoint(void);

	bool			findNext(bool& bDoneEntireDocument);
	bool			findNext(const UT_UCSChar* pFind, bool& bDoneEntireDocument);

	UT_uint32*		_computeFindPrefix(const UT_UCSChar* pFind);

	bool			_findNext(UT_uint32* pPrefix,
							 bool& bDoneEntireDocument);

	bool			findPrev(bool& bDoneEntireDocument);
	bool			findPrev(const UT_UCSChar* pFind, bool& bDoneEntireDocument);

	bool			_findPrev(UT_uint32* pPrefix,
							  bool& bDoneEntireDocument);

	bool			findReplaceReverse(bool& bDoneEntireDocument);

	bool			_findReplaceReverse(UT_uint32* pPrefix,
										bool& bDoneEntireDocument,
										bool bNoUpdate);

	bool			_findReplace(UT_uint32* pPrefix,
								 bool& bDoneEntireDocument,
								 bool bNoUpdate);


	bool			findReplace(bool& bDoneEntireDocument);

	UT_uint32		findReplaceAll();

// ----------------------

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	void			Test_Dump(void);
#endif

// ----------------------

	FV_DocCount 		countWords(bool bActuallyCountWords = true);

// -----------------------

	bool				insertPageNum(const gchar ** props, HdrFtrType hfType);
	virtual void        setPoint(UT_uint32 pt);
	void                ensureInsertionPointOnScreen(void);
    void                removeCaret(const std::string& sCaretID);
	void                addCaret(PT_DocPosition docPos,UT_sint32 iAuthorId);
	void                setPointRemote(PT_DocPosition docPos);
	void                updateCarets(PT_DocPosition docPos, UT_sint32 iLen);
	void		    fixInsertionPointCoords(void);

// -----------------------
	void                killBlink(void);
	void				setShowPara(bool);
	inline bool 	getShowPara(void) const { return m_bShowPara; };

	const fp_PageSize&	getPageSize(void) const;
	virtual UT_uint32	calculateZoomPercentForPageWidth() const;
	virtual UT_uint32	calculateZoomPercentForPageHeight() const;
	virtual UT_uint32	calculateZoomPercentForWholePage() const;
	void 			    setViewMode (ViewMode vm);
	ViewMode 			getViewMode (void) const  {return m_viewMode;}
	bool				isPreview(void) const {return VIEW_PREVIEW == m_viewMode;}
	void				setPreviewMode(PreViewMode pre) {m_previewMode = pre;}
	PreViewMode 		getPreviewMode(void) { return m_previewMode;}

	UT_uint32           getTabToggleAreaWidth() const;
	UT_sint32           getNormalModeXOffset() const;

	void				setScreenUpdateOnGeneralUpdate( bool bDoit)
		{m_bDontUpdateScreenOnGeneralUpdate = !bDoit;}
	bool				shouldScreenUpdateOnGeneralUpdate(void) const
		{ return !m_bDontUpdateScreenOnGeneralUpdate;}

	inline PD_Document * getDocument (void) const {return m_pDoc;}

	/* Revision related functions */
	void                toggleMarkRevisions();
	void                cmdAcceptRejectRevision(bool bReject, UT_sint32 x, UT_sint32 y);
	// NB: 'mark revisions' state is document-wide
	bool                isMarkRevisions() const;
	// NB: 'show revisions' state is view-specific
	bool                isShowRevisions() const {return m_bShowRevisions;}
	void                toggleShowRevisions();
	void                setShowRevisions(bool bShow);

	void                cmdSetRevisionLevel(UT_uint32 i);
	UT_uint32           getRevisionLevel()const;
	void                setRevisionLevel(UT_uint32 i);

	bool                cmdFindRevision(bool bNext, UT_sint32 xPos, UT_sint32 yPos);
	bool                doesSelectionContainRevision() const;

	void                updateRevisionMode();
  protected:
	void                _fixInsertionPointAfterRevision();
	bool                _makePointLegal(void);
  public:

	/* Table related functions */
	bool                isPointLegal(PT_DocPosition pos) const;
	bool                isPointLegal(void) const;
	bool				isInTable() const;
	fl_TableLayout *    getTableAtPos(PT_DocPosition) const;
	bool				isInTable(PT_DocPosition pos) const;
	bool                cmdAutoSizeCols(void);
	bool                cmdTextToTable(UT_uint32 iDelim);
	bool                cmdAutoSizeRows(void);
	bool                cmdAdvanceNextPrevCell(bool bGoNext);
	fp_CellContainer *  getCellAtPos(PT_DocPosition pos) const;
	PT_DocPosition      findCellPosAt(PT_DocPosition posTable, UT_sint32 row, UT_sint32 col) const;
	bool                _deleteCellAt(PT_DocPosition posTable,UT_sint32 row, UT_sint32 col);
	bool                _restoreCellParams(PT_DocPosition posTable, pf_Frag_Strux* tableSDH);
	bool                _changeCellParams(PT_DocPosition posTable,pf_Frag_Strux* tableSDH );
	bool                deleteCellAt(PT_DocPosition posTable,UT_sint32 row, UT_sint32 col);
	bool                cmdDeleteCell(PT_DocPosition pos);
	bool                cmdDeleteCol(PT_DocPosition pos);
	bool                cmdDeleteRow(PT_DocPosition pos);
	bool                cmdDeleteTable(PT_DocPosition pos, bool bDontNotify=false);
	bool                cmdInsertRow(PT_DocPosition posTable, bool bBfore);
	bool                cmdInsertCol(PT_DocPosition posTable, bool bBefore);
	bool                cmdSplitCells(AP_CellSplitType iSplitType);
	bool                cmdSelectColumn(PT_DocPosition posOfColumn);
	bool                cmdAutoFitTable(void);
	bool                cmdMergeCells(PT_DocPosition posSource, PT_DocPosition posDestination);
	bool                cmdTableToText(PT_DocPosition posSource,UT_sint32 iSepType);

	bool                _MergeCells( PT_DocPosition posDestination,PT_DocPosition posSource, bool bBefore);
	bool                getCellParams(PT_DocPosition posCol, UT_sint32 *iLeft,
									  UT_sint32 *iRight,UT_sint32 *iTop, UT_sint32 *iBot) const;
	bool				getCellLineStyle(PT_DocPosition posCell, UT_sint32 * pLeft, UT_sint32 * pRight,
										 UT_sint32 * pTop, UT_sint32 * pBot) const;
	bool				setCellFormat(const gchar * properties[], FormatTable applyTo, FG_Graphic * pFG, UT_String & sDataID);
	bool				getCellProperty(PT_DocPosition pos, const gchar * szPropName, gchar * &szPropValue) const;
	bool	            setTableFormat(const gchar * properties[]);
	bool	            setTableFormat(PT_DocPosition pos,const gchar * properties[]);
	bool                getCellFormat(PT_DocPosition pos, UT_String & sCellProps) const;

	UT_Error            cmdInsertTable(UT_sint32 numRows, UT_sint32 numCols,
									   const gchar * pPropsArray[]);
	void				_generalUpdate(void);

	UT_RGBColor			getColorShowPara(void) const { return m_colorShowPara; }
#ifdef ENABLE_SPELL
	UT_RGBColor			getColorSquiggle(FL_SQUIGGLE_TYPE iSquiggleType) const;
#endif
	UT_RGBColor			getColorMargin(void) const { return m_colorMargin; }
	UT_RGBColor			getColorSelBackground(void);
	UT_RGBColor			getColorSelForeground(void) const;
	UT_RGBColor			getColorFieldOffset(void) const { return m_colorFieldOffset; }
	UT_RGBColor			getColorImage(void) const { return m_colorImage; }
	UT_RGBColor			getColorImageResize(void) const { return m_colorImageResize; }
	UT_RGBColor			getColorHyperLink(void) const { return m_colorHyperLink; }
	UT_RGBColor			getColorAnnotation(const fp_Run * pRun) const;
	UT_RGBColor			getColorAnnotation(fp_Page * pPage,UT_uint32 pid) const;
	UT_RGBColor			getColorRDFAnchor(const fp_Run * pRun) const;
	UT_RGBColor			getColorRevisions(int rev) const {
		if ((rev < 0) || (rev > 9)) rev = 9;
		return m_colorRevisions[rev]; }
	UT_RGBColor			getColorHdrFtr(void) const { return m_colorHdrFtr; }
	UT_RGBColor			getColorColumnLine(void) const { return m_colorColumnLine; }

	void                getVisibleDocumentPagesAndRectangles(UT_GenericVector<UT_Rect*> &vRect,
															 UT_GenericVector<fp_Page*> &vPages) const;

	//
	// image selection && resizing && dragging functions
	//
	UT_sint32			getImageSelInfo() const;
	GR_Graphics::Cursor getImageSelCursor() const;
	bool                isImageSelected(void) const;

//
// Table resizing
//
	void                setDragTableLine(bool bSet)
                        { m_bDragTableLine = bSet;}
	bool                getDragTableLine(void) const
		                { return m_bDragTableLine;}
	void                setTopRuler(AP_TopRuler * pRuler)
                        { m_pTopRuler = pRuler;}
	AP_TopRuler *       getTopRuler(void) const
		                { return m_pTopRuler;}
	void                setLeftRuler(AP_LeftRuler * pRuler)
                        { m_pLeftRuler = pRuler;}
	AP_LeftRuler *       getLeftRuler(void) const
		                { return m_pLeftRuler;}


	const gchar **   getViewPersistentProps() const;
	FV_BIDI_Order	    getBidiOrder()const {return m_eBidiOrder;}
	void                setBidiOrder(FV_BIDI_Order o) {m_eBidiOrder = o;}

	bool                isMathSelected(UT_sint32 x, UT_sint32 y, PT_DocPosition & pos) const;
	// -- plugins
        bool                isMathLoaded(void) const;
	bool                isGrammarLoaded(void) const;
	// --

	UT_uint32			getNumHorizPages(void) const; //////////////////////////////////
	void				calculateNumHorizPages(void);
	UT_uint32			getMaxHeight(UT_uint32 iRow) const;
	UT_uint32			getWidthPrevPagesInRow(UT_uint32 iPageNumber) const;
	UT_uint32			getWidthPagesInRow(fp_Page *page) const;
	UT_uint32			getHorizPageSpacing(void) const;
	bool				rtlPages(void) const;

protected:
	void				_updateDatesBeforeSave(bool bOverwriteCreated);
	void				_saveAndNotifyPieceTableChange(void);
	void				_restorePieceTableState(void);

	void				_draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32, bool bDirtyRunsOnly, bool bClip=false);

	void				_drawBetweenPositions(PT_DocPosition left, PT_DocPosition right);
	bool				_clearBetweenPositions(PT_DocPosition left, PT_DocPosition right, bool bFullLineHeightRect);
	bool                		_drawOrClearBetweenPositions(PT_DocPosition iPos1, PT_DocPosition iPos2, bool bClear, bool bFullLineHeight);
	bool				_ensureInsertionPointOnScreen();
	void				_moveInsPtNextPrevPage(bool bNext);
	void				_moveInsPtNextPrevScreen(bool bNext, bool bClearSelection);
	void				_moveInsPtNextPrevLine(bool bNext);
	fp_Line *           _getNextLineInDoc(fp_Container * pCon) const;
	fp_Page *			_getCurrentPage(void) const;
	void				_moveInsPtNthPage(UT_sint32 n);
	void				_moveInsPtToPage(fp_Page *page);
	void				_insertSectionBreak(void);
	void				_getPageXandYOffset(const fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff, bool bYOnly) const;

	PT_DocPosition		_getDocPosFromPoint(PT_DocPosition iPoint, FV_DocPos dp, bool bKeepLooking=true) const;
	PT_DocPosition		_getDocPos(FV_DocPos dp, bool bKeepLooking=true) const;
	void				_findPositionCoords(PT_DocPosition pos,
											bool b,
											UT_sint32& x,
											UT_sint32& y,
											UT_sint32& x2, //these are needed for BiDi split carret
											UT_sint32& y2,

											UT_uint32& height,
											bool& bDirection,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun)const;

	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos) const;

	fp_Page*			_getPageForXY(UT_sint32 xPos,
									  UT_sint32 yPos,
									  UT_sint32& xClick,
									  UT_sint32& yClick) const;
	bool                _insertField(const char* szName,
									 const gchar ** extra_attrs = NULL,
									 const gchar ** extra_props = NULL);
	void				_moveToSelectionEnd(bool bForward);
	void				_eraseSelection(void);
	void				_clearSelection(bool bRedraw = true);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(PP_AttrProp *p_AttrProp_Before = NULL,
							 bool bNoUpdate = false,
							 bool bCaretLeft = false);
	bool				_insertFormatPair(const gchar * szName, const gchar * properties[]);
	void				_updateInsertionPoint();
	void				_fixInsertionPointCoords(bool bIgnoreAll = false);
	void				_fixInsertionPointCoords(fv_CaretProps * pCP) const;
	void				_fixAllInsertionPointCoords(void) const;

	void				_drawSelection();
	void				_extSel(UT_uint32 iOldPoint);
	void				_extSelToPos(PT_DocPosition pos);
	UT_Error			_insertGraphic(FG_Graphic*, const char*);
	UT_Error			_insertGraphic(FG_Graphic*, const char*,PT_DocPosition pos);

	UT_UCSChar *		_lookupSuggestion(fl_BlockLayout* pBL, fl_PartOfBlock* pPOB, UT_sint32 ndx);

	static void 		_autoScroll(UT_Worker * pTimer);
	static void 		_actuallyScroll(UT_Worker * pTimer);

	// localize handling of insertion point logic
	void				_setPoint(PT_DocPosition pt, bool bEOL = false);
	void				_setPoint(fv_CaretProps * pCP, PT_DocPosition pt, UT_sint32 iLen) const;
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2) const;
	bool				_charMotion(bool bForward,UT_uint32 countChars, bool bSkipCannotContainPoint = true);
	void				_doPaste(bool bUseClipboard, bool bHonorFormatting = true);
	void				_clearIfAtFmtMark(PT_DocPosition dpos);

#ifdef ENABLE_SPELL
	void				_checkPendingWordForSpell(void);
#endif

	bool				_isSpaceBefore(PT_DocPosition pos) const;
	void				_removeThisHdrFtr(fl_HdrFtrSectionLayout * pHdrFtr);
	void 				_cmdEditHdrFtr(HdrFtrType hfType);

	UT_Error			_deleteBookmark(const char* szName, bool bSignal, PT_DocPosition * pos1 = NULL, PT_DocPosition * pos2 = NULL);
	UT_Error			_deleteHyperlink(PT_DocPosition &i, bool bSignal);

	UT_Error			_deleteXMLID( const std::string& xmlid, bool bSignal, PT_DocPosition& posStart, PT_DocPosition& posEnd );
	UT_Error			_deleteXMLID( const std::string& xmlid, bool bSignal );
	fp_HyperlinkRun *   _getHyperlinkInRange(PT_DocPosition &posStart,
											 PT_DocPosition &posEnd);
	bool			    _charInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce = false);

	void                _adjustDeletePosition(UT_uint32 &iDocPos, UT_uint32 &iCount);


    void                incremenetBubbleBlockerCount();
    void                decremenetBubbleBlockerCount();
	bool                _changeCellTo(PT_DocPosition posTable,UT_sint32 rowOld,
									  UT_sint32 colOld, UT_sint32 left, UT_sint32 right,
									  UT_sint32 top, UT_sint32 bot);
	bool                _insertCellAt(PT_DocPosition posCell, UT_sint32 left, UT_sint32 right, UT_sint32 top, 
									  UT_sint32 bot, const gchar ** attrsBlock, const gchar ** propsBlock);
	bool                _changeCellAttach(PT_DocPosition posCell, UT_sint32 left, UT_sint32 right,
									  UT_sint32 top, UT_sint32 bot);


private:

	UT_uint32			m_iNumHorizPages; /////////////////////////////////////////////////
	UT_uint32			m_getNumHorizPagesCachedWindowWidth;
	bool				m_autoNumHorizPages;
	UT_uint32			m_horizPageSpacing;

	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	//the followingare BiDi specific, but need to be in place because of the
	//change to the signature of findPointCoords
	UT_sint32			m_xPoint2;
	UT_sint32			m_yPoint2;
	bool			    m_bPointDirection;
	bool				m_bDefaultDirectionRtl;
	bool				m_bUseHebrewContextGlyphs;
	UT_uint32			m_iPointHeight;
	UT_sint32			m_xPointSticky; 	// used only for _moveInsPtNextPrevLine()

	bool				m_bPointVisible;
	bool				m_bPointEOL;
	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	GR_Graphics*		m_pG;
	void*				m_pParentData;

	// autoscroll stuff
	UT_Timer *			m_pAutoScrollTimer;
	UT_sint32			m_xLastMouse;
	UT_sint32			m_yLastMouse;

	bool				m_bCursorIsOn;
	bool				m_bEraseSaysStopBlinking;
	bool				m_bCursorBlink;

	bool				m_bdontSpellCheckRightNow;
	fv_ChangeState		m_chg;

	// find and replace stuff
	bool				m_wrappedEnd;
	PT_DocPosition		m_startPosition;

	bool				m_doneFind;

	bool				m_bEditHdrFtr;
	fl_HdrFtrShadow *	m_pEditShadow;
	PT_DocPosition		m_iSavedPosition;
	bool				m_bNeedSavedPosition;
	PT_DocPosition		_BlockOffsetToPos(fl_BlockLayout * block, PT_DocPosition offset) const;

	fl_BlockLayout *	_findGetCurrentBlock(void) const;
	PT_DocPosition		_findGetCurrentOffset(void) const;
	UT_UCSChar *		_findGetNextBlockBuffer(fl_BlockLayout ** block, PT_DocPosition *offset);
	UT_UCSChar *		_findGetPrevBlockBuffer(fl_BlockLayout ** block, PT_DocPosition *offset, UT_sint32& endIndex);

	bool				m_bReverseFind;
	bool				m_bWholeWord;
	bool				m_bMatchCase;
	UT_UCSChar *		m_sFind;
	UT_UCSChar *		m_sReplace;

	UT_sint32			_findBlockSearchRegexp(const UT_UCSChar * haystack, const UT_UCSChar * needle);

	// prefs listener - to change cursor blink on/off (and possibly others)
	static void _prefsListener( XAP_Prefs *, UT_StringPtrMap *, void *);

	bool				 m_bShowPara;
	ViewMode			 m_viewMode;
	PreViewMode 		 m_previewMode;
	bool				 m_bDontUpdateScreenOnGeneralUpdate;
	//#TF had to change the whole logic of storing PT state, since
	//the earlier implementation did not work with nested calls to
	//_saveAndNotifyPieceTableChange();
	UT_uint32			m_iPieceTableState;

	UT_sint32           m_iMouseX;
	UT_sint32           m_iMouseY;

	UT_uint32           m_iViewRevision;

	bool				m_bWarnedThatRestartNeeded;

	// properties for image selection
	UT_Rect				m_selImageRect;
	GR_Graphics::Cursor	m_imageSelCursor;
	UT_sint32			m_ixResizeOrigin;
	UT_sint32			m_iyResizeOrigin;
	bool				m_bIsResizingImage;
	UT_Rect				m_curImageSel;
#if XAP_DONTUSE_XOR
	GR_Image*			m_curImageSelCache;
#endif
	// properties for image dragging
	bool				m_bIsDraggingImage;
	fp_Run *			m_pDraggedImageRun;
	UT_Rect				m_dragImageRect;
	UT_sint32			m_ixDragOrigin;
	UT_sint32			m_iyDragOrigin;

	// default color values
	UT_RGBColor			m_colorShowPara;
	UT_RGBColor			m_colorSpellSquiggle;
	UT_RGBColor			m_colorGrammarSquiggle;
	UT_RGBColor			m_colorMargin;
	UT_RGBColor			m_colorFieldOffset;
	UT_RGBColor			m_colorImage;
	UT_RGBColor			m_colorImageResize;
	UT_RGBColor			m_colorHyperLink;
	UT_RGBColor         m_colorRevisions[10];
	UT_RGBColor			m_colorHdrFtr;
	UT_RGBColor			m_colorColumnLine;
	UT_RGBColor         m_colorAnnotations[10];
	UT_RGBColor         m_colorRDFAnchors[10];

	UT_uint32 m_countDisable; // cursor disable count
	bool                m_bDragTableLine;
	EV_EditMouseContext m_prevMouseContext;
	AP_TopRuler *       m_pTopRuler;
	AP_LeftRuler *      m_pLeftRuler;
	bool                m_bInFootnote;

	FV_Caret_Listener * m_caretListener;
	bool m_bgColorInitted;
	PT_DocPosition      m_iLowDrawPoint;
	PT_DocPosition      m_iHighDrawPoint;
	mutable fv_PropCache        m_CharProps;
	mutable fv_PropCache        m_BlockProps;
	mutable fv_PropCache        m_SecProps;
	AV_ListenerId       m_CaretListID;
#ifdef TOOLKIT_GTK_ALL
	FV_UnixFrameEdit    m_FrameEdit;
#else
	FV_FrameEdit        m_FrameEdit;
#endif
#ifdef TOOLKIT_GTK_ALL
	FV_UnixVisualDrag   m_VisualDragText;
#else
	FV_VisualDragText   m_VisualDragText;
#endif
	FV_Selection        m_Selection;
	bool                m_bShowRevisions;

	FV_BIDI_Order       m_eBidiOrder;
	UT_uint32           m_iFreePass;
	bool                m_bDontNotifyListeners;
	UT_ByteBuf *        m_pLocalBuf;
	UT_sint32           m_iGrabCell;
#ifdef TOOLKIT_GTK_ALL
	FV_UnixVisualInlineImage  m_InlineImage;
#else
	FV_VisualInlineImage  m_InlineImage;
#endif
	bool                m_bInsertAtTablePending;
	PT_DocPosition      m_iPosAtTable;
	UT_GenericVector<fv_CaretProps *> m_vecCarets;
	UT_UTF8String       m_sDocUUID;
	bool				m_bAnnotationPreviewActive;
	UT_uint32			m_iAnnPviewID;
	bool                m_bAllowSmartQuoteReplacement;  // Enable/disable replacing of quote with smart quote
														// This allows temporarily disabling smart quotes to allow inserting ANSI quote.
    int                 m_bubbleBlockerCount;
	UT_sint32           m_iOldPageCount;

#if defined(TOOLKIT_GTK_ALL) && !defined(TOOLKIT_GTK2)
	FV_UnixSelectionHandles m_SelectionHandles;
#else
	FV_SelectionHandles m_SelectionHandles;
#endif

public:
	bool registerDoubleBufferingObject(FV_ViewDoubleBuffering *obj);
	bool unregisterDoubleBufferingObject(FV_ViewDoubleBuffering *obj);
private:
	FV_ViewDoubleBuffering *m_pViewDoubleBufferingObject;
};

#endif /* FV_VIEW_H */
