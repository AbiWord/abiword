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

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Options.h"

/*****************************************************************/

#define WIDGET_MENU_OPTION_PTR		"menuoptionptr"
#define WIDGET_MENU_VALUE_TAG		"value"

/*****************************************************************/

static void s_radio_toggled (GtkWidget * w, GtkWidget * c)
{
  GtkCList * clist = GTK_CLIST (c);
  gboolean b = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));
  int row = GPOINTER_TO_INT (gtk_object_get_user_data (GTK_OBJECT (w)));

  xxx_UT_DEBUGMSG(("DOM: toggled row: %d val: %d\n", row, b));

  gtk_clist_set_row_data (clist, row, GINT_TO_POINTER(b));
}

XAP_Dialog * AP_UnixDialog_Options::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_UnixDialog_Options * p = new AP_UnixDialog_Options(pFactory,id);
    return p;
}

AP_UnixDialog_Options::AP_UnixDialog_Options(XAP_DialogFactory * pDlgFactory,
					     XAP_Dialog_Id id)
  : AP_Dialog_Options(pDlgFactory,id)
{
}

AP_UnixDialog_Options::~AP_UnixDialog_Options(void)
{
}

/*****************************************************************/

void AP_UnixDialog_Options::runModal(XAP_Frame * pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
	// save for use with event
	m_pFrame = pFrame;

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
	do {
		gtk_main();

		switch ( m_answer )
		{
		case AP_Dialog_Options::a_OK:
			_storeWindowData();
			break;

		case AP_Dialog_Options::a_APPLY:
			UT_DEBUGMSG(("Applying changes\n"));
			_storeWindowData();
			break;

		case AP_Dialog_Options::a_CANCEL:
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		};

	} while ( m_answer == AP_Dialog_Options::a_APPLY );	
	
	if(mainWindow && GTK_IS_WIDGET(mainWindow))
	  gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_Options::event_clistClicked (int row, int col)
{
  GtkCList * clist = GTK_CLIST (m_toolbarClist);
  bool b = (bool)GPOINTER_TO_INT(gtk_clist_get_row_data (clist, row));

  gtk_object_set_user_data (GTK_OBJECT(m_checkbuttonViewShowTB), GINT_TO_POINTER(row));
  xxx_UT_DEBUGMSG (("DOM: setting row %d to %d\n", row, b));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonViewShowTB), (b ? TRUE : FALSE));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonViewHideTB), (b ? FALSE : TRUE));
}

void AP_UnixDialog_Options::event_OK(void)
{
    m_answer = AP_Dialog_Options::a_OK;
    gtk_main_quit();
}

void AP_UnixDialog_Options::event_Cancel(void)
{
    m_answer = AP_Dialog_Options::a_CANCEL;
    gtk_main_quit();
}

void AP_UnixDialog_Options::event_Apply(void)
{
    m_answer = AP_Dialog_Options::a_APPLY;
    gtk_main_quit();
}

void AP_UnixDialog_Options::event_WindowDelete(void)
{
    m_answer = AP_Dialog_Options::a_CANCEL;
    gtk_main_quit();
}

/*****************************************************************/
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        gtk_signal_connect(GTK_OBJECT(w), "activate",	\
                GTK_SIGNAL_FUNC(s_menu_item_activate),		\
                (gpointer) this);							\
        } while (0)

GtkWidget* AP_UnixDialog_Options::_constructWindowContents (GtkWidget * vbox)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkWidget *mainWindow;
	GtkWidget *dialog_vbox1;
	GtkWidget *notebook1;
	GtkWidget *hbox3;
	GtkWidget *vbox17;
	GtkWidget *frame13;
	GtkWidget *toolbar_clist;
	GtkWidget *vbox18;
	GtkWidget *frame14;
	GtkWidget *vbox19;
	GSList *vbox19_group = NULL;
	GtkWidget *show_toolbar;
	GtkWidget *hide_toolbar;
	GtkWidget *frame15;
	GtkWidget *vbox20;
	GSList *vbox20_group = NULL;
	GtkWidget *look_icon;
	GtkWidget *look_text;
	GtkWidget *look_both;
	GtkWidget *view_tooltips;
	GtkWidget *label4;
	GtkWidget *vbox30;
	GtkWidget *hbox9;
	GtkWidget *frame21;
	GtkWidget *vbox31;
	GtkWidget *check_spell;
	GtkWidget *hide_errors;
	GtkWidget *always_suggest;
	GtkWidget *suggest_from;
	GtkWidget *frame22;
	GtkWidget *vbox32;
	GtkWidget *ignore_upper;
	GtkWidget *ignore_nums;
	GtkWidget *ignore_inet;
	GtkWidget *vbox33;
	GtkWidget *hbox10;
	GtkWidget *hbox11;
	GtkWidget *table2;
	GtkWidget *label7;
	GtkWidget *custom_dict;
	GtkWidget *custom_dict_menu;
	GtkWidget *glade_menuitem;
	GtkWidget *label8;
	GtkWidget *button_dict;
	GtkWidget *button_edit;
	GtkWidget *button_reset;
	GtkWidget *label2;
	GtkWidget *vbox45;
	GtkWidget *vbox53;
	GtkWidget *vbox54;
	GtkWidget *hbox23;
	GtkWidget *frame38;
	GtkWidget *vbox55;
	GtkWidget *show_ruler;
	GtkWidget *show_statusbar;
	GtkWidget *blink_cursor;
	GtkWidget *frame39;
	GtkWidget *vbox56;
	GtkWidget *view_all;
	GtkWidget *view_hidden;
	GtkWidget *view_invis;
	GtkWidget *table4;
	GtkWidget *ruler_units;
	GtkWidget *ruler_units_menu;
	GtkWidget *page_size;
	GtkWidget *page_size_menu;
	GtkWidget *label22;
	GtkWidget *label21;
	GtkWidget *vbox58;
	GtkWidget *enable_sq;
	GtkWidget *label3;
	GtkWidget *vbox36;
	GtkWidget *frame40;
	GtkWidget *vbox57;
	GtkWidget *save_scheme;
	GtkWidget *hbox25;
	GtkWidget *label17;
	GtkWidget *current_scheme;
	GtkWidget *label10;
	GtkWidget *frame43;
	GtkWidget *hbox26;
	GtkWidget *hbox27;
	GtkWidget *autosave_cb;
	GtkObject *autosave_time_adj;
	GtkWidget *autosave_time;
	GtkWidget *label23;
	GtkWidget *hbox28;
	GtkWidget *label24;
	GtkWidget *autosave_ext;

