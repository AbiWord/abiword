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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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
#include "ap_Strings.h"
#include "fl_BlockLayout.h"
#include <fribidi/fribidi.h>
#include "pp_Revision.h"

class UT_GrowBuf;
class fp_Line;
class GR_Graphics;

#ifndef WITH_PANGO
class GR_Font;
#else
struct PangoFont;
#endif

class GR_Image;
class PD_Document;
class PP_AttrProp;
struct dg_DrawArgs;
class fl_CharWidths;
class fd_Field;
class fp_HyperlinkRun;

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
	FPRUN__LAST__					= 14
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

	As far as the formatter's concerned, each subclass behaves somewhat
	differently, but they can all be treated like rectangular blocks to
	be arranged.

	Convention: _setFoo(bar) is just this.foo = bar;
                 setFoo(bar) sets this.foo to bar,
                               but may also do other processing to maintain
							   internal state.
*/
class ABI_EXPORT fp_Run
{
public:
	fp_Run(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst,
		   UT_uint32 iLen, FP_RUN_TYPE iType);
	virtual ~fp_Run();

	// inline getter member functions
	FP_RUN_TYPE		        getType() const 				{ return m_iType; }
	fp_Line*		        getLine() const 				{ return m_pLine; }
	fl_BlockLayout*	        getBlock() const 				{ return m_pBL; }
	UT_sint32		        getX() const 					{ return m_iX; }
	UT_sint32		        getY() const 					{ return m_iY; }

	UT_sint32		        getHeight() const				{ return m_iHeight; }
	UT_sint32		        getWidth() const		        { return m_iWidth; }
	UT_uint32		        getAscent() const				{ return m_iAscent; }
	UT_uint32		        getDescent() const 				{ return m_iDescent; }
	virtual UT_uint32       getDrawingWidth() const         { return m_iWidth; }

#ifndef WITH_PANGO
	UT_sint32		        getHeightInLayoutUnits() const	{ return m_iHeightLayoutUnits; }
	UT_sint32		        getWidthInLayoutUnits() const	{ return m_iWidthLayoutUnits; }
	UT_uint32		        getAscentInLayoutUnits() const	{ return m_iAscentLayoutUnits; }
	UT_uint32		        getDescentInLayoutUnits() const	{ return m_iDescentLayoutUnits; }
#endif

	fp_Run* 		        getNext() const					{ return m_pNext; }
	fp_Run*			        getPrev() const					{ return m_pPrev; }
	UT_uint32		        getBlockOffset() const			{ return m_iOffsetFirst; }
	UT_uint32		        getLength() const				{ return m_iLen; }
	GR_Graphics*	        getGraphics() const				{ return m_pG; }
	GR_Graphics*	        getGR() const					{ return m_pG; }
	fp_HyperlinkRun *       getHyperlink() const 			{ return m_pHyperlink;}

	void                    getSpanAP(const PP_AttrProp * &pSpanAP, bool &bDeleteAfter);

	void					insertIntoRunListBeforeThis(fp_Run& newRun);
	void					insertIntoRunListAfterThis(fp_Run& newRun);
	fd_Field*				getField(void) const { return m_pField; }
	bool					isField(void) const { return (bool) (m_pField != NULL); }
	void					unlinkFromRunList();

	bool                    updateBackgroundColor(void);
	bool		            updateHighlightColor(void);
	bool				    updatePageColor(void);

	const UT_RGBColor		getPageColor(void);
	const UT_RGBColor 		getFGColor(void) const;

	virtual bool			hasLayoutProperties(void) const;

