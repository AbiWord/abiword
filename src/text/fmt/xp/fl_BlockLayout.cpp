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
#include <ctype.h>

#include "fl_BlockLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "sp_spell.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "fv_View.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "ut_png.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"

fl_BlockLayout::fl_BlockLayout(PL_StruxDocHandle sdh,
							   fb_LineBreaker* pBreaker,
							   fl_BlockLayout* pPrev,
							   fl_SectionLayout* pSectionLayout,
							   PT_AttrPropIndex indexAP)
	: fl_Layout(PTX_Block, sdh), m_gbCharWidths(256)
{
	m_pSectionLayout = pSectionLayout;
	m_pBreaker = pBreaker;
	m_pFirstRun = NULL;
	m_pFirstLine = NULL;
	m_pLastLine = NULL;

	m_bNeedsReformat = UT_TRUE;
	m_bFixCharWidths = UT_FALSE;
	m_bKeepTogether = UT_FALSE;
	m_bKeepWithNext = UT_FALSE;

	m_pLayout = m_pSectionLayout->getDocLayout();
	m_pDoc = m_pLayout->getDocument();

	setAttrPropIndex(indexAP);

	_lookupProperties();

	m_pPrev = pPrev;
	if (m_pPrev)
	{
		m_pNext = pPrev->m_pNext;
		m_pPrev->m_pNext = this;
		if (m_pNext)
		{
			m_pNext->m_pPrev = this;
		}
	}
	else
	{
		m_pNext = NULL;
	}
}

fl_TabStop::fl_TabStop()
{
	iPosition = 0;
	iType = 0;
}

void fl_BlockLayout::_lookupProperties(void)
{
	{
		const char* pszOrphans = getProperty("orphans");
		if (pszOrphans && pszOrphans[0])
		{
			m_iOrphansProperty = atoi(pszOrphans);
		}
		else
		{
			m_iOrphansProperty = 1;
		}
		
		const char* pszWidows = getProperty("widows");
		if (pszWidows && pszWidows[0])
		{
			m_iWidowsProperty = atoi(pszWidows);
		}
		else
		{	
			m_iWidowsProperty = 1;
		}

		if (m_iOrphansProperty < 1)
		{
			m_iOrphansProperty = 1;
		}
		if (m_iWidowsProperty < 1)
		{
			m_iWidowsProperty = 1;
		}
	}

	{
		const char* pszKeepTogether = getProperty("keep-together");
		if (pszKeepTogether
			&& (0 == UT_stricmp("yes", pszKeepTogether))
			)
		{
			m_bKeepTogether = UT_TRUE;
		}
	
		const char* pszKeepWithNext = getProperty("keep-with-next");
		if (pszKeepWithNext
			&& (0 == UT_stricmp("yes", pszKeepWithNext))
			)
		{
			m_bKeepWithNext = UT_TRUE;
		}
	}
	
	GR_Graphics* pG = m_pLayout->getGraphics();
	
	m_iTopMargin = pG->convertDimension(getProperty("margin-top"));
	m_iBottomMargin = pG->convertDimension(getProperty("margin-bottom"));
	m_iLeftMargin = pG->convertDimension(getProperty("margin-left"));
	m_iRightMargin = pG->convertDimension(getProperty("margin-right"));
	m_iTextIndent = pG->convertDimension(getProperty("text-indent"));

	{
		const char* pszAlign = getProperty("text-align");

		if (0 == UT_stricmp(pszAlign, "left"))
		{
			m_iAlignment = FL_ALIGN_BLOCK_LEFT;
		}
		else if (0 == UT_stricmp(pszAlign, "center"))
		{
			m_iAlignment = FL_ALIGN_BLOCK_CENTER;
		}
		else if (0 == UT_stricmp(pszAlign, "right"))
		{
			m_iAlignment = FL_ALIGN_BLOCK_RIGHT;
		}
		else if (0 == UT_stricmp(pszAlign, "justify"))
		{
			m_iAlignment = FL_ALIGN_BLOCK_JUSTIFY;
		}
		else
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

	const char* pszTabStops = getProperty("tabstops");
	if (pszTabStops && pszTabStops[0])
	{
		UT_uint32 iCount = m_vecTabs.getItemCount();
		UT_uint32 i;

		for (i=0; i<iCount; i++)
		{
			fl_TabStop* pTab = (fl_TabStop*) m_vecTabs.getNthItem(i);

			delete pTab;
		}
		m_vecTabs.clear();

		unsigned char iType = 0;
		UT_sint32 iPosition = 0;
		
		const char* pStart = pszTabStops;
		while (*pStart)
		{
			const char* pEnd = pStart;
			while (*pEnd && (*pEnd != ','))
			{
				pEnd++;
			}

			const char* p1 = pStart;
			while ((p1 < pEnd) && (*p1 != '/'))
			{
				p1++;
			}

			if (
				(p1 == pEnd)
				|| ((p1+1) == pEnd)
				)
			{
				iType = FL_TAB_LEFT;
			}
			else
			{
				switch (p1[1])
				{
				case 'L':
					iType = FL_TAB_LEFT;
					break;
				case 'R':
					iType = FL_TAB_RIGHT;
					break;
				case 'C':
					iType = FL_TAB_CENTER;
					break;
				default:
					iType = FL_TAB_LEFT;
					break;
				}
			}

			char pszPosition[32];
			UT_uint32 iPosLen = p1 - pStart;
		
			UT_ASSERT(iPosLen < 32);

			for (i=0; i<iPosLen; i++)
			{
				pszPosition[i] = pStart[i];
			}
			pszPosition[i] = 0;

			iPosition = pG->convertDimension(pszPosition);

			UT_ASSERT(iType > 0);
			UT_ASSERT(iPosition >= 0);
			
			fl_TabStop* pTabStop = new fl_TabStop();
			pTabStop->iPosition = iPosition;
			pTabStop->iType = iType;

			m_vecTabs.addItem(pTabStop);

			pStart = pEnd;
			if (*pStart)
			{
				while (*pStart == 32)
				{
					pStart++;
				}
			}
		}

		// TODO sort the tabs vector by position, ascending
	}
	
	m_iDefaultTabInterval = pG->convertDimension(getProperty("default-tab-interval"));

	// for now, just allow fixed multiples
	// TODO: if units were used, convert to exact spacing required
	m_dLineSpacing = atof(getProperty("line-height"));
	m_bExactSpacing = UT_FALSE;
}

fl_BlockLayout::~fl_BlockLayout()
{
	_purgeSquiggles();
	purgeLayout();

	UT_VECTOR_PURGEALL(fl_TabStop *, m_vecTabs);
}

void fl_BlockLayout::clearScreen(GR_Graphics* pG)
{
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		pLine->clearScreen();
 
		pLine = pLine->getNext();
	}
}

void fl_BlockLayout::_mergeRuns(fp_Run* pFirstRunToMerge, fp_Run* pLastRunToMerge)
{
	UT_ASSERT(pFirstRunToMerge != pLastRunToMerge);
	UT_ASSERT(pFirstRunToMerge->getType() == FPRUN_TEXT);
	UT_ASSERT(pLastRunToMerge->getType() == FPRUN_TEXT);

	fp_TextRun* pFirst = (fp_TextRun*) pFirstRunToMerge;

	UT_Bool bDone = UT_FALSE;
	while (!bDone)
	{
		if (pFirst->getNext() == pLastRunToMerge)
		{
			bDone = UT_TRUE;
		}

		pFirst->mergeWithNext();
	}
}

void fl_BlockLayout::coalesceRuns(void)
{
#if 0
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		pLine->coalesceRuns();
		
		pLine = pLine->getNext();
	}
#else	
	fp_Run* pFirstRunInChain = NULL;
	UT_uint32 iNumRunsInChain = 0;
	
	fp_Run* pCurrentRun = m_pFirstRun;
	fp_Run* pLastRun = NULL;

	while (pCurrentRun)
	{
		if (pCurrentRun->getType() == FPRUN_TEXT)
		{
			if (pFirstRunInChain)
			{
				if (
					(pCurrentRun->getLine() == pFirstRunInChain->getLine())
					&& (pCurrentRun->getAP() == pFirstRunInChain->getAP())
					&& ((!pLastRun)
						|| (
							(pCurrentRun->getBlockOffset() == (pLastRun->getBlockOffset() + pLastRun->getLength()))
							)
						)
					)
				{
					iNumRunsInChain++;
				}
				else
				{
					if (iNumRunsInChain > 1)
					{
						_mergeRuns(pFirstRunInChain, pLastRun);
					}

					pFirstRunInChain = pCurrentRun;
					iNumRunsInChain = 1;
				}
			}
			else
			{
				pFirstRunInChain = pCurrentRun;
				iNumRunsInChain = 1;
			}
		}
		else
		{
			if (iNumRunsInChain > 1)
			{
				_mergeRuns(pFirstRunInChain, pLastRun);
			}

			iNumRunsInChain = 0;
			pFirstRunInChain = NULL;
		}
		
		pLastRun = pCurrentRun;
		pCurrentRun = pCurrentRun->getNext();
	}

	if (iNumRunsInChain > 1)
	{
		_mergeRuns(pFirstRunInChain, pLastRun);
	}
