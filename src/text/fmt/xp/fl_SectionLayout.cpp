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
	
	UT_DEBUGMSG(("SEVIOR: In get Attribute, pAP = %x looking for %s \n",pAP,pszName));
	const XML_Char* pszAtt = NULL;
	pAP->getAttribute((XML_Char*)pszName, pszAtt);
	
	UT_DEBUGMSG(("SEVIOR: In getAttribute Found %s \n",pszAtt));

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

bool fl_SectionLayout::recalculateFields(void)
{
	bool bResult = false;
	
	fl_BlockLayout*	pBL = m_pFirstBlock;

	while (pBL)
	{
		bResult = pBL->recalculateFields() || bResult;

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

bool fl_SectionLayout::bl_doclistener_insertSection(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
													PL_StruxDocHandle sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																			PL_ListenerId lid,
																			PL_StruxFmtHandle sfhNew))
{
	return pBL->doclistener_insertSection(pcrx, sdh, lid, pfnBindHandles);
}


bool fl_SectionLayout::bl_doclistener_insertHdrFtrSection(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
													PL_StruxDocHandle sdh,
													PL_ListenerId lid,
													void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																			PL_ListenerId lid,
																			PL_StruxFmtHandle sfhNew))
{
	return pBL->doclistener_insertHdrFtrSection(pcrx, sdh, lid, pfnBindHandles);
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
		FV_View * pView = m_pLayout->getView();
		pView->draw();
		pBL = pBL->getNext();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_DocSectionLayout::fl_DocSectionLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_DOC)
{
	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

	m_pHeaderSL = NULL;
	m_pFooterSL = NULL;
	m_pFirstOwnedPage = NULL;
	_lookupProperties();
}

