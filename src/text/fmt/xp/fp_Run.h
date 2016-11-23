/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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



#ifndef FP_RUN_H
#define FP_RUN_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"
#include "ut_assert.h"
#include "xap_Strings.h"
#include "fl_BlockLayout.h"
#include "pp_Revision.h"
#include "ut_string_class.h"
#include "fp_ContainerObject.h"

class UT_GrowBuf;
class fp_Line;
class GR_Graphics;
class GR_Font;
class GR_Image;
class PD_Document;
class PP_AttrProp;
struct dg_DrawArgs;
class fl_CharWidths;
class fd_Field;
class fp_HyperlinkRun;
class fp_AnnotationRun;
class fp_RDFAnchorRun;

struct fp_RunSplitInfo
{
	UT_sint32 iLeftWidth;
	UT_sint32 iRightWidth;
	UT_sint32 iOffset;
};

// TODO The break type is not used. Is it put here looking forward,
// TODO or is it left from some earlier experiments?
enum FP_RUN_BREAK_TYPE
{
	BREAK_AUTO			= 0,
	BREAK_AVOID			= 1,
	BREAK_ALWAYS		= 2
};

enum FP_RUN_TYPE
{
	FPRUN__FIRST__					= 1,
	FPRUN_TEXT						= 1,
	FPRUN_IMAGE						= 2,
	FPRUN_TAB						= 3,
	FPRUN_FORCEDLINEBREAK			= 4,
	FPRUN_FORCEDCOLUMNBREAK			= 5,
	FPRUN_FORCEDPAGEBREAK			= 6,
	FPRUN_FIELD						= 7,
	FPRUN_FMTMARK					= 8,
	FPRUN_FIELDSTARTRUN				= 9,
	FPRUN_FIELDENDRUN				= 10,
	FPRUN_ENDOFPARAGRAPH            = 11,
	FPRUN_BOOKMARK					= 12,
	FPRUN_HYPERLINK					= 13,
	FPRUN_DIRECTIONMARKER           = 14,
	FPRUN_DUMMY                     = 15,
	FPRUN_MATH                      = 16,
	FPRUN_EMBED                      = 17,
	FPRUN__LAST__					= 18
};

enum FP_HYPERLINK_TYPE
{
    HYPERLINK_NORMAL =1,
    HYPERLINK_ANNOTATION = 2,
    HYPERLINK_RDFANCHOR = 3
};

// specifies how setX should handle screen clearing
enum FPRUN_CLEAR_SCREEN
{
	FP_CLEARSCREEN_AUTO,
	FP_CLEARSCREEN_FORCE,
	FP_CLEARSCREEN_NEVER
};


/*
	fp_Run represents a contiguous homogenous chunk on a single line.
	This file also defines the following subclasses:

		fp_TabRun
		fp_ForcedLineBreakRun
		fp_ForcedColumnBreakRun
		fp_ForcedPageBreakRun
		fp_ImageRun
		fp_FieldRun
		fp_FmtMarkRun
		fp_FieldStartRun
		fp_FieldEndRun
		fp_BookmarkRun
		fp_HyperlinkRun
		fp_AnnotationRun
		fp_RDFAnchorRun
		fp_DummyRun

	As far as the formatter's concerned, each subclass behaves somewhat
	differently, but they can all be treated like rectangular blocks to
	be arranged.

	Convention: _setFoo(bar) is just this.foo = bar;
                 setFoo(bar) sets this.foo to bar,
                               but may also do other processing to maintain
							   internal state.
*/
class ABI_EXPORT fp_Run : fp_ContainerObject
{
	friend class fg_FillType;
public:
	fp_Run(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst,
		   UT_uint32 iLen, FP_RUN_TYPE iType);
	virtual ~fp_Run();

	// inline getter member functions

	FP_RUN_TYPE		        getType() const 				{ return m_iType; }
	fp_Line*		        getLine() const 				{ return m_pLine; }
	fl_BlockLayout*	        getBlock() const 				{ return m_pBL; }
	UT_sint32		        getX() const 					{ return m_iX; }
	UT_sint32		        getY() const 					{ return m_iY; }

	UT_sint32		        getHeight() const;
	UT_sint32		        getWidth() const;
	UT_uint32		        getAscent() const;
	UT_uint32		        getDescent() const;
	virtual UT_sint32       getDrawingWidth() const;

	fp_Run* 		        getNextRun() const					{ return m_pNext; }
	fp_Run*			        getPrevRun() const					{ return m_pPrev; }
	bool                    isInSelectedTOC(void);
	virtual fp_ContainerObject * getNext(void) const { return NULL;}
	virtual fp_ContainerObject * getPrev(void) const { return NULL;}
	virtual fp_Container *       getNextContainerInSection(void) const { return NULL;}
	virtual fp_Container *       getPrevContainerInSection(void) const { return NULL;}
	virtual void                 setNext(fp_ContainerObject * /*pNull*/) {}
	virtual void                 setPrev(fp_ContainerObject * /*pNull*/) {}
	virtual void                 draw(GR_Graphics * /*pG*/) {}

	UT_uint32		    getBlockOffset() const			{ return m_iOffsetFirst; }
	UT_uint32		    getLength() const				{ return m_iLen; }
	GR_Graphics*	    getGraphics() const;
	fp_HyperlinkRun *   getHyperlink() const 			{ return m_pHyperlink;}
#if DEBUG
	virtual void        printText(void) {};
#endif

	void                getSpanAP(const PP_AttrProp * &pSpanAP);
	const PP_AttrProp * getSpanAP(void);


