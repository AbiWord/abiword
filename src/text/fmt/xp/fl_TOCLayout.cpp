/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior (msevior@physics.unimelb.edu.au>
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
#include "fl_TOCLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "fp_TOCContainer.h"
#include "fp_ContainerObject.h"
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
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "fv_View.h"
#include "pd_Style.h"

fl_TOCLayout::fl_TOCLayout(FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout) 
 	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_TOC,FL_CONTAINER_TOC,PTX_SectionTOC,pMyContainerLayout),
	  m_bNeedsRebuild(false),
	  m_bNeedsFormat(true),
	  m_bIsOnPage(false),
	  m_pDocSL(pDocSL),
	  m_bHasEndTOC(false)
{
	UT_ASSERT(m_pDocSL->getContainerType() == FL_CONTAINER_DOCSECTION);
//  m_pLayout->addTOC(this);  // Not implemented yet
	_createTOCContainer();
}

fl_TOCLayout::~fl_TOCLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting TOClayout %x \n",this));
	_purgeLayout();
	fp_TOCContainer * pTC = static_cast<fp_TOCContainer *>(getFirstContainer());
	while(pTC)
	{
		fp_TOCContainer * pNext = static_cast<fp_TOCContainer *>(pTC->getNext());
		if(pTC == static_cast<fp_TOCContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		delete pTC;
		pTC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);
	m_pLayout->removeTOC(this);
}
	
/*!
 * Returns the position in the document of the PTX_SectionTOC strux
 * This is very useful for determining the value of the footnote reference
 * and anchor. 
*/
PT_DocPosition fl_TOCLayout::getDocPosition(void) 
{
	PL_StruxDocHandle sdh = getStruxDocHandle();
    return 	m_pLayout->getDocument()->getStruxPosition(sdh);
}

/*!
 * This method returns the length of the footnote. This is such that 
 * getDocPosition() + getLength() is one value beyond the the EndFootnote
 * strux
 */
UT_uint32 fl_TOCLayout::getLength(void)
{
	PT_DocPosition startPos = getDocPosition();
	PL_StruxDocHandle sdhEnd = NULL;
	PL_StruxDocHandle sdhStart = getStruxDocHandle();
	bool bres;
	bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndTOC,&sdhEnd);
	UT_ASSERT(bres && sdhEnd);
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = static_cast<UT_uint32>(endPos - startPos + 1); 
	return length;
}


bool fl_TOCLayout::bl_doclistener_insertEndTOC(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	// The endFootnote strux actually needs a format handle to to this Footnote layout.
	// so we bind to this layout.

		
	PL_StruxFmtHandle sfhNew = static_cast<PL_StruxFmtHandle>(this);
	pfnBindHandles(sdh,lid,sfhNew);

//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() +  fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	m_bHasEndTOC = true;
	return true;
}


/*!
 * This signals an incomplete footnote section.
 */
bool fl_TOCLayout::doclistener_deleteEndTOC( const PX_ChangeRecord_Strux * pcrx)
{
	m_bHasEndTOC = false;
	return true;
}


fl_SectionLayout * fl_TOCLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return static_cast<fl_SectionLayout *>(pDSL);
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}

bool fl_TOCLayout::addBlock(fl_BlockLayout * pBlock)
{
	UT_UTF8String sStyle;
	pBlock->getStyle(sStyle);
	if(sStyle == m_sSourceStyle1)
	{
		_addBlockInVec(pBlock,&m_vecBlock1, m_sDestStyle1);
		return true;
	}
	if(sStyle == m_sSourceStyle2)
	{
		_addBlockInVec(pBlock,&m_vecBlock2,m_sDestStyle2);
		return true;
	}
	if(sStyle == m_sSourceStyle3)
	{
		_addBlockInVec(pBlock,&m_vecBlock3,m_sDestStyle3);
		return true;
	}
	if(sStyle == m_sSourceStyle4)
	{
		_addBlockInVec(pBlock,&m_vecBlock4,m_sDestStyle4);
		return true;
	}
	return false;
}

