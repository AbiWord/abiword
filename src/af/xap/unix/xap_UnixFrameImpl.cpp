/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_files.h"
#include "ut_sleep.h"
#include "xap_ViewListener.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrameImpl.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"
#include "ev_UnixMenuBar.h"
#include "ev_UnixMenuPopup.h"
#include "ev_UnixToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "fv_View.h"
#include "xad_Document.h"
#include "gr_Graphics.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Strings.h"

#include "fv_View.h"

#ifdef HAVE_GNOME
// sorry about the XAP/AP separation breakage, but this is more important
#include "ie_types.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"

#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#endif

#ifdef HAVE_GNOME

enum {
	TARGET_IMAGE,
	TARGET_URI_LIST,
	TARGET_URL,
	TARGET_UNKNOWN
} DragDropTypes;

static const GtkTargetEntry drag_types[] =
	{
		{"application/rtf", 0, TARGET_URI_LIST},
		{"text/richtext", 0, TARGET_URI_LIST},
		{"text/rtf", 0, TARGET_URI_LIST},
		{"application/msword", 0, TARGET_URI_LIST},
		{"application/x-applix-word", 0, TARGET_URI_LIST},
		{"application/x-palm-database", 0, TARGET_URI_LIST},
		{"application/vnd.palm", 0, TARGET_URI_LIST},
		{"text/plain", 0, TARGET_URI_LIST},
		{"text/abiword", 0, TARGET_URI_LIST},
		{"application/x-abiword", 0, TARGET_URI_LIST},
		{"text/xml", 0, TARGET_URI_LIST},
		{"text/vnd.wap.wml", 0, TARGET_URI_LIST},
		{"image/png", 0, TARGET_IMAGE},
		{"image/bmp", 0, TARGET_IMAGE},
		{"image/gif", 0, TARGET_IMAGE},
		{"image/x-xpixmap", 0, TARGET_IMAGE},
		{"image/bmp", 0, TARGET_IMAGE},
		{"image/x-bmp", 0, TARGET_IMAGE},
		{"image/x-cmu-raster", 0, TARGET_IMAGE},
		{"image/tiff", 0, TARGET_IMAGE},
		{"image/svg+xml", 0, TARGET_IMAGE},
		{"text/html", 0, TARGET_URL}, // hack
		{"text/html+xml", 0, TARGET_URL}, // hack
		{"_NETSCAPE_URL", 0, TARGET_URL},
		{"text/uri-list", 0, TARGET_URI_LIST}
	};

static int
s_mapMimeToUriType (const char * uri)
{
	if (!uri || !strlen(uri))
		return TARGET_UNKNOWN;

	char * mime = gnome_vfs_get_mime_type (uri);
	xxx_UT_DEBUGMSG(("DOM: mime %s dropped into AbiWord(%s)\n", mime, uri));

	int target = TARGET_UNKNOWN;

	for (size_t i = 0; i < NrElements (drag_types); i++)
		if (!UT_stricmp (mime, drag_types[i].target)) {
			target = drag_types[i].info;
			break;
		}
	
	g_free (mime);
	return target;
}

static bool
s_ensure_uri_on_disk (const gchar * uri, UT_String & outName)
{
	GnomeVFSResult    result = GNOME_VFS_OK;
	GnomeVFSHandle   *handle = NULL;
	gchar             buffer[1024];
	GnomeVFSFileSize  bytes_read;
	GnomeVFSURI 	 *hndl = NULL;	

	outName = "";
	hndl = gnome_vfs_uri_new (uri);
	if (hndl == NULL) 
		{
			xxx_UT_DEBUGMSG(("DOM: Invalid uri was %s \n", uri));
			return false;
		}

	if (gnome_vfs_uri_is_local (hndl)) {
		char * short_name = gnome_vfs_uri_to_string (hndl, (GnomeVFSURIHideOptions)(GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD |
													 GNOME_VFS_URI_HIDE_HOST_NAME | GNOME_VFS_URI_HIDE_HOST_PORT |
													 GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD | GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER));
		outName = short_name;
		g_free (short_name);
		gnome_vfs_uri_unref (hndl);
		return true;
	}

	char * outCName;
	int fd = g_file_open_tmp ("XXXXXX", &outCName, NULL);
	if (fd == -1) {
		xxx_UT_DEBUGMSG(("DOM: no temp dir\n"));
		gnome_vfs_uri_unref (hndl);
		return false;
	}
	outName = outCName;
	g_free (outCName);

	FILE * onDisk = fdopen (fd, "r");
	result = gnome_vfs_open_uri (&handle, hndl, GNOME_VFS_OPEN_READ);
	
	if(result != GNOME_VFS_OK)
		{
			xxx_UT_DEBUGMSG(("DOM: couldn't open handle\n"));
			gnome_vfs_uri_unref (hndl);
			fclose (onDisk);
			return false;
		}

	while (true) {
		result = gnome_vfs_read (handle, buffer, sizeof(buffer) - 1,
								 &bytes_read);
		if(!bytes_read || result != GNOME_VFS_OK) 
			break;
		fwrite (buffer, sizeof (char), bytes_read, onDisk);
	}

	fclose (onDisk);
	gnome_vfs_close (handle);
	gnome_vfs_uri_unref (hndl);

	return true;
}

static void
s_load_image (const UT_String & file, XAP_Frame * pFrame, FV_View * pView)
{
	IE_ImpGraphic * pIEG = 0;
	FG_Graphic    * pFG  = 0;

	UT_Error error = IE_ImpGraphic::constructImporter(file.c_str(), 0, &pIEG);
	if (error != UT_OK || !pIEG)
		{
			xxx_UT_DEBUGMSG(("Couldn't construct importer for %s\n", file.c_str()));
			return;
		}

	error = pIEG->importGraphic(file.c_str(), &pFG);

	DELETEP(pIEG);

	if (error != UT_OK || !pFG)
		{
			xxx_UT_DEBUGMSG(("Dom: could not import graphic (%s)\n", file.c_str()));
			return;
		}

	pView->cmdInsertGraphic(pFG, file.c_str());
	DELETEP(pFG);
}

static void
s_load_document (const UT_String & file, XAP_Frame * pFrame)
{
	XAP_Frame * pNewFrame = 0;
	
	if (pFrame->isDirty() || pFrame->getFilename() || 
		(pFrame->getViewNumber() > 0))
		pNewFrame = XAP_App::getApp()->newFrame ();
	else
		pNewFrame = pFrame;

	UT_Error error = pNewFrame->loadDocument(file.c_str(), 0 /* IEFT_Unknown */);
	if (error)
		{
			// TODO: warn user that we couldn't open that file		 
			// TODO: we crash if we just delete this without putting something
			// TODO: in it, so let's go ahead and open an untitled document
			// TODO: for now.
			xxx_UT_DEBUGMSG(("DOM: couldn't load document %s\n", file.c_str()));
			pNewFrame->loadDocument(NULL, 0 /* IEFT_Unknown */);
		}
}

