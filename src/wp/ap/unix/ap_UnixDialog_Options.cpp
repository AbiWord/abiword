/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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

// for gtkclist and gtkcolorsel
#undef GTK_DISABLE_DEPRECATED

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xap_Toolbar_Layouts.h"

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
	return new AP_UnixDialog_Options(pFactory,id);
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

    // save for use with event
    m_pFrame = pFrame;

    // Populate the window's data items
    _populateWindowData();
    _initUnixOnlyPrefs();

    switch ( abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_OK, false ) )
      {
      default:
		  event_OK (); break;
      }

    abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_Options::event_clistClicked (int row, int col)
{
  GtkCList * clist = GTK_CLIST (m_toolbarClist);
  bool b = static_cast<bool>(GPOINTER_TO_INT(gtk_clist_get_row_data (clist, row)));

  gtk_object_set_user_data (GTK_OBJECT(m_checkbuttonViewShowTB), GINT_TO_POINTER(row));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonViewShowTB), (b ? TRUE : FALSE));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonViewHideTB), (b ? FALSE : TRUE));
}

///
/// All this color selection code is stolen from the ap_UnixDialog_Background
/// dialog
///
#define CTI(c, v) (unsigned char)(c[v] * 255.0)

/* static */ void AP_UnixDialog_Options::s_color_changed(GtkWidget * csel,
			    AP_UnixDialog_Options * dlg)
{
  UT_ASSERT(csel && dlg);

  char color[10];

  GtkColorSelection * w = GTK_COLOR_SELECTION(csel);

  gdouble cur [4];

  gtk_color_selection_get_color (w, cur);
  sprintf(color,"#%02x%02x%02x",CTI(cur, 0), CTI(cur, 1), CTI(cur, 2));
	
  strncpy(dlg->m_CurrentTransparentColor,static_cast<const XML_Char *>(color),9);
}

#undef CTI

void AP_UnixDialog_Options::event_ChooseTransparentColor(void)
{
//
// Run the Background dialog over the options? No the title is wrong.
//
  GtkWidget * dlg;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dlg = abiDialogNew("color chooser dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ColorChooserLabel).utf8_str());

  abiAddStockButton(GTK_DIALOG(dlg), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL);

  GtkWidget *colorsel;

  colorsel = gtk_color_selection_new();
  gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colorsel), false);
  gtk_widget_show (colorsel);
  gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dlg)->vbox), colorsel);
  UT_RGBColor c;
  UT_parseColor(m_CurrentTransparentColor,c);

  gdouble currentColor[4] = { 0, 0, 0, 0 };
  currentColor[0] = (static_cast<gdouble>(c.m_red) / static_cast<gdouble>(255.0));
  currentColor[1] = (static_cast<gdouble>(c.m_grn) / static_cast<gdouble>(255.0));
  currentColor[2] = (static_cast<gdouble>(c.m_blu) / static_cast<gdouble>(255.0));

  gtk_color_selection_set_color (GTK_COLOR_SELECTION(colorsel),
				 currentColor);

  g_signal_connect (G_OBJECT(colorsel), "color-changed",
		      G_CALLBACK(s_color_changed),
		      static_cast<gpointer>(this));

//
// Do all the nice stuff and put the color selector on top of our current
// dialog.
//
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(m_windowMain,dlg);

	// Show the top level dialog,
	gtk_widget_show(dlg);

	// Make it modal, and stick it up top
	gtk_grab_add(dlg);

	// run into the gtk main loop for this window
	gtk_dialog_run(GTK_DIALOG(dlg));

//
// Finish up here after a close or window delete signal.
//
	abiDestroyWidget(dlg);

}

void AP_UnixDialog_Options::event_AllowTransparentColor(void)
{
//
// If this button is up we do not allow transparent color
//
	if(!GTK_TOGGLE_BUTTON (m_checkbuttonTransparentIsWhite)->active)
	{
		strncpy(m_CurrentTransparentColor,static_cast<const XML_Char *>("ffffff"),9);
		gtk_widget_set_sensitive(m_pushbuttonNewTransparentColor,FALSE);
	}
	else
		gtk_widget_set_sensitive(m_pushbuttonNewTransparentColor,TRUE);
}

void AP_UnixDialog_Options::event_OK(void)
{
    m_answer = AP_Dialog_Options::a_OK;
    _storeWindowData () ;
}

void AP_UnixDialog_Options::event_Cancel(void)
{
    m_answer = AP_Dialog_Options::a_CANCEL;
}

void AP_UnixDialog_Options::event_Apply(void)
{
    m_answer = AP_Dialog_Options::a_APPLY;
    _storeWindowData () ;
}

