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
#include "fv_View.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_units.h"

/*****************************************************************/

fp_TextRun::fp_TextRun(fl_BlockLayout* pBL,
					   GR_Graphics* pG,
					   UT_uint32 iOffsetFirst,
					   UT_uint32 iLen,
					   UT_Bool bLookupProperties)
:	fp_Run(pBL, pG, iOffsetFirst, iLen, FPRUN_TEXT),
	m_fPosition(TEXT_POSITION_NORMAL)
{
	m_pFont = NULL;
	m_pFontLayout = NULL;
	m_fDecorations = 0;
	m_iLineWidth = 0;
	m_bSquiggled = UT_FALSE;
	m_iSpaceWidthBeforeJustification = JUSTIFICATION_NOT_USED;

	if (bLookupProperties)
	{
		lookupProperties();
	}
}

fp_TextRun::~fp_TextRun()
{
}

void fp_TextRun::lookupProperties(void)
{
	clearScreen();
	
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	m_pBL->getSpanAttrProp(m_iOffsetFirst,UT_FALSE,&pSpanAP);
	m_pBL->getAttrProp(&pBlockAP);

	// look for fonts in this DocLayout's font cache
	FL_DocLayout * pLayout = m_pBL->getDocLayout();
	m_pFont = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_SCREEN_RESOLUTION);
	m_pFontLayout = pLayout->findFont(pSpanAP,pBlockAP,pSectionAP, FL_DocLayout::FIND_FONT_AT_LAYOUT_RESOLUTION);

	PD_Document * pDoc = m_pBL->getDocument();

	UT_parseColor(PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP, pDoc, UT_TRUE), m_colorFG);

	const XML_Char *pszDecor = PP_evalProperty("text-decoration",pSpanAP,pBlockAP,pSectionAP, pDoc, UT_TRUE);

	/*
	  TODO map line width to a property, not a hard-coded value
	*/
	m_iLineWidth = m_pG->convertDimension("0.8pt");
	
	m_fDecorations = 0;

	XML_Char* p;
	if (!UT_cloneString((char *&)p, pszDecor))
	{
		// TODO outofmem
	}
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

	const XML_Char * pszPosition = PP_evalProperty("text-position",pSpanAP,pBlockAP,pSectionAP, pDoc, UT_TRUE);

	if (0 == UT_stricmp(pszPosition, "superscript"))
	{
		m_fPosition = TEXT_POSITION_SUPERSCRIPT;
	}
	else if (0 == UT_stricmp(pszPosition, "subscript"))
	{
		m_fPosition = TEXT_POSITION_SUBSCRIPT;
	} 
	else m_fPosition = TEXT_POSITION_NORMAL;

	m_pG->setFont(m_pFont);
	m_iAscent = m_pG->getFontAscent();	
	m_iDescent = m_pG->getFontDescent();
	m_iHeight = m_pG->getFontHeight();

	m_pG->setFont(m_pFontLayout);
	m_iAscentLayoutUnits = m_pG->getFontAscent();	
	UT_ASSERT(m_iAscentLayoutUnits);
	m_iDescentLayoutUnits = m_pG->getFontDescent();
	m_iHeightLayoutUnits = m_pG->getFontHeight();

	m_pG->setFont(m_pFont);
}

UT_Bool fp_TextRun::canBreakAfter(void) const
{
	const UT_UCSChar* pSpan = NULL;
	UT_uint32 lenSpan = 0;
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

				if (pSpan[len-1] == UCS_SPACE)
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

			if (pSpan[0] == UCS_SPACE)
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

	// TODO we need to fix this code to use getSpanPtr the way it is used elsewhere in this file.
	
	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(m_iOffsetFirst, &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			for (UT_uint32 i=0; i<lenSpan; i++)
			{
				if (pSpan[i] != UCS_SPACE)
				{
					return UT_FALSE;
				}
			}

			return UT_TRUE;
		}

		return UT_FALSE;
	}

	// could assert here -- this should never happen, I think
	return UT_TRUE;
}

UT_Bool	fp_TextRun::findMaxLeftFitSplitPointInLayoutUnits(UT_sint32 iMaxLeftWidth, fp_RunSplitInfo& si, UT_Bool bForce)
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iLeftWidth = 0;
	UT_sint32 iRightWidth = m_iWidthLayoutUnits;

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

			if (
				(
					(UCS_SPACE == pSpan[i])
					&& ((i + offset) != (m_iOffsetFirst + m_iLen - 1))
					)
				|| bForce
				)
			{
				if (iLeftWidth - pCharWidths[i + offset] <= iMaxLeftWidth)
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

	if (
		(si.iOffset == -1)
		|| (si.iLeftWidth == m_iWidthLayoutUnits)
		)
	{
		// there were no split points which fit.
		return UT_FALSE;
	}


	return UT_TRUE;
}

