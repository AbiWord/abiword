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
#include <commctrl.h>
#include <stdio.h>
#include <limits.h>
#include "zmouse.h"

#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_ViewListener.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"
#include "ev_Win32Keyboard.h"
#include "ev_Win32Mouse.h"
#include "ev_Win32Menu.h"
#include "ev_Win32Toolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "ie_impGraphic.h"
#include "fv_View.h"

#ifdef _MSC_VER
#pragma warning(disable: 4355)	// 'this' used in base member initializer list
#endif

// TODO Fix the following header file. It seems to be incomplete
// TODO #include <ap_EditMethods.h>
// TODO In the mean time, define the needed function by hand
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

/*****************************************************************/

bool XAP_Win32Frame::RegisterClass(XAP_Win32App* app)
{
	WNDCLASSEX wndclass;

	// register class for the frame window
	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= CS_DBLCLKS;
	wndclass.lpfnWndProc	= XAP_Win32Frame::_FrameWndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= app->getInstance();
	wndclass.hIcon			= app->getIcon();
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wndclass.lpszMenuName	= NULL;
	wndclass.lpszClassName	= app->getApplicationName();
	wndclass.hIconSm		= app->getSmallIcon();

	ATOM a = RegisterClassEx(&wndclass);
	UT_ASSERT(a);

	return true;
}

/*****************************************************************/

XAP_Win32Frame::XAP_Win32Frame(XAP_Win32App * app)
:	XAP_Frame(app),
	m_pWin32App(app),
	m_pWin32Menu(0),
	m_pWin32Popup(0),
	m_iBarHeight(0),
	m_iStatusBarHeight(0),
	m_hwndFrame(0),
	m_hwndRebar(0),
	m_hwndContainer(0),
	m_hwndStatusBar(0),
	m_dialogFactory(this, app),
	m_mouseWheelMessage(0),
	m_iSizeWidth(0),
	m_iSizeHeight(0)
{
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_Win32Frame::XAP_Win32Frame(XAP_Win32Frame * f)
:	XAP_Frame(f),
	m_pWin32App(f->m_pWin32App),
	m_pWin32Menu(0),
	m_pWin32Popup(0),
	m_iBarHeight(0),
	m_iStatusBarHeight(0),
	m_hwndFrame(0),
	m_hwndRebar(0),
	m_hwndContainer(0),
	m_hwndStatusBar(0),
	m_dialogFactory(this, f->m_pWin32App),
	m_mouseWheelMessage(0),
	m_iSizeWidth(0),
	m_iSizeHeight(0)
{
}

XAP_Win32Frame::~XAP_Win32Frame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pWin32Menu);
	DELETEP(m_pWin32Popup);
}

bool XAP_Win32Frame::initialize(const char* szKeyBindingsKey,
								const char* szKeyBindingsDefaultValue,
								const char* szMenuLayoutKey,
								const char* szMenuLayoutDefaultValue,
								const char* szMenuLabelSetKey,
								const char* szMenuLabelSetDefaultValue,
								const char* szToolbarLayoutsKey,
								const char* szToolbarLayoutsDefaultValue,
								const char* szToolbarLabelSetKey,
								const char* szToolbarLabelSetDefaultValue)
{
	bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize(szKeyBindingsKey,
									szKeyBindingsDefaultValue,
									szMenuLayoutKey,
									szMenuLayoutDefaultValue,
									szMenuLabelSetKey,
									szMenuLabelSetDefaultValue,
									szToolbarLayoutsKey,
									szToolbarLayoutsDefaultValue,
									szToolbarLabelSetKey,
									szToolbarLabelSetDefaultValue);
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_Win32Keyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_Win32Mouse(pEEM);
	UT_ASSERT(m_pMouse);

	// TODO: Jeff, I'm currently showing in WinMain, to honor iCmdShow.
	// should we pass that via argv, to do it here for all frames?


	return true;
}

UT_sint32 XAP_Win32Frame::setInputMode(const char * szName)
{
	UT_sint32 result = XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;
}

