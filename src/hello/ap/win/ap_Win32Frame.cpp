/* AbiWord
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
#include <stdio.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_Win32Timer.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "xap_Win32Frame.h"
#include "ev_Win32Toolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "gr_Win32Graphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_Win32Frame.h"
#include "xap_Win32App.h"
#include "ev_Win32Mouse.h"
#include "ev_Win32Menu.h"
#include "ap_Win32StatusBar.h"
#include "ap_View.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Frame*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(AP_Win32Frame*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static char s_ContainerWndClassName[256];
static char s_DocumentWndClassName[256];

/*****************************************************************/

void AP_Win32Frame::_setVerticalScrollInfo(const SCROLLINFO * psi)
{
	// internal interface to GetScrollInfo() and SetScrollInfo()
	// to deal with 16-bit limitations of SB_THUMBTRACK.

	UT_uint32 scale, x;
	for (x=psi->nMax, scale=0; (x > 0x0000ffff); x>>=1, scale++)
		;

	m_vScale = scale;
	
	if (scale == 0)
	{
		SetScrollInfo(m_hwndVScroll,SB_CTL,psi,TRUE);
		return;
	}
	
	SCROLLINFO si = *psi;				// structure copy
	si.nMin >>= scale;
	si.nMax >>= scale;
	si.nPos >>= scale;
	si.nPage >>= scale;

	SetScrollInfo(m_hwndVScroll,SB_CTL,&si,TRUE);
	return;
}

void AP_Win32Frame::_getVerticalScrollInfo(SCROLLINFO * psi)
{
	GetScrollInfo(m_hwndVScroll,SB_CTL,psi);

	if (m_vScale)
	{
		psi->nMin <<= m_vScale;
		psi->nMax <<= m_vScale;
		psi->nPos <<= m_vScale;
		psi->nPage <<= m_vScale;
	}

	return;
}

/*****************************************************************/

void AP_Win32Frame::setXScrollRange(void)
{
}

void AP_Win32Frame::setYScrollRange(void)
{
}

