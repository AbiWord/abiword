/* AbiSource Application Framework
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


#ifndef AP_WIN32APP_H
#define AP_WIN32APP_H

#include <windows.h>
#include "xap_App.h"
#include "xap_Win32DialogFactory.h"
#include "xap_Win32Toolbar_ControlFactory.h"
class AP_Args;
class AP_Win32Toolbar_Icons;

/*****************************************************************
******************************************************************
** This file defines the Win32-platform-specific class for the
** cross-platform application.  This is used to hold all of the
** platform-specific, application-specific data.  Only one of these
** is created by the application.
******************************************************************
*****************************************************************/

class AP_Win32App : public AP_App
{
public:
	AP_Win32App(HINSTANCE hInstance, AP_Args * pArgs, const char * szAppName);
	virtual ~AP_Win32App(void);

	virtual UT_Bool			initialize(void);
	virtual AP_Frame * 		newFrame(void);
	virtual void			reallyExit(void);

	virtual HINSTANCE		getInstance() const;

	virtual AP_DialogFactory *				getDialogFactory(void);
	virtual AP_Toolbar_ControlFactory *		getControlFactory(void);

	static int WinMain (const char * szAppName, HINSTANCE hInstance, 
					HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);

protected:
	AP_Win32Toolbar_Icons *		m_pWin32ToolbarIcons;
	HINSTANCE					m_hInstance;
	AP_Win32DialogFactory			m_dialogFactory;
	AP_Win32Toolbar_ControlFactory	m_controlFactory;

	/* TODO put anything we need here. */
};

#endif /* AP_WIN32APP_H */
