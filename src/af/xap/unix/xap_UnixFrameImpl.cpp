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
 
  /*
  * Port to Maemo Development Platform 
  * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
  */

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <vector>

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_files.h"
#include "ut_sleep.h"
#include "xap_ViewListener.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrameImpl.h"
#include "xap_Frame.h"
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
#include "xap_Prefs.h"

#include "fv_View.h"

#ifdef HAVE_GNOME
// sorry about the XAP/AP separation breakage, but this is more important
#include "ie_types.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>

#include "ev_GnomeToolbar.h"
#endif

#ifdef HAVE_GNOME

enum {
	TARGET_DOCUMENT,
 	TARGET_IMAGE,
	TARGET_URI_LIST,
	TARGET_URL,
	TARGET_UNKNOWN
};

static const GtkTargetEntry static_drag_types[] = {
	{"text/uri-list", 	0, TARGET_URI_LIST},
	{"text/html", 		0, TARGET_URL}, // hack
	{"text/html+xml", 	0, TARGET_URL}, // hack
	{"_NETSCAPE_URL", 	0, TARGET_URL}
};

static GtkTargetEntry *drag_types = NULL;
static guint 		   n_drag_types = 0;

/*!
 * Build targets table from supported mime types.
 */
static void
s_init_drag_types (void)
{
	std::vector<const std::string *> mimeTypes;
	std::vector<const std::string *>::iterator iter;
	std::vector<const std::string *>::iterator end;
	guint idx;

	UT_ASSERT(drag_types == NULL);

	n_drag_types = G_N_ELEMENTS(static_drag_types) + 
				   IE_Imp::getSupportedMimeTypes().size() + 
				   IE_ImpGraphic::getSupportedMimeTypes().size();

	drag_types = new GtkTargetEntry[n_drag_types];

	// static types
	for (idx = 0; idx < G_N_ELEMENTS(static_drag_types); idx++) {
		drag_types[idx].target = static_drag_types[idx].target;
		drag_types[idx].flags = static_drag_types[idx].flags;
		drag_types[idx].info = static_drag_types[idx].info;
	}

	// document types
	mimeTypes = IE_Imp::getSupportedMimeTypes();
	iter = mimeTypes.begin();
	end = mimeTypes.end();
	while (iter != end) {
		drag_types[idx].target = const_cast<gchar *>((*iter)->c_str());
		drag_types[idx].flags = 0;
		drag_types[idx].info = TARGET_DOCUMENT;
		iter++;
		idx++;
	}
	
	// image types
	mimeTypes = IE_ImpGraphic::getSupportedMimeTypes();
	iter = mimeTypes.begin();
	end = mimeTypes.end();
	while (iter != end) {
		drag_types[idx].target = const_cast<gchar *>((*iter)->c_str());
		drag_types[idx].flags = 0;
		drag_types[idx].info = TARGET_IMAGE;
		iter++;
		idx++;
	}

	UT_ASSERT(idx < n_drag_types);
}

static int
s_mapMimeToUriType (const char * uri)
{
	if (!uri || !strlen(uri))
		return TARGET_UNKNOWN;

	char * mime = gnome_vfs_get_mime_type (uri);
	UT_DEBUGMSG(("DOM: mime %s dropped into AbiWord(%s)\n", mime, uri));

	int target = TARGET_UNKNOWN;

	for (size_t i = 0; i < n_drag_types; i++)
		if (!UT_stricmp (mime, drag_types[i].target)) {
			target = drag_types[i].info;
			break;
		}
	
	g_free (mime);
	return target;
}

