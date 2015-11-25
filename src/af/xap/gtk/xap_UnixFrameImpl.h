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


#ifndef XAP_UNIXFRAMEIMPL_H
#define XAP_UNIXFRAMEIMPL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
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
	XAP_UnixFrameImpl(XAP_Frame *pFrame);
	friend class XAP_Frame;

	virtual ~XAP_UnixFrameImpl();

	GtkWidget * getTopLevelWindow() const;
	void setTopLevelWindow(GtkWidget * window) { m_wTopLevelWindow = window; }
	GtkWidget * getVBoxWidget() const;
	gint getNewX(void)
	{ return m_iNewX;}
	gint getNewY(void)
	{ return m_iNewY;}
	void focusIMIn ();
	void focusIMOut ();
	void queueIMReset () {
	  need_im_reset = true;
	}
	void resetIMContext ();

	virtual GtkWidget * getViewWidget (void) const = 0;

private:
	void _setGeometry ();

protected:
	GtkIMContext *		    m_imContext;
	GtkWidget *		    m_wVBox;

	GtkWidget * 		    m_wSunkenBox;
	GtkWidget *		    m_wStatusBar;

	GtkWidget *		    m_wTopLevelWindow;
	EV_UnixMenuBar *	    m_pUnixMenu;

	bool need_im_reset;

	GtkIMContext * getIMContext();

	virtual bool _close();
	virtual bool _raise();
	virtual bool _show();

	virtual GtkWidget *  _createInternalWindow (void);

	virtual void _nullUpdate () const; // a virtual member function in xap_Frame
	virtual void _initialize();

	virtual void _setWindowIcon() = 0; // should eventually be handled be the inherited helper

	virtual GtkWidget * _createDocumentWindow() = 0;
	virtual GtkWidget * _createStatusBarWindow() = 0;

	void _createTopLevelWindow(void);
	bool _updateTitle();
	void _createIMContext(GdkWindow* w);
	UT_sint32 _setInputMode(const char * szName);
	virtual void _setCursor(GR_Graphics::Cursor cursor);

	virtual XAP_DialogFactory * _getDialogFactory();
	virtual EV_Menu * _getMainMenu();
	virtual EV_Toolbar * _newToolbar(XAP_Frame *pFrame,
				 const char *szLayout,
				 const char *szLanguage);

	virtual bool _runModalContextMenu(AV_View * pView, const char * szMenuName,
					  UT_sint32 x, UT_sint32 y);
	void setTimeOfLastEvent(guint32 eventTime);

	virtual void _queue_resize();
	virtual void _rebuildMenus(void);
	virtual void _rebuildToolbar(UT_uint32 ibar);
	GtkWidget * _getSunkenBox(void) {return m_wSunkenBox;}

	virtual void _setFullScreen(bool changeToFullScreen);

	void _imCommit (GtkIMContext * imc, const gchar * text);

	virtual void dragText();

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	// need to be able to get at this from XAP_UnixHildonApp
  public:
#endif
	static void _imCommit_cb(GtkIMContext *imc, const gchar* text, gpointer data);
	static void _imPreeditStart_cb (GtkIMContext *context, gpointer data);
	static void _imPreeditChanged_cb (GtkIMContext *context, gpointer data);
	static void _imPreeditEnd_cb (GtkIMContext *context, gpointer data);
	static gint _imRetrieveSurrounding_cb (GtkIMContext *context, gpointer data);
	static gint _imDeleteSurrounding_cb (GtkIMContext *slave, gint offset, gint n_chars, gpointer data);


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
			static gint key_release_event(GtkWidget* w, GdkEventKey* e);
			static gint delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/);
			static gint draw(GtkWidget * w, cairo_t * cr);
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

 private:
	bool                        m_bDoZoomUpdate;
	UT_sint32                   m_iNewX;
	UT_sint32                   m_iNewY;
	UT_sint32                   m_iNewWidth;
	UT_sint32                   m_iNewHeight;
	guint                       m_iZoomUpdateID;
	guint                       m_iAbiRepaintID;


	EV_UnixMenuPopup *			m_pUnixPopup; /* only valid while a context popup is up */
	AP_UnixDialogFactory        m_dialogFactory;

	UT_uint32                   m_iPreeditLen;
	UT_uint32                   m_iPreeditStart;
};
#endif /* XAP_UNIXFRAME_H */



