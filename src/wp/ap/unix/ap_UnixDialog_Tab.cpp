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
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"
#include "fl_BlockLayout.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
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
	m_bInSetCall		= UT_FALSE;

}

AP_UnixDialog_Tab::~AP_UnixDialog_Tab(void)
{

}

/*****************************************************************/

void AP_UnixDialog_Tab::runModal(XAP_Frame * pFrame)
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
    gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

    // Show the top level dialog,
    gtk_widget_show(mainWindow);

    // Make it modal, and stick it up top
    gtk_grab_add(mainWindow);

    // Run into the GTK event loop for this window.
	do {
		gtk_main();

		switch ( m_answer )
		{
		case AP_Dialog_Tab::a_OK:
			_storeWindowData();
			break;

		case AP_Dialog_Tab::a_APPLY:
			UT_DEBUGMSG(("Applying changes\n"));
			_storeWindowData();
			break;

		case AP_Dialog_Tab::a_CANCEL:
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		};

	} while ( m_answer == AP_Dialog_Tab::a_APPLY );	
	
	gtk_widget_destroy(mainWindow);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// unix specific handlers 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_UnixDialog_Tab::event_OK(void)
{
    m_answer = AP_Dialog_Tab::a_OK;
    gtk_main_quit();
}

void AP_UnixDialog_Tab::event_Cancel(void)
{
    m_answer = AP_Dialog_Tab::a_CANCEL;
    gtk_main_quit();
}

void AP_UnixDialog_Tab::event_Apply(void)
{
    m_answer = AP_Dialog_Tab::a_APPLY;
    gtk_main_quit();
}

void AP_UnixDialog_Tab::event_WindowDelete(void)
{
    m_answer = AP_Dialog_Tab::a_CANCEL;    
    gtk_main_quit();
}

/*****************************************************************/
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        gtk_signal_connect(GTK_OBJECT(w), "activate",	\
                GTK_SIGNAL_FUNC(s_menu_item_activate),		\
                (gpointer) this);							\
        } while (0)
GtkWidget* AP_UnixDialog_Tab::_constructWindow (void )
{
	
	GtkWidget *windowTabs;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowTabs = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	//windowTabs = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowTabs), "windowTabs", windowTabs);
	gtk_window_set_title (GTK_WINDOW (windowTabs), pSS->getValue( AP_STRING_ID_DLG_Tab_TabTitle));

	_constructWindowContents(windowTabs);
	return windowTabs;
}

