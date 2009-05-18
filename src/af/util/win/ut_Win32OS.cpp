/* AbiSource Program Utilities
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
#include <winspool.h>
#include "ut_assert.h"
#include "ut_locale.h"
#include "ut_iconv.h"
#include "ut_Win32OS.h"

/*!
 Returns Windows's OSVERSIONINFO structure
 */
OSVERSIONINFO& UT_GetWinVersion(void)
{
	static bool bInitialized = false;
	static OSVERSIONINFO os;

	if (!bInitialized)
	{
		os.dwOSVersionInfoSize = sizeof(os);
		BOOL bSuccess = GetVersionEx(&os);
		UT_ASSERT(bSuccess);
		bInitialized = true;
	}

	return os;
}

/*!
 Return true if we're running on Windows Vista, false otherwise
 */
bool UT_IsWinVista(void)
{
	return (UT_GetWinVersion().dwPlatformId == VER_PLATFORM_WIN32_NT
		 && UT_GetWinVersion().dwMajorVersion >= 6);
}

/*!
 Return true if we're running on Windows NT, false otherwise
 */
bool UT_IsWinNT(void)
{
	return UT_GetWinVersion().dwPlatformId == VER_PLATFORM_WIN32_NT;
}

/*!
 Return true if we're running on Windows 2000, false otherwise
 */
bool UT_IsWin2K(void)
{
	return (UT_GetWinVersion().dwPlatformId == VER_PLATFORM_WIN32_NT
		 && UT_GetWinVersion().dwMajorVersion >= 5);
}

/*!
 Return true if we're running on Windows 95, false otherwise
 */
bool UT_IsWin95(void)
{
	return (UT_GetWinVersion().dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
		 && UT_GetWinVersion().dwMajorVersion == 4 && UT_GetWinVersion().dwMinorVersion == 0);
}

/*****************************************************************/

/*!
 This function loads and locks a dialog template resource. 

 \param hinst
 \param lpszResName Name of the resource

 Returns the address of the locked resource.
 The caller is responsible for any unlocking/releasing necessary.
 This function is used by the various tabbed dialogs to load
 the sub-dialogs.
 */
