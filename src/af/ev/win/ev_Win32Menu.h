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
#include "ev_Menu.h"
#include "ap_Menu_Id.h"
class FV_View;
class AP_Win32App;
class AP_Win32Frame;

/*****************************************************************/

class EV_Win32Menu : public EV_Menu
{
public:
	EV_Win32Menu(AP_Win32App * pWin32App, AP_Win32Frame * pWin32Frame,
				 const char * szMenuLayoutName,
				 const char * szMenuLabelSetName);
	~EV_Win32Menu(void);

	UT_Bool				synthesize(void);
	UT_Bool				onCommand(FV_View * pView, HWND hWnd, WPARAM wParam);
	UT_Bool				onInitMenu(FV_View * pView, HWND hWnd, HMENU hMenuBar);

	inline AP_Menu_Id	MenuIdFromWmCommand(UINT cmd)		{ return (AP_Menu_Id)(cmd - WM_USER); };
	inline UINT			WmCommandFromMenuId(AP_Menu_Id id)	{ return (id + WM_USER); };

protected:
	AP_Win32App *		m_pWin32App;
	AP_Win32Frame *		m_pWin32Frame;
};

#endif /* EV_WIN32MENU_H */
