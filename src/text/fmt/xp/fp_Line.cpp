 
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


#include "fp_Line.h"
#include "fp_BlockSlice.h"
#include "fp_Run.h"
#include "dg_DrawArgs.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

FP_Line::FP_Line(UT_sint32 maxWidth) 
{
	m_iMaxWidth = maxWidth;
	m_iWidth = 0;
	m_iHeight = 0;
	m_pBlockSlice = NULL;
	m_pBlockSliceData = NULL;
	m_bDirty = UT_FALSE;
}

fp_RunInfo::fp_RunInfo(FP_Run* p)
{
	pRun = p;
	xoff = 0;
	yoff = 0;
}

FP_Line::~FP_Line()
{
	UT_VECTOR_PURGEALL(fp_RunInfo, m_vecRunInfos);
}

void FP_Line::setBlockSlice(FP_BlockSlice* pBlockSlice, void* p)
{
	m_pBlockSlice = pBlockSlice;
	m_pBlockSliceData = p;
}

FP_BlockSlice* FP_Line::getBlockSlice() const
{
	return m_pBlockSlice;
}

UT_uint32 FP_Line::getHeight() const
{
	return m_iHeight;
}

UT_uint32 FP_Line::getWidth() const
{
	return m_iWidth;
}

UT_uint32 FP_Line::getMaxWidth() const
{
	return m_iMaxWidth;
}

void FP_Line::setNext(FP_Line* p)
{
	m_pNext = p;
}

void FP_Line::setPrev(FP_Line* p)
{
	m_pPrev = p;
}

FP_Line* FP_Line::getNext() const
{
	return m_pNext;
}

FP_Line* FP_Line::getPrev() const
{
	return m_pPrev;
}

int FP_Line::countRuns() const
{
	return m_vecRunInfos.getItemCount();
}

FP_Run* FP_Line::getFirstRun() const
{
	fp_RunInfo* psi = (fp_RunInfo*) m_vecRunInfos.getFirstItem();

	return psi->pRun;
}

FP_Run* FP_Line::getLastRun() const
{
	fp_RunInfo* psi = (fp_RunInfo*) m_vecRunInfos.getLastItem();

	return psi->pRun;
}

UT_Bool FP_Line::removeRun(FP_Run* pRun)
{
	int numRuns = m_vecRunInfos.getItemCount();
	UT_Bool bAdjust = UT_FALSE;
	UT_sint32 iAdjust = 0;
	UT_Bool bResult = UT_FALSE;
	int i;

	for (i = 0; i < numRuns; i++)
	{
		fp_RunInfo* psi = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		if (bAdjust)
		{
			psi->xoff -= iAdjust;
		}
		else if (psi->pRun == pRun)
		{
			iAdjust = pRun->getWidth();
			bAdjust = UT_TRUE;
		}
	}

	for (i = 0; i < numRuns; i++)
	{
		fp_RunInfo* psi = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		if (psi->pRun == pRun)
		{
			m_vecRunInfos.deleteNthItem(i);
			m_iWidth -= iAdjust;

			pRun->setLine(NULL, NULL);

			return UT_TRUE;
		}
	}

	return UT_FALSE;
}

void FP_Line::insertRun(FP_Run* pRun, UT_Bool bClear, UT_Bool bNewData)
{
	fp_RunInfo* psi = (fp_RunInfo*) pRun->getLineData();
	
	if (bNewData == UT_TRUE)
	{
		psi = new fp_RunInfo(pRun);
	}

	pRun->setLine(this, psi);

	m_vecRunInfos.insertItemAt(psi, 0);

	UT_sint32 count = m_vecRunInfos.getItemCount();
	UT_sint32 width = pRun->getWidth();

	m_iWidth += width;
	
	for (UT_sint32 i = 0; i < count; i++)
	{
		fp_RunInfo *p = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		if (bClear == UT_TRUE)
			p->pRun->clearScreen();
		
		if (i == 0)
			p->xoff = 0;
		else
			p->xoff += width;
	}
	
	_recalcHeight();
}

void FP_Line::addRun(FP_Run* pRun)
{
	fp_RunInfo* psi;
	
	psi = new fp_RunInfo(pRun);
	pRun->setLine(this, psi);
	psi->xoff = m_iWidth;

	m_vecRunInfos.addItem(psi);
	
	m_iWidth += pRun->getWidth();

	_recalcHeight();
}