#endif	
}

void fl_BlockLayout::collapse(void)
{
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		pRun->setLine(NULL);
		
		pRun = pRun->getNext();
	}
	
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		_removeLine(pLine);
		pLine = m_pFirstLine;
	}

	UT_ASSERT(m_pFirstLine == NULL);
	UT_ASSERT(m_pLastLine == NULL);
}

void fl_BlockLayout::purgeLayout(void)
{
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		_removeLine(pLine);
		pLine = m_pFirstLine;
	}

	UT_ASSERT(m_pFirstLine == NULL);
	UT_ASSERT(m_pLastLine == NULL);
	
	while (m_pFirstRun)
	{
		fp_Run* pNext = m_pFirstRun->getNext();
		delete m_pFirstRun;
		m_pFirstRun = pNext;
	}
}

void fl_BlockLayout::_removeLine(fp_Line* pLine)
{
	if (m_pFirstLine == pLine)
	{
		m_pFirstLine = m_pFirstLine->getNext();
	}
			
	if (m_pLastLine == pLine)
	{
		m_pLastLine = m_pLastLine->getPrev();
	}

	pLine->setBlock(NULL);
	pLine->remove();

	delete pLine;
}

void fl_BlockLayout::_removeAllEmptyLines(void)
{
	fp_Line* pLine;

	pLine = m_pFirstLine;
	while (pLine)
	{
		if (pLine->countRuns() == 0)
		{
			_removeLine(pLine);
			pLine = m_pFirstLine;
		}
		else
		{
			pLine = pLine->getNext();
		}
	}
}

UT_Bool fl_BlockLayout::truncateLayout(fp_Run* pTruncRun)
{
	// special case, nothing to do
	if (!pTruncRun)
	{
		return UT_TRUE;
	}

	if (m_pFirstRun == pTruncRun)
	{
		m_pFirstRun = NULL;
	}

	fp_Run* pRun;
	
	// remove runs from screen
	pRun = pTruncRun;
	while (pRun)
	{
		pRun->clearScreen();
		pRun = pRun->getNext();
	}

	// remove runs from lines
	pRun = pTruncRun;
	while (pRun)
	{
		fp_Line* pLine = pRun->getLine();
		UT_ASSERT(pLine);

		pLine->removeRun(pRun);

		pRun = pRun->getNext();
	}

	_removeAllEmptyLines();
	
	return UT_TRUE;
}

void fl_BlockLayout::checkForEndOnForcedBreak(void)
{
	fp_Line* pLastLine = getLastLine();
	fp_Run* pLastRun = pLastLine->getLastRun();
	if (pLastRun->canContainPoint())
	{
		return;
	}

	/*
	  We add a zero-length text run on a line by itself.
	*/

	fp_Line* pNewLine;
	
	GR_Graphics* pG = m_pLayout->getGraphics();
	fp_TextRun* pNewRun = new fp_TextRun(this, pG, pLastRun->getBlockOffset() + pLastRun->getLength(), 0);
	pNewRun->setPrev(pLastRun);
	pLastRun->setNext(pNewRun);

	pNewLine = getNewLine();
	UT_ASSERT(pNewLine == m_pLastLine);

	// the line just contains the empty run
	pNewLine->addRun(pNewRun);

	pNewLine->layout();
}

int fl_BlockLayout::format()
{
	if (m_bFixCharWidths)
	{
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			pRun->recalcWidth();
			
			pRun = pRun->getNext();
		}
	}
	
	if (m_pFirstRun)
	{
		if (!m_pFirstLine)
		{
			// start a new line
			fp_Line* pLine = getNewLine();

			fp_Run* pTempRun = m_pFirstRun;
			while (pTempRun)
			{
				pLine->addRun(pTempRun);
				pTempRun = pTempRun->getNext();
			}
		}

		recalculateFields();
//		debug_dumpRunList();
		m_pBreaker->breakParagraph(this);
		_removeAllEmptyLines();
		checkForEndOnForcedBreak();
	}
	else
	{
		_removeAllEmptyLines();
		
		// we don't ... construct just enough to keep going
		GR_Graphics* pG = m_pLayout->getGraphics();
		m_pFirstRun = new fp_TextRun(this, pG, 0, 0);
		m_pFirstRun->fetchCharWidths(&m_gbCharWidths);

		if (!m_pFirstLine)
		{
			getNewLine();
		}

		// the line just contains the empty run
		m_pFirstLine->addRun(m_pFirstRun);

		m_pFirstLine->layout();
	}

	m_bNeedsReformat = UT_FALSE;

	return 0;	// TODO return code
}

fp_Run* fl_BlockLayout::getFirstRun()
{
	return m_pFirstRun;
}

fp_Line* fl_BlockLayout::getNewLine(void)
{
	fp_Line* pLine = new fp_Line();
	UT_ASSERT(pLine);

	pLine->setBlock(this);
	pLine->setNext(NULL);
	
	fp_Column* pCol = NULL;
	
	if (m_pLastLine)
	{
		fp_Line* pOldLastLine = m_pLastLine;
		
		UT_ASSERT(m_pFirstLine);
		UT_ASSERT(!m_pLastLine->getNext());

		pLine->setPrev(m_pLastLine);
		m_pLastLine->setNext(pLine);
		m_pLastLine = pLine;

		pCol = pOldLastLine->getColumn();

		pCol->insertLineAfter(pLine, pOldLastLine);
	}
	else
	{
		UT_ASSERT(!m_pFirstLine);
		m_pFirstLine = pLine;
		m_pLastLine = m_pFirstLine;
		pLine->setPrev(NULL);
		fp_Line* pPrevLine = NULL;

		if (m_pPrev)
		{
			pPrevLine = m_pPrev->getLastLine();
			pCol = pPrevLine->getColumn();
		}
		else if (m_pNext && m_pNext->getFirstLine())
		{
			pCol = m_pNext->getFirstLine()->getColumn();
		}
		else if (m_pSectionLayout->getFirstColumn())
		{
			// TODO assert something here about what's in that column
			pCol = m_pSectionLayout->getFirstColumn();
		}
		else
		{
			pCol = m_pSectionLayout->getNewColumn();
		}
		
		pCol->insertLineAfter(pLine, pPrevLine);
	}

	return pLine;
}

const char*	fl_BlockLayout::getProperty(const XML_Char * pszName) const
{
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL; // TODO do we care about section-level inheritance?
	
	getAttrProp(&pBlockAP);
	
	return PP_evalProperty(pszName,pSpanAP,pBlockAP,pSectionAP);
}

UT_uint32 fl_BlockLayout::getPosition(UT_Bool bActualBlockPos) const
{
	PT_DocPosition pos = m_pDoc->getStruxPosition(m_sdh);

	// it's usually more useful to know where the runs start
	if (!bActualBlockPos)
		pos += fl_BLOCK_STRUX_OFFSET;

	return pos;
}

UT_GrowBuf * fl_BlockLayout::getCharWidths(void)
{
	return &m_gbCharWidths;
}

void fl_BlockLayout::getLineSpacing(double& dSpacing, UT_Bool& bExact) const
{
	dSpacing = m_dLineSpacing;
	bExact = m_bExactSpacing;
}

UT_Bool fl_BlockLayout::getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	return m_pDoc->getSpanPtr(m_sdh, offset+fl_BLOCK_STRUX_OFFSET, ppSpan, pLength);
}

UT_Bool	fl_BlockLayout::getBlockBuf(UT_GrowBuf * pgb) const
{
	return m_pDoc->getBlockBuf(m_sdh, pgb);
}

