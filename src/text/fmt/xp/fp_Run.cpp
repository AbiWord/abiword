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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fp_Run.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "pp_Property.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "gr_DrawArgs.h"
#include "fv_View.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"


#define FREEP(p)	do { if (p) free(p); } while (0)

/*****************************************************************/

/*
  TODO this file is too long -- it needs to be broken
  up into several pieces.
*/

fp_Run::fp_Run(fl_BlockLayout* pBL,
					   GR_Graphics* pG,
					   UT_uint32 iOffsetFirst,
					   UT_uint32 iLen,
					   unsigned char iType)
{
	m_pG = pG;
	m_pBL = pBL;
	m_iOffsetFirst = iOffsetFirst;
	m_iLen = iLen;
	m_iType = iType;
	
	m_bDirty = UT_TRUE;		// a run which has just been created is not onscreen, therefore it is dirty
	m_iWidth = 0;
	m_iHeight = 0;
	m_iX = 0;
	m_iY = 0;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pLine = NULL;
	m_iAscent = 0;
	m_iDescent = 0;
}

fp_Run::~fp_Run()
{
}

#ifndef NDEBUG
void	fp_Run::debug_dump(void)
{
	UT_DEBUGMSG(("fp_Run:  0x%p\n\tm_iType = %d\n\tm_iOffsetFirst = %d\n\tm_iLen = %d\n\tm_bDirty = %d\n\tm_iX = %d\n\tm_iY = %d\n\tm_iWidth = %d\n\tm_iHeight = %d\n\tm_pNext = 0x%p\n\tm_pPrev = 0x%p\n",
				 this,
				 m_iType,
				 m_iOffsetFirst,
				 m_iLen,
				 m_bDirty,
				 m_iX,
				 m_iY,
				 m_iWidth,
				 m_iHeight,
				 m_pNext,
				 m_pPrev));
}
#endif

void	fp_Run::setX(UT_sint32 iX)
{
	if (iX == m_iX)
	{
		return;
	}

	clearScreen();
	
	m_iX = iX;
}

void	fp_Run::setY(UT_sint32 iY)
{
	if (iY == m_iY)
	{
		return;
	}
	
	clearScreen();
	
	m_iY = iY;
}
	
void fp_Run::setLine(fp_Line* pLine)
{
	if (pLine == m_pLine)
	{
		return;
	}
	
	clearScreen();
	
	m_pLine = pLine;
}

void fp_Run::setBlock(fl_BlockLayout * pBL)
{
	m_pBL = pBL;
}

void fp_Run::setNext(fp_Run* p)
{
	m_pNext = p;
}

void fp_Run::setPrev(fp_Run* p)
{
	m_pPrev = p;
}

UT_Bool fp_Run::isLastRunOnLine(void) const
{
	return (m_pLine->getLastRun() == this);
}

UT_Bool fp_Run::isFirstRunOnLine(void) const
{
	return (m_pLine->getFirstRun() == this);
}

UT_Bool fp_Run::isOnlyRunOnLine(void) const
{
	if (m_pLine->countRuns() == 1)
	{
		UT_ASSERT(isFirstRunOnLine());
		UT_ASSERT(isLastRunOnLine());

		return UT_TRUE;
	}

	return UT_FALSE;
}

void fp_Run::setLength(UT_uint32 iLen)
{
	if (iLen == m_iLen)
	{
		return;
	}

	clearScreen();
	
	m_iLen = iLen;
}

void fp_Run::setBlockOffset(UT_uint32 offset)
{
	m_iOffsetFirst = offset;
}

void fp_Run::clearScreen(UT_Bool bFullLineHeightRect)
{
	if (m_bDirty)
	{
		// no need to clear if we've already done so.
		return;
	}

	if (!m_pLine)
	{
		// nothing to clear if this run is not currently on a line
		return;
	}
	
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	_clearScreen(bFullLineHeightRect);
	
	// make sure we only get erased once
	m_bDirty = UT_TRUE;
}

void fp_Run::draw(dg_DrawArgs* pDA)
{
	if (pDA->bDirtyRunsOnly)
	{
		if (!m_bDirty)
		{
			return;
		}
	}
	
	_draw(pDA);

	m_bDirty = UT_FALSE;
}

