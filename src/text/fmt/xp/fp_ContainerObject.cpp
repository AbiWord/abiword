/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior
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

#include <stdlib.h>
#include <string.h>

#include "fp_ContainerObject.h"
#include "fl_SectionLayout.h"
#include "fl_DocLayout.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fp_Column.h"
#include "fv_View.h"
#include "fp_FootnoteContainer.h"
#include "fl_FootnoteLayout.h"
#include "fl_DocLayout.h"
#include "fp_Column.h"

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_ContainerObject::fp_ContainerObject(FP_ContainerType iType, fl_SectionLayout* pSectionLayout)
	:       m_iConType(iType),
			m_pSectionLayout(pSectionLayout),
			m_iDirection(FRIBIDI_TYPE_UNSET)
{
	UT_ASSERT(pSectionLayout);
}

/*!
  Destruct container
 */
fp_ContainerObject::~fp_ContainerObject()
{
	m_iConType = static_cast<FP_ContainerType>(-1);
}

/*!
 * return true is this container is of column type
 */
bool fp_ContainerObject::isColumnType(void) const
{
  bool b = (m_iConType == FP_CONTAINER_COLUMN) 
	  || (m_iConType == FP_CONTAINER_COLUMN_SHADOW)
	  || (m_iConType == FP_CONTAINER_COLUMN_POSITIONED)
	  || (m_iConType == FP_CONTAINER_FOOTNOTE)
	  ;
  return b;
}

/*!
 * Return a pointer to the Graphics class for this object
 */
GR_Graphics * fp_ContainerObject::getGraphics(void) const
{
	return getSectionLayout()->getDocLayout()->getGraphics();
}

/*
 *----------------------------------------------------------------------
 */

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_Container::fp_Container(FP_ContainerType iType, fl_SectionLayout* pSectionLayout)
	:       fp_ContainerObject(iType, pSectionLayout),
			m_pContainer(NULL),
			m_pNext(NULL),
			m_pPrev(NULL),
			m_pMyBrokenContainer(NULL),
			m_FillType(NULL,this,FG_FILL_TRANSPARENT)
{
	m_vecContainers.clear();
	m_FillType.setDocLayout(pSectionLayout->getDocLayout());
}

/*!
  Destruct container
 */
fp_Container::~fp_Container()
{
//
// The containers referenced in the m_vecContainers vector are owned by their
// respective fl_Layout classes and so are not destructed here.
//
}

/*!
 * This returns a pointer to a container broken across a page that
 * contains this container. This is used (in the first instance) by the
 * Table pagination code which breaks tables across pages. This returns
 * the pointer to the broken table this container is within.
 */
fp_Container * fp_Container::getMyBrokenContainer(void) const
{
	return m_pMyBrokenContainer;
}

/*!
 * Sets the pointer to the broken table which contains this container.
 */
void fp_Container::setMyBrokenContainer(fp_Container * pMyBroken)
{
	m_pMyBrokenContainer = pMyBroken;
}

/*!
 * Recursive clears all broken containers contained by this container
 */
void fp_Container::clearBrokenContainers(void)
{
        if(m_pMyBrokenContainer)  // avoid unnecessarily dirtying of memory pages
	        m_pMyBrokenContainer = NULL;

	UT_uint32 i =0;
	for(i=0;i<countCons();i++)
	{
		fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon)
		{
			pCon->clearBrokenContainers();
		}
	}
}

/*!
 * The container that encloses this container.
 */
void fp_Container::setContainer(fp_Container * pCO)
{
	m_pContainer = pCO;
	if(pCO != NULL)
	{
		m_FillType.setParent(pCO->getFillType());
	}
	else
	{
		m_FillType.setParent(NULL);
	}
}

/*!
 * Return the container that encloses this container.
 */
fp_Container * fp_Container::getContainer(void) const
{
	return m_pContainer;
}


/*!
 * Return the column that encloses this container.
 */
fp_Container* fp_Container::getColumn(void) const
{
	const fp_Container * pCon = this;
	while(pCon && ((!pCon->isColumnType())) )
	{
		pCon = pCon->getContainer();
	}
	return const_cast<fp_Container *>(pCon);
}

/*!
 * Get the page containing this container.
 */
