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

#include <string.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_string.h"

#include "ap_Prefs.h"
#include "fl_SectionLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "gr_Graphics.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "px_CR_Glob.h"
#include "fv_View.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

/*
  TODO this file is now really too long.  divide it up
  into smaller ones.
*/

fl_SectionLayout::fl_SectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, SectionType iType)
	: fl_Layout(PTX_Section, sdh)
{
	UT_ASSERT(pLayout);

	m_iType = iType;
	
	m_pLayout = pLayout;
	m_pDoc = pLayout->getDocument();
	m_pLB = NULL;
	m_pFirstBlock = NULL;
	m_pLastBlock = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pHdrFtrSL = NULL;
	setAttrPropIndex(indexAP);
}

fl_SectionLayout::~fl_SectionLayout()
{
	if (m_pLB)
	{
		delete m_pLB;
	}
}

const char*	fl_SectionLayout::getAttribute(const char * pszName) const
{
	const PP_AttrProp * pAP = NULL;
	getAttrProp(&pAP);

	const XML_Char* pszAtt = NULL;
	pAP->getAttribute((XML_Char*)pszName, pszAtt);
	
	return pszAtt;
}

void fl_SectionLayout::setNext(fl_SectionLayout* pSL)
{
	m_pNext = pSL;
}

void fl_SectionLayout::setPrev(fl_SectionLayout* pSL)
{
	m_pPrev = pSL;
}

FL_DocLayout* fl_SectionLayout::getDocLayout(void) const
{
	return m_pLayout;
}

fl_BlockLayout * fl_SectionLayout::getFirstBlock(void) const
{
	return m_pFirstBlock;
}

fl_BlockLayout * fl_SectionLayout::getLastBlock(void) const
{
	return m_pLastBlock;
}

bool fl_SectionLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	bool bResult = false;
	
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		bResult = pBL->recalculateFields(iUpdateCount) || bResult;

		pBL = pBL->getNext();
	}

	return bResult;
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
	pBL->setSectionLayout(this);
}

fl_BlockLayout * fl_SectionLayout::insertBlock(PL_StruxDocHandle sdh, fl_BlockLayout * pPrev, PT_AttrPropIndex indexAP)
{
	fl_BlockLayout* pBL=NULL;
	if(getType() ==  FL_SECTION_HDRFTR) 
		pBL = new fl_BlockLayout(sdh, _getLineBreaker(), pPrev, this, indexAP,true);
	else
		pBL = new fl_BlockLayout(sdh, _getLineBreaker(), pPrev, this, indexAP);

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
		pBL->transferListFlags();
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
	pBL->setSectionLayout(NULL);
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

void fl_SectionLayout::markAllRunsDirty(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->markAllRunsDirty();
		pBL = pBL->getNext();
	}
}

void fl_SectionLayout::_purgeLayout()
{
	fl_BlockLayout*	pBL = m_pLastBlock;

	while (pBL)
	{
		fl_BlockLayout* pNuke = pBL;

		pBL = pBL->getPrev();

		delete pNuke;
	}

	return;
}

bool fl_SectionLayout::bl_doclistener_populateSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
	if(pBL->getPrev()!= NULL && pBL->getPrev()->getLastLine()==NULL)
	{
		UT_DEBUGMSG(("In bl_doclistner_pop no LastLine \n"));
		UT_DEBUGMSG(("getPrev = %d this = %d \n",pBL->getPrev(),pBL));
		//  UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	return pBL->doclistener_populateSpan(pcrs, blockOffset, len);
}

bool fl_SectionLayout::bl_doclistener_populateObject(fl_BlockLayout* pBL, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
	return pBL->doclistener_populateObject(blockOffset, pcro);
}
	
bool fl_SectionLayout::bl_doclistener_insertSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
	return pBL->doclistener_insertSpan(pcrs);
}

bool fl_SectionLayout::bl_doclistener_deleteSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
	return pBL->doclistener_deleteSpan(pcrs);
}

bool fl_SectionLayout::bl_doclistener_changeSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_SpanChange * pcrsc)
{
	return pBL->doclistener_changeSpan(pcrsc);
}

bool fl_SectionLayout::bl_doclistener_deleteStrux(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx)
{
	return pBL->doclistener_deleteStrux(pcrx);
}

bool fl_SectionLayout::bl_doclistener_changeStrux(fl_BlockLayout* pBL, const PX_ChangeRecord_StruxChange * pcrxc)
{
	return pBL->doclistener_changeStrux(pcrxc);
}

bool fl_SectionLayout::bl_doclistener_insertBlock(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
												  PL_StruxDocHandle sdh,
												  PL_ListenerId lid,
												  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																		  PL_ListenerId lid,
																		  PL_StruxFmtHandle sfhNew))
{
	if (pBL)
	{
		return pBL->doclistener_insertBlock(pcrx, sdh, lid, pfnBindHandles);
	}
	else
	{
		// Insert the block at the beginning of the section
		fl_BlockLayout*	pNewBL = insertBlock(sdh, NULL, pcrx->getIndexAP());
		if (!pNewBL)
		{
			UT_DEBUGMSG(("no memory for BlockLayout\n"));
			return false;
		}

		return pNewBL->doclistener_insertFirstBlock(pcrx, sdh, 
													lid, pfnBindHandles);
	}
}

bool fl_SectionLayout::bl_doclistener_insertSection(fl_BlockLayout* pBL, 
													SectionType iType,
													const PX_ChangeRecord_Strux * pcrx,
													PL_StruxDocHandle sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																			PL_ListenerId lid,
																			PL_StruxFmtHandle sfhNew))
{
	return pBL->doclistener_insertSection(pcrx, iType, sdh, lid, pfnBindHandles);
}

bool fl_SectionLayout::bl_doclistener_insertObject(fl_BlockLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
	return pBL->doclistener_insertObject(pcro);
}

bool fl_SectionLayout::bl_doclistener_deleteObject(fl_BlockLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
	return pBL->doclistener_deleteObject(pcro);
}

bool fl_SectionLayout::bl_doclistener_changeObject(fl_BlockLayout* pBL, const PX_ChangeRecord_ObjectChange * pcroc)
{
	return pBL->doclistener_changeObject(pcroc);
}

bool fl_SectionLayout::bl_doclistener_insertFmtMark(fl_BlockLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
	return pBL->doclistener_insertFmtMark(pcrfm);
}

bool fl_SectionLayout::bl_doclistener_deleteFmtMark(fl_BlockLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
	return pBL->doclistener_deleteFmtMark(pcrfm);
}

bool fl_SectionLayout::bl_doclistener_changeFmtMark(fl_BlockLayout* pBL, const PX_ChangeRecord_FmtMarkChange * pcrfmc)
{
	return pBL->doclistener_changeFmtMark(pcrfmc);
}

/*!
 * This method updates the Background color in all the runs from the Page color
 * in the DocSectionLayout.
 */
void fl_SectionLayout::updateBackgroundColor(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->updateBackgroundColor();
		pBL = pBL->getNext();
	}
	if(getType() != FL_SECTION_DOC)
		return;
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(this);	
	UT_Vector vecHdrFtr;
	pDSL->getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		pHdrFtr->updateBackgroundColor();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_DocSectionLayout::fl_DocSectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, SectionType iType)
	: fl_SectionLayout(pLayout, sdh, indexAP, iType)
{
	UT_ASSERT((iType == FL_SECTION_DOC || iType == FL_SECTION_ENDNOTE));

	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

	m_pHeaderSL = NULL;
	m_pFooterSL = NULL;
	m_pHeaderEvenSL = NULL;
	m_pFooterEvenSL = NULL;
	m_pHeaderFirstSL = NULL;
	m_pFooterFirstSL = NULL;
	m_pHeaderLastSL = NULL;
	m_pFooterLastSL = NULL;
	m_pEndnoteSL = NULL;
	m_pEndnoteOwnerSL = NULL;
	m_pFirstOwnedPage = NULL;
	m_bNeedsFormat = false;
	m_bNeedsRebuild = false;
	m_bNeedsSectionBreak = true;
	_lookupProperties();
}

fl_DocSectionLayout::~fl_DocSectionLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();

	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		delete pHdrFtr;
	}

	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		fp_Column* pNext = pCol->getNext();

		delete pCol;

		pCol = pNext;
	}
}