/*****************************************************************/
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)			\
        do {							\
	        g_signal_connect(G_OBJECT(w), "activate",	\
                G_CALLBACK(s_menu_item_activate),		\
                static_cast<gpointer>(this));			\
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
	GtkWidget *label21;
	GtkWidget *vbox58;
	GtkWidget *enable_sq;
	GtkWidget *hbox58;
	GtkWidget *checkWhiteForTransparent;
	GtkWidget *pushChooseColorForTransparent;
	GtkWidget *checkAllowCustomToolbars;
	GtkWidget *checkEnableSmoothScrolling;
	GtkWidget *checkAutoLoadPlugins;
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
	GtkWidget *frame44;
	GtkWidget *check_splash;
	GtkWidget *vbox29;
	GtkWidget *label23;
	GtkWidget *hbox28;
	GtkWidget *label24;
	GtkWidget *autosave_ext;

	GtkWidget *frame42;
	GtkWidget *vbox59;
	GtkWidget *rtl_dominant;
	GtkWidget *save_context_glyphs;
	GtkWidget *hebrew_context_glyphs;

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

	frame13 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Toolbars).utf8_str());
	gtk_widget_show (frame13);
	gtk_box_pack_start (GTK_BOX (vbox17), frame13, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame13), GTK_SHADOW_NONE);

	toolbar_clist = gtk_clist_new (1);
	gtk_widget_show (toolbar_clist);
	gtk_container_add (GTK_CONTAINER (frame13), toolbar_clist);
	gtk_container_set_border_width (GTK_CONTAINER (toolbar_clist), 5);

	gtk_clist_column_titles_passive (GTK_CLIST (toolbar_clist));

	gtk_clist_freeze (GTK_CLIST (toolbar_clist));
	gtk_clist_clear  (GTK_CLIST (toolbar_clist));

	gtk_clist_set_column_title (GTK_CLIST (toolbar_clist), 1,
				    pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Toolbars).utf8_str());

	const gchar *data[2];
	data[1] = 0;

	const UT_Vector & vec = XAP_App::getApp()->getToolbarFactory()->getToolbarNames();
	UT_uint32 i;
	for (i = 0; i < vec.getItemCount(); i++) {
		data[0] = static_cast<const gchar *>(reinterpret_cast<const UT_UTF8String*>(vec.getNthItem(i))->utf8_str());
		gtk_clist_append (GTK_CLIST(toolbar_clist), const_cast<gchar **>(data));
	}

	gtk_clist_thaw (GTK_CLIST (toolbar_clist));

	g_signal_connect (G_OBJECT (toolbar_clist), "select_row",
			    G_CALLBACK (s_clist_clicked), static_cast<gpointer>(this));

#if 0
	g_signal_connect (G_OBJECT (toolbar_clist), "unselect_row",
			    G_CALLBACK (NULL), static_cast<gpointer>(this));
