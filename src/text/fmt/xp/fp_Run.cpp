 
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


#include <stdlib.h>
#include <string.h>

#include "fp_Run.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "pp_Property.h"
#include "dg_Graphics.h"
#include "pd_Document.h"
#include "dg_DrawArgs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"


fp_Run::fp_Run(fl_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties)
{
	m_pG = pG;
	m_pBL = pBL;

	m_bCanSplit = 1;
	m_iLineBreakBefore = BREAK_AUTO;
	m_iLineBreakAfter = BREAK_AUTO;

	m_iOffsetFirst = iOffsetFirst;
	m_iLen = iLen;
	m_iWidth = 0;
	m_iHeight = 0;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pLine = NULL;
	m_pLineData = NULL;
	m_iExtraWidth = 0;
	m_pFont = NULL;
	m_fDecorations = 0;

	if (bLookupProperties)
		lookupProperties();
}

void fp_Run::setLine(fp_Line* pLine, void* p)
{
	m_pLine = pLine;
	m_pLineData = p;
}

fp_Line* fp_Run::getLine() const
{
	return m_pLine;
}

void* fp_Run::getLineData()
{
	return m_pLineData;
}

fl_BlockLayout* fp_Run::getBlock() const
{
	return m_pBL;
}

DG_Graphics* fp_Run::getGraphics() const
{
	return m_pG;
}

UT_uint32	fp_Run::getHeight() const
{
	return m_iHeight;
}

UT_uint32	fp_Run::getWidth() const
{
	return m_iWidth;
}

UT_uint32 fp_Run::getAscent()
{
	return m_iAscent;
}

UT_uint32 fp_Run::getDescent()
{
	return m_iDescent;
}

UT_uint32 fp_Run::getLength() const
{
	return m_iLen;
}

UT_uint32 fp_Run::getBlockOffset() const
{
	return m_iOffsetFirst;
}

void fp_Run::lookupProperties(void)
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst+fl_BLOCK_STRUX_OFFSET,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getLayout();
	m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP),

	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP), m_colorFG);

	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP);

	/*
	  TODO -- m_fDecorations supports multiple simultanous decors.  Unfortunately,
	  our use of UT_stricmp only allows one.
	*/
	
	m_fDecorations = 0;
	if (0 == UT_stricmp(pszDecor, "underline"))
	{
		m_fDecorations |= TEXT_DECOR_UNDERLINE;
	}
	if (0 == UT_stricmp(pszDecor, "overline"))
	{
		m_fDecorations |= TEXT_DECOR_OVERLINE;
	}
	if (0 == UT_stricmp(pszDecor, "line-through"))
	{
		m_fDecorations |= TEXT_DECOR_LINETHROUGH;
	}

	m_pG->setFont(m_pFont);
	m_iAscent = m_pG->getFontAscent();	
	m_iDescent = m_pG->getFontDescent();
	m_iHeight = m_pG->getFontHeight();
}

void fp_Run::setNext(fp_Run* p)
{
	m_pNext = p;
}

void fp_Run::setPrev(fp_Run* p)
{
	m_pPrev = p;
}

fp_Run* fp_Run::getPrev() const
{
	return m_pPrev;
}

fp_Run* fp_Run::getNext() const
{
	return m_pNext;
}

UT_Bool fp_Run::canSplit() const
{
	return m_bCanSplit;
}

UT_Bool fp_Run::canBreakAfter() const
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

UT_Bool fp_Run::canBreakBefore() const
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

UT_Bool fp_Run::getLineBreakBefore() const
{
	return m_iLineBreakBefore;
}

UT_Bool fp_Run::getLineBreakAfter() const
{
	return m_iLineBreakAfter;
}

int fp_Run::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(si.iOffset >= (UT_sint32)m_iOffsetFirst);
	UT_ASSERT(si.iOffset < (UT_sint32)(m_iOffsetFirst + m_iLen));

	fp_Run* pNew = new fp_Run(m_pBL, m_pG, si.iOffset+1, m_iLen - (si.iOffset - m_iOffsetFirst) - 1, UT_FALSE);
	UT_ASSERT(pNew);
	pNew->m_pFont = this->m_pFont;
	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;
	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;
	
	pNew->m_iWidth = si.iRightWidth;
	
	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		m_pNext->m_pPrev = pNew;
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