#ifdef BIDI_ENABLED
	GtkWidget *frame42;
	GtkWidget *vbox59;
	GtkWidget *rtl_dominant;
#endif
	
	mainWindow = m_windowMain;
	dialog_vbox1 = vbox;
	gtk_widget_show (dialog_vbox1);
	
	notebook1 = gtk_notebook_new ();
	gtk_widget_show (notebook1);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), notebook1, TRUE, TRUE, 0);

	// TODO: fully support the toolbar tab

	hbox3 = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hbox3);
	gtk_container_add (GTK_CONTAINER (notebook1), hbox3);
	gtk_container_set_border_width (GTK_CONTAINER (hbox3), 4);
	
	vbox17 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox17);
	gtk_box_pack_start (GTK_BOX (hbox3), vbox17, TRUE, TRUE, 0);
	
	frame13 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Toolbars));
	gtk_widget_show (frame13);
	gtk_box_pack_start (GTK_BOX (vbox17), frame13, TRUE, TRUE, 0);
	
	toolbar_clist = gtk_clist_new (1);
	gtk_widget_show (toolbar_clist);
	gtk_container_add (GTK_CONTAINER (frame13), toolbar_clist);
	gtk_container_set_border_width (GTK_CONTAINER (toolbar_clist), 5);

	gtk_clist_column_titles_passive (GTK_CLIST (toolbar_clist));

	gtk_clist_freeze (GTK_CLIST (toolbar_clist));
	gtk_clist_clear  (GTK_CLIST (toolbar_clist));

	gtk_clist_set_column_title (GTK_CLIST (toolbar_clist), 1,
				    pSS->getValue(AP_STRING_ID_DLG_Options_Label_Toolbars));

	gchar *data[1];
	
	data[0] = (gchar *)pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewStandardTB);
	gtk_clist_append (GTK_CLIST(toolbar_clist), data);

	data[0] = (gchar *)pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewFormatTB);
	gtk_clist_append (GTK_CLIST(toolbar_clist), data);

	data[0] = (gchar *)pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewExtraTB);
	gtk_clist_append (GTK_CLIST(toolbar_clist), data);

	gtk_clist_thaw (GTK_CLIST (toolbar_clist));

	gtk_signal_connect (GTK_OBJECT (toolbar_clist), "select_row",
			    GTK_SIGNAL_FUNC (s_clist_clicked), (gpointer)this);

#if 0
	gtk_signal_connect (GTK_OBJECT (toolbar_clist), "unselect_row",
			    GTK_SIGNAL_FUNC (NULL), (gpointer)this);