bool AP_Win32Frame::RegisterClass(XAP_Win32App * app)
{
	// NB: can't access 'this' members from a static member function

	if (!XAP_Win32Frame::RegisterClass(app))
		return false;

	WNDCLASSEX  wndclass;
	ATOM a;

	// register class for the container window (this will contain the document
	// and the rulers and the scroll bars)
	
	sprintf(s_ContainerWndClassName, "%sContainer", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS | CS_OWNDC;
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

	a = RegisterClassEx(&wndclass);
	UT_ASSERT(a);
	
	// register class for the actual document window
	sprintf(s_DocumentWndClassName, "%sDocument", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS | CS_OWNDC;
	wndclass.lpfnWndProc   = AP_Win32Frame::_DocumentWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_DocumentWndClassName;
	wndclass.hIconSm       = NULL;

	a = RegisterClassEx(&wndclass);
	UT_ASSERT(a);
	
	if (!AP_Win32StatusBar::RegisterClass(app))
		return false;
	
	return true;
}

AP_Win32Frame::AP_Win32Frame(XAP_Win32App * app)
	: XAP_Win32Frame(app)
{
	m_hwndDeadLowerRight = NULL;
	m_hwndVScroll = NULL;
	m_hwndHScroll = NULL;
	m_hwndDocument = NULL;
	m_hwndStatusBar = NULL;

	m_pWin32StatusBar = NULL;
}

AP_Win32Frame::AP_Win32Frame(AP_Win32Frame * f)
	: XAP_Win32Frame(static_cast<XAP_Win32Frame *>(f))
{
	m_hwndDeadLowerRight = NULL;
	m_hwndVScroll = NULL;
	m_hwndHScroll = NULL;
	m_hwndDocument = NULL;
	m_hwndStatusBar = NULL;

	m_pWin32StatusBar = NULL;
}

AP_Win32Frame::~AP_Win32Frame(void)
{
	killFrameData();
	DELETEP(m_pWin32StatusBar);
}

bool AP_Win32Frame::initialize(void)
{
	if (!initFrameData())
		return false;

	if (!XAP_Win32Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
									AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
									AP_PREF_KEY_MenuLabelSet, AP_PREF_DEFAULT_MenuLabelSet,
									AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
									AP_PREF_KEY_ToolbarLabelSet, AP_PREF_DEFAULT_ToolbarLabelSet))
		return false;

	_createTopLevelWindow();

	loadDocument(NULL,0);

	return true;
}

XAP_Frame * AP_Win32Frame::cloneFrame(void)
{
	AP_Win32Frame * pClone = new AP_Win32Frame(this);
	ENSUREP(pClone);

	if (!pClone->initialize())
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

HWND AP_Win32Frame::_createDocumentWindow(HWND hwndParent,
										  UT_uint32 iLeft, UT_uint32 iTop,
										  UT_uint32 iWidth, UT_uint32 iHeight)
{
	// create the window(s) that the user will consider to be the
	// document window -- the thing between the bottom of the toolbars
	// and the top of the status bar.  return the handle to it.
		
	// create a child window for us -- this will be the 'container'.
	// the 'container' will in turn contain the document window, the
	// rulers, and the various scroll bars and other dead space.

	RECT r;
	int cxVScroll, cyHScroll;
	
	HWND hwndContainer = CreateWindowEx(WS_EX_CLIENTEDGE, s_ContainerWndClassName, NULL,
										WS_CHILD | WS_VISIBLE,
										iLeft, iTop, iWidth, iHeight,
										hwndParent, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(hwndContainer);
	SWL(hwndContainer, this);
	
	// now create all the various windows inside the container window.

	GetClientRect(hwndContainer,&r);
	cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

	m_hwndVScroll = CreateWindowEx(0,"ScrollBar",NULL,
								   WS_CHILD | WS_VISIBLE | SBS_VERT,
								   r.right-cxVScroll, 0, cxVScroll, r.bottom-cyHScroll,
								   hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndVScroll);
	SWL(m_hwndVScroll, this);

	m_hwndHScroll = CreateWindowEx(0,"ScrollBar",NULL,
								   WS_CHILD | WS_VISIBLE | SBS_HORZ,
								   0, r.bottom-cyHScroll, r.right-cxVScroll, cyHScroll,
								   hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndHScroll);
	SWL(m_hwndHScroll, this);

#if 1 // if the StatusBar is enabled, our lower-right corner is a dead spot
#  define XX_StyleBits		(WS_DISABLED | SBS_SIZEBOX)
#else // otherwise it is an active grip
#  define XX_StyleBits		(SBS_SIZEGRIP)
#endif

	m_hwndDeadLowerRight = CreateWindowEx(0,"ScrollBar",NULL,
										  WS_CHILD | WS_VISIBLE | XX_StyleBits,
										  r.right-cxVScroll, r.bottom-cyHScroll, cxVScroll, cyHScroll,
										  hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndDeadLowerRight);
	SWL(m_hwndDeadLowerRight, this);

	// create a child window for us.
	m_hwndDocument = CreateWindowEx(0, s_DocumentWndClassName, NULL,
									WS_CHILD | WS_VISIBLE,
									0, 0,
									r.right - cxVScroll,
									r.bottom - cyHScroll,
									hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndDocument);
	SWL(m_hwndDocument, this);

	return hwndContainer;
}

bool AP_Win32Frame::loadDocument(const char * szFilename, int ieft)
{
	GR_Win32Graphics * pG = new GR_Win32Graphics(GetDC(m_hwndDocument), m_hwndDocument, getApp());
	pG->setFont(pG->findFont("Times New Roman", "normal", NULL, "bold", NULL, "72pt"));
	pG->setZoomPercentage(100);

	REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);

	AP_View* pView = new AP_View(m_app, pG, this);
	REPLACEP(m_pView, pView);

	if (m_pWin32StatusBar)
		m_pWin32StatusBar->setView(pView);

	return true;
}
	
void AP_Win32Frame::_scrollFuncY(void* pData, UT_sint32 yoff, UT_sint32 /*ylimit*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.

	// scroll event came in (probably from an EditMethod (like a PageDown
	// or insertData or something).  update the on-screen scrollbar and
	// then warp the document window contents.
	
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	pWin32Frame->_getVerticalScrollInfo(&si);
	si.nPos = yoff;
	pWin32Frame->_setVerticalScrollInfo(&si);
	pWin32Frame->_getVerticalScrollInfo(&si); // values may have been clamped
	pWin32Frame->m_pView->setYScrollOffset(si.nPos);
}

void AP_Win32Frame::_scrollFuncX(void* pData, UT_sint32 xoff, UT_sint32 /*xlimit*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.

	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	HWND hwndH = pWin32Frame->m_hwndHScroll;
	GetScrollInfo(hwndH, SB_CTL, &si);

	si.nPos = xoff;
	SetScrollInfo(hwndH, SB_CTL, &si, TRUE);

	GetScrollInfo(hwndH, SB_CTL, &si);	// may have been clamped
	pWin32Frame->m_pView->setXScrollOffset(si.nPos);
}

/*****************************************************************/

#define SCROLL_LINE_SIZE 20

LRESULT CALLBACK AP_Win32Frame::_ContainerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Frame * f = GWL(hwnd);
	AV_View * pView = NULL;

	if (f)
		pView = f->m_pView;

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
			MoveWindow(f->m_hwndDocument, 0, 0,
					   nWidth - cxVScroll, nHeight - cyHScroll, TRUE);
		}
		return 0;
	}

	case WM_VSCROLL:
	{
		int nScrollCode = (int) LOWORD(wParam); // scroll bar value 

		SCROLLINFO si;
		memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		f->_getVerticalScrollInfo(&si);

		switch (nScrollCode)
		{
		default:
			return 0;
			
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			if (si.nPos < 0)
				si.nPos = 0;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_LINEDOWN:
			si.nPos += SCROLL_LINE_SIZE;
			break;
		case SB_LINEUP:
			si.nPos -= SCROLL_LINE_SIZE;
			if (si.nPos < 0)
				si.nPos = 0;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = (int)HIWORD(wParam); // dynamic scroll box position -- a 16-bit value
			si.nPos <<= f->m_vScale;
			break;
		}

		f->_setVerticalScrollInfo(&si);				// notify window of new value.
		f->_getVerticalScrollInfo(&si);				// update from window, in case we got clamped
		pView->sendVerticalScrollEvent(si.nPos);	// now tell the view

		return 0;
	}

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
			pView->sendHorizontalScrollEvent(si.nPos);
		}

		return 0;
	}

	case WM_SYSCOLORCHANGE:
	{
		if (f)
		{
			SendMessage(f->m_hwndDocument,WM_SYSCOLORCHANGE,0,0);
		}
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
	
	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
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

	case WM_SETCURSOR:
		return 1;

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

			// may need to scroll to keep everything in sync.
			// the following is necessary to make sure that the
			// window de-scrolls as it gets larger.
			
			SCROLLINFO si;
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;

			f->_getVerticalScrollInfo(&si);
			pView->sendVerticalScrollEvent(si.nPos,si.nMax-si.nPage);

			GetScrollInfo(f->m_hwndHScroll, SB_CTL, &si);
			pView->sendHorizontalScrollEvent(si.nPos,si.nMax-si.nPage);
		}
		return 0;
	}

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		UT_Rect r(ps.rcPaint.left,ps.rcPaint.top,
				  ps.rcPaint.right-ps.rcPaint.left,
				  ps.rcPaint.bottom-ps.rcPaint.top);
		pView->draw(&r);
		EndPaint(hwnd, &ps);

		return 0;
	}

	case WM_TIMER:
	{
		TIMERPROC * pfn = (TIMERPROC *)lParam;
		UT_ASSERT( (void *)(pfn) == (void *)(Global_Win32TimerProc) );
		Global_Win32TimerProc(hwnd,WM_TIMER,(UINT)wParam,NULL);
		return 0;
	}
	
	case WM_DESTROY:
		return 0;
	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

/*****************************************************************/

bool AP_Win32Frame::initFrameData(void)
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	AP_FrameData* pData = new AP_FrameData();
	m_pData = (void*) pData;
	
	return (pData ? true : false);
}

void AP_Win32Frame::killFrameData(void)
{
	AP_FrameData * pData = (AP_FrameData *) m_pData;
	DELETEP(pData);
	m_pData = NULL;
}

void AP_Win32Frame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	// translate the given document mouse coordinates into absolute screen coordinates.

	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen(m_hwndDocument,&pt);
	x = pt.x;
	y = pt.y;
}

HWND AP_Win32Frame::_createStatusBarWindow(HWND hwndParent,
										   UT_uint32 iLeft, UT_uint32 iTop,
										   UT_uint32 iWidth)
{
	m_pWin32StatusBar = new AP_Win32StatusBar(this);
	UT_ASSERT(m_pWin32StatusBar);
	m_hwndStatusBar = m_pWin32StatusBar->createWindow(hwndParent,iLeft,iTop,iWidth);

	return m_hwndStatusBar;
}

void AP_Win32Frame::setStatusMessage(const char * szMsg)
{
	m_pWin32StatusBar->setStatusMessage(szMsg);
}
