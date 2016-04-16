/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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

#ifndef FL_BLOCKLAYOUT_H
#define FL_BLOCKLAYOUT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_growbuf.h"
#include "ut_xml.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"		// FIXME: this is needed for the friend'ed function
#include "fg_Graphic.h"
#include "fl_AutoLists.h"
#include "pp_Property.h"
#include "fl_ContainerLayout.h"
#include "fl_SectionLayout.h"
#include "fb_LineBreaker.h"
#include "ut_string_class.h"
#include "ut_misc.h"
#include "pp_PropertyMap.h"

// number of DocPositions occupied by the block strux
#define fl_BLOCK_STRUX_OFFSET	1

class fl_Squiggles;
class fl_SpellSquiggles;
class fl_GrammarSquiggles;
class FL_DocLayout;
class fl_SectionLayout;
class fl_ContainerLayout;
class fb_LineBreaker;
class fb_Alignment;
class fp_Line;
class fp_Run;
class GR_Graphics;
class PD_Document;
class PP_Property;
class PX_ChangeRecord_FmtMark;
class PX_ChangeRecord_FmtMarkChange;
class PX_ChangeRecord_Object;
class PX_ChangeRecord_ObjectChange;
class PX_ChangeRecord_Span;
class PX_ChangeRecord_SpanChange;
class PX_ChangeRecord_Strux;
class PX_ChangeRecord_StruxChange;
class pf_Frag_Strux;
class pf_Frag_Object;
class fl_AutoNum;
class fp_VerticalContainer;
class fp_HyperlinkRun;

// Tab types and leaders
typedef enum {
	FL_TAB_NONE = 0,
	FL_TAB_LEFT,
	FL_TAB_CENTER,
	FL_TAB_RIGHT,
	FL_TAB_DECIMAL,
	FL_TAB_BAR,
	__FL_TAB_MAX
} eTabType;

typedef enum {
	FL_LEADER_NONE = 0,
	FL_LEADER_DOT,
	FL_LEADER_HYPHEN,
	FL_LEADER_UNDERLINE,
	FL_LEADER_THICKLINE,
	FL_LEADER_EQUALSIGN,
	__FL_LEADER_MAX
} eTabLeader;


/*
	Blocks are stored in a linked list which contains all of the blocks in
	the normal flow, in order.
*/

class SpellChecker;
class fl_TabStop;
ABI_EXPORT void buildTabStops(const char* pszTabStops, UT_GenericVector<fl_TabStop*> &m_vecTabs);

class ABI_EXPORT fl_BlockLayout : public fl_ContainerLayout
{
	friend class fl_Squiggles;
	friend class fl_SpellSquiggles;
	friend class fl_GrammarSquiggles;
	friend class fl_DocListener;
	friend class fl_TOCLayout;
	friend class fb_LineBreaker;

#ifdef ENABLE_SPELL
	// TODO: shack - code should be moved from toggleAuto to a function in
	// here - to handle the squiggles
	friend void FL_DocLayout::_toggleAutoSpell(bool bSpell);
#endif

public:
	fl_BlockLayout(pf_Frag_Strux* sdh,
				   fl_ContainerLayout* pPrev, fl_SectionLayout*,
				   PT_AttrPropIndex indexAP, bool bIsHdrFtr = false);
	~fl_BlockLayout();

	typedef enum _eSpacingPolicy
	{
		spacing_MULTIPLE,
		spacing_EXACT,
		spacing_ATLEAST
	} eSpacingPolicy;

    void                formatAll(void);
	virtual void        format(void);
	void                formatWrappedFromHere(fp_Line * pLine,fp_Page * pPage);
	fp_Line *           getNextWrappedLine(UT_sint32 iX,
											  UT_sint32 iHeight,
										   fp_Page * pPage);
	void                getLeftRightForWrapping(UT_sint32 iX,
												UT_sint32 iHeight,
												UT_sint32 & iMinLeft,
												UT_sint32 & iMinRight,
												UT_sint32 & iMinWidth);
	virtual bool		recalculateFields(UT_uint32 iUpdateCount);

	virtual void		redrawUpdate();
	virtual void        updateLayout(bool /*bDoAll*/) {}
	virtual fp_Container * getNewContainer(fp_Container * pCon = NULL);
	FV_View *		getView(void) const {
		UT_return_val_if_fail( m_pLayout, NULL );
		return m_pLayout->getView();
	}

