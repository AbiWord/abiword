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
#include "fl_FootnoteLayout.h"
#include "fl_Layout.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fb_LineBreaker.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fp_Column.h"
#include "fp_FootnoteContainer.h"
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

fl_FootnoteLayout::fl_FootnoteLayout(FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout)
 	: fl_SectionLayout(pLayout, sdh, indexAP, FL_SECTION_FOOTNOTE,FL_CONTAINER_FOOTNOTE,PTX_SectionFootnote,pMyContainerLayout),
	  m_pDocSL(pDocSL),
	  m_bNeedsFormat(true),
	  m_bNeedsRebuild(false),
	  m_iFootnotePID(0),
	  m_bHasEndFootnote(false),
	  m_bIsOnPage(false)
{
	m_pLayout->addFootnote(this);
	_createFootnoteContainer();
}

fl_FootnoteLayout::~fl_FootnoteLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting Footlayout %x \n",this));
	_purgeLayout();
	fp_FootnoteContainer * pFC = (fp_FootnoteContainer *) getFirstContainer();
	while(pFC)
	{
		fp_FootnoteContainer * pNext = (fp_FootnoteContainer *) pFC->getNext();
		if(pFC == (fp_FootnoteContainer *) getLastContainer())
		{
			pNext = NULL;
		}
		delete pFC;
		pFC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);
	m_pLayout->removeFootnote(this);
}

/*!
 * This method creates a new footnote with its properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_FootnoteLayout::_createFootnoteContainer(void)
{
	_lookupProperties();
	fp_FootnoteContainer * pFootnoteContainer = new fp_FootnoteContainer((fl_SectionLayout *) this);
	setFirstContainer(pFootnoteContainer);
	setLastContainer(pFootnoteContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	UT_ASSERT(pDSL != NULL);

	fp_Container * pCon = pCL->getLastContainer();
	UT_ASSERT(pCon);
	UT_sint32 iWidth = pCon->getWidth();
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	UT_sint32 iWidthLayout = pCon->getWidthInLayoutUnits();
#endif
	if(iWidth == 0)
	{
		iWidth = pCon->getPage()->getWidth();
		pCon->setWidth(iWidth);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
		iWidthLayout = pCon->getPage()->getWidthInLayoutUnits();
		pCon->setWidthInLayoutUnits(iWidthLayout);
#endif
	}
	pFootnoteContainer->setWidth(iWidth);
#if !defined(WITH_PANGO) && defined(USE_LAYOUT_UNITS)
	pFootnoteContainer->setWidthInLayoutUnits(iWidthLayout);
#endif
}
	
/*!
 * Returns the position in the document of the PTX_SectionFootnote strux
 * This is very useful for determining the value of the footnote reference
 * and anchor. 
*/
PT_DocPosition fl_FootnoteLayout::getDocPosition(void) 
{
	PL_StruxDocHandle sdh = getStruxDocHandle();
    return 	m_pLayout->getDocument()->getStruxPosition(sdh);
}

/*!
 * This method returns the length of the footnote. This is such that 
 * getDocPosition() + getLength() is one value beyond the the EndFootnote
 * strux
 */
UT_uint32 fl_FootnoteLayout::getLength(void)
{
	PT_DocPosition startPos = getDocPosition();
	PL_StruxDocHandle sdhEnd = NULL;
	PL_StruxDocHandle sdhStart = getStruxDocHandle();
	bool bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote,&sdhEnd);
	UT_ASSERT(bres && sdhEnd);
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = (UT_uint32) (endPos - startPos + 1); 
	return length;
}

