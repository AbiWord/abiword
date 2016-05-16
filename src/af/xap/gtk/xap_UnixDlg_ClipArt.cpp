/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2006 Rob Staudinger <robert.staudinger@gmail.com>
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_ClipArt.h"
#include "xap_UnixDlg_ClipArt.h"

enum {
  COL_PATH,
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  NUM_COLS
};

static gint clipartCount = 0;

/**
 * Create list store for the icon view.
 */
static GtkListStore *
create_store ()
{
	GtkListStore *store;

	store = gtk_list_store_new (NUM_COLS,
								G_TYPE_STRING, 
								G_TYPE_STRING, 
								GDK_TYPE_PIXBUF);

	return store;
}

/**
 * Fill list store.
 */
static gboolean
fill_store (XAP_UnixDialog_ClipArt *self)
{
	gboolean ret = self->fillStore();
	if (!ret) {
		GtkWidget *dlg = self->getDialog ();
		const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet ();
		std::string s;
		pSS->getValueUTF8(XAP_STRING_ID_DLG_CLIPART_Error, s);

		GtkWidget *err = gtk_message_dialog_new (GTK_WINDOW (dlg), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", s.c_str());
		gtk_dialog_run (GTK_DIALOG (err));
		gtk_widget_destroy (err); err = NULL;

		gtk_dialog_response(GTK_DIALOG(dlg), GTK_RESPONSE_CANCEL);
	}
	return FALSE;
}

/**
 * Clipart clicked handler.
 */
static void
item_activated (GtkIconView 			* /*icon_view*/,
				GtkTreePath 			* /*tree_path*/,
				XAP_UnixDialog_ClipArt 	*self)
{
	self->onItemActivated();
}

/**
 *
 */
XAP_Dialog * XAP_UnixDialog_ClipArt::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	XAP_UnixDialog_ClipArt * p = new XAP_UnixDialog_ClipArt(pFactory,id);
	return p;
}

/**
 *
 */
XAP_UnixDialog_ClipArt::XAP_UnixDialog_ClipArt(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_ClipArt(pDlgFactory, id)
{}

/**
 *
 */
XAP_UnixDialog_ClipArt::~XAP_UnixDialog_ClipArt()
{
	this->dir_path = NULL;
	this->progress = NULL;
	this->icon_view = NULL;
	this->store = NULL;
}

/**
 *
 */
void XAP_UnixDialog_ClipArt::runModal(XAP_Frame * pFrame)
{
	GtkWidget	*scroll;
	GList 		*list;
	GError		*error;

	std::string s;
	const XAP_StringSet *pSS = m_pApp->getStringSet ();

	UT_ASSERT(pFrame);

	this->dlg = abiDialogNew ("clipart dialog", TRUE, pSS->getValue (XAP_STRING_ID_DLG_CLIPART_Title));
	gtk_window_set_default_size (GTK_WINDOW (this->dlg), 640, 480);
	abiAddButton(GTK_DIALOG(this->dlg),
                 pSS->getValue(XAP_STRING_ID_DLG_Cancel),
                 GTK_RESPONSE_CANCEL);
	abiAddButton(GTK_DIALOG(this->dlg),
                 pSS->getValue(XAP_STRING_ID_DLG_OK), GTK_RESPONSE_OK);
	connectFocus(GTK_WIDGET(this->dlg), pFrame);

	GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_pack_start(GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(this->dlg))), vbox, TRUE,TRUE,0);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_CLIPART_Loading, s);
	this->progress = gtk_progress_bar_new ();
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (this->progress), s.c_str());
	gtk_box_pack_start (GTK_BOX (vbox), this->progress, FALSE, FALSE, 0);

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

	this->store = create_store ();

	this->icon_view = gtk_icon_view_new ();
	gtk_icon_view_set_text_column (GTK_ICON_VIEW (this->icon_view), COL_DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (this->icon_view), COL_PIXBUF);
	gtk_icon_view_set_column_spacing (GTK_ICON_VIEW (this->icon_view), 0);
	gtk_icon_view_set_row_spacing (GTK_ICON_VIEW (this->icon_view), 0);
	gtk_icon_view_set_columns (GTK_ICON_VIEW (this->icon_view), -1);
	gtk_container_add (GTK_CONTAINER (scroll), this->icon_view);
	g_signal_connect (this->icon_view, "item_activated", G_CALLBACK (item_activated), (gpointer) this);

	gtk_widget_show_all (this->dlg);

	/* Dom says we just use that dir for now and hope for someone to build an openclipart client */
	this->dir_path = getInitialDir ();
	g_idle_add ((GSourceFunc) fill_store, this);

	switch (abiRunModalDialog(GTK_DIALOG(this->dlg), pFrame, this, GTK_RESPONSE_CANCEL, false)) {
	case GTK_RESPONSE_OK:
		list = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (this->icon_view));
		if (list && list->data) {
			gchar *graphic = NULL;
			gchar *graphicUri = NULL;
			GtkTreePath *treePath;
			GtkTreeIter  iter;
			treePath = (GtkTreePath *) list->data;
			gtk_tree_model_get_iter (GTK_TREE_MODEL (this->store), &iter, treePath);
			gtk_tree_model_get (GTK_TREE_MODEL (this->store), &iter, COL_PATH, &graphic, -1);
			if (graphic) {
				error = NULL;
				graphicUri = g_filename_to_uri (graphic, NULL, &error);
				setGraphicName (graphicUri);
				g_free (graphic);
				g_free (graphicUri);
				setAnswer (XAP_Dialog_ClipArt::a_OK);
			}
			else {
				setAnswer (XAP_Dialog_ClipArt::a_CANCEL);
			}
			g_list_foreach (list, (void (*)(void*, void*)) gtk_tree_path_free, NULL);
			g_list_free (list);
		}
		break;
	default:
		break;
	}

	abiDestroyWidget(this->dlg);
}

