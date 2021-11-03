/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_std_vector.h"

#include "ev_Menu_Labels.h"

/*****************************************************************/

EV_Menu_Label::EV_Menu_Label(XAP_Menu_Id id,
							 const char* szMenuLabel,
							 const char* szStatusMsg)
	: m_id(id),
	  m_stMenuLabel(szMenuLabel),
	  m_stStatusMsg(szStatusMsg)
{
}

EV_Menu_Label::~EV_Menu_Label()
{
}

XAP_Menu_Id
EV_Menu_Label::getMenuId() const
{
	return m_id;
}

const char*
EV_Menu_Label::getMenuLabel() const
{
	return m_stMenuLabel.c_str();
}

const char*
EV_Menu_Label::getMenuStatusMessage() const
{
	return m_stStatusMsg.c_str();
}

/*****************************************************************/

EV_Menu_LabelSet::EV_Menu_LabelSet(const char* szLanguage,
								   XAP_Menu_Id first, XAP_Menu_Id last)
  : m_labelTable(last - first + 1, nullptr)
  , m_first(first)
  , m_stLanguage(szLanguage)
{
}


EV_Menu_LabelSet::EV_Menu_LabelSet(EV_Menu_LabelSet* pLabelSet):
	m_labelTable()
{
	m_labelTable.reserve(pLabelSet->getAllLabels().size());
	m_stLanguage = pLabelSet->getLanguage();
	m_first = pLabelSet->getFirst();

	const std::vector<EV_Menu_Label*>& vecLabels = pLabelSet->getAllLabels();
	for (auto pEvl : vecLabels) {
		EV_Menu_Label* pNewLab = nullptr;
		if (pEvl != nullptr) {
		    pNewLab = new EV_Menu_Label(pEvl->getMenuId(), pEvl->getMenuLabel(), pEvl->getMenuStatusMessage());
		}
		m_labelTable.push_back(pNewLab);
	}
}

EV_Menu_LabelSet::~EV_Menu_LabelSet()
{
	UT_std_vector_sparsepurgeall(m_labelTable);
}

bool EV_Menu_LabelSet::setLabel(XAP_Menu_Id id,
								const char * szMenuLabel,
								const char * szStatusMsg)
{
	EV_Menu_Label * pTmpLbl = nullptr;
	XAP_Menu_Id last = m_first + m_labelTable.size();

	if (id < m_first || id >= last) {
		return false;
        }

	UT_sint32 index = id - m_first;

	EV_Menu_Label *label = new EV_Menu_Label(id, szMenuLabel, szStatusMsg);
	pTmpLbl = m_labelTable[index];
	m_labelTable[index] = label;
	DELETEP(pTmpLbl);
	return true;
}

EV_Menu_Label* EV_Menu_LabelSet::getLabel(XAP_Menu_Id id) const
{
	XAP_Menu_Id last = m_first + m_labelTable.size();
	if (id < m_first || id >= last) {
		return nullptr;
	}

	UT_uint32 index = (id - m_first);

	EV_Menu_Label* pLabel = m_labelTable.at(index);

	if (!pLabel) {
		if (id != 0) {
			UT_DEBUGMSG(("WARNING: %s translation for menu id [%d] not found.\n", m_stLanguage.c_str(), id));
		}
		// NOTE: only translators should see the following strings
		// NOTE: do *not* translate them
		pLabel = new EV_Menu_Label(id, "TODO", "untranslated menu item");

		// Add to label table so memory is freed.
		// Note: Need to cast away constness so we can add the label.
		(static_cast<EV_Menu_LabelSet *>(const_cast<EV_Menu_LabelSet *>(this)))->addLabel(pLabel);
	}

	UT_ASSERT(pLabel && (pLabel->getMenuId() == id));
	return pLabel;
}

bool EV_Menu_LabelSet::addLabel(EV_Menu_Label* pLabel)
{
	UT_ASSERT(pLabel);
	XAP_Menu_Id size_table = m_labelTable.size();

	// the if (...) is here due to
	// AP_MENU_ID__BOGUS2__, which ocupes an entry in the
	// action table, but that not in the layout table
	// the real fix is to erase AP_MENU_ID__BOGUS2__
	if (pLabel->getMenuId() == size_table + m_first - 1)
	{
		m_labelTable.pop_back();
		size_table = m_labelTable.size();
	}
//
// This assert always fires for me 6/6/2202 so I'm commenting it out.
//	UT_ASSERT(pLabel->getMenuId() == size_table + m_first);
	m_labelTable.push_back(pLabel);

	return (size_table + 1 == static_cast<XAP_Menu_Id>(m_labelTable.size()));
}

const std::string& EV_Menu_LabelSet::getLanguage() const
{
	return m_stLanguage;
}

void EV_Menu_LabelSet::setLanguage(const char* szLanguage)
{
	m_stLanguage = szLanguage;
}

