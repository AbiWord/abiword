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

#include "ut_types.h"
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

FILE * fpLog = NULL;

/*****************************************************************/

#define GWL(hwnd)		(XAP_Win32Frame*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(XAP_Win32Frame*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

UT_Bool XAP_Win32Frame::RegisterClass(XAP_Win32App * app)
{
	// NB: can't access 'this' members from a static member function
	WNDCLASSEX  wndclass;
	ATOM a;

	// register class for the frame window
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = XAP_Win32Frame::_FrameWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = app->getApplicationName();
	wndclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	a = RegisterClassEx(&wndclass);
	UT_ASSERT(a);

	fpLog = fopen("c:\\message.log","w");
	
	return UT_TRUE;
}

/*****************************************************************/

XAP_Win32Frame::XAP_Win32Frame(XAP_Win32App * app)
	: XAP_Frame(static_cast<XAP_App *>(app)),
	  m_dialogFactory(this)
{
	m_pWin32App = app;
	m_pWin32Keyboard = NULL;
	m_pWin32Mouse = NULL;
	m_pWin32Menu = NULL;
	m_pWin32Popup = NULL;
	m_pView = NULL;
	m_hwndFrame = NULL;
	m_hwndRebar = NULL;
	m_hwndContainer = NULL;
	m_iSizeWidth = 0;
	m_iSizeHeight = 0;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

XAP_Win32Frame::XAP_Win32Frame(XAP_Win32Frame * f)
	: XAP_Frame(static_cast<XAP_Frame *>(f)),
	  m_dialogFactory(this)
{
	m_pWin32App = f->m_pWin32App;
	m_pWin32Keyboard = NULL;
	m_pWin32Mouse = NULL;
	m_pWin32Menu = NULL;
	m_pWin32Popup = NULL;
	m_pView = NULL;
	m_hwndFrame = NULL;
	m_hwndRebar = NULL;
	m_hwndContainer = NULL;
	m_iSizeWidth = 0;
	m_iSizeHeight = 0;
}

XAP_Win32Frame::~XAP_Win32Frame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pWin32Keyboard);
	DELETEP(m_pWin32Mouse);
	DELETEP(m_pWin32Menu);
	DELETEP(m_pWin32Popup);
	UT_VECTOR_PURGEALL(EV_Win32Toolbar *, m_vecWin32Toolbars);
}

UT_Bool XAP_Win32Frame::initialize(void)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	bResult = XAP_Frame::initialize();
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	EV_EditEventMapper * pEEM = getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pWin32Keyboard = new ev_Win32Keyboard(pEEM);
	UT_ASSERT(m_pWin32Keyboard);
	
	m_pWin32Mouse = new EV_Win32Mouse(pEEM);
	UT_ASSERT(m_pWin32Mouse);

	// TODO: Jeff, I'm currently showing in WinMain, to honor iCmdShow.
	// should we pass that via argv, to do it here for all frames?

	return UT_TRUE;
}

UT_sint32 XAP_Win32Frame::setInputMode(const char * szName)
{
	UT_sint32 result = XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pWin32Keyboard->setEditEventMap(pEEM);
		m_pWin32Mouse->setEditEventMap(pEEM);
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

EV_Win32Mouse * XAP_Win32Frame::getWin32Mouse(void)
{
	return m_pWin32Mouse;
}

ev_Win32Keyboard * XAP_Win32Frame::getWin32Keyboard(void)
{
	return m_pWin32Keyboard;
}

AP_DialogFactory * XAP_Win32Frame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void XAP_Win32Frame::_createTopLevelWindow(void)
{
	RECT r;
	UT_uint32 iHeight, iWidth;

	// create a top-level window for us.
	// TODO get the default window size from preferences or something.

	m_hwndFrame = CreateWindow(m_pWin32App->getApplicationName(),
							   m_pWin32App->getApplicationTitleForTitleBar(),
							   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndFrame);
	SWL(m_hwndFrame, this);					// bind this frame to its window

	// synthesize a menu from the info in our
	// base class and install it into the window.
	m_pWin32Menu = new EV_Win32MenuBar(m_pWin32App,
									   getEditEventMapper(),
									   m_szMenuLayoutName,
									   m_szMenuLabelSetName);
	UT_ASSERT(m_pWin32Menu);
	UT_Bool bResult = m_pWin32Menu->synthesizeMenuBar();
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
	m_iBarHeight = 0;

	UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		EV_Win32Toolbar * pWin32Toolbar
			= new EV_Win32Toolbar(m_pWin32App,this,
								 (const char *)m_vecToolbarLayoutNames.getNthItem(k),
								 m_szToolbarLabelSetName);
		UT_ASSERT(pWin32Toolbar);
		bResult = pWin32Toolbar->synthesize();
		UT_ASSERT(bResult);
		
		m_vecWin32Toolbars.addItem(pWin32Toolbar);

		// for now, position each one manually
		// TODO: put 'em all in a rebar instead
		HWND hwndBar = pWin32Toolbar->getWindow();

		GetClientRect(hwndBar, &r);
		iHeight = r.bottom - r.top;

		m_iBarHeight += iHeight;
	}

	// figure out how much room is left for the child
	GetClientRect(m_hwndFrame, &r);
	iHeight = r.bottom - r.top;
	iWidth = r.right - r.left;

	m_iSizeWidth = iWidth;
	m_iSizeHeight = iHeight;
	
	// force rebar to resize itself
	// TODO for some reason, we give REBAR the height of the FRAME
	// TODO and let it decide how much it actually needs....
	MoveWindow(m_hwndRebar, 0, 0, iWidth, iHeight, TRUE);

	GetClientRect(m_hwndRebar, &r);
	m_iBarHeight = r.bottom - r.top + 6;

	UT_ASSERT(iHeight > m_iBarHeight);
	iHeight -= m_iBarHeight;

	m_hwndContainer = _createDocumentWindow(m_hwndFrame, 0, m_iBarHeight, iWidth, iHeight);

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.

	m_hwndStatusBar = _createStatusBarWindow(m_hwndFrame,0,m_iBarHeight+iHeight,iWidth);
	GetClientRect(m_hwndStatusBar,&r);
	m_iStatusBarHeight = r.bottom;

	// we let our caller decide when to show m_hwndFrame.

	return;
}

