 
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
#include <math.h>
#include <string.h>

#include "fp_Column.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_BlockLayout.h"
#include "fp_SectionSlice.h"
#include "fp_BlockSlice.h"
#include "dg_Property.h"
#include "dg_Graphics.h"
#include "dg_DrawArgs.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

// ----------------------------------------------------------------
// TODO polygon column ?
// TODO bezier curve enclosed column ?
// TODO TeX parshape-style column, which simply specifies the length of each line.

FP_Column::FP_Column(FL_SectionLayout* pSectionLayout)
{
	m_pNext = NULL;
	m_pSectionSlice = NULL;
	m_pSectionSliceData = NULL;
	m_pSectionLayout = pSectionLayout;
	m_pFirstSlice = NULL;
	m_pG = m_pSectionLayout->getLayout()->getGraphics();
}

fp_BlockSliceInfo::fp_BlockSliceInfo(FP_BlockSlice* p)
{
	UT_ASSERT(p);

	pSlice = p;
	yoff = 0;
}

FP_Column::~FP_Column()
{
	while (m_pFirstSlice)
	{
		fp_BlockSliceInfo* pNext = m_pFirstSlice->pNext;
		delete m_pFirstSlice;
		m_pFirstSlice = pNext;
	}
}

void FP_Column::setSectionSlice(FP_SectionSlice* pSectionSlice, void* p)
{
	m_pSectionSlice = pSectionSlice;
	m_pSectionSliceData = p;
}

FP_SectionSlice* FP_Column::getSectionSlice() const
{
	return m_pSectionSlice;
}

void FP_Column::setNext(FP_Column*p)
{
	m_pNext = p;
}

void FP_Column::setPrev(FP_Column*p)
{
	m_pPrev = p;
}

FP_Column* FP_Column::getNext()
{
	return m_pNext;
}

FP_Column* FP_Column::getPrev()
{
	return m_pPrev;
}

void FP_Column::getOffsets(FP_BlockSlice* pBS, void* pData, UT_sint32& xoff, UT_sint32& yoff)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pSectionSlice->getOffsets(this, m_pSectionSliceData, my_xoff, my_yoff);
	
	fp_BlockSliceInfo* pBSI = (fp_BlockSliceInfo*) pData;
	UT_ASSERT(pBS == pBSI->pSlice);
	UT_ASSERT(pData == _findSlice(pBS));

	xoff = my_xoff + 0;
	yoff = my_yoff + pBSI->yoff;
}

void FP_Column::getScreenOffsets(FP_BlockSlice* pBS, void* pData,
								 UT_sint32& xoff, UT_sint32& yoff,
								 UT_sint32& width, UT_sint32& height)
{
	UT_sint32 my_xoff;
	UT_sint32 my_yoff;

	m_pSectionSlice->getScreenOffsets(this, m_pSectionSliceData,
									  my_xoff, my_yoff, width, height);
	
	fp_BlockSliceInfo* pBSI = (fp_BlockSliceInfo*) pData;
	UT_ASSERT(pBS == pBSI->pSlice);

	xoff = my_xoff + 0;
	yoff = my_yoff + pBSI->yoff;
}

fp_BlockSliceInfo*		FP_Column::_findSlice(FP_BlockSlice* p)
{
	fp_BlockSliceInfo*	pNode = m_pFirstSlice;

	while (pNode && (pNode->pSlice != p))
	{
		pNode = pNode->pNext;
	}

	return pNode;
}

void FP_Column::removeBlockSlice(FP_BlockSlice* p)
{
	fp_BlockSliceInfo*	pNode = m_pFirstSlice;
	fp_BlockSliceInfo*  pPrev = NULL;

	while (pNode && (pNode->pSlice != p))
	{
		pPrev = pNode;
		pNode = pNode->pNext;
	}

	UT_ASSERT(pNode);
	
	if (pPrev)
	{
		pPrev->pNext = pNode->pNext;
	}
	else
	{
		m_pFirstSlice = pNode->pNext;
	}

	_repositionSlices();
}

