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



#ifndef FP_RUN_H
#define FP_RUN_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_types.h"
#include "ut_misc.h"
#include "pt_Types.h"
#include "ut_assert.h"
#include "ap_Strings.h"


class UT_GrowBuf;
class fl_BlockLayout;
class fp_Line;
class GR_Graphics;
class GR_Font;
class GR_Image;
class PD_Document;
class PP_AttrProp;
struct dg_DrawArgs;
class fl_CharWidths;

struct fp_RunSplitInfo
{
	UT_sint32 iLeftWidth;
	UT_sint32 iRightWidth;
	UT_sint32 iOffset;
};

// TODO The break type is not used. Is it put here looking forward,
// TODO or is it left from some earlier experiments?
enum FP_RUN_BREAK_TYPE
{
	BREAK_AUTO			= 0,
	BREAK_AVOID			= 1,
	BREAK_ALWAYS		= 2
};

// TODO 
enum FP_RUN_RELATIVE_POINT_POSITION
{
	FP_RUN_INSIDE      	= 1,
	FP_RUN_NOT         	= 2
};

enum FP_RUN_TYPE
{
	FPRUN__FIRST__					= 1,
	FPRUN_TEXT						= 1,
	FPRUN_IMAGE						= 2,
	FPRUN_TAB						= 3,
	FPRUN_FORCEDLINEBREAK			= 4,
	FPRUN_FORCEDCOLUMNBREAK			= 5,
	FPRUN_FORCEDPAGEBREAK			= 6,
	FPRUN_FIELD						= 7,
	FPRUN_FMTMARK					= 8,
	FPRUN__LAST__					= 8
};

/*
	fp_Run represents a contiguous homogenous chunk on a single line.  
	This file also defines the following subclasses:

		fp_TabRun
		fp_ForcedLineBreakRun
		fp_ForcedColumnBreakRun
		fp_ForcedPageBreakRun
		fp_ImageRun
		fp_FieldRun
		fp_FmtMarkRun

	As far as the formatter's concerned, each subclass behaves somewhat 
	differently, but they can all be treated like rectangular blocks to 
	be arranged.  
*/

class fp_Run
{
public:
	fp_Run(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, unsigned char iType);
	virtual ~fp_Run() = 0;

	inline	unsigned char	getType(void) const 			{ return m_iType; }

	inline fp_Line*			getLine(void) const 			{ return m_pLine; }
	inline fl_BlockLayout*	getBlock(void) const 			{ return m_pBL; }
	inline UT_sint32		getX(void) const 				{ return m_iX; }
	inline UT_sint32		getY(void) const 				{ return m_iY; }
	inline UT_sint32		getHeight(void) const     		{ return m_iHeight; }
	inline UT_sint32		getWidth(void) const     		{ return m_iWidth; }
	inline UT_sint32		getWidthInLayoutUnits(void) const     { return m_iWidthLayoutUnits; }
	inline fp_Run* 			getNext(void) const     		{ return m_pNext; }
	inline fp_Run*			getPrev(void) const     		{ return m_pPrev; }
	inline UT_uint32		getBlockOffset(void) const     	{ return m_iOffsetFirst; }
	inline UT_uint32		getLength(void) const     		{ return m_iLen; }
	inline GR_Graphics*     getGraphics(void) const     	{ return m_pG; }
	inline UT_uint32		getAscent(void) const 			{ return m_iAscent; }
	inline UT_uint32		getDescent(void) const 			{ return m_iDescent; }
	inline UT_uint32		getAscentInLayoutUnits(void) const 		{ return m_iAscentLayoutUnits; }
	inline UT_uint32		getDescentInLayoutUnits(void) const 	{ return m_iDescentLayoutUnits; }
	void					insertIntoRunListBeforeThis(fp_Run& newRun);
	void					insertIntoRunListAfterThis(fp_Run& newRun);
	void					unlinkFromRunList();
	
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
	void            		clearScreen(UT_Bool bFullLineHeightRect = UT_FALSE);
	void					markAsDirty(void)	{ m_bDirty = UT_TRUE; }
	UT_Bool					isDirty(void) const { return m_bDirty; }
	virtual UT_uint32 		containsOffset(UT_uint32 iOffset);
	virtual UT_Bool			canContainPoint(void) const;
	virtual const PP_AttrProp* getAP(void) const;
	virtual void			fetchCharWidths(fl_CharWidths * pgbCharWidths);
	virtual	UT_Bool			recalcWidth(void);
	