	void					setLine(fp_Line*);
	void					setBlock(fl_BlockLayout * pBL) { _setBlock(pBL); }
	void					setX(UT_sint32, FPRUN_CLEAR_SCREEN eClearScreen = FP_CLEARSCREEN_AUTO);
	void					setY(UT_sint32);
	void					setBlockOffset(UT_uint32);
	void					setLength(UT_uint32);
	void					setNext(fp_Run*, bool bRefresh = true);
	void					setPrev(fp_Run*, bool bRefresh = true);
	void					setHyperlink(fp_HyperlinkRun * pH);
	void					markWidthDirty() {m_bRecalcWidth = true;}
	bool					isFirstRunOnLine(void) const;
	bool					isLastRunOnLine(void) const;
	bool					isOnlyRunOnLine(void) const;
	bool					isFirstVisRunOnLine(void) const;
	bool					isLastVisRunOnLine(void) const;
	void					markDrawBufferDirty() {m_bRefreshDrawBuffer = true;}
	void					draw(dg_DrawArgs*);
	void            		clearScreen(bool bFullLineHeightRect = false);
	void					markAsDirty(void)	{ m_bDirty = true; }
	bool					isDirty(void) const { return m_bDirty; }
	bool			        canContainPoint(void) const;
	virtual const PP_AttrProp* getAP(void) const;
	virtual void			fetchCharWidths(fl_CharWidths * pgbCharWidths);
	virtual	bool			recalcWidth(void);

	virtual void			_draw(dg_DrawArgs*) = 0;
    void                    _drawTextLine(UT_sint32, UT_sint32, UT_uint32, UT_uint32, UT_UCSChar *);
	virtual void       		_clearScreen(bool bFullLineHeightRect) = 0;
	virtual bool			canBreakAfter(void) const = 0;
	virtual bool			canBreakBefore(void) const = 0;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return false; }
	virtual bool			alwaysFits(void) const { return false; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	virtual UT_sint32		findTrailingSpaceDistance(void) const { return 0; }
#ifndef WITH_PANGO
	virtual UT_sint32		findTrailingSpaceDistanceInLayoutUnits(void) const { return 0; }
#endif
	virtual bool			findFirstNonBlankSplitPoint(fp_RunSplitInfo& /*si*/) { return false; }
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL) = 0;
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection) = 0;
	void			        lookupProperties(void);
	virtual bool			doesContainNonBlankData(void) const { return true; }	// Things like text whould return false if it is all spaces.
	void                    drawDecors(UT_sint32 xoff, UT_sint32 yoff);
	virtual bool			isSuperscript(void) const { return false; }
	virtual bool			isSubscript(void) const { return false; }
    bool			        isUnderline(void) const ;
	bool			        isOverline(void) const ;
	bool			        isStrikethrough(void) const ;
	bool			        isTopline(void) const ;
	bool			        isBottomline(void) const ;
	void			        setLinethickness(UT_sint32 max_linethickness);
    UT_sint32		        getLinethickness(void) ;
	void			        setUnderlineXoff(UT_sint32 xoff);
	UT_sint32		        getUnderlineXoff(void);
	void			        setOverlineXoff(UT_sint32 xoff) ;
	UT_sint32		        getOverlineXoff(void) ;
	void			        setMaxUnderline(UT_sint32 xoff) ;
	UT_sint32		        getMaxUnderline(void) ;
	void			        setMinOverline(UT_sint32 xoff) ;
	UT_sint32		        getMinOverline(void) ;
	UT_sint32               getToplineThickness(void);

	virtual FriBidiCharType	getDirection() const { return m_iDirection; };
	FriBidiCharType			getVisDirection();
	virtual void            setDirection(FriBidiCharType iDirection = FRIBIDI_TYPE_WS);
	void					setVisDirection(FriBidiCharType iDir);
	UT_uint32               getVisPosition(UT_uint32 ilogPos);
	UT_uint32               getVisPosition(UT_uint32 iLogPos, UT_uint32 iLen);
	UT_uint32               getOffsetFirstVis();
	UT_uint32               getOffsetLog(UT_uint32 iVisOff);
	//virtual void            setDirectionProperty(FriBidiCharType dir);
	fp_Run *				getNextVisual();
	fp_Run *				getPrevVisual();

	bool                    containsRevisions(){return (m_pRevisions != NULL);}
	// would prefer to make the return value const, but the
	// getLastRevision() and related functions use internal cache so
	// they could not be called
	PP_RevisionAttr *       getRevisions() const {return m_pRevisions;}
	FPVisibility            isHidden() const {return m_eHidden;}
	void                    setVisibility(FPVisibility eVis) {m_eHidden = eVis;}


