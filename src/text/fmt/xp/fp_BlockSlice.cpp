 
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

#include "fp_BlockSlice.h"
#include "fl_BlockLayout.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "dg_DrawArgs.h"
#include "dg_Graphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_vector.h"

fp_BlockSlice::fp_BlockSlice(fl_BlockLayout *pBlock)
{
	m_pBlock = pBlock;
	m_pColumn = NULL;
	m_pColumnData = NULL;
	m_iHeight = 0;
	m_iTotalLineHeight = 0;
}

fp_LineInfo::fp_LineInfo(fp_Line* l, UT_sint32 _base_xoff, UT_sint32 _xoff, UT_sint32 _yoff)
{
	pLine = l;
	xoff = _xoff;
	yoff = _yoff;
	base_xoff = _base_xoff;
}

fp_BlockSlice::~fp_BlockSlice()
{
	UT_VECTOR_PURGEALL(fp_Sliver, m_vecSlivers);
	
	deleteLines();
}

UT_uint32 fp_BlockSlice::getHeight()
{
	return m_iHeight;
}

fp_Column* fp_BlockSlice::getColumn()
{
	return m_pColumn;
}

void fp_BlockSlice::setColumn(fp_Column* pCol, void* pData)
{
	UT_ASSERT(pCol);
	UT_ASSERT(!m_pColumn);

	UT_DEBUGMSG(("setColumn: fp_BlockSlice=0x%x,  column=0x%x\n", this, pCol));

	m_pColumn = pCol;
	m_pColumnData = pData;
}

fl_BlockLayout* fp_BlockSlice::getBlock()
{
	return m_pBlock;
}

