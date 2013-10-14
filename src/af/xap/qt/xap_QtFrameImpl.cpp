/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 2013 Hubert Figuiere
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

#include <QMainWindow>
#include <QMenuBar>

#include "ev_QtKeyboard.h"
#include "ev_QtMouse.h"
#include "xap_QtApp.h"
#include "xap_QtFrameImpl.h"
#include "ev_QtMenuBar.h"
#include "ev_QtToolbar.h"

XAP_QtFrameImpl::XAP_QtFrameImpl(XAP_Frame *pFrame)
	: XAP_FrameImpl(pFrame)
	, m_dialogFactory(XAP_App::getApp(), pFrame)
	, m_pQtMenuBar(NULL)
	, m_topLevel(NULL)
{
}

XAP_QtFrameImpl::~XAP_QtFrameImpl()
{
	delete m_pQtMenuBar;
	delete m_topLevel;
}

bool XAP_QtFrameImpl::_close()
{
#warning TODO implement
}

bool XAP_QtFrameImpl::_raise()
{
#warning TODO implement
}

bool XAP_QtFrameImpl::_show()
{
#warning TODO implement
}

void XAP_QtFrameImpl::_nullUpdate () const
{
#warning TODO implement
}

void XAP_QtFrameImpl::_initialize()
{
	// get a handle to our keyboard binding mechanism
 	// and to our mouse binding mechanism.
 	EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
 	UT_ASSERT(pEEM);

	m_pKeyboard = new EV_QtKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new EV_QtMouse(pEEM);
	UT_ASSERT(m_pMouse);
}

void XAP_QtFrameImpl::_setCursor(GR_Graphics::Cursor cursor)
{
#warning TODO implement
}

XAP_DialogFactory * XAP_QtFrameImpl::_getDialogFactory()
{
	return &m_dialogFactory;
}

void XAP_QtFrameImpl::_createTopLevelWindow()
{
	if(m_iFrameMode == XAP_NormalFrame) {
		m_topLevel = new QMainWindow(NULL, 0);
		m_topLevel->setWindowTitle(XAP_App::getApp()->getApplicationTitleForTitleBar());
		m_topLevel->showMaximized();
	}

	if (m_iFrameMode != XAP_NoMenusWindowLess) {
		// synthesize a menu from the info in our base class.
		m_pQtMenuBar = new EV_QtMenuBar(static_cast<XAP_QtApp*>(XAP_App::getApp()), getFrame(), m_szMenuLayoutName,
										 m_szMenuLabelSetName);
		UT_return_if_fail(m_pQtMenuBar);
		UT_DebugOnly<bool> bResult;
		bResult = m_pQtMenuBar->synthesizeMenuBar();
		UT_ASSERT(bResult);
	}

	/* If refactoring the toolbars code, please make sure that toolbars
	 * are created AFTER the main menu bar has been synthesized, otherwise
	 * the embedded build will stop working
	 */
	if(m_iFrameMode == XAP_NormalFrame)
	{
		_createToolbars();
	}

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).
	m_wSunkenBox = _createDocumentWindow();
	m_topLevel->setCentralWidget(m_wSunkenBox);

	m_wStatusBar = NULL;

#ifdef ENABLE_STATUSBAR
	if(m_iFrameMode == XAP_NormalFrame)
		m_wStatusBar = _createStatusBarWindow();
#endif

	if (m_wStatusBar)
	{
		m_topLevel->setStatusBar(m_wStatusBar);
	}
}

EV_Menu * XAP_QtFrameImpl::_getMainMenu()
{
#warning TODO implement
}

EV_Toolbar * XAP_QtFrameImpl::_newToolbar(XAP_Frame *pFrame,
					  const char *szLayout,
					  const char *szLanguage)
{
	EV_QtToolbar *pToolbar = NULL;
	pToolbar = new EV_QtToolbar(static_cast<XAP_QtApp *>(XAP_App::getApp()), pFrame, szLayout, szLanguage);
	return pToolbar;
}

bool XAP_QtFrameImpl::_runModalContextMenu(AV_View * pView, const char * szMenuName,
					   UT_sint32 x, UT_sint32 y)
{
#warning TODO implement
}

void XAP_QtFrameImpl::_queue_resize()
{
#warning TODO implement
}

void XAP_QtFrameImpl::_rebuildMenus(void)
{
#warning TODO implement
}

void XAP_QtFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
#warning TODO implement
}

void XAP_QtFrameImpl::_setFullScreen(bool changeToFullScreen)
{
#warning TODO implement
}

