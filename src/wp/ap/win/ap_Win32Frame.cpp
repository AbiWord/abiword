/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_Win32Graphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_Win32Frame.h"
#include "xap_Win32App.h"
#include "ev_Win32Mouse.h"
#include "ev_Win32Keyboard.h"
#include "ap_Win32TopRuler.h"
#include "ap_Win32LeftRuler.h"
#include "ev_Win32Menu.h"
#include "ap_Win32StatusBar.h"
#include "ie_types.h"

#include <winuser.h>
#include <zmouse.h>

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

#ifdef _MSC_VER
// fix MSVC <= 6 (i.e. upto V.12+) bug
#define for if (0) {} else for
#endif

int GetMouseWheelLines()
{
 	OSVERSIONINFO Info = { 0 };

 	Info.dwOSVersionInfoSize = sizeof(Info);

 	if (GetVersionEx(&Info) &&
 		Info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
 		Info.dwMajorVersion == 4 &&
 		Info.dwMinorVersion == 0)
 	{
 		// Win95
 		UINT msgMSHWheelGetScrollLines = RegisterWindowMessage(MSH_SCROLL_LINES);
 		HWND hdlMSHWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
 		if (hdlMSHWheel && msgMSHWheelGetScrollLines)
 		{
 			return SendMessage(hdlMSHWheel, msgMSHWheelGetScrollLines, 0, 0);
 		}
 	}
 	else
 	{
 		// Win98, NT, 2K
 		UINT nScrollLines;
 		if (SystemParametersInfo(	SPI_GETWHEELSCROLLLINES,
 									0,
 									(PVOID) &nScrollLines,
 									0))
 		{
 			return nScrollLines;
 		}
 	}

 	return 3;
}

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Frame*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(AP_Win32Frame*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

static char s_ContainerWndClassName[256];
static char s_DocumentWndClassName[256];
static char s_LeftRulerWndClassName[256];

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
		SetScrollInfo(m_hWndVScroll, SB_CTL, psi, TRUE);
		return;
	}

	SCROLLINFO si = *psi;				// structure copy
	si.nMin >>= scale;
	si.nMax >>= scale;
	si.nPos >>= scale;
	si.nPage >>= scale;

	SetScrollInfo(m_hWndVScroll, SB_CTL, &si, TRUE);
	return;
}

