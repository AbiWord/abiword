/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#include "ut_types.h"
#include "pt_Types.h"
#include "ut_assert.h"

#include "fl_ColumnSetLayout.h"
#include "fl_Layout.h"
#include "fl_ColumnLayout.h"
#include "pd_Document.h"
#include "fl_SectionLayout.h"
#include "fl_DocLayout.h"


fl_ColumnSetLayout::fl_ColumnSetLayout(fl_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh)
	: fl_Layout(PTX_ColumnSet, sdh)
{
	m_pSectionLayout = pSectionLayout;
	m_pDoc = pSectionLayout->getLayout()->getDocument();
	m_pFirstColumnLayout = NULL;
	m_pLastColumnLayout = NULL;
}

fl_ColumnSetLayout::~fl_ColumnSetLayout()
{
	while (m_pFirstColumnLayout)
	{
		fl_ColumnLayout* pNext = m_pFirstColumnLayout->getNext();
		delete m_pFirstColumnLayout;
		m_pFirstColumnLayout = pNext;
	}
	
	m_pLastColumnLayout = NULL;
}

fl_SectionLayout * fl_ColumnSetLayout::getSectionLayout(void) const
{
	return m_pSectionLayout;
}

fl_ColumnLayout * fl_ColumnSetLayout::getFirstColumnLayout(void) const
{
	return m_pFirstColumnLayout;
}

void fl_ColumnSetLayout::appendColumnLayout(fl_ColumnLayout * pCL)
{
	if (!m_pLastColumnLayout)
	{
		UT_ASSERT(!m_pFirstColumnLayout);
		m_pFirstColumnLayout = pCL;
		m_pLastColumnLayout = pCL;
		pCL->setNext(NULL);
		pCL->setPrev(NULL);
	}
	else
	{
		m_pLastColumnLayout->setNext(pCL);
		pCL->setPrev(m_pLastColumnLayout);
		m_pLastColumnLayout = pCL;
		pCL->setNext(NULL);
	}
}

