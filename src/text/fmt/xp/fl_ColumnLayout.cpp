
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "pt_Types.h"

#include "fl_ColumnSetLayout.h"
#include "fl_Layout.h"
#include "fl_ColumnLayout.h"
#include "pd_Document.h"
#include "fl_SectionLayout.h"
#include "fl_DocLayout.h"
#include "pp_AttrProp.h"
#include "fp_Column.h"


FL_ColumnLayout::FL_ColumnLayout(FL_ColumnSetLayout * pCSL, PL_StruxDocHandle sdh)
	: fl_Layout(PTX_Column, sdh)
{
	m_pColumnSetLayout = pCSL;
	m_pDoc = m_pColumnSetLayout->getSectionLayout()->getLayout()->getDocument();
	m_prev = NULL;
	m_next = NULL;
}

FL_ColumnLayout::~FL_ColumnLayout()
{
}

FL_ColumnLayout * FL_ColumnLayout::setNext(FL_ColumnLayout * pCL)
{
	FL_ColumnLayout * pOld = m_next;
	m_next = pCL;
	return pOld;
}

FL_ColumnLayout * FL_ColumnLayout::setPrev(FL_ColumnLayout * pCL)
{
	FL_ColumnLayout * pOld = m_prev;
	m_prev = pCL;
	return pOld;
}

FL_ColumnLayout * FL_ColumnLayout::getNext(void) const
{
	return m_next;
}

FL_ColumnLayout * FL_ColumnLayout::getPrev(void) const
{
	return m_prev;
}

UT_Bool FL_ColumnLayout::getNewColumn(UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
									  FP_Column ** ppCol,
									  UT_sint32 * piXoff, UT_sint32 * piYoff) const
{
	// TODO make this routine fail rather than assert.
	
	UT_Bool bResult;
	FL_SectionLayout * pSL = m_pColumnSetLayout->getSectionLayout();
	PD_Document * pDoc = pSL->getLayout()->getDocument();
	UT_ASSERT(pDoc);

	const PP_AttrProp * pAP = NULL;

	bResult = pDoc->getAttrProp(m_apIndex,&pAP);
	UT_ASSERT(bResult && pAP);

	const XML_Char * szValueType;
	bResult = pAP->getAttribute("type",szValueType);
	UT_ASSERT(bResult);

	FP_Column * pCol = NULL;
	if (UT_XML_stricmp(szValueType,FP_BoxColumn::myTypeName()) == 0)
		pCol = new FP_BoxColumn(pSL,pAP,iWidthGiven,iHeightGiven, piXoff,piYoff);
	else if (UT_XML_stricmp(szValueType,FP_CircleColumn::myTypeName()) == 0)
		pCol = new FP_CircleColumn(pSL,pAP,iWidthGiven,iHeightGiven, piXoff,piYoff);
	else
	{
		UT_DEBUGMSG(("unknown column type\n"));
		UT_ASSERT(0);
	}
	if (!pCol)
	{
		UT_DEBUGMSG(("could not create column of type %s\n",szValueType));
		UT_ASSERT(0);
	}

	*ppCol = pCol;
	return UT_TRUE;
}

