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



#ifndef RUN_H
#define RUN_H

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"

class UT_GrowBuf;
class fl_BlockLayout;
class fp_Line;
class DG_Graphics;
class DG_Font;
class PD_Document;
struct dg_DrawArgs;

/*
	Note that fp_Run encapsulates all text measurement issues, as well as
	any intercharacter spacing problems such as kerning, ligatures, justification,
	and so on.  We could probably even bury hyphenation in here without affecting the
	line breaker algorithm itself.
*/

/*
	TODO so do we need right-to-left runs?
*/

struct fp_RunSplitInfo
{
	UT_sint32 iLeftWidth;
	UT_sint32 iRightWidth;
	UT_sint32 iOffset;
};

#define BREAK_AUTO		0
#define BREAK_AVOID		1
#define BREAK_ALWAYS	2

#define FP_RUN_INSIDE      1
#define FP_RUN_JUSTAFTER   2
#define FP_RUN_NOT         3

class fp_Run
{
	friend class fl_DocListener;

 public:
	fp_Run(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	void					setLine(fp_Line*, void*);
	fp_Line*				getLine() const;
	void*                   getLineData();

	fl_BlockLayout*			getBlock() const;
	DG_Graphics*            getGraphics() const;
	UT_uint32				getHeight() const;
	UT_uint32				getWidth() const;
	UT_uint32				getAscent();
	UT_uint32				getDescent();
	UT_uint32				getLength() const;
	UT_uint32				getBlockOffset() const;
	void					lookupProperties(void);

	void					setNext(fp_Run*);
	void					setPrev(fp_Run*);
	fp_Run* 				getNext() const;
	fp_Run*					getPrev() const;

	UT_Bool					canSplit() const;
	UT_Bool					canBreakAfter() const;
	UT_Bool					canBreakBefore() const;
	UT_Bool					getLineBreakBefore() const; 
	UT_Bool					getLineBreakAfter() const;
	int						split(fp_RunSplitInfo&);
	UT_Bool					split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	UT_Bool					findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si);
	UT_Bool					findMinLeftFitSplitPoint(fp_RunSplitInfo& si);

	void 					calcWidths(UT_GrowBuf * pgbCharWidths);
	void            		expandWidthTo(UT_uint32);

	void					mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	void					getOffsets(UT_uint32& xoff, UT_uint32& yoff);
	UT_uint32 				containsOffset(UT_uint32 iOffset);
	void 					findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);

	UT_Bool 				ins(UT_uint32 iOffset, UT_uint32 iCount, PT_AttrPropIndex indexAP);
	UT_Bool 				del(UT_uint32 iOffset, UT_uint32 iCount);

	void                    clearScreen(void);
	void					draw(dg_DrawArgs*);
	void					invert(UT_uint32 iStart, UT_uint32 iLen);
	void					dumpRun(void) const;
	
 protected:
	void					_drawDecors(UT_sint32, UT_sint32);
	void 					_getPartRect(UT_Rect* pRect, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
										 const UT_GrowBuf * pgbCharWidths);
	UT_uint32				_sumPartWidth(UT_uint32 iStart, UT_uint32 iLen, const UT_GrowBuf* pgbCharWidths);
	void					_drawPart(UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);
	void					_drawPartWithBackground(UT_RGBColor& clr, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);
	void 					_calcWidths(UT_GrowBuf * pgbCharWidths);

	fp_Line*				m_pLine;
	void*					m_pLineData;
	fp_Run*					m_pPrev;
	fp_Run*					m_pNext;
	UT_uint32				m_iOffsetFirst;
	UT_uint32				m_iLen;
	UT_sint32				m_iWidth;
	UT_sint32				m_iHeight;
	UT_uint32				m_iAscent;
	UT_uint32				m_iDescent;
	UT_uint32				m_iExtraWidth;
	int						m_bCanSplit;
	int						m_iLineBreakAfter;
	int						m_iLineBreakBefore;

	enum
	{
		TEXT_DECOR_UNDERLINE = 0x01,
		TEXT_DECOR_OVERLINE = 0x02,
		TEXT_DECOR_LINETHROUGH = 0x04
	};
	unsigned char			m_fDecorations;

	/*
	  This makes the assumption that all characters in a given run
	  can be obtained from the same font.  This may not be true.
	  For example, suppose that a run includes a mixture of latin
	  and non-latin characters.  The resulting glyphs will probably need to be
	  retrieved from multiple fonts.
	  TODO fix this issue
	*/
	DG_Font*			m_pFont;
	UT_RGBColor			m_colorFG;
	fl_BlockLayout*                 m_pBL;
	DG_Graphics*                    m_pG;
};

#endif /* RUN_H */


