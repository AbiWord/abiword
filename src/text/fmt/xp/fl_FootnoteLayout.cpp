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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "fp_Run.h"
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
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "fp_TableContainer.h"
#include "fv_View.h"

fl_EmbedLayout::fl_EmbedLayout(FL_DocLayout* pLayout, fl_DocSectionLayout* pDocSL, pf_Frag_Strux* sdh, PT_AttrPropIndex indexAP, fl_ContainerLayout * pMyContainerLayout, SectionType iSecType,fl_ContainerType myType,PTStruxType myStruxType)
 	: fl_SectionLayout(pLayout, sdh, indexAP, iSecType,myType,myStruxType,pMyContainerLayout),
	  m_bNeedsRebuild(false),
	  m_bNeedsFormat(true),
	  m_bIsOnPage(false),
	  m_pDocSL(pDocSL),
	  m_bHasEndFootnote(false),
	  m_iOldSize(0)
{
	UT_ASSERT(m_pDocSL->getContainerType() == FL_CONTAINER_DOCSECTION);
}

fl_EmbedLayout::~fl_EmbedLayout()
{
}
	
/*!
 * Returns the position in the document of the PTX_SectionFootnote strux
 * This is very useful for determining the value of the footnote reference
 * and anchor. 
*/
PT_DocPosition fl_EmbedLayout::getDocPosition(void) 
{
	pf_Frag_Strux* sdh = getStruxDocHandle();
	UT_return_val_if_fail( m_pLayout, 0 );
    return 	m_pLayout->getDocument()->getStruxPosition(sdh);
}

/*!
 * Return the block that contains this Embedded layout 
 */
fl_BlockLayout * fl_EmbedLayout::getContainingBlock(void)
{
  fl_ContainerLayout * pCL = getPrev();
  while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
  {
      pCL = pCL->getPrev();
  }
  if(pCL == NULL)
      return NULL;
  fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pCL);
  while(pBL && pBL->getPosition(true) > getDocPosition())
      pBL = pBL->getPrevBlockInDocument();
  return pBL;
}

/*!
 * This method returns the length of the footnote. This is such that 
 * getDocPosition() + getLength() is one value beyond the the EndFootnote
 * strux
 */
UT_uint32 fl_EmbedLayout::getLength(void)
{
	UT_return_val_if_fail( m_pLayout, 0 );
	PT_DocPosition startPos = getDocPosition();
	pf_Frag_Strux* sdhEnd = NULL;
	pf_Frag_Strux* sdhStart = getStruxDocHandle();
	UT_DebugOnly<bool> bres;
	if(getContainerType() == FL_CONTAINER_FOOTNOTE)
	{
		bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndFootnote,&sdhEnd);
		UT_ASSERT(bres);
	}
	else if(getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndEndnote,&sdhEnd);
		UT_ASSERT(bres);
	}
	else if(getContainerType() == FL_CONTAINER_ANNOTATION)
	{
		bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndAnnotation,&sdhEnd);
		UT_ASSERT(bres);
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	UT_ASSERT(bres && sdhEnd);
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = static_cast<UT_uint32>(endPos - startPos + 1); 
	return length;
}


bool fl_EmbedLayout::bl_doclistener_insertEndEmbed(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	// The endFootnote strux actually needs a format handle to to this Footnote layout.
	// so we bind to this layout. We also set a pointer to keep track of the endEmbed strux.

		
	fl_ContainerLayout* sfhNew = this;
	pfnBindHandles(sdh,lid,sfhNew);
	setEndStruxDocHandle(sdh);

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
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(getFirstLayout());
	pBL->updateEnclosingBlockIfNeeded();
	return true;
}


/*!
 * This signals an incomplete footnote section.
 */
bool fl_EmbedLayout::doclistener_deleteEndEmbed( const PX_ChangeRecord_Strux * /*pcrx*/)
{
	m_bHasEndFootnote = false;
	return true;
}


fl_SectionLayout * fl_EmbedLayout::getSectionLayout(void) const
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


bool fl_EmbedLayout::doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc)
{
	UT_ASSERT(pcrxc->getType()==PX_ChangeRecord::PXT_ChangeStrux);


	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	return true;
}


bool fl_EmbedLayout::recalculateFields(UT_uint32 iUpdateCount)
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


void fl_EmbedLayout::markAllRunsDirty(void)
{
	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->markAllRunsDirty();
		pCL = pCL->getNext();
	}
}