int FP_Column::insertBlockSliceAfter(FP_BlockSlice*	pBS, FP_BlockSlice*	pAfter, int iLineHeight)
{
	UT_ASSERT(pBS);
	UT_DEBUGMSG(("FP_Column::insertBlockSliceAfter - FP_Column=0x%x  slice=0x%x  pAfter=0x%x\n",
				 this,
				 pBS,
				 pAfter));

	fp_BlockSliceInfo* pNewNode;
	
	if (pAfter)
	{
		fp_BlockSliceInfo*	pPrevNode = _findSlice(pAfter);

		UT_ASSERT(pPrevNode);
		UT_ASSERT(pPrevNode->pSlice == pAfter);
		
		UT_sint32 iPrevBottomMargin =
			m_pG->convertDimension(pPrevNode->pSlice->getBlock()->getProperty(
				lookupProperty("margin-bottom")
				));
		
		UT_sint32 iMyTopMargin =
			m_pG->convertDimension(pBS->getBlock()->getProperty(
				lookupProperty("margin-top")
				));
		
		UT_sint32 iMargin = UT_MAX(iPrevBottomMargin, iMyTopMargin);

		UT_uint32 iY = pPrevNode->yoff + pAfter->getHeight() + iMargin;
		
		UT_uint32 iWidthAvailable = _getSliverWidth(iY, iLineHeight, NULL);
		if (iWidthAvailable == 0)
		{
			UT_DEBUGMSG(("Tried to insert a block slice, but no room.\n"));
			return -1;
		}

		pNewNode = new fp_BlockSliceInfo(pBS);
		if (!pNewNode)
		{
			return -1;
		}
		pBS->setColumn(this, pNewNode);

		pNewNode->pPrev = pPrevNode;
		pNewNode->pNext = pPrevNode->pNext;
		if (pPrevNode->pNext)
		{
			pPrevNode->pNext->pPrev = pNewNode;
		}
		pPrevNode->pNext = pNewNode;

		pNewNode->yoff = _calcSliceOffset(pNewNode, iLineHeight);
	}
	else
	{
		// insert it at the top of the column
		pNewNode = new fp_BlockSliceInfo(pBS);
		if (!pNewNode)
		{
			return -1;
		}
		pBS->setColumn(this, pNewNode);

		pNewNode->pPrev = NULL;
		pNewNode->pNext = m_pFirstSlice;
		m_pFirstSlice = pNewNode;
		
		pNewNode->yoff = _calcSliceOffset(pNewNode, iLineHeight);
	}

	UT_ASSERT(pNewNode);
	
	if (pNewNode->pNext)
	{
		return _repositionSlices();
	}
	else
	{
		return 0;
	}
}

UT_uint32 FP_Column::_calcSliceOffset(fp_BlockSliceInfo* pBSI, UT_uint32 iLineHeight)
{
	if (pBSI->pPrev)
	{
		UT_sint32 iPrevBottomMargin =
			m_pG->convertDimension(pBSI->pPrev->pSlice->getBlock()->getProperty(
				lookupProperty("margin-bottom")
				));
		
		UT_sint32 iMyTopMargin =
			m_pG->convertDimension(pBSI->pSlice->getBlock()->getProperty(
				lookupProperty("margin-top")
				));
		
		UT_sint32 iMargin = UT_MAX(iPrevBottomMargin, iMyTopMargin);

		return pBSI->pPrev->yoff + pBSI->pPrev->pSlice->getHeight() + iMargin;
	}
	else
	{
		if (iLineHeight)
		{
			return getTopOffset(iLineHeight);
		}
		else
		{
			return pBSI->yoff;
		}
	}
}

UT_Bool FP_Column::requestSliver(FP_BlockSlice* pBS, UT_uint32 iMoreHeightNeeded,
		UT_uint32* pX,
		UT_uint32* pWidth,
		UT_uint32* pHeight)
{
	UT_ASSERT(pBS);
	UT_ASSERT(pX);
	UT_ASSERT(pWidth);
	UT_ASSERT(pHeight);
	UT_ASSERT(iMoreHeightNeeded>0);

	fp_BlockSliceInfo*	pNode = _findSlice(pBS);
	UT_ASSERT(pNode);

	UT_uint32 iX;
	UT_uint32 iWidth = _getSliverWidth(pNode->yoff + pBS->getHeight(), iMoreHeightNeeded, &iX);
	if (0 == iWidth)
	{
		// the additional height would make this slice too big for the column
		return UT_FALSE;
	}

	// TODO note that we don't do margins for columns yet.
	*pX = iX;
	*pWidth = iWidth;
	*pHeight = iMoreHeightNeeded;
	
	return UT_TRUE;
}

