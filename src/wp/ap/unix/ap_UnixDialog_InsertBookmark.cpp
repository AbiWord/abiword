/* AbiWord
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
#include "ap_Dialog_InsertBookmark.h"
#include "ap_UnixDialog_InsertBookmark.h"
#include "ap_UnixDialog_All.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_InsertBookmark * p = new AP_UnixDialog_InsertBookmark(pFactory,id);
	return p;
}

AP_UnixDialog_InsertBookmark::AP_UnixDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_InsertBookmark(pDlgFactory,id)
{
	m_windowMain = 0;
	m_buttonOK = 0;
	m_buttonCancel = 0;
	m_comboEntry = 0;
}

AP_UnixDialog_InsertBookmark::~AP_UnixDialog_InsertBookmark(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_InsertBookmark * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_InsertBookmark * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static void s_delete_clicked(GtkWidget * widget, AP_UnixDialog_InsertBookmark * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Delete();
}

/***********************************************************************/
void AP_UnixDialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);

	// Populate the window's data items
	_setList();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(parentWindow, mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Show the top level dialog,
	gtk_widget_show_all(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	if(mainWindow && GTK_IS_WIDGET(mainWindow))
	  gtk_widget_destroy(mainWindow);

}

void AP_UnixDialog_InsertBookmark::event_OK(void)
{
	UT_ASSERT(m_windowMain);
	// get the bookmark name, if any (return cancel if no name given)	
	const XML_Char *mark = gtk_entry_get_text(GTK_ENTRY(m_comboEntry));
	if(mark && *mark)
	{
		xxx_UT_DEBUGMSG(("InsertBookmark: OK pressed, first char 0x%x\n", (UT_uint32)*mark));
		setAnswer(AP_Dialog_InsertBookmark::a_OK);
		setBookmark(mark);
	}
	else
	{
		setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	}
		
	gtk_main_quit();
}

void AP_UnixDialog_InsertBookmark::event_Cancel(void)
{
	setAnswer(AP_Dialog_InsertBookmark::a_CANCEL);
	gtk_main_quit();
}

void AP_UnixDialog_InsertBookmark::event_Delete(void)
{
	setAnswer(AP_Dialog_InsertBookmark::a_DELETE);
	gtk_main_quit();
}

void AP_UnixDialog_InsertBookmark::_setList(void)
{
	gint i;
	GList *glist=NULL;

	gint (*my_cmp)(const void *, const void *)
		= (gint (*)(const void *, const void *)) UT_XML_strcmp;
	for(i = 0; i < (gint)getExistingBookmarksCount(); i++)
		glist = g_list_insert_sorted(glist, (gchar *) getNthExistingBookmark(i),my_cmp);
	
	if (glist != NULL)
	  {
	    gtk_combo_set_popdown_strings(GTK_COMBO(m_comboBookmark), glist);
	    g_list_free (glist);
	  }
	
	gtk_entry_set_text(GTK_ENTRY(m_comboEntry), getBookmark());	
}

void  AP_UnixDialog_InsertBookmark::_constructWindowContents(GtkWidget * container )
{
  GtkWidget *label1;
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  label1 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Msg));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (container), label1, TRUE, FALSE, 3);

  m_comboBookmark = gtk_combo_new ();
  gtk_widget_show (m_comboBookmark);
  gtk_box_pack_start (GTK_BOX (container), m_comboBookmark, FALSE, FALSE, 3);

  m_comboEntry = GTK_COMBO (m_comboBookmark)->entry;
  gtk_widget_show (m_comboEntry);
  GTK_WIDGET_SET_FLAGS (m_comboEntry, GTK_CAN_DEFAULT);
}

GtkWidget*  AP_UnixDialog_InsertBookmark::_constructWindow(void)
{
  GtkWidget *frame1;
  GtkWidget *vbox2;
  GtkWidget *hseparator1;
  GtkWidget *hbox1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_windowMain = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (m_windowMain), pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Title));

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (m_windowMain), frame1);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 4);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame1), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  _constructWindowContents ( vbox2 );

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox1, TRUE, TRUE, 0);

  m_buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
  gtk_widget_show (m_buttonOK);
  gtk_box_pack_start (GTK_BOX (hbox1), m_buttonOK, FALSE, FALSE, 3);
  gtk_widget_set_usize (m_buttonOK, DEFAULT_BUTTON_WIDTH, 0);

  m_buttonDelete = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Delete));
  gtk_widget_show (m_buttonDelete);
  gtk_box_pack_start (GTK_BOX (hbox1), m_buttonDelete, FALSE, FALSE, 3);
  gtk_widget_set_usize (m_buttonDelete, DEFAULT_BUTTON_WIDTH, 0);

  m_buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
  gtk_widget_show (m_buttonCancel);
  gtk_box_pack_start (GTK_BOX (hbox1), m_buttonCancel, FALSE, FALSE, 3);
  gtk_widget_set_usize (m_buttonCancel, DEFAULT_BUTTON_WIDTH, 0);

  gtk_widget_grab_focus (m_comboEntry);
  gtk_widget_grab_default (m_comboEntry);

  // connect all the signals
  _connectSignals ();

  return m_windowMain;
}

void AP_UnixDialog_InsertBookmark::_connectSignals (void)
{
	// the control buttons
	g_signal_connect(G_OBJECT(m_buttonOK),
					   "clicked",
					   G_CALLBACK(s_ok_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(m_buttonCancel),
					   "clicked",
					   G_CALLBACK(s_cancel_clicked),
					   (gpointer) this);
	g_signal_connect(G_OBJECT(m_buttonDelete),
					   "clicked",
					   G_CALLBACK(s_delete_clicked),
					   (gpointer) this);
	
	g_signal_connect_after(G_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);
}
