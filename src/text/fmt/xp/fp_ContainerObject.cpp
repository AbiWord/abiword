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

/*!
  Create container
  \param iType Container type
  \param pSectionLayout Section layout type used for this container
 */
fp_ContainerObject::fp_ContainerObject(FP_ContainerType iType, fl_SectionLayout* pSectionLayout)
	:       m_iType(iType),
			m_pSectionLayout(pSectionLayout),
			m_pG(NULL),
			m_iDirection(FRIBIDI_TYPE_UNSET)
{
	UT_ASSERT(pSectionLayout);
	m_pG = m_pSectionLayout->getDocLayout()->getGraphics();
}

/*!
  Destruct container
 */
fp_ContainerObject::~fp_ContainerObject()
{
}


/*!
 * return true is this container is of column type
 */
bool fp_ContainerObject::isColumnType(void) const
{
  bool b = (m_iType == FP_CONTAINER_COLUMN) 
	  || (m_iType == FP_CONTAINER_COLUMN_SHADOW)
	  || (m_iType == FP_CONTAINER_COLUMN_POSITIONED)
	  || (m_iType == FP_CONTAINER_FOOTNOTE)
	  ;
  return b;
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
	m_pPrev(NULL)
{
	m_vecContainers.clear();
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
 * The container that encloses this container.
 */
void fp_Container::setContainer(fp_Container * pCO)
{
	m_pContainer = pCO;
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
		return ((fp_Column *)(pCon))->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN_POSITIONED)
	{
		return ((fp_Column *)(pCon))->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_COLUMN_SHADOW)
	{
		return ((fp_ShadowContainer *)(pCon))->getPage();
	}
	if(pCon->getContainerType() == FP_CONTAINER_FOOTNOTE)
	{
		return ((fp_FootnoteContainer *)(pCon))->getPage();
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
		fp_Column * pColumn = (fp_Column*) pColumnC;
		pDSL = pColumn->getDocSectionLayout();
	}
	else
	{
		fp_FootnoteContainer * pFC = (fp_FootnoteContainer *) pColumnC;
		fl_FootnoteLayout * pFL = (fl_FootnoteLayout *) pFC->getSectionLayout();
		pDSL = (fl_DocSectionLayout *) pFL->myContainingLayout();
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


bool fp_Container::isOnScreen() const
{
	UT_return_val_if_fail(getSectionLayout(),false);
	
	FV_View *pView = getSectionLayout()->getDocLayout()->getView();

	if(!pView)
	{
		return false;
	}
	
	UT_Vector vRect;
	UT_Vector vPages;

	pView->getVisibleDocumentPagesAndRectangles(vRect, vPages);

	UT_uint32 iCount = vPages.getItemCount();
	bool bRet = false;

	if(iCount)
	{
		fp_Page * pMyPage = getPage();

		if(pMyPage)
		{
			for(UT_uint32 i = 0; i < iCount; i++)
			{
				fp_Page * pPage = (fp_Page*)vPages.getNthItem(i);

				if(pPage == pMyPage)
				{
//
// Just getting an on screen page is enough for now I think.
//
					bRet = true;
					break;
#if 0
					UT_Rect r;
					UT_Rect *pR = (UT_Rect*)vRect.getNthItem(i);

					if(!getPageRelativeOffsets(r))
						break;

					bRet = r.intersectsRect(pR);
#endif
					break;
				}
		
			}
		}
		
		UT_VECTOR_PURGEALL(UT_Rect*,vRect);
	}
	
	return bRet;
}


