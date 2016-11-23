/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_Win32FrameImpl.h"
#include "commctrl.h"
#include <limits.h>					/* for INT_MAX */

#include "zmouse.h"
#ifdef __MINGW32__
#include "winezmouse.h"
#endif

#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_Win32OS.h"
#include "ut_Win32LocaleString.h"
#include "xap_Win32DialogBase.h"
#include "xap_ViewListener.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "ie_impGraphic.h"
#include "fv_View.h"
#include "ap_Win32App.h"
#include "fg_Graphic.h"
#include "gr_Win32Graphics.h"


#ifdef _MSC_VER
#pragma warning(disable: 4355)	// 'this' used in base member initializer list
#endif

#ifdef STRICT   
#define WHICHPROC	WNDPROC
#else   
#define WHICHPROC	FARPROC
#endif

/* Defines not present in old header files*/

#ifndef TBN_FIRST
#define TBN_FIRST               (0U-700U)       // toolbar
#endif

#ifndef TBN_DROPDOWN
#define TBN_DROPDOWN            (TBN_FIRST - 10)
#endif

#ifndef TBDDRET_DEFAULT
#define TBDDRET_DEFAULT 0 
#endif

WHICHPROC s_oldRedBar; 

// Where the heck is this function????
// TODO Fix the following header file. It seems to be incomplete
// TODO #include <ap_EditMethods.h>
// TODO In the mean time, define the needed function by hand
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

/*
	Owner drawn for child font combo box
*/
LRESULT CALLBACK s_rebarWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{		
		case WM_DRAWITEM:
		{
			 DRAWITEMSTRUCT* pDrawItem = (DRAWITEMSTRUCT*)lParam;
			 SendMessageW(pDrawItem->hwndItem, WM_DRAWITEM, wParam, lParam);
			 return TRUE;
		}

		case WM_MEASUREITEM:
		{							   			
			MEASUREITEMSTRUCT*	mesure = (MEASUREITEMSTRUCT*) lParam;

			if (mesure->CtlType==ODT_COMBOBOX)
			{
				mesure->itemHeight = 16;										
				return TRUE;
			}
			break;
		}
		
		default:
			break;		
	}

	return CallWindowProcW(s_oldRedBar, hWnd, uMessage, wParam, lParam);
}



XAP_Win32FrameImpl::XAP_Win32FrameImpl(XAP_Frame *pFrame) :
	XAP_FrameImpl(pFrame),
	m_hwndFrame(NULL),
	m_hwndRebar(NULL),
	m_hwndContainer(NULL),
	m_hwndStatusBar(NULL),
	m_dialogFactory(XAP_App::getApp(), pFrame), /* note: pFrame->getApp() not initialized properly yet! */		
	m_pWin32Menu(NULL),
	m_pWin32Popup(NULL),
	m_iBarHeight(0),
	m_iStatusBarHeight(0),
	m_iRealSizeHeight(0),
	m_iRealSizeWidth(0),
	m_iWindowStateBeforeFS(SW_SHOWNORMAL),
	m_iWindowXBeforeFS(0),
	m_iWindowYBeforeFS(0),
	m_iWindowHeightBeforeFS(0),
	m_iWindowWidthBeforeFS(0),
	m_mouseWheelMessage(0),
	m_iSizeWidth(0),
	m_iSizeHeight(0)
{
}

XAP_Win32FrameImpl::~XAP_Win32FrameImpl(void)
{
	// only delete the things we created...
	
	DELETEP(m_pWin32Menu);
	DELETEP(m_pWin32Popup);

	// have to reset the window long, so our message functions do not
	// try to derefernce it
	SetWindowLongPtrW(m_hwndFrame, GWLP_USERDATA,0);
}


#if 0
void XAP_Win32FrameImpl::_startViewAutoUpdater(void) {}
static void XAP_Win32FrameImpl::viewAutoUpdater(UT_Worker *wkr) {}
#endif


