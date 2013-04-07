/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002
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

#include "ap_Win32FrameImpl.h"

#include "ap_Win32Frame.h"
#include "ut_Win32Timer.h"
#include "ut_Win32OS.h"
#include "gr_Win32Graphics.h"
#include "gr_Painter.h"
#include "ap_Win32TopRuler.h"
#include "ap_Win32LeftRuler.h"
#include "ap_Win32StatusBar.h"
#include "ut_Win32LocaleString.h"

#include <winuser.h>

#include <zmouse.h>
#ifdef __MINGW32__
#include "winezmouse.h"
#include <imm.h>
#endif

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

// I really hate doing this, but this constant is only defined with _WIN32_WINNT >=
// 0x0501, i.e., winXP, and I do not want to enable the lot
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x109
#endif

#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR 0xFFFF
#endif

#define GWL(hwnd)		reinterpret_cast<AP_Win32Frame *>(GetWindowLongPtrW((hwnd), GWLP_USERDATA))
#define SWL(hwnd, f)	SetWindowLongPtrW((hwnd), GWLP_USERDATA,(LONG_PTR)(f))

// reserve space for static variables
wchar_t AP_Win32FrameImpl::s_ContainerWndClassName[MAXCNTWNDCLSNMSIZE];
wchar_t AP_Win32FrameImpl::s_DocumentWndClassName[MAXDOCWNDCLSNMSIZE];
//static char s_LeftRulerWndClassName[256];

AP_Win32FrameImpl::AP_Win32FrameImpl(AP_Frame *pFrame) :
	XAP_Win32FrameImpl(static_cast<XAP_Frame *>(pFrame)),
	m_hwndContainer(NULL),
	m_hwndTopRuler(NULL),
	m_hwndLeftRuler(NULL),
	m_hwndDocument(NULL),
	m_hWndHScroll(NULL),
	m_hWndVScroll(NULL),
	m_hWndGripperHack(NULL),
	m_vScale(0),
	m_bMouseWheelTrack(false),
	m_startMouseWheelY(0),
	m_startScrollPosition(0),
	m_bMouseActivateReceived(false)
{
}

AP_Win32FrameImpl::~AP_Win32FrameImpl(void)
{
}


XAP_FrameImpl * AP_Win32FrameImpl::createInstance(XAP_Frame *pFrame)
{
	XAP_FrameImpl *pFrameImpl = new AP_Win32FrameImpl(static_cast<AP_Frame *>(pFrame));

	UT_ASSERT_HARMLESS(pFrameImpl);

	return pFrameImpl;
}

void AP_Win32FrameImpl::_initialize(void)
{
	// FrameData initialized by AP_Win32Frame

	XAP_Win32FrameImpl::_initialize();

	_createTopLevelWindow();
}

HWND AP_Win32FrameImpl::_createDocumentWindow(XAP_Frame *pFrame, HWND hwndParent,
							UT_uint32 iLeft, UT_uint32 iTop,
							UT_uint32 iWidth, UT_uint32 iHeight)
{
	UT_return_val_if_fail(pFrame, NULL);

	// create the window(s) that the user will consider to be the
	// document window -- the thing between the bottom of the toolbars
	// and the top of the status bar.  return the handle to it.

	// create a child window for us -- this will be the 'container'.
	// the 'container' will in turn contain the document window, the
	// rulers, and the variousscroll bars and other dead space.

	m_hwndContainer = UT_CreateWindowEx(WS_EX_CLIENTEDGE, s_ContainerWndClassName, NULL,
									 WS_CHILD | WS_VISIBLE,
									 iLeft, iTop, iWidth, iHeight,
									 hwndParent, NULL, static_cast<XAP_Win32App *>(XAP_App::getApp())->getInstance(), NULL);
	UT_return_val_if_fail (m_hwndContainer, 0);
	// WARNING!!! many places expect an XAP_Frame or descendant!!!
	//SWL(m_hwndContainer, this);
	SWL(m_hwndContainer, pFrame);

	// now create all the various windows inside the container window.

	RECT r;
	GetClientRect(m_hwndContainer,&r);
	const int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	const int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

	m_hWndHScroll = UT_CreateWindowEx(0, L"ScrollBar", 0, WS_CHILD | WS_VISIBLE | SBS_HORZ,
									0, r.bottom - cyHScroll,
									r.right - cxVScroll, cyHScroll,
									m_hwndContainer,
									0, static_cast<XAP_Win32App *>(XAP_App::getApp())->getInstance(), 0);
	UT_return_val_if_fail (m_hWndHScroll, 0);
	// WARNING!!! many places expact an XAP_Frame or descendant!!!
	//SWL(m_hWndHScroll, this);
	SWL(m_hWndHScroll, pFrame);

	m_hWndVScroll = UT_CreateWindowEx(0, L"ScrollBar", 0, WS_CHILD | WS_VISIBLE | SBS_VERT,
									r.right - cxVScroll, 0,
									cxVScroll, r.bottom - cyHScroll,
									m_hwndContainer,
									0, static_cast<XAP_Win32App *>(XAP_App::getApp())->getInstance(), 0);
	UT_return_val_if_fail (m_hWndVScroll, 0);
	// WARNING!!! many places expact an XAP_Frame or descendant!!!
	//SWL(m_hWndVScroll, this);
	SWL(m_hWndVScroll, pFrame);

#if 1 // if the StatusBar is enabled, our lower-right corner is a dead spot
#  define XX_StyleBits          (WS_DISABLED | SBS_SIZEBOX)
#else
#  define XX_StyleBits          (SBS_SIZEGRIP)
#endif

	m_hWndGripperHack = UT_CreateWindowEx(0,L"ScrollBar", 0,
										WS_CHILD | WS_VISIBLE | XX_StyleBits,
										r.right-cxVScroll, r.bottom-cyHScroll, cxVScroll, cyHScroll,
										m_hwndContainer, NULL, static_cast<XAP_Win32App *>(XAP_App::getApp())->getInstance(), NULL);

	UT_return_val_if_fail (m_hWndGripperHack,0);
	// WARNING!!! many places expact an XAP_Frame or descendant!!!
	//SWL(m_hWndGripperHack, this);
	SWL(m_hWndGripperHack, pFrame);

	// create the rulers, if needed

	int yTopRulerHeight = 0;
	int xLeftRulerWidth = 0;

	/* Create Graphics */
	GR_Win32AllocInfo ai(GetDC(m_hwndContainer), m_hwndContainer);
	GR_Win32Graphics * pG = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);

	UT_return_val_if_fail (pG, 0);	   
	
	static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pG = pG;

	if (static_cast<AP_FrameData *>(pFrame->getFrameData())->m_bShowRuler)
	{		
		_createTopRuler(pFrame); 
		if (static_cast<AP_FrameData*>(pFrame->getFrameData())->m_pViewMode == VIEW_PRINT)
	       _createLeftRuler(pFrame);		
		
		_getRulerSizes(static_cast<AP_FrameData *>(pFrame->getFrameData()), yTopRulerHeight, xLeftRulerWidth);
	}

	// create a child window for us.
	m_hwndDocument = UT_CreateWindowEx(0, s_DocumentWndClassName, NULL,
									   WS_CHILD | WS_VISIBLE,
									   xLeftRulerWidth, yTopRulerHeight,
									   r.right - xLeftRulerWidth - cxVScroll,
									   r.bottom - yTopRulerHeight - cyHScroll,
									   m_hwndContainer, NULL,
									   static_cast<XAP_Win32App *>(XAP_App::getApp())->getInstance(), NULL);
	
	UT_return_val_if_fail (m_hwndDocument, 0);
	// WARNING!!! many places expact an XAP_Frame or descendant!!!
	//SWL(m_hwndDocument, this);
	SWL(m_hwndDocument, pFrame);

	return m_hwndContainer;
}

