/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <gnome.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_UnixGnomeApp.h"
#include "xap_UnixGnomeFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "ev_UnixGnomeMenuBar.h"
#include "ev_UnixGnomeToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"

// #include "ie_types.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/

/* Each time that a object is dropped in a widget, this function is called */
void XAP_UnixGnomeFrame::_dnd_drop_event(GtkWidget        *widget,
										 GdkDragContext   * /*context*/,
										 gint              /*x*/,
										 gint              /*y*/,
										 GtkSelectionData *selection_data,
										 guint             info,
										 guint             /*time*/)
{
	XAP_UnixFrame * pUnixFrame;
	XAP_App * pApp;
	GList *names;
	char *filename = NULL;
	g_return_if_fail(widget != NULL);

	pUnixFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(widget));
	pApp = pUnixFrame->getApp ();

	switch (info)
	{
	case TARGET_URI_LIST:
	{
		names = gnome_uri_list_extract_filenames ((char *) selection_data->data);

		if (!names)
			return;
                
		for (; names; names = names->next) {
			XAP_Frame * pNewUnixFrame = pApp->newFrame ();
			filename = (char *) names->data;

			if (!E2B(pNewUnixFrame->loadDocument(filename, 0 /* IEFT_Unknown */)))
			{
				// TODO: warn user that we couldn't open that file
				
#if 1
				// TODO we crash if we just delete this without putting something
				// TODO in it, so let's go ahead and open an untitled document
				// TODO for now.
				pNewUnixFrame->loadDocument(NULL, 0 /* IEFT_Unknown */);
#else
				delete pNewUnixFrame;
#endif
			}
		}

		gnome_uri_list_free_strings (names);
		break;
	}
	case TARGET_URL:
	{
		// TODO
#if 0
		XAP_Frame * pNewUnixFrame = pApp->newFrame();
		filename = (char *) selection_data->data;

		if (!E2B(pNewUnixFrame->loadDocument(filename, 0 /* IEFT_Unknown */)))
			pNewUnixFrame->loadDocument(NULL, 0 /* IEFT_Unknown */);

#endif
		break;
	}
	}
}

/*****************************************************************/

XAP_UnixGnomeFrame::XAP_UnixGnomeFrame(XAP_UnixGnomeApp * app)
	: XAP_UnixFrame(static_cast<XAP_UnixApp *> (app))
{
}

XAP_UnixGnomeFrame::XAP_UnixGnomeFrame(XAP_UnixApp * app)
	: XAP_UnixFrame(app)
{
}

XAP_UnixGnomeFrame::XAP_UnixGnomeFrame(XAP_UnixGnomeFrame * f)
	: XAP_UnixFrame(static_cast<XAP_UnixFrame *> (f))
{
}

XAP_UnixGnomeFrame::XAP_UnixGnomeFrame(XAP_UnixFrame * f)
	: XAP_UnixFrame(f)
{
}

XAP_UnixGnomeFrame::~XAP_UnixGnomeFrame(void)
{
}