bool XAP_Win32FrameImpl::_updateTitle(void) 
{
	UT_return_val_if_fail(m_hwndFrame, false);
	XAP_Frame *pFrame = getFrame();
	UT_return_val_if_fail(pFrame, false);


	if (!XAP_FrameImpl::_updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	UT_UTF8String sTmp = pFrame->getTitle();
	sTmp += " - ";
    sTmp += XAP_App::getApp()->getApplicationTitleForTitleBar();	
	XAP_Win32DialogBase::setWindowText (m_hwndFrame, sTmp.utf8_str());

	return true;
}

void XAP_Win32FrameImpl::_initialize(void)
{
	// we assume AP_{FE}Frame has already called XAP_Frame::initialize(...);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
	UT_return_if_fail(pEEM);

	m_pKeyboard = new ev_Win32Keyboard(pEEM);
	UT_return_if_fail(m_pKeyboard);
	
	m_pMouse = new EV_Win32Mouse(pEEM);
	UT_return_if_fail(m_pMouse);
}

void XAP_Win32FrameImpl::_createTopLevelWindow(void)
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
	} else {
		// Ensure the window fits current desktop area
		RECT rcDesktop, rcWindow;
#if (_WIN32_WINNT >= 0x0500)
		rcDesktop.left=rcDesktop.right=0;

		if (GetSystemMetrics(SM_CMONITORS)>1) {
			HMONITOR m;
			MONITORINFO mif;
			SetRect(&rcWindow,iPosX,iPosY,iPosX+iWidth,iPosY+iHeight);
			if ((m = MonitorFromRect(&rcWindow, MONITOR_DEFAULTTONEAREST))) {;
				mif.cbSize=sizeof(MONITORINFO);
				if (GetMonitorInfoW(m,&mif)) {
					rcDesktop=mif.rcWork;
				}
			}
		}

		if (rcDesktop.left==rcDesktop.right)
#endif
			if (!SystemParametersInfoW(SPI_GETWORKAREA,0,&rcDesktop,0))
				SetRect(&rcDesktop,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

		if (iWidth > (rcDesktop.right-rcDesktop.left)) iWidth=rcDesktop.right-rcDesktop.left;
		if (iHeight > (rcDesktop.bottom-rcDesktop.top)) iHeight=rcDesktop.bottom-rcDesktop.top;

		if (iPosX+iWidth > rcDesktop.right) iPosX=rcDesktop.right-iWidth;
		if (iPosY+iHeight > rcDesktop.bottom) iPosY=rcDesktop.bottom-iHeight;

		if (iPosX < rcDesktop.left) iPosX=rcDesktop.left;
		if (iPosY < rcDesktop.top)  iPosY=rcDesktop.top;
	}
	/* let Windows(R) place the Window for all but 1st one, for stairstep effect */
	if (!firstWindow)
	{
		iPosX = CW_USEDEFAULT;
		iPosY = CW_USEDEFAULT;
	}
	else firstWindow = false;

	UT_DEBUGMSG(("KJD: Window Frame should be %d x %d [width x height]\n", iWidth, iHeight));


	XAP_Win32App *pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());

	UT_Win32LocaleString str, title;
	str.fromASCII (pWin32App->getApplicationName());	
	title.fromASCII (pWin32App->getApplicationTitleForTitleBar());
		
	m_hwndFrame = UT_CreateWindowEx(0L, str.c_str(), title.c_str(),
									WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
									iPosX, iPosY, iWidth, iHeight,
									NULL, NULL, pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndFrame);


	// bind this frame to its window
	// WARNING: We assume in many places this refers to a XAP_Frame or descendant!!!
	//SetWindowLongPtrW(m_hwndFrame, GWLP_USERDATA,(LONG_PTR)this);
	SetWindowLongPtrW(m_hwndFrame, GWLP_USERDATA,(LONG_PTR)getFrame());

#ifndef UNICODE
	// remove this when we are a true unicode app
	m_mouseWheelMessage = RegisterWindowMessageW(L"MSWHEEL_ROLLMSG");
#else
	m_mouseWheelMessage = RegisterWindowMessageW(MSH_MOUSEWHEEL);