UT_Bool XAP_Win32Frame::close()
{
	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);

	return UT_TRUE;
}

UT_Bool XAP_Win32Frame::raise()
{
	BringWindowToTop(m_hwndFrame);

	return UT_TRUE;
}

UT_Bool XAP_Win32Frame::show()
{
	ShowWindow(m_hwndFrame, SW_SHOW);
	UpdateWindow(m_hwndFrame);

	return UT_TRUE;
}

UT_Bool XAP_Win32Frame::updateTitle()
{
	if (!XAP_Frame::updateTitle())
	{
		// no relevant change, so skip it
		return UT_FALSE;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pWin32App->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;
	
	const char * szTitle = getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);
	
	SetWindowText(m_hwndFrame, buf);

	return UT_TRUE;
}

LRESULT CALLBACK XAP_Win32Frame::_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if (fpLog)
	{
		fprintf(fpLog,"Msg [%lx] WParam [%lx] LParam [%lx]\n",iMsg,wParam,lParam);
		fflush(fpLog);
	}
	
	XAP_Win32Frame * f = GWL(hwnd);
	AV_View * pView = NULL;

	if (f)
	{
		pView = f->m_pView;
	}

	switch (iMsg)
	{
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
			nrToolbars = f->m_vecWin32Toolbars.getItemCount();
			for (k=0; k < nrToolbars; k++)
			{
				EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecWin32Toolbars.getNthItem(k);
				AP_Toolbar_Id id = t->ItemIdFromWmCommand(LOWORD(wParam));
				if (t->toolbarEvent(id))
					return 0;
			}
		}
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_INITMENU:
		if (f->m_pWin32Popup)
		{
			if (f->m_pWin32Popup->onInitMenu(pView,hwnd,(HMENU)wParam))
				return 0;
		}
		else if (f->m_pWin32Menu->onInitMenu(pView,hwnd,(HMENU)wParam))
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

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (f->m_pWin32Keyboard->onKeyDown(pView,hwnd,iMsg,wParam,lParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
	case WM_SYSCHAR:
	case WM_CHAR:
		if (f->m_pWin32Keyboard->onChar(pView,hwnd,iMsg,wParam,lParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) 
		{ 
        case TTN_NEEDTEXT:             
			{             
				UT_uint32 nrToolbars, k;
				nrToolbars = f->m_vecWin32Toolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)f->m_vecWin32Toolbars.getNthItem(k);
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

		// Process other notifications here
        default:
			break;
		} 
		break;

	case WM_SIZE:
	{
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);

		if (nWidth != (int) f->m_iSizeWidth)
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
		
		return 0;
	}

	case WM_CLOSE:
	{
		XAP_App * pApp = f->getApp();
		UT_ASSERT(pApp);

		const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
		UT_ASSERT(pEMC);

		EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindow");
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

		f->m_pWin32Keyboard->remapKeyboard((HKL)lParam);

		// We must propagate this message.
		
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	case WM_SYSCOLORCHANGE:
	{
		if (f->m_hwndRebar)
		{
			SendMessage(f->m_hwndRebar,WM_SYSCOLORCHANGE,0,0);

			REBARBANDINFO rbbi;
			ZeroMemory(&rbbi, sizeof(rbbi));
			rbbi.cbSize = sizeof(REBARBANDINFO);
			rbbi.fMask = RBBIM_COLORS;
			rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
			rbbi.clrBack = GetSysColor(COLOR_BTNFACE);

			UT_uint32 nrToolbars = f->m_vecWin32Toolbars.getItemCount();
			for (UT_uint32 k=0; k < nrToolbars; k++)
				SendMessage(f->m_hwndRebar, RB_SETBANDINFO,k,(LPARAM)&rbbi);
		}

		if (f->m_hwndContainer)
			SendMessage(f->m_hwndContainer,WM_SYSCOLORCHANGE,0,0);
		if (f->m_hwndStatusBar)
			SendMessage(f->m_hwndStatusBar,WM_SYSCOLORCHANGE,0,0);
		return 0;
	}

	case WM_DESTROY:
		return 0;

	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

/*****************************************************************/

UT_Bool XAP_Win32Frame::runModalContextMenu(AV_View * pView, const char * szMenuName,
											UT_sint32 x, UT_sint32 y)
{
	UT_Bool bResult = UT_FALSE;

	UT_ASSERT(!m_pWin32Popup);

	m_pWin32Popup = new EV_Win32MenuPopup(m_pWin32App,szMenuName,m_szMenuLabelSetName);
	if (m_pWin32Popup && m_pWin32Popup->synthesizeMenuPopup())
	{
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));

		translateDocumentToScreen(x,y);

		TrackPopupMenu(m_pWin32Popup->getMenuHandle(),
					   TPM_CENTERALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
					   x,y,0,m_hwndFrame,NULL);

		// the popup steals our capture, so we need to reset our counter.
		m_pWin32Mouse->reset();
	}

	DELETEP(m_pWin32Popup);
	return bResult;
}