	inline void         getBlockAP(const PP_AttrProp * &pBlockAP)
	                                     {getBlock()->getAP(pBlockAP);}


	void				insertIntoRunListBeforeThis(fp_Run& newRun);
	void				insertIntoRunListAfterThis(fp_Run& newRun);
	fd_Field*			getField(void) const { return m_pField; }
	bool				isField(void) const { return (bool) (m_pField != NULL); }
	void				unlinkFromRunList();

	const UT_RGBColor 	getFGColor(void) const;

	virtual bool		hasLayoutProperties(void) const;

	void				setLine(fp_Line*);
	void				setBlock(fl_BlockLayout * pBL) { _setBlock(pBL); }
	virtual void        setX(UT_sint32 x, bool bDontClearIfNeeded = false);
	void			    Run_setX(UT_sint32, FPRUN_CLEAR_SCREEN eClearScreen = FP_CLEARSCREEN_AUTO);
	virtual void		setY(UT_sint32);
	void				setBlockOffset(UT_uint32);
	void				setLength(UT_uint32 iLen, bool bRefresh = true);
	void				setNextRun(fp_Run*, bool bRefresh = true);
	void				setPrevRun(fp_Run*, bool bRefresh = true);
	void				setHyperlink(fp_HyperlinkRun * pH);
	void				markWidthDirty() {m_bRecalcWidth = true;}
	bool				isFirstRunOnLine(void) const;
	bool				isLastRunOnLine(void) const;
	bool				isOnlyRunOnLine(void) const;
	bool				isFirstVisRunOnLine(void) const;
	bool				isLastVisRunOnLine(void) const;
	void				markDrawBufferDirty()
	                        {m_eRefreshDrawBuffer = GRSR_Unknown;}
	void				orDrawBufferDirty(GRShapingResult eR)
                        {
							m_eRefreshDrawBuffer = (GRShapingResult)((UT_uint32)m_eRefreshDrawBuffer
																	 |(UT_uint32)eR);
}
	bool                isPrinting(void) const
	{ return m_bPrinting;}
	virtual void		draw(dg_DrawArgs*);
	virtual void        clearScreen(void);
	void                Run_ClearScreen(bool bFullLineHeightRect = false);
	virtual void        setWidth(UT_sint32 /*iW*/) {}
	virtual void        setHeight(UT_sint32 /*iH*/) {}
	virtual bool        isVBreakable(void) {return false;}
	virtual bool        isHBreakable(void) {return false;}
	virtual UT_sint32   wantVBreakAt(UT_sint32 i) {return i;}
	virtual UT_sint32   wantHBreakAt(UT_sint32 i) {return i;}
	virtual fp_ContainerObject * VBreakAt(UT_sint32) { return NULL;}
	virtual fp_ContainerObject * HBreakAt(UT_sint32) { return NULL;}

	void				markAsDirty(void);
	void                setCleared(void);
	bool				isDirty(void) const { return m_bDirty; }
	bool			    canContainPoint(void) const;
	bool		        recalcWidth(void);
	virtual void        updateOnDelete(UT_uint32 offset, UT_uint32 iLen);
	virtual void        updateVerticalMetric();

    virtual UT_Rect *   getScreenRect();
    virtual void        markDirtyOverlappingRuns(UT_Rect & recScreen);

