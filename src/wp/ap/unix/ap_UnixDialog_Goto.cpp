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

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_UnixDialog_Goto.h"

#include "fv_View.h"

/*****************************************************************/
XAP_Dialog * AP_UnixDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id id)
{
	AP_UnixDialog_Goto * p = new AP_UnixDialog_Goto(pFactory, id);
	return p;
}

AP_UnixDialog_Goto::AP_UnixDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Goto(pDlgFactory,id)
{
	m_wMainWindow = NULL;
}

AP_UnixDialog_Goto::~AP_UnixDialog_Goto(void)
{
}

static void s_goto (const char *number, AP_UnixDialog_Goto * me)
{
	UT_UCSChar *ucsnumber = (UT_UCSChar *) malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS_strcpy_char (ucsnumber, number);
	int target = me->getSelectedRow ();
	me->getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);
	free (ucsnumber);
}

static void s_gotoClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	char *number = gtk_entry_get_text (GTK_ENTRY (me->m_wEntry));
	s_goto ((const char *) number, me);
}

static void s_nextClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	s_goto ("+1", me);
}

static void s_prevClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	s_goto ("-1", me);
}

static void s_closeClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	me->destroy();
}

static void s_deleteClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	me->destroy();
}

void AP_UnixDialog_Goto::s_targetChanged (GtkWidget *clist, gint row, gint column,
										  GdkEventButton *event, AP_UnixDialog_Goto *me)
{
	me->setSelectedRow (row);
}

static void s_dataChanged (GtkWidget *widget, AP_UnixDialog_Goto * me)
{
	gchar *text = gtk_entry_get_text (GTK_ENTRY (widget));

	if (text[0] == '\0')
	{
		gtk_widget_set_sensitive (me->m_wGoto, FALSE);
		// TODO
	}
	else
	{
		// TODO
		gtk_widget_set_sensitive (me->m_wGoto, TRUE);
	}
}

void AP_UnixDialog_Goto::setSelectedRow (int row)
{
	m_iRow = row;
}

int AP_UnixDialog_Goto::getSelectedRow (void)
{
	return (m_iRow);
}

void AP_UnixDialog_Goto::runModeless (XAP_Frame * pFrame)
{
	_constructWindow ();
	UT_ASSERT (m_wMainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);

	// This magic command displays the frame that characters will be
	// inserted into.
	connectFocusModeless (GTK_WIDGET (m_wMainWindow), m_pApp);
}

void AP_UnixDialog_Goto::destroy (void)
{
	UT_ASSERT (m_wMainWindow);
	modeless_cleanup();
	gtk_widget_destroy(m_wMainWindow);
	m_wMainWindow = NULL;
}

void AP_UnixDialog_Goto::activate (void)
{
	UT_ASSERT (m_wMainWindow);
	gdk_window_raise (m_wMainWindow->window);
}

GtkWidget * AP_UnixDialog_Goto::_constructWindow (void)
{
	GtkWidget *vbox1;
	GtkWidget *hseparator1;
	GtkWidget *hbuttonbox1;
	GtkWidget *button4;
	GtkWidget *button5;
	GtkWidget *close_bt;
	GtkWidget *contents;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_wMainWindow = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_container_set_border_width (GTK_CONTAINER (m_wMainWindow), 4);
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), pSS->getValue (AP_STRING_ID_DLG_Goto_Title));
	gtk_signal_connect_after(GTK_OBJECT(m_wMainWindow),
							 "destroy",
							 GTK_SIGNAL_FUNC(s_deleteClicked),
							 this);
	gtk_signal_connect_after(GTK_OBJECT(m_wMainWindow),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_deleteClicked),
							 this);
	gtk_window_set_policy(GTK_WINDOW(m_wMainWindow), FALSE, FALSE, TRUE);

	vbox1 = gtk_vbox_new (FALSE, 4);
	gtk_container_add (GTK_CONTAINER (m_wMainWindow), vbox1);

	contents = _constructWindowContents ();
	gtk_box_pack_start (GTK_BOX (vbox1), contents, TRUE, TRUE, 0);
	
	// container for buttons
	hseparator1 = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, TRUE, TRUE, 0);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, TRUE, TRUE, 0);

	button4 = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Prev));
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);
	gtk_signal_connect (GTK_OBJECT (button4), "clicked",
						GTK_SIGNAL_FUNC (s_prevClicked), this);
	GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

	button5 = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Next));
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
	gtk_signal_connect (GTK_OBJECT (button5), "clicked",
						GTK_SIGNAL_FUNC (s_nextClicked), this);
	GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

	m_wGoto = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Goto));
	gtk_signal_connect_after(GTK_OBJECT(m_wGoto), "clicked",
							 GTK_SIGNAL_FUNC(s_gotoClicked), this);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), m_wGoto);
	GTK_WIDGET_SET_FLAGS (m_wGoto, GTK_CAN_DEFAULT);
	gtk_widget_set_sensitive (m_wGoto, FALSE);

	close_bt = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Close));
	gtk_signal_connect_after(GTK_OBJECT(close_bt), "clicked",
							 GTK_SIGNAL_FUNC(s_closeClicked), this);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), close_bt);
	GTK_WIDGET_SET_FLAGS (close_bt, (GTK_CAN_DEFAULT | GTK_HAS_DEFAULT));

	gtk_widget_show_all (m_wMainWindow);

	return (m_wMainWindow);
}

GtkWidget *AP_UnixDialog_Goto::_constructWindowContents (void)
{
	GtkWidget *table;
	GtkWidget *clist;
	GtkWidget *scrolledwindow1;
	GtkWidget *label;
	GtkWidget *label2;
	GtkWidget *label3;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	table = gtk_table_new (3, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_widget_show (table);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_table_attach (GTK_TABLE (table), scrolledwindow1, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	
	clist = gtk_clist_new (1);
	gtk_widget_show (clist);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), clist);
	gtk_clist_set_column_width (GTK_CLIST (clist), 0, 80);
	gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_BROWSE);
	gtk_signal_connect (GTK_OBJECT (clist), "select_row",
						GTK_SIGNAL_FUNC (s_targetChanged),
						this);
	m_iRow = 0;
	char **tmp = getJumpTargets ();
	for (int i = 0; tmp[i] != NULL; i++)
		gtk_clist_append(GTK_CLIST (clist), &tmp[i]);

	gtk_clist_column_titles_hide (GTK_CLIST (clist));
	
	label = gtk_label_new (pSS->getValue (AP_STRING_ID_DLG_Goto_Label_What));
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	m_wEntry = gtk_entry_new ();
	gtk_widget_show (m_wEntry);
	gtk_table_attach (GTK_TABLE (table), m_wEntry, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_signal_connect (GTK_OBJECT (m_wEntry), "changed",
						GTK_SIGNAL_FUNC (s_dataChanged), this);

	label2 = gtk_label_new (pSS->getValue (AP_STRING_ID_DLG_Goto_Label_Number));
	gtk_widget_show (label2);
	gtk_table_attach (GTK_TABLE (table), label2, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);
	
	label3 = gtk_label_new (pSS->getValue (AP_STRING_ID_DLG_Goto_Label_Help));
	gtk_widget_show (label3);
	gtk_table_attach (GTK_TABLE (table), label3, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
	
	return (table);
}

void AP_UnixDialog_Goto::_populateWindowData (void) {}

