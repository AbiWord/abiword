/* AbiSource Application Framework
 * Copyright (C) 2003 AbiSource, Inc.
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

#include "xap_Dialog_Id.h"
#include "xap_UnixGnomeDlg_About.h"
#include "xap_UnixDialogHelper.h"
#include "xap_App.h"
#include "ut_string_class.h"

XAP_Dialog * XAP_UnixGnomeDialog_About::static_constructor(XAP_DialogFactory * pFactory,
							   XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_About * p = new XAP_UnixGnomeDialog_About(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_About::XAP_UnixGnomeDialog_About(XAP_DialogFactory * pDlgFactory,
						     XAP_Dialog_Id id)
	: XAP_Dialog_About(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_About::~XAP_UnixGnomeDialog_About(void)
{
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixGnomeDialog_About::runModal(XAP_Frame * pFrame)
{
  static const gchar *authors[] = {"Abi the Ant <abi@abisource.com>",
				   NULL};

  static const gchar *documenters[] = {"David Chart <linux@dchart.demon.co.uk>",
				       NULL};

  static const gchar *gcopystr = "(c) 1998-2003 AbiSource, Inc., GNU GPL v2.0";

  UT_String str (XAP_App::getApp()->getAbiSuiteLibDir());
  str += "/icons/abiword_48.png";

  GdkPixbuf * logo = NULL;
  logo = gdk_pixbuf_new_from_file (str.c_str(), NULL); // ignore errors

  GtkWidget * mainWindow = gnome_about_new("AbiWord", 
					   XAP_App::s_szBuild_Version,
					   gcopystr,
					   NULL,
					   authors,
					   documenters,
					   NULL,
					   logo);
  UT_return_if_fail(mainWindow);

  GtkWidget *hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gnome_href_new ("http://www.abisource.com/", "AbiWord"),
		      FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mainWindow)->vbox),
		      hbox, TRUE, FALSE, 0);  
  gtk_widget_show_all (hbox);

  abiSetupModelessDialog (GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CLOSE, false);
  gtk_widget_show (mainWindow);
}