void fp_TextRun::mapXYToPosition(UT_sint32 x, UT_sint32 /*y*/, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	if (x <= 0)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst;
		// don't set bBOL to false here
		bEOL = UT_FALSE;

		UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

		return;
	}

	if (x >= m_iWidth)
	{
		pos = m_pBL->getPosition() + m_iOffsetFirst + m_iLen;

		UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
		UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

		return;
	}

	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
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

			UT_ASSERT(bEOL == UT_TRUE || bEOL == UT_FALSE);
			UT_ASSERT(bBOL == UT_TRUE || bBOL == UT_FALSE);

			return;
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void fp_TextRun::findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
//	UT_ASSERT(FP_RUN_NOT != containsOffset(iOffset));
	
	UT_sint32 xoff;
	UT_sint32 yoff;

	UT_ASSERT(m_pLine);
	m_pLine->getOffsets(this, xoff, yoff);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	const UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_uint32 offset = UT_MIN(iOffset, m_iOffsetFirst + m_iLen);

	for (UT_uint32 i=m_iOffsetFirst; i<offset; i++)
	{
		xoff += pCharWidths[i];
	}
	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yoff -= m_iAscent * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT)
	{
		yoff += m_iDescent /* * 3/2 */;
	}
	
	x = xoff;
	y = yoff;
	height = m_iHeight;

}