UT_Bool fp_BlockSlice::isFirstSliceInBlock(void)
{
	if (m_pBlock->getFirstSlice() == this)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool fp_BlockSlice::isLastSliceInBlock(void)
{
	if (m_pBlock->getLastSlice() == this)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

fp_Sliver* fp_BlockSlice::addSliver(UT_uint32 iX, UT_uint32 iWidth, UT_uint32 iHeight)
{
	m_iHeight += iHeight;

	fp_Sliver* pSliver;

	if (m_vecSlivers.getItemCount() > 0)
	{
		pSliver = (fp_Sliver*) m_vecSlivers.getLastItem();

		if (
			(pSliver->iX == iX) 
			&& (pSliver->iWidth == iWidth)
			)
		{
			pSliver->iHeight += iHeight;
			return pSliver;
		}
	}

	// we must be in a non-rectangular column.
	pSliver = new fp_Sliver();
	pSliver->iX = iX;
	pSliver->iWidth = iWidth;
	pSliver->iHeight = iHeight;
	m_vecSlivers.addItem(pSliver);

	return pSliver;
}

int fp_BlockSlice::countSlivers()
{
	return m_vecSlivers.getItemCount();
}

fp_Sliver* fp_BlockSlice::getNthSliver(int n)
{
	return (fp_Sliver*) m_vecSlivers.getNthItem(n);
}

UT_uint32 fp_BlockSlice::countLines()
{
	return m_vecLineInfos.getItemCount();
}

fp_Line* fp_BlockSlice::getNthLine(UT_uint32 n)
{
	UT_ASSERT(n < m_vecLineInfos.getItemCount());

	fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(n);

	UT_ASSERT(pLI);
	UT_ASSERT(pLI->pLine);
	
	return pLI->pLine;
}

void fp_BlockSlice::deleteLines()
{
	int count=m_vecLineInfos.getItemCount();
	for (int i=0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);

		delete pLI->pLine;
		delete pLI;
	}

	m_vecLineInfos.clear();
	m_iTotalLineHeight = 0;
}

void fp_BlockSlice::removeLine(fp_Line* pLine, void* p)
{
	fp_LineInfo* pLI = (fp_LineInfo*) p;
	UT_ASSERT(pLI);
	UT_ASSERT(pLI->pLine == pLine);

	UT_sint32 ndx = m_vecLineInfos.findItem(pLI);
	UT_ASSERT(ndx >= 0);

	if (ndx > 0)
	{
		m_vecLineInfos.deleteNthItem(ndx);

		UT_ASSERT(m_iTotalLineHeight >= pLine->getHeight());
		m_iTotalLineHeight -= pLine->getHeight();

		delete pLI->pLine;
		delete pLI;
	}
}

int	fp_BlockSlice::addLine(fp_Line* pLine)
{
	fp_Sliver* pSliver = NULL;
	UT_sint32 iLineHeight = pLine->getHeight();
	UT_ASSERT(iLineHeight > 0);

	if ((m_iTotalLineHeight + iLineHeight) > m_iHeight)
	{
		UT_uint32 iX;
		UT_uint32 iWidth;
		UT_uint32 iHeight;

		int result = m_pColumn->requestSliver(this, iLineHeight, &iX, &iWidth, &iHeight);
		UT_ASSERT(result);	// this request should have already been made and approved

		pSliver = addSliver(iX, iWidth, iHeight);
		m_pColumn->reportSliceHeightChanged(this, getHeight());
	}
	else
	{
		// we already have enough sliver space for this.
		int count = countSlivers();
		int iRunningHeight = 0;
		for (int i=0; i<count; i++)
		{
			fp_Sliver* pS = getNthSliver(i);
			if ((iRunningHeight + pS->iHeight) >= (m_iTotalLineHeight + iLineHeight))
			{
				// this is the sliver we need
				pSliver = pS;
				break;
			}
			iRunningHeight += pS->iHeight;
		}
	}

	UT_ASSERT(pSliver);
	UT_ASSERT((m_iTotalLineHeight + pLine->getHeight()) <= m_iHeight);

	fp_LineInfo*	pLI = new fp_LineInfo(pLine, pSliver->iX, pSliver->iX, m_iTotalLineHeight);
	pLine->setBlockSlice(this, pLI);
	m_vecLineInfos.addItem(pLI);
	m_iTotalLineHeight += pLine->getHeight();

	return 0;
}

void fp_BlockSlice::verifyColumnFit()
{
	int iCountSlivers = countSlivers();
	UT_uint32 iY = 0;
	int i;

	for (i=0; i<iCountSlivers; i++)
	{
		fp_Sliver* pSliver = getNthSliver(i);
		if (!m_pColumn->verifySliverFit(this, pSliver, iY))
		{
			// if the column says a sliver doesn't fit, then we just kill that one
			pSliver->iHeight = 0;
		}
		iY += pSliver->iHeight;
	}

	for (i=iCountSlivers-1; i>=0; i--)
	{
		fp_Sliver* pSliver = getNthSliver(i);
		if (pSliver->iHeight == 0)
		{
			m_vecSlivers.deleteNthItem(i);
			delete pSliver;
		}
	}

	m_iHeight = 0;
	iCountSlivers = countSlivers();
	for (i=0; i<iCountSlivers; i++)
	{
		fp_Sliver* pSliver = getNthSliver(i);
		m_iHeight += pSliver->iHeight;
	}

	UT_ASSERT(m_iTotalLineHeight <= m_iHeight);
}

void fp_BlockSlice::returnExtraSpace()
{
	int count = m_vecLineInfos.getItemCount();

	UT_uint32 iMaxY;
	if (count > 0)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(count-1);
		UT_ASSERT(pLI);
		iMaxY = pLI->yoff + pLI->pLine->getHeight();
	}
	else
	{
		iMaxY = 0;
	}

	if (iMaxY < m_iHeight)
	{
		int iCountSlivers = countSlivers();
		UT_uint32 iY = 0;
		UT_Bool bKilling = UT_FALSE;
		int i;

		for (i=0; i<iCountSlivers; i++)
		{
			fp_Sliver* pSliver = getNthSliver(i);
			if (bKilling)
			{
				pSliver->iHeight = 0;
			}
			else if ((iY + pSliver->iHeight) > iMaxY)
			{
				pSliver->iHeight = iMaxY - iY;
				bKilling = UT_TRUE;
				iY += pSliver->iHeight;
			}
		}

		for (i=iCountSlivers-1; i>=0; i--)
		{
			fp_Sliver* pSliver = getNthSliver(i);
			if (pSliver->iHeight == 0)
			{
				m_vecSlivers.deleteNthItem(i);
				delete pSliver;
			}
		}

		m_iHeight = iMaxY;
		UT_ASSERT(m_iTotalLineHeight <= getHeight());

		m_pColumn->reportSliceHeightChanged(this, getHeight());
	}
}

UT_uint32 fp_BlockSlice::requestLineSpace(UT_uint32 iHeightNeeded)
{
	UT_uint32 iX;
	UT_uint32 iWidth;
	UT_uint32 iHeight;

	if ((m_iTotalLineHeight + iHeightNeeded) <= m_iHeight)
	{
		// we already have enough sliver space for this.
		int count = countSlivers();
		int iRunningHeight = 0;
		for (int i=0; i<count; i++)
		{
			fp_Sliver* pSliver = getNthSliver(i);
			if ((iRunningHeight + pSliver->iHeight) >= (m_iTotalLineHeight + iHeightNeeded))
			{
				// this is the sliver we need
				return pSliver->iWidth;
			}
			iRunningHeight += pSliver->iHeight;
		}
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	if (m_pColumn->requestSliver(this, iHeightNeeded, &iX, &iWidth, &iHeight))
	{
		return iWidth;
	}
	else
	{
		return 0;
	}
}

void fp_BlockSlice::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bEOL)
{
	int count = m_vecLineInfos.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);

		// when hit testing lines within a BlockSlice, we ignore the X coord.
