/* AbiSource Setup Kit
 * Copyright (C) 1999 AbiSource, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "ask.h"

/*
 * This function returns a widget in a component created by Glade.
 * Call it with the toplevel widget in the component (i.e. a window/dialog),
 * or alternatively any widget in the component, and the name of the widget
 * you want returned.
 */
GtkWidget*
get_widget                             (GtkWidget       *widget,
                                        gchar           *widget_name);


 /*
  * This is an internally used function for setting notebook tabs. It is only
  * included in this header file so you don't get compilation warnings
  */
void
set_notebook_tab                       (GtkWidget       *notebook,
                                        gint             page_num,
                                        GtkWidget       *widget);

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory                   (gchar           *directory);

/* This is an internally used function to create pixmaps. */
GtkWidget*
create_pixmap                          (GtkWidget       *widget,
                                        gchar           *filename);

GtkWidget* create_window1 (void);

unsigned int ASK_getFileAttributes(const char* pszFileName)
{
	struct stat st;
	
	stat(pszFileName, &st);

	return st.st_mode;
}

void ASK_setFileAttributes(const char* pszFileName, unsigned int iAttributes)
{
	chmod(pszFileName, iAttributes);
}

GtkWidget* g_window;
GtkWidget* g_table;
GtkWidget* g_button_next;
GtkWidget* g_label_next;
GtkWidget* g_button_cancel;
GtkWidget* g_label_cancel;

#define BUTTON_NONE		0
#define BUTTON_NEXT		1
#define BUTTON_CANCEL	2
#define BUTTON_YES		3
#define BUTTON_NO		4

int g_iWhichButton;
int g_iWhichYesNoButton;

void cb_button_browse_clicked(GtkWidget *widget, gpointer data)
{
	/* TODO */
}

void cb_button_cancel_clicked(GtkWidget *widget, gpointer data)
{
	if (ASK_YesNo("Cancel?", "Are you sure you want to cancel the installation?"))
	{
		g_iWhichButton = BUTTON_CANCEL;
	
		gtk_main_quit();
	}
}

void cb_button_next_clicked(GtkWidget* widget, gpointer data)
{
	g_iWhichButton = BUTTON_NEXT;
	
	gtk_main_quit();
}

gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
		   gpointer   data )
{
    /* If you return FALSE in the "delete_event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

    g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete_event". */

    return(TRUE);
}

/* Another callback */
void destroy( GtkWidget *widget,
              gpointer   data )
{
    gtk_main_quit();
}

GtkWidget* create_window1 ()
{
  GtkWidget *window1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *hseparator1;
  GtkWidget *hbuttonbox1;
  GtkWidget *button_next;
  GtkWidget *button_cancel;
  GtkWidget *label_next;
  GtkWidget *label_cancel;

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window1), "window1", window1);
  gtk_widget_set_usize (window1, 640, 480);
  gtk_window_set_title (GTK_WINDOW (window1), "Setup");
  gtk_window_set_policy (GTK_WINDOW (window1), FALSE, FALSE, FALSE);

  gtk_signal_connect (GTK_OBJECT (window1), "delete_event",
					  GTK_SIGNAL_FUNC (delete_event), NULL);
    
  gtk_signal_connect (GTK_OBJECT (window1), "destroy",
					  GTK_SIGNAL_FUNC (destroy), NULL);
    
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_object_set_data (GTK_OBJECT (window1), "vbox1", vbox1);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (window1), vbox1);

  table1 = gtk_table_new (1, 1, FALSE);
  gtk_object_set_data (GTK_OBJECT (window1), "table1", table1);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);

  hseparator1 = gtk_hseparator_new ();
  gtk_object_set_data (GTK_OBJECT (window1), "hseparator1", hseparator1);
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, FALSE, TRUE, 0);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_object_set_data (GTK_OBJECT (window1), "hbuttonbox1", hbuttonbox1);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (hbuttonbox1), 10);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox1), 10);

  label_next = gtk_label_new("Next");
  gtk_widget_show(label_next);
  button_next = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button_next), label_next);
  gtk_object_set_data (GTK_OBJECT (window1), "button_next", button_next);
  gtk_widget_show (button_next);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button_next);
  gtk_signal_connect (GTK_OBJECT (button_next), "clicked",
					  GTK_SIGNAL_FUNC (cb_button_next_clicked), NULL);

  label_cancel = gtk_label_new("Cancel");
  gtk_widget_show(label_cancel);
  button_cancel = gtk_button_new ();
  gtk_container_add(GTK_CONTAINER(button_cancel), label_cancel);
  gtk_object_set_data (GTK_OBJECT (window1), "button_cancel", button_cancel);
  gtk_widget_show (button_cancel);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button_cancel);
  gtk_signal_connect (GTK_OBJECT (button_cancel), "clicked",
					  GTK_SIGNAL_FUNC (cb_button_cancel_clicked), NULL);

  g_window = window1;
  g_table = table1;
  g_button_next = button_next;
  g_label_next = label_next;
  g_button_cancel = button_cancel;
  g_label_cancel = label_cancel;
  
  return window1;
}