fl_DocSectionLayout::~fl_DocSectionLayout()
{
	// NB: be careful about the order of these
	_purgeLayout();

	DELETEP(m_pHeaderSL);
	DELETEP(m_pFooterSL);
	
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
			m_pHeaderSL = NULL;
		}
		else
		{
			m_pFooterSL = NULL;
		}
		return;
	}
	const char* pszID = pHFSL->getAttribute("id");

	const char* pszAtt = NULL;

	pszAtt = getAttribute("header");
	if (
		pszAtt
		&& (0 == UT_stricmp(pszAtt, pszID))
		)
	{
		m_pHeaderSL = pHFSL;
		return;
	}
	
	pszAtt = getAttribute("footer");
	if (
		pszAtt
		&& (0 == UT_stricmp(pszAtt, pszID))
		)
	{
		m_pFooterSL = pHFSL;
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

fp_Container* fl_DocSectionLayout::getFirstContainer()
{
	return m_pFirstColumn;
}

fp_Container* fl_DocSectionLayout::getLastContainer()
{
	return m_pLastColumn;
}

fp_Container* fl_DocSectionLayout::getNewContainer(void)
{
	/*
	  This is called to create a new column (or row of same).
	*/
	fp_Page* pPage = NULL;
	fp_Column* pLastColumn = (fp_Column*) getLastContainer();
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
			pPage = m_pLayout->addNewPage(this);
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
		fl_DocSectionLayout* pPrevSL = getPrevDocSection();
		if (pPrevSL)
		{
			//
			// Sevior this code should not be needed!
			//
			fp_Column * pPrevCol = (fp_Column *) pPrevSL->getLastContainer();
			if(pPrevCol == NULL)
			{
				UT_DEBUGMSG(("BUG! BUG! Prev section has no last container! Attempting to fix this \n"));
				pPrevSL->format();
			}
			fp_Page* pTmpPage = pPrevSL->getLastContainer()->getPage();
			if (m_bForceNewPage)
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
				pPage = m_pLayout->addNewPage(this);
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
//	if(m_pHeaderSL)
//	{
//		m_pHeaderSL->format();
//		m_pHeaderSL->layout();
//	}
//	if(m_pFooterSL)
//	{
//		m_pFooterSL->format();
//		m_pFooterSL->layout();
//	}
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
	
	breakSection();

	m_pLayout->deleteEmptyColumnsAndPages();
}

void fl_DocSectionLayout::redrawUpdate(void)
{
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		if (pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}
		
		pBL = pBL->getNext();
	}
	
	breakSection();

	m_pLayout->deleteEmptyColumnsAndPages();
	
}

bool fl_DocSectionLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);

	PT_AttrPropIndex indexAP = pcrxc->getIndexAP();
	const PP_AttrProp* pAP = NULL;
			
	bool bres = (m_pDoc->getAttrProp(indexAP, &pAP) && pAP);
	UT_ASSERT(bres);
	
	const XML_Char* pszSectionType = NULL;
	pAP->getAttribute("type", pszSectionType);

	setAttrPropIndex(pcrxc->getIndexAP());

	_lookupProperties();
	//
	// Clear the header/footers first
	//
  	if(m_pHeaderSL)
		m_pHeaderSL->collapse();
  	if(m_pFooterSL)
		m_pFooterSL->collapse();

	// clear all the columns
	fp_Column* pCol = m_pFirstColumn;
	while (pCol)
	{
		pCol->clearScreen();

		pCol = pCol->getNext();
	}
	//
	// Clear the header/footers too
	//
	if(m_pHeaderSL)
		m_pHeaderSL->clearScreen();
	if(m_pFooterSL)
		m_pFooterSL->clearScreen();

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


	m_pFirstColumn = NULL;
	m_pLastColumn = NULL;

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
	updateBackgroundColor();
	if(m_pHeaderSL)
	{
		m_pHeaderSL->format();
		m_pHeaderSL->updateBackgroundColor();
		m_pHeaderSL->redrawUpdate();
	}
	if(m_pFooterSL)
	{
		m_pFooterSL->format();
		m_pFooterSL->updateBackgroundColor();
		m_pFooterSL->redrawUpdate();
	}

	FV_View* pView = m_pLayout->getView();
	if (pView)
	{
		pView->notifyListeners(AV_CHG_TYPING | AV_CHG_FMTSECTION);
	}

	return false;
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

	const char* pszLeftMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszBottomMargin = NULL;
	const char* pszFooterMargin = NULL;
	const char* pszHeaderMargin = NULL;
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

	char defaultMargin[30];
	switch(dim)
	{
	case DIM_IN:
		strcpy(defaultMargin, "1.0in");
		break;

	case DIM_CM:
		strcpy(defaultMargin, "2.54cm");
		break;

	case DIM_PI:
		strcpy(defaultMargin, "6.0pi");
		break;

	case DIM_PT:
		strcpy(defaultMargin, "72.0pt");
		break;

	case DIM_MM:
		strcpy(defaultMargin, "25.4mm");
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
		strcpy(defaultMargin, "1.0in");	// TODO: what to do with this.
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
		m_iLeftMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin);
		m_iLeftMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin);
		m_dLeftMarginUserUnits = UT_convertDimensionless(defaultMargin);
	}
	
	if(pszTopMargin && pszTopMargin[0])
	{
		m_iTopMargin = m_pLayout->getGraphics()->convertDimension(pszTopMargin);
		m_iTopMarginLayoutUnits = UT_convertToLayoutUnits(pszTopMargin);
		m_dTopMarginUserUnits = UT_convertDimensionless(pszTopMargin);
	}
	else
	{
		m_iTopMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin);
		m_iTopMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin);
		m_dTopMarginUserUnits = UT_convertDimensionless(defaultMargin);
	}

	if(pszRightMargin && pszRightMargin[0])
	{
		m_iRightMargin = m_pLayout->getGraphics()->convertDimension(pszRightMargin);
		m_iRightMarginLayoutUnits = UT_convertToLayoutUnits(pszRightMargin);
		m_dRightMarginUserUnits = UT_convertDimensionless(pszRightMargin);
	}
	else
	{
		m_iRightMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin);
		m_iRightMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin);
		m_dRightMarginUserUnits = UT_convertDimensionless(defaultMargin);
	}

	if(pszBottomMargin && pszBottomMargin[0])
	{
		m_iBottomMargin = m_pLayout->getGraphics()->convertDimension(pszBottomMargin);
		m_iBottomMarginLayoutUnits = UT_convertToLayoutUnits(pszBottomMargin);
		m_dBottomMarginUserUnits = UT_convertDimensionless(pszBottomMargin);
	}
	else
	{
		m_iBottomMargin = m_pLayout->getGraphics()->convertDimension(defaultMargin);
		m_iBottomMarginLayoutUnits = UT_convertToLayoutUnits(defaultMargin);
		m_dBottomMarginUserUnits = UT_convertDimensionless(defaultMargin);
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
	setPaperColor();
	m_bForceNewPage = false;
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
	else if( pView && pView->getGraphics()->queryProperties(GR_Graphics::DGP_SCREEN))
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

bool fl_DocSectionLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Section);

	fl_DocSectionLayout* pPrevSL = getPrevDocSection();
	if (!pPrevSL)
	{
		// TODO shouldn't this just assert?
		UT_DEBUGMSG(("no prior SectionLayout"));
		return false;
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
		m_pNext->setPrev(pPrevSL);
	}

	if(m_pHeaderSL)
	{
		DELETEP(m_pHeaderSL);
	}
	if(m_pFooterSL)
	{
		DELETEP(m_pFooterSL);
	}

	m_pLayout->removeSection(this);

	pPrevSL->format();
	
