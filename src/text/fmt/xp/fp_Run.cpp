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
	m_iLen = iLen;
}

void fp_Run::setBlockOffset(UT_uint32 offset)
{
	m_iOffsetFirst = offset;
}

void fp_Run::clearScreen(void)
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

	_clearScreen();
	
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

fp_TextRun::fp_TextRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TEXT)
{
	m_pFont = NULL;
	m_fDecorations = 0;

	if (bLookupProperties)
	{
		lookupProperties();
	}
}

void fp_TextRun::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst+fl_BLOCK_STRUX_OFFSET,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();
	m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP);

	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP), m_colorFG);

	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP);

	m_fDecorations = 0;

	XML_Char*	p = strdup(pszDecor);
	UT_ASSERT(p || !pszDecor);
	XML_Char*	q = strtok(p, " ");

	while (q)
	{
		if (0 == UT_stricmp(q, "underline"))
		{
			m_fDecorations |= TEXT_DECOR_UNDERLINE;
		}
		else if (0 == UT_stricmp(q, "overline"))
		{
			m_fDecorations |= TEXT_DECOR_OVERLINE;
		}
		else if (0 == UT_stricmp(q, "line-through"))
		{
			m_fDecorations |= TEXT_DECOR_LINETHROUGH;
		}

		q = strtok(NULL, " ");
	}

	free(p);

	m_pG->setFont(m_pFont);
	m_iAscent = m_pG->getFontAscent();	
	m_iDescent = m_pG->getFontDescent();
	m_iHeight = m_pG->getFontHeight();
}

UT_Bool fp_TextRun::canBreakAfter(void) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	UT_Bool bContinue = UT_TRUE;

	if (len > 0)
	{
		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			UT_ASSERT(lenSpan>0);

			if (len <= lenSpan)
			{
				UT_ASSERT(len>0);

				if (pSpan[len-1] == 32)
				{
					return UT_TRUE;
				}

				bContinue = UT_FALSE;
			}
			else
			{
				offset += lenSpan;
				len -= lenSpan;
			}
		}
	}
	else if (!m_pNext)
	{
		return UT_TRUE;
	}

	if (m_pNext)
	{
		return m_pNext->canBreakBefore();
	}
	
	return UT_FALSE;
}

UT_Bool fp_TextRun::canBreakBefore(void) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;

	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(m_iOffsetFirst, &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			if (pSpan[0] == 32)
			{
				return UT_TRUE;
			}
		}
	}
	else
	{
		if (m_pNext)
			return m_pNext->canBreakBefore();
		else
			return UT_TRUE;
	}

	return UT_FALSE;
}

int fp_TextRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(si.iOffset >= (UT_sint32)m_iOffsetFirst);
	UT_ASSERT(si.iOffset < (UT_sint32)(m_iOffsetFirst + m_iLen));
	UT_ASSERT(si.iLeftWidth >= 0);
	UT_ASSERT(si.iRightWidth >= 0);

	clearScreen();
	
	fp_TextRun* pNew = new fp_TextRun(m_pBL, m_pG, si.iOffset+1, m_iLen - (si.iOffset - m_iOffsetFirst) - 1, UT_FALSE);
	UT_ASSERT(pNew);
	pNew->m_pFont = this->m_pFont;
	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;
	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;

	// NOTE (EWS) -- after a split, the new run is dirty, meaning that it does not need to be cleared off screen
	pNew->m_bDirty = UT_TRUE;
	
	pNew->m_iWidth = si.iRightWidth;
	
	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		m_pNext->setPrev(pNew);
	}
	m_pNext = pNew;

	m_iLen = si.iOffset - m_iOffsetFirst + 1;
	m_iWidth = si.iLeftWidth;

	// TODO this is generally called by linebreaker code and we assume that
	// TODO we are going to be on a new line.  therefore, we don't call the
	// TODO line and tell it to add the new run to the current line.  this
	// TODO is probably wrong.
	
	return 1;
}