void fl_DocSectionLayout::setHdrFtr(HdrFtrType iType, fl_HdrFtrSectionLayout* pHFSL)
{
	if(pHFSL == NULL)
	{
		if(iType == FL_HDRFTR_HEADER)
		{
			m_pHeaderSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_HEADER_EVEN)
		{
			m_pHeaderEvenSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_HEADER_FIRST)
		{
			m_pHeaderFirstSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_HEADER_LAST)
		{
			m_pHeaderLastSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_FOOTER)
		{
			m_pFooterSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_FOOTER_EVEN)
		{
			m_pFooterEvenSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_FOOTER_FIRST)
		{
			m_pFooterFirstSL = pHFSL;
		}
		else if(iType == FL_HDRFTR_FOOTER_LAST)
		{
			m_pFooterLastSL = pHFSL;
		}	
		checkAndRemovePages();
	return;
	}
	const char* pszID = pHFSL->getAttribute("id");

	const char* pszAtt = NULL;

	pszAtt = getAttribute("header");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID)) )
	{
		m_pHeaderSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("header-even");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID)) )
	{
		m_pHeaderEvenSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("header-first");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID)) )
	{
		m_pHeaderFirstSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("header-last");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID)) )
	{
		m_pHeaderLastSL = pHFSL;
		checkAndRemovePages();
		return;
	}
	
	pszAtt = getAttribute("footer");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID))	)
	{
		m_pFooterSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer-even");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID))	)
	{
		m_pFooterEvenSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer-first");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID))	)
	{
		m_pFooterFirstSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	pszAtt = getAttribute("footer-last");
	if (pszAtt && (0 == UT_stricmp(pszAtt, pszID))	)
	{
		m_pFooterLastSL = pHFSL;
		checkAndRemovePages();
		return;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeader(void)
{
	return m_pHeaderSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooter(void)
{
	return m_pFooterSL;
}


fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeaderEven(void)
{
	return m_pHeaderEvenSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooterEven(void)
{
	return m_pFooterEvenSL;
}


fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeaderFirst(void)
{
	return m_pHeaderFirstSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooterFirst(void)
{
	return m_pFooterFirstSL;
}


fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getHeaderLast(void)
{
	return m_pHeaderLastSL;
}

fl_HdrFtrSectionLayout*   fl_DocSectionLayout::getFooterLast(void)
{
	return m_pFooterLastSL;
}

void fl_DocSectionLayout::setEndnote(fl_DocSectionLayout* pEndnoteSL)
{
	UT_ASSERT(getType() == FL_SECTION_DOC);
	m_pEndnoteSL = pEndnoteSL;
}

fl_DocSectionLayout* fl_DocSectionLayout::getEndnote(void)
{
	UT_ASSERT(getType() == FL_SECTION_DOC);
	return m_pEndnoteSL;
}

void fl_DocSectionLayout::setEndnoteOwner(fl_DocSectionLayout* pDSL)
{
	UT_ASSERT(getType() == FL_SECTION_ENDNOTE);
	m_pEndnoteOwnerSL = pDSL;
}

fl_DocSectionLayout* fl_DocSectionLayout::getEndnoteOwner(void)
{
	UT_ASSERT(getType() == FL_SECTION_ENDNOTE);
	return m_pEndnoteOwnerSL;
}

fp_Container* fl_DocSectionLayout::getFirstContainer()
{
	return m_pFirstColumn;
}

fp_Container* fl_DocSectionLayout::getLastContainer()
{
	return m_pLastColumn;
}

/*!
  Create new container
  \return The newly created container

  This creates a new column or row of same.

*/
fp_Container* fl_DocSectionLayout::getNewContainer(fp_Line * pFirstLine)
{
	fp_Page* pPage = NULL;
	fp_Column* pLastColumn = (fp_Column*) getLastContainer();
	fp_Column* pAfterColumn = NULL;

	if (pLastColumn)
	{
		fp_Line * prevLine = NULL;
		fp_Page* pTmpPage = NULL;
		UT_sint32 nextLine = 0;
		UT_sint32 pageHeight = 0;
		pTmpPage = pLastColumn->getPage();
		nextLine = 0;
		if(pFirstLine != NULL)
		{
			prevLine = pFirstLine->getPrevLineInSection();
		}
//
// Calculate from the page height up to prevLine
//
		pageHeight = pTmpPage->getFilledHeightInLayoutUnits(prevLine);
		if(pFirstLine != NULL)
		{
			nextLine = pFirstLine->getHeightInLayoutUnits();
		}
		else if( pLastColumn->getLastLine())
		{
			nextLine = pLastColumn->getLastLine()->getHeightInLayoutUnits();
		}
		else
		{
			nextLine =12*14; // approximately one average line
		}
		UT_sint32 avail =  pTmpPage->getAvailableHeightInLayoutUnits();
		UT_sint32 newHeight = pageHeight+ 3*nextLine;
		xxx_UT_DEBUGMSG(("SEVIOR: Pageheight =%d nextlineheight =%d newheight = %d availableheight =%d linepos %d \n",pageHeight,nextLine,newHeight,avail));
		if( newHeight  >= avail || pFirstLine == NULL)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Container on new page \n"));
			if (pTmpPage->getNext())
			{
				pPage = pTmpPage->getNext();
			}
			else
			{
				pPage = m_pLayout->addNewPage(this);
			}
		}
		else
		{
			xxx_UT_DEBUGMSG(("SEVIOR: Container on current page \n"));
			pPage = pTmpPage;
			if(prevLine == NULL)
			{
				pAfterColumn = pPage->getNthColumnLeader(pPage->countColumnLeaders()-1);
			}
			else
			{
				pAfterColumn = static_cast<fp_Column *>(prevLine->getContainer())->getLeader();
			}

		}
	}
	else
	{
		// We currently have no columns in this section.  Time to
		// create some.  If there is a previous section, then we need
		// to start our section right after that one.  If not, then we
		// start our section on the first page.  If there is no first
		// page, then we need to create one.
		fl_DocSectionLayout* pPrevSL = getPrevDocSection();
		if (pPrevSL)
		{
//
// This should make sure the last column in the previous section  has it's last container
// on a sane page.
//
			//
			// Sevior this code should not be needed!
			//
			fp_Column * pPrevCol = (fp_Column *) pPrevSL->getLastContainer();
			while(pPrevCol == NULL)
			{
				UT_DEBUGMSG(("BUG! BUG! Prev section has no last container! Attempting to fix this \n"));
				pPrevSL->format();
			    pPrevCol = (fp_Column *) pPrevSL->getLastContainer();
			}
			fp_Page* pTmpPage = pPrevSL->getLastContainer()->getPage();
			fp_Line * prevLine = NULL;
			UT_sint32 nextLine = 0;
			if(pFirstLine != NULL)
			{
				prevLine = pFirstLine->getPrevLineInSection();
			}
			UT_sint32 pageHeight = pTmpPage->getFilledHeightInLayoutUnits(prevLine);
			if(pFirstLine != NULL)
			{
				nextLine = pFirstLine->getHeightInLayoutUnits();
			}
			else if(pPrevCol->getLastLine())
			{
				nextLine = pPrevCol->getLastLine()->getHeightInLayoutUnits();
			}
			else
			{
				nextLine = 12*14; //average height!
			}
			bool bForce = (pageHeight + 2*nextLine) >= pTmpPage->getAvailableHeightInLayoutUnits();
			if (m_bForceNewPage || bForce)
			{
				if (pTmpPage->getNext())
				{
					pPage = pTmpPage->getNext();
				}
				else
				{
					pPage = m_pLayout->addNewPage(this);
				}
			}
			else
			{
				pPage = pTmpPage;
#if 0 // This fixes bug 966 but introduces new problems - jskov 2001.06.10
				pAfterColumn = pPrevCol;
#else
				if(prevLine == NULL)
				{
					pAfterColumn = pPage->getNthColumnLeader(pPage->countColumnLeaders()-1);
				}
				else
				{
					pAfterColumn = static_cast<fp_Column *>(prevLine->getContainer())->getLeader();
				}
#endif
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
				pPage = m_pLayout->addNewPage(this);
			}
		}
	}

	UT_ASSERT(pPage);

	// Create row of columns
	fp_Column* pLeaderColumn = NULL;
	fp_Column* pTail = NULL;
	UT_uint32 i = 0;
	for (i=0; i<m_iNumColumns; i++)
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

	// Append added columns to any previous columns in this section.
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
	
	// Find last added column and set that as the last in the section.
	fp_Column* pLastNewCol = pLeaderColumn;
	while (pLastNewCol->getFollower())
	{
		pLastNewCol = pLastNewCol->getFollower();
	}
	m_pLastColumn = pLastNewCol;
	UT_ASSERT(!(m_pLastColumn->getNext()));
	UT_ASSERT(!(m_pLastColumn->getFollower()));

	pPage->insertColumnLeader(pLeaderColumn, pAfterColumn);
	
	fp_Column* pTmpCol = pLeaderColumn;
	i = 0;
 	while (pTmpCol)
	{
		UT_ASSERT(pTmpCol->getPage());
		
		pTmpCol = pTmpCol->getFollower();
		i++;
	}

	return pLeaderColumn;
}

void fl_DocSectionLayout::format(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->format();
		UT_sint32 count = 0;
		while(pBL->getLastLine() == NULL || pBL->getFirstLine()==NULL)
		{
			UT_DEBUGMSG(("Error formatting a block try again \n"));
			count = count + 1;
			pBL->format();
			if(count > 3)
			{
				UT_DEBUGMSG(("Give up trying to format. Hope for the best :-( \n"));
				break;
			}
		}
		pBL = pBL->getNext();
	}

	breakSection();
	m_bNeedsFormat = false;
}

void fl_DocSectionLayout::markAllRunsDirty(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->markAllRunsDirty();
		pBL = pBL->getNext();
	}
	if(m_pHeaderSL)
	{
		m_pHeaderSL->markAllRunsDirty();
	}
	if(m_pHeaderEvenSL)
	{
		m_pHeaderEvenSL->markAllRunsDirty();
	}
	if(m_pHeaderFirstSL)
	{
		m_pHeaderFirstSL->markAllRunsDirty();
	}
	if(m_pHeaderLastSL)
	{
		m_pHeaderLastSL->markAllRunsDirty();
	}
	if(m_pFooterSL)
	{
		m_pFooterSL->markAllRunsDirty();
	}
	if(m_pFooterEvenSL)
	{
		m_pFooterEvenSL->markAllRunsDirty();
	}
	if(m_pFooterFirstSL)
	{
		m_pFooterFirstSL->markAllRunsDirty();
	}
	if(m_pFooterLastSL)
	{
		m_pFooterLastSL->markAllRunsDirty();
	}
}

void fl_DocSectionLayout::updateLayout(void)
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
	
	if(needsSectionBreak())
	{
	breakSection();
	}
	
	if(!needsRebuild())
	{
		checkAndRemovePages();
		addValidPages();
	}
}

void fl_DocSectionLayout::redrawUpdate(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		if(pBL->hasUpdatableField())
		{
			bool bReformat = pBL->recalculateFields(getDocLayout()->getRedrawCount());
			if(bReformat)
			{
				pBL->format();
			}
		}
		if (pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}
		
		pBL = pBL->getNext();
	}
	
	if(needsSectionBreak() || needsRebuild())
	{
	    breakSection();
	    if(!needsRebuild())
	    {
		checkAndRemovePages();
		addValidPages();
	    }
	}
}

bool fl_DocSectionLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	return true;
}

void fl_DocSectionLayout::updateDocSection(void)
{

	const PP_AttrProp* pAP = NULL;
	bool bres = m_pDoc->getAttrProp(m_apIndex, &pAP);
	UT_ASSERT(bres);
	

	const XML_Char* pszSectionType = NULL;
	pAP->getAttribute("type", pszSectionType);

	_lookupProperties();

	// clear all the columns
    // Assume that all columns and formatting have already been removed via a collapseDocSection()
    //

	/*
	  TODO to more closely mirror the architecture we're using for BlockLayout, this code
	  should probably just set a flag, indicating the need to reformat this section.  Then,
	  when it's time to update everything, we'll actually do the format.
	*/

	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(false);
	}

	format();
	updateBackgroundColor();
	checkAndRemovePages();
	formatAllHdrFtr();
	if (m_pEndnoteSL)
	{
		m_pEndnoteSL->format();
	}
	markAllRunsDirty();

	if(pView)
	{
		pView->setScreenUpdateOnGeneralUpdate(true);
	}

//	if (pView)
//	{
//		pView->eraseInsertionPoint();
//		pView->updateScreen(false);
//		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTSECTION);
//	}

	return;
}

void fl_DocSectionLayout::_lookupProperties(void)
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
	pSectionAP->getProperty("columns", (const XML_Char *&)pszNumColumns);
	if (pszNumColumns && pszNumColumns[0])
	{
		m_iNumColumns = atoi(pszNumColumns);
	}
	else
	{
		m_iNumColumns = 1;
	}

	const char* pszColumnGap = NULL;
	pSectionAP->getProperty("column-gap", (const XML_Char *&)pszColumnGap);
	if (pszColumnGap && pszColumnGap[0])
	{
		m_iColumnGap = m_pLayout->getGraphics()->convertDimension(pszColumnGap);
		m_iColumnGapLayoutUnits = UT_convertToLayoutUnits(pszColumnGap);
	}
	else
	{
		m_iColumnGap = m_pLayout->getGraphics()->convertDimension("0.25in");
		m_iColumnGapLayoutUnits = UT_convertToLayoutUnits("0.25in");
	}

	const char* pszColumnLineBetween = NULL;
	pSectionAP->getProperty("column-line", (const XML_Char *&)pszColumnLineBetween);
	if (pszColumnLineBetween && pszColumnLineBetween[0])
	{
		m_bColumnLineBetween = (strcmp(pszColumnLineBetween, "on") == 0) ? true : false;
	}
	else
	{
		m_bColumnLineBetween = false;
	}

#ifdef BIDI_ENABLED	  /* column-order */
	//we use the mechanism used by BlockLayout, since otherwise we
	//cannot recode the default value
	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	
	const char * pszColumnOrder = PP_evalProperty("dom-dir",pSpanAP,pBlockAP,pSectionAP,m_pDoc,false);
    UT_DEBUGMSG(("column order: %s\n", pszColumnOrder));
	if (pszColumnOrder && pszColumnOrder[0])
	{
		m_iColumnOrder = strcmp(pszColumnOrder, "ltr")	? 1 : 0;
	}
	else
	{
		m_iColumnOrder = 0;
	}