	const char* getProperty(const gchar * pszName, bool bExpandStyles = true) const;
	std::unique_ptr<PP_PropertyType> getPropertyType(const gchar * szName,
											tProperty_type Type, bool bExpandStyles = true) const;
	void setAlignment(UT_uint32 iAlignCmd);
	UT_sint32       getLength(void) const;
	bool            isEmbeddedType(void) const;
	bool            isNotTOCable(void) const;
	bool            isLastRunInBlock(fp_Run * pRun) const;
	void            updateOffsets(PT_DocPosition posEmbedded,
								  UT_uint32 iEmebbedSize, UT_sint32 iSuggestedDiff);
	void            updateEnclosingBlockIfNeeded(void);
	fl_BlockLayout * getEnclosingBlock(void) const;
	UT_sint32       getEmbeddedOffset(UT_sint32 startOffset, fl_ContainerLayout *& pEmbedCL) const;
	void            shuffleEmbeddedIfNeeded(fl_BlockLayout * pBlock, UT_uint32 blockOffset);

	bool            getXYOffsetToLine(UT_sint32 & xoff, UT_sint32 & yoff, fp_Line * pLine) const;
	bool            setFramesOnPage(fp_Line * pLastLine);
	UT_sint32       getMinWrapWidth(void) const;
	UT_sint32       getHeightOfBlock(bool b_withMargins = true);
	fp_Line *       findLineWithFootnotePID(UT_uint32 pid) const;
	UT_sint32 getMaxNonBreakableRun(void) const;
	fp_Line* findPrevLineInDocument(fp_Line*) const;
	fp_Line* findNextLineInDocument(fp_Line*) const;
	virtual void     appendTextToBuf(UT_GrowBuf & buf) const;
	void             appendUTF8String(UT_UTF8String & sText) const;
	virtual fp_Run* getFirstRun(void) const { return m_pFirstRun; }
	inline void setFirstRun(fp_Run* pRun) { m_pFirstRun = pRun; }
	void        clearPrint(void) const;
	inline bool isListItem(void) const { return m_bListItem; }
	bool isFirstInList(void) const;
//	inline fl_AutoNum * getAutoNum(void) const { return m_pAutoNum; }
	void	getListAttributesVector(UT_GenericVector<const gchar*> * va) const;
	void  getListPropertyVector(UT_GenericVector<const gchar*> * vp) const;

	void  refreshRunProperties(void) const;
	char *	getFormatFromListType(FL_ListType iListType) const;
	void remItemFromList(void);
	virtual void listUpdate(void);
	void resumeList( fl_BlockLayout * prevList);
	void prependList( fl_BlockLayout * nextList);
	FL_ListType decodeListType(char * listformat) const;
	FL_ListType getListType(void) const;
	gchar* getListStyleString( FL_ListType iListType) const;
	FL_ListType getListTypeFromStyle( const gchar * style) const;
	fl_BlockLayout * getNextList(UT_uint32 id) const;
	bool isListLabelInBlock(void) const;
	void StartList( const gchar * style, pf_Frag_Strux* prevSDH = NULL);

	void StartList( FL_ListType lType, UT_uint32 start,
					const gchar * lDelim, const gchar * lDecimal,
					const gchar * fFont, float Align, float indent,
					UT_uint32 iParentID = 0, UT_uint32 level=0 );

	void StopListInBlock(void);
	void deleteListLabel(void);
	UT_UCSChar * getListLabel(void) const;
	void transferListFlags(void);
	UT_uint32 getLevel(void) const;
	void setStarting( bool bValue);
	void setStopping( bool bValue);
	fl_BlockLayout * getPreviousList(UT_uint32 id) const;
	fl_BlockLayout * getPreviousList(void) const;
	fl_BlockLayout * getPreviousListOfSameMargin(void) const;
	fl_BlockLayout * getParentItem(void) const;

#ifdef ENABLE_SPELL
	void findSpellSquigglesForRun(fp_Run* pRun) const;
	void drawGrammarSquiggles(void) const;
	void findGrammarSquigglesForRun(fp_Run* pRun) const;
#endif

	UT_uint32 canSlurp(fp_Line* pLine) const;

