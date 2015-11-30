/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef EV_UNIXTOOLBAR_H
#define EV_UNIXTOOLBAR_H

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"
#include "ap_Toolbar_Id.h"
#include "ap_UnixToolbar_StyleCombo.h"
#include "xap_FontPreview.h"

class XAP_UnixApp;
class XAP_Frame;
class EV_UnixToolbar_ViewListener;

class _wd;

class EV_UnixToolbar : public EV_Toolbar
{
public:
	EV_UnixToolbar(XAP_UnixApp * pUnixApp,
		       XAP_Frame *pFrame,
		       const char * szToolbarLayoutName,
		       const char * szToolbarLabelSetName);

	virtual ~EV_UnixToolbar(void);

	bool toolbarEvent(_wd * wd, const UT_UCSChar * pData, UT_uint32 dataLength);
	virtual bool synthesize(void);
	bool bindListenerToView(AV_View * pView);
	virtual bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);
	virtual bool repopulateStyles(void);
	UT_sint32 destroy(void);
	void      rebuildToolbar(UT_sint32 oldpos);
	XAP_UnixApp *	getApp(void);
	XAP_Frame * getFrame(void);
	void setCurrentEvent(GdkEvent * event) {m_eEvent = event;}
	virtual void show(void);
	virtual void hide(void);

	XAP_FontPreview *				m_pFontPreview;
	gint							m_pFontPreviewPositionX;
protected:
	virtual GtkToolbarStyle 		getStyle(void);
	virtual bool 					getDetachable(void) { return true; }
	virtual void 					setDetachable(gboolean /*detachable*/) { /* only the GNOME version does that ATM */ }

	virtual GtkBox*					_getContainer();

	void							_releaseListener(void);

	XAP_UnixApp *					m_pUnixApp;
	XAP_Frame *						m_pFrame;
	EV_UnixToolbar_ViewListener *	m_pViewListener;
	AV_ListenerId					m_lid;	/* view listener id */

	GdkEvent *                      m_eEvent;
	GtkWidget *						m_wToolbar;
	GtkSizeGroup *					m_wHSizeGroup;
	GtkSizeGroup *					m_wVSizeGroup;
	UT_GenericVector<_wd*>			m_vecToolbarWidgets;
};

#endif /* EV_UNIXTOOLBAR_H */
