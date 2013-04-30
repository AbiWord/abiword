/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
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

#include <QMenuBar>
#include <QMainWindow>

#include "ap_QtFrame.h"
#include "ap_QtFrameImpl.h"

AP_QtFrameImpl::AP_QtFrameImpl(AP_QtFrame *pQtFrame)
	: XAP_QtFrameImpl(pQtFrame)
{
}

AP_QtFrameImpl::~AP_QtFrameImpl()
{
}

XAP_FrameImpl * AP_QtFrameImpl::createInstance(XAP_Frame *pFrame)
{
	return new AP_QtFrameImpl(static_cast<AP_QtFrame*>(pFrame));
}

void AP_QtFrameImpl::_createWindow()
{
	_createTopLevelWindow();

// show?

	if(getFrame()->isMenuScrollHidden())
	{
	    _hideMenuScroll(true);
	}
}

void AP_QtFrameImpl::_hideMenuScroll(bool bHideMenuScroll)
{
	QWidget* menubar = getTopLevel()->menuBar();
#warning TODO implement scrollbar
	QWidget* scrollbar = NULL;
	if(menubar) {
		bHideMenuScroll ? menubar->hide() : menubar->show();
	}
	if(scrollbar) {
		bHideMenuScroll ? scrollbar->hide() : scrollbar->show();
	}
}

void AP_QtFrameImpl::_refillToolbarsInFrameData()
{
#warning TODO implement
}