void fl_EmbedLayout::updateLayout(void)
{
	if(needsReformat())
	{
		format();
	}
	m_vecFormatLayout.clear();
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

void fl_EmbedLayout::redrawUpdate(void)
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

void fl_EmbedLayout::setNeedsReformat(fl_ContainerLayout * /*pCL*/, UT_uint32 /*offset*/)
{
  m_bNeedsReformat = true;
  if(getSectionLayout())
    getSectionLayout()->setNeedsReformat(this);
}

void fl_EmbedLayout::updateLayout(bool /*bDoAll*/)
{
  fl_ContainerLayout * pBL = getFirstLayout();
  while(pBL)
  {
    pBL->format();
    pBL = pBL->getNext();
  }
}

bool fl_EmbedLayout::doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx)
{
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
	// Move cursor to its new position
	m_pLayout->getView()->setPoint(pcrx->getPosition());
//
// Remove all remaining structures
//
	if(getPrev())
	{
	  getPrev()->setNeedsReformat(getPrev());
	}
	collapse();
//	UT_ASSERT(pcrx->getStruxType()== PTX_SectionFootnote);
//
// Find the block that contains this layout.
//
	PT_DocPosition prevPos = pcrx->getPosition();
	fl_BlockLayout * pEncBlock =  m_pLayout->findBlockAtPosition(prevPos);
//
// Fix the offsets for the block
//
	m_bHasEndFootnote = false;
	pEncBlock->updateOffsets(prevPos,0,-getOldSize());
	getSectionLayout()->remove(this);	
	delete this;			// TODO whoa!  this construct is VERY dangerous.

	return true;
}


/*!
 * This method removes all layout structures contained by this layout
 * structure.
 */
void fl_EmbedLayout::_purgeLayout(void)
{
	xxx_UT_DEBUGMSG(("embedLayout: purge \n"));
	fl_ContainerLayout * pCL = getFirstLayout();
	while(pCL)
	{
		fl_ContainerLayout * pNext = pCL->getNext();
		delete pCL;
		pCL = pNext;
	}
}

/************************************************************************/

fl_FootnoteLayout::fl_FootnoteLayout(FL_DocLayout* pLayout, 
									 fl_DocSectionLayout* pDocSL, 
									 pf_Frag_Strux* sdh, 
									 PT_AttrPropIndex indexAP, 
									 fl_ContainerLayout * pMyContainerLayout)
 	: fl_EmbedLayout(pLayout, 
					 pDocSL, 
					 sdh, 
					 indexAP, 
					 pMyContainerLayout, 
					 FL_SECTION_FOOTNOTE,
					 FL_CONTAINER_FOOTNOTE,
					 PTX_SectionFootnote),
	  m_iFootnotePID(0)
{
	m_pLayout->addFootnote(this);
	_createFootnoteContainer();
}

fl_FootnoteLayout::~fl_FootnoteLayout()
{
	// NB: be careful about the order of these
	xxx_UT_DEBUGMSG(("Deleting Footlayout %x \n",this));
	_purgeLayout();
	fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(getFirstContainer());
	while(pFC)
	{
		fp_FootnoteContainer * pNext = static_cast<fp_FootnoteContainer *>(pFC->getNext());
		if(pFC == static_cast<fp_FootnoteContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		delete pFC;
		pFC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);

	UT_return_if_fail( m_pLayout );
	m_pLayout->removeFootnote(this);
}

/*!
 * This method creates a new footnote with its properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_FootnoteLayout::_createFootnoteContainer(void)
{
	lookupProperties();
	fp_FootnoteContainer * pFootnoteContainer = new fp_FootnoteContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pFootnoteContainer);
	setLastContainer(pFootnoteContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	UT_return_if_fail(pDSL != NULL);

	fp_Container * pCon = pCL->getLastContainer();
	UT_return_if_fail(pCon);
	UT_sint32 iWidth = pCon->getPage()->getWidth();
	iWidth = iWidth - pDSL->getLeftMargin() - pDSL->getRightMargin();
	pFootnoteContainer->setWidth(iWidth);
}

/*!
  Create a new Footnote container.
  \param If pPrevFootnote is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Footnote container
*/
fp_Container* fl_FootnoteLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("PLAM: creating new footnote container\n"));
	_createFootnoteContainer();
	m_bIsOnPage = false;
	return static_cast<fp_Container *>(getLastContainer());
}