static void
s_load_uri (XAP_Frame * pFrame, const char * uri)
{
	FV_View   * pView  = static_cast<FV_View*>(pFrame->getCurrentView ());

	int type = s_mapMimeToUriType (uri);
	if (type == TARGET_UNKNOWN)
		{
			xxx_UT_DEBUGMSG(("DOM: unknown uri type: %s\n", uri));
			return;
		}
	else if (type == TARGET_URL)
		{
			xxx_UT_DEBUGMSG(("DOM: hyperlink: %s\n", uri));
			pView->cmdInsertHyperlink(uri);
			return;
		}

	UT_String onDisk;
	if (!s_ensure_uri_on_disk (uri, onDisk))
		{
			xxx_UT_DEBUGMSG(("DOM: couldn't ensure %s on disk\n", uri));
			return;
		}

	xxx_UT_DEBUGMSG(("DOM: %s on disk\n", onDisk.c_str()));

	if (type == TARGET_IMAGE)
		{
			s_load_image (onDisk, pFrame, pView);
			return;
		}
	else
		{
			s_load_document (onDisk, pFrame);
			return;
		}
}

static void 
s_dnd_drop_event(GtkWidget        *widget,
				 GdkDragContext   * /*context*/,
				 gint              /*x*/,
				 gint              /*y*/,
				 GtkSelectionData *selection_data,
				 guint             info,
				 guint             /*time*/,
				 XAP_UnixFrameImpl * pFrameImpl)
{
	xxx_UT_DEBUGMSG(("DOM: dnd_drop_event being handled\n"));

	g_return_if_fail(widget != NULL);

	XAP_Frame * pFrame = pFrameImpl->getFrame ();

	const char * rawChar = reinterpret_cast<const char *>(selection_data->data);
	xxx_UT_DEBUGMSG(("DOM: text in selection = %s \n", rawChar));
	GList * names = gnome_vfs_uri_list_parse (rawChar);

	// single URI
	if (!names)
		s_load_uri (pFrame, rawChar);
	else {
		// multiple URIs
		for ( ; names != NULL; names = names->next) 
			{
				GnomeVFSURI * hndl = static_cast<GnomeVFSURI *>(names->data);
				char * uri = gnome_vfs_uri_to_string (hndl, GNOME_VFS_URI_HIDE_NONE);
				s_load_uri (pFrame, uri);
				g_free (uri);
			}
	}

	gnome_vfs_uri_list_free (names);
}

static void
s_dnd_real_drop_event (GtkWidget *widget, GdkDragContext * context, 
					   gint x, gint y, guint time, gpointer ppFrame)
{
	xxx_UT_DEBUGMSG(("DOM: dnd drop event\n"));
	GdkAtom selection = gdk_drag_get_selection(context);
	gtk_drag_get_data (widget,context,selection,time);
}

static void
s_dnd_drag_end (GtkWidget  *widget, GdkDragContext *context, gpointer ppFrame)
{
	xxx_UT_DEBUGMSG(("DOM: dnd end event\n"));
}

#endif // HAVE_GNOME

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

static void s_gtkMenuPositionFunc(GtkMenu * /* menu */, gint * x, gint * y, gboolean * push_in, gpointer user_data)
{
	struct UT_Point * p = static_cast<struct UT_Point *>(user_data);

	*x = p->x;
	*y = p->y;
	*push_in = TRUE ;
}

static void wmspec_change_state(bool add, GdkWindow *w, GdkAtom atom1, GdkAtom atom2)
{
   XEvent xev;
#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */  

   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = gdk_display;
   xev.xclient.window = GDK_WINDOW_XID (w);
   xev.xclient.message_type = gdk_x11_get_xatom_by_name ("_NET_WM_STATE");
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
   xev.xclient.data.l[1] = gdk_x11_atom_to_xatom (atom1);
   xev.xclient.data.l[2] = gdk_x11_atom_to_xatom (atom2);
   XSendEvent(gdk_display, GDK_WINDOW_XID (gdk_get_default_root_window ()),
              False, SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
}

static void wmspec_change_layer(bool fullscreen, GdkWindow *window)
{
   XEvent xev;
#define _WIN_LAYER_TOP        -1    /* remove/unset property */
#define _WIN_LAYER_NORMAL      4    /* add/set property */

   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = gdk_display;
   xev.xclient.window = GDK_WINDOW_XID (window);
   xev.xclient.message_type = gdk_x11_get_xatom_by_name ("_WIN_LAYER");
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = fullscreen ? _WIN_LAYER_TOP : _WIN_LAYER_NORMAL ;
   XSendEvent(gdk_display, GDK_WINDOW_XID (gdk_get_default_root_window ()),
              False, SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
}

/****************************************************************/
XAP_UnixFrameImpl::XAP_UnixFrameImpl(XAP_Frame *pFrame, XAP_UnixApp * pApp) : 
	XAP_FrameImpl(pFrame),
	m_imContext(NULL),
	need_im_reset (false),
	m_bDoZoomUpdate(false),
	m_iZoomUpdateID(0),
	m_iAbiRepaintID(0),
	m_pUnixApp(pApp),
	m_pUnixMenu(NULL),
	m_pUnixPopup(NULL),
	m_dialogFactory(pFrame, static_cast<XAP_App *>(pApp))
{
}

XAP_UnixFrameImpl::~XAP_UnixFrameImpl() 
{ 	
	if(m_bDoZoomUpdate) {
		gtk_timeout_remove(m_iZoomUpdateID);
	}

	// only delete the things we created...
	if(m_iAbiRepaintID)
	{
		gtk_timeout_remove(m_iAbiRepaintID);
	}

	DELETEP(m_pUnixMenu);
	DELETEP(m_pUnixPopup);

	// unref the input method context
	g_object_unref (G_OBJECT (m_imContext));
}


void XAP_UnixFrameImpl::_fe::realize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
}

void XAP_UnixFrameImpl::_fe::unrealize(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
}

void XAP_UnixFrameImpl::_fe::sizeAllocate(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
}

gint XAP_UnixFrameImpl::_fe::focusIn(GtkWidget * widget, GdkEvent * /*e*/,gpointer /*data*/)
{
  return FALSE;
}

gint XAP_UnixFrameImpl::_fe::focusOut(GtkWidget * /* w*/, GdkEvent * /*e*/,gpointer /*data*/)
{
  return FALSE;
}

void XAP_UnixFrameImpl::focusIMIn ()
{
	need_im_reset = true;
	gtk_im_context_focus_in(getIMContext());
}

void XAP_UnixFrameImpl::focusIMOut ()
{
	need_im_reset = true;
	gtk_im_context_focus_in(getIMContext());
}

void XAP_UnixFrameImpl::resetIMContext()
{
  if (need_im_reset)
    {
      need_im_reset = false;
      gtk_im_context_reset (getIMContext());
    }
}

gboolean XAP_UnixFrameImpl::_fe::focus_in_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	UT_ASSERT(pFrameImpl);
	XAP_Frame* pFrame = pFrameImpl->getFrame();
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(TRUE));
	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange(gtk_grab_get_current() == NULL || gtk_grab_get_current() == w ? AV_FOCUS_HERE : AV_FOCUS_NEARBY);
	pFrameImpl->focusIMIn ();
	return FALSE;
}