#endif

	vbox18 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox18);
	gtk_box_pack_start (GTK_BOX (hbox3), vbox18, TRUE, TRUE, 0);
	
	frame14 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Visible));
	gtk_widget_show (frame14);
	gtk_box_pack_start (GTK_BOX (vbox18), frame14, TRUE, TRUE, 0);
	
	vbox19 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox19);
	gtk_container_add (GTK_CONTAINER (frame14), vbox19);
	gtk_container_set_border_width (GTK_CONTAINER (vbox19), 1);
	
	show_toolbar = gtk_radio_button_new_with_label (vbox19_group, pSS->getValue(AP_STRING_ID_DLG_Options_Label_Show));
	vbox19_group = gtk_radio_button_group (GTK_RADIO_BUTTON (show_toolbar));
	gtk_widget_show (show_toolbar);
	gtk_box_pack_start (GTK_BOX (vbox19), show_toolbar, FALSE, FALSE, 0);

	gtk_signal_connect (GTK_OBJECT (show_toolbar), "toggled",
			    GTK_SIGNAL_FUNC (s_radio_toggled), toolbar_clist);
	
	hide_toolbar = gtk_radio_button_new_with_label (vbox19_group, pSS->getValue(AP_STRING_ID_DLG_Options_Label_Hide));
	vbox19_group = gtk_radio_button_group (GTK_RADIO_BUTTON (hide_toolbar));
	gtk_widget_show (hide_toolbar);
	gtk_box_pack_start (GTK_BOX (vbox19), hide_toolbar, FALSE, FALSE, 0);
	
	frame15 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Look));
	gtk_widget_show (frame15);
	gtk_box_pack_start (GTK_BOX (vbox18), frame15, TRUE, TRUE, 2);
	
	vbox20 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox20);
	gtk_container_add (GTK_CONTAINER (frame15), vbox20);
	gtk_container_set_border_width (GTK_CONTAINER (vbox20), 1);
	
	look_icon = gtk_radio_button_new_with_label (vbox20_group, pSS->getValue(AP_STRING_ID_DLG_Options_Label_Icons));
	vbox20_group = gtk_radio_button_group (GTK_RADIO_BUTTON (look_icon));
	gtk_widget_show (look_icon);
	gtk_box_pack_start (GTK_BOX (vbox20), look_icon, FALSE, FALSE, 0);
	
	look_text = gtk_radio_button_new_with_label (vbox20_group, pSS->getValue(AP_STRING_ID_DLG_Options_Label_Text));
	vbox20_group = gtk_radio_button_group (GTK_RADIO_BUTTON (look_text));
	gtk_widget_show (look_text);
	gtk_box_pack_start (GTK_BOX (vbox20), look_text, FALSE, FALSE, 0);
	
	look_both = gtk_radio_button_new_with_label (vbox20_group, pSS->getValue(AP_STRING_ID_DLG_Options_Label_Both));
	vbox20_group = gtk_radio_button_group (GTK_RADIO_BUTTON (look_both));
	gtk_widget_show (look_both);
	gtk_box_pack_start (GTK_BOX (vbox20), look_both, FALSE, FALSE, 0);

	// TODO: make these next 4 controls editable && usable
	gtk_widget_set_sensitive (look_icon, FALSE);
	gtk_widget_set_sensitive (look_text, FALSE);
	gtk_widget_set_sensitive (look_both, FALSE);

	view_tooltips = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewTooltips));
	gtk_widget_show (view_tooltips);
	gtk_box_pack_start (GTK_BOX (vbox18), view_tooltips, FALSE, FALSE, 2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view_tooltips), TRUE);
	gtk_widget_set_sensitive (view_tooltips, FALSE);

	label4 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Toolbars));
	gtk_widget_show (label4);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label4);


	// SPELLING TAB


	vbox30 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox30);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox30);
	gtk_container_set_border_width (GTK_CONTAINER (vbox30), 4);
	
	hbox9 = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hbox9);
	gtk_box_pack_start (GTK_BOX (vbox30), hbox9, TRUE, TRUE, 0);
	
	frame21 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_General));
	gtk_widget_show (frame21);
	gtk_box_pack_start (GTK_BOX (hbox9), frame21, TRUE, TRUE, 0);
	
	vbox31 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox31);
	gtk_container_add (GTK_CONTAINER (frame21), vbox31);
	
	check_spell = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellCheckAsType));
	gtk_widget_show (check_spell);
	gtk_box_pack_start (GTK_BOX (vbox31), check_spell, FALSE, FALSE, 0);
	
	hide_errors = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellHideErrors));
	gtk_widget_show (hide_errors);
	gtk_box_pack_start (GTK_BOX (vbox31), hide_errors, FALSE, FALSE, 0);
	
	always_suggest = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellSuggest));
	gtk_widget_show (always_suggest);
	gtk_box_pack_start (GTK_BOX (vbox31), always_suggest, FALSE, FALSE, 0);
	
	suggest_from = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellMainOnly));
	gtk_widget_show (suggest_from);
	gtk_box_pack_start (GTK_BOX (vbox31), suggest_from, FALSE, FALSE, 0);
	
	frame22 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Ignore));
	gtk_widget_show (frame22);
	gtk_box_pack_start (GTK_BOX (hbox9), frame22, TRUE, TRUE, 0);
	
	vbox32 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox32);
	gtk_container_add (GTK_CONTAINER (frame22), vbox32);
  
	ignore_upper = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellUppercase));
	gtk_widget_show (ignore_upper);
	gtk_box_pack_start (GTK_BOX (vbox32), ignore_upper, FALSE, FALSE, 0);
	
	ignore_nums = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellNumbers));
	gtk_widget_show (ignore_nums);
	gtk_box_pack_start (GTK_BOX (vbox32), ignore_nums, FALSE, FALSE, 0);

	ignore_inet = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellInternet));
	gtk_widget_show (ignore_inet);
	gtk_box_pack_start (GTK_BOX (vbox32), ignore_inet, FALSE, FALSE, 0);
	
	vbox33 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox33);
	gtk_box_pack_start (GTK_BOX (vbox30), vbox33, TRUE, TRUE, 0);
	
	hbox10 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox10);
	gtk_box_pack_start (GTK_BOX (vbox33), hbox10, TRUE, TRUE, 4);
	
	hbox11 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox11);
	gtk_box_pack_start (GTK_BOX (hbox10), hbox11, TRUE, TRUE, 0);
	
	table2 = gtk_table_new (2, 3, TRUE);
	gtk_widget_show (table2);
	gtk_box_pack_start (GTK_BOX (hbox11), table2, TRUE, TRUE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table2), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table2), 4);
	
	label7 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellIgnoredWord));
	gtk_widget_show (label7);
	gtk_table_attach (GTK_TABLE (table2), label7, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);
	
	custom_dict = gtk_option_menu_new ();
	gtk_widget_show (custom_dict);
	gtk_table_attach (GTK_TABLE (table2), custom_dict, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);
	custom_dict_menu = gtk_menu_new ();
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_CustomDict));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (custom_dict_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (custom_dict), custom_dict_menu);
  
	label8 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellCustomDict));
	gtk_widget_show (label8);
	gtk_table_attach (GTK_TABLE (table2), label8, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);
	
	button_dict = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_CustomDict));
	gtk_widget_show (button_dict);
	gtk_table_attach (GTK_TABLE (table2), button_dict, 2, 3, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	
	button_edit = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_IgnoreEdit));
	gtk_widget_show (button_edit);
	gtk_table_attach (GTK_TABLE (table2), button_edit, 2, 3, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	
	button_reset = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_IgnoreReset));
	gtk_widget_show (button_reset);
	gtk_table_attach (GTK_TABLE (table2), button_reset, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	label2 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_TabLabel_Spelling));
	gtk_widget_show (label2);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);


	// LAYOUT TAB


	vbox45 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox45);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox45);
	
	vbox53 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox53);
	gtk_box_pack_start (GTK_BOX (vbox45), vbox53, TRUE, TRUE, 0);
	
	vbox54 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox54);
	gtk_box_pack_start (GTK_BOX (vbox53), vbox54, TRUE, TRUE, 0);
	
	hbox23 = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hbox23);
	gtk_box_pack_start (GTK_BOX (vbox54), hbox23, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox23), 4);
	
	frame38 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewShowHide));
	gtk_widget_show (frame38);
	gtk_box_pack_start (GTK_BOX (hbox23), frame38, TRUE, TRUE, 0);
	
	vbox55 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox55);
	gtk_container_add (GTK_CONTAINER (frame38), vbox55);
	
	show_ruler = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewRuler));
	gtk_widget_show (show_ruler);
	gtk_box_pack_start (GTK_BOX (vbox55), show_ruler, FALSE, FALSE, 0);
	
	show_statusbar = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewStatusBar));
	gtk_widget_show (show_statusbar);
	gtk_box_pack_start (GTK_BOX (vbox55), show_statusbar, FALSE, FALSE, 0);
	
	blink_cursor = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewCursorBlink));
	gtk_widget_show (blink_cursor);
	gtk_box_pack_start (GTK_BOX (vbox55), blink_cursor, FALSE, FALSE, 0);
	
	frame39 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewViewFrame));
	gtk_widget_show (frame39);
	gtk_box_pack_start (GTK_BOX (hbox23), frame39, TRUE, TRUE, 0);
	
	vbox56 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox56);
	gtk_container_add (GTK_CONTAINER (frame39), vbox56);
	
	view_all = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewAll));
	gtk_widget_show (view_all);
	gtk_box_pack_start (GTK_BOX (vbox56), view_all, FALSE, FALSE, 0);
	
	view_hidden = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewHiddenText));
	gtk_widget_show (view_hidden);
	gtk_box_pack_start (GTK_BOX (vbox56), view_hidden, FALSE, FALSE, 0);
	
	view_invis = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewUnprintable));
	gtk_widget_show (view_invis);
	gtk_box_pack_start (GTK_BOX (vbox56), view_invis, FALSE, FALSE, 0);
	
	table4 = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (table4);
	gtk_box_pack_start (GTK_BOX (vbox54), table4, TRUE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table4), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table4), 10);

	// the ruler unit menu
	ruler_units = gtk_option_menu_new ();
	gtk_widget_show (ruler_units);
	gtk_table_attach (GTK_TABLE (table4), ruler_units, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	ruler_units_menu = gtk_menu_new ();

	// inches
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Unit_inch));
 	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) ruler_units);
 	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_IN));	
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	// cm
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Unit_cm));
 	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) ruler_units);
 	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_CM));	
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	// points
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Unit_points));
 	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) ruler_units);
  	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_PT));	
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);
  
	// pico
  	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Unit_pico));
  	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) ruler_units);
  	gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_PI));	
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
  	gtk_widget_show (glade_menuitem);
  	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (ruler_units), ruler_units_menu);

	// the page size menu
	page_size = gtk_option_menu_new ();
	gtk_widget_show (page_size);
	gtk_table_attach (GTK_TABLE (table4), page_size, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND), 0, 0);
	page_size_menu = gtk_menu_new ();
	for (int i = (int)fp_PageSize::A0; i < (int)fp_PageSize::Custom; i++)
	{
	    glade_menuitem = gtk_menu_item_new_with_label (fp_PageSize::PredefinedToName ((fp_PageSize::Predefined)i));
	    gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) page_size);
	    gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG, GINT_TO_POINTER(i));
	    CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	    gtk_widget_show (glade_menuitem);
	    gtk_menu_append (GTK_MENU (page_size_menu), glade_menuitem);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (page_size), page_size_menu);
	
	label22 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_DefaultPageSize));
	gtk_widget_show (label22);
	gtk_table_attach (GTK_TABLE (table4), label22, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label22), GTK_JUSTIFY_LEFT);
	
	label21 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewUnits));
	gtk_widget_show (label21);
	gtk_table_attach (GTK_TABLE (table4), label21, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label21), GTK_JUSTIFY_LEFT);
	
	vbox58 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox58);
	gtk_box_pack_start (GTK_BOX (vbox54), vbox58, TRUE, TRUE, 0);
	
	enable_sq = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SmartQuotesEnable));
	gtk_widget_show (enable_sq);
	gtk_box_pack_start (GTK_BOX (vbox58), enable_sq, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (enable_sq), 2);
	
	label3 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Layout));
	gtk_widget_show (label3);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label3);


	// PREFERENCES / MISC TAB


	vbox36 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox36);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox36);
	
	frame40 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Schemes));
	gtk_widget_show (frame40);
	gtk_box_pack_start (GTK_BOX (vbox36), frame40, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame40), 4);
	
	vbox57 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox57);
	gtk_container_add (GTK_CONTAINER (frame40), vbox57);
	gtk_container_set_border_width (GTK_CONTAINER (vbox57), 4);
	
	save_scheme = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_PrefsAutoSave));
	gtk_widget_show (save_scheme);
	gtk_box_pack_start (GTK_BOX (vbox57), save_scheme, FALSE, FALSE, 0);
	
	hbox25 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox25);
	gtk_box_pack_start (GTK_BOX (vbox57), hbox25, TRUE, TRUE, 0);
	
	label17 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_PrefsCurrentScheme));
	gtk_widget_show (label17);
	gtk_box_pack_start (GTK_BOX (hbox25), label17, FALSE, TRUE, 3);
	
	current_scheme = gtk_combo_new ();
	gtk_widget_show (current_scheme);
	gtk_box_pack_start (GTK_BOX (hbox25), current_scheme, TRUE, TRUE, 0);
	//gtk_entry_set_editable (GTK_ENTRY (current_scheme), FALSE);

