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
#include "fb_ColumnBreaker.h"
#include "fp_Page.h"
#include "fp_Column.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "pp_Property.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "fv_View.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

fl_SectionLayout::fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_Layout(PTX_Section, sdh)
{
	UT_ASSERT(pLayout);
	
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pLB = NULL;
	m_pCB = new fb_ColumnBreaker();
	m_pFirstBlock = NULL;
	m_pLastBlock = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;

	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

	setAttrPropIndex(indexAP);

	_lookupProperties();
}

void fl_SectionLayout::_lookupProperties(void)
{
	const PP_AttrProp* pSectionAP = NULL;
	
	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);

	/*
	  TODO shouldn't we be using PP_evalProperty like
	  the blockLayout does?

	  Yes, since PP_evalProperty does a fallback to the
	  last-chance defaults, whereas the code below is
	  hard-coding its own defaults.  Bad idea.
	*/
	
	const char* pszNumColumns = NULL;
	pSectionAP->getProperty("columns", pszNumColumns);
	if (pszNumColumns && pszNumColumns[0])
	{
		m_iNumColumns = atoi(pszNumColumns);
	}
	else
	{
		m_iNumColumns = 1;
	}

	const char* pszColumnGap = NULL;
	pSectionAP->getProperty("column-gap", pszColumnGap);
	if (pszColumnGap && pszColumnGap[0])
	{
		m_iColumnGap = m_pLayout->getGraphics()->convertDimension(pszColumnGap);
	}
	else
	{
		m_iColumnGap = m_pLayout->getGraphics()->convertDimension("0.25in");
	}

	const char* pszSpaceAfter = NULL;
	pSectionAP->getProperty("section-space-after", pszSpaceAfter);
	if (pszSpaceAfter && pszSpaceAfter[0])
	{
		m_iSpaceAfter = m_pLayout->getGraphics()->convertDimension(pszSpaceAfter);
	}
	else
	{
		m_iSpaceAfter = m_pLayout->getGraphics()->convertDimension("0in");
	}

	const char* pszLeftMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszBottomMargin = NULL;
	pSectionAP->getProperty("page-margin-left", pszLeftMargin);
	pSectionAP->getProperty("page-margin-top", pszTopMargin);
	pSectionAP->getProperty("page-margin-right", pszRightMargin);
	pSectionAP->getProperty("page-margin-bottom", pszBottomMargin);

	if (
		pszLeftMargin
		&& pszTopMargin
		&& pszRightMargin
		&& pszBottomMargin
		&& pszLeftMargin[0]
		&& pszTopMargin[0]
		&& pszRightMargin[0]
		&& pszBottomMargin[0]
		)
	{
		m_iLeftMargin = m_pLayout->getGraphics()->convertDimension(pszLeftMargin);
		m_iTopMargin = m_pLayout->getGraphics()->convertDimension(pszTopMargin);
		m_iRightMargin = m_pLayout->getGraphics()->convertDimension(pszRightMargin);
		m_iBottomMargin = m_pLayout->getGraphics()->convertDimension(pszBottomMargin);
	}
	else
	{
		m_iLeftMargin = UT_docUnitsFromPaperUnits(m_pLayout->getGraphics(), 100);
		m_iTopMargin = UT_docUnitsFromPaperUnits(m_pLayout->getGraphics(), 100);
		m_iRightMargin = UT_docUnitsFromPaperUnits(m_pLayout->getGraphics(), 100);
		m_iBottomMargin = UT_docUnitsFromPaperUnits(m_pLayout->getGraphics(), 100);
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

	delete m_pCB;
}

void fl_SectionLayout::setNext(fl_SectionLayout* pSL)
{
	m_pNext = pSL;
}

void fl_SectionLayout::setPrev(fl_SectionLayout* pSL)
{
	m_pPrev = pSL;
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

				if (pCol == m_pFirstColumn)
				{
					m_pFirstColumn = pLastInGroup->getNext();
				}

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

				fp_Column* pCol3 = pCol;
				pCol = pLastInGroup->getNext();
				while (pCol3)
				{
					fp_Column* pNext = pCol3->getFollower();

					delete pCol3;

					pCol3 = pNext;
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
		fl_SectionLayout* pPrevSL = getPrev();
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

void fl_SectionLayout::format()
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->format();
		pBL = pBL->getNext();
	}

	m_pCB->breakSection(this);
}

void fl_SectionLayout::updateLayout()
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
		}
		
		pBL = pBL->getNext();
	}

	m_pCB->breakSection(this);
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