#endif

	vbox18 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox18);
	gtk_box_pack_start (GTK_BOX (hbox3), vbox18, TRUE, TRUE, 0);

	frame14 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Visible).utf8_str());
	gtk_widget_show (frame14);
	gtk_box_pack_start (GTK_BOX (vbox18), frame14, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame14), GTK_SHADOW_NONE);

	vbox19 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox19);
	gtk_container_add (GTK_CONTAINER (frame14), vbox19);
	gtk_container_set_border_width (GTK_CONTAINER (vbox19), 1);

	show_toolbar = gtk_radio_button_new_with_label (vbox19_group, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Show).utf8_str());
	vbox19_group = gtk_radio_button_group (GTK_RADIO_BUTTON (show_toolbar));
	gtk_widget_show (show_toolbar);
	gtk_box_pack_start (GTK_BOX (vbox19), show_toolbar, FALSE, FALSE, 0);

	g_signal_connect (G_OBJECT (show_toolbar), "toggled",
			    G_CALLBACK (s_radio_toggled), toolbar_clist);

	hide_toolbar = gtk_radio_button_new_with_label (vbox19_group, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Hide).utf8_str());
	vbox19_group = gtk_radio_button_group (GTK_RADIO_BUTTON (hide_toolbar));
	gtk_widget_show (hide_toolbar);
	gtk_box_pack_start (GTK_BOX (vbox19), hide_toolbar, FALSE, FALSE, 0);

	frame15 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Look).utf8_str());
	gtk_widget_show (frame15);
	gtk_box_pack_start (GTK_BOX (vbox18), frame15, TRUE, TRUE, 2);
	gtk_frame_set_shadow_type(GTK_FRAME(frame15), GTK_SHADOW_NONE);

	vbox20 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox20);
	gtk_container_add (GTK_CONTAINER (frame15), vbox20);
	gtk_container_set_border_width (GTK_CONTAINER (vbox20), 1);

	look_icon = gtk_radio_button_new_with_label (vbox20_group, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Icons).utf8_str());
	vbox20_group = gtk_radio_button_group (GTK_RADIO_BUTTON (look_icon));
	gtk_widget_show (look_icon);
	gtk_box_pack_start (GTK_BOX (vbox20), look_icon, FALSE, FALSE, 0);

	look_text = gtk_radio_button_new_with_label (vbox20_group, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Text).utf8_str());
	vbox20_group = gtk_radio_button_group (GTK_RADIO_BUTTON (look_text));
	gtk_widget_show (look_text);
	gtk_box_pack_start (GTK_BOX (vbox20), look_text, FALSE, FALSE, 0);

	look_both = gtk_radio_button_new_with_label (vbox20_group, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Both).utf8_str());
	vbox20_group = gtk_radio_button_group (GTK_RADIO_BUTTON (look_both));
	gtk_widget_show (look_both);
	gtk_box_pack_start (GTK_BOX (vbox20), look_both, FALSE, FALSE, 0);

	// TODO: make these next 4 controls editable && usable
	gtk_widget_set_sensitive (look_icon, FALSE);
	gtk_widget_set_sensitive (look_text, FALSE);
	gtk_widget_set_sensitive (look_both, FALSE);

	view_tooltips = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewTooltips).utf8_str());
	gtk_widget_show (view_tooltips);
	gtk_box_pack_start (GTK_BOX (vbox18), view_tooltips, FALSE, FALSE, 2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view_tooltips), TRUE);
	gtk_widget_set_sensitive (view_tooltips, FALSE);

	label4 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Toolbars).utf8_str());
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

	frame21 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_General).utf8_str());
	gtk_widget_show (frame21);
	gtk_box_pack_start (GTK_BOX (hbox9), frame21, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame21), GTK_SHADOW_NONE);

	vbox31 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox31);
	gtk_container_add (GTK_CONTAINER (frame21), vbox31);

	check_spell = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellCheckAsType).utf8_str());

	gtk_widget_show (check_spell);
	gtk_box_pack_start (GTK_BOX (vbox31), check_spell, FALSE, FALSE, 0);

	hide_errors = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellHideErrors).utf8_str());
	gtk_widget_show (hide_errors);
	gtk_box_pack_start (GTK_BOX (vbox31), hide_errors, FALSE, FALSE, 0);

	always_suggest = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellSuggest).utf8_str());
	gtk_widget_show (always_suggest);
	gtk_box_pack_start (GTK_BOX (vbox31), always_suggest, FALSE, FALSE, 0);

	suggest_from = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellMainOnly).utf8_str());
	gtk_widget_show (suggest_from);
	gtk_box_pack_start (GTK_BOX (vbox31), suggest_from, FALSE, FALSE, 0);

	frame22 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Ignore).utf8_str());
	gtk_widget_show (frame22);
	gtk_box_pack_start (GTK_BOX (hbox9), frame22, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame22), GTK_SHADOW_NONE);

	vbox32 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox32);
	gtk_container_add (GTK_CONTAINER (frame22), vbox32);

	ignore_upper = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellUppercase).utf8_str());
	gtk_widget_show (ignore_upper);
	gtk_box_pack_start (GTK_BOX (vbox32), ignore_upper, FALSE, FALSE, 0);

	ignore_nums = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellNumbers).utf8_str());
	gtk_widget_show (ignore_nums);
	gtk_box_pack_start (GTK_BOX (vbox32), ignore_nums, FALSE, FALSE, 0);

	ignore_inet = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellInternet).utf8_str());
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

	label7 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellIgnoredWord).utf8_str());
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
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_CustomDict).utf8_str());
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (custom_dict_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (custom_dict), custom_dict_menu);

	label8 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellCustomDict).utf8_str());
	gtk_widget_show (label8);
	gtk_table_attach (GTK_TABLE (table2), label8, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);

	button_dict = gtk_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_CustomDict).utf8_str());
	gtk_widget_show (button_dict);
	gtk_table_attach (GTK_TABLE (table2), button_dict, 2, 3, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	button_edit = gtk_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_IgnoreEdit).utf8_str());
	gtk_widget_show (button_edit);
	gtk_table_attach (GTK_TABLE (table2), button_edit, 2, 3, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	button_reset = gtk_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_IgnoreReset).utf8_str());
	gtk_widget_show (button_reset);
	gtk_table_attach (GTK_TABLE (table2), button_reset, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	gtk_widget_set_sensitive (button_reset, FALSE);
	gtk_widget_set_sensitive (button_edit, FALSE);
	gtk_widget_set_sensitive (button_dict, FALSE);
	gtk_widget_set_sensitive (custom_dict, FALSE);

	label2 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_Spelling).utf8_str());
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

	frame38 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewShowHide).utf8_str());
	gtk_widget_show (frame38);
	gtk_box_pack_start (GTK_BOX (hbox23), frame38, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame38), GTK_SHADOW_NONE);

	vbox55 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox55);
	gtk_container_add (GTK_CONTAINER (frame38), vbox55);

	show_ruler = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewRuler).utf8_str());
	gtk_widget_show (show_ruler);
	gtk_box_pack_start (GTK_BOX (vbox55), show_ruler, FALSE, FALSE, 0);

	show_statusbar = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewStatusBar).utf8_str());
	gtk_widget_show (show_statusbar);
	gtk_box_pack_start (GTK_BOX (vbox55), show_statusbar, FALSE, FALSE, 0);

	blink_cursor = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewCursorBlink).utf8_str());
	gtk_widget_show (blink_cursor);
	gtk_box_pack_start (GTK_BOX (vbox55), blink_cursor, FALSE, FALSE, 0);

	frame39 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewViewFrame).utf8_str());
	gtk_widget_show (frame39);
	gtk_box_pack_start (GTK_BOX (hbox23), frame39, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame39), GTK_SHADOW_NONE);

	vbox56 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox56);
	gtk_container_add (GTK_CONTAINER (frame39), vbox56);

	view_all = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewAll).utf8_str());
	gtk_widget_show (view_all);
	gtk_box_pack_start (GTK_BOX (vbox56), view_all, FALSE, FALSE, 0);

	view_hidden = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewHiddenText).utf8_str());
	gtk_widget_show (view_hidden);
	gtk_box_pack_start (GTK_BOX (vbox56), view_hidden, FALSE, FALSE, 0);

	view_invis = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewUnprintable).utf8_str());
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
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_inch).utf8_str());
 	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(ruler_units));
 	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_IN));
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	// cm
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_cm).utf8_str());
 	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(ruler_units));
 	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_CM));
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	// points
	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_points).utf8_str());
 	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(ruler_units));
  	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_PT));
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	// picas
  	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_pica).utf8_str());
  	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(ruler_units));
  	g_object_set_data(G_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(DIM_PI));
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
  	gtk_widget_show (glade_menuitem);
  	gtk_menu_append (GTK_MENU (ruler_units_menu), glade_menuitem);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (ruler_units), ruler_units_menu);

	label21 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewUnits).utf8_str());
	gtk_widget_show (label21);
	gtk_table_attach (GTK_TABLE (table4), label21, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label21), GTK_JUSTIFY_LEFT);

	vbox58 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox58);
	gtk_box_pack_start (GTK_BOX (vbox54), vbox58, TRUE, TRUE, 0);

	enable_sq = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SmartQuotesEnable).utf8_str());
	gtk_widget_show (enable_sq);
	gtk_box_pack_start (GTK_BOX (vbox58), enable_sq, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (enable_sq), 2);

	hbox58 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox58);
	gtk_box_pack_start(GTK_BOX(vbox58),hbox58, TRUE, TRUE, 0);

	checkWhiteForTransparent = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_CheckWhiteForTransparent).utf8_str());
	gtk_widget_show (checkWhiteForTransparent);
	gtk_box_pack_start (GTK_BOX (hbox58), checkWhiteForTransparent, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (checkWhiteForTransparent), 2);

	pushChooseColorForTransparent = gtk_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ChooseForTransparent).utf8_str());
	gtk_widget_show (pushChooseColorForTransparent);
	gtk_box_pack_start (GTK_BOX (hbox58), pushChooseColorForTransparent, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (pushChooseColorForTransparent), 2);
