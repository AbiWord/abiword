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
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "gr_UnixGraphics.h"
#include "fl_BlockLayout.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Tab.h"

/*****************************************************************/

#define WIDGET_MENU_OPTION_PTR		"menuoptionptr"
#define WIDGET_MENU_VALUE_TAG		"value"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_UnixDialog_Tab * p = new AP_UnixDialog_Tab(pFactory,id);
    return p;
}

AP_UnixDialog_Tab::AP_UnixDialog_Tab(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_Dialog_Tab(pDlgFactory,id)
{
	m_current_alignment = FL_TAB_LEFT;
	m_current_leader	= FL_LEADER_NONE;
	m_bInSetCall		= false;

}

AP_UnixDialog_Tab::~AP_UnixDialog_Tab(void)
{

}

/*****************************************************************/

void AP_UnixDialog_Tab::runModal(XAP_Frame * pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_return_if_fail(mainWindow);

    // save for use with event
    m_pFrame = pFrame;

    _populateWindowData();

    // Populate the window's data items

    switch ( abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false) )
      {
      case BUTTON_OK:
	event_OK () ; break ;
      default:
	event_Cancel(); break ;
      }

    abiDestroyWidget(mainWindow);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// unix specific handlers 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_UnixDialog_Tab::event_OK(void)
{
    m_answer = AP_Dialog_Tab::a_OK;
    _storeWindowData();
}

void AP_UnixDialog_Tab::event_Cancel(void)
{
    m_answer = AP_Dialog_Tab::a_CANCEL;
}

/*****************************************************************/
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        g_signal_connect(G_OBJECT(w), "activate",	\
                G_CALLBACK(s_menu_item_activate),		\
                (gpointer) this);							\
        } while (0)

GtkWidget* AP_UnixDialog_Tab::_constructWindow (void )
{
	
	GtkWidget *windowTabs;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowTabs = abiDialogNew("tab dialog", TRUE, pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_TabTitle).c_str());
	g_object_set_data (G_OBJECT (windowTabs), "windowTabs", windowTabs);

	_constructWindowContents(windowTabs);

	// create the accelerators from &'s
	createLabelAccelerators(windowTabs);

	gtk_window_add_accel_group (GTK_WINDOW (windowTabs), accel_group);

	return windowTabs;
}

void    AP_UnixDialog_Tab::_constructGnomeButtons( GtkWidget * windowTabs)
{
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	buttonCancel = abiAddStockButton(GTK_DIALOG(windowTabs), GTK_STOCK_CANCEL, BUTTON_CANCEL);
	buttonOK = abiAddStockButton(GTK_DIALOG(windowTabs), GTK_STOCK_OK, BUTTON_OK);

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
}