HWND AP_Win32FrameImpl::_createStatusBarWindow(XAP_Frame *pFrame, HWND hwndParent,
										   UT_uint32 iLeft, UT_uint32 iTop,
										   UT_uint32 iWidth)
{
	UT_return_val_if_fail(pFrame, NULL);
	AP_Win32StatusBar * pStatusBar = new AP_Win32StatusBar(pFrame);
	UT_return_val_if_fail (pStatusBar, 0);
	_setHwndStatusBar(pStatusBar->createWindow(hwndParent,iLeft,iTop,iWidth));
	static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pStatusBar = pStatusBar;

	return _getHwndStatusBar();
}

void AP_Win32FrameImpl::_refillToolbarsInFrameData()
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32FrameImpl::_rebuildToolbar(UT_uint32 /*ibar*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32FrameImpl::_toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_Win32FrameImpl::toggleBar %d, %d\n", iBarNb, bBarOn));

	// TMN: Yes, this cast is correct. We store EV_Win32Toolbar in
	// m_vecToolbars, but we only need the EV_Toolbar part of it.
	EV_Toolbar* pToolbar = (EV_Toolbar*)m_vecToolbars.getNthItem(iBarNb);
	UT_return_if_fail(pToolbar);

	if (bBarOn)
	{
		pToolbar->show();
	}
	else
	{
		pToolbar->hide();
	}

	int iToolbarCount = 0;
	for (UT_sint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		EV_Win32Toolbar *pThisToolbar = (EV_Win32Toolbar *)m_vecToolbars.getNthItem(i);

		UT_ASSERT_HARMLESS(pThisToolbar);

		if (pThisToolbar)	// release build paranoia
			if ( pThisToolbar->bVisible() )
				iToolbarCount++;
	}

	if( !iToolbarCount || ((iToolbarCount == 1) && bBarOn) )
	{
		RECT r;

		ShowWindow( _getHwndRebar(), (iToolbarCount ? SW_SHOW : SW_HIDE) );

		MoveWindow( _getHwndRebar(), 0, 0, (iToolbarCount ? _getSizeWidth() : 1),
									  (iToolbarCount ? _getSizeHeight() : 1), TRUE);

		if( iToolbarCount )
		{
			GetClientRect(_getHwndRebar(), &r);
			_setBarHeight(r.bottom - r.top + 6);
		}
		else
			_setBarHeight(1);

		GetClientRect(_getHwndContainer(), &r);
		_onSize(static_cast<AP_FrameData *>(getFrame()->getFrameData()), r.right - r.left + 1, r.bottom - r.top + 1);
	}

	// Tell frame to recalculate layout (screen refresh)
	// Defer the screen repaint if we're in fullscreen mode
	if (!static_cast<AP_FrameData *>(getFrame()->getFrameData())->m_bIsFullScreen)
		getFrame()->queue_resize();
}

void AP_Win32FrameImpl::_bindToolbars(AV_View *pView)
{
	const UT_uint32 nrToolbars = m_vecToolbars.getItemCount();
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

void AP_Win32FrameImpl::_showOrHideToolbars(void)
{
	XAP_Frame* pFrame = getFrame();
	UT_return_if_fail ( pFrame );
	bool *bShowBar = static_cast<AP_FrameData *>(pFrame->getFrameData())->m_bShowBar;

	for (UT_sint32 i = 0; i < m_vecToolbarLayoutNames.getItemCount(); i++)
	{
		if (!bShowBar[i])
			pFrame->toggleBar( i, false );
	}
}

void AP_Win32FrameImpl::_showOrHideStatusbar(void)
{
	XAP_Frame* pFrame = getFrame();
	UT_return_if_fail ( pFrame );
	bool bShowStatusBar = static_cast<AP_FrameData *>(pFrame->getFrameData())->m_bShowStatusBar;
	pFrame->toggleStatusBar(bShowStatusBar);
}

void AP_Win32FrameImpl::_hideMenuScroll(bool /*bHideMenuScroll*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

/* helper methods for helper methods for _showDocument (meta-helper-methods?) :-)
 * called by AP_Frame::_replaceView which is called by AP_Frame::_showDocument 
 */
void AP_Win32FrameImpl::_getDocumentArea(RECT &r)
{
	HWND hwnd = getHwndDocument();
	GetClientRect(hwnd, &r);
	InvalidateRect(hwnd, NULL, TRUE);
}

UT_sint32 AP_Win32FrameImpl::_getDocumentAreaWidth(void)
{
	RECT r;
	_getDocumentArea(r);
	return r.right - r.left;
}

UT_sint32 AP_Win32FrameImpl::_getDocumentAreaHeight(void)
{
	RECT r;
	_getDocumentArea(r);
	return r.bottom - r.top;
}


void AP_Win32FrameImpl::_createTopRuler(XAP_Frame *pFrame)
{
	RECT r;
	int cxVScroll;

	GetClientRect(m_hwndContainer,&r);
	cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

	// create the top ruler
	AP_Win32TopRuler * pWin32TopRuler = new AP_Win32TopRuler(pFrame);
	UT_return_if_fail (pWin32TopRuler);
	m_hwndTopRuler = pWin32TopRuler->createWindow(m_hwndContainer,
												  0,0, (r.right - cxVScroll));
	static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pTopRuler = pWin32TopRuler;


	// get the width from the left ruler and stuff it into the top ruler.
	UT_uint32 xLeftRulerWidth = 0;
#if 0   // Why? We still want to draw the corner, don't we? Also, the width value is messed when leaving normal mode.
	if( m_hwndLeftRuler )
	{
		AP_Win32LeftRuler * pWin32LeftRuler = NULL;
		pWin32LeftRuler =  static_cast<AP_Win32LeftRuler *>(static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pLeftRuler);
		xLeftRulerWidth = pWin32LeftRuler->getWidth();
	}
#endif
	pWin32TopRuler->setOffsetLeftRuler(xLeftRulerWidth);
}
 

void AP_Win32FrameImpl::_createLeftRuler(XAP_Frame *pFrame)
{
	RECT r;
	int cyHScroll;

	GetClientRect(m_hwndContainer,&r);
	cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

	UT_uint32 yTopRulerHeight = 0;

	if( m_hwndTopRuler )
	{
		AP_FrameData * pFrameData = NULL;
		pFrameData = static_cast<AP_FrameData *>( pFrame->getFrameData() );
		AP_Win32TopRuler * pWin32TopRuler = NULL;
		pWin32TopRuler =  static_cast<AP_Win32TopRuler *>( pFrameData->m_pTopRuler );
		yTopRulerHeight =  pWin32TopRuler->getGraphics()->tdu(pWin32TopRuler->getHeight());
	}

	// create the left ruler
	AP_Win32LeftRuler * pWin32LeftRuler = new AP_Win32LeftRuler(pFrame);
	UT_return_if_fail (pWin32LeftRuler);
	m_hwndLeftRuler = pWin32LeftRuler->createWindow(m_hwndContainer,0,yTopRulerHeight,
													r.bottom - yTopRulerHeight - cyHScroll);
	static_cast<AP_FrameData *>(pFrame->getFrameData())->m_pLeftRuler = pWin32LeftRuler;

	// get the width from the left ruler and stuff it into the top ruler.
    if( m_hwndTopRuler )
	{
		AP_FrameData * pFrameData = NULL;
		pFrameData = static_cast<AP_FrameData *>( pFrame->getFrameData() );
		UT_uint32 xLeftRulerWidth = pWin32LeftRuler->getGraphics()->tdu(pWin32LeftRuler->getWidth());
		AP_Win32TopRuler * pWin32TopRuler = NULL;
		pWin32TopRuler =  static_cast<AP_Win32TopRuler *>( pFrameData->m_pTopRuler );
		pWin32TopRuler->setOffsetLeftRuler( xLeftRulerWidth );
	}
}

void AP_Win32FrameImpl::_toggleTopRuler(AP_Win32Frame *pFrame, bool bRulerOn)
{
	UT_return_if_fail(pFrame);
	AP_FrameData *pFrameData = pFrame->getAPFrameData();
	UT_return_if_fail(pFrameData);

	if (bRulerOn)
	{
		AP_TopRuler * pTop = pFrameData->m_pTopRuler;
		if(pTop)
		{
			delete pTop;
		}

		_createTopRuler(pFrame);

		pFrameData->m_pTopRuler->setView(pFrame->getCurrentView(), pFrame->getZoomPercentage());
	}
	else
	{
		// delete the actual widgets
		if (m_hwndTopRuler)
			DestroyWindow(m_hwndTopRuler);

		DELETEP(pFrameData->m_pTopRuler);
		FV_View * pFV = static_cast<FV_View *>(pFrame->getCurrentView());
		pFV->setTopRuler(NULL);
		m_hwndTopRuler = NULL;
	}

	// repack the child windows
	RECT r;
	GetClientRect(m_hwndContainer, &r);
	_onSize(pFrameData, r.right - r.left, r.bottom - r.top);
}

void AP_Win32FrameImpl::_toggleLeftRuler(AP_Win32Frame *pFrame, bool bRulerOn)
{
	UT_return_if_fail(pFrame);
	AP_FrameData *pFrameData = pFrame->getAPFrameData();
	UT_return_if_fail(pFrameData);
		

	if (bRulerOn && pFrameData->m_pViewMode == VIEW_PRINT)
	{
		//
		// If There is an old ruler just return
		//
		AP_LeftRuler * pLeft = pFrameData->m_pLeftRuler;
		if(pLeft)
		{
			delete pLeft;
		}

		_createLeftRuler(pFrame);

		pFrameData->m_pLeftRuler->setView(pFrame->getCurrentView(), pFrame->getZoomPercentage());
	}
	else
	{
		// delete the actual widgets
		if (m_hwndLeftRuler)
			DestroyWindow(m_hwndLeftRuler);

		DELETEP(pFrameData->m_pLeftRuler);
		FV_View * pFV = static_cast<FV_View *>(pFrame->getCurrentView());
		pFV->setLeftRuler(NULL);

		m_hwndLeftRuler = NULL;
	}

	// repack the child windows
	RECT r;
	GetClientRect(m_hwndContainer, &r);
	_onSize(pFrameData, r.right - r.left, r.bottom - r.top);
}

void AP_Win32FrameImpl::_translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	UT_return_if_fail(m_hwndDocument);

	// translate the given document mouse coordinates into absolute screen coordinates.

	POINT pt = { x, y };
	ClientToScreen(m_hwndDocument,&pt);
	x = pt.x;
	y = pt.y;
}

