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



#include "fp_Line.h"
#include "fp_BlockSlice.h"
#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "gr_DrawArgs.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

fp_Line::fp_Line(UT_sint32 maxWidth) 
{
	m_iMaxWidth = maxWidth;
	m_iWidth = 0;
	m_iHeight = 0;
	m_pBlockSlice = NULL;
	m_pBlockSliceData = NULL;
	m_bDirty = UT_FALSE;
}

fp_RunInfo::fp_RunInfo(fp_Run* p)
{
	pRun = p;
	xoff = 0;
	yoff = 0;
}

fp_Line::~fp_Line()
{
	UT_VECTOR_PURGEALL(fp_RunInfo, m_vecRunInfos);
}

UT_Bool fp_Line::isEmpty(void) const
{
	return ((m_vecRunInfos.getItemCount()) == 0);
}

void fp_Line::setBlockSlice(fp_BlockSlice* pBlockSlice, void* p)
{
	m_pBlockSlice = pBlockSlice;
	m_pBlockSliceData = p;
}

fp_BlockSlice* fp_Line::getBlockSlice() const
{
	return m_pBlockSlice;
}

UT_uint32 fp_Line::getHeight() const
{
	return m_iHeight;
}

UT_uint32 fp_Line::getWidth() const
{
	return m_iWidth;
}

UT_uint32 fp_Line::getMaxWidth() const
{
	return m_iMaxWidth;
}

void fp_Line::setNext(fp_Line* p)
{
	m_pNext = p;
}

void fp_Line::setPrev(fp_Line* p)
{
	m_pPrev = p;
}

fp_Line* fp_Line::getNext() const
{
	return m_pNext;
}

fp_Line* fp_Line::getPrev() const
{
	return m_pPrev;
}

int fp_Line::countRuns() const
{
	return m_vecRunInfos.getItemCount();
}

fp_Run* fp_Line::getFirstRun() const
{
	fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getFirstItem();

	return pRI->pRun;
}

fp_Run* fp_Line::getLastRun() const
{
	fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getLastItem();

	return pRI->pRun;
}

UT_Bool fp_Line::removeRun(fp_Run* pRun)
{
	int numRuns = m_vecRunInfos.getItemCount();
	UT_Bool bAdjust = UT_FALSE;
	UT_sint32 iAdjust = 0;
	int i;

	for (i = 0; i < numRuns; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		if (bAdjust)
		{
			pRI->xoff -= iAdjust;
		}
		else if (pRI->pRun == pRun)
		{
			iAdjust = pRun->getWidth();
			bAdjust = UT_TRUE;
		}
	}

	for (i = 0; i < numRuns; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		if (pRI->pRun == pRun)
		{
			m_vecRunInfos.deleteNthItem(i);
			delete pRI;
			m_iWidth -= iAdjust;

			pRun->setLine(NULL, NULL);

			return UT_TRUE;
		}
	}

	return UT_FALSE;
}

void fp_Line::insertRun(fp_Run* pRun, UT_Bool bClear, UT_Bool bNewData)
{
	fp_RunInfo* pRI = (fp_RunInfo*) pRun->getLineData();
	
	if (bNewData == UT_TRUE)
	{
		pRI = new fp_RunInfo(pRun);
	}

	pRun->setLine(this, pRI);

	m_vecRunInfos.insertItemAt(pRI, 0);

	UT_sint32 count = m_vecRunInfos.getItemCount();
	UT_sint32 width = pRun->getWidth();

	m_iWidth += width;
	
	for (UT_sint32 i = 0; i < count; i++)
	{
		fp_RunInfo *p = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		if (bClear == UT_TRUE)
		{
			p->pRun->clearScreen();
		}
		
		if (i == 0)
		{
			p->xoff = 0;
		}
		else
		{
			p->xoff += width;
		}
	}
	
	_recalcHeight();
}

void fp_Line::addRun(fp_Run* pRun)
{
	fp_RunInfo* pRI;
	
	pRI = new fp_RunInfo(pRun);
	pRun->setLine(this, pRI);
	pRI->xoff = m_iWidth;

	m_vecRunInfos.addItem(pRI);
	
	m_iWidth += pRun->getWidth();

	_recalcHeight();
}

