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

	virtual bool				initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue,
										   const char * szMenuLayoutKey, const char * szMenuLayoutDefaultValue,
										   const char * szMenuLabelSetKey, const char * szMenuLabelSetDefaultValue,
										   const char * szToolbarLayoutsKey, const char * szToolbarLayoutsDefaultValue,
										   const char * szToolbarLabelSetKey, const char * szToolbarLabelSetDefaultValue);

	virtual	XAP_Frame *			cloneFrame(void)=0;
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft)=0;
	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew)=0;
	virtual bool				close(void);
	virtual bool				raise(void);
	virtual bool				show(void);
	virtual bool				openURL(const char * szURL);
	virtual bool				updateTitle(void);
	virtual UT_sint32			setInputMode(const char * szName);

	GtkWidget *					getTopLevelWindow(void) const;
	GtkWidget *					getVBoxWidget(void) const;
	virtual XAP_DialogFactory *	getDialogFactory(void);
	virtual void				setXScrollRange(void)=0;
	virtual void				setYScrollRange(void)=0;
	virtual bool				runModalContextMenu(AV_View * pView, const char * szMenuName,
													UT_sint32 x, UT_sint32 y);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y) = 0;
	virtual void				setStatusMessage(const char * szMsg) = 0;

	void						setTimeOfLastEvent(guint32 eventTime);
	
	virtual void				toggleRuler(bool bRulerOn) = 0;
	virtual void				queue_resize();
protected:
	virtual GtkWidget *			_createDocumentWindow(void)=0;
	virtual GtkWidget *			_createStatusBarWindow(void)=0;
	virtual void				_createTopLevelWindow(void);
	virtual void				_setWindowIcon(void) = 0;

	virtual EV_Toolbar *		_newToolbar(XAP_App *app, XAP_Frame *frame, const char *, const char *);
	
	// TODO see why ev_UnixKeyboard has lowercase prefix...
	XAP_UnixApp *				m_pUnixApp;
	EV_UnixMenuBar *			m_pUnixMenu;
	EV_UnixMenuPopup *			m_pUnixPopup; /* only valid while a context popup is up */
	
	GtkWidget *					m_wTopLevelWindow;
	GtkWidget *					m_wVBox;
	GtkWidget * 				m_wSunkenBox;
	GtkWidget *					m_wStatusBar;
	guint                       m_iAbiRepaintID;
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
		static gint abi_expose_repaint( gpointer /* xap_UnixFrame * */ p);
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
};

#endif /* XAP_UNIXFRAME_H */