void AP_Win32FrameImpl::_setXScrollRange(AP_FrameData * pData, AV_View *pView)
{
	UT_return_if_fail(m_hwndDocument);

	RECT r;
	GetClientRect(m_hwndDocument, &r);
	const UT_uint32 iWindowWidth = r.right - r.left;
	const UT_uint32 iWidth = pView->getGraphics()->tdu(pData->m_pDocLayout->getWidth());
	XAP_Frame::tZoomType tZoom = getFrame()->getZoomType();	
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = iWidth;
	si.nPos = ((pView) ? pView->getGraphics()->tdu(pView->getXScrollOffset()) : 0);
	si.nPage = iWindowWidth;
	SetScrollInfo(m_hWndHScroll, SB_CTL, &si, TRUE);

	pView->sendHorizontalScrollEvent(pView->getGraphics()->tlu(si.nPos),pView->getGraphics()->tlu(si.nMax-si.nPage));

	// hide the horizontal scrollbar if the scroll range is such that the window can contain it all
	// show it otherwise
	// Hide the horizontal scrollbar if we've set to page width or fit to page.
	// This stops a resizing race condition.
 	if (iWidth <= iWindowWidth || tZoom == XAP_Frame::z_PAGEWIDTH || tZoom == XAP_Frame::z_WHOLEPAGE)
	{	 	
		if (IsWindowVisible (m_hWndHScroll)) {
			RECT r;

			ShowWindow (m_hWndHScroll, SW_HIDE);
			ShowWindow (m_hWndGripperHack, SW_HIDE);
			GetClientRect(m_hwndContainer, &r);
			_onSize(pData, r.right - r.left, r.bottom - r.top);
		}
	}
 	else
	{	
		if (!IsWindowVisible (m_hWndHScroll)) {
			RECT r;

			ShowWindow(m_hWndHScroll, SW_NORMAL);
			ShowWindow (m_hWndGripperHack, SW_NORMAL);
			GetClientRect(m_hwndContainer, &r);
			_onSize(pData, r.right - r.left, r.bottom - r.top);
		}		
	}

}

