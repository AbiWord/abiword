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
#include <QGraphicsView>

#include "ev_QtKeyboard.h"
#include "ev_QtMouse.h"
#include "xap_QtApp.h"
#include "xap_QtFrameImpl.h"

XAP_QtFrameImpl::XAP_QtFrameImpl(XAP_Frame *pFrame)
	: XAP_FrameImpl(pFrame)
	, m_dialogFactory(XAP_App::getApp(), pFrame)
	, m_topLevel(NULL)
{
}

XAP_QtFrameImpl::~XAP_QtFrameImpl()
{
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
		QGraphicsView* centralWidget = new QGraphicsView(m_topLevel);
		m_topLevel->setCentralWidget(centralWidget);
		m_topLevel->show();
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
#warning TODO implement
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