void    AP_UnixDialog_Tab::_constructGnomeButtons( GtkWidget * windowTabs)
{
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;
	GtkWidget *buttonApply;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	//
	// Gnome buttons
	//
	
	buttonApply = gtk_button_new_with_label (pSS->getValue( AP_STRING_ID_DLG_Options_Btn_Apply));
	gtk_widget_ref (buttonApply);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "buttonApply", buttonApply,
							(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonApply);
	gtk_container_add (GTK_CONTAINER (m_GnomeButtons), buttonApply);
	GTK_WIDGET_SET_FLAGS (buttonApply, GTK_CAN_DEFAULT);

	buttonOK = gtk_button_new_with_label (pSS->getValue( XAP_STRING_ID_DLG_OK));
	gtk_widget_ref (buttonOK);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "buttonOK", buttonOK,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (m_GnomeButtons), buttonOK);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label (pSS->getValue( XAP_STRING_ID_DLG_Cancel));
	gtk_widget_ref (buttonCancel);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "buttonCancel", buttonCancel,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (m_GnomeButtons), buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);
	m_buttonApply = buttonApply;
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
	GtkWidget *hseparator5;
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
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();
	const XAP_StringSet * pSS = m_pApp->getStringSet();


	table13 = gtk_table_new (5, 1, FALSE);
	gtk_widget_ref (table13);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "table13", table13,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table13);
	gtk_container_add (GTK_CONTAINER (windowTabs), table13);

	hbuttonbox4 = gtk_hbutton_box_new ();
	gtk_widget_ref (hbuttonbox4);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbuttonbox4", hbuttonbox4,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbuttonbox4);
	gtk_table_attach (GTK_TABLE (table13), hbuttonbox4, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox4), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox4), 5);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonbox4), 0, 0);

	buttonSet = gtk_button_new_with_label (pSS->getValue( AP_STRING_ID_DLG_Tab_Button_Set));
	gtk_widget_ref (buttonSet);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "buttonSet", buttonSet,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonSet);
	gtk_container_add (GTK_CONTAINER (hbuttonbox4), buttonSet);
	GTK_WIDGET_SET_FLAGS (buttonSet, GTK_CAN_DEFAULT);

	buttonClear = gtk_button_new_with_label (pSS->getValue( AP_STRING_ID_DLG_Tab_Button_Clear));
	gtk_widget_ref (buttonClear);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "buttonClear", buttonClear,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonClear);
	gtk_container_add (GTK_CONTAINER (hbuttonbox4), buttonClear);
	GTK_WIDGET_SET_FLAGS (buttonClear, GTK_CAN_DEFAULT);

	buttonClearAll = gtk_button_new_with_label (pSS->getValue( AP_STRING_ID_DLG_Tab_Button_ClearAll));
	gtk_widget_ref (buttonClearAll);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "buttonClearAll", buttonClearAll,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonClearAll);
	gtk_container_add (GTK_CONTAINER (hbuttonbox4), buttonClearAll);
	GTK_WIDGET_SET_FLAGS (buttonClearAll, GTK_CAN_DEFAULT);

	hbuttonbox3 = gtk_hbutton_box_new ();
	m_GnomeButtons = hbuttonbox3;
	gtk_widget_ref (hbuttonbox3);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbuttonbox3", hbuttonbox3,
				  (GtkDestroyNotify) gtk_widget_unref);
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
	_constructGnomeButtons( windowTabs);

	hseparator5 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator5);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hseparator5", hseparator5,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator5);
	gtk_table_attach (GTK_TABLE (table13), hseparator5, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);

	hbox10 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox10);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox10", hbox10,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox10);
	gtk_table_attach (GTK_TABLE (table13), hbox10, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_FILL), 0, 0);

	label8 = gtk_label_new (pSS->getValue( AP_STRING_ID_DLG_Tab_Label_TabToClear));
	gtk_widget_ref (label8);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "label8", label8,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label8);
	gtk_box_pack_start (GTK_BOX (hbox10), label8, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (label8), 10, 0);

	hbox11 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox11);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox11", hbox11,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox11);
	gtk_table_attach (GTK_TABLE (table13), hbox11, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (vbox4);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "vbox4", vbox4,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox4);
	gtk_box_pack_start (GTK_BOX (hbox11), vbox4, TRUE, TRUE, 5);

	hbox15 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox15);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox15", hbox15,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox15);
	gtk_box_pack_start (GTK_BOX (vbox4), hbox15, FALSE, FALSE, 5);

	label13 = gtk_label_new (pSS->getValue( AP_STRING_ID_DLG_Tab_Label_TabPosition));
	gtk_widget_ref (label13);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "label13", label13,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label13);
	gtk_box_pack_start (GTK_BOX (hbox15), label13, FALSE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);

	entryTabEntry = gtk_entry_new ();
	gtk_widget_ref (entryTabEntry);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "entryTabEntry", entryTabEntry,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (entryTabEntry);
	gtk_box_pack_start (GTK_BOX (vbox4), entryTabEntry, FALSE, FALSE, 1);

	hbox16 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox16);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox16", hbox16,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox16);
	gtk_box_pack_start (GTK_BOX (vbox4), hbox16, TRUE, TRUE, 1);

	label14 = gtk_label_new ("     ");
	gtk_widget_ref (label14);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "label14", label14,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label14);
	gtk_box_pack_start (GTK_BOX (hbox16), label14, FALSE, TRUE, 0);

	frame3 = gtk_frame_new (NULL);
	gtk_widget_ref (frame3);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "frame3", frame3,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (frame3);
	gtk_box_pack_start (GTK_BOX (hbox16), frame3, TRUE, TRUE, 0);

	listTabs = gtk_list_new ();
	gtk_widget_ref (listTabs);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "listTabs", listTabs,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listTabs);
	gtk_container_add (GTK_CONTAINER (frame3), listTabs);

	table14 = gtk_table_new (8, 2, FALSE);
	gtk_widget_ref (table14);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "table14", table14,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table14);
	gtk_box_pack_start (GTK_BOX (hbox11), table14, TRUE, TRUE, 0);

	hbox13 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox13);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox13", hbox13,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox13);
	gtk_table_attach (GTK_TABLE (table14), hbox13, 0, 2, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	label10 = gtk_label_new (pSS->getValue( AP_STRING_ID_DLG_Tab_Label_Alignment));
	gtk_widget_ref (label10);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "label10", label10,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label10);
	gtk_box_pack_start (GTK_BOX (hbox13), label10, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label10), 5, 0);

	hseparator6 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator6);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hseparator6", hseparator6,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator6);
	gtk_box_pack_start (GTK_BOX (hbox13), hseparator6, TRUE, TRUE, 0);

	hbox14 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox14);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox14", hbox14,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox14);
	gtk_table_attach (GTK_TABLE (table14), hbox14, 0, 2, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	label11 = gtk_label_new (pSS->getValue( AP_STRING_ID_DLG_Tab_Label_Leader));
	gtk_widget_ref (label11);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "label11", label11,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label11);
	gtk_box_pack_start (GTK_BOX (hbox14), label11, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label11), 5, 0);

	hseparator7 = gtk_hseparator_new ();
	gtk_widget_ref (hseparator7);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hseparator7", hseparator7,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hseparator7);
	gtk_box_pack_start (GTK_BOX (hbox14), hseparator7, TRUE, TRUE, 0);

	radiobuttonDecimal = gtk_radio_button_new_with_label (group_align_group, 
			pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Decimal));
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonDecimal));
	gtk_widget_ref (radiobuttonDecimal);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonDecimal", radiobuttonDecimal,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonDecimal);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonDecimal, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	radiobuttonLeft = gtk_radio_button_new_with_label (group_align_group, 
					pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Left));
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeft));
	gtk_widget_ref (radiobuttonLeft);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonLeft", radiobuttonLeft,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonLeft);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeft, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobuttonLeft), TRUE);

	radiobuttonCenter = gtk_radio_button_new_with_label (group_align_group, 
						pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Center));
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonCenter));
	gtk_widget_ref (radiobuttonCenter);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonCenter", radiobuttonCenter,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonCenter);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonCenter, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);

	radiobuttonRight = gtk_radio_button_new_with_label (group_align_group, 
				pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Right));
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonRight));
	gtk_widget_ref (radiobuttonRight);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonRight", radiobuttonRight,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonRight);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonRight, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);

	radiobuttonBar = gtk_radio_button_new_with_label (group_align_group, 
					pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Bar));
	group_align_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonBar));
	gtk_widget_ref (radiobuttonBar);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonBar", radiobuttonBar,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonBar);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonBar, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	radiobuttonLeaderDash = gtk_radio_button_new_with_label (group_leader_group, 
				pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Dash));
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderDash));
	gtk_widget_ref (radiobuttonLeaderDash);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonLeaderDash", radiobuttonLeaderDash,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonLeaderDash);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderDash, 1, 2, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	radiobuttonLeaderDot = gtk_radio_button_new_with_label (group_leader_group, 
				pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Dot));
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderDot));
	gtk_widget_ref (radiobuttonLeaderDot);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonLeaderDot", radiobuttonLeaderDot,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonLeaderDot);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderDot, 0, 1, 7, 8,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);

	radiobuttonLeaderNone = gtk_radio_button_new_with_label (group_leader_group, 
			pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_None));
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderNone));
	gtk_widget_ref (radiobuttonLeaderNone);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonLeaderNone", radiobuttonLeaderNone,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonLeaderNone);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderNone, 0, 1, 6, 7,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobuttonLeaderNone), TRUE);

	radiobuttonLeaderUnderline = gtk_radio_button_new_with_label (group_leader_group, 
					pSS->getValue( AP_STRING_ID_DLG_Tab_Radio_Underline));
	group_leader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonLeaderUnderline));
	gtk_widget_ref (radiobuttonLeaderUnderline);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "radiobuttonLeaderUnderline", radiobuttonLeaderUnderline,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (radiobuttonLeaderUnderline);
	gtk_table_attach (GTK_TABLE (table14), radiobuttonLeaderUnderline, 1, 2, 7, 8,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	hbox12 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox12);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "hbox12", hbox12,
							  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox12);
	gtk_table_attach (GTK_TABLE (table14), hbox12, 0, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 5, 5);

	label9 = gtk_label_new (pSS->getValue( AP_STRING_ID_DLG_Tab_Label_DefaultTS));
	gtk_widget_ref (label9);
	gtk_object_set_data_full (GTK_OBJECT (windowTabs), "label9", label9,
							  (GtkDestroyNotify) gtk_widget_unref);
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

	gtk_window_add_accel_group (GTK_WINDOW (windowTabs), accel_group);


    //////////////////////////////////////////////////////////////////////
	// END: glade stuff

    // the catch-alls
    gtk_signal_connect_after(GTK_OBJECT(windowTabs),
                             "delete_event",
                             GTK_SIGNAL_FUNC(s_delete_clicked),
                             (gpointer) this);


    gtk_signal_connect_after(GTK_OBJECT(windowTabs),
                             "destroy",
                             NULL,
                             NULL);

    //////////////////////////////////////////////////////////////////////
    // the control buttons
    gtk_signal_connect(GTK_OBJECT(m_buttonOK),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_ok_clicked),
                       (gpointer) this);
    
    gtk_signal_connect(GTK_OBJECT(m_buttonCancel),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_cancel_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(m_buttonApply),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_apply_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonSet),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_set_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonClear),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_clear_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonClearAll),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_clear_all_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(entryTabEntry),
                       "changed",
                       GTK_SIGNAL_FUNC(s_edit_change),
                       (gpointer) this);

    gtk_signal_connect(spinbuttonTabstop_adj,
                       "value_changed",
                       GTK_SIGNAL_FUNC(s_spin_default_changed),
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
	m_Widgets.setNthItem( id_BUTTON_APPLY,			m_buttonApply,				NULL);


	// some lists of signals to set
	tControl id;
	for ( id = id_ALIGN_LEFT; id <= id_ALIGN_BAR; id = (tControl)((UT_uint32)id + 1))
	{
		GtkWidget *w = _lookupWidget(id);
		gtk_signal_connect(GTK_OBJECT(w),
						   "toggled",
						   GTK_SIGNAL_FUNC(s_alignment_change),
						   (gpointer) this);

		// set the "userdata" to be the tALignment
		gtk_object_set_user_data( GTK_OBJECT(w), (gpointer)((UT_uint32)id - (UT_uint32)id_ALIGN_LEFT + (UT_uint32)FL_TAB_LEFT));
	}

	for ( id = id_LEADER_NONE; id <= id_LEADER_UNDERLINE; id = (tControl)((UT_uint32)id + 1))
	{
		GtkWidget *w = _lookupWidget(id);
		gtk_signal_connect(GTK_OBJECT(w),
						   "toggled",
						   GTK_SIGNAL_FUNC(s_leader_change),
						   (gpointer) this);

		// set the "userdata" to be the tALignment
		gtk_object_set_user_data( GTK_OBJECT(w), (gpointer)((UT_uint32)id - (UT_uint32)id_LEADER_NONE + (UT_uint32)FL_LEADER_NONE));
	}

	// create the accelerators from &'s
	createLabelAccelerators(windowTabs);

	// create user data tControl -> stored in widgets 
	for ( int i = 0; i < id_last; i++ )
	{

		GtkWidget *w = _lookupWidget( (tControl)i );
		UT_ASSERT( w && GTK_IS_WIDGET(w) );

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		UT_ASSERT( gtk_object_get_data(GTK_OBJECT(w), "tControl" ) == NULL);

		gtk_object_set_data( GTK_OBJECT(w), "tControl", (gpointer) i );
	}
}