void AP_Win32FrameImpl::_setYScrollRange(AP_FrameData * pData, AV_View *pView)
{
	UT_return_if_fail(m_hwndDocument);

	RECT r;
	GetClientRect(m_hwndDocument, &r);
	const UT_uint32 iWindowHeight = r.bottom - r.top;
	const UT_uint32 iHeight = pView->getGraphics()->tdu(pData->m_pDocLayout->getHeight());

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin = 0;
	si.nMax = iHeight;
	si.nPos = ((pView) ? pView->getGraphics()->tdu(pView->getYScrollOffset()) : 0);
	si.nPage = iWindowHeight;
	_setVerticalScrollInfo(&si);

	pView->sendVerticalScrollEvent(pView->getGraphics()->tlu(si.nPos),pView->getGraphics()->tlu(si.nMax-si.nPage));
}

// scroll event came in (probably from an EditMethod (like a PageDown
// or insertData or something).  update the on-screen scrollbar and
// then warp the document window contents.
void AP_Win32FrameImpl::_scrollFuncY(UT_sint32 yoff, UT_sint32 /*ylimit*/)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	GR_Graphics *pGr = getFrame()->getCurrentView()->getGraphics();

	_getVerticalScrollInfo(&si);
	si.nPos = pGr->tdu (yoff);
	_setVerticalScrollInfo(&si);
	_getVerticalScrollInfo(&si); // values may have been clamped
	getFrame()->getCurrentView()->setYScrollOffset(pGr->tlu(si.nPos));
}

void AP_Win32FrameImpl::_scrollFuncX(UT_sint32 xoff, UT_sint32 /*xlimit*/)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	GR_Graphics *pGr = getFrame()->getCurrentView()->getGraphics();

	HWND hwndH = _getHwndHScroll();
	GetScrollInfo(hwndH, SB_CTL, &si);

	si.nPos = pGr->tdu(xoff);
	SetScrollInfo(hwndH, SB_CTL, &si, TRUE);

	GetScrollInfo(hwndH, SB_CTL, &si);	// may have been clamped
	getFrame()->getCurrentView()->setXScrollOffset(pGr->tlu(si.nPos));
}