	virtual void		_draw(dg_DrawArgs*) = 0;
    void                _drawTextLine(UT_sint32, UT_sint32, UT_uint32, UT_uint32, UT_UCSChar *);
	virtual void       	_clearScreen(bool bFullLineHeightRect) = 0;
	virtual bool		canBreakAfter(void) const = 0;
	virtual bool		canBreakBefore(void) const = 0;
	bool		        letPointPass(void) const;
	virtual bool		isForcedBreak(void) const { return false; }
	virtual bool		alwaysFits(void) const { return false; }
	virtual bool		findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si,
												 bool bForce=false);
	void                clearPrint(void);
	virtual UT_sint32	findTrailingSpaceDistance(void) const { return 0; }
	virtual bool		findFirstNonBlankSplitPoint(fp_RunSplitInfo& /*si*/) { return false; }
	virtual void		mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos,
										PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC) = 0;

	virtual void 		findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y,
										UT_sint32& x2, UT_sint32& y2, UT_sint32& height,
										bool& bDirection) = 0;

	void			    lookupProperties(GR_Graphics * pG=NULL);
	virtual bool		doesContainNonBlankData(void) const { return true; }	// Things like text whould return false if it is all spaces.
	void                drawDecors(UT_sint32 xoff, UT_sint32 yoff, GR_Graphics * pG);
	virtual bool		isSuperscript(void) const { return false; }
	virtual bool		isSubscript(void) const { return false; }
    bool			    isUnderline(void) const ;
	bool			    isOverline(void) const ;
	bool			    isStrikethrough(void) const ;
	bool			    isTopline(void) const ;
	bool			    isBottomline(void) const ;
	void			    setLinethickness(UT_sint32 max_linethickness);
    UT_sint32		    getLinethickness(void) const;
	void			    setUnderlineXoff(UT_sint32 xoff);
	UT_sint32		    getUnderlineXoff(void) const;
	void			    setOverlineXoff(UT_sint32 xoff) ;
	UT_sint32		    getOverlineXoff(void) const;
	void			    setMaxUnderline(UT_sint32 xoff) ;
	UT_sint32		    getMaxUnderline(void) const;
	void			    setMinOverline(UT_sint32 xoff) ;
	UT_sint32		    getMinOverline(void) const;
	UT_sint32           getToplineThickness(void) const;

	virtual UT_BidiCharType	getDirection() const { return m_iDirection; };
	UT_BidiCharType		getVisDirection() const;
	virtual void        setDirection(UT_BidiCharType iDirection = UT_BIDI_WS);
	void				setVisDirection(UT_BidiCharType iDir);
	UT_uint32           getVisPosition(UT_uint32 ilogPos) const;
	UT_uint32           getVisPosition(UT_uint32 iLogPos, UT_uint32 iLen) const;
	UT_uint32           getOffsetFirstVis() const;
	UT_uint32           getOffsetLog(UT_uint32 iVisOff) const;
	fp_Run *			getNextVisual(); // FIXME make const
	fp_Run *			getPrevVisual(); // FIXME make const
	UT_sint32           getAuthorNum(void) const
	{ return m_iAuthorColor;};
	void                setAuthorNum(UT_sint32 i)
	{ m_iAuthorColor=i;};

	virtual UT_uint32   adjustCaretPosition(UT_uint32 iDocumentPosition, bool /*bForward*/)
	                           { return iDocumentPosition;}

	virtual void        adjustDeletePosition(UT_uint32 & /*pos1*/,
											 UT_uint32 & /*count*/) {}

	bool                containsRevisions() const {return (m_pRevisions != NULL);}
	// would prefer to make the return value const, but the
	// getLastRevision() and related functions use internal cache so
	// they could not be called
	PP_RevisionAttr *   getRevisions() const {return m_pRevisions;}
	FPVisibility        getVisibility() const {return m_eVisibility;}
	bool         isHidden() const {return _wouldBeHidden(m_eVisibility);}
	void                setVisibility(FPVisibility eVis);
	void                Fill(GR_Graphics * pG, UT_sint32 x, UT_sint32 y,
							 UT_sint32 width, UT_sint32 height);

	fg_FillType &       getFillType(void);
	const fg_FillType & getFillType(void) const;
	fp_Line *           getTmpLine(void) const
	{ return m_pTmpLine;}
	void                setTmpLine(fp_Line * pLine)
	{ m_pTmpLine = pLine;}
	UT_sint32           getTmpX(void) const
	{ return m_iTmpX;}
	void                setTmpX(UT_sint32 iX)
	{ m_iTmpX = iX;}
	UT_sint32           getTmpY(void) const
	{ return m_iTmpY;}
	void                setTmpY(UT_sint32 iY)
	{ m_iTmpY = iY;}
	UT_sint32           getTmpWidth(void) const
	{ return m_iTmpWidth;}
	void                setTmpWidth(UT_sint32 iWidth)
	{ m_iTmpWidth = iWidth;}
	bool                clearIfNeeded(void);

	// Indicates that if insertion point is placed at a position belonging to the run
	// and delete command is issued, it should apply to the following run (or previous
	// in case of backspace). This is used with invisible runs, such as hyperlinks,
	// bookmarks, and hidden hidden text, to ensure that these are not deleted behind
	// the users backs.
	bool        deleteFollowingIfAtInsPoint() const;

	bool        displayAnnotations(void) const;
    bool        displayRDFAnchors(void) const;
	// Methods for selection drawing
	void                 setSelectionMode(PT_DocPosition posLow, PT_DocPosition posHigh);
    void                 clearSelectionMode(void);
	bool                 isSelectionDraw(void) const;
	PT_DocPosition       posSelLow(void) const;
    PT_DocPosition       posSelHigh(void) const;
	UT_RGBColor			_getColorFG(void) const { return m_pColorFG; }

#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif
	void               setMustClearScreen(void)
	{ m_bMustClearScreen = true;}
	bool               getMustClearScreen(void)
	{return m_bMustClearScreen;}

protected:
	virtual bool        _deleteFollowingIfAtInsPoint() const;
	void				_inheritProperties(void);
	fp_Run*				_findPrevPropertyRun(void) const;

	FV_View*			_getView(void) const { return getBlock()->getView(); }
	// By convention, _getFoo and _setFoo have no side effects.
	// They can easily be inlined by a smart compiler.
	UT_RGBColor			_getColorPG(void) const { return m_pColorPG; }
	UT_RGBColor			_getColorHL(void) const { return m_pColorHL; }
	void				_setColorFG(UT_RGBColor c) { m_pColorFG = c; }
	void				_setColorHL(UT_RGBColor c) { m_pColorHL = c; }
	void                _setColorHL(const char *pszColor)
		{ m_pColorHL.setColor(pszColor); }

	void				_setLine(fp_Line* pLine) { m_pLine = pLine; }
	void				_setHeight(UT_sint32 iHeight)
							{ m_iHeight = iHeight;}
	virtual void		_setWidth(UT_sint32 iWidth)
                        	{ m_iWidth = iWidth; }

	// use these with great care -- most of the time we need to use
	// getWidth() and getHeight() which deal with
	// visibility/hiddenness issues
	UT_sint32           _getWidth() {return m_iWidth;}
	UT_sint32           _getHeight(){return m_iHeight;}

	void				_setBlock(fl_BlockLayout * pBL) { m_pBL = pBL; }
	void				_setAscent(int iAscent) { m_iAscent = iAscent; }
	void				_setDescent(int iDescent) {m_iDescent = iDescent;}
	void				_setX(int iX) { m_iX = iX; }
	void				_setY(int iY) { m_iY = iY; }
	void				_setDirection(UT_BidiCharType c) { m_iDirection = c; }
	UT_BidiCharType		_getDirection(void) const { return m_iDirection; }
	UT_BidiCharType		_getVisDirection(void) const { return m_iVisDirection; }
	const GR_Font *			_getFont(void) const;
	void  				_setFont(const GR_Font * f);
	unsigned char		_getDecorations(void) const { return m_fDecorations; }
	void				_setDecorations(unsigned char d) {m_fDecorations = d;}

	void				_orDecorations(unsigned char d) { m_fDecorations |= d; }
	UT_sint32			_getLineWidth(void) { return m_iLineWidth; }
	bool				_setLineWidth(UT_sint32 w)
	                         {
								 UT_sint32 o = m_iLineWidth;
								 m_iLineWidth = w;
								 return o != w;
							 }
	void				_setLength(UT_uint32 l) { m_iLen = l; }
	void				_setRevisions(PP_RevisionAttr * p) { m_pRevisions = p; }
	void				_setDirty(bool b);
	void				_setField(fd_Field * fd) { m_pField = fd; }
	void                _setHyperlink(fp_HyperlinkRun * pH) { m_pHyperlink = pH; }
	bool				_getRecalcWidth(void) const { return m_bRecalcWidth; }
	void				_setRecalcWidth(bool b) { m_bRecalcWidth = b; }

	GRShapingResult		_getRefreshDrawBuffer(void) const { return m_eRefreshDrawBuffer; }
	void				_setRefreshDrawBuffer(GRShapingResult eR)
	                         { m_eRefreshDrawBuffer = eR; }
	virtual void	    _lookupProperties(const PP_AttrProp * pSpanAP,
										  const PP_AttrProp * pBlockAP,
										  const PP_AttrProp * pSectionAP,
										  GR_Graphics * pG = NULL) = 0;

	virtual bool        _canContainPoint(void) const;
	virtual bool        _letPointPass(void) const;
	virtual	bool		_recalcWidth(void);
	bool         _wouldBeHidden(FPVisibility eVis) const;