#endif

	// synthesize a menu from the info in our
	// base class and install it into the window.
	m_pWin32Menu = new EV_Win32MenuBar(pWin32App,
							 XAP_App::getApp()->getEditEventMapper(),
							m_szMenuLayoutName,
							m_szMenuLabelSetName);
	UT_return_if_fail(m_pWin32Menu);
	UT_DebugOnly<bool> bResult = m_pWin32Menu->synthesizeMenuBar(getFrame());
	UT_ASSERT(bResult);

	HMENU oldMenu = GetMenu(m_hwndFrame);
	if (SetMenu(m_hwndFrame, m_pWin32Menu->getMenuHandle()))
	{
		DrawMenuBar(m_hwndFrame);
		if (oldMenu)
			DestroyMenu(oldMenu);
	}

	// create a rebar container for all the toolbars
	m_hwndRebar = UT_CreateWindowEx(0L, REBARCLASSNAMEW, NULL,
									WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
									WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
									RBS_VARHEIGHT | RBS_BANDBORDERS,
									0, 0, 0, 0,
									m_hwndFrame, NULL, pWin32App->getInstance(), NULL);
	UT_ASSERT(m_hwndRebar);
	
	/* override the window procedure*/
	s_oldRedBar = (WHICHPROC)GetWindowLongPtrW(m_hwndRebar, GWLP_WNDPROC);
	SetWindowLongPtrW(m_hwndRebar, GWLP_WNDPROC, (LONG_PTR)s_rebarWndProc);

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

	m_hwndContainer = _createDocumentWindow(getFrame(), m_hwndFrame, 0, m_iBarHeight, iWidth, iHeight);

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.

	m_hwndStatusBar = _createStatusBarWindow(getFrame(), m_hwndFrame,0,m_iBarHeight+iHeight,iWidth);
	GetClientRect(m_hwndStatusBar,&r);
	m_iStatusBarHeight = r.bottom;

	
	// Register drag and drop data and files
	m_dropTarget.setFrame(getFrame());
	RegisterDragDrop(m_hwndFrame, &m_dropTarget);	
		
	return;
}

bool XAP_Win32FrameImpl::_close(void)
{
	UT_return_val_if_fail(m_hwndFrame, false);

	// NOTE: This may not be the proper place, but it does mean that the
	// last window closed is the one that the window state is stored from.
	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT); // must do
	if (GetWindowPlacement(m_hwndFrame, &wndPlacement))
	{
		UT_uint32 nFlags = PREF_FLAG_GEOMETRY_POS | PREF_FLAG_GEOMETRY_SIZE;

		if (wndPlacement.showCmd == SW_SHOWMAXIMIZED)
			nFlags |= PREF_FLAG_GEOMETRY_MAXIMIZED;

		XAP_App::getApp()->setGeometry(wndPlacement.rcNormalPosition.left, 
				wndPlacement.rcNormalPosition.top, 
				wndPlacement.rcNormalPosition.right - wndPlacement.rcNormalPosition.left,
				wndPlacement.rcNormalPosition.bottom - wndPlacement.rcNormalPosition.top,	
				nFlags);
	}	
	else
	{
		// if failed to get placement then invalidate stored settings
		XAP_App::getApp()->setGeometry(0,0,0,0,0);
	}
	
	RevokeDragDrop(m_hwndFrame);
	

	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);
	m_hwndFrame = NULL;

	return true;
}

bool XAP_Win32FrameImpl::_raise(void)
{
	UT_return_val_if_fail(m_hwndFrame, false);
	BringWindowToTop(m_hwndFrame);
	return true;
}

bool XAP_Win32FrameImpl::_show(void)
{
	UT_return_val_if_fail(m_hwndFrame, false);

	ShowWindow(m_hwndFrame, SW_SHOW);
	UpdateWindow(m_hwndFrame);

	return true;
}


XAP_DialogFactory * XAP_Win32FrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

EV_Toolbar * XAP_Win32FrameImpl::_newToolbar(XAP_Frame *frame, const char *szLayout, const char *szLanguage)
{
	EV_Win32Toolbar *result = new EV_Win32Toolbar(static_cast<XAP_Win32App *>(XAP_App::getApp()), 
												  frame, 
												  szLayout, szLanguage);
	// for now, position each one manually
	// TODO: put 'em all in a rebar instead
	HWND hwndBar = result->getWindow();
	if (!hwndBar) 
		return result;

	RECT rcClient;
	GetClientRect(hwndBar, &rcClient);
	const UT_uint32 iHeight = rcClient.bottom - rcClient.top;
	
	m_iBarHeight += iHeight;

	return result;
}