UT_Bool fp_Run::canContainPoint(void) const
{
	return UT_TRUE;
}

UT_uint32 fp_Run::containsOffset(UT_uint32 iOffset)
{
	if ((iOffset >= m_iOffsetFirst) && (iOffset < (m_iOffsetFirst + m_iLen)))
	{
		return FP_RUN_INSIDE;
	}
	else if (iOffset == (m_iOffsetFirst + m_iLen))
	{
		return FP_RUN_JUSTAFTER;
	}
	else
	{
		return FP_RUN_NOT;
	}
}

void fp_Run::fetchCharWidths(UT_GrowBuf * pgbCharWidths)
{
	// do nothing.  subclasses may override this.
}

UT_Bool fp_Run::recalcWidth(void)
{
	// do nothing.  subclasses may override this.
	return UT_FALSE;
}

const PP_AttrProp* fp_Run::getAP(void) const
{
	const PP_AttrProp * pSpanAP = NULL;
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst+fl_BLOCK_STRUX_OFFSET,&pSpanAP);

	return pSpanAP;
}

fp_TabRun::fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TAB)
{
	lookupProperties();
}

void fp_TabRun::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst+fl_BLOCK_STRUX_OFFSET,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();
	GR_Font* pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);

	m_pG->setFont(pFont);
	m_iAscent = m_pG->getFontAscent();	
	m_iDescent = m_pG->getFontDescent();
	m_iHeight = m_pG->getFontHeight();
}

UT_Bool fp_TabRun::canBreakAfter(void) const
{
	return UT_FALSE;
}

UT_Bool fp_TabRun::canBreakBefore(void) const
{
	return UT_FALSE;
}

UT_Bool	fp_TabRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

void fp_TabRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_TabRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	if (iOffset == m_iOffsetFirst)
	{
		x = xoff;
	}
	else
	{
		UT_ASSERT(iOffset == (m_iOffsetFirst + m_iLen));
		
		x = xoff + getWidth();
	}
	
	y = yoff;
	height = m_iHeight;
}

void fp_TabRun::setWidth(UT_sint32 iWidth)
{
	m_iWidth = iWidth;
}

void fp_TabRun::_clearScreen(UT_Bool bFullLineHeightRect)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	
	m_pG->clearArea(xoff, yoff, m_iWidth, m_pLine->getHeight());
}

void fp_TabRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
	
	UT_RGBColor clrSelBackground(192, 192, 192);
	UT_RGBColor clrNormalBackground(255,255,255);

	UT_sint32 iFillHeight = m_pLine->getHeight();
	UT_sint32 iFillTop = pDA->yoff - m_pLine->getAscent();
		
	FV_View* pView = m_pBL->getDocLayout()->getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
	UT_ASSERT(iSel1 <= iSel2);
	
	UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;

	if (
		(iSel1 <= iRunBase)
		&& (iSel2 > iRunBase)
		)
	{
		m_pG->fillRect(clrSelBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
	}
	else
	{
		m_pG->fillRect(clrNormalBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
	}
}

fp_ForcedLineBreakRun::fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDLINEBREAK)
{
}

void fp_ForcedLineBreakRun::lookupProperties(void)
{
}

UT_Bool fp_ForcedLineBreakRun::canContainPoint(void) const
{
	return UT_FALSE;
}

UT_uint32 fp_ForcedLineBreakRun::containsOffset(UT_uint32 iOffset)
{
	return FP_RUN_NOT;
}

UT_Bool fp_ForcedLineBreakRun::canBreakAfter(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

UT_Bool fp_ForcedLineBreakRun::canBreakBefore(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

UT_Bool	fp_ForcedLineBreakRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

void fp_ForcedLineBreakRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ForcedLineBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;
	height = m_pLine->getHeight();
}

void fp_ForcedLineBreakRun::_clearScreen(UT_Bool bFullLineHeightRect)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedLineBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

fp_ImageRun::fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, GR_Image* pImage) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_IMAGE)
{
#if 0	// put this back later
	UT_ASSERT(pImage);
#endif
	
	m_pImage = pImage;
	
	lookupProperties();

	m_iAscent = m_iHeight;
	m_iDescent = 0;
}