void    AP_UnixDialog_Tab::_constructWindowContents( GtkWidget * windowTabs )
{
   //////////////////////////////////////////////////////////////////////
	// BEGIN: glade stuff

	GtkWidget *table13;
	GtkWidget *hbuttonbox4;
	GtkWidget *buttonSet;
	GtkWidget *buttonClear;
	GtkWidget *buttonClearAll;
	GtkWidget *hbuttonbox3;
	GtkWidget *hbox10;
	GtkWidget *label8;
	GtkWidget *hbox11;
	GtkWidget *vbox4;
	GtkWidget *hbox15;
	GtkWidget *label13;
	GtkWidget *entryTabEntry;
	GtkWidget *hbox16;
	GtkWidget *label14;
	GtkWidget *frame3;
	GtkWidget *listTabs;
	GtkWidget *table14;
	GtkWidget *hbox13;
	GtkWidget *label10;
	GtkWidget *hseparator6;
	GtkWidget *hbox14;
	GtkWidget *label11;
	GtkWidget *hseparator7;
	GSList *group_align_group = NULL;
	GtkWidget *radiobuttonDecimal;
	GtkWidget *radiobuttonLeft;
	GtkWidget *radiobuttonCenter;
	GtkWidget *radiobuttonRight;
	GtkWidget *radiobuttonBar;
	GSList *group_leader_group = NULL;
	GtkWidget *radiobuttonLeaderDash;
	GtkWidget *radiobuttonLeaderDot;
	GtkWidget *radiobuttonLeaderNone;
	GtkWidget *radiobuttonLeaderUnderline;
	GtkWidget *hbox12;
	GtkWidget *label9;
	GtkObject *spinbuttonTabstop_adj;
	GtkWidget *spinbuttonTabstop;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	table13 = gtk_table_new (5, 1, FALSE);
	gtk_widget_show (table13);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(windowTabs)->vbox), table13);

	hbuttonbox4 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox4);
	gtk_table_attach (GTK_TABLE (table13), hbuttonbox4, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 6, 6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox4), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox4), 9);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonbox4), 0, 0);

	buttonSet = gtk_button_new_with_label(pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Button_Set).c_str());
	gtk_widget_show (buttonSet);
	gtk_container_add (GTK_CONTAINER (hbuttonbox4), buttonSet);
	GTK_WIDGET_SET_FLAGS (buttonSet, GTK_CAN_DEFAULT);

	buttonClear = gtk_button_new_from_stock(GTK_STOCK_CLEAR);//gtk_button_new_with_label (pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Button_Clear));
	gtk_widget_show (buttonClear);
	gtk_container_add (GTK_CONTAINER (hbuttonbox4), buttonClear);
	GTK_WIDGET_SET_FLAGS (buttonClear, GTK_CAN_DEFAULT);

	buttonClearAll = gtk_button_new_with_label(pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Button_ClearAll).c_str());
	gtk_widget_show (buttonClearAll);
	gtk_container_add (GTK_CONTAINER (hbuttonbox4), buttonClearAll);
	GTK_WIDGET_SET_FLAGS (buttonClearAll, GTK_CAN_DEFAULT);

	hbuttonbox3 = gtk_hbutton_box_new ();
	m_GnomeButtons = hbuttonbox3;
	gtk_widget_show (hbuttonbox3);
	gtk_table_attach (GTK_TABLE (table13), hbuttonbox3, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox3), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox3), 5);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonbox3), 0, 0);

	//
	// Construct the buttons to be gnomified
	//
        m_wTable = table13;
	_constructGnomeButtons(windowTabs);


	hbox10 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox10);
	gtk_table_attach (GTK_TABLE (table13), hbox10, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);

	label8 = gtk_label_new (pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Label_TabToClear).c_str());
	gtk_widget_show (label8);
	gtk_box_pack_start (GTK_BOX (hbox10), label8, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label8), 10, 0);

	hbox11 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox11);
	gtk_table_attach (GTK_TABLE (table13), hbox11, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox4);
	gtk_box_pack_start (GTK_BOX (hbox11), vbox4, TRUE, TRUE, 5);

	hbox15 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox15);
	gtk_box_pack_start (GTK_BOX (vbox4), hbox15, FALSE, FALSE, 5);

	label13 = gtk_label_new (pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Label_TabPosition).c_str());
	gtk_widget_show (label13);
	gtk_box_pack_start (GTK_BOX (hbox15), label13, FALSE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);

	entryTabEntry = gtk_entry_new ();
	gtk_widget_show (entryTabEntry);
	gtk_box_pack_start (GTK_BOX (vbox4), entryTabEntry, FALSE, FALSE, 1);

	hbox16 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox16);
	gtk_box_pack_start (GTK_BOX (vbox4), hbox16, TRUE, TRUE, 1);

	label14 = gtk_label_new ("     ");
	gtk_widget_show (label14);
	gtk_box_pack_start (GTK_BOX (hbox16), label14, FALSE, TRUE, 0);

	frame3 = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame3), GTK_SHADOW_NONE);
	gtk_widget_show (frame3);
	gtk_box_pack_start (GTK_BOX (hbox16), frame3, TRUE, TRUE, 0);

	listTabs = gtk_list_new ();
	gtk_widget_show (listTabs);
	gtk_container_add (GTK_CONTAINER (frame3), listTabs);

	table14 = gtk_table_new (8, 2, FALSE);
	gtk_widget_show (table14);
	gtk_box_pack_start (GTK_BOX (hbox11), table14, TRUE, TRUE, 0);

	hbox13 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox13);
	gtk_table_attach (GTK_TABLE (table14), hbox13, 0, 2, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	label10 = gtk_label_new (pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Label_Alignment).c_str());
	gtk_widget_show (label10);
	gtk_box_pack_start (GTK_BOX (hbox13), label10, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label10), 5, 0);

	hseparator6 = gtk_hseparator_new ();
	gtk_widget_show (hseparator6);
	gtk_box_pack_start (GTK_BOX (hbox13), hseparator6, TRUE, TRUE, 0);

	hbox14 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox14);
	gtk_table_attach (GTK_TABLE (table14), hbox14, 0, 2, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	label11 = gtk_label_new (pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Label_Leader).c_str());
	gtk_widget_show (label11);
	gtk_box_pack_start (GTK_BOX (hbox14), label11, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label11), 5, 0);

	hseparator7 = gtk_hseparator_new ();
	gtk_widget_show (hseparator7);
	gtk_box_pack_start (GTK_BOX (hbox14), hseparator7, TRUE, TRUE, 0);

	radiobuttonDecimal = gtk_radio_button_new_with_label (group_align_group, 
			pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Decimal).c_str());
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonDecimal));
	gtk_widget_show (radiobuttonDecimal);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonDecimal, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	radiobuttonLeft = gtk_radio_button_new_with_label (group_align_group, 
					pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Left).c_str());
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeft));
	gtk_widget_show (radiobuttonLeft);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeft, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobuttonLeft), TRUE);

	radiobuttonCenter = gtk_radio_button_new_with_label (group_align_group, 
						pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Center).c_str());
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonCenter));
	gtk_widget_show (radiobuttonCenter);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonCenter, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);

	radiobuttonRight = gtk_radio_button_new_with_label (group_align_group, 
				pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Right).c_str());
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonRight));
	gtk_widget_show (radiobuttonRight);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonRight, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);

	radiobuttonBar = gtk_radio_button_new_with_label (group_align_group, 
					pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Bar).c_str());
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonBar));
	gtk_widget_show (radiobuttonBar);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonBar, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	radiobuttonLeaderDash = gtk_radio_button_new_with_label (group_leader_group, 
				pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Dash).c_str());
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderDash));
	gtk_widget_show (radiobuttonLeaderDash);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderDash, 1, 2, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	radiobuttonLeaderDot = gtk_radio_button_new_with_label (group_leader_group, 
				pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Dot).c_str());
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderDot));
	gtk_widget_show (radiobuttonLeaderDot);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderDot, 0, 1, 7, 8,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);

	radiobuttonLeaderNone = gtk_radio_button_new_with_label (group_leader_group, 
			pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_None).c_str());
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderNone));
	gtk_widget_show (radiobuttonLeaderNone);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderNone, 0, 1, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobuttonLeaderNone), TRUE);

	radiobuttonLeaderUnderline = gtk_radio_button_new_with_label (group_leader_group, 
					pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Radio_Underline).c_str());
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderUnderline));
	gtk_widget_show (radiobuttonLeaderUnderline);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderUnderline, 1, 2, 7, 8,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	hbox12 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox12);
	gtk_table_attach (GTK_TABLE (table14), hbox12, 0, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 5, 5);

	label9 = gtk_label_new (pSS->getValueUTF8( AP_STRING_ID_DLG_Tab_Label_DefaultTS).c_str());
	gtk_widget_show (label9);
	gtk_box_pack_start (GTK_BOX (hbox12), label9, FALSE, FALSE, 1);

	spinbuttonTabstop_adj = gtk_adjustment_new (1, -1000, 1000, 3, 3, 10);
        spinbuttonTabstop = gtk_entry_new();
	gtk_widget_show (spinbuttonTabstop);
	gtk_box_pack_start (GTK_BOX (hbox12), spinbuttonTabstop, TRUE, TRUE, 0);
	gtk_entry_set_editable( GTK_ENTRY( spinbuttonTabstop),FALSE);
	GtkWidget * spinbuttonTabstop_dum = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonTabstop_adj), 1, 0);
	gtk_widget_show (spinbuttonTabstop_dum);
	gtk_widget_set_usize(spinbuttonTabstop_dum,10,-2);
	gtk_box_pack_start (GTK_BOX (hbox12), spinbuttonTabstop_dum, FALSE,FALSE, 0);


    //////////////////////////////////////////////////////////////////////
	// END: glade stuff

    g_signal_connect(G_OBJECT(buttonSet),
                       "clicked",
                       G_CALLBACK(s_set_clicked),
                       (gpointer) this);

    g_signal_connect(G_OBJECT(buttonClear),
                       "clicked",
                       G_CALLBACK(s_clear_clicked),
                       (gpointer) this);

    g_signal_connect(G_OBJECT(buttonClearAll),
                       "clicked",
                       G_CALLBACK(s_clear_all_clicked),
                       (gpointer) this);

    g_signal_connect(G_OBJECT(entryTabEntry),
                       "changed",
                       G_CALLBACK(s_edit_change),
                       (gpointer) this);

    g_signal_connect(spinbuttonTabstop_adj,
                       "value_changed",
                       G_CALLBACK(s_spin_default_changed),
                       (gpointer) this);

    // Update member variables with the important widgets that
    // might need to be queried or altered later.


        m_iDefaultSpin =  (UT_sint32) GTK_ADJUSTMENT(spinbuttonTabstop_adj)->value;
        m_oDefaultSpin_adj = spinbuttonTabstop_adj;
	m_Widgets.setNthItem( id_EDIT_TAB,				entryTabEntry,		NULL);
	m_Widgets.setNthItem( id_LIST_TAB,				listTabs,			NULL);
	m_Widgets.setNthItem( id_SPIN_DEFAULT_TAB_STOP,	spinbuttonTabstop,	NULL);

	m_Widgets.setNthItem( id_ALIGN_LEFT,			radiobuttonLeft,	NULL);
	m_Widgets.setNthItem( id_ALIGN_CENTER,			radiobuttonCenter,	NULL);
	m_Widgets.setNthItem( id_ALIGN_RIGHT,			radiobuttonRight,	NULL);
	m_Widgets.setNthItem( id_ALIGN_DECIMAL,			radiobuttonDecimal,	NULL);
	m_Widgets.setNthItem( id_ALIGN_BAR,				radiobuttonBar,		NULL);

	m_Widgets.setNthItem( id_LEADER_NONE,			radiobuttonLeaderNone,		NULL);
	m_Widgets.setNthItem( id_LEADER_DOT,			radiobuttonLeaderDot,		NULL);
	m_Widgets.setNthItem( id_LEADER_DASH,			radiobuttonLeaderDash,		NULL);
	m_Widgets.setNthItem( id_LEADER_UNDERLINE,		radiobuttonLeaderUnderline,	NULL);

	m_Widgets.setNthItem( id_BUTTON_SET,			buttonSet,					NULL);
	m_Widgets.setNthItem( id_BUTTON_CLEAR,			buttonClear,				NULL);
	m_Widgets.setNthItem( id_BUTTON_CLEAR_ALL,		buttonClearAll,				NULL);

	m_Widgets.setNthItem( id_BUTTON_OK,			m_buttonOK,					NULL);
	m_Widgets.setNthItem( id_BUTTON_CANCEL,			m_buttonCancel,				NULL);

	// some lists of signals to set
	tControl id;
	for ( id = id_ALIGN_LEFT; id <= id_ALIGN_BAR; id = (tControl)((UT_uint32)id + 1))
	{
		GtkWidget *w = _lookupWidget(id);
		g_signal_connect(G_OBJECT(w),
						   "toggled",
						   G_CALLBACK(s_alignment_change),
						   (gpointer) this);

		// set the "userdata" to be the tALignment
		gtk_object_set_user_data( GTK_OBJECT(w), (gpointer)((UT_uint32)id - (UT_uint32)id_ALIGN_LEFT + (UT_uint32)FL_TAB_LEFT));
	}

	for ( id = id_LEADER_NONE; id <= id_LEADER_UNDERLINE; id = (tControl)((UT_uint32)id + 1))
	{
		GtkWidget *w = _lookupWidget(id);
		g_signal_connect(G_OBJECT(w),
						   "toggled",
						   G_CALLBACK(s_leader_change),
						   (gpointer) this);

		// set the "userdata" to be the tALignment
		gtk_object_set_user_data( GTK_OBJECT(w), (gpointer)((UT_uint32)id - (UT_uint32)id_LEADER_NONE + (UT_uint32)FL_LEADER_NONE));
	}

	// create user data tControl -> stored in widgets 
	for ( int i = 0; i < id_last; i++ )
	{

		GtkWidget *w = _lookupWidget( (tControl)i );
		UT_ASSERT( w && GTK_IS_WIDGET(w) );

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		UT_ASSERT( g_object_get_data(G_OBJECT(w), "tControl" ) == NULL);

		g_object_set_data( G_OBJECT(w), "tControl", (gpointer) i );
	}
}

