/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "fp_Run.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_units.h"
#include "fv_FrameEdit.h"
#include "ut_png.h"
#include "ut_bytebuf.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "fv_View.h"

static void s_border_properties (const gchar * border_color, const gchar * border_style, const gchar * border_width,
								 const gchar * color, PP_PropertyMap::Line & line);

static void s_background_properties (const gchar * pszBgStyle, const gchar * pszBgColor,
									 const gchar * pszBackgroundColor,
									 PP_PropertyMap::Background & background);


fl_FrameLayout::fl_FrameLayout(FL_DocLayout* pLayout, 
							   pf_Frag_Strux* sdh, 
							   PT_AttrPropIndex indexAP, 
							   fl_ContainerLayout * pMyContainerLayout)
 	: fl_SectionLayout(pLayout, 
					   sdh, 
					   indexAP, 
					   FL_SECTION_FRAME,
					   FL_CONTAINER_FRAME,
					   PTX_SectionFrame,
					   pMyContainerLayout),
	  m_iFrameType(FL_FRAME_TEXTBOX_TYPE),
	  m_iFramePositionTo(FL_FRAME_POSITIONED_TO_BLOCK),
	  m_bNeedsRebuild(false),
	  m_bNeedsFormat(true),
	  m_bIsOnPage(false),
	  m_bHasEndFrame(false),
	  m_iWidth(0),
	  m_iHeight(0),
	  m_iXpos(0),
	  m_iYpos(0),
	  m_iXpad(0),
	  m_iYpad(0),
	  m_iXColumn(0),
	  m_iYColumn(0),
	  m_iXPage(0),
	  m_iYPage(0),
	  m_iBoundingSpace(0),
	  m_iFrameWrapMode(FL_FRAME_ABOVE_TEXT),
	  m_bIsTightWrap(false),
	  m_iPrefPage(-1),
	  m_iPrefColumn(0),
	  m_bExpandHeight(false),
	  m_iMinHeight(0),
	  m_pParentContainer(NULL)
{
}

fl_FrameLayout::~fl_FrameLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting Framelayout %p \n",this));
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
//
// Remove pointers to this if they exist
//
	if(getDocLayout() && getDocLayout()->getView())
	{
		FV_FrameEdit * pFE = getDocLayout()->getView()->getFrameEdit();
		if(pFE->getFrameLayout() == this)
		{
			pFE->setMode(FV_FrameEdit_NOT_ACTIVE);
		}
	}
}

/*!
 * This loads all the properties of the container found in the piecetable
 * into the physical frame container
 */
void 	fl_FrameLayout::setContainerProperties(void)
{
	fp_FrameContainer * pFrame = static_cast<fp_FrameContainer *>(getLastContainer());
	if(pFrame == NULL)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	pFrame->setBackground(m_background  );
	pFrame->setBottomStyle(m_lineBottom  );
	pFrame->setTopStyle(m_lineTop  );
	pFrame->setLeftStyle(m_lineLeft  );
	pFrame->setRightStyle(m_lineRight );
	pFrame->setXpad(m_iXpad);
	pFrame->setYpad(m_iYpad);
	pFrame->setTightWrapping(m_bIsTightWrap);
	if(FL_FRAME_BELOW_TEXT ==  m_iFrameWrapMode)
        {
	        pFrame->setAbove(false);
	}
	else if(FL_FRAME_WRAPPED_TO_RIGHT == m_iFrameWrapMode)
	{
	        pFrame->setRightWrapped(true);
	}
	else if(FL_FRAME_WRAPPED_TO_LEFT == m_iFrameWrapMode)
	{
	  pFrame->setLeftWrapped(true);
	}
	else if(FL_FRAME_WRAPPED_TOPBOT == m_iFrameWrapMode)
	{
	        pFrame->setTopBot(true);
	}
//
// Now do the image for this frame.
//
	if(m_pGraphicImage)
	{
		if(m_pImageImage == NULL)
		{
			const PP_AttrProp * pAP = NULL;
			getAP(pAP);
			GR_Graphics * pG = getDocLayout()->getGraphics();
			UT_sint32 iWidth = pG->tlu(100);
			UT_sint32 iHeight = pG->tlu(100);
			if(m_pGraphicImage->getType() == FGT_Raster)
			{
				iWidth = pG->tlu(m_pGraphicImage->getWidth());
				iHeight = pG->tlu(m_pGraphicImage->getHeight());
			}
			GR_Image * pImage = m_pGraphicImage->generateImage(pG,pAP,iWidth,iHeight);
			m_iDocImageWidth = pFrame->getFullWidth();
			m_iDocImageHeight = pFrame->getFullHeight();
			m_iGraphicTick = getDocLayout()->getGraphicTick();
			UT_Rect rec(0,0,pFrame->getFullWidth(),pFrame->getFullHeight());
			if(rec.width < pG->tlu(3))
			{
				rec.width = pG->tlu(3);
			}
			if(rec.height < pG->tlu(3))
			{
				rec.height = pG->tlu(3);
			}
			if(pImage)
			{
				pImage->scaleImageTo(pG,rec);
			}
			m_pImageImage = pImage;
		}
		pFrame->getFillType().setImagePointer(m_pGraphicImage, &m_pImageImage);
	}
	if(m_iFrameWrapMode >= FL_FRAME_WRAPPED_TO_RIGHT)
	{ 
//
// Set text wrapping around frame
//
		pFrame->setWrapping(true);
	}
	pFrame->setPreferedPageNo(m_iPrefPage);
	pFrame->setPreferedColumnNo(m_iPrefColumn);
}