#ifdef BIDI_ENABLED
	frame42 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_BiDiOptions));
	gtk_widget_show (frame42);
	gtk_box_pack_start (GTK_BOX (vbox36), frame42, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame42), 4);

	vbox59 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox59);
	gtk_container_add (GTK_CONTAINER (frame42), vbox59);

	rtl_dominant = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_DirectionRtl));
	gtk_widget_show (rtl_dominant);
	gtk_box_pack_start (GTK_BOX (vbox59), rtl_dominant, FALSE, FALSE, 0);
#endif		
	
	// AUTO SAVE
	frame43 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_AutoSave));
	gtk_widget_show (frame43);
	gtk_box_pack_start (GTK_BOX (vbox36), frame43, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame43), 4);
	
	hbox26 = gtk_hbox_new (FALSE, 14);
	gtk_widget_show (hbox26);
	gtk_container_add (GTK_CONTAINER (frame43), hbox26);
	
	hbox27 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox27);
	gtk_box_pack_start (GTK_BOX (hbox26), hbox27, TRUE, TRUE, 0);
	
	autosave_cb = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_AutoSaveCurrent));
	gtk_widget_show (autosave_cb);
	gtk_box_pack_start (GTK_BOX (hbox27), autosave_cb, FALSE, FALSE, 0);
	
	autosave_time_adj = gtk_adjustment_new (5, 1, 120, 1, 5, 5);
	autosave_time = gtk_spin_button_new (GTK_ADJUSTMENT (autosave_time_adj), 1, 0);
	gtk_widget_show (autosave_time);
	gtk_box_pack_start (GTK_BOX (hbox27), autosave_time, FALSE, TRUE, 0);
	
	label23 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_Minutes));
	gtk_widget_show (label23);
	gtk_box_pack_start (GTK_BOX (hbox27), label23, FALSE, FALSE, 0);
	
	hbox28 = gtk_hbox_new (TRUE, 4);
	gtk_widget_show (hbox28);
	gtk_box_pack_start (GTK_BOX (hbox26), hbox28, TRUE, FALSE, 0);
	
	label24 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_WithExtension));
	gtk_widget_show (label24);
	gtk_box_pack_start (GTK_BOX (hbox28), label24, FALSE, FALSE, 0);
	
	autosave_ext = gtk_entry_new_with_max_length (5);
	gtk_widget_show (autosave_ext);
	gtk_box_pack_start (GTK_BOX (hbox28), autosave_ext, TRUE, TRUE, 0);
	gtk_widget_set_usize (autosave_ext, 50, -2);

	label10 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_TabLabel_Preferences));
	gtk_widget_show (label10);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label10);


	//////////////////////////////////////////////////////////////////

	m_notebook = notebook1;