DLGTEMPLATE * WINAPI UT_LockDlgRes(HINSTANCE hinst, LPCTSTR lpszResName)
{ 
    HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(hinst, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 	
} 

/*!
    This code is based on function by Philippe Randour <philippe_randour at hotmail dot
    com> and was found at http://bdn.borland.com/article/0,1410,28000,00.html

    (It is real pain that such an elementary task should be so hard)

    The caller must g_free the returned pointer when no longer needed
*/

#ifdef UNICODE
  #define GETDEFAULTPRINTER "GetDefaultPrinterW"
#else
  #define GETDEFAULTPRINTER "GetDefaultPrinterA"
#endif

char * UT_GetDefaultPrinterName()
{
	UT_uint32 iBufferSize = 128; // will become 2x bigger immediately in the loop
	char * pPrinterName = NULL; 
	DWORD rc;
	
	do
	{
		iBufferSize *= 2;

		if(pPrinterName)
			g_free(pPrinterName);
		
		pPrinterName = (char *) UT_calloc(sizeof(char),iBufferSize);
		UT_return_val_if_fail( pPrinterName, NULL );
		
		// the method of obtaining the name is version specific ...
		OSVERSIONINFO osvi;
		DWORD iNeeded, iReturned, iBuffSize;
		LPPRINTER_INFO_5 pPrinterInfo;
		char* p;

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			// get size of the buffer needed to call enum printers
			if (!EnumPrinters(PRINTER_ENUM_DEFAULT,NULL,5,NULL,0,&iNeeded,&iReturned))
			{
				if ((rc = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
				{
					return NULL;
				}
			}

			// allocate the buffer
			if ((pPrinterInfo = (LPPRINTER_INFO_5)LocalAlloc(LPTR,iNeeded)) == NULL)
			{
				rc = GetLastError();
			}
			else
			{
				// now get the default printer
				if (!EnumPrinters(PRINTER_ENUM_DEFAULT,NULL,5,
								  (LPBYTE) pPrinterInfo,iNeeded,&iNeeded,&iReturned))
				{
					rc = GetLastError();
				}
				else
				{
					if (iReturned > 0)
					{
						// here we copy the name to our own buffer
						if ((DWORD) lstrlen(pPrinterInfo->pPrinterName) > iBufferSize-1)
						{
							rc = ERROR_INSUFFICIENT_BUFFER;
						}
						else
						{
							lstrcpy(pPrinterName,pPrinterInfo->pPrinterName);
							rc = ERROR_SUCCESS;
						}
					}
					else
					{
						*pPrinterName = '0';
						rc = ERROR_SUCCESS;
					}
				}

				LocalFree(pPrinterInfo);
			}
		}
		else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if (osvi.dwMajorVersion >= 5) /* Windows 2000 or later */
			{
				iBuffSize = iBufferSize;

				HMODULE hWinSpool = LoadLibrary("winspool.drv");
				if (!hWinSpool)
					return NULL;

				HRESULT (WINAPI * fnGetDefaultPrinter)(LPTSTR, LPDWORD) =
					(HRESULT (WINAPI * )(LPTSTR, LPDWORD)) GetProcAddress(hWinSpool, GETDEFAULTPRINTER);
				
				if (!fnGetDefaultPrinter)
				{
					FreeLibrary(hWinSpool);
					return NULL;
				}

				if (!fnGetDefaultPrinter(pPrinterName,&iBuffSize))
					rc = GetLastError();
				else
					rc = ERROR_SUCCESS;

				FreeLibrary(hWinSpool);
			}
			else /* Windows NT 4.0 or earlier */
			{
				if (GetProfileString("windows","device","",pPrinterName,iBufferSize) == iBufferSize-1)
				{
					rc = ERROR_INSUFFICIENT_BUFFER;
				}
				else
				{
					p = pPrinterName;
					while (*p != '0' && *p != ',')
						++p;
					*p = '0';

					rc = ERROR_SUCCESS;
				}
			}
		}
	}
	while (rc == ERROR_INSUFFICIENT_BUFFER);
	
	return pPrinterName;
}

/*!
    This function obtains a DC for the default printer
    The caller needs to call DeleteDC when dc is no longer needed.
*/
HDC  UT_GetDefaultPrinterDC()
{
	char * pPrinterName  = UT_GetDefaultPrinterName();

	if(!pPrinterName || !*pPrinterName)
		return NULL;

	//	HANDLE hPrinter;
	//	if(!OpenPrinter(pPrinterName, &hPrinter, NULL))
	//		return NULL;

	const char * pDriver = UT_IsWinNT() ? "WINSPOOL" : NULL;
	HDC hdc = CreateDC(pDriver, pPrinterName, NULL, NULL);
	g_free(pPrinterName);
	return hdc;
}

ATOM UT_RegisterClassEx(UINT style, WNDPROC wndproc, HINSTANCE hInstance,
						HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, HICON hIconSm,
						const char * menu, const char * name,
						bool bForceANSI)
{
	if(!bForceANSI && UT_IsWinNT())
	{
		// first, transfer the menu and class name into wchar array
		// this code assumes that these are ASCII strings; this is true both for the win32
		// system names and our own
		WCHAR buff1[100];
		WCHAR buff2[100];
		
		const char * p = name;
		UT_uint32 i = 0;
		
		while(p && *p && i < 99) //don't overflow buff1 below
		{
			buff1[i] = *p;
			++p;
			++i;
		}
		buff1[i] = 0;
		
		p = menu;
		i = 0;
		
		while(p && *p && i < 99) //don't overflow buff2 below
		{
			buff2[i] = *p;
			++p;
			++i;
		}
		buff2[i] = 0;
		
		WNDCLASSEXW  wndclass;
		memset(&wndclass, 0, sizeof(wndclass));
		wndclass.cbSize        = sizeof(wndclass);
		wndclass.style         = style;
		wndclass.lpfnWndProc   = wndproc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInstance;
		wndclass.hIcon         = hIcon;
		wndclass.hCursor       = hCursor;
		wndclass.hbrBackground = hbrBackground;
		wndclass.lpszMenuName  = buff2;
		wndclass.lpszClassName = buff1;
		wndclass.hIconSm       = hIconSm;
		
		return RegisterClassExW(&wndclass);
	}
	else
	{
		WNDCLASSEXA  wndclass;
		memset(&wndclass, 0, sizeof(wndclass));
		wndclass.cbSize        = sizeof(wndclass);
		wndclass.style         = style;
		wndclass.lpfnWndProc   = wndproc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = hInstance;
		wndclass.hIcon         = hIcon;
		wndclass.hCursor       = hCursor;
		wndclass.hbrBackground = hbrBackground;
		wndclass.lpszMenuName  = menu;
		wndclass.lpszClassName = name;
		wndclass.hIconSm       = hIconSm;
		
		return RegisterClassExA(&wndclass);
	}
}

HWND UT_CreateWindowEx(DWORD dwExStyle, const char * pszClassName, const char * pszWindowName, DWORD dwStyle,
					   int x, int y, int nWidth, int nHeight,
					   HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam,
					   bool bForceANSI)
{
	if(!bForceANSI && UT_IsWinNT())
	{
		WCHAR buff1[100];

		// see comments in UT_RegisterClassEx
		const char * p = pszClassName;
		UT_uint32 i = 0;
		
		while(p && *p && i < 99) //don't overflow buff1 below
		{
			buff1[i] = *p;
			++p;
			++i;
		}
		buff1[i] = 0;

		// we need to use iconv here, because the name of the window might be localised
		// (in practice this matters very little, because this only sets the initial name
		// for the frame, which we immediately change once the windows is created)
		auto_iconv aic (UT_LocaleInfo::system().getEncoding().utf8_str(), ucs2Internal());
		WCHAR * ucs2str = (WCHAR*) UT_convert_cd(pszWindowName, strlen(pszWindowName)+1, aic, NULL, NULL);
		
		HWND hwnd = CreateWindowExW(dwExStyle, buff1, ucs2str, dwStyle,
									x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		g_free(ucs2str);
		return hwnd;
	}
	else
	{
		return CreateWindowExA(dwExStyle, pszClassName, pszWindowName,
							   dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}
	
}

LRESULT UT_DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam, bool bForceANSI)
{
	if(!bForceANSI&& UT_IsWinNT())
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	else
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

BOOL UT_SetWindowText(HWND hWnd, const char * lpString, bool bForceANSI)
{
	if(!bForceANSI&& UT_IsWinNT())
	{
		auto_iconv aic("UTF-8", ucs2Internal());
		WCHAR * ucs2 = (WCHAR*)UT_convert_cd(lpString, strlen(lpString)+1, aic, NULL, NULL);
		BOOL bRet = SetWindowTextW(hWnd, ucs2);
		g_free(ucs2);
		return bRet;
	}
	else
	{
		return SetWindowTextA(hWnd, lpString);
	}
}

BOOL UT_GetMessage(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax, bool bForceANSI)
{
	if(!bForceANSI&& UT_IsWinNT())
		return GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
	else
		return GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
}

LRESULT UT_DispatchMessage(const MSG *lpmsg, bool bForceANSI)
{
	if(!bForceANSI&& UT_IsWinNT())
		return DispatchMessageW(lpmsg);
	else
		return DispatchMessageA(lpmsg);
}

