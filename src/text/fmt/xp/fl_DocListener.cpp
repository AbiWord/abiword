 
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
#include "fl_DocListener.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_ColumnSetLayout.h"
#include "fl_ColumnLayout.h"
#include "fl_BlockLayout.h"
#include "pd_Document.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

fl_DocListener::fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout)
{
	m_pDoc = doc;
	m_pLayout = pLayout;
}

fl_DocListener::~fl_DocListener()
{
}

UT_Bool fl_DocListener::populate(PL_StruxFmtHandle sfh,
							   PX_ChangeRecord * pcr)
{
	UT_ASSERT(m_pLayout);
	UT_DEBUGMSG(("fl_DocListener::populate\n"));
	pcr->dump();

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertSpan);
	PX_ChangeRecord_Span * pcrs = static_cast<PX_ChangeRecord_Span *> (pcr);

	// paul-- a span is a sequence of text with the same formatting.
	// TODO -- span is block-relative, right?
	// HYP: append as a new run to last block of last section
	// ALT: if format same, append to last run of that block
	// ==>: pass enough info to delegate the decision  :-)

	return UT_TRUE;
}

UT_Bool fl_DocListener::populateStrux(PL_StruxDocHandle sdh,
									PX_ChangeRecord * pcr,
									PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(m_pLayout);
	UT_DEBUGMSG(("fl_DocListener::populateStrux\n"));
	pcr->dump();

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	PX_ChangeRecord_Strux * pcrx = static_cast<PX_ChangeRecord_Strux *> (pcr);

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			// append a SectionLayout to this DocLayout
			FL_SectionLayout* pSL = new FL_SectionLayout(m_pLayout, sdh);
			if (!pSL)
			{
				UT_DEBUGMSG(("no memory for SectionLayout"));
				return UT_FALSE;
			}
			pSL->setPTvars(pcr->getVarSetIndex(),pcr->getIndexAP());
			m_pLayout->m_vecSectionLayouts.addItem(pSL);

			psfh = (PL_StruxFmtHandle *)pSL;
		}
		break;

	case PTX_ColumnSet:
		{
			// locate the last SectionLayout
			int countSections = m_pLayout->m_vecSectionLayouts.getItemCount();
			UT_ASSERT(countSections > 0);
			FL_SectionLayout* pSL = (FL_SectionLayout*) m_pLayout->m_vecSectionLayouts.getNthItem(countSections - 1);
			UT_ASSERT(pSL);
			FL_ColumnSetLayout * pCSL = new FL_ColumnSetLayout(pSL,sdh);
			if (!pCSL)
			{
				UT_DEBUGMSG(("no memory for ColumnSetLayout"));
				return UT_FALSE;
			}
			pCSL->setPTvars(pcr->getVarSetIndex(),pcr->getIndexAP());
			UT_ASSERT(pSL->getColumnSetLayout()==NULL);
			pSL->setColumnSetLayout(pCSL);

			psfh = (PL_StruxFmtHandle *)pCSL;
		}
		break;
			
	case PTX_Column:
		{
			// locate the last SectionLayout
			int countSections = m_pLayout->m_vecSectionLayouts.getItemCount();
			UT_ASSERT(countSections > 0);
			FL_SectionLayout* pSL = (FL_SectionLayout*) m_pLayout->m_vecSectionLayouts.getNthItem(countSections - 1);
			UT_ASSERT(pSL);
			FL_ColumnSetLayout * pCSL =	pSL->getColumnSetLayout();
			UT_ASSERT(pCSL);
			FL_ColumnLayout * pCL = new FL_ColumnLayout(pCSL,sdh);
			if (!pCL)
			{
				UT_DEBUGMSG(("no memory for ColumnLayout"));
				return UT_FALSE;
			}
			pCL->setPTvars(pcr->getVarSetIndex(),pcr->getIndexAP());
			pCSL->appendColumnLayout(pCL);

			psfh = (PL_StruxFmtHandle *)pCL;
		}
		break;
			
	case PTX_Block:
		{
			// locate the last SectionLayout
			int countSections = m_pLayout->m_vecSectionLayouts.getItemCount();
			UT_ASSERT(countSections > 0);
			FL_SectionLayout* pSL = (FL_SectionLayout*) m_pLayout->m_vecSectionLayouts.getNthItem(countSections - 1);
			UT_ASSERT(pSL);

			// append a new BlockLayout to that SectionLayout
			FL_BlockLayout*	pBL = pSL->appendBlock(sdh);
			if (!pBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return UT_FALSE;
			}
			pBL->setPTvars(pcr->getVarSetIndex(),pcr->getIndexAP());

			psfh = (PL_StruxFmtHandle *)pBL;
		}
		break;
			
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool fl_DocListener::change(PL_StruxFmtHandle sfh,
							 PX_ChangeRecord * pcr)
{
	UT_DEBUGMSG(("fl_DocListener::change\n"));
	pcr->dump();

	return UT_TRUE;
}

UT_Bool fl_DocListener::insertStrux(PL_StruxFmtHandle sfh,
								  PX_ChangeRecord * pcr,
								  PL_StruxDocHandle sdh,
								  PL_StruxFmtHandle * psfh)
{
	UT_DEBUGMSG(("fl_DocListener::insertStrux\n"));
	pcr->dump();

	return UT_TRUE;
}

