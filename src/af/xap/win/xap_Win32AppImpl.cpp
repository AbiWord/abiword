/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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

#include "xap_Win32AppImpl.h"
#include "ut_string_class.h"


bool XAP_Win32AppImpl::openURL(const char * szURL)
{
	// NOTE: could get finer control over browser window via DDE 
	// NOTE: may need to fallback to WinExec for old NSCP versions

	UT_String sURL = szURL;

	// strip file:///\ from URL, the extra forward-slash back-slash is passed in from "View as Web Page"
	if ( "file:///\\" == sURL.substr(0,9) )
	{
		sURL = sURL.substr(9, sURL.size() - 9);
	}

	// strip "file://" from URL, win32 doesn't handle them well
	if ( "file://" == sURL.substr(0, 7) )
	{
		sURL = sURL.substr(7, sURL.size() - 7);
	}
	
	/* */
	for (unsigned int i=0; i<sURL.length();i++)	
		if (sURL[i]=='\\')	sURL[i]='/';		
	
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, false);
	XAP_Win32FrameImpl *pFImp =  (XAP_Win32FrameImpl *) pFrame->getFrameImpl();
	UT_return_val_if_fail(pFImp, false);
	
	
	int res = (int) ShellExecute(pFImp->getTopLevelWindow() /*(HWND)*/,
								 "open", sURL.c_str(), NULL, NULL, SW_SHOWNORMAL);

	// TODO: localized error messages
	// added more specific error messages as documented in http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes.asp

	if (res <= 32)	// show error message if failed to launch browser to display URL
	{
		UT_String errMsg;
		switch (res)
		{
			case 2:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: The system cannot find the file specified.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(pFImp->getTopLevelWindow(), errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;

			case 3:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: The system cannot find the path specified.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(pFImp->getTopLevelWindow(), errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;

			case 5:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: Access is denied.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(pFImp->getTopLevelWindow(), errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;

			default:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: \n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(pFImp->getTopLevelWindow(), errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;
		} /* switch (res) */
	} /* if (res <= 32) */

	return (res>32);
}

