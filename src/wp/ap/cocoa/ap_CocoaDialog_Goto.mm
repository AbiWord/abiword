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
#include "xap_CocoaDialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_CocoaDialog_Goto.h"

#include "fv_View.h"

/*****************************************************************/
XAP_Dialog * AP_CocoaDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id id)
{
	AP_CocoaDialog_Goto * p = new AP_CocoaDialog_Goto(pFactory, id);
	return p;
}

AP_CocoaDialog_Goto::AP_CocoaDialog_Goto(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Goto(pDlgFactory,id)
{
	m_wMainWindow = NULL;
	m_pBookmarks = NULL;
}

AP_CocoaDialog_Goto::~AP_CocoaDialog_Goto(void)
{
	if(m_pBookmarks)
	{
		delete [] m_pBookmarks;
		m_pBookmarks = 0;
	}
		
}

void AP_CocoaDialog_Goto::s_blist_clicked(GtkWidget *clist, gint row, gint column,
										  GdkEventButton *event, AP_CocoaDialog_Goto *me)
{
	gtk_entry_set_text(GTK_ENTRY(me->m_wEntry), (gchar*)me->m_pBookmarks[row]);
}

char *AP_CocoaDialog_Goto::s_convert(const char * st)
{
	UT_ASSERT(st);
	char *res = g_strdup (st);
	char *tmp = res;

	while (*tmp)
	{
		if (*tmp == '&')
			*tmp = '_';
		tmp++;
	}

	return res;
}

void AP_CocoaDialog_Goto::s_goto (const char *number, AP_CocoaDialog_Goto * me)
{
	UT_UCSChar *ucsnumber = (UT_UCSChar *) malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS4_strcpy_char (ucsnumber, number);
	int target = me->getSelectedRow ();
	me->getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);
	free (ucsnumber);
}

void AP_CocoaDialog_Goto::s_gotoClicked (GtkWidget * widget, AP_CocoaDialog_Goto * me)
{
	char *number = gtk_entry_get_text (GTK_ENTRY (me->m_wEntry));
	if (number && *number)
			s_goto ((const char *) number, me);
}

void AP_CocoaDialog_Goto::s_nextClicked (GtkWidget * widget, AP_CocoaDialog_Goto * me)
{
	s_goto ("+1", me);
}

void AP_CocoaDialog_Goto::s_prevClicked (GtkWidget * widget, AP_CocoaDialog_Goto * me)
{
	s_goto ("-1", me);
}

void AP_CocoaDialog_Goto::s_closeClicked (GtkWidget * widget, AP_CocoaDialog_Goto * me)
{
	me->destroy();
}

void AP_CocoaDialog_Goto::s_deleteClicked (GtkWidget * widget, gpointer /* data */,AP_CocoaDialog_Goto * me)
{
	me->destroy();
}



void AP_CocoaDialog_Goto::s_targetChanged (GtkWidget *clist, gint row, gint column,
										  GdkEventButton *event, AP_CocoaDialog_Goto *me)
{
	me->setSelectedRow (row);
}

void AP_CocoaDialog_Goto::s_dataChanged (GtkWidget *widget, AP_CocoaDialog_Goto * me)
{
	gchar *text = gtk_entry_get_text (GTK_ENTRY (widget));

	if (text[0] == '\0')
	{
		gtk_widget_grab_default (me->m_wClose);
		gtk_widget_set_sensitive (me->m_wGoto, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (me->m_wGoto, TRUE);
		gtk_widget_grab_default (me->m_wGoto);
	}
}

void AP_CocoaDialog_Goto::setSelectedRow (int row)
{
	m_iRow = row;
	if(row == (int) AP_JUMPTARGET_BOOKMARK)
	{
		gtk_widget_hide(m_dlabel);
		gtk_widget_show(m_swindow);
	}
	else
	{
		gtk_widget_hide(m_swindow);
		gtk_widget_show(m_dlabel);
	}
}

int AP_CocoaDialog_Goto::getSelectedRow (void)
{
	return (m_iRow);
}

void AP_CocoaDialog_Goto::runModeless (XAP_Frame * pFrame)
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

void AP_CocoaDialog_Goto::destroy (void)
{
	UT_ASSERT (m_wMainWindow);
	modeless_cleanup();
	if(m_wMainWindow && GTK_IS_WIDGET(m_wMainWindow))
	  gtk_widget_destroy(m_wMainWindow);
	m_wMainWindow = NULL;
}

void AP_CocoaDialog_Goto::activate (void)
{
	UT_ASSERT (m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), m_WindowName);
	gdk_window_raise (m_wMainWindow->window);
}


