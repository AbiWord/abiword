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


#ifndef FP_TEXTRUN_H
#define FP_TEXTRUN_H

#include "fp_Run.h"

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"

/*
	fp_TextRun represents a run of contiguous text sharing the same 
	properties.  
*/
#ifdef BIDI_ENABLED
#define MAX_SPAN_LEN 250   //initial size for m_pSpanBuff, realocated if needed
#include "ut_timer.h"
#endif

class ABI_EXPORT fp_TextRun : public fp_Run
{
public:
	fp_TextRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, bool bLookupProperties=true);
	virtual ~fp_TextRun();

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			alwaysFits(void) const;
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	virtual UT_sint32		findTrailingSpaceDistance(void) const;
	virtual UT_sint32		findTrailingSpaceDistanceInLayoutUnits(void) const;
	UT_uint32               countTrailingSpaces(void) const;
	void					drawSquiggle(UT_uint32, UT_uint32);
	
	bool					split(UT_uint32 iSplitOffset);

	virtual bool			hasLayoutProperties(void) const;
	virtual void			fetchCharWidths(fl_CharWidths * pgbCharWidths);
	virtual bool			recalcWidth(void);
	virtual bool            canContainPoint(void) const;
	bool					canMergeWithNext(void);
	void					mergeWithNext(void);

	enum
	{
		Width_type_display,
		Width_type_layout_units
	};
	enum
	{
		Calculate_full_width = -1
	};
	UT_sint32				simpleRecalcWidth(UT_sint32 iWidthType, UT_sint32 iLength = Calculate_full_width)
#ifndef BIDI_ENABLED
// the BIDI version of simpleRecalcWidth can modify m_pSpanBuff
// this is ugly but necessary
							const
#endif
							;

	void					resetJustification();
	void					distributeJustificationAmongstSpaces(UT_sint32 iAmount, UT_uint32 iSpacesInRun);
	UT_uint32				countJustificationPoints() const;

	bool					getCharacter(UT_uint32 run_offset, UT_UCSChar &Character) const;
	UT_sint32				findCharacter(UT_uint32 startPosition, UT_UCSChar Character) const;
	bool					isFirstCharacter(UT_UCSChar Character) const;
	bool					isLastCharacter(UT_UCSChar Character) const;
	virtual bool			doesContainNonBlankData(void) const;
	inline virtual bool	isSuperscript(void) const;
	inline virtual bool	isSubscript(void) const;
	GR_Font*				getFont(void) const
		{ return m_pScreenFont; }
	UT_RGBColor				getFGColor(void) const
		{ return m_colorFG; }

	const XML_Char * 			getLanguage() const
		{ return m_pLanguage; }

#ifdef BIDI_ENABLED
	UT_sint32               getStr(UT_UCSChar * str, UT_uint32 &iMax);
	//bool                 setUnicodeDirection();
    void					setDirection(FriBidiCharType dir);
    static bool				getUseContextGlyphs(){return s_bUseContextGlyphs;};
    // the usability of the following function is *very* limited, see the note in cpp file
    void					setDirOverride(FriBidiCharType dir);
    virtual FriBidiCharType	getDirection() const{return m_iDirOverride == FRIBIDI_TYPE_UNSET ? m_iDirection : m_iDirOverride;}

	/* needed for handling BiDi text, static because we need only one buffer
	   for all the instances, public so that we could inicialised them in the cpp file outside of the
	   constructor in order that the constructor can decide whether it is creating the first instance
	   or not*/
	UT_UCSChar * 		m_pSpanBuff;
	UT_uint32		    m_iSpanBuffSize;
	static UT_uint32    s_iClassInstanceCount;
	FriBidiCharType		m_iDirOverride;
	static bool			s_bUseContextGlyphs;
	static bool			s_bSaveContextGlyphs;
	static UT_Timer *	s_pPrefsTimer;
	static bool			s_bBidiOS;
private:
	static void			_refreshPrefs(UT_Worker * pTimer);
	void				_getContext(const UT_UCSChar *pSpan,
									UT_uint32 lenSpan,
									UT_uint32 len,
									UT_uint32 offset,
									UT_UCSChar * prev,
									UT_UCSChar * next) const;
	UT_UCSChar			_getContextGlyph(const UT_UCSChar * pSpan,
										 UT_uint32 len,
										 UT_uint32 offset,
										 UT_UCSChar *prev,
										 UT_UCSChar *next) const;
	//fp_Run * 			_getOldNext()const{return m_pOldNext;};
	void				_refreshDrawBuffer();
#endif

#ifdef FMT_TEST
public:
	virtual void			__dump(FILE * fp) const;
#endif	
	
protected:
	void					_fetchCharWidths(GR_Font* pFont, UT_uint16* pCharWidths);
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(bool bFullLineHeightRect);

    void                    _drawInvisibleSpaces(UT_sint32, UT_sint32);
    void                    _drawInvisibles(UT_sint32, UT_sint32);
	void					_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right);

	void 					_getPartRect(UT_Rect* pRect,
										 UT_sint32 xoff,
										 UT_sint32 yoff,
										 UT_uint32 iStart,
										 UT_uint32 iLen,
										 const UT_GrowBuf * pgbCharWidths);
	
	void					_drawPart(UT_sint32 xoff,
									  UT_sint32 yoff,
									  UT_uint32 iStart,
									  UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);

	void					_drawLastChar(UT_sint32 xoff, UT_sint32 yoff,const UT_GrowBuf * pgbCharWidths);
	void					_drawFirstChar(UT_sint32 xoff, UT_sint32 yoff);
	
	void					_fillRect(UT_RGBColor& clr,
									  UT_sint32 xoff,
									  UT_sint32 yoff,
									  UT_uint32 iStart,
									  UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);


	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte				m_fPosition;
	
	/*
	  This makes the assumption that all characters in a given run
	  can be obtained from the same font.  This may not be true.
	  For example, suppose that a run includes a mixture of latin
	  and non-latin characters.  The resulting glyphs will probably need to be
	  retrieved from multiple fonts.
	  TODO fix this issue
	*/
	//GR_Font*				m_pFont;
	//GR_Font*				m_pFontLayout;
	UT_RGBColor				m_colorFG;
	bool					m_bSquiggled;

	enum
	{
		JUSTIFICATION_NOT_USED = -1
	};
	UT_sint32				m_iSpaceWidthBeforeJustification;

	// !!! the m_pLanguage member cannot be set to an arbitrary string pointer
	// but only a pointer in the static table of the UT_Language class !!!
	const XML_Char *			m_pLanguage;

#if 0	
#ifdef BIDI_ENABLED
	fp_Run* m_pOldPrev;
	fp_Run* m_pOldNext;
#endif
	UT_uint32 m_iOldLen;
	GR_Font *m_pOldScreenFont;
	UT_sint32 m_iOldSpaceWidthBeforeJustification;
#endif

};

#endif /* FP_TEXTRUN_H */
