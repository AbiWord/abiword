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

#define HACK_RULER_SIZE		25			// TODO move this

#define DELETEP(p)		do { if (p) delete p; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static char s_ContainerWndClassName[256];
static char s_DocumentWndClassName[256];
static char s_TopRulerWndClassName[256];
static char s_LeftRulerWndClassName[256];

UT_Bool AP_Win32Frame::RegisterClass(AP_Win32App * app)
{
	// NB: can't access 'this' members from a static member function
	WNDCLASSEX  wndclass;

	// register class for the frame window
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = AP_Win32Frame::_FrameWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = app->getApplicationName();
	wndclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}

	// register class for the container window (this will contain the document
	// and the rulers and the scroll bars)
	
	sprintf(s_ContainerWndClassName, "%sContainer", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = AP_Win32Frame::_ContainerWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_ContainerWndClassName;
	wndclass.hIconSm       = NULL;

	if (!RegisterClassEx(&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}
	
	// register class for the actual document window
	sprintf(s_DocumentWndClassName, "%sDocument", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = AP_Win32Frame::_DocumentWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_DocumentWndClassName;
	wndclass.hIconSm       = NULL;

	if (!RegisterClassEx(&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}
	
	// register class for the top ruler
	sprintf(s_TopRulerWndClassName, "%sTopRuler", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = AP_Win32Frame::_TopRulerWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_TopRulerWndClassName;
	wndclass.hIconSm       = NULL;

	if (!RegisterClassEx(&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}
	
	// register class for the left ruler
	sprintf(s_LeftRulerWndClassName, "%sLeftRuler", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = AP_Win32Frame::_LeftRulerWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_LeftRulerWndClassName;
	wndclass.hIconSm       = NULL;

	if (!RegisterClassEx(&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}

	return UT_TRUE;
}

#define GWL(hwnd)		(AP_Win32Frame*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(AP_Win32Frame*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

/*****************************************************************/

AP_Win32Frame::AP_Win32Frame(AP_Win32App * app)
	: AP_Frame(static_cast<AP_App *>(app)),
	  m_dialogFactory(this)
{
	m_pWin32App = app;
	m_pWin32Keyboard = NULL;
	m_pWin32Mouse = NULL;
	m_pWin32Menu = NULL;
	m_pView = NULL;
	m_hwndFrame = NULL;
	m_hwndRebar = NULL;
	m_hwndContainer = NULL;
	m_hwndTopRuler = NULL;
	m_hwndLeftRuler = NULL;
	m_hwndDeadLowerRight = NULL;
	m_hwndVScroll = NULL;
	m_hwndHScroll = NULL;
	m_hwndDocument = NULL;
	m_iSizeWidth = 0;
	m_iSizeHeight = 0;
}

// TODO when cloning a new frame from an existing one
// TODO should we also clone any frame-persistent
// TODO dialog data ??

AP_Win32Frame::AP_Win32Frame(AP_Win32Frame * f)
	: AP_Frame(static_cast<AP_Frame *>(f)),
	  m_dialogFactory(this)
{
	m_pWin32App = f->m_pWin32App;
	m_pWin32Keyboard = NULL;
	m_pWin32Mouse = NULL;
	m_pWin32Menu = NULL;
	m_pView = NULL;
	m_hwndFrame = NULL;
	m_hwndRebar = NULL;
	m_hwndContainer = NULL;
	m_hwndTopRuler = NULL;
	m_hwndLeftRuler = NULL;
	m_hwndDeadLowerRight = NULL;
	m_hwndVScroll = NULL;
	m_hwndHScroll = NULL;
	m_hwndDocument = NULL;
	m_iSizeWidth = 0;
	m_iSizeHeight = 0;
}

AP_Win32Frame::~AP_Win32Frame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pWin32Keyboard);
	DELETEP(m_pWin32Mouse);
	DELETEP(m_pWin32Menu);
	UT_VECTOR_PURGEALL(EV_Win32Toolbar *, m_vecWin32Toolbars);
}

UT_Bool AP_Win32Frame::initialize(void)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	bResult = AP_Frame::initialize();
	UT_ASSERT(bResult);

	_createTopLevelWindow();
	
	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	m_pWin32Keyboard = new ev_Win32Keyboard(m_pEEM);
	UT_ASSERT(m_pWin32Keyboard);
	
	m_pWin32Mouse = new EV_Win32Mouse(m_pEEM);
	UT_ASSERT(m_pWin32Mouse);

	// ... add other stuff here...

	// TODO: Jeff, I'm currently showing in WinMain, to honor iCmdShow.
	// should we pass that via argv, to do it here for all frames?

	return UT_TRUE;
}

AP_Frame * AP_Win32Frame::cloneFrame(void)
{
	AP_Win32Frame * pClone = new AP_Win32Frame(this);
	ENSUREP(pClone);

	if (!pClone->initialize())
		goto Cleanup;

	if (!pClone->_showDocument())
		goto Cleanup;

	pClone->show();

	return pClone;

Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		m_pWin32App->forgetFrame(pClone);
		delete pClone;
	}

	return NULL;
}

HWND AP_Win32Frame::getTopLevelWindow(void) const
{
	return m_hwndFrame;
}

HWND AP_Win32Frame::getToolbarWindow(void) const
{
	return m_hwndRebar;
}

EV_Win32Mouse * AP_Win32Frame::getWin32Mouse(void)
{
	return m_pWin32Mouse;
}

ev_Win32Keyboard * AP_Win32Frame::getWin32Keyboard(void)
{
	return m_pWin32Keyboard;
}

AP_DialogFactory * AP_Win32Frame::getDialogFactory(void)
{
	return &m_dialogFactory;
}

void AP_Win32Frame::_createTopLevelWindow(void)
{
	RECT r, rTopRuler;
	UT_uint32 iHeight, iWidth;
	int cxVScroll, cyHScroll;

	// create a top-level window for us.
	// TODO get the default window size from preferences or something.

	m_hwndFrame = CreateWindow(m_pWin32App->getApplicationName(),
							   m_pWin32App->getApplicationTitleForTitleBar(),
							   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							   NULL, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndFrame);
	SWL(m_hwndFrame, this);					// bind this frame to its window

	// synthesize a menu from the info in our base class.
	m_pWin32Menu = new EV_Win32Menu(m_pWin32App,this,
								  m_szMenuLayoutName,
								  m_szMenuLabelSetName);
	UT_ASSERT(m_pWin32Menu);
	UT_Bool bResult = m_pWin32Menu->synthesize();
	UT_ASSERT(bResult);

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

	// create a child window for us -- this will be the 'container'.
	// the 'container' will in turn contain the document window, the
	// rulers, and the various scroll bars and other dead space.
	
	m_hwndContainer = CreateWindowEx(WS_EX_CLIENTEDGE, s_ContainerWndClassName, NULL,
									 WS_CHILD | WS_VISIBLE,
									 0, m_iBarHeight, iWidth, iHeight,
									 m_hwndFrame, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndContainer);
	SWL(m_hwndContainer, this);
	
	// now create all the various windows inside the container window.

	GetClientRect(m_hwndContainer,&r);
	cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

	m_hwndVScroll = CreateWindowEx(0,"ScrollBar",NULL,
								   WS_CHILD | WS_VISIBLE | SBS_VERT,
								   r.right-cxVScroll, 0, cxVScroll, r.bottom-cyHScroll,
								   m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndVScroll);
	SWL(m_hwndVScroll, this);

	m_hwndHScroll = CreateWindowEx(0,"ScrollBar",NULL,
								   WS_CHILD | WS_VISIBLE | SBS_HORZ,
								   0, r.bottom-cyHScroll, r.right-cxVScroll, cyHScroll,
								   m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndHScroll);
	SWL(m_hwndHScroll, this);

	m_hwndDeadLowerRight = CreateWindowEx(0,"ScrollBar",NULL,
										  WS_CHILD | WS_VISIBLE | WS_DISABLED | SBS_SIZEBOX,
										  r.right-cxVScroll, r.bottom-cyHScroll, cxVScroll, cyHScroll,
										  m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndHScroll);
	SWL(m_hwndHScroll, this);

	m_hwndTopRuler = CreateWindowEx(0, s_TopRulerWndClassName, NULL,
									WS_CHILD | WS_VISIBLE,
									0, 0, r.right - cxVScroll, HACK_RULER_SIZE,
									m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndTopRuler);
	SWL(m_hwndTopRuler, this);

	GetClientRect(m_hwndTopRuler, &rTopRuler);
	
	m_hwndLeftRuler = CreateWindowEx(0, s_LeftRulerWndClassName, NULL,
									 WS_CHILD | WS_VISIBLE,
									 0, rTopRuler.bottom,
									 HACK_RULER_SIZE, r.bottom - rTopRuler.bottom - cyHScroll,
									 m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndLeftRuler);
	SWL(m_hwndLeftRuler, this);

	// create a child window for us.
	m_hwndDocument = CreateWindowEx(0, s_DocumentWndClassName, NULL,
									WS_CHILD | WS_VISIBLE,
									HACK_RULER_SIZE, HACK_RULER_SIZE,
									r.right - HACK_RULER_SIZE - cxVScroll,
									r.bottom - HACK_RULER_SIZE - cyHScroll,
									m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndDocument);
	SWL(m_hwndDocument, this);

	// we let our caller decide when to show m_hwndFrame.

	return;
}

UT_Bool AP_Win32Frame::loadDocument(const char * szFilename)
{
	if (! AP_Frame::loadDocument(szFilename))
	{
		// we could not load the document.
		// TODO how should we complain to the user ??

		return UT_FALSE;
	}

	return _showDocument();
}
	
void AP_Win32Frame::_scrollFunc(void* pData, UT_sint32 xoff, UT_sint32 yoff)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	// Do the vertical

	HWND hwndV = pWin32Frame->m_hwndVScroll;
	GetScrollInfo(hwndV, SB_CTL, &si);

	si.nPos = yoff;
	SetScrollInfo(hwndV, SB_CTL, &si, TRUE);

	// TODO: move this logic back to shared code
	GetScrollInfo(hwndV, SB_CTL, &si);	// may have been clamped
	pWin32Frame->m_pView->setYScrollOffset(si.nPos);

	// Do the horizontal

	HWND hwndH = pWin32Frame->m_hwndHScroll;
	GetScrollInfo(hwndH, SB_CTL, &si);

	si.nPos = xoff;
	SetScrollInfo(hwndH, SB_CTL, &si, TRUE);

	// TODO: move this logic back to shared code
	GetScrollInfo(hwndH, SB_CTL, &si);	// may have been clamped
	pWin32Frame->m_pView->setXScrollOffset(si.nPos);
}

UT_Bool AP_Win32Frame::close()
{
	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);

	return UT_TRUE;
}

UT_Bool AP_Win32Frame::raise()
{
	BringWindowToTop(m_hwndFrame);

	return UT_TRUE;
}

UT_Bool AP_Win32Frame::show()
{
	ShowWindow(m_hwndFrame, SW_SHOWNORMAL);
	UpdateWindow(m_hwndFrame);

	return UT_TRUE;
}

UT_Bool AP_Win32Frame::updateTitle()
{
	if (!AP_Frame::updateTitle())
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

LRESULT CALLBACK AP_Win32Frame::_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Frame * f = GWL(hwnd);
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
		if (f->m_pWin32Menu->onCommand(pView,hwnd,wParam))
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
		if (f->m_pWin32Menu->onInitMenu(pView,hwnd,(HMENU)wParam))
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

		if (f->m_hwndContainer)
		{
			// leave room for the toolbars
			nHeight -= f->m_iBarHeight;

			MoveWindow(f->m_hwndContainer, 0, f->m_iBarHeight, nWidth, nHeight, TRUE);
		}

		f->m_iSizeWidth = nWidth;
		f->m_iSizeHeight = nHeight;
		
		return 0;
	}

	case WM_CLOSE:
	{
		AP_App * pApp = f->getApp();
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
	
	case WM_DESTROY:
		return 0;

	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

#define SCROLL_LINE_SIZE 20

LRESULT CALLBACK AP_Win32Frame::_ContainerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Frame * f = GWL(hwnd);
	AV_View * pView = NULL;

	if (f)
		pView = f->m_pView;

	UT_DEBUGMSG(("ContainerWndProc: [iMsg 0x%08lx][wParam 0x%08lx][lParam 0x%08lx]\n",iMsg,wParam,lParam));
	
	switch (iMsg)
	{
	case WM_SIZE:
	{
		if (f)
		{
			int nWidth = LOWORD(lParam);
			int nHeight = HIWORD(lParam);
			int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
			int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
			
			MoveWindow(f->m_hwndVScroll, nWidth-cxVScroll, 0, cxVScroll, nHeight-cyHScroll, TRUE);
			MoveWindow(f->m_hwndHScroll, 0, nHeight-cyHScroll, nWidth - cxVScroll, cyHScroll, TRUE);
			MoveWindow(f->m_hwndDeadLowerRight, nWidth-cxVScroll, nHeight-cyHScroll, cxVScroll, cyHScroll, TRUE);
			MoveWindow(f->m_hwndTopRuler, 0, 0, nWidth-cxVScroll, HACK_RULER_SIZE, TRUE);
			MoveWindow(f->m_hwndLeftRuler, 0, HACK_RULER_SIZE,
					   HACK_RULER_SIZE, nHeight - HACK_RULER_SIZE - cyHScroll, TRUE);
			MoveWindow(f->m_hwndDocument, HACK_RULER_SIZE, HACK_RULER_SIZE,
					   nWidth - HACK_RULER_SIZE - cxVScroll, nHeight - HACK_RULER_SIZE - cyHScroll, TRUE);
		}
		return 0;
	}

	case WM_VSCROLL:
	{
		int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
		int nPos = (int) HIWORD(wParam);  // scroll box position 

		SCROLLINFO si;
		memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(f->m_hwndVScroll, SB_CTL, &si);

		switch (nScrollCode)
		{
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(f->m_hwndVScroll, SB_CTL, &si, TRUE);
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			SetScrollInfo(f->m_hwndVScroll, SB_CTL, &si, TRUE);
			break;
		case SB_LINEDOWN:
			si.nPos += SCROLL_LINE_SIZE;
			SetScrollInfo(f->m_hwndVScroll, SB_CTL, &si, TRUE);
			break;
		case SB_LINEUP:
			si.nPos -= SCROLL_LINE_SIZE;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(f->m_hwndVScroll, SB_CTL, &si, TRUE);
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = nPos;
			SetScrollInfo(f->m_hwndVScroll, SB_CTL, &si, TRUE);
			break;
		}

		if (nScrollCode != SB_ENDSCROLL)
		{
			// in case we got clamped
			GetScrollInfo(f->m_hwndVScroll, SB_CTL, &si);

			// now tell the view
			pView->setYScrollOffset(si.nPos);
		}
	}
	return 0;

	case WM_HSCROLL:
	{
		int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
		int nPos = (int) HIWORD(wParam);  // scroll box position 

		SCROLLINFO si;
		memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(f->m_hwndHScroll, SB_CTL, &si);

		switch (nScrollCode)
		{
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(f->m_hwndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			SetScrollInfo(f->m_hwndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_LINEDOWN:
			si.nPos += SCROLL_LINE_SIZE;
			SetScrollInfo(f->m_hwndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_LINEUP:
			si.nPos -= SCROLL_LINE_SIZE;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(f->m_hwndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = nPos;
			SetScrollInfo(f->m_hwndHScroll, SB_CTL, &si, TRUE);
			break;
		}

		if (nScrollCode != SB_ENDSCROLL)
		{
			// in case we got clamped
			GetScrollInfo(f->m_hwndHScroll, SB_CTL, &si);

			// now tell the view
			pView->setXScrollOffset(si.nPos);
		}
	}
	return 0;

	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
}

LRESULT CALLBACK AP_Win32Frame::_TopRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

LRESULT CALLBACK AP_Win32Frame::_LeftRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

LRESULT CALLBACK AP_Win32Frame::_DocumentWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc;
	PAINTSTRUCT ps;

	AP_Win32Frame * f = GWL(hwnd);
	AV_View * pView = NULL;
	EV_Win32Mouse * pMouse = NULL;

	if (f)
	{
		pView = f->m_pView;
		pMouse = f->m_pWin32Mouse;
	}

	switch (iMsg)
	{
	case WM_CREATE:
		return 0;

	case WM_LBUTTONDOWN:
		pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_MBUTTONDOWN:
		pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON2,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_RBUTTONDOWN:
		pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON3,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case WM_LBUTTONDBLCLK:
		pMouse->onDoubleClick(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_MBUTTONDBLCLK:
		pMouse->onDoubleClick(pView,hwnd,EV_EMB_BUTTON2,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_RBUTTONDBLCLK:
		pMouse->onDoubleClick(pView,hwnd,EV_EMB_BUTTON3,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case WM_MOUSEMOVE:
		pMouse->onButtonMove(pView,hwnd,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case WM_LBUTTONUP:
		pMouse->onButtonUp(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_MBUTTONUP:
		pMouse->onButtonUp(pView,hwnd,EV_EMB_BUTTON2,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_RBUTTONUP:
		pMouse->onButtonUp(pView,hwnd,EV_EMB_BUTTON3,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case WM_SIZE:
	{
		if (pView)
		{
			int nWidth = LOWORD(lParam);
			int nHeight = HIWORD(lParam);

			pView->setWindowSize(nWidth, nHeight);
		}
		return 0;
	}

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

		UT_DEBUGMSG(("Calling draw()\n"));
		UT_Rect r(ps.rcPaint.left,ps.rcPaint.top,
				  ps.rcPaint.right-ps.rcPaint.left,
				  ps.rcPaint.bottom-ps.rcPaint.top);
		pView->draw(&r);

		EndPaint(hwnd, &ps);

		return 0;
	}

	case WM_DESTROY:
		return 0;
	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
