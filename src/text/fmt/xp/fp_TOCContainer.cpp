/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fp_TOCContainer.h"
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
#include "fl_TOCLayout.h"
#include "fv_View.h"
#include "gr_Painter.h"

/*!
  Create Table Of Contents container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_TOCContainer::fp_TOCContainer(fl_SectionLayout* pSectionLayout) 
	: fp_VerticalContainer(FP_CONTAINER_TOC, pSectionLayout)
{
}

/*!
  Destruct container
  \note The Containers in vector of the container are not
        destructed. They are owned by the logical hierarchy (i.e.,
		the fl_Container classes like fl_BlockLayout), not the physical
        hierarchy.
 */
fp_TOCContainer::~fp_TOCContainer()
{
}

/*! 
 * This method returns the value of the TOC reference (or anchor)
 */
UT_sint32 fp_TOCContainer::getValue(void)
{
	fl_TOCLayout * pTL = static_cast<fl_TOCLayout *>(getSectionLayout());
	return pTL->getTOCPID();
}

void fp_TOCContainer::clearScreen(void)
{
	if(getPage() == NULL)
	{
		return;
	}
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< static_cast<UT_sint32>(countCons()); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		pCon->clearScreen();
	}
}


void fp_TOCContainer::forceClearScreen(void)
{
	if(getPage() == NULL)
	{
		return;
	}
	fp_Container * pCon = NULL;
	UT_sint32 i = 0;
	for(i=0; i< static_cast<UT_sint32>(countCons()); i++)
	{
		pCon = static_cast<fp_Container *>(getNthCon(i));
		if(pCon->getContainerType() == FP_CONTAINER_LINE)
		{
			static_cast<fp_Line *>(pCon)->setScreenCleared(false);
		}
		pCon->clearScreen();
	}
}
	
void fp_TOCContainer::setContainer(fp_Container * pContainer)
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

fl_DocSectionLayout * fp_TOCContainer::getDocSectionLayout(void)
{
	fl_TOCLayout * pTL = static_cast<fl_TOCLayout *>(getSectionLayout());
	fl_DocSectionLayout * pDSL = static_cast<fl_DocSectionLayout *>(pTL->myContainingLayout());
	UT_ASSERT(pDSL && (pDSL->getContainerType() == FL_CONTAINER_DOCSECTION));
	return pDSL;
}


/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_TOCContainer::draw(GR_Graphics * pG)
{
	if(getPage() == NULL)
	{
		return;
	}
	dg_DrawArgs da;
	da.pG = pG;
	UT_Rect * pR = getScreenRect();
	da.xoff = pR->left;
	da.yoff = pR->top;
	da.bDirtyRunsOnly = false;
	delete pR;
	draw(&da);
}
/*!
 Draw container content
 \param pDA Draw arguments
 */
void fp_TOCContainer::draw(dg_DrawArgs* pDA)
{
	if(getPage() == NULL)
	{
		return;
	}

	xxx_UT_DEBUGMSG(("Footnote: Drawing unbroken footnote %x x %d, y %d width %d height %d \n",this,getX(),getY(),getWidth(),getHeight()));

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

fp_Container * fp_TOCContainer::getNextContainerInSection() const
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


fp_Container * fp_TOCContainer::getPrevContainerInSection() const
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

bool fp_TOCContainer::isVBreakable(void)
{
	return false;
}

fp_ContainerObject * fp_TOCContainer::VBreakAt(UT_sint32 vpos)
{
	return NULL;
}

UT_sint32  fp_TOCContainer::wantVBreakAt(UT_sint32 vpos)
{
	return vpos;
}

/*!
 * vpos is relative to the height of the TOC container, except if the
 * previous container was broken, in which case it's relative to the height
 * from the previous Ybreak.
 */
UT_sint32 wantVBreatAt(UT_sint32 vpos)
{
	return 0;
}

void fp_TOCContainer::layout(void)
{
	_setMaxContainerHeight(0);
	UT_sint32 iY = 0, iPrevY = 0;
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
}

