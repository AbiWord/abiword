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

#include <libgnomeui-2.0/gnome.h>

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
#include "xap_UnixDialogHelper.h"

// sorry about the XAP/AP separation breakage, but this is more important
#include "ie_types.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"
#include "fv_View.h"

#include "libgnomevfs/gnome-vfs.h"

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/

static int
s_mapMimeToUriType (const char * mime)
{
	UT_DEBUGMSG(("SEVIOR: mime %s dropped into AbiWord \n",mime));
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
	if (!UT_strcmp (mime, "image/jpeg"))
		return XAP_UnixGnomeFrame::TARGET_JPG;		
	if (!UT_strcmp (mime, "image/gif"))
		return XAP_UnixGnomeFrame::TARGET_GIF;		
	if (!UT_strcmp (mime, "image/x-xpixmap"))
		return XAP_UnixGnomeFrame::TARGET_XPM;		
	if (!UT_strcmp (mime, "image/bmp")) 
		return XAP_UnixGnomeFrame::TARGET_BMP;
	if (!UT_strcmp (mime, "image/x-bmp")) 
		return XAP_UnixGnomeFrame::TARGET_BMP;
	if (!UT_strcmp (mime, "image/x-cmu-raster")) 
		return XAP_UnixGnomeFrame::TARGET_RAS;
	if (!UT_strcmp (mime, "image/tiff")) 
		return XAP_UnixGnomeFrame::TARGET_TIF;
	if (!UT_strcmp (mime, "image/svg-xml")) 
		return XAP_UnixGnomeFrame::TARGET_SVG;
	if (!UT_strcmp (mime, "text/plain")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/abiword")) 
		return XAP_UnixGnomeFrame::TARGET_URI_LIST;
	if (!UT_strcmp (mime, "text/html")) 
		return XAP_UnixGnomeFrame::TARGET_URL;
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
	UT_DEBUGMSG(("SEVIOR: _dnd_drop_event being handled \n"));
	pUnixFrame = (XAP_UnixFrame *) gtk_object_get_user_data(GTK_OBJECT(widget));
	pApp = pUnixFrame->getApp ();

	char * rawChar = (char *) selection_data->data;
	UT_DEBUGMSG(("SEVIOR: text in selection = %s \n", rawChar));
	names = gnome_uri_list_extract_filenames (rawChar);
    bool bDropHTML = (strstr(rawChar,"html") !=0) || (strstr(rawChar,"htm") != 0);
	if (!names)
	{
		UT_DEBUGMSG(("SEVIOR: No filename found in drop event \n"));
//
// Drag an image into AbiWord. Use our magic auto -detection of image types
//
		UT_DEBUGMSG(("SEVIOR: Load image into Abiword \n"));
		GnomeVFSResult    result;
		GnomeVFSHandle   *handle;
		gchar             buffer[1024];
		GnomeVFSFileSize  bytes_read;
		GnomeVFSURI 	 *uri;
		UT_String stripped;
		UT_uint32 i =0;
		for(i=0; i < strlen(rawChar); i++)
		{
			if(rawChar[i] != '[' && rawChar[i] != ']')
			{
				stripped += rawChar[i];
			}
		}
		uri = gnome_vfs_uri_new (stripped.c_str());
		if (uri == NULL) 
		{
			UT_DEBUGMSG(("SEVIOR: Invalid uri text was %s \n",stripped.c_str()));
			return;
		}
		result = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ);
//
// Read a chunk to determine if this is an image.
//
		if(result == GNOME_VFS_OK)
		{
			UT_DEBUGMSG(("SEVIOR: uri %s openned OK \n",stripped.c_str()));
			result = gnome_vfs_read (handle, buffer, sizeof buffer - 1,
									 &bytes_read);
		}
		if(result==GNOME_VFS_OK && !bDropHTML)
		{
			UT_DEBUGMSG(("SEVIOR:  uri %s read OK \n",stripped.c_str()));
			UT_ByteBuf * Bytes = new UT_ByteBuf();
			Bytes->append((UT_Byte *)buffer,(UT_uint32) bytes_read);
			iegft = IEGFT_Unknown;
			error = IE_ImpGraphic::constructImporter(Bytes, iegft, &pIEG);
			if(!error)
			{
//
// We have an image! Load it into a byte buffer and import it into Abiword
//
				UT_DEBUGMSG(("SEVIOR: uri gave valid image data \n"));
				while( result==GNOME_VFS_OK ) 
				{
					result = gnome_vfs_read (handle, buffer, sizeof buffer - 1,
										 &bytes_read);
					if(!bytes_read) break;
					Bytes->append((UT_Byte *) buffer,(UT_uint32) bytes_read);
				}
			
				error = pIEG->importGraphic(Bytes, &pFG);
				if(error)
				{
					UT_DEBUGMSG(("Sevior: could not import graphic (%d)\n", error));
					DELETEP(pIEG);
					return;
				}
			
				DELETEP(pIEG);
			
				FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());
				error = pView->cmdInsertGraphic(pFG, rawChar);
				if (error)
				{
					UT_DEBUGMSG(("Sevior: could not insert graphic (%d)\n", error));
					DELETEP(pFG);
					return;
				}
				DELETEP(pFG);
				return;
			}
			else
			{
				delete Bytes;
			}
			result = gnome_vfs_close (handle);
			g_free (uri);
		}
			
		int type = s_mapMimeToUriType (gnome_mime_type_or_default (rawChar, "foobar"));
		if(type== TARGET_URL)
		{
			UT_DEBUGMSG(("DOM: target url\n"));
			FV_View * pView = (FV_View *) pUnixFrame->getCurrentView();
			pView->cmdInsertHyperlink(rawChar);
		}
		else if (strstr(rawChar,"http:") !=0)
		{
			UT_DEBUGMSG(("Sevior: Overiding dumb gnome parser Inserting URL \n"));
			FV_View * pView = (FV_View *) pUnixFrame->getCurrentView();
			pView->cmdInsertHyperlink(rawChar);
			return;
		}
		else
		{
			return;
		}
	}

	for (; names; names = names->next) 
	{
		filename = (char *) names->data;
		UT_DEBUGMSG(("SEVIOR: Selection %s found \n",filename));
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
			FV_View * pView = (FV_View *) pUnixFrame->getCurrentView();
			pView->cmdInsertHyperlink(filename);
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
		case TARGET_JPG:
		case TARGET_GIF:
		case TARGET_XPM:
		case TARGET_RAS:
		case TARGET_TIF:
		{
			if (type == TARGET_PNG)
			{
				iegft = IEGFT_PNG;
			}
			else if (type == TARGET_BMP)
			{
				iegft = IEGFT_BMP;
			}
			else if (type == TARGET_SVG)
			{
				iegft = IEGFT_SVG;
			}
			else
			{
				iegft = IEGFT_Unknown;
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

void XAP_UnixGnomeFrame::_dnd_real_drop_event (GtkWidget *widget, GdkDragContext * context, gint x, gint y, guint time, gpointer ppFrame)
{
	UT_DEBUGMSG(("SEVIOR: drop event on Gnome frame. \n"));
	GdkAtom selection = gdk_drag_get_selection(context);
	gtk_drag_get_data (widget,context,selection,time);
}

void XAP_UnixGnomeFrame::_dnd_drag_end(GtkWidget  *widget, GdkDragContext *context, gpointer ppFrame)
{
	UT_DEBUGMSG(("SEVIOR: drop end event on Gnome frame. \n"));
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
#if 1
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
		{"_NETSCAPE_URL", 0, TARGET_URL},
		{"text/uri-list", 0, TARGET_URI_LIST}
#else
		{"text/uri-list", 0, TARGET_URI_LIST}
#endif
	};
	static gint n_drag_types = sizeof(drag_types)/sizeof(drag_types[0]);

	if(m_iFrameMode == XAP_NormalFrame)
	{
		m_wTopLevelWindow = gnome_app_new((gchar *)(m_pUnixApp->getApplicationName()),
									  (gchar *)(m_pUnixApp->getApplicationTitleForTitleBar()));
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic_attr", NULL);
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic", NULL);
		gtk_window_set_policy(GTK_WINDOW(m_wTopLevelWindow), TRUE, TRUE, FALSE);
		gtk_window_set_wmclass(GTK_WINDOW(m_wTopLevelWindow),
						   m_pUnixApp->getApplicationName(),
						   m_pUnixApp->getApplicationName());
	} 
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow),this);

	gtk_drag_dest_set (m_wTopLevelWindow,
					   GTK_DEST_DEFAULT_ALL,
					   drag_types, 
					   n_drag_types, 
					   GDK_ACTION_COPY);
	if(m_iFrameMode == XAP_NormalFrame)
	{

		g_signal_connect(G_OBJECT(m_wTopLevelWindow), "realize",
 					   G_CALLBACK(_fe::realize), NULL);
		g_signal_connect(G_OBJECT(m_wTopLevelWindow), "unrealize",
 					   G_CALLBACK(_fe::unrealize), NULL);
	}
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "size_allocate",
 					   G_CALLBACK(_fe::sizeAllocate), NULL);
  
 	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_in_event",
 					   G_CALLBACK(_fe::focusIn), NULL);
 	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_out_event",
 					   G_CALLBACK(_fe::focusOut), NULL);

	g_signal_connect (G_OBJECT (m_wTopLevelWindow),
						"drag_data_get",
						G_CALLBACK (_dnd_drop_event), 
						(gpointer)this);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), 
					   "drag_data_received",
                       G_CALLBACK(_dnd_drop_event), 
					   (gpointer)this);

  	g_signal_connect(G_OBJECT(m_wTopLevelWindow), 
  					   "drag_drop",
                         G_CALLBACK(_dnd_real_drop_event), 
					   (gpointer)this);

  	g_signal_connect(G_OBJECT(m_wTopLevelWindow), 
  					   "drag_end",
                         G_CALLBACK(_dnd_drag_end), 
					   (gpointer)this);

	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "delete_event",
					   G_CALLBACK(_fe::delete_event), NULL);
	// here we connect the "destroy" event to a signal handler.  
	// This event occurs when we call gtk_widget_destroy() on the window,
	// or if we return 'FALSE' in the "delete_event" callback.
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "destroy",
					   G_CALLBACK(_fe::destroy), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   G_CALLBACK(_fe::focus_in_event), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   G_CALLBACK(_fe::focus_out_event), NULL);

	// create a VBox inside it.

	m_wVBox = gtk_vbox_new(FALSE,0);
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "vbox", m_wVBox);
	gtk_object_set_user_data(GTK_OBJECT(m_wVBox),this);
	if(m_iFrameMode == XAP_NormalFrame)
	{
		gnome_app_set_contents(GNOME_APP(m_wTopLevelWindow), m_wVBox);
	}
	else
	{
		gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);
	}
	// synthesize a menu from the info in our base class.
	if(m_iFrameMode == XAP_NormalFrame)
	{
		m_pUnixMenu = new EV_UnixGnomeMenuBar(m_pUnixApp,this,
											  m_szMenuLayoutName,
										  m_szMenuLabelSetName);
		UT_ASSERT(m_pUnixMenu);
		bResult = ((EV_UnixGnomeMenuBar *) m_pUnixMenu)->synthesizeMenuBar();
		UT_ASSERT(bResult);
		UT_ASSERT(m_pUnixMenu);
	}
	else
	{
		m_pUnixMenu = NULL;
	}

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.
	if(m_iFrameMode == XAP_NormalFrame)
	{
		gtk_widget_realize(m_wTopLevelWindow);
	}
	else
	{
		gtk_widget_realize(m_wVBox);
	}
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "key_press_event",
					   G_CALLBACK(_fe::key_press_event), NULL);


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
	m_wStatusBar = NULL;
	m_wStatusBar = _createStatusBarWindow();

	if (m_iFrameMode == XAP_NormalFrame)
	{
		gnome_app_set_statusbar(GNOME_APP(m_wTopLevelWindow), m_wStatusBar);
	}
	else
	{
		gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);
	}
	gtk_widget_show(m_wStatusBar);

	gtk_widget_show(m_wVBox);
	
	// set the icon
	_setWindowIcon();

	// set geometry hints as the user requested
	gint x, y;
	guint width, height;
	UT_uint32 f;

	m_pUnixApp->getWinGeometry(&x, &y, &width, &height, &f);

	// Get fall-back defaults from preferences
	UT_uint32 pref_flags, pref_width, pref_height;
	UT_sint32 pref_x, pref_y;
	m_pUnixApp->getPrefs()->getGeometry(&pref_x, &pref_y, &pref_width,
										&pref_height, &pref_flags);
	if (!(f & XAP_UnixApp::GEOMETRY_FLAG_SIZE)
		&& (pref_flags & PREF_FLAG_GEOMETRY_SIZE))
	{
		width = pref_width;
		height = pref_height;
		f |= XAP_UnixApp::GEOMETRY_FLAG_SIZE;
	}
	if (!(f & XAP_UnixApp::GEOMETRY_FLAG_POS)
		&& (pref_flags & PREF_FLAG_GEOMETRY_POS))
	{
		x = pref_x;
		y = pref_y;
		f |= XAP_UnixApp::GEOMETRY_FLAG_POS;
	}

	// Set the size if requested
	if (f & XAP_UnixApp::GEOMETRY_FLAG_SIZE)
	{
		gint abi_width = UT_MIN( gdk_screen_width() - 30, (gint)width);
		gint abi_height = UT_MIN( gdk_screen_height() - 100, (gint)height);
		gtk_widget_set_usize(m_wTopLevelWindow, abi_width, abi_height);
	}

	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.
	if (m_pUnixApp->getFrameCount() <= 1)
	{
		if (f & XAP_UnixApp::GEOMETRY_FLAG_POS)
		{
			gtk_widget_set_uposition(m_wTopLevelWindow, x, y);
		}
	}

	// Remember geometry settings for next time
	m_pUnixApp->getPrefs()->setGeometry(x, y, width, height, f);

	// we let our caller decide when to show m_wTopLevelWindow.
	UT_ASSERT(m_pUnixMenu);

	return;
}

