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

#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_About.h"
#include "xap_UnixGnomeDlg_About.h"

XAP_Dialog * XAP_UnixGnomeDialog_About::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_About * p = new XAP_UnixGnomeDialog_About(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_About::XAP_UnixGnomeDialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_UnixDialog_About(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_About::~XAP_UnixGnomeDialog_About(void)
{
}

/*****************************************************************/

GtkWidget * XAP_UnixGnomeDialog_About::_constructButtonOK(void)
{
	GtkWidget * buttonOK;

	buttonOK = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
	gtk_widget_show (buttonOK);
	gtk_widget_set_usize (buttonOK, 85, 24);

	return buttonOK;
}

GtkWidget * XAP_UnixGnomeDialog_About::_constructButtonURL(void)
{
	GtkWidget * buttonURL;

	buttonURL = gnome_href_new("http://www.abisource.com", "Abisource Site");
	gtk_widget_show (buttonURL);
	gtk_widget_set_usize (buttonURL, 140, 24);

	return buttonURL;
}