void fp_Line::splitRunInLine(fp_Run* pRun1, fp_Run* pRun2)
{
	// insert run2 after run1 in the current line.
	
	fp_RunInfo* pRunInfo1 = (fp_RunInfo*) pRun1->getLineData();
	fp_RunInfo* pRI;
	
	pRI = new fp_RunInfo(pRun2);
	pRun2->setLine(this, pRI);
	pRI->xoff = pRunInfo1->xoff + pRun1->getWidth();

	UT_sint32 count = m_vecRunInfos.getItemCount();
	UT_sint32 k;
	for (k=0; k<count; k++)
	{
		fp_RunInfo * p = (fp_RunInfo *)m_vecRunInfos.getNthItem(k);
		UT_ASSERT(p);
		if (p->pRun == pRun1)
		{
			m_vecRunInfos.insertItemAt(pRI,k+1);
			return;
		}
	}

	// we don't update m_iWidth or recalcHeight() since we
	// assume the space in run2 came from run1.
}

UT_uint32 fp_Line::getNumChars() const
{
	UT_uint32 iCountChars = 0;
	
	int iCountRuns = m_vecRunInfos.getItemCount();
	for (int i=0; i<iCountRuns; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		iCountChars += pRI->pRun->getLength();
	}

	return iCountChars;
}

void fp_Line::runSizeChanged(void *p, UT_sint32 oldWidth, UT_sint32 newWidth)
{
	fp_RunInfo* pRI = (fp_RunInfo*) p;

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

		pInfo->pRun->clearScreen();
		
		if (pRI == pInfo)
		{
			bIncr = UT_TRUE;
			continue;
		}

		if (bIncr)
		{
			pInfo->xoff += dx;
		}
	}
	
	m_iWidth += dx;

	/*
	  TODO we call _recalcHeight() here because when a run changes
	  width it might be changing height too.  Unfortunately, just
	  doing this is not sufficient.  We need a way to let the BlockSlice
	  know that our line height changed, since that will require other
	  adjustments in the BlockSlice.
	*/

	_recalcHeight();

	m_pBlockSlice->alignOneLine(this, m_pBlockSliceData);
}

void fp_Line::remove()
{
	if (m_pNext)
	{
		m_pNext->setPrev(m_pPrev);
	}

	if (m_pPrev)
	{
		m_pPrev->setNext(m_pNext);
	}

	m_pBlockSlice->removeLine(this, m_pBlockSliceData);
}

void fp_Line::mapXYToPosition(UT_sint32 x, UT_sint32 y, PT_DocPosition& pos, UT_Bool& bBOL, UT_Bool& bEOL)
{
	bBOL = UT_FALSE;

	if (x < 0)
	{
		x = 0;
		bBOL = UT_TRUE;
	}
	else if (x >= (UT_sint32)m_iWidth)
	{
		x = ((m_iWidth > 0) ? m_iWidth - 1 : 0);
	}

	// check all of the runs.
	int count = m_vecRunInfos.getItemCount();

	for (int i=0; i<count; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		UT_sint32 y2 = y - pRI->yoff - m_iAscent + pRI->pRun->getAscent();
		if ((x >= (UT_sint32)pRI->xoff) && (x < (UT_sint32)(pRI->xoff + pRI->pRun->getWidth())))
		{
			// when hit testing runs within a line, we ignore the Y coord
//			if (((y2) >= 0) && ((y2) < (pRI->pRun->getHeight())))
			{
				pRI->pRun->mapXYToPosition(x - pRI->xoff, y2, pos, bBOL, bEOL);

				return;
			}
		}
		else if (((x - pRI->xoff) == 0) && (pRI->pRun->getWidth() == 0))
		{
			// zero-length run
			{
				// this only happens in an empty line, right?
				UT_ASSERT(m_iWidth==0);
				UT_ASSERT(i==0);
				UT_ASSERT(count==1);

				pRI->pRun->mapXYToPosition(x - pRI->xoff, y2, pos, bBOL, bEOL);

				return;
			}
		}
	}

	// TODO pick the closest run
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void fp_Line::getOffsets(fp_Run* pRun, void* p, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pBlockSlice->getOffsets(this, m_pBlockSliceData, my_xoff, my_yoff);
	
	fp_RunInfo* pRI = (fp_RunInfo*) p;
	UT_ASSERT(pRI->pRun == pRun);

	xoff = my_xoff + pRI->xoff;
	yoff = my_yoff + pRI->yoff + m_iAscent - pRun->getAscent();
}

void fp_Line::getScreenOffsets(fp_Run* pRun, void* p, UT_sint32& xoff,
							   UT_sint32& yoff, UT_sint32& width,
							   UT_sint32& height, UT_Bool bLineHeight)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pBlockSlice->getScreenOffsets(this, m_pBlockSliceData, my_xoff,
									my_yoff, width, height);
	
	fp_RunInfo* pRI = (fp_RunInfo*) p;
	UT_ASSERT(pRI->pRun == pRun);

	xoff = my_xoff + pRI->xoff;

	if (bLineHeight)
		yoff = my_yoff;
	else
		yoff = my_yoff + pRI->yoff + m_iAscent - pRun->getAscent();

	width = m_iWidth;
	height = m_iHeight;
}

