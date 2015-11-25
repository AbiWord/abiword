/* 
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "ut_string.h"
#include "ut_assert.h"

#include "xap_Dialog_Id.h"
#include "xap_Frame.h"

#include "xap_UnixApp.h"
#include "xap_UnixFrameImpl.h"
#include "xap_UnixDlg_About.h"

XAP_Dialog * XAP_UnixDialog_About::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	XAP_UnixDialog_About * p = new XAP_UnixDialog_About(pFactory,id);
	return p;
}

XAP_UnixDialog_About::XAP_UnixDialog_About(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_About(pDlgFactory, id)
{}

XAP_UnixDialog_About::~XAP_UnixDialog_About(void)
{}

static void onAboutDialogActivate (GtkAboutDialog 	* /*about*/,
								   const gchar 		*link,
								   gpointer 		 /*data*/)
{
	XAP_App::getApp()->openURL(link);
}

void XAP_UnixDialog_About::runModal(XAP_Frame * /*pFrame*/)
{
	static const gchar *authors[] = {"Abi the Ant <abi@abisource.com>",
									 NULL};

	static const gchar *documenters[] = {"David Chart <linux@dchart.demon.co.uk>",
										 NULL};

	static const gchar *copyright = "(c) 1998-2012 Dom Lachowicz and other contributors, GNU GPL v2.0";

	static const gchar *website = "http://www.abisource.com";

	static GdkPixbuf * logo = NULL;
	static GtkWidget * dlg = NULL;

	// TODO Rob: use the more fancy "sidebar.png" logo, just like win32
	if (!logo) {
		std::string str (ICONDIR);
		str += "/hicolor/48x48/apps/abiword.png";
		logo = gdk_pixbuf_new_from_file (str.c_str(), NULL); // ignore errors
	}

	dlg = gtk_about_dialog_new();
	//JEAN: do we really need the "activate-link" signal?
	g_signal_connect(dlg, "activate-link", G_CALLBACK(onAboutDialogActivate), NULL);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dlg), authors);
	gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(dlg), documenters);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dlg), copyright);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dlg), logo);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dlg), XAP_App::s_szBuild_Version);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dlg), website);
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dlg), website);
	gtk_window_set_icon(GTK_WINDOW(dlg), logo);
	gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
}
