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
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_Menu_Labels.h"
#include "ut_vector.h"

/*****************************************************************/

EV_Menu_Label::EV_Menu_Label(XAP_Menu_Id id,
							 const char * szMenuLabel,
							 const char * szStatusMsg)
{
	m_id = id;
	UT_cloneString(m_szMenuLabel,szMenuLabel);
	UT_cloneString(m_szStatusMsg,szStatusMsg);
}

EV_Menu_Label::~EV_Menu_Label()
{
	FREEP(m_szMenuLabel);
	FREEP(m_szStatusMsg);
}

XAP_Menu_Id EV_Menu_Label::getMenuId() const
{
	return m_id;
}

const char * EV_Menu_Label::getMenuLabel() const
{
	return m_szMenuLabel;
}

const char * EV_Menu_Label::getMenuStatusMessage() const
{
	return m_szStatusMsg;
}

/*****************************************************************/

EV_Menu_LabelSet::EV_Menu_LabelSet(const char * szLanguage,
								   XAP_Menu_Id first, XAP_Menu_Id last)
	: m_labelTable(last - first + 1),
	  m_first(first),
	  m_last(last)
{
	// TODO it's bad to call malloc/calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	UT_cloneString(m_szLanguage,szLanguage);
	int size = m_labelTable.getItemCount();

	for (int i = 0; i < size; i++)
		m_labelTable.addItem(0);
}

EV_Menu_LabelSet::~EV_Menu_LabelSet()
{
	UT_VECTOR_SPARSEPURGEALL(EV_Menu_Label *, m_labelTable);
	FREEP(m_szLanguage);
}

bool EV_Menu_LabelSet::setLabel(XAP_Menu_Id id,
								const char * szMenuLabel,
								const char * szStatusMsg)
{
	void *tmp;
	if ((id < m_first) || (id > m_last))
		return false;

	UT_uint32 index = (id - m_first);
	EV_Menu_Label *label = new EV_Menu_Label(id, szMenuLabel, szStatusMsg);
	UT_sint32 error = m_labelTable.setNthItem(index, label, &tmp);
	EV_Menu_Label * pTmpLbl = static_cast<EV_Menu_Label *> (tmp);
	DELETEP(pTmpLbl);
	return (error == 0);
}

#ifdef __MRC__
EV_Menu_Label * EV_Menu_LabelSet::getLabel(XAP_Menu_Id id)
#else
EV_Menu_Label * EV_Menu_LabelSet::getLabel(XAP_Menu_Id id) const
#endif
{
	if ((id < m_first) || (id > m_last))
		return NULL;

	UT_uint32 index = (id - m_first);
	
	EV_Menu_Label * pLabel = static_cast<EV_Menu_Label *> (m_labelTable.getNthItem(index));

	if (!pLabel)
	{
		UT_DEBUGMSG(("WARNING: %s translation for menu id [%d] not found.\n", m_szLanguage, id));
		// NOTE: only translators should see the following strings
		// NOTE: do *not* translate them
		pLabel = new EV_Menu_Label(id, "TODO", "untranslated menu item");
//		m_labelTable.setNthItem [index] = pLabel;
	}

	UT_ASSERT(pLabel && (pLabel->getMenuId() == id));
	return pLabel;
}

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

bool EV_Menu_LabelSet::addLabel(EV_Menu_Label *pLabel)
{
	UT_ASSERT(pLabel);
	XAP_Menu_Id id = pLabel->getMenuId();
	m_last = max(id, m_last);
	m_first = min(id, m_first);
	return (m_labelTable.addItem(pLabel) == 0);
}

const char * EV_Menu_LabelSet::getLanguage() const
{
	return m_szLanguage;
}

void EV_Menu_LabelSet::setLanguage(const char *szLanguage)
{
	FREEP(m_szLanguage);
	UT_cloneString(m_szLanguage, szLanguage);
}