#ifdef FMT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	void					_inheritProperties(void);
	fp_Run*					_findPrevPropertyRun(void) const;

	FV_View*				_getView(void) const { return getBlock()->getView(); }
	// By convention, _getFoo and _setFoo have no side effects.
	// They can easily be inlined by a smart compiler.
	UT_RGBColor				_getColorPG(void) const { return m_pColorPG; }
	UT_RGBColor				_getColorFG(void) const { return m_pColorFG; }
	UT_RGBColor				_getColorHL(void) const { return m_pColorHL; }
	bool					_setColorFG(UT_RGBColor c)
								{
									UT_RGBColor o = c;
									m_pColorFG = c;
									return o != c;
								}
	bool					_setColorHL(UT_RGBColor c)
								{
									UT_RGBColor o = c;
									m_pColorHL = c;
									return o != c;
								}

	bool                    _setColorHL(const char *pszColor)
	                            {
		                            return m_pColorHL.setColor(pszColor);
								}
	
	void					_setLine(fp_Line* pLine) { m_pLine = pLine; }
	void					_setHeight(UT_sint32 iHeight)
								{ m_iHeight = iHeight;}
	void					_setWidth(UT_sint32 iWidth)
                        		{ m_iWidth = iWidth; }
	void					_setBlock(fl_BlockLayout * pBL) { m_pBL = pBL; }
	void					_setAscent(int iAscent) { m_iAscent = iAscent; }
	void					_setDescent(int iDescent) {m_iDescent = iDescent;}
	void					_setAscentLayoutUnits(int iAscent)
                                { m_iAscentLayoutUnits = iAscent; }
	void					_setDescentLayoutUnits(int iDescent)
			   					{ m_iDescentLayoutUnits = iDescent; }
	void					_setWidthLayoutUnits(int iWidth)
			   					{ m_iWidthLayoutUnits = iWidth; }
	void					_setHeightLayoutUnits(int iHeight)
			   					{ m_iHeightLayoutUnits = iHeight; }
	void					_setX(int iX) { m_iX = iX; }
	void					_setY(int iY) { m_iY = iY; }
	void					_setDirection(FriBidiCharType c) { m_iDirection = c; }
	FriBidiCharType			_getDirection(void) const { return m_iDirection; }
	FriBidiCharType			_getVisDirection(void) const { return m_iVisDirection; }
	GR_Font *				_getScreenFont(void) const { return m_pScreenFont; }
	void  					_setScreenFont(GR_Font * f) { m_pScreenFont = f; }
	GR_Font *				_getLayoutFont(void) const { return m_pLayoutFont; }
	void  					_setLayoutFont(GR_Font * f) { m_pLayoutFont = f; }
#ifdef WITH_PANGO
	PangoFont *				_getPangoFont(void) const { return m_pPangoFont; }
	void  					_setPangoFont(Pango * f) { m_pPangoFont = f; }
#endif

	unsigned char			_getDecorations(void) const { return m_fDecorations; }
	void					_setDecorations(unsigned char d) {m_fDecorations = d;}
	
	void					_orDecorations(unsigned char d) { m_fDecorations |= d; }
	UT_sint32				_getLineWidth(void) { return m_iLineWidth; }
	bool					_setLineWidth(UT_sint32 w)
	                             {
									 UT_sint32 o = m_iLineWidth;
									 m_iLineWidth = w;
									 return o != w;
								 }
	void					_setLength(UT_uint32 l) { m_iLen = l; }
	void					_setRevisions(PP_RevisionAttr * p) { m_pRevisions = p; }
	void					_setDirty(bool b) { m_bDirty = b; }
	void					_setField(fd_Field * fd) { m_pField = fd; }
	void                    _setHyperlink(fp_HyperlinkRun * pH) { m_pHyperlink = pH; }
	bool					_getRecalcWidth(void) const { return m_bRecalcWidth; }
	void					_setRecalcWidth(bool b) { m_bRecalcWidth = b; }
	bool					_getRefreshDrawBuffer(void) const { return m_bRefreshDrawBuffer; }
	void					_setRefreshDrawBuffer(bool b) { m_bRefreshDrawBuffer = b; }
	virtual void	        _lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP) = 0;

	virtual bool            _canContainPoint(void) const;

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
#ifndef WITH_PANGO
	UT_sint32				m_iHeightLayoutUnits;
	UT_sint32				m_iWidthLayoutUnits;
	UT_uint32				m_iAscentLayoutUnits;
	UT_uint32				m_iDescentLayoutUnits;