UT_Bool fp_TextRun::canMergeWithNext(void)
{
	if (!m_pNext ||
		!m_pLine ||
		m_pNext->getType() != FPRUN_TEXT ||
		!m_pNext->getLine())
	{
		return UT_FALSE;
	}

	
	fp_TextRun* pNext = static_cast<fp_TextRun*>(m_pNext);
	if (
		(pNext->m_iOffsetFirst != (m_iOffsetFirst + m_iLen))
		|| (pNext->m_fDecorations != m_fDecorations)
		|| (pNext->m_pFont != m_pFont)
		|| (pNext->m_pFontLayout != m_pFontLayout)
		|| (m_iHeight != pNext->m_iHeight)
		|| (m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
		|| (pNext->m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
		)
	{
		return UT_FALSE;
	}
	
	return UT_TRUE;
}

void fp_TextRun::mergeWithNext(void)
{
	UT_ASSERT(m_pNext && (m_pNext->getType() == FPRUN_TEXT));
	UT_ASSERT(m_pLine);
	UT_ASSERT(m_pNext->getLine());

	fp_TextRun* pNext = (fp_TextRun*) m_pNext;

	UT_ASSERT(pNext->m_iOffsetFirst == (m_iOffsetFirst + m_iLen));
	UT_ASSERT(pNext->m_pFont == m_pFont);	// is this legal?
	UT_ASSERT(pNext->m_pFontLayout == m_pFontLayout);	// is this legal?
	UT_ASSERT(pNext->m_fDecorations == m_fDecorations);
	UT_ASSERT(m_iAscent == pNext->m_iAscent);
	UT_ASSERT(m_iDescent == pNext->m_iDescent);
	UT_ASSERT(m_iHeight == pNext->m_iHeight);
	UT_ASSERT(m_iLineWidth == pNext->m_iLineWidth);
//	UT_ASSERT(m_iSpaceWidthBeforeJustification == pNext->m_iSpaceWidthBeforeJustification);


	m_iWidth += pNext->m_iWidth;
	m_iWidthLayoutUnits += pNext->m_iWidthLayoutUnits;
	m_iLen += pNext->m_iLen;
	m_bDirty = m_bDirty || pNext->m_bDirty;
	m_pNext = pNext->getNext();
	if (m_pNext)
	{
		m_pNext->setPrev(this);
	}

	pNext->getLine()->removeRun(pNext, UT_FALSE);

	delete pNext;
}

UT_Bool fp_TextRun::split(UT_uint32 iSplitOffset)
{
	UT_ASSERT(iSplitOffset >= m_iOffsetFirst);
	UT_ASSERT(iSplitOffset < (m_iOffsetFirst + m_iLen));
	
	fp_TextRun* pNew = new fp_TextRun(m_pBL, m_pG, iSplitOffset, m_iLen - (iSplitOffset - m_iOffsetFirst), UT_FALSE);
	UT_ASSERT(pNew);
	pNew->m_pFont = this->m_pFont;
	pNew->m_pFontLayout = this->m_pFontLayout;
	pNew->m_fDecorations = this->m_fDecorations;
	pNew->m_colorFG = this->m_colorFG;

	pNew->m_iAscent = this->m_iAscent;
	pNew->m_iDescent = this->m_iDescent;
	pNew->m_iHeight = this->m_iHeight;
	pNew->m_iAscentLayoutUnits = this->m_iAscentLayoutUnits;
	UT_ASSERT(pNew->m_iAscentLayoutUnits);
	pNew->m_iDescentLayoutUnits = this->m_iDescentLayoutUnits;
	pNew->m_iHeightLayoutUnits = this->m_iHeightLayoutUnits;
	pNew->m_iLineWidth = this->m_iLineWidth;
	pNew->m_bDirty = this->m_bDirty;
//	pNew->m_iSpaceWidthBeforeJustification = this->m_iSpaceWidthBeforeJustification;

	pNew->m_pPrev = this;
	pNew->m_pNext = this->m_pNext;
	if (m_pNext)
	{
		m_pNext->setPrev(pNew);
	}
	m_pNext = pNew;

	m_iLen = iSplitOffset - m_iOffsetFirst;

	m_pLine->insertRunAfter(pNew, this);

	m_iWidth = simpleRecalcWidth();
	m_iWidthLayoutUnits = simpleRecalcWidth(Width_type_layout_units);
	pNew->m_iX = m_iX + m_iWidth;
	pNew->m_iY = m_iY;
	pNew->m_iWidth = pNew->simpleRecalcWidth();
	pNew->m_iWidthLayoutUnits = pNew->simpleRecalcWidth(Width_type_layout_units);
	
	return UT_TRUE;
}

void fp_TextRun::_fetchCharWidths(GR_Font* pFont, UT_uint16* pCharWidths)
{
	UT_ASSERT(pCharWidths);
	UT_ASSERT(pFont);

	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst;
	UT_uint32 len = m_iLen;
	UT_Bool bContinue = UT_TRUE;

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		m_pG->setFont(pFont);

		if (len <= lenSpan)
		{
			m_pG->measureString(pSpan, 0, len, pCharWidths + offset);

			bContinue = UT_FALSE;
		}
		else
		{
			m_pG->measureString(pSpan, 0, lenSpan, pCharWidths + offset);

			offset += lenSpan;
			len -= lenSpan;
		}
	}
}
void fp_TextRun::fetchCharWidths(fl_CharWidths * pgbCharWidths)
{
	if (m_iLen == 0)
	{
		return;
	}

	UT_uint16* pCharWidths = pgbCharWidths->getCharWidths()->getPointer(0);
	_fetchCharWidths(m_pFont, pCharWidths);

	pCharWidths = pgbCharWidths->getCharWidthsLayoutUnits()->getPointer(0);
	_fetchCharWidths(m_pFontLayout, pCharWidths);

	m_pG->setFont(m_pFont);

}

UT_sint32 fp_TextRun::simpleRecalcWidth(UT_sint32 iWidthType) const
{
	UT_GrowBuf * pgbCharWidths;
	switch(iWidthType)
	{
		case Width_type_display:
			pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
			break;

		case Width_type_layout_units:
			pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return 0;
			break;
	}

	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);
	
	UT_sint32 iWidth = 0;
	if (m_iLen == 0)
		return 0;

	{
		const UT_UCSChar* pSpan;
		UT_uint32 lenSpan;
		UT_uint32 offset = m_iOffsetFirst;
		UT_uint32 len = m_iLen;
		UT_Bool bContinue = UT_TRUE;

		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			if (!bContinue)		// if this fails, we are out of sync with the PTbl.
			{					// this probably means we are the right half of a split
				break;			// made to break a paragraph.
			}
			UT_ASSERT(lenSpan>0);

			if (lenSpan > len)
			{
				lenSpan = len;
			}
		
			for (UT_uint32 i=0; i<lenSpan; i++)
			{
				iWidth += pCharWidths[i + offset];
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
	}

	return iWidth;
}

UT_Bool fp_TextRun::recalcWidth(void)
{
	UT_sint32 iWidth = simpleRecalcWidth(Width_type_display);
	
	if (iWidth == m_iWidth)
	{
		return UT_FALSE;
	}

	if (m_iWidth)
	{
		clearScreen();
	}
	
	m_iWidth = iWidth;
	m_iWidthLayoutUnits = simpleRecalcWidth(Width_type_layout_units);

	return UT_TRUE;
}

void fp_TextRun::_clearScreen(UT_Bool /* bFullLineHeightRect */)
{
	UT_ASSERT(!m_bDirty);
	UT_ASSERT(m_pG->queryProperties(GR_Graphics::DGP_SCREEN));

	if(!m_pLine->isEmpty() && m_pLine->getLastRun() == this)
	{
		// Last run on the line so clear to end.

		m_pLine->clearScreenFromRunToEnd(m_pLine->countRuns() - 1);
	}
	else
	{
		m_pG->setFont(m_pFont);
		
		/*
		  TODO this should not be hard-coded.  We need to figure out
		  what the appropriate background color for this run is, and
		  use that.  Note that it could vary on a run-by-run basis,
		  since document facilities allow the background color to be
		  changed, for things such as table cells.
		*/
		UT_RGBColor clrNormalBackground(255,255,255);
		m_pG->setColor(clrNormalBackground);
		
		UT_sint32 xoff = 0, yoff = 0;
		m_pLine->getScreenOffsets(this, xoff, yoff);
		
		if(!m_pLine->isEmpty() && m_pLine->getFirstRun() == this)
		{
			// First run on the line so add extra at start to clear any glyph before margin.
			// Left margin only.
			UT_sint32 halfcolumnGap = m_pLine->getColumnGap() / 2;

			m_pG->clearArea(xoff - halfcolumnGap, yoff, m_iWidth + halfcolumnGap, m_pLine->getHeight());
		}
		else
		{
			m_pG->clearArea(xoff, yoff, m_iWidth, m_pLine->getHeight());
		}
	}

}

void fp_TextRun::_draw(dg_DrawArgs* pDA)
{
	/*
	  Upon entry to this function, pDA->yoff is the BASELINE of this run, NOT
	  the top.
	*/

	UT_sint32 yTopOfRun = pDA->yoff - m_iAscent;
	UT_sint32 yTopOfSel = yTopOfRun;

	if (m_fPosition == TEXT_POSITION_SUPERSCRIPT)
	{
		yTopOfRun -= m_iAscent * 1/2;
	}
	else if (m_fPosition == TEXT_POSITION_SUBSCRIPT) 
	{
		yTopOfRun += m_iDescent /* * 3/2 */;
	} 
	
	UT_ASSERT(pDA->pG == m_pG);
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_uint32 iBase = m_pBL->getPosition();

	m_pG->setFont(m_pFont);
	m_pG->setColor(m_colorFG);

	/*
	  TODO this should not be hard-coded.  We should calculate an
	  appropriate selection background color based on the color
	  of the foreground text, probably.
	*/
	UT_RGBColor clrSelBackground(192, 192, 192);

	UT_uint32 iRunBase = iBase + m_iOffsetFirst;

	FV_View* pView = m_pBL->getDocLayout()->getView();
	UT_uint32 iSelAnchor = pView->getSelectionAnchor();
	UT_uint32 iPoint = pView->getPoint();

	UT_uint32 iSel1 = UT_MIN(iSelAnchor, iPoint);
	UT_uint32 iSel2 = UT_MAX(iSelAnchor, iPoint);
	
	UT_ASSERT(iSel1 <= iSel2);
	
	if (pView->getFocus()==AV_FOCUS_NONE || iSel1 == iSel2)
	{
		// nothing in this run is selected
		_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
	}
	else if (iSel1 <= iRunBase)
	{
		if (iSel2 <= iRunBase)
		{
			// nothing in this run is selected
			_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
		}
		else if (iSel2 >= (iRunBase + m_iLen))
		{
			// the whole run is selected
			
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, m_iLen, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
		}
		else
		{
			// the first part is selected, the second part is not

			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, m_iOffsetFirst, iSel2 - iRunBase, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, iSel2 - iRunBase, pgbCharWidths);

			_drawPart(pDA->xoff, yTopOfRun, iSel2 - iBase, m_iLen - (iSel2 - iRunBase), pgbCharWidths);
		}
	}
	else if (iSel1 >= (iRunBase + m_iLen))
	{
		// nothing in this run is selected
		_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, m_iLen, pgbCharWidths);
	}
	else
	{
		_drawPart(pDA->xoff, yTopOfRun, m_iOffsetFirst, iSel1 - iRunBase, pgbCharWidths);
		
		if (iSel2 >= (iRunBase + m_iLen))
		{
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, m_iLen - (iSel1 - iRunBase), pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, iSel1 - iBase, m_iLen - (iSel1 - iRunBase), pgbCharWidths);
		}
		else
		{
			_fillRect(clrSelBackground, pDA->xoff, yTopOfSel, iSel1 - iBase, iSel2 - iSel1, pgbCharWidths);
			_drawPart(pDA->xoff, yTopOfRun, iSel1 - iBase, iSel2 - iSel1, pgbCharWidths);

			_drawPart(pDA->xoff, yTopOfRun, iSel2 - iBase, m_iLen - (iSel2 - iRunBase), pgbCharWidths);
		}
	}

	_drawDecors(pDA->xoff, yTopOfRun);

	// TODO: draw this underneath (ie, before) the text and decorations
	m_bSquiggled = UT_FALSE;
	m_pBL->findSquigglesForRun(this);
}

