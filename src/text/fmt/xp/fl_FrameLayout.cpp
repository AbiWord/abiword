/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Martin Sevior (msevior@physics.unimelb.edu.au>
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
#include "fl_FrameLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "fp_FrameContainer.h"
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
#include "fv_View.h"
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"

fl_FrameLayout::fl_FrameLayout(FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout, SectionType iSecType,fl_ContainerType myType,PTStruxType myStruxType)
 	: fl_SectionLayout(pLayout, sdh, indexAP, iSecType,myType,myStruxType,pMyContainerLayout),
	  m_bNeedsRebuild(false),
	  m_bNeedsFormat(true),
	  m_bIsOnPage(false),
	  m_pDocSL(pDocSL),
	  m_bHasEndFrame(false)
{
	UT_ASSERT(m_pDocSL->getContainerType() == FL_CONTAINER_DOCSECTION);
}

fl_FrameLayout::~fl_FrameLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting Footlayout %x \n",this));
	_purgeLayout();
	fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(getFirstContainer());
	while(pFC)
	{
		fp_FrameContainer * pNext = static_cast<fp_FrameContainer *>(pFC->getNext());
		if(pFC == static_cast<fp_FrameContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		delete pFC;
		pFC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);
}
	
/*!
 * Returns the position in the document of the PTX_SectionFrame strux
 * This is very useful for determining the value of the footnote reference
 * and anchor. 
*/
PT_DocPosition fl_FrameLayout::getDocPosition(void) 
{
	PL_StruxDocHandle sdh = getStruxDocHandle();
    return 	m_pLayout->getDocument()->getStruxPosition(sdh);
}

/*!
 * This method returns the length of the footnote. This is such that 
 * getDocPosition() + getLength() is one value beyond the the EndFrame
 * strux
 */
UT_uint32 fl_FrameLayout::getLength(void)
{
	PT_DocPosition startPos = getDocPosition();
	PL_StruxDocHandle sdhEnd = NULL;
	PL_StruxDocHandle sdhStart = getStruxDocHandle();
	bool bres;
	if(getContainerType() == FL_CONTAINER_FOOTNOTE)
	{
		bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndFrame,&sdhEnd);
	}
	else if(getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote,&sdhEnd);
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	UT_ASSERT(bres && sdhEnd);
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = static_cast<UT_uint32>(endPos - startPos + 1); 
	return length;
}


bool fl_FrameLayout::bl_doclistener_insertEndFrame(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	// The endFrame strux actually needs a format handle to to this Frame layout.
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
	m_bHasEndFrame = true;
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(getFirstLayout());
	pBL->updateEnclosingBlockIfNeeded();
	return true;
}


/*!
 * This signals an incomplete footnote section.
 */
bool fl_FrameLayout::doclistener_deleteEndFrame( const PX_ChangeRecord_Strux * pcrx)
{
	m_bHasEndFrame = false;
	return true;
}


fl_SectionLayout * fl_FrameLayout::getSectionLayout(void) const
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


bool fl_FrameLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	return true;
}


bool fl_FrameLayout::recalculateFields(UT_uint32 iUpdateCount)
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


void fl_FrameLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_FrameLayout::updateLayout(void)
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

void fl_FrameLayout::redrawUpdate(void)
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


bool fl_FrameLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
//
// Remove all remaining structures
//
	collapse();
//	UT_ASSERT(pcrx->getStruxType()== PTX_SectionFrame);
//
// Find the block that contains this layout.
//
	PT_DocPosition prevPos = pcrx->getPosition();
	fl_BlockLayout * pEncBlock =  m_pLayout->findBlockAtPosition(prevPos);
//
// Fix the offsets for the block
//
	pEncBlock->updateOffsets(prevPos,0);

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
void fl_FrameLayout::_purgeLayout(void)
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
 * This method creates a new footnote with its properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_FrameLayout::_createFrameContainer(void)
{
	_lookupProperties();
	fp_FrameContainer * pFrameContainer = new fp_FrameContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pFrameContainer);
	setLastContainer(pFrameContainer);
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
	pFrameContainer->setWidth(iWidth);
}