//
// Custom toolbars.
//
	checkAllowCustomToolbars = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_CheckAllowCustomToolbars).utf8_str());
	gtk_widget_show (checkAllowCustomToolbars);
	gtk_box_pack_start (GTK_BOX (vbox58), checkAllowCustomToolbars, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (checkAllowCustomToolbars), 2);
//
// Smooth Scrolling
//
	checkEnableSmoothScrolling = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_CheckEnableSmoothScrolling).utf8_str());
	gtk_widget_show (checkEnableSmoothScrolling);
	gtk_box_pack_start (GTK_BOX (vbox58), checkEnableSmoothScrolling, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (checkEnableSmoothScrolling), 2);



	label3 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Layout).utf8_str());
	gtk_widget_show (label3);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label3);


	// PREFERENCES / MISC TAB


	vbox36 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox36);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox36);

	frame40 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Schemes).utf8_str());
	gtk_widget_show (frame40);
	gtk_box_pack_start (GTK_BOX (vbox36), frame40, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame40), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(frame40), GTK_SHADOW_NONE);

	vbox57 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox57);
	gtk_container_add (GTK_CONTAINER (frame40), vbox57);
	gtk_container_set_border_width (GTK_CONTAINER (vbox57), 4);

	save_scheme = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_PrefsAutoSave).utf8_str());
	gtk_widget_show (save_scheme);
	gtk_box_pack_start (GTK_BOX (vbox57), save_scheme, FALSE, FALSE, 0);

	hbox25 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox25);
	gtk_box_pack_start (GTK_BOX (vbox57), hbox25, TRUE, TRUE, 0);

	label17 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_PrefsCurrentScheme).utf8_str());
	gtk_widget_show (label17);
	gtk_box_pack_start (GTK_BOX (hbox25), label17, FALSE, TRUE, 3);

	current_scheme = gtk_combo_new ();
	gtk_widget_show (current_scheme);
	gtk_box_pack_start (GTK_BOX (hbox25), current_scheme, TRUE, TRUE, 0);
	//gtk_entry_set_editable (GTK_ENTRY (current_scheme), FALSE);

	frame42 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_BiDiOptions).utf8_str());
	gtk_widget_show (frame42);
	gtk_box_pack_start (GTK_BOX (vbox36), frame42, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame42), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(frame42), GTK_SHADOW_NONE);

	vbox59 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox59);
	gtk_container_add (GTK_CONTAINER (frame42), vbox59);

	rtl_dominant = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_DirectionRtl).utf8_str());
	gtk_widget_show (rtl_dominant);
	gtk_box_pack_start (GTK_BOX (vbox59), rtl_dominant, FALSE, FALSE, 0);

	// this is not implemented at the moment
	save_context_glyphs = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SaveContextGlyphs).utf8_str());
