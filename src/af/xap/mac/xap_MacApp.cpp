/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Args.h"
#include "xap_MacApp.h"
#include "xap_MacClipboard.h"
#include "xap_MacFrame.h"
#include "ap_MacFrame.h"
#include "xap_MacTlbr_Icons.h"
#include "xap_MacTlbr_ControlFactory.h"
#include "sp_spell.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

XAP_MacApp::XAP_MacApp(AP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	m_pMacToolbarIcons = 0;
}

XAP_MacApp::~XAP_MacApp(void)
{
	SpellCheckCleanup();

	DELETEP(m_pMacToolbarIcons);
	DELETEP(_pClipboard);
}

UT_Bool XAP_MacApp::initialize(void)
{
	// let our base class do it's thing.
	
	XAP_App::initialize();

	// load only one copy of the platform-specific icons.

//	m_pMacToolbarIcons = new AP_MacToolbar_Icons();
	
	// do anything else we need here...

	_pClipboard = new AP_MacClipboard();

	/*
	  TODO for now, we assume the dictionary
	  file is in the same directory as the EXE,
	  and that it is called 'american.hash'.  Later,
	  we will want to support spell checking in other
	  languages.
	*/
	char szDictionary[512];

	_getExeDir(szDictionary, 512);
	strcat(szDictionary, "american.hash");
	
	SpellCheckInit(szDictionary);
	
	return UT_TRUE;
}

XAP_Frame * XAP_MacApp::newFrame(void)
{
	AP_MacFrame * pMacFrame = new AP_MacFrame(this);

	if (pMacFrame)
		pMacFrame->initialize();

	return pMacFrame;
}

void XAP_MacApp::reallyExit(void)
{
	ExitToShell();
}

AP_DialogFactory * XAP_MacApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

AP_Toolbar_ControlFactory * XAP_MacApp::getControlFactory(void)
{
	return &m_controlFactory;
}

UT_uint32 XAP_MacApp::_getExeDir(char* pDirBuf, UT_uint32 iBufLen)
{
	return 0;
}