HWND XAP_Win32Frame::getTopLevelWindow(void) const
{
	return m_hwndFrame;
}

HWND XAP_Win32Frame::getToolbarWindow(void) const
{
	return m_hwndRebar;
}

XAP_DialogFactory * XAP_Win32Frame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_Win32Frame::_createTopLevelWindow(void)
{
	RECT r;
	UT_uint32 iHeight, iWidth;
	UT_sint32 iPosX, iPosY;
	static bool firstWindow = true;	/* position only 1st window! */

	// create a top-level window for us.
	// get the default window size from preferences or something.
	// should set size for all, but position only on 1st created
	// TODO determine where to save & restore from Window flag (since
	//      we can't use the geometry flag (its some other junk about validity of pos & size)
	//      so we can properly restore Maximized/Minimized/Normal mode windows
	// For now we can let Win32App set the flag for the 1st opened window

	// get window width & height from preferences
	UT_uint32 t_flag;		// dummy variable
	if ( !(XAP_App::getApp()->getGeometry(&iPosX,&iPosY,&iWidth,&iHeight,&t_flag)) ||
           !((iWidth > 0) && (iHeight > 0)) )
	{
		UT_DEBUGMSG(("Unable to obtain saved geometry, using window defaults!\n"));
		iWidth = CW_USEDEFAULT;
		iHeight = CW_USEDEFAULT;
		iPosX = CW_USEDEFAULT;
		iPosY = CW_USEDEFAULT;
	}
	/* let Windows(R) place the Window for all but 1st one, for stairstep effect */
	if (!firstWindow)
	{
		iPosX = CW_USEDEFAULT;
		iPosY = CW_USEDEFAULT;
	}
	else firstWindow = false;

	m_hwndFrame = CreateWindow(m_pWin32App->getApplicationName(),
							   m_pWin32App->getApplicationTitleForTitleBar(),
							   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
							   iPosX, iPosY, iWidth, iHeight,
							   NULL, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndFrame);

	// bind this frame to its window
	SetWindowLong(m_hwndFrame, GWL_USERDATA,(LONG)this);

	m_mouseWheelMessage = RegisterWindowMessage(MSH_MOUSEWHEEL);

	// synthesize a menu from the info in our
	// base class and install it into the window.
	m_pWin32Menu = new EV_Win32MenuBar(m_pWin32App,
									   getEditEventMapper(),
									   m_szMenuLayoutName,
									   m_szMenuLabelSetName);
	UT_ASSERT(m_pWin32Menu);
	bool bResult = m_pWin32Menu->synthesizeMenuBar(this);
	UT_ASSERT(bResult);

	HMENU oldMenu = GetMenu(m_hwndFrame);
	if (SetMenu(m_hwndFrame, m_pWin32Menu->getMenuHandle()))
	{
		DrawMenuBar(m_hwndFrame);
		if (oldMenu)
			DestroyMenu(oldMenu);
	}

	// create a rebar container for all the toolbars
	m_hwndRebar = CreateWindowEx(0L, REBARCLASSNAME, NULL,
								 WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
								 WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
								 RBS_VARHEIGHT | RBS_BANDBORDERS,
								 0, 0, 0, 0,
								 m_hwndFrame, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndRebar);

	// create a toolbar instance for each toolbar listed in our base class.

	_createToolbars();

	// figure out how much room is left for the child
	GetClientRect(m_hwndFrame, &r);
	iHeight = r.bottom - r.top;
	iWidth = r.right - r.left;

	m_iSizeWidth = iWidth;
	m_iSizeHeight = iHeight;
	
	// force rebar to resize itself
	// TODO for some reason, we give REBAR the height of the FRAME
	// TODO and let it decide how much it actually needs....
	if( m_hwndRebar != NULL )
	{
		MoveWindow(m_hwndRebar, 0, 0, iWidth, iHeight, TRUE);

		GetClientRect(m_hwndRebar, &r);
		m_iBarHeight = r.bottom - r.top + 6;

		UT_ASSERT(iHeight > m_iBarHeight);
		iHeight -= m_iBarHeight;
	}
	else
		m_iBarHeight = 0;

	m_hwndContainer = _createDocumentWindow(m_hwndFrame, 0, m_iBarHeight, iWidth, iHeight);

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.

	m_hwndStatusBar = _createStatusBarWindow(m_hwndFrame,0,m_iBarHeight+iHeight,iWidth);
	GetClientRect(m_hwndStatusBar,&r);
	m_iStatusBarHeight = r.bottom;

	// Allow drag-and drop
	DragAcceptFiles(m_hwndFrame, true);

	// we let our caller decide when to show m_hwndFrame.

	return;
}