void fl_FootnoteLayout::_insertFootnoteContainer(fp_Container * pNewFC)
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
		pPage->insertFootnoteContainer(static_cast<fp_FootnoteContainer*>(pNewFC));
		m_bIsOnPage = true;
	}
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
	m_bNeedsReformat = false;
}


/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_FootnoteLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const gchar *pszFootnotePID = NULL;
	if(!pSectionAP->getAttribute("footnote-id",pszFootnotePID))
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
	fp_FootnoteContainer *pFC = static_cast<fp_FootnoteContainer *>(getFirstContainer());
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

void fl_FootnoteLayout::collapse(void)
{
	_localCollapse();
	fp_FootnoteContainer *pFC = static_cast<fp_FootnoteContainer *>(getFirstContainer());
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
		fp_FootnoteContainer * pPrev = static_cast<fp_FootnoteContainer *>(pFC->getPrev());
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

/************************************************************************/

fl_AnnotationLayout::fl_AnnotationLayout(FL_DocLayout* pLayout, 
									 fl_DocSectionLayout* pDocSL, 
									 pf_Frag_Strux* sdh, 
									 PT_AttrPropIndex indexAP, 
									 fl_ContainerLayout * pMyContainerLayout)
 	: fl_EmbedLayout(pLayout, 
					 pDocSL, 
					 sdh, 
					 indexAP, 
					 pMyContainerLayout, 
					 FL_SECTION_ANNOTATION,
					 FL_CONTAINER_ANNOTATION,
					 PTX_SectionAnnotation),
	  m_iAnnotationPID(0)
{
	m_pLayout->addAnnotation(this);
	_createAnnotationContainer();
}

fl_AnnotationLayout::~fl_AnnotationLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting Annotationlayout %p \n",this));
	_purgeLayout();
	fp_AnnotationContainer * pAC = static_cast<fp_AnnotationContainer *>(getFirstContainer());
	while(pAC)
	{
		fp_AnnotationContainer * pNext = static_cast<fp_AnnotationContainer *>(pAC->getNext());
		if(pAC == static_cast<fp_AnnotationContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		delete pAC;
		pAC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);

	UT_return_if_fail( m_pLayout );
	m_pLayout->removeAnnotation(this);
}

/*!
 * This method creates a new footnote with its properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_AnnotationLayout::_createAnnotationContainer(void)
{
	lookupProperties();
	fp_AnnotationContainer * pAnnotationContainer = new fp_AnnotationContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pAnnotationContainer);
	setLastContainer(pAnnotationContainer);
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	UT_return_if_fail(pDSL != NULL);

	fp_Container * pCon = pCL->getLastContainer();
	UT_return_if_fail(pCon);
	UT_sint32 iWidth = pCon->getPage()->getWidth();
	iWidth = iWidth - pDSL->getLeftMargin() - pDSL->getRightMargin();
	pAnnotationContainer->setWidth(iWidth);
}

/*!
  Create a new Annotation container.
  \param If pPrevAnnotation is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Annotation container
*/
fp_Container* fl_AnnotationLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("Creating new Annotation container\n"));
	_createAnnotationContainer();
	m_bIsOnPage = false;
	return static_cast<fp_Container *>(getLastContainer());
}

void fl_AnnotationLayout::_insertAnnotationContainer(fp_Container * pNewAC)
{
	UT_DEBUGMSG(("inserting annotation container into container list\n"));
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
// Code to find the Line that contains the Annotation reference
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
	pNewAC->setContainer(NULL);

	// need to put onto page as well, in the appropriate place.
	UT_ASSERT(pPage);

	if(pPage)
	{
		pPage->insertAnnotationContainer(static_cast<fp_AnnotationContainer*>(pNewAC));
		m_bIsOnPage = true;
	}
}