bool AP_Win32FrameImpl::_RegisterClass(XAP_Win32App * app)
{
	// This is a static method, so can't access 'this' or other non-static items directly

	if (!XAP_Win32FrameImpl::_RegisterClass(app))
		return false;

	ATOM a;
    UT_Win32LocaleString str;	

	// register class for the container window (this will contain the document
	// and the rulers and the scroll bars)

    str.fromASCII (app->getApplicationName());	
	_snwprintf(s_ContainerWndClassName, MAXCNTWNDCLSNMSIZE, L"%sContainer", str.c_str());
 	

	a = UT_RegisterClassEx(CS_DBLCLKS | CS_OWNDC, AP_Win32FrameImpl::_ContainerWndProc, app->getInstance(),
						   NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL,
						   NULL, s_ContainerWndClassName);
	
	UT_return_val_if_fail(a, false);

	// register class for the actual document window
	//_snprintf(s_DocumentWndClassName, MAXDOCWNDCLSNMSIZE, "%sDocument", app->getApplicationName());
  	wcscpy (s_DocumentWndClassName, L"Document");
	a = UT_RegisterClassEx(CS_DBLCLKS | CS_OWNDC, AP_Win32FrameImpl::_DocumentWndProc, app->getInstance(),
						   NULL, NULL, NULL, NULL,
						   NULL, s_DocumentWndClassName);

	UT_return_val_if_fail(a, false);

	if (!AP_Win32TopRuler::registerClass(app) ||
		!AP_Win32LeftRuler::registerClass(app))
	{
		return false;
	}

	return true;
}

/*****************************************************************/

#define SCROLL_LINE_SIZE 20

void AP_Win32FrameImpl::_getRulerSizes(AP_FrameData * pData, int &yTopRulerHeight, int &xLeftRulerWidth)
{
	UT_return_if_fail(pData);

	if (pData->m_pTopRuler)
		yTopRulerHeight = pData->m_pG->tdu(pData->m_pTopRuler->getHeight());
	else
		yTopRulerHeight = 0;

	if (pData->m_pLeftRuler)
		xLeftRulerWidth = pData->m_pG->tdu(pData->m_pLeftRuler->getWidth());
	else
		xLeftRulerWidth = 0;
}

void AP_Win32FrameImpl::_onSize(AP_FrameData* pData, int nWidth, int nHeight)
{
	UT_return_if_fail(m_hwndDocument);
	UT_return_if_fail(m_hWndVScroll);
	UT_return_if_fail(m_hWndHScroll);
	UT_return_if_fail(m_hWndGripperHack);

	int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	const int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	int yTopRulerHeight = 0;
	int xLeftRulerWidth = 0;	

	_getRulerSizes(pData, yTopRulerHeight, xLeftRulerWidth);

	MoveWindow(m_hWndHScroll, 0, nHeight-cyHScroll, nWidth - cxVScroll, cyHScroll, TRUE);
	MoveWindow(m_hWndGripperHack, nWidth-cxVScroll, nHeight-cyHScroll, cxVScroll, cyHScroll, TRUE);

	if (IsWindowVisible (m_hWndHScroll) == FALSE)
		cyHScroll = 0;	
	
	MoveWindow(m_hWndVScroll, nWidth-cxVScroll, 0, cxVScroll, nHeight-cyHScroll, TRUE);		

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


void AP_Win32FrameImpl::_setVerticalScrollInfo(const SCROLLINFO * psi)
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

void AP_Win32FrameImpl::_getVerticalScrollInfo(SCROLLINFO * psi)
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

int AP_Win32FrameImpl::_getMouseWheelLines()
{
	UINT nScrollLines;
	if (SystemParametersInfoW(	SPI_GETWHEELSCROLLLINES,
								0,
								(PVOID) &nScrollLines,
								0))
	{
		return nScrollLines;
	}

 	return 3;
}

/////////////////////////////////////////////////////////////////////////

void AP_Win32FrameImpl::_startTracking(UT_sint32 /*x*/, UT_sint32 y)
{
	m_startMouseWheelY = y;
	m_bMouseWheelTrack = true;

	m_startScrollPosition = GetScrollPos(m_hWndVScroll, SB_CTL);

	SetCapture(m_hwndDocument);
}

void AP_Win32FrameImpl::_endTracking(UT_sint32 /*x*/, UT_sint32 /*y*/)
{
	m_bMouseWheelTrack = false;
	ReleaseCapture();
}

void AP_Win32FrameImpl::_track(UT_sint32 /*x*/, UT_sint32 y)
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

		SendMessageW(m_hwndContainer, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, (WORD)iNewPosition), 0);
	}
	else
	{
		int iNewPosition = m_startScrollPosition + (iMax - m_startScrollPosition) * (y - m_startMouseWheelY) / (rect.bottom - m_startMouseWheelY);

		SendMessageW(m_hwndContainer, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, (WORD)iNewPosition), 0);
	}

}

/*****************************************************************/