UT_Bool FP_Column::verifySliverFit(FP_BlockSlice* pBS, fp_Sliver* pSliver, UT_sint32 iY)
{
	UT_ASSERT(pBS);
	UT_ASSERT(pSliver);

	fp_BlockSliceInfo*	pNode = _findSlice(pBS);
	UT_ASSERT(pNode);

	UT_uint32 iX;
	UT_uint32 iWidth = _getSliverWidth(iY + pNode->yoff, pSliver->iHeight, &iX);
	if (iWidth == pSliver->iWidth)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

void FP_Column::reportSliceHeightChanged(FP_BlockSlice* pBS, UT_uint32 iNewHeight)
{
	fp_BlockSliceInfo*	pNode = _findSlice(pBS);
	UT_ASSERT(pNode);

	if (pNode->pNext)
	{
		UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
		// TODO perhaps we should _repositionSlices ONCE after all edits to a paragraph
		// are done, rather than doing it so often?
		_repositionSlices();
	}
	else
	{
		// TODO do we need to do anything here, really?
	}
}

int FP_Column::_repositionSlices()
{
	UT_ASSERT(m_pG->queryProperties(DG_Graphics::DGP_SCREEN));
		
	fp_BlockSliceInfo* pListNode = m_pFirstSlice;
	while (pListNode)
	{
		UT_uint32 iCalcOffset = _calcSliceOffset(pListNode, 0);
		if (pListNode->yoff != iCalcOffset)
		{
			pListNode->pSlice->clearScreen(m_pG);

			if ((pListNode->yoff > iCalcOffset) && (!(pListNode->pNext)))
			{
				pListNode->pSlice->getBlock()->setNeedsReformat(UT_TRUE);
			}
			
			pListNode->yoff = iCalcOffset;
			UT_uint32 yCur = iCalcOffset;

			if (!pListNode->pSlice->getBlock()->needsReformat())
			{
				UT_Bool bFits = UT_TRUE;
				int countSlivers = pListNode->pSlice->countSlivers();
				for (int i=0; i<countSlivers; i++)
				{
					fp_Sliver* pSliver = pListNode->pSlice->getNthSliver(i);
				
					UT_uint32 iX;
					UT_uint32 wid = _getSliverWidth(yCur, pSliver->iHeight, &iX);
					// TODO shouldn't we also check the iX?
					if (wid < pSliver->iWidth)
					{
						UT_DEBUGMSG(("_repositionSlices:  found a sliver which no longer fits in the column.\n"));
					
						// this sliver no longer fits!!
						bFits = UT_FALSE;
						pListNode->pSlice->getBlock()->setNeedsReformat(UT_TRUE);
						break;
					}
				}

				if (bFits)
				{
					pListNode->pSlice->draw(m_pG);
				}
			}
		}

		pListNode = pListNode->pNext;
	}

	return 0;
}

void FP_Column::draw(dg_DrawArgs* pDA)
{
	// draw each FP_BlockSlice on in the column

	fp_BlockSliceInfo* pListNode = m_pFirstSlice;
	while (pListNode)
	{
		dg_DrawArgs da = *pDA;
		da.yoff += pListNode->yoff;
		pListNode->pSlice->draw(&da);
		pListNode = pListNode->pNext;
	}
}

void FP_Column::dump()
{
	fp_BlockSliceInfo* pListNode = m_pFirstSlice;
	while (pListNode)
	{
		UT_DEBUGMSG(("FP_Column::dump(0x%x) - FP_BlockSlice 0x%x in FL_BlockLayout 0x%x.  Offset=%d, height=%d\n", 
			this, 
			pListNode->pSlice, 
			pListNode->pSlice->getBlock(), 
			pListNode->yoff, 
			pListNode->pSlice->getHeight()));
		//pListNode->pSlice->dump();

		pListNode = pListNode->pNext;
	}
}

FP_BoxColumn::FP_BoxColumn(FL_SectionLayout* pSL, 
					 UT_uint32 width, UT_uint32 height) : FP_Column(pSL)
{
	m_iWidth = width;
	m_iHeight = height;
}

UT_uint32 FP_BoxColumn::_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX)
{
	if ((iY + iHeight) >= m_iHeight)
	{
		return 0;
	}

	if (pX)
	{
		*pX = 0;
	}

	return m_iWidth;
}

FP_CircleColumn::FP_CircleColumn(FL_SectionLayout* pSL, UT_uint32 iRadius) : FP_Column(pSL)
{
	m_iRadius = iRadius;
}

