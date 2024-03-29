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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#pragma once

#include <vector>
#include <string>

#include "ut_string_class.h"

#include "xap_Types.h"

/*****************************************************************
******************************************************************
** This file defines a framework for the string labels which
** are used with menus.  This binding allows us to localize
** menus in a platform-independent manner.
**
** We create one EV_Menu_Label per menu-item (such as "File|Open")
** per language per application (not per window or actual menu).
**
** We create one EV_Menu_LabelSet per language per application.
**
******************************************************************
*****************************************************************/

// TODO decide if we should make all labels Unicode

class ABI_EXPORT EV_Menu_Label
{
	friend class XAP_Menu_Factory;
public:
	EV_Menu_Label(XAP_Menu_Id id,
				  const char* szMenuLabel,		/* label on the actual menu itself */
				  const char* szStatusMsg);		/* status bar message */
	~EV_Menu_Label();

	XAP_Menu_Id						getMenuId() const;
	const char*						getMenuLabel() const;
	const char*						getMenuStatusMessage() const;

private:
	XAP_Menu_Id						m_id;
	UT_String						m_stMenuLabel;
	UT_String						m_stStatusMsg;
};

/*****************************************************************/

class ABI_EXPORT EV_Menu_LabelSet					/* a glorified array with bounds checking */
{
	friend class XAP_Menu_Factory;
public:
	EV_Menu_LabelSet(const char * szLanguage,
					 XAP_Menu_Id first, XAP_Menu_Id last);
	EV_Menu_LabelSet(EV_Menu_LabelSet * pLabelSet);
	~EV_Menu_LabelSet();

	bool				setLabel(XAP_Menu_Id id,
								 const char * szMenuLabel,
								 const char * szStatusMsg);
	bool				addLabel(EV_Menu_Label *pLabel);
	EV_Menu_Label *		getLabel(XAP_Menu_Id id) const;
	XAP_Menu_Id         getFirst(void) const { return m_first;}
	const std::string& getLanguage() const;
	void				setLanguage(const char *szLanguage);

	const std::vector<EV_Menu_Label*>& getAllLabels() const
        { return m_labelTable; }

private:
        std::vector<EV_Menu_Label*> m_labelTable;
	XAP_Menu_Id			m_first;
	std::string m_stLanguage;	/* for the convenience of the app only	*/
};