fp_Run* fl_BlockLayout::findPointCoords(PT_DocPosition iPos, UT_Bool bEOL, UT_sint32& x, UT_sint32& y, UT_sint32& height)
{
	// find the run which has this position inside it.
	UT_ASSERT(iPos >= getPosition());
	UT_uint32 iRelOffset = iPos - getPosition();

	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iWhere = pRun->containsOffset(iRelOffset);
		if (FP_RUN_INSIDE == iWhere)
		{
			pRun->findPointCoords(iRelOffset, x, y, height);
			return pRun;
		}
		else if (bEOL && (FP_RUN_JUSTAFTER == iWhere))
		{
			fp_Run* pNext = pRun->getNext();
			fp_Line* pNextLine = NULL;

			if (pNext)
				pNextLine = pNext->getLine();

			if (pNextLine != pRun->getLine())
			{
				pRun->findPointCoords(iRelOffset, x, y, height);
				return pRun;
			}
		}
		
		pRun = pRun->getNext();
	}

	pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iWhere = pRun->containsOffset(iRelOffset);
		if ((FP_RUN_JUSTAFTER == iWhere))
		{
			pRun->findPointCoords(iRelOffset, x, y, height);
			return pRun;
		}

		if (!pRun->getNext())
		{
			// this is the last run, we're not going to get another chance, so try harder
			if (iRelOffset > (pRun->getBlockOffset() + pRun->getLength()))
			{
				pRun->findPointCoords(iRelOffset, x, y, height);
				return pRun;
			}
		}
		
		pRun = pRun->getNext();
	}

	if (iRelOffset < m_pFirstRun->getBlockOffset())
	{
		m_pFirstRun->findPointCoords(iRelOffset, x, y, height);
		return m_pFirstRun;
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fp_Line* fl_BlockLayout::getLastLine()
{
	return m_pLastLine;
}

fp_Line* fl_BlockLayout::getFirstLine()
{
	return m_pFirstLine;
}

fp_Line* fl_BlockLayout::findPrevLineInDocument(fp_Line* pLine)
{
	if (pLine->getPrev())
	{
		return pLine->getPrev();
	}
	else
	{
		if (m_pPrev)
		{
			return m_pPrev->getLastLine();
		}
		else
		{
			fl_SectionLayout* pSL = m_pSectionLayout->getPrev();

			if (!pSL)
			{
				// at EOD, so just bail
				return NULL;
			}

			fl_BlockLayout* pBlock = pSL->getLastBlock();
			UT_ASSERT(pBlock);
			return pBlock->getLastLine();
		}
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fp_Line* fl_BlockLayout::findNextLineInDocument(fp_Line* pLine)
{
	if (pLine->getNext())
	{
		return pLine->getNext();
	}
	
	if (m_pNext)
	{
		// grab the first line from the next block
		return m_pNext->getFirstLine();
	}
	else
	{
		// there is no next line in this section, try the next
		fl_SectionLayout* pSL = m_pSectionLayout->getNext();

		if (!pSL)
		{
			// at EOD, so just bail
			return NULL;
		}

		fl_BlockLayout* pBlock = pSL->getFirstBlock();
		UT_ASSERT(pBlock);
		return pBlock->getFirstLine();
	}
 
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

fl_BlockLayout* fl_BlockLayout::getNextBlockInDocument(void) const
{
	if (m_pNext)
	{
		return m_pNext;
	}

	// keep going (check next section)
	fl_SectionLayout* pSL = m_pSectionLayout->getNext();
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getFirstBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

fl_BlockLayout* fl_BlockLayout::getPrevBlockInDocument(void) const
{
	if (m_pPrev)
		return m_pPrev;

	// keep going (check prev section)
	fl_SectionLayout* pSL = m_pSectionLayout->getPrev();
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getLastBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

void fl_BlockLayout::dump()
{
}

/*****************************************************************/
/*****************************************************************/

fl_PartOfBlock::fl_PartOfBlock(void)
{
	iOffset = 0;
	iLength = 0;
}

UT_Bool fl_PartOfBlock::doesTouch(UT_uint32 offset, UT_uint32 length) const
{
	UT_uint32 start1, end1, start2, end2;

	start1 = iOffset;
	end1 =   iOffset + iLength;
	start2 = offset;
	end2 =   offset + length;

	if (end1 == start2)
	{
		return UT_TRUE;
	}
	if (end2 == start1)
	{
		return UT_TRUE;
	}

	/* they overlap */
	if ((start1 <= start2) && (start2 <= end1))
	{
		return UT_TRUE;
	}
	if ((start2 <= start1) && (start1 <= end2))
	{
		return UT_TRUE;
	}

	return UT_FALSE;
}

/*****************************************************************/
/*****************************************************************/

void fl_BlockLayout::_purgeSquiggles(void)
{
	UT_VECTOR_PURGEALL(fl_PartOfBlock *, m_vecSquiggles);

	m_vecSquiggles.clear();
}

UT_sint32 fl_BlockLayout::_findSquiggle(UT_uint32 iOffset) const
{
	// get the squiggle which spans iOffset, if any
	UT_sint32 res = -1;

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=0; j<iSquiggles; j++)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j);

		if ((pPOB->iOffset <= iOffset) && 
			((pPOB->iOffset + pPOB->iLength) >= iOffset))
		{
			res = j;
			break;
		}
	}

	return res;
}

void fl_BlockLayout::_addSquiggle(UT_uint32 iOffset, UT_uint32 iLen)
{
	fl_PartOfBlock*	pPOB = new fl_PartOfBlock();
	if (!pPOB)
	{
		// TODO handle outofmem
	}
	
	pPOB->iOffset = iOffset;
	pPOB->iLength = iLen;

	m_vecSquiggles.addItem(pPOB);

	_updateSquiggle(pPOB);
}

void fl_BlockLayout::_updateSquiggle(fl_PartOfBlock* pPOB)
{
	FV_View* pView = m_pLayout->getView();

	PT_DocPosition pos1 = getPosition() + pPOB->iOffset;
	PT_DocPosition pos2 = pos1 + pPOB->iLength;

	pView->_clearBetweenPositions(pos1, pos2, UT_TRUE);
}

void fl_BlockLayout::_insertSquiggles(UT_uint32 iOffset, UT_uint32 iLength, fl_BlockLayout* pBlock)
{
#if 0
	UT_sint32 chg = iLength;

	// first deal with pending word, if any
	if (m_pLayout->isPendingWord())
	{
		if (!m_pLayout->touchesPendingWord(this, iOffset, 0))
		{
			// not affected by insert, so check it
			m_pLayout->checkPendingWord();
		}
	}

	// remove squiggle broken by this insert
	UT_sint32 iBroken = _findSquiggle(iOffset);
	if (iBroken >= 0)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(iBroken);
		_updateSquiggle(pPOB);
		m_vecSquiggles.deleteNthItem(iBroken);
		delete pPOB;
	}

	// move all trailing squiggles
	_moveSquiggles(iOffset, chg, pBlock);

	// recheck at boundary
	_recalcPendingWord(iOffset, chg);
#else
	m_pLayout->queueBlockForSpell(this);
#endif
}

void fl_BlockLayout::_deleteSquiggles(UT_uint32 iOffset, UT_uint32 iLength, fl_BlockLayout* pBlock)
{
#if 0
	UT_sint32 chg = -(UT_sint32)iLength;

	// first deal with pending word, if any
	if (m_pLayout->isPendingWord())
	{
		if (!m_pLayout->touchesPendingWord(this, iOffset, chg))
		{
			// not affected by delete, so check it
			m_pLayout->checkPendingWord();
		}
	}

	// remove all deleted squiggles
	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=iSquiggles; j>0; j--)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j-1);

		if (pPOB->doesTouch(iOffset, iLength))
		{
			_updateSquiggle(pPOB);
			m_vecSquiggles.deleteNthItem(j-1);
			delete pPOB;
		}
	}

	// move all trailing squiggles
	_moveSquiggles(iOffset, chg, pBlock);

	// recheck at boundary
	_recalcPendingWord(iOffset, chg);

	// check the newly pending word
	m_pLayout->checkPendingWord();
#else
	m_pLayout->queueBlockForSpell(this);
#endif
}