UT_uint32 FP_CircleColumn::_getSliverWidth(UT_uint32 iY, UT_uint32 iHeight, UT_uint32* pX)
{
	UT_uint32 halfwidth;

	if (iY > m_iRadius)
	{
		double vert = (iY + iHeight) - m_iRadius;
		halfwidth = (UT_uint32) sqrt((m_iRadius * m_iRadius) - (vert * vert));
	}
	else
	{
		double vert = m_iRadius - iY;
		halfwidth = (UT_uint32) sqrt((m_iRadius * m_iRadius) - (vert * vert));
	}

	if (pX)
	{
		*pX = m_iRadius - halfwidth;
	}

	return 2 * halfwidth;
}

UT_uint32 FP_BoxColumn::getTopOffset(UT_uint32 iLineHeight)
{
	return 0;
}

UT_uint32 FP_CircleColumn::getTopOffset(UT_uint32 iLineHeight)
{
	return iLineHeight;
}

UT_Bool FP_BoxColumn::containsPoint(UT_sint32 x, UT_sint32 y)
{
	if ((x < 0) || (x >= (UT_sint32)m_iWidth))
	{
		return UT_FALSE;
	}
	if ((y < 0) || (y >= (UT_sint32)m_iHeight))
	{
		return UT_FALSE;
	}

	return UT_TRUE;
}

void FP_Column::mapXYToBufferPosition(UT_sint32 x, UT_sint32 y, UT_uint32& pos, UT_Bool& bRight)
{
	fp_BlockSliceInfo* pListNode = m_pFirstSlice;
	UT_uint32 iMinDist = 0xffffffff;
	fp_BlockSliceInfo* pMinDist = NULL;
	
	while (pListNode)
	{
		int countSlivers = pListNode->pSlice->countSlivers();
		UT_sint32 iY = 0;
		for (int i=0; i<countSlivers; i++)
		{
			fp_Sliver* pSliver = pListNode->pSlice->getNthSliver(i);

			// when hit testing slivers within a column, we ignore the X coord
//			if ((x >= pSliver->iX) && (x < (pSliver->iX + pSliver->iWidth)))
			{
				if (((y - (UT_sint32)pListNode->yoff) >= iY) && ((y - (UT_sint32)pListNode->yoff) < (iY + (UT_sint32)pSliver->iHeight)))
				{
					pListNode->pSlice->mapXYToBufferPosition(x, y - pListNode->yoff, pos, bRight);
					return;
				}
			}

			UT_uint32 iDistTop;
			UT_uint32 iDistBottom;
			UT_uint32 iDist;

			iDistTop = UT_ABS((iY - (y - (UT_sint32) pListNode->yoff)));
			iDistBottom = UT_ABS(((iY + (UT_sint32) pSliver->iHeight) - (y - (UT_sint32) pListNode->yoff)));
			
			iDist = UT_MIN(iDistTop, iDistBottom);
			if (iDist < iMinDist)
			{
				pMinDist = pListNode;
				iMinDist = iDist;
			}

			iY += pSliver->iHeight;
		}
		
		pListNode = pListNode->pNext;
	}

	UT_ASSERT(pMinDist);

	pMinDist->pSlice->mapXYToBufferPosition(x, y - pMinDist->yoff, pos, bRight);
	return;
}

UT_Bool FP_CircleColumn::containsPoint(UT_sint32 x, UT_sint32 y)
{
	if ((sqrt(((y - m_iRadius) * (y - m_iRadius)) + ((x - m_iRadius) * (x - m_iRadius)))) > m_iRadius)
	{
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_uint32 	FP_CircleColumn::distanceFromPoint(UT_sint32 x, UT_sint32 y)
{
	UT_uint32 dist = (UT_uint32) ((sqrt(((y - m_iRadius) * (y - m_iRadius)) + ((x - m_iRadius) * (x - m_iRadius)))));
	return dist - m_iRadius;
}

UT_uint32	FP_BoxColumn::distanceFromPoint(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 dx;
	UT_sint32 dy;
	
	if (x < 0)
	{
		dx = -x;
	}
	else if (x > (UT_sint32)m_iWidth)
	{
		dx = x - m_iWidth;
	}
	else
	{
		dx = 0;
	}

	if (y < 0)
	{
		dy = -y;
	}
	else if (y > (UT_sint32)m_iHeight)
	{
		dy = y - m_iHeight;
	}
	else
	{
		dy = 0;
	}

	UT_uint32 dist = (UT_uint32) (sqrt((dx * dx) + (dy * dy)));

	return dist;
}

