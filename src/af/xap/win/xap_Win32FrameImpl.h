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

	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp) = 0;

	/** public functions replacing replacing usage of XAP_Win32Frame **/
	void updateKeyboardFocus(void)			{  SetFocus(_getTopLevelWindow());  }
	void enableWindowInput(bool bActive)		{  EnableWindow(_getTopLevelWindow(), bActive);  }
	LRESULT sendMsgToFrame(UINT Msg, WPARAM wParam, LPARAM lParam)	{  return SendMessage(_getTopLevelWindow(), Msg, wParam, lParam);  }

	inline HWND getTopLevelWindow(void) const 	{  return _getTopLevelWindow();  }
	inline HWND getToolbarWindow(void) const		{  return m_hwndRebar;  }

protected:
	friend class XAP_Frame;

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
	virtual EV_Toolbar * _newToolbar(XAP_App *app, XAP_Frame *frame, const char *szLayout, const char *szLanguage);
	virtual EV_Menu* _getMainMenu(void);

	// Useful to refresh the size of the Frame.  For instance,
	// when the user selects hide statusbar, the Frame has to be
	// resized in order to fill the gap leaved by the statusbar
	virtual void _queue_resize(void);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y);
	virtual void _setFullScreen(bool isFullScreen);
	virtual bool _openURL(const char * szURL);
	virtual void _nullUpdate (void) const;
	virtual void _setCursor(GR_Graphics::Cursor cursor);

	static bool _RegisterClass(XAP_Win32App * app);

/*** Win32 help functions ***/
	virtual void				_translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
	virtual HWND				_getTopLevelWindow(void) const {  return m_hwndFrame;  }

private:
	HWND						m_hwndFrame; /* the entire window, menu, toolbar, document, etc. */
	HWND						m_hwndRebar;
	HWND						m_hwndContainer; /* the document and all rulers and scroll bars */
	HWND						m_hwndStatusBar;

	AP_Win32DialogFactory			m_dialogFactory;	/* class defined[.h] in XAP, implemented[.cpp] in AP */

	EV_Win32MenuBar *				m_pWin32Menu;
	EV_Win32MenuPopup *			m_pWin32Popup; /* only valid while a context popup is up */
	UT_uint32					m_iBarHeight;
	UT_uint32					m_iRealSizeHeight;
	UT_uint32					m_iRealSizeWidth;
};

#endif /* XAP_WIN32FRAME_H */
