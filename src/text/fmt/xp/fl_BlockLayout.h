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

#ifndef FL_BLOCKLAYOUT_H
#define FL_BLOCKLAYOUT_H

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

// number of DocPositions occupied by the block strux
#define fl_BLOCK_STRUX_OFFSET	1

class FL_DocLayout;
class fl_SectionLayout;
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
class fl_PartOfBlock;
class fl_AutoNum;

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


class fl_CharWidths
{
public:
	fl_CharWidths()	: m_gbCharWidths(256), m_gbCharWidthsLayoutUnits(256)
		{
		}

private:

	UT_GrowBuf m_gbCharWidths;
	UT_GrowBuf m_gbCharWidthsLayoutUnits;

public:

	bool ins(UT_uint32 position, UT_uint32 length)
		{
			m_gbCharWidths.ins(position, length);
			return m_gbCharWidthsLayoutUnits.ins(position, length);
		}

	bool del(UT_uint32 position, UT_uint32 amount)
		{
			m_gbCharWidths.del(position, amount);
			return m_gbCharWidthsLayoutUnits.del(position, amount);
		}
	UT_uint32 getLength(void) const
		{
			UT_ASSERT(m_gbCharWidths.getLength() == m_gbCharWidthsLayoutUnits.getLength());
			return m_gbCharWidths.getLength();
		}
	bool ins(UT_uint32 position, const fl_CharWidths &Other, UT_uint32 offset, UT_uint32 length)
		{
			m_gbCharWidths.ins(position, Other.m_gbCharWidths.getPointer(offset), length);
			return m_gbCharWidthsLayoutUnits.ins(position, Other.m_gbCharWidthsLayoutUnits.getPointer(offset), length);
		}
	void truncate(UT_uint32 position)
		{
			m_gbCharWidths.truncate(position);
			m_gbCharWidthsLayoutUnits.truncate(position);
		}

	UT_GrowBuf *getCharWidths()
		{
			return &m_gbCharWidths;
		}
	UT_GrowBuf *getCharWidthsLayoutUnits()
		{
			return &m_gbCharWidthsLayoutUnits;
		}


};


/*
	Blocks are stored in a linked list which contains all of the blocks in
	the normal flow, in order.
*/

class fl_TabStop;
void buildTabStops(GR_Graphics * pG, const char* pszTabStops, UT_Vector &m_vecTabs);

class fl_BlockLayout : public fl_Layout
{
	friend class fl_DocListener;

	// TODO: shack - code should be moved from toggleAuto to a function in
	// here - to handle the squiggles
	friend void FL_DocLayout::_toggleAutoSpell(bool bSpell);

public:
	fl_BlockLayout(PL_StruxDocHandle sdh, fb_LineBreaker*, 
				   fl_BlockLayout*, fl_SectionLayout*, 
				   PT_AttrPropIndex indexAP, bool bIsHdrFtr);
	fl_BlockLayout(PL_StruxDocHandle sdh, fb_LineBreaker*, 
				   fl_BlockLayout*, fl_SectionLayout*, 
				   PT_AttrPropIndex indexAP);
	~fl_BlockLayout();

	typedef enum _eSpacingPolicy
	{
		spacing_MULTIPLE,
		spacing_EXACT,
		spacing_ATLEAST
	} eSpacingPolicy;

	int 		format();
	bool		recalculateFields(void);
	
	void		redrawUpdate();

	fp_Line*	getNewLine(void);
	FV_View *       getView(void);

	const char*	getProperty(const XML_Char * pszName, bool bExpandStyles = true) const;
	void setAlignment(UT_uint32 iAlignCmd);

	inline fl_BlockLayout* getNext(void) const { return m_pNext; }
	inline fl_BlockLayout* getPrev(void) const { return m_pPrev; }

	void setNext(fl_BlockLayout*);
	void setPrev(fl_BlockLayout*);

	fl_BlockLayout* getNextBlockInDocument(void) const;
	fl_BlockLayout* getPrevBlockInDocument(void) const;
	