LRESULT CALLBACK AP_Win32FrameImpl::_ContainerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Frame * f = reinterpret_cast<AP_Win32Frame *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	if (!f)
	{
		return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	AP_Win32FrameImpl *fImpl = f->getAPWin32FrameImpl();
	UT_return_val_if_fail(fImpl, UT_DefWindowProc(hwnd, iMsg, wParam, lParam));

	AV_View* pView = f->getCurrentView();
	//UT_ASSERT(pView);		/* only fatal for some messages */
	UT_return_val_if_fail((pView && ((iMsg == WM_VSCROLL) || (iMsg == WM_HSCROLL))) || ((iMsg != WM_VSCROLL) && (iMsg != WM_HSCROLL)), 0);

	switch (iMsg)
	{
		case WM_SETFOCUS:
		{
			SetFocus(fImpl->getHwndDocument());
			return 0;
		}

		case WM_SIZE:
		{
			const int nWidth = LOWORD(lParam);
			const int nHeight = HIWORD(lParam);

			fImpl->_onSize(static_cast<AP_FrameData *>(f->getFrameData()), nWidth, nHeight);
			return 0;
		}

		case WM_VSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam); // scroll bar value

			SCROLLINFO si;
			memset(&si, 0, sizeof(si));

			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			fImpl->_getVerticalScrollInfo(&si);

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
					si.nPos <<= fImpl->m_vScale;
					break;
			}

			fImpl->_setVerticalScrollInfo(&si);				// notify window of new value.
			fImpl->_getVerticalScrollInfo(&si);				// update from window, in case we got clamped
			pView->sendVerticalScrollEvent(pView->getGraphics()->tlu(si.nPos));	// now tell the view

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
			GetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si);

			switch (nScrollCode)
			{
				case SB_PAGEUP:
					si.nPos -= si.nPage;
					if (si.nPos < 0)
					{
						si.nPos = 0;
					}
					SetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si, TRUE);
					break;
				case SB_PAGEDOWN:
					si.nPos += si.nPage;
					SetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si, TRUE);
					break;
				case SB_LINEDOWN:
					si.nPos += SCROLL_LINE_SIZE;
					SetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si, TRUE);
					break;
				case SB_LINEUP:
					si.nPos -= SCROLL_LINE_SIZE;
					if (si.nPos < 0)
					{
						si.nPos = 0;
					}
					SetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si, TRUE);
					break;
				case SB_THUMBPOSITION:
				case SB_THUMBTRACK:
					si.nPos = nPos;
					SetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si, TRUE);
					break;
			}

			if (nScrollCode != SB_ENDSCROLL)
			{
				// in case we got clamped
				GetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si);

				// now tell the view
				pView->sendHorizontalScrollEvent(pView->getGraphics()->tlu(si.nPos));
			}

			return 0;
		}

		case WM_SYSCOLORCHANGE:
		{
			SendMessageW(fImpl->m_hwndTopRuler,WM_SYSCOLORCHANGE,0,0);
			SendMessageW(fImpl->m_hwndLeftRuler,WM_SYSCOLORCHANGE,0,0);
			SendMessageW(fImpl->m_hwndDocument,WM_SYSCOLORCHANGE,0,0);
			return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
		}

	 	case WM_MOUSEWHEEL:
 		{	 
			const int iDelta = (short) HIWORD(wParam);

			if (wParam & MK_CONTROL)	// Zoom in & Zoom out
			{	
				EV_Win32Mouse*	pMouse = (EV_Win32Mouse *) fImpl->m_pMouse;
 		
 				if (iDelta>0)
 					pMouse->onButtonWheel(pView,hwnd,EV_EMB_BUTTON4,wParam,LOWORD(lParam),HIWORD(lParam));
 				else
 					pMouse->onButtonWheel(pView,hwnd,EV_EMB_BUTTON5,wParam,LOWORD(lParam),HIWORD(lParam));

				break;
			} 			
 			
			const int cWheelLines = _getMouseWheelLines();

			if (WHEEL_PAGESCROLL == cWheelLines)
			{
				WORD wDir = (iDelta < 0) ? SB_PAGEDOWN : SB_PAGEUP;
				SendMessageW(hwnd,
							WM_VSCROLL,
							MAKELONG(wDir, 0),
							0);
			}
			else
			{

				// Calculate the movement offset to an integer resolution
				const int iMove = (iDelta * cWheelLines) / WHEEL_DELTA;

				// Get current scroll position
				SCROLLINFO si;
				memset(&si, 0, sizeof(si));

				si.cbSize = sizeof(si);
				si.fMask = SIF_ALL;
				fImpl->_getVerticalScrollInfo(&si);

				// Clip new position to limits
				int iNewPos = si.nPos - (((iMove)?iMove:1) * SCROLL_LINE_SIZE);
				if (iNewPos > si.nMax) iNewPos = si.nMax;
				if (iNewPos < si.nMin) iNewPos = si.nMin;

				if (iNewPos != si.nPos)
				{
					// If position has changed set new position
					iNewPos >>= fImpl->m_vScale;
					SendMessageW(hwnd,
							WM_VSCROLL,
							MAKELONG(SB_THUMBPOSITION, iNewPos),
							0);
				}
			}
			return 0;
		}

		default:
			return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	return 0; // to silence the compiler
}