int ASK_WaitForUserAction(void)
{
	gtk_main();

	return (g_iWhichButton == BUTTON_NEXT);
}

int ASK_DoScreen_readme(char* pszText)
{
	int result;
	GtkWidget* text;

	gtk_table_resize(GTK_TABLE(g_table), 1, 1);

	text = gtk_text_new(NULL, NULL);

	gtk_text_insert (GTK_TEXT (text), NULL, &text->style->black, NULL,
					 pszText, -1);
	
	gtk_table_attach (GTK_TABLE (g_table), text, 0, 1, 0, 1,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_show (text);
	
	result = ASK_WaitForUserAction();

	gtk_container_remove(GTK_CONTAINER(g_table), text);

	/* TODO destroy */

	return result;
}

int ASK_DoScreen_license(char* pszText)
{
	int result;
	GtkWidget* text;
	GtkWidget* label;

	gtk_table_resize(GTK_TABLE(g_table), 1, 2);

	text = gtk_text_new(NULL, NULL);

	/* TODO add a vertical scrollbar */
	
	gtk_text_insert (GTK_TEXT (text), NULL, &text->style->black, NULL,
					 pszText, -1);
	
	gtk_table_attach (GTK_TABLE (g_table), text, 0, 1, 0, 1,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_show (text);

	label = gtk_label_new("Do you accept the terms of this license?\nYou must accept this license in order to continue with the installation.");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label), 10, 10);
	gtk_table_attach (GTK_TABLE (g_table), label, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_widget_show (label);

	gtk_label_set_text(GTK_LABEL(g_label_next), "Yes");
	gtk_label_set_text(GTK_LABEL(g_label_cancel), "No");
	
	/* TODO change the labels */
	
	result = ASK_WaitForUserAction();

	gtk_label_set_text(GTK_LABEL(g_label_next), "Next");
	gtk_label_set_text(GTK_LABEL(g_label_cancel), "Cancel");
	
	gtk_container_remove(GTK_CONTAINER(g_table), text);
	gtk_container_remove(GTK_CONTAINER(g_table), label);

	/* TODO destroy */

	return result;
}

int ASK_DoScreen_choosedir(char* pszFileSetName, char* pszDefaultPath, char** ppszPath)
{
	int result;
	GtkWidget *label3;
	GtkWidget *label2;
	GtkWidget *label4;
	GtkWidget* button_browse;
	GtkWidget* entry;
	
	gtk_table_resize(GTK_TABLE(g_table), 6, 3);

	label2 = gtk_label_new ("Choose Directory");
	gtk_object_set_data (GTK_OBJECT (g_window), "label2", label2);
	{
		GtkStyle* style;
		GdkFont* bigfont = gdk_font_load("-adobe-helvetica-bold-r-*-*-34-*-100-100-*-*-*-*");
		
		style = gtk_style_copy (gtk_widget_get_style (label2));
		gdk_font_unref (style->font);
		style->font = bigfont;
		gdk_font_ref (style->font);
		gtk_widget_set_style (label2, style);
	}
	
	gtk_widget_show (label2);
	gtk_table_attach (GTK_TABLE (g_table), label2, 0, 5, 0, 1,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_misc_set_padding (GTK_MISC (label2), 10, 10);

	{
		char buf[2048];

		sprintf(buf, "Please choose a directory where '%s' will be installed.", pszFileSetName);
		label3 = gtk_label_new (buf);
	
		gtk_object_set_data (GTK_OBJECT (g_window), "label3", label3);
		gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
		gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
		gtk_misc_set_padding (GTK_MISC (label3), 10, 10);
		gtk_table_attach (GTK_TABLE (g_table), label3, 0, 5, 1, 2,
						  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL | GTK_EXPAND, 0, 0);
		gtk_widget_show (label3);
	}
	
	label4 = gtk_label_new ("Directory:");
	gtk_object_set_data (GTK_OBJECT (g_window), "label4", label4);
	gtk_widget_show (label4);
	gtk_table_attach (GTK_TABLE (g_table), label4, 0, 1, 2, 3,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_misc_set_padding (GTK_MISC (label4), 10, 10);
	
	button_browse = gtk_button_new_with_label ("Browse...");
	gtk_table_attach (GTK_TABLE (g_table), button_browse, 4, 5, 2, 3,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_widget_show (button_browse);
	gtk_signal_connect (GTK_OBJECT (button_browse), "clicked",
						GTK_SIGNAL_FUNC (cb_button_browse_clicked), NULL);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), pszDefaultPath);
	gtk_widget_show(entry);
	gtk_table_attach (GTK_TABLE (g_table), entry, 1, 4, 2, 3,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);

	result = ASK_WaitForUserAction();

	{
		char* pszPath = gtk_entry_get_text(GTK_ENTRY(entry));

		*ppszPath = strdup(pszPath);
	}
	
	gtk_container_remove(GTK_CONTAINER(g_table), label2);
	gtk_container_remove(GTK_CONTAINER(g_table), label3);
	gtk_container_remove(GTK_CONTAINER(g_table), label4);

	/* TODO destroy everything */
	
	gtk_table_resize(GTK_TABLE(g_table), 1, 1);
	
	return result;
}

int ASK_DoScreen_welcome(void)
{
	int result;
	GtkWidget *label3;
	GtkWidget *label2;
	GtkWidget *label4;
	
	gtk_table_resize(GTK_TABLE(g_table), 1, 3);

	label3 = gtk_label_new ("Welcome to the AbiWord setup program.  This program will install AbiWord\non your system.The setup program will guide you at each step of the process.\nYou may cancel the setup at any time by pressing the Cancel button below.");
	gtk_object_set_data (GTK_OBJECT (g_window), "label3", label3);
	{
		GtkStyle* style;
		GdkFont* bigfont = gdk_font_load("-adobe-helvetica-medium-r-*-*-16-*-100-100-*-*-*-*");
		
		style = gtk_style_copy (gtk_widget_get_style (label3));
		gdk_font_unref (style->font);
		style->font = bigfont;
		gdk_font_ref (style->font);
		gtk_widget_set_style (label3, style);
	}
	gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label3), 10, 10);
	gtk_table_attach (GTK_TABLE (g_table), label3, 0, 1, 1, 2,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_widget_show (label3);

	label2 = gtk_label_new ("AbiWord Setup");
	gtk_object_set_data (GTK_OBJECT (g_window), "label2", label2);
	{
		GtkStyle* style;
		GdkFont* bigfont = gdk_font_load("-adobe-helvetica-bold-r-*-*-34-*-100-100-*-*-*-*");
		
		style = gtk_style_copy (gtk_widget_get_style (label2));
		gdk_font_unref (style->font);
		style->font = bigfont;
		gdk_font_ref (style->font);
		gtk_widget_set_style (label2, style);
	}
	
	gtk_widget_show (label2);
	gtk_table_attach (GTK_TABLE (g_table), label2, 0, 1, 0, 1,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);
	gtk_misc_set_padding (GTK_MISC (label2), 10, 10);

	label4 = gtk_label_new ("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n");
	gtk_object_set_data (GTK_OBJECT (g_window), "label4", label4);
	gtk_misc_set_alignment (GTK_MISC (label4), 0, 0);
	gtk_label_set_line_wrap(GTK_LABEL(label4), TRUE);
	gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label4), 10, 10);
	gtk_widget_show (label4);
	gtk_table_attach (GTK_TABLE (g_table), label4, 0, 1, 2, 3,
					  (GtkAttachOptions) GTK_EXPAND | GTK_FILL, (GtkAttachOptions) GTK_FILL, 0, 0);

	result = ASK_WaitForUserAction();

	gtk_container_remove(GTK_CONTAINER(g_table), label2);
	gtk_container_remove(GTK_CONTAINER(g_table), label3);
	gtk_container_remove(GTK_CONTAINER(g_table), label4);

	/* TODO destroy these labels */
	
	gtk_table_resize(GTK_TABLE(g_table), 1, 1);
	
	return result;
}

