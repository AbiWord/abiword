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
class fd_Field;

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
	FPRUN_FIELDSTARTRUN				= 9,
	FPRUN_FIELDENDRUN				= 10,
	FPRUN__LAST__					= 11
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
		fp_FieldStartRun
		fp_FieldEndRun

	As far as the formatter's concerned, each subclass behaves somewhat 
	differently, but they can all be treated like rectangular blocks to 
	be arranged.  
*/

class fp_Run
{
public:
	fp_Run(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, FP_RUN_TYPE iType);
	virtual ~fp_Run() = 0;

	// inline getter member functions
	FP_RUN_TYPE		getType() const 				{ return m_iType; }
	fp_Line*		getLine() const 				{ return m_pLine; }
	fl_BlockLayout*	getBlock() const 				{ return m_pBL; }
	UT_sint32		getX() const 					{ return m_iX; }
	UT_sint32		getY() const 					{ return m_iY; }
	UT_sint32		getHeight() const				{ return m_iHeight; }
	UT_sint32		getHeightInLayoutUnits() const	{ return m_iHeightLayoutUnits; }
	UT_sint32		getWidth() const				{ return m_iWidth; }
	UT_sint32		getWidthInLayoutUnits() const	{ return m_iWidthLayoutUnits; }
	fp_Run* 		getNext() const					{ return m_pNext; }
	fp_Run*			getPrev() const					{ return m_pPrev; }
	UT_uint32		getBlockOffset() const			{ return m_iOffsetFirst; }
	UT_uint32		getLength() const				{ return m_iLen; }
	GR_Graphics*	getGraphics() const				{ return m_pG; }
	UT_uint32		getAscent() const				{ return m_iAscent; }
	UT_uint32		getDescent() const 				{ return m_iDescent; }
	UT_uint32		getAscentInLayoutUnits() const	{ return m_iAscentLayoutUnits; }
	UT_uint32		getDescentInLayoutUnits() const	{ return m_iDescentLayoutUnits; }

	void					insertIntoRunListBeforeThis(fp_Run& newRun);
	void					insertIntoRunListAfterThis(fp_Run& newRun);
	fd_Field*				getField(void) { return m_pField;}
	bool					isField(void) { return (bool) (m_pField != NULL) ;}
	void					unlinkFromRunList();
	
	void					setLine(fp_Line*);
	void					setBlock(fl_BlockLayout *);
	void					setX(UT_sint32);
	void					setY(UT_sint32);
	void					setBlockOffset(UT_uint32);
	void					setLength(UT_uint32);
	
	void					setNext(fp_Run*);
	void					setPrev(fp_Run*);
	bool					isFirstRunOnLine(void) const;
	bool					isLastRunOnLine(void) const;
	bool					isOnlyRunOnLine(void) const;
#ifdef BIDI_ENABLED
	bool					isFirstVisRunOnLine(void) const;
	bool					isLastVisRunOnLine(void) const;
#endif

	void					draw(dg_DrawArgs*);
	void            		clearScreen(bool bFullLineHeightRect = false);
	void					markAsDirty(void)	{ m_bDirty = true; }
	bool					isDirty(void) const { return m_bDirty; }
	virtual bool			canContainPoint(void) const;
	virtual const PP_AttrProp* getAP(void) const;
	virtual void			fetchCharWidths(fl_CharWidths * pgbCharWidths);
	virtual	bool			recalcWidth(void);
	