	inline fp_Line* getFirstLine(void) const { return m_pFirstLine; }
	inline fp_Line* getLastLine(void) const { return m_pLastLine; }

	fp_Line* findPrevLineInDocument(fp_Line*);
	fp_Line* findNextLineInDocument(fp_Line*);

	inline fp_Run* getFirstRun(void) const { return m_pFirstRun; }

	inline bool isListItem(void) const { return m_bListItem; }
	bool isFirstInList(void);
//	inline fl_AutoNum * getAutoNum(void) const { return m_pAutoNum; }
	void    getListAttributesVector( UT_Vector * va);
	void  getListPropertyVector( UT_Vector * vp);

	char *  getFormatFromListType(List_Type iListType);
	void remItemFromList(void);
	virtual void listUpdate(void);
	void resumeList( fl_BlockLayout * prevList);
	void prependList( fl_BlockLayout * nextList);
	List_Type decodeListType(char * listformat);
	List_Type getListType(void);
	XML_Char* getListStyleString( List_Type iListType);
	List_Type getListTypeFromStyle( const XML_Char * style);
	fl_BlockLayout * getNextList(UT_uint32 id);
	bool isListLabelInBlock(void);
	void StartList( const XML_Char * style);

	void StartList( List_Type lType, UT_uint32 start,const XML_Char * lDelim, const XML_Char * lDecimal, const XML_Char * fFont, float Align, float indent, UT_uint32 iParentID = 0, UT_uint32 level=0 );

	void StopList(void);
	void deleteListLabel(void);
	UT_UCSChar * getListLabel(void);
	void transferListFlags(void);
	UT_uint32 getLevel(void);
	void setStarting( bool bValue);
	void setStopping( bool bValue);
	fl_BlockLayout * getPreviousList(UT_uint32 id);
	fl_BlockLayout * getPreviousList(void);
	inline fl_BlockLayout * getParentItem(void);
	
	void findSquigglesForRun(fp_Run* pRun);
	UT_uint32 canSlurp(fp_Line* pLine) const;

	fl_CharWidths * getCharWidths(void);

	PT_DocPosition getPosition(bool bActualBlockPos=false) const;
	fp_Run* findPointCoords(PT_DocPosition position, bool bEOL, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);

	bool getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	bool	getBlockBuf(UT_GrowBuf * pgb) const;

	void clearScreen(GR_Graphics*);

	inline UT_sint32	getTextIndent(void) const { return m_iTextIndent; }
	inline UT_sint32	getTextIndentInLayoutUnits(void) const { return m_iTextIndentLayoutUnits; }
	inline UT_sint32	getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32	getLeftMarginInLayoutUnits(void) const { return m_iLeftMarginLayoutUnits; }
	inline UT_sint32	getRightMargin(void) const { return m_iRightMargin; }
	inline UT_sint32	getRightMarginInLayoutUnits(void) const { return m_iRightMarginLayoutUnits; }
	inline UT_sint32	getTopMargin(void) const { return m_iTopMargin; }
	inline UT_sint32	getTopMarginInLayoutUnits(void) const { return m_iTopMarginLayoutUnits; }
	inline UT_sint32	getBottomMargin(void) const { return m_iBottomMargin; }
	inline UT_sint32	getBottomMarginInLayoutUnits(void) const { return m_iBottomMarginLayoutUnits; }
	inline fb_Alignment *		getAlignment(void) const { return m_pAlignment; }
	inline FL_DocLayout* 		getDocLayout(void) const { return m_pLayout; }
	inline fl_SectionLayout* 	getSectionLayout(void) { return m_pSectionLayout; }
	fl_DocSectionLayout * getDocSectionLayout(void);

	void setSectionLayout(fl_SectionLayout* pSectionLayout);

	void getLineSpacing(double& dSpacing, double &dSpacingLayout, eSpacingPolicy& eSpacing) const;
						
	void updateBackgroundColor(void);