EV_Menu* XAP_Win32FrameImpl::_getMainMenu(void)
{
	return m_pWin32Menu;
}

// Useful to refresh the size of the Frame.  For instance,
// when the user selects hide statusbar, the Frame has to be
// resized in order to fill the gap leaved by the statusbar
void XAP_Win32FrameImpl::_queue_resize(void)
{
	::SendMessageW(m_hwndFrame, WM_SIZE, 0, MAKELONG(m_iRealSizeWidth, m_iRealSizeHeight));
}


bool XAP_Win32FrameImpl::_runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y)
{
	bool bResult = false;

	UT_return_val_if_fail((m_pWin32Popup==NULL), false);

	x = pView->getGraphics()->tdu(x);
	y = pView->getGraphics()->tdu(y);

	m_pWin32Popup = new EV_Win32MenuPopup(static_cast<XAP_Win32App*>(XAP_App::getApp()),szMenuName,m_szMenuLabelSetName);
	m_pWin32Popup->setTrackMenu(true);

	if (m_pWin32Popup && m_pWin32Popup->synthesizeMenuPopup(getFrame()))
	{
		
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
		
		_translateDocumentToScreen(x,y);

		TrackPopupMenu(m_pWin32Popup->getMenuHandle(),
					    TPM_TOPALIGN | TPM_RIGHTBUTTON,
					   x,y,0,m_hwndFrame,NULL);

		// the popup steals our capture, so we need to reset our counter.
		EV_Win32Mouse *pWin32Mouse = static_cast<EV_Win32Mouse *>(m_pMouse);
		pWin32Mouse->reset();
	}

	DELETEP(m_pWin32Popup);
	return bResult;
}

void XAP_Win32FrameImpl::_setFullScreen(bool isFullScreen)
{
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	HWND hwndFrame = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();

	// Get the window's style so we can add or remove the titlebar later
	long hStyle = GetWindowLongPtrW(hwndFrame, GWL_STYLE);

	if (isFullScreen)
	{
		// Save window state prior to fullscreen (are we maximized?)
		WINDOWPLACEMENT wndPlacement;
		wndPlacement.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(hwndFrame, &wndPlacement))
		{
			m_iWindowStateBeforeFS = wndPlacement.showCmd;
		}
		else
		{
			// Couldn't retrieve windowplacement info
			// Assume we were at Normal (non-maximized) state
			m_iWindowStateBeforeFS = SW_SHOWNORMAL;
		}

		// Save window dimensions, as well
		RECT rc;

		if(!GetWindowRect(hwndFrame, &rc))
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			// try to pick some sane defaults that will work with any monitor
			m_iWindowXBeforeFS = 0;
			m_iWindowYBeforeFS = 0;
			m_iWindowHeightBeforeFS = 400;
			m_iWindowWidthBeforeFS = 400;
		}
		else
		{
			m_iWindowXBeforeFS = rc.left;
			m_iWindowYBeforeFS = rc.top;
			m_iWindowHeightBeforeFS = rc.bottom - rc.top;
			m_iWindowWidthBeforeFS = rc.right - rc.left;
		}
		
	}

	// Add or remove title-bar and border
	SetWindowLongPtrW(hwndFrame, GWL_STYLE, isFullScreen ? hStyle & ~WS_CAPTION : hStyle | WS_CAPTION);

	// We hide the window before maximizing
	// to ensure it displays with the proper geometry
	// and to bypass the window resizing animation
	ShowWindow(hwndFrame, SW_HIDE);

	ShowWindow(hwndFrame, isFullScreen ? SW_SHOWMAXIMIZED : m_iWindowStateBeforeFS);

	// TODO: does the following code work well with multiple monitors?
	if(isFullScreen)
	{
		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);

		if((height > 0) && (width > 0))
		{
			if(!SetWindowPos(hwndFrame, 0, 0, 0, width, height, 0))
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}
	else
	{
		if(!SetWindowPos(hwndFrame, 0, m_iWindowXBeforeFS, m_iWindowYBeforeFS, m_iWindowWidthBeforeFS, m_iWindowHeightBeforeFS, 0))
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}

	UpdateWindow(hwndFrame);
}




