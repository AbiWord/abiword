
#include "ut_types.h"
#include "pt_Types.h"
#include "ut_assert.h"

#include "fl_ColumnSetLayout.h"
#include "fl_Layout.h"
#include "fl_ColumnLayout.h"
#include "pd_Document.h"
#include "fl_SectionLayout.h"
#include "fl_DocLayout.h"


FL_ColumnSetLayout::FL_ColumnSetLayout(FL_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh)
	: fl_Layout(PTX_ColumnSet, sdh)
{
	m_pSectionLayout = pSectionLayout;
	m_pDoc = pSectionLayout->getLayout()->getDocument();
	m_pFirstColumnLayout = NULL;
	m_pLastColumnLayout = NULL;
}

FL_ColumnSetLayout::~FL_ColumnSetLayout()
{
}

FL_SectionLayout * FL_ColumnSetLayout::getSectionLayout(void) const
{
	return m_pSectionLayout;
}

FL_ColumnLayout * FL_ColumnSetLayout::getFirstColumnLayout(void) const
{
	return m_pFirstColumnLayout;
}

void FL_ColumnSetLayout::appendColumnLayout(FL_ColumnLayout * pCL)
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