//		if (((x - pLI->xoff) >= 0) && ((x - pLI->xoff) < (pLI->pLine->getWidth())))
		{
			if (((y - pLI->yoff) >= 0) && ((y - pLI->yoff) < (UT_sint32)(pLI->pLine->getHeight())))
			{
				pLI->pLine->mapXYToPosition(x - pLI->xoff, y - pLI->yoff, pos, bEOL);
				return;
			}
		}

		// TODO it might be better to move these special cases outside the loop
		if ((i == 0) && (y < pLI->yoff))
		{
			pLI->pLine->mapXYToPosition(x - pLI->xoff, y - pLI->yoff, pos, bEOL);
			return;
		}
		
		if ((i == (count-1)) && (y >= (pLI->yoff + (UT_sint32)pLI->pLine->getHeight())))
		{
			pLI->pLine->mapXYToPosition(x - pLI->xoff, y - pLI->yoff, pos, bEOL);
			return;
		}
	}

	// TODO pick the closest line
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_BlockSlice::getOffsets(fp_Line* pLine, void* p, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pColumn->getOffsets(this, m_pColumnData, my_xoff, my_yoff);
	
	fp_LineInfo* pLI = (fp_LineInfo*) p;
	UT_ASSERT(pLI->pLine == pLine);

	xoff = my_xoff + pLI->xoff;
	yoff = my_yoff + pLI->yoff;
}

void fp_BlockSlice::getScreenOffsets(fp_Line* pLine, void* p,
									 UT_sint32& xoff, UT_sint32& yoff,
									 UT_sint32& width, UT_sint32& height)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pColumn->getScreenOffsets(this, m_pColumnData, my_xoff, my_yoff,
								width, height);
	
	fp_LineInfo* pLI = (fp_LineInfo*) p;
	UT_ASSERT(pLI->pLine == pLine);

	xoff = my_xoff + pLI->xoff;
	yoff = my_yoff + pLI->yoff;
}

void fp_BlockSlice::clearScreen(DG_Graphics* pG)
{
	UT_ASSERT(pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;
	UT_sint32 my_height, my_width;
	
	m_pColumn->getScreenOffsets(this, m_pColumnData, my_xoff, my_yoff,
								my_width, my_height);

	int count = m_vecLineInfos.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);

		pLI->pLine->clearScreen();
	}
}

void fp_BlockSlice::draw(DG_Graphics* pG)
{
	UT_ASSERT(pG->queryProperties(DG_Graphics::DGP_SCREEN));
	
	// draw each line in the slice.

	int count = m_vecLineInfos.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);

		pLI->pLine->draw(pG);
	}
}

void fp_BlockSlice::draw(dg_DrawArgs* pDA)
{
	// draw each line in the slice.

	int count = m_vecLineInfos.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);

		dg_DrawArgs da = *pDA;
		da.xoff += pLI->xoff;
		da.yoff += pLI->yoff;
		pLI->pLine->draw(&da);
	}
}

void fp_BlockSlice::remove()
{
	m_pColumn->removeBlockSlice(this);
}

void fp_BlockSlice::align()
{
	int count = m_vecLineInfos.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);
		alignOneLine(pLI);
	}
}

void fp_BlockSlice::alignOneLine(fp_LineInfo* pLI)
{
	UT_sint32 iExtraWidth = pLI->pLine->getMaxWidth() - pLI->pLine->getWidth();
	UT_uint32 iAlignCmd = m_pBlock->getAlignment();

//	if (iExtraWidth > 0)
	{
		/*
		  the line is not as wide as the space allocated for it.  check to see if
		  we need to justify it.
		*/
		switch (iAlignCmd)
		{
		case FL_ALIGN_BLOCK_LEFT:
			pLI->xoff = pLI->base_xoff;
			break;
		case FL_ALIGN_BLOCK_RIGHT:
			pLI->xoff = pLI->base_xoff + iExtraWidth;
			break;
		case FL_ALIGN_BLOCK_CENTER:
			pLI->xoff = pLI->base_xoff + (iExtraWidth / 2);
			break;
		case FL_ALIGN_BLOCK_JUSTIFY:
			pLI->xoff = pLI->base_xoff;
			// pLine->expandWidthTo(pSliver->iWidth);
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
	}
}

void fp_BlockSlice::alignOneLine(fp_Line* pLine, void* p)
{
	fp_LineInfo* pLI = (fp_LineInfo*) p;
	UT_ASSERT(pLI->pLine == pLine);
	
	alignOneLine(pLI);
}

void fp_BlockSlice::dump()
{
	int count = m_vecLineInfos.getItemCount();
	for (int i = 0; i<count; i++)
	{
		fp_LineInfo* pLI = (fp_LineInfo*) m_vecLineInfos.getNthItem(i);

		UT_DEBUGMSG(("fp_BlockSlice::dump(0x%x) - fp_Line 0x%x\n", pLI->pLine));
	}
}
