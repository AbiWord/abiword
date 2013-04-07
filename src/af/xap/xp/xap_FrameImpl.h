/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 William Lachance
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

#ifndef XAP_FrameImpl_H
#define XAP_FrameImpl_H

#include "ut_types.h"
#include "ut_vector.h"
#include "gr_Graphics.h"

#define MAX_TITLE_LENGTH 256

class XAP_Frame;
class XAP_App;
class XAP_DialogFactory;
class EV_Menu;
class EV_Toolbar;
class EV_Mouse;
class EV_Keyboard;
class UT_Timer;
class UT_Worker;
class GR_Graphics;
class AV_View;

typedef enum _FrameModes
{
	XAP_NormalFrame,	// Normal Frame
	XAP_NoMenusWindowLess,	// No toplevel window or menus
	XAP_WindowLess // No toplevel window but menus are OK.
} XAP_FrameMode;

class ABI_EXPORT XAP_FrameImpl
{
public:
	XAP_Frame*	getFrame() { return m_pFrame; };	/* needed for Obj-C access */
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame) = 0;
	const UT_GenericVector<EV_Toolbar*> & _getToolbars() const
					{ return m_vecToolbars; };

	virtual void                notifyViewChanged(AV_View * pView); // default dows nothing

protected:
	XAP_FrameImpl(XAP_Frame *pFrame);
	virtual ~XAP_FrameImpl(void);

	friend class XAP_Frame;

	void _startViewAutoUpdater(void);
	static void viewAutoUpdater(UT_Worker *wkr);

	virtual bool _updateTitle();

	virtual void _initialize() = 0;
	virtual bool _close() = 0;
	virtual bool _raise() = 0;
	virtual bool _show() = 0;

	virtual XAP_DialogFactory * _getDialogFactory() = 0;
	virtual EV_Toolbar * _newToolbar(XAP_Frame *frame, const char *szLayout, const char *szLanguage) = 0;
	virtual EV_Menu* _getMainMenu() = 0;
	virtual void _rebuildMenus(void)= 0;
	virtual void _createToolbars();
	virtual void _refillToolbarsInFrameData() = 0;
	virtual void _rebuildToolbar(UT_uint32 ibar) = 0;
	// Useful to refresh the size of the Frame.  For instance,
	// when the user selects hide statusbar, the Frame has to be
	// resized in order to fill the gap leaved by the statusbar
	virtual void _queue_resize() = 0;

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y) = 0;
	virtual void _setFullScreen(bool isFullScreen) = 0;
	virtual void _hideMenuScroll(bool bHideMenuScroll) = 0;
	virtual void _nullUpdate () const = 0;
	virtual void _setCursor(GR_Graphics::Cursor cursor) = 0;

	virtual UT_RGBColor getColorSelBackground () const;
	virtual UT_RGBColor getColorSelForeground () const;

	EV_Mouse * m_pMouse;
	EV_Keyboard * m_pKeyboard;
	XAP_FrameMode m_iFrameMode;

	UT_uint32 m_ViewAutoUpdaterID;
	UT_Timer * m_ViewAutoUpdater;

	UT_Vector m_vecToolbarLayoutNames;
	const char * m_szToolbarLabelSetName;	/* language for toolbars */
	const char * m_szToolbarAppearance;
	UT_GenericVector<EV_Toolbar*> m_vecToolbars;

	const char * m_szMenuLayoutName;
	const char * m_szMenuLabelSetName;		/* language for menus */

	virtual void dragText() {}

private:
	XAP_Frame * m_pFrame;
};

#endif