#if 0
	// currently not implemented
	gtk_widget_show (save_context_glyphs);
#endif
	gtk_box_pack_start (GTK_BOX (vbox59), save_context_glyphs, FALSE, FALSE, 0);

	hebrew_context_glyphs = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_HebrewContextGlyphs).utf8_str());
	gtk_widget_show (hebrew_context_glyphs);
	gtk_box_pack_start (GTK_BOX (vbox59), hebrew_context_glyphs, FALSE, FALSE, 0);

	// AUTO SAVE
	frame43 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_AutoSave).utf8_str());
	gtk_widget_show (frame43);
	gtk_box_pack_start (GTK_BOX (vbox36), frame43, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame43), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(frame43), GTK_SHADOW_NONE);

	hbox26 = gtk_hbox_new (FALSE, 14);
	gtk_widget_show (hbox26);
	gtk_container_add (GTK_CONTAINER (frame43), hbox26);

	hbox27 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox27);
	gtk_box_pack_start (GTK_BOX (hbox26), hbox27, TRUE, TRUE, 0);

	autosave_cb = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_AutoSaveCurrent).utf8_str());
	gtk_widget_show (autosave_cb);
	gtk_box_pack_start (GTK_BOX (hbox27), autosave_cb, FALSE, FALSE, 0);

	autosave_time_adj = gtk_adjustment_new (5, 1, 120, 1, 5, 5);
	autosave_time = gtk_spin_button_new (GTK_ADJUSTMENT (autosave_time_adj), 1, 0);
	gtk_widget_show (autosave_time);
	gtk_box_pack_start (GTK_BOX (hbox27), autosave_time, FALSE, TRUE, 0);

	label23 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Minutes).utf8_str());
	gtk_widget_show (label23);
	gtk_box_pack_start (GTK_BOX (hbox27), label23, FALSE, FALSE, 0);

	hbox28 = gtk_hbox_new (TRUE, 4);
	gtk_widget_show (hbox28);
	gtk_box_pack_start (GTK_BOX (hbox26), hbox28, TRUE, FALSE, 0);

	label24 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_WithExtension).utf8_str());
	gtk_widget_show (label24);
	gtk_box_pack_start (GTK_BOX (hbox28), label24, FALSE, FALSE, 0);

	autosave_ext = gtk_entry_new_with_max_length (5);
	gtk_widget_show (autosave_ext);
	gtk_box_pack_start (GTK_BOX (hbox28), autosave_ext, TRUE, TRUE, 0);
	gtk_widget_set_size_request (autosave_ext, 50, -1);

	frame44 = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_Misc).utf8_str());
	gtk_widget_show (frame44);
	gtk_box_pack_start (GTK_BOX (vbox36), frame44, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame44), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(frame44), GTK_SHADOW_NONE);

	vbox29 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox29);
	gtk_container_add (GTK_CONTAINER (frame44), vbox29);

	check_splash = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ShowSplash).utf8_str());
	gtk_widget_show (check_splash);
	gtk_box_pack_start (GTK_BOX (vbox29), check_splash, FALSE, FALSE, 0);

