/* AbiWord
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_UnixDialog_Insert_DateTime.h"

/*****************************************************************/

#define	LIST_ITEM_INDEX_KEY "index"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
															   XAP_Dialog_Id id)
{
	AP_UnixDialog_Insert_DateTime * p = new AP_UnixDialog_Insert_DateTime(pFactory,id);
	return p;
}

AP_UnixDialog_Insert_DateTime::AP_UnixDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id id)
	: AP_Dialog_Insert_DateTime(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_listFormats = NULL;
}

AP_UnixDialog_Insert_DateTime::~AP_UnixDialog_Insert_DateTime(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_Insert_DateTime * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Insert_DateTime * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_Insert_DateTime * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

/*****************************************************************/

void AP_UnixDialog_Insert_DateTime::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_Insert_DateTime::event_OK(void)
{
	UT_ASSERT(m_windowMain && m_listFormats);
	
	// find item selected in list box, save it to m_iFormatIndex

	GList * listitem = GTK_LIST(m_listFormats)->selection;

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if (!(listitem) || !(listitem)->data)
	{
		m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
		gtk_main_quit();
		return;
	}

	// since we only do single mode selection, there is only one
	// item in the GList we just got back

	gint indexdata = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(listitem->data), LIST_ITEM_INDEX_KEY));

	m_iFormatIndex = indexdata;
	
	m_answer = AP_Dialog_Insert_DateTime::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_Insert_DateTime::event_Cancel(void)
{
	m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Insert_DateTime::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;	
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Insert_DateTime::_constructWindow(void)
{
	GtkWidget *windowMain;
	GtkWidget *hboxMain;
	GtkWidget *vboxFormats;
	GtkWidget *labelFormats;
	GtkWidget *scrolledwindowFormats;
	GtkWidget *viewportFormats;
	GtkWidget *listFormats;
	GtkWidget *vbuttonboxButtons;
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * unixstr = NULL;	// used for conversions

	windowMain = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowMain), "windowMain", windowMain);
	gtk_widget_set_usize (windowMain, 270, 240);
	gtk_container_set_border_width (GTK_CONTAINER (windowMain), 10);
	gtk_window_set_title (GTK_WINDOW (windowMain), pSS->getValue(AP_STRING_ID_DLG_DateTime_DateTimeTitle));
	gtk_window_set_policy (GTK_WINDOW (windowMain), FALSE, FALSE, FALSE);

	hboxMain = gtk_hbox_new (FALSE, 5);
	gtk_widget_ref (hboxMain);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "hboxMain", hboxMain,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hboxMain);
	gtk_container_add (GTK_CONTAINER (windowMain), hboxMain);

	vboxFormats = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (vboxFormats);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "vboxFormats", vboxFormats,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vboxFormats);
	gtk_box_pack_start (GTK_BOX (hboxMain), vboxFormats, TRUE, TRUE, 0);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_DateTime_AvailableFormats));
	labelFormats = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_widget_ref (labelFormats);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "labelFormats", labelFormats,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelFormats);
	gtk_box_pack_start (GTK_BOX (vboxFormats), labelFormats, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (labelFormats), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelFormats), 0, 0.5);

	scrolledwindowFormats = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_ref (scrolledwindowFormats);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "scrolledwindowFormats", scrolledwindowFormats,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (scrolledwindowFormats);
	gtk_box_pack_start (GTK_BOX (vboxFormats), scrolledwindowFormats, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowFormats), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	viewportFormats = gtk_viewport_new (NULL, NULL);
	gtk_widget_ref (viewportFormats);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "viewportFormats", viewportFormats,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (viewportFormats);
	gtk_container_add (GTK_CONTAINER (scrolledwindowFormats), viewportFormats);

	listFormats = gtk_list_new ();
	gtk_widget_ref (listFormats);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "listFormats", listFormats,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_list_set_selection_mode (GTK_LIST(listFormats), GTK_SELECTION_SINGLE);
	gtk_widget_show (listFormats);
	gtk_container_add (GTK_CONTAINER (viewportFormats), listFormats);

	vbuttonboxButtons = gtk_vbutton_box_new ();
	gtk_widget_ref (vbuttonboxButtons);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "vbuttonboxButtons", vbuttonboxButtons,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbuttonboxButtons);
	gtk_box_pack_start (GTK_BOX (hboxMain), vbuttonboxButtons, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonboxButtons), GTK_BUTTONBOX_START);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_ref (buttonOK);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "buttonOK", buttonOK,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (vbuttonboxButtons), buttonOK);
	gtk_widget_set_usize (buttonOK, -2, 33);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_ref (buttonCancel);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "buttonCancel", buttonCancel,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (vbuttonboxButtons), buttonCancel);
	gtk_widget_set_usize (buttonCancel, -2, 33);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	// the control buttons
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	// the catch-alls
	
	gtk_signal_connect_after(GTK_OBJECT(windowMain),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowMain),
							 "destroy",
							 NULL,
							 NULL);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowMain;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_listFormats = listFormats;

	return windowMain;
}

void AP_UnixDialog_Insert_DateTime::_populateWindowData(void)
{
	UT_ASSERT(m_windowMain && m_listFormats);
	
	// NOTE : this code is similar to the Windows dialog code to do
	// NOTE : the same thing.  if you are implementing this dialog
	// NOTE : for a new front end, this is the formatting logic 
	// NOTE : you'll want to use to populate your list
	
	int i;

	// this constant comes from ap_Dialog_Insert_DateTime.h
    char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];
	
    time_t tim = time(NULL);
	
    struct tm *pTime = localtime(&tim);

	GList * list = NULL;
	GtkWidget * listitem;
	
	// build GList of gtk_list_items
    for (i = 0; InsertDateTimeFmts[i] != NULL; i++)
	{
        strftime(szCurrentDateTime, CURRENT_DATE_TIME_SIZE, InsertDateTimeFmts[i], pTime);

		// allocate a list item with that string
		listitem = gtk_list_item_new_with_label(szCurrentDateTime);
		UT_ASSERT(listitem);

		gtk_widget_show(listitem);
		
		// store index in data pointer
		gtk_object_set_data(GTK_OBJECT(listitem), LIST_ITEM_INDEX_KEY, GINT_TO_POINTER(i));

		// add to list
	    list = g_list_append(list, (void *) listitem);
	}

	// TODO : Does the gtk_list free this list for me?  I think it does.
	
	// add GList items to list box
	gtk_list_append_items(GTK_LIST(m_listFormats), list);

	// now select first item in box
	if (i > 0)
		gtk_list_select_item(GTK_LIST(m_listFormats), 0);

}