#endif

	UT_uint32				m_iOffsetFirst;
	UT_uint32				m_iLen;
	GR_Graphics*			m_pG;
	bool					m_bDirty;		// run erased @ old coords, needs to be redrawn
	fd_Field*				m_pField;
	FriBidiCharType			m_iDirection;   //#TF direction of the run 0 for left-to-right, 1 for right-to-left
	FriBidiCharType			m_iVisDirection;
	bool 					m_bRefreshDrawBuffer;

	// the run highlight color. If the property is transparent use the page color
	UT_RGBColor             m_pColorHL;

#ifndef WITH_PANGO
	GR_Font * 				m_pScreenFont;
	GR_Font * 				m_pLayoutFont;
#else
	PangoFont * 			m_pPangoFont;
#endif

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
	FPVisibility            m_eHidden;
};

class ABI_EXPORT fp_TabRun : public fp_Run
{
public:
	fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool 			hasLayoutProperties(void) const;
	void			       	setTabWidth(UT_sint32);
	void			       	setLeader(eTabLeader iTabType);
	eTabLeader			    getLeader(void);
	void                    setTabType(eTabType iTabType);
	eTabType                getTabType(void) const;
protected:
	virtual void			_drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 iHeight);
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);
private:
	//UT_RGBColor			    m_colorFG;
private:
	eTabLeader			    m_leader;
    eTabType                m_TabType;
};

