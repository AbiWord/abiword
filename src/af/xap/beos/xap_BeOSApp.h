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


#ifndef XAP_BEOSAPP_H
#define XAP_BEOSAPP_H

#include <Be.h>

#include "xap_App.h"
#include "xap_BeOSDialogFactory.h"
#include "xap_BeOSToolbar_ControlFactory.h"
class XAP_Args;
class AP_BeOSToolbar_Icons;

/*****************************************************************
******************************************************************
** This file defines the beos-platform-specific class for the
** cross-platform application.  This is used to hold all of the
** platform-specific, application-specific data.  Only one of these
** is created by the application.
******************************************************************
*****************************************************************/
class XAP_BeOSApp;

class ABI_BApp:public BApplication {
	public:
		ABI_BApp(void);
		virtual void RefsReceived(BMessage *msg);
		virtual void SetXAP_App(XAP_BeOSApp *app) { m_pApp = app; };

	XAP_BeOSApp 	*m_pApp;
};

class XAP_BeOSApp : public XAP_App
{
public:
	XAP_BeOSApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_BeOSApp(void);

	virtual UT_Bool					initialize(void);
	//For handling the double click messages, containing new file info
	virtual XAP_Frame * 				newFrame(const char *path) = 0;
	virtual void					reallyExit(void);

	virtual XAP_DialogFactory *				getDialogFactory(void);
	virtual XAP_Toolbar_ControlFactory *	getControlFactory(void);
	virtual XAP_Prefs *						getPrefs(void) const = 0;
	virtual UT_Bool							getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const = 0;
	virtual const XAP_StringSet *			getStringSet(void) const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange) = 0;
	virtual UT_Bool							canPasteFromClipboard(void) = 0;
	virtual const char *					getUserPrivateDirectory(void);
	virtual void 						_setAbiSuiteLibDir(void);


	ABI_BApp				m_BApp;		

protected:
	AP_BeOSToolbar_Icons *			m_pBeOSToolbarIcons;
	AP_BeOSDialogFactory			m_dialogFactory;
	AP_BeOSToolbar_ControlFactory	m_controlFactory;

};

#endif /* XAP_BEOSAPP_H */
