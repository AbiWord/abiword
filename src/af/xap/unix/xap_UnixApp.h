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


#ifndef AP_UNIXAPP_H
#define AP_UNIXAPP_H

#include "ap_App.h"
#include "ap_UnixDialogFactory.h"
#include "ap_UnixToolbar_ControlFactory.h"
class AP_Args;
class AP_UnixToolbar_Icons;

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application.  This is used to hold all of the
** platform-specific, application-specific data.  Only one of these
** is created by the application.
******************************************************************
*****************************************************************/

class AP_UnixApp : public AP_App
{
public:
	AP_UnixApp(AP_Args * pArgs, const char * szAppName);
	virtual ~AP_UnixApp(void);

	virtual UT_Bool					initialize(void);
	virtual AP_Frame * 				newFrame(void);

	virtual AP_DialogFactory *				getDialogFactory(void);
	virtual AP_Toolbar_ControlFactory *		getControlFactory(void);

	static int main (const char * szAppName, int argc, char ** argv);

protected:
	AP_UnixToolbar_Icons *			m_pUnixToolbarIcons;
	AP_UnixDialogFactory			m_dialogFactory;
	AP_UnixToolbar_ControlFactory	m_controlFactory;

	/* TODO put anything we need here.  for example, our
	** TODO connection to the XServer.
	*/
};

#endif /* AP_UNIXAPP_H */
