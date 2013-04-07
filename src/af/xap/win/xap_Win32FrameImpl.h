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


#ifndef XAP_WIN32FRAMEIMPL_H
#define XAP_WIN32FRAMEIMPL_H

#include <windows.h>
#include <winuser.h>
#include "xap_FrameImpl.h"
#include "xap_Win32App.h"
#include "xap_Win32DialogFactory.h"
#include "xap_Win32DragAndDrop.h"

#include "ev_Win32Keyboard.h"
#include "ev_Win32Mouse.h"
#include "ev_Win32Menu.h"
#include "ev_Win32Toolbar.h"


/********************************************************************
*********************************************************************
** This file defines the Win32-platform-specific class for the
** cross-platform application frame helper.  This is used to hold all
** Win32-specific data.  One of these is created for each top-level
** document window.
*********************************************************************
********************************************************************/

class ABI_EXPORT XAP_Win32FrameImpl : public XAP_FrameImpl
{
 public:
	XAP_Win32FrameImpl(XAP_Frame *pFrame);
	virtual ~XAP_Win32FrameImpl(void);

	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame) = 0;

	inline HWND getTopLevelWindow(void) const 	{  return _getTopLevelWindow();  }
	inline HWND getToolbarWindow(void) const		{  return m_hwndRebar;  }

protected:
	friend class XAP_Frame;

	virtual UT_RGBColor getColorSelBackground () const;
	virtual UT_RGBColor getColorSelForeground () const;

#if 0
	void _startViewAutoUpdater(void);
	static void viewAutoUpdater(UT_Worker *wkr);
#endif

	virtual bool _updateTitle(void);

	virtual void _initialize(void);
	virtual bool _close(void);
	virtual bool _raise(void);
	virtual bool _show(void);

	virtual XAP_DialogFactory * _getDialogFactory(void);
	virtual EV_Toolbar * _newToolbar(XAP_Frame *frame, const char *szLayout, const char *szLanguage);
	virtual EV_Menu* _getMainMenu(void);
	virtual void _rebuildMenus(void);

	// Useful to refresh the size of the Frame.  For instance,
	// when the user selects hide statusbar, the Frame has to be
	// resized in order to fill the gap leaved by the statusbar
	virtual void _queue_resize(void);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y);
	virtual void _setFullScreen(bool isFullScreen);
	virtual void _nullUpdate (void) const;
	virtual void _setCursor(GR_Graphics::Cursor cursor);

	static bool _RegisterClass(XAP_Win32App * app);

	/*** Win32 help functions ***/
	virtual void				_translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
	virtual HWND				_getTopLevelWindow(void) const {  return m_hwndFrame;  }

	virtual HWND				_createDocumentWindow(XAP_Frame *pFrame, HWND hwndParent,
								UT_uint32 iLeft, UT_uint32 iTop, UT_uint32 iWidth, UT_uint32 iHeight) = 0;
	virtual HWND				_createStatusBarWindow(XAP_Frame *pFrame, HWND hwndParent,
								UT_uint32 iLeft, UT_uint32 iTop, UT_uint32 iWidth) = 0;
	void						_createTopLevelWindow(void);

	void						_setHwndRebar(HWND hWnd)     {  m_hwndRebar = hWnd;  }
	HWND						_getHwndRebar(void)          {  return m_hwndRebar;  }
	void						_setHwndContainer(HWND hWnd) {  m_hwndContainer = hWnd;  }
	HWND						_getHwndContainer(void)      {  return m_hwndContainer;  }
	void						_setHwndStatusBar(HWND hWnd) {  m_hwndStatusBar = hWnd;  }
	HWND						_getHwndStatusBar(void)      {  return m_hwndStatusBar;  }

	UT_uint32					_getBarHeight(void) { return m_iBarHeight; }
	void 						_setBarHeight(UT_uint32 iBarHeight) { m_iBarHeight = iBarHeight; }
	UT_uint32					_getSizeWidth(void) { return m_iSizeWidth; }
	UT_uint32					_getSizeHeight(void) { return m_iSizeHeight; }

	/** window class related functions **/
	static LRESULT CALLBACK			_FrameWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND						m_hwndFrame; /* the entire window, menu, toolbar, document, etc. */
	HWND						m_hwndRebar;
	HWND						m_hwndContainer; /* the document and all rulers and scroll bars */
	HWND						m_hwndStatusBar;

	AP_Win32DialogFactory			m_dialogFactory;	/* class defined[.h] in XAP, implemented[.cpp] in AP */

	EV_Win32MenuBar *				m_pWin32Menu;
	EV_Win32MenuPopup *			m_pWin32Popup; /* only valid while a context popup is up */
	UT_uint32					m_iBarHeight;
	UT_uint32					m_iStatusBarHeight;
	UT_uint32					m_iRealSizeHeight;
	UT_uint32					m_iRealSizeWidth;
	UT_uint32					m_iWindowStateBeforeFS; /* was Abi maximized/normal/minimized before we went full-screen */
	UT_uint32					m_iWindowXBeforeFS;
	UT_uint32					m_iWindowYBeforeFS;
	UT_uint32					m_iWindowHeightBeforeFS;
	UT_uint32					m_iWindowWidthBeforeFS;

	UINT						m_mouseWheelMessage;
	XAP_Win32DropTarget			m_dropTarget;

	/* These 2 variables are used in the frame
	** to remember the last values we used to set
	** the size of client area of the frame (on
	** a resize of the frame window).  That is,
	** the values we used to calculate the layout
	** of the various toolbars, view, and anything
	** else that goes in the frame window.  We do
	** this because Win32 plays funny games with
	** window sizes (NonClient vs Client coordinates).
	** We do this so that we can short-circuit some
	** of the resizing and the resulting flashing.
	*/
	UT_uint32					m_iSizeWidth;
	UT_uint32					m_iSizeHeight;

public:

	UT_UTF8String				m_sColorBack;
	UT_UTF8String				m_sColorFore;

};

#endif /* XAP_WIN32FRAME_H */