	virtual void			_draw(dg_DrawArgs*) = 0;
    void                    _drawTextLine(UT_sint32, UT_sint32, UT_uint32, UT_uint32, UT_UCSChar *);
	virtual void       		_clearScreen(bool bFullLineHeightRect) = 0;
	virtual bool			canBreakAfter(void) const = 0;
	virtual bool			canBreakBefore(void) const = 0;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return false; }
	virtual bool			alwaysFits(void) const { return false; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false) = 0;
	virtual UT_sint32		findTrailingSpaceDistance(void) const { return 0; }	
	virtual UT_sint32		findTrailingSpaceDistanceInLayoutUnits(void) const { return 0; }	
	virtual bool			findFirstNonBlankSplitPoint(fp_RunSplitInfo& /*si*/) { return false; }
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL) = 0;
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection) = 0;
	virtual void			lookupProperties(void) = 0;
	virtual bool			doesContainNonBlankData(void) const { return true; }	// Things like text whould return false if it is all spaces.
	virtual bool			isSuperscript(void) const { return false; }
	virtual bool			isSubscript(void) const { return false; }
	virtual bool			isUnderline(void) const { return false; };
	virtual bool			isOverline(void) const { return false; };
	virtual bool			isStrikethrough(void) const { return false; };
	virtual void			setLinethickness(UT_sint32 max_linethickness) { return; };
	virtual UT_sint32		getLinethickness(void) {return 0; } ;
	virtual void			setUnderlineXoff(UT_sint32 xoff) { return; };
	virtual UT_sint32		getUnderlineXoff(void) { return 0; } ;
	virtual void			setOverlineXoff(UT_sint32 xoff){ return ;};
	virtual UT_sint32		getOverlineXoff(void) {return 0; };
	virtual void			setMaxUnderline(UT_sint32 xoff){ return; };
	virtual UT_sint32		getMaxUnderline(void) {return 0; };
	virtual void			setMinOverline(UT_sint32 xoff) {return; };
	virtual UT_sint32		getMinOverline(void) { return 0; };

#ifdef BIDI_ENABLED
	UT_sint32				inline getDirection(){return m_iDirection; };
	UT_sint32               getVisDirection();
	virtual void            setDirection(UT_sint32 iDirection = -1);
	UT_uint32               getVisPosition(UT_uint32 ilogPos);
	UT_uint32               getVisPosition(UT_uint32 iLogPos, UT_uint32 iLen);
	UT_uint32               getOffsetFirstVis();
	UT_uint32               getOffsetLog(UT_uint32 iVisOff);
	//virtual bool         setUnicodeDirection();
	virtual void            setDirectionProperty(UT_sint32 dir);	
#endif	

#ifdef FMT_TEST
	virtual void			__dump(FILE * fp) const;
#endif	
	
protected:
	FP_RUN_TYPE				m_iType;
	fp_Line*				m_pLine;
	fl_BlockLayout*			m_pBL;
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
	GR_Graphics*			m_pG;
	bool					m_bDirty;		// run erased @ old coords, needs to be redrawn
	fd_Field*				m_pField;
#ifdef BIDI_ENABLED
	UT_sint32				m_iDirection;   //#TF direction of the run 0 for left-to-right, 1 for right-to-left
#endif

	// the paper's color at any given time
	UT_RGBColor             m_colorBG;

private:
	fp_Run(const fp_Run&);			// no impl.
	void operator=(const fp_Run&);	// no impl.
};

class fp_TabRun : public fp_Run
{
 public:
	fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	void					setWidth(UT_sint32);
	
protected:
	UT_RGBColor			m_colorFG;

	virtual void			_drawArrow(UT_uint32 iLeft,UT_uint32 iTop,UT_uint32 iWidth, UT_uint32 iHeight);
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class fp_ForcedLineBreakRun : public fp_Run
{
 public:
	fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			canContainPoint(void) const;
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class fp_FieldStartRun : public fp_Run
{
 public:
	fp_FieldStartRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			canContainPoint(void) const;
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class fp_FieldEndRun : public fp_Run
{
 public:
	fp_FieldEndRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			canContainPoint(void) const;
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class fp_ForcedColumnBreakRun : public fp_Run
{
 public:
	fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			canContainPoint(void) const;
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class fp_ForcedPageBreakRun : public fp_Run
{
 public:
	fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			canContainPoint(void) const;
	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			isForcedBreak(void) const { return true; }
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
};

class fp_ImageRun : public fp_Run
{
public:
	fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, GR_Image* pImage);
	virtual ~fp_ImageRun();

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	
protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);

	GR_Image*				m_pImage;
};

#define FPFIELD_MAX_LENGTH	63

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
	fp_FieldTypesEnum	m_Type;
	const char*			m_Desc;
	XAP_String_Id		m_DescId;
};

struct fp_FieldData
{
	fp_FieldTypesEnum	m_Type;
	fp_FieldsEnum		m_Num;
	const char*			m_Desc;
	const char*			m_Tag;
	XAP_String_Id		m_DescId;
};


extern fp_FieldTypeData fp_FieldTypes[];
extern fp_FieldData fp_FieldFmts[];

class fp_FieldRun : public fp_Run
{
public:
	fp_FieldRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);
	virtual ~fp_FieldRun() {};

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual fp_FieldsEnum	getFieldType(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);

	virtual bool			isSuperscript(void) const;
	virtual bool			isSubscript(void) const;

	bool					_setValue(UT_UCSChar *p_new_value);					

	virtual bool			calculateValue(void);
	
protected:
	virtual void			_draw(dg_DrawArgs*) {};
	virtual void			_defaultDraw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);

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
	UT_Byte					m_fPosition;
};