//
// Variables to draw underlines for all runs
//
	enum
	{
		TEXT_DECOR_UNDERLINE = 		0x01,
		TEXT_DECOR_OVERLINE = 		0x02,
		TEXT_DECOR_LINETHROUGH = 	0x04,
		TEXT_DECOR_TOPLINE = 	    0x08,
		TEXT_DECOR_BOTTOMLINE = 	0x10
	};

private:
	fp_Run(const fp_Run&);			// no impl.
	void operator=(const fp_Run&);	// no impl.

	FP_RUN_TYPE				m_iType;
	fp_Line*				m_pLine;
	fl_BlockLayout*			m_pBL;
	fp_Run*					m_pNext;
	fp_Run*					m_pPrev;
	UT_sint32				m_iX;
	UT_sint32				m_iOldX;
	UT_sint32				m_iY;
	UT_sint32				m_iWidth;
	UT_sint32				m_iHeight;
	UT_uint32				m_iAscent;
	UT_uint32				m_iDescent;

	UT_uint32				m_iOffsetFirst;
	UT_uint32				m_iLen;
	bool					m_bDirty;		// run erased @ old coords, needs to be redrawn
	fd_Field*				m_pField;
	UT_BidiCharType			m_iDirection;   //#TF direction of the run 0 for left-to-right, 1 for right-to-left
	UT_BidiCharType			m_iVisDirection;
	GRShapingResult			m_eRefreshDrawBuffer;

	// the run highlight color. If the property is transparent use the page color
	UT_RGBColor             m_pColorHL;

	const GR_Font * 		m_pFont;

	bool					m_bRecalcWidth;

	unsigned char			m_fDecorations;
	UT_sint32				m_iLineWidth;
	UT_sint32               m_iLinethickness;
	UT_sint32               m_iUnderlineXoff;
	UT_sint32               m_imaxUnderline;
	UT_sint32               m_iminOverline;
	UT_sint32               m_iOverlineXoff;
	fp_HyperlinkRun *		m_pHyperlink;
	PP_RevisionAttr *       m_pRevisions;

	// A local cache of the page color. This makes clearscreen() a bit faster
	UT_RGBColor       		m_pColorPG;
	UT_RGBColor 			m_pColorFG;
	FPVisibility            m_eVisibility;
	bool                    m_bIsCleared;
	fg_FillType             m_FillType;
	bool                    m_bPrinting;
	UT_sint32               m_iTmpX;
	UT_sint32               m_iTmpY;
	UT_sint32               m_iTmpWidth;
	fp_Line *               m_pTmpLine;

	// Variables for selection drawing
    bool                    m_bDrawSelection;
    PT_DocPosition          m_iSelLow;
    PT_DocPosition          m_iSelHigh;
	bool                    m_bMustClearScreen;
	UT_sint32               m_iAuthorColor;
#ifdef DEBUG
	UT_uint32               m_iFontAllocNo;
#endif
};

class ABI_EXPORT fp_TabRun : public fp_Run
{
public:
	fp_TabRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool 			hasLayoutProperties(void) const;
	void			       	setTabWidth(UT_sint32);
	void			       	setLeader(eTabLeader iTabType);
	eTabLeader			    getLeader(void);
	void                    setTabType(eTabType iTabType);
	eTabType                getTabType(void) const;
	bool                    isTOCTab(void);
	void                    setTOCTab(void)
	{ m_bIsTOC = true;}
	void                    setTOCTabListLabel(void);
	bool                    isTOCTabListLabel(void) const
	{ return m_bIsTOCListLabel;}

protected:
	virtual void			_drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 iHeight);
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual bool			_letPointPass(void) const;

private:
	//UT_RGBColor			    m_colorFG;