//	FV_View* pView = m_pLayout->getView();
//      if (pView)
//  	{
//  		pView->_setPoint(pcrx->getPosition());
//  	}

	delete this;			// TODO whoa!  this construct is VERY dangerous.
	
	return true;
}

void fl_DocSectionLayout::addOwnedPage(fp_Page* pPage)
{
	// TODO do we really need the vecOwnedPages member? YES!!!

	if(m_pFirstOwnedPage == NULL)
		m_pFirstOwnedPage = pPage;
	fp_Page * pPrev = m_pFirstOwnedPage;

	if (m_pHeaderSL)
	{
		if(pPrev && pPrev->getOwningSection() == this && pPrev->getHeaderP() == NULL )
			prependOwnedHeaderPage(pPrev);

		m_pHeaderSL->addPage(pPage);
	}

	if (m_pFooterSL)
	{
		if(pPrev && pPrev->getOwningSection() == this && pPrev->getFooterP() == NULL)
			prependOwnedFooterPage(pPrev);
		m_pFooterSL->addPage(pPage);
	}
}

void fl_DocSectionLayout::prependOwnedHeaderPage(fp_Page* pPage)
{
	//
	// Skip back through the pages until the first owned page of this section
	//
	fp_Page * pPrev = pPage->getPrev();
	if(pPrev && pPrev->getOwningSection() == this && pPrev->getHeaderP() == NULL)
		prependOwnedHeaderPage(pPrev);
	m_pHeaderSL->addPage(pPage);
}


void fl_DocSectionLayout::prependOwnedFooterPage(fp_Page* pPage)
{
	//
	// Skip back through the pages until the first owned page of this section
	//
	fp_Page * pPrev = pPage->getPrev();
	if(pPrev && pPrev->getOwningSection() == this && pPrev->getFooterP() == NULL)
		prependOwnedFooterPage(pPrev);
	m_pFooterSL->addPage(pPage);
}

	
void fl_DocSectionLayout::deleteOwnedPage(fp_Page* pPage)
{
	// TODO do we really need the vecOwnedPages member?
	
	if (m_pHeaderSL)
	{
		m_pHeaderSL->deletePage(pPage);
	}

	if (m_pFooterSL)
	{
		m_pFooterSL->deletePage(pPage);
	}
}

