 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#ifndef RUN_H
#define RUN_H

#include <stdlib.h>
#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"

class UT_GrowBuf;
class FL_BlockLayout;
class FP_Line;
class DG_Graphics;
class DG_Font;
class PD_Document;
struct dg_DrawArgs;

/*
	Note that FP_Run encapsulates all text measurement issues, as well as
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

class FP_Run
{
	friend class fl_DocListener;

 public:
	FP_Run(FL_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	void					setLine(FP_Line*, void*);
	FP_Line*				getLine() const;
	void*                   getLineData();

	FL_BlockLayout*			getBlock() const;
	DG_Graphics*            getGraphics() const;
	UT_uint32				getHeight() const;
	UT_uint32				getWidth() const;
	UT_uint32				getAscent();
	UT_uint32				getDescent();
	UT_uint32				getLength() const;
	UT_uint32				getBlockOffset() const;
	void					lookupProperties(void);

	void					setNext(FP_Run*);
	void					setPrev(FP_Run*);
	FP_Run* 				getNext() const;
	FP_Run*					getPrev() const;

	UT_Bool					canSplit() const;
	UT_Bool					canBreakAfter() const;
	UT_Bool					canBreakBefore() const;
	UT_Bool					getLineBreakBefore() const; 
	UT_Bool					getLineBreakAfter() const;
	int						split(fp_RunSplitInfo&);
	UT_Bool					split(UT_uint32 splitOffset);
	UT_Bool					findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si);
	UT_Bool					findMinLeftFitSplitPoint(fp_RunSplitInfo& si);

	void 					calcWidths(UT_GrowBuf * pgbCharWidths);
	void            		expandWidthTo(UT_uint32);

	void					mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bRight);
	void					getOffsets(UT_uint32& xoff, UT_uint32& yoff);
	UT_uint32 				containsOffset(UT_uint32 iOffset);
	void 					findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);

#ifdef BUFFER	// top-down edit operations -- obsolete?
	UT_Bool 				insertData(UT_uint32 iOffset, UT_uint32 iCount);
	UT_Bool 				deleteChars(UT_uint32 iOffset, UT_uint32 iCountUnits, UT_uint32 iCountChars);
	UT_Bool					insertInlineMarker(UT_uint32 newMarkerOffset, UT_uint32 markerSize);
#endif

	void                    clearScreen(void);
	void					draw(dg_DrawArgs*);
	void					invert(UT_uint32 iStart, UT_uint32 iLen);
	void					dumpRun(void) const;
	
 protected:
	void					_drawDecors(UT_sint32, UT_sint32);
	void 					_getPartRect(UT_Rect* pRect, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
										 const UT_GrowBuf * pgbCharWidths);
	void					_drawPart(UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
									  const UT_GrowBuf * pgbCharWidths);
	void 					_calcWidths(UT_GrowBuf * pgbCharWidths);

	FP_Line*				m_pLine;
	void*					m_pLineData;
	FP_Run*					m_pPrev;
	FP_Run*					m_pNext;
	PD_Document*	       	m_pDoc;
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
	FL_BlockLayout*                 m_pBL;
	DG_Graphics*                    m_pG;
};

#endif /* RUN_H */


