/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


fl_ColumnLayout::fl_ColumnLayout(fl_ColumnSetLayout * pCSL, PL_StruxDocHandle sdh)
	: fl_Layout(PTX_Column, sdh)
{
	m_pColumnSetLayout = pCSL;
	m_pDoc = m_pColumnSetLayout->getSectionLayout()->getLayout()->getDocument();
	m_prev = NULL;
	m_next = NULL;
}

fl_ColumnLayout::~fl_ColumnLayout()
{
}

fl_ColumnLayout * fl_ColumnLayout::setNext(fl_ColumnLayout * pCL)
{
	fl_ColumnLayout * pOld = m_next;
	m_next = pCL;
	return pOld;
}

fl_ColumnLayout * fl_ColumnLayout::setPrev(fl_ColumnLayout * pCL)
{
	fl_ColumnLayout * pOld = m_prev;
	m_prev = pCL;
	return pOld;
}

fl_ColumnLayout * fl_ColumnLayout::getNext(void) const
{
	return m_next;
}

fl_ColumnLayout * fl_ColumnLayout::getPrev(void) const
{
	return m_prev;
}

UT_Bool fl_ColumnLayout::getNewColumn(UT_uint32 iWidthGiven, UT_uint32 iHeightGiven,
									  fp_Column ** ppCol,
									  UT_sint32 * piXoff, UT_sint32 * piYoff) const
{
	// TODO make this routine fail rather than assert.
	
	UT_Bool bResult;
	fl_SectionLayout * pSL = m_pColumnSetLayout->getSectionLayout();
	PD_Document * pDoc = pSL->getLayout()->getDocument();
	UT_ASSERT(pDoc);

	const PP_AttrProp * pAP = NULL;

	bResult = pDoc->getAttrProp(m_apIndex,&pAP);
	UT_ASSERT(bResult && pAP);

	const XML_Char * szValueType;
	bResult = pAP->getAttribute("type",szValueType);
	UT_ASSERT(bResult);

	fp_Column * pCol = NULL;
	if (UT_XML_stricmp(szValueType,fp_BoxColumn::myTypeName()) == 0)
		pCol = new fp_BoxColumn(pSL,pAP,iWidthGiven,iHeightGiven, piXoff,piYoff);
	else if (UT_XML_stricmp(szValueType,fp_CircleColumn::myTypeName()) == 0)
		pCol = new fp_CircleColumn(pSL,pAP,iWidthGiven,iHeightGiven, piXoff,piYoff);
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