	inline UT_uint32 getProp_Orphans(void) const { return m_iOrphansProperty; }
	inline UT_uint32 getProp_Widows(void) const { return m_iWidowsProperty; }
	inline bool getProp_KeepTogether(void) const { return m_bKeepTogether; }
	inline bool getProp_KeepWithNext(void) const { return m_bKeepWithNext; }
#ifdef BIDI_ENABLED
	inline bool getDominantDirection(void) const { return m_bDomDirection; }
	void setDominantDirection(bool bDirection);
#endif

	bool isHdrFtr(void) { return m_bIsHdrFtr;}
	void setHdrFtr(void) { m_bIsHdrFtr = true;}
	void clearHdrFtr(void) { m_bIsHdrFtr = false;}

	void checkSpelling(void);
	void debugFlashing(void);
 	bool	findNextTabStop(UT_sint32 iStartX, UT_sint32 iMaxX,
							UT_sint32& iPosition, eTabType& iType, 
							eTabLeader &iLeader );
 	bool	findNextTabStopInLayoutUnits(UT_sint32 iStartX, UT_sint32 iMaxX,
										 UT_sint32& iPosition, 
										 eTabType& iType, 
										 eTabLeader &iLeader);

	inline UT_sint32 getDefaultTabInterval(void) const { return m_iDefaultTabInterval; }
	inline UT_sint32 getTabsCount(void) const { return (UT_sint32) m_vecTabs.getItemCount(); }

	bool doclistener_populateSpan(const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	bool doclistener_populateObject(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	
	bool doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs);
	bool doclistener_deleteSpan(const PX_ChangeRecord_Span * pcrs);
	bool doclistener_changeSpan(const PX_ChangeRecord_SpanChange * pcrsc);
	bool doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	bool doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	bool doclistener_insertFirstBlock(const PX_ChangeRecord_Strux * pcrx,
									  PL_StruxDocHandle sdh,
									  PL_ListenerId lid,
									  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															  PL_ListenerId lid,
															  PL_StruxFmtHandle sfhNew));
	bool doclistener_insertBlock(const PX_ChangeRecord_Strux * pcrx,
								 PL_StruxDocHandle sdh,
								 PL_ListenerId lid,
								 void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
														 PL_ListenerId lid,
														 PL_StruxFmtHandle sfhNew));
	bool doclistener_insertSection(const PX_ChangeRecord_Strux * pcrx,
								   PL_StruxDocHandle sdh,
								   PL_ListenerId lid,
								   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
														   PL_ListenerId lid,
														   PL_StruxFmtHandle sfhNew));
	bool doclistener_insertHdrFtrSection(const PX_ChangeRecord_Strux * pcrx,
								   PL_StruxDocHandle sdh,
								   PL_ListenerId lid,
								   void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
														   PL_ListenerId lid,
														   PL_StruxFmtHandle sfhNew));
	bool doclistener_insertObject(const PX_ChangeRecord_Object * pcro);
	bool doclistener_deleteObject(const PX_ChangeRecord_Object * pcro);
	bool doclistener_changeObject(const PX_ChangeRecord_ObjectChange * pcroc);

	bool doclistener_insertFmtMark(const PX_ChangeRecord_FmtMark * pcrfm);
	bool doclistener_deleteFmtMark(const PX_ChangeRecord_FmtMark * pcrfm);
	bool doclistener_changeFmtMark(const PX_ChangeRecord_FmtMarkChange * pcrfmc);
	
	void					purgeLayout(void);
	void					collapse(void);
	void					coalesceRuns(void);

	void					setNeedsReformat(void) { m_bNeedsReformat = true; }
	inline bool			needsReformat(void) const { return m_bNeedsReformat; }

	void					setNeedsRedraw(void) { m_bNeedsRedraw = true; }
	inline bool			needsRedraw(void) const { return m_bNeedsRedraw; }

	bool					checkWord(fl_PartOfBlock* pPOB);
	fl_PartOfBlock*			getSquiggle(UT_uint32 iOffset) const;
	void					recheckIgnoredWords();

	static bool			s_EnumTabStops(void * myThis, UT_uint32 k, fl_TabStop *pTabInfo);
	
	inline void			addBackgroundCheckReason(UT_uint32 reason) {m_uBackgroundCheckReasons |= reason;}
	inline void			removeBackgroundCheckReason(UT_uint32 reason) {m_uBackgroundCheckReasons &= ~reason;}
	inline bool		hasBackgroundCheckReason(UT_uint32 reason) const {return ((m_uBackgroundCheckReasons & reason) ? true : false);}

	// The following is a set of bit flags giving the reason this block is
	// queued for background checking.  See specific values in fl_DocLayout.h
	UT_uint32				m_uBackgroundCheckReasons;

