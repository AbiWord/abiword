/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include "xap_Win32FrameImpl.h"

#include "ut_debugmsg.h"
#include "xap_ViewListener.h"
#include "ev_EditMethod.h"

#include <limits.h>					/* for INT_MAX */


XAP_Win32FrameImpl::XAP_Win32FrameImpl(XAP_Frame *pFrame) :
	XAP_FrameImpl(pFrame),
	m_hwndFrame(NULL),
	m_dialogFactory(pFrame, pFrame->getApp()),
	m_pWin32Menu(NULL),
	m_pWin32Popup(NULL),
	m_iBarHeight(0),
	m_iRealSizeWidth(0),
	m_iRealSizeHeight(0)
{
}

XAP_Win32FrameImpl::~XAP_Win32FrameImpl()
{
}


#if 0
void XAP_Win32FrameImpl::_startViewAutoUpdater(void) {}
static void XAP_Win32FrameImpl::viewAutoUpdater(UT_Worker *wkr) {}
#endif


bool XAP_Win32FrameImpl::_updateTitle() 
{
	UT_return_val_if_fail(m_hwndFrame, false);
	UT_return_val_if_fail(m_pFrame, false);
	UT_return_val_if_fail(m_pFrame->getApp(), false);

	if (!m_pFrame->updateTitle())
	{
		// no relevant change, so skip it
		return false;
	}

	UT_String sTmp = m_pFrame->getTitle(INT_MAX);
	sTmp += " - ";
	sTmp += m_pFrame->getApp()->getApplicationTitleForTitleBar();
	
	SetWindowText(m_hwndFrame, sTmp.c_str());

	return true;
}

void XAP_Win32FrameImpl::_initialize()
{
	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	EV_EditEventMapper * pEEM = m_pFrame->getEditEventMapper();
	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_Win32Keyboard(pEEM);
	UT_ASSERT(m_pKeyboard);
	
	m_pMouse = new EV_Win32Mouse(pEEM);
	UT_ASSERT(m_pMouse);
}

bool XAP_Win32FrameImpl::_close()
{
	UT_return_val_if_fail(m_hwndFrame, false);
	UT_return_val_if_fail(m_pFrame, false);
	UT_return_val_if_fail(m_pFrame->getApp(), false);

	// NOTE: This may not be the proper place, but it does mean that the
	// last window closed is the one that the window state is stored from.
	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT); // must do
	if (GetWindowPlacement(m_hwndFrame, &wndPlacement))
	{
		m_pFrame->getApp()->setGeometry(wndPlacement.rcNormalPosition.left, 
				wndPlacement.rcNormalPosition.top, 
				wndPlacement.rcNormalPosition.right - wndPlacement.rcNormalPosition.left,
				wndPlacement.rcNormalPosition.bottom - wndPlacement.rcNormalPosition.top,
				wndPlacement.showCmd
				);
	}
	else
	{
		// if failed to get placement then invalidate stored settings
		m_pFrame->getApp()->setGeometry(0,0,0,0,0);
	}
	
	RevokeDragDrop(m_hwndFrame);
	

	// NOTE: this should only be called from the closeWindow edit method
	DestroyWindow(m_hwndFrame);
	m_hwndFrame = NULL;

	return true;
}

bool XAP_Win32FrameImpl::_raise()
{
	UT_return_val_if_fail(m_hwndFrame, false);
	BringWindowToTop(m_hwndFrame);
	return true;
}

bool XAP_Win32FrameImpl::_show()
{
	UT_return_val_if_fail(m_hwndFrame, false);

	ShowWindow(m_hwndFrame, SW_SHOW);
	UpdateWindow(m_hwndFrame);

	return true;
}


XAP_DialogFactory * XAP_Win32FrameImpl::_getDialogFactory()
{
	return &m_dialogFactory;
}

EV_Toolbar * XAP_Win32FrameImpl::_newToolbar(XAP_App *app, XAP_Frame *frame, const char *szLayout, const char *szLanguage)
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

EV_Menu* XAP_Win32FrameImpl::_getMainMenu()
{
	return m_pWin32Menu;
}


#if 0
void XAP_Win32FrameImpl::_createToolbars() {}
void _refillToolbarsInFrameData() {}
#endif


void XAP_Win32FrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	// TODO: currently does nothing
}

// Useful to refresh the size of the Frame.  For instance,
// when the user selects hide statusbar, the Frame has to be
// resized in order to fill the gap leaved by the statusbar
void XAP_Win32FrameImpl::_queue_resize()
{
	::SendMessage(m_hwndFrame, WM_SIZE, 0, MAKELONG(m_iRealSizeWidth, m_iRealSizeHeight));
}


bool XAP_Win32FrameImpl::_runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y)
{
	bool bResult = false;

	UT_return_val_if_fail((m_pWin32Popup==NULL), false);
	UT_return_val_if_fail(m_pFrame, false);
	UT_return_val_if_fail(m_pFrame->getApp(), false);

	m_pWin32Popup = new EV_Win32MenuPopup(static_cast<XAP_Win32App*>(m_pFrame->getApp()),szMenuName,m_szMenuLabelSetName);
	if (m_pWin32Popup && m_pWin32Popup->synthesizeMenuPopup(m_pFrame))
	{
		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));

		_translateDocumentToScreen(x,y);

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

void XAP_Win32FrameImpl::_setFullScreen(bool isFullScreen)
{
	// TODO: currently does nothing
}


bool XAP_Win32FrameImpl::_openURL(const char * szURL)
{
	// NOTE: could get finer control over browser window via DDE 
	// NOTE: may need to fallback to WinExec for old NSCP versions

	int res = (int) ShellExecute(m_hwndFrame /*(HWND) top level window */, "open", szURL, NULL, NULL, SW_SHOWNORMAL);

	// TODO: more specific (and localized) error messages ??
	if (res <= 32)	// show error message if failed to launch browser to display URL
	{
		UT_String errMsg = "Error ("; 
		errMsg += res;  errMsg += ") displaying URL: \n";
		errMsg += " [ ";  errMsg += szURL;  errMsg += " ] ";
		MessageBox(m_hwndFrame, errMsg.c_str(), "Error displaying URL", MB_OK|MB_ICONEXCLAMATION);
	}

	return (res>32);
}


void XAP_Win32FrameImpl::_nullUpdate () const
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

void XAP_Win32FrameImpl::_setCursor(GR_Graphics::Cursor cursor)
{
	// TODO: currently does nothing
}

