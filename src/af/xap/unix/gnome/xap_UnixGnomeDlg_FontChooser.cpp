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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_dialogHelper.h"
#include "xap_UnixGnomeDlg_FontChooser.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "gr_UnixGraphics.h"
#define SIZE_STRING_SIZE        10


XAP_Dialog * XAP_UnixGnomeDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_FontChooser * p = new XAP_UnixGnomeDialog_FontChooser(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_FontChooser::XAP_UnixGnomeDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_UnixDialog_FontChooser(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_FontChooser::~XAP_UnixGnomeDialog_FontChooser(void)
{
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = XAP_Dialog_FontChooser::a_CANCEL;
	gtk_main_quit();
}

static void s_ok_clicked(GtkWidget * /* widget */,
						 XAP_Dialog_FontChooser::tAnswer * answer)
{	*answer = XAP_Dialog_FontChooser::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * /* widget */,
							 XAP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = XAP_Dialog_FontChooser::a_CANCEL;
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * XAP_UnixGnomeDialog_FontChooser::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowFontSelection;
	GtkWidget *windowContents;

	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;
	XML_Char * unixstr = NULL;

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_UFS_FontTitle) );
	windowFontSelection = gnome_dialog_new (unixstr, NULL);
	FREEP(unixstr);
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "windowFontSelection", windowFontSelection);
	gtk_window_set_policy (GTK_WINDOW (windowFontSelection), FALSE, FALSE, FALSE);

	gnome_dialog_append_button(GNOME_DIALOG(windowFontSelection), GNOME_STOCK_BUTTON_OK);
	gnome_dialog_append_button(GNOME_DIALOG(windowFontSelection), GNOME_STOCK_BUTTON_CANCEL);

	windowContents = constructWindowContents(GTK_OBJECT (windowFontSelection));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (windowFontSelection)->vbox), windowContents, FALSE, FALSE, 0);

	buttonOK = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowFontSelection)->buttons, 0) );
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "buttonOK", buttonOK);
	gtk_widget_grab_default (buttonOK);

	buttonCancel = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (windowFontSelection)->buttons, 1) );
	gtk_object_set_data (GTK_OBJECT (windowFontSelection), "buttonCancel", buttonCancel);

	gtk_signal_connect_after(GTK_OBJECT(windowFontSelection),
							  "destroy",
							  NULL,
							  NULL);
	gtk_signal_connect_after(GTK_OBJECT(windowFontSelection),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) &m_answer);

	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) &m_answer);
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) &m_answer);

	return windowFontSelection;
}
