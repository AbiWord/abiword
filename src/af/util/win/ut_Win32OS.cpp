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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
#include <windows.h>
#include <winspool.h>
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_locale.h"
#include "ut_iconv.h"
#include "ut_Win32OS.h"

/*!
 Returns Windows's OSVERSIONINFO structure
 */
OSVERSIONINFOW& UT_GetWinVersion(void)
{
	static bool bInitialized = false;
	static OSVERSIONINFOW os;

	if (!bInitialized)
	{
		os.dwOSVersionInfoSize = sizeof(os);
		UT_DebugOnly<BOOL> bSuccess = GetVersionExW(&os);
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
DLGTEMPLATE * WINAPI UT_LockDlgRes(HINSTANCE hinst, LPCWSTR lpszResName)
{ 
    HRSRC hrsrc = FindResourceW(NULL, lpszResName,  (LPWSTR)RT_DIALOG); 
    HGLOBAL hglb = LoadResource(hinst, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 	
} 

/*!
    This code is based on function by Philippe Randour <philippe_randour at hotmail dot
    com> and was found at http://bdn.borland.com/article/0,1410,28000,00.html

    (It is real pain that such an elementary task should be so hard)

    The caller must g_free the returned pointer when no longer needed
*/


#define GETDEFAULTPRINTER "GetDefaultPrinterW"


wchar_t * UT_GetDefaultPrinterName()
{
	UT_uint32 iBufferSize = 128; // will become 2x bigger immediately in the loop
	wchar_t * pPrinterName = NULL; 
	DWORD rc;
	
	do
	{
		iBufferSize *= 2;

		if(pPrinterName)
			g_free(pPrinterName);
		
		pPrinterName = (wchar_t *) UT_calloc(sizeof(wchar_t),iBufferSize);
		UT_return_val_if_fail( pPrinterName, NULL );
		
		// the method of obtaining the name is version specific ...
		OSVERSIONINFOW osvi;
		DWORD iNeeded, iReturned, iBuffSize;
		LPPRINTER_INFO_5W pPrinterInfo;
		wchar_t* p;

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
		GetVersionExW(&osvi);

		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			// get size of the buffer needed to call enum printers
			if (!EnumPrintersW(PRINTER_ENUM_DEFAULT,NULL,5,NULL,0,&iNeeded,&iReturned))
			{
				if ((rc = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
				{
					return NULL;
				}
			}

			// allocate the buffer
			if ((pPrinterInfo = (LPPRINTER_INFO_5W)LocalAlloc(LPTR,iNeeded)) == NULL)
			{
				rc = GetLastError();
			}
			else
			{
				// now get the default printer
				if (!EnumPrintersW(PRINTER_ENUM_DEFAULT,NULL,5,
								  (LPBYTE) pPrinterInfo,iNeeded,&iNeeded,&iReturned))
				{
					rc = GetLastError();
				}
				else
				{
					if (iReturned > 0)
					{
						// here we copy the name to our own buffer
						if ((DWORD) wcslen(pPrinterInfo->pPrinterName) > iBufferSize-1)
						{
							rc = ERROR_INSUFFICIENT_BUFFER;
						}
						else
						{
							wcscpy(pPrinterName,pPrinterInfo->pPrinterName);
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

				HMODULE hWinSpool = LoadLibraryW(L"winspool.drv");
				if (!hWinSpool)
					return NULL;

				HRESULT (WINAPI * fnGetDefaultPrinter)(LPWSTR, LPDWORD) =
					(HRESULT (WINAPI * )(LPWSTR, LPDWORD)) GetProcAddress(hWinSpool, GETDEFAULTPRINTER);
				
				if (!fnGetDefaultPrinter)
				{
					FreeLibrary(hWinSpool);
					return NULL;
				}

                bool i =false;
				if (!fnGetDefaultPrinter(pPrinterName,&iBuffSize))
                        i = true;
                         
                if(i)
					rc = GetLastError();
				else
					rc = ERROR_SUCCESS;

				FreeLibrary(hWinSpool);
			}
			else /* Windows NT 4.0 or earlier */
			{
				if (GetProfileStringW(L"windows",L"device",L"",pPrinterName,iBufferSize) == iBufferSize-1)
				{
					rc = ERROR_INSUFFICIENT_BUFFER;
				}
				else
				{
					p = pPrinterName;
					while (*p != '0' && *p !=L',')
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
	wchar_t * pPrinterName  = UT_GetDefaultPrinterName();

	if(!pPrinterName || !*pPrinterName)
		return NULL;

	//	HANDLE hPrinter;
	//	if(!OpenPrinter(pPrinterName, &hPrinter, NULL))
	//		return NULL;

	const wchar_t * pDriver = UT_IsWinNT() ? L"WINSPOOL" : NULL;
	HDC hdc = CreateDCW(pDriver, pPrinterName, NULL, NULL);
	g_free(pPrinterName);
	return hdc;
}



ATOM UT_RegisterClassEx(UINT style, WNDPROC wndproc, HINSTANCE hInstance,
 						HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, HICON hIconSm,
						const wchar_t * menu, const wchar_t * name)
 
{
    ATOM atom;
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
	wndclass.lpszMenuName  = menu;
	wndclass.lpszClassName = name;
	wndclass.hIconSm       = hIconSm;
	
	atom = RegisterClassExW (&wndclass);	
	UT_ASSERT(atom);
	return atom;
}


LRESULT UT_DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam)
{
	return DefWindowProcW(hWnd, Msg, wParam, lParam);	
}


BOOL UT_SetWindowText(HWND hWnd, const wchar_t * lpString)
{
	return SetWindowTextW(hWnd, lpString);
}



BOOL UT_GetMessage(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax)
{
	return GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
}

LRESULT UT_DispatchMessage(const MSG *lpmsg)
{
	return DispatchMessageW(lpmsg);
}

HWND UT_CreateWindowEx(DWORD dwExStyle, const wchar_t * pszClassName, const wchar_t * pszWindowName, DWORD dwStyle,
 					   int x, int y, int nWidth, int nHeight,
					   HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
					   
 {	
	HWND hwnd =  CreateWindowExW(dwExStyle, pszClassName, pszWindowName,
		dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);	
			
	UT_ASSERT(hwnd);
	return hwnd;	
 }
