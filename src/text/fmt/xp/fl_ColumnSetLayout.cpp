
#include "ut_types.h"
#include "pt_Types.h"
#include "ut_assert.h"
#include "fl_ColumnSetLayout.h"
#include "fl_ColumnLayout.h"
#include "pd_Document.h"
#include "fl_SectionLayout.h"
#include "fl_DocLayout.h"


FL_ColumnSetLayout::FL_ColumnSetLayout(FL_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh)
{
	m_sdh = sdh;
	m_pSectionLayout = pSectionLayout;
	m_pFirstColumnLayout = NULL;
	m_pLastColumnLayout = NULL;
	m_vsIndex = 0;
	m_apIndex = 0;
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


void FL_ColumnSetLayout::setPTvars(PT_VarSetIndex vsIndex, PT_AttrPropIndex apIndex)
{
	m_vsIndex = vsIndex;
	m_apIndex = apIndex;
}

UT_Bool FL_ColumnSetLayout::getAttrProp(const PP_AttrProp ** ppAP) const
{
	PD_Document * pDoc = m_pSectionLayout->getLayout()->getDocument();
	return pDoc->getAttrProp(m_vsIndex,m_apIndex,ppAP);
}