void AP_Win32Frame::_getVerticalScrollInfo(SCROLLINFO * psi)
{
	GetScrollInfo(m_hWndVScroll, SB_CTL, psi);

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

void AP_Win32Frame::setZoomPercentage(UT_uint32 iZoom)
{
	_showDocument(iZoom);
}

UT_uint32 AP_Win32Frame::getZoomPercentage(void)
{
	return static_cast<AP_FrameData*>(m_pData)->m_pG->getZoomPercentage();
}

UT_Error AP_Win32Frame::_showDocument(UT_uint32 iZoom)
{
	if (!m_pDoc)
	{
		UT_DEBUGMSG(("Can't show a non-existent document\n"));
		return UT_IE_FILENOTFOUND;
	}

	if (!m_pData)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_IE_IMPORTERROR;
	}

	GR_Win32Graphics*			pG						= 0;
	FL_DocLayout*				pDocLayout				= 0;
	AV_View*					pView					= 0;
	AV_ScrollObj*				pScrollObj				= 0;
	ap_ViewListener*			pViewListener			= 0;
	AD_Document*				pOldDoc					= 0;
	ap_Scrollbar_ViewListener*	pScrollbarViewListener	= 0;
	AV_ListenerId				lid;
	AV_ListenerId				lidScrollbarViewListener;
	UT_uint32					point					= 0;
	HWND hwnd = m_hwndDocument;

	pG = new GR_Win32Graphics(GetDC(hwnd), hwnd, getApp());
	ENSUREP(pG);

	pG->setZoomPercentage(iZoom);

	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP(pDocLayout);

//	pDocLayout->formatAll();

	pView = new FV_View(getApp(), this, pDocLayout);
	ENSUREP(pView);

//	pDocLayout->fillLayouts();


	// The "AV_ScrollObj pScrollObj" receives
	// send{Vertical,Horizontal}ScrollEvents
	// from both the scroll-related edit methods
	// and from the UI callbacks.
	//
	// The "ap_ViewListener pViewListener" receives
	// change notifications as the document changes.
	// This ViewListener is responsible for keeping
	// the title-bar up to date (primarily title
	// changes, dirty indicator, and window number).
	//
	// The "ap_Scrollbar_ViewListener pScrollbarViewListener"
	// receives change notifications as the doucment changes.
	// This ViewListener is responsible for recalibrating the
	// scrollbars as pages are added/removed from the document.
	//
	// Each Toolbar will also get a ViewListener so that
	// it can update toggle buttons, and other state-indicating
	// controls on it.
	//
	// TODO we ***really*** need to re-do the whole scrollbar thing.
	// TODO we have an addScrollListener() using an m_pScrollObj
	// TODO and a View-Listener, and a bunch of other widget stuff.
	// TODO and its very confusing.

	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	ENSUREP(pScrollObj);
	pViewListener = new ap_ViewListener(this);
	ENSUREP(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP(pScrollbarViewListener);

	if (!pView->addListener(pViewListener,&lid) ||
		!pView->addListener(pScrollbarViewListener, &lidScrollbarViewListener))
	{
		goto Cleanup;
	}

	{
		const UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
		for (UT_uint32 k = 0; k < nrToolbars; ++k)
		{
			// TODO Toolbars are a frame-level item, but a view-listener is
			// TODO a view-level item.  I've bound the toolbar-view-listeners
			// TODO to the current view within this frame and have code in the
			// TODO toolbar to allow the view-listener to be rebound to a different
			// TODO view.  in the future, when we have support for multiple views
			// TODO in the frame (think splitter windows), we will need to have
			// TODO a loop like this to help change the focus when the current
			// TODO view changes.

			EV_Win32Toolbar* pWin32Toolbar = (EV_Win32Toolbar *)m_vecToolbars.getNthItem(k);
			pWin32Toolbar->bindListenerToView(pView);
		}
	}

	/****************************************************************
	*****************************************************************
	** If we reach this point, everything for the new document has
	** been created.  We can now safely replace the various fields
	** within the structure.  Nothing below this point should fail.
	*****************************************************************
	****************************************************************/

	// switch to new view, cleaning up previous settings
	if (static_cast<AP_FrameData*>(m_pData)->m_pDocLayout)
	{
		pOldDoc = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getDocument();
	}

	REPLACEP(static_cast<AP_FrameData*>(m_pData)->m_pG, pG);
	REPLACEP(static_cast<AP_FrameData*>(m_pData)->m_pDocLayout, pDocLayout);
	if (pOldDoc != m_pDoc)
	{
		UNREFP(pOldDoc);
	}

	{
		const AV_Focus prevFocus = m_pView ? m_pView->getFocus() : AV_FOCUS_NONE;
		REPLACEP(m_pView, pView);
		m_pView->setFocus(prevFocus);
	}

	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener,pScrollbarViewListener);
	m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets
	// we do not destroy and recreate the TopRuler,LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler here; that is done as the frame is
	// created.
	if (static_cast<AP_FrameData*>(m_pData)->m_pTopRuler)
		static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->setView(pView, iZoom);
	if (static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler)
		static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->setView(pView, iZoom);
	static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->setView(pView);

	pView->setInsertMode(static_cast<AP_FrameData*>(m_pData)->m_bInsertMode);
    ((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);

	RECT r;
	GetClientRect(hwnd, &r);
	m_pView->setWindowSize(r.right - r.left, r.bottom - r.top);
	InvalidateRect(hwnd, NULL, TRUE);

	updateTitle();

	pDocLayout->fillLayouts();

	// we cannot do this before we fill the layout, because if we are
	// running in default RTL mode, the X scroll needs to be able to
	// have a valid block for its calculations
	setXScrollRange();
	setYScrollRange();

	if (m_pView != NULL)
	{
		// we cannot just set the insertion position to that of the previous
		// view, since the new document could be shorter or completely
		// different from the previous one (see bug 2615)
		// Instead we have to test that the original position is within
		// the editable bounds, and if not, we will set the point
		// to the end of the document (i.e., if reloading an earlier
		// version of the same document we try to get the point as near
		// the users editing position as possible
		point = ((FV_View *) m_pView)->getPoint();
		PT_DocPosition posEOD;
		static_cast<FV_View *>(pView)->getEditableBounds(true, posEOD, false);
		if(point > posEOD)
			point = posEOD;
	}
	if (point != 0)
		((FV_View *) m_pView)->moveInsPtTo(point);
	m_pView->draw();

	if (static_cast<AP_FrameData*>(m_pData)->m_pTopRuler)
		static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->draw(NULL);
	if (static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler)
		static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->draw(NULL);
	static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->draw();

	return UT_OK;

Cleanup:
	// clean up anything we created here
	DELETEP(pG);
	DELETEP(pDocLayout);
	DELETEP(pView);
	DELETEP(pViewListener);
	DELETEP(pScrollObj);
	DELETEP(pScrollbarViewListener);

	// change back to prior document
	UNREFP(m_pDoc);
	m_pDoc = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getDocument();

	return UT_IE_ADDLISTENERERROR;
}

void AP_Win32Frame::setXScrollRange(void)
{
	RECT r;
	GetClientRect(m_hwndDocument, &r);
	const UT_uint32 iWindowWidth = r.right - r.left;
	const UT_uint32 iWidth = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getWidth();

	SCROLLINFO si = { 0 };

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = iWidth;
	si.nPos = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	si.nPage = iWindowWidth;
	SetScrollInfo(m_hWndHScroll, SB_CTL, &si, TRUE);

	m_pView->sendHorizontalScrollEvent(si.nPos,si.nMax-si.nPage);
}

void AP_Win32Frame::setYScrollRange(void)
{
	RECT r;
	GetClientRect(m_hwndDocument, &r);
	const UT_uint32 iWindowHeight = r.bottom - r.top;
	const UT_uint32 iHeight = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getHeight();

	SCROLLINFO si = { 0 };

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = iHeight;
	si.nPos = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	si.nPage = iWindowHeight;
	_setVerticalScrollInfo(&si);

	m_pView->sendVerticalScrollEvent(si.nPos,si.nMax-si.nPage);
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

	if (!AP_Win32TopRuler::RegisterClass(app) ||
		!AP_Win32LeftRuler::RegisterClass(app) ||
		!AP_Win32StatusBar::RegisterClass(app))
	{
		return false;
	}

	return true;
}

AP_Win32Frame::AP_Win32Frame(XAP_Win32App * app)
:	XAP_Win32Frame(app),
	m_bMouseWheelTrack(false),
	m_bMouseActivateReceived(false),
	m_hWndHScroll(0),
	m_hWndVScroll(0),
	m_hWndGripperHack(0)
{
	m_hwndContainer = NULL;
	m_hwndTopRuler = NULL;
	m_hwndLeftRuler = NULL;
	m_hwndDocument = NULL;
	m_hwndStatusBar = NULL;
}

AP_Win32Frame::AP_Win32Frame(AP_Win32Frame * f)
:	XAP_Win32Frame(static_cast<XAP_Win32Frame *>(f)),
	m_bMouseWheelTrack(false),
	m_bMouseActivateReceived(false),
	m_hWndHScroll(0),
	m_hWndVScroll(0),
	m_bFirstAfterFocus(false),
	m_hWndGripperHack(0)
{
	m_hwndContainer = NULL;
	m_hwndTopRuler = NULL;
	m_hwndLeftRuler = NULL;
	m_hwndDocument = NULL;
	m_hwndStatusBar = NULL;
}

AP_Win32Frame::~AP_Win32Frame(void)
{
	killFrameData();
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
	_showOrHideToolbars();
	_showOrHideStatusbar();
	return true;
}

XAP_Frame* AP_Win32Frame::cloneFrame(void)
{
	AP_Win32Frame* pClone = new AP_Win32Frame(this);

	UT_ASSERT(pClone);
	return pClone;
}
XAP_Frame * AP_Win32Frame::buildFrame(XAP_Frame * pF)
{
	AP_Win32Frame * pClone = static_cast<AP_Win32Frame *>(pF);
	if (pClone && pClone->initialize())
	{
		const UT_Error error = pClone->_showDocument();
		if (!error)
		{
			pClone->show();
			return pClone;
		}
	}

	// clean up anything we created here
	if (pClone)
	{
		m_pWin32App->forgetFrame(pClone);
		delete pClone;
	}

	return 0;
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
	// rulers, and the variousscroll bars and other dead space.

	m_hwndContainer = CreateWindowEx(WS_EX_CLIENTEDGE, s_ContainerWndClassName, NULL,
									 WS_CHILD | WS_VISIBLE,
									 iLeft, iTop, iWidth, iHeight,
									 hwndParent, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndContainer);
	SWL(m_hwndContainer, this);

	// now create all the various windows inside the container window.

	RECT r;
	GetClientRect(m_hwndContainer,&r);
	const int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	const int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

	m_hWndHScroll = CreateWindowEx(0, "ScrollBar", 0, WS_CHILD | WS_VISIBLE | SBS_HORZ,
									0, r.bottom - cyHScroll,
									r.right - cxVScroll, cyHScroll,
									m_hwndContainer,
									0, m_pWin32App->getInstance(), 0);
	UT_ASSERT(m_hWndHScroll);
	SWL(m_hWndHScroll, this);

	m_hWndVScroll = CreateWindowEx(0, "ScrollBar", 0, WS_CHILD | WS_VISIBLE | SBS_VERT,
									r.right - cxVScroll, 0,
									cxVScroll, r.bottom - cyHScroll,
									m_hwndContainer,
									0, m_pWin32App->getInstance(), 0);
	UT_ASSERT(m_hWndVScroll);
	SWL(m_hWndVScroll, this);

#if 1 // if the StatusBar is enabled, our lower-right corner is a dead spot
#  define XX_StyleBits          (WS_DISABLED | SBS_SIZEBOX)
#else
#  define XX_StyleBits          (SBS_SIZEGRIP)
#endif

	m_hWndGripperHack = CreateWindowEx(0,"ScrollBar", 0,
										WS_CHILD | WS_VISIBLE | XX_StyleBits,
										r.right-cxVScroll, r.bottom-cyHScroll, cxVScroll, cyHScroll,
										m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hWndGripperHack);
	SWL(m_hWndGripperHack, this);

	// create the rulers, if needed

	int yTopRulerHeight = 0;
	int xLeftRulerWidth = 0;

	if (static_cast<AP_FrameData*>(m_pData)->m_bShowRuler)
	{
		_createRulers();
		_getRulerSizes(yTopRulerHeight, xLeftRulerWidth);
	}

	// create a child window for us.
	m_hwndDocument = CreateWindowEx(0, s_DocumentWndClassName, NULL,
									WS_CHILD | WS_VISIBLE,
									xLeftRulerWidth, yTopRulerHeight,
									r.right - xLeftRulerWidth - cxVScroll,
									r.bottom - yTopRulerHeight - cyHScroll,
									m_hwndContainer, NULL, m_pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndDocument);
	SWL(m_hwndDocument, this);

	return m_hwndContainer;
}

void AP_Win32Frame::_getRulerSizes(int &yTopRulerHeight, int &xLeftRulerWidth)
{
	if (static_cast<AP_FrameData*>(m_pData)->m_pTopRuler)
		yTopRulerHeight = static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->getHeight();
	else
		yTopRulerHeight = 0;

	if (static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler)
		xLeftRulerWidth = static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->getWidth();
	else
		xLeftRulerWidth = 0;
}

void AP_Win32Frame::_createRulers(void)
{
	_createTopRuler();
	_createLeftRuler();
}

void AP_Win32Frame::_createTopRuler(void)
{
	RECT r;
	int cxVScroll, cyHScroll;

	GetClientRect(m_hwndContainer,&r);
	cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

	// create the top ruler
	AP_Win32TopRuler * pWin32TopRuler = new AP_Win32TopRuler(this);
	UT_ASSERT(pWin32TopRuler);
	m_hwndTopRuler = pWin32TopRuler->createWindow(m_hwndContainer,
												  0,0, (r.right - cxVScroll));
	static_cast<AP_FrameData*>(m_pData)->m_pTopRuler = pWin32TopRuler;


	// get the width from the left ruler and stuff it into the top ruler.
	UT_uint32 xLeftRulerWidth = 0;
    if( m_hwndLeftRuler )
	{
		AP_Win32LeftRuler * pWin32LeftRuler = NULL;
		pWin32LeftRuler =  (AP_Win32LeftRuler *) static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler;
		xLeftRulerWidth = pWin32LeftRuler->getWidth();
	}
	pWin32TopRuler->setOffsetLeftRuler(xLeftRulerWidth);
}

void AP_Win32Frame::_createLeftRuler(void)
{
	RECT r;
	int cxVScroll, cyHScroll;

	GetClientRect(m_hwndContainer,&r);
	cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

	UT_uint32 yTopRulerHeight = 0;

	if( m_hwndTopRuler )
	{
		AP_Win32TopRuler * pWin32TopRuler = NULL;
		pWin32TopRuler =  (AP_Win32TopRuler * ) static_cast<AP_FrameData*>(m_pData)->m_pTopRuler;
		yTopRulerHeight = pWin32TopRuler->getHeight();
	}

	// create the left ruler
	AP_Win32LeftRuler * pWin32LeftRuler = new AP_Win32LeftRuler(this);
	UT_ASSERT(pWin32LeftRuler);
	m_hwndLeftRuler = pWin32LeftRuler->createWindow(m_hwndContainer,0,yTopRulerHeight,
													r.bottom - yTopRulerHeight - cyHScroll);
	static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler = pWin32LeftRuler;

	// get the width from the left ruler and stuff it into the top ruler.
    if( m_hwndTopRuler )
	{
		UT_uint32 xLeftRulerWidth = pWin32LeftRuler->getWidth();
		AP_Win32TopRuler * pWin32TopRuler = NULL;
		pWin32TopRuler =  (AP_Win32TopRuler * ) static_cast<AP_FrameData*>(m_pData)->m_pTopRuler;
		pWin32TopRuler->setOffsetLeftRuler(xLeftRulerWidth);
	}
}

UT_Error AP_Win32Frame::loadDocument(const char * szFilename, int ieft, bool createNew)
{
  UT_ASSERT(UT_TODO);
  return UT_OK;
}

UT_Error AP_Win32Frame::loadDocument(const char * szFilename, int ieft)
{
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	const bool bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}

	UT_Error err;
	err = _loadDocument(szFilename, (IEFileType) ieft);
	if (err)
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return err;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_Win32Frame * pFrame = (AP_Win32Frame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

UT_Error AP_Win32Frame::_importDocument(const char * szFilename, int ieft,
									  bool markClean)
{
	UT_DEBUGMSG(("DOM: trying to import %s (%d, %d)\n", szFilename, ieft, markClean));

	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it,
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);

	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->importFile(szFilename, ieft, markClean);
	if (!errorCode)
		goto ReplaceDocument;

	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	m_iUntitled = _getNextUntitledNumber();

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}

UT_Error AP_Win32Frame::importDocument(const char * szFilename, int ieft,
									  bool markClean)
{
	bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	UT_Error errorCode;
	errorCode =  _importDocument(szFilename, (IEFileType) ieft, markClean);
	if (errorCode)
	{
		return errorCode;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_Win32Frame * pFrame = (AP_Win32Frame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

void AP_Win32Frame::_scrollFuncY(void* pData, UT_sint32 yoff, UT_sint32 /*ylimit*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.

	// scroll event came in (probably from an EditMethod (like a PageDown
	// or insertData or something).  update the on-screen scrollbar and
	// then warp the document window contents.

	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pData);
	UT_ASSERT(pWin32Frame);

	SCROLLINFO si = { 0 };
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

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	HWND hwndH = pWin32Frame->m_hWndHScroll;
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

	if (!f)
	{
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	AV_View* pView = f->m_pView;

	switch (iMsg)
	{
	case WM_SETFOCUS:
	{
		SetFocus(f->m_hwndDocument);
		return 0;
	}

	case WM_SIZE:
	{
		const int nWidth = LOWORD(lParam);
		const int nHeight = HIWORD(lParam);

		f->_onSize(nWidth, nHeight);
		return 0;
	}

	case WM_VSCROLL:
	{
		int nScrollCode = (int) LOWORD(wParam); // scroll bar value

		SCROLLINFO si = { 0 };

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

		SCROLLINFO si = { 0 };

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(f->m_hWndHScroll, SB_CTL, &si);

		switch (nScrollCode)
		{
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(f->m_hWndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			SetScrollInfo(f->m_hWndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_LINEDOWN:
			si.nPos += SCROLL_LINE_SIZE;
			SetScrollInfo(f->m_hWndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_LINEUP:
			si.nPos -= SCROLL_LINE_SIZE;
			if (si.nPos < 0)
			{
				si.nPos = 0;
			}
			SetScrollInfo(f->m_hWndHScroll, SB_CTL, &si, TRUE);
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos = nPos;
			SetScrollInfo(f->m_hWndHScroll, SB_CTL, &si, TRUE);
			break;
		}

		if (nScrollCode != SB_ENDSCROLL)
		{
			// in case we got clamped
			GetScrollInfo(f->m_hWndHScroll, SB_CTL, &si);

			// now tell the view
			pView->sendHorizontalScrollEvent(si.nPos);
		}

		return 0;
	}

	case WM_SYSCOLORCHANGE:
	{
		SendMessage(f->m_hwndTopRuler,WM_SYSCOLORCHANGE,0,0);
		SendMessage(f->m_hwndLeftRuler,WM_SYSCOLORCHANGE,0,0);
		SendMessage(f->m_hwndDocument,WM_SYSCOLORCHANGE,0,0);
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

 	case WM_MOUSEWHEEL:
 	{
 		// Get delta
 		const int iDelta = (short) HIWORD(wParam);

 		// Calculate the movement offset to an integer resolution
 		const int iMove = (iDelta * GetMouseWheelLines()) / WHEEL_DELTA;

 		// Get current scroll position
 		SCROLLINFO si = { 0 };

 		si.cbSize = sizeof(si);
 		si.fMask = SIF_ALL;
 		f->_getVerticalScrollInfo(&si);

 		// Clip new position to limits
 		int iNewPos = si.nPos - (iMove * SCROLL_LINE_SIZE);
 		if (iNewPos > si.nMax) iNewPos = si.nMax;
 		if (iNewPos < si.nMin) iNewPos = si.nMin;

 		if (iNewPos != si.nPos)
 		{
 			// If position has changed set new position
 			SendMessage(hwnd,
 						WM_VSCROLL,
 						MAKELONG(SB_THUMBPOSITION, iNewPos),
 						NULL);
 		}
	}
	return 0;

	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
}

void AP_Win32Frame::_onSize(int nWidth, int nHeight)
{
	const int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	const int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	int yTopRulerHeight = 0;
	int xLeftRulerWidth = 0;

	_getRulerSizes(yTopRulerHeight, xLeftRulerWidth);

	MoveWindow(m_hWndVScroll, nWidth-cxVScroll, 0, cxVScroll, nHeight-cyHScroll, TRUE);
	MoveWindow(m_hWndHScroll, 0, nHeight-cyHScroll, nWidth - cxVScroll, cyHScroll, TRUE);
	MoveWindow(m_hWndGripperHack, nWidth-cxVScroll, nHeight-cyHScroll, cxVScroll, cyHScroll, TRUE);


	if (m_hwndTopRuler)
	{
		MoveWindow(m_hwndTopRuler, 0, 0, nWidth - cxVScroll, yTopRulerHeight, TRUE);
		InvalidateRect(m_hwndTopRuler, NULL, TRUE);
	}

	if (m_hwndLeftRuler)
		MoveWindow(m_hwndLeftRuler, 0, yTopRulerHeight,
				   xLeftRulerWidth, nHeight - yTopRulerHeight - cyHScroll, TRUE);

	MoveWindow(m_hwndDocument, xLeftRulerWidth, yTopRulerHeight,
			   nWidth - xLeftRulerWidth - cxVScroll,
			   nHeight - yTopRulerHeight - cyHScroll, TRUE);
}



LRESULT CALLBACK AP_Win32Frame::_DocumentWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Frame * f = GWL(hwnd);

	if (!f)
	{
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	AV_View*		pView = f->m_pView;
	EV_Win32Mouse*	pMouse = (EV_Win32Mouse *) f->m_pMouse;

	switch (iMsg)
	{
	case WM_MOUSEACTIVATE:
		f->m_bMouseActivateReceived = true;
		return MA_ACTIVATE;

	case WM_SETFOCUS:			
		if (pView)
		{
			pView->focusChange(AV_FOCUS_HERE);
			
			if (GetKeyState(VK_LBUTTON)>0)
				f->m_bFirstAfterFocus = true;

			if(!f->m_bMouseActivateReceived)
			{
				
				
				// HACK:	Capture leaving a tool bar combo.
				// We need to get a mouse down signal.
				// windows is not activated so send a mouse down if the mouse is pressed.

				UT_DEBUGMSG(("%s(%d): Need to set mouse down\n", __FILE__, __LINE__));

//				GetKeyState
			}
		}
		return 0;

	case WM_KILLFOCUS:
		if (pView)
		{
			pView->focusChange(AV_FOCUS_NONE);			
		}
		return 0;

	case WM_CREATE:
		return 0;

	case WM_SETCURSOR:
		{
			FV_View * pFV = static_cast<FV_View *>(pView);
			GR_Graphics * pG = pFV->getGraphics();
			GR_Win32Graphics * pGWin32 = static_cast<GR_Win32Graphics *>(pG);
			pGWin32->handleSetCursorMessage();
		}
		return 1;

	case WM_LBUTTONDOWN:
		
		/* When we get the focus back 					*/
		if (f->m_bFirstAfterFocus)
			f->m_bFirstAfterFocus = false;	
		else
		{
			
			if(GetFocus() != hwnd)
			{
				SetFocus(hwnd);
				pView = f->m_pView;
				pMouse = (EV_Win32Mouse *) f->m_pMouse;
			}
			pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
			f->m_bMouseActivateReceived = false;
		}
		return 0;
	case WM_MBUTTONDOWN:
		f->_startTracking(LOWORD(lParam), HIWORD(lParam));
//		pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON2,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_RBUTTONDOWN:
		pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON3,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case WM_LBUTTONDBLCLK:
		pMouse->onDoubleClick(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_MBUTTONDBLCLK:
//		pMouse->onDoubleClick(pView,hwnd,EV_EMB_BUTTON2,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_RBUTTONDBLCLK:
		pMouse->onDoubleClick(pView,hwnd,EV_EMB_BUTTON3,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		UT_DEBUGMSG(("WM_KEYDOWN %d  - %d\n",wParam, lParam));
	    ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(f->m_pKeyboard);	    
	    
     	if (pWin32Keyboard->onKeyDown(pView,hwnd,iMsg,wParam,lParam))
     		return DefWindowProc(hwnd,iMsg,wParam,lParam);
 		else
 			return false;	    				 	     			
    		
	}
	case WM_SYSCHAR:
	case WM_CHAR:
	{
		UT_DEBUGMSG(("WM_CHAR %d  - %d\n",wParam, lParam));
	    ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(f->m_pKeyboard);
	   
	    
		pWin32Keyboard->onChar(pView,hwnd,iMsg,wParam,lParam);		
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
	}
	case WM_IME_CHAR:
	{
		ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(f->m_pKeyboard);
		if (pWin32Keyboard->onIMEChar(pView,hwnd,iMsg,wParam,lParam))
			return 0;
		return DefWindowProc(hwnd,iMsg,wParam,lParam);
	}

	case WM_MOUSEMOVE:
		if (f->_isTracking())
		{
			f->_track(LOWORD(lParam),HIWORD(lParam));
		}
		else
		{
			pMouse->onButtonMove(pView,hwnd,wParam,LOWORD(lParam),HIWORD(lParam));
		}
		return 0;

	case WM_LBUTTONUP:
		if(GetFocus() != hwnd)	// HACK: get focus back from tool bar combos.
		{
			SetFocus(hwnd);
			pView = f->m_pView;
			pMouse = (EV_Win32Mouse *) f->m_pMouse;
			pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
		}
		pMouse->onButtonUp(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;
	case WM_MBUTTONUP:
		f->_endTracking(LOWORD(lParam), HIWORD(lParam));
//		pMouse->onButtonUp(pView,hwnd,EV_EMB_BUTTON2,wParam,LOWORD(lParam),HIWORD(lParam));
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

			SCROLLINFO si = { 0 };
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;

			f->_getVerticalScrollInfo(&si);
			pView->sendVerticalScrollEvent(si.nPos,si.nMax-si.nPage);

			GetScrollInfo(f->m_hWndHScroll, SB_CTL, &si);
			pView->sendHorizontalScrollEvent(si.nPos,si.nMax-si.nPage);
		}
		return 0;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

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

	case WM_INPUTLANGCHANGE:	// let the XAP_Win32Frame handle this
		return ::SendMessage(f->m_hwndFrame, WM_INPUTLANGCHANGE, wParam, lParam);

	} /* switch (iMsg) */

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

/*****************************************************************/

bool AP_Win32Frame::initFrameData(void)
{
	UT_ASSERT(!m_pData);

	AP_FrameData* pData = new AP_FrameData(m_pWin32App);
	m_pData = (void*) pData;

	return pData != 0;
}

void AP_Win32Frame::killFrameData(void)
{
	AP_FrameData * pData = static_cast<AP_FrameData*>(m_pData);
	DELETEP(pData);
	m_pData = NULL;
}

UT_Error AP_Win32Frame::_loadDocument(const char * szFilename, IEFileType ieft)
{
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it,
		// TODO: query user if document is changed...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);

	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
	}
	else
	{
		const UT_Error err = pNewDoc->readFromFile(szFilename, ieft);
		if (err)
		{
			UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
			UNREFP(pNewDoc);
			return err;
		}
	}

	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}

void AP_Win32Frame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	// translate the given document mouse coordinates into absolute screen coordinates.

	POINT pt = { x, y };
	ClientToScreen(m_hwndDocument,&pt);
	x = pt.x;
	y = pt.y;
}

HWND AP_Win32Frame::_createStatusBarWindow(HWND hwndParent,
										   UT_uint32 iLeft, UT_uint32 iTop,
										   UT_uint32 iWidth)
{
	AP_Win32StatusBar * pStatusBar = new AP_Win32StatusBar(this);
	UT_ASSERT(pStatusBar);
	m_hwndStatusBar = pStatusBar->createWindow(hwndParent,iLeft,iTop,iWidth);
	static_cast<AP_FrameData*>(m_pData)->m_pStatusBar = pStatusBar;

	return m_hwndStatusBar;
}

void AP_Win32Frame::setStatusMessage(const char * szMsg)
{
	static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}

UT_Error AP_Win32Frame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}

void AP_Win32Frame::toggleRuler(bool bRulerOn)
{
	toggleTopRuler( bRulerOn );
	toggleLeftRuler( bRulerOn );
}

void AP_Win32Frame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData*>(getFrameData());
	UT_ASSERT(pFrameData);

	if (bRulerOn)
	{
		UT_ASSERT(!pFrameData->m_pTopRuler);

		_createTopRuler();

		static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->setView(m_pView, getZoomPercentage());
	}
	else
	{
		// delete the actual widgets
		if (m_hwndTopRuler)
			DestroyWindow(m_hwndTopRuler);

		DELETEP(static_cast<AP_FrameData*>(m_pData)->m_pTopRuler);

		m_hwndTopRuler = NULL;
	}

	// repack the child windows
	RECT r;
	GetClientRect(m_hwndContainer, &r);
	_onSize(r.right - r.left, r.bottom - r.top);
}

void AP_Win32Frame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData*>(getFrameData());
	UT_ASSERT(pFrameData);

	if (bRulerOn)
	{
		//
		// If There is an old ruler just return
		//
		if(m_hwndLeftRuler)
		{
			return;
		}
		UT_ASSERT(!pFrameData->m_pLeftRuler);

		_createLeftRuler();

		static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->setView(m_pView, getZoomPercentage());
	}
	else
	{
		// delete the actual widgets
		if (m_hwndLeftRuler)
			DestroyWindow(m_hwndLeftRuler);

		DELETEP(static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler);

		m_hwndLeftRuler = NULL;
	}

	// repack the child windows
	RECT r;
	GetClientRect(m_hwndContainer, &r);
	_onSize(r.right - r.left, r.bottom - r.top);
}