	PT_DocPosition getPosition(bool bActualBlockPos=false) const;
	fp_Run* findPointCoords(PT_DocPosition position, bool bEOL,
							UT_sint32& x, UT_sint32& y, UT_sint32& x2,
							UT_sint32& y2, UT_sint32& height, bool& bDirection) const;

	fp_Run* findRunAtOffset(UT_uint32 offset) const;

	bool	getBlockBuf(UT_GrowBuf * pgb) const;

	void clearScreen(GR_Graphics*) const;


	void                getStyle(UT_UTF8String & sStyle) const;
	UT_sint32	        getTextIndent(void) const;
	inline UT_sint32	getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32	getRightMargin(void) const { return m_iRightMargin; }
	inline UT_sint32	getTopMargin(void) const { return m_iTopMargin; }
	inline UT_sint32	getBottomMargin(void) const { return m_iBottomMargin; }
	inline fb_Alignment *		getAlignment(void) const { return m_pAlignment; }
	virtual FL_DocLayout*		getDocLayout(void) const { return m_pLayout; }
	virtual fl_SectionLayout*	getSectionLayout(void) const { return m_pSectionLayout;}
	fl_DocSectionLayout * getDocSectionLayout(void) const;

	void setSectionLayout(fl_SectionLayout* pSectionLayout);

	void getLineSpacing(double& dSpacing,
						eSpacingPolicy& eSpacing) const;

	inline UT_uint32 getProp_Orphans(void) const { return m_iOrphansProperty; }
	inline UT_uint32 getProp_Widows(void) const { return m_iWidowsProperty; }
	inline bool getProp_KeepTogether(void) const { return m_bKeepTogether; }
	inline bool getProp_KeepWithNext(void) const { return m_bKeepWithNext; }

	inline UT_BidiCharType getDominantDirection(void) const { return m_iDomDirection; }
	void setDominantDirection(UT_BidiCharType iDirection);

#ifdef ENABLE_SPELL
	inline fl_SpellSquiggles* getSpellSquiggles(void) const { return m_pSpellSquiggles; }
	inline fl_GrammarSquiggles* getGrammarSquiggles(void) const { return  m_pGrammarSquiggles; }
#endif

	bool isHdrFtr(void) const;
	void setHdrFtr(void) { m_bIsHdrFtr = true;}
	void clearHdrFtr(void) { m_bIsHdrFtr = false;}

#ifdef ENABLE_SPELL
	bool checkSpelling(void);
#endif
	void debugFlashing(void);
	bool	findNextTabStop(UT_sint32 iStartX, UT_sint32 iMaxX,
							UT_sint32& iPosition, eTabType& iType,
							eTabLeader &iLeader ) const;
	bool	findPrevTabStop(UT_sint32 iStartX, UT_sint32 iMaxX,
							UT_sint32& iPosition, eTabType& iType,
							eTabLeader &iLeader ) const;
	bool    hasUpdatableField(void) { return m_bHasUpdatableField;}
	void    setUpdatableField(bool bValue) { m_bHasUpdatableField = bValue;}
	inline UT_sint32 getDefaultTabInterval(void) const { return m_iDefaultTabInterval; }
	inline UT_sint32 getTabsCount(void) const {
		return m_vecTabs.getItemCount();
	}

