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

#include "fp_TextRun.h"
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

fp_TextRun::fp_TextRun(fl_BlockLayout* pBL, GR_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties) : fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TEXT)
{
	m_pFont = NULL;
	m_fDecorations = 0;
	m_iLineWidth = 0;

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

	/*
	  TODO map line width to a property, not a hard-coded value
	*/
	m_iLineWidth = m_pG->convertDimension("0.8pt");
	
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
		{
			return m_pNext->canBreakBefore();
		}
		else
		{
			return UT_TRUE;
		}
	}

	return UT_FALSE;
}

UT_Bool fp_TextRun::alwaysFits(void) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;

	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(m_iOffsetFirst, &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			for (UT_uint32 i=0; i<lenSpan; i++)
			{
				if (pSpan[i] != 32)
				{
					return UT_FALSE;
				}
			}

			return UT_TRUE;
		}

		return UT_FALSE;
	}
	else
	{
		// could assert here -- this should never happen, I think
		return UT_TRUE;
	}

	return UT_TRUE;
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

UT_Bool fp_TextRun::splitSimple(UT_uint32 splitOffset)
{
	UT_ASSERT(splitOffset >= m_iOffsetFirst);
	UT_ASSERT(splitOffset < (m_iOffsetFirst + m_iLen));
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	
	/*
		NOTE: When inserting a block between these two runs, we need to 
		temporarily "fix" its offset so that the calcwidths calculation
		will work in place within the old block's charwidths buffer. 
	*/
	UT_uint32 offsetNew = splitOffset;

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
//	pNew->calcWidths(pgbCharWidths);

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

void fp_TextRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
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
#if 1
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	UT_sint32 xoff = 0, yoff = 0;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, xoff, yoff);
	
	m_pG->clearArea(xoff, yoff, m_iWidth, m_pLine->getHeight());
#else
	/*
	  The following code is an attempt to fix bug 7, but
	  drawing the chars doesn't work, since the piece table
	  has already been updated, so the chars are no longer
	  available for drawing.  :-(
	*/
	
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	m_pG->setFont(m_pFont);
	
	UT_RGBColor clrNormalBackground(255,255,255);
	m_pG->setColor(clrNormalBackground);
	
	UT_sint32 xoff = 0, yoff = 0;
	m_pLine->getScreenOffsets(this, xoff, yoff);
	yoff += m_pLine->getAscent();
	
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	_drawPartWithBackground(clrNormalBackground, xoff, yoff, m_iOffsetFirst, m_iLen, pgbCharWidths);
#endif	
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
	  TODO I *think* this line width should be proportional
	  to the size of the font.
	*/
	m_pG->setLineWidth(m_iLineWidth);
	
	if (m_fDecorations & TEXT_DECOR_UNDERLINE)
	{
		UT_sint32 iDrop = (m_pLine->getDescent() / 3);
		m_pG->drawLine(xoff, yoff + iDrop, xoff+getWidth(), yoff + iDrop);
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
	UT_sint32 iAscent = m_pLine->getAscent();

	UT_RGBColor clrSquiggle(255, 0, 0);
	m_pG->setColor(clrSquiggle);
	
	m_pLine->getScreenOffsets(this, xoff, yoff);

	UT_Rect r;
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();  
	_getPartRect( &r, xoff, yoff + m_iAscent, iOffset, iLen, pgbCharWidths);

	_drawSquiggle(r.top + iAscent + 1, r.left, r.left + r.width); 
}

