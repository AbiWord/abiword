/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include <stdio.h>
#include <string.h>
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "xap_QNXDlg_Encoding.h"
//#include "ut_dialogHelper.h"


XAP_Dialog * XAP_QNXDialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_QNXDialog_Encoding * p = new XAP_QNXDialog_Encoding(pFactory,id);
	return p;
}

XAP_QNXDialog_Encoding::XAP_QNXDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Encoding(pDlgFactory,id)
{

}

XAP_QNXDialog_Encoding::~XAP_QNXDialog_Encoding(void)
{
}



/*****************************************************************/

// These are all static callbacks, bound to GTK or GDK events.

#if 0
static void s_ok_clicked(PtWidget_t * widget,
						 XAP_QNXDialog_Encoding * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_OK();
}

static void s_cancel_clicked(PtWidget_t * widget,
							 XAP_QNXDialog_Encoding * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_Cancel();
}

static void s_clist_event(PtWidget_t * widget,
						  GdkEventButton * event,
						  XAP_QNXDialog_Encoding * dlg)
{
	UT_ASSERT(widget && event && dlg);

	// Only respond to double clicks
	if (event->type == GDK_2BUTTON_PRESS)
	{
		dlg->event_DoubleClick();
	}
}

static void s_delete_clicked(PtWidget_t * /* widget */,
							 gpointer /* data */,
							 XAP_QNXDialog_Encoding * dlg)
{
	UT_ASSERT(dlg);

	dlg->event_WindowDelete();
}
#endif

/*****************************************************************/

void XAP_QNXDialog_Encoding::runModal(XAP_Frame * pFrame)
{
#if 0
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow,pFrame);
	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
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
#else
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
#endif
}

void XAP_QNXDialog_Encoding::event_OK(void)
{
#if 0
	// Query the list for its selection.
	gint row = _getFromList();

	if (row >= 0)
		_setSelectionIndex((UT_uint32) row);
	
	_setAnswer (XAP_Dialog_Encoding::a_OK);
	gtk_main_quit();
#endif
}

void XAP_QNXDialog_Encoding::event_Cancel(void)
{
#if 0
	_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
	gtk_main_quit();
#endif
}

void XAP_QNXDialog_Encoding::event_DoubleClick(void)
{
#if 0
	// Query the list for its selection.	
	gint row = _getFromList();

	// If it found something, return with it
	if (row >= 0)
	{
		_setSelectionIndex ((UT_uint32) row);
		event_OK();
	}
#endif
}

void XAP_QNXDialog_Encoding::event_WindowDelete(void)
{
#if 0
	_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
	gtk_main_quit();
#endif
}

/*****************************************************************/

int XAP_QNXDialog_Encoding::_getFromList(void)
{
	return -1;
}

PtWidget_t * XAP_QNXDialog_Encoding::_constructWindow(void)
{
#if 0
	// This is the top level GTK widget, the window.
	// It's created with a "dialog" style.
	PtWidget_t *windowMain;

	// This is the top level organization widget, which packs
	// things vertically
	PtWidget_t *vboxMain;

	// The top item in the vbox is a simple label
	PtWidget_t *labelActivate;

	// The second item in the vbox is a scrollable area
	PtWidget_t *scrollWindows;
	
	// The child of the scrollable area is our list of windows
	PtWidget_t *clistWindows;
	
	// These are the buttons.
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

#ifndef HAVE_GNOME
	// The third (and bottom) item in the vbox is a horizontal
	// button box, which holds our two action buttons.
	PtWidget_t *buttonboxAction;

	// Create the new top level window.
	windowMain = gtk_window_new (GTK_WINDOW_DIALOG);
gtk_window_set_title (GTK_WINDOW (windowMain), pSS->getValueUTF8(XAP_STRING_ID_DLG_UENC_EncTitle ).utf8_str();
	// This policy allows the window to let the window manager shrink and grow it.
	gtk_window_set_policy (GTK_WINDOW (windowMain), TRUE, TRUE, FALSE);

	vboxMain = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxMain);
	gtk_container_add (GTK_CONTAINER (windowMain), vboxMain);

#else
windowMain = gnome_dialog_new (pSS->getValueUTF8(XAP_STRING_ID_DLG_UENC_EncTitle).utf8_str(), GNOME_STOCK_BUTTON_OK,
				       GNOME_STOCK_BUTTON_CANCEL, NULL);
	vboxMain = GNOME_DIALOG(windowMain)->vbox;
	buttonOK = GTK_WIDGET (g_list_first (GNOME_DIALOG (windowMain)->buttons)->data);
	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowMain)->buttons)->data);
#endif

labelActivate = gtk_label_new (pSS->getValueUTF8(XAP_STRING_ID_DLG_UENC_EncLabel ).utf8_str();
	gtk_widget_show (labelActivate);
	gtk_box_pack_start (GTK_BOX (vboxMain), labelActivate, FALSE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (labelActivate), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (labelActivate), 0, 0);
	gtk_misc_set_padding (GTK_MISC (labelActivate), 10, 5);

	scrollWindows = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollWindows),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
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

buttonOK = gtk_button_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_OK ).utf8_str();
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (buttonboxAction), buttonOK);

buttonCancel = gtk_button_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel ).utf8_str();
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
	
	g_signal_connect(G_OBJECT(buttonOK),
					   "clicked",
					   G_CALLBACK(s_ok_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(buttonCancel),
					   "clicked",
					   G_CALLBACK(s_cancel_clicked),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(clistWindows),
					   "button_press_event",
					   G_CALLBACK(s_clist_event),
					   (gpointer) this);

	// Since each modal dialog is raised through gtk_main(),
	// we need to bind a callback to the top level window's
	// "delete" event to make sure we actually exit
	// with gtk_main_quit(), for the case that the user used
	// a window manager to close us.

	g_signal_connect(G_OBJECT(windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);

	g_signal_connect_after(G_OBJECT(windowMain),
							 "destroy",
							 NULL,
							 NULL);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowMain;
	m_clistWindows = clistWindows;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

#ifdef HAVE_GNOME
	setDefaultButton (GNOME_DIALOG(windowMain), 0); // ok button
#endif

	return windowMain;
#else
	return NULL;
#endif
}

void XAP_QNXDialog_Encoding::_populateWindowData(void)
{
#if 0
	// We just do one thing here, which is fill the list with
	// all the windows.

	for (UT_uint32 i = 0; i < _getEncodingsCount(); i++)
	{
		const XML_Char* s = _getAllEncodings()[i];

		gint row = gtk_clist_append(GTK_CLIST(m_clistWindows), (gchar **) &s);
		gtk_clist_set_row_data(GTK_CLIST(m_clistWindows), row, GINT_TO_POINTER(i));
	} 

	// Select the one we're in
	gtk_clist_select_row(GTK_CLIST(m_clistWindows), _getSelectionIndex(), 0);
#endif
}