gboolean XAP_UnixFrameImpl::_fe::focus_out_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	UT_ASSERT(pFrameImpl);
	XAP_Frame* pFrame = pFrameImpl->getFrame();
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange(AV_FOCUS_NONE);
	pFrameImpl->focusIMOut();
	return FALSE;
}

gint XAP_UnixFrameImpl::_fe::button_press_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
	AV_View * pView = pFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pFrame->getMouse());

	gtk_grab_add(w);

	if (e->state & GDK_SHIFT_MASK)
		pUnixFrameImpl->resetIMContext ();

	if (pView)
		pUnixMouse->mouseClick(pView,e);
	return 1;
}

gint XAP_UnixFrameImpl::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
	AV_View * pView = pFrame->getCurrentView();

	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pFrame->getMouse());

	gtk_grab_remove(w);

	if (pView)
		pUnixMouse->mouseUp(pView,e);

	return 1;
}

/*!
 * Background zoom updater. It updates the view zoom level after all configure
 * events have been processed. This is
 */
gint XAP_UnixFrameImpl::_fe::do_ZoomUpdate(gpointer /* XAP_UnixFrameImpl * */ p)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(p);
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	if(!pView || pFrame->isFrameLocked() ||
	   (pUnixFrameImpl->m_bDoZoomUpdate && (pView->getGraphics()->tdu(pView->getWindowWidth()) == pUnixFrameImpl->m_iNewWidth) && (pView->getGraphics()->tdu(pView->getWindowHeight()) == pUnixFrameImpl->m_iNewHeight)))
	{
		pUnixFrameImpl->m_iZoomUpdateID = 0;
		pUnixFrameImpl->m_bDoZoomUpdate = false;
		return FALSE;
	}

	pUnixFrameImpl->m_bDoZoomUpdate = true;

	UT_sint32 iNewWidth = 0;
	UT_sint32 iNewHeight = 0;
	do
	{
		// currently, we blow away the old view.  This will change, rendering
		// the loop superfluous.
		pView = pFrame->getCurrentView();

		if(!pView)
		{
			pUnixFrameImpl->m_iZoomUpdateID = 0;
			pUnixFrameImpl->m_bDoZoomUpdate = false;
			return FALSE;
		}

		// oops, we're not ready yet.
		if (pView->isLayoutFilling())
			return TRUE;

		iNewWidth = pUnixFrameImpl->m_iNewWidth;
		iNewHeight = pUnixFrameImpl->m_iNewHeight;

		pUnixFrameImpl->_startViewAutoUpdater(); 
		pView->setWindowSize(iNewWidth, iNewHeight);
		pFrame->updateZoom();

	}
	while((iNewWidth != pUnixFrameImpl->m_iNewWidth) || (iNewHeight != pUnixFrameImpl->m_iNewHeight));

	pUnixFrameImpl->m_iZoomUpdateID = 0;
	pUnixFrameImpl->m_bDoZoomUpdate = false;
	return FALSE;
}

gint XAP_UnixFrameImpl::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// This is basically a resize event.

	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();

	if (pView)
	{
		pUnixFrameImpl->m_iNewWidth = e->width;
		pUnixFrameImpl->m_iNewHeight = e->height;
		XAP_App * pApp = pFrame->getApp();
		UT_sint32 x,y;
		UT_uint32 width,height,flags;

		pApp->getGeometry(&x,&y,&width,&height,&flags);
//
// Who ever wants to change this code in the future. The height and widths you
// get from the event struct are the height and widths of the drawable area of
// the screen. We want the height and width of the entire widget which we get
// from the m_wTopLevelWindow widget.
// -- MES
//
        GtkWindow * pWin = NULL;
		if(pFrame->getFrameMode() == XAP_NormalFrame)
			pWin = GTK_WINDOW(pUnixFrameImpl->m_wTopLevelWindow);
        else
            pWin = GTK_WINDOW(pUnixFrameImpl->m_wTopLevelWindow->window);

        gint gwidth,gheight;
        gtk_window_get_size(pWin,&gwidth,&gheight);
        pApp->setGeometry(e->x,e->y,gwidth,gheight,flags);

		// Dynamic Zoom Implementation
        if(!pUnixFrameImpl->m_bDoZoomUpdate && (pUnixFrameImpl->m_iZoomUpdateID == 0))
            pUnixFrameImpl->m_iZoomUpdateID = g_idle_add(static_cast<GSourceFunc>(do_ZoomUpdate), static_cast<gpointer>(pUnixFrameImpl));
	}
	return 1;
}

gint XAP_UnixFrameImpl::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
	AV_View * pView = pFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pFrame->getMouse());

	if (pView)
		pUnixMouse->mouseMotion(pView, e);

	return 1;
}

gint XAP_UnixFrameImpl::_fe::scroll_notify_event(GtkWidget* w, GdkEventScroll* e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
	AV_View * pView = pFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pFrame->getMouse());

	if (pView)
		pUnixMouse->mouseScroll(pView, e);

	return 1;
}

gint XAP_UnixFrameImpl::_fe::key_release_event(GtkWidget* w, GdkEventKey* e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));

	// Let IM handle the event first.
	if (gtk_im_context_filter_keypress(pUnixFrameImpl->getIMContext(), e)) {
	    xxx_UT_DEBUGMSG(("IMCONTEXT keyevent swallow: %lu\n", e->keyval));
		pUnixFrameImpl->queueIMReset ();
	    return 0;
	}
	return TRUE;
}