void fl_TOCLayout::_addBlockInVec(fl_BlockLayout * pBlock, UT_Vector * pVecBlocks, UT_UTF8String & sStyle)
{
// First find where to put the block.
	PT_DocPosition posNew = pBlock->getPosition();
	fl_BlockLayout * pPrevBL = NULL;
	UT_sint32 i = 0;
	bool bFound = false;
	for(i=0; i< static_cast<UT_sint32>(pVecBlocks->getItemCount()); i++)
	{
		pPrevBL = static_cast<fl_BlockLayout *>(pVecBlocks->getNthItem(i));
		if(pPrevBL->getPosition() > posNew)
		{
			bFound = true;
			break;
		}
	}
	UT_sint32 iThisVec = i;
	for(i=0; i< static_cast<UT_sint32>(m_vecAllBlocks.getItemCount()); i++)
	{
		pPrevBL = static_cast<fl_BlockLayout *>(m_vecAllBlocks.getNthItem(i));
		if(pPrevBL->getPosition() > posNew)
		{
			bFound = true;
			break;
		}
	}
	if(bFound)
	{
		if(i > 0)
		{
			pPrevBL =  static_cast<fl_BlockLayout *>(pVecBlocks->getNthItem(i-1));
		}
		else
		{
			pPrevBL = NULL;
		}
	}
	UT_sint32 iAllBlocks = i;
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(sStyle.utf8_str(),&pStyle);
	fl_TOCListener * pListen = new fl_TOCListener(this,pPrevBL,pStyle);
	PT_DocPosition posStart,posEnd;
	posStart = pBlock->getPosition(true);
	posEnd = posStart + static_cast<PT_DocPosition>(pBlock->getLength()) -1;
	PD_DocumentRange * docRange = new PD_DocumentRange(m_pDoc,posStart,posEnd);
	m_pDoc->tellListenerSubset(pListen, docRange);
	delete docRange;
	delete pListen;
	fl_BlockLayout * pNewBlock;
	if(pPrevBL)
	{
		pNewBlock = static_cast<fl_BlockLayout *>(pPrevBL->getNext());
	}
	else
	{
		pNewBlock = static_cast<fl_BlockLayout *>(getFirstLayout());
	}
//
// OK Now add the block to out vectors.
//
	if(iAllBlocks == 0)
	{
		m_vecAllBlocks.insertItemAt(static_cast<void *>(pNewBlock),0);
	}
	else if (iAllBlocks < static_cast<UT_sint32>(m_vecAllBlocks.getItemCount()-1))
	{
		m_vecAllBlocks.insertItemAt(static_cast<void *>(pNewBlock),iAllBlocks+1);
	}
	else
	{
		m_vecAllBlocks.addItem(static_cast<void *>(pNewBlock));
	}
	if(iThisVec == 0)
	{
		pVecBlocks->insertItemAt(static_cast<void *>(pNewBlock),0);
	}
	else if (iThisVec < static_cast<UT_sint32>(pVecBlocks->getItemCount()-1))
	{
		pVecBlocks->insertItemAt(static_cast<void *>(pNewBlock),iAllBlocks+1);
	}
	else
	{
		pVecBlocks->addItem(static_cast<void *>(pNewBlock));
	}
}

bool fl_TOCLayout::removeBlock(fl_BlockLayout * pBlock)
{
	return true;
}


bool fl_TOCLayout::isStyleInTOC(UT_UTF8String & sStyle)
{
	if(sStyle == m_sSourceStyle1)
	{
		return true;
	}
	if(sStyle == m_sSourceStyle2)
	{
		return true;
	}
	if(sStyle == m_sSourceStyle3)
	{
		return true;
	}
	if(sStyle == m_sSourceStyle4)
	{
		return true;
	}
	return false;
}



bool fl_TOCLayout::isBlockInTOC(fl_BlockLayout * pBlock)
{
	PL_StruxDocHandle sdh = pBlock->getStruxDocHandle();
	UT_sint32 i = 0;
	for(i=0; i< static_cast<UT_sint32>(m_vecBlock1.getItemCount()); i++)
	{
		fl_BlockLayout *pBL = static_cast<fl_BlockLayout *>(m_vecBlock1.getNthItem(i));
		if(pBL->getStruxDocHandle() == sdh)
		{
			return true;
		}
	}
	for(i=0; i< static_cast<UT_sint32>(m_vecBlock2.getItemCount()); i++)
	{
		fl_BlockLayout *pBL = static_cast<fl_BlockLayout *>(m_vecBlock2.getNthItem(i));
		if(pBL->getStruxDocHandle() == sdh)
		{
			return true;
		}
	}
	for(i=0; i< static_cast<UT_sint32>(m_vecBlock3.getItemCount()); i++)
	{
		fl_BlockLayout *pBL = static_cast<fl_BlockLayout *>(m_vecBlock3.getNthItem(i));
		if(pBL->getStruxDocHandle() == sdh)
		{
			return true;
		}
	}
	for(i=0; i< static_cast<UT_sint32>(m_vecBlock4.getItemCount()); i++)
	{
		fl_BlockLayout *pBL = static_cast<fl_BlockLayout *>(m_vecBlock4.getNthItem(i));
		if(pBL->getStruxDocHandle() == sdh)
		{
			return true;
		}
	}
	return false;
}

