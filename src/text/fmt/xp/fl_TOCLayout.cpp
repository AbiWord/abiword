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
//	m_pLayout->removeTOC(this); // Not implemented yet
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


bool fl_TOCLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
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