void fl_BlockLayout::_recalcPendingWord(UT_uint32 iOffset, UT_sint32 chg)
{
	UT_ASSERT(chg);

	// on exit, there's either a single unchecked pending word, or nothing

	UT_GrowBuf pgb(1024);
	UT_Bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);

	UT_uint32 iFirst = iOffset;
	UT_uint32 iAbs = (UT_uint32) ((chg > 0) ? chg : -chg);
	UT_uint32 iLen = iAbs;

	/*
	  first, we expand this region outward until we get a word delimiter
	  on each side
	*/
	while ((iFirst > 0) && UT_isWordDelimiter(pBlockText[iFirst]))
	{
		iFirst--;
	}

	while ((iFirst > 0) && !UT_isWordDelimiter(pBlockText[iFirst]))
	{
		iFirst--;
	}
	if (UT_isWordDelimiter(pBlockText[iFirst]))
	{
		iFirst++;
	}

	UT_ASSERT(iOffset>=iFirst);
	iLen += (iOffset-iFirst);

	UT_uint32 iBlockSize = pgb.getLength();
	while ((iFirst + iLen < iBlockSize) && !UT_isWordDelimiter(pBlockText[iFirst + iLen]))
	{
		iLen++;
	}

	/*
		then we figure out what to do with this expanded span 
	*/
	if (chg > 0)
	{
		// insert
		
		// TODO: consume/check all words except perhaps the last one
	}
	else
	{
		// delete

		fl_PartOfBlock* pPending = NULL;

		if (m_pLayout->isPendingWord())
		{
			pPending = m_pLayout->getPendingWord();
			UT_ASSERT(pPending);
		}
			
		UT_ASSERT(chg < 0);

		// TODO: no need to create if this assert fires?  
		UT_ASSERT((iFirst+iLen) > (iOffset+iAbs));

		if (!pPending)
		{
			pPending = new fl_PartOfBlock();
			UT_ASSERT(pPending);
		}

		if (pPending)
		{
			// WARNING: old content not gone from blockbuf yet, thus funky math
			pPending->iOffset = iFirst;
			pPending->iLength = iLen - iAbs;
			
			m_pLayout->setPendingWord(this, pPending);
		}
	}
}

void fl_BlockLayout::_moveSquiggles(UT_uint32 iOffset, UT_sint32 chg, fl_BlockLayout* pNewBlock)
{
	UT_ASSERT(chg);

	// move existing squiggles to reflect insert/delete at iOffset
	// all subsequent squiggles should be switched to (non-null) pBlock

	UT_uint32 target = (chg > 0) ? iOffset : (UT_uint32)((UT_sint32)iOffset - chg);

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=iSquiggles; j>0; j--)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j-1);

		// only interested in squiggles after change
		// overlapping squiggles get handled elsewhere
		if (pPOB->iOffset >= target)
		{
			// yep, this one moves
			pPOB->iOffset = (UT_uint32)((UT_sint32)pPOB->iOffset + chg);

			if (pNewBlock)
			{
				UT_ASSERT(pNewBlock != this);
				UT_ASSERT(chg < 0);
					
				// move squiggle to another block
				pNewBlock->m_vecSquiggles.addItem(pPOB);
				m_vecSquiggles.deleteNthItem(j-1);
			}	
		}
	}
}

/*****************************************************************/
/*****************************************************************/

void fl_BlockLayout::checkSpelling(void)
{
	// destructively recheck the entire block
	// called from timer context, so we need to toggle IP

	UT_GrowBuf pgb(1024);
	UT_Bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);
	UT_uint32 eor = pgb.getLength(); /* end of region */

	FV_View* pView = m_pLayout->getView();
	UT_Bool bUpdateScreen = UT_FALSE;


	// remove any existing squiggles from the screen...
	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=0; j<iSquiggles; j++)
	{
		fl_PartOfBlock* pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j);

		if (pPOB->iOffset < eor)
		{
			// this one's still in the block
			if ((pPOB->iOffset + pPOB->iLength) > eor)
			{
				pPOB->iLength = eor - pPOB->iOffset;
			}

			if (!bUpdateScreen)
			{
				bUpdateScreen = UT_TRUE;
				if (pView)
					pView->_eraseInsertionPoint();
			}

			_updateSquiggle(pPOB);
		}
	}

	// ... and forget about them
	_purgeSquiggles();

	// now start checking
	UT_uint32 wordBeginning = 0, wordLength = 0;
	UT_Bool bFound;
	UT_Bool bAllUpperCase;

	while (wordBeginning < eor)
	{
		// skip delimiters...
		while ((wordBeginning < eor) && (UT_isWordDelimiter( pBlockText[wordBeginning])))
		{
			wordBeginning++;
		}

		// ignore initial quote
		if (pBlockText[wordBeginning] == '\'')
			wordBeginning++;

		if (wordBeginning < eor)
		{
			// we're at the start of a word. find end of word
			bFound = UT_FALSE;
			bAllUpperCase = UT_TRUE;
			wordLength = 0;
			while ((!bFound) && ((wordBeginning + wordLength) < eor))
			{
				if ( UT_TRUE == UT_isWordDelimiter( pBlockText[wordBeginning + wordLength] ))
				{
					bFound = UT_TRUE;
				}
				else
				{
					if (bAllUpperCase)
						bAllUpperCase = UT_UCS_isupper(pBlockText[wordBeginning + wordLength]);

					wordLength++;
				}
			}

			// ignore terminal quote
			if (pBlockText[wordBeginning + wordLength - 1] == '\'')
				wordLength--;

			// for some reason, the spell checker fails on all 1-char words & really big ones
			if ((wordLength > 1) && 
				(!bAllUpperCase) &&		// TODO: iff relevant Option is set
				(!UT_UCS_isdigit(pBlockText[wordBeginning]) && 
				(wordLength < 100)))
			{
				if (! SpellCheckNWord16( &(pBlockText[wordBeginning]), wordLength))
				{
					// unknown word...
					if (!bUpdateScreen)
					{
						bUpdateScreen = UT_TRUE;
						if (pView)
							pView->_eraseInsertionPoint();
					}

					// squiggle it
					_addSquiggle(wordBeginning, wordLength);
				}
			}

			wordBeginning += (wordLength + 1);
		}
	}

	if (bUpdateScreen && pView)
	{
		pView->_updateScreen();
		pView->_drawInsertionPoint();
	}
}

void fl_BlockLayout::checkWord(fl_PartOfBlock* pPOB)
{
	UT_ASSERT(pPOB);
	if (!pPOB)
		return;

	// consume word in pPOB -- either squiggle or delete it

	UT_GrowBuf pgb(1024);
	UT_Bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);
	UT_uint32 eor = pPOB->iOffset + pPOB->iLength; /* end of region */

	UT_uint32 wordBeginning = pPOB->iOffset, wordLength = 0;
	UT_Bool bAllUpperCase = UT_FALSE;

	UT_ASSERT(wordBeginning <= pgb.getLength());
	UT_ASSERT(eor <= pgb.getLength());

	while (!bAllUpperCase && ((wordBeginning + wordLength) < eor))
	{
		UT_ASSERT(!UT_isWordDelimiter( pBlockText[wordBeginning + wordLength] ));

		if (bAllUpperCase)
			bAllUpperCase = UT_UCS_isupper(pBlockText[wordBeginning + wordLength]);

		wordLength++;
	}

	wordLength = pPOB->iLength;

	// for some reason, the spell checker fails on all 1-char words & really big ones
	if ((wordLength > 1) && 
		(!bAllUpperCase) &&		// TODO: iff relevant Option is set
		(!UT_UCS_isdigit(pBlockText[wordBeginning]) && 
		(wordLength < 100)))
	{
		if (! SpellCheckNWord16( &(pBlockText[wordBeginning]), wordLength))
		{
			// squiggle it
			m_vecSquiggles.addItem(pPOB);

			_updateSquiggle(pPOB);
		}
		else
		{
			// forget about it
			delete pPOB;
		}
	}
}

/*****************************************************************/
/*****************************************************************/