static bool
s_ensure_uri_on_disk (const gchar * uri, UT_UTF8String & outName)
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
			UT_DEBUGMSG(("DOM: Invalid uri was %s \n", uri));
			return false;
		}

	if (gnome_vfs_uri_is_local (hndl)) {
		char * short_name = gnome_vfs_uri_to_string (hndl, static_cast<GnomeVFSURIHideOptions>(GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD |
													 GNOME_VFS_URI_HIDE_HOST_NAME | GNOME_VFS_URI_HIDE_HOST_PORT |
													 GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD | GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER));
		UT_uint32 i = 0;
		UT_uint32 len = strlen(short_name);
		UT_UTF8String left = "";
		while(i < len)
		{
			if(short_name[i] != '%')
			{
				left += short_name[i];
				i += 1;
			}
			else if(i+2 < len)
			{
				char c = 16*(short_name[i+1] - '0') +  short_name[i+2] - '0';
				left += c;
				i += 3;
			}
			else
			{
				left += short_name[i];
				i +=1;
			}
		}
		outName = left;
		UT_DEBUGMSG(("short_name %s szTmp %s \n",short_name,outName.utf8_str()));
		g_free (short_name);
		gnome_vfs_uri_unref (hndl);
		return true;
	}

	char * outCName;
	int fd = g_file_open_tmp ("XXXXXX", &outCName, NULL);
	if (fd == -1) {
		UT_DEBUGMSG(("DOM: no temp dir\n"));
		gnome_vfs_uri_unref (hndl);
		return false;
	}
	UT_uint32 i = 0;
	UT_uint32 len = strlen(outCName);
	UT_UTF8String left = "";
	while(i < len)
	{
		if(outCName[i] != '%')
		{
			left += outCName[i];
			i += 1;
		}
		else if(i+2 < len)
		{
			char c = 16*(outCName[i+1] - '0') +  outCName[i+2] - '0';
			left += c;
			i += 3;
		}
		else
		{
			left += outCName[i];
			i +=1;
		}
	}
	outName = left;
	UT_DEBUGMSG(("outCName %s szTmp %s \n",outCName,outName.utf8_str()));
	g_free (outCName);

	FILE * onDisk = fdopen (fd, "r");
	result = gnome_vfs_open_uri (&handle, hndl, GNOME_VFS_OPEN_READ);
	
	if(result != GNOME_VFS_OK)
		{
			UT_DEBUGMSG(("DOM: couldn't open handle\n"));
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
s_load_image (const UT_UTF8String & file, XAP_Frame * pFrame, FV_View * pView)
{
	IE_ImpGraphic * pIEG = 0;
	FG_Graphic    * pFG  = 0;
	UT_Error error = IE_ImpGraphic::constructImporter(file.utf8_str(), 0, &pIEG);
	if (error != UT_OK || !pIEG)
		{
			UT_DEBUGMSG(("Couldn't construct importer for %s\n", file.utf8_str()));
			return;
		}

	error = pIEG->importGraphic(file.utf8_str(), &pFG);

	DELETEP(pIEG);

	if (error != UT_OK || !pFG)
		{
			UT_DEBUGMSG(("Dom: could not import graphic (%s)\n", file.utf8_str()));
			return;
		}

	pView->cmdInsertGraphic(pFG);
	DELETEP(pFG);
}

static void
s_load_image (UT_ByteBuf * bytes, XAP_Frame * pFrame, FV_View * pView)
{
	IE_ImpGraphic * pIEG = 0;
	FG_Graphic    * pFG  = 0;
	UT_Error error = IE_ImpGraphic::constructImporter(bytes, 0, &pIEG);
	if (error != UT_OK || !pIEG)
		{
			UT_DEBUGMSG(("Couldn't construct importer for data buffer\n"));
			return;
		}

	error = pIEG->importGraphic(bytes, &pFG);

	DELETEP(pIEG);

	if (error != UT_OK || !pFG)
		{
			UT_DEBUGMSG(("JK: could not import graphic from data buffer\n"));
			return;
		}

	pView->cmdInsertGraphic(pFG);
	DELETEP(pFG);
}

static void
s_load_document (const UT_UTF8String & file, XAP_Frame * pFrame)
{
	XAP_Frame * pNewFrame = 0;
	if (pFrame->isDirty() || pFrame->getFilename() || 
		(pFrame->getViewNumber() > 0))
		pNewFrame = XAP_App::getApp()->newFrame ();
	else
		pNewFrame = pFrame;

	UT_Error error = pNewFrame->loadDocument(file.utf8_str(), 0 /* IEFT_Unknown */);
	if (error)
		{
			// TODO: warn user that we couldn't open that file		 
			// TODO: we crash if we just delete this without putting something
			// TODO: in it, so let's go ahead and open an untitled document
			// TODO: for now.
			UT_DEBUGMSG(("DOM: couldn't load document %s\n", file.utf8_str()));
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
			UT_DEBUGMSG(("DOM: unknown uri type: %s\n", uri));
			return;
		}

	UT_UTF8String onDisk;
	if (!s_ensure_uri_on_disk (uri, onDisk))
		{
			UT_DEBUGMSG(("DOM: couldn't ensure %s on disk\n", uri));
			return;
		}

	UT_DEBUGMSG(("DOM: %s on disk\n", onDisk.utf8_str()));

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
s_load_uri_list (XAP_Frame * pFrame, const char * uriList)
{
	GList * names = gnome_vfs_uri_list_parse (uriList);

	// multiple URIs
	for ( ; names != NULL; names = names->next) 
		{
			GnomeVFSURI * hndl = static_cast<GnomeVFSURI *>(names->data);
			gchar * uri = gnome_vfs_uri_to_string (hndl, GNOME_VFS_URI_HIDE_NONE);
			s_load_uri (pFrame, uri);
			g_free (uri);
		}

	gnome_vfs_uri_list_free (names);
}

static void 
s_dnd_drop_event(GtkWidget        *widget,
				 GdkDragContext   *context,
				 gint              /*x*/,
				 gint              /*y*/,
				 GtkSelectionData *selection_data,
				 guint             info,
				 guint             /*time*/,
				 XAP_UnixFrameImpl * pFrameImpl)
{
	UT_DEBUGMSG(("DOM: dnd_drop_event being handled\n"));

	g_return_if_fail(widget != NULL);

	XAP_Frame * pFrame = pFrameImpl->getFrame ();
	FV_View   * pView  = static_cast<FV_View*>(pFrame->getCurrentView ());

	char *targetName = gdk_atom_name(selection_data->target);
	UT_DEBUGMSG(("JK: target in selection = %s \n", targetName));
	g_free (targetName);

	if (info == TARGET_URI_LIST) 
		{
			const char * rawChar = reinterpret_cast<const char *>(selection_data->data);
			UT_DEBUGMSG(("DOM: text in selection = %s \n", rawChar));
			s_load_uri_list (pFrame, rawChar);
		} 
	else if (info == TARGET_DOCUMENT) 
		{
			UT_DEBUGMSG(("JK: Document target as data buffer\n"));
		}
	else if (info == TARGET_IMAGE) 
		{
			UT_ByteBuf * bytes = new UT_ByteBuf( selection_data->length );
		  
			UT_DEBUGMSG(("JK: Image target\n"));
			bytes->append (selection_data->data, selection_data->length);
			s_load_image (bytes, pFrame, pView);
		}
	else if (info == TARGET_URL)
		{
			const char * uri = reinterpret_cast<const char *>(selection_data->data);
			UT_DEBUGMSG(("DOM: hyperlink: %s\n", uri));
			pView->cmdInsertHyperlink(uri);
		}
	else if (info == TARGET_URI_LIST) 
		{
			UT_DEBUGMSG(("JK: 'URI List' target\n"));
		}
}

static void
s_dnd_real_drop_event (GtkWidget *widget, GdkDragContext * context, 
					   gint x, gint y, guint time, gpointer ppFrame)
{
	UT_DEBUGMSG(("DOM: dnd drop event\n"));
	GdkAtom selection = gdk_drag_get_selection(context);
	gtk_drag_get_data (widget,context,selection,time);
}

static void
s_dnd_drag_end (GtkWidget  *widget, GdkDragContext *context, gpointer ppFrame)
{
	UT_DEBUGMSG(("DOM: dnd end event\n"));
}

#endif // HAVE_GNOME

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/****************************************************************/
XAP_UnixFrameImpl::XAP_UnixFrameImpl(XAP_Frame *pFrame) : 
	XAP_FrameImpl(pFrame),
	m_imContext(NULL),
	m_pUnixMenu(NULL),
	need_im_reset (false),
	m_bDoZoomUpdate(false),
	m_iNewX(0),
	m_iNewY(0),
	m_iNewWidth(0),
	m_iNewHeight(0),
	m_iZoomUpdateID(0),
	m_iAbiRepaintID(0),
	m_pUnixPopup(NULL),
	m_dialogFactory(XAP_App::getApp(), pFrame)
{
}

XAP_UnixFrameImpl::~XAP_UnixFrameImpl() 
{ 	
	if(m_bDoZoomUpdate) {
		g_source_remove(m_iZoomUpdateID);
	}

	// only delete the things we created...
	if(m_iAbiRepaintID)
	{
		g_source_remove(m_iAbiRepaintID);
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
	xxx_UT_DEBUGMSG(("oooooooooooooooooooooooo XAP_UnixFrameImpl::focusIMIn(), this 0x%x\n", this));
	need_im_reset = true;
	gtk_im_context_focus_in(getIMContext());
	gtk_im_context_reset (getIMContext());
}

void XAP_UnixFrameImpl::focusIMOut ()
{
	xxx_UT_DEBUGMSG(("oooooooooooooooooooooooo XAP_UnixFrameImpl::focusIMOut(), this 0x%x\n", this));
	need_im_reset = true;
	gtk_im_context_focus_out(getIMContext());
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
	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	UT_ASSERT(pFrameImpl);
	xxx_UT_DEBUGMSG(("\n===========================\nfocus_in_event: frame 0x%x\n=========================\n", pFrameImpl));
	
	XAP_Frame* pFrame = pFrameImpl->getFrame();
	g_object_set_data(G_OBJECT(w), "toplevelWindowFocus",
						GINT_TO_POINTER(TRUE));
	if (pFrame->getCurrentView())
	{
		pFrame->getCurrentView()->focusChange(gtk_grab_get_current() == NULL || gtk_grab_get_current() == w ? AV_FOCUS_HERE : AV_FOCUS_NEARBY);
	}
	pFrameImpl->focusIMIn ();
	//
	// FIXME: Return TRUE because of a bug in gtk2. If you return FALSE (like
	// you really should) you get a superfluous expose event which causes the
	// the screen to flicker. Thanks the Bernhard Herzoff (of Sketch fame) 
	// for giving the helpful clue here. 
    //
	// Try it again for gtk2.2 to see if this bug is still present.
	//
	return TRUE; 
}

gboolean XAP_UnixFrameImpl::_fe::focus_out_event(GtkWidget *w,GdkEvent */*event*/,gpointer /*user_data*/)
{
	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	UT_ASSERT(pFrameImpl);
	xxx_UT_DEBUGMSG(("\n===========================\nfocus_out_event: frame 0x%x\n=========================\n", pFrameImpl));
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
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	pUnixFrameImpl->setTimeOfLastEvent(e->time);
	AV_View * pView = pFrame->getCurrentView();
	EV_UnixMouse * pUnixMouse = static_cast<EV_UnixMouse *>(pFrame->getMouse());
 
	gtk_grab_add(w);

	pUnixFrameImpl->resetIMContext ();

	if (pView)
		pUnixMouse->mouseClick(pView,e);
	return 1;
}

gint XAP_UnixFrameImpl::_fe::button_release_event(GtkWidget * w, GdkEventButton * e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
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
	UT_sint32 prevWidth = 0;
	UT_sint32 prevHeight = 0;
	UT_sint32 iNewWidth = 0;
	UT_sint32 iNewHeight = 0;
	if(pView)
	{
		prevWidth = pView->getGraphics()->tdu(pView->getWindowWidth());
		prevHeight = pView->getGraphics()->tdu(pView->getWindowHeight());
		iNewWidth = pUnixFrameImpl->m_iNewWidth;
		iNewHeight = pUnixFrameImpl->m_iNewHeight;
		xxx_UT_DEBUGMSG(("OldWidth %d NewWidth %d \n",prevWidth,iNewWidth));
	}
	if(!pView || pFrame->isFrameLocked() ||
	   ((pUnixFrameImpl->m_bDoZoomUpdate) && (prevWidth == iNewWidth) && (prevHeight == iNewHeight)))
	{
		pUnixFrameImpl->m_iZoomUpdateID = 0;
		pUnixFrameImpl->m_bDoZoomUpdate = false;
		if(pView && !pFrame->isFrameLocked())
		{
				GR_Graphics * pGr = pView->getGraphics ();
				UT_Rect rClip;
				rClip.left = pGr->tlu(0);
				UT_sint32 iHeight = abs(iNewHeight - prevHeight);
				rClip.top = pGr->tlu(iNewHeight - iHeight);
				rClip.width = pGr->tlu(iNewWidth)+1;
				rClip.height = pGr->tlu(iHeight)+1;
				pView->setWindowSize(iNewWidth, iNewHeight);
				pView->draw(&rClip);
		}
		if(pView)
			pView->setWindowSize(iNewWidth, iNewHeight);
		return FALSE;
	}
	if(!pView || pFrame->isFrameLocked() ||
	   ((prevWidth == iNewWidth) && (pFrame->getZoomType() != XAP_Frame::z_WHOLEPAGE)))
	{
		xxx_UT_DEBUGMSG(("Abandoning zoom widths are equal \n"));
		pUnixFrameImpl->m_iZoomUpdateID = 0;
		pUnixFrameImpl->m_bDoZoomUpdate = false;
		if(pView && !pFrame->isFrameLocked())
		{
				GR_Graphics * pGr = pView->getGraphics ();
				UT_Rect rClip;
				rClip.left = pGr->tlu(0);
				UT_sint32 iHeight = abs(iNewHeight - prevHeight);
				rClip.top = pGr->tlu(iNewHeight - iHeight);
				rClip.width = pGr->tlu(iNewWidth)+1;
				rClip.height = pGr->tlu(iHeight)+1;
				pView->setWindowSize(iNewWidth, iNewHeight);
				pView->draw(&rClip);
		}
		if(pView)
			pView->setWindowSize(iNewWidth, iNewHeight);
		return FALSE;
	}

	pUnixFrameImpl->m_bDoZoomUpdate = true;
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

		// don't need this for quickZoom
#if 0
		pUnixFrameImpl->_startViewAutoUpdater(); 
#endif
		pView->setWindowSize(iNewWidth, iNewHeight);
		pFrame->quickZoom(); // was update zoom

	}
	while((iNewWidth != pUnixFrameImpl->m_iNewWidth) || (iNewHeight != pUnixFrameImpl->m_iNewHeight));

	pUnixFrameImpl->m_iZoomUpdateID = 0;
	pUnixFrameImpl->m_bDoZoomUpdate = false;
	return FALSE;
}

gint XAP_UnixFrameImpl::_fe::configure_event(GtkWidget* w, GdkEventConfigure *e)
{
	// This is basically a resize event.

	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();
	if (pView)
	{
		pUnixFrameImpl->m_iNewWidth = e->width;
		pUnixFrameImpl->m_iNewHeight = e->height;
		XAP_App * pApp = XAP_App::getApp();
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

#ifdef HAVE_HILDON
#else
        GtkWindow * pWin = NULL;
		if(pFrame->getFrameMode() == XAP_NormalFrame)
			pWin = GTK_WINDOW(pUnixFrameImpl->m_wTopLevelWindow);
        else
            pWin = GTK_WINDOW(pUnixFrameImpl->m_wTopLevelWindow->window);

        gint gwidth,gheight;
        gtk_window_get_size(pWin,&gwidth,&gheight);
        pApp->setGeometry(e->x,e->y,gwidth,gheight,flags);
#endif

		// Dynamic Zoom Implementation

		if(!pUnixFrameImpl->m_bDoZoomUpdate && (pUnixFrameImpl->m_iZoomUpdateID == 0))
		{
			pUnixFrameImpl->m_iZoomUpdateID = g_idle_add(static_cast<GSourceFunc>(do_ZoomUpdate), static_cast<gpointer>(pUnixFrameImpl));
		}
	}
	return 1;
}

gint XAP_UnixFrameImpl::_fe::motion_notify_event(GtkWidget* w, GdkEventMotion* e)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	if(e->type == GDK_MOTION_NOTIFY)
	{
		//
		// swallow queued drag events and just get the last one.
		//
		GdkEvent  * eNext = gdk_event_peek();
		if(eNext && eNext->type == GDK_MOTION_NOTIFY)
		{
			g_object_unref(G_OBJECT(e));
			e = reinterpret_cast<GdkEventMotion *>(eNext);
			while(eNext && eNext->type == GDK_MOTION_NOTIFY)
			{
				xxx_UT_DEBUGMSG(("Swallowing drag event \n"));
				gdk_event_free(eNext);
				eNext = gdk_event_get();
				gdk_event_free(reinterpret_cast<GdkEvent *>(e));
				e = reinterpret_cast<GdkEventMotion *>(eNext);
				eNext = gdk_event_peek();
			}
			if(eNext != NULL)
			{
				gdk_event_free(eNext);
			}
		}
	}

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
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
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
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));

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
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));

	// Let IM handle the event first.
	if (gtk_im_context_filter_keypress(pUnixFrameImpl->getIMContext(), e)) {
		pUnixFrameImpl->queueIMReset ();

		if ((e->state & GDK_MOD1_MASK) ||
			(e->state & GDK_MOD3_MASK) ||
			(e->state & GDK_MOD4_MASK))
			return 0;

		// ... else, stop this signal
		g_signal_stop_emission (G_OBJECT(w), 
								g_signal_lookup ("key_press_event", 
												 G_OBJECT_TYPE (w)), 0);
		return 1;
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
	g_signal_stop_emission (G_OBJECT(w), 
							g_signal_lookup ("key_press_event", 
											 G_OBJECT_TYPE (w)), 0);
	return 1;
}

