/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 

#include <windows.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_Win32Ap.h"
#include "ap_Win32Frame.h"
#include "ev_Win32Keyboard.h"
#include "ev_Win32Mouse.h"
#include "ev_Win32Menu.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "gr_Win32Graphics.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

//LRESULT CALLBACK AP_Win32Frame::_WndProc (HWND, UINT, WPARAM, LPARAM) ;

UT_Bool AP_Win32Frame::RegisterClass(AP_Win32Ap * ap)
{
	// NB: can't access 'this' members from a static member function
	WNDCLASSEX  wndclass ;

	wndclass.cbSize        = sizeof (wndclass) ;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS ;
	wndclass.lpfnWndProc   = AP_Win32Frame::_WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = sizeof(AP_Win32Frame*) ;
	wndclass.hInstance     = ap->getInstance() ;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = ap->getApplicationName() ;
	wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION) ;

	if (!RegisterClassEx (&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}

	return UT_TRUE;
}

#define GWL(hwnd)		(AP_Win32Frame*)GetWindowLong((hwnd), 0)
#define SWL(hwnd, f)	(AP_Win32Frame*)SetWindowLong((hwnd), 0, (LONG)(f))

/*****************************************************************/

AP_Win32Frame::AP_Win32Frame(AP_Win32Ap * ap)
	: AP_Frame(static_cast<AP_Ap *>(ap))
{
	m_pWin32Ap = ap;
	m_pWin32Keyboard = NULL;
	m_pWin32Mouse = NULL;
	m_pWin32Menu = NULL;
	m_pView = NULL;
	m_hwnd = NULL;
}

AP_Win32Frame::~AP_Win32Frame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pWin32Keyboard);
	DELETEP(m_pWin32Mouse);
	DELETEP(m_pWin32Menu);
}

UT_Bool AP_Win32Frame::initialize(int * pArgc, char *** pArgv)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	bResult = AP_Frame::initialize(pArgc,pArgv);
	UT_ASSERT(bResult);

	_createTopLevelWindow();
	
	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	m_pWin32Keyboard = new ev_Win32Keyboard(m_pEEM);
	UT_ASSERT(m_pWin32Keyboard);
	
	m_pWin32Mouse = new EV_Win32Mouse(m_pEEM);
	UT_ASSERT(m_pWin32Mouse);

	// ... add other stuff (like tool bar) here...

	// TODO: Jeff, I'm currently showing in WinMain, to honor iCmdShow.
	// should we pass that via argv, to do it here for all frames?

	return UT_TRUE;
}

HWND AP_Win32Frame::getTopLevelWindow(void) const
{
	return m_hwnd;
}

EV_Win32Mouse * AP_Win32Frame::getWin32Mouse(void)
{
	return m_pWin32Mouse;
}

ev_Win32Keyboard * AP_Win32Frame::getWin32Keyboard(void)
{
	return m_pWin32Keyboard;
}

void AP_Win32Frame::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	// TODO get the default window size from preferences or something.
	m_hwnd = CreateWindow (m_pWin32Ap->getApplicationName(),	// window class name
				m_pWin32Ap->getApplicationTitleForTitleBar(),	// window caption
				WS_OVERLAPPEDWINDOW
				| WS_VSCROLL
				,					     // window style
				CW_USEDEFAULT,           // initial x position
				CW_USEDEFAULT,           // initial y position
				CW_USEDEFAULT,           // initial x size
				CW_USEDEFAULT,           // initial y size
				NULL,                    // parent window handle
				NULL,                    // window menu handle
				m_pWin32Ap->getInstance(),       // program instance handle
				NULL) ;		             // creation parameters

	UT_ASSERT(m_hwnd);

	// bind this frame to its window
	SWL(m_hwnd, this);

	// synthesize a menu from the info in our base class.
	m_pWin32Menu = new EV_Win32Menu(m_pWin32Ap,this);
	UT_ASSERT(m_pWin32Menu);
	UT_Bool bResult = m_pWin32Menu->synthesize();
	UT_ASSERT(bResult);

	// we let our caller decide when to show m_hwnd.

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

	HWND hwnd = m_hwnd;
	
	// TODO fix prefix on class Win32Graphics

	m_pG = new Win32Graphics(GetDC(hwnd), hwnd);
	UT_ASSERT(m_pG);
	m_pDocLayout = new FL_DocLayout(m_pDoc, m_pG);
	UT_ASSERT(m_pDocLayout);
  
	m_pDocLayout->formatAll();
			
	RECT r;
	GetClientRect(hwnd, &r);
	UT_uint32 iWindowHeight = r.bottom - r.top;

	UT_uint32 iHeight = m_pDocLayout->getHeight();
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iHeight;
	si.nPos = 0;
	si.nPage = iWindowHeight * 10 / 9;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

	// TODO we are overwriting m_pView,m_pScrollObj
	// TODO verify that if we do a new document or FileOpen
	// TODO on an existing window that we clean up the previous
	// TODO values of these.
	
	m_pView = new FV_View(m_pDocLayout);
	m_pScrollObj = new FV_ScrollObj(this,_scrollFunc);

	m_pView->addScrollListener(m_pScrollObj);