UT_Bool fl_BlockLayout::doclistener_populateSpan(const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
	PT_BufIndex bi = pcrs->getBufIndex();
	const UT_UCSChar* pChars = m_pDoc->getPointer(bi);

	/*
	  walk through the characters provided and find any
	  control characters.  Then, each control character gets
	  handled specially.  Normal characters get grouped into
	  runs as usual.
	*/
	UT_uint32 	iNormalBase = 0;
	UT_Bool		bNormal = UT_FALSE;
	UT_uint32 i;
	for (i=0; i<len; i++)
	{
		switch (pChars[i])
		{
		case UCS_FF:	// form feed, forced page break
		case UCS_VTAB:	// vertical tab, forced column break
		case UCS_LF:	// newline
		case UCS_TAB:	// tab
			if (bNormal)
			{
				_doInsertTextSpan(iNormalBase + blockOffset, i - iNormalBase);
				bNormal = UT_FALSE;
			}

			/*
			  Now, depending upon the kind of control char we found,
			  we add a control run which corresponds to it.
			*/
			switch (pChars[i])
			{
			case UCS_FF:
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;
				
			case UCS_VTAB:
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;
				
			case UCS_LF:
				_doInsertForcedLineBreakRun(i + blockOffset);
				break;
				
			case UCS_TAB:
				_doInsertTabRun(i + blockOffset);
				break;
				
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
			break;
			
		default:
			if (!bNormal)
			{
				bNormal = UT_TRUE;
				iNormalBase = i;
			}
			break;
		}
	}

	UT_ASSERT(i == len);

	if (bNormal && (iNormalBase < i))
	{
		_doInsertTextSpan(iNormalBase + blockOffset, i - iNormalBase);
	}

	return UT_TRUE;
}

UT_Bool	fl_BlockLayout::_doInsertTextSpan(PT_BlockOffset blockOffset, UT_uint32 len)
{
	fp_TextRun* pNewRun = new fp_TextRun(this, m_pLayout->getGraphics(), blockOffset, len);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	if (_doInsertRun(pNewRun))
	{
#if 0
		/*
		  This code is an attempt to coalesce text runs on the fly.
		  It fails because the newly merged run is half-dirty,
		  half-not.  The newly inserted portion is clearly dirty.
		  It has not even been drawn on screen yet.  The previously
		  existent portion is not dirty.  If we want to do this
		  merge, then we have two choices.  First, we could
		  erase the old portion, merge, and consider the result
		  to be dirty.  OR, we could draw the new portion on
		  screen, merge, and consider the result to be NOT
		  dirty.  The first approach causes flicker.  The second
		  approach won't work now since we don't know the
		  position of the layout.
		*/
		
		fp_Run* pPrev = pNewRun->getPrev();
		if (
			pPrev
			&& (pPrev->getType() == FPRUN_TEXT)
			)
		{
			fp_TextRun* pPrevTextRun = (fp_TextRun*) pPrev;
			
			if (pPrevTextRun->canMergeWithNext())
			{
				pPrevTextRun->mergeWithNext();
			}
		}
#endif
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool	fl_BlockLayout::_doInsertForcedLineBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedLineBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_Bool	fl_BlockLayout::_doInsertForcedPageBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedPageBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_Bool	fl_BlockLayout::_doInsertForcedColumnBreakRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_ForcedColumnBreakRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_Bool	fl_BlockLayout::_doInsertTabRun(PT_BlockOffset blockOffset)
{
	fp_Run* pNewRun = new fp_TabRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_Bool	fl_BlockLayout::_doInsertImageRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
	GR_Image* pImage = NULL;

	/*
	  Get the attribute list for this offset, lookup the dataid
	  for the image, and get the dataItem.  The bytes in the
	  dataItem should be a PNG image.
	*/
	
	const PP_AttrProp * pSpanAP = NULL;
	UT_Bool bFoundSpanAP = getSpanAttrProp(blockOffset + fl_BLOCK_STRUX_OFFSET,&pSpanAP);
	if (bFoundSpanAP && pSpanAP)
	{
		const XML_Char* pszDataID = NULL;
		UT_Bool bFoundDataID = pSpanAP->getAttribute("dataid", pszDataID);
		if (bFoundDataID && pszDataID)
		{
			const UT_ByteBuf* pBB = NULL;

			UT_Bool bFoundDataItem = m_pDoc->getDataItemDataByName(pszDataID, &pBB, NULL, NULL);
			if (bFoundDataItem && pBB)
			{
				GR_Graphics* pG = m_pLayout->getGraphics();

				/*
				  Now we need to know the display size of the new image.
				*/

				const XML_Char *pszWidth;
				const XML_Char *pszHeight;
				UT_Bool bFoundWidthProperty = pSpanAP->getProperty("width", pszWidth);
				UT_Bool bFoundHeightProperty = pSpanAP->getProperty("height", pszHeight);

				UT_sint32 iDisplayWidth = 0;
				UT_sint32 iDisplayHeight = 0;
				if (bFoundWidthProperty && bFoundHeightProperty && pszWidth && pszHeight && pszWidth[0] && pszHeight[0])
				{
					iDisplayWidth = pG->convertDimension(pszWidth);
					iDisplayHeight = pG->convertDimension(pszHeight);
				}
				else
				{
					UT_sint32 iImageWidth;
					UT_sint32 iImageHeight;

					UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
	
					if (pG->queryProperties(GR_Graphics::DGP_SCREEN))
					{
						iDisplayWidth = iImageWidth;
						iDisplayHeight = iImageHeight;
					}
					else
					{
						double fScale = pG->getResolution() / 72.0;
			
						iDisplayWidth = (UT_sint32) (iImageWidth * fScale);
						iDisplayHeight = (UT_sint32) (iImageHeight * fScale);
					}
				}

				UT_ASSERT(iDisplayWidth > 0);
				UT_ASSERT(iDisplayHeight > 0);

				pImage = pG->createNewImage(pszDataID, pBB, iDisplayWidth, iDisplayHeight);
			}
		}
	}
	
	fp_ImageRun* pNewRun = new fp_ImageRun(this, m_pLayout->getGraphics(), blockOffset, 1, pImage);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_Bool	fl_BlockLayout::_doInsertFieldRun(PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
	fp_FieldRun* pNewRun = new fp_FieldRun(this, m_pLayout->getGraphics(), blockOffset, 1);
	UT_ASSERT(pNewRun);	// TODO check for outofmem

	return _doInsertRun(pNewRun);
}

UT_Bool	fl_BlockLayout::_doInsertRun(fp_Run* pNewRun)
{
	PT_BlockOffset blockOffset = pNewRun->getBlockOffset();
	UT_uint32 len = pNewRun->getLength();
	
#ifndef NDEBUG	
	_assertRunListIntegrity();
#endif
	
	m_gbCharWidths.ins(blockOffset, len);
	if (pNewRun->getType() == FPRUN_TEXT)
	{
		fp_TextRun* pNewTextRun = (fp_TextRun*) pNewRun;
		
		pNewTextRun->fetchCharWidths(&m_gbCharWidths);
		pNewTextRun->recalcWidth();
	}
	
	if (m_pFirstRun && !(m_pFirstRun->getNext()) && (m_pFirstRun->getLength() == 0))
	{
		/*
		  We special case the situation where we are inserting into
		  a block which has nothing but the fake run.
		*/

		m_pFirstRun->getLine()->removeRun(m_pFirstRun);
		delete m_pFirstRun;
		m_pFirstRun = NULL;

		m_pFirstRun = pNewRun;
		m_pLastLine->addRun(pNewRun);

		return UT_TRUE;
	}
	
	UT_Bool bInserted = UT_FALSE;
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		
		if (
			((iRunBlockOffset + iRunLength) <= blockOffset)
			)
		{
			// nothing to do.  the insert occurred AFTER this run
		}
		else if (iRunBlockOffset > blockOffset)
		{
			UT_ASSERT(bInserted);
			
			// the insert is occuring BEFORE this run, so we just move the run offset
			pRun->setBlockOffset(iRunBlockOffset + len);
		}
		else if (iRunBlockOffset == blockOffset)
		{
			UT_ASSERT(!bInserted);

			bInserted = UT_TRUE;
			
			// the insert is right before this run.
			pRun->setBlockOffset(iRunBlockOffset + len);

			pNewRun->setPrev(pRun->getPrev());
			pNewRun->setNext(pRun);
			if (pRun->getPrev())
			{
				pRun->getPrev()->setNext(pNewRun);
			}

			pRun->setPrev(pNewRun);

			if (m_pFirstRun == pRun)
			{
				m_pFirstRun = pNewRun;
			}

			pRun->getLine()->insertRunBefore(pNewRun, pRun);
		}
		else
		{
			UT_ASSERT(!bInserted);
			
			UT_ASSERT(FP_RUN_INSIDE == pRun->containsOffset(blockOffset));
			UT_ASSERT(pRun->getType() == FPRUN_TEXT);	// only textual runs can be split anyway

			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
			pTextRun->split(blockOffset);
			
			UT_ASSERT(pRun->getNext());
			UT_ASSERT(pRun->getNext()->getBlockOffset() == blockOffset);

			UT_ASSERT(pTextRun->getNext());
			UT_ASSERT(pTextRun->getNext()->getType() == FPRUN_TEXT);
			
			fp_TextRun* pOtherHalfOfSplitRun = (fp_TextRun*) pTextRun->getNext();
			
			pTextRun->recalcWidth();

			bInserted = UT_TRUE;
			
			pRun = pRun->getNext();
			
			iRunBlockOffset = pRun->getBlockOffset();
			iRunLength = pRun->getLength();

			UT_ASSERT(iRunBlockOffset == blockOffset);
			
			// the insert is right before this run.
			pRun->setBlockOffset(iRunBlockOffset + len);

			pNewRun->setPrev(pRun->getPrev());
			pNewRun->setNext(pRun);
			if (pRun->getPrev())
			{
				pRun->getPrev()->setNext(pNewRun);
			}

			pRun->setPrev(pNewRun);

			if (m_pFirstRun == pRun)
			{
				m_pFirstRun = pNewRun;
			}

			pRun->getLine()->insertRunBefore(pNewRun, pRun);
			
			pOtherHalfOfSplitRun->recalcWidth();
		}
		
		pRun = pRun->getNext();
	}

	if (!bInserted)
	{
		pRun = m_pFirstRun;
		fp_Run * pLastRun = NULL;
		UT_uint32 offset = 0;
		while (pRun)
		{
			pLastRun = pRun;
			offset += pRun->getLength();
			pRun = pRun->getNext();
		}

		UT_ASSERT(offset==blockOffset);

		if (pLastRun)
		{
			pLastRun->setNext(pNewRun);
			pNewRun->setPrev(pLastRun);
		}
		else
		{
			m_pFirstRun = pNewRun;
		}

		if (m_pLastLine)
		{
			m_pLastLine->addRun(pNewRun);
		}
	}

#ifndef NDEBUG	
	_assertRunListIntegrity();
#endif
	
	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_insertSpan(const PX_ChangeRecord_Span * pcrs)
{
	UT_ASSERT(pcrs->getType()==PX_ChangeRecord::PXT_InsertSpan);

	UT_ASSERT(pcrs->getPosition() >= getPosition());
	
	PT_BlockOffset blockOffset = (pcrs->getPosition() - getPosition());
	UT_uint32 len = pcrs->getLength();
	UT_ASSERT(len>0);

	PT_BufIndex bi = pcrs->getBufIndex();
	const UT_UCSChar* pChars = m_pDoc->getPointer(bi);

	/*
	  walk through the characters provided and find any
	  control characters.  Then, each control character gets
	  handled specially.  Normal characters get grouped into
	  runs as usual.
	*/
	UT_uint32 	iNormalBase = 0;
	UT_Bool		bNormal = UT_FALSE;
	UT_uint32 i;
	for (i=0; i<len; i++)
	{
		switch (pChars[i])
		{
		case UCS_FF:	// form feed, forced page break
		case UCS_VTAB:	// vertical tab, forced column break
		case UCS_LF:	// newline
		case UCS_TAB:	// tab
			if (bNormal)
			{
				_doInsertTextSpan(blockOffset + iNormalBase, i - iNormalBase);
				bNormal = UT_FALSE;
			}

			/*
			  Now, depending upon the kind of control char we found,
			  we add a control run which corresponds to it.
			*/
			switch (pChars[i])
			{
			case UCS_FF:
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;
				
			case UCS_VTAB:
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;
				
			case UCS_LF:
				_doInsertForcedLineBreakRun(blockOffset + i);
				break;
				
			case UCS_TAB:
				_doInsertTabRun(blockOffset + i);
				break;
				
			default:
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
			break;
			
		default:
			if (!bNormal)
			{
				bNormal = UT_TRUE;
				iNormalBase = i;
			}
			break;
		}
	}

	UT_ASSERT(i == len);

	if (bNormal && (iNormalBase < i))
	{
		_doInsertTextSpan(blockOffset + iNormalBase, i - iNormalBase);
	}
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		// all done, so clear any temp formatting
		if (pView->_isPointAP())
		{
			UT_ASSERT(pcrs->getIndexAP()==pView->_getPointAP());
			pView->_clearPointAP(UT_FALSE);
		}
			
		pView->_setPoint(pcrs->getPosition()+len);
	}

	_insertSquiggles(blockOffset, len);

	return UT_TRUE;
}

#ifndef NDEBUG
void fl_BlockLayout::_assertRunListIntegrity(void)
{
	fp_Run* pRun = m_pFirstRun;
	UT_uint32 iOffset = 0;
	while (pRun)
	{
		UT_ASSERT(iOffset == pRun->getBlockOffset());

		iOffset += pRun->getLength();
		
		pRun = pRun->getNext();
	}
}
#endif /* !NDEBUG */

UT_Bool fl_BlockLayout::_delete(PT_BlockOffset blockOffset, UT_uint32 len)
{
#ifndef NDEBUG	
	_assertRunListIntegrity();
#endif

	/*
	  TODO the attempts herein to do fetchCharWidths will fail.
	*/
	
	m_gbCharWidths.del(blockOffset, len);

	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 iRunBlockOffset = pRun->getBlockOffset();
		UT_uint32 iRunLength = pRun->getLength();
		fp_Run* pNextRun = pRun->getNext();	// remember where we're going, since this run may get axed
		
		if (
			((iRunBlockOffset + iRunLength) <= blockOffset)
			)
		{
			// nothing to do.  the delete occurred AFTER this run
		}
		else if (iRunBlockOffset >= (blockOffset + len))
		{
			// the delete occurred entirely before this run.

			pRun->setBlockOffset(iRunBlockOffset - len);
		}
		else
		{
			if (blockOffset >= iRunBlockOffset)
			{
				if ((blockOffset + len) < (iRunBlockOffset + iRunLength))
				{
					// the deleted section is entirely within this run
					pRun->setLength(iRunLength - len);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					m_bFixCharWidths = UT_TRUE;
				}
				else
				{
					int iDeleted = iRunBlockOffset + iRunLength - blockOffset;
					UT_ASSERT(iDeleted > 0);

					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					m_bFixCharWidths = UT_TRUE;
				}
			}
			else
			{
				if ((blockOffset + len) < (iRunBlockOffset + iRunLength))
				{
					int iDeleted = blockOffset + len - iRunBlockOffset;
					UT_ASSERT(iDeleted > 0);
					pRun->setBlockOffset(iRunBlockOffset - (len - iDeleted));
					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					m_bFixCharWidths = UT_TRUE;
				}
				else
				{
					/*
					  the deletion spans the entire run.
					  time to delete it
					*/

					pRun->setLength(0);
				}
			}

			if (pRun->getLength() == 0)
			{
				fp_Line* pLine = pRun->getLine();
				UT_ASSERT(pLine);

				pLine->removeRun(pRun);

				if (pRun->getNext())
				{
					pRun->getNext()->setPrev(pRun->getPrev());
				}

				if (pRun->getPrev())
				{
					pRun->getPrev()->setNext(pRun->getNext());
				}

				if (m_pFirstRun == pRun)
				{
					m_pFirstRun = pRun->getNext();
				}

				delete pRun;
			}
		}

		pRun = pNextRun;
	}

#ifndef NDEBUG	
	_assertRunListIntegrity();
#endif

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_deleteSpan(const PX_ChangeRecord_Span * pcrs)
{
	UT_ASSERT(pcrs->getType()==PX_ChangeRecord::PXT_DeleteSpan);
			
	PT_BlockOffset blockOffset = (pcrs->getPosition() - getPosition());
	UT_uint32 len = pcrs->getLength();
	UT_ASSERT(len>0);

	_delete(blockOffset, len);
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
 		pView->_resetSelection();
 		pView->_setPoint(pcrs->getPosition());
	}

	_deleteSquiggles(blockOffset, len);

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_changeSpan(const PX_ChangeRecord_SpanChange * pcrsc)
{
	UT_ASSERT(pcrsc->getType()==PX_ChangeRecord::PXT_ChangeSpan);
		
	PT_BlockOffset blockOffset = (pcrsc->getPosition() - getPosition());
	UT_uint32 len = pcrsc->getLength();
	UT_ASSERT(len > 0);
					
	/*
	  The idea here is to invalidate the charwidths for 
	  the entire span whose formatting has changed.
	  We may need to split runs at one or both ends.
	*/
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		UT_uint32 runOffset = pRun->getBlockOffset();
		UT_uint32 iWhere = pRun->containsOffset(blockOffset+len);

		if (pRun->getType() == FPRUN_TEXT)
		{
			fp_TextRun* pTextRun = (fp_TextRun*) pRun;
			if ((iWhere == FP_RUN_INSIDE) && ((blockOffset+len) > runOffset))
			{
				// split at right end of span
				pTextRun->split(blockOffset+len);
				pTextRun->getNext()->recalcWidth();
			}

			iWhere = pRun->containsOffset(blockOffset);
			if ((iWhere == FP_RUN_INSIDE) && (blockOffset > runOffset))
			{
				// split at left end of span
				pTextRun->split(blockOffset);
				pTextRun->getNext()->recalcWidth();
			}

			if ((runOffset >= blockOffset) && (runOffset < blockOffset + len))
			{
				pTextRun->lookupProperties();
				pTextRun->fetchCharWidths(&m_gbCharWidths);
				pTextRun->recalcWidth();
			}
		}

		UT_ASSERT(runOffset==pRun->getBlockOffset());
		pRun = pRun->getNext();
	}

	setNeedsReformat();

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	fl_BlockLayout*	pPrevBL = m_pPrev;
	if (!pPrevBL)
	{
		UT_DEBUGMSG(("no prior BlockLayout\n"));
		return UT_FALSE;
	}

	// erase the old version
	clearScreen(m_pLayout->getGraphics());

	if ((pPrevBL->m_pFirstRun)
		&& !(pPrevBL->m_pFirstRun->getNext())
		&& (pPrevBL->m_pFirstRun->getLength() == 0))
	{
		// we have a fake run in pPrevBL.  Kill it.
		fp_Run * pNuke = pPrevBL->m_pFirstRun;

		// detach from their line
		fp_Line* pLine = pNuke->getLine();
		UT_ASSERT(pLine);
										
		pLine->removeRun(pNuke);
		delete pNuke;
		
		pPrevBL->m_pFirstRun = NULL;
	}

	/*
	  The idea here is to append the runs of the deleted block,
	  if any, at the end of the previous block.
	*/
	UT_uint32 offset = 0;
	if (m_pFirstRun)
	{
		// figure out where the merge point is
		fp_Run * pRun = pPrevBL->m_pFirstRun;
		fp_Run * pLastRun = NULL;

		while (pRun)
		{
			pLastRun = pRun;
			offset += pRun->getLength();
			pRun = pRun->getNext();
		}

		// link them together
		if (pLastRun)
		{
			// skip over any zero-length runs
			pRun = m_pFirstRun;

			while (pRun && pRun->getLength() == 0)
			{
				fp_Run * pNuke = pRun;
				
				pRun = pNuke->getNext();

				// detach from their line
				fp_Line* pLine = pNuke->getLine();
				UT_ASSERT(pLine);
										
				pLine->removeRun(pNuke);
										
				delete pNuke;
			}

			m_pFirstRun = pRun;

			// then link what's left
			pLastRun->setNext(m_pFirstRun);

			if (m_pFirstRun)
			{
				m_pFirstRun->setPrev(pLastRun);
			}
		}
		else
		{
			pPrevBL->m_pFirstRun = m_pFirstRun;
		}

		// merge charwidths
		UT_uint32 lenNew = m_gbCharWidths.getLength();

		pPrevBL->m_gbCharWidths.ins(offset, m_gbCharWidths.getPointer(0), lenNew);

		fp_Line* pLastLine = pPrevBL->getLastLine();
		
		// tell all the new runs where they live
		pRun = m_pFirstRun;
		while (pRun)
		{
			pRun->setBlockOffset(pRun->getBlockOffset() + offset);
			pRun->setBlock(pPrevBL);
			
			// detach from their line
			fp_Line* pLine = pRun->getLine();
			UT_ASSERT(pLine);
			
			pLine->removeRun(pRun);

			pLastLine->addRun(pRun);

			pRun = pRun->getNext();
		}

		// runs are no longer attached to this block
		m_pFirstRun = NULL;
	}

	// get rid of everything else about the block
	purgeLayout();

	pPrevBL->m_pNext = m_pNext;
							
	if (m_pNext)
	{
		m_pNext->m_pPrev = pPrevBL;
	}

	fl_SectionLayout* pSL = m_pSectionLayout;
	UT_ASSERT(pSL);
	pSL->removeBlock(this);

#if 0
	// move all squiggles to previous block
	_deleteSquiggles(0, offset, pPrevBL);
	// TODO: instead, merge squiggles from the two blocks
	// TODO: just check boundary word
#else
	m_pLayout->queueBlockForSpell(pPrevBL);
#endif 
	// in case we've never checked this one
	m_pLayout->dequeueBlock(this);

	// update the display
//	pPrevBL->_lookupProperties();	// TODO: this may be needed
	pPrevBL->setNeedsReformat();

	FV_View* pView = pPrevBL->m_pLayout->getView();
	if (pView)
	{
		pView->_setPoint(pcrx->getPosition());
	}

	delete this;			// TODO whoa!  this construct is VERY dangerous.
	
	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	// erase the old version
	clearScreen(m_pLayout->getGraphics());
	setAttrPropIndex(pcrxc->getIndexAP());

	_lookupProperties();

	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		pLine->recalcHeight();	// line-height
		pLine->recalcMaxWidth();

		pLine = pLine->getNext();
	}
	
	setNeedsReformat();

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_insertBlock(const PX_ChangeRecord_Strux * pcrx,
												PL_StruxDocHandle sdh,
												PL_ListenerId lid,
												void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																		PL_ListenerId lid,
																		PL_StruxFmtHandle sfhNew))
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);
					
	fl_SectionLayout* pSL = m_pSectionLayout;
	UT_ASSERT(pSL);
	fl_BlockLayout*	pNewBL = pSL->insertBlock(sdh, this, pcrx->getIndexAP());
	if (!pNewBL)
	{
		UT_DEBUGMSG(("no memory for BlockLayout\n"));
		return UT_FALSE;
	}

	// must call the bind function to complete the exchange
	// of handles with the document (piece table) *** before ***
	// anything tries to call down into the document (like all
	// of the view listeners).

	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pNewBL;
	pfnBindHandles(sdh,lid,sfhNew);
	
	/*
	  The idea here is to divide the runs of the existing block 
	  into two equivalence classes.  This may involve 
	  splitting an existing run.  
	  
	  All runs and lines remaining in the existing block are
	  fine, although the last run should be redrawn.
	  
	  All runs in the new block need their offsets fixed, and 
	  that entire block needs to be formatted from scratch. 
	*/

	// figure out where the breakpoint is
	PT_BlockOffset blockOffset = (pcrx->getPosition() - getPosition());

	fp_Run* pFirstNewRun = NULL;
	fp_Run* pRun = NULL;
	
	if (0 == blockOffset)
	{
		// everything goes in new block
		pFirstNewRun = m_pFirstRun;
	}
	else
	{
		pRun = m_pFirstRun;
		while (pRun)
		{			
			UT_uint32 iWhere = pRun->containsOffset(blockOffset);

			if (iWhere == FP_RUN_INSIDE)
			{
				if (
					(blockOffset > pRun->getBlockOffset())
					)
				{
					UT_ASSERT(pRun->getType() == FPRUN_TEXT);
				
					// split here
					fp_TextRun* pTextRun = (fp_TextRun*) pRun;
				
					pTextRun->split(blockOffset);
				}
				break;
			}
			else if (iWhere == FP_RUN_JUSTAFTER)
			{
				// no split needed
				break;
			}
						
			pRun = pRun->getNext();
		}

		if (pRun)
		{
			pFirstNewRun = pRun->getNext();

			// break run sequence
			pRun->setNext(NULL);
			if (pFirstNewRun)
			{
				pFirstNewRun->setPrev(NULL);
			}
		}
	}

	// split charwidths across the two blocks
	UT_uint32 lenNew = m_gbCharWidths.getLength() - blockOffset;
	if (lenNew > 0)
	{
		// NOTE: we do the length check on the outside for speed
		pNewBL->m_gbCharWidths.ins(0, m_gbCharWidths.getPointer(blockOffset), lenNew);
		m_gbCharWidths.truncate(blockOffset);
	}

	// move remaining runs to new block
	pNewBL->m_pFirstRun = pFirstNewRun;

	pRun = pFirstNewRun;
	while (pRun)
	{
		pRun->setBlockOffset(pRun->getBlockOffset() - blockOffset);
		pRun->setBlock(pNewBL);
		pRun->fetchCharWidths(&pNewBL->m_gbCharWidths);
		pRun->recalcWidth();
						
		pRun = pRun->getNext();
	}

	// explicitly truncate rest of this block's layout
	truncateLayout(pFirstNewRun);

	pNewBL->setNeedsReformat();

	coalesceRuns();
	
	setNeedsReformat();
	
	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}

