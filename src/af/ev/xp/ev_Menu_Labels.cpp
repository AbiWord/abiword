/* AbiSource Program Utilities
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
 


#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ev_Menu_Labels.h"

#define FREEP(p)	do { if (p) free(p); } while (0)
#define DELETEP(p)	do { if (p) delete(p); } while (0)

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

const char * EV_Menu_Label::getMenuLabel(void) const
{
	return m_szMenuLabel;
}

const char * EV_Menu_Label::getMenuStatusMessage(void) const
{
	return m_szStatusMsg;
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
	FREEP(m_szLanguage);

	if (!m_labelTable)
		return;

	UT_uint32 k, kLimit;
	for (k=0, kLimit=(m_last-m_first+1); (k<kLimit); k++)
		DELETEP(m_labelTable[k]);
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
	DELETEP(m_labelTable[index]);
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


