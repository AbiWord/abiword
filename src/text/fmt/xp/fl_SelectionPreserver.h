/* AbiWord
 * Copyright (c) 2003 Dom Lachowicz
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

#ifndef FL_SELECTION_PRESERVER_H
#define FL_SELECTION_PRESERVER_H

#include "pt_Types.h"

class FV_View;

class ABI_EXPORT FL_SelectionPreserver
{
public:

	explicit FL_SelectionPreserver (FV_View * pView);
	~FL_SelectionPreserver ();

	bool cmdCharInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce = false);
private:
	FL_SelectionPreserver ();
	FL_SelectionPreserver (const FL_SelectionPreserver & other);
	FL_SelectionPreserver& operator=(const FL_SelectionPreserver & other);

	FV_View * m_pView;
	bool m_bHadSelection;
	PD_DocumentRange m_docRange;
};

#endif /* FL_SELECTION_PRESERVER_H */
