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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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

void XAP_UnixDialog_About::runModal(XAP_Frame * pFrame)
{
	static const gchar *authors[] = {"Abi the Ant <abi@abisource.com>",
									 NULL};

	static const gchar *documenters[] = {"David Chart <linux@dchart.demon.co.uk>",
										 NULL};

	static const gchar *copyright = "(c) 1998-2010 Dom Lachowicz and other contributors, GNU GPL v2.0";

	static const gchar *website = "http://www.abisource.com";

	static GdkPixbuf * logo = NULL;
	static GtkWidget * dlg = NULL;

	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
	GtkWidget * parent;

	// TODO Rob: use the more fancy "sidebar.png" logo, just like win32
	if (!logo) {
		UT_String str (DATADIR);
		str += "/icons/abiword_48.png";
		logo = gdk_pixbuf_new_from_file (str.c_str(), NULL); // ignore errors
	}

	// Get the GtkWindow of the parent frame
	// TODO fix that in hildon frame impl
	parent = gtk_widget_get_parent(pUnixFrameImpl->getTopLevelWindow());

	dlg = gtk_about_dialog_new();
	gtk_about_dialog_set_url_hook(onAboutDialogActivate, NULL, NULL);
	gtk_show_about_dialog(GTK_WINDOW (parent), 
						  "authors", authors, 
						  "documenters", documenters, 
						  "copyright", copyright, 
						  "logo", logo, 
						  "version", XAP_App::s_szBuild_Version, 
						  "website", website, 
						  "website-label", website, 
						  NULL);
}
