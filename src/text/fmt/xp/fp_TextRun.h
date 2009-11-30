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


#ifndef FP_TEXTRUN_H
#define FP_TEXTRUN_H

#include "fp_Run.h"

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"
#include "gr_RenderInfo.h"

#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
/*
	fp_TextRun represents a run of contiguous text sharing the same
	properties.
*/
#define MAX_SPAN_LEN 250   //initial size for m_pSpanBuff, realocated if needed
#include "ut_timer.h"

class ABI_EXPORT fp_TextRun : public fp_Run
{
public:
	fp_TextRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen, bool bLookupProperties=true);
	virtual ~fp_TextRun();

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC);
	virtual void			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			alwaysFits(void) const;
	virtual bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	virtual UT_sint32		findTrailingSpaceDistance(void) const;
#ifdef ENABLE_SPELL
	void				drawSquiggle(UT_uint32, UT_uint32,FL_SQUIGGLE_TYPE iSquiggle);
#endif
	bool				split(UT_uint32 iSplitOffset,UT_sint32 iLenSkip=0);
#if DEBUG
	virtual void            printText(void);
#endif
	void                    appendTextToBuf(UT_GrowBuf & buf) const;
	virtual bool			hasLayoutProperties(void) const;
	//virtual void			fetchCharWidths(fl_CharWidths * pgbCharWidths);
	bool					canMergeWithNext(void);
	void					mergeWithNext(void);
	bool                    findFirstNonBlankSplitPoint(fp_RunSplitInfo & splitInfo);
	bool                    isOneItem(fp_Run * pNext);
	enum
	{
		Calculate_full_width = -1
	};
	UT_sint32				simpleRecalcWidth(UT_sint32 iLength = Calculate_full_width);

	void					resetJustification(bool bPermanent);
	void					justify(UT_sint32 iAmount, UT_uint32 iSpacesInRun);
	UT_sint32				countJustificationPoints(bool bLast) const;

	bool					getCharacter(UT_uint32 run_offset, UT_UCSChar &Character) const;
	UT_sint32				findCharacter(UT_uint32 startPosition, UT_UCSChar Character) const;
	bool					isFirstCharacter(UT_UCSChar Character) const;
	bool					isLastCharacter(UT_UCSChar Character) const;
	virtual bool			doesContainNonBlankData(void) const;
	inline virtual bool isSuperscript(void) const;
	inline virtual bool isSubscript(void) const;
	const GR_Font*				getFont(void) const
		{ return _getFont(); }
	const gchar *			getLanguage() const;


	UT_sint32				getStr(UT_UCSChar * str, UT_uint32 &iMax);

	// applies provided values of direction and override to the run
	void					setDirection(UT_BidiCharType dir, UT_BidiCharType override);

	// the usability of the following function is *very* limited, see the note in cpp file
	void					setDirOverride(UT_BidiCharType dir);
	virtual UT_BidiCharType getDirection() const;

	UT_BidiCharType 		getDirOverride() const { return m_iDirOverride; }

	void				breakNeighborsAtDirBoundaries();
	void				breakMeAtDirBoundaries(UT_BidiCharType iNewOverride);
	void                setShapingRequired(GRShapingResult eR)
	                         {m_pRenderInfo->m_eShapingResult = eR;}
	void                orShapingRequired(GRShapingResult eR)
	                      {
							m_pRenderInfo->m_eShapingResult =
								(GRShapingResult)((UT_uint32)m_pRenderInfo->m_eShapingResult
												  | (UT_uint32)eR);
	                      }

	void                itemize(void);
	void                setItem(GR_Item * i);
	const GR_Item *           getItem() const {return m_pItem;}
	

	virtual void        updateOnDelete(UT_uint32 offset, UT_uint32 iLen);

	virtual UT_uint32   adjustCaretPosition(UT_uint32 iDocumentPosition, bool bForward);
	virtual void        adjustDeletePosition(UT_uint32 &pos1, UT_uint32 &count);
	
	static UT_uint32	s_iClassInstanceCount;
	UT_BidiCharType 	m_iDirOverride;
	static bool 		s_bBidiOS;

	void                measureCharWidths();

	GR_ShapingInfo::TextTransform getTextTransform() const { return m_TextTransform;}
	void setTextTransform(GR_ShapingInfo::TextTransform transform) { m_TextTransform = transform; }
	
private:
	GR_ShapingInfo::TextTransform m_TextTransform;

	bool				_refreshDrawBuffer();
	bool				_addupCharWidths(void);
	virtual void        _lookupProperties(const PP_AttrProp * pSpanAP,
										   const PP_AttrProp * pBlockAP,
										   const PP_AttrProp * pSectionAP,
										  GR_Graphics * pG);

#ifdef FMT_TEST
public:
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	virtual bool			_recalcWidth(void);
	virtual bool			_canContainPoint(void) const;
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect = true);

	void					_drawInvisibleSpaces(UT_sint32, UT_sint32);
	void					_drawInvisibles(UT_sint32, UT_sint32);
#ifdef ENABLE_SPELL
	void					_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right,FL_SQUIGGLE_TYPE iSquiggle);
#endif
	void					_getPartRect(UT_Rect* pRect,
										 UT_sint32 xoff,
										 UT_sint32 yoff,
										 UT_uint32 iStart,
										 UT_uint32 iLen);

	void					_drawLastChar(bool bSelection);
	
	void					_drawFirstChar(bool bSelection);

	void					_fillRect(UT_RGBColor& clr,
									  UT_sint32 xoff,
									  UT_sint32 yoff,
									  UT_uint32 iStart,
									  UT_uint32 iLen,
									  UT_Rect & rect,
									  GR_Graphics * pG);

private:
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte 			m_fPosition;

	/*
	  This makes the assumption that all characters in a given run
	  can be obtained from the same font.  This may not be true.
	  For example, suppose that a run includes a mixture of latin
	  and non-latin characters.  The resulting glyphs will probably need to be
	  retrieved from multiple fonts.
	  TODO fix this issue
	*/
	bool					m_bSpellSquiggled;
	bool					m_bGrammarSquiggled;

	// !!! the m_pLanguage member cannot be set to an arbitrary string pointer
	// but only a pointer in the static table of the UT_Language class !!!
	const gchar *		m_pLanguage;
	bool					m_bIsOverhanging;

	bool                    m_bKeepWidths;

	const GR_Item *               m_pItem;
	GR_RenderInfo   *       m_pRenderInfo;
};

#endif /* FP_TEXTRUN_H */