/////////////////////////////////////////////////////////////////////////

void AP_Win32Frame::_startTracking(UT_sint32 x, UT_sint32 y)
{
	m_startMouseWheelY = y;
	m_bMouseWheelTrack = true;

	m_startScrollPosition = GetScrollPos(m_hWndVScroll, SB_CTL);

	SetCapture(m_hwndDocument);

}

void AP_Win32Frame::_endTracking(UT_sint32 x, UT_sint32 y)
{
	m_bMouseWheelTrack = false;
	ReleaseCapture();
}

void AP_Win32Frame::_track(UT_sint32 x, UT_sint32 y)
{
	UT_sint32 Delta = y - m_startMouseWheelY;

	// map delta to scroll bar range.

	int iMin, iMax;
	RECT rect;
	GetClientRect(m_hwndDocument, &rect);

	if(y < rect.top || y > rect.bottom)
		return;

	GetScrollRange(m_hWndVScroll, SB_CTL, &iMin, &iMax);

	if(Delta < 0)
	{
		int iNewPosition = (m_startScrollPosition - iMin) * (y - rect.top) / (m_startMouseWheelY - rect.top);

		SendMessage(m_hwndContainer, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, (WORD)iNewPosition), NULL);
	}
	else
	{
		int iNewPosition = m_startScrollPosition + (iMax - m_startScrollPosition) * (y - m_startMouseWheelY) / (rect.bottom - m_startMouseWheelY);

		SendMessage(m_hwndContainer, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, (WORD)iNewPosition), NULL);
	}

}