UT_Bool fp_TextRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(splitOffset >= m_iOffsetFirst);
	UT_ASSERT(splitOffset < (m_iOffsetFirst + m_iLen));
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	
	/*
		NOTE: When inserting a block between these two runs, we need to 
		temporarily "fix" its offset so that the calcwidths calculation
		will work in place within the old block's charwidths buffer. 
	*/
	UT_uint32 offsetNew = (bInsertBlock ? splitOffset + fl_BLOCK_STRUX_OFFSET: splitOffset);

	fp_TextRun* pNew = new fp_TextRun(m_pBL, m_pG, offsetNew, m_iLen - (splitOffset - m_iOffsetFirst), UT_FALSE);
	UT_ASSERT(pNew);
	pNew->m_pFont = this->m_pFont;
	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;
	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;
	
	pNew->m_bDirty = UT_TRUE;
	
	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		m_pNext->setPrev(pNew);
	}
	m_pNext = pNew;

	m_iLen = splitOffset - m_iOffsetFirst;

	/*
		The ordering of the next three lines gets everything 
		positioned properly using the existing primitives, albeit 
		at the cost of more flicker than is theoretically needed. 

		1.  Fix the width of the left run.  This usually shrinks the 
		line, which erases all subsequent runs and moves them left.
		
		2. Insert the new run into the line, creating a properly 
		positioned RunInfo for it.  

		3.  Fix the width of the new run.  This usually kicks 
		subsequent runs to the right.  They've already been erased,
		so it doesn't matter.

		I don't like it, but it seems to work.  PCR
	*/
	calcWidths(pgbCharWidths);
	m_pLine->insertRunAfter(pNew, this);
	pNew->calcWidths(pgbCharWidths);

	// clean up immediately after doing the charwidths calculation 
	if (bInsertBlock)
	{
		pNew->m_iOffsetFirst -= fl_BLOCK_STRUX_OFFSET;
	}

	return UT_TRUE;
}

UT_Bool	fp_TextRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iLeftWidth = 0;
	UT_sint32 iRightWidth = m_iWidth;

	si.iOffset = -1;

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	UT_Bool bContinue = UT_TRUE;

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		if (lenSpan > len)
		{
			lenSpan = len;
		}
		
		for (UT_uint32 i=0; i<lenSpan; i++)
		{
			iLeftWidth += pCharWidths[i + offset];
			iRightWidth -= pCharWidths[i + offset];

			if ((32 == pSpan[i]) || bForce)
			{
				if (iLeftWidth <= iMaxLeftWidth)
				{
					si.iLeftWidth = iLeftWidth;
					si.iRightWidth = iRightWidth;
					si.iOffset = i + offset;
				}
				else
				{
					bContinue = UT_FALSE;
					break;
				}
			}
		}

		if (len <= lenSpan)
		{
			bContinue = UT_FALSE;
		}
		else
		{
			offset += lenSpan;
			len -= lenSpan;
		}
	}

	if ((si.iOffset == -1) || (si.iLeftWidth == m_iWidth))
	{
		// there were no split points which fit.
		return UT_FALSE;
	}

	UT_ASSERT(si.iLeftWidth <= iMaxLeftWidth);
	UT_ASSERT(si.iLeftWidth < m_iWidth);

	return UT_TRUE;
}

UT_Bool fp_TextRun::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	_calcWidths(pgbCharWidths);
	
	// TODO we could be smarter about this
	return UT_TRUE;
}

void fp_TextRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	if  (x <= 0)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst;
		// don't set bBOL to false here
		bEOL = UT_FALSE;
		return;
	}

	if (x >= m_iWidth)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;
		return;
	}

	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	// catch the case of a click directly on the left half of the first character in the run
	if (x < (pCharWidths[m_iOffsetFirst] / 2))
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst;
		bBOL = UT_FALSE;
		bEOL = UT_FALSE;
		return;
	}
	
	UT_sint32 iWidth = 0;
	for (UT_uint32 i=m_iOffsetFirst; i<(m_iOffsetFirst + m_iLen); i++)
	{
		iWidth += pCharWidths[i];
		if (iWidth > x)
		{
			if ((iWidth - x) <= (pCharWidths[i] / 2))
			{
				i++;
			}

			// NOTE: this allows inserted text to be coalesced in the PT
			bEOL = UT_TRUE;

			pos = m_pBL->getPosition() + i;
			return;
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_TextRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	
	m_pLine->getOffsets(this, xoff, yoff);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_uint32 offset = UT_MIN(iOffset, m_iOffsetFirst + m_iLen);

	for (UT_uint32 i=m_iOffsetFirst; i<offset; i++)
	{
		xoff += pCharWidths[i];
	}

	x = xoff;
	y = yoff;
	height = m_iHeight;
}

void fp_TextRun::_calcWidths(UT_GrowBuf * pgbCharWidths)
{
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	UT_Bool bContinue = UT_TRUE;

	clearScreen();
	
	m_iWidth = 0;

	// that's enough for zero-length run
	if (m_iLen == 0)
		return;

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		m_pG->setFont(m_pFont);

		if (len <= lenSpan)
		{
			m_iWidth += m_pG->measureString(pSpan, 0, len, pCharWidths + offset);

			bContinue = UT_FALSE;
		}
		else
		{
			m_iWidth += m_pG->measureString(pSpan, 0, lenSpan, pCharWidths + offset);

			offset += lenSpan;
			len -= lenSpan;
		}
	}

	UT_ASSERT(m_iWidth >= 0);
}