void AP_CocoaDialog_Goto::notifyActiveFrame(XAP_Frame *pFrame)
{
        UT_ASSERT(m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), m_WindowName);
}

GtkWidget * AP_CocoaDialog_Goto::_constructWindow (void)
{
	GtkWidget *vbox;
	GtkWidget *actionarea;
	GtkWidget *contents;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_wMainWindow = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (m_wMainWindow), 4);
        ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow),m_WindowName);
	gtk_window_set_policy(GTK_WINDOW(m_wMainWindow), FALSE, FALSE, TRUE);
	vbox = GTK_DIALOG (m_wMainWindow)->vbox;
	actionarea = GTK_DIALOG (m_wMainWindow)->action_area;

	contents = _constructWindowContents ();

	// TODO: This call must be in _constructWindowContents
	gtk_window_add_accel_group (GTK_WINDOW (m_wMainWindow), m_accelGroup);

	gtk_box_pack_start (GTK_BOX (vbox), contents, TRUE, TRUE, 0);
	
	// Buttons
	m_wPrev = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Prev));
	gtk_container_add (GTK_CONTAINER (actionarea), m_wPrev);
 	GTK_WIDGET_SET_FLAGS (m_wPrev, GTK_CAN_DEFAULT);

	m_wNext = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Next));
	gtk_container_add (GTK_CONTAINER (actionarea), m_wNext);
 	GTK_WIDGET_SET_FLAGS (m_wNext, GTK_CAN_DEFAULT);

	m_wGoto = gtk_button_new_with_label (pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Goto));
	gtk_container_add (GTK_CONTAINER (actionarea), m_wGoto);
 	GTK_WIDGET_SET_FLAGS (m_wGoto, GTK_CAN_DEFAULT);
	gtk_widget_set_sensitive (m_wGoto, FALSE);

	m_wClose = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Close));
	gtk_container_add (GTK_CONTAINER (actionarea), m_wClose);
 	GTK_WIDGET_SET_FLAGS (m_wClose, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (m_wClose);

	gtk_widget_show_all (m_wMainWindow);
	
	//initially hide the bookmark list; also we want the bookmark list to be
	// of same size as the descriptive text, so that when we swap them
	// the whole window does not get resized
	gtk_widget_hide(m_swindow);
	gtk_widget_set_usize(m_swindow,(gint)m_dlabel->allocation.width,(gint)m_dlabel->allocation.height);
	
	_connectSignals ();

	return (m_wMainWindow);
}