GtkWidget *AP_UnixDialog_Tab::_lookupWidget ( tControl id )
{
	UT_ASSERT(m_Widgets.getItemCount() > (UT_uint32)id );

	GtkWidget *w = (GtkWidget*)m_Widgets.getNthItem((UT_uint32)id);
	UT_ASSERT(w && GTK_IS_WIDGET(w));

	return w;
}

void AP_UnixDialog_Tab::_controlEnable( tControl id, UT_Bool value )
{
	GtkWidget *w = _lookupWidget(id);
	UT_ASSERT( w && GTK_IS_WIDGET(w) );
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
// UNIX level events
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 
// sample callback function
/*static*/ void AP_UnixDialog_Tab::s_ok_clicked(GtkWidget * /*widget*/, gpointer data)
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(dlg); 
	dlg->event_OK(); 
}

/*static*/ void AP_UnixDialog_Tab::s_cancel_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Cancel(); 
}

/*static*/ void AP_UnixDialog_Tab::s_apply_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Apply(); 
}

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
	UT_ASSERT(widget && dlg); 
	dlg->_event_Set();	
}

/*static*/ void AP_UnixDialog_Tab::s_clear_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	dlg->_event_Clear(); 
}

/*static*/ void AP_UnixDialog_Tab::s_clear_all_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
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
	UT_ASSERT(dlg); 
	UT_ASSERT(widget && GTK_IS_LIST_ITEM(widget));

	// get the -1, 0.. (n-1) index
	dlg->m_iGtkListIndex = gtk_list_child_position(GTK_LIST(dlg->_lookupWidget(id_LIST_TAB)), widget);

	dlg->_event_TabSelected( dlg->m_iGtkListIndex );
}