UT_sint32 fl_FrameLayout::getBoundingSpace(void) const
{
	return m_iBoundingSpace;
}

/*!
 * Returns the position in the document of the PTX_SectionFrame strux
*/
PT_DocPosition fl_FrameLayout::getDocPosition(void) 
{
	pf_Frag_Strux* sdh = getStruxDocHandle();
    return 	m_pLayout->getDocument()->getStruxPosition(sdh);
}

/*!
 * This method returns the length of the Frame. This is such that 
 * getDocPosition() + getLength() is one value beyond the the EndFrame
 * strux
 */
UT_uint32 fl_FrameLayout::getLength(void)
{
	PT_DocPosition startPos = getDocPosition();
	pf_Frag_Strux* sdhEnd = NULL;
	pf_Frag_Strux* sdhStart = getStruxDocHandle();
	UT_DebugOnly<bool> bres;
	bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndFrame,&sdhEnd);
	UT_ASSERT(bres && sdhEnd);
	if(sdhEnd == NULL)
	{
	  return 1;
	}
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = static_cast<UT_uint32>(endPos - startPos + 1); 
	return length;
}


/*!
 * This code actually inserts a block AFTER the frame in the docsectionlayout
 * Code copied from tablelayout
 */
bool fl_FrameLayout::insertBlockAfter(fl_ContainerLayout* /*pLBlock*/,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{

	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_InsertStrux);
	UT_ASSERT(pcrx->getStruxType()==PTX_Block);

	fl_ContainerLayout * pNewCL = NULL;
	fl_ContainerLayout * pMyCL = myContainingLayout();
	pNewCL = pMyCL->insert(sdh,this,pcrx->getIndexAP(), FL_CONTAINER_BLOCK);
	fl_BlockLayout * pBlock = static_cast<fl_BlockLayout *>(pNewCL);
//
// Set the sectionlayout of this frame to that of the block since it is that scope
//
	pBlock->setSectionLayout(static_cast<fl_SectionLayout *>(myContainingLayout()));
	pNewCL->setContainingLayout(myContainingLayout());

		// Must call the bind function to complete the exchange of handles
		// with the document (piece table) *** before *** anything tries
		// to call down into the document (like all of the view
		// listeners).
		
	fl_ContainerLayout* sfhNew = pNewCL;
	pfnBindHandles(sdh,lid,sfhNew);
//
// increment the insertion point in the view.
//
	FV_View* pView = m_pLayout->getView();
	if (pView && (pView->isActive() || pView->isPreview()))
	{
		pView->setPoint(pcrx->getPosition() + fl_BLOCK_STRUX_OFFSET);
	}
	else if(pView && pView->getPoint() > pcrx->getPosition())
	{
		pView->setPoint(pView->getPoint() +  fl_BLOCK_STRUX_OFFSET);
	}
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	return true;
}

bool fl_FrameLayout::bl_doclistener_insertEndFrame(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew))
{
	// The endFrame strux actually needs a format handle to to this Frame layout.
	// so we bind to this layout. We also set a pointer to keep track of the endFrame strux.

	
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
	if(pView)
		pView->updateCarets(pcrx->getPosition(),1);
	m_bHasEndFrame = true;
	return true;
}


/*!
 * This signals an incomplete frame section.
 */
bool fl_FrameLayout::doclistener_deleteEndFrame( const PX_ChangeRecord_Strux * /*pcrx*/)
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
	fp_FrameContainer * pFrameC = static_cast<fp_FrameContainer *>(getFirstContainer());
	UT_GenericVector<fl_ContainerLayout *> AllLayouts;
	AllLayouts.clear();
	fp_Page * pPage = NULL;
	UT_sint32 i = 0;
	if(pFrameC)
	{
	    pPage = pFrameC->getPage();
		if (pPage)
		{
			pPage->getAllLayouts(AllLayouts);
			for(i=0; i< AllLayouts.getItemCount();i++)
			{
				fl_ContainerLayout * pCL = AllLayouts.getNthItem(i);
				pCL->collapse();
			}
		}
	}
	setAttrPropIndex(pcrxc->getIndexAP());
	collapse();
	lookupProperties();
	format();
	for(i=0; i< AllLayouts.getItemCount();i++)
	{
	    fl_ContainerLayout * pCL = AllLayouts.getNthItem(i);
	    pCL->format();
	    xxx_UT_DEBUGMSG(("Format block %x \n",pBL));
	    pCL->markAllRunsDirty();
	}
	getDocSectionLayout()->markAllRunsDirty();
	return true;
}


