 
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
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "dg_Property.h"
#include "dg_Graphics.h"
#include "dg_DocBuffer.h"
#include "dg_DrawArgs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"

FP_Run::FP_Run(FL_BlockLayout* pBL, DG_Graphics* pG, UT_uint32 iOffsetFirst, UT_uint32 iLen, UT_Bool bLookupProperties)
{
	m_pG = pG;
	m_pBL = pBL;

	m_pBuffer = m_pBL->getBuffer();

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

void FP_Run::setLine(FP_Line* pLine, void* p)
{
	m_pLine = pLine;
	m_pLineData = p;
}

FP_Line* FP_Run::getLine() const
{
	return m_pLine;
}

void* FP_Run::getLineData()
{
	return m_pLineData;
}

FL_BlockLayout* FP_Run::getBlock() const
{
	return m_pBL;
}

DG_Graphics* FP_Run::getGraphics() const
{
	return m_pG;
}

UT_uint32	FP_Run::getHeight() const
{
	return m_iHeight;
}

UT_uint32	FP_Run::getWidth() const
{
	return m_iWidth;
}

UT_uint32 FP_Run::getAscent()
{
	return m_iAscent;
}

UT_uint32 FP_Run::getDescent()
{
	return m_iDescent;
}

UT_uint32 FP_Run::getLength() const
{
	return m_iLen;
}

UT_uint32 FP_Run::getFirstPosition() const
{
	return m_iOffsetFirst;
}

void FP_Run::lookupProperties(void)
{
	UT_uint32 iAbsolute = m_pBL->getBufferAddress() + m_iOffsetFirst;
	UT_uint32 posMarker;
	DG_DocMarkerId idMarker;
	DG_DocMarker* pMarker = NULL;
	UT_Bool bResult;

	bResult = m_pBuffer->findMarker(UT_FALSE, iAbsolute, &posMarker, &idMarker, &pMarker);
	UT_ASSERT(bResult);
	UT_ASSERT(pMarker);
	if (pMarker->getType() & DG_MT_END)
	{
		pMarker = pMarker->getParent();
	}

	/*
	  TODO -- note that we currently assume font-family to be a single name,
	  not a list.  This is broken.
	*/

	m_pFont = m_pG->findFont(pMarker->getProperty(lookupProperty("font-family")),
							 pMarker->getProperty(lookupProperty("font-style")),
							 pMarker->getProperty(lookupProperty("font-variant")),
							 pMarker->getProperty(lookupProperty("font-weight")),
							 pMarker->getProperty(lookupProperty("font-stretch")),
							 pMarker->getProperty(lookupProperty("font-size")));

	UT_parseColor(pMarker->getProperty(lookupProperty("color")), m_colorFG);

	const char *pszDecor = pMarker->getProperty(lookupProperty("text-decoration"));
	/*
	  m_fDecorations supports multiple simultanous decors.  Unfortunately,
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

void FP_Run::setNext(FP_Run* p)
{
	m_pNext = p;
}

void FP_Run::setPrev(FP_Run* p)
{
	m_pPrev = p;
}

FP_Run* FP_Run::getPrev() const
{
	return m_pPrev;
}

FP_Run* FP_Run::getNext() const
{
	return m_pNext;
}

UT_Bool FP_Run::canSplit() const
{
	return m_bCanSplit;
}

UT_Bool FP_Run::canBreakAfter() const
{
	const UT_UCSChar* p1;
	UT_uint32   iLen1;
	const UT_UCSChar* p2;
	UT_uint32	iLen2;
	
	m_pBL->fetchPointers(m_iOffsetFirst, m_iLen, &p1, &iLen1, &p2, &iLen2);
	if (p2)
	{
		if (p2[iLen2-1] == 32)
		{
			return UT_TRUE;
		}
	}
	else
	{
		UT_ASSERT(iLen1 == m_iLen);
		if (p1[iLen1-1] == 32)
		{
			return UT_TRUE;
		}
	}

	if (m_pNext)
	{
		return m_pNext->canBreakBefore();
	}
	
	return UT_FALSE;
}

UT_Bool FP_Run::canBreakBefore() const
{
	const UT_UCSChar* p1;
	UT_uint32   iLen1;
	const UT_UCSChar* p2;
	UT_uint32	iLen2;
	
	m_pBL->fetchPointers(m_iOffsetFirst, m_iLen, &p1, &iLen1, &p2, &iLen2);
	if (p1[0] == 32)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool FP_Run::getLineBreakBefore() const
{
	return m_iLineBreakBefore;
}

UT_Bool FP_Run::getLineBreakAfter() const
{
	return m_iLineBreakAfter;
}

int FP_Run::split(fp_RunSplitInfo& si)
{
	UT_ASSERT(si.iOffset >= (UT_sint32)m_iOffsetFirst);
	UT_ASSERT(si.iOffset < (UT_sint32)(m_iOffsetFirst + m_iLen));

	FP_Run* pNew = new FP_Run(m_pBL, m_pG, si.iOffset+1, m_iLen - (si.iOffset - m_iOffsetFirst) - 1, UT_FALSE);
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

UT_Bool FP_Run::split(UT_uint32 splitOffset)
{
	UT_ASSERT(splitOffset >= m_iOffsetFirst);
	UT_ASSERT(splitOffset < (m_iOffsetFirst + m_iLen));
    UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	FP_Run* pNew = new FP_Run(m_pBL, m_pG, splitOffset, m_iLen - (splitOffset - m_iOffsetFirst), UT_FALSE);
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
	
	calcWidths(pCharWidths);
	pNew->calcWidths(pCharWidths);
	
	// TODO who deals with iLineBreak{Before,After},bCanSplit,iExtraWidth,etc...
	
	return UT_TRUE;
}

UT_Bool	FP_Run::findMaxLeftFitSplitPoint(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si)
{
    UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	UT_sint32 iLeftWidth = 0;
	UT_sint32 iRightWidth = m_iWidth;

	si.iOffset = -1;

	const UT_UCSChar* p1;
	UT_uint32   iLen1;
	const UT_UCSChar* p2;
	UT_uint32	iLen2;

	m_pBL->fetchPointers(m_iOffsetFirst, m_iLen, &p1, &iLen1, &p2, &iLen2);
	UT_UCSChar ch;
	
	for (UT_uint32 i=0; i<(iLen1 + iLen2); i++)
	{
		iLeftWidth += pCharWidths[i + m_iOffsetFirst];
		iRightWidth -= pCharWidths[i + m_iOffsetFirst];

		if (i < iLen1)
		{
			ch = p1[i];
		}
		else
		{
			ch = p2[i - iLen1];
		}
		
		if (32 == ch)
		{
			if (iLeftWidth <= iMaxLeftWidth)
			{
				si.iLeftWidth = iLeftWidth;
				si.iRightWidth = iRightWidth;
				si.iOffset = i + m_iOffsetFirst;
			}
			else
			{
			    break;
			}
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

UT_Bool	FP_Run::findMinLeftFitSplitPoint(fp_RunSplitInfo& si)
{
    UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	UT_sint32 iLeftWidth = 0;
	UT_sint32 iRightWidth = m_iWidth;

	si.iOffset = -1;

	const UT_UCSChar* p1;
	UT_uint32   iLen1;
	const UT_UCSChar* p2;
	UT_uint32	iLen2;

	m_pBL->fetchPointers(m_iOffsetFirst, m_iLen, &p1, &iLen1, &p2, &iLen2);
	UT_UCSChar ch;
	
	for (UT_uint32 i=0; i<(iLen1 + iLen2); i++)
	{
		iLeftWidth += pCharWidths[i + m_iOffsetFirst];
		iRightWidth -= pCharWidths[i + m_iOffsetFirst];

		if (i < iLen1)
		{
			ch = p1[i];
		}
		else
		{
			ch = p2[i - iLen1];
		}
		
		if (32 == ch)
		{
			si.iLeftWidth = iLeftWidth;
			si.iRightWidth = iRightWidth;
			si.iOffset = i + m_iOffsetFirst;
			break;
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

void FP_Run::_calcWidths(UT_uint16* pCharWidths)
{
	const UT_UCSChar* p1;
	UT_uint32   iLen1;
	const UT_UCSChar* p2;
	UT_uint32	iLen2;
	
	m_pBL->fetchPointers(m_iOffsetFirst, m_iLen, &p1, &iLen1, &p2, &iLen2);

	m_pG->setFont(m_pFont);
    m_iWidth = m_pG->measureString(p1, 0, iLen1, pCharWidths + m_iOffsetFirst);
	if (p2)
	{
		m_iWidth += m_pG->measureString(p2, 0, iLen2, pCharWidths + m_iOffsetFirst + iLen1);
	}
}

void FP_Run::calcWidths(UT_uint16* pCharWidths)
{
	UT_sint32 iOldWidth = m_iWidth;

	_calcWidths(pCharWidths);
	
	// let our parent know that we are changing underneath them ...
	if (m_pLine)
		m_pLine->runSizeChanged(m_pLineData, iOldWidth, m_iWidth);
}

void FP_Run::expandWidthTo(UT_uint32 iNewWidth)
{
#if 0	// this does not work yet
	m_iExtraWidth = iNewWidth - m_iWidth;
	m_iWidth = iNewWidth;
#endif
}

void FP_Run::mapXYToBufferPosition(UT_sint32 x, UT_sint32 y, UT_uint32& pos, UT_Bool& bRight)
{
    UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	if  (x <= 0)
	{
		pos = m_pBL->getBufferAddress() + m_iOffsetFirst;
		bRight = UT_TRUE;
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

			if (i == (m_iOffsetFirst + m_iLen))
			{
				bRight = UT_FALSE;
			}
			else
			{
				bRight = UT_TRUE;
			}
			
			pos = m_pBL->getBufferAddress() + i;
			return;
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void FP_Run::getOffsets(UT_uint32& xoff, UT_uint32& yoff)
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
UT_uint32 FP_Run::containsOffset(UT_uint32 iOffset)
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

void FP_Run::findPointCoords(UT_uint32 iOffset, UT_uint32& x, UT_uint32& y, UT_uint32& height)
{
	// UT_ASSERT(FP_RUN_INSIDE == containsOffset(iOffset));
	
	UT_sint32 xoff;
	UT_sint32 yoff;

	m_pLine->getOffsets(this, m_pLineData, xoff, yoff);
    UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	for (UT_uint32 i=m_iOffsetFirst; i<iOffset; i++)
	{
		xoff += pCharWidths[i];
	}

	x = xoff;
	y = yoff;
	height = m_iHeight;
}

UT_Bool FP_Run::insertData(UT_uint32 iOffset, UT_uint32 iCount)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	if ((m_iOffsetFirst + m_iLen) < iOffset)
	{
		// nothing to do.  the insert occurred AFTER this run
		return UT_FALSE;
	}

	if (m_iOffsetFirst > iOffset)
	{
		m_iOffsetFirst += iCount;
		return UT_FALSE;
	}

	m_iLen += iCount;
	
	return UT_TRUE;
}

UT_Bool FP_Run::deleteChars(UT_uint32 iOffset, UT_uint32 iCountUnits, UT_uint32 iCountChars)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_ASSERT(iCountChars <= iCountUnits);
	UT_uint32 iOldWidth = getWidth();
	
	if ((m_iOffsetFirst + m_iLen) <= iOffset)
	{
		// nothing to do.  the insert occurred AFTER this run
		return UT_FALSE;
	}

	if (m_iOffsetFirst >= (iOffset + iCountUnits))
	{
		// the insert occurred entirely before this run.
		
		m_iOffsetFirst -= iCountChars;
		return UT_FALSE;
	}

	clearScreen();
	
	if (iOffset >= m_iOffsetFirst)
	{
		if ((iOffset + iCountUnits) < (m_iOffsetFirst + m_iLen))
		{
			// the deleted section is entirely within this run
			UT_ASSERT(iCountUnits == iCountChars);
			
			m_iLen -= iCountUnits;
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
		if ((iOffset + iCountUnits) < (m_iOffsetFirst + m_iLen))
		{
			int iDeleted = iOffset + iCountUnits - m_iOffsetFirst;
			UT_ASSERT(iDeleted > 0);
			m_iOffsetFirst -= (iCountChars - iDeleted);
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

UT_Bool FP_Run::insertInlineMarker(UT_uint32 newMarkerOffset, UT_uint32 markerSize)
{
	if (newMarkerOffset <= m_iOffsetFirst)
	{
		// insert occured before (or at the begining of) this run.
		// we need to update the offset in this run.

		m_iOffsetFirst += markerSize;
		return UT_TRUE;
	}
	else if (m_iOffsetFirst+m_iLen <= newMarkerOffset)
	{
		// insert occured after this run, we don't need to bother it.
		return UT_TRUE;
	}
	else
	{
		// insert occured inside this run, we need to split it.
		// we will let our caller update the new marker that we create.
		split(newMarkerOffset);
		return UT_TRUE;
	}
}

void FP_Run::clearScreen(void)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0, width, height;
	
	m_pLine->getScreenOffsets(this, m_pLineData, xoff, yoff, width, height);
	m_pG->clearArea(xoff, yoff, m_iWidth, m_iHeight);
}

void FP_Run::invert(UT_uint32 iStart, UT_uint32 iLen)
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
	UT_sint32 xoff = 0, yoff = 0, width, height;
	
	m_pLine->getScreenOffsets(this, m_pLineData, xoff, yoff, width, height);

	UT_Rect r;
	UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	_getPartRect(&r, xoff, yoff + m_iAscent, iStart, iLen, pCharWidths);
	m_pG->invertRect(&r);
}

void FP_Run::draw(dg_DrawArgs* pDA)
{
	UT_ASSERT(pDA->pG == m_pG);
	
	UT_uint32 iBlockBase = m_pBL->getBufferAddress();
	
	const UT_UCSChar* p1;
	UT_uint32   iLen1;
	const UT_UCSChar* p2;
	UT_uint32	iLen2;
	
	m_pBL->fetchPointers(m_iOffsetFirst, m_iLen, &p1, &iLen1, &p2, &iLen2);

	UT_uint16* pCharWidths = m_pBL->getCharWidthArray();

	m_pG->setFont(m_pFont);
	m_pG->setColor(m_colorFG);
	_drawPart(pDA->xoff, pDA->yoff, m_iOffsetFirst, m_iLen, p1, iLen1, p2, iLen2, pCharWidths);

	_drawDecors(pDA->xoff, pDA->yoff);
}

void FP_Run::_getPartRect(UT_Rect* pRect, UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
						  const UT_uint16* pCharWidths)
{
	pRect->left = xoff;
	pRect->top = yoff - m_iAscent;
	pRect->height = m_iHeight;
	pRect->width = 0;

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

void FP_Run::_drawPart(UT_sint32 xoff, UT_sint32 yoff, UT_uint32 iStart, UT_uint32 iLen,
					   const UT_UCSChar* p1,
					   UT_uint32 iLen1,
					   const UT_UCSChar* p2,
					   UT_uint32 iLen2,
					   const UT_uint16* pCharWidths)
{
	if (iStart < (m_iOffsetFirst + iLen1))
	{
		UT_uint32 iLenDraw;
		if ((iStart + iLen) > (m_iOffsetFirst + iLen1))
		{
			iLenDraw = m_iOffsetFirst + iLen1 - iStart;
		}
		else
		{
			iLenDraw = iLen;
		}
		
		UT_uint32 iLeftWidth = 0;
		
		for (UT_uint32 i=m_iOffsetFirst; i<iStart; i++)
		{
			iLeftWidth += pCharWidths[i];
		}
		
		m_pG->drawChars(p1, 
						iStart - m_iOffsetFirst, 
						iLenDraw,
						xoff + iLeftWidth,
						yoff);

	}

	if ((iStart + iLen) > (m_iOffsetFirst + iLen1))
	{
		UT_ASSERT(p2);
		UT_uint32 iLeftWidth = 0;
		
		for (UT_uint32 i=0; i<iLen1; i++)
		{
			iLeftWidth += pCharWidths[i + m_iOffsetFirst];
		}

		UT_uint32 iStartDraw;
		UT_uint32 iLenDraw;
		if (iStart > (m_iOffsetFirst + iLen1))
		{
			for (UT_uint32 i=m_iOffsetFirst + iLen1; i<iStart; i++)
			{
				iLeftWidth += pCharWidths[i];
			}

			iStartDraw = iStart - iLen1 - m_iOffsetFirst;
			iLenDraw = iLen;
		}
		else
		{
			iStartDraw = 0;
			iLenDraw = iStart + iLen - (m_iOffsetFirst + iLen1);
		}

		m_pG->drawChars(p2, iStartDraw, iLenDraw, xoff + iLeftWidth, yoff);
	}
}

void FP_Run::_drawDecors(UT_sint32 xoff, UT_sint32 yoff)
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

void FP_Run::dumpRun(void) const
{
	UT_DEBUGMSG(("Run: 0x%08lx offset %8d length %4d  width=%d\n",this,m_iOffsetFirst,m_iLen,m_iWidth));
	m_pLine->dumpRunInfo(this, m_pLineData);
	
	return;
}