class fp_FieldTimeRun : public fp_FieldRun
{
public:
	
	fp_FieldTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class fp_FieldPageNumberRun : public fp_FieldRun
{
public:
	
	fp_FieldPageNumberRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class fp_FieldPageCountRun : public fp_FieldRun
{
public:
	
	fp_FieldPageCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

class fp_FieldDateRun : public fp_FieldRun
{
 public:
        fp_FieldDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// BEGIN DOM

// document-related information fields

// count of characters in the document
// including white spaces
class fp_FieldCharCountRun : public fp_FieldRun
{
 public:
  fp_FieldCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// count of the non-blank characters
// in the document
class fp_FieldNonBlankCharCountRun : public fp_FieldRun
{
 public:
  fp_FieldNonBlankCharCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// count of the #lines in the document
class fp_FieldLineCountRun : public fp_FieldRun
{
 public:
  fp_FieldLineCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// count of the #para in the document
class fp_FieldParaCountRun : public fp_FieldRun
{
 public:
  fp_FieldParaCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// count of #words in the document
class fp_FieldWordCountRun : public fp_FieldRun
{
 public:
  fp_FieldWordCountRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};


// date-releated fields

// Americans - mm/dd/yy
class fp_FieldMMDDYYRun : public fp_FieldRun
{
 public:
  fp_FieldMMDDYYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// most of the world - dd/mm/yy
class fp_FieldDDMMYYRun : public fp_FieldRun
{
 public:
  fp_FieldDDMMYYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// April 18, 1979
class fp_FieldMonthDayYearRun : public fp_FieldRun
{
  public:
  fp_FieldMonthDayYearRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// Apr. 18, 1979
class fp_FieldMthDayYearRun : public fp_FieldRun
{
 public:
  fp_FieldMthDayYearRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// default representation for your locale. includes time too
class fp_FieldDefaultDateRun : public fp_FieldRun
{
 public:
  fp_FieldDefaultDateRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// default for your locale, not appending the time
class fp_FieldDefaultDateNoTimeRun : public fp_FieldRun
{
 public:
  fp_FieldDefaultDateNoTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// day of the week (Wednesday)
class fp_FieldWkdayRun : public fp_FieldRun
{
 public:
  fp_FieldWkdayRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// day of year (i.e. 72)
class fp_FieldDOYRun : public fp_FieldRun
{
 public:
  fp_FieldDOYRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// military (zulu) time
class fp_FieldMilTimeRun : public fp_FieldRun
{
 public:
  fp_FieldMilTimeRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// prints am or pm
class fp_FieldAMPMRun : public fp_FieldRun
{
 public:
  fp_FieldAMPMRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// milliseconds since the epoch, for you geeks out there :-)
class fp_FieldTimeEpochRun : public fp_FieldRun
{
 public:
  fp_FieldTimeEpochRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// your time zone (EST, for example)
class fp_FieldTimeZoneRun : public fp_FieldRun
{
 public:
  fp_FieldTimeZoneRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// application runs

// build id
class fp_FieldBuildIdRun : public fp_FieldRun
{
 public:
  fp_FieldBuildIdRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// build version (i.e. 0.7.13)
class fp_FieldBuildVersionRun : public fp_FieldRun
{
 public:
  fp_FieldBuildVersionRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen);

	virtual bool			calculateValue(void);
	virtual void			_draw(dg_DrawArgs* pDA) { _defaultDraw(pDA); }
};

// END DOM

class fp_FmtMarkRun : public fp_Run
{
public:
	fp_FmtMarkRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst);

	virtual void			lookupProperties(void);
	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			letPointPass(void) const;
	virtual bool			findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, bool bForce=false);
	virtual const PP_AttrProp* getAP(void) const;
	virtual bool			isSuperscript(void) const { return false; }
	virtual bool			isSubscript(void)  const { return false; }

protected:
	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte					m_fPosition;
};

#endif /* FP_RUN_H */






