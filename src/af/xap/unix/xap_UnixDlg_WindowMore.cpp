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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_UnixDlg_WindowMore.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_UnixDialog_WindowMore * p = new XAP_UnixDialog_WindowMore(pFactory,id);
	return p;
}

XAP_UnixDialog_WindowMore::XAP_UnixDialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_UnixDialog_WindowMore::~XAP_UnixDialog_WindowMore(void)
{
}

/*****************************************************************/

// These are all static callbacks, bound to GTK or GDK events.

static void s_ok_clicked(GtkWidget * widget,
						 XAP_UnixDialog_WindowMore * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget,
							 XAP_UnixDialog_WindowMore * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_Cancel();
}

static void s_clist_event(GtkWidget * widget,
						  GdkEventButton * event,
						  XAP_UnixDialog_WindowMore * dlg)
{
	UT_ASSERT(widget && event && dlg);

	// Only respond to double clicks
	if (event->type == GDK_2BUTTON_PRESS)
	{
		dlg->event_DoubleClick();
	}
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_UnixDialog_WindowMore * dlg)
{
	UT_ASSERT(dlg);

	dlg->event_WindowDelete();
}

/*****************************************************************/

void XAP_UnixDialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	// Initialize member so we know where we are now
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
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

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	if(mainWindow && GTK_IS_WIDGET(mainWindow))
	  gtk_widget_destroy(mainWindow);
}

void XAP_UnixDialog_WindowMore::event_OK(void)
{
	// Query the list for its selection.
	gint row = _GetFromList();

	if (row >= 0)
		m_ndxSelFrame = (UT_uint32) row;
	
	m_answer = XAP_Dialog_WindowMore::a_OK;
	gtk_main_quit();
}

void XAP_UnixDialog_WindowMore::event_Cancel(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;
	gtk_main_quit();
}

void XAP_UnixDialog_WindowMore::event_DoubleClick(void)
{
	// Query the list for its selection.	
	gint row = _GetFromList();

	// If it found something, return with it
	if (row >= 0)
	{
		m_ndxSelFrame = (UT_uint32) row;
		event_OK();
	}
}

void XAP_UnixDialog_WindowMore::event_WindowDelete(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;	
	gtk_main_quit();
}

/*****************************************************************/

gint XAP_UnixDialog_WindowMore::_GetFromList(void)
{
	// Grab the selected index and store it in the member data
	GList * selectedRow = GTK_CLIST(m_clistWindows)->selection;

	if (selectedRow)
	{
		gint rowNumber = GPOINTER_TO_INT(selectedRow->data);
		if (rowNumber >= 0)
		{
			// Store the value
			return rowNumber;
		}
		else
		{
			// We have a selection but no rows in it...
			// funny.
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return -1;
		}
	}

	// No selected rows
	return -1;
}

GtkWidget * XAP_UnixDialog_WindowMore::_constructWindow(void)
{
	// This is the top level GTK widget, the window.
	// It's created with a "dialog" style.
	GtkWidget *windowMain;

	// This is the top level organization widget, which packs
	// things vertically
	GtkWidget *vboxMain;

	// The top item in the vbox is a simple label
	GtkWidget *labelActivate;

	// The second item in the vbox is a scrollable area
	GtkWidget *scrollWindows;
	
	// The child of the scrollable area is our list of windows
	GtkWidget *clistWindows;
	
	// These are the buttons.
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

#ifndef HAVE_GNOME
	// The third (and bottom) item in the vbox is a horizontal
	// button box, which holds our two action buttons.
	GtkWidget *buttonboxAction;

	// Create the new top level window.
	windowMain = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW (windowMain), pSS->getValue(XAP_STRING_ID_DLG_MW_MoreWindows));
	// This policy allows the window to let the window manager shrink and grow it.
	gtk_window_set_policy (GTK_WINDOW (windowMain), TRUE, TRUE, FALSE);

	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowMain), vboxMain);