fl_BlockLayout * fl_TOCLayout::getMatchingBlock(fl_BlockLayout * pBlock)
{
	return pBlock;
}

bool fl_TOCLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	_purgeLayout();
	_lookupProperties();
	_createTOCContainer();
	return true;
}


bool fl_TOCLayout::recalculateFields(UT_uint32 iUpdateCount)
{

	bool bResult = false;
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		bResult = pBL->recalculateFields(iUpdateCount) || bResult;
		pBL = pBL->getNext();
	}
	return bResult;
}


void fl_TOCLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_TOCLayout::updateLayout(void)
{
	if(needsReformat())
	{
		format();
	}
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsReformat())
		{
			pBL->format();
		}

		pBL = pBL->getNext();
	}
}

void fl_TOCLayout::redrawUpdate(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		if (pBL->needsRedraw())
		{
			pBL->redrawUpdate();
		}

		pBL = pBL->getNext();
	}
}


bool fl_TOCLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
//
// Remove all remaining structures
//
	collapse();
//	UT_ASSERT(pcrx->getStruxType()== PTX_SectionTOC);
//

	fl_ContainerLayout * pPrev = getPrev();
	fl_ContainerLayout * pNext = getNext();

	if(pPrev != NULL)
	{
		pPrev->setNext(pNext);
	}
	else
	{
		myContainingLayout()->setFirstLayout(pNext);
	}
	if(pNext != NULL)
	{
		pNext->setPrev(pPrev);
	}
	else
	{
		myContainingLayout()->setLastLayout(pPrev);
	}
	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}


/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_TOCLayout::_purgeLayout(void)
{
	UT_DEBUGMSG(("embedLayout: purge \n"));
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
	m_vecAllBlocks.clear();
	m_vecBlock1.clear();
	m_vecBlock2.clear();
	m_vecBlock3.clear();
	m_vecBlock4.clear();
}


/*!
 * This method creates a new TOC.
 */
void fl_TOCLayout::_createTOCContainer(void)
{
	_lookupProperties();
	fp_TOCContainer * pTOCContainer = new fp_TOCContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pTOCContainer);
	setLastContainer(pTOCContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	UT_ASSERT(pDSL != NULL);

	fp_Container * pCon = pCL->getLastContainer();
	UT_ASSERT(pCon);
	UT_sint32 iWidth = pCon->getPage()->getWidth();
	iWidth = iWidth - pDSL->getLeftMargin() - pDSL->getRightMargin();
	pTOCContainer->setWidth(iWidth);
	m_pLayout->fillTOC(this);
}

/*!
  Create a new TOC container.
  \return The newly created TOC container
*/
fp_Container* fl_TOCLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("creating new TOC Physical container\n"));
	_createTOCContainer();
	return static_cast<fp_Container *>(getLastContainer());
}

void fl_TOCLayout::_insertTOCContainer(fp_Container * pNewTOC)
{
	UT_DEBUGMSG(("inserting TOC container into container list\n"));
	fl_ContainerLayout * pUPCL = myContainingLayout();
	fl_ContainerLayout * pPrevL = static_cast<fl_ContainerLayout *>(m_pLayout->findBlockAtPosition(getDocPosition()-1));
	fp_Container * pPrevCon = NULL;
	fp_Container * pUpCon = NULL;
	fp_Page * pPage = NULL;

	// get the owning container
	if(pPrevL != NULL)
	{
		pPrevCon = pPrevL->getLastContainer();
		pUpCon = pPrevCon->getContainer();
	}
	else
	{
		pUpCon = pUPCL->getLastContainer();
	}
	if(pPrevCon)
	{
		pPage = pPrevCon->getPage();
	}
	else
	{
		pPage = pUpCon->getPage();
	}
//
// FIXME
//
	pNewTOC->setContainer(NULL);
}


