 
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

FL_SectionLayout::FL_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh)
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

fp_ColumnInfo::fp_ColumnInfo(FP_Column* _pCol, UT_sint32 _iXoff, UT_sint32 _iYoff)
{
	pColumn = _pCol;
	xoff = _iXoff;
	yoff = _iYoff;
}

FL_SectionLayout::~FL_SectionLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();

	UT_VECTOR_PURGEALL(FP_SectionSlice, m_vecSlices);
	UT_VECTOR_PURGEALL(FP_Column, m_vecColumns);

	if (m_pColumnSetLayout)
		delete m_pColumnSetLayout;

	if (m_pLB)
		delete m_pLB;
}

FL_DocLayout* FL_SectionLayout::getLayout()
{
	return m_pLayout;
}

FP_Column* FL_SectionLayout::getNewColumn()
{
	/*
		get the current page.
		ask it for space.
		within that space, create columns, in accordance with our ColumnSetLayout.
		add those columns to our column list, linking them up properly.
		return the first column in the new space.
	*/

	FP_SectionSlice* pSlice = NULL;
	FP_Page* pPage = NULL;
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

	FL_ColumnSetLayout * pCSL = m_pColumnSetLayout;
#if 0									// TODO support default column model on document
	if (!pCSL)
		pCSL = m_pLayout->getColumnSetLayout();
#endif
	UT_ASSERT(pCSL);

	// walk thru the list of column layouts and
	// let each one instantiate a FP_Column of
	// the right type and size.
	
	FL_ColumnLayout * pCL = pCSL->getFirstColumnLayout();
	UT_ASSERT(pCL);
	while (pCL)
	{
		UT_sint32 xoff, yoff;
		FP_Column* pCol = NULL;
		pCL->getNewColumn(iWidthSlice,iHeightSlice, &pCol,&xoff,&yoff);
		UT_ASSERT(pCol);

		// append the FP_Column to the list of columns.
		
		pSlice->addColumn(pCol, xoff, yoff);
		if (m_vecColumns.getItemCount())
		{
			FP_Column* pcLast = (FP_Column*) m_vecColumns.getLastItem();
			pcLast->setNext(pCol);
		}
		m_vecColumns.addItem(pCol);

		pCL = pCL->getNext();
	}

	return pSlice->getFirstColumn();
}

int FL_SectionLayout::format()
{
	UT_Bool bStillGoing = UT_TRUE;
	FL_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL && bStillGoing)
	{
		pBL->format();
		pBL = pBL->getNext();
	}

	return 0;	// TODO return code
}

UT_Bool FL_SectionLayout::reformat()
{
	UT_Bool bResult = UT_FALSE;
	FL_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
			pBL->draw(m_pLayout->getGraphics());
			bResult = UT_TRUE;
		}

		pBL = pBL->getNext();
	}

	return bResult;
}

void FL_SectionLayout::_purgeLayout()
{
	FL_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		FL_BlockLayout* pNuke = pBL;

		pBL = pBL->getNext();

		delete pNuke;
	}

	return;
}

void FL_SectionLayout::setColumnSetLayout(FL_ColumnSetLayout * pcsl)
{
	m_pColumnSetLayout = pcsl;
}

FL_ColumnSetLayout * FL_SectionLayout::getColumnSetLayout(void) const
{
	return m_pColumnSetLayout;
}

FL_BlockLayout * FL_SectionLayout::getFirstBlock(void) const
{
	return m_pFirstBlock;
}

FL_BlockLayout * FL_SectionLayout::insertBlock(PL_StruxDocHandle sdh, FL_BlockLayout * pPrev)
{
	FL_BlockLayout*	pBL = new FL_BlockLayout(sdh, _getLineBreaker(), pPrev, this);
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

FL_BlockLayout * FL_SectionLayout::appendBlock(PL_StruxDocHandle sdh)
{
	return insertBlock(sdh, m_pLastBlock);
}

FB_LineBreaker * FL_SectionLayout::_getLineBreaker(void)
{
	if (!m_pLB)
	{
		SimpleLineBreaker* slb = new SimpleLineBreaker();

		m_pLB = slb;
	}

	UT_ASSERT(m_pLB);

	return m_pLB;
}