class ABI_EXPORT fp_ForcedLineBreakRun : public fp_Run
{
public:
	fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class ABI_EXPORT fp_FieldStartRun : public fp_Run
{
public:
	fp_FieldStartRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class ABI_EXPORT fp_FieldEndRun : public fp_Run
{
public:
	fp_FieldEndRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class ABI_EXPORT fp_ForcedColumnBreakRun : public fp_Run
{
public:
	fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class ABI_EXPORT fp_ForcedPageBreakRun : public fp_Run
{
public:
	fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class ABI_EXPORT fp_EndOfParagraphRun : public fp_Run
{
public:
	fp_EndOfParagraphRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual UT_uint32       getDrawingWidth() const { return m_iDrawWidth;}

//
// Tomas this breaks line breaking....
//	virtual bool			doesContainNonBlankData(void) const { return false; }	// Things like text whould return false if it is all spaces.

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(bool bFullLineHeightRect);

private:
	UT_uint32				m_iXoffText;
	UT_uint32				m_iYoffText;
	UT_uint32				m_iDrawWidth;

};

class ABI_EXPORT fp_BookmarkRun : public fp_Run
{
public:
	fp_BookmarkRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	bool 				isStartOfBookmark() const {return m_bIsStart;};
	const XML_Char * 	getName() const {return m_pName;};
	bool 				isComrade(fp_BookmarkRun *pBR) const;

	virtual bool canBreakAfter(void) const;
	virtual bool canBreakBefore(void) const;
	virtual bool letPointPass(void) const;

	virtual void mapXYToPosition(UT_sint32 x,
								 UT_sint32 y,
								 PT_DocPosition& pos,
								 bool& bBOL,
								 bool& bEOL);

	virtual void findPointCoords(UT_uint32 iOffset,
								 UT_sint32& x,
								 UT_sint32& y,
								 UT_sint32& x2,
								 UT_sint32& y2,
								 UT_sint32& height,
								 bool& bDirection);


private:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void _clearScreen(bool /* bFullLineHeightRect */);
	virtual void _draw(dg_DrawArgs* /*pDA */);

	bool m_bIsStart;
	#define BOOKMARK_NAME_SIZE 30
	XML_Char	  	m_pName[BOOKMARK_NAME_SIZE + 1];
	po_Bookmark		* m_pBookmark;
};

class ABI_EXPORT fp_HyperlinkRun : public fp_Run
{
public:
	fp_HyperlinkRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	~fp_HyperlinkRun();
	bool 				isStartOfHyperlink() const {return m_bIsStart;};
	const XML_Char * 	getTarget() const {return (const XML_Char *)m_pTarget;};

	virtual bool canBreakAfter(void) const;
	virtual bool canBreakBefore(void) const;
	virtual bool letPointPass(void) const;

	virtual void mapXYToPosition(UT_sint32 x,
								 UT_sint32 y,
								 PT_DocPosition& pos,
								 bool& bBOL,
								 bool& bEOL);

	virtual void findPointCoords(UT_uint32 iOffset,
								 UT_sint32& x,
								 UT_sint32& y,
								 UT_sint32& x2,
								 UT_sint32& y2,
								 UT_sint32& height,
								 bool& bDirection);


private:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void _clearScreen(bool /* bFullLineHeightRect */);
	virtual void _draw(dg_DrawArgs* /*pDA */);

	bool m_bIsStart;
	XML_Char *	  	m_pTarget;
};


class ABI_EXPORT fp_ImageRun : public fp_Run
{
public:
	fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, FG_Graphic * pGraphic);
	virtual ~fp_ImageRun();

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	const char *            getDataId(void) const;
	virtual bool 			hasLayoutProperties(void) const;
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);

private:
	FG_Graphic *             m_pFGraphic;
	GR_Image*				m_pImage;
	UT_sint32               m_iImageWidth;
	UT_sint32               m_iImageHeight;
#ifndef WITH_PANGO
	UT_sint32               m_iImageWidthLayoutUnits;
	UT_sint32               m_iImageHeightLayoutUnits;
#endif
	UT_String               m_WidthProp;
	UT_String               m_HeightProp;
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

class ABI_EXPORT fp_FieldRun : public fp_Run
{
public:
	fp_FieldRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual ~fp_FieldRun() {return;};

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual fp_FieldsEnum	getFieldType(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool 			hasLayoutProperties(void) const;

	virtual bool			isSuperscript(void) const;
	virtual bool			isSubscript(void) const;

	bool					_setValue(UT_UCSChar *p_new_value);

	virtual bool			calculateValue(void);
	virtual bool			recalcWidth(void);
	virtual UT_UCSChar *    getValue(void) const { return (UT_UCSChar *) m_sFieldValue;}
	virtual UT_uint32		needsFrequentUpdates() {return 0;}

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*) {};
	virtual void			_defaultDraw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	const XML_Char *		_getParameter() const { return m_pParameter; }

private:
#ifndef WITH_PANGO
	GR_Font*				m_pFont;
	GR_Font*				m_pFontLayout;
#else
	//PangoFont *           m_pPangoFont; // I do not think we need this, just refer to fp_Run
#endif

	//UT_RGBColor				m_colorFG;
	UT_RGBColor				m_colorBG;
	UT_UCSChar				m_sFieldValue[FPFIELD_MAX_LENGTH];
	fp_FieldsEnum			m_iFieldType;
	const XML_Char *		m_pParameter;
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte					m_fPosition;
};

class ABI_EXPORT fp_FieldFootnoteRefRun : public fp_FieldRun
{
public:

	fp_FieldFootnoteRefRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_ENDNOTE;};
#if 0
	UT_uint32				getPID() const {return m_iPID;}
private:
	UT_uint32 m_iPID;
#endif
};

class ABI_EXPORT fp_FieldFootnoteAnchorRun : public fp_FieldRun
{
public:

	fp_FieldFootnoteAnchorRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_ENDNOTE;};
#if 0
	UT_uint32				getPID() const {return m_iPID;}
private:
	UT_uint32 m_iPID;
#endif
};

class ABI_EXPORT fp_FieldTimeRun : public fp_FieldRun
{
public:

	fp_FieldTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

class ABI_EXPORT fp_FieldPageNumberRun : public fp_FieldRun
{
public:

	fp_FieldPageNumberRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PAGE;};
};

class ABI_EXPORT fp_FieldPageReferenceRun : public fp_FieldRun
{
public:

	fp_FieldPageReferenceRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PAGE;};
};

class ABI_EXPORT fp_FieldPageCountRun : public fp_FieldRun
{
public:

	fp_FieldPageCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PAGE;};
};

