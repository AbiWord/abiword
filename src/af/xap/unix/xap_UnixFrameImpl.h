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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef XAP_UNIXFRAMEIMPL_H
#define XAP_UNIXFRAMEIMPL_H

#include <gtk/gtkwidget.h>
#include <gtk/gtkadjustment.h>
#include <gdk/gdktypes.h>
#include "xap_FrameImpl.h"
#include "ut_vector.h"
#include "xap_UnixDialogFactory.h"
#include "xap_UnixApp.h"

class EV_UnixMenuBar;
class EV_UnixMenuPopup;

/********************************************************************
*********************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application frame helper.  This is used to hold all
** unix-specific data.  One of these is created for each top-level
** document window.
*********************************************************************
********************************************************************/

class XAP_UnixFrameImpl : public XAP_FrameImpl
{
 public:
	XAP_UnixFrameImpl(XAP_Frame *pFrame, XAP_UnixApp *pUnixApp);
	friend class XAP_Frame;
	virtual ~XAP_UnixFrameImpl();

	GtkWidget * getTopLevelWindow() const;
	void setTopLevelWindow(GtkWidget * window) { m_wTopLevelWindow = window; }
	void createTopLevelWindow(void);
	GtkWidget * getVBoxWidget() const;

	void setShowDocLocked(bool bShowDocLocked) { m_bShowDocLocked = bShowDocLocked; }
	bool getShowDocLocked() { return m_bShowDocLocked; }

protected:
	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();

	virtual void _nullUpdate () const; // a virtual member function in xap_Frame
	virtual void _initialize();
	
	virtual void _setWindowIcon() = 0; // should eventually be handled be the inherited helper

	virtual GtkWidget * _createDocumentWindow() = 0;
	virtual GtkWidget * _createStatusBarWindow() = 0;

	bool _updateTitle();
	UT_sint32 _setInputMode(const char * szName);
	virtual void _setCursor(GR_Graphics::Cursor cursor);
	
	virtual XAP_DialogFactory * _getDialogFactory();
	virtual EV_Menu * _getMainMenu();
	virtual EV_Toolbar * _newToolbar(XAP_App *pApp, XAP_Frame *pFrame,
				 const char *szLayout,
				 const char *szLanguage);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
					  UT_sint32 x, UT_sint32 y);
	void setTimeOfLastEvent(guint32 eventTime);
	
	virtual void _queue_resize();
	void _rebuildMenus(void);
	void _rebuildToolbar(UT_uint32 ibar);
	GtkWidget * _getSunkenBox(void) {return m_wSunkenBox;}

	virtual bool _openURL(const char * szURL);
	virtual void _setFullScreen(bool changeToFullScreen);

	class _fe
        {
	friend class XAP_Frame;
	public:
		static gint button_press_event(GtkWidget * w, GdkEventButton * e);
		static gint button_release_event(GtkWidget * w, GdkEventButton * e);
		static gint configure_event(GtkWidget* w, GdkEventConfigure *e);
		static gint motion_notify_event(GtkWidget* w, GdkEventMotion* e);
		static gint scroll_notify_event(GtkWidget* w, GdkEventScroll* e);
		static gint key_press_event(GtkWidget* w, GdkEventKey* e);
		static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
		static gint expose(GtkWidget * w, GdkEventExpose* pExposeEvent);
		static gint abi_expose_repaint( gpointer /* xap_UnixFrame * */ p);
		static gint do_ZoomUpdate( gpointer /* xap_UnixFrame * */ p);
		static void vScrollChanged(GtkAdjustment * w, gpointer /*data*/);
		static void hScrollChanged(GtkAdjustment * w, gpointer /*data*/);
		static void destroy (GtkWidget * /*widget*/, gpointer /*data*/);
		static gboolean focus_in_event(GtkWidget *w,GdkEvent *event,gpointer user_data);
		static gboolean focus_out_event(GtkWidget *w,GdkEvent *event,gpointer user_data);

		static void realize(GtkWidget * widget, GdkEvent */* e*/,gpointer /*data*/);
		static void unrealize(GtkWidget * widget, GdkEvent */* e */,gpointer /* data */);
		static void sizeAllocate(GtkWidget * widget, GdkEvent */* e */,gpointer /* data */);
		static gint focusIn(GtkWidget * widget, GdkEvent */* e */,gpointer /* data */);
		static gint focusOut(GtkWidget * /*widget*/, GdkEvent */* e */,gpointer /* data */);
	};
	friend class _fe;

	bool m_bShowDocLocked;

 private:
	bool                        m_bDoZoomUpdate;
	UT_sint32                   m_iNewWidth;
	UT_sint32                   m_iNewHeight;
	guint                       m_iZoomUpdateID;
	guint                       m_iAbiRepaintID;

	GtkWidget *		    m_wTopLevelWindow;
	GtkWidget *		    m_wVBox;
	GtkWidget * 		    m_wSunkenBox;
	GtkWidget *		    m_wStatusBar;

	XAP_UnixApp *				m_pUnixApp;
	EV_UnixMenuBar *			m_pUnixMenu;
	EV_UnixMenuPopup *			m_pUnixPopup; /* only valid while a context popup is up */
	AP_UnixDialogFactory		        m_dialogFactory;
};
#endif /* XAP_UNIXFRAME_H */