#endif

	const char* pszSpaceAfter = NULL;
	pSectionAP->getProperty("section-space-after", (const XML_Char *&)pszSpaceAfter);
	if (pszSpaceAfter && pszSpaceAfter[0])
	{
		m_iSpaceAfter = m_pLayout->getGraphics()->convertDimension(pszSpaceAfter);
		m_iSpaceAfterLayoutUnits = UT_convertToLayoutUnits(pszSpaceAfter);
	}
	else
	{
		m_iSpaceAfter = m_pLayout->getGraphics()->convertDimension("0in");
		m_iSpaceAfterLayoutUnits = UT_convertToLayoutUnits("0in");
	}

	const char* pszRestart = NULL;
	pSectionAP->getProperty("section-restart", (const XML_Char *&)pszRestart);
	if (pszRestart && pszRestart[0])
	{
		m_bRestart = (UT_strcmp(pszRestart,"1")==0);
	}
	else
	{
		m_bRestart = false;
	}

	const char* pszRestartValue = NULL;
	pSectionAP->getProperty("section-restart-value", (const XML_Char *&)pszRestartValue);
	if (pszRestartValue && pszRestartValue[0])
	{
		m_iRestartValue = atoi(pszRestartValue);
	}
	else
	{
		m_iRestartValue = 1;
	}

	const char* pszLeftMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszBottomMargin = NULL;
	const char* pszFooterMargin = NULL;
	const char* pszHeaderMargin = NULL;
	const char* pszMaxColumnHeight = NULL;
	pSectionAP->getProperty("page-margin-left", (const XML_Char *&)pszLeftMargin);
	pSectionAP->getProperty("page-margin-top", (const XML_Char *&)pszTopMargin);
	pSectionAP->getProperty("page-margin-right", (const XML_Char *&)pszRightMargin);
	pSectionAP->getProperty("page-margin-bottom", (const XML_Char *&)pszBottomMargin);
	pSectionAP->getProperty("page-margin-footer", (const XML_Char *&)pszFooterMargin);
	pSectionAP->getProperty("page-margin-header", (const XML_Char *&)pszHeaderMargin);


	const XML_Char * szRulerUnits;
	UT_Dimension dim;
	if (XAP_App::getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
		dim = UT_determineDimension(szRulerUnits);
	else
		dim = DIM_IN;

	UT_String defaultMargin;
	switch(dim)
	{
	case DIM_IN:
		defaultMargin = "1.0in";
		break;

	case DIM_CM:
		defaultMargin = "2.54cm";
		break;

	case DIM_PI:
		defaultMargin = "6.0pi";
		break;

	case DIM_PT:
		defaultMargin= "72.0pt";
		break;

	case DIM_MM:
		defaultMargin= "25.4mm";
		break;

		// TODO: PX, and PERCENT
		// let them fall through to the default now
		// and we don't use them anyway
#if 0
	case DIM_PX:
	case DIM_PERCENT:
#endif
	case DIM_none:
	default:
		defaultMargin = "1.0in";	// TODO: what to do with this.
		break;

	}

	if(pszLeftMargin && pszLeftMargin[0])
	{
		m_iLeftMargin = m_pLayout->getGraphics()->convertDimension(pszLeftMargin);
		m_iLeftMarginLayoutUnits = UT_convertToLayoutUnits(pszLeftMargin);
		m_dLeftMarginUserUnits = UT_convertDimensionless(pszLeftMargin);
	}
	else
	{
		m_iLeftMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin.c_str());
		m_iLeftMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin.c_str());
		m_dLeftMarginUserUnits = UT_convertDimensionless(defaultMargin.c_str());
	}
	
	if(pszTopMargin && pszTopMargin[0])
	{
		m_iTopMargin = m_pLayout->getGraphics()->convertDimension(pszTopMargin);
		m_iTopMarginLayoutUnits = UT_convertToLayoutUnits(pszTopMargin);
		m_dTopMarginUserUnits = UT_convertDimensionless(pszTopMargin);
	}
	else
	{
		m_iTopMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin.c_str());
		m_iTopMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin.c_str());
		m_dTopMarginUserUnits = UT_convertDimensionless(defaultMargin.c_str());
	}

	if(pszRightMargin && pszRightMargin[0])
	{
		m_iRightMargin = m_pLayout->getGraphics()->convertDimension(pszRightMargin);
		m_iRightMarginLayoutUnits = UT_convertToLayoutUnits(pszRightMargin);
		m_dRightMarginUserUnits = UT_convertDimensionless(pszRightMargin);
	}
	else
	{
		m_iRightMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin.c_str());
		m_iRightMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin.c_str());
		m_dRightMarginUserUnits = UT_convertDimensionless(defaultMargin.c_str());
	}

	if(pszBottomMargin && pszBottomMargin[0])
	{
		m_iBottomMargin = m_pLayout->getGraphics()->convertDimension(pszBottomMargin);
		m_iBottomMarginLayoutUnits = UT_convertToLayoutUnits(pszBottomMargin);
		m_dBottomMarginUserUnits = UT_convertDimensionless(pszBottomMargin);
	}
	else
	{
		m_iBottomMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin.c_str());
		m_iBottomMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin.c_str());
		m_dBottomMarginUserUnits = UT_convertDimensionless(defaultMargin.c_str());
	}

	if(pszFooterMargin && pszFooterMargin[0])
	{
		m_iFooterMargin = m_pLayout->getGraphics()->convertDimension(pszFooterMargin);
		m_iFooterMarginLayoutUnits = UT_convertToLayoutUnits(pszFooterMargin);
		m_dFooterMarginUserUnits = UT_convertDimensionless(pszFooterMargin);
	}
	else
	{
		m_iFooterMargin = m_pLayout->getGraphics()->convertDimension("0.0in");
		m_iFooterMarginLayoutUnits = UT_convertToLayoutUnits("0.0in");
		m_dFooterMarginUserUnits = UT_convertDimensionless("0.0in");
	}

	if(pszHeaderMargin && pszHeaderMargin[0])
	{
		m_iHeaderMargin = m_pLayout->getGraphics()->convertDimension(pszHeaderMargin);
		m_iHeaderMarginLayoutUnits = UT_convertToLayoutUnits(pszHeaderMargin);
		m_dHeaderMarginUserUnits = UT_convertDimensionless(pszHeaderMargin);
	}
	else
	{
		m_iHeaderMargin = m_pLayout->getGraphics()->convertDimension("0.0in");
		m_iHeaderMarginLayoutUnits = UT_convertToLayoutUnits("0.0in");
		m_dHeaderMarginUserUnits = UT_convertDimensionless("0.0in");
	}

	pSectionAP->getProperty("section-max-column-height", (const XML_Char *&)pszMaxColumnHeight);
	if (pszMaxColumnHeight && pszMaxColumnHeight[0])
	{
		m_iMaxSectionColumnHeight = m_pLayout->getGraphics()->convertDimension(pszMaxColumnHeight);
		m_iMaxSectionColumnHeightInLayoutUnits = UT_convertToLayoutUnits(pszMaxColumnHeight);
	}
	else
	{
		m_iMaxSectionColumnHeight = m_pLayout->getGraphics()->convertDimension("0in");
		m_iMaxSectionColumnHeightInLayoutUnits = UT_convertToLayoutUnits("0in");
	}

	setPaperColor();
	m_bForceNewPage = false;
}

UT_sint32 fl_DocSectionLayout::getTopMargin(void) const 
{ 
	return m_iTopMargin; 
}

UT_sint32 fl_DocSectionLayout::getTopMarginInLayoutUnits(void) const 
{	
	return m_iTopMarginLayoutUnits; 
}

UT_sint32 fl_DocSectionLayout::getBottomMargin(void) const 
{ 	
	return m_iBottomMargin; 
}

UT_sint32 fl_DocSectionLayout::getBottomMarginInLayoutUnits(void) const 
{
	return m_iBottomMarginLayoutUnits; 
}


/*!
 * Set the color of the background paper in the following order of precedence
 * 1. If The section level proper "background-color" is present and is
 *    not transparent use that.
 * 2. If this section is being displayed to the screen use the
 *     ColorForTransparency preference item color.
 * 3. Otherwise use white
 */
void fl_DocSectionLayout::setPaperColor(void)
{
	const PP_AttrProp* pSectionAP = NULL;
	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);

	const char* pszClrPaper = NULL;
	pSectionAP->getProperty("background-color", (const XML_Char *&)pszClrPaper);
	FV_View * pView = m_pLayout->getView();
	if(pszClrPaper && UT_strcmp(pszClrPaper,"transparent") != 0)
		UT_parseColor(pszClrPaper,m_clrPaper);
	else if( pView ) /* && pView->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN) ) */
	{
		XAP_App * pApp = pView->getApp();
		XAP_Prefs * pPrefs = pApp->getPrefs();
		const XML_Char * pszTransparentColor = NULL;
		pPrefs->getPrefsValue((const XML_Char * ) XAP_PREF_KEY_ColorForTransparent,&pszTransparentColor);
		UT_parseColor(pszTransparentColor,m_clrPaper);
	}
	else
	{
		UT_parseColor("ffffff",m_clrPaper);
	}
}

/*!
 * Return a pointer the current background color.
 */
UT_RGBColor * fl_DocSectionLayout::getPaperColor(void) 
{
	return &m_clrPaper;
}

/*! 
 * Delete Empty Column containers in this section.
 */
void fl_DocSectionLayout::deleteEmptyColumns(void)
{
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			fp_Column* pCol2 = pCol;
			bool bAllEmpty = true;
			fp_Column* pLastInGroup = NULL;
			
			while (pCol2)
			{
				if (!pCol2->isEmpty())
				{
					bAllEmpty = false;
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
				if(pCol->getPage() != NULL)
				{
					pCol->getPage()->removeColumnLeader(pCol);
				}
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

UT_uint32 fl_DocSectionLayout::getNumColumns(void) const
{
	return m_iNumColumns;
}

UT_uint32 fl_DocSectionLayout::getColumnGap(void) const
{
	return m_iColumnGap;
}

UT_uint32 fl_DocSectionLayout::getColumnGapInLayoutUnits(void) const
{
	return m_iColumnGapLayoutUnits;
}

#ifdef BIDI_ENABLED	
UT_uint32 fl_DocSectionLayout::getColumnOrder(void) const
{
	return m_iColumnOrder;
}
#endif


fl_DocSectionLayout* fl_DocSectionLayout::getNextDocSection(void) const
{
	fl_DocSectionLayout * pSL = (fl_DocSectionLayout *) getNext();
	if(pSL != NULL && pSL->getType()== FL_SECTION_DOC)
		return pSL;
	return NULL;
}

fl_DocSectionLayout* fl_DocSectionLayout::getPrevDocSection(void) const
{
	fl_DocSectionLayout * pSL = (fl_DocSectionLayout *) getPrev();
	while(pSL != NULL && pSL->getType()!= FL_SECTION_DOC)
	{
		pSL = (fl_DocSectionLayout *) pSL->getPrev();
	}
	return pSL;
}

void fl_DocSectionLayout::collapseDocSection(void)
{
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		pCol->clearScreen();

		pCol = pCol->getNext();
	}
	//
	// Clear the header/footers too
	//
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	fl_HdrFtrSectionLayout * pHdrFtr = NULL;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		pHdrFtr->clearScreen();
	}
	//
	// Collapse the header/footers now
	//
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		pHdrFtr->collapse();
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

//
// Remove all the empty pages thus created. Don't notify of the deletion though.
//
	getDocLayout()->deleteEmptyPages(true);
//
// This Doc Section No longer owns pages so this becomes NULL
//
	m_pFirstOwnedPage = NULL;
}

bool fl_DocSectionLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Section || pcrx->getStruxType()==PTX_SectionEndnote);

	fl_DocSectionLayout* pPrevSL = getPrevDocSection();
	if (!pPrevSL)
	{
		// TODO shouldn't this just assert?
		UT_DEBUGMSG(("no prior SectionLayout"));
		return false;
	}
	
	if(m_pEndnoteOwnerSL)
	{
		m_pEndnoteOwnerSL->setEndnote(0);
	}

//
// Collapse previous section too. We need this so it can be rebuilt properly.
//
	pPrevSL->collapseDocSection();

	// clear all the columns
	collapseDocSection();

	if(m_pHeaderSL)
	{
		DELETEP(m_pHeaderSL);
		m_pHeaderSL = NULL;
	}
	if(m_pHeaderEvenSL)
	{
		DELETEP(m_pHeaderEvenSL);
		m_pHeaderEvenSL = NULL;
	}
	if(m_pHeaderFirstSL)
	{
		DELETEP(m_pHeaderFirstSL);
		m_pHeaderFirstSL = NULL;
	}
	if(m_pHeaderLastSL)
	{
		DELETEP(m_pHeaderLastSL);
		m_pHeaderLastSL = NULL;
	}
	if(m_pFooterSL)
	{
		DELETEP(m_pFooterSL);
		m_pFooterSL = NULL;
	}
	if(m_pFooterEvenSL)
	{
		DELETEP(m_pFooterEvenSL);
		m_pFooterEvenSL = NULL;
	}
	if(m_pFooterFirstSL)
	{
		DELETEP(m_pFooterFirstSL);
		m_pFooterFirstSL = NULL;
	}
	if(m_pFooterLastSL)
	{
		DELETEP(m_pFooterLastSL);
		m_pFooterLastSL = NULL;
	}