	bool doclistener_populateSpan(const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	bool doclistener_populateObject(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);

	bool doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs);
	bool doclistener_deleteSpan(const PX_ChangeRecord_Span * pcrs);
	bool doclistener_changeSpan(const PX_ChangeRecord_SpanChange * pcrsc);
	bool doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	bool doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	bool doclistener_insertFirstBlock(const PX_ChangeRecord_Strux * pcrx,
									  pf_Frag_Strux* sdh,
									  PL_ListenerId lid,
									  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
															  PL_ListenerId lid,
															  fl_ContainerLayout* sfhNew));
	bool doclistener_insertBlock(const PX_ChangeRecord_Strux * pcrx,
								 pf_Frag_Strux* sdh,
								 PL_ListenerId lid,
								 void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
														 PL_ListenerId lid,
														 fl_ContainerLayout* sfhNew));
	bool doclistener_insertSection(const PX_ChangeRecord_Strux * pcrx,
								   SectionType iType,
								   pf_Frag_Strux* sdh,
								   PL_ListenerId lid,
								   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
														   PL_ListenerId lid,
														   fl_ContainerLayout* sfhNew));

	fl_SectionLayout *  doclistener_insertTable(const PX_ChangeRecord_Strux * pcrx,
								   SectionType iType,
								   pf_Frag_Strux* sdh,
								   PL_ListenerId lid,
								   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
														   PL_ListenerId lid,
														   fl_ContainerLayout* sfhNew));
	fl_SectionLayout *  doclistener_insertFrame(const PX_ChangeRecord_Strux * pcrx,
								   SectionType iType,
								   pf_Frag_Strux* sdh,
								   PL_ListenerId lid,
								   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
														   PL_ListenerId lid,
														   fl_ContainerLayout* sfhNew));

	bool doclistener_insertObject(const PX_ChangeRecord_Object * pcro);
	bool doclistener_deleteObject(const PX_ChangeRecord_Object * pcro);
	bool doclistener_changeObject(const PX_ChangeRecord_ObjectChange * pcroc);

	bool doclistener_insertFmtMark(const PX_ChangeRecord_FmtMark * pcrfm);
	bool doclistener_deleteFmtMark(const PX_ChangeRecord_FmtMark * pcrfm);
	bool doclistener_changeFmtMark(const PX_ChangeRecord_FmtMarkChange * pcrfmc);

	void					purgeLayout(void);
	virtual void			collapse(void);
	virtual bool			isCollapsed(void) const
		{return m_bIsCollapsed;}
	void					coalesceRuns(void) const;
	virtual void			setNeedsReformat(fl_ContainerLayout * pCL, UT_uint32 offset = 0);
	inline bool 		    needsReformat(void) const
		{ return (m_iNeedsReformat >= 0); }
	virtual void			setNeedsRedraw(void);
	virtual bool 		    needsRedraw(void) const
		{ return m_bNeedsRedraw; }
	virtual void			markAllRunsDirty(void);
	UT_sint32               findLineInBlock(fp_Line * pLine) const;

	bool                    isWordDelimiter(UT_UCS4Char c, UT_UCS4Char next, UT_UCS4Char prev, UT_uint32 iBlockPos) const;
	bool                    isSentenceSeparator(UT_UCS4Char c, UT_uint32 iBlockPos) const;
#ifdef ENABLE_SPELL
	bool					checkWord(fl_PartOfBlock* pPOB) const;
	void					recheckIgnoredWords();
#endif
	void                    setStyleInTOC(bool b)
	{	m_bStyleInTOC = b;}
	void                    forceSectionBreak(void);
	bool                    isContainedByTOC(void) const
	    { return m_bIsTOC;}
	FootnoteType            getTOCNumType(void) const;
	eTabLeader              getTOCTabLeader(UT_sint32 iOff) const;
	UT_sint32               getTOCTabPosition(UT_sint32 iOff) const;
	void                    setAccumHeight(UT_sint32 i)
	{ m_iAccumulatedHeight =i;}
	UT_sint32               getAccumHeight(void) const
	{ return m_iAccumulatedHeight;}
	static bool 		s_EnumTabStops(void * myThis, UT_uint32 k, fl_TabStop *pTabInfo);

	inline void 		addBackgroundCheckReason(UT_uint32 reason) {m_uBackgroundCheckReasons |= reason;}
	inline void 		removeBackgroundCheckReason(UT_uint32 reason) {m_uBackgroundCheckReasons &= ~reason;}
	inline bool 	hasBackgroundCheckReason(UT_uint32 reason) const {return ((m_uBackgroundCheckReasons & reason) ? true : false);}

	// The following is a set of bit flags giving the reason this block is
	// queued for background checking.	See specific values in fl_DocLayout.h
	UT_uint32				m_uBackgroundCheckReasons;
	void                    setPrevListLabel(bool b)
	{ m_bPrevListLabel = b;}
	bool                    getNextTableElement(UT_GrowBuf * buf,
												PT_DocPosition startPos,
												PT_DocPosition & begPos,
												PT_DocPosition & endPos,
												UT_UTF8String & sWord,
												UT_uint32 iDelim) const;
	bool                   itemizeSpan(PT_BlockOffset blockOffset, UT_uint32 len,GR_Itemization & I);
	const UT_RGBColor      getShadingingForeColor(void) const;
	const UT_RGBColor      getShadingingBackColor(void) const;
	UT_sint32              getPattern(void) const;

	const PP_PropertyMap::Line & getBottom () const { return m_lineBottom; }
	const PP_PropertyMap::Line & getLeft ()   const { return m_lineLeft; }
	const PP_PropertyMap::Line & getRight ()  const { return m_lineRight; }
	const PP_PropertyMap::Line & getTop ()    const { return m_lineTop; }

	bool                   hasBorders(void) const;
	bool                   canMergeBordersWithPrev(void) const;
	bool                   canMergeBordersWithNext(void) const;
	void                   setLineHeightBlockWithBorders(int whichLine = 0);