fp_ImageRun::~fp_ImageRun()
{
	if (m_pImage)
	{
		delete m_pImage;
	}
}

void fp_ImageRun::lookupProperties(void)
{
	if (m_pImage)
	{
		m_iWidth = m_pImage->getDisplayWidth();
		m_iHeight = m_pImage->getDisplayHeight();
	}
	else
	{
		// If we have no image, we simply insert a one-inch-square "slug"
			
		m_iWidth = m_pG->convertDimension("1in");
		m_iHeight = m_pG->convertDimension("1in");
	}
		
	UT_ASSERT(m_iWidth > 0);
	UT_ASSERT(m_iHeight > 0);

	m_iAscent = m_iHeight;
	m_iDescent = 0;
}

UT_Bool fp_ImageRun::canBreakAfter(void) const
{
	return UT_TRUE;
}

UT_Bool fp_ImageRun::canBreakBefore(void) const
{
	return UT_TRUE;
}

UT_Bool	fp_ImageRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

void fp_ImageRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	if (x > m_iWidth)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
		bBOL = UT_FALSE;
		bEOL = UT_FALSE;
	}
	else
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst;
		bBOL = UT_FALSE;
		bEOL = UT_FALSE;
	}
}

void fp_ImageRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;
	height = m_iHeight;
}

void fp_ImageRun::_clearScreen(UT_Bool bFullLineHeightRect)
{
	UT_ASSERT(!m_bDirty);
	
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = m_pLine->getHeight();
	m_pG->clearArea(xoff, yoff, m_iWidth, iLineHeight);
}

void fp_ImageRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

	UT_sint32 xoff = 0, yoff = 0;
	m_pLine->getScreenOffsets(this, xoff, yoff);

	if (m_pImage)
	{
		m_pG->drawImage(m_pImage, xoff, yoff);
	}
	else
	{
		UT_RGBColor clr(0, 0, 255);
		m_pG->fillRect(clr, xoff, yoff, m_iWidth, m_iHeight);
	}
}

fp_FieldRun::fp_FieldRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELD)
{
	m_pFont = NULL;

	m_sFieldValue[0] = 0;
	
	lookupProperties();

	// TODO allow other field types
	m_iFieldType = FPFIELD_TIME;

	calculateValue();
}

UT_Bool fp_FieldRun::calculateValue(void)
{
	UT_UCSChar sz_ucs_FieldValue[FPFIELD_MAX_LENGTH + 1];
	sz_ucs_FieldValue[0] = 0;
	
	switch (m_iFieldType)
	{
	case FPFIELD_TIME:
	{
		char szFieldValue[FPFIELD_MAX_LENGTH + 1];

		time_t	tim = time(NULL);
		struct tm *pTime = localtime(&tim);
	
		strftime(szFieldValue, FPFIELD_MAX_LENGTH, "%I:%M:%S %p", pTime);

		UT_UCS_strcpy_char(sz_ucs_FieldValue, szFieldValue);
		break;
	}
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
	}
	
	if (0 != UT_UCS_strcmp(sz_ucs_FieldValue, m_sFieldValue))
	{
		clearScreen();
		
		UT_UCS_strcpy(m_sFieldValue, sz_ucs_FieldValue);

		{
			unsigned short aCharWidths[FPFIELD_MAX_LENGTH];
	
			m_pG->setFont(m_pFont);
			UT_sint32 iNewWidth = m_pG->measureString(m_sFieldValue, 0, UT_UCS_strlen(m_sFieldValue), aCharWidths);

			if (iNewWidth != m_iWidth)
			{
				clearScreen();
				m_iWidth = iNewWidth;
				return UT_TRUE;
			}

			return UT_FALSE;
		}
	}

	return UT_FALSE;
}

void fp_FieldRun::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst+fl_BLOCK_STRUX_OFFSET,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();
	m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP),

	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP), m_colorFG);

	m_pG->setFont(m_pFont);
	m_iAscent = m_pG->getFontAscent();	
	m_iDescent = m_pG->getFontDescent();
	m_iHeight = m_pG->getFontHeight();
}