void XAP_UnixGnomeFrame::rebuildToolbar(UT_uint32 ibar)
{
//
// Destroy the old toolbar
//
	GtkOrientation bOrient;
	const char* szGtkName = NULL;
	EV_Toolbar * pToolbar = (EV_Toolbar *) m_vecToolbars.getNthItem(ibar);
	const char * szTBName = (const char *) m_vecToolbarLayoutNames.getNthItem(ibar);
	EV_UnixGnomeToolbar * pUTB = static_cast<EV_UnixGnomeToolbar *>( pToolbar);
	GnomeDockItem * wDockItem = pUTB->destroy(&bOrient,&szGtkName);
//
// Delete the old class
//
	delete pToolbar;
//
// Build a new one.
//
	pToolbar = _newToolbar(m_app, (XAP_Frame *) this,szTBName,
						   (const char *) m_szToolbarLabelSetName);
	static_cast<EV_UnixGnomeToolbar *>(pToolbar)->rebuildToolbar(wDockItem, 
																 bOrient, 
																 szGtkName);
	delete [] szGtkName;
	m_vecToolbars.setNthItem(ibar, (void *) pToolbar, NULL);
//
// Refill the framedata pointers
//
	refillToolbarsInFrameData();
	repopulateCombos();
}

void  XAP_UnixGnomeFrame::rebuildMenus(void)
{
	((EV_UnixGnomeMenuBar *) m_pUnixMenu)->destroy();
//
// Delete the old class
//
	DELETEP(m_pUnixMenu);
//
// Build a new one.
//
	m_pUnixMenu = new EV_UnixGnomeMenuBar(m_pUnixApp,this,
									 m_szMenuLayoutName,
									 m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);

	((EV_UnixGnomeMenuBar *) m_pUnixMenu)->synthesizeMenuBar();
}

