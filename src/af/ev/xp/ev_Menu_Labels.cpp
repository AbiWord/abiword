 
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

#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ev_Menu_Labels.h"

#define FREEP(p)	do { if (p) free(p); } while (0)

/*****************************************************************/

EV_Menu_Label::EV_Menu_Label(AP_Menu_Id id,
							 const char * szMenuLabel,
							 const char * szToolTip,
							 const char * szStatusMsg)
{
	m_id = id;
	UT_cloneString(m_szMenuLabel,szMenuLabel);
	UT_cloneString(m_szToolTip,szToolTip);
	UT_cloneString(m_szStatusMsg,szStatusMsg);
}

EV_Menu_Label::~EV_Menu_Label(void)
{
	FREEP(m_szMenuLabel);
	FREEP(m_szToolTip);
	FREEP(m_szStatusMsg);
}

AP_Menu_Id EV_Menu_Label::getMenuId(void) const
{
	return m_id;
}

/*****************************************************************/

EV_Menu_LabelSet::EV_Menu_LabelSet(const char * szLanguage,
								   AP_Menu_Id first, AP_Menu_Id last)
{
	// TODO tis bad to call malloc/calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	UT_cloneString(m_szLanguage,szLanguage);
	m_labelTable = (EV_Menu_Label **)calloc((last-first+1),sizeof(EV_Menu_Label *));
	m_first = first;
	m_last = last;
}

EV_Menu_LabelSet::~EV_Menu_LabelSet(void)
{
	if (!m_labelTable)
		return;

	UT_uint32 k, kLimit;
	for (k=0, kLimit=(m_last-m_first+1); (k<kLimit); k++)
		FREEP(m_labelTable[k]);
	free(m_labelTable);
}

UT_Bool EV_Menu_LabelSet::setLabel(AP_Menu_Id id,
								   const char * szMenuLabel,
								   const char * szToolTip,
								   const char * szStatusMsg)
{
	if ((id < m_first) || (id > m_last))
		return UT_FALSE;

	UT_uint32 index = (id - m_first);
	FREEP(m_labelTable[index]);
	m_labelTable[index] = new EV_Menu_Label(id,szMenuLabel,szToolTip,szStatusMsg);
	return (m_labelTable[index] != NULL);
}

EV_Menu_Label * EV_Menu_LabelSet::getLabel(AP_Menu_Id id) const
{
	if ((id < m_first) || (id > m_last))
		return NULL;

	UT_uint32 index = (id - m_first);
	
	EV_Menu_Label * pLabel = m_labelTable[index];
	UT_ASSERT(pLabel && (pLabel->getMenuId()==id));
	return pLabel;
}

const char * EV_Menu_LabelSet::getLanguage(void) const
{
	return m_szLanguage;
}