//
// Auto Load Plugins.
//
	checkAutoLoadPlugins = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_CheckAutoLoadPlugins).utf8_str());
	gtk_widget_show (checkAutoLoadPlugins);
	gtk_box_pack_start (GTK_BOX (vbox29), checkAutoLoadPlugins, TRUE, TRUE, 0);

	label10 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_Preferences).utf8_str());
	gtk_widget_show (label10);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label10);


	//////////////////////////////////////////////////////////////////

	m_notebook = notebook1;
	m_checkbuttonOtherDirectionRtl = rtl_dominant;
	m_checkbuttonOtherSaveContextGlyphs = save_context_glyphs;
	m_checkbuttonOtherHebrewContextGlyphs = hebrew_context_glyphs;
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

	m_checkbuttonTransparentIsWhite = checkWhiteForTransparent;
	m_pushbuttonNewTransparentColor = pushChooseColorForTransparent;

	m_checkbuttonAllowCustomToolbars      = checkAllowCustomToolbars;
//
// Have to reset the toolbar is the result of a toggle is to turn off 
// custom toolabrs
//
	g_signal_connect(G_OBJECT(m_checkbuttonAllowCustomToolbars),
			   "toggled",
			   G_CALLBACK(s_toolbars_toggled),
			   static_cast<gpointer>(this));

	m_checkbuttonEnableSmoothScrolling      = checkEnableSmoothScrolling;
	m_checkbuttonAutoLoadPlugins      = checkAutoLoadPlugins;

	m_checkbuttonSmartQuotesEnable	        = enable_sq;

	m_checkbuttonPrefsAutoSave		= save_scheme;
	m_comboPrefsScheme			= current_scheme;

	m_checkbuttonShowSplash = check_splash;

	m_checkbuttonViewShowRuler		= show_ruler;
	m_listViewRulerUnits			= ruler_units;
	m_checkbuttonViewCursorBlink	= blink_cursor;

	m_checkbuttonViewShowStatusBar	= show_statusbar;
	m_checkbuttonViewAll			= view_all;
	m_checkbuttonViewHiddenText		= view_hidden;
	m_checkbuttonViewUnprintable	= view_invis;

	// TODO: rulers
	m_checkbuttonViewShowTB	        = show_toolbar;
	m_checkbuttonViewHideTB         = hide_toolbar;
	m_toolbarClist = toolbar_clist;

	// to enable/disable other controls (hide errors)
	g_signal_connect(G_OBJECT(m_checkbuttonSpellCheckAsType),
			   "toggled",
			   G_CALLBACK(s_checkbutton_toggle),
			   static_cast<gpointer>(this));

	// to enable/disable other screen colors from white
	g_signal_connect(G_OBJECT( m_checkbuttonTransparentIsWhite),
			   "toggled",
			   G_CALLBACK(s_allowTransparentColor),
			   static_cast<gpointer>(this));


	// to choose another color for the screen
	g_signal_connect(G_OBJECT( m_pushbuttonNewTransparentColor ),
			   "clicked",
			   G_CALLBACK(s_chooseTransparentColor),
			   static_cast<gpointer>(this));

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
	GtkWidget * buttonApply = 0;
	GtkWidget * buttonOk;
	GtkWidget * buttonCancel = 0;
	GtkWidget * buttonDefaults;

	mainWindow = abiDialogNew("options dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_Options_OptionsTitle).utf8_str());

#if 0
	buttonCancel = abiAddStockButton(GTK_DIALOG(mainWindow), GTK_STOCK_CANCEL, BUTTON_CANCEL);
#endif

	buttonDefaults = gtk_button_new_from_stock (GTK_STOCK_REVERT_TO_SAVED);
	gtk_widget_show (buttonDefaults);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(mainWindow)->action_area), buttonDefaults);
	GTK_WIDGET_SET_FLAGS (buttonDefaults, GTK_CAN_DEFAULT);

	//////////////////////////////////////////////////////////////////////
	// the control buttons
	g_signal_connect(G_OBJECT(buttonDefaults),
			   "clicked",
			   G_CALLBACK(s_defaults_clicked),
			   static_cast<gpointer>(this));

#if 0
	buttonApply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_show (buttonApply);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(mainWindow)->action_area), buttonApply);
	GTK_WIDGET_SET_FLAGS (buttonApply, GTK_CAN_DEFAULT);

	g_signal_connect(G_OBJECT(buttonApply),
			   "clicked",
			   G_CALLBACK(s_apply_clicked),
			   static_cast<gpointer>(this));