bool XAP_UnixGnomeFrame::openURL(const char * szURL)
{
#if 0
	GnomeVFSResult    result;
	GnomeVFSHandle   *handle;
	gchar             buffer[1024];
	GnomeVFSFileSize  bytes_read;
	GnomeVFSURI 	 *uri;

	uri = gnome_vfs_uri_new (szURL);
	if (uri == NULL) 
	{
		gnome_url_show(szURL);
		return false;
	}
	result = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ);
//
// Read a chunk to determine if this is an image.
//
	if(result == GNOME_VFS_OK)
	{
		result = gnome_vfs_read (handle, buffer, sizeof buffer - 1,
									 &bytes_read);
		if(result==GNOME_VFS_OK)
		{
			UT_ByteBuf * Bytes = new UT_ByteBuf();
			Bytes->append((UT_Byte *)buffer,(UT_uint32) bytes_read);
//
// TODO load document via gnome_vfs
//
		}
	}

#endif
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
//
// OK lets not immediately drop the menu if the user releases the mouse button.
// From the gtk FAQ.
//

		GdkEvent * event = gtk_get_current_event();
		GdkEventButton *bevent = (GdkEventButton *) event; 
		gnome_popup_menu_do_popup_modal (GTK_WIDGET (pUnixPopup->getMenuHandle ()),
										 NULL, NULL,bevent,(void *) bevent);
 	}
	XAP_Frame * pFrame = (XAP_Frame *) this;
	if (pFrame->getCurrentView())
	{
		pFrame->getCurrentView()->focusChange( AV_FOCUS_HERE);
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