UT_Bool fp_Run::split(UT_uint32 splitOffset, UT_Bool bInsertBlock)
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

	fp_Run* pNew = new fp_Run(m_pBL, m_pG, offsetNew, m_iLen - (splitOffset - m_iOffsetFirst), UT_FALSE);
	UT_ASSERT(pNew);
	pNew->m_pFont = this->m_pFont;
	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;
	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;
	
	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		m_pNext->m_pPrev = pNew;
	}
	m_pNext = pNew;

	m_pLine->splitRunInLine(this,pNew);
	m_iLen = splitOffset - m_iOffsetFirst;
	
	calcWidths(pgbCharWidths);
	pNew->calcWidths(pgbCharWidths);

	// clean up immediately after doing the charwidths calculation 
	if (bInsertBlock)
		pNew->m_iOffsetFirst -= fl_BLOCK_STRUX_OFFSET;

	// TODO who deals with iLineBreak{Before,After},bCanSplit,iExtraWidth,etc...
	
	return UT_TRUE;
}

UT_Bool	fp_Run::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si)
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

		for (UT_uint32 i=0; i<lenSpan; i++)
		{
			iLeftWidth += pCharWidths[i + offset];
			iRightWidth -= pCharWidths[i + offset];

			if (32 == pSpan[i])
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

UT_Bool	fp_Run::findMinLeftFitSplitPoint(fp_RunSplitInfo& si)
{
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

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

		for (UT_uint32 i=0; i<lenSpan; i++)
		{
			iLeftWidth += pCharWidths[i + offset];
			iRightWidth -= pCharWidths[i + offset];

			
			if (32 == pSpan[i])					// TODO isn't this a bit english specific ??
			{
				si.iLeftWidth = iLeftWidth;
				si.iRightWidth = iRightWidth;
				si.iOffset = i + offset;
				bContinue = UT_FALSE;
				break;
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

	UT_ASSERT(si.iLeftWidth < m_iWidth);

	return UT_TRUE;
}

void fp_Run::_calcWidths(UT_GrowBuf * pgbCharWidths)
{
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	UT_Bool bContinue = UT_TRUE;

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
}

void fp_Run::calcWidths(UT_GrowBuf * pgbCharWidths)
{
	UT_sint32 iOldWidth = m_iWidth;

	_calcWidths(pgbCharWidths);
	
	// let our parent know that we are changing underneath them ...
	if (m_pLine)
		m_pLine->runSizeChanged(m_pLineData, iOldWidth, m_iWidth);
}

void fp_Run::expandWidthTo(UT_uint32 iNewWidth)
{
#if 0	// this does not work yet
	m_iExtraWidth = iNewWidth - m_iWidth;
	m_iWidth = iNewWidth;
#endif
}

void fp_Run::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bEOL)
{
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	if  (x <= 0)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst;
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

void fp_Run::getOffsets(UT_uint32& xoff, UT_uint32& yoff)
{
	UT_uint32 h;
	
	findPointCoords(m_iOffsetFirst, xoff, yoff, h);
	yoff += m_iAscent;
}

/*
  TODO -- this is kind of a hack.  It stems from a problem we have
  with insertion points.  Namely, the position of an insertion
  point can be ambiguous if it is only expressed in terms of a buffer
  offset.
*/
UT_uint32 fp_Run::containsOffset(UT_uint32 iOffset)
{
	/*
	  Seems like I keep changing this.  the second comparison below is
	  < now, because the selection code keeps finding the wrong run
	  when searching for the insertion point.
	*/
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

void fp_Run::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
{
	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	
	UT_sint32 xoff;
	UT_sint32 yoff;

	m_pLine->getOffsets(this, m_pLineData, xoff, yoff);
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

void fp_Run::clearScreen(void)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0, width, height;
	
	// need to clear full height of line, in case we had a selection
	m_pLine->getScreenOffsets(this, m_pLineData, xoff, yoff, width, height, UT_TRUE);
	m_pG->clearArea(xoff, yoff, m_iWidth, height);
}

void fp_Run::invert(UT_uint32 iStart, UT_uint32 iLen)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0, width, height;
	
	m_pLine->getScreenOffsets(this, m_pLineData, xoff, yoff, width, height, UT_TRUE);

	UT_Rect r;

	_getPartRect(&r, xoff, yoff + m_iAscent, iStart, iLen, m_pBL->getCharWidths());

	// inverting to line height avoids "staggered" selections
	r.height = height;
	m_pG->invertRect(&r);
}

void fp_Run::draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths();

	m_pG->setFont(m_pFont);
	m_pG->setColor(m_colorFG);
	_drawPart(pDA->xoff, pDA->yoff, m_iOffsetFirst, m_iLen, pgbCharWidths);

	_drawDecors(pDA->xoff, pDA->yoff);
}

void fp_Run::_getPartRect(UT_Rect* pRect, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
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

void fp_Run::_drawPart(UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
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

			for (UT_uint32 i=offset; i<offset+lenSpan; i++)
			{
				iLeftWidth += pCharWidths[i];
			}

			offset += lenSpan;
			len -= lenSpan;

			UT_ASSERT(offset >= m_iOffsetFirst);
			UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
		}
	}
}

void fp_Run::_drawDecors(UT_sint32 xoff, UT_sint32 yoff)
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

void fp_Run::dumpRun(void) const
{
	UT_DEBUGMSG(("Run: 0x%08lx offset %8d length %4d  width=%d\n",this,m_iOffsetFirst,m_iLen,m_iWidth));
	m_pLine->dumpRunInfo(this, m_pLineData);
	
	return;
}

UT_Bool fp_Run::ins(UT_uint32 iOffset, UT_uint32 iCount, PT_AttrPropIndex indexAP)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	if ((m_iOffsetFirst + m_iLen) < iOffset)
	{
		// nothing to do.  the insert occurred AFTER this run
		return UT_FALSE;
	}

	if ((m_iOffsetFirst > iOffset) || 
		((m_iOffsetFirst == iOffset) && (iOffset > 0)))
	{
		m_iOffsetFirst += iCount;
		return UT_FALSE;
	}

	m_iLen += iCount;
	
	return UT_TRUE;
}