#endif

	buttonOk = abiAddStockButton(GTK_DIALOG(mainWindow), GTK_STOCK_CLOSE, GTK_RESPONSE_OK);

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
		GtkWidget *w = _lookupWidget( static_cast<tControl>(i) );
		if (!(w && GTK_IS_WIDGET (w)))
		  continue;

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		UT_ASSERT( g_object_get_data(G_OBJECT(w), "tControl" ) == NULL);

		g_object_set_data( G_OBJECT(w), "tControl", reinterpret_cast<gpointer>(i) );
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

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// other
	case id_CHECK_SMART_QUOTES_ENABLE:
		return m_checkbuttonSmartQuotesEnable;

	case id_SHOWSPLASH:
		return m_checkbuttonShowSplash;

	case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
		return m_checkbuttonOtherDirectionRtl;

	case id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS:
		return m_checkbuttonOtherSaveContextGlyphs;

	case id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS:
		return m_checkbuttonOtherHebrewContextGlyphs;

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

	case id_CHECK_ALLOW_CUSTOM_TOOLBARS:
		return m_checkbuttonAllowCustomToolbars;

	case id_CHECK_ENABLE_SMOOTH_SCROLLING:
		return m_checkbuttonEnableSmoothScrolling;

	case id_CHECK_AUTO_LOAD_PLUGINS:
		return m_checkbuttonAutoLoadPlugins;

	case id_CHECK_COLOR_FOR_TRANSPARENT_IS_WHITE:
		return  m_checkbuttonTransparentIsWhite;

	case id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
		return  m_pushbuttonNewTransparentColor;

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


bool AP_UnixDialog_Options::_gatherViewShowToolbar(UT_uint32 row)
{
	UT_ASSERT (m_toolbarClist);
	bool b = static_cast<bool>(GPOINTER_TO_INT (gtk_clist_get_row_data (GTK_CLIST (m_toolbarClist), row)));
	xxx_UT_DEBUGMSG(("DOM: _gather %d %d\n", row, b));
	return b;
}

void AP_UnixDialog_Options::_setViewShowToolbar(UT_uint32 row, bool b)
{
	UT_ASSERT (m_toolbarClist);
	xxx_UT_DEBUGMSG(("DOM: _set %d %d\n", row, b));
	gtk_clist_set_row_data (GTK_CLIST (m_toolbarClist), row, GINT_TO_POINTER(b));
}


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

DEFINE_GET_SET_BOOL(OtherDirectionRtl);
DEFINE_GET_SET_BOOL(OtherSaveContextGlyphs);
DEFINE_GET_SET_BOOL(OtherHebrewContextGlyphs);

DEFINE_GET_SET_BOOL(AutoSaveFile);
DEFINE_GET_SET_BOOL(ShowSplash);
DEFINE_GET_SET_BOOL(PrefsAutoSave);
DEFINE_GET_SET_BOOL(ViewShowRuler);
DEFINE_GET_SET_BOOL(ViewShowStatusBar);

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
	char nb[12];
	int val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod));
	g_snprintf(nb, 12, "%d", val);
	stRetVal = nb;
}

void AP_UnixDialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_EDITABLE(m_textAutoSaveFilePeriod));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod), atoi(stPeriod.c_str()));
}

UT_Dimension AP_UnixDialog_Options::_gatherViewRulerUnits(void)
{
	UT_ASSERT(m_listViewRulerUnits && GTK_IS_OPTION_MENU(m_listViewRulerUnits));
	return (UT_Dimension)(GPOINTER_TO_INT(g_object_get_data( G_OBJECT(m_listViewRulerUnits), WIDGET_MENU_VALUE_TAG )));
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
	search_data *value = static_cast<search_data *>(_value);

	if ( !GTK_IS_MENU_ITEM(widget))
		return;

	value->index++;

	gint v = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(widget), value->key ));
	if ( v == GPOINTER_TO_INT(value->data) )
	{
		// UT_DEBUGMSG(("search_for_value [%d]", static_cast<gint>(value->data) ));
		value->found = value->index;
	}
}