/**
 * Fill list store updating progress bar as we go.
 */
gboolean XAP_UnixDialog_ClipArt::fillStore()
{
	GDir 		*dir;
	const gchar *name;
	GdkPixbuf	*pixbuf;
	GError		*error;
	GtkTreeIter  iter;
	gint		 _count;

	if (!g_file_test (this->dir_path, G_FILE_TEST_IS_DIR)) {
		return FALSE;
	}

	error = NULL;
	dir = g_dir_open (this->dir_path, 0, &error);
	if (error) {
		g_warning ("%s", error->message);
		g_error_free (error);
		return FALSE;
	}

	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (this->progress), 0.);
	_count = 0;
	name = g_dir_read_name (dir);
	while (name != NULL) {

		gchar *file_path, *display_name;

		/* We ignore hidden files that start with a '.' */
		if (name[0] == '.') {
			goto next;
		}

		file_path = g_build_filename (this->dir_path, name, NULL);
		if (g_file_test (file_path, G_FILE_TEST_IS_DIR)) {
			goto next;
		}

		display_name = g_filename_to_utf8 (name, -1, NULL, NULL, NULL);
		error = NULL;
		pixbuf = gdk_pixbuf_new_from_file_at_size (file_path, 48, 48, &error);
		if (error) {
			g_warning ("%s", error->message);
			g_error_free (error);
			goto next;
		}

		gtk_list_store_append (this->store, &iter);
		gtk_list_store_set (this->store, &iter,
							COL_PATH, file_path,
							COL_DISPLAY_NAME, display_name,
							COL_PIXBUF, pixbuf,
							-1);
		g_free (file_path);					file_path = NULL;
		g_free (display_name); 				display_name = NULL;
		g_object_unref (G_OBJECT (pixbuf)); pixbuf = NULL;

		if (clipartCount) {
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (this->progress), 
										   _count / clipartCount * 100.);
		}
		else {
			gtk_progress_bar_pulse (GTK_PROGRESS_BAR (this->progress));
		}
		_count++;
		if (_count % 10 == 0) {
			gtk_main_iteration_do (FALSE);
		}
next:
		name = g_dir_read_name (dir);
	}
	clipartCount = _count;

	gtk_icon_view_set_model (GTK_ICON_VIEW (this->icon_view), 
							 GTK_TREE_MODEL (this->store));
	g_object_unref (G_OBJECT (this->store));
	gtk_widget_hide (this->progress);

	return TRUE;
}

/**
 * Clipart clicked handler.
 */
void XAP_UnixDialog_ClipArt::onItemActivated()
{
	gtk_dialog_response(GTK_DIALOG(this->dlg), GTK_RESPONSE_OK);
}
