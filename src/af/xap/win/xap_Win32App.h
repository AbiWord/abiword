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


#ifndef XAP_WIN32APP_H
#define XAP_WIN32APP_H

#include <windows.h>
#include "xap_App.h"
#include "xap_Win32DialogFactory.h"
#include "xap_Win32_TB_CFactory.h"
#include "xap_Strings.h"

class XAP_Win32Slurp;
class XAP_Args;
class AP_Win32Toolbar_Icons;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_Win32App : public XAP_App
{
public:
	XAP_Win32App(HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_Win32App(void);

	virtual UT_Bool							initialize(void);
	virtual XAP_Frame *						newFrame(void) = 0;
	virtual void							reallyExit(void);

	virtual HINSTANCE						getInstance() const;

	virtual XAP_DialogFactory *				getDialogFactory(void);
	virtual XAP_Toolbar_ControlFactory *	getControlFactory(void);
	virtual XAP_Prefs *						getPrefs(void) const = 0;
	virtual UT_Bool							getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const = 0;
	virtual const XAP_StringSet *			getStringSet(void) const = 0;
	virtual const char *					getUserPrivateDirectory(void);

protected:
	UT_uint32								_getExeDir(char* pDirBuf, UT_uint32 iBufLen);
	
	AP_Win32Toolbar_Icons *					m_pWin32ToolbarIcons;
	HINSTANCE								m_hInstance;
	AP_Win32DialogFactory					m_dialogFactory;
	AP_Win32Toolbar_ControlFactory			m_controlFactory;

	XAP_Win32Slurp *						m_pSlurp;
};

#endif /* XAP_WIN32APP_H */