LRESULT CALLBACK AP_Win32FrameImpl::_DocumentWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Frame * f = reinterpret_cast<AP_Win32Frame *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	if (!f || !IsWindow(hwnd))
	{
		return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	AP_Win32FrameImpl *fImpl = static_cast<AP_Win32FrameImpl *>(f->getFrameImpl());
	UT_return_val_if_fail(fImpl, UT_DefWindowProc(hwnd, iMsg, wParam, lParam));

	AV_View* 		pView = f->getCurrentView();
	EV_Win32Mouse*	pMouse = (EV_Win32Mouse *) fImpl->m_pMouse;

	switch (iMsg)
	{
		case WM_MOUSEACTIVATE:
			fImpl->m_bMouseActivateReceived = true;
			return MA_ACTIVATE;

		case WM_SETFOCUS:			
			if (pView)
			{					
				pView->focusChange(AV_FOCUS_HERE);

				if(!fImpl->m_bMouseActivateReceived)
				{
					// HACK:	Capture leaving a tool bar combo.
					// We need to get a mouse down signal.
					// windows is not activated so send a mouse down if the mouse is pressed.

					UT_DEBUGMSG(("%s(%d): Need to set mouse down\n", __FILE__, __LINE__));

					// GetKeyState
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
			return 1;
		}

		case WM_LBUTTONDOWN:
			if(GetFocus() != hwnd)
			{
				SetFocus(hwnd);
				pView = f->getCurrentView();
				pMouse = (EV_Win32Mouse *) fImpl->m_pMouse;
			}
			pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
			fImpl->m_bMouseActivateReceived = false;
			return 0;

		case WM_MBUTTONDOWN:
			fImpl->_startTracking(LOWORD(lParam), HIWORD(lParam));
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

		case WM_SYSKEYDOWN:
		{
			UT_DEBUGMSG(("WM_SYSKEYDOWN %d  - %d\n",wParam, lParam));
			ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fImpl->m_pKeyboard);

			BYTE	keyboardState[256];

			::GetKeyboardState(keyboardState);

			UT_DEBUGMSG(("ev_Win32Keyboard::_getModifierState VK_LCONTROL: %u, VK_RCONTROL %u, VK_LMENU %u, VK_LMENU %u, VK_CONTROL %u, VK_MENU %u\n",
				(GetKeyState(VK_LCONTROL) & 0x8000), (GetKeyState(VK_RCONTROL) & 0x8000),
				(GetKeyState(VK_LMENU) & 0x8000), (GetKeyState(VK_LMENU) & 0x8000),
				(GetKeyState(VK_CONTROL) & 0x8000), (GetKeyState(VK_MENU) & 0x8000)	));

			UT_DEBUGMSG(("ev_Win32Keyboard::GetKeyboardState VK_LCONTROL: %u, VK_RCONTROL %u, VK_LMENU %u, VK_LMENU %u, VK_CONTROL %u, VK_MENU %u\n",
				(keyboardState[VK_LCONTROL]), (keyboardState[VK_RCONTROL]),
				(keyboardState[VK_LMENU] ), (keyboardState[VK_LMENU]),
				(keyboardState[VK_CONTROL]), (keyboardState[VK_MENU])	));

	    
	     	if (pWin32Keyboard->onKeyDown(pView,hwnd,iMsg,wParam,lParam))
    			return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
	 		else
 				return false;	    				 	     			
		
		}

		case WM_KEYUP:
		{
			BYTE	keyboardState[256];

			::GetKeyboardState(keyboardState);

			/*
				Win 9.x does not diferenciate between left and right keys, we have to
				do it ourselfs. Win NT based system do.
			*/
			if (wParam == VK_CONTROL)
			{
				keyboardState[(lParam & 0x1000000) ? VK_RCONTROL : VK_LCONTROL] &= ~0x80;
				SetKeyboardState (keyboardState);
			}
			else if (wParam == VK_MENU)
			{						
				keyboardState[(lParam & 0x1000000) ? VK_RMENU : VK_LMENU]  &= ~0x80;
				SetKeyboardState (keyboardState);
			}

			break;
		}
		case WM_KEYDOWN:
		{
			UT_DEBUGMSG(("WM_KEYDOWN %d  - %d\n",wParam, lParam));
			ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fImpl->m_pKeyboard);

				BYTE	keyboardState[256];

				::GetKeyboardState(keyboardState);

				/*
					Win 9.x does not diferenciate between left and right keys, we have to
					do it ourselfs. Win NT based system do.
				*/
				if (wParam == VK_CONTROL)
				{
					keyboardState[(lParam & 0x1000000) ? VK_RCONTROL : VK_LCONTROL] |= 0x80;
					SetKeyboardState (keyboardState);
				}
				else if (wParam == VK_MENU)
				{						
					keyboardState[(lParam & 0x1000000) ? VK_RMENU : VK_LMENU] |= 0x80;
					SetKeyboardState (keyboardState);
				}				

				/*
				UT_DEBUGMSG(("ev_Win32Keyboard::_getModifierState VK_LCONTROL: %u, VK_RCONTROL %u, VK_RMENU %u, VK_LMENU %u, VK_CONTROL %u, VK_MENU %u\n",
					(GetKeyState(VK_LCONTROL) & 0x8000), (GetKeyState(VK_RCONTROL) & 0x8000),
					(GetKeyState(VK_RMENU) & 0x8000), (GetKeyState(VK_LMENU) & 0x8000),
					(GetKeyState(VK_CONTROL) & 0x8000), (GetKeyState(VK_MENU) & 0x8000)	));

				UT_DEBUGMSG(("ev_Win32Keyboard::GetKeyboardState VK_LCONTROL: %u, VK_RCONTROL %u, VK_RMENU %u, VK_LMENU %u, VK_CONTROL %u, VK_MENU %u, sys %u\n",
					(keyboardState[VK_LCONTROL]), (keyboardState[VK_RCONTROL]),
					(keyboardState[VK_RMENU] ), (keyboardState[VK_LMENU]),
					(keyboardState[VK_CONTROL]), (keyboardState[VK_MENU]),
					(lParam & 0x1000000) ));
				*/

	    
		     	if (pWin32Keyboard->onKeyDown(pView,hwnd,iMsg,wParam,lParam))
     				return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
	 		else
 				return false;	    				 	     			
    		
		}

		case WM_SYSCHAR:
		{
			UT_DEBUGMSG(("WM_SYSCHAR %d  - %d\n",wParam, lParam));
			return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
		}

		case WM_CHAR:
		{
//            bool is_uni = IsWindowUnicode (hwnd);
			UT_DEBUGMSG(("WM_CHAR %d  - %d\n",wParam, lParam));
			ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fImpl->m_pKeyboard);
	    
			pWin32Keyboard->onChar(pView,hwnd,iMsg,wParam,lParam);		
			return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
		}

		case WM_UNICHAR:
		{
			UT_DEBUGMSG(("WM_CHAR %d  - %d\n",wParam, lParam));

			// UNICODE_NOCHAR is sent to test if we can handle this message
			if(wParam == UNICODE_NOCHAR)
				return 1;
			
			ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fImpl->m_pKeyboard);
	    
			pWin32Keyboard->onUniChar(pView,hwnd,iMsg,wParam,lParam);		
			return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
		}

		case WM_IME_CHAR:
		{
			ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fImpl->m_pKeyboard);
			if (pWin32Keyboard->onIMEChar(pView,hwnd,iMsg,wParam,lParam))
				return 0;
			return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
		}

		case WM_MOUSEMOVE:
			if (fImpl->_isTracking())
			{
				fImpl->_track(LOWORD(lParam),HIWORD(lParam));
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
				pView = f->getCurrentView();
				pMouse = (EV_Win32Mouse *) fImpl->m_pMouse;
				pMouse->onButtonDown(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
			}
			pMouse->onButtonUp(pView,hwnd,EV_EMB_BUTTON1,wParam,LOWORD(lParam),HIWORD(lParam));
			return 0;

		case WM_MBUTTONUP:
			fImpl->_endTracking(LOWORD(lParam), HIWORD(lParam));
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
				
				// when resizing, set the rulers to their new height/width
				if ((f->getAPFrameData()->m_pTopRuler) && (f->getAPFrameData()->m_pLeftRuler))
				{
					f->getAPFrameData()->m_pTopRuler->setWidth(nWidth + pView->getGraphics()->tdu(f->getAPFrameData()->m_pLeftRuler->getWidth()));
					f->getAPFrameData()->m_pLeftRuler->setHeight(nHeight + pView->getGraphics()->tdu(f->getAPFrameData()->m_pTopRuler->getHeight()));
				} else {
					if (f->getAPFrameData()->m_pTopRuler)
						f->getAPFrameData()->m_pTopRuler->setWidth(nWidth);
					if (f->getAPFrameData()->m_pLeftRuler)
						f->getAPFrameData()->m_pLeftRuler->setHeight(nHeight);
				}

				// may need to scroll to keep everything in sync.
				// the following is necessary to make sure that the
				// window de-scrolls as it gets larger.

				SCROLLINFO si;
				memset(&si, 0, sizeof(si));

				si.cbSize = sizeof(si);
				si.fMask = SIF_ALL;

				fImpl->_getVerticalScrollInfo(&si);
				pView->sendVerticalScrollEvent(pView->getGraphics()->tlu(si.nPos),pView->getGraphics()->tlu(si.nMax-si.nPage));

				GetScrollInfo(fImpl->m_hWndHScroll, SB_CTL, &si);
				pView->sendHorizontalScrollEvent(pView->getGraphics()->tlu(si.nPos),pView->getGraphics()->tlu(si.nMax-si.nPage));
			}
			return 0;
		}

		case WM_PAINT:
		{
						
			FV_View * pFV = static_cast<FV_View *>(pView);
			GR_Graphics * pG = pFV->getGraphics();

			GR_Painter caretDisablerPainter(pG); // not an elegant way to disable all carets, but it works beautifully - MARCM
			
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			UT_return_val_if_fail(hdc, 0);
			
			UT_Rect r(pG->tlu(ps.rcPaint.left), pG->tlu(ps.rcPaint.top),
					pG->tlu(ps.rcPaint.right-ps.rcPaint.left+1),
					pG->tlu(ps.rcPaint.bottom-ps.rcPaint.top+1));
			pView->draw(&r);

			EndPaint(hwnd, &ps);

			return 0;
		}

		case WM_TIMER:
		{
			UT_DebugOnly<TIMERPROC *> pfn = (TIMERPROC *)lParam;
			UT_ASSERT_HARMLESS( (void *)(pfn) == (void *)(Global_Win32TimerProc) );
			Global_Win32TimerProc(hwnd,WM_TIMER,(UINT)wParam, 0);
			return 0;
		}

		case WM_DESTROY:
			return 0;

		case WM_INPUTLANGCHANGE:	// let the XAP_Win32Frame handle this
			return ::SendMessageW(fImpl->_getTopLevelWindow(), WM_INPUTLANGCHANGE, wParam, lParam);
