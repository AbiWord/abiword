/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 2012 Hubert Figuiere
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


#ifndef XAP_QTFRAMEIMPL_H
#define XAP_QTFRAMEIMPL_H

#include <QTextEdit>
#include <QStatusBar>

#include "xap_FrameImpl.h"
#include "xap_QtDialogFactory.h"

class QMainWindow;
class EV_QtMenuBar;

class XAP_QtFrameImpl
	: public XAP_FrameImpl
{
public:
	XAP_QtFrameImpl(XAP_Frame *pFrame);
	friend class XAP_Frame;
	virtual ~XAP_QtFrameImpl();

	QMainWindow* getTopLevel() const
		{ return m_topLevel; }
	virtual void _createTopLevelWindow();

protected:
	EV_QtMenuBar *		m_pQtMenuBar;
	QTextEdit *		m_wSunkenBox;
	QStatusBar *		m_wStatusBar;

	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();

	virtual void _nullUpdate () const; // a virtual member function in xap_Frame
	virtual void _initialize();

	virtual QTextEdit * _createDocumentWindow() = 0;
	virtual QStatusBar * _createStatusBarWindow() = 0;

	virtual void _setCursor(GR_Graphics::Cursor cursor);

	virtual XAP_DialogFactory * _getDialogFactory();
	virtual EV_Menu * _getMainMenu();
	virtual EV_Toolbar * _newToolbar(XAP_Frame *pFrame,
									 const char *szLayout,
									 const char *szLanguage);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
									  UT_sint32 x, UT_sint32 y);

	virtual void _queue_resize();
	virtual void _rebuildMenus(void);
	virtual void _rebuildToolbar(UT_uint32 ibar);

	virtual void _setFullScreen(bool changeToFullScreen);

private:
	XAP_QtDialogFactory        m_dialogFactory;
	QMainWindow* m_topLevel;
};

#endif
