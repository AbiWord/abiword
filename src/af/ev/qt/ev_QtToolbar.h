/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef EV_QTTOOLBAR_H
#define EV_QTTOOLBAR_H

#include <vector>
#include <QEvent>
#include <QWidget>
#include <QBoxLayout>
#include <QToolBar>
#include <Qt>

#include "ev_Toolbar.h"
#include "xav_Listener.h"
#include "xap_FontPreview.h"
#include "xap_Strings.h"

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "xav_Listener.h"
#include "ap_Toolbar_Id.h"

class XAP_QtApp;
class XAP_Frame;
class EV_QtToolbar_ViewListener;

class _wd;

class EV_QtToolbar : public EV_Toolbar
{
public:
	EV_QtToolbar(XAP_QtApp * pQtApp,
		       XAP_Frame *pFrame,
		       const char * szToolbarLayoutName,
		       const char * szToolbarLabelSetName);

	virtual ~EV_QtToolbar(void);

	bool toolbarEvent(_wd * wd, const UT_UCSChar * pData, UT_uint32 dataLength);
	virtual bool synthesize(void);
	bool bindListenerToView(AV_View * pView);
	virtual bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);
	virtual bool repopulateStyles(void);
	UT_sint32 destroy(void);
	void      rebuildToolbar(UT_sint32 oldpos);
	XAP_QtApp *	getApp(void);
	XAP_Frame * getFrame(void);
	void setCurrentEvent(QEvent * event) {m_eEvent = event;}
	virtual void show(void);
	virtual void hide(void);

	XAP_FontPreview *				m_pFontPreview;
	gint						m_pFontPreviewPositionX;
protected:
	virtual Qt::ToolButtonStyle			getStyle(void);
	virtual bool					getDetachable(void) { return true; }
	virtual void					setDetachable(gboolean /*detachable*/) { /* only the GNOME version does that ATM */ }

	virtual QBoxLayout*				_getContainer();

	void							_releaseListener(void);

	XAP_QtApp *						m_pQtApp;
	XAP_Frame *						m_pFrame;
	AV_ListenerId						m_lid;	/* view listener id */

	QEvent *						m_eEvent;
	QToolBar *						m_wToolbar;
	QWidget *						m_wHandleBox;
	std::vector<_wd*>					m_vecToolbarWidgets;
};

#endif /* EV_QTTOOLBAR_H */