#ifdef FMT_TEST
	void					__dump(FILE * fp) const;
#endif

protected:

	bool                    _spellCheckWord(const UT_UCSChar * word, UT_uint32 len, UT_uint32 blockPos);

	bool					_truncateLayout(fp_Run* pTruncRun);

#ifndef NDEBUG
	void					_assertRunListIntegrityImpl(void);
#endif
	inline void				_assertRunListIntegrity(void);
	
	void 					_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge);
	
	bool					_doInsertRun(fp_Run* pNewRun);
	bool					_delete(PT_BlockOffset blockOffset, UT_uint32 len);

	bool					_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len);
	bool					_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset);
	bool					_doInsertFieldStartRun(PT_BlockOffset blockOffset);
	bool					_doInsertFieldEndRun(PT_BlockOffset blockOffset);
	bool					_doInsertForcedColumnBreakRun(PT_BlockOffset blockOffset);
	bool					_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset);
	bool					_doInsertTabRun(PT_BlockOffset blockOffset);
	bool					_doInsertImageRun(PT_BlockOffset blockOffset, FG_Graphic* pFG);
	bool					_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	bool					_deleteFmtMark(PT_BlockOffset blockOffset);
	
	void					_lookupProperties(void);
	void					_removeLine(fp_Line*);
	void					_removeAllEmptyLines(void);

	void					_purgeSquiggles(void);
	UT_sint32				_findSquiggle(UT_uint32 iOffset) const;
	void					_addSquiggle(UT_uint32 iOffset, UT_uint32 iLen, bool bIsIgnored = false);
	void					_updateSquiggle(fl_PartOfBlock* pPOB);
	void					_insertSquiggles(UT_uint32 iOffset,
											 UT_uint32 iLength);
	void					_breakSquiggles(UT_uint32 iOffset,
											fl_BlockLayout* pNewBL);
	void					_deleteSquiggles(UT_uint32 iOffset,
											 UT_uint32 iLength);
	void					_mergeSquiggles(UT_uint32 iOffset,
											fl_BlockLayout* pPrevBL);
	void					_moveSquiggles(UT_uint32 iOffset,
										   UT_sint32 chg,
										   fl_BlockLayout* pBlock=NULL);
	void					_recalcPendingWord(UT_uint32 iOffset, UT_sint32 chg);
	bool					_checkMultiWord(const UT_UCSChar* pBlockText,
											UT_uint32 iStart, 
											UT_uint32 eor,
											bool bToggleIP);

	UT_uint32				_getLastChar();
	void					_stuffAllRunsOnALine(void);
	void					_insertEndOfParagraphRun(void);
	void					_purgeEndOfParagraphRun(void);
	void					_breakLineAfterRun(fp_Run* /*pRun*/);

	static void				_prefsListener(XAP_App * /*pApp*/, XAP_Prefs *pPrefs, UT_AlphaHashTable * /*phChanges*/, void * data);

	void					_createListLabel(void);
	void					_deleteListLabel(void);
	inline void				_addBlockToPrevList( fl_BlockLayout * prevBlockInList, UT_uint32 level);	
	inline void				_prependBlockToPrevList( fl_BlockLayout * nextBlockInList);	

	bool					m_bNeedsReformat;
	bool					m_bNeedsRedraw;
	bool					m_bFixCharWidths;
	bool                                 m_bCursorErased;
	bool                                 m_bIsHdrFtr;
	fl_CharWidths			m_gbCharWidths;

	FL_DocLayout*	       	m_pLayout;
	fb_LineBreaker*			m_pBreaker;

	fl_BlockLayout*			m_pPrev;
	fl_BlockLayout*			m_pNext;

	fp_Run*					m_pFirstRun;
	fl_SectionLayout*		m_pSectionLayout;

	fp_Line*				m_pFirstLine;
	fp_Line*				m_pLastLine;

	UT_Vector				m_vecTabs;
	UT_sint32				m_iDefaultTabInterval;
	UT_sint32				m_iDefaultTabIntervalLayoutUnits;

	// read-only caches of the underlying properties
	UT_uint32				m_iOrphansProperty;
	UT_uint32				m_iWidowsProperty;
	UT_sint32				m_iTopMargin;
	UT_sint32				m_iTopMarginLayoutUnits;
	UT_sint32				m_iBottomMargin;
	UT_sint32				m_iBottomMarginLayoutUnits;
	UT_sint32				m_iLeftMargin;
	UT_sint32				m_iLeftMarginLayoutUnits;
	UT_sint32				m_iRightMargin;
	UT_sint32				m_iRightMarginLayoutUnits;
	UT_sint32				m_iTextIndent;
	UT_sint32				m_iTextIndentLayoutUnits;
	fb_Alignment *			m_pAlignment;
	double					m_dLineSpacing;
	double					m_dLineSpacingLayoutUnits;
	//bool					m_bExactSpacing;
	eSpacingPolicy			m_eSpacingPolicy;
	bool					m_bKeepTogether;
	bool					m_bKeepWithNext;
	const XML_Char *		m_szStyle;

	//fl_AutoNum *			m_pAutoNum;
	bool					m_bListItem;
	bool					m_bStartList;
	bool					m_bStopList;
	bool					m_bListLabelCreated;

	// spell check stuff
	UT_Vector				m_vecSquiggles;