fp_Page * fp_Container::getPage(void) const
{
	fp_Container * pCon = getColumn();
	if(pCon == NULL)
	{
		return NULL;
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN)
	{
		return static_cast<fp_Column *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN_POSITIONED)
	{
		return static_cast<fp_Column *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		return static_cast<fp_ShadowContainer *>(pCon)->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_FOOTNOTE)
	{
		return static_cast<fp_FootnoteContainer *>(pCon)->getPage();
	}
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

bool fp_Container::getPageRelativeOffsets(UT_Rect &r) const
{
	// the X offset of a container is relative to page margin, what we
	// want is offset relative to the page edge
	fp_Container * pColumnC = getColumn();
	
	UT_return_val_if_fail(pColumnC,false);
	fl_DocSectionLayout * pDSL = NULL;
	if(pColumnC->getContainerType() != FP_CONTAINER_FOOTNOTE)
	{
		fp_Column * pColumn = static_cast<fp_Column*>(pColumnC);
		pDSL = pColumn->getDocSectionLayout();
	}
	else
	{
		fp_FootnoteContainer * pFC = static_cast<fp_FootnoteContainer *>(pColumnC);
		fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(pFC->getSectionLayout());
		pDSL = static_cast<fl_DocSectionLayout *>(pFL->myContainingLayout());
	}
	UT_return_val_if_fail(pDSL,false);
	UT_ASSERT(pDSL->getContainerType() == FL_CONTAINER_DOCSECTION);
	r.left   = pDSL->getLeftMargin();
	r.top    = pDSL->getTopMargin();
	r.width  = getDrawingWidth();
	r.height = getHeight();

	r.left += getX();
	r.top  += getY();
	return true;
}

void  fp_Container::deleteNthCon(UT_sint32 i)
{
	fp_Container * pCon = static_cast<fp_Container *>(getNthCon(i));
	if(pCon->getContainer() == this)
	{
		UT_ASSERT(0);
		pCon->setContainer(NULL);
	}
	m_vecContainers.deleteNthItem(i);
}

bool fp_Container::isOnScreen() const
{
	UT_return_val_if_fail(getSectionLayout(),false);
	
	FV_View *pView = getSectionLayout()->getDocLayout()->getView();

	if(!pView)
	{
		return false;
	}
	if(getPage())
	{
		return getPage()->isOnScreen();
	}
	return false;
}

/*!
 * Need to be able to fiddle with the fillType so don't do a const return;
 */
fg_FillType * fp_Container::getFillType(void)
{
	return & m_FillType;
}

/*!
 * To draw the background under text. Basic idea is to draw the colour
 * or image defined here unless the fill type is transparent. If't transparent
 * we recursively call it's parents until it's not transparent.
 */
fg_FillType::fg_FillType(fg_FillType *pParent, fp_ContainerObject * pContainer, FG_Fill_Type iType):
	m_pParent(pParent),
	m_pContainer(pContainer),
	m_pDocLayout(NULL),
	m_FillType(iType),
	m_pImage(NULL),
	m_pGraphic(NULL),
	m_iGraphicTick(0),
	m_bTransparentForPrint(false),
	m_color(255,255,255)
{
}

fg_FillType::~fg_FillType(void)
{
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
}

/*!
 * Set the parent of this fill class.
 */
void  fg_FillType::setParent(fg_FillType * pParent)
{
	m_pParent = pParent;
}

/*!
 * set this class to have a solid color fill
 */
void fg_FillType::setColor(UT_RGBColor & color)
{
	UT_DEBUGMSG(("Fill type set to color class \n"));
	m_FillType = FG_FILL_COLOR;
	m_color = color;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
}

/*!
 * set this class to have a solid color fill unless this is a NULL string
 * pointer 
 */
void fg_FillType::setColor(const char * pszColor)
{
	if(pszColor)
	{
		if(UT_strcmp(pszColor,"transparent") == 0)
		{
			m_FillType = FG_FILL_TRANSPARENT;
		}
		else
		{
			m_FillType = FG_FILL_COLOR;
		}
		m_color.setColor(pszColor);
	}
	else
	{
		m_FillType = FG_FILL_TRANSPARENT;
	}
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);
}

/*!
 * set this class to be transparent.
 */
void fg_FillType::setTransparent(void)
{
	m_FillType = FG_FILL_TRANSPARENT;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);	
}

/*!
 * set this class to have an image background for fills.
 */
void fg_FillType::setImage(FG_Graphic * pGraphic, GR_Image * pImage)
{
	m_FillType = FG_FILL_IMAGE;
	DELETEP(m_pImage);
	DELETEP(m_pGraphic);	
	m_pImage = pImage;
	m_pGraphic = pGraphic;
}

/*!
 * Set the doc layout for this class.
 */