gint XAP_UnixFrameImpl::_fe::delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	XAP_App * pApp = XAP_App::getApp();
	UT_ASSERT(pApp);
	if(pApp->isBonoboRunning())
		return FALSE;

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
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	FV_View * pView = static_cast<FV_View *>(pUnixFrameImpl->getFrame()->getCurrentView());
	if((pUnixFrameImpl->m_bDoZoomUpdate) || (pUnixFrameImpl->m_iZoomUpdateID != 0))
	{
		return TRUE;
	}
	if(pView)
	{
		GR_Graphics * pGr = pView->getGraphics ();
		UT_Rect rClip;
		xxx_UT_DEBUGMSG(("Expose area: x %d y %d width %d  height %d \n",pExposeEvent->area.x,pExposeEvent->area.y,pExposeEvent->area.width,pExposeEvent->area.height));
		rClip.left = pGr->tlu(pExposeEvent->area.x);
		rClip.top = pGr->tlu(pExposeEvent->area.y);
		rClip.width = pGr->tlu(pExposeEvent->area.width)+1;
		rClip.height = pGr->tlu(pExposeEvent->area.height)+1;
		pGr->setExposePending(false);
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
	xxx_UT_DEBUGMSG(("-----------------Doing Repaint----------------\n"));
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
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
	XAP_Frame* pFrame = pUnixFrameImpl->getFrame();
	AV_View * pView = pFrame->getCurrentView();

	if (pView)
		pView->sendVerticalScrollEvent(static_cast<UT_sint32>(w->value));
}