gint XAP_UnixFrameImpl::_fe::key_press_event(GtkWidget* w, GdkEventKey* e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));

	// Let IM handle the event first.
	if (gtk_im_context_filter_keypress(pUnixFrameImpl->getIMContext(), e)) {
		pUnixFrameImpl->queueIMReset ();
	    return 0;
	}

	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
	AV_View * pView = pFrame->getCurrentView();
	ev_UnixKeyboard * pUnixKeyboard = static_cast<ev_UnixKeyboard *>(pFrame->getKeyboard());

	if (pView)
		pUnixKeyboard->keyPressEvent(pView, e);

	// HACK : This one's ugly.  If we continue through the callback chain,
	// HACK : GTK will pick up key presses and hand them off to widgets with
	// HACK : focus (like toolbar combos).  This is bad, since we have already
	// HACK : acted on the key press (like space, we would insert a space
	// HACK : in the document, but GTK will let space mean "open the menu"
	// HACK : to a combo).  The user is confused and things are annoying.
	// HACK :
	// HACK : We _could_ block all GTK key handling, and do everything
	// HACK : ourselves, but then we lose the automatic menu accelerator
	// HACK : bindings (Alt-F for File menu).
	// HACK :
	// HACK : What we do is let ONLY Alt-modified keys through to GTK.

	// If a modifier is down, return to let GTK catch

	// don't let GTK handle keys when mod2 (numlock) or mod5 (scroll lock) are down

	// What's "LOCK_MASK"?  I can't seem to trigger it with caps lock, scroll lock, or
	// num lock.
//		(e->state & GDK_LOCK_MASK))		// catch all keys with "num lock" down for now

	if ((e->state & GDK_MOD1_MASK) ||
		(e->state & GDK_MOD3_MASK) ||
		(e->state & GDK_MOD4_MASK))
		return 0;

	// ... else, stop this signal
	gtk_signal_emit_stop_by_name(GTK_OBJECT(w), "key_press_event");
	return 1;
}

gint XAP_UnixFrameImpl::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	if(pApp->isBonoboRunning())
	{
		return FALSE;
	}
	const EV_Menu_ActionSet * pMenuActionSet = pApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	// was "closeWindow", TRUE, FALSE
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
	UT_ASSERT(pEM);

	if (pEM)
	{
		if (pEM->Fn(pFrame->getCurrentView(),NULL))
		{
			// returning FALSE means destroy the window, continue along the
			// chain of Gtk destroy events

			return FALSE;
		}
	}

	// returning TRUE means do NOT destroy the window; halt the message
	// chain so it doesn't see destroy
	return TRUE;
}

gint XAP_UnixFrameImpl::_fe::expose(GtkWidget * w, GdkEventExpose* pExposeEvent)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	FV_View * pView = static_cast<FV_View *>(pUnixFrameImpl->getFrame()->getCurrentView());
	if(pView)
	{
		GR_Graphics * pGr = pView->getGraphics ();
		UT_Rect rClip;
		rClip.left = pGr->tlu(pExposeEvent->area.x);
		rClip.top = pGr->tlu(pExposeEvent->area.y);
		rClip.width = pGr->tlu(pExposeEvent->area.width);
		rClip.height = pGr->tlu(pExposeEvent->area.height);

		pView->draw(&rClip);
	}
	return FALSE;
}

/*!
 * Background abi repaint function.
\param XAP_UnixFrameImpl * p pointer to the FrameImpl that initiated this background
       repainter.
 */
gint XAP_UnixFrameImpl::_fe::abi_expose_repaint(gpointer p)
{
//
// Grab our pointer so we can do useful stuff.
//
	UT_Rect localCopy;
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(p);
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	FV_View * pV = static_cast<FV_View *>(pFrame->getCurrentView());
	if(!pV || (pV->getPoint() == 0))
		return TRUE;

	GR_Graphics * pG = pV->getGraphics();
	if(pG->isDontRedraw())
	{
//
// Come back later
//
		return TRUE;
	}
	pG->setSpawnedRedraw(true);
	if(pG->isExposePending())
	{
		while(pG->isExposedAreaAccessed())
		{
			pFrame->nullUpdate();
			UT_usleep(10); // 10 microseconds
		}
		pG->setExposedAreaAccessed(true);
		localCopy.set(pG->getPendingRect()->left,pG->getPendingRect()->top,
					  pG->getPendingRect()->width,pG->getPendingRect()->height);
//
// Clear out this set of expose info
//
		pG->setExposePending(false);
		pG->setExposedAreaAccessed(false);
		pV->draw(&localCopy);
	}
//
// OK we've finshed. Wait for the next signal
//
	pG->setSpawnedRedraw(false);
	return TRUE;
}

void XAP_UnixFrameImpl::_fe::vScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();

	if (pView)
		pView->sendVerticalScrollEvent(static_cast<UT_sint32>(w->value));
}

void XAP_UnixFrameImpl::_fe::hScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(gtk_object_get_user_data(GTK_OBJECT(w)));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();

	if (pView)
		pView->sendHorizontalScrollEvent(static_cast<UT_sint32>(w->value));
}

void XAP_UnixFrameImpl::_fe::destroy(GtkWidget * /*widget*/, gpointer /*data*/)
{
}

/*****************************************************************/

void XAP_UnixFrameImpl::_nullUpdate() const
{
	for (UT_uint32 i = 0; (i < 5) && gtk_events_pending(); i++)
		gtk_main_iteration ();
}

void XAP_UnixFrameImpl::_initialize()
{
    	// get a handle to our keyboard binding mechanism
 	// and to our mouse binding mechanism.
 	EV_EditEventMapper * pEEM = getFrame()->getEditEventMapper();
 	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_UnixKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new EV_UnixMouse(pEEM);
	UT_ASSERT(m_pMouse);

	//
	// Start background repaint
	//
	if(m_iAbiRepaintID == 0)
		m_iAbiRepaintID = gtk_timeout_add(100,static_cast<GtkFunction>(XAP_UnixFrameImpl::_fe::abi_expose_repaint), static_cast<gpointer>(this));
	else
	{
		gtk_timeout_remove(m_iAbiRepaintID);
		m_iAbiRepaintID = gtk_timeout_add(100,static_cast<GtkFunction>(XAP_UnixFrameImpl::_fe::abi_expose_repaint), static_cast<gpointer>(this));
	}
}