void fp_TextRun::_clearScreen(void)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	
	m_pG->clearArea(xoff, yoff, m_iWidth, m_pLine->getHeight());
}

void fp_TextRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	UT_uint32 iBase = m_pBL->getPosition();

	m_pG->setFont(m_pFont);
	m_pG->setColor(m_colorFG);

	/*
	  TODO this should not be hard-coded.  We should calculate an
	  appropriate selection background color based on the color
	  of the foreground text, probably.
	*/
	UT_RGBColor clrSelBackground(192, 192, 192);

	/*
	  TODO this should not be hard-coded.  We need to figure out
	  what the appropriate background color for this run is, and
	  use that.  Note that it could vary on a run-by-run basis,
	  since document facilities allow the background color to be
	  changed, for things such as table cells.
	*/
	UT_RGBColor clrNormalBackground(255,255,255);

	UT_uint32 iRunBase = iBase + m_iOffsetFirst;

	UT_ASSERT(pDA->iSelPos1 <= pDA->iSelPos2);
	
	if (pDA->iSelPos1 == pDA->iSelPos2)
	{
		// nothing in this run is selected
		_drawPartWithBackground(clrNormalBackground, pDA->xoff, pDA->yoff, m_iOffsetFirst, m_iLen, pgbCharWidths);
	}
	else if (pDA->iSelPos1 <= iRunBase)
	{
		if (pDA->iSelPos2 <= iRunBase)
		{
			// nothing in this run is selected
			_drawPartWithBackground(clrNormalBackground, pDA->xoff, pDA->yoff, m_iOffsetFirst, m_iLen, pgbCharWidths);
		}
		else if (pDA->iSelPos2 >= (iRunBase + m_iLen))
		{
			// the whole run is selected
			
			_drawPartWithBackground(clrSelBackground, pDA->xoff, pDA->yoff, m_iOffsetFirst, m_iLen, pgbCharWidths);
		}
		else
		{
			// the first part is selected, the second part is not

			_drawPartWithBackground(clrSelBackground, pDA->xoff, pDA->yoff, m_iOffsetFirst, pDA->iSelPos2 - iRunBase, pgbCharWidths);

			_drawPartWithBackground(clrNormalBackground, pDA->xoff, pDA->yoff, pDA->iSelPos2 - iBase, m_iLen - (pDA->iSelPos2 - iRunBase), pgbCharWidths);
		}
	}
	else if (pDA->iSelPos1 >= (iRunBase + m_iLen))
	{
		// nothing in this run is selected
		_drawPartWithBackground(clrNormalBackground, pDA->xoff, pDA->yoff, m_iOffsetFirst, m_iLen, pgbCharWidths);
	}
	else
	{
		_drawPartWithBackground(clrNormalBackground, pDA->xoff, pDA->yoff, m_iOffsetFirst, pDA->iSelPos1 - iRunBase, pgbCharWidths);
		
		if (pDA->iSelPos2 >= (iRunBase + m_iLen))
		{
			_drawPartWithBackground(clrSelBackground, pDA->xoff, pDA->yoff, pDA->iSelPos1 - iBase, m_iLen - (pDA->iSelPos1 - iRunBase), pgbCharWidths);
		}
		else
		{
			_drawPartWithBackground(clrSelBackground, pDA->xoff, pDA->yoff, pDA->iSelPos1 - iBase, pDA->iSelPos2 - pDA->iSelPos1, pgbCharWidths);

			_drawPartWithBackground(clrNormalBackground, pDA->xoff, pDA->yoff, pDA->iSelPos2 - iBase, m_iLen - (pDA->iSelPos2 - iRunBase), pgbCharWidths);
		}
	}

	_drawDecors(pDA->xoff, pDA->yoff);

	m_pBL->findSquigglesForRun(this);
}

