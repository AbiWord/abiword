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
#include "ut_string.h"
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
#include "ev_UnixGnomeMenuPopup.h"
#include "ev_UnixGnomeToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "xad_Document.h"

// sorry about the XAP/AP separation breakage, but this is more important
#include "ie_types.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"
#include "fv_View.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/

static int
s_mapMimeToUriType (const char * mime)
{
	if (!UT_strcmp (mime, "application/rtf")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "application/msword")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "application/x-applix-word")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "application/x-palm-database")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "application/vnd.palm"))
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "image/png"))
		return XAP_UnixGnomeFrame::TARGET_PNG;		
	if (!UT_strcmp (mime, "image/bmp")) 
		return XAP_UnixGnomeFrame::TARGET_BMP;
	if (!UT_strcmp (mime, "image/svg-xml")) 
		return XAP_UnixGnomeFrame::TARGET_SVG;
	if (!UT_strcmp (mime, "text/plain")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/abiword")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/html")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/xml")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/vnd.wap.wml")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/richtext")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/rtf")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "_NETSCAPE_URL")) 
		return XAP_UnixGnomeFrame::TARGET_URL;
	
	return XAP_UnixGnomeFrame::TARGET_UNKNOWN;
}

/* Each time that a object is dropped in a widget, this function is called */
void XAP_UnixGnomeFrame::_dnd_drop_event(GtkWidget        *widget,
										 GdkDragContext   * /*context*/,
										 gint              /*x*/,
										 gint              /*y*/,
										 GtkSelectionData *selection_data,
										 guint             info,
										 guint             /*time*/,
										 XAP_UnixGnomeFrame * pFrame)
{
	XAP_UnixFrame * pUnixFrame;
	XAP_App * pApp;
	GList *names;
	char *filename = NULL;
	g_return_if_fail(widget != NULL);

	XAP_Frame * pNewUnixFrame;
	IE_ImpGraphic * pIEG;
	FG_Graphic * pFG;
	IEGraphicFileType iegft;
	UT_Error error;
	
	pUnixFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(widget));
	pApp = pUnixFrame->getApp ();

	names = gnome_uri_list_extract_filenames ((char *) selection_data->data);

	if (!names)
		return;
	
	for (; names; names = names->next) {
		filename = (char *) names->data;

		int type = s_mapMimeToUriType (gnome_mime_type_or_default (filename, "foobar"));
		switch (type) {

		case TARGET_URI_LIST:

			if (pFrame->isDirty() || pFrame->getFilename() || 
				(pFrame->getViewNumber() > 0))
			{
				pNewUnixFrame = pApp->newFrame ();
			}
			else
			{
				pNewUnixFrame = pUnixFrame;
			}
			error = pNewUnixFrame->loadDocument(filename, 0 /* IEFT_Unknown */);
			if (error)
			{
				// TODO: warn user that we couldn't open that file
				
				// TODO: we crash if we just delete this without putting something
				// TODO: in it, so let's go ahead and open an untitled document
				// TODO: for now.
				pNewUnixFrame->loadDocument(NULL, 0 /* IEFT_Unknown */);
			}
			
		case TARGET_URL:
		{
			// TODO

			UT_DEBUGMSG(("DOM: target url\n"));
#if 0
			error = pNewUnixFrame->loadDocument(filename, IEFT_Unknown);
			
			if (error)
				pNewUnixFrame->loadDocument(NULL, IEFT_Unknown);
			
#endif
			break;
		}

		case TARGET_PNG:
		case TARGET_SVG:
		case TARGET_BMP:
		{
			if (type == TARGET_PNG)
			{
				iegft = IEGFT_PNG;
			}
			else if (type == TARGET_BMP)
			{
				iegft = IEGFT_BMP;
			}
			else
			{
				iegft = IEGFT_SVG;
			}
			
			error = IE_ImpGraphic::constructImporter(filename, iegft, &pIEG);
			if(error)
			{
				UT_DEBUGMSG(("DOM: could not construct importer (%d)\n", 
							 error));
				return;
			}
			
			error = pIEG->importGraphic(filename, &pFG);
			if(error)
			{
				UT_DEBUGMSG(("DOM: could not import graphic (%d)\n", error));
				DELETEP(pIEG);
				return;
			}
			
			DELETEP(pIEG);
			
			FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());

			error = pView->cmdInsertGraphic(pFG, filename);
			if (error)
			{
				UT_DEBUGMSG(("DOM: could not insert graphic (%d)\n", error));
				DELETEP(pFG);
				return;
			}
			
			DELETEP(pFG);
			break;
		}
		
		}
	}
		
	gnome_uri_list_free_strings (names);
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
	bool bResult;
	static GtkTargetEntry drag_types[] =
	{
#if 0
		// i'm concerned that DND doesn't work for mime-types other
		// than text/uri-list. i can't get this to behave well with
        // nautilus or gmc
		{"application/rtf", 0, TARGET_URI_LIST},
		{"application/msword", 0, TARGET_URI_LIST},
		{"application/x-applix-word", 0, TARGET_URI_LIST},
		{"application/x-palm-database", 0, TARGET_URI_LIST},
		{"application/vnd.palm", 0, TARGET_URI_LIST},
		{"image/png", 0, TARGET_PNG},
		{"image/bmp", 0, TARGET_BMP},
		{"image/svg-xml", 0, TARGET_SVG},
		{"text/plain", 0, TARGET_URI_LIST},
		{"text/abiword", 0, TARGET_URI_LIST},
		{"text/html", 0, TARGET_URI_LIST},
		{"text/xml", 0, TARGET_URI_LIST},
		{"text/vnd.wap.wml", 0, TARGET_URI_LIST},
		{"text/richtext", 0, TARGET_URI_LIST},
		{"text/rtf", 0, TARGET_URI_LIST},
		{"_NETSCAPE_URL", 0, TARGET_URL}
#else
		{"text/uri-list", 0, TARGET_URI_LIST}
#endif
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
					   GTK_DEST_DEFAULT_ALL,
					   drag_types, 
					   n_drag_types, 
					   GDK_ACTION_COPY);

	gtk_signal_connect (GTK_OBJECT (m_wTopLevelWindow),
						"drag_data_get",
						GTK_SIGNAL_FUNC (_dnd_drop_event), 
						(gpointer)this);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), 
					   "drag_data_received",
                       GTK_SIGNAL_FUNC(_dnd_drop_event), 
					   (gpointer)this);

	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "delete_event",
					   GTK_SIGNAL_FUNC(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "destroy",
					   GTK_SIGNAL_FUNC(_fe::destroy), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   GTK_SIGNAL_FUNC(_fe::focus_in_event), NULL);
	gtk_signal_connect(GTK_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   GTK_SIGNAL_FUNC(_fe::focus_out_event), NULL);

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
	{
                gint abi_width = UT_MIN( gdk_screen_width() - 30, 813);
                gint abi_height = UT_MIN( gdk_screen_height() - 100, 836);
	        gtk_widget_set_usize(m_wTopLevelWindow,
							 abi_width,
							 abi_height);
	}

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