void fp_TextRun::_fillRect(UT_RGBColor& clr,
						   UT_sint32 xoff,
						   UT_sint32 yoff,
						   UT_uint32 iPos1,
						   UT_uint32 iLen,
						   const UT_GrowBuf* pgbCharWidths)
{
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/
	
	if (m_pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		UT_Rect r;

		_getPartRect(&r, xoff, yoff, iPos1, iLen, pgbCharWidths);
		r.height = m_pLine->getHeight();
		r.top = r.top + m_iAscent - m_pLine->getAscent();
		
		m_pG->fillRect(clr, r.left, r.top, r.width, r.height);
	}
}

void fp_TextRun::_getPartRect(UT_Rect* pRect,
							  UT_sint32 xoff,
							  UT_sint32 yoff,
							  UT_uint32 iStart,
							  UT_uint32 iLen,
							  const UT_GrowBuf * pgbCharWidths)
{
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/
	
	pRect->left = xoff;
	pRect->top = yoff;
	pRect->height = m_iHeight;
	pRect->width = 0;

	// that's enough for zero-length run
	if (m_iLen == 0)
	{
		return;
	}

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

void fp_TextRun::_drawPart(UT_sint32 xoff,
						   UT_sint32 yoff,
						   UT_uint32 iStart,
						   UT_uint32 iLen,
						   const UT_GrowBuf * pgbCharWidths)
{
	/*
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/
	
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = iStart;
	UT_uint32 len = iLen;
	UT_Bool bContinue = UT_TRUE;

	// don't even try to draw a zero-length run
	if (m_iLen == 0)
	{
		return;
	}

	UT_ASSERT(offset >= m_iOffsetFirst);
	UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);

	UT_uint32 iLeftWidth = 0;
	const UT_uint16 * pCharWidths = pgbCharWidths->getPointer(0);
	
	UT_uint32 i;
	for (i=m_iOffsetFirst; i<iStart; i++)
	{
		iLeftWidth += pCharWidths[i];
	}

	while (bContinue)
	{
		bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
		UT_ASSERT(lenSpan>0);

		if (len <= lenSpan)
		{
			m_pG->drawChars(pSpan, 0, len, xoff + iLeftWidth, yoff);

			bContinue = UT_FALSE;
		}
		else
		{
			m_pG->drawChars(pSpan, 0, lenSpan, xoff + iLeftWidth, yoff);
			for(i = 0; i < lenSpan; i++)
			{
				iLeftWidth += pCharWidths[offset + i];
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
	  Upon entry to this function, yoff is the TOP of the run,
	  NOT the baseline.
	*/

     /*
	Here is the code to work out the position and thickness of under
        and overlines for a run of text. This is neccessary because an
underline or overline could shift position depending on the text size -
particularly for subscripts and superscripts. We can't work out where to put 
the lines until the end of the lined span. This info is saved in the fp_TextRun
class. If a underline or overline is pending (because the next run continues
 the underline or overline), mark the next run as dirty to make sure it is
drawn.
     */

	if( (m_fDecorations & (TEXT_DECOR_UNDERLINE | TEXT_DECOR_OVERLINE | 
			TEXT_DECOR_LINETHROUGH)) == 0)
	{
		return;
	}

	const UT_sint32 old_LineWidth = m_iLineWidth;
	UT_sint32 cur_linewidth = 1+ (UT_MAX(10,m_iAscent)-10)/8;
	UT_sint32 iDrop = 0;
	fp_Run* P_Run = getPrev();
	fp_Run* N_Run = getNext();
	const UT_Bool b_Underline = isUnderline();
	const UT_Bool b_Overline = isOverline();
	const UT_Bool b_Strikethrough = isStrikethrough();
	const UT_Bool b_Firstrun = (P_Run == NULL) || (m_pLine->getFirstRun()== this);
	const UT_Bool b_Lastrun = (N_Run == NULL) || (m_pLine->getLastRun()== this);

	/*
	  If the previous run is NULL or if this is the first run of a line, 
we are on the first run of the line so set the linethickness, start of the line span and the overline and underline positions from the current measurements.
	*/
        if(P_Run == NULL || b_Firstrun )
	{
	        setLinethickness(cur_linewidth);
		if(b_Underline)
		{
		     iDrop = yoff + m_iAscent + m_iDescent/3;
		     setUnderlineXoff( xoff);
		     setMaxUnderline(iDrop);
		}
		if(b_Overline)
		{
		     iDrop = yoff + 1 + (UT_MAX(10,m_iAscent) - 10)/8;
		     setOverlineXoff( xoff);
		     setMinOverline(iDrop);
		}
	}
	/*
	  Otherwise look to see if the previous run had an underline or 
overline. If it does, merge the information with the present information. Take
the Maximum of the underline offsets and the minimum of the overline offsets.
Always take the maximum of the linewidths. If there is no previous underline
or overline set the underline and overline locations with the current data.
	*/
	else
	{
	  if (!P_Run->isUnderline() && !P_Run->isOverline() && 
	      !P_Run->isStrikethrough())
	      {
	             setLinethickness(cur_linewidth);
	      }
	      else
	      {
		     setLinethickness(UT_MAX(P_Run->getLinethickness(),cur_linewidth));
	      }
 	      if (b_Underline)
	      {
		     iDrop = yoff + m_iAscent + m_iDescent/3;
		     if(!P_Run->isUnderline())
		     {
		           setUnderlineXoff( xoff);
		           setMaxUnderline(iDrop);
		     }
		     else
		     {
		           setUnderlineXoff( P_Run->getUnderlineXoff());
                           setMaxUnderline(UT_MAX( P_Run->getMaxUnderline(), iDrop));
		     }
		}
 	      if (b_Overline)
	      {
		     iDrop = yoff + 1 + (UT_MAX(10,m_iAscent) - 10)/8;
		     if(!P_Run->isOverline())
		     {
		           setOverlineXoff( xoff);
		           setMinOverline(iDrop);
		     }
		     else
		     {
		           setOverlineXoff( P_Run->getOverlineXoff());
                           setMinOverline(UT_MIN( P_Run->getMinOverline(), iDrop));
		     }
		}
	}
	m_iLineWidth = getLinethickness();
	m_pG->setLineWidth(m_iLineWidth);
	/*
	  If the next run returns NULL or if we are on the last run
 we've reached the of the line of text so the overlines and underlines must 
be drawn.
	*/
	if(N_Run == NULL  || b_Lastrun)
	{
 	        if ( b_Underline)
		{
		     iDrop = UT_MAX( getMaxUnderline(), iDrop);
		     UT_sint32 totx = getUnderlineXoff();
		     m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		}
		if ( b_Overline)
		{
		     iDrop = UT_MIN( getMinOverline(), iDrop);
		     UT_sint32 totx = getOverlineXoff();
		     m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		}
	}
	/* 
	   Otherwise look to see if the next run has an underline or overline
if not, draw the line, if does mark the next run as dirty to make sure it
is drawn later.
	*/
	else
	{
 	        if ( b_Underline )
		{
		     if(!N_Run->isUnderline())
		     {
		          iDrop = UT_MAX( getMaxUnderline(), iDrop);
		          UT_sint32 totx = getUnderlineXoff();
		          m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		     }
		     else
		     {
		          N_Run->markAsDirty();
		     }
		}
 	        if ( b_Overline )
		{
		     if(!N_Run->isOverline())
		     {
		          iDrop = UT_MIN( getMinOverline(), iDrop);
		          UT_sint32 totx = getOverlineXoff();
		          m_pG->drawLine(totx, iDrop, xoff+getWidth(), iDrop);
		     }
		     else
		     {
		          N_Run->markAsDirty();
		     }
		}
	}
	/*
	  We always want strikethrough to go right through the middle of the
text so we can keep the original code.
	*/
	if ( b_Strikethrough)
	{
		iDrop = yoff + getAscent() * 2 / 3;
		m_pG->drawLine(xoff, iDrop, xoff+getWidth(), iDrop);
	}
	/* 
	   Restore the previous line width.
	*/ 
	m_iLineWidth = old_LineWidth;
	m_pG->setLineWidth(m_iLineWidth);
}

void fp_TextRun::_drawSquiggle(UT_sint32 top, UT_sint32 left, UT_sint32 right)
{
	if (!(m_pG->queryProperties(GR_Graphics::DGP_SCREEN)))
	{
		return;
	}

	m_bSquiggled = UT_TRUE;
	
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
//	UT_ASSERT(iLen > 0);
	if (iLen == 0)
	{
		// I think this is safe, although it begs the question, why did we get called if iLen is zero?  TODO
		return;
	}

	UT_sint32 xoff = 0, yoff = 0;
	UT_sint32 iAscent = m_pLine->getAscent();
	UT_sint32 iDescent = m_pLine->getDescent();

	// we'd prefer squiggle to leave one pixel below the baseline, 
	// but we need to force all three pixels inside the descent
	UT_sint32 iGap = (iDescent > 3) ? 1 : (iDescent - 3);

	UT_RGBColor clrSquiggle(255, 0, 0);
	m_pG->setColor(clrSquiggle);
	
	m_pLine->getScreenOffsets(this, xoff, yoff);

	UT_Rect r;
	const UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();  
	_getPartRect( &r, xoff, yoff, iOffset, iLen, pgbCharWidths);

	_drawSquiggle(r.top + iAscent + iGap, r.left, r.left + r.width); 
}

UT_sint32 fp_TextRun::findCharacter(UT_uint32 startPosition, UT_UCSChar Character) const
{
	// NOTE: startPosition is run-relative
	// NOTE: return value is block-relative (don't ask me why)
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;
	UT_uint32 offset = m_iOffsetFirst + startPosition;
	UT_uint32 len = m_iLen - startPosition;
	UT_Bool bContinue = UT_TRUE;

	if ((m_iLen > 0) && (startPosition < m_iLen))
	{
		UT_uint32 i;

		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			UT_ASSERT(lenSpan>0);

			if (len <= lenSpan)
			{
				for(i = 0; i < len; i++)
				{
					if (pSpan[i] == Character)
						return offset + i;
				}

				bContinue = UT_FALSE;
			}
			else
			{
				for(i = 0; i < lenSpan; i++)
				{
					if (pSpan[i] == Character)
						return offset + i;
				}

				offset += lenSpan;
				len -= lenSpan;

				UT_ASSERT(offset >= m_iOffsetFirst);
				UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
			}
		}
	}

	// not found

	return -1;
}