UT_Bool fp_FieldRun::canBreakAfter(void) const
{
	return UT_TRUE;
}

UT_Bool fp_FieldRun::canBreakBefore(void) const
{
	return UT_TRUE;
}

UT_Bool	fp_FieldRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

void fp_FieldRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_FieldRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;
	height = m_iHeight;
}

void fp_FieldRun::_clearScreen(UT_Bool bFullLineHeightRect)
{
	UT_ASSERT(!m_bDirty);
	
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	UT_sint32 iLineHeight = m_pLine->getHeight();
	m_pG->clearArea(xoff, yoff, m_iWidth, iLineHeight);
}

void fp_FieldRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);

	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;

		/*
		  TODO we might want special colors for fields.  We might also
		  want the colors to be calculated on the fly instead of
		  hard-coded.  See comment above in fp_TextRun::_draw*.
		  For now, we are hard-coding a couple of shades of grey,
		  with fields always being drawn a little darker than the
		  surrounding text.
		*/
		
		UT_RGBColor clrSelBackground(112, 112, 112);
		UT_RGBColor clrNormalBackground(220, 220, 220);

		UT_sint32 iFillHeight = m_pLine->getHeight();
		UT_sint32 iFillTop = pDA->yoff - m_pLine->getAscent();
		
		FV_View* pView = m_pBL->getDocLayout()->getView();
		UT_uint32 iSelAnchor = pView->getSelectionAnchor();
		UT_uint32 iPoint = pView->getPoint();

		UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
		UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
		UT_ASSERT(iSel1 <= iSel2);
	
		if (
			(iSel1 <= iRunBase)
			&& (iSel2 > iRunBase)
			)
		{
			m_pG->fillRect(clrSelBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
		}
		else
		{
			m_pG->fillRect(clrNormalBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
		}
	}

	m_pG->setFont(m_pFont);
	m_pG->setColor(m_colorFG);
	
	m_pG->drawChars(m_sFieldValue, 0, UT_UCS_strlen(m_sFieldValue), pDA->xoff, pDA->yoff - m_iAscent);
}

fp_ForcedColumnBreakRun::fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDCOLUMNBREAK)
{
}

void fp_ForcedColumnBreakRun::lookupProperties(void)
{
}

UT_Bool fp_ForcedColumnBreakRun::canContainPoint(void) const
{
	return UT_FALSE;
}

UT_uint32 fp_ForcedColumnBreakRun::containsOffset(UT_uint32 iOffset)
{
	return FP_RUN_NOT;
}

UT_Bool fp_ForcedColumnBreakRun::canBreakAfter(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

UT_Bool fp_ForcedColumnBreakRun::canBreakBefore(void) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

UT_Bool	fp_ForcedColumnBreakRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

void fp_ForcedColumnBreakRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ForcedColumnBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;
	height = m_pLine->getHeight();
}

void fp_ForcedColumnBreakRun::_clearScreen(UT_Bool bFullLineHeightRect)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedColumnBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

fp_ForcedPageBreakRun::fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDPAGEBREAK)
{
	lookupProperties();
}

void fp_ForcedPageBreakRun::lookupProperties(void)
{
}

UT_Bool fp_ForcedPageBreakRun::canContainPoint(void) const
{
	return UT_FALSE;
}

UT_uint32 fp_ForcedPageBreakRun::containsOffset(UT_uint32 iOffset)
{
	return FP_RUN_NOT;
}

UT_Bool fp_ForcedPageBreakRun::canBreakAfter(void) const
{
	return UT_FALSE;
}

UT_Bool fp_ForcedPageBreakRun::canBreakBefore(void) const
{
	return UT_FALSE;
}

UT_Bool	fp_ForcedPageBreakRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

void fp_ForcedPageBreakRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ForcedPageBreakRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	x = xoff;
	y = yoff;
	height = m_pLine->getHeight();
}

void fp_ForcedPageBreakRun::_clearScreen(UT_Bool bFullLineHeightRect)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedPageBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

