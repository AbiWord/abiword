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

struct fp_RunSplitInfo
{
	UT_sint32 iLeftWidth;
	UT_sint32 iRightWidth;
	UT_sint32 iOffset;
};

#define BREAK_AUTO			0
#define BREAK_AVOID			1
#define BREAK_ALWAYS		2

#define FP_RUN_INSIDE      	1
#define FP_RUN_JUSTAFTER   	2
#define FP_RUN_NOT         	3

#define FPRUN_TEXT						1
#define FPRUN_IMAGE						2
#define FPRUN_TAB						3
#define FPRUN_FORCEDLINEBREAK			4
#define FPRUN_FORCEDCOLUMNBREAK			5
#define FPRUN_FORCEDPAGEBREAK			6
#define FPRUN_FIELD						7

class fp_Run
{
public:
	fp_Run(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, unsigned char iType);

	inline	unsigned char	getType(void) const 			{ return m_iType; }

	inline fp_Line*			getLine(void) const 			{ return m_pLine; }
	inline fl_BlockLayout*	getBlock(void) const 			{ return m_pBL; }
	inline UT_sint32		getX(void) const 				{ return m_iX; }
	inline UT_sint32		getY(void) const 				{ return m_iY; }
	inline UT_uint32		getHeight(void) const     		{ return m_iHeight; }
	inline UT_uint32		getWidth(void) const     		{ return m_iWidth; }
	inline fp_Run* 			getNext(void) const     		{ return m_pNext; }
	inline fp_Run*			getPrev(void) const     		{ return m_pPrev; }
	inline UT_uint32		getBlockOffset(void) const     	{ return m_iOffsetFirst; }
	inline UT_uint32		getLength(void) const     		{ return m_iLen; }
	inline DG_Graphics*     getGraphics(void) const     	{ return m_pG; }
	inline UT_uint32		getAscent(void) const 			{ return m_iAscent; }
	inline UT_uint32		getDescent(void) const 			{ return m_iDescent; }
	
	void					setLine(fp_Line*);
	void					setBlock(fl_BlockLayout *);
	void					setX(UT_sint32);
	void					setY(UT_sint32);
	void					setBlockOffset(UT_uint32);
	void					setLength(UT_uint32);
	
	void					setNext(fp_Run*);
	void					setPrev(fp_Run*);
	UT_Bool					isFirstRunOnLine(void) const;
	UT_Bool					isLastRunOnLine(void) const;
	UT_Bool					isOnlyRunOnLine(void) const;

	void					draw(dg_DrawArgs*);
	void            		clearScreen(void);
	UT_uint32 				containsOffset(UT_uint32 iOffset);
	
	virtual void			_draw(dg_DrawArgs*) = 0;
	virtual void       		_clearScreen(void) = 0;
	virtual UT_Bool			canBreakAfter(void) const = 0;
	virtual UT_Bool			canBreakBefore(void) const = 0;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE) = 0;
	virtual int				split(fp_RunSplitInfo&) = 0;
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE) = 0;
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL) = 0;
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height) = 0;
	virtual void			lookupProperties(void) = 0;
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths) = 0;
	
protected:
	unsigned char			m_iType;
	fp_Line*				m_pLine;
	fl_BlockLayout*         m_pBL;
	fp_Run*					m_pNext;
	fp_Run*					m_pPrev;
	UT_sint32				m_iX;
	UT_sint32				m_iY;
	UT_sint32				m_iHeight;
	UT_sint32				m_iWidth;
	UT_uint32				m_iOffsetFirst;
	UT_uint32				m_iLen;
	UT_uint32				m_iAscent;
	UT_uint32				m_iDescent;
	DG_Graphics*            m_pG;
	UT_Bool					m_bDirty;		// run erased @ old coords, needs to be redrawn
};

class fp_TextRun : public fp_Run
{
 public:
	fp_TextRun(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	virtual void			lookupProperties(void);
	virtual int				split(fp_RunSplitInfo&);
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths);
	
	void					drawSquiggle(UT_uint32, UT_uint32);
	
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

	/*
	  This makes the assumption that all characters in a given run
	  can be obtained from the same font.  This may not be true.
	  For example, suppose that a run includes a mixture of latin
	  and non-latin characters.  The resulting glyphs will probably need to be
	  retrieved from multiple fonts.
	  TODO fix this issue
	*/
	DG_Font*				m_pFont;
	UT_RGBColor				m_colorFG;
};

class fp_TabRun : public fp_Run
{
 public:
	fp_TabRun(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	virtual void			lookupProperties(void);
	virtual int				split(fp_RunSplitInfo&);
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(void);
};

class fp_ForcedLineBreakRun : public fp_Run
{
 public:
	fp_ForcedLineBreakRun(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	virtual void			lookupProperties(void);
	virtual int				split(fp_RunSplitInfo&);
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(void);
};

class fp_ImageRun : public fp_Run
{
 public:
	fp_ImageRun(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	virtual void			lookupProperties(void);
	virtual int				split(fp_RunSplitInfo&);
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(void);
};

#define FPFIELD_MAX_LENGTH	63

#define FPFIELD_TIME		1

class fp_FieldRun : public fp_Run
{
 public:
	fp_FieldRun(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties=UT_TRUE);

	virtual void			lookupProperties(void);
	virtual int				split(fp_RunSplitInfo&);
	virtual UT_Bool			split(UT_uint32 splitOffset, UT_Bool bInsertBlock=UT_FALSE);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_Bool			calcWidths(UT_GrowBuf * pgbCharWidths);

	UT_Bool					calculateValue(void);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(void);

	DG_Font*				m_pFont;
	UT_RGBColor				m_colorFG;
	UT_UCSChar				m_sFieldValue[FPFIELD_MAX_LENGTH];
	unsigned char			m_iFieldType;
};

#endif /* RUN_H */