void fp_TextRun::_drawPartWithBackground(UT_RGBColor& clr, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iPos1, UT_uint32 iLen, const UT_GrowBuf* pgbCharWidths)
{
	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_Rect r;

		_getPartRect(&r, xoff, yoff + m_iAscent, iPos1, iLen, pgbCharWidths);
		r.height = m_pLine->getHeight();
		r.top -= m_pLine->getAscent();
	
		m_pG->fillRect(clr, r.left, r.top, r.width, r.height);
	}

	_drawPart(xoff, yoff, iPos1, iLen, pgbCharWidths);
}

void fp_TextRun::_getPartRect(UT_Rect* pRect, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
						  const UT_GrowBuf * pgbCharWidths)
{
	pRect->left = xoff;
	pRect->top = yoff - m_iAscent;
	pRect->height = m_iHeight;
	pRect->width = 0;

	// that's enough for zero-length run
	if (m_iLen == 0)
		return;

	const UT_uint16 * pCharWidths = pgbCharWidths->getPointer(0);

	UT_uint32 i;
	if (iStart > m_iOffsetFirst)
	{
		for (i=m_iOffsetFirst; i<iStart; i++)
		{
			pRect->left += pCharWidths[i];
		}
	}
	for (i=iStart; i<(iStart + iLen); i++)
	{
		pRect->width += pCharWidths[i];
	}
}

void fp_TextRun::_drawPart(UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
					   const UT_GrowBuf * pgbCharWidths)
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = iStart;
	UT_uint32 len = iLen;
	UT_Bool bContinue = UT_TRUE;

	// don't even try to draw a zero-length run
	if (m_iLen == 0)
		return;

	UT_ASSERT(offset >= m_iOffsetFirst);
	UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);

	UT_uint32 iLeftWidth = 0;
	const UT_uint16 * pCharWidths = pgbCharWidths->getPointer(0);
	
	for (UT_uint32 i=m_iOffsetFirst; i<iStart; i++)
	{
		iLeftWidth += pCharWidths[i];
	}

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		if (len <= lenSpan)
		{
			m_pG->drawChars(pSpan, 0, len, xoff + iLeftWidth, yoff - m_iAscent);

			bContinue = UT_FALSE;
		}
		else
		{
			m_pG->drawChars(pSpan, 0, lenSpan, xoff + iLeftWidth, yoff - m_iAscent);

			for (UT_uint32 i2=offset; i2<offset+lenSpan; i2++)
			{
				iLeftWidth += pCharWidths[i2];
			}

			offset += lenSpan;
			len -= lenSpan;

			UT_ASSERT(offset >= m_iOffsetFirst);
			UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
		}
	}
}

void fp_TextRun::_drawDecors(UT_sint32 xoff, UT_sint32 yoff)
{
	/*
		TODO these decorations have problems.  They're getting drawn
		as hairlines on the printer.  Plus, the underline and strike
		features could be done in the font, at least on Windows they
		can.
	*/
	if (m_fDecorations & TEXT_DECOR_UNDERLINE)
	{
		m_pG->drawLine(xoff, yoff, xoff+getWidth(), yoff);
	}

	if (m_fDecorations & TEXT_DECOR_OVERLINE)
	{
		UT_sint32 y2 = yoff - getAscent();
		m_pG->drawLine(xoff, y2, xoff+getWidth(), y2);
	}

	if (m_fDecorations & TEXT_DECOR_LINETHROUGH)
	{
		UT_sint32 y2 = yoff - getAscent()/3;
		m_pG->drawLine(xoff, y2, xoff+getWidth(), y2);
	}
}