/*!
  Create a new Frame container.
  \params If pPrevFrame is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Frame container
*/
fp_Container* fl_FrameLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("PLAM: creating new footnote container\n"));
	_createFrameContainer();
	m_bIsOnPage = false;
	return static_cast<fp_Container *>(getLastContainer());
}

void fl_FrameLayout::_insertFrameContainer(fp_Container * pNewFC)
{
	UT_DEBUGMSG(("inserting footnote container into container list\n"));
	fl_ContainerLayout * pUPCL = myContainingLayout();
	fl_ContainerLayout * pPrevL = static_cast<fl_ContainerLayout *>(m_pLayout->findBlockAtPosition(getDocPosition()-1));
	fp_Container * pPrevCon = NULL;
	fp_Container * pUpCon = NULL;
	fp_Page * pPage = NULL;

	// get the owning container
	if(pPrevL != NULL)
	{
		pPrevCon = pPrevL->getLastContainer();
		if(pPrevL->getContainerType() == FL_CONTAINER_BLOCK)
		{
//
// Code to find the Line that contains the footnote reference
//
			PT_DocPosition posFL = getDocPosition() - 1;
			UT_ASSERT(pPrevL->getContainerType() == FL_CONTAINER_BLOCK);
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pPrevL);
			fp_Run * pRun = pBL->getFirstRun();
			PT_DocPosition posBL = pBL->getPosition();
			while(pRun && ((posBL + pRun->getBlockOffset() + pRun->getLength()) < posFL))
			{
				pRun = pRun->getNextRun();
			}
			if(pRun && pRun->getLine())
			{
				pPrevCon = static_cast<fp_Container *>(pRun->getLine());
			}
		}
		if(pPrevCon == NULL)
		{
			pPrevCon = pPrevL->getLastContainer();
		}
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
	pNewFC->setContainer(NULL);

	// need to put onto page as well, in the appropriate place.
//	UT_ASSERT(pPage);
	if(pPage)
	{
		pPage->insertFrameContainer(static_cast<fp_FrameContainer*>(pNewFC));
		m_bIsOnPage = true;
	}
}


void fl_FrameLayout::format(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting first container is %x \n",getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	if(!m_bIsOnPage)
	{
		_insertFrameContainer(getFirstContainer());
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
	static_cast<fp_FrameContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
}

void fl_FrameLayout::_lookupProperties(void)
{
 	const PP_AttrProp* pSectionAP = NULL;

	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const XML_Char *pszFramePID = NULL;
	if(!pSectionAP || !pSectionAP->getAttribute("footnote-id",pszFramePID))
	{
		m_iFramePID = 0;
	}
	else
	{
		m_iFramePID = atoi(pszFramePID);
	}
}

void fl_FrameLayout::_localCollapse(void)
{
	// ClearScreen on our Cell. One Cell per layout.
	fp_FrameContainer *pFC = static_cast<fp_FrameContainer *>(getFirstContainer());
	if (pFC)
	{
		pFC->clearScreen();
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

void fl_FrameLayout::collapse(void)
{
	_localCollapse();
	fp_FrameContainer *pFC = static_cast<fp_FrameContainer *>(getFirstContainer());
	if (pFC)
	{
//
// Remove it from the page.
//
		if(pFC->getPage())
		{
			pFC->getPage()->removeFrameContainer(pFC);
			pFC->setPage(NULL);
		}
//
// remove it from the linked list.
//
		fp_FrameContainer * pPrev = static_cast<fp_FrameContainer *>(pFC->getPrev());
		if(pPrev)
		{
			pPrev->setNext(pFC->getNext());
		}
		if(pFC->getNext())
		{
			pFC->getNext()->setPrev(pPrev);
		}
		delete pFC;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
}