	virtual void			_draw(dg_DrawArgs*) = 0;
    void                    _drawTextLine(UT_sint32, UT_sint32, UT_uint32, UT_uint32, UT_UCSChar *);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect) = 0;
	virtual UT_Bool			canBreakAfter(void) const = 0;
	virtual UT_Bool			canBreakBefore(void) const = 0;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			isForcedBreak(void) const { return UT_FALSE; }
	virtual UT_Bool			alwaysFits(void) const { return UT_FALSE; }
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE) = 0;
	virtual UT_sint32		findTrailingSpaceDistance(void) const { return 0; }	
	virtual UT_sint32		findTrailingSpaceDistanceInLayoutUnits(void) const { return 0; }	
	virtual UT_Bool			findFirstNonBlankSplitPoint(fp_RunSplitInfo& si) { return UT_FALSE; }
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL) = 0;
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height) = 0;
	virtual void			lookupProperties(void) = 0;
	virtual UT_Bool			doesContainNonBlankData(void) const { return UT_TRUE; }	// Things like text whould return false if it is all spaces.
	virtual UT_Bool			isSuperscript(void) const { return UT_FALSE; }
	virtual UT_Bool			isSubscript(void) const { return UT_FALSE; }
	virtual UT_Bool                         isUnderline(void) const { return UT_FALSE; };
	virtual UT_Bool                         isOverline(void) const { return UT_FALSE; };
	virtual UT_Bool                         isStrikethrough(void) const { return UT_FALSE; };
	virtual void                            setLinethickness(UT_sint32 max_linethickness) { return; };
	virtual UT_sint32                       getLinethickness(void) {return 0; } ;
	virtual void                            setUnderlineXoff(UT_sint32 xoff) { return; };
	virtual UT_sint32                       getUnderlineXoff(void) { return 0; } ;
	virtual void                            setOverlineXoff(UT_sint32 xoff){ return ;};
	virtual UT_sint32                       getOverlineXoff(void) {return 0; };
	virtual void                            setMaxUnderline(UT_sint32 xoff){ return; };
	virtual UT_sint32                       getMaxUnderline(void) {return 0; };
	virtual void                            setMinOverline(UT_sint32 xoff) {return; };
	virtual UT_sint32                       getMinOverline(void) { return 0; };
	
#ifdef FMT_TEST
	virtual void			__dump(FILE * fp) const;
#endif	
	
protected:
	unsigned char			m_iType;
	fp_Line*				m_pLine;
	fl_BlockLayout*         m_pBL;
	fp_Run*					m_pNext;
	fp_Run*					m_pPrev;
	UT_sint32				m_iX;
	UT_sint32				m_iY;
	UT_sint32				m_iHeight;
	UT_sint32				m_iHeightLayoutUnits;
	UT_sint32				m_iWidth;
	UT_sint32				m_iWidthLayoutUnits;
	UT_uint32				m_iOffsetFirst;
	UT_uint32				m_iLen;
	UT_uint32				m_iAscent;
	UT_uint32				m_iDescent;
	UT_uint32				m_iAscentLayoutUnits;
	UT_uint32				m_iDescentLayoutUnits;
	GR_Graphics*            m_pG;
	UT_Bool					m_bDirty;		// run erased @ old coords, needs to be redrawn

private:
	fp_Run(const fp_Run&);			// no impl.
	void operator=(const fp_Run&);	// no impl.
};

class fp_TabRun : public fp_Run
{
 public:
	fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	void					setWidth(UT_sint32);
	
protected:
    UT_RGBColor             m_colorFG;

    virtual void            _drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 iHeight);
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);
};

class fp_ForcedLineBreakRun : public fp_Run
{
 public:
	fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual UT_Bool			canContainPoint(void) const;
	virtual UT_uint32 		containsOffset(UT_uint32 iOffset);
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			isForcedBreak(void) const { return UT_TRUE; }
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);
};

