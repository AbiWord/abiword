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
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Column.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"

#include "ut_assert.h"

fl_SectionLayout::fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_Layout(PTX_Section, sdh)
{
	UT_ASSERT(pLayout);
	
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pLB = NULL;
	m_pFirstBlock = NULL;
	m_pLastBlock = NULL;

	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

	setAttrPropIndex(indexAP);
	const PP_AttrProp* pSectionAP = NULL;
	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);

	XML_Char* pszNumColumns = NULL;
	XML_Char* pszColumnGap = NULL;

	pSectionAP->getAttribute("NUM_COLUMNS", pszNumColumns);
	pSectionAP->getAttribute("COLUMN_GAP", pszColumnGap);

	if (pszNumColumns && pszNumColumns[0] && pszColumnGap && pszColumnGap[0])
	{
		m_iNumColumns = atoi(pszNumColumns);
		m_iColumnGap = m_pLayout->getGraphics()->convertDimension(pszColumnGap);
	}
	else
	{
		m_iNumColumns = 1;
		m_iColumnGap = m_pLayout->getGraphics()->convertDimension("0.25in");
	}
	
	m_bForceNewPage = UT_FALSE;
}

fl_SectionLayout::~fl_SectionLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();

	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		fp_Column* pNext = pCol->getNext();

		delete pCol;

		pCol = pNext;
	}
	
	if (m_pLB)
	{
		delete m_pLB;
	}
}

void fl_SectionLayout::deleteEmptyColumns(void)
{
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			fp_Column* pCol2 = pCol;
			UT_Bool bAllEmpty = UT_TRUE;
			fp_Column* pLastInGroup = NULL;
			
			while (pCol2)
			{
				if (!pCol2->isEmpty())
				{
					bAllEmpty = UT_FALSE;
				}

				if (!(pCol2->getFollower()))
				{
					pLastInGroup = pCol2;
				}
				
				pCol2 = pCol2->getFollower();
			}

			if (bAllEmpty)
			{
				UT_ASSERT(pLastInGroup);

				pCol->getPage()->removeColumnLeader(pCol);

				if (pLastInGroup == m_pLastColumn)
				{
					m_pLastColumn = pCol->getPrev();
				}
				
				if (pCol->getPrev())
				{
					pCol->getPrev()->setNext(pLastInGroup->getNext());
				}

				if (pLastInGroup->getNext())
				{
					pLastInGroup->getNext()->setPrev(pCol->getPrev());
				}

				fp_Column* pCol2 = pCol;
				pCol = pLastInGroup->getNext();
				while (pCol2)
				{
					fp_Column* pNext = pCol2->getFollower();

					delete pCol2;

					pCol2 = pNext;
				}
			}
			else
			{
				pCol = pLastInGroup->getNext();
			}
		}
		else
		{
			pCol = pCol->getNext();
		}
	}
}

FL_DocLayout* fl_SectionLayout::getDocLayout() const
{
	return m_pLayout;
}

fp_Column* fl_SectionLayout::getFirstColumn() const
{
	return m_pFirstColumn;
}

fp_Column* fl_SectionLayout::getLastColumn() const
{
	return m_pLastColumn;
}