UT_Bool fp_TextRun::getCharacter(UT_uint32 run_offset, UT_UCSChar &Character) const
{
	const UT_UCSChar* pSpan;
	UT_uint32 lenSpan;

	UT_ASSERT(run_offset < m_iLen);

	if (m_iLen > 0)
	{
		if (m_pBL->getSpanPtr(run_offset + m_iOffsetFirst, &pSpan, &lenSpan))
		{
			UT_ASSERT(lenSpan>0);

			Character = pSpan[0];
			return UT_TRUE;
		}
	}

	// not found

	return UT_FALSE;
}

UT_Bool fp_TextRun::isLastCharacter(UT_UCSChar Character) const
{
	UT_UCSChar c;

	if (getCharacter(m_iLen - 1, c))
		return c == Character;

	// not found

	return UT_FALSE;
}

UT_Bool fp_TextRun::isFirstCharacter(UT_UCSChar Character) const
{
	UT_UCSChar c;

	if (getCharacter(0, c))
		return c == Character;

	// not found

	return UT_FALSE;
}


UT_Bool	fp_TextRun::doesContainNonBlankData(void) const
{
	if(m_iLen > 0)
	{
		const UT_UCSChar* pSpan;
		UT_uint32 lenSpan;
		UT_uint32 offset = m_iOffsetFirst;
		UT_uint32 len = m_iLen;
		UT_Bool bContinue = UT_TRUE;

		UT_uint32 i;

		while (bContinue)
		{
			bContinue = m_pBL->getSpanPtr(offset, &pSpan, &lenSpan);
			UT_ASSERT(lenSpan>0);

			if (len <= lenSpan)
			{
				for(i = 0; i < len; i++)
				{
					if (pSpan[i] != UCS_SPACE)
						return UT_TRUE;
				}

				bContinue = UT_FALSE;
			}
			else
			{
				for(i = 0; i < lenSpan; i++)
				{
					if (pSpan[i] != UCS_SPACE)
						return UT_TRUE;
				}

				offset += lenSpan;
				len -= lenSpan;

				UT_ASSERT(offset >= m_iOffsetFirst);
				UT_ASSERT(offset + len <= m_iOffsetFirst + m_iLen);
			}
		}
	}

	// Only spaces found;

	return UT_FALSE;
}

