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

#include <windows.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Args.h"
#include "xap_Win32App.h"
#include "xap_Win32Clipboard.h"
#include "xap_Win32Frame.h"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_Win32_TB_CFactory.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

XAP_Win32App::XAP_Win32App(HINSTANCE hInstance, AP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	UT_ASSERT(hInstance);

	m_hInstance = hInstance;
	m_pWin32ToolbarIcons = 0;
}

XAP_Win32App::~XAP_Win32App(void)
{
	DELETEP(m_pWin32ToolbarIcons);
	DELETEP(_pClipboard);
}

HINSTANCE XAP_Win32App::getInstance() const
{
	return m_hInstance;
}

UT_Bool XAP_Win32App::initialize(void)
{
	// let our base class do it's thing.
	
	XAP_App::initialize();

	// load only one copy of the platform-specific icons.

	m_pWin32ToolbarIcons = new AP_Win32Toolbar_Icons();
	
	// do anything else we need here...

	_pClipboard = new AP_Win32Clipboard();
	
	return UT_TRUE;
}

void XAP_Win32App::reallyExit(void)
{
	PostQuitMessage (0);
}

AP_DialogFactory * XAP_Win32App::getDialogFactory(void)
{
	return &m_dialogFactory;
}

AP_Toolbar_ControlFactory * XAP_Win32App::getControlFactory(void)
{
	return &m_controlFactory;
}

UT_uint32 XAP_Win32App::_getExeDir(char* pDirBuf, UT_uint32 iBufLen)
{
	UT_uint32 iResult = GetModuleFileName(NULL, pDirBuf, iBufLen);

	if (iResult > 0)
	{
		char* p = pDirBuf + strlen(pDirBuf);
		while (*p != '\\')
		{
			p--;
		}
		UT_ASSERT(p > pDirBuf);
		p++;
		*p = 0;
	}

	return iResult;
}