void  fg_FillType::setDocLayout(FL_DocLayout * pDocLayout)
{
	m_pDocLayout = pDocLayout;
	if(m_pDocLayout)
	{
		m_iGraphicTick = m_pDocLayout->getGraphicTick();
	}
}

/*!
 * Set that the fill should be transperent if we're printing.
 */
void fg_FillType::setTransparentForPrint(bool bTransparentForPrint)
{
	m_bTransparentForPrint = bTransparentForPrint;
}

/*!
 * Return the parent of this class.
 */
fg_FillType * fg_FillType::getParent(void) const
{
	return m_pParent;
}

/*!
 * Return the filltype of this class.
 */
FG_Fill_Type fg_FillType::getFillType(void) const
{
	return m_FillType;
}

void fg_FillType::_regenerateImage(GR_Graphics * pG)
{
	UT_return_if_fail(m_pGraphic);
	UT_return_if_fail(m_pDocLayout);
	DELETEP(m_pImage);
	m_pImage = m_pGraphic->regenerateImage(pG);
	m_iGraphicTick = m_pDocLayout->getGraphicTick();
}

/*!
 * Actually do the fill for this class.
 */
void fg_FillType::Fill(GR_Graphics * pG, UT_sint32 & srcX, UT_sint32 & srcY, UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
{
	UT_Rect src;
	UT_Rect dest;
	xxx_UT_DEBUGMSG(("----Called fill -- Parent = %x Container %x FillType %d \n",m_pParent,m_pContainer,m_FillType));
	if(!pG->queryProperties(GR_Graphics::DGP_SCREEN))
	{
		if(m_bTransparentForPrint)
		{
			if(getParent() && m_pContainer)
			{
				 UT_sint32 newX = x + (m_pContainer->getX());
				 UT_sint32 newY = y + (m_pContainer->getY());
				 getParent()->Fill(pG,newX,newY,x,y,width,height);
				 return;
			 }
			 return;
		 }
		 if(m_FillType == FG_FILL_TRANSPARENT)
		 {
			 if(getParent() && m_pContainer )
			 {
				 UT_sint32 newX = srcX + (m_pContainer->getX());
				 UT_sint32 newY = srcY + (m_pContainer->getY());
				 getParent()->Fill(pG,newX,newY,x,y,width,height);
				 return;
			 }
			 return;
		 }
		 if(m_FillType == FG_FILL_IMAGE)
		 {
			 _regenerateImage(pG);
			 m_iGraphicTick = 99999999;
			 src.left = srcX;
			 src.top = srcY;
			 src.width = width;
			 src.height = height;
			 dest.left = x;
			 dest.top = y;
			 dest.width = width;
			 dest.height = height;
			 pG->fillRect(m_pImage,src,dest);
			 return;
		 }
		 if(m_FillType == FG_FILL_COLOR)
		 {
			 pG->fillRect(m_color,x,y,width,height);
			 return;
		 }
	 }
	 if(m_FillType == FG_FILL_TRANSPARENT)
	 {
		 xxx_UT_DEBUGMSG(("Fill type transparent ! \n"));
		 if(getParent() && m_pContainer)
		 {
			 xxx_UT_DEBUGMSG(("Fill type transparent chaining up to parent %x ! \n",getParent()));
			 UT_sint32 newX = srcX + (m_pContainer->getX());
			 UT_sint32 newY = srcY + (m_pContainer->getY());
			 getParent()->Fill(pG,newX,newY,x,y,width,height);
			 return;
		 }
		 xxx_UT_DEBUGMSG(("Fill type transparent but no parent ! \n"));
		 UT_RGBColor white(255,255,255);
		 pG->fillRect(white,x,y,width,height);
		 return;
	 }
	 if(m_FillType == FG_FILL_COLOR)
	 {
		 xxx_UT_DEBUGMSG(("Fill type Color ! \n"));
		 pG->fillRect(m_color,x,y,width,height);
		 return;
	 }
	 if(m_FillType == FG_FILL_IMAGE)
	 {
		 xxx_UT_DEBUGMSG(("Fill type Image ! \n"));
		 if(m_pDocLayout->getGraphicTick() != m_iGraphicTick)
		 {
			 _regenerateImage(pG);
		 }
		src.left = srcX;
		src.top = srcY;
		src.width = width;
		src.height = height;
		dest.left = x;
		dest.top = y;
		dest.width = width;
		dest.height = height;
		pG->fillRect(m_pImage,src,dest);
	}		
	return;
}