UT_sint32 fl_DocSectionLayout::breakSection(void)
{
	fl_BlockLayout* pFirstBlock = getFirstBlock();
	if (!pFirstBlock)
	{
		return 0;
	}

	fp_Line* pCurrentLine = pFirstBlock->getFirstLine();
		
	fp_Column* pCurColumn = (fp_Column*) getFirstContainer();

	while (pCurColumn)
	{
		fp_Line* pFirstLineToKeep = pCurrentLine;
		fp_Line* pLastLineToKeep = NULL;
		fp_Line* pOffendingLine = NULL;
		
		UT_sint32 iMaxColHeight = pCurColumn->getMaxHeightInLayoutUnits();
		UT_sint32 iWorkingColHeight = 0;

		fp_Line* pCurLine = pFirstLineToKeep;
		while (pCurLine)
		{
			UT_sint32 iLineHeight = pCurLine->getHeightInLayoutUnits();
			UT_sint32 iLineMarginAfter = pCurLine->getMarginAfterInLayoutUnits();
			UT_sint32 iTotalLineSpace = iLineHeight + iLineMarginAfter;

			if ((iWorkingColHeight + iTotalLineSpace) > iMaxColHeight)
			{
				pOffendingLine = pCurLine;

				/*
				  We have found the offending line (the first one which won't fit in the
				  column) and we now need to decide whether we can break the column
				  just before it.
				*/

				if (pOffendingLine == pFirstLineToKeep)
				{
					/*
					  Wow!  The very first line in this column won't fit.
					  
					  Big line.  (or maybe a small column)
					  
					  TODO what should we do here?  For now, we force it.
					*/
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
					break;
				}
			}

			pCurLine = pCurLine->getNextLineInSection();
		}

		if (pLastLineToKeep)
		{
			pCurrentLine = pLastLineToKeep->getNextLineInSection();
		}
		else
		{
			pCurrentLine = NULL;
		}
		
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

		fp_Column* pNextColumn = NULL;
		
		if (pLastLineToKeep)
		{
			UT_ASSERT(pLastLineToKeep->getContainer() == pCurColumn);
			
			if (pCurColumn->getLastLine() != pLastLineToKeep)
			{
				// make sure there is a next column
				pNextColumn = pCurColumn->getNext();
				if (!pNextColumn)
				{
					pNextColumn = (fp_Column*) getNewContainer();
				}

				pCurColumn->bumpLines(pLastLineToKeep);
			}
		}

		UT_ASSERT((!pLastLineToKeep) || (pCurColumn->getLastLine() == pLastLineToKeep));
			
		pCurColumn->layout();

		pCurColumn = pCurColumn->getNext();
	}

	return 0; // TODO return code
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

struct _PageHdrFtrShadowPair
{
	fp_Page*			pPage;
	fl_HdrFtrShadow*	pShadow;
};

fl_HdrFtrSectionLayout::fl_HdrFtrSectionLayout(HdrFtrType iHFType, FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_HDRFTR)
{
	m_pDocSL = pDocSL;
	m_iHFType = iHFType;
	m_iType = FL_SECTION_HDRFTR;
	m_pVirContainer = NULL;
	fl_Layout::setType(PTX_SectionHdrFtr); // Set the type of this strux
}

fl_HdrFtrSectionLayout::~fl_HdrFtrSectionLayout()
{
	
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		delete pPair->pShadow;
	}
	_purgeLayout();
	DELETEP(m_pVirContainer);
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
	UT_VECTOR_PURGEALL(struct _PageHdrFtrShadowPair*, m_vecPages);
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		fp_Page * ppPage = pPair->pPage;
		delete pPair->pShadow;
		if (getHFType() == FL_HDRFTR_HEADER)
		{
			ppPage->removeHeader();
		}
		else 
		{
			ppPage->removeFooter();
		}
		delete pPair;
	}
	m_vecPages.clear();
	DELETEP(m_pVirContainer);
}

bool fl_HdrFtrSectionLayout::recalculateFields(void)
{
	bool bResult = false;
	
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		UT_ASSERT(pPair->pShadow);
		bResult = pPair->pShadow->recalculateFields() || bResult;
	}

	return bResult;
}


fl_HdrFtrShadow * fl_HdrFtrSectionLayout::getFirstShadow(void)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	if(iCount != 0)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(0);
		return pPair->pShadow;
	}
	return NULL;
}

fp_Container* fl_HdrFtrSectionLayout::getFirstContainer()
{
	return m_pVirContainer;
}


fp_Container* fl_HdrFtrSectionLayout::getLastContainer()
{
	return m_pVirContainer;
}

fp_Container* fl_HdrFtrSectionLayout::getNewContainer(void)
{
	DELETEP(m_pVirContainer);
	UT_sint32 iWidth = m_pDocSL->getFirstContainer()->getPage()->getWidth();
	UT_sint32 iWidthLayout = m_pDocSL->getFirstContainer()->getPage()->getWidthInLayoutUnits() - m_pDocSL->getLeftMarginInLayoutUnits() - m_pDocSL->getRightMarginInLayoutUnits();
	m_pVirContainer = (fp_Container *) new fp_VirtualContainer(iWidth,iWidthLayout, (fl_SectionLayout *) this);
	return m_pVirContainer;
}

fl_HdrFtrShadow *  fl_HdrFtrSectionLayout::findShadow(fp_Page* pPage)
{
       UT_uint32 iPage = _findShadow(pPage);
       if(iPage < 0)
	        return NULL;
       struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(iPage);
       return pPair->pShadow;
}