#if 0
	if (m_vecSquiggles.getItemCount() > 0)
	{
		// we have squiggles, so move them 
		_insertSquiggles(blockOffset, blockOffset, pNewBL);
		// TODO: instead, split squiggles between blocks
		// TODO: just check boundary word
		// TODO: what if this never was checked?  
	}
	else
#endif
	{
		// this block may never have been checked
		// just to be safe, let's make sure both will
		m_pLayout->queueBlockForSpell(this);
		m_pLayout->queueBlockForSpell(pNewBL);
	}
	
	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_insertSection(const PX_ChangeRecord_Strux * pcrx,
												  PL_StruxDocHandle sdh,
												  PL_ListenerId lid,
												  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																		  PL_ListenerId lid,
																		  PL_StruxFmtHandle sfhNew))
{
	// insert a section at the location given in the change record.
	// everything from this point forward (to the next section) needs
	// to be re-parented to this new section.  we also need to verify
	// that this insertion point is at the end of the block (and that
	// another block follows).  this is because because a section
	// cannot contain content.
	
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Section);
	
	fl_SectionLayout* pSL = new fl_SectionLayout(m_pLayout, sdh, pcrx->getIndexAP());
	if (!pSL)
	{
		UT_DEBUGMSG(("no memory for SectionLayout"));
		return UT_FALSE;
	}
	
	m_pLayout->insertSectionAfter(m_pSectionLayout, pSL);
	
	// must call the bind function to complete the exchange
	// of handles with the document (piece table) *** before ***
	// anything tries to call down into the document (like all
	// of the view listeners).

	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pSL;
	pfnBindHandles(sdh,lid,sfhNew);

	fl_SectionLayout* pOldSL = m_pSectionLayout;
	fl_BlockLayout* pBL = getNext();
	while (pBL)
	{
		fl_BlockLayout* pNext = pBL->getNext();

		pBL->collapse();
		pOldSL->removeBlock(pBL);
		pSL->addBlock(pBL);
		pBL->m_pSectionLayout = pSL;
		pBL->m_bNeedsReformat = UT_TRUE;

		pBL = pNext;
	}

	pOldSL->deleteEmptyColumns();

	return UT_TRUE;
}

