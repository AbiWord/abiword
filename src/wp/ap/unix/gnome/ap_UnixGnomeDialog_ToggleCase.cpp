/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixGnomeDialog_ToggleCase.h"
#include "xap_UnixDialogHelper.h"

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_ToggleCase * p = new AP_UnixGnomeDialog_ToggleCase(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_ToggleCase::AP_UnixGnomeDialog_ToggleCase(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_UnixDialog_ToggleCase(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_ToggleCase::~AP_UnixGnomeDialog_ToggleCase(void)
{
}

/*****************************************************************/

static void s_cancel_clicked(GtkWidget * /* widget */,
			     AP_Dialog_ToggleCase * tc)
{
	tc->setAnswer(AP_Dialog_ToggleCase::a_CANCEL);
	gtk_main_quit();
}

static void s_delete_clicked(GtkWidget * w,
			     gpointer /* data */,
			     AP_Dialog_ToggleCase * tc)
{
        s_cancel_clicked (w, tc);
}

static void s_ok_clicked(GtkWidget * /* widget */,
			 AP_Dialog_ToggleCase * tc)
{	tc->setAnswer(AP_Dialog_ToggleCase::a_OK);
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * AP_UnixGnomeDialog_ToggleCase::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowMain;
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	windowMain = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_ToggleCase_Title),
						GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);

	_constructWindowContents(GNOME_DIALOG (windowMain)->vbox);

	buttonOK = GTK_WIDGET (g_list_first (GNOME_DIALOG (windowMain)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (buttonOK);

	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowMain)->buttons)->data);

	g_signal_connect_after(G_OBJECT(windowMain),
				 "destroy",
				 NULL,
				 NULL);
	g_signal_connect(G_OBJECT(windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);

	g_signal_connect(G_OBJECT(buttonOK),
			   "clicked",
			   G_CALLBACK(s_ok_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(buttonCancel),
			   "clicked",
			   G_CALLBACK(s_cancel_clicked),
			   (gpointer) this);
	
	g_signal_connect (G_OBJECT(windowMain),
			    "close",
			    G_CALLBACK(s_cancel_clicked),
			    (gpointer) this);

	setDefaultButton (GNOME_DIALOG(windowMain), 1);
	return windowMain;
}