#ifdef BIDI_ENABLED
	m_checkbuttonOtherDirectionRtl = rtl_dominant;
#endif
	m_checkbuttonAutoSaveFile = autosave_cb;
	m_textAutoSaveFileExt = autosave_ext;
	m_textAutoSaveFilePeriod = autosave_time;

	m_checkbuttonSpellCheckAsType	        = check_spell;
	m_checkbuttonSpellHideErrors	        = hide_errors;
	m_checkbuttonSpellSuggest		= always_suggest;
	m_checkbuttonSpellMainOnly		= suggest_from;
	m_checkbuttonSpellUppercase		= ignore_upper;
	m_checkbuttonSpellNumbers		= ignore_nums;
	m_checkbuttonSpellInternet		= ignore_inet;
	m_listSpellDicts			= custom_dict;
	m_listSpellDicts_menu			= custom_dict_menu;
	m_buttonSpellDictionary			= button_dict;
	m_buttonSpellIgnoreEdit			= button_edit;
	m_buttonSpellIgnoreReset		= button_reset;

	m_checkbuttonSmartQuotesEnable	        = enable_sq;
	m_listDefaultPageSize			= page_size;

	m_checkbuttonPrefsAutoSave		= save_scheme;
	m_comboPrefsScheme			= current_scheme;

	m_checkbuttonViewShowRuler		= show_ruler;
	m_listViewRulerUnits			= ruler_units;
	m_checkbuttonViewCursorBlink	        = blink_cursor;

	m_checkbuttonViewShowStatusBar	        = show_statusbar;
	m_checkbuttonViewAll			= view_all;
	m_checkbuttonViewHiddenText		= view_hidden;
	m_checkbuttonViewUnprintable	        = view_invis;

	// TODO: rulers
	m_checkbuttonViewShowTB	= show_toolbar;
	m_checkbuttonViewHideTB = hide_toolbar;
	m_toolbarClist = toolbar_clist;

	gtk_signal_connect(GTK_OBJECT(m_buttonSpellIgnoreEdit),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_ignore_edit_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_buttonSpellIgnoreReset),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_ignore_reset_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_buttonSpellDictionary),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_dict_edit_clicked),
			   (gpointer) this);
	
	// to enable/disable other controls (hide errors)
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonSpellCheckAsType),
			   "toggled",
			   GTK_SIGNAL_FUNC(s_checkbutton_toggle),
			   (gpointer) this);

	_setNotebookPageNum (0);
	//gtk_clist_select_row (GTK_CLIST (toolbar_clist), 0, 0);
	gtk_clist_moveto (GTK_CLIST (toolbar_clist), 0, 0, 0, 0);
	return notebook1;
}