UT_sint32 fl_HdrFtrSectionLayout::_findShadow(fp_Page* pPage)
{
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		if (pPair->pPage == pPage)
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
void fl_HdrFtrSectionLayout::changeStrux( fl_DocSectionLayout * pSL)
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
	UT_ASSERT(0 > _findShadow(pPage));
	UT_sint32 icnt = m_vecPages.getItemCount();
	if(_findShadow(pPage) > -1)
		return;
	struct _PageHdrFtrShadowPair* pPair = new _PageHdrFtrShadowPair;
	// TODO outofmem
	pPair->pPage = pPage;
	pPair->pShadow = new fl_HdrFtrShadow(m_pLayout, pPage, this, m_sdh, m_apIndex);
	//
	// Make sure we register the shadow before populating it.
	//
	m_vecPages.addItem(pPair);
	//
	// Populate the shadow
	//
	icnt = m_vecPages.getItemCount();
	fl_ShadowListener* pShadowListener = new fl_ShadowListener(this, pPair->pShadow);
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
	PD_DocumentRange * docRange = new PD_DocumentRange(m_pDoc,posStart,posEnd);
	m_pDoc->tellListenerSubset(pShadowListener, docRange);
	delete docRange;
	delete pShadowListener;
}

void fl_HdrFtrSectionLayout::deletePage(fp_Page* pPage)
{
	UT_sint32 iShadow = _findShadow(pPage);
//
// This shadow might have already been deleted via the collapse method
//
	if(iShadow <= 0)
		return;
	struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(iShadow);
	UT_ASSERT(pPair);

	UT_ASSERT(pPair->pShadow);


	fp_Page * ppPage = pPair->pPage; 
	delete pPair->pShadow;
	if (getHFType() == FL_HDRFTR_HEADER)
	{
		ppPage->removeHeader();
	}
	else 
	{
		ppPage->removeFooter();
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
	fl_BlockLayout*	pBL = m_pFirstBlock;
	while (pBL)
	{
		pBL->format();
		pBL = pBL->getNext();
	}
}

/*!
 *  Just collapse the HdrFtrSectionLayout blocks for an insertBlock method.
 *  This removes all lines and references to containers but leaves the blocks
 *  and runs intack.
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
	return ppBL;
}

/*!
 * Format the overall HdrFtrSectionLayout in it's virtual container.
 * Also check that all the correct pages have been found for this HdrFtr. Then
 * format the Shadows.
 */ 
void fl_HdrFtrSectionLayout::format(void)
{
	localFormat();
	UT_uint32 iCount = m_vecPages.getItemCount();
	//
	// Fail safe code to add pages if we don't have them.
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
		if(pNewPage != NULL && pNewPage != pOldPage)
		{
			fl_DocSectionLayout* pDocSec = pNewPage->getOwningSection();
			if(pDocSec == m_pDocSL && _findShadow(pNewPage) < 0)
			{
				addPage(pNewPage);
			}
		}
		pCol = pCol->getNext();
	}
	iCount = m_vecPages.getItemCount();
	
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		pPair->pShadow->format();
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
		if(m_pVirContainer)
			static_cast<fp_VirtualContainer *>(m_pVirContainer)->layout();
 	}

	//
	// update Just the  blocks in the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->pShadow->updateLayout();
	}
}

/*!
 * Layout the overall HdrFtr and everything underneath it.
 */
void fl_HdrFtrSectionLayout::layout(void)
{
    if(m_pVirContainer)
	static_cast<fp_VirtualContainer *>(m_pVirContainer)->layout();
	//
	// update the shadowlayouts
	//
  	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->pShadow->layout();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->pShadow->updateBackgroundColor();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->pShadow->clearScreen();
	}
}

void fl_HdrFtrSectionLayout::redrawUpdate(void)
{
//
// Do another layout but don't redraw.
//
	if(m_pVirContainer)
		static_cast<fp_VirtualContainer *>(m_pVirContainer)->layout();
	//
	// Don't need to draw here since this is never displayed on the screen?
	// 
	UT_uint32 iCount = m_vecPages.getItemCount();
	for (UT_uint32 i=0; i<iCount; i++)
	{
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		pPair->pShadow->redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		UT_DEBUGMSG(("SEVIOR: SectionLayout for shadow %d is %x \n",i,pShadowBL->getSectionLayout()));
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteStrux(pcrx)
			&& bResult;
	}
	// Update the overall block too.

	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_deleteStrux(pcrx)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();

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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
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

bool fl_HdrFtrSectionLayout::bl_doclistener_insertBlock(fl_BlockLayout* pBL, const PX_ChangeRecord_Strux * pcrx,
														PL_StruxDocHandle sdh,
														PL_ListenerId lid,
														void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																				PL_ListenerId lid,
																				PL_StruxFmtHandle sfhNew))
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		// Find matching block in this shadow.

		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_insertBlock(pcrx,sdh,lid,pfnBindHandles)
			&& bResult;
	}
