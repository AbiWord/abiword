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

#include "ut_types.h"

#include "fl_SectionLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fl_ColumnSetLayout.h"
#include "fl_ColumnLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_SectionSlice.h"
#include "fp_Column.h"
#include "pd_Document.h"

#include "ut_assert.h"

fl_SectionLayout::fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh)
	: fl_Layout(PTX_Section, sdh)
{
	UT_ASSERT(pLayout);
	
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pLB = NULL;
	m_pColumnSetLayout = NULL;
	m_pFirstBlock = NULL;
	m_pLastBlock = NULL;
}

fp_ColumnInfo::fp_ColumnInfo(fp_Column* _pCol, UT_sint32 _iXoff, UT_sint32 _iYoff)
{
	pColumn = _pCol;
	xoff = _iXoff;
	yoff = _iYoff;
}

fl_SectionLayout::~fl_SectionLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();

	UT_VECTOR_PURGEALL(fp_SectionSlice, m_vecSlices);
	UT_VECTOR_PURGEALL(fp_Column, m_vecColumns);

	if (m_pColumnSetLayout)
		delete m_pColumnSetLayout;

	if (m_pLB)
		delete m_pLB;
}

FL_DocLayout* fl_SectionLayout::getLayout()
{
	return m_pLayout;
}

fp_Column* fl_SectionLayout::getNewColumn()
{
	/*
		get the current page.
		ask it for space.
		within that space, create columns, in accordance with our ColumnSetLayout.
		add those columns to our column list, linking them up properly.
		return the first column in the new space.
	*/

	fp_SectionSlice* pSlice = NULL;
	fp_Page* pPage = NULL;
	if (m_pLayout->countPages() > 0)
	{
		pPage = m_pLayout->getLastPage();
	}

	if (pPage && pPage->requestSpace(this, &pSlice))
	{
		UT_ASSERT(pSlice);
	}
	else
	{
		pPage = m_pLayout->addNewPage();
		if (!pPage->requestSpace(this, &pSlice))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}

	UT_ASSERT(pSlice);
	m_vecSlices.addItem(pSlice);

	UT_sint32 iWidthSlice = pSlice->getWidth();
	UT_sint32 iHeightSlice = pSlice->getHeight();

	fl_ColumnSetLayout * pCSL = m_pColumnSetLayout;
#if 0									// TODO support default column model on document
	if (!pCSL)
		pCSL = m_pLayout->getColumnSetLayout();
#endif
	UT_ASSERT(pCSL);

	// walk thru the list of column layouts and
	// let each one instantiate a fp_Column of
	// the right type and size.
	
	fl_ColumnLayout * pCL = pCSL->getFirstColumnLayout();
	UT_ASSERT(pCL);
	while (pCL)
	{
		UT_sint32 xoff, yoff;
		fp_Column* pCol = NULL;
		pCL->getNewColumn(iWidthSlice,iHeightSlice, &pCol,&xoff,&yoff);
		UT_ASSERT(pCol);

		// append the fp_Column to the list of columns.
		
		pSlice->addColumn(pCol, xoff, yoff);
		if (m_vecColumns.getItemCount())
		{
			fp_Column* pcLast = (fp_Column*) m_vecColumns.getLastItem();
			pcLast->setNext(pCol);
		}
		m_vecColumns.addItem(pCol);

		pCL = pCL->getNext();
	}

	return pSlice->getFirstColumn();
}

int fl_SectionLayout::format()
{
	UT_Bool bStillGoing = UT_TRUE;
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL && bStillGoing)
	{
		pBL->complete_format();
		pBL = pBL->getNext();
	}

	return 0;	// TODO return code
}

UT_Bool fl_SectionLayout::reformat()
{
	UT_Bool bResult = UT_FALSE;
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		if (pBL->needsCompleteReformat())
		{
			pBL->complete_format();
			pBL->draw(m_pLayout->getGraphics());
			
			bResult = UT_TRUE;
		}

		pBL = pBL->getNext();
	}

	return bResult;
}

void fl_SectionLayout::_purgeLayout()
{
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		fl_BlockLayout* pNuke = pBL;

		pBL = pBL->getNext();

		delete pNuke;
	}

	return;
}

void fl_SectionLayout::setColumnSetLayout(fl_ColumnSetLayout * pcsl)
{
	m_pColumnSetLayout = pcsl;
}

fl_ColumnSetLayout * fl_SectionLayout::getColumnSetLayout(void) const
{
	return m_pColumnSetLayout;
}

fl_BlockLayout * fl_SectionLayout::getFirstBlock(void) const
{
	return m_pFirstBlock;
}

fl_BlockLayout * fl_SectionLayout::appendBlock(PL_StruxDocHandle sdh)
{
	return insertBlock(sdh, m_pLastBlock);
}

fl_BlockLayout * fl_SectionLayout::insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev)
{
	fl_BlockLayout*	pBL = new fl_BlockLayout(sdh, _getLineBreaker(), pPrev, this);
	if (!pBL)
		return pBL;

	if (!m_pLastBlock)
	{
		UT_ASSERT(!m_pFirstBlock);
		m_pFirstBlock = pBL;
		m_pLastBlock = pBL;
	}
	else if (m_pLastBlock == pPrev)
	{
		m_pLastBlock = pBL;
	}
	else if (!pPrev)
	{
		m_pFirstBlock = pBL;
	}

	return pBL;
}

fl_BlockLayout * fl_SectionLayout::removeBlock(fl_BlockLayout * pBL)
{
	if (!pBL)
		return pBL;

	UT_ASSERT(pBL != m_pFirstBlock);
	UT_ASSERT(m_pLastBlock);

	if (m_pLastBlock == pBL)
		m_pLastBlock = pBL->getPrev();

	return pBL;
}

fb_LineBreaker * fl_SectionLayout::_getLineBreaker(void)
{
	if (!m_pLB)
	{
		fb_SimpleLineBreaker* slb = new fb_SimpleLineBreaker();

		m_pLB = slb;
	}

	UT_ASSERT(m_pLB);

	return m_pLB;
}