UT_Bool fp_TextRun::isSuperscript(void) const 
{
	return (m_fPosition == TEXT_POSITION_SUPERSCRIPT);
}

UT_Bool fp_TextRun::isSubscript(void) const
{
	return (m_fPosition == TEXT_POSITION_SUBSCRIPT);
}

UT_Bool fp_TextRun::isUnderline(void) const
{
         return ((m_fDecorations & TEXT_DECOR_UNDERLINE) !=  0);
}

UT_Bool fp_TextRun::isOverline(void) const
{
         return ((m_fDecorations & TEXT_DECOR_OVERLINE) !=  0);
}

UT_Bool fp_TextRun::isStrikethrough(void) const
{
         return ((m_fDecorations & TEXT_DECOR_LINETHROUGH) !=  0);
}

void fp_TextRun::setLinethickness(UT_sint32 max_linethickness)
{
         m_iLinethickness = max_linethickness;
}

void fp_TextRun::setUnderlineXoff(UT_sint32 xoff)
{
         m_iUnderlineXoff = xoff;
}

UT_sint32 fp_TextRun::getUnderlineXoff(void)
{
         return m_iUnderlineXoff;
}

void fp_TextRun::setOverlineXoff(UT_sint32 xoff)
{
         m_iOverlineXoff = xoff;
}

