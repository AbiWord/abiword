 
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
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_Strux.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fp_Page.h"
#include "pd_Document.h"
#include "dg_Graphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

FL_DocLayout::FL_DocLayout(PD_Document* doc, DG_Graphics* pG)
{
	m_pDoc = doc;
	m_pG = pG;
	m_pLayoutView = NULL;
}

FL_DocLayout::~FL_DocLayout()
{
	UT_VECTOR_PURGEALL(FP_Page, m_vecPages);
	UT_VECTOR_PURGEALL(FL_SectionLayout, m_vecSectionLayouts);

	if (m_pDoc)
		delete m_pDoc;
}

void FL_DocLayout::setLayoutView(DG_LayoutView* pLayoutView)
{
	FP_Page* pPage = getFirstPage();
	
	while (pPage)
	{
		pPage->setLayoutView(pLayoutView);
		
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
	FP_Page*		pPage = new FP_Page(this, m_pLayoutView, 850, 1100, 100, 100, 100, 100);
	if (pLastPage)
	{
		UT_ASSERT(pLastPage->getNext() == NULL);

		pLastPage->setNext(pPage);
	}
	m_vecPages.addItem(pPage);

	return pPage;
}

int FL_DocLayout::formatAll()
{
	UT_ASSERT(m_pDoc);
	UT_DEBUGMSG(("BEGIN Formatting document: 0x%x\n", this));

#ifdef BUFFER	// formatAll
	UT_UCSChar data;
	DG_DocMarkerId dmid;
	UT_uint32 pos = 0;

	while (1)
	{
		switch (m_pBuffer->getOneItem(pos,&data,&dmid))
		{
		case DG_DBPI_END:
			UT_DEBUGMSG(("END Formatting document: 0x%x\n", this));
			return 0;

		case DG_DBPI_DATA:
			break;
			
		case DG_DBPI_MARKER:
		{
			DG_DocMarker* pMarker = m_pBuffer->fetchDocMarker(dmid);
			if ((pMarker->getType() & DG_MT_SECTION)
				&& !(pMarker->getType() & DG_MT_END))
			{
				FL_SectionLayout* pSL = new FL_SectionLayout(this, pMarker);
				pSL->format();
				m_vecSectionLayouts.addItem(pSL);
			}
			break;
		}
			
		case DG_DBPI_ERROR:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		m_pBuffer->inc(pos);
	}
#endif /* BUFFER */
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

/*****************************************************************/
/*****************************************************************/


UT_Bool FL_DocLayout::populate(PL_StruxFmtHandle sfh,
							   PX_ChangeRecord * pcr)
{
	UT_DEBUGMSG(("FL_DocLayout::populate\n"));
	pcr->dump();

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertSpan);
	PX_ChangeRecord_Span * pcrs = static_cast<PX_ChangeRecord_Span *> (pcr);

	// TODO -- span is block-relative, right?
	// HYP: append as a new run to last block of last section
	// ALT: if format same, append to last run of that block
	// ==>: pass enough info to delegate the decision  :-)

	return UT_TRUE;
}

UT_Bool FL_DocLayout::populateStrux(PL_StruxDocHandle sdh,
									PX_ChangeRecord * pcr,
									PL_StruxFmtHandle * psfh)
{
	UT_DEBUGMSG(("FL_DocLayout::populateStrux\n"));
	pcr->dump();

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	PX_ChangeRecord_Strux * pcrx = static_cast<PX_ChangeRecord_Strux *> (pcr);

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			// append a SectionLayout to this DocLayout
			FL_SectionLayout* pSL = new FL_SectionLayout(this, sdh);
			m_vecSectionLayouts.addItem(pSL);

			psfh = (PL_StruxFmtHandle *) &pSL;
		}
		break;

	case PTX_ColumnSet:
		{
			// TODO -- Jeff?
		}
		break;
			
	case PTX_Column:
		{
			// TODO -- Jeff?
		}
		break;
			
	case PTX_Block:
		{
			// locate the last SectionLayout
			int countSections = m_vecSectionLayouts.getItemCount();
			UT_ASSERT(countSections > 0);
			FL_SectionLayout* pSL = (FL_SectionLayout*) m_vecSectionLayouts.getNthItem(countSections - 1);

			// TODO -- append a new BlockLayout to that SectionLayout
		}
		break;
			
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool FL_DocLayout::change(PL_StruxFmtHandle sfh,
							 PX_ChangeRecord * pcr)
{
	UT_DEBUGMSG(("FL_DocLayout::change\n"));
	pcr->dump();

	return UT_TRUE;
}

UT_Bool FL_DocLayout::insertStrux(PL_StruxFmtHandle sfh,
								  PX_ChangeRecord * pcr,
								  PL_StruxDocHandle sdh,
								  PL_StruxFmtHandle * psfh)
{
	UT_DEBUGMSG(("FL_DocLayout::insertStrux\n"));
	pcr->dump();

	return UT_TRUE;
}