bool fl_FootnoteLayout::bl_doclistener_insertEndFootnote(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
{
	// The endFootnote strux actually needs a format handle to to this Footnote layout.
	// so we bind to this layout.

		
	PL_StruxFmtHandle sfhNew = (PL_StruxFmtHandle) this;
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
	m_bHasEndFootnote = true;
	fl_BlockLayout * pBL = (fl_BlockLayout *) getFirstLayout();
	pBL->updateEnclosingBlockIfNeeded();
	return true;
}

/*!
 * This signals an incomplete footnote section.
 */
bool fl_FootnoteLayout::doclistener_deleteEndFootnote( const PX_ChangeRecord_Strux * pcrx)
{
	m_bHasEndFootnote = false;
	return true;
}

fl_SectionLayout * fl_FootnoteLayout::getSectionLayout(void) const
{
	fl_ContainerLayout * pDSL = myContainingLayout();
	while(pDSL)
	{
		if(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION)
		{
			return (fl_SectionLayout *) pDSL;
		}
		pDSL = pDSL->myContainingLayout();
	}
	return NULL;
}

/*!
  Create a new Footnote container.
  \params If pPrevFootnote is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Footnote container
*/
fp_Container* fl_FootnoteLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("PLAM: creating new footnote container\n"));
	_createFootnoteContainer();
	m_bIsOnPage = false;
	return (fp_Container *) getLastContainer();
}

void fl_FootnoteLayout::_insertFootnoteContainer(fp_Container * pNewFC)
{
	UT_DEBUGMSG(("inserting footnote container into container list\n"));
	fl_ContainerLayout * pUPCL = myContainingLayout();
	fl_ContainerLayout * pPrevL = (fl_ContainerLayout *) m_pLayout->findBlockAtPosition(getDocPosition()-1);
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
			fl_BlockLayout * pBL = (fl_BlockLayout *) pPrevL;
			fp_Run * pRun = pBL->getFirstRun();
			PT_DocPosition posBL = pBL->getPosition();
			while(pRun && ((posBL + pRun->getBlockOffset() + pRun->getLength()) < posFL))
			{
				pRun = pRun->getNext();
			}
			if(pRun && pRun->getLine())
			{
				pPrevCon = (fp_Container *) pRun->getLine();
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
	UT_ASSERT(pPage);
	pPage->insertFootnoteContainer((fp_FootnoteContainer*)pNewFC);
	m_bIsOnPage = true;
}


void fl_FootnoteLayout::format(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting first container is %x \n",getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	if(!m_bIsOnPage)
	{
		_insertFootnoteContainer(getFirstContainer());
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
	static_cast<fp_FootnoteContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
}

void fl_FootnoteLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_FootnoteLayout::updateLayout(void)
{
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

void fl_FootnoteLayout::redrawUpdate(void)
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

bool fl_FootnoteLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	return true;
}

bool fl_FootnoteLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	while (pBL)
	{
		pBL->recalculateFields(iUpdateCount);
		pBL = pBL->getNext();
	}
	return true;
}

void fl_FootnoteLayout::_lookupProperties(void)
{
 	const PP_AttrProp* pSectionAP = NULL;

	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const XML_Char *pszFootnotePID = NULL;
	if(!pSectionAP || !pSectionAP->getAttribute("footnote-id",pszFootnotePID))
	{
		m_iFootnotePID = 0;
	}
	else
	{
		m_iFootnotePID = atoi(pszFootnotePID);
	}
}

void fl_FootnoteLayout::_localCollapse(void)
{
	// ClearScreen on our Cell. One Cell per layout.
	fp_FootnoteContainer *pFC = (fp_FootnoteContainer *) getFirstContainer();
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
}

void fl_FootnoteLayout::collapse(void)
{
	_localCollapse();
	fp_FootnoteContainer *pFC = (fp_FootnoteContainer *) getFirstContainer();
	if (pFC)
	{
//
// Remove it from the page.
//
		if(pFC->getPage())
		{
			pFC->getPage()->removeFootnoteContainer(pFC);
			pFC->setPage(NULL);
		}
//
// remove it from the linked list.
//
		fp_FootnoteContainer * pPrev = (fp_FootnoteContainer *) pFC->getPrev();
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

bool fl_FootnoteLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	UT_ASSERT(pcrx->getStruxType()== PTX_SectionFootnote);
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

	collapse();
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
void fl_FootnoteLayout::_purgeLayout(void)
{
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
}
