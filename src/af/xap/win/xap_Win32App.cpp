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
#include "xap_Win32Slurp.h"

/*****************************************************************/

XAP_Win32App::XAP_Win32App(HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	UT_ASSERT(hInstance);

	m_hInstance = hInstance;
	m_pWin32ToolbarIcons = 0;

	_setAbiSuiteLibDir();
}

XAP_Win32App::~XAP_Win32App(void)
{
	DELETEP(m_pWin32ToolbarIcons);
	DELETEP(_pClipboard);

	m_pSlurp->disconnectSlurper();
	DELETEP(m_pSlurp);
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

	m_pSlurp = new XAP_Win32Slurp(this);
	m_pSlurp->connectSlurper();
	char bufExePathname[4096];
	GetModuleFileName(NULL,bufExePathname,NrElements(bufExePathname));

	// TODO these are Application-Specific values.  Move them out of here.
	m_pSlurp->stuffRegistry(".abw",getApplicationName(),bufExePathname,"application/abiword");
	
	return UT_TRUE;
}

void XAP_Win32App::reallyExit(void)
{
	PostQuitMessage (0);
}

XAP_DialogFactory * XAP_Win32App::getDialogFactory(void)
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_Win32App::getControlFactory(void)
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

const char * XAP_Win32App::getUserPrivateDirectory(void)
{
	/* return a pointer to a static buffer */
	
	char * szAbiDir = "AbiSuite";

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

	static char buf[PATH_MAX];
	memset(buf,0,sizeof(buf));

	DWORD len, len1, len2;

	// On NT, USERPROFILE seems to be set to the directory containing per-user
	// information.  we'll try that first.
	
	len = GetEnvironmentVariable("USERPROFILE",buf,PATH_MAX);
	if (len)
	{
		UT_DEBUGMSG(("Getting preferences directory from USERPROFILE [%s].\n",buf));
	}
	else
	{
		// If that doesn't work, look for HOMEDRIVE and HOMEPATH.  HOMEPATH
		// is mentioned in the GetWindowsDirectory() documentation at least.
		// These may be set if the SysAdmin did so in the Admin tool....
	
		len1 = GetEnvironmentVariable("HOMEDRIVE",buf,PATH_MAX);
		len2 = GetEnvironmentVariable("HOMEPATH",&buf[len1],PATH_MAX-len1);
		if (len1 && len2)
		{
			UT_DEBUGMSG(("Getting preferences directory from HOMEDRIVE and HOMEPATH [%s].\n",buf));
		}
		else
		{
			// If that doesn't work, let's just stick it in the WINDOWS directory.

			len = GetWindowsDirectory(buf,PATH_MAX);
			if (len)
			{
				UT_DEBUGMSG(("Getting preferences directory from GetWindowsDirectory() [%s].\n",buf));
			}
			else
			{
				// If that doesn't work, stick it in "C:\"...

				strcpy(buf,"C:\\");
			}
		}
	}

	if (strlen(buf)+strlen(szAbiDir)+2 >= PATH_MAX)
		return NULL;

	if (buf[strlen(buf)-1] != '\\')
		strcat(buf,"\\");
	strcat(buf,szAbiDir);
	return buf;
}

void XAP_Win32App::_setAbiSuiteLibDir(void)
{
	char buf[PATH_MAX];
	char buf2[PATH_MAX];
	const char * sz = NULL;

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 0;	// Win32 does not put the program name in argv[0], so [0] is the first argument
	int k;
	
	for (k=nFirstArg; k<kLimit; k++)
		if ((*m_pArgs->m_argv[k] == '-') && (UT_stricmp(m_pArgs->m_argv[k],"-lib")==0) && (k+1 < kLimit))
		{
			strcpy(buf,m_pArgs->m_argv[k+1]);
			int len = strlen(buf);
			if (buf[len-1]=='\\')		// trim trailing slash
				buf[len-1] = 0;
			XAP_App::_setAbiSuiteLibDir(buf);
			return;
		}
	
	// if not, see if ABISUITE_HOME was set in the environment

	if (GetEnvironmentVariable("ABISUITE_HOME",buf,sizeof(buf)) > 0)
	{
		char * p = buf;
		int len = strlen(p);
		if ( (p[0]=='"') && (p[len-1]=='"') )
		{
			// trim leading and trailing DQUOTES
			p[len-1]=0;
			p++;
			len -= 2;
		}
		if (p[len-1]=='\\')				// trim trailing slash
			p[len-1] = 0;
		XAP_App::_setAbiSuiteLibDir(p);
		return;
	}

	// [Win32 only] if not, use something relative to <exedir>
	// if we are in normal distribution format, we have:
	//
	// .../AbiSuite/AbiWord/bin/AbiWord.exe
	//                     /strings/*.strings
	//                     /help/EnUS/*.html
	//                     /samples/EnUS/*.abw
	//             /AbiShow/bin/AbiShow.exe
	//                     /strings/*.strings
	//                     /help/EnUS/*.html
	//                     /samples/EnUS/*.abw
	//             /dictionary/*.hash
	//
	// we want to set the library directory to .../AbiSuite
	// (aka "getExeDir()/../..")
	//
	// if this is a developer build in the canonical build
	// directory, we have:
	//
	// $(OUT)/$os_..._$dbg/bin/AbiWord.exe
	//                        /AbiShow.exe
	//                    /obj/*.obj
	//                    /AbiSuite/AbiWord/strings/*.strings
	//                                     /help/...
	//                                     /samples/...
	//                             /AbiShow/...
	//                             /dictionary/*.hash
	//
	// note that the bin directory is in a different place.
	// in this case, we want to set the library directory to
	// $(OUT)/$os_..._$dbg/AbiSuite
	// (aka "getExeDir()/../AbiSuite")
	
	if (_getExeDir(buf,sizeof(buf)) > 0)
	{
		int len = strlen(buf);
		if (buf[len-1]=='\\')
			buf[len-1] = 0;

		strcpy(buf2,buf);
		
		UT_Vector v;
		char * p = strtok(buf2,"\\");
		v.addItem(p);
		while ( (p=strtok(NULL,"\\")) )
			v.addItem(p);

		int n = v.getItemCount();
		if (   (n > 2)
			&& (UT_stricmp((const char *)v.getNthItem(n-1),"bin")==0)
			&& (UT_stricmp((const char *)v.getNthItem(n-3),"AbiSuite")==0))
		{
			strcat(buf,"\\..\\..");		// TODO trim the string rather than use ..'s
			XAP_App::_setAbiSuiteLibDir(buf);
			return;
		}

		if (   (n > 1)
			&& (UT_stricmp((const char *)v.getNthItem(n-1),"bin")==0))
		{
			strcat(buf,"\\..\\AbiSuite"); // TODO trim the string rather than use ..'s
			XAP_App::_setAbiSuiteLibDir(buf);
			return;
		}

		// [win32 only] if none of this works, just leave it the exe directory.

		XAP_App::_setAbiSuiteLibDir(buf);
		return;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return;
}

void XAP_Win32App::enableAllTopLevelWindows(UT_Bool b)
{
	UT_uint32 iCount = m_vecFrames.getItemCount();
	
	for (UT_uint32 ndx=0; ndx<iCount; ndx++)
	{
		XAP_Win32Frame * pFrame = (XAP_Win32Frame *) m_vecFrames.getNthItem(ndx);

		EnableWindow(pFrame->getTopLevelWindow(), b);
	}
}


















