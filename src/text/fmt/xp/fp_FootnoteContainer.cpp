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
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "gr_DrawArgs.h"
#include "ut_vector.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "fl_FootnoteLayout.h"

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
}

/*! 
 * This method returns the value of the footnote reference (or anchor)
 */
UT_sint32 fp_FootnoteContainer::getValue(void)
{
	fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) getSectionLayout();
	FL_DocLayout * pDL = pFL->getDocLayout();
	return pDL->getFootnoteVal(pFL->getFootnotePID());
}

void fp_FootnoteContainer::clearScreen(void)
{

	UT_sint32 pos = getPage()->findFootnoteContainer(this);
	if(pos == 0)
	{
		fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		UT_RGBColor * pBGColor = pDSL->getPaperColor();
		UT_sint32 iLeftMargin = pDSL->getLeftMargin();
		UT_sint32 iRightMargin = pDSL->getRightMargin();
		UT_sint32 diff = getPage()->getWidth()/10;
		UT_sint32 xoff,yoff;
		getPage()->getScreenOffsets(this,xoff,yoff);
		UT_sint32 xoffStart = xoff  + diff;
		UT_sint32 xoffEnd = xoff + getPage()->getWidth() - iLeftMargin -iRightMargin - diff;
		getGraphics()->setColor(*pBGColor);
		UT_sint32 iLineThick = pDSL->getFootnoteLineThickness();
		getGraphics()->setLineWidth(iLineThick);
		UT_sint32 yline = yoff;
		yline = yline - iLineThick - 4; // FIXME This should not be a magic numer!
		xxx_UT_DEBUGMSG(("fp_TableContainer: clearScreen (%d,%d) to (%d,%d) \n",xoffStart,yline,xoffEnd,yline));
		getGraphics()->fillRect(*pBGColor,xoffStart-1, yline, xoffEnd-xoffStart +2, iLineThick+1);
	}

	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< (UT_sint32) countCons(); i++)
	{
		pCon = (fp_Container *) getNthCon(i);
		pCon->clearScreen();
	}
}
	
void fp_FootnoteContainer::setContainer(fp_Container * pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer())
	{
		clearScreen();
	}
	fp_Container::setContainer(pContainer);
}

fl_DocSectionLayout * fp_FootnoteContainer::getDocSectionLayout(void)
{
	fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) getSectionLayout();
	fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *) pFL->myContainingLayout();
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return pDSL;
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_FootnoteContainer::draw(dg_DrawArgs* pDA)
{
	UT_sint32 pos = getPage()->findFootnoteContainer(this);
	xxx_UT_DEBUGMSG(("fp_Footnote:draw: pos %d \n",pos));
	if(pos == 0)
	{
		UT_RGBColor black(0,0,0);
		fl_DocSectionLayout * pDSL = getPage()->getOwningSection();
		UT_sint32 iLeftMargin = pDSL->getLeftMargin();
		UT_sint32 iRightMargin = pDSL->getRightMargin();
		UT_sint32 diff = getPage()->getWidth()/10;
		UT_sint32 xoffStart = pDA->xoff + diff;
		UT_sint32 xoffEnd = pDA->xoff + getPage()->getWidth() -iLeftMargin - iRightMargin - diff;
		UT_sint32 yline = pDA->yoff;
		pDA->pG->setColor(black);
		pDA->pG->setLineProperties(1.0,
									 GR_Graphics::JOIN_MITER,
									 GR_Graphics::CAP_BUTT,
									 GR_Graphics::LINE_SOLID);

		UT_sint32 iLineThick = pDSL->getFootnoteLineThickness();
		iLineThick = UT_MAX(1,iLineThick);
		pDA->pG->setLineWidth(iLineThick);
		yline = yline - iLineThick - 3; // FIXME This should not be a magic numer!
		xxx_UT_DEBUGMSG(("Drawline form (%d,%d) to (%d,%d) \n",xoffStart,yline,xoffEnd,yline));
		pDA->pG->drawLine(xoffStart, yline, xoffEnd, yline);
	}
	xxx_UT_DEBUGMSG(("Footnote: Drawing unbroken footnote %x x %d, y %d width %d height %d \n",this,getX(),getY(),getWidth(),getHeight()));

//
// Only draw the lines in the clipping region.
//
	dg_DrawArgs da = *pDA;

	UT_uint32 count = countCons();
	for (UT_uint32 i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);
		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY();
		pContainer->draw(&da);
	}
    _drawBoundaries(pDA);
}

fp_Container * fp_FootnoteContainer::getNextContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pNext = pCL->getNext();
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_FootnoteContainer::getPrevContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pPrev = pCL->getPrev();
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}