bool XAP_UnixGnomeFrame::openURL(const char * szURL)
{
	gnome_url_show(szURL);
	return false;
}

/*****************************************************************/

bool XAP_UnixGnomeFrame::runModalContextMenu(AV_View *  pView, const char * szMenuName,	UT_sint32 x, UT_sint32 y)
{
 	bool bResult = true;
 	UT_ASSERT(!m_pUnixPopup);
	
	EV_UnixGnomeMenuPopup *pUnixPopup = new EV_UnixGnomeMenuPopup(m_pUnixApp,this,szMenuName,m_szMenuLabelSetName);

 	if (pUnixPopup && pUnixPopup->synthesizeMenuPopup())
 	{
		fflush (stdout);
 		GtkWidget * w = gtk_grab_get_current();
 		if (w)
 		{
 			//UT_DEBUGMSG(("Ungrabbing mouse [before popup].\n"));
 			gtk_grab_remove(w);
 		}
		
		// the popup will steal the mouse and so we won't get the
 		// button_release_event and we won't know to release our
 		// grab.  so let's do it here.  (when raised from a keyboard
 		// context menu, we may not have a grab, but that should be ok.
                pUnixPopup->refreshMenu(pView);
		gnome_popup_menu_do_popup_modal (GTK_WIDGET (pUnixPopup->getMenuHandle ()),
										 NULL, NULL, NULL, NULL);
 	}

 	DELETEP(pUnixPopup);
 	return bResult;
}


EV_Toolbar * XAP_UnixGnomeFrame::_newToolbar(XAP_App *app, XAP_Frame *frame,
											 const char *szLayout,
											 const char *szLanguage)
{
	return (new EV_UnixGnomeToolbar(static_cast<XAP_UnixGnomeApp *>(app), 
									static_cast<XAP_UnixGnomeFrame *>(frame), 
									szLayout, szLanguage));

}




