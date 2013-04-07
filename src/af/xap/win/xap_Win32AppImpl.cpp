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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#include <windows.h>
#include <winreg.h>

#include "xap_Win32AppImpl.h"
#include "ut_string_class.h"
#include "ut_path.h"
#include "xap_Frame.h"
#include "xap_App.h"
#include "xap_Win32FrameImpl.h"

#include "ut_Win32LocaleString.h"

bool XAP_Win32AppImpl::openURL(const char * szURL)
{
	// NOTE: could get finer control over browser window via DDE 
	// NOTE: may need to fallback to WinExec for old NSCP versions

	UT_String sURL = szURL;

	// If this is a file:// URL, strip off file:// and make it backslashed
	if (sURL.substr(0, 7) == "file://")
	{
		sURL = sURL.substr(7, sURL.size() - 7);

		// View as WebPage likes to throw in an extra /\ just for fun, strip it off
		if (sURL.substr(0, 2) == "/\\")
			sURL = sURL.substr(2, sURL.size() - 2);

		if (sURL.substr(0, 1) == "/")
			sURL = sURL.substr(1, sURL.size() - 1);
		
		// Convert all forwardslashes to backslashes
		for (unsigned int i=0; i<sURL.length();i++)	
			if (sURL[i]=='/')	
                sURL[i]='\\';

		// Convert from longpath to 8.3 shortpath, in case of spaces in the path
		char* longpath = NULL;
		char* shortpath = NULL;
		longpath = new char[PATH_MAX];
		shortpath = new char[PATH_MAX];
		strcpy(longpath, sURL.c_str());
		DWORD retval = GetShortPathName(longpath, shortpath, PATH_MAX);
		if((retval == 0) || (retval > PATH_MAX))
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			DELETEP(longpath);
			DELETEP(shortpath);
			return false;
		}
		sURL = shortpath;
		DELETEP(longpath);
		DELETEP(shortpath);
	}

	// Query the registry for the default browser so we can directly invoke it
	UT_String sBrowser;
	HKEY hKey;
	unsigned long lType;
	DWORD dwSize;
	LPWSTR szValue = NULL;
	UT_Win32LocaleString str,str2;
	UT_UTF8String utf8;

	if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"http\\shell\\open\\command", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if(RegQueryValueExW(hKey, NULL, NULL, &lType, NULL, &dwSize) == ERROR_SUCCESS)
		{
			szValue = new WCHAR[dwSize + 1];
			RegQueryValueExW(hKey, NULL, NULL, &lType, (LPBYTE) szValue, &dwSize);
			str.fromLocale(szValue);
			utf8=str.utf8_str();
			sBrowser = utf8.utf8_str();
			DELETEP(szValue);
		}
		RegCloseKey(hKey);
	}

	/* Now that we have sBrowser from the registry, we need to parse it out.
	 * If the first character is a double-quote, everything up to and including
	 * the next double-quote is the sBrowser command. Everything after the
	 * double-quote is appended to the parameters.
	 * If the first character is NOT a double-quote, we assume
	 * everything up to the first whitespace is the command and anything after
	 * is appended to the parameters.
	 */

	int iDelimiter;
	if (sBrowser.substr(0, 1) == "\"")
		iDelimiter = UT_String_findCh(sBrowser.substr(1, sBrowser.length()-1), '"')+2;
	else
		iDelimiter = UT_String_findCh(sBrowser.substr(0, sBrowser.length()), ' ');

	// Store params into a separate UT_String before we butcher sBrowser
	UT_String sParams = sBrowser.substr(iDelimiter+1, sBrowser.length()-iDelimiter+1);
	// Cut params off of sBrowser so all we're left with is the broweser path & executable
	sBrowser = sBrowser.substr(0, iDelimiter);

	// Check for a %1 passed in from the registry.  If we find it,
	// substitute our URL for %1.  Otherwise, just append sURL to params.
	const char *pdest = strstr(sParams.c_str(), "%1");
	if (pdest != NULL)
	{
		int i = pdest - sParams.c_str() + 1;
		sParams = sParams.substr(0, i-1) + sURL + sParams.substr(i+1, sParams.length()-i+1);
	}
	else
	{
		sParams = sParams + " " + sURL;
	}

	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, false);
	XAP_Win32FrameImpl *pFImp =  (XAP_Win32FrameImpl *) pFrame->getFrameImpl();
	UT_return_val_if_fail(pFImp, false);

	str.fromUTF8(sBrowser.c_str());
	str2.fromUTF8(sParams.c_str());

	intptr_t res = (intptr_t) ShellExecuteW(pFImp->getTopLevelWindow() /*(HWND)*/,
								 L"open", str.c_str(), str2.c_str(), NULL, SW_SHOW );

	// TODO: localized error messages
	// added more specific error messages as documented in http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes.asp
	if (res <= 32)
	{
		UT_String errMsg;
		switch (res)
		{
			case ERROR_FILE_NOT_FOUND:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: The system cannot find the file specified.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
				}
				break;

			case ERROR_PATH_NOT_FOUND:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: The system cannot find the path specified.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
				}
				break;

			case SE_ERR_ACCESSDENIED:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: Access is denied.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
				}
				break;

			default:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: \n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
				}
				break;
		} /* switch (res) */
		if (errMsg[0]) {
			str.fromUTF8(errMsg.c_str());
			MessageBoxW(pFImp->getTopLevelWindow(),str.c_str(), 
				L"Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
		}
	} /* if (res <= 32) */

	return (res>32);
}