void FP_Line::splitRunInLine(FP_Run* pRun1, FP_Run* pRun2)
{
	// insert run2 after run1 in the current line.
	
	fp_RunInfo* psi;
	
	psi = new fp_RunInfo(pRun2);
	pRun2->setLine(this, psi);
	psi->xoff = m_iWidth;

	UT_sint32 count = m_vecRunInfos.getItemCount();
	UT_sint32 k;
	for (k=0; k<count; k++)
	{
		fp_RunInfo * p = (fp_RunInfo *)m_vecRunInfos.getNthItem(k);
		UT_ASSERT(p);
		if (p->pRun == pRun1)
		{
			m_vecRunInfos.insertItemAt(psi,k+1);
			return;
		}
	}

	// we don't update m_iWidth or recalseHeight() since we
	// assume the space in run2 came from run1.
}

UT_uint32 FP_Line::getNumChars() const
{
	UT_uint32 iCountChars = 0;
	
	int countRuns = m_vecRunInfos.getItemCount();
	for (int i=0; i<countRuns; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		iCountChars += pSI->pRun->getLength();
	}

	return iCountChars;
}

void FP_Line::runSizeChanged(void *p, UT_sint32 oldWidth, UT_sint32 newWidth)
{
	fp_RunInfo* psi = (fp_RunInfo*) p;

	//UT_ASSERT(newWidth);
	
	// we've changed ...
	m_bDirty = UT_TRUE;

	UT_sint32 dx = newWidth - oldWidth;

	UT_sint32 count = m_vecRunInfos.getItemCount();

	UT_Bool bIncr = UT_FALSE;

	// search thru the list of runs.  when we find the current run,
	// we need to increment all the runs that follow us
	for (UT_sint32 i = 0; i < count; i++)
	{
		fp_RunInfo* pInfo = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		/*
		  The following line is suspect.  We're getting things
		  erased that aren't supposed to be.  Is the Run struct
		  actually in sync with its notion of its screen position?
		  If not, we'll be erasing random stuff here.
		*/
		pInfo->pRun->clearScreen();
		
		if (psi == pInfo)
		{
			bIncr = UT_TRUE;
			continue;
		}

		if (bIncr)
			pInfo->xoff += dx;
		
	}
	
	m_iWidth += dx;

	m_pBlockSlice->alignOneLine(this, m_pBlockSliceData);
}

void FP_Line::mapXYToBufferPosition(UT_sint32 x, UT_sint32 y, UT_uint32& pos, UT_Bool& bRight)
{
	if (x < 0)
	{
		x = 0;
	}
	else if (x >= (UT_sint32)m_iWidth)
	{
		x = m_iWidth - 1;
	}

	// check all of the runs.
	int count = m_vecRunInfos.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		UT_sint32 y2 = y - pSI->yoff - m_iAscent + pSI->pRun->getAscent();
		if (((x - pSI->xoff) >= 0) && ((x - pSI->xoff) < (pSI->pRun->getWidth())))
		{
			// when hit testing runs within a line, we ignore the Y coord
//			if (((y2) >= 0) && ((y2) < (pSI->pRun->getHeight())))
			{
				pSI->pRun->mapXYToBufferPosition(x - pSI->xoff, y2, pos, bRight);
				return;
			}
		}
	}

	// TODO pick the closest run
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void FP_Line::getOffsets(FP_Run* pRun, void* p, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pBlockSlice->getOffsets(this, m_pBlockSliceData, my_xoff, my_yoff);
	
	fp_RunInfo* pRI = (fp_RunInfo*) p;
	UT_ASSERT(pRI->pRun == pRun);

	xoff = my_xoff + pRI->xoff;
	yoff = my_yoff + pRI->yoff + m_iAscent - pRun->getAscent();
}

void FP_Line::getScreenOffsets(FP_Run* pRun, void* p, UT_sint32& xoff,
							   UT_sint32& yoff, UT_sint32& width,
							   UT_sint32& height)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pBlockSlice->getScreenOffsets(this, m_pBlockSliceData, my_xoff,
									my_yoff, width, height);
	
	fp_RunInfo* pRI = (fp_RunInfo*) p;
	UT_ASSERT(pRI->pRun == pRun);

	xoff = my_xoff + pRI->xoff;
	yoff = my_yoff + pRI->yoff + m_iAscent - pRun->getAscent();
}

#if UNUSED
void FP_Line::getAbsoluteCoords(UT_sint32& x, UT_sint32& y)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pBlockSlice->getOffsets(this, m_pBlockSliceData, my_xoff, my_yoff);

	x = my_xoff;
	y = my_yoff + m_iAscent;
}
#endif