bool XAP_Win32Frame::close()
{
	// NOTE: This may not be the proper place, but it does mean that the
	// last window closed is the one that the window state is stored from.
	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT); // must do
	if (GetWindowPlacement(m_hwndFrame, &wndPlacement))
	{
		getApp()->setGeometry(wndPlacement.rcNormalPosition.left, 
				wndPlacement.rcNormalPosition.top, 
				wndPlacement.rcNormalPosition.right - wndPlacement.rcNormalPosition.left,
				wndPlacement.rcNormalPosition.bottom - wndPlacement.rcNormalPosition.top,
				/* flag is meant for info about the position & size info stored, not a generic flag
				 * TODO: figure out where to store this then, so we can max/min/normal again
				wndPlacement.showCmd
				*/
				PREF_FLAG_GEOMETRY_POS | PREF_FLAG_GEOMETRY_SIZE
				);
	}
	else
	{
		// if failed to get placement then invalidate stored settings
		getApp()->setGeometry(0,0,0,0,0);
	}

	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);

	return true;
}

bool XAP_Win32Frame::raise()
{
	BringWindowToTop(m_hwndFrame);

	return true;
}

bool XAP_Win32Frame::show()
{
	ShowWindow(m_hwndFrame, SW_SHOW);
	UpdateWindow(m_hwndFrame);

	return true;
}