//
// Collapse the subsequent sections too. These will be reformatted in a few lines.
//
	fl_DocSectionLayout * pDSL = getNextDocSection();
	while(pDSL != NULL)
	{
		pDSL->collapseDocSection();
		pDSL = pDSL->getNextDocSection();
	}
//
// OK set the links and move all blocks in this section into the previous section.
//
	fl_BlockLayout * pBCur = getFirstBlock();
	fl_BlockLayout * pBPrev = pPrevSL->getLastBlock();
	UT_ASSERT(pBCur && pBPrev);
	
	pBCur->setPrev(pBPrev);
	pBPrev->setNext(pBCur);
	while(pBCur != NULL)
	{
		pBCur->setSectionLayout(pPrevSL);
		pBCur = pBCur->getNext();
	}
	m_pFirstBlock = NULL;
	m_pLastBlock = NULL;
//
// Get this before we remove this section from the run list!
//
    pDSL = getNextDocSection();
	m_pLayout->removeSection(this);
	pPrevSL->format();
	
//	FV_View* pView = m_pLayout->getView();
//      if (pView)
//  	{
//  		pView->_setPoint(pcrx->getPosition());
//  	}
//
// Update the following sections.
//
	while(pDSL != NULL)
	{
		pDSL->updateDocSection();
		pDSL = pDSL->getNextDocSection();
	}
	delete this;			// TODO whoa!  this construct is VERY dangerous.
	
	return true;
}

void fl_DocSectionLayout::addOwnedPage(fp_Page* pPage)
{
	// TODO do we really need the vecOwnedPages member? YES!!!

	if(m_pFirstOwnedPage == NULL)
		m_pFirstOwnedPage = pPage;
	fp_Page * pPrev = m_pFirstOwnedPage;

//
// The addPage methods will add the page to the correct HdrFtrSL.
//
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		if(pHdrFtr->getHFType() < FL_HDRFTR_FOOTER)
		{
			if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_HEADER) == NULL )
				prependOwnedHeaderPage(pPrev);

			pHdrFtr->addPage(pPage);
		}
		else
		{
			if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_FOOTER) == NULL)
			{
				prependOwnedFooterPage(pPrev);
			}
			pHdrFtr->addPage(pPage);
		}
	}
	fl_DocSectionLayout * pDSL = this;
	while(pDSL != NULL)
	{
		pDSL->checkAndRemovePages();
		pDSL->addValidPages();
		pDSL = pDSL->getNextDocSection();
	}
}

void fl_DocSectionLayout::prependOwnedHeaderPage(fp_Page* pPage)
{
	//
	// Skip back through the pages until the first owned page of this section
	//
	fp_Page * pPrev = pPage->getPrev();
	if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_HEADER) == NULL)
	{
		prependOwnedHeaderPage(pPrev);
	}
//
// The addPage methods will add the page to the correct HdrFtrSL.
//
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		if(pHdrFtr->getHFType() < FL_HDRFTR_FOOTER)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: prepending page %x \n",pPage));
			pHdrFtr->addPage(pPage);
		}
	}
}


void fl_DocSectionLayout::prependOwnedFooterPage(fp_Page* pPage)
{
	//
	// Skip back through the pages until the first owned page of this section
	//
	fp_Page * pPrev = pPage->getPrev();
	if(pPrev && pPrev->getOwningSection() == this && pPrev->getHdrFtrP(FL_HDRFTR_FOOTER) == NULL)
	{
		prependOwnedFooterPage(pPrev);
	}
//
// The addPage methods will add the page to the correct HdrFtrSL.
//
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		if(pHdrFtr->getHFType() >= FL_HDRFTR_FOOTER)
		{
			pHdrFtr->addPage(pPage);
		}
	}
}


/*!
 * This fills a vector with all the valid header/footers.
 */
void fl_DocSectionLayout::getVecOfHdrFtrs(UT_Vector * vecHdrFtr)
{
	vecHdrFtr->clear();
	if (m_pHeaderFirstSL != NULL)
	{
		vecHdrFtr->addItem((void *) m_pHeaderFirstSL);
	}
	if (m_pHeaderLastSL  != NULL)
	{
		vecHdrFtr->addItem((void *) m_pHeaderLastSL);
	}
	if (m_pHeaderEvenSL  != NULL)
	{
		vecHdrFtr->addItem((void *) m_pHeaderEvenSL);
	}
	if (m_pHeaderSL  != NULL)
	{
		vecHdrFtr->addItem((void *) m_pHeaderSL);
	}
	if (m_pFooterFirstSL != NULL)
	{
		vecHdrFtr->addItem((void *) m_pFooterFirstSL);
	}
	if (m_pFooterLastSL != NULL)
	{
		vecHdrFtr->addItem((void *) m_pFooterLastSL);
	}
	if (m_pFooterEvenSL != NULL)
	{
		vecHdrFtr->addItem((void *) m_pFooterEvenSL);
	}
	if (m_pFooterSL != NULL)
	{
		vecHdrFtr->addItem((void *) m_pFooterSL);
	}
}

/*!
 * This method formats all the header/footers
 */
void fl_DocSectionLayout::formatAllHdrFtr(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Doing formatAllHdrFtr \n"));
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		xxx_UT_DEBUGMSG(("SEVIOR: Doing formatting %x in formatAllHdrFtr \n",pHdrFtr));
		pHdrFtr->format();
	}
}

/*!
 * This method checks each header for valid pages and removes the page if it's not
 * valid. ie it remove odd pages from even headers etc.
 */
void fl_DocSectionLayout::checkAndRemovePages(void)
{
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		pHdrFtr->checkAndRemovePages();
	}
}


/*!
 * This method adds valid pages to every valid header/footer in the docsection if 
 * they're not there already.
 */
void fl_DocSectionLayout::addValidPages(void)
{
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		pHdrFtr->addValidPages();
	}
}

/*!
 * This method deletes the owned page from the DocSectionLayout and all 
 * the header files.
 */ 	
void fl_DocSectionLayout::deleteOwnedPage(fp_Page* pPage)
{
	UT_Vector vecHdrFtr;
	getVecOfHdrFtrs( &vecHdrFtr);
	UT_uint32 i = 0;
	for(i = 0; i < vecHdrFtr.getItemCount(); i++)
	{
		fl_HdrFtrSectionLayout * pHdrFtr = (fl_HdrFtrSectionLayout *) vecHdrFtr.getNthItem(i);
		if(pHdrFtr->isPageHere(pPage))
		{
			pHdrFtr->deletePage(pPage);
		}
	}
//
// Remove this page from the list of owned pages
//
	if(m_pFirstOwnedPage == pPage)
	{
		fp_Page * pNext = pPage->getNext();
		if(pNext && pNext->getOwningSection() == this)
		{
			m_pFirstOwnedPage = pNext;
		}
		else
		{
			m_pFirstOwnedPage = NULL;
		}
	}
	fl_DocSectionLayout * pDSL = this;
	while(pDSL != NULL && !getDocLayout()->isLayoutDeleting())
	{
		pDSL->checkAndRemovePages();
		pDSL->addValidPages();
		pDSL = pDSL->getNextDocSection();
	}
}

