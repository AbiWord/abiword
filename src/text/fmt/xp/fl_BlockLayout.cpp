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
#include "px_ChangeRecord_Object.h"
#include "px_ChangeRecord_ObjectChange.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_SpanChange.h"
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord_StruxChange.h"
#include "fv_View.h"
#include "xap_App.h"
#include "xap_Clipboard.h"

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

FL_DocLayout* fl_BlockLayout::getDocLayout()
{
	return m_pLayout;
}

fl_SectionLayout * fl_BlockLayout::getSectionLayout()
{
	return m_pSectionLayout;
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
	_destroySpellCheckLists();
	purgeLayout();

	UT_VECTOR_PURGEALL(fl_TabStop *, m_vecTabs);
}

void fl_BlockLayout::_fixColumns(void)
{
	if (!m_pFirstLine)
	{
		return;
	}
	
	fp_Column* pCol = NULL;
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		if (pLine->getColumn() != pCol)
		{
			pCol = pLine->getColumn();
			pCol->updateLayout();
		}
 
		pLine = pLine->getNext();
	}

	fp_Column* pPrevCol = m_pFirstLine->getColumn()->getPrev();
	if (pPrevCol)
	{
		pPrevCol->updateLayout();
	}

	fp_Column* pNextCol = m_pLastLine->getColumn()->getNext();
	if (pNextCol)
	{
		pNextCol->updateLayout();
	}
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
	/*
	  TODO I'm not completely happy with this code.  It *should* be
	  possible to merge runs without any screen activity at all.
	*/
	
	UT_ASSERT(pFirstRunToMerge);
	UT_ASSERT(pLastRunToMerge);

	UT_sint32 iTotalWidth = 0;
	UT_sint32 iTotalLength = 0;

	fp_Run* pRun = pFirstRunToMerge;
	for (;;)
	{
		UT_ASSERT(pRun->getType() == FPRUN_TEXT);

		iTotalWidth += pRun->getWidth();
		iTotalLength += pRun->getLength();
		
		if (pRun == pLastRunToMerge)
		{
			break;
		}

		pRun = pRun->getNext();
	}

	pFirstRunToMerge->setLength(iTotalLength);

	fp_Run* pFirstRunToNuke = pFirstRunToMerge->getNext();
	pFirstRunToMerge->setNext(pLastRunToMerge->getNext());
	if (pLastRunToMerge->getNext())
	{
		pLastRunToMerge->getNext()->setPrev(pFirstRunToMerge);
	}

	pRun = pFirstRunToNuke;
	for (;;)
	{
		fp_Run* pNext = pRun->getNext();

		pRun->getLine()->removeRun(pRun);
		delete pRun;

		if (pRun == pLastRunToMerge)
		{
			break;
		}
		pRun = pNext;
	}

	pFirstRunToMerge->calcWidths(&m_gbCharWidths);
}