void XAP_UnixFrameImpl::_setCursor(GR_Graphics::Cursor c)
{
//	if (m_cursor == c)
//		return;
//	m_cursor = c;
	FV_View * pView = static_cast<FV_View *>(getFrame()->getCurrentView());
	if(pView)
	{
		GR_Graphics * pG = pView->getGraphics();
		if(pG && pG->queryProperties( GR_Graphics::DGP_PAPER))
			return;
	}
	if(getTopLevelWindow() == NULL || (m_iFrameMode != XAP_NormalFrame))
		return;

	GdkCursorType cursor_number;

	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_Graphics::GR_CURSOR_DEFAULT:
		cursor_number = GDK_LEFT_PTR;
		break;

	case GR_Graphics::GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;

	//I have changed the shape of the arrow so get a consistent
	//behaviour in the bidi build; I think the new arrow is better
	//for the purpose anyway

	case GR_Graphics::GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_SB_RIGHT_ARROW; //GDK_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_LEFTARROW:
		cursor_number = GDK_SB_LEFT_ARROW; //GDK_LEFT_PTR;
		break;

	case GR_Graphics::GR_CURSOR_IMAGE:
		cursor_number = GDK_FLEUR;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_NW:
		cursor_number = GDK_TOP_LEFT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_N:
		cursor_number = GDK_TOP_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_NE:
		cursor_number = GDK_TOP_RIGHT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_E:
		cursor_number = GDK_RIGHT_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_SE:
		cursor_number = GDK_BOTTOM_RIGHT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_S:
		cursor_number = GDK_BOTTOM_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_SW:
		cursor_number = GDK_BOTTOM_LEFT_CORNER;
		break;

	case GR_Graphics::GR_CURSOR_IMAGESIZE_W:
		cursor_number = GDK_LEFT_SIDE;
		break;

	case GR_Graphics::GR_CURSOR_LEFTRIGHT:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_UPDOWN:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_EXCHANGE:
		cursor_number = GDK_EXCHANGE;
		break;

	case GR_Graphics::GR_CURSOR_GRAB:
		cursor_number = GDK_HAND1;
		break;

	case GR_Graphics::GR_CURSOR_LINK:
		cursor_number = GDK_HAND2;
		break;

	case GR_Graphics::GR_CURSOR_WAIT:
		cursor_number = GDK_WATCH;
		break;
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(getTopLevelWindow()->window, cursor);
	gdk_window_set_cursor(getVBoxWidget()->window, cursor);
	gdk_window_set_cursor(m_wSunkenBox->window, cursor);
	gdk_window_set_cursor(m_wStatusBar->window, cursor);
	gdk_cursor_destroy(cursor);
}

UT_sint32 XAP_UnixFrameImpl::_setInputMode(const char * szName)
{
	XAP_Frame*	pFrame = getFrame();
	UT_sint32 result = pFrame->XAP_Frame::setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = pFrame->getEditEventMapper();
		UT_ASSERT(pEEM);

		m_pKeyboard->setEditEventMap(pEEM);
		m_pMouse->setEditEventMap(pEEM);
	}

	return result;
}

GtkWidget * XAP_UnixFrameImpl::getTopLevelWindow(void) const
{
	return m_wTopLevelWindow;
}

GtkWidget * XAP_UnixFrameImpl::getVBoxWidget(void) const
{
	return m_wVBox;
}

XAP_DialogFactory * XAP_UnixFrameImpl::_getDialogFactory(void)
{
	return &m_dialogFactory;
}

// TODO: split me up into smaller pieces/subfunctions
void XAP_UnixFrameImpl::createTopLevelWindow(void)
{
	// create a top-level window for us.

	static GdkPixbuf * wmIcon = NULL ;

	// load the icon only once
	if (!wmIcon)
	{
		GError *err = NULL ;
		UT_String icon_location = XAP_App::getApp()->getAbiSuiteLibDir();
		icon_location += "/icons/abiword_16.xpm" ;
		wmIcon = gdk_pixbuf_new_from_file(icon_location.c_str(),&err);
	}
	
	bool bResult;
	if(m_iFrameMode == XAP_NormalFrame)
	{
		m_wTopLevelWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic_attr", NULL);
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic", NULL);
		gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
				     m_pUnixApp->getApplicationTitleForTitleBar());
		gtk_window_set_resizable(GTK_WINDOW(m_wTopLevelWindow), TRUE);
		gtk_window_set_role(GTK_WINDOW(m_wTopLevelWindow), "topLevelWindow");
		if ( wmIcon )
			gtk_window_set_icon(GTK_WINDOW(m_wTopLevelWindow), wmIcon);
	}
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	gtk_object_set_user_data(GTK_OBJECT(m_wTopLevelWindow), this); 

	_setGeometry ();

	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "realize",
					   G_CALLBACK(_fe::realize), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "unrealize",
					   G_CALLBACK(_fe::unrealize), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "size_allocate",
					   G_CALLBACK(_fe::sizeAllocate), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_in_event",
					   G_CALLBACK(_fe::focusIn), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "focus_out_event",
					   G_CALLBACK(_fe::focusOut), NULL);

#ifdef HAVE_GNOME
	gtk_drag_dest_set (m_wTopLevelWindow,
					   GTK_DEST_DEFAULT_ALL,
					   drag_types, 
					   NrElements(drag_types), 
					   GDK_ACTION_COPY);

	g_signal_connect (G_OBJECT (m_wTopLevelWindow),
					  "drag_data_get",
					  G_CALLBACK (s_dnd_drop_event), 
					  static_cast<gpointer>(this));
	g_signal_connect (G_OBJECT (m_wTopLevelWindow), 
					  "drag_data_received",
					  G_CALLBACK (s_dnd_drop_event), 
					  static_cast<gpointer>(this));	
  	g_signal_connect (G_OBJECT (m_wTopLevelWindow), 
					  "drag_drop",
					  G_CALLBACK (s_dnd_real_drop_event), 
					  static_cast<gpointer>(this));
	
  	g_signal_connect (G_OBJECT (m_wTopLevelWindow), 
					  "drag_end",
					  G_CALLBACK (s_dnd_drag_end), 
					  static_cast<gpointer>(this));
#endif

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
	gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);

	if (m_iFrameMode != XAP_NoMenusWindowLess) {
		// synthesize a menu from the info in our base class.
		m_pUnixMenu = new EV_UnixMenuBar(m_pUnixApp, getFrame(), m_szMenuLayoutName,
										 m_szMenuLabelSetName);
		UT_ASSERT(m_pUnixMenu);
		bResult = m_pUnixMenu->synthesizeMenuBar();
		UT_ASSERT(bResult);
	}

	// create a toolbar instance for each toolbar listed in our base class.
	// TODO for some reason, the toolbar functions require the TLW to be
	// TODO realized (they reference m_wTopLevelWindow->window) before we call them.

	if(m_iFrameMode == XAP_NormalFrame)
		gtk_widget_realize(m_wTopLevelWindow);

	_createIMContext(m_wTopLevelWindow->window);

	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "key_press_event",
					   G_CALLBACK(_fe::key_press_event), NULL);
	g_signal_connect(G_OBJECT(m_wTopLevelWindow), "key_release_event",
					   G_CALLBACK(_fe::key_release_event), NULL);

	if(m_iFrameMode == XAP_NormalFrame)
		_createToolbars();

	// Let the app-specific frame code create the contents of
	// the child area of the window (between the toolbars and
	// the status bar).
	m_wSunkenBox = _createDocumentWindow();
	gtk_container_add(GTK_CONTAINER(m_wVBox), m_wSunkenBox);
	gtk_widget_show(m_wSunkenBox);

	// Create statusLet the app-specific frame code create the status bar
	// if it wants to.  we will put it below the document
	// window (a peer with toolbars and the overall sunkenbox)
	// so that it will appear outside of the scrollbars.
	m_wStatusBar = NULL;
	if(m_iFrameMode == XAP_NormalFrame)
		m_wStatusBar = _createStatusBarWindow();

	if (m_wStatusBar) 
	{
		gtk_widget_show(m_wStatusBar);
		gtk_box_pack_end(GTK_BOX(m_wVBox), m_wStatusBar, FALSE, FALSE, 0);
	}

	gtk_widget_show(m_wVBox);

	// set the icon
	if(m_iFrameMode == XAP_NormalFrame)
		_setWindowIcon();
}

