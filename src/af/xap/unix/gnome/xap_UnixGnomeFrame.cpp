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

#include <gnome.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_UnixGnomeFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "ev_UnixGnomeMenu.h"
#include "ev_UnixGnomeToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/

/* Each time that a object is dropped in a widget, this function is called */
//  void XAP_UnixGnomeFrame::_gfe::dnd_drop_event(GtkWidget        *widget,
//  											  GdkDragContext   *context,
//  											  gint              /*x*/,
//  											  gint              /*y*/,
//  											  GtkSelectionData *selection_data,
//  											  guint             info,
//  											  guint             /*time*/)
//  {
//  	XAP_UnixFrame * pUnixFrame;
//      g_return_if_fail(widget != NULL);

//  	pUnixFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(widget));

//      switch (info)
//      {
//      case TARGET_URL:
//      {
//  		GList *files;
//  		GList *ltmp;

//  		files = gnome_uri_list_extract_filenames((gchar *) selection_data->data);

//  		for(ltmp = files; ltmp; ltmp = g_list_next(ltmp))
//  		{
//  			const char *mimetype;
			
//  			mimetype = (const char *) gnome_mime_type((const gchar *) ltmp->data);
			
//  /*	    if((mimetype != NULL) &&
//  	    (strcmp(mimetype,"x-url/http") == 0 ||
//  	    strcmp(mimetype,"x-url/ftp") == 0)) */
//  			if(mimetype != NULL)
//  			{
//  //				g_print("File dropped: %s.  Mime type: %s\n", (gchar *) ltmp->data, mimetype);
//  				pUnixFrame->loadDocument((const char *) ltmp->data, 0 /*IEFT_Unknow*/);
//  			}
//  		}
		
//  		gnome_uri_list_free_strings (files);
//  		break;
//      }
//      }
//  }

/*****************************************************************/

XAP_UnixGnomeFrame::XAP_UnixGnomeFrame(XAP_UnixApp * app)
	: XAP_UnixFrame(app)
{
}

XAP_UnixGnomeFrame::XAP_UnixGnomeFrame(XAP_UnixGnomeFrame * f)
	: XAP_UnixFrame(static_cast<XAP_UnixFrame *>(f))
{
}

XAP_UnixGnomeFrame::~XAP_UnixGnomeFrame(void)
{
}

void XAP_UnixGnomeFrame::_createTopLevelWindow(void)
{
	// create a top-level window for us.
	UT_Bool bResult;

	m_wTopLevelWindow = gnome_app_new((gchar *)(m_pUnixApp->getApplicationName()),
									  (gchar *)(m_pUnixApp->getApplicationTitleForTitleBar()));
	gtk_object_set_data(GTK_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);
	gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pUnixApp->getApplicationName(),
						   m_pUnixApp->getApplicationName());

	// This is now done with --geometry parsing.
	//gtk_widget_set_usize(GTK_WIDGET(m_wTopLevelWindow), 700, 650);

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
	bResult = m_pUnixMenu->synthesizeMenuBar();
	UT_ASSERT(bResult);

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.
	gtk_widget_realize(m_wTopLevelWindow);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "key_press_event",
					   GTK_SIGNAL_FUNC(_fe::key_press_event), NULL);

	UT_uint32 nrToolbars = m_vecToolbarLayoutNames.getItemCount();
	for (UT_uint32 k=0; k < nrToolbars; k++)
	{
		EV_UnixGnomeToolbar * pUnixToolbar
			= new EV_UnixGnomeToolbar(m_pUnixApp,this,
									  (const char *)m_vecToolbarLayoutNames.getNthItem(k),
									  m_szToolbarLabelSetName);
		UT_ASSERT(pUnixToolbar);
		bResult = pUnixToolbar->synthesize();
		UT_ASSERT(bResult);

		m_vecUnixToolbars.addItem(pUnixToolbar);
	}

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