#ifdef ENABLE_SPELL
	/** put in queue for spellchecking after prev. If prev == NULL is put at the head */
	void enqueueToSpellCheckAfter(fl_BlockLayout *prev);
	/** remove from the spellchecking queue */
	void dequeueFromSpellCheck(void);
	/** call to clear the queue. Warning, you can mess up things */
	void clearQueueing(void)
	{
		m_prevToSpell = m_nextToSpell = NULL;
	}
	fl_BlockLayout *nextToSpell(void) const
	{
		return m_nextToSpell;
	}
	/** return true if the block is queued */
	bool isQueued(void) const
	{
		return (m_prevToSpell != NULL)
			|| (m_pLayout->spellQueueHead() == this);
	}
#endif

#ifdef FMT_TEST
	void					__dump(FILE * fp) const;
#endif

private:
	virtual bool            _canContainPoint() const;

protected:

	void					_recalcPendingWord(UT_uint32 iOffset, UT_sint32 chg) const;
	bool					_doCheckWord(fl_PartOfBlock* pPOB,
										 const UT_UCSChar* pBlockText,
										 UT_sint32 iLength,
										 bool bAddSquiggle = true,
										 bool bClearScreen = true) const;

#ifdef ENABLE_SPELL
	bool					_spellCheckWord(const UT_UCSChar * word, UT_uint32 len, UT_uint32 blockPos) const;
	SpellChecker * _getSpellChecker (UT_uint32 blockPos) const;
#endif

	bool					_truncateLayout(fp_Run* pTruncRun);

#ifndef NDEBUG
	void					_assertRunListIntegrityImpl(void) const;
#endif
	void             			_assertRunListIntegrity(void) const;

	void					_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge) const;

	bool					_doInsertRun(fp_Run* pNewRun);
	bool					_delete(PT_BlockOffset blockOffset, UT_uint32 len);
	bool					_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len);
	bool					_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset);
	bool					_doInsertFieldStartRun(PT_BlockOffset blockOffset);
	bool					_doInsertFieldEndRun(PT_BlockOffset blockOffset);
	bool					_doInsertBookmarkRun(PT_BlockOffset blockOffset);
	bool					_doInsertHyperlinkRun(PT_BlockOffset blockOffset);
	bool					_doInsertAnnotationRun(PT_BlockOffset blockOffset);
	bool					_doInsertRDFAnchorRun(PT_BlockOffset blockOffset);
    void                    _finishInsertHyperlinkedNewRun( PT_BlockOffset blockOffset, fp_HyperlinkRun* pNewRun );
	bool					_doInsertMathRun(PT_BlockOffset blockOffset,PT_AttrPropIndex indexAP,pf_Frag_Object* oh);
	bool					_doInsertEmbedRun(PT_BlockOffset blockOffset,PT_AttrPropIndex indexAP,pf_Frag_Object* oh);