bool XAP_Win32Frame::openURL(const char * szURL)
{
	// NOTE: could get finer control over browser window via DDE 
	// NOTE: may need to fallback to WinExec for old NSCP versions

	UT_String sURL = szURL;
	HWND hwnd = getTopLevelWindow();

	// strip "file://" from URL, win32 doesn't handle them well
	if ( "file://" == sURL.substr(0, 7) )
	{
		sURL = sURL.substr(7, sURL.size() - 7);
	}
	
	for (int i=0; i<sURL.length();i++)
	  if (sURL[i]=='\\')      sURL[i]='/';
	  
	int res = (int) ShellExecute(hwnd, "open", sURL.c_str(), NULL, NULL, SW_SHOWNORMAL);

	// TODO: more specific (and localized) error messages ??
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
					MessageBox(hwnd, errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;

			case 3:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: The system cannot find the path specified.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(hwnd, errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;

			case 5:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: Access is denied.\n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(hwnd, errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;

			default:
				{
					errMsg = "Error ("; 
					errMsg += UT_String_sprintf("%d", res);
					errMsg += ") displaying URL: \n";
					errMsg += " [ ";  errMsg += sURL;  errMsg += " ] ";
					MessageBox(hwnd, errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
				}
				break;
		} /* switch (res) */
	} /* if (res <= 32) */

	return (res>32);
}

bool XAP_Win32Frame::updateTitle()
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	UT_String sTmp = getTitle(INT_MAX);
	sTmp += " - ";
	sTmp += m_pWin32App->getApplicationTitleForTitleBar();
	
	SetWindowText(m_hwndFrame, sTmp.c_str());

	return true;
}

LRESULT CALLBACK XAP_Win32Frame::_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32Frame * f = (XAP_Win32Frame*)GetWindowLong(hwnd, GWL_USERDATA);

	if (!f)
	{
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
	}
	
	AV_View * pView = NULL;

	pView = f->m_pView;

	if(iMsg == f->m_mouseWheelMessage)
	{
		wParam = MAKEWPARAM(0, (short)(int)wParam);
		return SendMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
	}

	switch (iMsg)
	{
	case WM_EXITMENULOOP:
	case WM_SETFOCUS:
		if (pView)
		{
			pView->focusChange(AV_FOCUS_HERE);
			SetFocus(f->m_hwndContainer);
		}
		return 0;

	case WM_ENTERMENULOOP:
	case WM_KILLFOCUS:
		if (pView)
		{
			pView->focusChange(AV_FOCUS_NONE);
		}
		return 0;

	case WM_CREATE:
		return 0;

	case WM_COMMAND:
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onCommand(pView,hwnd,wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onCommand(pView,hwnd,wParam))
		{
			return 0;
		}
		else if (HIWORD(wParam) == 0)
		{
			// after menu passes on it, give each of the toolbars a chance
			UT_uint32 nrToolbars, k;
			nrToolbars = f->m_vecToolbars.getItemCount();
			for (k=0; k < nrToolbars; k++)
			{
				EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecToolbars.getNthItem(k);
				XAP_Toolbar_Id id = t->ItemIdFromWmCommand(LOWORD(wParam));
				if (t->toolbarEvent(id))
					return 0;
			}
		}
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_INITMENU:
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onInitMenu(f,pView,hwnd,(HMENU)wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onInitMenu(f,pView,hwnd,(HMENU)wParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_MENUSELECT:
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) 
		{
		case TTN_NEEDTEXT:
			{
				UT_uint32 nrToolbars, k;
				nrToolbars = f->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecToolbars.getNthItem(k);
					if (t->getToolTip(lParam))
						break;
				}
			}
			break;

		case RBN_HEIGHTCHANGE:
			{
				RECT r;
				GetClientRect(f->m_hwndFrame, &r);
				int nWidth = r.right - r.left;
				int nHeight = r.bottom - r.top;

				GetClientRect(f->m_hwndRebar, &r);
				f->m_iBarHeight = r.bottom - r.top + 6;

				if (f->m_hwndContainer)
				{
					// leave room for the toolbars
					nHeight -= f->m_iBarHeight;

					MoveWindow(f->m_hwndContainer, 0, f->m_iBarHeight, nWidth, nHeight, TRUE);
				}
			}
			break;

		case NM_CUSTOMDRAW:
			{
				LPNMCUSTOMDRAW  pNMcd = (LPNMCUSTOMDRAW)lParam;
				UT_uint32 nrToolbars, k;
				nrToolbars = f->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecToolbars.getNthItem(k);
					if( pNMcd->hdr.hwndFrom == t->getWindow() )
					{
						if( pNMcd->dwDrawStage == CDDS_PREPAINT )
						{
							return CDRF_NOTIFYPOSTPAINT;
						}
						if( pNMcd->dwDrawStage == CDDS_POSTPAINT )
						{
							RECT  rc;
							HBRUSH	hBr = NULL;

							rc.top    = pNMcd->rc.top;
							rc.bottom = pNMcd->rc.bottom;
							hBr = GetSysColorBrush( COLOR_3DFACE );

							HWND  hWndChild = FindWindowEx( pNMcd->hdr.hwndFrom, NULL, NULL, NULL );
							while( hWndChild != NULL )
							{
								RECT   rcChild;
								POINT  pt;
								GetWindowRect( hWndChild, &rcChild );
								pt.x = rcChild.left;
								pt.y = rcChild.top;
								ScreenToClient( pNMcd->hdr.hwndFrom, &pt );
								rc.left = pt.x;
								pt.x = rcChild.right;
								pt.y = rcChild.bottom;
								ScreenToClient( pNMcd->hdr.hwndFrom, &pt );
								rc.right = pt.x;
								FillRect( pNMcd->hdc, &rc, hBr );
								hWndChild = FindWindowEx( pNMcd->hdr.hwndFrom, hWndChild, NULL, NULL );
							}

							if( hBr != NULL )
							{
								DeleteObject( hBr );
								hBr = NULL;
							}
						}
						break;
					}
				}
			}
			break;

		// Process other notifications here
		default:
			break;
		} /* switch (((LPNMHDR) lParam)->code) */
		break;

	case WM_SIZE:
		{
			int nWidth = LOWORD(lParam);
			int nHeight = HIWORD(lParam);

			if( pView && !pView->isLayoutFilling() )
			{
				f->_startViewAutoUpdater();

				if (nWidth != (int) f->m_iSizeWidth && f->m_hwndRebar != NULL)
				{
					MoveWindow(f->m_hwndRebar, 0, 0, nWidth, f->m_iBarHeight, TRUE); 
				}

				// leave room for the toolbars and the status bar
				nHeight -= f->m_iBarHeight;
				nHeight -= f->m_iStatusBarHeight;

				if (f->m_hwndContainer)
					MoveWindow(f->m_hwndContainer, 0, f->m_iBarHeight, nWidth, nHeight, TRUE);

				if (f->m_hwndStatusBar)
					MoveWindow(f->m_hwndStatusBar, 0, f->m_iBarHeight+nHeight, nWidth, f->m_iStatusBarHeight, TRUE);
				
				f->m_iSizeWidth = nWidth;
				f->m_iSizeHeight = nHeight;

				f->updateZoom();
			}

			return 0;
		}

	case WM_CLOSE:
		{
			XAP_App * pApp = f->getApp();
			UT_ASSERT(pApp);

			const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
			UT_ASSERT(pEMC);

			EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
			UT_ASSERT(pEM);						// make sure it's bound to something

			if (pEM)
			{
				(*pEM->getFn())(pView,NULL);
				return 0;
			}

			// let the window be destroyed
			break;
		}

	case WM_INPUTLANGCHANGE:
		{
			UT_DEBUGMSG(("Frame received input language change\n"));

			// This will remap the static tables used by all frames.
			// (see the comment in ev_Win32Keyboard.cpp.)
			ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(f->m_pKeyboard);
			pWin32Keyboard->remapKeyboard((HKL)lParam);

			// Do not propagate this message.
			
			return 1; //DefWindowProc(hwnd, iMsg, wParam, lParam);
		}

	case WM_MOUSEWHEEL:
		{
			return SendMessage(f->m_hwndContainer, iMsg, wParam, lParam);
		}

	case WM_SYSCOLORCHANGE:
		{
			if (f->m_hwndRebar)
			{
				SendMessage(f->m_hwndRebar,WM_SYSCOLORCHANGE,0,0);

				REBARBANDINFO rbbi = { 0 };
				rbbi.cbSize = sizeof(REBARBANDINFO);
				rbbi.fMask = RBBIM_COLORS;
				rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
				rbbi.clrBack = GetSysColor(COLOR_BTNFACE);

				UT_uint32 nrToolbars = f->m_vecToolbars.getItemCount();
				for (UT_uint32 k=0; k < nrToolbars; k++)
					SendMessage(f->m_hwndRebar, RB_SETBANDINFO,k,(LPARAM)&rbbi);
			}

			if (f->m_hwndContainer)
				SendMessage(f->m_hwndContainer,WM_SYSCOLORCHANGE,0,0);
			if (f->m_hwndStatusBar)
				SendMessage(f->m_hwndStatusBar,WM_SYSCOLORCHANGE,0,0);
			return 0;
		}

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP) wParam; 
			// How many files were dropped?
			int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			char szFileName[_MAX_PATH];
			int i,pathlength;
			for (i=0; i<count; i++)
			{
				pathlength = DragQueryFile(hDrop, i, NULL, 0);
				if (pathlength < _MAX_PATH)
				{
					DragQueryFile(hDrop, i, szFileName, _MAX_PATH);
					XAP_App * pApp = f->getApp();
					UT_ASSERT(pApp);
					FV_View* pView = (FV_View *) f->getCurrentView();
					XAP_Frame * pNewFrame = 0;
					IEGraphicFileType iegft = IEGFT_Unknown;					
					IE_ImpGraphic *pIEG;
					FG_Graphic* pFG;
					UT_Error errorCode;

					/*
						The user may be dropping every kind of file
						Check first if the file is a graphic. If it's a graphics. I we insert it 
						in the document, if not we assume that it's document 		
					*/								
					// If there is no import graphic, it's a document...
					errorCode = IE_ImpGraphic::constructImporter(szFileName, iegft, &pIEG);
					if(errorCode == UT_OK)
					{						
						errorCode = pIEG->importGraphic(szFileName, &pFG);
						if(errorCode != UT_OK || !pFG)
						{
							s_CouldNotLoadFileMessage(f, szFileName, errorCode);							
							DELETEP(pIEG);
							return 0;
						}

						DELETEP(pIEG);					

						errorCode = pView->cmdInsertGraphic(pFG, szFileName);
						if (errorCode != UT_OK)
						{
							s_CouldNotLoadFileMessage(f, szFileName, errorCode);							
							DELETEP(pFG);
							return 0;
						}
						
						DELETEP(pFG);
					  }
					else
					{	
						// Check if the current document is empty.
						if (f->isDirty() || f->getFilename() ||
							(f->getViewNumber() > 0))
						{
							pNewFrame = pApp->newFrame();
							if (pNewFrame == NULL)
							{
								f->setStatusMessage("Could not open another window");
								return 0;
							}
						}
						else
						{
							pNewFrame = f;
						}

						UT_Error error = pNewFrame->loadDocument(szFileName, IEFT_Unknown);
						if (error != UT_OK)
						{
							if (f != pNewFrame)
								pNewFrame->close();
							s_CouldNotLoadFileMessage(f, szFileName, error);
						}
						else
						{
							pNewFrame->show();
						}
					  }
				}
				else
				{
				}
			}
			DragFinish(hDrop);
		}
		return 0;

	case WM_DESTROY:
		return 0;

	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

