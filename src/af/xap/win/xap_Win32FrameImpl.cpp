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

#include "xap_ViewListener.h"
#include "ev_EditMethod.h"


XAP_Win32FrameImpl::XAP_Win32FrameImpl(XAP_Frame *pFrame) :
	XAP_FrameImpl(pFrame),
	m_hwndFrame(NULL)
{
}

XAP_Win32FrameImpl::~XAP_Win32FrameImpl()
{
}


#if 0
void XAP_Win32FrameImpl::_startViewAutoUpdater(void) {}
static void XAP_Win32FrameImpl::viewAutoUpdater(UT_Worker *wkr) {}

bool XAP_Win32FrameImpl::_updateTitle() {super._updateTitle()}
#endif


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
	return NULL;
}

EV_Toolbar * XAP_Win32FrameImpl::_newToolbar(XAP_App *app, XAP_Frame *frame, const char *szLayout, const char *szLanguage)
{
	return NULL;
}

EV_Menu* XAP_Win32FrameImpl::_getMainMenu()
{
	return NULL;
}


#if 0
void XAP_Win32FrameImpl::_createToolbars() {}
void _refillToolbarsInFrameData() {}
#endif


void XAP_Win32FrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
}

// Useful to refresh the size of the Frame.  For instance,
// when the user selects hide statusbar, the Frame has to be
// resized in order to fill the gap leaved by the statusbar
void XAP_Win32FrameImpl::_queue_resize()
{
}


bool XAP_Win32FrameImpl::_runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y)
{
	return false;
}

void XAP_Win32FrameImpl::_setFullScreen(bool isFullScreen)
{
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
}

void XAP_Win32FrameImpl::_setCursor(GR_Graphics::Cursor cursor)
{
}

