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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
// #include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"
#include "xap_UnixDlg_About.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete p; } while (0)

#define DEFAULT_BUTTON_WIDTH 85

/*****************************************************************/
AP_Dialog * AP_UnixDialog_About::static_constructor(AP_DialogFactory * pFactory,
															 AP_Dialog_Id id)
{
	AP_UnixDialog_About * p = new AP_UnixDialog_About(pFactory,id);
	return p;
}

AP_UnixDialog_About::AP_UnixDialog_About(AP_DialogFactory * pDlgFactory,
											 AP_Dialog_Id id)
	: AP_Dialog_About(pDlgFactory,id)
{
}

AP_UnixDialog_About::~AP_UnixDialog_About(void)
{
}

static void s_okClicked(GtkWidget * widget, gpointer data, gpointer extra)
{
	// just quit out of the dialog
	gtk_main_quit();
}

void AP_UnixDialog_About::runModal(XAP_Frame * pFrame)
{
	GtkWidget* dialog;
	GtkWidget* dialog_vbox1;
	GtkWidget* dialog_action_area1;
	GtkWidget* vbox1;
	GtkWidget* hbox1;
	GtkWidget* label1;
	GtkWidget* label2;
	GtkWidget* label3;
	GtkWidget* label4;
	GtkWidget* label5;
	GtkWidget* label6;
	GtkWidget* okButton;
	GtkWidget* frame;
	
	XAP_App* pApp = pFrame->getApp();
	
	char buf[2048];
	sprintf(buf, XAP_ABOUT_TITLE, pApp->getApplicationName());
	
	dialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT (dialog), "About AbiWord", dialog);
	gtk_window_set_title (GTK_WINDOW (dialog), buf);
	gtk_window_set_policy (GTK_WINDOW (dialog), TRUE, TRUE, FALSE);
	gtk_container_border_width(GTK_CONTAINER(dialog), 5);
    gtk_window_set_policy(GTK_WINDOW(dialog), FALSE, FALSE, TRUE);

	gtk_widget_realize(dialog);
	
	gtk_signal_connect_after(GTK_OBJECT(dialog),
							 "destroy",
							 NULL,
							 NULL);

	gtk_signal_connect_after(GTK_OBJECT(dialog),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_okClicked),
							 NULL);
	
	dialog_vbox1 = GTK_DIALOG (dialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog_vbox1", dialog_vbox1);
	gtk_widget_show (dialog_vbox1);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox1), frame, TRUE, TRUE, 0);
	
	sprintf(buf, XAP_ABOUT_DESCRIPTION, pApp->getApplicationName());
			
	label5 = gtk_label_new (buf);
	gtk_widget_show (label5);
	gtk_container_add(GTK_CONTAINER(frame), label5);
	
	label1 = gtk_label_new (XAP_ABOUT_COPYRIGHT);
	gtk_object_set_data (GTK_OBJECT (dialog), "label1", label1);
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (vbox1), label1, TRUE, TRUE, 5);
	
	sprintf(buf, XAP_ABOUT_GPL, pApp->getApplicationName());
	
	label2 = gtk_label_new (buf);
	gtk_object_set_data (GTK_OBJECT (dialog), "label2", label2);
	gtk_widget_show (label2);
	gtk_box_pack_start (GTK_BOX (vbox1), label2, TRUE, TRUE, 5);
	
	sprintf(buf, "Version: %s", XAP_App::s_szBuild_Version); 

	label3 = gtk_label_new (buf);
	gtk_object_set_data (GTK_OBJECT (dialog), "label3", label3);
	gtk_widget_show (label3);
	gtk_box_pack_start (GTK_BOX (vbox1), label3, TRUE, TRUE, 5);
	
	label4 = gtk_label_new (XAP_ABOUT_URL);
	gtk_object_set_data (GTK_OBJECT (dialog), "label4", label4);
	gtk_widget_show (label4);
	gtk_box_pack_start (GTK_BOX (vbox1), label4, TRUE, TRUE, 5);

	sprintf(buf, "Build options: %s", XAP_App::s_szBuild_Options);
	
	label6 = gtk_label_new (buf);
	gtk_widget_show (label6);
	gtk_box_pack_start (GTK_BOX (vbox1), label6, TRUE, TRUE, 5);

	dialog_action_area1 = GTK_DIALOG (dialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (dialog), "dialog_action_area1", dialog_action_area1);
	gtk_widget_show (dialog_action_area1);
	gtk_container_border_width (GTK_CONTAINER (dialog_action_area1), 10);
	
	okButton = gtk_button_new_with_label ("OK");
	gtk_object_set_data (GTK_OBJECT (dialog), "okButton", okButton);
	gtk_widget_show (okButton);
	gtk_widget_set_usize(okButton, DEFAULT_BUTTON_WIDTH, 0);
	
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), okButton, FALSE, FALSE, 0);

	gtk_signal_connect(GTK_OBJECT(okButton), "clicked", GTK_SIGNAL_FUNC(s_okClicked), NULL);
	
	gtk_widget_show(dialog);

	gtk_grab_add(GTK_WIDGET(dialog));
	
	gtk_main();

	gtk_widget_destroy(dialog);
}