void XAP_UnixFrameImpl::_createIMContext(GdkWindow *w)
{
	m_imContext = gtk_im_multicontext_new();
	gtk_im_context_set_use_preedit (m_imContext, FALSE); // show alternate preedit stuff in a separate window or somesuch
	gtk_im_context_set_client_window(m_imContext, w);
	g_signal_connect(G_OBJECT(m_imContext), "commit", 
					 G_CALLBACK(_imCommit_cb), this);
	g_signal_connect (m_imContext, "preedit_changed",
					  G_CALLBACK (_imPreeditChanged_cb), this);
	g_signal_connect (m_imContext, "retrieve_surrounding",
					  G_CALLBACK (_imRetrieveSurrounding_cb), this);
	g_signal_connect (m_imContext, "delete_surrounding",
					  G_CALLBACK (_imDeleteSurrounding_cb), this);
}

void XAP_UnixFrameImpl::_imPreeditChanged_cb (GtkIMContext *context, gpointer data)
{
	UT_DEBUGMSG(("Preedit Changed\n"));

#if 0
	gchar *preedit_string;
	gint cursor_pos;
	
	gtk_im_context_get_preedit_string (context,
									   &preedit_string, NULL,
									   &cursor_pos);
	entry->preedit_length = strlen (preedit_string);
	cursor_pos = CLAMP (cursor_pos, 0, g_utf8_strlen (preedit_string, -1));
	entry->preedit_cursor = cursor_pos;
	g_free (preedit_string);
	
	gtk_entry_recompute (entry);
#endif
}

gint XAP_UnixFrameImpl::_imRetrieveSurrounding_cb (GtkIMContext *context, gpointer data)
{
	UT_DEBUGMSG(("Retrieve Surrounding\n"));

#if 0
  gtk_im_context_set_surrounding (context,
				  entry->text,
				  entry->n_bytes,
				  g_utf8_offset_to_pointer (entry->text, entry->current_pos) - entry->text);
#endif

	return TRUE;
}

gint XAP_UnixFrameImpl::_imDeleteSurrounding_cb (GtkIMContext *slave, gint offset, gint n_chars, gpointer data)
{
	XAP_UnixFrameImpl * pImpl = static_cast<XAP_UnixFrameImpl*>(data);
	FV_View * pView = static_cast<FV_View*>(pImpl->getFrame()->getCurrentView ());

	PT_DocPosition insPt = pView->getInsPoint ();
	if ((gint) insPt + offset < 0)
		return TRUE;

	pView->moveInsPtTo (insPt + offset);
	pView->cmdCharDelete (true, n_chars);

	return TRUE;
}

// Actual keyboard commit should be done here.
void XAP_UnixFrameImpl::_imCommit_cb(GtkIMContext *imc, const gchar *text, gpointer data)
{
	XAP_UnixFrameImpl * impl = static_cast<XAP_UnixFrameImpl*>(data);
	impl->_imCommit (imc, text);
}

// Actual keyboard commit should be done here.
void XAP_UnixFrameImpl::_imCommit(GtkIMContext *imc, const gchar * text)
{
	XAP_Frame* pFrame = getFrame();
	AV_View * pView   = pFrame->getCurrentView();
	ev_UnixKeyboard * pUnixKeyboard = static_cast<ev_UnixKeyboard *>(pFrame->getKeyboard());

	pUnixKeyboard->charDataEvent(pView, static_cast<EV_EditBits>(0), text, strlen(text));
}

GtkIMContext * XAP_UnixFrameImpl::getIMContext()
{
	return m_imContext;
}