GtkWidget* AP_UnixDialog_Options::_constructWindow ()
{
        //////////////////////////////////////////////////////////////////////

	// for the internationalization	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	GtkWidget * mainWindow;
	GtkWidget * hbuttonbox;
	GtkWidget * buttonApply;
	GtkWidget * buttonDefaults;
	GtkWidget * buttonOk;
	GtkWidget * buttonCancel;

	mainWindow = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (mainWindow), pSS->getValue(AP_STRING_ID_DLG_Options_OptionsTitle));
	GTK_WINDOW (mainWindow)->type = GTK_WINDOW_DIALOG;
	gtk_window_set_policy (GTK_WINDOW (mainWindow), TRUE, TRUE, FALSE);

	hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (mainWindow)->action_area), hbuttonbox);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox), 10);

	buttonApply = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Apply ));
	gtk_widget_show (buttonApply);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), buttonApply);
	GTK_WIDGET_SET_FLAGS (buttonApply, GTK_CAN_DEFAULT);

	buttonDefaults = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Default));
	gtk_widget_show (buttonDefaults);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), buttonDefaults);
	GTK_WIDGET_SET_FLAGS (buttonDefaults, GTK_CAN_DEFAULT);

	buttonOk = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show (buttonOk);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), buttonOk);
	GTK_WIDGET_SET_FLAGS (buttonOk, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);


	// the catch-alls
	gtk_signal_connect(GTK_OBJECT(mainWindow),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) this);


	gtk_signal_connect_after(GTK_OBJECT(mainWindow),
				 "destroy",
				 NULL,
				 NULL);

	//////////////////////////////////////////////////////////////////////
	// the control buttons
	gtk_signal_connect(GTK_OBJECT(buttonOk),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_ok_clicked),
			   (gpointer) this);
    
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_cancel_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonDefaults),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_defaults_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonApply),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_apply_clicked),
			   (gpointer) this);
	

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = mainWindow;

	_constructWindowContents(GTK_DIALOG(mainWindow)->vbox);

	m_buttonDefaults				= buttonDefaults;
	m_buttonApply					= buttonApply;
	m_buttonOK						= buttonOk;
	m_buttonCancel					= buttonCancel;

	// create the accelerators from &'s
	createLabelAccelerators(mainWindow);

	// create user data tControl -> stored in widgets
	for ( int i = 0; i < id_last; i++ )
	{
		GtkWidget *w = _lookupWidget( (tControl)i );
		if (!(w && GTK_IS_WIDGET (w)))
		  continue;

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		UT_ASSERT( gtk_object_get_data(GTK_OBJECT(w), "tControl" ) == NULL);

		gtk_object_set_data( GTK_OBJECT(w), "tControl", (gpointer) i );
	}

	return mainWindow;
}

GtkWidget *AP_UnixDialog_Options::_lookupWidget ( tControl id )
{
	switch (id)
	{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// spell
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		return m_checkbuttonSpellCheckAsType;

	case id_CHECK_SPELL_HIDE_ERRORS:
		return m_checkbuttonSpellHideErrors;

	case id_CHECK_SPELL_SUGGEST:
		return m_checkbuttonSpellSuggest;

	case id_CHECK_SPELL_MAIN_ONLY:
		return m_checkbuttonSpellMainOnly;

	case id_CHECK_SPELL_UPPERCASE:
		return m_checkbuttonSpellUppercase;

	case id_CHECK_SPELL_NUMBERS:
		return m_checkbuttonSpellNumbers;

	case id_CHECK_SPELL_INTERNET:
		return m_checkbuttonSpellInternet;

	case id_LIST_DICTIONARY:
		return m_listSpellDicts;

	case id_BUTTON_DICTIONARY_EDIT:
		return m_buttonSpellDictionary;

	case id_BUTTON_IGNORE_RESET:
		return m_buttonSpellIgnoreReset;

	case id_BUTTON_IGNORE_EDIT:
		return m_buttonSpellIgnoreEdit;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// other
	case id_CHECK_SMART_QUOTES_ENABLE:
		return m_checkbuttonSmartQuotesEnable;

	case id_LIST_DEFAULT_PAGE_SIZE:
		return m_listDefaultPageSize;

#ifdef BIDI_ENABLED
	case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
		return m_checkbuttonOtherDirectionRtl;
#endif

	case id_CHECK_AUTO_SAVE_FILE:
		return m_checkbuttonAutoSaveFile;

	case id_TEXT_AUTO_SAVE_FILE_EXT:
		return m_textAutoSaveFileExt;
		
	case id_TEXT_AUTO_SAVE_FILE_PERIOD:
		return m_textAutoSaveFilePeriod;
		
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// prefs
	case id_CHECK_PREFS_AUTO_SAVE:
		return m_checkbuttonPrefsAutoSave;

	case id_COMBO_PREFS_SCHEME:
		return m_comboPrefsScheme;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// view
	case id_CHECK_VIEW_SHOW_RULER:
		return m_checkbuttonViewShowRuler;

	case id_LIST_VIEW_RULER_UNITS:
		return m_listViewRulerUnits;

	case id_CHECK_VIEW_CURSOR_BLINK:
		return m_checkbuttonViewCursorBlink;

	case id_CHECK_VIEW_SHOW_STATUS_BAR:
		return m_checkbuttonViewShowStatusBar;

	case id_CHECK_VIEW_ALL:
		return m_checkbuttonViewAll;

	case id_CHECK_VIEW_HIDDEN_TEXT:
		return m_checkbuttonViewHiddenText;

	case id_CHECK_VIEW_UNPRINTABLE:
		return m_checkbuttonViewUnprintable;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// general

	case id_BUTTON_DEFAULTS:
		return m_buttonDefaults;

	case id_BUTTON_OK:
		return m_buttonOK;

	case id_BUTTON_CANCEL:
		return m_buttonCancel;

	case id_BUTTON_APPLY:
		return m_buttonApply;

		// not implemented
	case id_BUTTON_SAVE:
	case id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR:
	case id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR:
	case id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR:
	  return 0;

	default:
		UT_ASSERT("Unknown Widget");
		return 0;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return 0;
}

void AP_UnixDialog_Options::_controlEnable( tControl id, bool value )
{
	GtkWidget *w = _lookupWidget(id);

	if (w && GTK_IS_WIDGET (w))
	  gtk_widget_set_sensitive( w, value );
}


#define DEFINE_CLIST_GET_SET_BOOL(itm, row) \
bool AP_UnixDialog_Options::_gather##itm(void) { \
        UT_ASSERT (m_toolbarClist); \
        bool b = (bool)GPOINTER_TO_INT (gtk_clist_get_row_data (GTK_CLIST (m_toolbarClist), row)); \
        xxx_UT_DEBUGMSG(("DOM: _gather %d %d\n", row, b)); \
        return b; \
} \
void AP_UnixDialog_Options::_set##itm(bool b) { \
        UT_ASSERT (m_toolbarClist); \
        xxx_UT_DEBUGMSG(("DOM: _set %d %d\n", row, b)); \
        gtk_clist_set_row_data (GTK_CLIST (m_toolbarClist), row, GINT_TO_POINTER(b)); \
}