#ifdef COPY_ON_DEMAND
		case WM_RENDERFORMAT:
			if(static_cast<AP_Win32App *>(XAP_App::getApp())->copyFmtToClipboardOnDemand((UINT)wParam))
				return 0;
			else
				break;
			
		case WM_RENDERALLFORMATS:
			if(static_cast<AP_Win32App *>(XAP_App::getApp())->copyAllFmtsToClipboardOnDemand())
				return 0;
			else
				break;
#endif
	} /* switch (iMsg) */

	return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
}

UT_RGBColor AP_Win32FrameImpl::getColorSelBackground () const
{
	DWORD bgcolor = GetSysColor(COLOR_HIGHLIGHT);

	unsigned char red   = static_cast<unsigned char>(((bgcolor)      ) & 0xff);
	unsigned char green = static_cast<unsigned char>(((bgcolor) >> 8 ) & 0xff); 
 	unsigned char blue  = static_cast<unsigned char>(((bgcolor) >> 16) & 0xff);
	
	if( !bgcolor )
	{
		red = green = blue = 192;
	}

	return UT_RGBColor( red, green, blue );
}

GR_Win32Graphics *AP_Win32FrameImpl::createDocWndGraphics(void)
{
	GR_Win32AllocInfo ai(GetDC(getHwndDocument()), getHwndDocument());
	return (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);
}