void XAP_Win32FrameImpl::_nullUpdate (void) const
{
	MSG msg;
	for( int i = 0 ; i < 10 ; i++ )
	{
		if( PeekMessageW( &msg, (HWND) NULL, 0, 0, PM_REMOVE) )
		{
			DispatchMessageW(&msg); 
		} 
	}
}

void XAP_Win32FrameImpl::_setCursor(GR_Graphics::Cursor /*cursor*/)
{
	FV_View* pView = (FV_View *) getFrame()->getCurrentView();

	// this is legitimate, when we first load, the frame has no view
	if(!pView)
		return;
	
	GR_Win32Graphics * pG = (GR_Win32Graphics*)pView->getGraphics();
	UT_return_if_fail( pG );

	pG->handleSetCursorMessage();
}

UT_RGBColor XAP_Win32FrameImpl::getColorSelBackground () const
{
	DWORD dwResult = GetSysColor(COLOR_HIGHLIGHT);
	static UT_RGBColor clr((unsigned char)GetRValue(dwResult),
						   (unsigned char)GetGValue(dwResult),
						   (unsigned char)GetBValue(dwResult));

	return clr;
}

UT_RGBColor XAP_Win32FrameImpl::getColorSelForeground () const
{
	DWORD dwResult = GetSysColor(COLOR_HIGHLIGHTTEXT);
	static UT_RGBColor clr((unsigned char)GetRValue(dwResult),
						   (unsigned char)GetGValue(dwResult),
						   (unsigned char)GetBValue(dwResult));

	return clr;
}

#define MAXAPPNAME 256
bool XAP_Win32FrameImpl::_RegisterClass(XAP_Win32App * app)
{
	ATOM a = UT_RegisterClassEx(CS_DBLCLKS, XAP_Win32FrameImpl::_FrameWndProc, app->getInstance(),
								app->getIcon(), LoadCursor(NULL,IDC_ARROW), (HBRUSH)(COLOR_BTNFACE+1), app->getSmallIcon(),
								NULL, /*app->getApplicationName()*/ L"Abiword");
	
	UT_return_val_if_fail(a, false);

	return true;
}

/*!
 * This code is used by the dynamic menu API to rebuild the menus after a
 * a change in the menu structure.
 */
void XAP_Win32FrameImpl::_rebuildMenus(void)
{
	
	// FIXME - This should do as advertized!  RP
	/*
	// destroy old menu
	
	m_pWin32Menu->destroy();
	DELETEP(m_pWin32Menu);
	
	// build new one.
	m_pWin32Menu = new EV_Win32MenuBar(static_cast<XAP_Win32App*>(XAP_App::getApp()), getFrame(),
					 m_szMenuLayoutName,
					 m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bool bResult = m_pUnixMenu->rebuildMenuBar();
	UT_ASSERT(bResult);
	*/

	return;
	/* Unix version:
	// destroy old menu
	m_pUnixMenu->destroy();
	DELETEP(m_pUnixMenu);
	
	// build new one.
	m_pUnixMenu = new EV_UnixMenuBar(static_cast<XAP_UnixApp*>(XAP_App::getApp()), getFrame(),
					 m_szMenuLayoutName,
					 m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bool bResult = m_pUnixMenu->rebuildMenuBar();
	UT_ASSERT(bResult);
	*/
}

/*****************************************************************/