//
// Find Matching Block in this HdrFtrSectionLayout!!
//
	fl_BlockLayout * ppBL = findMatchingBlock(pBL);

//	localFormat();
	m_pDoc->allowChangeInsPoint();

    bResult = ppBL->doclistener_insertBlock(pcrx,sdh,lid,pfnBindHandles)
		&& bResult;
//
// Mark the Block as HdrFtr
//
	ppBL->getNext()->setHdrFtr();

	format();
	updateLayout();
	redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);

		bResult = pPair->pShadow->bl_doclistener_insertSection(pBL, pcrx, sdh, lid, pfnBindHandles)
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_insertObject(pcro)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_insertObject(pcro)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteObject(pcro)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_deleteObject(pcro)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_changeObject(pcroc)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_changeObject(pcroc)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_insertFmtMark(pcrfm)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_insertFmtMark(pcrfm)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_deleteFmtMark(pcrfm)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
	bResult = pBL->doclistener_deleteFmtMark(pcrfm)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();
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
		struct _PageHdrFtrShadowPair* pPair = (struct _PageHdrFtrShadowPair*) m_vecPages.getNthItem(i);
		// Find matching block in this shadow.
		pShadowBL = pPair->pShadow->findMatchingBlock(pBL);
		bResult = pShadowBL->doclistener_changeFmtMark(pcrfmc)
			&& bResult;
	}
	// Update the overall block too.
	m_pDoc->allowChangeInsPoint();
	pBL = findMatchingBlock(pBL);
   	bResult = pBL->doclistener_changeFmtMark(pcrfmc)
		&& bResult;

	format();
	updateLayout();
	redrawUpdate();

	return bResult;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_HdrFtrShadow::fl_HdrFtrShadow(FL_DocLayout* pLayout, fp_Page* pPage, fl_HdrFtrSectionLayout* pHdrFtrSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP)
	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_SHADOW)
{
	m_pHdrFtrSL = pHdrFtrSL;
	m_pPage = pPage;
	if (m_pHdrFtrSL->getHFType() == FL_HDRFTR_HEADER)
	{
		m_pContainer = m_pPage->getHeaderContainer(m_pHdrFtrSL);
	}
	else if (m_pHdrFtrSL->getHFType() == FL_HDRFTR_FOOTER)
	{
		m_pContainer =  m_pPage->getFooterContainer(m_pHdrFtrSL);
	}
	m_iType = FL_SECTION_SHADOW;
	fl_Layout::setType(PTX_Section); // Set the type of this strux
}

fl_HdrFtrShadow::~fl_HdrFtrShadow()
{
	_purgeLayout();
}

fp_Container* fl_HdrFtrShadow::getFirstContainer()
{
	if (m_pHdrFtrSL->getHFType() == FL_HDRFTR_HEADER)
	{
		return m_pPage->getHeaderContainer(m_pHdrFtrSL);
	}
	else if (m_pHdrFtrSL->getHFType() == FL_HDRFTR_FOOTER)
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

fp_Container* fl_HdrFtrShadow::getNewContainer(void)
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
		return NULL;
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
//
// Now the point MIGHT be in this last block. Use code from pd_Document
// to find out. Have to check whether we're out of docrange first
//
	PT_DocPosition posEnd;
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
	while (pBL)
	{
		if(pView)
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
//	UT_DEBUGMSG(("fl_ShadowListener::populate shadow %x \n",m_pShadow));

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

// sterwill -- is this call to getSectionLayout() needed?  pBLSL is not used.
			
//			fl_SectionLayout* pBLSL = m_pCurrentBL->getSectionLayout();
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
//	UT_DEBUGMSG(("fl_ShadowListener::populateStrux\n"));

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
				if (
					(0 == UT_strcmp(pszSectionType, "header"))
					|| (0 == UT_strcmp(pszSectionType, "footer"))
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
				if (
					(0 == UT_strcmp(pszSectionType, "header"))
					|| (0 == UT_strcmp(pszSectionType, "footer"))
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