GtkWidget *AP_UnixDialog_Tab::_lookupWidget ( tControl id )
{
	UT_return_val_if_fail(m_Widgets.getItemCount() > (UT_uint32)id, NULL);

	GtkWidget *w = (GtkWidget*)m_Widgets.getNthItem((UT_uint32)id);
	UT_ASSERT(w && GTK_IS_WIDGET(w));

	return w;
}

void AP_UnixDialog_Tab::_controlEnable( tControl id, bool value )
{
	GtkWidget *w = _lookupWidget(id);
	UT_return_if_fail( w && GTK_IS_WIDGET(w) );
	gtk_widget_set_sensitive( w, value );
}


void AP_UnixDialog_Tab::_spinChanged(void)
{
        UT_sint32 i =  (UT_sint32) GTK_ADJUSTMENT(m_oDefaultSpin_adj)->value;
	UT_sint32 amt = i - m_iDefaultSpin;
	if(amt < 0)
	      amt = -1;
	else if(amt > 0)
	      amt = 1;
	_doSpin(id_SPIN_DEFAULT_TAB_STOP, amt);
	m_iDefaultSpin = i;
}


/*****************************************************************/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// WP level events
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
/*static*/ void AP_UnixDialog_Tab::s_spin_default_changed(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	dlg->_spinChanged();
}