/*static*/ void AP_UnixDialog_Tab::s_list_deselect(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_list_deselect\n"));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// edit box stuff

/*static*/ void AP_UnixDialog_Tab::s_edit_change(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_edit_change\n"));

	dlg->_event_TabChange();
}

/*static*/ void AP_UnixDialog_Tab::s_alignment_change( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 

	// we're only interested in "i'm not toggled"
	if ( dlg->m_bInSetCall || gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget)) == FALSE ) 
		return;

	dlg->m_current_alignment = (eTabType)((UT_uint32)gtk_object_get_user_data(GTK_OBJECT(widget)));

	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_alignment_change [%c]\n", AlignmentToChar(dlg->m_current_alignment)));
	dlg->_event_AlignmentChange();
}

/*static*/ void AP_UnixDialog_Tab::s_leader_change( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(widget && dlg); 
	
	// we're only interested in "i'm not toggled"
	if ( dlg->m_bInSetCall || gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget)) == FALSE ) 
		return;
	
	dlg->m_current_leader = (eTabLeader)((UT_uint32)gtk_object_get_user_data(GTK_OBJECT(widget)));
	
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_leader_change\n"));
	dlg->_event_somethingChanged();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// when a window is closed
/*static*/ void AP_UnixDialog_Tab::s_delete_clicked(GtkWidget * /* widget */, GdkEvent * /*event*/, gpointer data )
{ 
	AP_UnixDialog_Tab * dlg = (AP_UnixDialog_Tab *)data;
	UT_ASSERT(dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Tab::s_delete_clicked\n"));
	dlg->event_WindowDelete(); 
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

		}
	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id); 
	UT_ASSERT(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message
	m_bInSetCall = UT_TRUE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = UT_FALSE;

	m_current_alignment = a;

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_UnixDialog_Tab::_gatherLeader()
{
	return FL_LEADER_NONE;
}

void AP_UnixDialog_Tab::_setLeader( eTabLeader a )
{
	// NOTE - tControl id_LEADER_NONE .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_LEADER_NONE + (UT_uint32)a);	
	UT_ASSERT( id >= id_LEADER_NONE && id <= id_LEADER_UNDERLINE );

	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id );
	UT_ASSERT(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message

	m_bInSetCall = UT_TRUE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = UT_FALSE;
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
		gtk_signal_connect(GTK_OBJECT(li),
						   "select",
						   GTK_SIGNAL_FUNC(s_list_select),
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




