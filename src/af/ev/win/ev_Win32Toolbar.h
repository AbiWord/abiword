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
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"

class XAP_Win32App;
class XAP_Win32Frame;
class AP_Win32Toolbar_Icons;
class EV_Win32Toolbar_ViewListener;
class EV_Toolbar_Action;

// HACK: it'd be nice to guarantee that menu and toolbar IDs don't overlap
#ifdef AP_MENU_ID__BOGUS2__
#define _ev_MENU_OFFSET		AP_MENU_ID__BOGUS2__
#else
#define _ev_MENU_OFFSET		1000
#endif

class EV_Win32Toolbar : public EV_Toolbar
{
public:
	EV_Win32Toolbar(XAP_Win32App * pWin32App, XAP_Win32Frame * pWin32Frame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	~EV_Win32Toolbar(void);

	UT_Bool toolbarEvent(AP_Toolbar_Id id,
						 UT_UCSChar * pData = 0,
						 UT_uint32 dataLength = 0);
	UT_Bool synthesize(void);
	UT_Bool bindListenerToView(AV_View * pView);
	UT_Bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);
	UT_Bool getToolTip(LPARAM lParam);

	HWND getWindow(void) const;

	/*
		Note that the namespaces for toolbar and menu command ids 
		do *not* overlap.  
	*/
	inline AP_Toolbar_Id	ItemIdFromWmCommand(UINT cmd)			{ return (AP_Toolbar_Id)(cmd - WM_USER - _ev_MENU_OFFSET); };
	inline UINT				WmCommandFromItemId(AP_Toolbar_Id id)	{ return (id + WM_USER + _ev_MENU_OFFSET); };

protected:
	void							_releaseListener(void);
	HWND							_getControlWindow(AP_Toolbar_Id id);

	UT_Bool							_refreshID(AP_Toolbar_Id id);
	UT_Bool							_refreshItem(AV_View * pView, 
												 const EV_Toolbar_Action * pAction, 
												 AP_Toolbar_Id id);

	static LRESULT CALLBACK			_ComboWndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK			_ComboEditWndProc(HWND, UINT, WPARAM, LPARAM);

	XAP_Win32App *					m_pWin32App;
	XAP_Win32Frame *				m_pWin32Frame;
	EV_Win32Toolbar_ViewListener *	m_pViewListener;
	AV_ListenerId					m_lid;	/* view listener id */

	HWND							m_hwnd;
	AP_Win32Toolbar_Icons *			m_pWin32ToolbarIcons;
	UT_Vector						m_vecToolbarWidgets;
};

#endif /* EV_WIN32TOOLBAR_H */