void fl_BlockLayout::findSquigglesForRun(fp_Run* pRun)
{
	UT_uint32  runBlockOffset = pRun->getBlockOffset();
	UT_uint32  runLength = pRun->getLength();
	fl_PartOfBlock*	pPOB;

	/* For all misspelled words in this run, call the run->drawSquiggle() method */

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 i;
	for (i=0; i<iSquiggles; i++)
	{
		pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(i);

		if (
			!(pPOB->iOffset >= (runBlockOffset + runLength))
			&& !((pPOB->iOffset + pPOB->iLength) <= runBlockOffset)
			)
		{
			UT_uint32 iStart;
			UT_uint32 iLen;
			if (pPOB->iOffset <= runBlockOffset)
			{
				iStart = runBlockOffset;
			}
			else
			{
				iStart = pPOB->iOffset;
			}
		
			if ((pPOB->iOffset + pPOB->iLength) >= (runBlockOffset + runLength))
			{
				iLen = runLength + runBlockOffset - iStart;
			}
			else
			{
				iLen = pPOB->iOffset + pPOB->iLength - iStart;
			}

			UT_ASSERT(pRun->getType() == FPRUN_TEXT);
			(static_cast<fp_TextRun*>(pRun))->drawSquiggle(iStart, iLen);
		}
	}
}