void XAP_UnixFrameImpl::_fe::hScrollChanged(GtkAdjustment * w, gpointer /*data*/)
{
	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(w), "user_data"));
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
	UT_DEBUGMSG (("XAP_UnixFrameImpl::_initialize()\n"));

    	// get a handle to our keyboard binding mechanism
 	// and to our mouse binding mechanism.
 	EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
 	UT_ASSERT(pEEM);

	m_pKeyboard = new ev_UnixKeyboard(pEEM);
	UT_ASSERT(m_pKeyboard);

	m_pMouse = new EV_UnixMouse(pEEM);
	UT_ASSERT(m_pMouse);

	//
	// Start background repaint
	//
#if 0
	if(m_iAbiRepaintID == 0)
		m_iAbiRepaintID = g_timeout_add_full(0,100,static_cast<GtkFunction>(XAP_UnixFrameImpl::_fe::abi_expose_repaint), static_cast<gpointer>(this),NULL);
	else
	{
		g_source_remove(m_iAbiRepaintID);
		m_iAbiRepaintID = g_timeout_add_full(0, 100,static_cast<GtkFunction>(XAP_UnixFrameImpl::_fe::abi_expose_repaint), static_cast<gpointer>(this), NULL);
	}
#endif
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

	case GR_Graphics::GR_CURSOR_HLINE_DRAG:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_VLINE_DRAG:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_CROSSHAIR:
		cursor_number = GDK_CROSSHAIR;
		break;

	case GR_Graphics::GR_CURSOR_DOWNARROW:
		cursor_number = GDK_SB_DOWN_ARROW;
		break;

	case GR_Graphics::GR_CURSOR_DRAGTEXT:
		cursor_number = GDK_TARGET;
		break;

	case GR_Graphics::GR_CURSOR_COPYTEXT:
		cursor_number = GDK_DRAPED_BOX;
		break;
	}
	xxx_UT_DEBUGMSG(("Set cursor number in Frame %d to %d \n",c,cursor_number));
	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(getTopLevelWindow()->window, cursor);
	gdk_window_set_cursor(getVBoxWidget()->window, cursor);

	gdk_window_set_cursor(m_wSunkenBox->window, cursor);
	gdk_window_set_cursor(m_wStatusBar->window, cursor);
	gdk_cursor_unref(cursor);
}