void fp_FootnoteContainer::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
	iY= 0;
	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		pContainer = (fp_Container*) getNthCon(i);
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

		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();

		iY += iContainerHeight;
		iY += iContainerMarginAfter;
		//iY +=  0.5;

		if (pPrevContainer)
		{
			pPrevContainer->setAssignedScreenHeight(iY - iPrevY);
		}
		pPrevContainer = pContainer;
		iPrevY = iY;
	}

	// Correct height position of the last line
	if (pPrevContainer)
	{
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
	}

	if (getHeight() == iY)
	{
		return;
	}

	setHeight(iY);
	getPage()->footnoteHeightChanged();
}



/*!
  Create Endnote container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_EndnoteContainer::fp_EndnoteContainer(fl_SectionLayout* pSectionLayout) 
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
fp_EndnoteContainer::~fp_EndnoteContainer()
{
	m_pPage = NULL;
}

void fp_EndnoteContainer::setPage(fp_Page * pPage)
{
	if(pPage && (m_pPage != NULL) && m_pPage != pPage)
	{
		clearScreen();
		getSectionLayout()->markAllRunsDirty();
	}
	m_pPage = pPage;
}

/*! 
 * This method returns the value of the footnote reference (or anchor)
 */
UT_sint32 fp_EndnoteContainer::getValue(void)
{
	fl_EndnoteLayout * pFL = (fl_EndnoteLayout *) getSectionLayout();
	FL_DocLayout * pDL = pFL->getDocLayout();
	return pDL->getEndnoteVal(pFL->getEndnotePID());
}

void fp_EndnoteContainer::clearScreen(void)
{
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< (UT_sint32) countCons(); i++)
	{
		pCon = (fp_Container *) getNthCon(i);
		pCon->clearScreen();
	}
}
	
void fp_EndnoteContainer::setContainer(fp_Container * pContainer)
{
	if (pContainer == getContainer())
	{
		return;
	}

	if (getContainer())
	{
		clearScreen();
	}
	fp_Container::setContainer(pContainer);
}

fl_DocSectionLayout * fp_EndnoteContainer::getDocSectionLayout(void)
{
	fl_EndnoteLayout * pFL = (fl_EndnoteLayout *) getSectionLayout();
	fl_DocSectionLayout * pDSL = (fl_DocSectionLayout *) pFL->myContainingLayout();
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return pDSL;
}

/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_EndnoteContainer::draw(dg_DrawArgs* pDA)
{
	UT_DEBUGMSG(("Endnote: Drawing unbroken Endnote %x x %d, y %d width %d height %d \n",this,getX(),getY(),getWidth(),getHeight()));

//
// Only draw the lines in the clipping region.
//
	dg_DrawArgs da = *pDA;

	UT_uint32 count = countCons();
	for (UT_uint32 i = 0; i<count; i++)
	{
		fp_ContainerObject* pContainer = (fp_ContainerObject*) getNthCon(i);
		da.xoff = pDA->xoff + pContainer->getX();
		da.yoff = pDA->yoff + pContainer->getY();
		pContainer->draw(&da);
	}
    _drawBoundaries(pDA);
}

fp_Container * fp_EndnoteContainer::getNextContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pNext = pCL->getNext();
	if(pNext)
	{
		return pNext->getFirstContainer();
	}
	return NULL;
}


fp_Container * fp_EndnoteContainer::getPrevContainerInSection() const
{

	fl_ContainerLayout * pCL = (fl_ContainerLayout *) getSectionLayout();
	fl_ContainerLayout * pPrev = pCL->getPrev();
	if(pPrev)
	{
		return pPrev->getLastContainer();
	}
	return NULL;
}

void fp_EndnoteContainer::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
	iY= 0;
	UT_uint32 iCountContainers = countCons();
	fp_Container *pContainer, *pPrevContainer = NULL;
	for (UT_uint32 i=0; i < iCountContainers; i++)
	{
		pContainer = (fp_Container*) getNthCon(i);
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
		UT_sint32 iContainerHeight = pContainer->getHeight();
		UT_sint32 iContainerMarginAfter = pContainer->getMarginAfter();

		iY += iContainerHeight;
		iY += iContainerMarginAfter;

		if (pPrevContainer)
		{
			pPrevContainer->setAssignedScreenHeight(iY - iPrevY);
		}
		pPrevContainer = pContainer;
		iPrevY = iY;
	}

	// Correct height position of the last line
	if (pPrevContainer)
	{
		pPrevContainer->setAssignedScreenHeight(iY - iPrevY + 1);
	}

	UT_sint32 iNewHeight = iY;

	if (getHeight() == iNewHeight)
	{
		return;
	}

	setHeight(iNewHeight);
	getPage()->footnoteHeightChanged();
}