/*!
  Layout sections on pages
  \return zero

  This is the function that controls how sections and thereby columns,
  blocks, and lines are laid out on the pages. Doing so it refers to
  the various layout configurations such as orphan/widow controls and
  break Runs embedded in the text. 

  \fixme This function should move to fb_ColumnBreaker.cpp
*/
UT_sint32 fl_DocSectionLayout::breakSection(fl_BlockLayout * pLastValidBlock)
{
	fl_BlockLayout* pFirstBlock = NULL;
	fp_Line* pCurrentLine = NULL;
	fp_Column* pCurColumn = NULL;
	if(pLastValidBlock == NULL)
	{
		pFirstBlock = getFirstBlock();
		if (!pFirstBlock)
		{
			return 0;
		}
		pCurrentLine = pFirstBlock->getFirstLine();
		pCurColumn = (fp_Column*) getFirstContainer();
	}
//
// This branch is from _reformat in fp_Page. A column that used to exist has been bumped off 
// the page by an expanding column. We need to layout from the last valid block onwards. If
// start the beginning we get stuck in an infinite loop. The last valid block, is the last block
// that has all it's lines correctly laid out on a page.
//
	else
	{
		pFirstBlock = pLastValidBlock;
		if (!pFirstBlock)
		{
			return 0;
		}
		pCurrentLine = pFirstBlock->getFirstLine();
		pCurColumn = (fp_Column*) pCurrentLine->getContainer();
	}

	while (pCurColumn)
	{
		fp_Line* pFirstLineToKeep = pCurrentLine;
		fp_Line* pLastLineToKeep = NULL;
		fp_Line* pOffendingLine = NULL;
		UT_sint32 iMaxSecCol = getMaxSectionColumnHeightInLayoutUnits();
 		UT_sint32 iMaxColHeight = pCurColumn->getMaxHeightInLayoutUnits();
		bool bEquivColumnBreak = false;
		xxx_UT_DEBUGMSG(("SEVIOR: iMaxSecCol = %d iMaxColHeight = %d \n",iMaxSecCol,iMaxColHeight));
		if((iMaxSecCol > 0) && (iMaxSecCol < iMaxColHeight))
		{
			iMaxColHeight = iMaxSecCol;
		    bEquivColumnBreak = true;
		}
		UT_sint32 iWorkingColHeight = 0;

		fp_Line* pCurLine = pFirstLineToKeep;

		// Special handling of columns that should be skipped due to
		// page breaks. If the previous line contains a page break,
		// skip the present column if it is on the same page.
		if (pCurLine)
		{ 
			fp_Line* pPrevLine = pCurLine->getPrev();
			if (pPrevLine && pPrevLine->containsForcedPageBreak()
				&& (pCurColumn->getPage() == pPrevLine->getContainer()->getPage()))
			{
				pCurColumn = pCurColumn->getNext();
				continue;
			}
		}
		bool bBreakOnColumnBreak = false;
		bool bBreakOnPageBreak = false;
		UT_sint32  iTotalLineSpace = 0;
		while (pCurLine)
		{
			UT_sint32 iLineHeight = pCurLine->getHeightInLayoutUnits();
			UT_sint32 iLineMarginAfter = pCurLine->getMarginAfterInLayoutUnits();
			iTotalLineSpace = iLineHeight + iLineMarginAfter;

			if ((iWorkingColHeight + iTotalLineSpace) > iMaxColHeight )
			{
				pOffendingLine = pCurLine;

				/*
				  We have found the offending line (the first one which won't fit in the
				  column) and we now need to decide whether we can break the column
				  just before it.  */

				if (pOffendingLine == pFirstLineToKeep)
				{
					// Wow!  The very first line in this column won't
					// fit.  Big line.  (or maybe a small column)
					// TODO: what should we do here?  For now, we
					// force it.

					pLastLineToKeep = pFirstLineToKeep;
				}
				else
				{
					fl_BlockLayout* pBlock = pOffendingLine->getBlock();
					UT_uint32 iWidows = pBlock->getProp_Widows();
					UT_uint32 iOrphans = pBlock->getProp_Orphans();

					UT_uint32 iNumLinesBeforeOffending = 0;
					UT_uint32 iNumLinesAfterOffending = 0;
					bool bFoundOffending = false;
					
					fp_Line* pFirstLineInBlock = pBlock->getFirstLine();
					pCurLine = pFirstLineInBlock;
					while (pCurLine)
					{
						if (bFoundOffending)
						{
							iNumLinesAfterOffending++;
						}
						else
						{
							if (pCurLine == pOffendingLine)
							{
								iNumLinesAfterOffending = 1;
								bFoundOffending = true;
							}
							else
							{
								iNumLinesBeforeOffending++;
							}
						}
						
						pCurLine = pCurLine->getNext();
					}

					UT_uint32 iNumLinesInBlock = iNumLinesBeforeOffending + iNumLinesAfterOffending;

					UT_uint32 iNumBlockLinesInThisColumn = 0;
					pCurLine = pOffendingLine->getPrev();
					while (pCurLine)
					{
						iNumBlockLinesInThisColumn++;
						if (pCurLine == pFirstLineToKeep)
						{
							break;
						}

						pCurLine = pCurLine->getPrev();
					}

					if (
						pBlock->getProp_KeepTogether()
						&& (iNumLinesBeforeOffending == iNumBlockLinesInThisColumn)
						&& (pBlock->getFirstLine() != pFirstLineToKeep)
						)
					{
						/*
						  This block wants to be kept all in the same column.
						  Bump the whole block to the next column.
						*/

						/*
						  If the block is simply too big to fit in a
						  single column, then we can spawn an infinite
						  loop by continually punting it to the next
						  one.  So, we assume that if the first line
						  in the block is the first line in this
						  column, we just keep it and cope.  This will
						  be slightly incorrect in cases where pushing
						  it to the next column would allow the block
						  to try to live in a larger column, thus
						  staying all together.
						*/
						
						pLastLineToKeep = pFirstLineInBlock->getPrevLineInSection();
					}
					else if (
						(iNumLinesInBlock < (iWidows + iOrphans))
						&& (iNumLinesBeforeOffending == iNumBlockLinesInThisColumn)
						)
					{
						/*
						  There are not enough lines to divide between the
						  two columns while still satisfying both constraints.
						  Bump the whole block to the next column.
						*/
						
						pLastLineToKeep = pFirstLineInBlock->getPrevLineInSection();
					}
					else if (
						(iNumLinesBeforeOffending < iOrphans)
						&& (iNumLinesBeforeOffending == iNumBlockLinesInThisColumn)
						)
					{
						/*
						  We're leaving too few lines in the current column.
						  Bump the whole block.
						*/

						pLastLineToKeep = pFirstLineInBlock->getPrevLineInSection();
					}
					else if (
						(iNumLinesAfterOffending < iWidows)
						&& ((iWidows - iNumLinesAfterOffending) < iNumBlockLinesInThisColumn)
						)
					{
						/*
						  There aren't going to be enough lines in the next
						  column.  Bump just enough.
						*/

						UT_uint32 iNumLinesNeeded = (iWidows - iNumLinesAfterOffending);
						pLastLineToKeep = pOffendingLine->getPrevLineInSection();
						for (UT_uint32 iBump = 0; iBump < iNumLinesNeeded; iBump++)
						{
							pLastLineToKeep = pLastLineToKeep->getPrevLineInSection();
						}
					}
					else
					{
						pLastLineToKeep = pOffendingLine->getPrevLineInSection();
					}
				}
				break;
			}
			else
			{
				iWorkingColHeight += iTotalLineSpace;
				if (
					pCurLine->containsForcedColumnBreak()
					|| pCurLine->containsForcedPageBreak() 
					)
				{
					pLastLineToKeep = pCurLine;
					bBreakOnColumnBreak = ( pCurLine->containsForcedColumnBreak()) ;
					bBreakOnPageBreak = pCurLine->containsForcedPageBreak();
					if((iWorkingColHeight >=  iMaxColHeight))
						bBreakOnColumnBreak = false;
					break;
				}
			}

			pCurLine = pCurLine->getNextLineInSection();
		} 
//
// End of big while loop here. After this we've found LastLineToKeep
//
		bEquivColumnBreak = bEquivColumnBreak && ( iMaxColHeight < (iWorkingColHeight + iTotalLineSpace));
		if (pLastLineToKeep)
		{
			pCurrentLine = pLastLineToKeep->getNextLineInSection();
		}
		else
		{
			pCurrentLine = NULL;
		}
//
// OK fill our column with content between pFirstLineToKeep and pLastLineToKeep
//		
		pCurLine = pFirstLineToKeep;
		while (pCurLine)
		{
			if (pCurLine->getContainer() != pCurColumn)
			{
				pCurLine->getContainer()->removeLine(pCurLine);
				pCurColumn->addLine(pCurLine);
			}

			if (pCurLine == pLastLineToKeep)
			{
				break;
			}
			else
			{
				pCurLine = pCurLine->getNextLineInSection();
			}
		}

	
		if (pLastLineToKeep
			&& pCurColumn->getLastLine() != pLastLineToKeep)
		{
			UT_ASSERT(pLastLineToKeep->getContainer() == pCurColumn);
			
			fp_Page* pPrevPage = pCurColumn->getPage();

			fp_Column* pNextColumn = pCurColumn;
			do
			{
				// Make sure there is a next column and that it
				// falls on the next page if there's a page break.
				pNextColumn = pNextColumn->getNext();
				if(bBreakOnColumnBreak || bEquivColumnBreak)
				{
					if((pNextColumn != NULL) &&( pNextColumn != pCurColumn->getFollower()) && (pNextColumn->getPage() != pCurColumn->getPage()))
					{
						pNextColumn = NULL;
					}
				}
				if (!pNextColumn)
				{
					if(bBreakOnColumnBreak || bEquivColumnBreak)
					{
						pNextColumn = (fp_Column*) getNewContainer(pLastLineToKeep->getNextLineInSection());
					}
					else
					{
						pNextColumn = (fp_Column*) getNewContainer(NULL);
					}
				}
			} while (pLastLineToKeep->containsForcedPageBreak() 
					 && (pNextColumn->getPage() == pPrevPage));
		   
			// Bump content down the columns
			while (pCurColumn != NULL && pCurColumn != pNextColumn)
			{
				pCurColumn->bumpLines(pLastLineToKeep);
				pCurColumn->layout();
				pCurColumn = pCurColumn->getNext();

				// This is only relevant for the initial column. All
				// other columns should flush their entire content.
				pLastLineToKeep = NULL;
			}
		}
		else
		{
			UT_ASSERT((!pLastLineToKeep) || (pCurColumn->getLastLine() == pLastLineToKeep));
			
			pCurColumn->layout();

			pCurColumn = pCurColumn->getNext();
		}
	}

	return 0; // TODO return code
}

/*!
 * This method returns true if the pPage pointer matches the header/footer type
 * given.
\param hfType The type of the header/Footer
\param fp_Page * pThisPage pointer to the page queried. 
 */
bool fl_DocSectionLayout::isThisPageValid(HdrFtrType hfType, fp_Page * pThisPage)
{
	typedef enum {odd,even,first,last} PageType;
	PageType FirstLast = odd;
	PageType OddEven = odd;
	fp_Page * pPage = m_pFirstOwnedPage;
	if(!pPage)
	{
	    return false;
	}
//
// No header/footerness assigned yet. Page is invalid.
//
	if(hfType == FL_HDRFTR_NONE)
	{
		return false;
	}
	if(pThisPage == pPage)
	{
		FirstLast = first;
	}
	if(hfType == FL_HDRFTR_HEADER_FIRST)
	{
		if(FirstLast == first)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	if(hfType == FL_HDRFTR_FOOTER_FIRST)
	{
		if(FirstLast == first)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
//
// If there is a header page defined and this is a header page bail now!
//
	if(m_pHeaderFirstSL && (FirstLast == first) && (hfType < FL_HDRFTR_FOOTER))
	{
		return false;
	}
	if(m_pFooterFirstSL && (FirstLast == first) && (hfType >= FL_HDRFTR_FOOTER))
	{
		return false;
	}

	fp_Page * pNext = pPage->getNext();
	while(pNext && (pNext->getOwningSection() == this))
	{
		pPage = pNext;
		pNext = pNext->getNext();
	}
	if(pPage == pThisPage)
	{
		FirstLast = last;
	}
	if(hfType == FL_HDRFTR_HEADER_LAST)
	{
		if(FirstLast == last)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	if(hfType == FL_HDRFTR_FOOTER_LAST)
	{
		if(FirstLast == last)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
//
// If there is a Last SL  defined and this is the last page in the SLpage bail now!
//
	if(m_pHeaderLastSL && (FirstLast == last) &&  (hfType < FL_HDRFTR_FOOTER) )
	{
		return false;
	}
	if(m_pFooterLastSL && (FirstLast == last) &&  (hfType >= FL_HDRFTR_FOOTER))
	{
		return false;
	}

	UT_sint32 count = (UT_sint32) getDocLayout()->countPages();
	UT_sint32 i =0;
	bool bFound = false;
	for(i=0; (i < count) && !bFound ; i++)
	{
		bFound = (getDocLayout()->getNthPage(i) == pThisPage);
	}
	if(bFound)
	{
		UT_sint32 j = i/2;
		if(j*2 != i)
		{
			OddEven = odd;
		}
		else
		{
			OddEven = even;
		}
	}
	else
	{
		UT_ASSERT(0);
	}
	if((hfType == FL_HDRFTR_HEADER_EVEN) || (hfType == FL_HDRFTR_FOOTER_EVEN))
	{
		if(OddEven == even)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
//
// If there is an Even SL  defined and this is an even page in the SL page bail now!
//
	if(m_pHeaderEvenSL && (OddEven == even) &&  (hfType < FL_HDRFTR_FOOTER))
	{
		return false;
	}
	if(m_pFooterEvenSL && (OddEven == even) &&  (hfType >= FL_HDRFTR_FOOTER))
	{
		return false;
	}
	return true; //if we're here all pages are valid.
}

void fl_DocSectionLayout::checkAndAdjustColumnGap(UT_sint32 iLayoutWidth)
{
	// Check to make sure column gap is not to wide to fit on the page with the
	// given number of columns.

	if(m_iNumColumns > 1)
	{
		UT_sint32 minColumnWidth = m_pLayout->getGraphics()->convertDimension("0.5in");	//TODO should this dimension be hard coded.
		UT_sint32 iColWidth = (iLayoutWidth - (UT_sint32)(((m_iNumColumns - 1) * m_iColumnGap))) / (UT_sint32)m_iNumColumns;

		if(iColWidth < minColumnWidth)
		{
			m_iColumnGap = (iLayoutWidth - minColumnWidth * m_iNumColumns) / (m_iNumColumns - 1);
		}
			
	}

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT _PageHdrFtrShadowPair
{
public:
	_PageHdrFtrShadowPair(void)
	   {
			m_pPage = NULL;
			m_pShadow = NULL;
		}
	virtual ~_PageHdrFtrShadowPair(void)
		{
			m_pPage = NULL;
			m_pShadow = NULL;
		}
	void setPage (fp_Page * pPage) { m_pPage = pPage;}
	void setShadow (fl_HdrFtrShadow * pShadow) { m_pShadow = pShadow;}
	fp_Page * getPage(void) const {return m_pPage;}
	fl_HdrFtrShadow * getShadow(void) const {return m_pShadow;}
private:
	fp_Page*			m_pPage;
	fl_HdrFtrShadow*	m_pShadow;
};


fl_HdrFtrSectionLayout::fl_HdrFtrSectionLayout(HdrFtrType iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_HDRFTR)
{
	m_pDocSL = pDocSL;
	m_iHFType = iHFType;
	m_iType = FL_SECTION_HDRFTR;
	m_pHdrFtrContainer = NULL;
	fl_Layout::setType(PTX_SectionHdrFtr); // Set the type of this strux
	xxx_UT_DEBUGMSG(("SEVIOR: Creating HFType =%d \n",m_iHFType));
//
// Since we're almost certainly removing blocks at the end of the doc, tell the
// view to remember the current position on the active view.
//
	FV_View * pView = m_pLayout->getView();
	if(pView && pView->isActive())
	{
		pView->markSavedPositionAsNeeded();
	}
}

fl_HdrFtrSectionLayout::~fl_HdrFtrSectionLayout()
{
	xxx_UT_DEBUGMSG(("SEVIOR: Deleting HFType =%d \n",m_iHFType));
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		delete pPair->getShadow();
	}
	_purgeLayout();
	DELETEP(m_pHdrFtrContainer);
//
// Take this section layout out of the linked list
//
	m_pLayout->removeHdrFtrSection((fl_SectionLayout *) this);
//
// Null out pointer to this HdrFtrSection in the attached DocLayoutSection
//
	m_pDocSL->setHdrFtr(m_iHFType, NULL);
//
// Since we're almost certainly removing blocks at the end of the doc, tell the
// view to remember the current position on the active view.
//
	FV_View * pView = m_pLayout->getView();
	if(pView && pView->isActive())
	{
		pView->markSavedPositionAsNeeded();
	}
//
	UT_VECTOR_PURGEALL(_PageHdrFtrShadowPair*, m_vecPages);
}

/*!
 * This method removes all the lines and containers associated with the shadows
 * and the lines associated with this HdrFtrSectionLayout.
 *
 */ 
void fl_HdrFtrSectionLayout::collapse(void)
{
//
// If a view exists and we're editting a header footer take the pointer out of
// the header/footer. This will also clear the box around the header/footer
//
	FV_View * pView = m_pLayout->getView();
	if(pView && pView->isHdrFtrEdit())
	{
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0,0,false);
		pView->rememberCurrentPosition();
	}

	localCollapse();
	UT_uint32 iCount = m_vecPages.getItemCount();
	UT_uint32 i;
	for (i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		fp_Page * ppPage = pPair->getPage();
		delete pPair->getShadow();
		ppPage->removeHdrFtr(getHFType());
		delete pPair;
	}
	m_vecPages.clear();
	DELETEP(m_pHdrFtrContainer);
}

/*!
 * This method removes the block pBlock from all the shadowLayouts.
 */
void fl_HdrFtrSectionLayout::collapseBlock(fl_BlockLayout *pBlock)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	UT_uint32 i;
	for (i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		fl_BlockLayout * pShadowBL = pPair->getShadow()->findMatchingBlock(pBlock);
		UT_ASSERT(pShadowBL);
		if(pShadowBL)
		{
			// In case we've never checked this one
			m_pLayout->dequeueBlockForBackgroundCheck(pShadowBL);
			pPair->getShadow()->removeBlock( pShadowBL);
			delete pShadowBL;
			pPair->getShadow()->format();
		}
	}
}

bool fl_HdrFtrSectionLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	bool bResult = false;
	
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		UT_ASSERT(pPair->getShadow());
		bResult = pPair->getShadow()->recalculateFields(iUpdateCount) || bResult;
	}

	return bResult;
}


fl_HdrFtrShadow * fl_HdrFtrSectionLayout::getFirstShadow(void)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	if(iCount != 0)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(0);
		return pPair->getShadow();
	}
	return NULL;
}