DEFINE_CLIST_GET_SET_BOOL(ViewShowStandardBar, 0);
DEFINE_CLIST_GET_SET_BOOL(ViewShowFormatBar, 1);
DEFINE_CLIST_GET_SET_BOOL(ViewShowExtraBar, 2);
#undef DEFINE_CLIST_GET_SET_BOOL


#define DEFINE_GET_SET_BOOL(button) \
bool     AP_UnixDialog_Options::_gather##button(void) {				\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	return gtk_toggle_button_get_active(								\
				GTK_TOGGLE_BUTTON(m_checkbutton##button) ); }			\
void        AP_UnixDialog_Options::_set##button(bool b) {	\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	gtk_toggle_button_set_active (										\
				GTK_TOGGLE_BUTTON(m_checkbutton##button), b ); }

#define DEFINE_GET_SET_TEXT(widget) \
char *		AP_UnixDialog_Options::_gather##widget() {				\
	UT_ASSERT(m_text##widget && GTK_IS_EDITABLE(m_text##widget));	\
	return gtk_editable_get_chars(GTK_EDITABLE(m_text##widget), 0, -1); }			\
\
void		AP_UnixDialog_Options::_set##widget(const char *t) {	\
	int pos = 0;													\
	UT_ASSERT(m_text##widget && GTK_IS_EDITABLE(m_text##widget));	\
	gtk_editable_delete_text(GTK_EDITABLE(m_text##widget), 0, -1);				\
	gtk_editable_insert_text(GTK_EDITABLE(m_text##widget), t, strlen(t), &pos);	\
}
 
DEFINE_GET_SET_BOOL(SpellCheckAsType);
DEFINE_GET_SET_BOOL(SpellHideErrors);
DEFINE_GET_SET_BOOL(SpellSuggest);
DEFINE_GET_SET_BOOL(SpellMainOnly);
DEFINE_GET_SET_BOOL(SpellUppercase);
DEFINE_GET_SET_BOOL(SpellNumbers);
DEFINE_GET_SET_BOOL(SpellInternet);
DEFINE_GET_SET_BOOL(SmartQuotesEnable);

#ifdef BIDI_ENABLED
DEFINE_GET_SET_BOOL(OtherDirectionRtl);
#endif

#if 1 // TODO: JCA
DEFINE_GET_SET_BOOL(AutoSaveFile);
#endif

DEFINE_GET_SET_BOOL(PrefsAutoSave);
DEFINE_GET_SET_BOOL(ViewShowRuler);
DEFINE_GET_SET_BOOL(ViewShowStatusBar);

#if 1 // TODO: JCA
void AP_UnixDialog_Options::_gatherAutoSaveFileExt(UT_String &stRetVal)
{
	UT_ASSERT(m_textAutoSaveFileExt && GTK_IS_EDITABLE(m_textAutoSaveFileExt));
	char *tmp = gtk_editable_get_chars(GTK_EDITABLE(m_textAutoSaveFileExt), 0, -1);
	stRetVal = tmp;
	g_free(tmp);
}

void AP_UnixDialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	int pos = 0;
	UT_ASSERT(m_textAutoSaveFileExt && GTK_IS_EDITABLE(m_textAutoSaveFileExt));
	gtk_editable_delete_text(GTK_EDITABLE(m_textAutoSaveFileExt), 0, -1);
	gtk_editable_insert_text(GTK_EDITABLE(m_textAutoSaveFileExt), stExt.c_str(), stExt.size(), &pos);
}

void AP_UnixDialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_SPIN_BUTTON(m_textAutoSaveFilePeriod));
	char nb[10];
	int val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod));
	snprintf(nb, 10, "%d", val);
	stRetVal = nb;
}

void AP_UnixDialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_EDITABLE(m_textAutoSaveFilePeriod));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod), atoi(stPeriod.c_str()));
}
#endif

UT_Dimension AP_UnixDialog_Options::_gatherViewRulerUnits(void) 
{				
	UT_ASSERT(m_listViewRulerUnits && GTK_IS_OPTION_MENU(m_listViewRulerUnits)); 
	return (UT_Dimension)((gint)gtk_object_get_data( GTK_OBJECT(m_listViewRulerUnits), WIDGET_MENU_VALUE_TAG )); 
}			