LRESULT CALLBACK XAP_Win32FrameImpl::_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	XAP_Frame * f = (XAP_Frame*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (!f)
	{
		return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
	}
	
	XAP_Win32FrameImpl * fimpl = static_cast<XAP_Win32FrameImpl *>(f->getFrameImpl());
	UT_return_val_if_fail(fimpl, UT_DefWindowProc(hwnd,iMsg,wParam,lParam));

	AV_View * pView = NULL;

	pView = f->getCurrentView();

	if(iMsg == fimpl->m_mouseWheelMessage)
	{
		wParam = MAKEWPARAM(0, (short)(int)wParam);
		return SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
	}

	switch (iMsg)
	{
	
	case WM_SETFOCUS:
		{
			// on set focus we want to make sure that the kbd layout
			// is the same as it was before we lost focus. If the user
			// was in some other application and changed the layout in
			// there to something else and now returns to AW then we
			// should not be in that different layout (this should not
			// happen as the layouts are supposed to be thread
			// specific, but I noted that sometimes even thought we
			// have the correct layout, the icon on the task bar is
			// out of sync with our layout; this should fix that).
			XAP_Win32App *pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
			UT_ASSERT( pWin32App );

			if(pWin32App)
			{
				ActivateKeyboardLayout(pWin32App->getHKL(),0);
			}
			
			
			// fall through ...
		}
			
	case WM_EXITMENULOOP:
		if (pView)
		{
			pView->focusChange(AV_FOCUS_HERE);
			SetFocus(fimpl->m_hwndContainer);
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
		if (fimpl->m_pWin32Popup)
		{
			if (fimpl->m_pWin32Popup->onCommand(pView,hwnd,wParam))
				return 0;
		}
		else if (fimpl->m_pWin32Menu->onCommand(pView,hwnd,wParam))
		{
			return 0;
		}
		else if (HIWORD(wParam) == 0)
		{
			// after menu passes on it, give each of the toolbars a chance
			UT_uint32 nrToolbars, k;
			nrToolbars = fimpl->m_vecToolbars.getItemCount();
			for (k=0; k < nrToolbars; k++)
			{
				EV_Win32Toolbar * t = (EV_Win32Toolbar *)fimpl->m_vecToolbars.getNthItem(k);
				XAP_Toolbar_Id id = t->ItemIdFromWmCommand(LOWORD(wParam));
				if (t->toolbarEvent(id))
					return 0;
			}
		}
		return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);

	case WM_INITMENU:
		if (fimpl->m_pWin32Popup)
		{
			if (fimpl->m_pWin32Popup->onInitMenu(f,pView,hwnd,(HMENU)wParam))
				return 0;
		}
		else if (fimpl->m_pWin32Menu->onInitMenu(f,pView,hwnd,(HMENU)wParam))
			return 0;
		return UT_DefWindowProc(hwnd,iMsg,wParam,lParam);
		
	case WM_MEASUREITEM: 
	{	

		if (fimpl->m_pWin32Popup)
			(fimpl->m_pWin32Popup->onMeasureItem(hwnd, wParam,lParam));
		else 
			fimpl->m_pWin32Menu->onMeasureItem(hwnd, wParam,lParam);					

		return 0;		
	} 

	case WM_MENUCHAR: 
	{	
		if (fimpl->m_pWin32Popup)
			return (fimpl->m_pWin32Popup->onMenuChar(hwnd, wParam,lParam));
		else 
			return fimpl->m_pWin32Menu->onMenuChar(hwnd, wParam,lParam);					
		
	} 

 	case WM_DRAWITEM: 
 	{

		if (fimpl->m_pWin32Popup)
		{
			fimpl->m_pWin32Popup->onDrawItem(hwnd,wParam,lParam);
		}
		else 
			fimpl->m_pWin32Menu->onDrawItem(hwnd,wParam,lParam);

		return 0;					
	}

	case WM_MENUSELECT:
	{
		if (fimpl->m_pWin32Popup)
		{
			fimpl->m_pWin32Popup->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam);
		}
		else 
			fimpl->m_pWin32Menu->onMenuSelect(f,pView,hwnd,(HMENU)lParam,wParam);

		return 0;					

	}

	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) 
		{
		
		case TBN_DROPDOWN:
		{
			HWND hWnd = ((LPNMHDR) lParam)->hwndFrom;
			EV_Win32Toolbar * t = (EV_Win32Toolbar *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);						
			t->onDropArrow(((LPNMTOOLBARW) lParam)->iItem);			
			Sleep(500); /* At least, half second where the arrow is shown as pressed*/			
			return TBDDRET_DEFAULT;			/* Windows restores the pushed button*/
		}

		case TTN_NEEDTEXTW:
			{
				UT_uint32 nrToolbars, k;
				nrToolbars = fimpl->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)fimpl->m_vecToolbars.getNthItem(k);
					if (t->getToolTip(lParam))
						break;
				}
			}
			break;

		case RBN_HEIGHTCHANGE:
			{
				RECT r;
				GetClientRect(fimpl->m_hwndFrame, &r);
				int nWidth = r.right - r.left;
				int nHeight = r.bottom - r.top;

				GetClientRect(fimpl->m_hwndRebar, &r);
				fimpl->m_iBarHeight = r.bottom - r.top + 6;

				if (fimpl->m_hwndContainer)
				{
					// leave room for the toolbars
					nHeight -= fimpl->m_iBarHeight;

					MoveWindow(fimpl->m_hwndContainer, 0, fimpl->m_iBarHeight, nWidth, nHeight, TRUE);
				}
								
				f->queue_resize();
			}
			break;

		case NM_CUSTOMDRAW:
			{
				LPNMCUSTOMDRAW pNMcd = (LPNMCUSTOMDRAW)lParam;
				UT_uint32 nrToolbars, k;
				nrToolbars = fimpl->m_vecToolbars.getItemCount();
				for (k=0; k < nrToolbars; k++)
				{
					EV_Win32Toolbar * t = (EV_Win32Toolbar *)fimpl->m_vecToolbars.getNthItem(k);
					if( t && (pNMcd->hdr.hwndFrom == t->getWindow()) )
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

							HWND  hWndChild = FindWindowExW( pNMcd->hdr.hwndFrom, NULL, NULL, NULL );
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
								hWndChild = FindWindowExW( pNMcd->hdr.hwndFrom, hWndChild, NULL, NULL );
							}

							/* Don't delete hBr since it was obtained using GetSysColorBrush, so System owned */
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

		UT_DEBUGMSG ((("xap_Win32Frame::WM_SIZE %u - %u\n"), nWidth, nHeight));
		
		if( pView && !pView->isLayoutFilling() )
		{
			fimpl->m_iRealSizeHeight = nHeight;
			fimpl->m_iRealSizeWidth = nWidth;
			
			fimpl->_startViewAutoUpdater();

			if (nWidth != (int) fimpl->m_iSizeWidth && fimpl->m_hwndRebar != NULL)
			{
				MoveWindow(fimpl->m_hwndRebar, 0, 0, nWidth, fimpl->m_iBarHeight, TRUE); 
			}

			// leave room for the toolbars and the status bar
			nHeight -= fimpl->m_iBarHeight;

			if (::IsWindowVisible(fimpl->m_hwndStatusBar))
				nHeight -= fimpl->m_iStatusBarHeight;							
			
				
			if (fimpl->m_hwndStatusBar)
				MoveWindow(fimpl->m_hwndStatusBar, 0, fimpl->m_iBarHeight+nHeight, nWidth, fimpl->m_iStatusBarHeight, TRUE);

			if (fimpl->m_hwndContainer)
				MoveWindow(fimpl->m_hwndContainer, 0, fimpl->m_iBarHeight, nWidth, nHeight, TRUE);			
			
			fimpl->m_iSizeWidth = nWidth;
			fimpl->m_iSizeHeight = nHeight;

			f->updateZoom();
		}

		return 0;
	}

	case WM_CLOSE:
	{
		XAP_App * pApp = XAP_App::getApp();
		UT_return_val_if_fail(pApp,0);

		const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
		UT_return_val_if_fail(pEMC,0);

		EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
		UT_ASSERT_HARMLESS(pEM);						// make sure it's bound to something

		if (pEM)
		{
			pEM->Fn(pView,NULL);
			return 0;
		}

		// let the window be destroyed
		break;
	}

	case WM_INPUTLANGCHANGE:
	{
		UT_DEBUGMSG(("XAP_Win32FrameImpl::_FrameWndProc: received input language change\n"));

		// This will remap the static tables used by all frames.
		// (see the comment in ev_Win32Keyboard.cpp.)
		ev_Win32Keyboard *pWin32Keyboard = static_cast<ev_Win32Keyboard *>(fimpl->m_pKeyboard);
		pWin32Keyboard->remapKeyboard((HKL)lParam);

		// set the language
		XAP_Win32App *pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
		UT_ASSERT_HARMLESS( pWin32App );

		if(pWin32App)
			pWin32App->setKbdLanguage((HKL)lParam);

		// Do not propagate this message.
		
		return 1; //DefWindowProc(hwnd, iMsg, wParam, lParam);
	}

	case WM_MOUSEWHEEL:
	{
		return SendMessageW(fimpl->m_hwndContainer, iMsg, wParam, lParam);
	}

	case WM_SYSCOLORCHANGE:
	{
		if (fimpl->m_hwndRebar)
		{
			SendMessageW(fimpl->m_hwndRebar,WM_SYSCOLORCHANGE,0,0);

			REBARBANDINFOW rbbi;
			memset(&rbbi, 0, sizeof(rbbi));

			rbbi.cbSize = sizeof(REBARBANDINFO);
			rbbi.fMask = RBBIM_COLORS;
			rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
			rbbi.clrBack = GetSysColor(COLOR_BTNFACE);

			UT_uint32 nrToolbars = fimpl->m_vecToolbars.getItemCount();
			for (UT_uint32 k=0; k < nrToolbars; k++)
				SendMessageW(fimpl->m_hwndRebar, RB_SETBANDINFO,k,(LPARAM)&rbbi);
		}

		if (fimpl->m_hwndContainer)
			SendMessageW(fimpl->m_hwndContainer,WM_SYSCOLORCHANGE,0,0);
		if (fimpl->m_hwndStatusBar)
			SendMessageW(fimpl->m_hwndStatusBar,WM_SYSCOLORCHANGE,0,0);
		return 0;
	}

	case WM_DROPFILES:              //TODO: CHECK
		{
			HDROP hDrop = (HDROP) wParam; 
			// How many files were dropped?
			int count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
			WCHAR szFileName[PATH_MAX];
			UT_Win32LocaleString str;
			int i,pathlength;
			for (i=0; i<count; i++)
			{
				pathlength = DragQueryFileW(hDrop, i, NULL, 0);
				if (pathlength < PATH_MAX)
				{
					DragQueryFileW(hDrop, i, szFileName, PATH_MAX);
					XAP_App * pApp = XAP_App::getApp();
					UT_return_val_if_fail(pApp, 0);
					FV_View* pCurrentView = (FV_View *) f->getCurrentView();
					XAP_Frame * pNewFrame = 0;
					IEGraphicFileType iegft = IEGFT_Unknown;					
					IE_ImpGraphic *pIEG;
					FG_ConstGraphicPtr pFG;
					UT_Error errorCode = UT_ERROR;

					/*
						The user may be dropping any kind of file
						Check first if the file is a graphic. If it's a graphic we insert it 
						in the document, if not we assume that it's a document 		
					*/								
					// If there is no import graphic, it's a document...
					str.fromLocale(szFileName);
					char * uri = UT_go_filename_to_uri(str.utf8_str().utf8_str());
					if(uri)
						errorCode = IE_ImpGraphic::constructImporter(uri, iegft, &pIEG);

					if(errorCode == UT_OK)
					{						
						errorCode = pIEG->importGraphic(uri, pFG);

						DELETEP(pIEG);
						if(errorCode == UT_OK && pFG)
						{
						  errorCode = pCurrentView->cmdInsertGraphic(pFG);
						}
					  }

					if (errorCode != UT_OK)
					{	
						// Check if the current document is empty.
						if (f->isDirty() || f->getFilename() ||
							(f->getViewNumber() > 0))
						{
							pNewFrame = pApp->newFrame();
							if (pNewFrame == NULL)
							{
							  errorCode = UT_ERROR;
							  f->setStatusMessage("Could not open another window");
							  g_free(uri);
							  return 0;
							}
						}
						else
						{
							pNewFrame = f;
						}

						if(uri)
							errorCode = pNewFrame->loadDocument(uri, IEFT_Unknown);

						if (errorCode != UT_OK)
						{
							if (f != pNewFrame)
								pNewFrame->close();
						}
						else
						{
							pNewFrame->show();
						}
					  }

					if (errorCode != UT_OK)
					  {
					    s_CouldNotLoadFileMessage(f, uri, errorCode);
					  }
					g_free(uri);
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

	return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
}