void FP_Line::_recalcHeight()
{
	UT_sint32 count = m_vecRunInfos.getItemCount();
	UT_sint32 i;

	UT_uint32 iMaxAscent = 0;
	UT_uint32 iMaxDescent = 0;

	for (i=0; i<count; i++)
	{
		UT_uint32 iAscent;

		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		iAscent = pSI->pRun->getAscent();
		iMaxAscent = UT_MAX(iMaxAscent, iAscent);
	}

	for (i=0; i<count; i++)
	{
		UT_uint32 iDescent;

		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		iDescent = pSI->pRun->getDescent();
		iMaxDescent = UT_MAX(iMaxDescent, iDescent);
	}

	m_iAscent = iMaxAscent;
	m_iHeight = iMaxAscent + iMaxDescent;
}

void FP_Line::expandWidthTo(UT_uint32 iNewWidth)
{
	UT_uint32 iPrevWidth = m_iWidth;
	UT_ASSERT(iNewWidth > iPrevWidth);

	UT_uint32 iMoreWidth = iNewWidth - iPrevWidth;

	int count = m_vecRunInfos.getItemCount();
	UT_sint32 i;
	
	for (i=0; i<count; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		UT_uint32 iCurSpanWidth = pSI->pRun->getWidth();
		UT_uint32 iNewSpanWidth = 
			iCurSpanWidth 
			+ ((UT_uint32) ((iCurSpanWidth / ((double) iPrevWidth)) * iMoreWidth));
		pSI->pRun->expandWidthTo(iNewSpanWidth);
	}

	m_iWidth = 0;
	for (i=0; i<count; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);
		pSI->xoff = m_iWidth;

		m_iWidth += pSI->pRun->getWidth();
	}
}

void FP_Line::shrink(UT_sint32 width)
{
	m_iWidth -= width;
}

void FP_Line::clearScreen()
{
	int count = m_vecRunInfos.getItemCount();

	for (int i=0; i < count; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		pSI->pRun->clearScreen();
	}
}

void FP_Line::draw(DG_Graphics* pG)
{
	UT_ASSERT(m_iWidth <= m_iMaxWidth);
	
	UT_sint32 my_xoff = 0, my_yoff = 0;
	UT_sint32 width, height;
	
	m_pBlockSlice->getScreenOffsets(this, m_pBlockSliceData, my_xoff,
									my_yoff, width, height);

	dg_DrawArgs da;
	da.yoff = my_yoff + m_iAscent;
	da.xoff = my_xoff;
	da.x = 0;
	da.y = 0;
	da.pG = pG;
	da.width = m_iWidth;
	da.height = m_iHeight;
	
	int count = m_vecRunInfos.getItemCount();

	UT_sint32 yOff = my_yoff;
	
	my_yoff += m_iAscent;

	for (int i=0; i < count; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		da.xoff += pSI->xoff;
		da.yoff += pSI->yoff;
		pSI->pRun->draw(&da);
		da.xoff -= pSI->xoff;
		da.yoff -= pSI->yoff;
	}
}

void FP_Line::draw(dg_DrawArgs* pDA)
{
	const UT_sint32 height = pDA->height;
	const UT_sint32 y = pDA->y;
	
	int count = m_vecRunInfos.getItemCount();

	UT_sint32 yOff = pDA->yoff;
	
	pDA->yoff += m_iAscent;

	for (int i=0; i<count; i++)
	{
		fp_RunInfo* pSI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);
		UT_sint32 rHeight = pSI->pRun->getHeight();

		// TODO adjust this clipping to draw lines which are partially visible
		// TODO - X clipping - for now, just clip against y
		if (((((UT_sint32)pSI->yoff + yOff) >= y) &&
		     (((UT_sint32)pSI->yoff + yOff) <= (y + height))) ||
		    ((((UT_sint32)pSI->yoff + yOff) <= y) &&
		     (((UT_sint32)pSI->yoff + yOff + rHeight) > y)))
		{
			dg_DrawArgs da = *pDA;
			da.xoff += pSI->xoff;
			da.yoff += pSI->yoff;
		    pSI->pRun->draw(&da);
		}
	}
}

void FP_Line::align()
{
	UT_ASSERT(m_pBlockSlice);
	
	m_pBlockSlice->alignOneLine(this, m_pBlockSliceData);
}

void FP_Line::dumpRunInfo(const FP_Run* pRun, void *p)
{
	fp_RunInfo* pRI = (fp_RunInfo*) p;
	UT_ASSERT(pRI->pRun == pRun);

	UT_DEBUGMSG(("Run: 0x%08lx xoff=%d\n", pRun, pRI->xoff));
}
