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



#ifndef BLOCKLAYOUT_H
#define BLOCKLAYOUT_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "fl_Layout.h"

// number of DocPositions occupied by the block strux
#define fl_BLOCK_STRUX_OFFSET	1

// TODO the following should be an enum
#define FL_ALIGN_BLOCK_LEFT		1
#define FL_ALIGN_BLOCK_RIGHT    2
#define FL_ALIGN_BLOCK_CENTER   3
#define FL_ALIGN_BLOCK_JUSTIFY  4

class FL_DocLayout;
class fl_SectionLayout;
class fb_LineBreaker;
class fp_Line;
class fp_Run;
class GR_Graphics;
class PD_Document;
class PP_Property;
class PX_ChangeRecord_Object;
class PX_ChangeRecord_ObjectChange;
class PX_ChangeRecord_Span;
class PX_ChangeRecord_SpanChange;
class PX_ChangeRecord_Strux;
class PX_ChangeRecord_StruxChange;

/*
	This struct is used to represent a part of the block.  Pointers
	to this struct are the things contained in m_vecSquiggles and in 
	FL_DocLayout::m_pPendingWord.
*/
struct fl_PartOfBlock
{
	fl_PartOfBlock();

	UT_uint32	iOffset;
	UT_uint32	iLength;
};

class fl_BlockLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	fl_BlockLayout(PL_StruxDocHandle sdh, fb_LineBreaker*, fl_BlockLayout*, fl_SectionLayout*, PT_AttrPropIndex indexAP);
	~fl_BlockLayout();

	int 		format();
	UT_Bool		recalculateFields(void);
	
	fp_Line*	getNewLine(UT_sint32 iHeight);

	const char*	getProperty(const XML_Char * pszName) const;
	void setAlignment(UT_uint32 iAlignCmd);

	/*
		Blocks are stored in a linked list which contains all of the blocks in
		the normal flow, in order.
	*/

	fl_BlockLayout* getNext(UT_Bool bKeepGoing) const;
	fl_BlockLayout* getPrev(UT_Bool bKeepGoing) const;
	fp_Line* getFirstLine();
	fp_Line* getLastLine();
	fp_Line* findPrevLineInDocument(fp_Line*);
	fp_Line* findNextLineInDocument(fp_Line*);
	fp_Run* getFirstRun();
	void findSquigglesForRun(fp_Run* pRun);
	UT_uint32 canSlurp(fp_Line* pLine) const;

	UT_GrowBuf * getCharWidths(void);

	PT_DocPosition getPosition(UT_Bool bActualBlockPos=UT_FALSE) const;
	fp_Run* findPointCoords(PT_DocPosition position, UT_Bool bEOL, UT_sint32& x, UT_sint32& y, UT_sint32& height);

	UT_Bool getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	UT_Bool	getBlockBuf(UT_GrowBuf * pgb) const;

	UT_Bool truncateLayout(fp_Run* pTruncRun);

	void clearScreen(GR_Graphics*);

	void dump();

	inline UT_sint32	getTextIndent(void) const { return m_iTextIndent; }
	inline UT_sint32	getLeftMargin(void) const { return m_iLeftMargin; }
	inline UT_sint32	getRightMargin(void) const { return m_iRightMargin; }
	inline UT_sint32	getTopMargin(void) const { return m_iTopMargin; }
	inline UT_sint32	getBottomMargin(void) const { return m_iBottomMargin; }
	inline UT_uint32	getAlignment(void) const { return m_iAlignment; }
	inline FL_DocLayout* 		getDocLayout(void) const { return m_pLayout; }
	inline fl_SectionLayout* 	getSectionLayout(void) { return m_pSectionLayout; }

	void getLineSpacing(double& dSpacing, UT_Bool& bExact) const;

	UT_uint32 getOrphansProperty(void) const;
	UT_uint32 getWidowsProperty(void) const;
	void checkForWidowsAndOrphans(void);

	void checkSpelling(void);
	UT_Bool	findNextTabStop(UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition, unsigned char& iType);

	UT_Bool doclistener_populateSpan(const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len);
	UT_Bool doclistener_populateObject(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	
	UT_Bool doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs);
	UT_Bool doclistener_deleteSpan(const PX_ChangeRecord_Span * pcrs);
	UT_Bool doclistener_changeSpan(const PX_ChangeRecord_SpanChange * pcrsc);
	UT_Bool doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	UT_Bool doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	UT_Bool doclistener_insertStrux(const PX_ChangeRecord_Strux * pcrx,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));
	UT_Bool doclistener_insertObject(const PX_ChangeRecord_Object * pcro);
	UT_Bool doclistener_deleteObject(const PX_ChangeRecord_Object * pcro);
	UT_Bool doclistener_changeObject(const PX_ChangeRecord_ObjectChange * pcroc);
	
	void					purgeLayout(void);
	void					collapse(void);
	void					coalesceRuns(void);
	
protected:

#ifndef NDEBUG
	void					_assertRunListIntegrity(void);
#endif
	
	void 					_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge);
	
	UT_Bool					_doInsertRun(fp_Run* pNewRun);
	UT_Bool					_delete(PT_BlockOffset blockOffset, UT_uint32 len);

	UT_Bool					_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len);
	UT_Bool					_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset);
	UT_Bool					_doInsertForcedColumnBreakRun(PT_BlockOffset blockOffset);
	UT_Bool					_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset);
	UT_Bool					_doInsertTabRun(PT_BlockOffset blockOffset);
	UT_Bool					_doInsertImageRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	UT_Bool					_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro);
	
	void					_lookupProperties(void);
	void			 		_fixColumns(void);
	void					_removeLine(fp_Line*);
	void					_removeAllEmptyLines(void);

	void					_purgeSquiggles(void);
	void					_addPartNotSpellChecked(UT_uint32 iOffset, UT_uint32 iLen);
	void					_addSquiggle(UT_uint32 iOffset, UT_uint32 iLen);
	void					_updateSquiggle(fl_PartOfBlock* pPOB);

	UT_uint32				_getLastChar();

	UT_GrowBuf				m_gbCharWidths;

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

	// read-only caches of the underlying properties
	UT_uint32				m_iOrphansProperty;
	UT_uint32				m_iWidowsProperty;
	UT_sint32				m_iTopMargin;
	UT_sint32				m_iBottomMargin;
	UT_sint32				m_iLeftMargin;
	UT_sint32				m_iRightMargin;
	UT_sint32				m_iTextIndent;
	UT_uint32				m_iAlignment;
	double					m_dLineSpacing;
	UT_Bool					m_bExactSpacing;

	// spell check stuff
	UT_Vector				m_vecSquiggles;
};

#define FL_TAB_LEFT				1
#define FL_TAB_RIGHT			2
#define FL_TAB_CENTER			3

struct fl_TabStop
{
	fl_TabStop();
	
	UT_sint32		iPosition;
	unsigned char	iType;
};

#endif /* BLOCKLAYOUT_H */