//	bool					_deleteBookmarkRun(PT_BlockOffset blockOffset);
	bool					_doInsertForcedColumnBreakRun(PT_BlockOffset blockOffset);
	bool					_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset);
	bool					_doInsertTabRun(PT_BlockOffset blockOffset);
	bool					_doInsertTOCTabRun(PT_BlockOffset blockOffset);
	bool					_doInsertTOCListLabelRun(PT_BlockOffset blockOffset);
	bool					_doInsertTOCHeadingRun(PT_BlockOffset blockOffset);
	bool                    _doInsertTOCListTabRun(PT_BlockOffset blockOffset);
	bool					_doInsertImageRun(PT_BlockOffset blockOffset, FG_Graphic* pFG, pf_Frag_Object* oh);
	bool					_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	bool					_doInsertFieldTOCRun(PT_BlockOffset blockOffset);
	bool                    _doInsertDirectionMarkerRun(PT_BlockOffset blockOffset, UT_UCS4Char iM);
	bool					_deleteFmtMark(PT_BlockOffset blockOffset);

	virtual void			_lookupProperties(const PP_AttrProp* pAP);
	virtual void			_lookupMarginProperties(const PP_AttrProp* pAP);
	void					_removeLine(fp_Line*, bool bRemoveFromContainer, bool bReCalc);
	void                    _purgeLine(fp_Line*);
	void					_removeAllEmptyLines(void);

	bool					_checkMultiWord(UT_sint32 iStart,
											UT_sint32 eor,
											bool bToggleIP) const;

	UT_uint32				_getLastChar();
	void					_stuffAllRunsOnALine(void);
	void					_insertEndOfParagraphRun(void);
	void					_purgeEndOfParagraphRun(void);
	void					_breakLineAfterRun(fp_Run* /*pRun*/);

	static void 			_prefsListener(XAP_Prefs *pPrefs, UT_StringPtrMap * /*phChanges*/, void * data);

	void					_createListLabel(void);
	void					_deleteListLabel(void);
	inline void 			_addBlockToPrevList( fl_BlockLayout * prevBlockInList, UT_uint32 level);
	inline void 			_prependBlockToPrevList( fl_BlockLayout * nextBlockInList);
	UT_sint32 				m_iNeedsReformat; // will store offset
											  // from which reformat
											  // is need, -1 if not
	bool					m_bNeedsRedraw;
	bool				    m_bIsHdrFtr;

	FL_DocLayout*			m_pLayout;
	fb_LineBreaker 		    m_Breaker;

	fp_Run* 				m_pFirstRun;
	fl_SectionLayout*		m_pSectionLayout;

	UT_GenericVector<fl_TabStop*>	m_vecTabs;
	UT_sint32				m_iDefaultTabInterval;
	// read-only caches of the underlying properties
	UT_uint32				m_iOrphansProperty;
	UT_uint32				m_iWidowsProperty;
	UT_sint32				m_iTopMargin;
	UT_sint32				m_iBottomMargin;
	UT_sint32				m_iLeftMargin;
	UT_sint32				m_iRightMargin;
	UT_sint32				m_iTextIndent;
	fb_Alignment *			m_pAlignment;
	double					m_dLineSpacing;
	//bool					m_bExactSpacing;
	eSpacingPolicy			m_eSpacingPolicy;
	bool					m_bKeepTogether;
	bool					m_bKeepWithNext;

	bool                    m_bStartList;
	bool                    m_bStopList;
    bool                    m_bListLabelCreated;
#ifdef ENABLE_SPELL
	fl_SpellSquiggles *     m_pSpellSquiggles;
	fl_GrammarSquiggles *   m_pGrammarSquiggles;
	fl_BlockLayout          *m_nextToSpell;
	fl_BlockLayout          *m_prevToSpell;
#endif
	bool                    m_bListItem;
	const gchar *		m_szStyle;
	bool                    m_bIsCollapsed;
	bool                    m_bHasUpdatableField;

	UT_BidiCharType 		m_iDomDirection;
	UT_BidiCharType 		m_iDirOverride;

	bool                    m_bIsTOC;
	bool                    m_bStyleInTOC;
	UT_sint32               m_iTOCLevel;

	bool                    m_bSameYAsPrevious;
	UT_sint32               m_iAccumulatedHeight;
	fp_VerticalContainer *  m_pVertContainer;
	UT_sint32               m_iLinePosInContainer;
	bool                    m_bForceSectionBreak;
	bool                    m_bPrevListLabel;
    UT_sint32               m_iAdditionalMarginAfter;
	UT_RGBColor             m_ShadingForeColor;
	UT_RGBColor             m_ShadingBackColor;
	UT_sint32               m_iPattern;

	PP_PropertyMap::Line    m_lineBottom;
	PP_PropertyMap::Line    m_lineLeft;
	PP_PropertyMap::Line    m_lineRight;
	PP_PropertyMap::Line    m_lineTop;
	bool                    m_bCanMergeBordersWithNext;
	bool                    m_bHasBorders;
};