void XAP_UnixGnomeFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	UT_Bool bResult;
	static GtkTargetEntry drag_types[] =
	{
		{ "text/uri-list", 0, TARGET_URI_LIST },
		{ "_NETSCAPE_URL", 0, TARGET_URL }
	};
	static gint n_drag_types = sizeof(drag_types)/sizeof(drag_types[0]);

	m_wTopLevelWindow = gnome_app_new((gchar *)(m_pUnixApp->getApplicationName()),
									  (gchar *)(m_pUnixApp->getApplicationTitleForTitleBar()));
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);
	gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pUnixApp->getApplicationName(),
						   m_pUnixApp->getApplicationName());

	gtk_drag_dest_set (m_wTopLevelWindow,
					   GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
					   drag_types, n_drag_types, GDK_ACTION_COPY);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "drag_data_received",
                       GTK_SIGNAL_FUNC(_dnd_drop_event), NULL);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "delete_event",
					   GTK_SIGNAL_FUNC(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "destroy",
					   GTK_SIGNAL_FUNC(_fe::destroy), NULL);

	// create a VBox inside it.
	
	m_wVBox = gtk_vbox_new(FALSE,0);
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
	gtk_object_set_user_data(GTK_OBJECT(m_wVBox),this);
	gnome_app_set_contents(GNOME_APP(m_wTopLevelWindow), m_wVBox);

	// synthesize a menu from the info in our base class.

	m_pUnixMenu = new EV_UnixGnomeMenuBar(m_pUnixApp,this,
										  m_szMenuLayoutName,
										  m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bResult = ((EV_UnixGnomeMenuBar *) m_pUnixMenu)->synthesizeMenuBar();
	UT_ASSERT(bResult);


	UT_ASSERT(m_pUnixMenu);



	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.
	gtk_widget_realize(m_wTopLevelWindow);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "key_press_event",
					   GTK_SIGNAL_FUNC(_fe::key_press_event), NULL);

	_createToolbars();

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).

	m_wSunkenBox = _createDocumentWindow();
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_wSunkenBox);
	gtk_widget_show(m_wSunkenBox);

	// Let the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.

	m_wStatusBar = _createStatusBarWindow();
	if (m_wStatusBar)
	{
		gnome_app_set_statusbar(GNOME_APP(m_wTopLevelWindow), m_wStatusBar);
		gtk_widget_show(m_wStatusBar);
	}
	
	gtk_widget_show(m_wVBox);
	
	// set the icon
	_setWindowIcon();

	// set geometry hints as the user requested
	gint x, y;
	guint width, height;
	XAP_UnixApp::windowGeometryFlags f;

	m_pUnixApp->getGeometry(&x, &y, &width, &height, &f);

	// Set the size if requested
	
	if (f & XAP_UnixApp::GEOMETRY_FLAG_SIZE)
		gtk_widget_set_usize(m_wTopLevelWindow,
							 width,
							 height);

	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.

	if (m_pUnixApp->getFrameCount() <= 1)
		if (f & XAP_UnixApp::GEOMETRY_FLAG_POS)
			gtk_widget_set_uposition(m_wTopLevelWindow,
									 x,
									 y);

	// we let our caller decide when to show m_wTopLevelWindow.


	UT_ASSERT(m_pUnixMenu);


	return;
}

UT_Bool XAP_UnixGnomeFrame::openURL(const char * szURL)
{
	gnome_url_show(szURL);
	return UT_FALSE;
}

/*****************************************************************/

//  UT_Bool XAP_UnixGnomeFrame::runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
//  												UT_sint32 x, UT_sint32 y)
//  {
//  	UT_Bool bResult = UT_TRUE;

//  	UT_ASSERT(!m_pUnixPopup);

//  	m_pUnixPopup = new EV_UnixMenuPopup(m_pUnixApp,this,szMenuName,m_szMenuLabelSetName);
//  	if (m_pUnixPopup && m_pUnixPopup->synthesizeMenuPopup())
//  	{
//  		// the popup will steal the mouse and so we won't get the
//  		// button_release_event and we won't know to release our
//  		// grab.  so let's do it here.  (when raised from a keyboard
//  		// context menu, we may not have a grab, but that should be ok.

//  		GtkWidget * w = gtk_grab_get_current();
//  		if (w)
//  		{
//  			//UT_DEBUGMSG(("Ungrabbing mouse [before popup].\n"));
//  			gtk_grab_remove(w);
//  		}

//  		translateDocumentToScreen(x,y);

//  		UT_DEBUGMSG(("ContextMenu: %s at [%d,%d]\n",szMenuName,x,y));
//  		UT_Point pt;
//  		pt.x = x;
//  		pt.y = y;
//  		gtk_menu_popup(GTK_MENU(m_pUnixPopup->getMenuHandle()), NULL, NULL,
//  					   s_gtkMenuPositionFunc, &pt, 3, 0);

//  		// We run this menu synchronously, since GTK doesn't.
//  		// Popup menus have a special "unmap" function to call
//  		// gtk_main_quit() when they're done.
//  		gtk_main();
//  	}

//  	DELETEP(m_pUnixPopup);
//  	return bResult;
//  }


EV_Toolbar * XAP_UnixGnomeFrame::_newToolbar(XAP_App *app, XAP_Frame *frame,
					const char *szLayout,
					const char *szLanguage)
{
	return (new EV_UnixGnomeToolbar(static_cast<XAP_UnixGnomeApp *>(app), 
									static_cast<XAP_UnixGnomeFrame *>(frame), 
									szLayout, szLanguage));
}
