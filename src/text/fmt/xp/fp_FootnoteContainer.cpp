/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Patrick Lam <plam@mit.edu>
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
#include <math.h>
#include <string.h>

#include "fp_FootnoteContainer.h"
#include "fp_Column.h"
#include "fp_Page.h"
#include "fp_Line.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fl_FootnoteLayout.h"
#include "fv_View.h"
#include "gr_Painter.h"

/*!
  Create Footnote container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_FootnoteContainer::fp_FootnoteContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_FOOTNOTE, pSectionLayout),
	  m_pPage(NULL)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_FootnoteContainer::~fp_FootnoteContainer()
{
	m_pPage = NULL;
}

void fp_FootnoteContainer::setPage(fp_Page * pPage)
{
	if(pPage && (m_pPage != NULL) && m_pPage != pPage)
	{
		clearScreen();
		m_pPage->removeFootnoteContainer(this);
		getSectionLayout()->markAllRunsDirty();
	}
	m_pPage = pPage;
	if(pPage)
	{
		getFillType()->setParent(pPage->getFillType());
	}
	else
	{
		getFillType()->setParent(NULL);
	}
}

/*! 
 * This method returns the value of the footnote reference (or anchor)
 */
UT_sint32 fp_FootnoteContainer::getValue(void)
{
	fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(getSectionLayout());
	FL_DocLayout * pDL = pFL->getDocLayout();
	return pDL->getFootnoteVal(pFL->getFootnotePID());
}

void fp_FootnoteContainer::clearScreen(void)
{
	if(getPage() == NULL)
	{
		return;
	}
	UT_sint32 pos = getPage()->findFootnoteContainer(this);
	if(pos == 0)
	{
		fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		UT_RGBColor * pBGColor = getFillType()->getColor();
		double iLeftMargin = pDSL->getLeftMargin();
		double iRightMargin = pDSL->getRightMargin();
//		double diff = getPage()->getWidth()/10;
		double diff = 0; // FIXME make a property
		double xoff,yoff;
		getPage()->getScreenOffsets(this,xoff,yoff);
		double xoffStart = xoff  + diff;
		double width = (getPage()->getWidth() - iLeftMargin -iRightMargin)/3;
		double xoffEnd = xoff + width - getGraphics()->tlu(1);

		getGraphics()->setColor(*pBGColor);
		double iLineThick = pDSL->getFootnoteLineThickness();
		getGraphics()->setLineWidth(iLineThick);
		double yline = yoff;
		yline = yline - iLineThick - 4; // FIXME This should not be a magic numer!
		xxx_UT_DEBUGMSG(("fp_TableContainer: clearScreen (%.2f,%.2f) to (%.2f,%.2f) \n",xoffStart,yline,xoffEnd,yline));
		double srcX = getX() + diff -1; // AND THIS
		double srcY = getY() - iLineThick -4; // AND THIS .. sort the mess out please!
		getFillType()->Fill(getGraphics(),srcX,srcY,xoffStart-1, yline, xoffEnd-xoffStart +2, iLineThick+1); // and ALL THE +1 +2 -1... FIX IT!
	}

	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< static_cast<UT_sint32>(countCons()); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		pCon->clearScreen();
	}
}
	
void fp_FootnoteContainer::setContainer(fp_Container * pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer() && (pContainer != NULL))
	{
		clearScreen();
	}
	fp_Container::setContainer(pContainer);
}

fl_DocSectionLayout * fp_FootnoteContainer::getDocSectionLayout(void)
{
	fl_FootnoteLayout * pFL = static_cast<fl_FootnoteLayout *>(getSectionLayout());
	fl_ContainerLayout * pDSL = pFL->myContainingLayout();
	while(pDSL && pDSL->getContainerType() != FL_CONTAINER_DOCSECTION)
	{
		pDSL = pDSL->myContainingLayout();
	}
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return static_cast<fl_DocSectionLayout *>(pDSL);
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_FootnoteContainer::draw(dg_DrawArgs* pDA)
{
	if(getPage() == NULL)
	{
		return;
	}
	UT_sint32 pos = getPage()->findFootnoteContainer(this);
	xxx_UT_DEBUGMSG(("fp_Footnote:draw: pos %d \n",pos));
	if(pos == 0)
	{
		UT_RGBColor black(0,0,0);
		fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		double iLeftMargin = pDSL->getLeftMargin();
		double iRightMargin = pDSL->getRightMargin();
//		double diff = getPage()->getWidth()/10;
		double diff = 0; // FIXME make a property
		double xoffStart = pDA->xoff + diff;
		double width = (getPage()->getWidth() - iLeftMargin -iRightMargin)/3;
		double xoffEnd = pDA->xoff + width;

		double yline = pDA->yoff;
		pDA->pG->setColor(black);
		pDA->pG->setLineProperties(pDA->pG->tlu(1),
									 GR_Graphics::JOIN_MITER,
									 GR_Graphics::CAP_PROJECTING,
									 GR_Graphics::LINE_SOLID);

		double iLineThick = pDSL->getFootnoteLineThickness();
		iLineThick = UT_MAX(1,iLineThick);
		pDA->pG->setLineWidth(iLineThick);
		yline = yline - iLineThick - 3; // FIXME This should not be a magic numer!
		xxx_UT_DEBUGMSG(("Drawline form (%.2f,%.2f) to (%.2f,%.2f) \n",xoffStart,yline,xoffEnd,yline));

		GR_Painter painter (pDA->pG);
		painter.drawLine(xoffStart, yline, xoffEnd, yline);
	}
	xxx_UT_DEBUGMSG(("Footnote: Drawing unbroken footnote %x x %.2f, y %.2f width %.2f height %.2f \n",this,getX(),getY(),getWidth(),getHeight()));

//
// Only draw the lines in the clipping region.
//
	dg_DrawArgs da = *pDA;

	UT_uint32 count = countCons();
	for (UT_uint32 i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));
		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY();
		pContainer->draw(&da);
	}
    _drawBoundaries(pDA);
}