/*
	This class is used to represent a part of the block.  Pointers
	to this class are the things contained in m_vecSquiggles and in
	FL_DocLayout::m_pPendingWordForSpell
*/
class ABI_EXPORT fl_PartOfBlock
{
public:
	fl_PartOfBlock();
	fl_PartOfBlock(UT_sint32 iOffset, UT_sint32 iPTLength,
				   bool bIsIgnored = false);

	bool             doesTouch(UT_sint32 iOffset, UT_sint32 iLength) const;
	inline UT_sint32 getOffset(void) const { return m_iOffset; }
	inline UT_sint32 getPTLength(void)const{ return m_iPTLength; }
	inline bool 	 getIsIgnored(void) const { return m_bIsIgnored; }

	inline void 	 setOffset(UT_sint32 iOffset) { m_iOffset = iOffset; }
	inline void 	 setPTLength(UT_sint32 iLength) { m_iPTLength = iLength; }
	inline void 	 setIsIgnored(bool bIsIgnored) { m_bIsIgnored = bIsIgnored; }
	void             setInvisible(void)
	{m_bIsInvisible = true;}
	bool             isInvisible(void)
	{ return m_bIsInvisible;}
	void             setGrammarMessage(UT_UTF8String & sMsg);
	void             getGrammarMessage(UT_UTF8String & sMsg) const;

private:
	UT_sint32	m_iOffset;
	UT_sint32   m_iPTLength;

	bool		m_bIsIgnored;
	bool        m_bIsInvisible;
	UT_UTF8String  m_sGrammarMessage;
};

class ABI_EXPORT fl_TabStop
{
public:

	fl_TabStop();

	UT_sint32		getPosition() const { return iPosition;}
	void			setPosition(UT_sint32 value) { iPosition = value;}
	eTabType		getType() { return iType;}
	void			setType(eTabType type) { iType = type;}
	eTabLeader		getLeader() { return iLeader;};
	void			setLeader(eTabLeader leader) { iLeader = leader;}
	UT_uint32		getOffset() { return iOffset;}
	void			setOffset(UT_uint32 value) { iOffset = value;}

	fl_TabStop& operator = (const fl_TabStop &Other)
		{
			iPosition = Other.iPosition;
			iType = Other.iType;
			iLeader = Other.iLeader;
			iOffset = Other.iOffset;
			return *this;
		}

protected:

	UT_sint32		iPosition;
	eTabType		iType;
	eTabLeader		iLeader;
	UT_uint32		iOffset;
};

#ifdef ENABLE_SPELL
class ABI_EXPORT fl_BlockSpellIterator
{
	friend class fl_BlockLayout;

	UT_GrowBuf*     m_pgb;

	const fl_BlockLayout* m_pBL;

	UT_sint32       m_iWordOffset;
	UT_sint32       m_iWordLength;

	UT_sint32       m_iStartIndex;
	UT_sint32       m_iPrevStartIndex;
	UT_UCSChar*     m_pText;
	UT_sint32       m_iLength;

	UT_UCSChar*     m_pMutatedString;

	UT_sint32       m_iSentenceStart;
	UT_sint32       m_iSentenceEnd;

    bool            _ignoreFirstWordCharacter(const UT_UCSChar c) const;
    bool            _ignoreLastWordCharacter(const UT_UCSChar c) const;

public:
	fl_BlockSpellIterator(const fl_BlockLayout* pBL, UT_sint32 iPos = 0);
	~fl_BlockSpellIterator();

	bool            nextWordForSpellChecking(const UT_UCSChar*& pWord,
											 UT_sint32& iLength,
											 UT_sint32& iBlockPos,
											 UT_sint32& iPTLength);
	void              updateBlock(void);
	void              updateSentenceBoundaries(void);

	UT_sint32         getBlockLength(void) const;

	void              revertToPreviousWord(void);

    const UT_UCSChar* getCurrentWord(UT_sint32& iLength) const;
    const UT_UCSChar* getPreWord(UT_sint32& iLength) const;
    const UT_UCSChar* getPostWord(UT_sint32& iLength) const;
};
#endif

#endif /* FL_BLOCKLAYOUT_H */