fp_Container* fl_HdrFtrSectionLayout::getFirstContainer()
{
	return m_pHdrFtrContainer;
}


fp_Container* fl_HdrFtrSectionLayout::getLastContainer()
{
	return m_pHdrFtrContainer;
}

fp_Container* fl_HdrFtrSectionLayout::getNewContainer(fp_Line * pFirstLine)
{
	DELETEP(m_pHdrFtrContainer);
	UT_sint32 iWidth = m_pDocSL->getFirstContainer()->getPage()->getWidth();
	UT_sint32 iWidthLayout = m_pDocSL->getFirstContainer()->getPage()->getWidthInLayoutUnits() - m_pDocSL->getLeftMarginInLayoutUnits() - m_pDocSL->getRightMarginInLayoutUnits();
	m_pHdrFtrContainer = (fp_Container *) new fp_HdrFtrContainer(iWidth,iWidthLayout, (fl_SectionLayout *) this);
	return m_pHdrFtrContainer;
}

bool fl_HdrFtrSectionLayout::isPageHere(fp_Page * pPage)
{
	return (_findShadow(pPage) >=0 );
}

fl_HdrFtrShadow *  fl_HdrFtrSectionLayout::findShadow(fp_Page* pPage)
{
       UT_sint32 iPage = _findShadow(pPage);
       if(iPage < 0)
	        return NULL;
       _PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(iPage);
       return pPair->getShadow();
}

UT_sint32 fl_HdrFtrSectionLayout::_findShadow(fp_Page* pPage)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		if (pPair->getPage() == pPage)
		{
			return i;
		}
	}

	return -1;
}
/*!
 * This method converts a previously existing section to this header/footer.
 * Code liberally stolen from fl_DocSectionLayout::doclistener_deleteStrux
 \param fl_DocSectionLayout * pSL sectionlayout to be converted to a 
 *     HdrFtrSectionLayout
*/
void fl_HdrFtrSectionLayout::changeIntoHdrFtrSection( fl_DocSectionLayout * pSL)
{
	fl_DocSectionLayout* pPrevSL = pSL->getPrevDocSection();
	UT_ASSERT(pPrevSL);
	// clear all the columns
	fp_Column* pCol =NULL;

	pCol = (fp_Column *) pSL->getFirstContainer();
	while (pCol)
	{
		pCol->clearScreen();

		pCol = pCol->getNext();
	}

	// remove all the columns from their pages
	pCol = (fp_Column *) pSL->getFirstContainer();
	while (pCol)
	{
		if (pCol->getLeader() == pCol)
		{
			pCol->getPage()->removeColumnLeader(pCol);
		}

		pCol = pCol->getNext();
	}


	// get rid of all the layout information for every block
	fl_BlockLayout*	pBL = pSL->getFirstBlock();
	while (pBL)
	{
		pBL->collapse();

		pBL = pBL->getNext();
	}

	//
	// Change the section type
	//

	// transfer the Sections' blocks into this header/footer

	while (pSL->getFirstBlock())
	{
		pBL = pSL->getFirstBlock();
		pSL->removeBlock(pBL);
		addBlock(pBL);
		pBL->setSectionLayout(this);
		pBL->setHdrFtr();
	}
	//
	// Remove old section from the section linked list!!
	//
	m_pLayout->removeSection(pSL);
//
	DELETEP(pSL); // Old Section layout is totally gone
	//
	// Create and Format the shadows
	//
	format();

	// Finished! we now have a header/footer
}

/*! 
 * Remove the strux identifing this as a seperate section has been deleted so
 * we have to remove this HdrFtrSectionLayout class and all the shadow sections
 * attached to it. The blocks in this class are moved to the DocSectionLayout
 * associated with this class. 
 * I do this because I expect that this will be called as part
 * on an undo "Insert Header" command. The rest of the undo needs blocks to
 * delete so I'm putting them there to keep the rest of the undo code happy
\param pcrx the changerecord identifying this action as necesary. 
\returns true
*/
bool fl_HdrFtrSectionLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_SectionHdrFtr);
//
// Get last doc section. Move all the blocks from here to there after deleting 
// this strux.
//
	fl_DocSectionLayout* pPrevSL = m_pDocSL;
	if (!pPrevSL)
	{
		UT_DEBUGMSG(("no prior SectionLayout"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
//
// Get rid of all the shadows, all the containers, and all the layout 
// information for all the blocks.
//	
	collapse();
//
// Now copy these line-less blocks into the previous docSectionLayout.
// Note: I expect that these blocks will be deleted by a later delete strux
// on these blocks.
//
	fl_BlockLayout * pBL = NULL;
	while (m_pFirstBlock)
	{
		pBL = m_pFirstBlock;
		removeBlock(pBL);
		pPrevSL->addBlock(pBL);
	}
//
// Format the new section containing the blocks.
//
	pPrevSL->format(); 
//
// Finally delete this HdrFtrSectionLayout. This could be done the docListener
// class but here I'm following the convention for the DocSectionLayout. It
// works there so I hope it works here. The HdrFtrSection destructor takes care
// of the details of unlinking the section etc.
//
	delete this;
	return true;
}

void fl_HdrFtrSectionLayout::addPage(fp_Page* pPage)
{
//
//  Sevior:
//  This triggers if we're rebuilding a section before page is defined like in a section change
//  strux. Reinstate if needed to find other bugs.
//	UT_ASSERT(0 > _findShadow(pPage));
//
	
	if(_findShadow(pPage) > -1)
		return;
//
// Check this page is valid for this type of hdrftr
//
	if(!getDocSectionLayout()->isThisPageValid(m_iHFType, pPage))
	{
		return;
	}
	//
	// Check if the first Block for this HdrFtr exists yet. This can happen on
    // some zooms.
	//
	if(getFirstBlock() == NULL)
	{
		return;
	}
	//
	// see if this page has a shadow attached already. This can happen
    // is a page goes from being odd to even.
	//
	fp_ShadowContainer* pOldShadow = pPage->getHdrFtrP(m_iHFType);
	//
	// If so remove it.
	//
	if(pOldShadow != NULL)
	{
		pOldShadow->getHdrFtrSectionLayout()->deletePage(pPage);
	}
	_PageHdrFtrShadowPair* pPair = new _PageHdrFtrShadowPair();
	// TODO outofmem
	xxx_UT_DEBUGMSG(("SEVIOR: Add page %x to pair %x \n",pPage,pPair));
	pPair->setPage(pPage);
	pPair->setShadow(new fl_HdrFtrShadow(m_pLayout, pPage, this, m_sdh, m_apIndex));
	//
	// Make sure we register the shadow before populating it.
	//
	m_vecPages.addItem(pPair);
	//
	// Populate the shadow
	//

	fl_ShadowListener* pShadowListener = new fl_ShadowListener(this, pPair->getShadow());
//
// Populate with just this section so find the start and end of it
//
	PT_DocPosition posStart,posEnd,posDocEnd;
	posStart = getFirstBlock()->getPosition(true) - 1;
	posEnd = getLastBlock()->getPosition(false);
	fp_Run * pRun = getLastBlock()->getFirstRun();
	while(pRun->getNext() != NULL)
	{
		pRun = pRun->getNext();
	}
	posEnd += pRun->getBlockOffset();
	PL_StruxDocHandle sdh=NULL;
	bool bres;
	bres = m_pDoc->getStruxOfTypeFromPosition(posEnd, PTX_Block, &sdh);
	m_pDoc->getBounds(true,posDocEnd);
	while(bres && sdh == getLastBlock()->getStruxDocHandle()
		  && posEnd <= posDocEnd)
	{
		posEnd++;
		bres = m_pDoc->getStruxOfTypeFromPosition(posEnd, PTX_Block, &sdh);
	}
	posEnd--;
	UT_ASSERT(posEnd > posStart);
	PD_DocumentRange * docRange = new PD_DocumentRange(m_pDoc,posStart,posEnd);
	m_pDoc->tellListenerSubset(pShadowListener, docRange);
	delete docRange;
	delete pShadowListener;
	markAllRunsDirty();
}

bool fl_HdrFtrSectionLayout::isPointInHere(PT_DocPosition pos)
{
//
// Skip through the blocks in this shadow to find the one containing this
// point.
//
    fl_BlockLayout*	pBL = m_pFirstBlock;
	if(pBL == NULL)
		return false;
	if(pos < pBL->getPosition())
	{
//
// This corner case is that pos == position of the HdrFtr strux
//
		if(pos == (pBL->getPosition() - 1))
		{
			return true;
		}
		return false;;
	}
//
// OK see if the next hdrftr is ahaead of the pos
//
	fl_HdrFtrSectionLayout * pHF = (fl_HdrFtrSectionLayout *) getNext();
	if(pHF == NULL)
	{
		PT_DocPosition posEOD;
		m_pDoc->getBounds(true,posEOD);
		if(pos <= posEOD)
		{
			return true;
		}
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	fl_BlockLayout * ppBL = pHF->getFirstBlock();
	if(ppBL != NULL)
	{
		if(pos < (ppBL->getPosition()-1))
		{
			return true;
		}
		return false;
	}

	fl_BlockLayout* pNext = pBL->getNext();
	while(pNext != NULL && pNext->getPosition( true) < pos)
	{
		pBL = pNext;
		pNext = pNext->getNext();
	}
	if(pNext != NULL)
	{
		return true;
	}
	else if(pBL && pBL->getPosition() == pos)
	{
		return true;
	}
//
// Now the point MIGHT be in this last block. Use code from pd_Document
// to find out. Have to check whether we're out of docrange first
//
	PL_StruxDocHandle sdh=NULL;
	bool bres;
	bres = m_pDoc->getStruxOfTypeFromPosition(pos, PTX_Block, &sdh);
	if(bres && sdh == pBL->getStruxDocHandle())
	{
		return true;
	}
	return false;
}

/*!
 * Removes the shadow and the corresponding element pointing to the shadow for this
 * Page.
 */
void fl_HdrFtrSectionLayout::deletePage(fp_Page* pPage)
{
	UT_sint32 iShadow = _findShadow(pPage);
//
// This shadow might have already been deleted via the collapse method
//
	if(iShadow <  0)
		return;
	struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(iShadow);
	UT_ASSERT(pPair);

	UT_ASSERT(pPair->getShadow());


	fp_Page * ppPage = pPair->getPage();
	UT_ASSERT(pPage == ppPage);
	delete pPair->getShadow();
	if(getDocLayout()->findPage(ppPage) >= 0)
	{
			ppPage->removeHdrFtr(getHFType());
	}
	delete pPair;
	m_vecPages.deleteNthItem(iShadow);
}


/*!
 *  Just format the HdrFtrSectionLayout blocks for an insertBlock method.
 *  these blocks will be collapsed afterwards.
 */
void fl_HdrFtrSectionLayout::localFormat(void)
{
	xxx_UT_DEBUGMSG(("Doing a Local Format of the hdrftr section \n"));
	if(!getDocSectionLayout())
		return;
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		pBL->setHdrFtr();
		pBL->format();
		pBL = pBL->getNext();
	}
}

/*!
 *  Just collapse the HdrFtrSectionLayout blocks for an insertBlock method.
 *  This removes all lines and references to containers but leaves the blocks
 *  and runs intact.
 */
void fl_HdrFtrSectionLayout::localCollapse(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->collapse();
		pBL = pBL->getNext();
	}
}

/*!
 * This routine returns the matching block within this HdrFtrSectionLayout of the
 * shadow. 
 \param fl_BlockLayout * Pointer to block in shadow
 \returns the pinter to the matching block in the HdrFtr
 */
fl_BlockLayout* fl_HdrFtrSectionLayout::findMatchingBlock(fl_BlockLayout* pBL)
{
	fl_BlockLayout* ppBL = m_pFirstBlock;
	while(ppBL && (ppBL->getStruxDocHandle() != pBL->getStruxDocHandle()))
	{
		ppBL = ppBL->getNext();
	}
	UT_ASSERT(ppBL);
	xxx_UT_DEBUGMSG(("This header/footer is %x in findmatchingBlock \n",this));
	return ppBL;
}

/*!
 * This method checks that the pages in this header are valid and removes them if 
 * they're not.
 */
void fl_HdrFtrSectionLayout::checkAndRemovePages(void)
{
	UT_sint32 iCount = (UT_sint32) m_vecPages.getItemCount();
//
// Check that the pages we have are still valid. Delete them if they're not.
//
	UT_sint32 i = 0;
	UT_Vector pageForDelete;
	for(i =0; i< iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		UT_ASSERT(pPair);
		UT_ASSERT(pPair->getShadow());

		fp_Page * ppPage = pPair->getPage();
		if(getDocLayout()->findPage(ppPage) >= 0)
		{
			if(!getDocSectionLayout()->isThisPageValid(getHFType(),ppPage))
			{
				pageForDelete.addItem((void *) ppPage);
			}
		}
		else
		{
			pageForDelete.addItem((void *) ppPage);
		}
	}
	for(i=0; i< (UT_sint32) pageForDelete.getItemCount(); i++)
	{
		fp_Page * pPage = (fp_Page *) pageForDelete.getNthItem(i);
		deletePage(pPage);
	}
	if( pageForDelete.getItemCount() > 0)
	{
		markAllRunsDirty();
	}
}

/*! 
 * This method adds valid pages to the collection of shadows.
 */
void fl_HdrFtrSectionLayout::addValidPages(void)
{
	//
	// Check that all the pages this header/footer should have are 
    // in place.
	// We have to extract this information from m_pDocSL
	// Loop through all the columns in m_pDocSl and find the pages owned
	// by m_pDocSL
	//
	fp_Column * pCol = (fp_Column *) m_pDocSL->getFirstContainer();
	fp_Page * pOldPage = NULL;
	fp_Page * pNewPage = NULL;
	while(pCol)
	{
		pNewPage = pCol->getPage();
		if((pNewPage != NULL) && (pNewPage != pOldPage) && (getDocLayout()->findPage(pNewPage) >=0))
		{
			fl_DocSectionLayout* pDocSec = pNewPage->getOwningSection();
			if(pDocSec == m_pDocSL && _findShadow(pNewPage) < 0)
			{
//
// The addPage Method checks that only valid pages are added to this HdrFtr based on
// the HFType
//
				addPage(pNewPage);
			}
		}
		pCol = pCol->getNext();
	}
}

/*!
 * Format the overall HdrFtrSectionLayout in it's virtual container.
 * Also check that all the correct pages have been found for this HdrFtr. Then
 * format the Shadows.
 */ 
void fl_HdrFtrSectionLayout::format(void)
{
	if(getFirstBlock() == NULL)
	{
		return;
	}
	UT_sint32 iCount =0;
	UT_sint32 i = 0;
	localFormat();
	//
	// Fail safe code to add pages if we don't have them.
	//
	addValidPages();

	iCount = m_vecPages.getItemCount();
	
	for (i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		pPair->getShadow()->format();
	}
	layout();
}

void fl_HdrFtrSectionLayout::updateLayout(void)
{
	bool bredraw = false;
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			bredraw = true;
			pBL->format();
		}
		
		pBL = pBL->getNext();
	}
	if(bredraw == true)
	{
		if(m_pHdrFtrContainer)
			static_cast<fp_HdrFtrContainer *>(m_pHdrFtrContainer)->layout();
 	}

	//
	// update Just the  blocks in the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->getShadow()->updateLayout();
	}
}

