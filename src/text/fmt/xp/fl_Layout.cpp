 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#include "ut_types.h"
#include "pt_Types.h"
#include "ut_assert.h"

#include "fl_Layout.h"
#include "pd_Document.h"


fl_Layout::fl_Layout(PTStruxType type, PL_StruxDocHandle sdh)
{
	UT_ASSERT(sdh);

	m_type = type;
	m_sdh = sdh;
	m_apIndex = 0;

	m_pDoc = NULL;		// set by child
}

fl_Layout::~fl_Layout()
{
}

PTStruxType	fl_Layout::getType(void) const
{
	return m_type;
}

void fl_Layout::setAttrPropIndex(PT_AttrPropIndex apIndex)
{
	m_apIndex = apIndex;
}

UT_Bool fl_Layout::getAttrProp(const PP_AttrProp ** ppAP) const
{
	return m_pDoc->getAttrProp(m_apIndex,ppAP);
}

UT_Bool fl_Layout::getSpanAttrProp(UT_uint32 offset, const PP_AttrProp ** ppAP) const
{
	return m_pDoc->getSpanAttrProp(m_sdh,offset,ppAP);
}