void fl_AnnotationLayout::format(void)
{
	UT_DEBUGMSG(("SEVIOR: Formatting Annotations first container is %p\n", getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	if(!m_bIsOnPage)
	{
		_insertAnnotationContainer(getFirstContainer());
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
	static_cast<fp_AnnotationContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
}


/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_AnnotationLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const gchar *pszAnnotationPID = NULL;
	if(!pSectionAP->getAttribute("annotation-id",pszAnnotationPID))
	{
		m_iAnnotationPID = 0;
	}
	else
	{
		m_iAnnotationPID = atoi(pszAnnotationPID);
	}
	const char* pszAuthor;
	const char* pszTitle;
	const char *pszDate;
	if(!pSectionAP->getProperty("annotation-author", (const char *&)pszAuthor))
	{
	        pszAuthor = "";
	}
	m_sAuthor = pszAuthor;
	if(!pSectionAP->getProperty("annotation-title", (const char *&)pszTitle))
	{
	        pszTitle = "";
	}
	m_sTitle = pszTitle;
	if(!pSectionAP->getProperty("annotation-date", (const char *&)pszDate))
	{
	        pszDate = "";
	}
	m_sDate = pszDate;
	UT_DEBUGMSG(("Annotation _lookupProps Author|%s| Title |%s| \n", m_sAuthor.utf8_str(),m_sTitle.utf8_str()));
}

void fl_AnnotationLayout::_localCollapse(void)
{
	// ClearScreen on our Cell. One Cell per layout.
	fp_AnnotationContainer *pAC = static_cast<fp_AnnotationContainer *>(getFirstContainer());
	if (pAC)
	{
		pAC->clearScreen();
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

fp_AnnotationRun *  fl_AnnotationLayout::getAnnotationRun(void)
{
        PT_DocPosition posFL = getDocPosition() - 1;
	fl_ContainerLayout * pPrevL = static_cast<fl_ContainerLayout *>(m_pLayout->findBlockAtPosition(posFL));

	// get the owning container
	if(pPrevL != NULL)
	{
		if(pPrevL->getContainerType() == FL_CONTAINER_BLOCK)
		{
//
// Code to find the Line that contains the Annotation reference
//
			UT_ASSERT(pPrevL->getContainerType() == FL_CONTAINER_BLOCK);
			fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pPrevL);
			fp_Run * pRun = pBL->getFirstRun();
			PT_DocPosition posBL = pBL->getPosition();
			while(pRun && ((posBL + pRun->getBlockOffset() + pRun->getLength()) <= posFL))
			{
				pRun = pRun->getNextRun();
			}
			if(pRun && (pRun->getType() == FPRUN_HYPERLINK))
			{
			    fp_HyperlinkRun * pHRun = static_cast<fp_HyperlinkRun *>(pRun);
			    if(pHRun->getHyperlinkType() == HYPERLINK_ANNOTATION)
			    {
				fp_AnnotationRun * pARun = static_cast<fp_AnnotationRun * >(pHRun);
				if(pARun->getPID() == getAnnotationPID())
				{
				    return pARun;
				}
			    }
			}
			else
			{
			    return NULL;
			}
		}
		else
	        {
		    return NULL;
		}
	}
	return NULL;
}
	

void fl_AnnotationLayout::collapse(void)
{
	_localCollapse();
	fp_AnnotationContainer *pAC = static_cast<fp_AnnotationContainer *>(getFirstContainer());
	if (pAC)
	{
//
// Remove it from the page.
//
		if(pAC->getPage())
		{
			pAC->getPage()->removeAnnotationContainer(pAC);
			pAC->setPage(NULL);
		}
//
// remove it from the linked list.
//
		fp_AnnotationContainer * pPrev = static_cast<fp_AnnotationContainer *>(pAC->getPrev());
		if(pPrev)
		{
			pPrev->setNext(pAC->getNext());
		}
		if(pAC->getNext())
		{
			pAC->getNext()->setPrev(pPrev);
		}
		delete pAC;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
	m_bIsOnPage = false;
}



/************************************************************************/

fl_EndnoteLayout::fl_EndnoteLayout(FL_DocLayout* pLayout, 
								   fl_DocSectionLayout* pDocSL, 
								   pf_Frag_Strux* sdh, 
								   PT_AttrPropIndex indexAP, 
								   fl_ContainerLayout * pMyContainerLayout)
 	: fl_EmbedLayout(pLayout, 
					 pDocSL, 
					 sdh, 
					 indexAP, 
					 pMyContainerLayout, 
					 FL_SECTION_ENDNOTE,
					 FL_CONTAINER_ENDNOTE,
					 PTX_SectionEndnote),
	  m_iEndnotePID(0)
{
        UT_DEBUGMSG(("Create Endnote section %p from pos %d \n",this,getPosition()));
	m_pLayout->addEndnote(this);
	UT_DEBUGMSG(("myContaining Layout %s \n",myContainingLayout()->getContainerString()));
	_createEndnoteContainer();
}

fl_EndnoteLayout::~fl_EndnoteLayout()
{
	// NB: be careful about the order of these
	xxx_UT_DEBUGMSG(("Deleting Endlayout %x \n",this));
	_purgeLayout();
	fp_EndnoteContainer * pEC = static_cast<fp_EndnoteContainer *>(getFirstContainer());
	while(pEC)
	{
		fp_EndnoteContainer * pNext = static_cast<fp_EndnoteContainer *>(pEC->getNext());
		if(pEC == static_cast<fp_EndnoteContainer *>(getLastContainer()))
		{
			pNext = NULL;
		}
		m_pLayout->removeEndnoteContainer(pEC);
		delete pEC;
		pEC = pNext;
	}

	setFirstContainer(NULL);
	setLastContainer(NULL);
	m_pLayout->removeEndnote(this);
}

/*!
 * This method creates a new footnote with its properties initially set
 * from the Attributes/properties of this Layout
 */
void fl_EndnoteLayout::_createEndnoteContainer(void)
{
	lookupProperties();
	fp_EndnoteContainer * pEndnoteContainer = new fp_EndnoteContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pEndnoteContainer);
	setLastContainer(pEndnoteContainer);
	fl_DocSectionLayout * pDSL = m_pLayout->getDocSecForEndnote(pEndnoteContainer);
	UT_sint32 iWidth = m_pLayout->getLastPage()->getWidth();
	iWidth = iWidth - pDSL->getLeftMargin() - pDSL->getRightMargin();
	pEndnoteContainer->setWidth(iWidth);
	m_bNeedsReformat = true;
	m_bNeedsFormat = true;
}


void fl_EndnoteLayout::_localCollapse(void)
{
	// ClearScreen on our Cell. One Cell per layout.
	fp_EndnoteContainer *pFC = static_cast<fp_EndnoteContainer *>(getFirstContainer());
	UT_DEBUGMSG(("fl_endnote: _localCollapse First Container %p \n",pFC));
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

void fl_EndnoteLayout::collapse(void)
{
	UT_DEBUGMSG(("Collapsing  Endnote %p \n",this));
	_localCollapse();
	fp_EndnoteContainer *pFC = static_cast<fp_EndnoteContainer *>(getFirstContainer());
	while(pFC)
	{
		fp_EndnoteContainer *pNext = static_cast<fp_EndnoteContainer *>(pFC->getLocalNext());
		m_pLayout->removeEndnoteContainer(pFC);
		fp_EndnoteContainer * pPrev = static_cast<fp_EndnoteContainer *>(pFC->getPrev());
		if(pPrev)
		{
			pPrev->setNext(pFC->getNext());
		}
		if(pFC->getNext())
		{
			pFC->getNext()->setPrev(pPrev);
		}
		delete pFC;
		pFC = pNext;
	}
	setFirstContainer(NULL);
	setLastContainer(NULL);
	m_bIsOnPage = false;
}


void fl_EndnoteLayout::format(void)
{
	UT_DEBUGMSG(("SEVIOR: Formatting Endnote first container is %p \n",getFirstContainer()));
	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	if(!m_bIsOnPage)
	{
		_insertEndnoteContainer(getFirstContainer());
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
	static_cast<fp_EndnoteContainer *>(getFirstContainer())->layout();
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
	bool bOnPage = (getFirstContainer()->getPage() != NULL);
	FV_View * pView = NULL;
	if(m_pLayout)
		pView = m_pLayout->getView();
	if(bOnPage && pView && !pView->isLayoutFilling())
	{
	       getDocSectionLayout()->setNeedsSectionBreak(true,NULL);
	}
	UT_ASSERT(getFirstContainer()->getPage());
}


/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_EndnoteLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);
	// I can't think of any properties we need for now.
	// If we need any later, we'll add them. -PL
	const gchar *pszEndnotePID = NULL;
	if(!pSectionAP->getAttribute("endnote-id",pszEndnotePID))
	{
		m_iEndnotePID = 0;
	}
	else
	{
		m_iEndnotePID = atoi(pszEndnotePID);
	}
}


/*!
  Create a new Endote container.
  \param If pPrevFootnote is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Endnote container
*/
fp_Container* fl_EndnoteLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("fl_EndnoteLayoutx: creating new Endnote container\n"));
	_createEndnoteContainer();
	m_bIsOnPage = false;
	return static_cast<fp_Container *>(getLastContainer());
}


void fl_EndnoteLayout::_insertEndnoteContainer(fp_Container * pNewEC)
{
	xxx_UT_DEBUGMSG(("inserting endnote container into DocLayout list\n"));

	m_pLayout->insertEndnoteContainer(static_cast<fp_EndnoteContainer *>(pNewEC));
	m_bIsOnPage = true;
}