private:
	eTabLeader			    m_leader;
    eTabType                m_TabType;
	bool                    m_bIsTOC;
	bool                    m_bIsTOCListLabel;
};

class ABI_EXPORT fp_ForcedLineBreakRun : public fp_Run
{
public:
	fp_ForcedLineBreakRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
};

class ABI_EXPORT fp_FieldStartRun : public fp_Run
{
public:
	fp_FieldStartRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
};

class ABI_EXPORT fp_FieldEndRun : public fp_Run
{
public:
	fp_FieldEndRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
};

class ABI_EXPORT fp_ForcedColumnBreakRun : public fp_Run
{
public:
	fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
};

class ABI_EXPORT fp_ForcedPageBreakRun : public fp_Run
{
public:
	fp_ForcedPageBreakRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isForcedBreak(void) const { return true; }

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
};

class ABI_EXPORT fp_EndOfParagraphRun : public fp_Run
{
public:
	fp_EndOfParagraphRun(fl_BlockLayout* pBL,  UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isToc);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual UT_sint32       getDrawingWidth() const { return static_cast<UT_sint32>(m_iDrawWidth);}

//
// Tomas this breaks line breaking....
//	virtual bool			doesContainNonBlankData(void) const { return false; }	// Things like text whould return false if it is all spaces.

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
	virtual bool			_recalcWidth(void);

private:
	UT_uint32				m_iXoffText;
	UT_uint32				m_iYoffText;
	UT_uint32				m_iDrawWidth;

};

class ABI_EXPORT fp_BookmarkRun : public fp_Run
{
public:
	fp_BookmarkRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	bool 				isStartOfBookmark() const {return m_bIsStart;};
	const gchar * 	getName() const {return m_pName;};
	bool 				isComrade(fp_BookmarkRun *pBR) const;

	virtual bool canBreakAfter(void) const;
	virtual bool canBreakBefore(void) const;

	virtual void mapXYToPosition(UT_sint32 x,
								 UT_sint32 y,
								 PT_DocPosition& pos,
								 bool& bBOL,
								 bool& bEOL,
								 bool & isTOC);

	virtual void findPointCoords(UT_uint32 iOffset,
								 UT_sint32& x,
								 UT_sint32& y,
								 UT_sint32& x2,
								 UT_sint32& y2,
								 UT_sint32& height,
								 bool& bDirection);
	virtual bool hasLayoutProperties(void) const
	{ return false; }

	// for the purposes of linebreaking, just whitespace
	virtual bool doesContainNonBlankData(void) const { return false; }

	UT_uint32 getBookmarkedDocPosition(bool bAfter) const;


private:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void _clearScreen(bool /* bFullLineHeightRect */);
	virtual void _draw(dg_DrawArgs* /*pDA */);
	virtual bool _letPointPass(void) const;
	virtual bool _canContainPoint(void) const;
	virtual bool _deleteFollowingIfAtInsPoint() const;

	bool m_bIsStart;
	#define BOOKMARK_NAME_SIZE 30
	gchar	  	m_pName[BOOKMARK_NAME_SIZE + 1];
	po_Bookmark		* m_pBookmark;
};

class ABI_EXPORT fp_HyperlinkRun : public fp_Run
{
public:
	fp_HyperlinkRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual ~fp_HyperlinkRun();
	virtual FP_HYPERLINK_TYPE getHyperlinkType(void) const { return HYPERLINK_NORMAL;}
	bool 				isStartOfHyperlink() const {return m_bIsStart;};
	const gchar * 	getTarget() const {return static_cast<const gchar *>(m_pTarget);};
	const gchar * 	getTitle() const {return static_cast<const gchar *>(m_pTitle);};
	
	virtual bool canBreakAfter(void) const;
	virtual bool canBreakBefore(void) const;

	virtual void mapXYToPosition(UT_sint32 x,
								 UT_sint32 y,
								 PT_DocPosition& pos,
								 bool& bBOL,
								 bool& bEOL,
								 bool &isTOC);

	virtual void findPointCoords(UT_uint32 iOffset,
								 UT_sint32& x,
								 UT_sint32& y,
								 UT_sint32& x2,
								 UT_sint32& y2,
								 UT_sint32& height,
								 bool& bDirection);

	virtual bool hasLayoutProperties(void) const
	{ return false; }

	// for the purposes of linebreaking, just whitespace
	virtual bool doesContainNonBlankData(void) const { return false; }

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void _clearScreen(bool /* bFullLineHeightRect */);
	virtual void _draw(dg_DrawArgs* /*pDA */);
	virtual bool _letPointPass(void) const;
	virtual bool _canContainPoint(void) const;
	virtual bool _deleteFollowingIfAtInsPoint() const;
	void _setTarget( const gchar * pTarget );
	void _setTitle( const gchar * pTitle );
	void _setTargetFromAPAttribute( const gchar* pAttrName );
	void _setTitleFromAPAttribute( const gchar* pAttrName );
	bool m_bIsStart;
	gchar *	  	m_pTarget;
	gchar *		m_pTitle;
};



class ABI_EXPORT fp_AnnotationRun : public fp_HyperlinkRun
{
public:
	fp_AnnotationRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual ~fp_AnnotationRun();
	virtual FP_HYPERLINK_TYPE getHyperlinkType(void) const { return HYPERLINK_ANNOTATION; }
	UT_uint32 getPID(void) { return m_iPID;}
	const char * getValue(void);
    void         recalcValue(void);
	virtual bool canBreakAfter(void) const;
	virtual bool canBreakBefore(void) const;
	UT_sint32    getRealWidth(void) const {return m_iRealWidth;}
    void         cleanDraw(dg_DrawArgs*);
	UT_sint32    calcWidth(void);

 protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_recalcWidth(void);
	bool                    _setValue(void);
	virtual void            _setWidth(UT_sint32 iWidth);
	virtual bool _letPointPass(void) const;
	virtual bool _canContainPoint(void) const;
    virtual void _lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * pBlockAP,
									const PP_AttrProp * pSectionAP,
								   GR_Graphics * pG);
 private:
	UT_uint32               m_iPID;
	UT_UTF8String           m_sValue;
	UT_sint32               m_iRealWidth;
};

class ABI_EXPORT fp_RDFAnchorRun : public fp_HyperlinkRun
{
public:
	fp_RDFAnchorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual ~fp_RDFAnchorRun();
	virtual FP_HYPERLINK_TYPE getHyperlinkType(void) const { return HYPERLINK_RDFANCHOR; }
	UT_uint32 getPID(void) { return m_iPID;}
	const char * getValue(void);
    void         recalcValue(void);
	virtual bool canBreakAfter(void) const;
	virtual bool canBreakBefore(void) const;
	UT_sint32    getRealWidth(void) const {return m_iRealWidth;}
    void         cleanDraw(dg_DrawArgs*);
	UT_sint32    calcWidth(void);

    std::string  getXMLID();

 protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_recalcWidth(void);
	bool                    _setValue(void);
	virtual void            _setWidth(UT_sint32 iWidth);
	virtual bool _letPointPass(void) const;
	virtual bool _canContainPoint(void) const;
    virtual void _lookupProperties(const PP_AttrProp * pSpanAP,
									const PP_AttrProp * pBlockAP,
									const PP_AttrProp * pSectionAP,
								   GR_Graphics * pG);
 private:
	UT_uint32               m_iPID;
	UT_UTF8String           m_sValue;
	UT_sint32               m_iRealWidth;
};

class ABI_EXPORT fp_ImageRun : public fp_Run
{
public:
	fp_ImageRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen, FG_GraphicPtr && pGraphic,  pf_Frag_Object* oh);
	virtual ~fp_ImageRun();

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	const char *            getDataId(void) const;
	virtual bool 			hasLayoutProperties(void) const;
	virtual GR_Image * 				getImage();
	void                     regenerateImage(GR_Graphics * pG);
	UT_sint32               getPointHeight(void)
	{ return m_iPointHeight;}
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;

private:
	FG_GraphicPtr           m_pFGraphic;
	GR_Image*				m_pImage;
	UT_sint32               m_iImageWidth;
	UT_sint32               m_iImageHeight;
	UT_String               m_sCachedWidthProp;
	UT_String               m_sCachedHeightProp;
	UT_sint32               m_iPointHeight;
	const PP_AttrProp *     m_pSpanAP;
	UT_uint32               m_iGraphicTick;
	bool                    m_bImageForPrinter;
	pf_Frag_Object*         m_OH;
};

#define FPFIELD_MAX_LENGTH	127

#define  _FIELD(type,desc,tag)  /*nothing*/
#define  _FIELDTYPE(type,desc)  FPFIELDTYPE_##type,

enum fp_FieldTypesEnum { FPFIELDTYPE_START,

#include "fp_Fields.h"

						 FPFIELDTYPE_END };

#undef  _FIELD
#undef  _FIELDTYPE

#define  _FIELD(type,desc,tag)  FPFIELD_##tag,
#define  _FIELDTYPE(type,desc)  /*nothing*/

enum fp_FieldsEnum { FPFIELD_start,

#include "fp_Fields.h"

					 FPFIELD_end };

#undef  _FIELD
#undef  _FIELDTYPE


struct fp_FieldTypeData
{
	fp_FieldTypesEnum	m_Type;
	const char*			m_Desc;
	XAP_String_Id		m_DescId;
};

struct fp_FieldData
{
	fp_FieldTypesEnum	m_Type;
	fp_FieldsEnum		m_Num;
	const char*			m_Desc;
	const char*			m_Tag;
	XAP_String_Id		m_DescId;
};


extern fp_FieldTypeData fp_FieldTypes[];
extern fp_FieldData fp_FieldFmts[];

// these constants define how frequently our fields get updated
// (in 1/2 seconds)
// you know, we should be able to not have to scan for endnote changes;
// they should be told to us.
#define FIELD_UPDATE_ENDNOTE       3
#define FIELD_UPDATE_TIME          1
#define FIELD_UPDATE_DATE        240
#define FIELD_UPDATE_PAGE         20
#define FIELD_UPDATE_LINE_COUNT   10
#define FIELD_UPDATE_WORD_COUNT    4
#define FIELD_UPDATE_PARA_COUNT   20
#define FIELD_UPDATE_META         10
#define FIELD_UPDATE_MAILMERGE    10

class ABI_EXPORT fp_FieldRun : public fp_Run
{
public:
	fp_FieldRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual ~fp_FieldRun();

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual fp_FieldsEnum	getFieldType(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool 			hasLayoutProperties(void) const;

	virtual bool			isSuperscript(void) const;
	virtual bool			isSubscript(void) const;

	bool					_setValue(const UT_UCSChar *p_new_value);

	virtual bool			calculateValue(void);
	virtual const UT_UCSChar *    getValue(void) const { return reinterpret_cast<const UT_UCS4Char *>(m_sFieldValue);}
	virtual UT_uint32		needsFrequentUpdates() {return 0;}

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*) {};
	virtual void			_defaultDraw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	const gchar *		_getParameter() const { return m_pParameter; }
	virtual bool			_letPointPass(void) const;
	virtual bool			_recalcWidth(void);

private:

	//UT_RGBColor				m_colorFG;
	UT_RGBColor				m_colorBG;
	UT_UCS4Char				m_sFieldValue[FPFIELD_MAX_LENGTH];
	fp_FieldsEnum			m_iFieldType;
	const gchar *		m_pParameter;
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte					m_fPosition;
};


class ABI_EXPORT fp_FieldEndnoteRefRun : public fp_FieldRun
{
public:

	fp_FieldEndnoteRefRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldEndnoteRefRun(){}

	virtual bool			calculateValue(void);
	virtual bool		    canBreakBefore(void) const;
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_ENDNOTE;};
	UT_uint32				getPID() const {return m_iPID;}

private:
	UT_uint32 m_iPID;
};


class ABI_EXPORT fp_FieldEndnoteAnchorRun : public fp_FieldRun
{
public:

	fp_FieldEndnoteAnchorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldEndnoteAnchorRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_ENDNOTE;};
	UT_uint32				getPID() const {return m_iPID;}

private:
	UT_uint32 m_iPID;
};

class ABI_EXPORT fp_FieldFootnoteRefRun : public fp_FieldRun
{
public:

	fp_FieldFootnoteRefRun(fl_BlockLayout* pBL,UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldFootnoteRefRun(){}

	virtual bool			calculateValue(void);
	virtual bool		    canBreakBefore(void) const;
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_ENDNOTE;};
	UT_uint32				getPID() const {return m_iPID;}

private:
	UT_uint32 m_iPID;
};

class ABI_EXPORT fp_FieldFootnoteAnchorRun : public fp_FieldRun
{
public:

	fp_FieldFootnoteAnchorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldFootnoteAnchorRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_ENDNOTE;};
	UT_uint32				getPID() const {return m_iPID;}

private:
	UT_uint32 m_iPID;
};

class ABI_EXPORT fp_FieldTimeRun : public fp_FieldRun
{
public:

	fp_FieldTimeRun(fl_BlockLayout* pBL,  UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldTimeRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

class ABI_EXPORT fp_FieldPageNumberRun : public fp_FieldRun
{
public:

	fp_FieldPageNumberRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldPageNumberRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PAGE;};
};

class ABI_EXPORT fp_FieldPageReferenceRun : public fp_FieldRun
{
public:

	fp_FieldPageReferenceRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldPageReferenceRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PAGE;};
};

class ABI_EXPORT fp_FieldPageCountRun : public fp_FieldRun
{
public:

	fp_FieldPageCountRun(fl_BlockLayout* pBL,  UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldPageCountRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PAGE;};
};