/*!
 * Mark all runs and lines in the all shadows for redraw.
 */
void fl_HdrFtrSectionLayout::markAllRunsDirty(void)
{
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->getShadow()->markAllRunsDirty();
	}
}

/*!
 * Layout the overall HdrFtr and everything underneath it.
 */
void fl_HdrFtrSectionLayout::layout(void)
{
    if(m_pHdrFtrContainer)
	static_cast<fp_HdrFtrContainer *>(m_pHdrFtrContainer)->layout();
	//
	// update the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->getShadow()->layout();
	}
}

/*!
 * This method updates the background color in the header/footer section and
 * all the shadows associated with it.
 */
void fl_HdrFtrSectionLayout::updateBackgroundColor(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->updateBackgroundColor();
		pBL = pBL->getNext();
	}
	//
	// update Just the  blocks in the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->getShadow()->updateBackgroundColor();
	}
}


void fl_HdrFtrSectionLayout::clearScreen(void)
{
	//
	// update Just the  blocks in the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->getShadow()->clearScreen();
	}
}

void fl_HdrFtrSectionLayout::redrawUpdate(void)
{
//
// Do another layout but don't redraw.
//
	if(m_pHdrFtrContainer)
		static_cast<fp_HdrFtrContainer *>(m_pHdrFtrContainer)->layout();
	//
	// Don't need to draw here since this is never displayed on the screen?
	// 
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->getShadow()->redrawUpdate();
	}

}

bool fl_HdrFtrSectionLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	setAttrPropIndex(pcrxc->getIndexAP());

	// TODO what happens here?

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

void fl_HdrFtrSectionLayout::_lookupProperties(void)
{
}

bool fl_HdrFtrSectionLayout::bl_doclistener_populateSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_Span * pcrs, PT_BlockOffset blockOffset, UT_uint32 len)
{
//
// We need to populate block in the header/footer but to do that we need the
// header/footer to be fomatted. So do it then unformat after.
//
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();

	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		if(pShadowBL == NULL)
		{
			return false;
		}
		bResult = pShadowBL->doclistener_populateSpan(pcrs,blockOffset,len)
			&& bResult;
	}
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_populateSpan(pcrs,blockOffset,len)
			&& bResult;
	return bResult;
}

/*!
 * Now for all these methods which manipulate the shadow sections, turn off
 * Insertion Point changes while the shadows are manipulated. 
 * Re Enabled insertion point changes for the overall hdrftrsection so it
 * is changed just once.
 */

bool fl_HdrFtrSectionLayout::bl_doclistener_populateObject(fl_BlockLayout* pBL, PT_BlockOffset blockOffset, const PX_ChangeRecord_Object * pcro)
{
//
// We need to populate block in the header/footer but to do that we need the
// header/footer to be fomatted. So do it then unformat after.
//  
  	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		if(pShadowBL == NULL)
		{
			return false;
		}
		bResult = pShadowBL->doclistener_populateObject(blockOffset,pcro)
			&& bResult;
	}
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
  	bResult = pBL->doclistener_populateObject(blockOffset,pcro)
  		&& bResult;
	return bResult;
}
	
bool fl_HdrFtrSectionLayout::bl_doclistener_insertSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_insertSpan(pcrs)
			&& bResult;
	}
	m_pDoc->allowChangeInsPoint();
	// Update the overall block too.
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_insertSpan(pcrs)
	&& bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_Span * pcrs)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteSpan(pcrs)
			&& bResult;
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_deleteSpan(pcrs)
		&& bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeSpan(fl_BlockLayout* pBL, const PX_ChangeRecord_SpanChange * pcrsc)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_changeSpan(pcrsc)
			&& bResult;
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_changeSpan(pcrsc)
		&& bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteStrux(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteStrux(pcrx)
			&& bResult;
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_deleteStrux(pcrx) && bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeStrux(fl_BlockLayout* pBL, const PX_ChangeRecord_StruxChange * pcrxc)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_changeStrux(pcrxc)
			&& bResult;
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
    bResult = pBL->doclistener_changeStrux(pcrxc)
		&& bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertBlock(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx,PL_StruxDocHandle sdh,PL_ListenerId lid,void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,	PL_ListenerId lid, PL_StruxFmtHandle sfhNew))
{
	bool bResult = true;
//
// Now insert it into all the shadows.
//
	UT_uint32 iCount = m_vecPages.getItemCount();
	fl_BlockLayout * pShadowBL = NULL;
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.
		if(pBL)
		{
			pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
			bResult = pShadowBL->doclistener_insertBlock(pcrx,sdh,lid,pfnBindHandles)
				&& bResult;
		}
		else
//
// This is the first block in the shadow
//
		{
			fl_BlockLayout*	pNewBL = pPair->getShadow()->insertBlock(sdh, NULL, pcrx->getIndexAP());
			if (!pNewBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout\n"));
				return false;
			}
			bResult = bResult && pNewBL->doclistener_insertFirstBlock(pcrx, sdh, 
													lid, pfnBindHandles);
		}
	}
//
// Find Matching Block in this HdrFtrSectionLayout!!
//
	if(pBL)
	{
		fl_BlockLayout * ppBL = findMatchingBlock(pBL);
		m_pDoc->allowChangeInsPoint();

		ppBL->setHdrFtr();
		bResult = ppBL->doclistener_insertBlock(pcrx,sdh,lid,pfnBindHandles)
			&& bResult;
//
// Mark the Block as HdrFtr
//
		ppBL->getNext()->setHdrFtr();
	}
	else
