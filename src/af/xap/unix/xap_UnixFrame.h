/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef XAP_UNIXFRAME_H
#define XAP_UNIXFRAME_H

#include <gtk/gtk.h>
#include "xap_Frame.h"
#include "ut_vector.h"
#include "xap_UnixDialogFactory.h"
class XAP_UnixApp;
class ev_UnixKeyboard;
class EV_UnixMouse;
class EV_UnixMenuBar;
class EV_UnixMenuPopup;

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** unix-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class XAP_UnixFrame : public XAP_Frame
{
public:
	XAP_UnixFrame(XAP_UnixApp * app);
	XAP_UnixFrame(XAP_UnixFrame * f);
	virtual ~XAP_UnixFrame(void);

	virtual UT_Bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void)=0;
	virtual UT_Bool				loadDocument(const char * szFilename)=0;
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);
	virtual UT_Bool				updateTitle(void);
	virtual UT_sint32			setInputMode(const char * szName);

	GtkWidget *					getTopLevelWindow(void) const;
	GtkWidget *					getVBoxWidget(void) const;
	EV_UnixMouse *				getUnixMouse(void);
	ev_UnixKeyboard *			getUnixKeyboard(void);

	virtual AP_DialogFactory *	getDialogFactory(void);
	virtual void				setXScrollRange(void)=0;
	virtual void				setYScrollRange(void)=0;
	virtual UT_Bool				runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
	
protected:
	virtual GtkWidget *			_createDocumentWindow(void)=0;
	virtual void				_createTopLevelWindow(void);

	// TODO see why ev_UnixKeyboard has lowercase prefix...
	XAP_UnixApp *				m_pUnixApp;
	ev_UnixKeyboard *			m_pUnixKeyboard;
	EV_UnixMouse *				m_pUnixMouse;
	EV_UnixMenuBar *			m_pUnixMenu;
	EV_UnixMenuPopup *			m_pUnixPopup; /* only valid while a context popup is up */
	UT_Vector					m_vecUnixToolbars;
	
	GtkWidget *					m_wTopLevelWindow;
	GtkWidget *					m_wVBox;
	GtkWidget * 				m_wSunkenBox;

	AP_UnixDialogFactory		m_dialogFactory;

protected:

	class _fe
	{
	public:
		static gint button_press_event(GtkWidget * w, GdkEventButton * e);
		static gint button_release_event(GtkWidget * w, GdkEventButton * e);
		static gint configure_event(GtkWidget* w, GdkEventConfigure *e);
		static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e);
		static gint key_press_event(GtkWidget* w, GdkEventKey* e);
		static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static gint expose(GtkWidget * w, GdkEventExpose* pExposeEvent);
		static void vScrollChanged(GtkAdjustment * w, gpointer /*data*/);
		static void hScrollChanged(GtkAdjustment * w, gpointer /*data*/);
		static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
	};

};

#endif /* XAP_UNIXFRAME_H */
