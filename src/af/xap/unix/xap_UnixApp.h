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


#ifndef XAP_UNIXAPP_H
#define XAP_UNIXAPP_H

#include "xap_App.h"
#include "xap_UnixDialogFactory.h"
#include "xap_Unix_TB_CFactory.h"
#include "xap_UnixFontManager.h"

#include <gdk/gdk.h>

class AP_Args;
class AP_UnixToolbar_Icons;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_UnixApp : public XAP_App
{
public:
	XAP_UnixApp(AP_Args * pArgs, const char * szAppName);
	virtual ~XAP_UnixApp(void);

	virtual UT_Bool					initialize(void);
	virtual XAP_Frame * 			newFrame(void) = 0;
	virtual void					reallyExit(void);

	virtual AP_DialogFactory *				getDialogFactory(void);
	virtual AP_Toolbar_ControlFactory *		getControlFactory(void);
	virtual XAP_Prefs *		getPrefs(void) const = 0;
	virtual UT_Bool			getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const = 0;
	XAP_UnixFontManager *					getFontManager(void);

protected:
	AP_UnixToolbar_Icons *			m_pUnixToolbarIcons;
	AP_UnixDialogFactory			m_dialogFactory;
	AP_UnixToolbar_ControlFactory	m_controlFactory;
	XAP_UnixFontManager *			m_fontManager;
	
	/* TODO put anything we need here.  for example, our
	** TODO connection to the XServer.
	*/
};

#endif /* XAP_UNIXAPP_H */