void XAP_UnixFrameImpl::_setGeometry ()
{
	UT_sint32 app_x = 0;
	UT_sint32 app_y = 0;
	UT_uint32 app_w = 0;
	UT_uint32 app_h = 0;
	UT_uint32 app_f = 0;

	XAP_App * pApp = m_pUnixApp->XAP_App::getApp ();
	pApp->getGeometry (&app_x, &app_y, &app_w, &app_h, &app_f);
	// (ignore app_x, app_y & app_f since the WM will set them for us just fine)
	  
	// This is now done with --geometry parsing.
	if (app_w == 0) app_w = 760;
	if (app_h == 0) app_h = 520;

	UT_DEBUGMSG(("xap_UnixFrameImpl: app-width=%lu, app-height=%lu\n",
				 static_cast<unsigned long>(app_w),static_cast<unsigned long>(app_h)));

	// set geometry hints as the user requested
	gint user_x = 0;
	gint user_y = 0;
	guint user_w = static_cast<guint>(app_w);
	guint user_h = static_cast<guint>(app_h);
	UT_uint32 user_f = 0;

	m_pUnixApp->getWinGeometry (&user_x, &user_y, &user_w, &user_h, &user_f);

	UT_DEBUGMSG(("xap_UnixFrameImpl: user-width=%u, user-height=%u\n",
				 static_cast<unsigned>(user_w),static_cast<unsigned>(user_h)));

	// Get fall-back defaults from preferences
	UT_sint32 pref_x = 0;
	UT_sint32 pref_y = 0;
	UT_uint32 pref_w = static_cast<UT_uint32>(app_w);
	UT_uint32 pref_h = static_cast<UT_uint32>(app_h);
	UT_uint32 pref_f = 0;

	m_pUnixApp->getPrefs()->getGeometry (&pref_x, &pref_y, &pref_w, &pref_h, &pref_f);

	UT_DEBUGMSG(("xap_UnixFrameImpl: pref-width=%lu, pref-height=%lu\n",
				 static_cast<unsigned long>(pref_w),static_cast<unsigned long>(pref_h)));

	if (!(user_f & XAP_UnixApp::GEOMETRY_FLAG_SIZE))
		if (pref_f & PREF_FLAG_GEOMETRY_SIZE)
			{
				user_w = static_cast<guint>(pref_w);
				user_h = static_cast<guint>(pref_h);
				user_f |= XAP_UnixApp::GEOMETRY_FLAG_SIZE;
			}
	if (!(user_f & XAP_UnixApp::GEOMETRY_FLAG_POS))
		if (pref_f & PREF_FLAG_GEOMETRY_POS)
			{
				user_x = static_cast<gint>(pref_x);
				user_y = static_cast<gint>(pref_y);
				user_f |= XAP_UnixApp::GEOMETRY_FLAG_POS;
			}

	UT_DEBUGMSG(("xap_UnixFrameImpl: user-x=%d, user-y=%d\n",
				 static_cast<int>(user_x),static_cast<int>(user_y)));

	if (!(user_f & XAP_UnixApp::GEOMETRY_FLAG_SIZE))
		{
			user_w = static_cast<guint>(app_w);
			user_h = static_cast<guint>(app_h);
		}

	GdkGeometry geom;
	geom.min_width   = 100;
	geom.min_height  = 100;
	geom.base_width  = user_w;
	geom.base_height = user_h;
	geom.width_inc  = 10; // ??
	geom.height_inc = 10;

	if(getFrame()->getFrameMode() == XAP_NormalFrame)
	{
		gtk_window_set_geometry_hints (GTK_WINDOW(m_wTopLevelWindow), m_wTopLevelWindow, &geom,
									   (GdkWindowHints)(GDK_HINT_MIN_SIZE|GDK_HINT_BASE_SIZE|GDK_HINT_RESIZE_INC));

		gtk_window_set_default_size (GTK_WINDOW(m_wTopLevelWindow), user_w, user_h);
	}

#if 0
	bool cap_resize = false;
	const XAP_Prefs * pPrefs = m_pUnixApp->getPrefs ();
	if (pPrefs) pPrefs->getPrefsValueBool (XAP_PREF_KEY_CapResize, &cap_resize);

	if (cap_resize)
	    gtk_window_resize(GTK_WINDOW(m_wTopLevelWindow), w, h);
	else
	    gtk_widget_set_usize(m_wTopLevelWindow, w, h); // deprecated
#endif

	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.

	if (m_pUnixApp->getFrameCount () <= 1)
		if (user_f & XAP_UnixApp::GEOMETRY_FLAG_POS)
			{
				gtk_widget_set_uposition (m_wTopLevelWindow, user_x, user_y);
			}

	// Remember geometry settings for next time
	m_pUnixApp->getPrefs()->setGeometry (user_x, user_y, user_w, user_h, user_f);
}

/*!
 * This code is used by the dynamic menu API to rebuild the menus after a
 * a change in the menu structure.
 */
void XAP_UnixFrameImpl::_rebuildMenus(void)
{
	// destroy old menu
	m_pUnixMenu->destroy();
	DELETEP(m_pUnixMenu);
	
	// build new one.
	m_pUnixMenu = new EV_UnixMenuBar(m_pUnixApp, getFrame(),
					 m_szMenuLayoutName,
					 m_szMenuLabelSetName);
	UT_ASSERT(m_pUnixMenu);
	bool bResult = m_pUnixMenu->rebuildMenuBar();
	UT_ASSERT(bResult);
}

/*!
 * This code is used by the dynamic toolbar API to rebuild a toolbar after a
 * a change in the toolbar structure.
 */
void XAP_UnixFrameImpl::_rebuildToolbar(UT_uint32 ibar)
{
	XAP_Frame*	pFrame = getFrame();
	// Destroy the old toolbar
	EV_Toolbar * pToolbar = static_cast<EV_Toolbar *>(m_vecToolbars.getNthItem(ibar));
	const char * szTBName = reinterpret_cast<const char *>(m_vecToolbarLayoutNames.getNthItem(ibar));
	EV_UnixToolbar * pUTB = static_cast<EV_UnixToolbar *>(pToolbar);
	UT_sint32 oldpos = pUTB->destroy();

	// Delete the old class
	delete pToolbar;
	if(oldpos < 0) {
		return;
	}

	// Build a new one.
	pToolbar = _newToolbar(m_pUnixApp, pFrame, szTBName,
			       static_cast<const char *>(m_szToolbarLabelSetName));
	static_cast<EV_UnixToolbar *>(pToolbar)->rebuildToolbar(oldpos);
	m_vecToolbars.setNthItem(ibar, static_cast<void *>(pToolbar), NULL);
	// Refill the framedata pointers

	pFrame->refillToolbarsInFrameData();
	pFrame->repopulateCombos();
}

bool XAP_UnixFrameImpl::_close()
{
	gtk_widget_destroy(m_wTopLevelWindow);

	return true;
}

bool XAP_UnixFrameImpl::_raise()
{
	UT_ASSERT(m_wTopLevelWindow);
	gdk_window_raise(m_wTopLevelWindow->window);

	return true;
}

bool XAP_UnixFrameImpl::_show()
{
	if(m_wTopLevelWindow) {
		gtk_widget_show(m_wTopLevelWindow);
	}

	return true;
}

bool XAP_UnixFrameImpl::_updateTitle()
{
	if (!XAP_FrameImpl::_updateTitle() || (m_wTopLevelWindow== NULL) || (m_iFrameMode != XAP_NormalFrame))
	{
		// no relevant change, so skip it
		return false;
	}

	char buf[256];
	buf[0] = 0;

	const char * szAppName = m_pUnixApp->getApplicationTitleForTitleBar();

	int len = 256 - strlen(szAppName) - 4;

	const char * szTitle = getFrame()->getTitle(len);

	sprintf(buf, "%s - %s", szTitle, szAppName);
	if(getFrame()->getFrameMode() == XAP_NormalFrame)
		gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow), buf);
	return true;
}

