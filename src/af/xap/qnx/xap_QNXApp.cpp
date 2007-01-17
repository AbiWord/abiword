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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <Pt.h>

#include "ut_debugmsg.h"
#include "ut_uuid.h"

#include "ut_path.h"
#include "ut_string.h"
#include "xap_Args.h"
#include "xap_QNXApp.h"
#include "xap_FakeClipboard.h"
#include "gr_QNXImage.h"
#include "xap_QNXToolbar_Icons.h"
#include "xap_QNX_TB_CFactory.h"
#include "xap_Prefs.h"
#include "gr_QNXGraphics.h"

/*****************************************************************/

XAP_QNXApp::XAP_QNXApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	m_pQNXToolbarIcons = 0;

	_setAbiSuiteLibDir();

	memset(&m_geometry,0,sizeof(m_geometry));

	// create an instance of UT_UUIDGenerator or appropriate derrived class
	_setUUIDGenerator(new UT_UUIDGenerator());

	// register graphics allocator
	GR_GraphicsFactory * pGF = getGraphicsFactory();
	UT_ASSERT( pGF );

	if(pGF)
	{
		bool bSuccess = pGF->registerClass(GR_QNXGraphics::graphicsAllocator,
										   GR_QNXGraphics::graphicsDescriptor,
										   GR_QNXGraphics::s_getClassId());

		// we are in deep trouble if this did not succeed
		UT_ASSERT( bSuccess );
		pGF->registerAsDefault(GR_QNXGraphics::s_getClassId(), true);
		pGF->registerAsDefault(GR_QNXGraphics::s_getClassId(), false);
	}
}

XAP_QNXApp::~XAP_QNXApp(void)
{
	DELETEP(m_pQNXToolbarIcons);
}

bool XAP_QNXApp::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	XAP_App::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);

	/*******************************/
  
	// load only one copy of the platform-specific icons.
	
	m_pQNXToolbarIcons = new AP_QNXToolbar_Icons();
	
	// do any thing we need here...

	return true;
}

void XAP_QNXApp::reallyExit(void)
{
	//There must be a nicer way to drop out of the event loop
	PtQuitMainLoop();
}

UT_sint32 XAP_QNXApp::makeDirectory(const char *path, UT_sint32 mode) const
{
        return mkdir ( path, mode );
}

XAP_DialogFactory * XAP_QNXApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_QNXApp::getControlFactory(void)
{
	return &m_controlFactory;
}

void * XAP_QNXApp::getFontManager(void)
{
	return NULL;
}

const char * XAP_QNXApp::getUserPrivateDirectory(void)
{
	/* return a pointer to a static buffer */
	char * szAbiDir = ".AbiSuite";
	static char buf[PATH_MAX];
	memset(buf,0,sizeof(buf));
	
	char * szHome = getenv("HOME");
	if (!szHome || !*szHome)
		szHome = "./";
	
	if (strlen(szHome)+strlen(szAbiDir)+2 >= PATH_MAX)
		return NULL;
	
	strcpy(buf,szHome);
	if (buf[strlen(buf)-1] != '/')
		strcat(buf,"/");
	strcat(buf,szAbiDir);
	return buf;
}

bool XAP_QNXApp::_loadFonts(void)
{
	//Not needed in QNX
	return true;
}

void XAP_QNXApp::setWinGeometry(int x,int y,UT_uint32 width,UT_uint32 height, UT_uint32 flags)
{
	// TODO : do some range checking?
 	m_geometry.x = x;
 	m_geometry.y = y;
 	m_geometry.width = width;
 	m_geometry.height = height;
 	m_geometry.flags = flags;
}

void XAP_QNXApp::getWinGeometry(int *x,int *y,UT_uint32 *width,UT_uint32 *height,UT_uint32 *flags)
{
	UT_ASSERT(x && y && width && height);
	*x = m_geometry.x;
	*y = m_geometry.y;
	*width = m_geometry.width;
	*height = m_geometry.height;
	*flags = m_geometry.flags;
}

void XAP_QNXApp::_setAbiSuiteLibDir()
{
	char buf[PATH_MAX];
//	char buf2[PATH_MAX]; // not used?

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 1;	// QNX puts the program name in argv[0], so [1] is the first argument
	int k;
	
	for (k=nFirstArg; k<kLimit; k++)
		if ((*m_pArgs->m_argv[k] == '-') && (g_ascii_strcasecmp(m_pArgs->m_argv[k],"-lib")==0) && (k+1 < kLimit))
		{
			strcpy(buf,m_pArgs->m_argv[k+1]);
			int len = strlen(buf);
			if (buf[len-1]=='/')		// trim trailing slash
				buf[len-1] = 0;
			XAP_App::_setAbiSuiteLibDir(buf);
			return;
		}
	
	// if not, see if ABIWORD_DATADIR was set in the environment

	const char * sz = getenv("ABIWORD_DATADIR");
	if (sz && *sz)
	{
		strcpy(buf,sz);
		char * p = buf;
		int len = strlen(p);
		if ( (p[0]=='"') && (p[len-1]=='"') )
		{
			// trim leading and trailing DQUOTES
			p[len-1]=0;
			p++;
			len -= 2;
		}
		if (p[len-1]=='/')				// trim trailing slash
			p[len-1] = 0;
		XAP_App::_setAbiSuiteLibDir(p);
		return;
	}

	// otherwise, use the hard-coded value
	XAP_App::_setAbiSuiteLibDir(getAbiSuiteHome());
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/*
void XAP_QNXApp::setTimeOfLastEvent(int eventTime)
{
	//Not needed for QNX
	//m_eventTime = eventTime;
}
*/