fp_PageSize::Predefined AP_UnixDialog_Options::_gatherDefaultPageSize(void)
{
	UT_ASSERT(m_listDefaultPageSize && GTK_IS_OPTION_MENU(m_listDefaultPageSize));
	return (fp_PageSize::Predefined) ((gint)gtk_object_get_data( GTK_OBJECT(m_listDefaultPageSize), WIDGET_MENU_VALUE_TAG ));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// This function will lookup a option box by the value stored in the 
//	user data under the key WIDGET_MENU_VALUE_TAG
//
typedef struct {
	int index;
	int found;
	gchar *key;
	gpointer data;
} search_data;

static void search_for_value ( GtkWidget *widget, gpointer _value )
{
	search_data *value = (search_data *)_value;

	if ( !GTK_IS_MENU_ITEM(widget))
		return;

	value->index++;

	gint v = (gint) gtk_object_get_data( GTK_OBJECT(widget), value->key );
	if ( v == (gint)value->data )
	{
		// UT_DEBUGMSG(("search_for_value [%d]", (gint) value->data ));
		value->found = value->index;
	}
}

// returns -1 if not found
int option_menu_set_by_key ( GtkWidget *option_menu, gpointer value, gchar *key )
{
	UT_ASSERT( option_menu && key && GTK_IS_OPTION_MENU(option_menu));

	// at least make sure the value will be restored by the _gather
	gtk_object_set_data( GTK_OBJECT(option_menu), key, value);

	// lookup for the key with the value of dim
	search_data data = { -1, -1, key, value };

	GtkWidget *menu = gtk_option_menu_get_menu( GTK_OPTION_MENU(option_menu));
	UT_ASSERT(menu&&GTK_IS_MENU(menu));

	// iterate through all the values
	gtk_container_forall ( GTK_CONTAINER(menu), search_for_value, (gpointer) &data );

	// if we found a value that matches, then say select it
	if ( data.found >= 0 )
	{
		gtk_option_menu_set_history( GTK_OPTION_MENU(option_menu), data.found );
		//UT_DEBUGMSG(("search found %d\n", data.found ));
	}
	else
		UT_DEBUGMSG(("%s:%f search NOT found (searched %d indexes)\n", __FILE__, __LINE__, data.index ));

	return data.found;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void    AP_UnixDialog_Options::_setViewRulerUnits(UT_Dimension dim) 
{	
	UT_ASSERT(m_listViewRulerUnits && GTK_IS_OPTION_MENU(m_listViewRulerUnits)); 

	int r = option_menu_set_by_key ( m_listViewRulerUnits, (gpointer)dim, WIDGET_MENU_VALUE_TAG ); 
	UT_ASSERT( r != -1 );
}

void AP_UnixDialog_Options::_setDefaultPageSize(fp_PageSize::Predefined pre)
{
	UT_ASSERT(m_listDefaultPageSize && GTK_IS_OPTION_MENU(m_listDefaultPageSize));

	int r = option_menu_set_by_key ( m_listDefaultPageSize, (gpointer)pre, WIDGET_MENU_VALUE_TAG );
	UT_ASSERT( r != -1 );
}

DEFINE_GET_SET_BOOL	(ViewCursorBlink);

DEFINE_GET_SET_BOOL	(ViewAll);
DEFINE_GET_SET_BOOL	(ViewHiddenText);
DEFINE_GET_SET_BOOL	(ViewUnprintable);

#undef DEFINE_GET_SET_BOOL

int AP_UnixDialog_Options::_gatherNotebookPageNum(void) 
{				
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook)); 
	return gtk_notebook_get_current_page( GTK_NOTEBOOK(m_notebook) ); 
}			

void    AP_UnixDialog_Options::_setNotebookPageNum(int pn) 
{	
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook)); 
	gtk_notebook_set_page( GTK_NOTEBOOK(m_notebook), pn ); 
}

/*****************************************************************/

// sample callback function
/*static*/ void AP_UnixDialog_Options::s_ok_clicked(GtkWidget * /*widget*/, gpointer data)
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->event_OK(); 
}

/*static*/ void AP_UnixDialog_Options::s_cancel_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Cancel(); 
}

/*static*/ void AP_UnixDialog_Options::s_apply_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Apply(); 
}

/*static*/ void AP_UnixDialog_Options::s_delete_clicked(GtkWidget * /* widget */, GdkEvent * /*event*/, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Options::s_delete_clicked\n"));
	dlg->event_WindowDelete(); 
}


/*static*/ void AP_UnixDialog_Options::s_ignore_reset_clicked( GtkWidget * /* widget */, gpointer  data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->_event_IgnoreReset(); 
}

/*static*/ void AP_UnixDialog_Options::s_ignore_edit_clicked( GtkWidget * /* widget */, gpointer  data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->_event_IgnoreEdit(); 
}

/*static*/ void AP_UnixDialog_Options::s_dict_edit_clicked( GtkWidget * /* widget */, gpointer  data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->_event_DictionaryEdit(); 
}

/*static*/ void AP_UnixDialog_Options::s_defaults_clicked( GtkWidget *widget, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(widget && dlg); 
	dlg->_event_SetDefaults(); 
}


// these function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
/*static*/ void AP_UnixDialog_Options::s_checkbutton_toggle( GtkWidget *w, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	UT_ASSERT(w && GTK_IS_WIDGET(w));

	int i = (int) gtk_object_get_data( GTK_OBJECT(w), "tControl" );
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
}

/*static*/ gint AP_UnixDialog_Options::s_menu_item_activate(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;

	UT_ASSERT(widget && dlg);

	GtkWidget *option_menu = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_MENU_OPTION_PTR);
	UT_ASSERT( option_menu && GTK_IS_OPTION_MENU(option_menu));

	gpointer p = gtk_object_get_data( GTK_OBJECT(widget),
												WIDGET_MENU_VALUE_TAG);

	gtk_object_set_data( GTK_OBJECT(option_menu), WIDGET_MENU_VALUE_TAG, p );

	//TODO: This code is now shared between RulerUnits and DefaultPaperSize
	//so anyone who wants to resurect this msg. needs to add a conditional
	//UT_DEBUGMSG(("s_menu_item_activate [%d %s]\n", p, UT_dimensionName( (UT_Dimension)((UT_uint32)p)) ) );

	return TRUE;
}

/* static */ void AP_UnixDialog_Options::s_clist_clicked (GtkWidget *w, gint row, gint col, 
							  GdkEvent *evt, gpointer d)
{
  AP_UnixDialog_Options * dlg = static_cast <AP_UnixDialog_Options *>(d);
  dlg->event_clistClicked (row, col);
}