//////////////////////////////////////////////////////////////////
// Object-related stuff
//////////////////////////////////////////////////////////////////

UT_Bool fl_BlockLayout::doclistener_populateObject(PT_BlockOffset blockOffset,
												   const PX_ChangeRecord_Object * pcro)
{
	switch (pcro->getObjectType())
	{
	case PTO_Image:
		UT_DEBUGMSG(("Populate:InsertObject:Image:\n"));
		_doInsertImageRun(blockOffset, pcro);
		return UT_TRUE;
		
	case PTO_Field:
		UT_DEBUGMSG(("Populate:InsertObject:Field:\n"));
		_doInsertFieldRun(blockOffset, pcro);
		return UT_TRUE;
				
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool fl_BlockLayout::doclistener_insertObject(const PX_ChangeRecord_Object * pcro)
{
	PT_BlockOffset blockOffset = 0;

	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Image:\n"));
		blockOffset = (pcro->getPosition() - getPosition());
		_doInsertImageRun(blockOffset, pcro);
		break;
	}
		
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Field:\n"));
		blockOffset = (pcro->getPosition() - getPosition());
		_doInsertFieldRun(blockOffset, pcro);
		break;
	}
	
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition() + 1);
	}

	_insertSquiggles(blockOffset, 1);	// TODO: are objects always one wide?

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_deleteObject(const PX_ChangeRecord_Object * pcro)
{
	PT_BlockOffset blockOffset = 0;

	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:DeleteObject:Image:\n"));
		blockOffset = (pcro->getPosition() - getPosition());
		_delete(blockOffset, 1);
		break;
	}
	
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:DeleteObject:Field:\n"));
		blockOffset = (pcro->getPosition() - getPosition());
		_delete(blockOffset, 1);
		break;
	}		

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
	
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition());
	}

	_deleteSquiggles(blockOffset, 1);	// TODO: are objects always one wide?

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_changeObject(const PX_ChangeRecord_ObjectChange * pcroc)
{
	switch (pcroc->getObjectType())
	{
	case PTO_Image:
		UT_DEBUGMSG(("Edit:ChangeObject:Image:\n"));
		// TODO ... deal with image object ...
		return UT_TRUE;
		
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:ChangeObject:Field:\n"));
		PT_BlockOffset blockOffset = (pcroc->getPosition() - getPosition());
		fp_Run* pRun = m_pFirstRun;
		while (pRun)
		{
			if (pRun->getBlockOffset() == blockOffset)
			{
				UT_ASSERT(pRun->getType() == FPRUN_FIELD);
				fp_FieldRun* pFieldRun = static_cast<fp_FieldRun*>(pRun);

				pFieldRun->clearScreen();
				pFieldRun->lookupProperties();

				goto done;
			}
			pRun = pRun->getNext();
		}
	
		return UT_FALSE;
	}		

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}

done:
	setNeedsReformat();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcroc->getPosition());
	}

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::recalculateFields(void)
{
	UT_Bool bResult = UT_FALSE;
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		if (pRun->getType() == FPRUN_FIELD)
		{
			fp_FieldRun* pFieldRun = (fp_FieldRun*) pRun;

			UT_Bool bSizeChanged = pFieldRun->calculateValue();

			bResult = bResult || bSizeChanged;
		}
		
		pRun = pRun->getNext();
	}

	return bResult;
}

UT_Bool	fl_BlockLayout::findNextTabStop(UT_sint32 iStartX, UT_sint32 iMaxX, UT_sint32& iPosition, unsigned char& iType)
{
	UT_uint32 iCountTabs = m_vecTabs.getItemCount();
	UT_uint32 i;
	for (i=0; i<iCountTabs; i++)
	{
		fl_TabStop* pTab = (fl_TabStop*) m_vecTabs.getNthItem(i);

		if (pTab->iPosition > iMaxX)
		{
			break;
		}
		
		if (pTab->iPosition > iStartX)
		{
			iPosition = pTab->iPosition;
			iType = pTab->iType;

			return UT_TRUE;
		}
	}
	
	// now, handle the default tabs

	if (m_iLeftMargin > iStartX)
	{
		iPosition = m_iLeftMargin;
		iType = FL_TAB_LEFT;
		return UT_TRUE;
	}
	
	UT_ASSERT(m_iDefaultTabInterval > 0);
	UT_sint32 iPos = 0;
	for (;;)
	{
		if (iPos > iStartX)
		{
			iPosition = iPos;
			iType = FL_TAB_LEFT;
			return UT_TRUE;
		}

		iPos += m_iDefaultTabInterval;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return UT_FALSE;
}

void fl_BlockLayout::setNext(fl_BlockLayout* pBL)
{
	m_pNext = pBL;
}

void fl_BlockLayout::setPrev(fl_BlockLayout* pBL)
{
	m_pPrev = pBL;
}

#ifndef NDEBUG
void fl_BlockLayout::debug_dumpRunList(void)
{
	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{
		pRun->debug_dump();
		
		pRun = pRun->getNext();
	}
}
#endif
