 
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
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_SectionSlice.h"
#include "fp_Column.h"
//#include "dg_ColumnModel.h"
#include "pd_Document.h"

#include "ut_assert.h"

FL_SectionLayout::FL_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh)
{
	UT_ASSERT(pLayout);
	UT_ASSERT(sdh);
	
	m_pLayout = pLayout;
	m_sdh = sdh;
	m_pDoc = pLayout->getDocument();
	m_pLB = NULL;
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

#ifdef COLUMNMODEL
	if (m_sdh)
	{
		DG_ColumnModel * pCM = m_sdh->getColumnModel();

		if (pCM)
			delete pCM;
	}
#endif

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
		within that space, create columns, in accordance with our columnModel.
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

#ifdef COLUMNMODEL
	DG_ColumnModel * pCM = m_sdh->getColumnModel();
	if (!pCM)
		pCM = NULL; // TODO get default model from document
	UT_ASSERT(pCM);

	UT_uint32 kCMI=0;
	UT_sint32 xoff, yoff;
	FP_Column* pCol;

	for (kCMI=0; (pCM->getNewColumn(kCMI,this,iWidthSlice,iHeightSlice, &pCol,&xoff,&yoff)); kCMI++)
	{
		pSlice->addColumn(pCol, xoff, yoff);
		if (m_vecColumns.getItemCount())
		{
			FP_Column* pcLast = (FP_Column*) m_vecColumns.getLastItem();
			pcLast->setNext(pCol);
		}
		m_vecColumns.addItem(pCol);
	}
#endif /* COLUMNMODEL */
	return pSlice->getFirstColumn();
}

int FL_SectionLayout::format()
{
	// TODO -- why recreate this for each format call?
	SimpleLineBreaker* slb = new SimpleLineBreaker();

	// remember this so we can delete it later
	m_pLB = slb;

#ifdef BUFFER	// format
	UT_uint32 iSectionStart = m_pBuffer->getMarkerPosition(m_sdh);
	
	UT_UCSChar data;
	DG_DocMarkerId dmid;
	UT_uint32 pos = iSectionStart;
	FL_BlockLayout* pPrevBlock = NULL;

	while (1)
	{
		switch (m_pBuffer->getOneItem(pos,&data,&dmid))
		{
		case DG_DBPI_END:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;

		case DG_DBPI_DATA:
			break;
			
		case DG_DBPI_MARKER:
		{
			DG_DocMarker* pMarker = m_pBuffer->fetchDocMarker(dmid);
			if ((pMarker->getType() & DG_MT_SECTION)
				&& (pMarker->getType() & DG_MT_END))
			{
				return 0;
			}
			else if ((pMarker->getType() & DG_MT_BLOCK)
					 && !(pMarker->getType() & DG_MT_END))
			{
				FL_BlockLayout*	pBL = new FL_BlockLayout(pMarker, slb, pPrevBlock, this);
				pBL->format();

				pPrevBlock = pBL;
			}
			break;
		}
			
		case DG_DBPI_ERROR:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		m_pBuffer->inc(pos);
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
#endif /* BUFFER */
	return 0;	// TODO return code
}

UT_Bool FL_SectionLayout::reformat()
{
	UT_Bool bResult = UT_FALSE;
#ifdef BUFFER	// reformat
	UT_uint32 iSectionStart = m_pBuffer->getMarkerPosition(m_sdh);
	
	UT_UCSChar data;
	DG_DocMarkerId dmid;
	UT_uint32 pos = iSectionStart;

	while (1)
	{
		switch (m_pBuffer->getOneItem(pos,&data,&dmid))
		{
		case DG_DBPI_END:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;

		case DG_DBPI_DATA:
			break;
			
		case DG_DBPI_MARKER:
		{
			DG_DocMarker* pMarker = m_pBuffer->fetchDocMarker(dmid);
			if ((pMarker->getType() & DG_MT_SECTION)
				&& (pMarker->getType() & DG_MT_END))
			{
				return bResult;
			}
			else if ((pMarker->getType() & DG_MT_BLOCK)
					 && !(pMarker->getType() & DG_MT_END))
			{
				FL_BlockLayout*	pBL = (FL_BlockLayout*) pMarker->getBlock();

				if (pBL->needsReformat())
				{
					pBL->format();
					pBL->draw(m_pLayout->getGraphics());
					bResult = UT_TRUE;
				}
			}
			break;
		}
			
		case DG_DBPI_ERROR:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		m_pBuffer->inc(pos);
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
#endif /* BUFFER */
	return 0;	// TODO return code
}

// TODO -- write an iterator so functions like this aren't so gross
void FL_SectionLayout::_purgeLayout()
{
#ifdef BUFFER	// _purgeLayout
	UT_uint32 iSectionStart = m_pBuffer->getMarkerPosition(m_sdh);
	
	UT_UCSChar data;
	DG_DocMarkerId dmid;
	UT_uint32 pos = iSectionStart;

	while (1)
	{
		switch (m_pBuffer->getOneItem(pos,&data,&dmid))
		{
		case DG_DBPI_END:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;

		case DG_DBPI_DATA:
			break;
			
		case DG_DBPI_MARKER:
		{
			DG_DocMarker* pMarker = m_pBuffer->fetchDocMarker(dmid);
			if ((pMarker->getType() & DG_MT_SECTION)
				&& (pMarker->getType() & DG_MT_END))
			{
				return;
			}
			else if ((pMarker->getType() & DG_MT_BLOCK)
					 && !(pMarker->getType() & DG_MT_END))
			{
				FL_BlockLayout*	pBL = (FL_BlockLayout*) pMarker->getBlock();

				delete pBL;
			}
			break;
		}
			
		case DG_DBPI_ERROR:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
		m_pBuffer->inc(pos);
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
#endif /* BUFFER */
	return;
}