class ABI_EXPORT fp_FieldDateRun : public fp_FieldRun
{
public:
	fp_FieldDateRun(fl_BlockLayout* pBL,  UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldDateRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};


class ABI_EXPORT fp_FieldFileNameRun : public fp_FieldRun
{
public:
	fp_FieldFileNameRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldFileNameRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldShortFileNameRun : public fp_FieldRun
{
public:
	fp_FieldShortFileNameRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldShortFileNameRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// BEGIN DOM

// document-related information fields

// count of characters in the document
// including white spaces
class ABI_EXPORT fp_FieldCharCountRun : public fp_FieldRun
{
public:
	fp_FieldCharCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldCharCountRun() {}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// count of the non-blank characters
// in the document
class ABI_EXPORT fp_FieldNonBlankCharCountRun : public fp_FieldRun
{
public:
	fp_FieldNonBlankCharCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldNonBlankCharCountRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// count of the #lines in the document
class ABI_EXPORT fp_FieldLineCountRun : public fp_FieldRun
{
public:
	fp_FieldLineCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldLineCountRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_LINE_COUNT;};
};


// Sum the contents of the row of a table
class ABI_EXPORT fp_FieldTableSumRows : public fp_FieldRun
{
public:
	fp_FieldTableSumRows(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldTableSumRows(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_WORD_COUNT;};
};


// Sum the contents of the row of a table
class ABI_EXPORT fp_FieldTableSumCols : public fp_FieldRun
{
public:
	fp_FieldTableSumCols(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldTableSumCols(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_WORD_COUNT;};
};

// count of the #para in the document
class ABI_EXPORT fp_FieldParaCountRun : public fp_FieldRun
{
public:
	fp_FieldParaCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldParaCountRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PARA_COUNT;};
};

// count of #words in the document
class ABI_EXPORT fp_FieldWordCountRun : public fp_FieldRun
{
public:
	fp_FieldWordCountRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldWordCountRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_WORD_COUNT;};
};


// date-releated fields

// Americans - mm/dd/yy
class ABI_EXPORT fp_FieldMMDDYYRun : public fp_FieldRun
{
public:
	fp_FieldMMDDYYRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldMMDDYYRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// most of the world - dd/mm/yy
class ABI_EXPORT fp_FieldDDMMYYRun : public fp_FieldRun
{
public:
	fp_FieldDDMMYYRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldDDMMYYRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// April 18, 1979
class ABI_EXPORT fp_FieldMonthDayYearRun : public fp_FieldRun
{
public:
	fp_FieldMonthDayYearRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldMonthDayYearRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// Apr. 18, 1979
class ABI_EXPORT fp_FieldMthDayYearRun : public fp_FieldRun
{
public:
	fp_FieldMthDayYearRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldMthDayYearRun (){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// default representation for your locale. includes time too
class ABI_EXPORT fp_FieldDefaultDateRun : public fp_FieldRun
{
public:
	fp_FieldDefaultDateRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldDefaultDateRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// default for your locale, not appending the time
class ABI_EXPORT fp_FieldDefaultDateNoTimeRun : public fp_FieldRun
{
public:
	fp_FieldDefaultDateNoTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldDefaultDateNoTimeRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// day of the week (Wednesday)
class ABI_EXPORT fp_FieldWkdayRun : public fp_FieldRun
{
public:
	fp_FieldWkdayRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldWkdayRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// day of year (i.e. 72)
class ABI_EXPORT fp_FieldDOYRun : public fp_FieldRun
{
public:
	fp_FieldDOYRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldDOYRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// military (zulu) time
class ABI_EXPORT fp_FieldMilTimeRun : public fp_FieldRun
{
public:
	fp_FieldMilTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldMilTimeRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// prints am or pm
class ABI_EXPORT fp_FieldAMPMRun : public fp_FieldRun
{
public:
	fp_FieldAMPMRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldAMPMRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// milliseconds since the epoch, for you geeks out there :-)
class ABI_EXPORT fp_FieldTimeEpochRun : public fp_FieldRun
{
public:
	fp_FieldTimeEpochRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldTimeEpochRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

class ABI_EXPORT fp_FieldDateTimeCustomRun : public fp_FieldRun
{
public:
	fp_FieldDateTimeCustomRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldDateTimeCustomRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// your time zone (EST, for example)
class ABI_EXPORT fp_FieldTimeZoneRun : public fp_FieldRun
{
public:
	fp_FieldTimeZoneRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldTimeZoneRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// application runs

// build id
class ABI_EXPORT fp_FieldBuildIdRun : public fp_FieldRun
{
public:
	fp_FieldBuildIdRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldBuildIdRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// build version (i.e. 0.7.13)
class ABI_EXPORT fp_FieldBuildVersionRun : public fp_FieldRun
{
public:
	fp_FieldBuildVersionRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldBuildVersionRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildOptionsRun : public fp_FieldRun
{
public:
	fp_FieldBuildOptionsRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldBuildOptionsRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildTargetRun : public fp_FieldRun
{
public:
	fp_FieldBuildTargetRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldBuildTargetRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildCompileDateRun : public fp_FieldRun
{
public:
	fp_FieldBuildCompileDateRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldBuildCompileDateRun (){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildCompileTimeRun : public fp_FieldRun
{
public:
	fp_FieldBuildCompileTimeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual ~fp_FieldBuildCompileTimeRun(){}

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldMailMergeRun : public fp_FieldRun
{
 public:
  fp_FieldMailMergeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);

  virtual ~fp_FieldMailMergeRun(){}

  virtual bool			calculateValue(void);
  virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }

  virtual UT_uint32		needsFrequentUpdates(){ return FIELD_UPDATE_MAILMERGE; }
};

class ABI_EXPORT fp_FieldMetaRun : public fp_FieldRun
{
 public:
  fp_FieldMetaRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen, const char * which);

  virtual ~fp_FieldMetaRun(){}

  virtual bool			calculateValue(void);
  virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }

  virtual UT_uint32		needsFrequentUpdates(){ return FIELD_UPDATE_META;}

 private:
  std::string m_which;
};

class ABI_EXPORT fp_FieldMetaTitleRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaTitleRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaTitleRun(){}
};

class ABI_EXPORT fp_FieldMetaCreatorRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaCreatorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaCreatorRun(){}
};

class ABI_EXPORT fp_FieldMetaSubjectRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaSubjectRun(fl_BlockLayout* pBL,  UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaSubjectRun(){}
};

class ABI_EXPORT fp_FieldMetaPublisherRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaPublisherRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaPublisherRun(){}
};

class ABI_EXPORT fp_FieldMetaDateRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaDateRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaDateRun(){}
};

class ABI_EXPORT fp_FieldMetaDateLastChangedRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaDateLastChangedRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaDateLastChangedRun(){}
};

class ABI_EXPORT fp_FieldMetaTypeRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaTypeRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaTypeRun(){}
};

class ABI_EXPORT fp_FieldMetaLanguageRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaLanguageRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaLanguageRun(){}
};

class ABI_EXPORT fp_FieldMetaRightsRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaRightsRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaRightsRun(){}
};

class ABI_EXPORT fp_FieldMetaKeywordsRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaKeywordsRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaKeywordsRun(){}
};

class ABI_EXPORT fp_FieldMetaContributorRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaContributorRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaContributorRun(){}
};

class ABI_EXPORT fp_FieldMetaCoverageRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaCoverageRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaCoverageRun(){}
};

class ABI_EXPORT fp_FieldMetaDescriptionRun : public fp_FieldMetaRun
{
 public:
  fp_FieldMetaDescriptionRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
  virtual ~fp_FieldMetaDescriptionRun(){}
};

// END DOM

class ABI_EXPORT fp_FmtMarkRun : public fp_Run
{
public:
	fp_FmtMarkRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isSuperscript(void) const ;
	virtual bool			isSubscript(void)  const;
	virtual bool 			hasLayoutProperties(void) const {return true;}

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;

private:
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte					m_fPosition;
};


class ABI_EXPORT fp_DummyRun : public fp_Run
{
public:
	fp_DummyRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isSuperscript(void) const ;
	virtual bool			isSubscript(void)  const;
	virtual bool 			hasLayoutProperties(void) const {return false;}

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
};

#endif /* FP_RUN_H */










