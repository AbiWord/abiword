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

// for gtkclist. todo: use treeview
#undef GTK_DISABLE_DEPRECATED

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

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
	m_pBookmarks = NULL;
}

AP_UnixDialog_Goto::~AP_UnixDialog_Goto(void)
{
	if(m_pBookmarks)
	{
		delete [] m_pBookmarks;
		m_pBookmarks = 0;
	}
		
}

void AP_UnixDialog_Goto::s_blist_clicked(GtkWidget *clist, gint row, gint column,
										  GdkEventButton *event, AP_UnixDialog_Goto *me)
{
	gtk_entry_set_text(GTK_ENTRY(me->m_wEntry), (gchar*)me->m_pBookmarks[row]);
}

char *AP_UnixDialog_Goto::s_convert(const char * st)
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

void AP_UnixDialog_Goto::s_goto (const char *number, AP_UnixDialog_Goto * me)
{
	UT_UCSChar *ucsnumber = (UT_UCSChar *) malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS4_strcpy_char (ucsnumber, number);
	int target = me->getSelectedRow ();
	me->getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);
	free (ucsnumber);
}

void AP_UnixDialog_Goto::s_response (GtkWidget * widget, gint id, AP_UnixDialog_Goto * me )
{
  switch ( id )
    {
    case BUTTON_PREVIOUS:
      s_prevClicked ( widget, me ) ; break ;
    case BUTTON_NEXT:
      s_nextClicked ( widget, me ) ; break ;
    case BUTTON_GOTO:
      s_gotoClicked ( widget, me ) ; break ;
    default:
      abiDestroyWidget ( widget ) ; break ; // will emit other signals for us
    }
}

void AP_UnixDialog_Goto::s_gotoClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	const char *number = gtk_entry_get_text (GTK_ENTRY (me->m_wEntry));
	if (number && *number)
			s_goto ((const char *) number, me);
}

void AP_UnixDialog_Goto::s_nextClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	s_goto ("+1", me);
}

void AP_UnixDialog_Goto::s_prevClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	s_goto ("-1", me);
}

void AP_UnixDialog_Goto::s_closeClicked (GtkWidget * widget, AP_UnixDialog_Goto * me)
{
	me->destroy();
}

void AP_UnixDialog_Goto::s_deleteClicked (GtkWidget * widget, gpointer /* data */,AP_UnixDialog_Goto * me)
{
	me->destroy();
}



void AP_UnixDialog_Goto::s_targetChanged (GtkWidget *clist, gint row, gint column,
										  GdkEventButton *event, AP_UnixDialog_Goto *me)
{
	me->setSelectedRow (row);
}

void AP_UnixDialog_Goto::s_dataChanged (GtkWidget *widget, AP_UnixDialog_Goto * me)
{
	const gchar *text = gtk_entry_get_text (GTK_ENTRY (widget));

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

void AP_UnixDialog_Goto::setSelectedRow (int row)
{
	m_iRow = row;

	XAP_String_Id id = AP_STRING_ID_DLG_Goto_Label_Number;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	if(row == (int) AP_JUMPTARGET_BOOKMARK)
	{
		gtk_widget_hide(m_dlabel);
		gtk_widget_show(m_swindow);
		gtk_widget_set_sensitive (m_wPrev, FALSE);
		gtk_widget_set_sensitive (m_wNext, FALSE);
		id = AP_STRING_ID_DLG_Goto_Label_Name;
	}
	else
	{
		gtk_widget_hide(m_swindow);
		gtk_widget_show(m_dlabel);
		gtk_widget_set_sensitive (m_wPrev, TRUE);
		gtk_widget_set_sensitive (m_wNext, TRUE);
	}

	// change string ids
	char * tmp = s_convert ((char*)pSS->getValueUTF8 (id).utf8_str());
	gtk_label_parse_uline (GTK_LABEL (m_numberLabel), tmp);
	g_free (tmp);
}

int AP_UnixDialog_Goto::getSelectedRow (void)
{
	return (m_iRow);
}

void AP_UnixDialog_Goto::runModeless (XAP_Frame * pFrame)
{
	_constructWindow ();
	setSelectedRow ( 0 ) ;

	UT_ASSERT (m_wMainWindow);

	abiSetupModelessDialog( GTK_DIALOG(m_wMainWindow),pFrame, this, BUTTON_CLOSE );
}

void AP_UnixDialog_Goto::destroy (void)
{
	UT_ASSERT (m_wMainWindow);
	modeless_cleanup();
	abiDestroyWidget(m_wMainWindow);
	m_wMainWindow = NULL;
}

void AP_UnixDialog_Goto::activate (void)
{
	UT_ASSERT (m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), m_WindowName);
	gdk_window_raise (m_wMainWindow->window);
}


