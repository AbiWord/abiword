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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32TOOLBAR_STYLECOMBO_H
#define AP_WIN32TOOLBAR_STYLECOMBO_H

#include <windows.h>
#include "xap_Types.h"
#include "ev_Toolbar_Control.h"
#include "pd_Document.h"
#include "xap_Frame.h"
class EV_Toolbar;

/*****************************************************************/

class ABI_EXPORT AP_Win32Toolbar_StyleCombo : public EV_Toolbar_Control
{
public:
	AP_Win32Toolbar_StyleCombo(EV_Toolbar * pToolbar, XAP_Toolbar_Id id);
	virtual ~AP_Win32Toolbar_StyleCombo(void);

	virtual bool		populate(void);
	bool				repopulate(void);

	virtual UT_uint32   getDroppedWidth() const { return m_nDroppedWidth; }

	static EV_Toolbar_Control *		static_constructor(EV_Toolbar *, XAP_Toolbar_Id id);

protected:
	UT_uint32		m_nDroppedWidth;	// width of the dropped-down list
    PD_Document*	m_pDocument;
	XAP_Frame*	m_pFrame;
};

#endif /* AP_WIN32TOOLBAR_STYLECOMBO_H */