void fp_TextRun::_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right)
{
	if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN)))
	{
		return;
	}
	
	UT_sint32 nPoints = (right - left + 3)/2;
	UT_ASSERT(nPoints > 1);

	/*
		NB: This array gets recopied inside the polyLine implementation
			to move the coordinates into a platform-specific point 
			structure.  They're all x, y but different widths.  Bummer. 
	*/
	UT_Point * points = (UT_Point *)calloc(nPoints, sizeof(UT_Point));
	UT_ASSERT(points);

	points[0].x = left;
	points[0].y = top;

	UT_Bool bTop = UT_FALSE;

	for (UT_sint32 i = 1; i < nPoints; i++, bTop = !bTop)
	{
		points[i].x = points[i-1].x + 2;
		points[i].y = (bTop ? top : top + 2);
	}

	if (points[nPoints-1].x > right)
	{
		points[nPoints-1].x = right;
		points[nPoints-1].y = top + 1;
	}

	m_pG->polyLine(points, nPoints);

	FREEP(points);
}

void fp_TextRun::drawSquiggle(UT_uint32 iOffset, UT_uint32 iLen)
{
	UT_ASSERT(iLen > 0);
	
	UT_sint32 xoff = 0, yoff = 0;

	UT_RGBColor clrSquiggle(255, 0, 0);
	m_pG->setColor(clrSquiggle);
	
	m_pLine->getScreenOffsets(this, xoff, yoff);

	UT_Rect r;
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();  
	_getPartRect( &r, xoff, yoff + m_iAscent, iOffset, iLen, pgbCharWidths);

#if 0
	m_pG->drawLine(r.left,
				   r.top + m_iAscent + 1, 
				   r.left + r.width,
				   r.top + m_iAscent + 1);
#else
	_drawSquiggle(r.top + m_iAscent + 1, r.left, r.left + r.width); 
#endif
}

fp_TabRun::fp_TabRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TAB)
{
	if (bLookupProperties)
	{
		lookupProperties();
	}
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

int fp_TabRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool fp_TabRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool	fp_TabRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

UT_Bool fp_TabRun::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	return UT_TRUE;
}

void fp_TabRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_TabRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
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

void fp_TabRun::_clearScreen(void)
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
	
	UT_uint32 iRunBase = m_pBL->getPosition() + m_iOffsetFirst;
	UT_ASSERT(pDA->iSelPos1 <= pDA->iSelPos2);

	UT_RGBColor clrSelBackground(192, 192, 192);
	UT_RGBColor clrNormalBackground(255,255,255);

	UT_sint32 iFillHeight = m_pLine->getHeight();
	UT_sint32 iFillTop = pDA->yoff - m_pLine->getAscent();
		
	if (
		(pDA->iSelPos1 <= iRunBase)
		&& (pDA->iSelPos2 > iRunBase)
		)
	{
		m_pG->fillRect(clrSelBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
	}
	else
	{
		m_pG->fillRect(clrNormalBackground, pDA->xoff, iFillTop, m_iWidth, iFillHeight);
	}
}

fp_ForcedLineBreakRun::fp_ForcedLineBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDLINEBREAK)
{
	if (bLookupProperties)
	{
		lookupProperties();
	}
}

void fp_ForcedLineBreakRun::lookupProperties(void)
{
}

UT_Bool fp_ForcedLineBreakRun::canBreakAfter(void) const
{
	return UT_FALSE;
}

UT_Bool fp_ForcedLineBreakRun::canBreakBefore(void) const
{
	return UT_FALSE;
}

int fp_ForcedLineBreakRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool fp_ForcedLineBreakRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool	fp_ForcedLineBreakRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

UT_Bool fp_ForcedLineBreakRun::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	return UT_TRUE;
}

void fp_ForcedLineBreakRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ForcedLineBreakRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
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

void fp_ForcedLineBreakRun::_clearScreen(void)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedLineBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

fp_ImageRun::fp_ImageRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, GR_Image* pImage, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_IMAGE)
{
#if 0	// put this back later
	UT_ASSERT(pImage);
#endif
	
	m_pImage = pImage;
	
	if (bLookupProperties)
	{
		lookupProperties();
	}

	m_iAscent = m_iHeight;
	m_iDescent = 0;
}

