 
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


#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "fl_DocListener.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_ColumnSetLayout.h"
#include "fl_ColumnLayout.h"
#include "fl_BlockLayout.h"
#include "fp_Page.h"
#include "pd_Document.h"
#include "dg_Graphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

FL_DocLayout::FL_DocLayout(PD_Document* doc, DG_Graphics* pG)
{
	m_pDoc = doc;
	m_pG = pG;
	m_pView = NULL;

	// TODO the following (both the new() and the addListener() cause
	// TODO malloc's to occur.  we are currently inside a constructor
	// TODO and cannot report failure.
	
	m_pDocListener = new fl_DocListener(doc, this);

	if (doc->addListener(static_cast<PL_Listener *>(m_pDocListener),&m_lid))
	{
		FILE * fpDump1 = fopen("dump1","w");
		doc->dump(fpDump1);
		fclose(fpDump1);
	}
}

FL_DocLayout::~FL_DocLayout()
{
	if (m_pDoc)
		m_pDoc->removeListener(m_lid);

	UT_VECTOR_PURGEALL(FP_Page, m_vecPages);
	UT_VECTOR_PURGEALL(FL_SectionLayout, m_vecSectionLayouts);

	if (m_pDoc)
		delete m_pDoc;
}

void FL_DocLayout::setView(FV_View* pView)
{
	FP_Page* pPage = getFirstPage();
	
	while (pPage)
	{
		pPage->setView(pView);
		
		pPage = pPage->getNext();
	}
}

PD_Document* FL_DocLayout::getDocument() const
{
	return m_pDoc;
}

DG_Graphics* FL_DocLayout::getGraphics()
{
	return m_pG;
}

UT_uint32 FL_DocLayout::getHeight()
{
	UT_uint32 iHeight = 0;
	int count = m_vecPages.getItemCount();

	for (int i=0; i<count; i++)
	{
		FP_Page* p = (FP_Page*) m_vecPages.getNthItem(i);

		iHeight += p->getHeight();
	}

	return iHeight;
}

int FL_DocLayout::countPages()
{
	return m_vecPages.getItemCount();
}

FP_Page* FL_DocLayout::getNthPage(int n)
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);

	return (FP_Page*) m_vecPages.getNthItem(n);
}

FP_Page* FL_DocLayout::getFirstPage()
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);

	return (FP_Page*) m_vecPages.getNthItem(0);
}

FP_Page* FL_DocLayout::getLastPage()
{
	UT_ASSERT(m_vecPages.getItemCount() > 0);

	return (FP_Page*) m_vecPages.getNthItem(m_vecPages.getItemCount()-1);
}

FP_Page* FL_DocLayout::addNewPage()
{
	FP_Page*		pLastPage;

	if (countPages() > 0)
	{
		pLastPage = getLastPage();
	}
	else
	{
		pLastPage = NULL;
	}
	
	// TODO pass the margins.  which ones?
	FP_Page*		pPage = new FP_Page(this, m_pView, 850, 1100, 100, 100, 100, 100);
	if (pLastPage)
	{
		UT_ASSERT(pLastPage->getNext() == NULL);

		pLastPage->setNext(pPage);
	}
	m_vecPages.addItem(pPage);

	return pPage;
}

FL_BlockLayout* FL_DocLayout::findBlockAtPosition(PT_DocPosition pos)
{
	FL_BlockLayout* pBL = NULL;
	PL_StruxFmtHandle sfh;

	if (m_pDoc->getStruxFromPosition(m_lid, pos, &sfh))
	{
		fl_Layout * pL = (fl_Layout *)sfh;
		switch (pL->getType())
		{
		case PTX_Block:
			pBL = static_cast<FL_BlockLayout *>(pL);
			break;
				
		case PTX_Section:
		case PTX_ColumnSet:
		case PTX_Column:
		default:
			UT_ASSERT((0));
		}
	}
	else
	{
		UT_ASSERT((0));
	}


	return pBL;
}

int FL_DocLayout::formatAll()
{
	UT_ASSERT(m_pDoc);
	UT_DEBUGMSG(("BEGIN Formatting document: 0x%x\n", this));

	UT_Bool bStillGoing = UT_TRUE;
	int countSections = m_vecSectionLayouts.getItemCount();

	while (bStillGoing)
	{
		bStillGoing = UT_FALSE;
		
		for (int i=0; i<countSections; i++)
		{
			FL_SectionLayout* pSL = (FL_SectionLayout*) m_vecSectionLayouts.getNthItem(i);

			bStillGoing = pSL->format() || bStillGoing;
		}
	}

	UT_DEBUGMSG(("END Formatting document: 0x%x\n", this));
	return 0;
}

int FL_DocLayout::reformat()
{
	UT_Bool bStillGoing = UT_TRUE;
	int countSections = m_vecSectionLayouts.getItemCount();

	while (bStillGoing)
	{
		bStillGoing = UT_FALSE;
		
		for (int i=0; i<countSections; i++)
		{
			FL_SectionLayout* pSL = (FL_SectionLayout*) m_vecSectionLayouts.getNthItem(i);

			bStillGoing = pSL->reformat() || bStillGoing;
		}
	}

	return 0;
}

void FL_DocLayout::dump()
{
	int count = m_vecPages.getItemCount();
	UT_DEBUGMSG(("FL_DocLayout::dump(0x%x) contains %d pages.\n", this, m_vecPages.getItemCount()));

	for (int i=0; i<count; i++)
	{
		FP_Page* p = (FP_Page*) m_vecPages.getNthItem(i);

		p->dump();
	}

	// TODO dump the section layouts
}