class ABI_EXPORT fp_FieldDateRun : public fp_FieldRun
{
public:
	fp_FieldDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

class ABI_EXPORT fp_FieldFileNameRun : public fp_FieldRun
{
public:
	fp_FieldFileNameRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

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
	fp_FieldCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// count of the non-blank characters
// in the document
class ABI_EXPORT fp_FieldNonBlankCharCountRun : public fp_FieldRun
{
public:
	fp_FieldNonBlankCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// count of the #lines in the document
class ABI_EXPORT fp_FieldLineCountRun : public fp_FieldRun
{
public:
	fp_FieldLineCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_LINE_COUNT;};
};

// count of the #para in the document
class ABI_EXPORT fp_FieldParaCountRun : public fp_FieldRun
{
public:
	fp_FieldParaCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_PARA_COUNT;};
};

// count of #words in the document
class ABI_EXPORT fp_FieldWordCountRun : public fp_FieldRun
{
public:
	fp_FieldWordCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_WORD_COUNT;};
};


// date-releated fields

// Americans - mm/dd/yy
class ABI_EXPORT fp_FieldMMDDYYRun : public fp_FieldRun
{
public:
	fp_FieldMMDDYYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// most of the world - dd/mm/yy
class ABI_EXPORT fp_FieldDDMMYYRun : public fp_FieldRun
{
public:
	fp_FieldDDMMYYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// April 18, 1979
class ABI_EXPORT fp_FieldMonthDayYearRun : public fp_FieldRun
{
public:
	fp_FieldMonthDayYearRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// Apr. 18, 1979
class ABI_EXPORT fp_FieldMthDayYearRun : public fp_FieldRun
{
public:
	fp_FieldMthDayYearRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// default representation for your locale. includes time too
class ABI_EXPORT fp_FieldDefaultDateRun : public fp_FieldRun
{
public:
	fp_FieldDefaultDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// default for your locale, not appending the time
class ABI_EXPORT fp_FieldDefaultDateNoTimeRun : public fp_FieldRun
{
public:
	fp_FieldDefaultDateNoTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// day of the week (Wednesday)
class ABI_EXPORT fp_FieldWkdayRun : public fp_FieldRun
{
public:
	fp_FieldWkdayRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// day of year (i.e. 72)
class ABI_EXPORT fp_FieldDOYRun : public fp_FieldRun
{
public:
	fp_FieldDOYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// military (zulu) time
class ABI_EXPORT fp_FieldMilTimeRun : public fp_FieldRun
{
public:
	fp_FieldMilTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// prints am or pm
class ABI_EXPORT fp_FieldAMPMRun : public fp_FieldRun
{
public:
	fp_FieldAMPMRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_DATE;};
};

// milliseconds since the epoch, for you geeks out there :-)
class ABI_EXPORT fp_FieldTimeEpochRun : public fp_FieldRun
{
public:
	fp_FieldTimeEpochRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
	virtual UT_uint32		needsFrequentUpdates(){return FIELD_UPDATE_TIME;};
};

// your time zone (EST, for example)
class ABI_EXPORT fp_FieldTimeZoneRun : public fp_FieldRun
{
public:
	fp_FieldTimeZoneRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// application runs

// build id
class ABI_EXPORT fp_FieldBuildIdRun : public fp_FieldRun
{
public:
	fp_FieldBuildIdRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// build version (i.e. 0.7.13)
class ABI_EXPORT fp_FieldBuildVersionRun : public fp_FieldRun
{
public:
	fp_FieldBuildVersionRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildOptionsRun : public fp_FieldRun
{
public:
	fp_FieldBuildOptionsRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildTargetRun : public fp_FieldRun
{
public:
	fp_FieldBuildTargetRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildCompileDateRun : public fp_FieldRun
{
public:
	fp_FieldBuildCompileDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class ABI_EXPORT fp_FieldBuildCompileTimeRun : public fp_FieldRun
{
public:
	fp_FieldBuildCompileTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// END DOM

class ABI_EXPORT fp_FmtMarkRun : public fp_Run
{
public:
	fp_FmtMarkRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual const PP_AttrProp* getAP(void) const;
	virtual bool			isSuperscript(void) const { return false; }
	virtual bool			isSubscript(void)  const { return false; }

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP);

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);

private:
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte					m_fPosition;
};

#endif /* FP_RUN_H */