#else
	windowMain = gnome_dialog_new (pSS->getValue(XAP_STRING_ID_DLG_MW_MoreWindows), GNOME_STOCK_BUTTON_OK,
				       GNOME_STOCK_BUTTON_CANCEL, NULL);
	vboxMain = GNOME_DIALOG(windowMain)->vbox;
	buttonOK = GTK_WIDGET (g_list_first (GNOME_DIALOG (windowMain)->buttons)->data);
	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowMain)->buttons)->data);
#endif

	labelActivate = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_MW_Activate));
	gtk_widget_show (labelActivate);
	gtk_box_pack_start (GTK_BOX (vboxMain), labelActivate, FALSE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (labelActivate), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelActivate), 0, 0);
	gtk_misc_set_padding (GTK_MISC (labelActivate), 10, 5);

	scrollWindows = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollWindows);
	gtk_widget_set_usize(scrollWindows, 350, 210);
	gtk_container_set_border_width(GTK_CONTAINER(scrollWindows), 10);
	gtk_box_pack_start(GTK_BOX(vboxMain), scrollWindows, TRUE, TRUE, 0);

	clistWindows = gtk_clist_new (1);
	gtk_widget_show (clistWindows);
//	gtk_box_pack_start (GTK_BOX (vboxMain), clistWindows, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER(scrollWindows), clistWindows);
//	gtk_widget_set_usize (clistWindows, 350, 210);
//	gtk_container_set_border_width (GTK_CONTAINER (clistWindows), 10);
	gtk_clist_set_column_width (GTK_CLIST (clistWindows), 0, 80);
	gtk_clist_column_titles_hide (GTK_CLIST (clistWindows));

#ifndef HAVE_GNOME
	buttonboxAction = gtk_hbutton_box_new ();
	gtk_widget_show (buttonboxAction);
	gtk_box_pack_start (GTK_BOX (vboxMain), buttonboxAction, FALSE, TRUE, 0);
	gtk_container_border_width (GTK_CONTAINER (buttonboxAction), 11);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonboxAction), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (buttonboxAction), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (buttonboxAction), 81, 27);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (buttonboxAction), 0, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (buttonboxAction), buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (buttonboxAction), buttonCancel);
#endif

	/*
	  After we construct our widgets, we attach callbacks to static
	  callback functions so we can respond to their events.  In this
	  dialog, we will want to respond to both buttons (OK and Cancel),
	  double-clicks on the clist, which will be treated like a
	  click on the OK button.
	*/

	/*
	  For a callback data item, the way we have the events routed,
	  we pass a pointer to this instance of the class dialog, so that
	  the static callbacks (which have no access to instance data)
	  can just offload the real work through a select few public
	  class methods.
	*/
	
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(clistWindows),
					   "button_press_event",
					   GTK_SIGNAL_FUNC(s_clist_event),
					   (gpointer) this);

	// Since each modal dialog is raised through gtk_main(),
	// we need to bind a callback to the top level window's
	// "delete" event to make sure we actually exit
	// with gtk_main_quit(), for the case that the user used
	// a window manager to close us.

	gtk_signal_connect(GTK_OBJECT(windowMain),
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
	m_clistWindows = clistWindows;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	return windowMain;
}

void XAP_UnixDialog_WindowMore::_populateWindowData(void)
{
	// We just do one thing here, which is fill the list with
	// all the windows.

	for (UT_uint32 i = 0; i < m_pApp->getFrameCount(); i++)
	{
		XAP_Frame * f = m_pApp->getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getTitle(128);	// TODO: chop this down more? 
		
		gint row = gtk_clist_append(GTK_CLIST(m_clistWindows), (gchar **) &s);
		gtk_clist_set_row_data(GTK_CLIST(m_clistWindows), row, GINT_TO_POINTER(i));
	} 

	// Select the one we're in
	gtk_clist_select_row(GTK_CLIST(m_clistWindows), m_ndxSelFrame, 0);
}