bool XAP_UnixFrameImpl::_runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
					       UT_sint32 x, UT_sint32 y)
{
	XAP_Frame*	pFrame = getFrame();
	bool bResult = true;

	UT_ASSERT(!m_pUnixPopup);

	// WL_REFACTOR: we DON'T want to do this
	m_pUnixPopup = new EV_UnixMenuPopup(m_pUnixApp, pFrame, szMenuName, m_szMenuLabelSetName);
	if (m_pUnixPopup && m_pUnixPopup->synthesizeMenuPopup())
	{
		// Add our InputMethod selection item to the popup menu. Borrowed
		// from gtkentry.c
		GtkWidget * menuitem;
		GtkWidget * submenu;
		GtkWidget * menu = m_pUnixPopup->getMenuHandle();

		menuitem = gtk_separator_menu_item_new ();
		gtk_widget_show (menuitem);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

		const XAP_StringSet * pSS = m_pUnixApp->getStringSet();

		menuitem = gtk_menu_item_new_with_label (pSS->getValue(XAP_STRING_ID_XIM_Methods));

		gtk_widget_show (menuitem);
		submenu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
		
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

		gtk_im_multicontext_append_menuitems(GTK_IM_MULTICONTEXT(m_imContext), 
											 GTK_MENU_SHELL(submenu));

		// the popup will steal the mouse and so we won't get the
		// button_release_event and we won't know to release our
		// grab.  so let's do it here.  (when raised from a keyboard
		// context menu, we may not have a grab, but that should be ok.
		GtkWidget * w = gtk_grab_get_current();
		if (w)
			gtk_grab_remove(w);

		//
		// OK lets not immediately drop the menu if the user releases the mouse button.
		// From the gtk FAQ.
		//
		GdkEvent * event = gtk_get_current_event();
		GdkEventButton *bevent = reinterpret_cast<GdkEventButton *>(event);

		GtkRequisition req ;
		gtk_widget_size_request (m_pUnixPopup->getMenuHandle(), &req);
		if(w)
		{
			gdk_window_get_origin(w->window, &x,&y);
		}
		x += static_cast<UT_sint32>(bevent->x);
		y += static_cast<UT_sint32>(bevent->y);

		UT_Point pt;
		pt.x = x;
		pt.y = y;

		gtk_menu_popup(GTK_MENU(m_pUnixPopup->getMenuHandle()), NULL, NULL,
			       s_gtkMenuPositionFunc, &pt, bevent->button, bevent->time);


		// We run this menu synchronously, since GTK doesn't.
		// Popup menus have a special "unmap" function to call
		// gtk_main_quit() when they're done.
		gtk_main();
	}

	if (pFrame->getCurrentView())
		pFrame->getCurrentView()->focusChange( AV_FOCUS_HERE);

	DELETEP(m_pUnixPopup);
	return bResult;
}

void XAP_UnixFrameImpl::setTimeOfLastEvent(guint32 eventTime)
{
	m_pUnixApp->setTimeOfLastEvent(eventTime);
}

void XAP_UnixFrameImpl::_queue_resize()
{
	gtk_widget_queue_resize(m_wTopLevelWindow);
}

EV_Menu* XAP_UnixFrameImpl::_getMainMenu()
{
	return m_pUnixMenu;
}

UT_String XAP_UnixFrameImpl::_localizeHelpUrl (bool bLocal, const char * pathBefore, 
											   const char * pathAfter)
{
#ifdef HAVE_GNOME
	UT_String path (pathAfter);
	path += ".html";
	return path;
#else
	return XAP_FrameImpl::_localizeHelpUrl (bLocal, pathBefore, pathAfter);
#endif
}

bool XAP_UnixFrameImpl::_openHelpURL(const char * szURL)
{
#ifdef HAVE_GNOME
	GError * err = NULL;	

	UT_DEBUGMSG(("DOM: Help URL: %s\n", szURL));

	gnome_help_display (szURL, NULL, &err);
	if (err != NULL) {
		UT_DEBUGMSG(("DOM: help error: %s\n", err->message));
		g_error_free (err);
	}
	return false;
#else
	return _openURL (szURL);
#endif
}

bool XAP_UnixFrameImpl::_openURL(const char * szURL)
{
#ifdef HAVE_GNOME
	gnome_url_show(szURL, NULL);
	return false;
#else
	static char *fmtstring = NULL;
  	char *execstring = NULL;

	if(fmtstring)			// lookup browser result when we have
	  {				// already calculated it once before
		if(strstr(fmtstring, "netscape"))
			execstring = g_strdup_printf(fmtstring, szURL, szURL);
		else
		  execstring = g_strdup_printf(fmtstring, szURL);

		system(execstring);
		g_free (execstring);
		return false;					// why do we return false?
	}

	// TODO: we should move this into a preferences item,
	// TODO: but every other platform has a default browser
	// TODO: or text/html handler in a global registry

	// ORDER:
	// Use value of environment variable BROWSER, if valid; otherwise:
	// 1) konqueror
	// 2) mozilla
	// 3) netscape
	// 4) kdehelp
	// 5) lynx in an xterm
  	char *env_browser = getenv ("BROWSER");
  	if (env_browser)
  	{
		if(progExists(env_browser))
		{
			if (strstr (env_browser, "netscape"))
			{
				fmtstring = g_strdup_printf("%s -remote openURL\\('%%s'\\) || %s '%%s' &", env_browser, env_browser);
				if (fmtstring) execstring = g_strdup_printf(fmtstring, szURL, szURL);
			}
			else
			{
				fmtstring = g_strdup_printf("%s '%%s' &", env_browser);
				if (fmtstring) execstring = g_strdup_printf(fmtstring, szURL);
			}
		}
  	}
	if (fmtstring == 0)
	{
		if(progExists("konqueror"))
		{
			fmtstring = "konqueror '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("galeon"))
		{
		  	fmtstring = "galeon '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		// Anyone know how to find out where it might be, regardless?
		else if(progExists("mozilla"))
		{
		        fmtstring = "mozilla '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		// Anyone know how to find out where it might be, regardless?
		else if(progExists("phoenix"))
		{
		        fmtstring = "phoenix '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("netscape"))
		{
			// Try to connect to a running Netscape, if not, start new one
			fmtstring = "netscape -remote openURL\\('%s'\\) || netscape '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL, szURL);
		}
		else if(progExists("khelpcenter"))
		{
			fmtstring = "khelpcenter '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("gnome-help-browser"))
		{
			fmtstring = "gnome-help-browser '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("lynx"))
		{
			fmtstring = "xterm -e lynx '%s' &";
			execstring = g_strdup_printf(fmtstring, szURL);
		}
		else if(progExists("w3m"))
		{
			fmtstring = "xterm -e w3m '%s' &";
	 		execstring = g_strdup_printf(fmtstring, szURL);
	 	}
	}
	if (execstring)
	{
		system (execstring);
		g_free (execstring);
	}

	return false;
#endif
}


void XAP_UnixFrameImpl::_setFullScreen(bool changeToFullScreen)
{
	wmspec_change_layer(changeToFullScreen, GTK_WIDGET(m_wTopLevelWindow)->window);
	wmspec_change_state(changeToFullScreen, GTK_WIDGET(m_wTopLevelWindow)->window,
			    gdk_atom_intern ("_NET_WM_STATE_FULLSCREEN", TRUE),
			    GDK_NONE);
}


EV_Toolbar * XAP_UnixFrameImpl::_newToolbar(XAP_App *pApp, XAP_Frame *pFrame,
					      const char *szLayout,
					      const char *szLanguage)
{
	return (new EV_UnixToolbar(static_cast<XAP_UnixApp *>(pApp), pFrame, szLayout, szLanguage));
}