void fl_TOCLayout::format(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting TOC container is %x \n",getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	fl_ContainerLayout*	pBL = getFirstLayout();
	
	while (pBL)
	{
		pBL->format();
		UT_sint32 count = 0;
		while(pBL->getLastContainer() == NULL || pBL->getFirstContainer()==NULL)
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
	static_cast<fp_TOCContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
}

void fl_TOCLayout::_lookupProperties(void)
{
 	const PP_AttrProp* pSectionAP = NULL;

	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const XML_Char *pszTOCPID = NULL;
	if(!pSectionAP || !pSectionAP->getAttribute("toc-id",pszTOCPID))
	{
		m_iTOCPID = 0;
	}
	else
	{
		m_iTOCPID = atoi(pszTOCPID);
	}
}

void fl_TOCLayout::_localCollapse(void)
{
	// ClearScreen on our Cell. One Cell per layout.
	fp_TOCContainer *pTC = static_cast<fp_TOCContainer *>(getFirstContainer());
	if (pTC)
	{
		pTC->clearScreen();
	}

	// get rid of all the layout information for every containerLayout
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->collapse();
		pCL = pCL->getNext();
	}
	m_bNeedsReformat = true;
}

void fl_TOCLayout::collapse(void)
{
	_localCollapse();
	fp_TOCContainer *pTC = static_cast<fp_TOCContainer *>(getFirstContainer());
	if (pTC)
	{
//
// remove it from the linked list.
//
		fp_TOCContainer * pPrev = static_cast<fp_TOCContainer *>(pTC->getPrev());
		if(pPrev)
		{
			pPrev->setNext(pTC->getNext());
		}
		if(pTC->getNext())
		{
			pTC->getNext()->setPrev(pPrev);
		}
		delete pTC;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

fl_TOCListener::fl_TOCListener(fl_TOCLayout* pTOCL, fl_BlockLayout* pPrevBL, PD_Style * pStyle)
{
	UT_ASSERT(pTOCL);

	m_pDoc = pTOCL->getDocLayout()->getDocument();
	m_pTOCL = pTOCL;
	m_pPrevBL = pPrevBL;
	m_bListening = false;
	m_pCurrentBL = NULL;
	m_pStyle = pStyle;
}

fl_TOCListener::~fl_TOCListener()
{
}

bool fl_TOCListener::populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr)
{
	if (!m_bListening)
	{
		return true;
	}

	UT_ASSERT(m_pTOCL);
	UT_DEBUGMSG(("fl_TOCListener::populate block %x \n",m_pCurrentBL));

	bool bResult = false;
	FV_View* pView = m_pTOCL->getDocLayout()->getView();
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
			const fl_Layout * pL = static_cast<const fl_Layout *>(sfh);
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(pL)));
		}
		PT_BlockOffset blockOffset = pcrs->getBlockOffset();
		UT_uint32 len = pcrs->getLength();


		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_populateSpan(pcrs, blockOffset, len);
		goto finish_up;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

		{
			const fl_Layout * pL = static_cast<const fl_Layout *>(sfh);
			UT_ASSERT(pL->getType() == PTX_Block);
			UT_ASSERT(m_pCurrentBL == (static_cast<const fl_ContainerLayout *>(pL)));
		}
		PT_BlockOffset blockOffset = pcro->getBlockOffset();

// sterwill -- is this call to getSectionLayout() needed?  pBLSL is not used.

//			fl_SectionLayout* pBLSL = m_pCurrentBL->getSectionLayout();
		bResult = static_cast<fl_BlockLayout *>(m_pCurrentBL)->doclistener_populateObject(blockOffset,pcro);
		goto finish_up;
	}
	default:
		UT_DEBUGMSG(("Unknown Change record = %d \n",pcr->getType()));
		//
		// We're not printing
		//
		if(pView != NULL)
		{
			pView->setPoint(oldPos);
		}
		return true;
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

bool fl_TOCListener::populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(m_pTOCL);
	UT_DEBUGMSG(("fl_ShadowListener::populateStrux\n"));

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	PT_AttrPropIndex iTOC = m_pStyle->getIndexAP();
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	m_bListening = true;
	switch (pcrx->getStruxType())
	{
	case PTX_Block:
	{
		if (m_bListening)
		{
			// append a new BlockLayout to that SectionLayout
			fl_ContainerLayout*	pBL = m_pTOCL->insert(sdh,m_pPrevBL, iTOC,FL_CONTAINER_BLOCK);
			UT_DEBUGMSG(("New TOC block %x created and set as current \n",pBL));
			if (!pBL)
			{
				UT_DEBUGMSG(("no memory for BlockLayout"));
				return false;
			}
			m_pCurrentBL = pBL;
			*psfh = static_cast<PL_StruxFmtHandle>(pBL);
		}

	}
	break;

	default:
		UT_ASSERT(0);
		return false;
	}
	//
	// We're not printing
	//
	return true;
}

bool fl_TOCListener::change(PL_StruxFmtHandle /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool fl_TOCListener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

bool fl_TOCListener::signal(UT_uint32 /*iSignal*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return false;
}


