/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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
#include "xap_Strings.h"
#include "xap_UnixGnomeDlg_PluginManager.h"
#include "ut_dialogHelper.h"

XAP_Dialog * XAP_UnixGnomeDialog_PluginManager::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_PluginManager * p = new XAP_UnixGnomeDialog_PluginManager(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_PluginManager::XAP_UnixGnomeDialog_PluginManager(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_UnixDialog_PluginManager(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_PluginManager::~XAP_UnixGnomeDialog_PluginManager(void)
{
}

/*****************************************************************/

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 gpointer /* dlg */)
{
	gtk_main_quit();
}

static void s_close_clicked(GtkWidget * /* widget */,
							gpointer /* dlg */)
{
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * XAP_UnixGnomeDialog_PluginManager::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowPluginManager;
	GtkWidget *buttonClose;

	windowPluginManager = gnome_dialog_new (pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangTitle),
						GNOME_STOCK_BUTTON_CLOSE, NULL);

	buttonClose = GTK_WIDGET (g_list_first (GNOME_DIALOG (windowPluginManager)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (buttonClose, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (buttonClose);

	gtk_signal_connect_after(GTK_OBJECT(windowPluginManager),
							 "destroy",
							 NULL,
							 NULL);

	gtk_signal_connect(GTK_OBJECT(windowPluginManager),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_delete_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(buttonClose),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_close_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(windowPluginManager),
					   "close",
					   GTK_SIGNAL_FUNC(s_close_clicked),
					   (gpointer) this);

	// make cancel button the default
	setDefaultButton (GNOME_DIALOG (windowPluginManager), 1);

	m_windowMain = windowPluginManager;
	_constructWindowContents(GNOME_DIALOG (windowPluginManager)->vbox);

	return windowPluginManager;
}