// returns -1 if not found
static int option_menu_set_by_key ( GtkWidget *option_menu, gpointer value, gchar *key )
{
	UT_ASSERT( option_menu && key && GTK_IS_OPTION_MENU(option_menu));

	// at least make sure the value will be restored by the _gather
	g_object_set_data( G_OBJECT(option_menu), key, value);

	// lookup for the key with the value of dim
	search_data data = { -1, -1, key, value };

	GtkWidget *menu = gtk_option_menu_get_menu( GTK_OPTION_MENU(option_menu));
	UT_ASSERT(menu&&GTK_IS_MENU(menu));

	// iterate through all the values
	gtk_container_forall ( GTK_CONTAINER(menu), search_for_value, static_cast<gpointer>(&data) );

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void    AP_UnixDialog_Options::_setViewRulerUnits(UT_Dimension dim)
{
	UT_ASSERT(m_listViewRulerUnits && GTK_IS_OPTION_MENU(m_listViewRulerUnits));

	int r = option_menu_set_by_key ( m_listViewRulerUnits, reinterpret_cast<gpointer>(dim), WIDGET_MENU_VALUE_TAG );
	
	if (r < 0)
		UT_DEBUGMSG(("option_menu_set_by_key failed\n"));
}

DEFINE_GET_SET_BOOL	(ViewCursorBlink);

DEFINE_GET_SET_BOOL	(ViewAll);
DEFINE_GET_SET_BOOL	(ViewHiddenText);
DEFINE_GET_SET_BOOL	(ViewUnprintable);
DEFINE_GET_SET_BOOL (AllowCustomToolbars);
DEFINE_GET_SET_BOOL (EnableSmoothScrolling);
DEFINE_GET_SET_BOOL (AutoLoadPlugins);

#undef DEFINE_GET_SET_BOOL

int AP_UnixDialog_Options::_gatherNotebookPageNum(void)
{
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook));
	return gtk_notebook_get_current_page( GTK_NOTEBOOK(m_notebook) );
}

void    AP_UnixDialog_Options::_setNotebookPageNum(int pn)
{
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook));
	gtk_notebook_set_current_page( GTK_NOTEBOOK(m_notebook), pn );
}

/*****************************************************************/
//
// Reset custom toolbars
//
/* static */ void AP_UnixDialog_Options::s_toolbars_toggled(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	if(dlg->isInitialPopulationHappenning())
	{
		return;
	}
//
// Save the new toolbar state
//
	dlg->event_OK();
//
// If the toolbar preference is now off, reset to default.
//
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
	{
		XAP_App * pApp = XAP_App::getApp();
		pApp->resetToolbarsToDefault();
	}
}

/*static*/ void AP_UnixDialog_Options::s_apply_clicked(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	dlg->event_Apply();
}

/*static*/ void AP_UnixDialog_Options::s_defaults_clicked( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	dlg->_event_SetDefaults();

#if 0
	// repopulate controls
	dlg->_populateWindowData();
	dlg->_initUnixOnlyPrefs();
#endif
}


/*static*/ void AP_UnixDialog_Options::s_chooseTransparentColor( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	dlg->event_ChooseTransparentColor();
}


/*static*/ void AP_UnixDialog_Options::s_allowTransparentColor( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	dlg->event_AllowTransparentColor();
}


// these function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
/*static*/ void AP_UnixDialog_Options::s_checkbutton_toggle( GtkWidget *w, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(dlg);
	UT_ASSERT(w && GTK_IS_WIDGET(w));

	int i = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(w), "tControl" ));
	UT_DEBUGMSG(("s_checkbutton_toggle: control id = %d\n", i));
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
}

/*static*/ gint AP_UnixDialog_Options::s_menu_item_activate(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);

	UT_ASSERT(widget && dlg);

	GtkWidget *option_menu = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(widget),
												 WIDGET_MENU_OPTION_PTR));
	UT_ASSERT( option_menu && GTK_IS_OPTION_MENU(option_menu));

	gpointer p = g_object_get_data( G_OBJECT(widget),
												WIDGET_MENU_VALUE_TAG);

	g_object_set_data( G_OBJECT(option_menu), WIDGET_MENU_VALUE_TAG, p );

	//TODO: This code is now shared between RulerUnits and DefaultPaperSize
	//so anyone who wants to resurect this msg. needs to add a conditional
	//UT_DEBUGMSG(("s_menu_item_activate [%d %s]\n", p, UT_dimensionName( (UT_Dimension)(reinterpret_cast<UT_uint32>(p))) ) );

	return TRUE;
}

/* static */ void AP_UnixDialog_Options::s_clist_clicked (GtkWidget *w, gint row, gint col,
							  GdkEvent *evt, gpointer d)
{
  AP_UnixDialog_Options * dlg = static_cast <AP_UnixDialog_Options *>(d);
  dlg->event_clistClicked (row, col);
}

void AP_UnixDialog_Options::_initUnixOnlyPrefs()
{
	if(UT_strcmp(m_CurrentTransparentColor,"ffffff") == 0)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonTransparentIsWhite), FALSE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonTransparentIsWhite), TRUE);
	}
}

void AP_UnixDialog_Options::_saveUnixOnlyPrefs()
{
}

void AP_UnixDialog_Options::_storeWindowData(void)
{
	_saveUnixOnlyPrefs();
	AP_Dialog_Options::_storeWindowData();
}