//	m_pMouse->reset();

	// enough HACKs to get a clean redisplay?
	m_pView->setWindowSize(r.right - r.left, iWindowHeight);
	InvalidateRect(hwnd, NULL, true);

	return UT_TRUE;
}

// TODO: extract/improve old fallback logic from here
#if OLD_VERSION
UT_Bool _showDoc(HWND hwnd, DG_Graphics *pG)
{
	if (!doc || !pG)
		return UT_FALSE;

	FL_DocLayout * pOldDL = pDocLayout;
	FV_View* pOldLV = pView;
  
	pDocLayout = new FL_DocLayout(doc, pG);
	UT_ASSERT(pDocLayout);

	if (!pDocLayout)
	{
		// blow away failed attempt
		if (doc)
			delete doc;

		// stick with prior state
		pDocLayout = pOldDL;
		doc = pDocLayout->getDocument();
		return UT_FALSE;
	}

	pDocLayout->formatAll();
			
	RECT r;
	GetClientRect(hwnd, &r);
	UT_uint32 iWindowHeight = r.bottom - r.top;

	UT_uint32 iHeight = pDocLayout->getHeight();
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iHeight;
	si.nPos = 0;
	si.nPage = iWindowHeight * 10 / 9;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

	pView = new FV_View(pDocLayout);

	if (!pView)
	{
		// blow away failed attempt
		if (pDocLayout)
			delete pDocLayout;

		// stick with prior state
		pDocLayout = pOldDL;
		pView = pOldLV;
		doc = pDocLayout->getDocument();
		return UT_FALSE;
	}

	if (pScrollObj)
		delete pScrollObj;

	pScrollObj = new FV_ScrollObj;

	pScrollObj->m_pData = NULL;
	pScrollObj->m_pfn = _scrollFunc;
	pView->addScrollListener(pScrollObj);
	pMouse->reset();

	// this is gonna work, so blow away prior state
	if (pOldDL)
		delete pOldDL;

	if (pOldLV)
		delete pOldLV;

	// enough HACKs to get a clean redisplay?
	pView->setWindowSize(r.right - r.left, iWindowHeight);
	InvalidateRect(hwnd, NULL, true);

	return UT_TRUE;
}
#endif /* OLD_VERSION */


void AP_Win32Frame::_scrollFunc(void* pData, UT_sint32 xoff, UT_sint32 yoff)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);

	HWND hwnd = pWin32Frame->m_hwnd;
		
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &si);

	si.nPos = yoff;

	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

	// TODO: move this logic back to shared code
	GetScrollInfo(hwnd, SB_VERT, &si);	// may have been clamped
	pWin32Frame->m_pView->setYScrollOffset(si.nPos);
}

#define SCROLL_LINE_SIZE 20

LRESULT CALLBACK AP_Win32Frame::_WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc ;
	PAINTSTRUCT ps ;

	AP_Win32Frame * f = GWL(hwnd);
	FV_View * pView = NULL;
	EV_Win32Mouse * pMouse = NULL;

	if (f)
	{
		pView = f->m_pView;
		pMouse = f->m_pWin32Mouse;
	}

	switch (iMsg)
	{
	case WM_CREATE :
	{
#if 0
		g_hwnd = hwnd;
		pG = new Win32Graphics(GetDC(hwnd), hwnd);
		pScrollObj = NULL;
		
		pEMC = AP_GetEditMethods();
		UT_Bool bResult = AP_LoadBindings("default",pEMC,&pEBM);
		pEEM = new EV_EditEventMapper(pEBM);
		pMenu = new dg_Win32Menu(pEMC);
		pMouse = new EV_Win32Mouse(pEEM);
		pKeyboard = new ev_Win32Keyboard(pEEM);

		if ((!*szFilename) || 
			(!_showFile(strdup(szFilename), hwnd, pG)))
		{
			// default to empty file if none given
			if (!_newFile(hwnd, pG))
				UT_ASSERT(UT_TODO);		// can't launch anything.  then what?
		}
#endif
	}
	return 0 ;

	case WM_VSCROLL:
	{
		int nScrollCode = (int) LOWORD(wParam); // scroll bar value 
		int nPos = (int) HIWORD(wParam);  // scroll box position 

		SCROLLINFO si;
		memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);

		switch (nScrollCode)
		{
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			break;
		case SB_LINEDOWN:
			si.nPos += SCROLL_LINE_SIZE;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			break;
		case SB_LINEUP:
			si.nPos -= SCROLL_LINE_SIZE;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = nPos;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			break;
		}

		if (nScrollCode != SB_ENDSCROLL)
			pView->setYScrollOffset(si.nPos);
	}
	return 0;

	case WM_COMMAND:
		if (f->m_pWin32Menu->onCommand(pView,hwnd,iMsg,wParam,lParam))
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
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);

		if (pView)
			pView->setWindowSize(nWidth, nHeight);
		return 0;
	}
		
	case WM_PAINT :
	{
		hdc = BeginPaint (hwnd, &ps) ;

		UT_DEBUGMSG(("Calling draw()\n"));
		
		pView->draw();
		
		EndPaint (hwnd, &ps) ;

		return 0 ;
	}

	case WM_DESTROY :
		PostQuitMessage (0) ;
		return 0 ;
	}

	return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}