void AP_UnixDialog_Goto::notifyActiveFrame(XAP_Frame *pFrame)
{
        UT_ASSERT(m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), m_WindowName);
}

GtkWidget * AP_UnixDialog_Goto::_constructWindow (void)
{
	GtkWidget *vbox;
	GtkWidget *actionarea;
	GtkWidget *contents;

        ConstructWindowName();
	m_wMainWindow = abiDialogNew( "goto dialog", TRUE, m_WindowName );
	gtk_container_set_border_width (GTK_CONTAINER (m_wMainWindow), 4);

	vbox = GTK_DIALOG (m_wMainWindow)->vbox;
	actionarea = GTK_DIALOG (m_wMainWindow)->action_area;

	contents = _constructWindowContents ();

	// TODO: This call must be in _constructWindowContents
	gtk_window_add_accel_group (GTK_WINDOW (m_wMainWindow), m_accelGroup);

	gtk_box_pack_start (GTK_BOX (vbox), contents, TRUE, TRUE, 0);
	
	// Buttons
	m_wClose = abiAddStockButton(GTK_DIALOG(m_wMainWindow), GTK_STOCK_CLOSE, BUTTON_CLOSE);
	m_wPrev = abiAddStockButton(GTK_DIALOG(m_wMainWindow), GTK_STOCK_GO_BACK, BUTTON_PREVIOUS);
	m_wNext = abiAddStockButton(GTK_DIALOG(m_wMainWindow), GTK_STOCK_GO_FORWARD, BUTTON_NEXT);
	m_wGoto = abiAddStockButton(GTK_DIALOG(m_wMainWindow), GTK_STOCK_JUMP_TO, BUTTON_GOTO);

	//const XAP_StringSet * pSS = m_pApp->getStringSet();
	//m_wGoto = gtk_button_new_with_label (pSS->getValueUTF8 (AP_STRING_ID_DLG_Goto_Btn_Goto).utf8_str());

	gtk_widget_show_all (m_wMainWindow);
	
	//initially hide the bookmark list; also we want the bookmark list to be
	// of same size as the descriptive text, so that when we swap them
	// the whole window does not get resized
	gtk_window_set_default_size(GTK_WINDOW(m_swindow),(gint)m_dlabel->allocation.width,(gint)m_dlabel->allocation.height);
	gtk_widget_hide(m_swindow);
	
	_connectSignals ();

	return (m_wMainWindow);
}

GtkWidget *AP_UnixDialog_Goto::_constructWindowContents (void)
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
	tmp = s_convert((char*)pSS->getValueUTF8(AP_STRING_ID_DLG_Goto_Label_What).utf8_str());
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
	m_numberLabel = number_lb;
	tmp = s_convert ((char*)pSS->getValueUTF8 (AP_STRING_ID_DLG_Goto_Label_Number).utf8_str());
	number_lb_key = gtk_label_parse_uline (GTK_LABEL (number_lb), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox2), number_lb, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (number_lb), 0, 0.5);

	m_wEntry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), m_wEntry, FALSE, FALSE, 0);

	m_dlabel = gtk_label_new (pSS->getValueUTF8 (AP_STRING_ID_DLG_Goto_Label_Help).utf8_str());
	gtk_box_pack_start (GTK_BOX (vbox2), m_dlabel, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (m_dlabel), GTK_JUSTIFY_FILL);
	gtk_label_set_line_wrap (GTK_LABEL (m_dlabel), TRUE);
	gtk_misc_set_alignment (GTK_MISC (m_dlabel), 0, 0.5);
	
	// the bookmark list
	m_swindow  = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_swindow),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox2), m_swindow, FALSE, FALSE, 0);
	gtk_widget_hide(m_swindow);
	
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

void AP_UnixDialog_Goto::_populateWindowData (void) {}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_Goto * dlg)
{
	UT_ASSERT(dlg);
	UT_DEBUGMSG(("DOM: destroying dialog\n"));
	dlg->destroy();
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * dlg)
{
	abiDestroyWidget(widget);
}

void AP_UnixDialog_Goto::_connectSignals(void)
{
	g_signal_connect_after(G_OBJECT(m_wMainWindow),
			       "response",
			       G_CALLBACK(s_response),
			       this);

	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_wMainWindow),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(m_wMainWindow),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);
}
