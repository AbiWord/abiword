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


class fp_TextRun : public fp_Run
{
 public:
	fp_TextRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);
	virtual ~fp_TextRun();

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			alwaysFits(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_sint32		findTrailingSpaceDistance(void) const;
	virtual UT_sint32		findTrailingSpaceDistanceInLayoutUnits(void) const;
	void					drawSquiggle(UT_uint32, UT_uint32);
	
	UT_Bool					split(UT_uint32 iSplitOffset);

	virtual void			fetchCharWidths(fl_CharWidths * pgbCharWidths);
	virtual UT_Bool			recalcWidth(void);

	UT_Bool					canMergeWithNext(void);
	void					mergeWithNext(void);

	enum
	{
		Width_type_display,
		Width_type_layout_units
	};
	UT_sint32				simpleRecalcWidth(UT_sint32 iWidthType = Width_type_display) const;

	void					resetJustification();
	void					distributeJustificationAmongstSpaces(UT_sint32 iAmount, UT_uint32 iSpacesInRun);
	UT_uint32				countJustificationPoints() const;

	UT_Bool					getCharacter(UT_uint32 run_offset, UT_UCSChar &Character) const;
	UT_sint32				findCharacter(UT_uint32 startPosition, UT_UCSChar Character) const;
	UT_Bool					isFirstCharacter(UT_UCSChar Character) const;
	UT_Bool					isLastCharacter(UT_UCSChar Character) const;
	virtual UT_Bool			doesContainNonBlankData(void) const;
	virtual UT_Bool				isSuperscript(void) const;
	virtual UT_Bool				isSubscript(void) const;
	UT_uint32				countTrailingSpaces(void) const;

#ifdef FMT_TEST
	virtual void			__dump(FILE * fp) const;
#endif	
	
protected:
	void					_fetchCharWidths(GR_Font* pFont, UT_uint16* pCharWidths);
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);
	
	void					_drawDecors(UT_sint32, UT_sint32);
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

	void					_fillRect(UT_RGBColor& clr,
									  UT_sint32 xoff,
									  UT_sint32 yoff,
									  UT_uint32 iStart,
									  UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);

	enum
	{
		TEXT_DECOR_UNDERLINE = 		0x01,
		TEXT_DECOR_OVERLINE = 		0x10,
		TEXT_DECOR_LINETHROUGH = 	0x04
	};
	unsigned char			m_fDecorations;
	UT_sint32				m_iLineWidth;

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
	GR_Font*				m_pFont;
	GR_Font*				m_pFontLayout;
	UT_RGBColor				m_colorFG;
	UT_Bool					m_bSquiggled;

	enum
	{
		JUSTIFICATION_NOT_USED = -1
	};
	UT_sint32				m_iSpaceWidthBeforeJustification;

};

#endif /* FP_TEXTRUN_H */