void cb_button_no_clicked(GtkWidget *widget, gpointer data)
{
	g_iWhichYesNoButton = BUTTON_NO;
	
	gtk_main_quit();
}

void cb_button_yes_clicked(GtkWidget* widget, gpointer data)
{
	g_iWhichYesNoButton = BUTTON_YES;
	
	gtk_main_quit();
}

int ASK_YesNo(char* pszTitle, char* pszMessage)
{
	GtkWidget *dialog1;
	GtkWidget *dialog_vbox1;
	GtkWidget *label1;
	GtkWidget *dialog_action_area1;
	GtkWidget *hbuttonbox1;
	GtkWidget *button2;
	GtkWidget *button3;

	dialog1 = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT (dialog1), "dialog1", dialog1);
	gtk_window_set_title (GTK_WINDOW (dialog1), pszTitle);
	gtk_window_set_policy (GTK_WINDOW (dialog1), TRUE, TRUE, FALSE);

	dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
	gtk_object_set_data (GTK_OBJECT (dialog1), "dialog_vbox1", dialog_vbox1);
	gtk_widget_show (dialog_vbox1);

	label1 = gtk_label_new (pszMessage);
	gtk_object_set_data (GTK_OBJECT (dialog1), "label1", label1);
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), label1, TRUE, TRUE, 0);

	dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
	gtk_object_set_data (GTK_OBJECT (dialog1), "dialog_action_area1", dialog_action_area1);
	gtk_widget_show (dialog_action_area1);
	gtk_container_border_width (GTK_CONTAINER (dialog_action_area1), 10);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_object_set_data (GTK_OBJECT (dialog1), "hbuttonbox1", hbuttonbox1);
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), hbuttonbox1, TRUE, TRUE, 0);

	button2 = gtk_button_new_with_label ("Yes");
	gtk_object_set_data (GTK_OBJECT (dialog1), "button2", button2);
	gtk_widget_show (button2);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), button2);
	gtk_signal_connect (GTK_OBJECT (button2), "clicked",
						GTK_SIGNAL_FUNC (cb_button_yes_clicked), NULL);

	button3 = gtk_button_new_with_label ("No");
	gtk_object_set_data (GTK_OBJECT (dialog1), "button3", button3);
	gtk_widget_show (button3);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), button3);
	gtk_signal_connect (GTK_OBJECT (button3), "clicked",
						GTK_SIGNAL_FUNC (cb_button_no_clicked), NULL);

	gtk_widget_show (dialog1);
  
	gtk_main();

	return (g_iWhichYesNoButton == BUTTON_YES);
}

int ASK_gtk_init(int argc, char**argv)
{
	GtkWidget *window1;

	gtk_set_locale ();
	gtk_init (&argc, &argv);

	window1 = create_window1 ();
	gtk_widget_show (window1);

	return 0;
}