fp_Container * fp_FootnoteContainer::getNextContainerInSection() const
{

	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pNext = pCL->getNext();
	while(pNext && pNext->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		pNext = pNext->getNext();
	}
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_FootnoteContainer::getPrevContainerInSection() const
{

	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	fl_ContainerLayout * pPrev = pCL->getPrev();
	while(pPrev && pPrev->getContainerType() == FL_CONTAINER_ENDNOTE)
	{
		pPrev = pPrev->getPrev();
	}
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}

void fp_FootnoteContainer::layout(void)
{
	_setMaxContainerHeight(0);
	double iY = 0, iPrevY = 0;
	iY= 0;
	fl_DocSectionLayout * pDSL = getDocSectionLayout();
	double iMaxFootHeight = 0;
	iMaxFootHeight = pDSL->getActualColumnHeight();
	iMaxFootHeight -= getGraphics()->tlu(20)*3;
	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		pContainer = static_cast<fp_Container*>(getNthCon(i));
//
// This is to speedup redraws.
//
		if(pContainer->getHeight() > _getMaxContainerHeight())
			_setMaxContainerHeight(pContainer->getHeight());

		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
		}
			
		pContainer->setY(iY);

		double iContainerHeight = pContainer->getHeight();
		double iContainerMarginAfter = pContainer->getMarginAfter();

		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		if(iY > iMaxFootHeight)
		{
			iY = iMaxFootHeight;
		}
		else
		{
			if (pPrevContainer)
			{
				pPrevContainer->setAssignedScreenHeight(iY - iPrevY);
			}
		}
		pPrevContainer = pContainer;
		iPrevY = iY;
	}

	// Correct height position of the last line
	if (pPrevContainer)
	{
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1); // FIX THIS, WE DON'T WANT TO HAVE +1 IN THE CODE!!!
	}

	if (getHeight() == iY)
	{
		return;
	}

	setHeight(iY);
//	UT_ASSERT(pPage);
	fp_Page * pPage = getPage();
	if(pPage)
	{
		pPage->footnoteHeightChanged();
	}
}



/*!
  Create Endnote container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_EndnoteContainer::fp_EndnoteContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_ENDNOTE, pSectionLayout),
	  m_pLocalNext(NULL),
	  m_pLocalPrev(NULL),
	  m_iY(0),
	  m_bOnPage(false),
	  m_bCleared(false)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_EndnoteContainer::~fp_EndnoteContainer()
{
	UT_DEBUGMSG(("deleting endnote container %x \n",this));
	m_pLocalNext = NULL;
	m_pLocalPrev = NULL;
	m_bOnPage = false;
}

void fp_EndnoteContainer::setY(double iY)
{
	if(iY == m_iY)
	{
		return;
	}
	clearScreen();
	m_iY = iY;
}

double fp_EndnoteContainer::getY(void) const
{
	return m_iY;
}

/*! 
 * This method returns the value of the footnote reference (or anchor)
 */
UT_sint32 fp_EndnoteContainer::getValue(void)
{
	fl_EndnoteLayout * pFL = static_cast<fl_EndnoteLayout *>(getSectionLayout());
	FL_DocLayout * pDL = pFL->getDocLayout();
	return pDL->getEndnoteVal(pFL->getEndnotePID());
}

