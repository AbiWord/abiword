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

static void s_border_properties (const XML_Char * border_color, const XML_Char * border_style, const XML_Char * border_width,
								 const XML_Char * color, PP_PropertyMap::Line & line);

static void s_background_properties (const XML_Char * pszBgStyle, const XML_Char * pszBgColor,
									 const XML_Char * pszBackgroundColor,
									 PP_PropertyMap::Background & background);


fl_FrameLayout::fl_FrameLayout(FL_DocLayout* pLayout, 
							   fl_DocSectionLayout* pDocSL, 
							   PL_StruxDocHandle sdh, 
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
	  m_pDocSL(pDocSL),
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
	  m_iFrameWrapMode(FL_FRAME_ABOVE_TEXT)
{
	UT_ASSERT(m_pDocSL->getContainerType() == FL_CONTAINER_DOCSECTION);
}

fl_FrameLayout::~fl_FrameLayout()
{
	// NB: be careful about the order of these
	UT_DEBUGMSG(("Deleting Framelayout %x \n",this));
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	pFrame->setBackground(m_background  );
	pFrame->setBottomStyle(m_lineBottom  );
	pFrame->setTopStyle(m_lineTop  );
	pFrame->setLeftStyle(m_lineLeft  );
	pFrame->setRightStyle(m_lineRight );
	pFrame->setXpad(m_iXpad);
	pFrame->setYpad(m_iYpad);
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
			double iWidth = pG->tlu(100);
			double iHeight = pG->tlu(100);
			if(m_pGraphicImage->getType() == FGT_Raster)
			{
				UT_sint32 iImageWidth;
				UT_sint32 iImageHeight;
				UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(m_pGraphicImage)->getRaster_PNG();
				UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
				iWidth = pG->tlu(iImageWidth);
				iHeight = pG->tlu(iImageHeight);
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
			pImage->scaleImageTo(pG,rec);
			m_pImageImage = pImage;
		}
		pFrame->getFillType()->setImagePointer(&m_pGraphicImage,&m_pImageImage);
	}
	if(m_iFrameWrapMode >= FL_FRAME_WRAPPED_TO_RIGHT)
	{ 
//
// Set text wrapping around frame
//
		pFrame->setWrapping(true);
	}
}

double fl_FrameLayout::getBoundingSpace(void) const
{
	return m_iBoundingSpace;
}

/*!
 * Returns the position in the document of the PTX_SectionFrame strux
*/
PT_DocPosition fl_FrameLayout::getDocPosition(void) 
{
	PL_StruxDocHandle sdh = getStruxDocHandle();
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
	PL_StruxDocHandle sdhEnd = NULL;
	PL_StruxDocHandle sdhStart = getStruxDocHandle();
	bool bres;
	bres = m_pLayout->getDocument()->getNextStruxOfType(sdhStart,PTX_EndFrame,&sdhEnd);
	UT_ASSERT(bres && sdhEnd);
	PT_DocPosition endPos = m_pLayout->getDocument()->getStruxPosition(sdhEnd);
	UT_uint32 length = static_cast<UT_uint32>(endPos - startPos + 1); 
	return length;
}


/*!
 * This code actually inserts a block AFTER the frame in the docsectionlayout
 * Code copied from tablelayout
 */
bool fl_FrameLayout::insertBlockAfter(fl_ContainerLayout* pLBlock,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew))
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
		
	PL_StruxFmtHandle sfhNew = static_cast<PL_StruxFmtHandle>(pNewCL);
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
	return true;
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
	return true;
}