#ifdef BIDI_ENABLED
	bool					m_bDomDirection;
#endif

};

/*
	This class is used to represent a part of the block.  Pointers
	to this class are the things contained in m_vecSquiggles and in
	FL_DocLayout::m_pPendingWordForSpell
*/
class fl_PartOfBlock
{
public:
	fl_PartOfBlock();

	bool doesTouch(UT_uint32 offset, UT_uint32 length) const;

	UT_uint32	iOffset;
	UT_uint32	iLength;

	bool		bIsIgnored;

protected:
};

class fl_TabStop
{
public:
	
	fl_TabStop();
	
	UT_sint32		getPosition() { return iPosition;}
	void			setPosition(UT_sint32 value) { iPosition = value;}
	UT_sint32		getPositionLayoutUnits() { return iPositionLayoutUnits;}
	void			setPositionLayoutUnits(UT_sint32 value) { iPositionLayoutUnits = value;}
	eTabType		getType() { return iType;}
	void			setType(eTabType type) { iType = type;}
	eTabLeader		getLeader() { return iLeader;};
	void			setLeader(eTabLeader leader) { iLeader = leader;}
	UT_uint32		getOffset() { return iOffset;}
	void			setOffset(UT_uint32 value) { iOffset = value;}

	void operator = (const fl_TabStop &Other)
		{
			iPosition = Other.iPosition;
			iPositionLayoutUnits = Other.iPositionLayoutUnits;
			iType = Other.iType;
			iLeader = Other.iLeader;
			iOffset = Other.iOffset;
		}

protected:

	UT_sint32		iPosition;
	UT_sint32		iPositionLayoutUnits;
	eTabType		iType;
	eTabLeader		iLeader;
	UT_uint32		iOffset;
};

#endif /* FL_BLOCKLAYOUT_H */
