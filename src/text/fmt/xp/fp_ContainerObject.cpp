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
	while(pCon && ((pCon->getContainerType() != FP_CONTAINER_COLUMN) && (pCon->getContainerType() != FP_CONTAINER_COLUMN_POSITIONED) &&  (pCon->getContainerType() != FP_CONTAINER_COLUMN_SHADOW)) )
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
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}