/*!
 * This signals an incomplete frame section.
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
	lookupProperties();
	format();
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
#if 0
	fp_FrameContainer * pFrameC = getFirstContainer();
	if(pFrameC && pFrameC->getPage())
	{
		pFrameC->getPage()->markDirtyOverlappingRuns(pFrameC);
	}
#endif
//
// Remove all remaining structures
//
	collapse();
//	UT_ASSERT(pcrx->getStruxType()== PTX_SectionFrame);
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
//
// Remove from the list of frames in the previous block
//
	fl_ContainerLayout * pCL = getPrev();
	while(pCL && pCL->getContainerType() != FL_CONTAINER_BLOCK)
	{
		pCL = pCL->getPrev();
	}
	if(pCL == NULL)
	{
		UT_DEBUGMSG(("No BlockLayout before this frame! \n"));
		return false;
	}
	fl_BlockLayout * pBL = static_cast<fl_BlockLayout *>(pCL);
	pBL->removeFrame(this);
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
	fl_ContainerLayout * pCL = myContainingLayout();
	while(pCL!= NULL && pCL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pCL = pCL->myContainingLayout();
	}
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pCL);
	UT_ASSERT(pDSL != NULL);

	fp_Container * pCon = pCL->getLastContainer();
	UT_ASSERT(pCon);
	pFrameContainer->setWidth(m_iWidth);
	pFrameContainer->setHeight(m_iHeight);
	// Now do Frame image

	const PP_AttrProp* pSectionAP = NULL;
	m_pLayout->getDocument()->getAttrProp(m_apIndex, &pSectionAP);

	const XML_Char * pszDataID = NULL;
	pSectionAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const XML_Char *&)pszDataID);
	DELETEP(m_pGraphicImage);
	DELETEP(m_pImageImage);
	if(pszDataID && *pszDataID)
	{
		UT_DEBUGMSG(("!!!Found image of file %s \n",pszDataID));
		m_pGraphicImage = FG_Graphic::createFromStrux(this);
	}

	setContainerProperties();
}

/*!
  Create a new Frame container.
  \params If pPrevFrame is non-null place the new cell after this in the linked
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

void fl_FrameLayout::_insertFrameContainer(fp_Container * pNewFC)
{

// This is all done fl_BlockLayout::setFramesOnPage

}


void fl_FrameLayout::miniFormat(void)
{
	fl_ContainerLayout*	pBL = getFirstLayout();
	
	while (pBL)
	{
		pBL->format();
		pBL = pBL->getNext();
	}
	fp_FrameContainer * pFrame = static_cast<fp_FrameContainer *>(getFirstContainer());
	pFrame->layout();
	pFrame->getFillType()->setWidthHeight(getDocLayout()->getGraphics(),pFrame->getFullWidth(),pFrame->getFullHeight(),false);
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
}

void fl_FrameLayout::format(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: Formatting first container is %x \n",getFirstContainer()));
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
	static_cast<fp_FrameContainer *>(getFirstContainer())->layout();
	if(!m_bIsOnPage)
	{
//
// Place it on the correct page.
//
		fl_ContainerLayout * pCL = getPrev();
		while(pCL && ((pCL->getContainerType() == FL_CONTAINER_ENDNOTE) ||
					  (pCL->getContainerType() == FL_CONTAINER_FOOTNOTE) ||
					  (pCL->getContainerType() == FL_CONTAINER_TOC) ||
					  (pCL->getContainerType() == FL_CONTAINER_FRAME)  ))
		{
			pCL = pCL->getPrev();
		}
		if(pCL == NULL)
		{
			UT_DEBUGMSG(("No BlockLayout before this frame! \n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		fl_BlockLayout * pBL = NULL;
		if(pCL->getContainerType() != FL_CONTAINER_BLOCK)
		{
			pCL = pCL->getPrevBlockInDocument();
			pBL = static_cast<fl_BlockLayout *>(pCL);
		}
		else
		{
			pBL = static_cast<fl_BlockLayout *>(pCL);
		}
		UT_return_if_fail(pBL);
		UT_sint32 count = pBL->getNumFrames();
		if(count == 0)
		{
			UT_DEBUGMSG(("BlockLayout does not contain this frame! \n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		UT_sint32 i =0;
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
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		pBL->setFramesOnPage(NULL);
	}
	m_bNeedsFormat = false;
	m_bNeedsReformat = false;
	fl_DocSectionLayout * pDSL = getDocSectionLayout();
	fp_FrameContainer * pFC = static_cast<fp_FrameContainer *>(getFirstContainer());
	if(pFC)
	{
		pDSL->setNeedsSectionBreak(true,pFC->getPage());
	}
}

/*!
    this function is only to be called by fl_ContainerLayout::lookupProperties()
    all other code must call lookupProperties() instead
*/
void fl_FrameLayout::_lookupProperties(const PP_AttrProp* pSectionAP)
{
	UT_return_if_fail(pSectionAP);

	const XML_Char *pszFrameType = NULL;
	const XML_Char *pszPositionTo = NULL;
	const XML_Char *pszWrapMode = NULL;
	const XML_Char *pszXpos = NULL;
	const XML_Char *pszYpos = NULL;
	const XML_Char *pszColXpos = NULL;
	const XML_Char *pszColYpos = NULL;
	const XML_Char *pszPageXpos = NULL;
	const XML_Char *pszPageYpos = NULL;
	const XML_Char *pszWidth = NULL;
	const XML_Char *pszHeight = NULL;
	const XML_Char *pszXpad = NULL;
	const XML_Char *pszYpad = NULL;

	const XML_Char * pszColor = NULL;
	const XML_Char * pszBorderColor = NULL;
	const XML_Char * pszBorderStyle = NULL;
	const XML_Char * pszBorderWidth = NULL;

	const XML_Char * pszBoundingSpace = NULL;
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
	else 
	{
		UT_DEBUGMSG(("Unknown wrap-mode %s \n",pszWrapMode));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		m_iFrameWrapMode = FL_FRAME_ABOVE_TEXT;
	}

// Xpos

	if(!pSectionAP || !pSectionAP->getProperty("xpos",pszXpos))
	{
		m_iXpos = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		m_iXpos = UT_convertToLogicalUnits(pszXpos);
	}
	UT_DEBUGMSG(("xpos for frame is %s \n",pszXpos));
// Ypos

	if(!pSectionAP || !pSectionAP->getProperty("ypos",pszYpos))
	{
		m_iYpos = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		m_iYpos = UT_convertToLogicalUnits(pszYpos);
	}
	UT_DEBUGMSG(("ypos for frame is %s \n",pszYpos));

// ColXpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-col-xpos",pszColXpos))
	{
		m_iXColumn = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		m_iXColumn = UT_convertToLogicalUnits(pszColXpos);
	}
	UT_DEBUGMSG(("ColXpos for frame is %s \n",pszColXpos));
// colYpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-col-ypos",pszColYpos))
	{
		m_iYColumn = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		m_iYColumn = UT_convertToLogicalUnits(pszColYpos);
	}
	UT_DEBUGMSG(("ColYpos for frame is %s units %d \n",pszColYpos,m_iYColumn));


// PageXpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-page-xpos",pszPageXpos))
	{
		m_iXPage = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		m_iXPage = UT_convertToLogicalUnits(pszPageXpos);
	}
	UT_DEBUGMSG(("PageXpos for frame is %s \n",pszPageXpos));
// PageYpos

	if(!pSectionAP || !pSectionAP->getProperty("frame-page-ypos",pszPageYpos))
	{
		m_iYPage = UT_convertToLogicalUnits("0.0in");
	}
	else
	{
		m_iYPage = UT_convertToLogicalUnits(pszPageYpos);
	}
	UT_DEBUGMSG(("PageYpos for frame is %s units %d \n",pszColYpos,m_iYPage));

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

	const XML_Char * pszBgStyle = NULL;
	const XML_Char * pszBgColor = NULL;
	const XML_Char * pszBackgroundColor = NULL;

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

static void s_background_properties (const XML_Char * pszBgStyle, const XML_Char * pszBgColor,
									 const XML_Char * pszBackgroundColor,
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

static void s_border_properties (const XML_Char * border_color, const XML_Char * border_style, const XML_Char * border_width,
								 const XML_Char * color, PP_PropertyMap::Line & line)
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
					line.m_thickness = static_cast<double>(thickness / UT_PAPER_UNITS_PER_INCH);
				}
			else
				line.m_thickness = UT_convertToLogicalUnits (border_width);

			if (!line.m_thickness)
				{
					double thickness = UT_LAYOUT_RESOLUTION;
					line.m_thickness = static_cast<double>(thickness / UT_PAPER_UNITS_PER_INCH);
				}
		}
	else // ??
		{
			double thickness = UT_LAYOUT_RESOLUTION;
			line.m_thickness = static_cast<double>(thickness / UT_PAPER_UNITS_PER_INCH);
		}
}