class fp_ForcedColumnBreakRun : public fp_Run
{
 public:
	fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual UT_Bool			canContainPoint(void) const;
	virtual UT_uint32 		containsOffset(UT_uint32 iOffset);
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			isForcedBreak(void) const { return UT_TRUE; }
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);
};

class fp_ForcedPageBreakRun : public fp_Run
{
 public:
	fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual UT_Bool			canContainPoint(void) const;
	virtual UT_uint32 		containsOffset(UT_uint32 iOffset);
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			isForcedBreak(void) const { return UT_TRUE; }
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);
};

class fp_ImageRun : public fp_Run
{
public:
	fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, GR_Image* pImage);
	virtual ~fp_ImageRun();

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);

	GR_Image*				m_pImage;
};

#define FPFIELD_MAX_LENGTH	63

/*
#define FPFIELD_TIME		1
#define FPFIELD_PAGE_NUMBER	2
#define FPFIELD_PAGE_COUNT	3
<<<<<<< fp_Run.h
#define FPFIELD_LIST_LABEL	4
=======
*/

#define  _FIELD(type,desc,tag)  /*nothing*/
#define  _FIELDTYPE(type,desc)  FPFIELDTYPE_##type,

enum fp_FieldTypesEnum { FPFIELDTYPE_START,

#include "fp_Fields.h"

FPFIELDTYPE_END };

#undef  _FIELD
#undef  _FIELDTYPE

#define  _FIELD(type,desc,tag)  FPFIELD_##tag,
#define  _FIELDTYPE(type,desc)  /*nothing*/

enum fp_FieldsEnum { FPFIELD_start,

#include "fp_Fields.h"

FPFIELD_end };

#undef  _FIELD
#undef  _FIELDTYPE


struct fp_FieldTypeData
{
    fp_FieldTypesEnum  m_Type;
    const char*        m_Desc;
    XAP_String_Id      m_DescId;
};

struct fp_FieldData
{
    fp_FieldTypesEnum  m_Type;
    fp_FieldsEnum      m_Num;
    const char*        m_Desc;
    const char*        m_Tag;
    XAP_String_Id      m_DescId;
};


extern fp_FieldTypeData fp_FieldTypes[];
extern fp_FieldData fp_FieldFmts[];

class fp_FieldRun : public fp_Run
{
public:
	fp_FieldRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);

	UT_Bool					calculateValue(void);
        virtual UT_Bool                 isSuperscript(void) const;
        virtual UT_Bool                 isSubscript(void) const;
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);

	GR_Font*				m_pFont;
	GR_Font*				m_pFontLayout;
	UT_RGBColor				m_colorFG;
	UT_RGBColor				m_colorBG;
	UT_UCSChar				m_sFieldValue[FPFIELD_MAX_LENGTH];
	fp_FieldsEnum			m_iFieldType;
        enum
        {
                TEXT_POSITION_NORMAL,
                TEXT_POSITION_SUPERSCRIPT,
                TEXT_POSITION_SUBSCRIPT
        };
        UT_Byte                         m_fPosition;
};

class fp_FmtMarkRun : public fp_Run
{
public:
	fp_FmtMarkRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst);

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height);
	virtual UT_Bool			canBreakAfter(void) const;
	virtual UT_Bool			canBreakBefore(void) const;
	virtual UT_Bool			letPointPass(void) const;
	virtual UT_Bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce=UT_FALSE);
	virtual UT_uint32 		containsOffset(UT_uint32 iOffset);
	virtual const PP_AttrProp* getAP(void) const;
	virtual UT_Bool			isSuperscript(void) const { return UT_FALSE; }
	virtual UT_Bool			isSubscript(void)  const { return UT_FALSE; }

protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(UT_Bool bFullLineHeightRect);
        enum
        {
                TEXT_POSITION_NORMAL,
                TEXT_POSITION_SUPERSCRIPT,
                TEXT_POSITION_SUBSCRIPT
        };
        UT_Byte                         m_fPosition;
};

#endif /* FP_RUN_H */