void fl_BlockLayout::coalesceRuns(void)
{
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
		return UT_TRUE;

	if (pTruncRun->getBlock() != this)
	{
		// be safe
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
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

#if 0	
	// remove empty lines 
	while (m_pLastLine && (m_pLastLine != m_pFirstLine))
	{
		if (m_pLastLine->countRuns())
			break;

		fp_Line* pLine = m_pLastLine;
		m_pLastLine = m_pLastLine->getPrev();

		pLine->remove();
	}
#endif

	_removeAllEmptyLines();
	
	return UT_TRUE;
}

int fl_BlockLayout::format()
{
	if (m_pFirstRun)
	{
		if (!m_pFirstLine)
		{
			// start a new line
			fp_Line* pLine = getNewLine(m_pFirstRun->getHeight());

			fp_Run* pTempRun = m_pFirstRun;
			while (pTempRun)
			{
				pLine->addRun(pTempRun);
				pTempRun = pTempRun->getNext();
			}
		}

		recalculateFields();
		m_pBreaker->breakParagraph(this);
		_removeAllEmptyLines();
	}
	else
	{
		_removeAllEmptyLines();
		
		// we don't ... construct just enough to keep going
		GR_Graphics* pG = m_pLayout->getGraphics();
		m_pFirstRun = new fp_TextRun(this, pG, 0, 0);
		m_pFirstRun->calcWidths(&m_gbCharWidths);

		if (!m_pFirstLine)
		{
			getNewLine(m_pFirstRun->getHeight());
		}

		// the line just contains the empty run
		m_pFirstLine->addRun(m_pFirstRun);

		m_pFirstLine->layout();
	}

	_fixColumns();

	checkForWidowsAndOrphans();

	return 0;	// TODO return code
}

fp_Run* fl_BlockLayout::getFirstRun()
{
	return m_pFirstRun;
}

fp_Line* fl_BlockLayout::getNewLine(UT_sint32 iHeight)
{
	UT_ASSERT(iHeight > 0);

	/*
	  Calling fixColumns every time we need to create a new line
	  is a bit heavy, but it seems to be necessary.  Specifically,
	  there are cases where we are inserting a new line into a column
	  which is not updated, and the resulting calculations become
	  very wrong.
	*/
	_fixColumns();
	
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

		if (!(pCol->insertLineAfter(pLine, pOldLastLine, iHeight)))
		{
			fp_Column* pNextCol = pCol->getNext();
			if (!pNextCol)
			{
				pNextCol = m_pSectionLayout->getNewColumn();
			}
			UT_ASSERT(pNextCol);

			while (pCol->getLastLine() != pOldLastLine)
			{
				pCol->moveLineToNextColumn(pCol->getLastLine());
			}
			
			pNextCol->insertLineAfter(pLine, NULL, iHeight);
			UT_ASSERT(pLine->getColumn() == pNextCol);
		}
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
		
		if (!(pCol->insertLineAfter(pLine, pPrevLine, iHeight)))
		{
			pCol = pCol->getNext();
			if (!pCol)
			{
				pCol = m_pSectionLayout->getNewColumn();
			}

			UT_ASSERT(pCol);
		
			pCol->insertLineAfter(pLine, NULL, iHeight);
			UT_ASSERT(pLine->getColumn() == pCol);
		}
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

UT_uint32 fl_BlockLayout::getOrphansProperty(void) const
{
	return m_iOrphansProperty;
}

UT_uint32 fl_BlockLayout::getWidowsProperty(void) const
{
	return m_iWidowsProperty;
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
			fl_SectionLayout* pSL = m_pLayout->getPrevSection(m_pSectionLayout);

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
		fl_SectionLayout* pSL = m_pLayout->getNextSection(m_pSectionLayout);

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

fl_BlockLayout* fl_BlockLayout::getNext(UT_Bool bKeepGoing) const
{
	if (m_pNext || !bKeepGoing)
		return m_pNext;

	// keep going (check next section)
	fl_SectionLayout* pSL = m_pLayout->getNextSection(m_pSectionLayout);
	fl_BlockLayout* pBL = NULL;

	if (pSL)
	{
		pBL = pSL->getFirstBlock();
		UT_ASSERT(pBL);
	}

	return pBL;
}

fl_BlockLayout* fl_BlockLayout::getPrev(UT_Bool bKeepGoing) const
{
	if (m_pPrev || !bKeepGoing)
		return m_pPrev;

	// keep going (check prev section)
	fl_SectionLayout* pSL = m_pLayout->getPrevSection(m_pSectionLayout);
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

fl_PartOfBlock::fl_PartOfBlock(void)
{
	iOffset = 0;
	iLength = 0;
}

void fl_BlockLayout::_destroySpellCheckLists(void)
{
	UT_VECTOR_PURGEALL(fl_PartOfBlock *, m_vecSquiggles);

	m_vecSquiggles.clear();
}

#if 0
/* if two regions touch or overlap, return true, else false */
static UT_Bool doesTouch(fl_PartOfBlock *pPOB, UT_uint32 offset, UT_uint32 length)
{
	UT_uint32 start1, end1, start2, end2;

	start1 = pPOB->iOffset;
	end1 =   pPOB->iOffset + pPOB->iLength;
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
#endif

void fl_BlockLayout::_addPartNotSpellChecked(UT_uint32 iOffset, UT_uint32 iLen)
{
	// -------------------------
	// -------------------------
	// -------------------------
	/*
	  TODO HACK
	  for now, checkSpelling has been modified to check the entire block
	  every time.  So, for now, we don't actually build the list of
	  regions which need spell checking.  We just add the block to the queue.
	*/
	m_pLayout->queueBlockForSpell(this);
	// -------------------------
	// -------------------------
	// -------------------------

#if 0
	/*
	  EWS note:  this version of this method was intended to simplify
	  things by keeping only ONE unchecked region instead of a list.
	*/
	/*
	  first, we expand this region outward until we get a word delimiter
	  on each side
	*/
	
	UT_GrowBuf pgb(1024);

	UT_Bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);

	while ((iOffset > 0) && UT_isWordDelimiter(pBlockText[iOffset]))
	{
		iOffset--;
	}

	while ((iOffset > 0) && !UT_isWordDelimiter(pBlockText[iOffset]))
	{
		iOffset--;
	}
	if (UT_isWordDelimiter(pBlockText[iOffset]))
	{
		iOffset++;
	}

	UT_uint32 iBlockSize = pgb.getLength();
	while ((iOffset + iLen < iBlockSize) && !UT_isWordDelimiter(pBlockText[iOffset + iLen]))
	{
		iLen++;
	}

	fl_PartOfBlock*	pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.head();

	if (pPOB)
	{
		if (pPOB->iOffset > iOffset)
		{
			pPOB->iOffset = iOffset;
		}

		if ((pPOB->iOffset + pPOB->iLength) < (iOffset + iLen))
		{
			pPOB->iLength = iOffset + iLen - pPOB->iOffset;
		}
	}
	else
	{
		pPOB = new fl_PartOfBlock();
		if (!pPOB)
		{
			// TODO handle outofmem
		}
		
		pPOB->iOffset = iOffset;
		pPOB->iLength = iLen;
	
		(void) m_lstNotSpellChecked.tail();
		m_lstNotSpellChecked.append(pPOB);
	}

    pPOB = (fl_PartOfBlock *) m_vecSquiggles.head();
	while ((pPOB != (fl_PartOfBlock *) 0))
	{
		if (doesTouch(pPOB, iOffset, iLen))
		{
			m_vecSquiggles.remove();
			pPOB = (fl_PartOfBlock *) m_vecSquiggles.current();
		}
		else
		{
			pPOB = (fl_PartOfBlock *) m_vecSquiggles.next();
		}
	}

	m_pLayout->queueBlockForSpell(this);
#endif
	
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

	/*
	  EWS note -- the following version of this method is the one DaveT
	  wrote.
	*/
#if 0

	UT_Bool foundAdjoiningRegion = UT_FALSE;
    UT_uint32 invalidStart, invalidLength;

/*

***** if region touches a bad word, absorb it into invalid region

TODO:
This code needs to be in here to be in here... however at the 
moment, its presence is causing an assert later on of an offset
extending beyond the block size... no time fix now.
*/

#if 1
    pPOB = (fl_PartOfBlock *) m_vecSquiggles.head();
	while ((pPOB != (fl_PartOfBlock *) 0))
	{
		if (doesTouch(pPOB, iOffset, iLen))
		{
			invalidStart = ( pPOB->iOffset < iOffset) ? pPOB->iOffset : iOffset;
			invalidLength  = ((pPOB->iOffset + pPOB->iLength) > (iOffset + iLen)) ? 
				(pPOB->iOffset + pPOB->iLength) : (iOffset + iLen);
			invalidLength  = invalidLength - invalidStart;
			iOffset = invalidStart;
			iLen = invalidLength;
			m_vecSquiggles.remove();
			pPOB = (fl_PartOfBlock *) m_vecSquiggles.current();
		}
		else
		{
			pPOB = (fl_PartOfBlock *) m_vecSquiggles.next();
		}
	}
#endif
	
	// TODO: This will work fine when adding text.... but what about deletion?
	
	/* merge neighboring regions... */
#if 0
	pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.head();
	while ( (!foundAdjoiningRegion) && (pPOB != (fl_PartOfBlock *) 0))
	{
		if ((pPOB->iOffset + pPOB->iLength) == iOffset)
		{
			/* new region comes right after this region... so extend this region */
			pPOB->iLength += iLen;
			foundAdjoiningRegion = UT_TRUE;
		}
		else if ((iOffset + iLen) == pPOB->iOffset)
		{
			/* new region comes right before this region... so extend this region */
			pPOB->iOffset = iOffset;
			pPOB->iLength += iLen;
			foundAdjoiningRegion = UT_TRUE;
		} 
		else if ((pPOB->iOffset < iOffset) && (iOffset < (pPOB->iOffset + pPOB->iLength)))
		{
			/* overlapping regions... this region is in front of new region*/
			if ((pPOB->iOffset + pPOB->iLength) < (iOffset + iLen))
			{
				pPOB->iLength = iLen + (iOffset - pPOB->iOffset);
			}
			/* else, 
			   new region is a subset of this region... do nothing 
			*/

			foundAdjoiningRegion = UT_TRUE;
		}
		else if ((iOffset < pPOB->iOffset) && (pPOB->iOffset < (iOffset + iLen)))
		{
			/* overlapping regions... new region is in front of this region */
			if ((iOffset + iLen) < (pPOB->iOffset + pPOB->iLength))
			{
				pPOB->iOffset = iOffset;
				pPOB->iLength = pPOB->iLength + (pPOB->iOffset - iOffset);
			}
			else
			{
				/* new region is a superset of this region */
				pPOB->iOffset = iOffset;
				pPOB->iLength = iLen;
			}
			foundAdjoiningRegion = UT_TRUE;
		}
		else if ((iOffset == pPOB->iOffset) && (iLen == pPOB->iLength))
		{
			/* same region... do nothing */
			foundAdjoiningRegion = UT_TRUE;
		}
		
		pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.next();
	}
#endif

/********************/
	/* grow invalid region if they touch */
    pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.head();
	while ((pPOB != (fl_PartOfBlock *) 0) && (!foundAdjoiningRegion))
	{
		if (doesTouch(pPOB, iOffset,iLen))
		{
			invalidStart = ( pPOB->iOffset < iOffset) ? pPOB->iOffset : iOffset;
			invalidLength  = ((pPOB->iOffset + pPOB->iLength) > (iOffset + iLen)) ? 
				(pPOB->iOffset + pPOB->iLength) : (iOffset + iLen);
			invalidLength  = invalidLength - invalidStart;

			pPOB->iOffset = invalidStart;
			pPOB->iLength = invalidLength;

			pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.current();
			foundAdjoiningRegion = UT_TRUE;
		}
		else
			pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.next();
	}

/********************/
	
	if (!foundAdjoiningRegion)
	{
		pPOB = new fl_PartOfBlock();
		if (!pPOB)
		{
			// TODO handle outofmem
		}
		
		pPOB->iOffset = iOffset;
		pPOB->iLength = iLen;
	
		(void) m_lstNotSpellChecked.tail();
		m_lstNotSpellChecked.append(pPOB);
	}

	m_pLayout->queueBlockForSpell(this);
#endif

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
}

void fl_BlockLayout::checkSpelling(void)
{
	/*
	  NOTE -- for the moment, I've disabled the spell checker
	  by inserting the 'return' below.  It crashes a lot, and
	  its handling of redraws is horrible.  --EWS
	*/

	return;
	
	// TODO: don't existing squiggles need to be cleared, too?
	_destroySpellCheckLists();


	// check the entire block
	UT_GrowBuf pgb(1024);

	UT_Bool bRes = getBlockBuf(&pgb);
	UT_ASSERT(bRes);

	const UT_UCSChar* pBlockText = pgb.getPointer(0);

	UT_uint32 wordBeginning = 0, wordLength = 0;
	UT_uint32 eor = pgb.getLength(); /* end of region */
	UT_Bool found;

	while (wordBeginning < eor)
	{
		// skip delimiters...
		while ((wordBeginning < eor) && (UT_isWordDelimiter( pBlockText[wordBeginning])))
		{
			wordBeginning++;
		}

		if (wordBeginning < eor)
		{
			// we're at the start of a word. find end of word
			found = UT_FALSE;
			wordLength = 0;
			while ((!found) && ((wordBeginning + wordLength) < eor))
			{
				if ( UT_TRUE == UT_isWordDelimiter( pBlockText[wordBeginning + wordLength] ))
				{
					found = UT_TRUE;
				}
				else
				{
					wordLength++;
				}
			}

			// for some reason, the spell checker fails on all 1-char words & really big ones
			if ((wordLength > 1) && (!isdigit(pBlockText[wordBeginning]) && (wordLength < 100)))
			{
				if (! SpellCheckNWord16( &(pBlockText[wordBeginning]), wordLength))
				{
					// unknown word... squiggle it
					_addSquiggle(wordBeginning, wordLength);
				}
			}

			wordBeginning += (wordLength + 1);
		}
	}

#if 0	
	/*
	  TODO we don't REALLY want to redraw the whole block after every spell check.
	  This is causing display dirt, since the insertion point gets erased outside
	  the context of the code which manages it.  So, thus, the following hack.

	  TODO now that I've cleaned up the rest of the redraw scenarios, the importance
	  of cleaning this one up is even higher.  This redraw MUST go.  -EWS
	*/
	m_pLayout->getView()->_eraseInsertionPoint();
	draw(m_pLayout->getGraphics());
	m_pLayout->getView()->_drawInsertionPoint();
#endif	
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
		case 12:	// form feed, forced page break
		case 11:	// vertical tab, forced column break
		case 10:	// newline
		case 9:		// tab
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
			case 12:
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;
				
			case 11:
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;
				
			case 10:
				_doInsertForcedLineBreakRun(i + blockOffset);
				break;
				
			case 9:
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

	return _doInsertRun(pNewRun);
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

			void* pHandle = NULL;
			UT_Bool bFoundDataItem = m_pDoc->getDataItemDataByName(pszDataID, &pBB, NULL, NULL);
			if (bFoundDataItem && pBB)
			{
				GR_ImageFactory* pIF = AP_App::getImageFactory();

				pImage = pIF->createNewImage(pszDataID);
				pImage->convertFromPNG(pBB);
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
		pNewRun->calcWidths(&m_gbCharWidths);
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

			pNewRun->lookupProperties();
			pNewRun->calcWidths(&m_gbCharWidths);
			pRun->calcWidths(&m_gbCharWidths);

			pRun->getLine()->insertRunBefore(pNewRun, pRun);
		}
		else
		{
			UT_ASSERT(!bInserted);
			
			UT_ASSERT(FP_RUN_INSIDE == pRun->containsOffset(blockOffset));
			UT_ASSERT(pRun->getType() == FPRUN_TEXT);	// only textual runs can be split anyway

			/*
			  We're pretty tricky.  :-)  This call is going to split
			  the run.  The result is that the next time through the loop,
			  the case above will hit, and the new run will be inserted
			  right between the two halves of the split.
			*/

			fp_TextRun* pTextRun = static_cast<fp_TextRun*>(pRun);
			pTextRun->splitSimple(blockOffset);
			
			UT_ASSERT(pRun->getNext());
			UT_ASSERT(pRun->getNext()->getBlockOffset() == blockOffset);
					
			// pick up new formatting for this run
			pRun->lookupProperties();
			pRun->calcWidths(&m_gbCharWidths);
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

		pNewRun->calcWidths(&m_gbCharWidths);
			
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

//	m_gbCharWidths.ins(blockOffset, len);

	AV_ChangeMask mask = AV_CHG_TYPING | AV_CHG_FMTCHAR;
	
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
		case 12:	// form feed, forced page break
		case 11:	// vertical tab, forced column break
		case 10:	// newline
		case 9:		// tab
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
			case 12:
				_doInsertForcedPageBreakRun(i + blockOffset);
				break;
				
			case 11:
				_doInsertForcedColumnBreakRun(i + blockOffset);
				break;
				
			case 10:
				_doInsertForcedLineBreakRun(blockOffset + i);
				break;
				
			case 9:
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
	
	format();

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
		pView->notifyListeners(mask);
	}

/***************************************************************************************/

	UT_DEBUGMSG(("insertSpan"));

	/* Update spell check lists */

    fl_PartOfBlock *pPOB;

#if SPELL_WORD
	// TODO: see how this affects m_pPendingWord
    pPOB = m_pDoc->m_pPendingWord;
	if (pPOB->iOffset > blockOffset)
	{
		/* 
		   text insertion comes before this non-spell checked region, 
		   so update the non-spell check region.
		*/
		pPOB->iOffset += len;
	}
#endif

	UT_uint32 iSquiggles = m_vecSquiggles.getItemCount();
	UT_uint32 j;
	for (j=0; j<iSquiggles; j++)
	{
		pPOB = (fl_PartOfBlock *) m_vecSquiggles.getNthItem(j);

		if (pPOB->iOffset > blockOffset)
		{
			/* 
			   text insertion comes before this spell checked/unknown word region, 
			   so update this region.
			*/
			pPOB->iOffset += len;
		}
	}

	/* Invalidate the region */
	// TODO no need to invalidate if the only char inserted was not
	// a space?
	
	_addPartNotSpellChecked(blockOffset, len);

	UT_DEBUGMSG(("insertSpan spell finished, bad words=%d",
				 m_vecSquiggles.getItemCount()));
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
					pRun->calcWidths(&m_gbCharWidths);
				}
				else
				{
					int iDeleted = iRunBlockOffset + iRunLength - blockOffset;
					UT_ASSERT(iDeleted > 0);

					pRun->setLength(iRunLength - iDeleted);
					UT_ASSERT((pRun->getLength() == 0) || (pRun->getType() == FPRUN_TEXT));	// only textual runs could have a partial deletion
					pRun->calcWidths(&m_gbCharWidths);
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
					pRun->calcWidths(&m_gbCharWidths);
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
	
	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcrs->getPosition());
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
	}

#if 0	
/*****  SPELL CHECK UPDATE follows *************************************************************/

	UT_Bool takeCurrent = UT_FALSE;

	/* update index's for non-spell checked regions */
    fl_PartOfBlock *pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.head();
	while ((pPOB) != (fl_PartOfBlock *) 0)
	{
		if ((pPOB->iOffset + pPOB->iLength) > blockOffset)
		{
			if (pPOB->iOffset < blockOffset)
			{
				if ((pPOB->iOffset + pPOB->iLength) > (blockOffset + len))
				{
					/* deletion is a subset of this region */
					pPOB->iLength -= len;
				}
				else
				{
					/* deletion overlaps the end of this region */
					pPOB->iLength = blockOffset - pPOB->iOffset;
				}
			}
			else if ((blockOffset + len) < (pPOB->iOffset + pPOB->iLength) )
			{
				if ((blockOffset +len ) < pPOB->iOffset)
				{
					/* deletion is wholly infront of this region */
					pPOB->iOffset -= len;
				}
				else
				{
					/* deletion overlaps over the front of this region */
					pPOB->iLength = (pPOB->iOffset + pPOB->iLength) - (blockOffset + len);
					pPOB->iOffset = blockOffset;
				}
			}
			else 
			{
				/* this region is a subset of the deletion... remove from list */
				m_vecSquiggles.remove();
				delete pPOB;
				
				takeCurrent = UT_TRUE;
			}
		}
		else if ((pPOB->iOffset == blockOffset) && (pPOB->iLength == len))
		{
			/* deletion _is_ this same region */
			m_vecSquiggles.remove();
			delete pPOB;
			
			takeCurrent = UT_TRUE;
		}


		if (!takeCurrent)
		{
			pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.next();
		}
		else
		{
			pPOB = (fl_PartOfBlock *) m_lstNotSpellChecked.current();
			takeCurrent = UT_FALSE;
		}
	}

	// update the misspelled word list (m_vecSquiggles)...

	takeCurrent = UT_FALSE;
	pPOB = (fl_PartOfBlock *) m_vecSquiggles.head();
	while ((pPOB) != (fl_PartOfBlock *) 0)
	{
		if ((pPOB->iOffset + pPOB->iLength) > blockOffset)
		{
			if ((blockOffset + len) < pPOB->iOffset)
			{
				/* Doesn't intersect, but it's after deletion */
				pPOB->iOffset -= len;
			}
			else
			{
				/* does intersect... invalidate the whole region  */
				beginning = (blockOffset < pPOB->iOffset)? blockOffset: pPOB->iOffset;
				end = ((pPOB->iOffset + pPOB->iLength) > (blockOffset + len)) ? 
					(pPOB->iOffset + pPOB->iLength) : (blockOffset + len);

				_addPartNotSpellChecked(beginning, (end - ((UT_uint32) beginning)) );

				/* Remove the word from list */
				m_vecSquiggles.remove();
				delete pPOB;
				takeCurrent = UT_TRUE;
			}

		} else if ((pPOB->iOffset == blockOffset) && (pPOB->iLength == len))
		{
			/* deletion region _is_ this misspelled word */
			m_vecSquiggles.remove();
			delete pPOB;
			takeCurrent = UT_TRUE;
		}
		

		if (!takeCurrent)
		{
			pPOB = (fl_PartOfBlock *) m_vecSquiggles.next();
		}
		else
		{
			pPOB = (fl_PartOfBlock *) m_vecSquiggles.current();
			takeCurrent = UT_FALSE;
		}
	
	}

	UT_DEBUGMSG(("deleteSpan spell finished, invalid regions = %d, bad words=%d",
				 m_lstNotSpellChecked.size(),
				 m_vecSquiggles.size()));

/****  End of Spell check code ***********************************************************/
#endif

	m_pLayout->queueBlockForSpell(this);

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_changeSpan(const PX_ChangeRecord_SpanChange * pcrsc)
{
	// TODO:  This span needs to be invalidated for the spell checker....

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
		if ((iWhere == FP_RUN_INSIDE) && ((blockOffset+len) > runOffset))
		{
			// split at right end of span
			pRun->split(blockOffset+len);
		}

		iWhere = pRun->containsOffset(blockOffset);
		if ((iWhere == FP_RUN_INSIDE) && (blockOffset > runOffset))
		{
			// split at left end of span
			pRun->split(blockOffset);
		}

		if ((runOffset >= blockOffset) && (runOffset < blockOffset + len))
		{
			pRun->clearScreen();
			pRun->lookupProperties();
			pRun->calcWidths(&m_gbCharWidths);
		}

		UT_ASSERT(runOffset==pRun->getBlockOffset());
		pRun = pRun->getNext();
	}

	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
	}

	UT_DEBUGMSG(("ChangeSpan spell finished, bad words=%d",
							m_vecSquiggles.getItemCount()));

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	fl_BlockLayout*	pPrevBL = m_pPrev;
	if (!pPrevBL)
	{
		UT_DEBUGMSG(("no prior BlockLayout"));
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
	if (m_pFirstRun)
	{
		// figure out where the merge point is
		fp_Run * pRun = pPrevBL->m_pFirstRun;
		fp_Run * pLastRun = NULL;
		UT_uint32 offset = 0;

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
			pRun->calcWidths(&pPrevBL->m_gbCharWidths);

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

	// update the display
//	pPrevBL->_lookupProperties();	// TODO: this may be needed
	pPrevBL->format();

	pPrevBL->_destroySpellCheckLists();
	m_pLayout->queueBlockForSpell(pPrevBL);
							
	FV_View* pView = pPrevBL->m_pLayout->getView();
	if (pView)
	{
		pView->_setPoint(pcrx->getPosition());
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR | AV_CHG_FMTBLOCK);
	}

	/*
	  TODO if this block is currently on the spell check
	  queue, we have to remove it!
	*/
	
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

	fp_Column* pCol = NULL;
	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		pLine->recalcHeight();	// line-height

		if (pLine->getColumn() != pCol)
		{
			pCol = pLine->getColumn();
			pCol->setNeedsLayoutUpdate();	// line-height, margin-top, margin-bottom
		}
 
		pLine = pLine->getNext();
	}
	
	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTBLOCK);
	}

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_insertStrux(const PX_ChangeRecord_Strux * pcrx,
												PL_StruxDocHandle sdh,
												PL_ListenerId lid,
												void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																		PL_ListenerId lid,
																		PL_StruxFmtHandle sfhNew))
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
					
	fl_SectionLayout* pSL = m_pSectionLayout;
	UT_ASSERT(pSL);
	fl_BlockLayout*	pNewBL = pSL->insertBlock(sdh, this, pcrx->getIndexAP());
	if (!pNewBL)
	{
		UT_DEBUGMSG(("no memory for BlockLayout"));
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

	fp_Run* pRun = m_pFirstRun;
	while (pRun)
	{			
		UT_uint32 iWhere = pRun->containsOffset(blockOffset);

		if (iWhere == FP_RUN_INSIDE)
		{
			if ((blockOffset > pRun->getBlockOffset()) || (blockOffset == 0))
			{
				// split here
				pRun->split(blockOffset, UT_TRUE);
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

	fp_Run* pFirstNewRun = NULL;
	if (pRun)
	{
		pFirstNewRun = pRun->getNext();

		// last line of old block is dirty
		pRun->getLine()->clearScreen();
		pRun->getLine()->layout();
		// we redraw the line below

		// break run sequence
		pRun->setNext(NULL);
		if (pFirstNewRun)
		{
			pFirstNewRun->setPrev(NULL);
		}
	}
	else if (blockOffset == 0)
	{
		// everything goes in new block
		pFirstNewRun = m_pFirstRun;
	}

	// explicitly truncate rest of this block's layout
	truncateLayout(pFirstNewRun);

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
		pRun->calcWidths(&pNewBL->m_gbCharWidths);
						
		pRun = pRun->getNext();
	}

	pNewBL->format();
	
	checkForWidowsAndOrphans();

	coalesceRuns();
	
	format();
	
	_destroySpellCheckLists();
	m_pLayout->queueBlockForSpell(this);

	m_pLayout->queueBlockForSpell(pNewBL);
	
	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
	}
	
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

void fl_BlockLayout::checkForWidowsAndOrphans(void)
{
start_over:
	UT_uint32 iCountLines = 0;
	UT_uint32 iCountColumns = 0;
	fp_Column* pLastColSeen = NULL;

	fp_Line* pLine = m_pFirstLine;
	while (pLine)
	{
		iCountLines++;

		if (pLine->getColumn() != pLastColSeen)
		{
			iCountColumns++;

			UT_ASSERT(!pLastColSeen || (pLastColSeen->getLastLine() == pLine->getPrev()));
			
			pLastColSeen = pLine->getColumn();
		}
		
		pLine = pLine->getNext();
	}

	if (iCountLines > 0)
	{
		if (
			(m_pFirstLine->getColumn()->getFirstLine() == m_pFirstLine)
			&& (m_pFirstLine->getColumn()->getPrev())
			&& (
				(iCountLines <= getOrphansProperty())
				|| (iCountLines >= (getOrphansProperty() + getWidowsProperty()))
				)
			)
		{
			/*
			  Our first line is at the top of a column.  We should check
			  to see if the first few lines can be moved backwards into the previous
			  column.
			*/

			UT_uint32 iCounter = 0;
			pLine = m_pFirstLine;
			UT_sint32 iHeightNeeded = 0;
			
			while (pLine && (iCounter < getOrphansProperty()))
			{
				iHeightNeeded += pLine->getHeight();
				pLine = pLine->getNext();
				iCounter++;
			}

			fp_Column* pSecondColumn = m_pFirstLine->getColumn();
			fp_Column* pPrevCol = pSecondColumn->getPrev();
			UT_sint32 iSpace = pPrevCol->getSpaceAtBottom();
			UT_sint32 iMargin = m_pFirstLine->getMarginBefore();

			if (iSpace >= (iHeightNeeded + iMargin))
			{
				fp_Line* pMoveLine = m_pFirstLine;
				UT_uint32 iNumLinesToPushBack = getOrphansProperty();
				if (iNumLinesToPushBack > iCountLines)
				{
					iNumLinesToPushBack = iCountLines;
				}
				
				for (UT_uint32 i=0; i<iNumLinesToPushBack; i++)
				{
					pPrevCol->moveLineFromNextColumn(pMoveLine);
					pMoveLine = pMoveLine->getNext();
				}

				pSecondColumn->updateLayout();
			}
		}
		
		// TODO should we put an else here?
		
		if (iCountColumns > 1)
		{
			pLastColSeen = NULL;
			pLine = m_pFirstLine;
			UT_uint32 iCountLinesInColumn = 0;
			while (pLine)
			{
				if (pLine->getColumn() != pLastColSeen)
				{
					if (pLastColSeen)
					{
						if (iCountLinesInColumn < getOrphansProperty())
						{
							fp_Line* pMoveLine = pLine->getPrev();
							for (UT_uint32 i=0; i<iCountLinesInColumn; i++)
							{
								UT_ASSERT(pMoveLine->getColumn() == pLastColSeen);
								UT_ASSERT(pLastColSeen->getLastLine() == pMoveLine);
								
								pLastColSeen->moveLineToNextColumn(pMoveLine);
								pMoveLine = pMoveLine->getPrev();
							}

							pLastColSeen->getNext()->updateLayout();

							goto start_over;
						}
					}

					pLastColSeen = pLine->getColumn();
					iCountLinesInColumn = 1;
				}
				else
				{
					iCountLinesInColumn++;
				}
		
				pLine = pLine->getNext();
			}

			if (iCountLinesInColumn < getWidowsProperty())
			{
				// widow problem
				
				fp_Column* pPrevCol = pLastColSeen->getPrev();
				UT_ASSERT(pPrevCol);
				
				pLine = pPrevCol->getLastLine();
				UT_ASSERT(pLine->getBlock() == this);
				
				UT_uint32 iCountLinesInPrevColumn = 0;
				while (pLine && (pLine->getColumn() == pPrevCol))
				{
					iCountLinesInPrevColumn++;
					pLine = pLine->getPrev();
				}

				UT_uint32 iNumLinesToBeMoved;
				
				if (
					(iCountLinesInPrevColumn > getOrphansProperty())
					&& ((iCountLinesInPrevColumn - getOrphansProperty() + iCountLinesInColumn) >= getWidowsProperty())
					)
				{
					/*
					  Move just enough
					*/
					iNumLinesToBeMoved = getWidowsProperty() - iCountLinesInColumn;
				}
				else
				{
					/*
					  There aren't enough lines in the prev column to
					  move just a few.  Move 'em all!
					*/

					iNumLinesToBeMoved = iCountLinesInPrevColumn;
				}

				fp_Line* pMoveLine = pPrevCol->getLastLine();
				UT_ASSERT(pMoveLine->getBlock() == this);
				for (UT_uint32 i=0; i<iNumLinesToBeMoved; i++)
				{
					UT_ASSERT(pMoveLine->getColumn() == pPrevCol);
								
					pPrevCol->moveLineToNextColumn(pMoveLine);
					pMoveLine = pMoveLine->getPrev();
				}

				pPrevCol->getNext()->updateLayout();
			}
		}
	}
}

UT_uint32 fl_BlockLayout::canSlurp(fp_Line* pLine) const
{
	UT_ASSERT(pLine);
	UT_ASSERT(pLine->getColumn());
	UT_ASSERT(pLine->getColumn()->getFirstLine() == pLine);
	
	UT_uint32 iCountLinesInBlock = 0;
	fp_Column* pCol = pLine->getColumn();
	fp_Line* pOrigLine = pLine;
	while (pLine && pLine->getColumn() == pCol)
	{
		iCountLinesInBlock++;
		pLine = pLine->getNext();
	}

	if ((pLine == NULL)
		&& (pOrigLine == m_pFirstLine)
		)
	{
		return iCountLinesInBlock;
	}
	
	if (iCountLinesInBlock > getWidowsProperty())
	{
		return (iCountLinesInBlock - getWidowsProperty());
	}
	else
	{
		return 0;
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
	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Image:\n"));
		PT_BlockOffset blockOffset = (pcro->getPosition() - getPosition());
		_doInsertImageRun(blockOffset, pcro);
		break;
	}
		
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:InsertObject:Field:\n"));
		PT_BlockOffset blockOffset = (pcro->getPosition() - getPosition());
		_doInsertFieldRun(blockOffset, pcro);
		break;
	}
	
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
	
	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition() + 1);
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
	}

	return UT_TRUE;
}

UT_Bool fl_BlockLayout::doclistener_deleteObject(const PX_ChangeRecord_Object * pcro)
{
	switch (pcro->getObjectType())
	{
	case PTO_Image:
	{
		UT_DEBUGMSG(("Edit:DeleteObject:Image:\n"));

		PT_BlockOffset blockOffset = (pcro->getPosition() - getPosition());
		_delete(blockOffset, 1);
		break;
	}
	
	case PTO_Field:
	{
		UT_DEBUGMSG(("Edit:DeleteObject:Field:\n"));

		PT_BlockOffset blockOffset = (pcro->getPosition() - getPosition());
		_delete(blockOffset, 1);
		break;
	}		

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
	
	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcro->getPosition());
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
	}

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
				pFieldRun->calcWidths(&m_gbCharWidths);

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
	format();

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_resetSelection();
		pView->_setPoint(pcroc->getPosition());
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTCHAR);
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