/*static*/ void AP_UnixDialog_Tab::s_set_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 
	dlg->_event_Set();	
}

/*static*/ void AP_UnixDialog_Tab::s_clear_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 
	dlg->_event_Clear(); 
}

/*static*/ void AP_UnixDialog_Tab::s_clear_all_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 
	dlg->_event_ClearAll(); 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Listbox stuff

// TODO - This should be moved to XAP code, but the methods in which GTK and
// windows handles selection/deselection differ so much, it's easier just to
// code up the hooks directly.

/*static*/ void AP_UnixDialog_Tab::s_list_select(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(dlg); 
	UT_return_if_fail(widget && GTK_IS_LIST_ITEM(widget));

	// get the -1, 0.. (n-1) index
	dlg->m_iGtkListIndex = gtk_list_child_position(GTK_LIST(dlg->_lookupWidget(id_LIST_TAB)), widget);

	dlg->_event_TabSelected( dlg->m_iGtkListIndex );
}

/*static*/ void AP_UnixDialog_Tab::s_list_deselect(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_list_deselect\n"));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// edit box stuff

/*static*/ void AP_UnixDialog_Tab::s_edit_change(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_edit_change\n"));

	dlg->_event_TabChange();
}

/*static*/ void AP_UnixDialog_Tab::s_alignment_change( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 

	// we're only interested in "i'm not toggled"
	if ( dlg->m_bInSetCall || gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget)) == FALSE ) 
		return;

	dlg->m_current_alignment = (eTabType)GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(widget)));

	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_alignment_change [%c]\n", AlignmentToChar(dlg->m_current_alignment)));
	dlg->_event_AlignmentChange();
}