UT_sint32 fp_TextRun::getOverlineXoff(void)
{
         return m_iOverlineXoff;
}

void fp_TextRun::setMaxUnderline(UT_sint32 maxh)
{
         m_imaxUnderline = maxh;
}

UT_sint32 fp_TextRun::getMaxUnderline(void)
{
         return m_imaxUnderline;
}

void fp_TextRun::setMinOverline(UT_sint32 minh)
{
         m_iminOverline = minh;
}

UT_sint32 fp_TextRun::getMinOverline(void)
{
         return m_iminOverline;
}

UT_sint32 fp_TextRun::getLinethickness( void)
{
         return m_iLinethickness;
}

UT_sint32 fp_TextRun::findTrailingSpaceDistance(void) const
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iTrailingDistance = 0;

	if(m_iLen > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = m_iLen - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iTrailingDistance += pCharWidths[i + m_iOffsetFirst];
			}
			else
			{
				break;
			}
		}

	}

	return iTrailingDistance;
}

UT_sint32 fp_TextRun::findTrailingSpaceDistanceInLayoutUnits(void) const
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidthsLayoutUnits();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_sint32 iTrailingDistance = 0;

	if(m_iLen > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = m_iLen - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iTrailingDistance += pCharWidths[i + m_iOffsetFirst];
			}
			else
			{
				break;
			}
		}

	}

	return iTrailingDistance;
}