bool fl_FrameLayout::recalculateFields(UT_uint32 iUpdateCount)
{
	// ingnore frames in normal view mode
	FV_View * pView = getDocLayout()->getView();
	GR_Graphics * pG = getDocLayout()->getGraphics();
	UT_return_val_if_fail( pView && pG, false );
	
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

void fl_FrameLayout::updateLayout(bool /*bDoAll*/)
{
	// ingnore frames in normal view mode
	FV_View * pView = getDocLayout()->getView();
	GR_Graphics * pG = getDocLayout()->getGraphics();
	UT_return_if_fail( pView && pG);
	
	xxx_UT_DEBUGMSG(("UpdsateLayout in in framelayout \n"));
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
	UT_UNUSED(pcrx);
	UT_ASSERT(pcrx->getType()==PX_ChangeRecord::PXT_DeleteStrux);
#if 0
	fp_FrameContainer * pFrameC = getFirstContainer();
	if(pFrameC && pFrameC->getPage())
	{
		pFrameC->getPage()->markDirtyOverlappingRuns(pFrameC);
	}
#endif
	fp_FrameContainer * pFrameC = static_cast<fp_FrameContainer *>(getFirstContainer());
	UT_GenericVector<fl_BlockLayout *> vecBlocks;
	pFrameC->getBlocksAroundFrame(vecBlocks);
	UT_sint32 i = 0;
	for(i=0; i< vecBlocks.getItemCount();i++)
	{
	  fl_BlockLayout * pBL = vecBlocks.getNthItem(i);
	  pBL->collapse();
	  xxx_UT_DEBUGMSG(("Collapse block %x \n",pBL));
	}

//
// Remove all remaining structures
//
	collapse();
	myContainingLayout()->remove(this);
	UT_DEBUGMSG(("Unlinking frame Layout %p \n",this));
//
// Remove from the list of frames in the previous block
//
	fl_ContainerLayout * pCL = getParentContainer();
	fl_BlockLayout * pBL = NULL;
	if(pCL)
	{
		if(!pCL->removeFrame(this))
		{
			UT_DEBUGMSG(("Whoops! Frame not found in container %p\n",pCL));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
	}
	for(i=0; i< vecBlocks.getItemCount();i++)
	{
	  pBL = vecBlocks.getNthItem(i);
	  pBL->format();
	  xxx_UT_DEBUGMSG(("Format block %p\n",pBL));
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
	UT_DEBUGMSG(("FrameLayout: purge \n"));
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
	lookupProperties();
	fp_FrameContainer * pFrameContainer = new fp_FrameContainer(static_cast<fl_SectionLayout *>(this));
	setFirstContainer(pFrameContainer);
	setLastContainer(pFrameContainer);
	pFrameContainer->setWidth(m_iWidth);
	pFrameContainer->setHeight(m_iHeight);
	// Now do Frame image

	const PP_AttrProp* pSectionAP = NULL;
	// This is a real NO-NO: must *always* call getAP()
	// m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);
	getAP(pSectionAP);
	
	const gchar * pszDataID = NULL;
	pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const gchar *&)pszDataID);
	DELETEP(m_pImageImage);
	//
	// Set the image size from the full width
	//
	setImageWidth(pFrameContainer->getFullWidth());
	setImageHeight(pFrameContainer->getFullHeight());
	if(pszDataID && *pszDataID)
	{
		UT_DEBUGMSG(("!!!Found image of file %s \n",pszDataID));
		m_pGraphicImage = FG_Graphic::createFromStrux(this);
	}

	setContainerProperties();
}

/*!
  Create a new Frame container.
  \param If pPrevFrame is non-null place the new cell after this in the linked
          list, otherwise just append it to the end.
  \return The newly created Frame container
*/
fp_Container* fl_FrameLayout::getNewContainer(fp_Container *)
{
	UT_DEBUGMSG(("creating new Frame container\n"));
	_createFrameContainer();
	m_bIsOnPage = false;
	return static_cast<fp_Container *>(getLastContainer());
}

void fl_FrameLayout::_insertFrameContainer(fp_Container * /*pNewFC*/)
{

// This is all done fl_BlockLayout::setFramesOnPage

}


void fl_FrameLayout::miniFormat(void)
{
	// ingnore frames in normal view mode
	FV_View * pView = getDocLayout()->getView();
	GR_Graphics * pG = getDocLayout()->getGraphics();
	UT_return_if_fail( pView && pG );
	
	fl_ContainerLayout*	pBL = getFirstLayout();
	
	while (pBL)
	{
		pBL->format();
		pBL = pBL->getNext();
	}
	fp_FrameContainer * pFrame = static_cast<fp_FrameContainer *>(getFirstContainer());
	pFrame->layout();
	pFrame->getFillType().setWidthHeight(getDocLayout()->getGraphics(),pFrame->getFullWidth(),pFrame->getFullHeight(),false);
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
}

void fl_FrameLayout::format(void)
{
	// ingnore frames in normal view mode
	FV_View * pView = getDocLayout()->getView();
	GR_Graphics * pG = getDocLayout()->getGraphics();
	UT_return_if_fail( pView && pG );
	
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting first container is %x \n",getFirstContainer()));
	if(isHidden() > FP_VISIBLE)
	{
		xxx_UT_DEBUGMSG(("Don't format FRAME coz I'm hidden! \n"));
		return;
	}

	if(getFirstContainer() == NULL)
	{
		getNewContainer();
	}
	fl_ContainerLayout*	pBL2 = getFirstLayout();
	while (pBL2)
	{
		pBL2->format();
		UT_sint32 count = 0;
		while(pBL2->getLastContainer() == NULL || pBL2->getFirstContainer()==NULL)
		{
			UT_DEBUGMSG(("Error formatting a block try again \n"));
			count = count + 1;
			pBL2->format();
			if(count > 3)
			{
				UT_DEBUGMSG(("Give up trying to format. Hope for the best :-( \n"));
				break;
			}
		}
		pBL2 = pBL2->getNext();
	}
	static_cast<fp_FrameContainer *>(getFirstContainer())->layout();
	bool bPlacedOnPage = false;
	if(!m_bIsOnPage && !(getDocLayout()->isLayoutFilling()))
	{
//
// Place it on the correct page.
//
		fl_ContainerLayout * pCL = getParentContainer();
		if((!pCL) || pCL->getContainerType() != FL_CONTAINER_BLOCK)
		{
			UT_DEBUGMSG(("No BlockLayout or wrong layout associated with this frame! \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		fl_BlockLayout * pBL = NULL;
		pBL = static_cast<fl_BlockLayout *>(pCL);
		UT_sint32 count = pBL->getNumFrames();
		UT_sint32 i = 0;
		for(i=0; i<count; i++)
		{
			fl_FrameLayout * pFL = pBL->getNthFrameLayout(i);
			if(pFL == this)
			{
				break;
			}
		}
		if(count == i)
		{
			UT_DEBUGMSG(("BlockLayout does not contain this frame! \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		if(!pBL->isCollapsed())
		{
			m_bIsOnPage = pBL->setFramesOnPage(NULL);
			if(!m_bIsOnPage)
			{
				setNeedsReformat(this);
			}
		}
		if(m_bIsOnPage)
			bPlacedOnPage = true;
	}
	m_bNeedsFormat = m_bIsOnPage;
	m_bNeedsReformat = m_bIsOnPage;
	if(!m_bIsOnPage)
	{
		setNeedsReformat(this);
	}
	if(!m_bIsOnPage)
	{
		return;
	}
	if(bPlacedOnPage)
	{
		fl_DocSectionLayout * pDSL = getDocSectionLayout();
		fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(getFirstContainer());
		if(pFC)
		{
			pDSL->setNeedsSectionBreak(true,pFC->getPage());
		}
	}
}

void fl_FrameLayout::setNeedsReformat(fl_ContainerLayout * /*pCL*/,UT_uint32 /*offset*/)
{
        m_bNeedsReformat = true;
        myContainingLayout()->setNeedsReformat(this);
}
/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_FrameLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);

	FV_View * pView = getDocLayout()->getView();
	GR_Graphics * pG = getDocLayout()->getGraphics();
	UT_return_if_fail( pView && pG );

	const gchar *pszFrameType = NULL;
	const gchar *pszPositionTo = NULL;
	const gchar *pszWrapMode = NULL;
	const gchar *pszXpos = NULL;
	const gchar *pszYpos = NULL;
	const gchar *pszColXpos = NULL;
	const gchar *pszColYpos = NULL;
	const gchar *pszPageXpos = NULL;
	const gchar *pszPageYpos = NULL;
	const gchar *pszWidth = NULL;
	const gchar *pszHeight = NULL;
	const gchar *pszXpad = NULL;
	const gchar *pszYpad = NULL;

	const gchar * pszColor = NULL;
	const gchar * pszBorderColor = NULL;
	const gchar * pszBorderStyle = NULL;
	const gchar * pszBorderWidth = NULL;

	const gchar * pszBoundingSpace = NULL;
	const gchar * pszTightWrapped = NULL;
	const gchar * pszPrefPage = NULL;
	const gchar * pszPrefColumn = NULL;

	const gchar * pszExpandHeight = NULL;
	const gchar * pszPercentWidth = NULL;
	const gchar * pszMinHeight = NULL;
// Frame Type

	if(!pSectionAP || !pSectionAP->getProperty("frame-type",pszFrameType))
	{
		m_iFrameType = FL_FRAME_TEXTBOX_TYPE;
	}
	else if(strcmp(pszFrameType,"textbox") == 0)
	{
		m_iFrameType = FL_FRAME_TEXTBOX_TYPE;
	}
	else if(strcmp(pszFrameType,"image") == 0)
	{
		m_iFrameType = FL_FRAME_WRAPPER_IMAGE;
	}
	else 
	{
		UT_DEBUGMSG(("Unknown Frame Type %s \n",pszFrameType));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_iFrameType = FL_FRAME_TEXTBOX_TYPE;
	}

// Position-to

	if((!pSectionAP || !pSectionAP->getProperty("position-to",pszPositionTo)))
	{
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
	}
	else if(strcmp(pszPositionTo,"block-above-text") == 0)
	{
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
	}
	else if(strcmp(pszPositionTo,"column-above-text") == 0)
	{
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_COLUMN;
	}
	else if(strcmp(pszPositionTo,"page-above-text") == 0)
	{
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_PAGE;
	}
	else 
	{
		UT_DEBUGMSG(("Unknown Position to %s \n",pszPositionTo));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_iFramePositionTo =  FL_FRAME_POSITIONED_TO_BLOCK;
	}


// wrap-mode
	
	if(!pSectionAP || !pSectionAP->getProperty("wrap-mode",pszWrapMode))
	{
		m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}
	else if(strcmp(pszWrapMode,"above-text") == 0)
	{
		m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}
	else if(strcmp(pszWrapMode,"below-text") == 0)
	{
		m_iFrameWrapMode = FL_FRAME_BELOW_TEXT;
	}
	else if(strcmp(pszWrapMode,"wrapped-to-right") == 0)
	{
		m_iFrameWrapMode = FL_FRAME_WRAPPED_TO_RIGHT;
	}
	else if(strcmp(pszWrapMode,"wrapped-to-left") == 0)
	{
		m_iFrameWrapMode = FL_FRAME_WRAPPED_TO_LEFT;
	}
	else if(strcmp(pszWrapMode,"wrapped-both") == 0)
	{
		m_iFrameWrapMode = FL_FRAME_WRAPPED_BOTH_SIDES;
	}
	else if(strcmp(pszWrapMode,"wrapped-topbot") == 0)
	{
		m_iFrameWrapMode = FL_FRAME_WRAPPED_TOPBOT;
	}
	else 
	{
		UT_DEBUGMSG(("Unknown wrap-mode %s \n",pszWrapMode));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}
	//
	// Wrap type
	//
	if(!pSectionAP || !pSectionAP->getProperty("tight-wrap",pszTightWrapped))
	{
		m_bIsTightWrap = false;
	}
	else if(strcmp(pszTightWrapped,"1") == 0)
	{
		m_bIsTightWrap = true;
	}
	else
	{
		m_bIsTightWrap = false;
	}

// Xpos

	if(!pSectionAP || !pSectionAP->getProperty("xpos",pszXpos))
	{
		m_iXpos = 0;
	}
	else
	{
		m_iXpos = UT_convertToLogicalUnits(pszXpos);
	}
	UT_DEBUGMSG(("xpos for frame is %s \n",pszXpos));
// Ypos

	if(!pSectionAP || !pSectionAP->getProperty("ypos",pszYpos))
	{
		m_iYpos = 0;
	}
	else
	{
		m_iYpos = UT_convertToLogicalUnits(pszYpos);
	}
	UT_DEBUGMSG(("ypos for frame is %s \n",pszYpos));

// ColXpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-col-xpos",pszColXpos))
	{
		m_iXColumn = 0;
	}
	else
	{
		m_iXColumn = UT_convertToLogicalUnits(pszColXpos);
	}
	UT_DEBUGMSG(("ColXpos for frame is %s \n",pszColXpos));
// colYpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-col-ypos",pszColYpos))
	{
		m_iYColumn = 0;
	}
	else
	{
		m_iYColumn = UT_convertToLogicalUnits(pszColYpos);
	}
	UT_DEBUGMSG(("ColYpos for frame is %s units %d \n",pszColYpos,m_iYColumn));


// PageXpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-page-xpos",pszPageXpos))
	{
		m_iXPage = 0;
	}
	else
	{
		m_iXPage = UT_convertToLogicalUnits(pszPageXpos);
	}
	UT_DEBUGMSG(("PageXpos for frame is %s units %d \n",pszPageXpos,m_iXPage));
// PageYpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-page-ypos",pszPageYpos))
	{
		m_iYPage = 0;
	}
	else
	{
		m_iYPage = UT_convertToLogicalUnits(pszPageYpos);
	}
	UT_DEBUGMSG(("PageYpos for frame is %s units %d \n",pszPageYpos,m_iYPage));


// Width

	if(!pSectionAP || !pSectionAP->getProperty("frame-width",pszWidth))
	{
		m_iWidth = UT_convertToLogicalUnits("1.0in");
	}
	else
	{
		m_iWidth = UT_convertToLogicalUnits(pszWidth);
	}
	if(m_iWidth < m_pLayout->getGraphics()->tlu(2))
	{
		m_iWidth = m_pLayout->getGraphics()->tlu(2);
	}
	UT_DEBUGMSG(("Width %s \n",pszWidth));
// Height

	if(!pSectionAP || !pSectionAP->getProperty("frame-height",pszHeight))
	{
		m_iHeight = UT_convertToLogicalUnits("1.0in");
	}
	else
	{
		m_iHeight = UT_convertToLogicalUnits(pszHeight);
	}
	if(m_iHeight < m_pLayout->getGraphics()->tlu(2))
	{
		m_iHeight = m_pLayout->getGraphics()->tlu(2);
	}
	m_iMinHeight = m_iHeight;
	UT_DEBUGMSG(("Height %s \n",pszHeight));

// Xpadding


	if(!pSectionAP || !pSectionAP->getProperty("xpad",pszXpad))
	{
		m_iXpad = UT_convertToLogicalUnits("0.03in");
	}
	else
	{
		m_iXpad = UT_convertToLogicalUnits(pszXpad);
	}


// Ypadding


	if(!pSectionAP || !pSectionAP->getProperty("ypad",pszYpad))
	{
		m_iYpad = UT_convertToLogicalUnits("0.03in");
	}
	else
	{
		m_iYpad = UT_convertToLogicalUnits(pszYpad);
	}


	/* Frame-border properties:
	 */

	pSectionAP->getProperty ("color", pszColor);

	pSectionAP->getProperty ("bot-color",pszBorderColor);
	pSectionAP->getProperty ("bot-style",pszBorderStyle);
	pSectionAP->getProperty ("bot-thickness",pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineBottom);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("left-color", pszBorderColor);
	pSectionAP->getProperty ("left-style", pszBorderStyle);
	pSectionAP->getProperty ("left-thickness", pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineLeft);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("right-color",pszBorderColor);
	pSectionAP->getProperty ("right-style",pszBorderStyle);
	pSectionAP->getProperty ("right-thickness", pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineRight);

	pszBorderColor = NULL;
	pszBorderStyle = NULL;
	pszBorderWidth = NULL;

	pSectionAP->getProperty ("top-color",  pszBorderColor);
	pSectionAP->getProperty ("top-style",  pszBorderStyle);
	pSectionAP->getProperty ("top-thickness",pszBorderWidth);

	s_border_properties (pszBorderColor, pszBorderStyle, pszBorderWidth, pszColor, m_lineTop);

	/* Frame fill
	 */
	m_background.reset ();

	const gchar * pszBgStyle = NULL;
	const gchar * pszBgColor = NULL;
	const gchar * pszBackgroundColor = NULL;

	pSectionAP->getProperty ("bg-style",    pszBgStyle);
	pSectionAP->getProperty ("bgcolor",     pszBgColor);
	pSectionAP->getProperty ("background-color", pszBackgroundColor);

	s_background_properties (pszBgStyle, pszBgColor, pszBackgroundColor, m_background);

//
// Bounding Space
//
	if(!pSectionAP || !pSectionAP->getProperty("bounding-space",pszBoundingSpace))
	{
		m_iBoundingSpace = UT_convertToLogicalUnits("0.05in");
	}
	else
	{
		m_iBoundingSpace = UT_convertToLogicalUnits(pszBoundingSpace);
	}
	//
	// Preferred Page
	//
	if(!pSectionAP || !pSectionAP->getProperty("frame-pref-page",pszPrefPage))
	{
		m_iPrefPage = -1;
	}
	else
	{
		if(pszPrefPage && *pszPrefPage != 0)
			m_iPrefPage = atoi(pszPrefPage);
		else
			m_iPrefPage = -1;
	}
	//
	// Preferred column
	//
	if(!pSectionAP || !pSectionAP->getProperty("frame-pref-column",pszPrefColumn))
	{
		m_iPrefColumn = 0;
	}
	else
	{
		if(pszPrefColumn && *pszPrefColumn != 0)
			m_iPrefColumn = atoi(pszPrefColumn);
		else
			m_iPrefColumn = -1;
	}
	// 
	// Percent Width
	//
	if(pSectionAP && pSectionAP->getProperty("frame-rel-width",pszPercentWidth))
	{
		if(pszPercentWidth && (m_iWidth <= (m_pLayout->getGraphics()->tlu(2)+3)))
		{
			double frac_width = UT_convertFraction(pszPercentWidth);
			fl_DocSectionLayout * pDSL = getDocSectionLayout();
			m_iWidth = frac_width*pDSL->getActualColumnWidth();
		}
	}

	//
	// Min Height
	//
	if(pSectionAP && pSectionAP->getProperty("frame-min-height",pszMinHeight))
	{
		if(pszMinHeight)
		{
			m_iMinHeight = UT_convertToLogicalUnits(pszMinHeight);
			m_bExpandHeight = true;
		}
	}

	//
	// Expandable Height
	//
	if(pSectionAP && pSectionAP->getProperty("frame-expand-height",pszExpandHeight))
	{
		m_iMinHeight = m_iHeight;
		m_bExpandHeight = true;
	}

	//
	// left/right aligned
	//
	const char * pszAlign = NULL;
	if(pSectionAP && pSectionAP->getProperty("frame-horiz-align",pszAlign))
	{
		if(pszAlign && (strcmp(pszAlign,"right") == 0) && (m_iXpos == 0))
		{
			fl_DocSectionLayout * pDSL = getDocSectionLayout();
			m_iXpos =  pDSL->getActualColumnWidth() - m_iWidth;
		}
	}

}

void fl_FrameLayout::_lookupMarginProperties(const PP_AttrProp* pSectionAP)
{
	
	UT_return_if_fail(pSectionAP);
	FV_View * pView = getDocLayout()->getView();
	GR_Graphics * pG = getDocLayout()->getGraphics();
	UT_return_if_fail( pView && pG );
	
	UT_sint32 iFramePositionTo = m_iFramePositionTo;
	FL_FrameWrapMode iFrameWrapMode = m_iFrameWrapMode;
	bool bIsTightWrap = m_bIsTightWrap;
	UT_sint32 iXpos = m_iXpos;
	UT_sint32 iYpos = m_iYpos;
	UT_sint32 iXColumn = m_iXColumn;
	UT_sint32 iYColumn = m_iYColumn;
	UT_sint32 iXPage = m_iXPage;
	UT_sint32 iYPage = m_iYPage;

	
	if(pView->getViewMode() == VIEW_NORMAL && !pG->queryProperties(GR_Graphics::DGP_PAPER))
	{
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
		m_iFrameWrapMode = FL_FRAME_WRAPPED_TO_RIGHT;
		m_bIsTightWrap = false;
		m_iXpos = 0;
		m_iYpos = 0;
		m_iXColumn = 0;
		m_iYColumn = 0;
		m_iXPage = 0;
		m_iYPage = 0;
	}
	else
	{
		const gchar *pszPositionTo = NULL;
		const gchar *pszWrapMode = NULL;
		const gchar *pszXpos = NULL;
		const gchar *pszYpos = NULL;
		const gchar *pszColXpos = NULL;
		const gchar *pszColYpos = NULL;
		const gchar *pszPageXpos = NULL;
		const gchar *pszPageYpos = NULL;
		const gchar * pszTightWrapped = NULL;


		// Position-to

		if(!pSectionAP || !pSectionAP->getProperty("position-to",pszPositionTo))
		{
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
		}
		else if(strcmp(pszPositionTo,"block-above-text") == 0)
		{
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
		}
		else if(strcmp(pszPositionTo,"column-above-text") == 0)
		{
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_COLUMN;
		}
		else if(strcmp(pszPositionTo,"page-above-text") == 0)
		{
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_PAGE;
		}
		else 
		{
			UT_DEBUGMSG(("Unknown Position to %s \n",pszPositionTo));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_iFramePositionTo =  FL_FRAME_POSITIONED_TO_BLOCK;
		}


		// wrap-mode

		if(!pSectionAP || !pSectionAP->getProperty("wrap-mode",pszWrapMode))
		{
			m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
		}
		else if(strcmp(pszWrapMode,"above-text") == 0)
		{
			m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
		}
		else if(strcmp(pszWrapMode,"below-text") == 0)
		{
			m_iFrameWrapMode = FL_FRAME_BELOW_TEXT;
		}
		else if(strcmp(pszWrapMode,"wrapped-to-right") == 0)
		{
			m_iFrameWrapMode = FL_FRAME_WRAPPED_TO_RIGHT;
		}
		else if(strcmp(pszWrapMode,"wrapped-to-left") == 0)
		{
			m_iFrameWrapMode = FL_FRAME_WRAPPED_TO_LEFT;
		}
		else if(strcmp(pszWrapMode,"wrapped-both") == 0)
		{
			m_iFrameWrapMode = FL_FRAME_WRAPPED_BOTH_SIDES;
		}
		else if(strcmp(pszWrapMode,"wrapped-topbot") == 0)
		{
			m_iFrameWrapMode = FL_FRAME_WRAPPED_TOPBOT;
		}
		else 
		{
			UT_DEBUGMSG(("Unknown wrap-mode %s \n",pszWrapMode));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
		}
		//
		// Wrap type
		//
		if(!pSectionAP || !pSectionAP->getProperty("tight-wrap",pszTightWrapped))
		{
			m_bIsTightWrap = false;
		}
		else if(strcmp(pszTightWrapped,"1") == 0)
		{
			m_bIsTightWrap = true;
		}
		else
		{
			m_bIsTightWrap = false;
		}

		// Xpos

		if(!pSectionAP || !pSectionAP->getProperty("xpos",pszXpos))
		{
			m_iXpos = 0;
		}
		else
		{
			m_iXpos = UT_convertToLogicalUnits(pszXpos);
		}
		UT_DEBUGMSG(("xpos for frame is %s \n",pszXpos));
		// Ypos

		if(!pSectionAP || !pSectionAP->getProperty("ypos",pszYpos))
		{
			m_iYpos = 0;
		}
		else
		{
			m_iYpos = UT_convertToLogicalUnits(pszYpos);
		}
		UT_DEBUGMSG(("ypos for frame is %s \n",pszYpos));

		// ColXpos

		if(!pSectionAP || !pSectionAP->getProperty("frame-col-xpos",pszColXpos))
		{
			m_iXColumn = 0;
		}
		else
		{
			m_iXColumn = UT_convertToLogicalUnits(pszColXpos);
		}
		UT_DEBUGMSG(("ColXpos for frame is %s \n",pszColXpos));
		// colYpos

		if(!pSectionAP || !pSectionAP->getProperty("frame-col-ypos",pszColYpos))
		{
			m_iYColumn = 0;
		}
		else
		{
			m_iYColumn = UT_convertToLogicalUnits(pszColYpos);
		}
		UT_DEBUGMSG(("ColYpos for frame is %s units %d \n",pszColYpos,m_iYColumn));


		// PageXpos

		if(!pSectionAP || !pSectionAP->getProperty("frame-page-xpos",pszPageXpos))
		{
			m_iXPage = 0;
		}
		else
		{
			m_iXPage = UT_convertToLogicalUnits(pszPageXpos);
		}
		UT_DEBUGMSG(("PageXpos for frame is %s \n",pszPageXpos));
		// PageYpos

		if(!pSectionAP || !pSectionAP->getProperty("frame-page-ypos",pszPageYpos))
		{
			m_iYPage = 0;
		}
		else
		{
			m_iYPage = UT_convertToLogicalUnits(pszPageYpos);
		}
		UT_DEBUGMSG(("PageYpos for frame is %s units %d \n",pszColYpos,m_iYPage));

	}

	fl_ContainerLayout*	pCL = getFirstLayout();
	while (pCL)
	{
		pCL->lookupMarginProperties();
		pCL = pCL->getNext();
	}

	if(iFramePositionTo != m_iFramePositionTo || iFrameWrapMode != m_iFrameWrapMode ||
	   bIsTightWrap != m_bIsTightWrap || iXpos != m_iXpos || iYpos != m_iYpos ||
	   iXColumn != m_iXColumn || iYColumn != m_iYColumn || iXPage != m_iXPage ||
	   iYPage != m_iYPage)
	{
		collapse();
	}
}

void fl_FrameLayout::localCollapse(void)
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
	FV_View * pView = getDocLayout()->getView();
	if(pView)
	{
		if(pView->getFrameEdit()->getFrameLayout() == this)
		{
			pView->getFrameEdit()->setMode(FV_FrameEdit_NOT_ACTIVE);
		}
	}
	localCollapse();
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

// Frame Background

static void s_background_properties (const gchar * pszBgStyle, const gchar * pszBgColor,
									 const gchar * pszBackgroundColor,
									 PP_PropertyMap::Background & background)
{
	if (pszBgStyle)
		{
			if (strcmp (pszBgStyle, "0") == 0)
				{
					background.m_t_background = PP_PropertyMap::background_none;
				}
			else if (strcmp (pszBgStyle, "1") == 0)
				{
					if (pszBgColor)
						{
							background.m_t_background = PP_PropertyMap::background_type (pszBgColor);
							if (background.m_t_background == PP_PropertyMap::background_solid)
								UT_parseColor (pszBgColor, background.m_color);
						}

				}
		}

	if (pszBackgroundColor)
		{
			background.m_t_background = PP_PropertyMap::background_type (pszBackgroundColor);
			if (background.m_t_background == PP_PropertyMap::background_solid)
				UT_parseColor (pszBackgroundColor, background.m_color);
		}
}

static void s_border_properties (const gchar * border_color, const gchar * border_style, const gchar * border_width,
								 const gchar * color, PP_PropertyMap::Line & line)
{
	/* frame-border properties:
	 * 
	 * (1) color      - defaults to value of "color" property
	 * (2) line-style - defaults to solid (in contrast to "none" in CSS)
	 * (3) thickness  - defaults to 1 layout unit (??, vs "medium" in CSS)
	 */
	line.reset ();

	PP_PropertyMap::TypeColor t_border_color = PP_PropertyMap::color_type (border_color);
	if (t_border_color)
		{
			line.m_t_color = t_border_color;
			if (t_border_color == PP_PropertyMap::color_color)
				UT_parseColor (border_color, line.m_color);
		}
	else if (color)
		{
			PP_PropertyMap::TypeColor t_color = PP_PropertyMap::color_type (color);

			line.m_t_color = t_color;
			if (t_color == PP_PropertyMap::color_color)
				UT_parseColor (color, line.m_color);
		}

	line.m_t_linestyle = PP_PropertyMap::linestyle_type (border_style);
	if (!line.m_t_linestyle)
		line.m_t_linestyle = PP_PropertyMap::linestyle_solid;

	line.m_t_thickness = PP_PropertyMap::thickness_type (border_width);
	if (line.m_t_thickness == PP_PropertyMap::thickness_length)
		{
			if (UT_determineDimension (border_width, (UT_Dimension)-1) == DIM_PX)
				{
					double thickness = UT_LAYOUT_RESOLUTION * UT_convertDimensionless (border_width);
					line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
				}
			else
				line.m_thickness = UT_convertToLogicalUnits (border_width);

			if (!line.m_thickness)
				{
					double thickness = UT_LAYOUT_RESOLUTION;
					line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
				}
		}
	else // ??
		{
			double thickness = UT_LAYOUT_RESOLUTION;
			line.m_thickness = static_cast<UT_sint32>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
}

