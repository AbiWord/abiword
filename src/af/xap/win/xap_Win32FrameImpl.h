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
	virtual ~XAP_Win32FrameImpl();

	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp) = 0;

protected:
	friend class XAP_Frame;

#if 0
	void _startViewAutoUpdater(void);
	static void viewAutoUpdater(UT_Worker *wkr);
#endif

	virtual bool _updateTitle();

	virtual void _initialize();
	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();

	virtual XAP_DialogFactory * _getDialogFactory();
	virtual EV_Toolbar * _newToolbar(XAP_App *app, XAP_Frame *frame, const char *szLayout, const char *szLanguage);
	virtual EV_Menu* _getMainMenu();
#if 0
	virtual void _createToolbars();
	virtual void _refillToolbarsInFrameData() = 0;
#endif
	virtual void _rebuildToolbar(UT_uint32 ibar);
	// Useful to refresh the size of the Frame.  For instance,
	// when the user selects hide statusbar, the Frame has to be
	// resized in order to fill the gap leaved by the statusbar
	virtual void _queue_resize();

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y);
	virtual void _setFullScreen(bool isFullScreen);
	virtual bool _openURL(const char * szURL);
	virtual void _nullUpdate () const;
	virtual void _setCursor(GR_Graphics::Cursor cursor);

#if formetolookat
	EV_Mouse * m_pMouse;
	EV_Keyboard * m_pKeyboard;
	XAP_FrameMode m_iFrameMode;

	UT_uint32 m_ViewAutoUpdaterID;
	UT_Timer * m_ViewAutoUpdater;

	UT_Vector m_vecToolbarLayoutNames;
	const char * m_szToolbarLabelSetName;	/* language for toolbars */
	const char * m_szToolbarAppearance;
	UT_Vector m_vecToolbars;

	const char * m_szMenuLayoutName;
	const char * m_szMenuLabelSetName;		/* language for menus */

	XAP_Frame * m_pFrame;
#endif


/*** Win32 help functions ***/
	virtual void				_translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;


private:
	HWND						m_hwndFrame; /* the entire window, menu, toolbar, document, etc. */
	AP_Win32DialogFactory			m_dialogFactory;	/* class defined[.h] in XAP, implemented[.cpp] in AP */

	EV_Win32MenuBar *				m_pWin32Menu;
	EV_Win32MenuPopup *			m_pWin32Popup; /* only valid while a context popup is up */
	UT_uint32					m_iBarHeight;
	UT_uint32					m_iRealSizeHeight;
	UT_uint32					m_iRealSizeWidth;
};

#endif /* XAP_WIN32FRAME_H */