GtkWidget *AP_CocoaDialog_Goto::_constructWindowContents (void)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *what_lb;
	GtkWidget *clist;
	GtkWidget *vbox2;
	GtkWidget *number_lb;
	GtkWidget *contents;
	GtkWidget *blist;

	guint number_lb_key;
	guint what_lb_key;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	char *tmp;

	m_accelGroup = gtk_accel_group_new ();

	contents = gtk_vbox_new (FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_box_pack_start (GTK_BOX (contents), hbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	what_lb = gtk_label_new ("");
	tmp = s_convert((char*)pSS->getValue(AP_STRING_ID_DLG_Goto_Label_What));
	what_lb_key = gtk_label_parse_uline (GTK_LABEL (what_lb), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), what_lb, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (what_lb), 0, 0.5);

	clist = gtk_clist_new (1);
	gtk_box_pack_start (GTK_BOX (vbox), clist, TRUE, TRUE, 0);
	gtk_clist_set_column_width (GTK_CLIST (clist), 0, 80);
	gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_BROWSE);
	gtk_clist_column_titles_hide (GTK_CLIST (clist));
	m_iRow = 0;
	char **tmp2 = getJumpTargets ();
	for (int i = 0; tmp2[i] != NULL; i++)
		gtk_clist_append (GTK_CLIST (clist), &tmp2[i]);
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	number_lb = gtk_label_new ("");
	tmp = s_convert ((char*)pSS->getValue (AP_STRING_ID_DLG_Goto_Label_Number));
	number_lb_key = gtk_label_parse_uline (GTK_LABEL (number_lb), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox2), number_lb, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (number_lb), 7.45058e-09, 0.5);

	m_wEntry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), m_wEntry, FALSE, FALSE, 0);

	m_dlabel = gtk_label_new (pSS->getValue (AP_STRING_ID_DLG_Goto_Label_Help));
	gtk_box_pack_start (GTK_BOX (vbox2), m_dlabel, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (m_dlabel), GTK_JUSTIFY_FILL);
	gtk_label_set_line_wrap (GTK_LABEL (m_dlabel), TRUE);
	gtk_misc_set_alignment (GTK_MISC (m_dlabel), 0, 0.5);
	
	// the bookmark list
	m_swindow  = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_swindow),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_hide(m_swindow);
    gtk_box_pack_start (GTK_BOX (vbox2), m_swindow, FALSE, FALSE, 0);
	
	blist = gtk_clist_new (1);
	gtk_clist_set_selection_mode(GTK_CLIST(blist), GTK_SELECTION_BROWSE);
    gtk_clist_column_titles_hide(GTK_CLIST(blist));
    //gtk_box_pack_start (GTK_BOX (vbox2), m_blist, FALSE, FALSE, 0);

    if(m_pBookmarks)
    	delete [] m_pBookmarks;
	m_pBookmarks = new const XML_Char *[getExistingBookmarksCount()];
	
    for (int i = 0; i < (int)getExistingBookmarksCount(); i++)
    	m_pBookmarks[i] = getNthExistingBookmark(i);

    int (*my_cmp)(const void *, const void *) =
    	(int (*)(const void*, const void*)) UT_XML_strcmp;
    	
    qsort(m_pBookmarks, getExistingBookmarksCount(),sizeof(XML_Char*),my_cmp);

    for (int i = 0; i < (int)getExistingBookmarksCount(); i++)
    	gtk_clist_append (GTK_CLIST (blist), (gchar**) &m_pBookmarks[i]);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_swindow),blist);
	

    //add signal handlers
	g_signal_connect (G_OBJECT (clist), "select_row",
						G_CALLBACK (s_targetChanged),
						this);
	g_signal_connect (G_OBJECT (m_wEntry), "changed",
						G_CALLBACK (s_dataChanged), this);
	g_signal_connect (G_OBJECT (m_wEntry), "activate",
						G_CALLBACK (s_gotoClicked), this);
						
	g_signal_connect (G_OBJECT (blist), "select_row",
						G_CALLBACK (s_blist_clicked), this);

	gtk_widget_add_accelerator (clist, "grab_focus", m_accelGroup,
								what_lb_key, GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_widget_add_accelerator (m_wEntry, "grab_focus", m_accelGroup,
								number_lb_key, GDK_MOD1_MASK, (GtkAccelFlags) 0);

	return contents;
}

void AP_CocoaDialog_Goto::_populateWindowData (void) {}

void AP_CocoaDialog_Goto::_connectSignals(void)
{
	g_signal_connect_after(G_OBJECT(m_wMainWindow),
							 "destroy",
							 NULL,
							 NULL);
	//
        // Don't use connect_after in modeless dialog
	g_signal_connect(G_OBJECT(m_wMainWindow),
						     "delete_event",
						     G_CALLBACK(s_deleteClicked), (gpointer) this);
	g_signal_connect (G_OBJECT (m_wPrev), "clicked",
						G_CALLBACK (s_prevClicked), this);
	g_signal_connect (G_OBJECT (m_wNext), "clicked",
						G_CALLBACK (s_nextClicked), this);
	g_signal_connect (G_OBJECT (m_wGoto), "clicked",
						G_CALLBACK (s_gotoClicked), this);
	g_signal_connect (G_OBJECT (m_wClose), "clicked",
						G_CALLBACK (s_closeClicked), this);
}
