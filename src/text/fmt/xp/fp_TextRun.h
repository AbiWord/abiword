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


#ifndef FP_TEXTRUN_H
#define FP_TEXTRUN_H

#include "fp_Run.h"

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"

class fp_TextRun : public fp_Run
{
 public:
	fp_TextRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	virtual void			lookupProperties(void);
	virtual int				split(fp_RunSplitInfo&);
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			alwaysFits(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths);
	
	void					drawSquiggle(UT_uint32, UT_uint32);
	
	UT_Bool					splitSimple(UT_uint32 splitOffset);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(void);
	
	void					_drawDecors(UT_sint32, UT_sint32);
	void					_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right);
	void 					_getPartRect(UT_Rect* pRect, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
										 const UT_GrowBuf * pgbCharWidths);
	void					_drawPart(UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);
	void					_drawPartWithBackground(UT_RGBColor& clr, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);
	void 					_calcWidths(UT_GrowBuf * pgbCharWidths);

	enum
	{
		TEXT_DECOR_UNDERLINE = 		0x01,
		TEXT_DECOR_OVERLINE = 		0x02,
		TEXT_DECOR_LINETHROUGH = 	0x04
	};
	unsigned char			m_fDecorations;
	UT_sint32				m_iLineWidth;

	/*
	  This makes the assumption that all characters in a given run
	  can be obtained from the same font.  This may not be true.
	  For example, suppose that a run includes a mixture of latin
	  and non-latin characters.  The resulting glyphs will probably need to be
	  retrieved from multiple fonts.
	  TODO fix this issue
	*/
	GR_Font*				m_pFont;
	UT_RGBColor				m_colorFG;
};

#endif /* FP_TEXTRUN_H */