void AP_Win32Frame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_Win32Frame::toggleBar %d, %d\n", iBarNb, bBarOn));

	// TMN: Yes, this cast is correct. We store EV_Win32Toolbar in
	// m_vecToolbars, but we only need the EV_Toolbar part of it.
	EV_Toolbar* pToolbar = (EV_Toolbar*)m_vecToolbars.getNthItem(iBarNb);

	UT_ASSERT(pToolbar);

	if (!pToolbar)	// release build paranoia
	{
		return;
	}

	if (bBarOn)
	{
		pToolbar->show();
	}
	else
	{
		pToolbar->hide();
	}

	int iToolbarCount = 0;
	for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		EV_Win32Toolbar *pThisToolbar = (EV_Win32Toolbar *)m_vecToolbars.getNthItem(i);

		UT_ASSERT(pThisToolbar);

		if (pThisToolbar)	// release build paranoia
			if ( pThisToolbar->bVisible() )
				iToolbarCount++;
	}

	if( !iToolbarCount || ((iToolbarCount == 1) && bBarOn) )
	{
		RECT r;

		ShowWindow( m_hwndRebar, (iToolbarCount ? SW_SHOW : SW_HIDE) );

		MoveWindow(m_hwndRebar, 0, 0, (iToolbarCount ? m_iSizeWidth : 1),
									  (iToolbarCount ? m_iSizeHeight : 1), TRUE);

		if( iToolbarCount )
		{
			GetClientRect(m_hwndRebar, &r);
			m_iBarHeight = r.bottom - r.top + 6;
		}
		else
			m_iBarHeight = 1;

		GetClientRect(m_hwndContainer, &r);
		_onSize(r.right - r.left, r.bottom - r.top);
	}

	// We *need* to make the window recalc its layout after adding/removing a
	// toolbar in the rebar control. Since we have no "recalcLayout" I'm
	// aware of we use this not-so-good-but-old idiom of resizing the window.
	RECT rc;
	GetWindowRect(m_hwndFrame, &rc);
	const int cx = rc.right - rc.left;
	const int cy = rc.bottom - rc.top;
	const UINT fFlags =
		SWP_FRAMECHANGED	|
		SWP_NOACTIVATE		|
		SWP_NOCOPYBITS		|
		SWP_NOMOVE			|
		SWP_NOOWNERZORDER	|
		SWP_NOZORDER;
	SetWindowPos(m_hwndFrame, 0, 0, 0, cx - 1, cy - 1, fFlags | SWP_NOREDRAW);
	SetWindowPos(m_hwndFrame, 0, 0, 0, cx, cy, fFlags);
}

void AP_Win32Frame::toggleStatusBar(bool bStatusBarOn)
{
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(getFrameData());
	UT_ASSERT(pFrameData);

	if (bStatusBarOn)
	{
		pFrameData->m_pStatusBar->show();
	}
	else
	{
		pFrameData->m_pStatusBar->hide();
	}

	UpdateWindow(m_hwndContainer);
}

void AP_Win32Frame::_showOrHideToolbars(void)
{
	bool *bShowBar = static_cast<AP_FrameData*> (m_pData)->m_bShowBar;

	for (UT_uint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		if (!bShowBar[i])
			toggleBar( i, false );
	}
}

void AP_Win32Frame::_showOrHideStatusbar(void)
{
	bool bShowStatusBar = static_cast<AP_FrameData*> (m_pData)->m_bShowStatusBar;
	toggleStatusBar(bShowStatusBar);
}