void fl_SectionLayout::addBlock(fl_BlockLayout* pBL)
{
	if (m_pLastBlock)
	{
		UT_ASSERT(m_pLastBlock->getNext() == NULL);
		
		pBL->setNext(NULL);
		pBL->setPrev(m_pLastBlock);
		m_pLastBlock->setNext(pBL);
		m_pLastBlock = pBL;
	}
	else
	{
		UT_ASSERT(!m_pFirstBlock);
		
		pBL->setNext(NULL);
		pBL->setPrev(NULL);
		m_pFirstBlock = pBL;
		m_pLastBlock = m_pFirstBlock;
	}
}

fl_BlockLayout * fl_SectionLayout::insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev, PT_AttrPropIndex indexAP)
{
	fl_BlockLayout*	pBL = new fl_BlockLayout(sdh, _getLineBreaker(), pPrev, this, indexAP);
	if (!pBL)
	{
		return pBL;
	}

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

void fl_SectionLayout::removeBlock(fl_BlockLayout * pBL)
{
	UT_ASSERT(pBL);
	UT_ASSERT(m_pFirstBlock);
	
	if (pBL->getPrev())
	{
		pBL->getPrev()->setNext(pBL->getNext());
	}

	if (pBL->getNext())
	{
		pBL->getNext()->setPrev(pBL->getPrev());
	}
	
	if (pBL == m_pFirstBlock)
	{
		m_pFirstBlock = m_pFirstBlock->getNext();
		if (!m_pFirstBlock)
		{
			m_pLastBlock = NULL;
		}
	}

	if (pBL == m_pLastBlock)
	{
		m_pLastBlock = m_pLastBlock->getPrev();
		if (!m_pLastBlock)
		{
			m_pFirstBlock = NULL;
		}
	}

	pBL->setNext(NULL);
	pBL->setPrev(NULL);
}

fb_LineBreaker * fl_SectionLayout::_getLineBreaker(void)
{
	if (!m_pLB)
	{
		fb_LineBreaker* slb = new fb_LineBreaker();

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

UT_Bool fl_SectionLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	setAttrPropIndex(pcrxc->getIndexAP());

	_lookupProperties();

	// clear all the columns
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		pCol->clearScreen();

		pCol = pCol->getNext();
	}

	// remove all the columns from their pages
	pCol = m_pFirstColumn;
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			pCol->getPage()->removeColumnLeader(pCol);
		}

		pCol = pCol->getNext();
	}

	// get rid of all the layout information for every block
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->collapse();

		pBL = pBL->getNext();
	}

	// delete all our columns
	pCol = m_pFirstColumn;
	while (pCol)
	{
		fp_Column* pNext = pCol->getNext();

		delete pCol;

		pCol = pNext;
	}

	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/
	
	format();
	
	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTSECTION);
	}

	return UT_FALSE;
}

#if 0
UT_Bool fl_SectionLayout::doclistener_insertStrux(const PX_ChangeRecord_Strux * pcrx,
												  PL_StruxDocHandle sdh,
												  PL_ListenerId lid,
												  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																		  PL_ListenerId lid,
																		  PL_StruxFmtHandle sfhNew))
{
	// TODO i don't believe that this function is necessary.
	
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);

	UT_ASSERT(UT_TODO);

#if 0	
	fl_SectionLayout * pNewSL = NULL;						// TODO

	// must call the bind function to complete the exchange
	// of handles with the document (piece table) *** before ***
	// anything tries to call down into the document (like all
	// of the view listeners).
	
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle)pNewSL;
	pfnBindHandles(sdh,lid,sfhNew);
#endif
	
	return UT_FALSE;
}
#endif

UT_Bool fl_SectionLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Section);

	fl_SectionLayout* pPrevSL = m_pPrev;
	if (!pPrevSL)
	{
		// TODO shouldn't this just assert?
		UT_DEBUGMSG(("no prior SectionLayout"));
		return UT_FALSE;
	}
	
	// clear all the columns
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		pCol->clearScreen();

		pCol = pCol->getNext();
	}

	// remove all the columns from their pages
	pCol = m_pFirstColumn;
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			pCol->getPage()->removeColumnLeader(pCol);
		}

		pCol = pCol->getNext();
	}

	// get rid of all the layout information for every block
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->collapse();

		pBL = pBL->getNext();
	}

	// delete all our columns
	pCol = m_pFirstColumn;
	while (pCol)
	{
		fp_Column* pNext = pCol->getNext();

		delete pCol;

		pCol = pNext;
	}

	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

	while (m_pFirstBlock)
	{
		pBL = m_pFirstBlock;
		removeBlock(pBL);
		pPrevSL->addBlock(pBL);
	}
	
	pPrevSL->m_pNext = m_pNext;
							
	if (m_pNext)
	{
		m_pNext->m_pPrev = pPrevSL;
	}

	m_pLayout->removeSection(this);

	pPrevSL->format();
	
	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->_setPoint(pcrx->getPosition());
	}

	delete this;			// TODO whoa!  this construct is VERY dangerous.
	
	return UT_TRUE;
}