void fp_TextRun::resetJustification()
{
	if(m_iSpaceWidthBeforeJustification != JUSTIFICATION_NOT_USED)
	{
		UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
		UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

		UT_sint32 i = findCharacter(0, UCS_SPACE);

		while (i >= 0)
		{
			if(pCharWidths[i] != m_iSpaceWidthBeforeJustification)
			{
				// set width of spaces back to normal.

				m_iWidth -= pCharWidths[i] - m_iSpaceWidthBeforeJustification;
				pCharWidths[i] = m_iSpaceWidthBeforeJustification;
			}

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
		}
	}
	
	m_iSpaceWidthBeforeJustification = JUSTIFICATION_NOT_USED;
}

void fp_TextRun::distributeJustificationAmongstSpaces(UT_sint32 iAmount, UT_uint32 iSpacesInRun)
{
	UT_GrowBuf * pgbCharWidths = m_pBL->getCharWidths()->getCharWidths();
	UT_uint16* pCharWidths = pgbCharWidths->getPointer(0);

	UT_ASSERT(iSpacesInRun);

	if(iSpacesInRun && m_iLen > 0)
	{
		m_iWidth += iAmount;
	
		UT_sint32 i = findCharacter(0, UCS_SPACE);

		while ((i >= 0) && (iSpacesInRun))
		{
			// remember how wide spaces in this run "really" were
			if(m_iSpaceWidthBeforeJustification == JUSTIFICATION_NOT_USED)
				m_iSpaceWidthBeforeJustification = pCharWidths[i];

			UT_sint32 iThisAmount = iAmount / iSpacesInRun;

			pCharWidths[i] += iThisAmount;

			iAmount -= iThisAmount;

			iSpacesInRun--;

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
		}
	}

	UT_ASSERT(iAmount == 0);
}

UT_uint32 fp_TextRun::countTrailingSpaces(void) const
{
	UT_uint32 iCount = 0;

	if(m_iLen > 0)
	{
		UT_UCSChar c;

		UT_sint32 i;
		for (i = m_iLen - 1; i >= 0; i--)
		{
			if(getCharacter(i, c) && (UCS_SPACE == c))
			{
				iCount++;
			}
			else
			{
				break;
			}
		}
	}

	return iCount;
}

UT_uint32 fp_TextRun::countJustificationPoints(void) const
{
	UT_uint32 iCount = 0;

	if(m_iLen > 0)
	{
		UT_sint32 i = findCharacter(0, UCS_SPACE);

		while (i >= 0)
		{
			iCount++;

			// keep looping
			i = findCharacter(i+1-m_iOffsetFirst, UCS_SPACE);
		}
	}

	return iCount;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#ifdef FMT_TEST
void fp_TextRun::__dump(FILE * fp) const
{
	fp_Run::__dump(fp);

	if (m_iLen == 0)
		return;

	fprintf(fp,"      [");
	{
		const UT_UCSChar* pSpan;
		UT_uint32 lenSpan;

		UT_uint32 koff=m_iOffsetFirst;
		UT_uint32 klen=m_iLen;
		
		while (m_pBL->getSpanPtr(koff, &pSpan, &lenSpan) && (klen > 0))
		{
			UT_uint32 kdraw = MyMin(klen,lenSpan);
			for (UT_uint32 k=0; k<kdraw; k++)
			{
				// a cheap unicode to ascii hack...
				unsigned char c = (unsigned char)(pSpan[k] & 0x00ff);
				fprintf(fp,"%c",c);
			}

			klen -= kdraw;
			koff += lenSpan;
		}
	}
	fprintf(fp,"]\n");
		
}
#endif
