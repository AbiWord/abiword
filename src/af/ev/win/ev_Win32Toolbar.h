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

#ifndef EV_WIN32TOOLBAR_H
#define EV_WIN32TOOLBAR_H

#include <windows.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ap_Menu_Id.h"
#include "ap_Toolbar_Id.h"
#include "ev_Toolbar.h"
#include "fv_Listener.h"

class AP_Win32App;
class AP_Win32Frame;
class AP_Win32Toolbar_Icons;
class EV_Win32Toolbar_ViewListener;


class EV_Win32Toolbar : public EV_Toolbar
{
public:
	EV_Win32Toolbar(AP_Win32App * pWin32App, AP_Win32Frame * pWin32Frame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	~EV_Win32Toolbar(void);

	UT_Bool toolbarEvent(AP_Toolbar_Id id);
	UT_Bool synthesize(void);
	UT_Bool bindListenerToView(FV_View * pView);
	UT_Bool refreshToolbar(FV_View * pView, FV_ChangeMask mask);
	UT_Bool getToolTip(LPARAM lParam);

	HWND getWindow(void) const;

	/*
		Note that the namespaces for toolbar and menu command ids 
		do *not* overlap.  
	*/
	inline AP_Toolbar_Id	ItemIdFromWmCommand(UINT cmd)			{ return (AP_Toolbar_Id)(cmd - WM_USER - AP_MENU_ID__BOGUS2__); };
	inline UINT				WmCommandFromItemId(AP_Toolbar_Id id)	{ return (id + WM_USER + AP_MENU_ID__BOGUS2__); };

protected:
	void							_releaseListener(void);

	AP_Win32App *					m_pWin32App;
	AP_Win32Frame *					m_pWin32Frame;
	EV_Win32Toolbar_ViewListener *	m_pViewListener;
	FV_ListenerId					m_lid;	/* view listener id */

	HWND							m_hwnd;
	AP_Win32Toolbar_Icons *			m_pWin32ToolbarIcons;
	UT_Vector						m_vecToolbarWidgets;
};

#endif /* EV_WIN32TOOLBAR_H */