void fp_ImageRun::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst+fl_BLOCK_STRUX_OFFSET,&pSpanAP);

	const XML_Char *pszWidth = PP_evalProperty("width",pSpanAP,pBlockAP,pSectionAP);
	const XML_Char *pszHeight = PP_evalProperty("height",pSpanAP,pBlockAP,pSectionAP);

	UT_sint32 iWidth = 0;
	UT_sint32 iHeight = 0;
	if (pszWidth && pszHeight)
	{
		iWidth = atoi(pszWidth);
		iHeight = atoi(pszHeight);
	}

	if ((iWidth == 0) || (iHeight == 0))
	{
		UT_sint32 iImageWidth;
		UT_sint32 iImageHeight;
		
		if (m_pImage)
		{
			iImageWidth = m_pImage->getWidth();
			iImageHeight = m_pImage->getHeight();
		}
		else
		{
			iImageWidth = m_pG->convertDimension("1in");
			iImageHeight = m_pG->convertDimension("1in");
		}
		
		if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
		{
			m_iWidth = iImageWidth;
			m_iHeight = iImageHeight;
		}
		else
		{
			double fScale = m_pG->getResolution() / 72.0;
			
			m_iWidth = (UT_sint32) (iImageWidth * fScale);
			m_iHeight = (UT_sint32) (iImageHeight * fScale);
		}
	}
	else
	{
		m_iWidth = iWidth;
		m_iHeight = iHeight;
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

int fp_ImageRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool fp_ImageRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool	fp_ImageRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

UT_Bool fp_ImageRun::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	// TODO
	return UT_FALSE;
}

void fp_ImageRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ImageRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
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

void fp_ImageRun::_clearScreen(void)
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
		m_pG->drawImage(m_pImage, xoff, yoff, m_iWidth, m_iHeight);
	}
	else
	{
		UT_RGBColor clr(0, 0, 255);
		m_pG->fillRect(clr, xoff, yoff, m_iWidth, m_iHeight);
	}
}

fp_FieldRun::fp_FieldRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FIELD)
{
	m_pFont = NULL;

	m_sFieldValue[0] = 0;
	
	if (bLookupProperties)
	{
		lookupProperties();
	}

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

		return calcWidths(NULL);
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

int fp_FieldRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool fp_FieldRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool	fp_FieldRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

UT_Bool fp_FieldRun::calcWidths(UT_GrowBuf * pgbCharWidths)
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

void fp_FieldRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_FieldRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
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

void fp_FieldRun::_clearScreen(void)
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
		UT_ASSERT(pDA->iSelPos1 <= pDA->iSelPos2);

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
		
		if (
			(pDA->iSelPos1 <= iRunBase)
			&& (pDA->iSelPos2 > iRunBase)
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

fp_ForcedColumnBreakRun::fp_ForcedColumnBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDCOLUMNBREAK)
{
	if (bLookupProperties)
	{
		lookupProperties();
	}
}

void fp_ForcedColumnBreakRun::lookupProperties(void)
{
}

UT_Bool fp_ForcedColumnBreakRun::canBreakAfter(void) const
{
	return UT_FALSE;
}

UT_Bool fp_ForcedColumnBreakRun::canBreakBefore(void) const
{
	return UT_FALSE;
}

int fp_ForcedColumnBreakRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool fp_ForcedColumnBreakRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool	fp_ForcedColumnBreakRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

UT_Bool fp_ForcedColumnBreakRun::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	return UT_TRUE;
}

void fp_ForcedColumnBreakRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ForcedColumnBreakRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
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

void fp_ForcedColumnBreakRun::_clearScreen(void)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedColumnBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

fp_ForcedPageBreakRun::fp_ForcedPageBreakRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_FORCEDPAGEBREAK)
{
	if (bLookupProperties)
	{
		lookupProperties();
	}
}

void fp_ForcedPageBreakRun::lookupProperties(void)
{
}

UT_Bool fp_ForcedPageBreakRun::canBreakAfter(void) const
{
	return UT_FALSE;
}

UT_Bool fp_ForcedPageBreakRun::canBreakBefore(void) const
{
	return UT_FALSE;
}

int fp_ForcedPageBreakRun::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool fp_ForcedPageBreakRun::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

UT_Bool	fp_ForcedPageBreakRun::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	return UT_FALSE;
}

UT_Bool fp_ForcedPageBreakRun::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	return UT_TRUE;
}

void fp_ForcedPageBreakRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	pos = m_pBL->getPosition() + m_iOffsetFirst;
	bBOL = UT_FALSE;
	bEOL = UT_FALSE;
}

void fp_ForcedPageBreakRun::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
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

void fp_ForcedPageBreakRun::_clearScreen(void)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));
}

void fp_ForcedPageBreakRun::_draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
}