//
// First block in the section
//
	{
		fl_BlockLayout*	pNewBL = insertBlock(sdh, NULL, pcrx->getIndexAP());
		if (!pNewBL)
		{
			UT_DEBUGMSG(("no memory for BlockLayout\n"));
			return false;
		}
		bResult = bResult && pNewBL->doclistener_insertFirstBlock(pcrx, sdh, 
													lid, pfnBindHandles);
		m_pDoc->allowChangeInsPoint();
		pNewBL->setHdrFtr();
	}
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertSection(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
														  PL_StruxDocHandle sdh,
														  PL_ListenerId lid,
														  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																				  PL_ListenerId lid,
																				  PL_StruxFmtHandle sfhNew))
{
	// TODO this should NEVER happen, right?
	UT_DEBUGMSG(("Insert Section is header/footer!!! \n"));
	UT_ASSERT(0);
	bool bResult = true;
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		bResult = pPair->getShadow()->bl_doclistener_insertSection(pBL, FL_SECTION_DOC, pcrx, sdh, lid, pfnBindHandles)
			&& bResult;
	}

	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertObject(fl_BlockLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_insertObject(pcro)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_insertObject(pcro) && bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteObject(fl_BlockLayout* pBL, const PX_ChangeRecord_Object * pcro)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteObject(pcro)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_deleteObject(pcro) && bResult;

	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeObject(fl_BlockLayout* pBL, const PX_ChangeRecord_ObjectChange * pcroc)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_changeObject(pcroc)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_changeObject(pcroc) && bResult;

	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_insertFmtMark(fl_BlockLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_insertFmtMark(pcrfm)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_insertFmtMark(pcrfm) && bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_deleteFmtMark(fl_BlockLayout* pBL, const PX_ChangeRecord_FmtMark * pcrfm)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteFmtMark(pcrfm)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_deleteFmtMark(pcrfm)	&& bResult;
	return bResult;
}

bool fl_HdrFtrSectionLayout::bl_doclistener_changeFmtMark(fl_BlockLayout* pBL, const PX_ChangeRecord_FmtMarkChange * pcrfmc)
{
	bool bResult = true;
	fl_BlockLayout * pShadowBL = NULL;
	UT_uint32 iCount = m_vecPages.getItemCount();
	m_pDoc->setDontChangeInsPoint();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		_PageHdrFtrShadowPair* pPair = (_PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->getShadow()->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_changeFmtMark(pcrfmc)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_changeFmtMark(pcrfmc) && bResult;
	return bResult;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_HdrFtrShadow::fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pHdrFtrSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_SHADOW)
{
	m_pHdrFtrSL = pHdrFtrSL;
	m_pPage = pPage;
	if (m_pHdrFtrSL->getHFType() < FL_HDRFTR_FOOTER)
	{
		m_pContainer = m_pPage->buildHeaderContainer(m_pHdrFtrSL);
	}
	else
	{
		m_pContainer =  m_pPage->buildFooterContainer(m_pHdrFtrSL);
	}
	xxx_UT_DEBUGMSG(("SEVIOR: created shadow %x with container %x \n",this,m_pContainer));
	m_iType = FL_SECTION_SHADOW;
	fl_Layout::setType(PTX_Section); // Set the type of this strux
}

fl_HdrFtrShadow::~fl_HdrFtrShadow()
{
	_purgeLayout();
}

fp_Container* fl_HdrFtrShadow::getFirstContainer()
{
	if (m_pHdrFtrSL->getHFType() < FL_HDRFTR_FOOTER)
	{
		return m_pPage->getHeaderContainer(m_pHdrFtrSL);
	}
	else
	{
		return m_pPage->getFooterContainer(m_pHdrFtrSL);
	}
	
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return NULL;
}


fp_Container* fl_HdrFtrShadow::getLastContainer()
{
	UT_ASSERT(UT_TODO);

	return NULL;
}

fp_Container* fl_HdrFtrShadow::getNewContainer(fp_Line * pFirstLine)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return NULL;
}

fl_BlockLayout* fl_HdrFtrShadow::findMatchingBlock(fl_BlockLayout* pBL)
{
	// This routine returns the matching block within this shadow of the
	// hdrftrSectionlayout. 
	//
	fl_BlockLayout* ppBL = m_pFirstBlock;
	while(ppBL && (ppBL->getStruxDocHandle() != pBL->getStruxDocHandle()))
	{
		ppBL = ppBL->getNext();
	}
	UT_ASSERT(ppBL);
	xxx_UT_DEBUGMSG(("Search for block in shadow %x \n",this));
	return ppBL;
}
	  
void fl_HdrFtrShadow::format(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->format();
		pBL = pBL->getNext();
	}
}

/*!
 * Scans through the shadow looking for the block at the specified Document
 * Position.
 \param pos the Document position
 \return A pointer to the block containing the point. Returns NULL if no block
         is found
 */
fl_BlockLayout * fl_HdrFtrShadow::findBlockAtPosition(PT_DocPosition pos)
{
//
// Skip through the blocks in this shadow to find the one containing this
// point.
//
    fl_BlockLayout*	pBL = m_pFirstBlock;
	if(pBL == NULL)
		return NULL;
	if(pos < pBL->getPosition())
	{
//
// This corner case is that pos == position of the HdrFtr strux
//
		if(pos == (pBL->getPosition() - 1))
		{
			return pBL;
		}
		return NULL;
	}
	fl_BlockLayout* pNext = pBL->getNext();
	while(pNext != NULL && pNext->getPosition( true) < pos)
	{
		pBL = pNext;
		pNext = pNext->getNext();
	}
	if(pNext != NULL)
	{
		return pBL;
	}
	else if(pBL && pBL->getPosition() == pos)
	{
		return pBL;
	}
//
// Next corner case. See if position is inside the edittableBounds of this
// section
//
	PT_DocPosition posEnd;
	FV_View * pView = m_pLayout->getView();
	if(pView)
	{
		pView->getEditableBounds(true,posEnd);
		if(pos <= posEnd)
			return pBL;
	}
//
// Now the point MIGHT be in this last block. Use code from pd_Document
// to find out. Have to check whether we're out of docrange first
//
	m_pDoc->getBounds(true,posEnd);
	if(pos > posEnd)
		return NULL;
	PL_StruxDocHandle sdh=NULL;
	bool bres;
	bres = m_pDoc->getStruxOfTypeFromPosition(pos, PTX_Block, &sdh);
	if(bres && sdh == pBL->getStruxDocHandle())
		return pBL;
//
// Not here!!
//
	return NULL;
}

void fl_HdrFtrShadow::updateLayout(void)
{
	bool bredraw = false;
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			bredraw = true;
			pBL->format();
		}
		
		pBL = pBL->getNext();
	}
	if(bredraw == true)
	{
		//    clearScreen();
		m_pContainer->layout();
 	}
}

void fl_HdrFtrShadow::layout(void)
{
	m_pContainer->layout();
}

void fl_HdrFtrShadow::clearScreen(void)
{
	UT_ASSERT(m_pContainer);
	if(m_pContainer)
		m_pContainer->clearScreen();
}

void fl_HdrFtrShadow::redrawUpdate(void)
{
	FV_View * pView = m_pLayout->getView();
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL && (pView != NULL))
	{
		if(pBL->hasUpdatableField())
		{
			bool bReformat = pBL->recalculateFields(getDocLayout()->getRedrawCount());
			if(bReformat)
			{
				pBL->format();
			}
		}
		if(pView && pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}
		pBL = pBL->getNext();
	}
	m_pContainer->layout();
}
bool fl_HdrFtrShadow::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	setAttrPropIndex(pcrxc->getIndexAP());

	// TODO

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}


void fl_HdrFtrShadow::_lookupProperties(void)
{
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_ShadowListener::fl_ShadowListener(fl_HdrFtrSectionLayout* pHFSL, fl_HdrFtrShadow* pShadow)
{
	UT_ASSERT(pHFSL);
	UT_ASSERT(pShadow);

	m_pDoc = pHFSL->getDocLayout()->getDocument();
	m_pHFSL = pHFSL;
	m_pShadow = pShadow;
	m_bListening = false;
	m_pCurrentBL = NULL;
}

fl_ShadowListener::~fl_ShadowListener()
{
}

bool fl_ShadowListener::populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr)
{
	if (!m_bListening)
	{
		return true;
	}
	
	UT_ASSERT(m_pShadow);
	UT_DEBUGMSG(("fl_ShadowListener::populate shadow %x \n",m_pShadow));

	bool bResult = false;
	FV_View* pView = m_pHFSL->getDocLayout()->getView();
	PT_DocPosition oldPos = 0;
	//
	// We're not printing
	//
	if(pView != NULL)
	{
		oldPos = pView->getPoint();
	}
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<fl_BlockLayout *>(pL)));
		}
		PT_BlockOffset blockOffset = pcrs->getBlockOffset();
		UT_uint32 len = pcrs->getLength();

			
		bResult = m_pCurrentBL->doclistener_populateSpan(pcrs, blockOffset, len);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<fl_BlockLayout *>(pL)));
		}
		PT_BlockOffset blockOffset = pcro->getBlockOffset();

// sterwill -- is this call to getSectionLayout() needed?  pBLSL is not used.

//			fl_SectionLayout* pBLSL = m_pCurrentBL->getSectionLayout();
		bResult = m_pCurrentBL->doclistener_populateObject(blockOffset,pcro);
		goto finish_up;
	}
	
	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		//	const PX_ChangeRecord_FmtMark * pcrfm = static_cast<const PX_ChangeRecord_FmtMark *>(pcr);

		{
			fl_Layout * pL = (fl_Layout *)sfh;
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<fl_BlockLayout *>(pL)));
		}
		bResult = m_pCurrentBL->doclistener_insertFmtMark( (const PX_ChangeRecord_FmtMark *) pcr);
		goto finish_up;
	}

	default:
		UT_DEBUGMSG(("Unknown Change record = %d \n",pcr->getType())); 
		UT_ASSERT(0);
		//
		// We're not printing
		//
		if(pView != NULL)
		{
			pView->setPoint(oldPos);
		}
		return false;
	}

 finish_up:
	//
	// We're not printing
	//
	if(pView != NULL)
	{
		pView->setPoint(oldPos);
	}
	return bResult;
}

bool fl_ShadowListener::populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(m_pShadow);
	UT_DEBUGMSG(("fl_ShadowListener::populateStrux\n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	FV_View* pView = m_pHFSL->getDocLayout()->getView();
	PT_DocPosition oldPos = 0;
	if(pView != NULL)
	{
		oldPos = pView->getPoint();
	}

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
	{
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		if (m_pDoc->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				m_bListening = false;
			}
			else
			{
				if ( (0 == UT_strcmp(pszSectionType, "header"))
					|| (0 == UT_strcmp(pszSectionType, "footer"))
					 || (0 == UT_strcmp(pszSectionType, "header-first"))
					|| (0 == UT_strcmp(pszSectionType, "footer-first"))
					 || (0 == UT_strcmp(pszSectionType, "header-even"))
					|| (0 == UT_strcmp(pszSectionType, "footer-even"))
					 || (0 == UT_strcmp(pszSectionType, "header-last"))
					|| (0 == UT_strcmp(pszSectionType, "footer-last"))
					)
				{
					// TODO verify id match
					
					m_bListening = true;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			// TODO fail?
			return false;
		}
	}
	break;

	case PTX_SectionHdrFtr:
	{
		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		if (m_pDoc->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				m_bListening = false;
			}
			else
			{
				if ( (0 == UT_strcmp(pszSectionType, "header"))
					|| (0 == UT_strcmp(pszSectionType, "footer"))
					 || (0 == UT_strcmp(pszSectionType, "header-first"))
					|| (0 == UT_strcmp(pszSectionType, "footer-first"))
					 || (0 == UT_strcmp(pszSectionType, "header-even"))
					|| (0 == UT_strcmp(pszSectionType, "footer-even"))
					 || (0 == UT_strcmp(pszSectionType, "header-last"))
					|| (0 == UT_strcmp(pszSectionType, "footer-last"))
					)
				{
					// TODO verify id match
					
					m_bListening = true;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			// TODO fail?
			return false;
		}
	}
	break;

	case PTX_Block:
	{
		if (m_bListening)
		{
			// append a new BlockLayout to that SectionLayout
			fl_BlockLayout*	pBL = m_pShadow->appendBlock(sdh, pcr->getIndexAP());
			if (!pBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}
			m_pCurrentBL = pBL;	
			*psfh = (PL_StruxFmtHandle)pBL;
		}
		
	}
	break;
			
	default:
		UT_ASSERT(0);
		//
		// We're not printing
		//
		if(pView != NULL)
		{
			pView->setPoint(oldPos);
		}
		return false;
	}
	//
	// We're not printing
	//
	if(pView != NULL)
	{
		pView->setPoint(oldPos);
	}
	return true;
}

bool fl_ShadowListener::change(PL_StruxFmtHandle /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return false;
}

bool fl_ShadowListener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/,
									PL_StruxDocHandle /*sdh*/,
									PL_ListenerId /*lid*/,
									void (* /*pfnBindHandles*/)(PL_StruxDocHandle sdhNew,
																PL_ListenerId lid,
																PL_StruxFmtHandle sfhNew))
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return false;
}

bool fl_ShadowListener::signal(UT_uint32 /*iSignal*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}