/*static*/ void AP_UnixDialog_Tab::s_leader_change( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_return_if_fail(widget && dlg); 
	
	// we're only interested in "i'm not toggled"
	if ( dlg->m_bInSetCall || gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget)) == FALSE ) 
		return;
	
	dlg->m_current_leader = (eTabLeader)GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(widget)));
	
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_leader_change\n"));
	dlg->_event_somethingChanged();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabType AP_UnixDialog_Tab::_gatherAlignment()
{
	return m_current_alignment;
}

void AP_UnixDialog_Tab::_setAlignment( eTabType a )
{
	tControl id = id_ALIGN_LEFT;
	
	
	switch(a)
		{
		case FL_TAB_LEFT:
			id = id_ALIGN_LEFT;
			break;

		case FL_TAB_CENTER:
			id = id_ALIGN_CENTER;
			break;

		case FL_TAB_RIGHT:
			id = id_ALIGN_RIGHT;
			break;

		case FL_TAB_DECIMAL:
			id = id_ALIGN_DECIMAL;
			break;

		case FL_TAB_BAR:
			id = id_ALIGN_BAR;
			break;

			// FL_TAB_NONE, __FL_TAB_MAX
		default:
		  return;
		}
	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id); 
	UT_return_if_fail(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message
	m_bInSetCall = true;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = false;

	m_current_alignment = a;

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_UnixDialog_Tab::_gatherLeader()
{
	return m_current_leader;
}

void AP_UnixDialog_Tab::_setLeader( eTabLeader a )
{
	// NOTE - tControl id_LEADER_NONE .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_LEADER_NONE + (UT_uint32)a);	
	UT_return_if_fail( id >= id_LEADER_NONE && id <= id_LEADER_UNDERLINE );

	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id );
	UT_return_if_fail(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message

	m_bInSetCall = true;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = false;

	m_current_leader = a;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const XML_Char* AP_UnixDialog_Tab::_gatherDefaultTabStop()
{
        return gtk_entry_get_text( GTK_ENTRY( _lookupWidget( id_SPIN_DEFAULT_TAB_STOP ) ) );
}

void AP_UnixDialog_Tab::_setDefaultTabStop( const XML_Char* defaultTabStop )
{
	GtkWidget *w = _lookupWidget( id_SPIN_DEFAULT_TAB_STOP );

	// then set the text

	gtk_entry_set_text( GTK_ENTRY(w), defaultTabStop );
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void AP_UnixDialog_Tab::_setTabList(UT_uint32 count)
{
	GList *gList = NULL;
	GtkList *wList = GTK_LIST(_lookupWidget( id_LIST_TAB ));
	UT_uint32 i;

	// clear all the items from the list
	gtk_list_clear_items( wList, 0, -1 );

	for ( i = 0; i < count; i++ )
	{
		GtkWidget *li = gtk_list_item_new_with_label( _getTabDimensionString(i));

		// we want to DO stuff
		g_signal_connect(G_OBJECT(li),
						   "select",
						   G_CALLBACK(s_list_select),
						   (gpointer) this);

		// show this baby
		gtk_widget_show(li);

		gList = g_list_append( gList, li );
	}
	
	gtk_list_insert_items( wList, gList, 0 );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_UnixDialog_Tab::_gatherSelectTab()
{
	return m_iGtkListIndex;
}

void AP_UnixDialog_Tab::_setSelectTab( UT_sint32 v )
{
	m_iGtkListIndex = v;

	if ( v == -1 )	// we don't want to select anything
	{
		gtk_list_unselect_all(GTK_LIST(_lookupWidget(id_LIST_TAB)));
	}
	else
	{
		GtkList *wList = GTK_LIST(_lookupWidget( id_LIST_TAB ));
		gtk_list_select_item( wList, m_iGtkListIndex);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_UnixDialog_Tab::_gatherTabEdit()
{
	return gtk_entry_get_text( GTK_ENTRY( _lookupWidget( id_EDIT_TAB ) ) );
}

void AP_UnixDialog_Tab::_setTabEdit( const char *pszStr )
{
	GtkWidget *w = _lookupWidget( id_EDIT_TAB );

	// first, we stop the entry from sending the changed signal to our handler
	gtk_signal_handler_block_by_data(  GTK_OBJECT(w), (gpointer) this );

	// then set the text
	gtk_entry_set_text( GTK_ENTRY(w), pszStr );

	// turn signals back on
	gtk_signal_handler_unblock_by_data(  GTK_OBJECT(w), (gpointer) this );
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_UnixDialog_Tab::_clearList()
{
	GtkList *wList = GTK_LIST(_lookupWidget( id_LIST_TAB ));

	// clear all the items from the list
	gtk_list_clear_items( wList, 0, -1 );
}