#if UNUSED
void fp_Line::getAbsoluteCoords(UT_sint32& x, UT_sint32& y)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pBlockSlice->getOffsets(this, m_pBlockSliceData, my_xoff, my_yoff);

	x = my_xoff;
	y = my_yoff + m_iAscent;
}
#endif

void fp_Line::_recalcHeight()
{
	UT_sint32 count = m_vecRunInfos.getItemCount();
	UT_sint32 i;

	UT_uint32 iMaxAscent = 0;
	UT_uint32 iMaxDescent = 0;

	for (i=0; i<count; i++)
	{
		UT_uint32 iAscent;

		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		iAscent = pRI->pRun->getAscent();
		iMaxAscent = UT_MAX(iMaxAscent, iAscent);
	}

	for (i=0; i<count; i++)
	{
		UT_uint32 iDescent;

		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		iDescent = pRI->pRun->getDescent();
		iMaxDescent = UT_MAX(iMaxDescent, iDescent);
	}

	m_iAscent = iMaxAscent;
	m_iHeight = iMaxAscent + iMaxDescent;
}

UT_uint32 fp_Line::getAscent(void) const
{
	return m_iAscent;
}

void fp_Line::expandWidthTo(UT_uint32 iNewWidth)
{
	UT_uint32 iPrevWidth = m_iWidth;
	UT_ASSERT(iNewWidth > iPrevWidth);

	UT_uint32 iMoreWidth = iNewWidth - iPrevWidth;

	int count = m_vecRunInfos.getItemCount();
	UT_sint32 i;
	
	for (i=0; i<count; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		UT_uint32 iCurSpanWidth = pRI->pRun->getWidth();
		UT_uint32 iNewSpanWidth = 
			iCurSpanWidth 
			+ ((UT_uint32) ((iCurSpanWidth / ((double) iPrevWidth)) * iMoreWidth));
		pRI->pRun->expandWidthTo(iNewSpanWidth);
	}

	m_iWidth = 0;
	for (i=0; i<count; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);
		pRI->xoff = m_iWidth;

		m_iWidth += pRI->pRun->getWidth();
	}
}

void fp_Line::shrink(UT_sint32 width)
{
	m_iWidth -= width;
}

void fp_Line::clearScreen()
{
	int count = m_vecRunInfos.getItemCount();

	for (int i=0; i < count; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		pRI->pRun->clearScreen();
	}
}

void fp_Line::draw(DG_Graphics* pG)
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

	// TODO: The following line means that no selection will be drawn.  Is this right?
	da.iSelPos1 = da.iSelPos2 = 0;
	
	int count = m_vecRunInfos.getItemCount();

	my_yoff += m_iAscent;

	for (int i=0; i < count; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);

		da.xoff += pRI->xoff;
		da.yoff += pRI->yoff;
		pRI->pRun->draw(&da);
		da.xoff -= pRI->xoff;
		da.yoff -= pRI->yoff;
	}
}

void fp_Line::draw(dg_DrawArgs* pDA)
{
	const UT_sint32 height = pDA->height;
	const UT_sint32 y = pDA->y;
	
	int count = m_vecRunInfos.getItemCount();

	UT_sint32 yOff = pDA->yoff;
	
	pDA->yoff += m_iAscent;

	for (int i=0; i<count; i++)
	{
		fp_RunInfo* pRI = (fp_RunInfo*) m_vecRunInfos.getNthItem(i);
		UT_sint32 rHeight = pRI->pRun->getHeight();

		// TODO adjust this clipping to draw lines which are partially visible
		// TODO - X clipping - for now, just clip against y
		if (((((UT_sint32)pRI->yoff + yOff) >= y) &&
		     (((UT_sint32)pRI->yoff + yOff) <= (y + height))) ||
		    ((((UT_sint32)pRI->yoff + yOff) <= y) &&
		     (((UT_sint32)pRI->yoff + yOff + rHeight) > y)))
		{
			dg_DrawArgs da = *pDA;
			da.xoff += pRI->xoff;
			da.yoff += pRI->yoff;
		    pRI->pRun->draw(&da);
		}
	}
}

void fp_Line::align()
{
	UT_ASSERT(m_pBlockSlice);
	
	m_pBlockSlice->alignOneLine(this, m_pBlockSliceData);
}

void fp_Line::dumpRunInfo(const fp_Run* pRun, void *p)
{
	fp_RunInfo* pRI = (fp_RunInfo*) p;
	UT_ASSERT(pRI->pRun == pRun);

	UT_DEBUGMSG(("Run: 0x%08lx xoff=%d\n", pRun, pRI->xoff));
}