void fp_EndnoteContainer::clearScreen(void)
{
	UT_DEBUGMSG(("Clearscreen on Endnote container %x , height = %d \n",this,getHeight()));
	fl_ContainerLayout * pCL = static_cast<fl_ContainerLayout *>(getSectionLayout());
	pCL->setNeedsRedraw();
	if(!m_bOnPage)
	{
		return;
	}
	if(m_bCleared)
	{
		return;
	}
	if(getColumn() && (getHeight() != 0))
	{
		if(getPage() == NULL)
		{
			return;
		}
		fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		if(pDSL == NULL)
		{
			return;
		}
		double iLeftMargin = pDSL->getLeftMargin();
		double iRightMargin = pDSL->getRightMargin();
		double iWidth = getPage()->getWidth();
		iWidth = iWidth - iLeftMargin - iRightMargin;
		double xoff,yoff;
		static_cast<fp_Column *>(getColumn())->getScreenOffsets(this,xoff,yoff);
		double srcX = getX();
		double srcY = getY();
		getFillType()->Fill(getGraphics(),srcX,srcY,xoff,yoff,iWidth,getHeight());
	}
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< static_cast<UT_sint32>(countCons()); i++)
	{
		xxx_UT_DEBUGMSG(("Clear screen on container %d in endnote \n",i));
		pCon = static_cast<fp_Container *>(getNthCon(i));
		pCon->clearScreen();
	}
	m_bCleared = true;
}

/*!
 * These are for Endnote containers broken over page boundaries
 */
fp_EndnoteContainer * fp_EndnoteContainer::getLocalNext(void)
{
	return m_pLocalNext;
}

/*!
 * These are for Endnote containers broken over page boundaries
 */
fp_EndnoteContainer * fp_EndnoteContainer::getLocalPrev(void)
{
	return m_pLocalPrev;
}

void fp_EndnoteContainer::setContainer(fp_Container * pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer() && (pContainer != NULL))
	{
		clearScreen();
	}
	if(pContainer != NULL)
	{
		m_bOnPage = true;
	}
	else
	{
		m_bOnPage = false;
	}
	fp_Container::setContainer(pContainer);
}

fl_DocSectionLayout * fp_EndnoteContainer::getDocSectionLayout(void)
{
	fl_EndnoteLayout * pFL = static_cast<fl_EndnoteLayout *>(getSectionLayout());
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pFL->myContainingLayout());
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return pDSL;
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_EndnoteContainer::draw(dg_DrawArgs* pDA)
{
	xxx_UT_DEBUGMSG(("Endnote: Drawing unbroken Endnote %x x %.2fd, y %.2f width %.2f height %.2f \n",this,getX(),getY(),getWidth(),getHeight()));
	xxx_UT_DEBUGMSG(("pDA yoff %d \n",pDA->yoff));
	const UT_Rect * pClipRect = pDA->pG->getClipRect();
	if(pClipRect)
	{
		UT_DEBUGMSG(("clip y %.2f height %.2f \n",pClipRect->top, pClipRect->height));
	}
	m_bCleared = false;
//
// Only draw the lines in the clipping region.
//
	dg_DrawArgs da = *pDA;

	UT_uint32 count = countCons();
	for (UT_uint32 i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = static_cast<fp_ContainerObject*>(getNthCon(i));
		xxx_UT_DEBUGMSG(("Drawing Endnote container %d pointer %x \n",i,pContainer));
		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY();
		pContainer->draw(&da);
	}
    _drawBoundaries(pDA);
}

fp_Container * fp_EndnoteContainer::getNextContainerInSection() const
{
	return static_cast<fp_Container *>(getNext());
}


fp_Container * fp_EndnoteContainer::getPrevContainerInSection() const
{
	return static_cast<fp_Container *>(getPrev());
}

void fp_EndnoteContainer::layout(void)
{
	_setMaxContainerHeight(0);
	double iY = 0, iPrevY = 0;
	iY= 0;
	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		pContainer = static_cast<fp_Container*>(getNthCon(i));
//
// This is to speedup redraws.
//
		if(pContainer->getHeight() > _getMaxContainerHeight())
			_setMaxContainerHeight(pContainer->getHeight());

		if(pContainer->getY() != iY)
		{
			pContainer->clearScreen();
		}
			
		pContainer->setY(iY);
		double iContainerHeight = pContainer->getHeight();
		double iContainerMarginAfter = pContainer->getMarginAfter();
		if (pPrevContainer)
		{
			pPrevContainer->setAssignedScreenHeight(iY - iPrevY);
		}
		iPrevY = iY;

		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		pPrevContainer = pContainer;
	}

	// Correct height position of the last line
	if (pPrevContainer)
	{
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
	}

	double iNewHeight = iY;

	if (getHeight() == iNewHeight)
	{
		return;
	}
	setHeight(iNewHeight);
	fl_EndnoteLayout * pEL = static_cast<fl_EndnoteLayout *>(getSectionLayout());
	FL_DocLayout * pDL = pEL->getDocLayout();
	fl_DocSectionLayout * pDSL = pDL->getDocSecForEndnote(this);
	fp_Page * pPage = getPage();
	pDSL->setNeedsSectionBreak(true,pPage);
}