/*****************************************************************/

bool XAP_Win32Frame::runModalContextMenu(AV_View * pView, const char * szMenuName,
											UT_sint32 x, UT_sint32 y)
{
	bool bResult = false;

	UT_ASSERT(!m_pWin32Popup);

	m_pWin32Popup = new EV_Win32MenuPopup(m_pWin32App,szMenuName,m_szMenuLabelSetName);
	if (m_pWin32Popup && m_pWin32Popup->synthesizeMenuPopup(this))
	{
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));

		translateDocumentToScreen(x,y);

		TrackPopupMenu(m_pWin32Popup->getMenuHandle(),
					   TPM_CENTERALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
					   x,y,0,m_hwndFrame,NULL);

		// the popup steals our capture, so we need to reset our counter.
		EV_Win32Mouse *pWin32Mouse = static_cast<EV_Win32Mouse *>(m_pMouse);
		pWin32Mouse->reset();
	}

	DELETEP(m_pWin32Popup);
	return bResult;
}

EV_Toolbar * XAP_Win32Frame::_newToolbar(XAP_App*		app,
										 XAP_Frame*		frame,
										 const char*	szLayout,
										 const char*	szLanguage)
{
	EV_Win32Toolbar *result = new EV_Win32Toolbar(static_cast<XAP_Win32App *>(app), 
												  static_cast<XAP_Win32Frame *>(frame), 
												  szLayout, szLanguage);
	// for now, position each one manually
	// TODO: put 'em all in a rebar instead
	HWND hwndBar = result->getWindow();
	
	RECT rcClient;
	GetClientRect(hwndBar, &rcClient);
	const UT_uint32 iHeight = rcClient.bottom - rcClient.top;
	
	m_iBarHeight += iHeight;

	return result;
}

EV_Menu* XAP_Win32Frame::getMainMenu()
{
	return m_pWin32Menu;
}

void XAP_Win32Frame::nullUpdate() const
{
	MSG msg;
	for( int i = 0 ; i < 10 ; i++ )
	{
		if( PeekMessage( &msg, (HWND) NULL, 0, 0, PM_REMOVE) )
		{
	        DispatchMessage(&msg); 
    	} 
	}
}

void XAP_Win32Frame::setCursor(GR_Graphics::Cursor c)
{
	// TODO - currently does nothing
}