UT_sint32 XAP_UnixFrameImpl::_setInputMode(const char * szName)
{
	UT_sint32 result = XAP_App::getApp()->setInputMode(szName);
	if (result == 1)
	{
		// if it actually changed we need to update keyboard and mouse

		EV_EditEventMapper * pEEM = XAP_App::getApp()->getEditEventMapper();
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
void XAP_UnixFrameImpl::_createTopLevelWindow(void)
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
		gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow),
				     XAP_App::getApp()->getApplicationTitleForTitleBar());
		gtk_window_set_resizable(GTK_WINDOW(m_wTopLevelWindow), TRUE);
		gtk_window_set_role(GTK_WINDOW(m_wTopLevelWindow), "topLevelWindow");
		if ( wmIcon )
			gtk_window_set_icon(GTK_WINDOW(m_wTopLevelWindow), wmIcon);
		
		gtk_window_set_resizable(GTK_WINDOW(m_wTopLevelWindow), TRUE);
		gtk_window_set_role(GTK_WINDOW(m_wTopLevelWindow), "topLevelWindow");		
		
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic_attr", NULL);
		g_object_set_data(G_OBJECT(m_wTopLevelWindow), "ic", NULL);		
		
	}
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindow",
						m_wTopLevelWindow);
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "toplevelWindowFocus",
						GINT_TO_POINTER(FALSE));
	g_object_set_data(G_OBJECT(m_wTopLevelWindow), "user_data", this); 

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
	if (drag_types == NULL) {
		s_init_drag_types();
	}
	gtk_drag_dest_set (m_wTopLevelWindow,
					   GTK_DEST_DEFAULT_ALL,
					   drag_types, 
					   n_drag_types, 
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
	g_object_set_data(G_OBJECT(m_wVBox),"user_data", this);
	gtk_container_add(GTK_CONTAINER(m_wTopLevelWindow), m_wVBox);

	if (m_iFrameMode != XAP_NoMenusWindowLess) {
		// synthesize a menu from the info in our base class.
		m_pUnixMenu = new EV_UnixMenuBar(static_cast<XAP_UnixApp*>(XAP_App::getApp()), getFrame(), m_szMenuLayoutName,
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
	xxx_UT_DEBUGMSG(("\n======================\nXAP_UnixFrameImp::_createIMContext(0x%x), this 0x%x\n====================\n",
				 w, this));
	
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

	XAP_UnixFrameImpl * pImpl = static_cast<XAP_UnixFrameImpl*>(data);
	FV_View * pView = static_cast<FV_View*>(pImpl->getFrame()->getCurrentView ());

	PT_DocPosition begin_p, end_p, here;

	begin_p = pView->mapDocPosSimple (FV_DOCPOS_BOB);
	end_p = pView->mapDocPosSimple (FV_DOCPOS_EOB);
	here = pView->getInsPoint ();

	UT_UCSChar * text = pView->getTextBetweenPos (begin_p, end_p);

	if (!text)
		return TRUE;

	UT_UTF8String utf (text);
	DELETEPV(text);

	gtk_im_context_set_surrounding (context,
									utf.utf8_str(),
									utf.byteLength (),
									g_utf8_offset_to_pointer(utf.utf8_str(), here - begin_p) - utf.utf8_str());

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
	xxx_UT_DEBUGMSG(("\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\nXAP_UnixFrameImpl::_imCommit(0x%x), this 0x%x\n<<<<<<<<<<<<<<<<<<\n",
				 imc, this));
				
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

	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(XAP_App::getApp ());
	pApp->getGeometry (&app_x, &app_y, &app_w, &app_h, &app_f);
	// (ignore app_x, app_y & app_f since the WM will set them for us just fine)
	  
	// This is now done with --geometry parsing.
	if (app_w == 0 || app_w > USHRT_MAX) app_w = 760;
        if (app_h == 0 || app_h > USHRT_MAX) app_h = 520;

	UT_DEBUGMSG(("xap_UnixFrameImpl: app-width=%lu, app-height=%lu\n",
				 static_cast<unsigned long>(app_w),static_cast<unsigned long>(app_h)));

	// set geometry hints as the user requested
	gint user_x = 0;
	gint user_y = 0;
	guint user_w = static_cast<guint>(app_w);
	guint user_h = static_cast<guint>(app_h);
	UT_uint32 user_f = 0;

	pApp->getWinGeometry (&user_x, &user_y, &user_w, &user_h, &user_f);

	UT_DEBUGMSG(("xap_UnixFrameImpl: user-width=%u, user-height=%u\n",
				 static_cast<unsigned>(user_w),static_cast<unsigned>(user_h)));

	// Get fall-back defaults from preferences
	UT_sint32 pref_x = 0;
	UT_sint32 pref_y = 0;
	UT_uint32 pref_w = static_cast<UT_uint32>(app_w);
	UT_uint32 pref_h = static_cast<UT_uint32>(app_h);
	UT_uint32 pref_f = 0;

	pApp->getPrefs()->getGeometry (&pref_x, &pref_y, &pref_w, &pref_h, &pref_f);

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

	if (user_w > USHRT_MAX) user_w = app_w;
        if (user_h > USHRT_MAX) user_h = app_h;

	GdkGeometry geom;
	geom.min_width   = 100;
	geom.min_height  = 100;
	if(getFrame()->getFrameMode() == XAP_NormalFrame)
	{
		gtk_window_set_geometry_hints (GTK_WINDOW(m_wTopLevelWindow), m_wTopLevelWindow, &geom,
									   static_cast<GdkWindowHints>(GDK_HINT_MIN_SIZE));

		gtk_window_set_default_size (GTK_WINDOW(m_wTopLevelWindow), user_w, user_h);
	}

	// Because we're clever, we only honor this flag when we
	// are the first (well, only) top level frame available.
	// This is so the user's window manager can find better
	// places for new windows, instead of having our windows
	// pile upon each other.

	if (pApp->getFrameCount () <= 1)
		if (user_f & XAP_UnixApp::GEOMETRY_FLAG_POS)
			{
				gtk_window_move (GTK_WINDOW(m_wTopLevelWindow), user_x, user_y);
			}

	// Remember geometry settings for next time
	pApp->getPrefs()->setGeometry (user_x, user_y, user_w, user_h, user_f);
			
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
	m_pUnixMenu = new EV_UnixMenuBar(static_cast<XAP_UnixApp*>(XAP_App::getApp()), getFrame(),
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
	pToolbar = _newToolbar(pFrame, szTBName,
			       static_cast<const char *>(m_szToolbarLabelSetName));
	static_cast<EV_UnixToolbar *>(pToolbar)->rebuildToolbar(oldpos);
	m_vecToolbars.setNthItem(ibar, pToolbar, NULL);
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
	gtk_window_present(GTK_WINDOW (m_wTopLevelWindow));	
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

	if(getFrame()->getFrameMode() == XAP_NormalFrame)
	{
		const char * szTitle = getFrame()->getTitle(MAX_TITLE_LENGTH);
		gtk_window_set_title(GTK_WINDOW(m_wTopLevelWindow), szTitle);
	}
	return true;
}

bool XAP_UnixFrameImpl::_runModalContextMenu(AV_View * /* pView */, const char * szMenuName,
					       UT_sint32 x, UT_sint32 y)
{
	XAP_Frame*	pFrame = getFrame();
	bool bResult = true;

	UT_ASSERT(!m_pUnixPopup);

	// WL_REFACTOR: we DON'T want to do this
	m_pUnixPopup = new EV_UnixMenuPopup(static_cast<XAP_UnixApp*>(XAP_App::getApp()),
										pFrame, szMenuName, m_szMenuLabelSetName);
	
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

		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

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

		gtk_menu_popup(GTK_MENU(m_pUnixPopup->getMenuHandle()), NULL, NULL,
			       NULL, NULL, bevent->button, bevent->time);

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
	static_cast<XAP_UnixApp*>(XAP_App::getApp())->setTimeOfLastEvent(eventTime);
}

void XAP_UnixFrameImpl::_queue_resize()
{
	gtk_widget_queue_resize(m_wTopLevelWindow);
}

EV_Menu* XAP_UnixFrameImpl::_getMainMenu()
{
	return m_pUnixMenu;
}


void XAP_UnixFrameImpl::_setFullScreen(bool changeToFullScreen)
{
	if (changeToFullScreen)
		gtk_window_fullscreen (GTK_WINDOW(m_wTopLevelWindow));
	else
		gtk_window_unfullscreen (GTK_WINDOW(m_wTopLevelWindow));
}

EV_Toolbar * XAP_UnixFrameImpl::_newToolbar(XAP_Frame *pFrame,
											const char *szLayout,
											const char *szLanguage)
{
	EV_UnixToolbar *pToolbar = NULL;
#ifdef HAVE_GNOME
	pToolbar = new EV_GnomeToolbar(static_cast<XAP_UnixApp *>(XAP_App::getApp()), pFrame, szLayout, szLanguage);
#else
	pToolbar = new EV_UnixToolbar(static_cast<XAP_UnixApp *>(XAP_App::getApp()), pFrame, szLayout, szLanguage);
#endif
	return pToolbar;
}
