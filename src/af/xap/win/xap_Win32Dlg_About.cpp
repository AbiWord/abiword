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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"
#include "ut_Win32LocaleString.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_Win32Dlg_About.h"

#include "gr_Win32Graphics.h"
#include "gr_Win32Image.h"
#include "gr_Win32USPGraphics.h"
#include "ut_bytebuf.h"
#include "ut_png.h"

/*****************************************************************/

extern unsigned char g_pngSidebar[];		// see ap_wp_sidebar.cpp
extern unsigned long g_pngSidebar_sizeof;	// see ap_wp_sidebar.cpp

bool XAP_Win32Dialog_About::s_bEventLoopDone;

/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_About::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_About * p = new XAP_Win32Dialog_About(pFactory,id);
	return p;
}

XAP_Win32Dialog_About::XAP_Win32Dialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_Dialog_About(pDlgFactory,id)
{
	m_pGrImageSidebar = NULL;
}

XAP_Win32Dialog_About::~XAP_Win32Dialog_About(void)
{
	DELETEP(m_pGrImageSidebar);
}

const int ABOUT_WIDTH		= 500;
const int ABOUT_HEIGHT		= 398;
const int BUTTON_WIDTH		= 64;
const int BUTTON_HEIGHT		= 24;
const int BUTTON_GAP		= 10;
const int HEADING_HEIGHT	= 36;
const int VERSION_HEIGHT	= 24;
const int COPYRIGHT_HEIGHT	= 24;
const int GPL_HEIGHT		= 180;

const int ID_BUTTON_URL		= 3000;