UT_Bool fp_Run::del(UT_uint32 iOffset, UT_uint32 iCount)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
	if ((m_iOffsetFirst + m_iLen) <= iOffset)
	{
		// nothing to do.  the insert occurred AFTER this run
		return UT_FALSE;
	}

	if (m_iOffsetFirst >= (iOffset + iCount))
	{
		// the insert occurred entirely before this run.
		
		m_iOffsetFirst -= iCount;
		return UT_FALSE;
	}

	clearScreen();
	
	if (iOffset >= m_iOffsetFirst)
	{
		if ((iOffset + iCount) < (m_iOffsetFirst + m_iLen))
		{
			// the deleted section is entirely within this run
			m_iLen -= iCount;
		}
		else
		{
			int iDeleted = m_iOffsetFirst + m_iLen - iOffset;
			UT_ASSERT(iDeleted > 0);
			m_iLen -= iDeleted;
		}
	}
	else
	{
		if ((iOffset + iCount) < (m_iOffsetFirst + m_iLen))
		{
			int iDeleted = iOffset + iCount - m_iOffsetFirst;
			UT_ASSERT(iDeleted > 0);
			m_iOffsetFirst -= (iCount - iDeleted);
			m_iLen -= iDeleted;
		}
		else
		{
			/*
			  the deletion spans the entire run.  we set its
			  length, in chars, to zero.  it will be deleted.
			*/
			
			m_iLen = 0;
		}
	}
	
	return UT_TRUE;
}