fp_Column* fl_SectionLayout::getNewColumn(void)
{
	/*
	  This is called to create a new column (or row of same).
	*/
	fp_Page* pPage = NULL;
	fp_Column* pLastColumn = getLastColumn();
	fp_Column* pAfterColumn = NULL;
	UT_Bool bAddAtEndOfPage = UT_FALSE;
	
	if (pLastColumn)
	{
		fp_Page* pTmpPage = pLastColumn->getPage();
		if (pTmpPage->getNext())
		{
			pPage = pTmpPage->getNext();
		}
		else
		{
			pPage = m_pLayout->addNewPage();
		}
	}
	else
	{
		/*
		  We currently have no columns.  Time to create some.
		  If there is a previous section, then we need to
		  start our section right after that one.  If not, then
		  we start our section on the first page.  If there is no
		  first page, then we need to create one.
		*/
		fl_SectionLayout* pPrevSL = m_pLayout->getPrevSection(this);
		if (pPrevSL)
		{
			fp_Page* pTmpPage = pPrevSL->getLastColumn()->getPage();
			if (m_bForceNewPage)
			{
				if (pTmpPage->getNext())
				{
					pPage = pTmpPage->getNext();
				}
				else
				{
					pPage = m_pLayout->addNewPage();
				}
			}
			else
			{
				pPage = pTmpPage;
				pAfterColumn = pPage->getNthColumnLeader(pPage->countColumnLeaders()-1);
			}
		}
		else
		{
			if (m_pLayout->countPages() > 0)
			{
				pPage = m_pLayout->getFirstPage();
			}
			else
			{
				pPage = m_pLayout->addNewPage();
			}
		}
	}

	UT_ASSERT(pPage);

	fp_Column* pLeaderColumn = NULL;
	fp_Column* pTail = NULL;
	for (UT_uint32 i=0; i<m_iNumColumns; i++)
	{
		fp_Column* pCol = new fp_Column(this);

		if (pTail)
		{
			pCol->setLeader(pLeaderColumn);
			pTail->setFollower(pCol);
			pTail->setNext(pCol);
			pCol->setPrev(pTail);
			
			pTail = pCol;
		}
		else
		{
			pLeaderColumn = pTail = pCol;
			pLeaderColumn->setLeader(pLeaderColumn);
		}
	}

	fp_Column* pLastNewCol = pLeaderColumn;
	while (pLastNewCol->getFollower())
	{
		pLastNewCol = pLastNewCol->getFollower();
	}

	if (m_pLastColumn)
	{
		UT_ASSERT(m_pFirstColumn);

		m_pLastColumn->setNext(pLeaderColumn);
		pLeaderColumn->setPrev(m_pLastColumn);
	}
	else
	{
		UT_ASSERT(!m_pFirstColumn);
		
		m_pFirstColumn = pLeaderColumn;
	}
	
	m_pLastColumn = pLastNewCol;
	UT_ASSERT(!(m_pLastColumn->getNext()));

	pPage->insertColumnLeader(pLeaderColumn, pAfterColumn);
	
	fp_Column* pTmpCol = pLeaderColumn;
 	while (pTmpCol)
	{
		UT_ASSERT(pTmpCol->getPage());
		
		pTmpCol = pTmpCol->getFollower();
	}

	return pLeaderColumn;
}

int fl_SectionLayout::format()
{
	UT_Bool bStillGoing = UT_TRUE;
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL && bStillGoing)
	{
		pBL->format();
		pBL = pBL->getNext(UT_FALSE);
	}

	return 0;	// TODO return code
}

UT_Bool fl_SectionLayout::reformat()
{
	UT_Bool bResult = UT_FALSE;
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
			
			bResult = UT_TRUE;
		}

		pBL = pBL->getNext(UT_FALSE);
	}

	return bResult;
}

void fl_SectionLayout::_purgeLayout()
{
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		fl_BlockLayout* pNuke = pBL;

		pBL = pBL->getNext(UT_FALSE);

		delete pNuke;
	}

	return;
}

fl_BlockLayout * fl_SectionLayout::getFirstBlock(void) const
{
	return m_pFirstBlock;
}

fl_BlockLayout * fl_SectionLayout::getLastBlock(void) const
{
	return m_pLastBlock;
}

fl_BlockLayout * fl_SectionLayout::appendBlock(PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
{
	return insertBlock(sdh, m_pLastBlock, indexAP);
}

fl_BlockLayout * fl_SectionLayout::insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev, PT_AttrPropIndex indexAP)
{
	fl_BlockLayout*	pBL = new fl_BlockLayout(sdh, _getLineBreaker(), pPrev, this, indexAP);
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
	{
		m_pLastBlock = pBL->getPrev(UT_FALSE);
	}

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

UT_uint32 fl_SectionLayout::getNumColumns(void) const
{
	return m_iNumColumns;
}

UT_uint32 fl_SectionLayout::getColumnGap(void) const
{
	return m_iColumnGap;
}