void XAP_Win32Dialog_About::runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
    UT_Win32LocaleString str, strbis;
	m_pFrame = pFrame;

	UT_ByteBuf * pBB = new UT_ByteBuf(g_pngSidebar_sizeof);
	pBB->ins(0,g_pngSidebar,g_pngSidebar_sizeof);

	UT_sint32 iImageWidth;
	UT_sint32 iImageHeight;
		
	UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
	
	m_pGrImageSidebar = new GR_Win32Image(NULL);
	m_pGrImageSidebar->convertFromBuffer(pBB, "image/png", iImageWidth, iImageHeight);

	DELETEP(pBB);
	const wchar_t * pClassName = L"AbiSource_About";
	
	ATOM a = UT_RegisterClassEx(CS_HREDRAW | CS_VREDRAW, (WNDPROC) s_dlgProc, pWin32App->getInstance(),
								NULL, LoadCursorW(NULL, (LPCWSTR)IDC_ARROW), GetSysColorBrush(COLOR_BTNFACE), NULL,
								NULL, pClassName);
	if (!a)
	{
		::MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	HWND hWndFrame = static_cast<XAP_Win32FrameImpl*>(m_pFrame->getFrameImpl())->getTopLevelWindow();

	RECT rcScreen;
	rcScreen.left=rcScreen.right=0;

#if (_WIN32_WINNT >= 0x0500 )
	if (GetSystemMetrics(SM_CMONITORS) > 1) {
		HMONITOR m = MonitorFromWindow(hWndFrame,MONITOR_DEFAULTTONEAREST);
		if (m) {
			MONITORINFO mi;
			mi.cbSize=sizeof(MONITORINFO);
			if (GetMonitorInfoW(m,&mi))
				rcScreen = mi.rcWork;
		}
	} else 
#endif
		SystemParametersInfoW(SPI_GETWORKAREA,0,&rcScreen,0);

	if (rcScreen.left == rcScreen.top)
		SetRect(&rcScreen,0,0,GetSystemMetrics(SM_CXFULLSCREEN),
				GetSystemMetrics(SM_CYFULLSCREEN));

	const int iScreenWidth  = rcScreen.right - rcScreen.left;
	const int iScreenHeight = rcScreen.bottom - rcScreen.top;

	BringWindowToTop(hWndFrame);
	pWin32App->enableAllTopLevelWindows(FALSE);

	wchar_t buf[1024];
	const XAP_StringSet*  pSS = XAP_App::getApp()->getStringSet();
	str.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_ABOUT_Title));
	strbis.fromASCII (XAP_App::getApp()->getApplicationName());
	swprintf(buf, str.c_str(), strbis.c_str());

	HWND hwndAbout = UT_CreateWindowEx(	0L, pClassName,
										buf,
										WS_OVERLAPPED | WS_VISIBLE,
										rcScreen.left + (iScreenWidth - ABOUT_WIDTH) / 2,
										rcScreen.top + (iScreenHeight - ABOUT_HEIGHT) /2,
										ABOUT_WIDTH,
										ABOUT_HEIGHT,
										hWndFrame,
										NULL,
										pWin32App->getInstance(),
										NULL);
	if (!hwndAbout)
	{
		UnregisterClassW(pClassName, pWin32App->getInstance());
		::MessageBeep(MB_ICONEXCLAMATION);
		return;
	}

	SetWindowLongPtrW(hwndAbout, GWLP_USERDATA, (LONG_PTR)this);

	RECT rcClient;
	GetClientRect(hwndAbout, &rcClient);
	const int iWidth  = rcClient.right;
	const int iHeight = rcClient.bottom;
	
	str.fromUTF8 (pSS->getValue(XAP_STRING_ID_DLG_OK));		
	HWND hwndOK = CreateWindowW(L"BUTTON",
							   str.c_str(),
							   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
							   iWidth - BUTTON_WIDTH - BUTTON_GAP,
							   iHeight - BUTTON_HEIGHT - BUTTON_GAP,
							   BUTTON_WIDTH,
							   BUTTON_HEIGHT,
							   hwndAbout,
							   (HMENU) IDOK,
							   pWin32App->getInstance(),
							   NULL);

	HWND hwndURL = CreateWindowW(L"BUTTON",
								L"www.abisource.com",
								WS_CHILD | WS_VISIBLE | WS_TABSTOP,
								iWidth - BUTTON_WIDTH - BUTTON_GAP - 2*BUTTON_WIDTH - BUTTON_GAP,
								iHeight - BUTTON_HEIGHT - BUTTON_GAP,
								BUTTON_WIDTH*2,
								BUTTON_HEIGHT,
								hwndAbout,
								(HMENU) ID_BUTTON_URL,
								pWin32App->getInstance(),
								NULL);

    str.fromUTF8 (XAP_App::getApp()->getApplicationName());
	HWND hwndStatic_Heading = CreateWindowW(L"STATIC",
										   str.c_str(),
										   WS_CHILD | WS_VISIBLE | SS_CENTER,
										   iImageWidth,
										   BUTTON_GAP,
										   iWidth - iImageWidth,
										   HEADING_HEIGHT,
										   hwndAbout,
										   (HMENU) 3001,
										   pWin32App->getInstance(),
										   NULL);

	const char *versiontext=pSS->getValue(XAP_STRING_ID_DLG_ABOUT_Version);
	UT_UTF8String version = UT_UTF8String_sprintf(versiontext,XAP_App::s_szBuild_Version);
	str.fromUTF8(version.utf8_str());
   
	HWND hwndStatic_Version = CreateWindowW(L"STATIC",
										   str.c_str(),
										   WS_CHILD | WS_VISIBLE | SS_CENTER,
										   iImageWidth + BUTTON_GAP,
										   BUTTON_GAP + HEADING_HEIGHT + BUTTON_GAP,
										   iWidth - 2*BUTTON_GAP - iImageWidth,
										   VERSION_HEIGHT,
										   hwndAbout,
										   (HMENU) 3002,
										   pWin32App->getInstance(),
										   NULL);

    str.fromASCII (XAP_ABOUT_COPYRIGHT);
	HWND hwndStatic_Copyright = CreateWindowW(L"STATIC",
											 str.c_str(),
											 WS_CHILD | WS_VISIBLE | SS_LEFT,
											 iImageWidth + BUTTON_GAP,
											 BUTTON_GAP + HEADING_HEIGHT + BUTTON_GAP + VERSION_HEIGHT + 1*BUTTON_GAP,
											 iWidth - 2*BUTTON_GAP - iImageWidth,
											 COPYRIGHT_HEIGHT,
											 hwndAbout,
											 (HMENU) 3003,
											 pWin32App->getInstance(),
											 NULL);

    str.fromASCII (XAP_App::getApp()->getApplicationName());
	strbis.fromASCII (XAP_ABOUT_GPL_LONG);
	swprintf(buf, strbis.c_str(), str.c_str());
	HWND hwndStatic_GPL = CreateWindowW(L"STATIC",
									   buf,
									   WS_CHILD | WS_VISIBLE | SS_LEFT,
									   iImageWidth + BUTTON_GAP,
									   BUTTON_GAP + HEADING_HEIGHT + BUTTON_GAP + VERSION_HEIGHT + 1*BUTTON_GAP + VERSION_HEIGHT + BUTTON_GAP,
									   iWidth - 2*BUTTON_GAP - iImageWidth,
									   GPL_HEIGHT,
									   hwndAbout,
									   (HMENU) 3004,
									   pWin32App->getInstance(),
									   NULL);

	HWND hwndStatic_USP_Version = 0;
	
	if(pWin32App->getLastFocussedFrame()->getCurrentView()->getGraphics()->getClassId()==GRID_WIN32_UNISCRIBE)
	{
		GR_Win32USPGraphics * pUSP = static_cast<GR_Win32USPGraphics*>(
												  pWin32App->getLastFocussedFrame()->getCurrentView()->getGraphics());

		str.fromASCII (pUSP->getUSPVersion());
		hwndStatic_USP_Version = CreateWindowW(L"STATIC",
												   str.c_str(),
												   WS_CHILD | WS_VISIBLE | SS_LEFT,
												   BUTTON_GAP/2,
 				   								   iHeight - 3*BUTTON_HEIGHT/4,
												   iImageWidth - BUTTON_GAP/2,
												   3*BUTTON_HEIGHT/4,
												   hwndAbout,
												   (HMENU) 3005,
												   pWin32App->getInstance(),
												   NULL);
	
		
	}
	
	LOGFONTW lf;
	memset(&lf, 0, sizeof(lf));

	wcscpy(lf.lfFaceName, L"MS Shell Dlg 2");
	
	lf.lfHeight = 13;
	lf.lfWeight = 0;
	HFONT hfontPrimary = CreateFontIndirectW(&lf);

	lf.lfHeight = 12;
	lf.lfWeight = 0;
	HFONT hfontSmall = CreateFontIndirectW(&lf);

	wcscpy(lf.lfFaceName, L"Arial");
	lf.lfHeight = 36;
	lf.lfWeight = FW_BOLD; 
	HFONT hfontHeading = CreateFontIndirectW(&lf);
	
	SendMessageW(hwndStatic_Heading, WM_SETFONT, (WPARAM) hfontHeading, 0);

	HWND rgFontReceivers[] =
		{ hwndOK, hwndURL, hwndStatic_Version, hwndStatic_Copyright };

	for (UT_uint32 iWnd = 0; iWnd < G_N_ELEMENTS(rgFontReceivers); ++iWnd)
	{
		SendMessageW(rgFontReceivers[iWnd], WM_SETFONT, (WPARAM) hfontPrimary, 0);
	}

	SendMessageW(hwndStatic_GPL, WM_SETFONT, (WPARAM) hfontSmall, 0);

	if(hwndStatic_USP_Version)
		SendMessageW(hwndStatic_USP_Version, WM_SETFONT, (WPARAM) hfontSmall, 0);
	
	// the event loop
	{
		MSG msg;

		s_bEventLoopDone = false;
	
		while (!s_bEventLoopDone)
		{
			if (GetMessageW(&msg, NULL, 0, 0))
			{
				if( hwndAbout && IsDialogMessageW( hwndAbout, &msg ) )
					continue;

				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
			{
				break;
			}
		}
	}

	DestroyWindow(hwndAbout);

	UnregisterClassW(pClassName, pWin32App->getInstance());

	pWin32App->enableAllTopLevelWindows(true);

	if (GetWindow(hWndFrame, GW_HWNDFIRST) != hWndFrame)
	{
		BringWindowToTop(hWndFrame);
	}

	if (GetFocus() != hWndFrame)
	{
		SetFocus(hWndFrame);
	}
}

