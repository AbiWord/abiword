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

#ifndef EV_WIN32MENU_H
#define EV_WIN32MENU_H

#include <windows.h>
#include "ut_types.h"
#include "xap_Types.h"
#include "ev_Menu.h"

class AV_View;
class XAP_Win32App;
class EV_EditEventMapper;
class XAP_Frame;
class XAP_Win32Frame;

/*****************************************************************/

class EV_Win32Menu : public EV_Menu
{
public:
	EV_Win32Menu(XAP_Win32App * pWin32App,
				 const EV_EditEventMapper * pEEM,
				 const char * szMenuLayoutName,
				 const char * szMenuLabelSetName);
	~EV_Win32Menu(void);

	bool				synthesizeMenu(XAP_Frame * pFrame, HMENU menuRoot);
	bool				onCommand(AV_View * pView, HWND hWnd, WPARAM wParam);
	bool				onInitMenu(XAP_Frame * pFrame, AV_View * pView, HWND hWnd, HMENU hMenuBar);
	bool				onMenuSelect(XAP_Win32Frame * pFrame, AV_View * pView,
									 HWND hWnd, HMENU hMenu, WPARAM wParam);

	inline HMENU		getMenuHandle(void) const			{ return m_myMenu; };
	inline XAP_Menu_Id	MenuIdFromWmCommand(UINT cmd)		{ return (XAP_Menu_Id)(cmd - WM_USER); };
	inline UINT			WmCommandFromMenuId(XAP_Menu_Id id)	{ return (id + WM_USER); };

	virtual bool		_doAddMenuItem(UT_uint32 id) { UT_ASSERT(UT_TODO); return false;/* TODO */ }

protected:
	XAP_Win32App *				m_pWin32App;
	const EV_EditEventMapper *	m_pEEM;

	HMENU						m_myMenu;
};

/*****************************************************************/

class EV_Win32MenuBar : public EV_Win32Menu
{
public:
	EV_Win32MenuBar(XAP_Win32App * pWin32App,
					const EV_EditEventMapper * pEEM,
					const char * szMenuLayoutName,
					const char * szMenuLabelSetName);
	~EV_Win32MenuBar(void);

	bool				synthesizeMenuBar(XAP_Frame * pFrame);
};

/*****************************************************************/

class EV_Win32MenuPopup : public EV_Win32Menu
{
public:
	EV_Win32MenuPopup(XAP_Win32App * pWin32App,
					  const char * szMenuLayoutName,
					  const char * szMenuLabelSetName);
	~EV_Win32MenuPopup(void);

	bool				synthesizeMenuPopup(XAP_Frame * pFrame);
};

#endif /* EV_WIN32MENU_H */