BOOL CALLBACK XAP_Win32Dialog_About::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_About * pThis = (XAP_Win32Dialog_About *)GetWindowLongPtrW(hWnd,GWLP_USERDATA);

	if (!pThis)
	{
		return UT_DefWindowProc(hWnd, msg, wParam, lParam);
	}
	
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
			
		RECT	r;
		GetClientRect(hWnd, &r);
		r.right = pThis->m_pGrImageSidebar->getDisplayWidth();
		FillRect(hdc, &r, GetSysColorBrush(COLOR_BTNFACE));


		GR_Win32AllocInfo ai(hdc,hWnd);
		GR_Win32Graphics *pGR = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);
		UT_return_val_if_fail(pGR, 0);
		
		pGR->drawImage(pThis->m_pGrImageSidebar,
					   0,
					   (r.bottom - pThis->m_pGrImageSidebar->getDisplayHeight())/2);

		delete pGR;
		
		EndPaint(hWnd,&ps);
		return 0;
	}

	case WM_CHAR:
	{
		switch (wParam)
		{
		case 13:	// CR
		case 27:	// ESC
		case 32:	// SPACE
			s_bEventLoopDone = true;
			return 0;
		}
		
		break;
	}

	case WM_COMMAND:
		return pThis->_onCommand(hWnd,wParam,lParam);

	case WM_DESTROY:
		s_bEventLoopDone = true;
		return 0;
	}

	return UT_DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL XAP_Win32Dialog_About::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case IDCANCEL:
	case IDOK:							// also XAP_RID_DIALOG_ABOUT_BTN_OK
		s_bEventLoopDone = true;
		return 0;

	case ID_BUTTON_URL:
		XAP_App::getApp()->openURL("http://www.abisource.com/");
		return 0;
		
	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}
