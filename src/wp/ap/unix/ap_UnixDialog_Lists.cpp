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
#include "ap_Dialog_Lists.h"
#include "ap_UnixDialog_Lists.h"

/*****************************************************************/

static AP_UnixDialog_Lists * Current_Dialog;


AP_UnixDialog_Lists::AP_UnixDialog_Lists(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Lists(pDlgFactory,id)
{
	m_wMainWindow = NULL;
	Current_Dialog = this;
}

XAP_Dialog * AP_UnixDialog_Lists::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_UnixDialog_Lists * p = new AP_UnixDialog_Lists(pFactory,id);
	return p;
}


AP_UnixDialog_Lists::~AP_UnixDialog_Lists(void)
{
}

static void s_startChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->startChanged ();
}

static void s_stopChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->stopChanged ();
}

static void s_startvChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->startvChanged ();
}

static void s_applyClicked (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->applyClicked();
}

static void s_closeClicked (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->destroy();
}

static void s_deleteClicked (GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Lists * me)
{
	me->destroy();
}

static gboolean s_update (void)
{
	Current_Dialog->updateDialog();
	return TRUE;
}

void AP_UnixDialog_Lists::runModeless (XAP_Frame * pFrame)
{
	_constructWindow ();
	UT_ASSERT (m_wMainWindow);

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid = (UT_sint32) getDialogId ();
	m_pApp->rememberModelessId(sid, (XAP_Dialog_Modeless *) m_pDialog);
 
	// This magic command displays the frame that characters will be
	// inserted into.
        // This variation runs the additional static function shown afterwards.
        // Only use this if you need to to update the dialog upon focussing.

	connectFocusModelessOther (GTK_WIDGET (m_wMainWindow), m_pApp, (gboolean (*)(void)) s_update);

	// Populate the dialog
	updateDialog();

	// Now Display the dialog
	gtk_widget_show_all (m_wMainWindow);

	// Next construct a timer for auto-updating the dialog
	GR_Graphics * pG = NULL;
	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists,this,pG);
	m_bDestroy_says_stopupdating = UT_FALSE;

	// OK fire up the auto-updater for 0.5 secs

	m_pAutoUpdateLists->set(500);

}

         
void    AP_UnixDialog_Lists::autoupdateLists(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);
	// this is a static callback method and does not have a 'this' pointer.
	AP_UnixDialog_Lists * pDialog =  (AP_UnixDialog_Lists *) pTimer->getInstanceData();
	// Handshaking code

	if( pDialog->m_bDestroy_says_stopupdating != UT_TRUE)
	{
		pDialog->m_bAutoUpdate_happening_now = UT_TRUE;
		pDialog->updateDialog();
		pDialog->m_bAutoUpdate_happening_now = UT_FALSE;
	}
}        

void AP_UnixDialog_Lists::destroy (void)
{
	UT_ASSERT (m_wMainWindow);
	m_bDestroy_says_stopupdating = UT_TRUE;
	while (m_bAutoUpdate_happening_now == UT_TRUE) ;
	m_pAutoUpdateLists->stop();
	m_answer = AP_Dialog_Lists::a_CLOSE;	
	modeless_cleanup();
	gtk_widget_destroy(m_wMainWindow);
	m_wMainWindow = NULL;
	DELETEP(m_pAutoUpdateLists);
}

void AP_UnixDialog_Lists::activate (void)
{
	UT_ASSERT (m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), m_WindowName);
	updateDialog();
	gdk_window_raise (m_wMainWindow->window);
}

void AP_UnixDialog_Lists::notifyActiveFrame(XAP_Frame *pFrame)
{
        UT_ASSERT(m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), m_WindowName);
	updateDialog();
}

void  AP_UnixDialog_Lists::applyClicked(void)
{
        gchar * szStartValue;
	GtkWidget* wlisttype;

	//
	// Failsafe code to make sure the start, stop and change flags are set
        // as shown on the GUI.
	//

       if (GTK_TOGGLE_BUTTON (m_wCheckstartlist)->active)
       {
	       wlisttype=gtk_menu_get_active(GTK_MENU(m_wOption_types_menu));
	       m_iListType = (List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
	       szStartValue =gtk_entry_get_text( GTK_ENTRY (m_wNew_startingvaluev) );
	       m_bStartList = UT_TRUE;
	       if(m_iListType == NUMBERED_LIST)
	       {
		      m_newStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType, "%*%d.");
	       }
	       else if (m_iListType == LOWERCASE_LIST)
	       {
		      m_newStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%a.");
	       }
	       else if (m_iListType == UPPERCASE_LIST)
	       {
		      m_newStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%A.");
	       }
	       else if (m_iListType == BULLETED_LIST)
	       {
		 //	      gchar c = *szStartValue;
		      m_newStartValue = 1;
		      strcpy((gchar *) m_newListType, "%b");
	       }
       }
       else
       {
	       m_bStartList = UT_FALSE;
       }

       if (GTK_TOGGLE_BUTTON (m_wCheckstoplist)->active)
       {
	       m_bStopList = UT_TRUE;
       }
       else
       {
	       m_bStopList = UT_FALSE;
       }

       if (GTK_TOGGLE_BUTTON (m_wCur_changestart_button)->active)
       {
	       m_bChangeStartValue = UT_TRUE;
	       wlisttype=gtk_menu_get_active(GTK_MENU(m_wCur_Option_types_menu));
	       m_iListType = (List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
	       szStartValue =gtk_entry_get_text( GTK_ENTRY (m_wCur_startingvaluev) );
	       m_bStartList = UT_TRUE;
	       if(m_iListType == NUMBERED_LIST)
	       {
		      m_curStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType, "%*%d.");
	       }
	       else if (m_iListType == LOWERCASE_LIST)
	       {
		      m_curStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%a.");
	       }
	       else if (m_iListType == UPPERCASE_LIST)
	       {
		      m_curStartValue = atoi(szStartValue);
		      strcpy((gchar *) m_newListType,"%*%A.");
	       }
	       else if (m_iListType == BULLETED_LIST)
	       {
		 //	      gchar c = *szStartValue;
		      m_curStartValue = 1;
		      strcpy((gchar *) m_newListType, "%b");
	       }
       }
       else
       {
	       m_bChangeStartValue = UT_FALSE;
       }
       if (GTK_TOGGLE_BUTTON (m_wCheckresumelist)->active)
       {
	       m_bresumeList = UT_TRUE;
       }
       else
       {
	       m_bresumeList = UT_FALSE;
       }

       Apply();

       // Make all checked buttons inactive 
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstartlist),FALSE);
       setAllSensitivity();
}

void  AP_UnixDialog_Lists::startChanged(void)
{
       if (GTK_TOGGLE_BUTTON (m_wCheckstartlist)->active)
       {
	       m_bStartList = UT_TRUE;
	       m_bStopList = UT_FALSE;
	       m_bChangeStartValue = UT_FALSE;
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
       }
       else
       {
	       m_bStartList = UT_FALSE;
       }
       setAllSensitivity();
}

void  AP_UnixDialog_Lists::stopChanged(void)
{
       if (GTK_TOGGLE_BUTTON (m_wCheckstoplist)->active)
       {
	       m_bStopList = UT_TRUE;
	       m_bChangeStartValue = UT_FALSE;
	       m_bStartList = UT_FALSE;
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstartlist),FALSE);
       }
       else
       {
	       m_bStopList = UT_FALSE;
       }
       setAllSensitivity();
}


void  AP_UnixDialog_Lists::startvChanged(void)
{
       if (GTK_TOGGLE_BUTTON (m_wCur_changestart_button)->active)
       {
	       m_bChangeStartValue = UT_TRUE;
	       m_bStartList = UT_FALSE;
	       m_bStopList = UT_FALSE;
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstartlist),FALSE);
       }
       else
       {
	       m_bChangeStartValue = UT_FALSE;
       }
       setAllSensitivity();
}


void AP_UnixDialog_Lists::updateDialog(void)
{
	_populateWindowData();
	setAllSensitivity();
}

void AP_UnixDialog_Lists::setAllSensitivity(void)
{ 
       gtk_widget_set_sensitive( m_wCheckstartlist,TRUE);
       PopulateDialogData();
       if(m_isListAtPoint == UT_TRUE)
       {
	       gtk_widget_set_sensitive( m_wCheckstoplist,TRUE);
	       gtk_widget_set_sensitive( m_wCur_listtype,TRUE);
	       gtk_widget_set_sensitive( m_wCur_listtypev,TRUE);
	       gtk_widget_set_sensitive( m_wCur_listlabel,TRUE);
	       gtk_widget_set_sensitive( m_wCur_listlabelv,TRUE);
	       gtk_widget_set_sensitive( m_wCheckresumelist,FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
       }
       else
       {
	       gtk_widget_set_sensitive( m_wCheckstoplist,FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckstoplist),FALSE);
	       m_bStopList = UT_FALSE;
	       gtk_widget_set_sensitive( m_wCur_listtype,FALSE);
	       gtk_widget_set_sensitive( m_wCur_listtypev,FALSE);
	       gtk_widget_set_sensitive( m_wCur_listlabel,FALSE);
	       gtk_widget_set_sensitive( m_wCur_listlabelv,FALSE);
	       gtk_widget_set_sensitive( m_wCur_changestart_button,FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
	       m_bChangeStartValue = UT_FALSE;
	       gtk_widget_set_sensitive( m_wCur_Option_types,FALSE);
	       gtk_widget_set_sensitive( m_wCur_startingvaluel,FALSE);
	       gtk_widget_set_sensitive( m_wCur_startingvaluev,FALSE);
	       if(m_previousListExistsAtPoint == UT_TRUE)
	       {
	               gtk_widget_set_sensitive( m_wCheckresumelist,TRUE);
	       }
	       else
	       {
	               gtk_widget_set_sensitive( m_wCheckresumelist,FALSE);
		       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
	       }

       }
       if(!GTK_TOGGLE_BUTTON( m_wCur_changestart_button)->active)
       {
	       gtk_widget_set_sensitive( m_wCur_Option_types,FALSE);
	       gtk_widget_set_sensitive( m_wCur_startingvaluel,FALSE);
	       gtk_widget_set_sensitive( m_wCur_startingvaluev,FALSE);
       }

       if(GTK_TOGGLE_BUTTON( m_wCheckstoplist)->active)
       {
	       gtk_widget_set_sensitive( m_wCur_changestart_button,FALSE);
	       gtk_widget_set_sensitive( m_wCheckresumelist,FALSE);
	       gtk_widget_set_sensitive( m_wCur_Option_types,FALSE);
	       gtk_widget_set_sensitive( m_wCur_startingvaluel,FALSE);
	       gtk_widget_set_sensitive( m_wCur_startingvaluev,FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
       }
       else if(m_isListAtPoint == UT_TRUE)
       {
	       gtk_widget_set_sensitive( m_wCur_changestart_button,TRUE);
	       if(GTK_TOGGLE_BUTTON( m_wCur_changestart_button)->active)
	       {
	                gtk_widget_set_sensitive( m_wCur_Option_types,TRUE);
	                gtk_widget_set_sensitive( m_wCur_startingvaluel,TRUE);
	                gtk_widget_set_sensitive( m_wCur_startingvaluev,TRUE);
	       }
       }
       if(GTK_TOGGLE_BUTTON( m_wCheckstartlist)->active)
       {
	       gtk_widget_set_sensitive( m_wNewlisttypel,TRUE);
	       gtk_widget_set_sensitive( m_wOption_types,TRUE);
	       gtk_widget_set_sensitive( m_wOption_types_menu,TRUE);
	       gtk_widget_set_sensitive( m_wNew_startingvaluel,TRUE);
	       gtk_widget_set_sensitive( m_wNew_startingvaluev,TRUE);
	       gtk_widget_set_sensitive( m_wCheckresumelist,FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCur_changestart_button),FALSE);
               gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wCheckresumelist),FALSE);
       }
       else
       {
	       gtk_widget_set_sensitive( m_wNewlisttypel,FALSE);
	       gtk_widget_set_sensitive( m_wOption_types,FALSE);
	       gtk_widget_set_sensitive( m_wOption_types_menu,FALSE);
	       gtk_widget_set_sensitive( m_wNew_startingvaluel,FALSE);
	       gtk_widget_set_sensitive( m_wNew_startingvaluev,FALSE);
       }
}

GtkWidget * AP_UnixDialog_Lists::_constructWindow (void)
{
	GtkWidget *contents;
	GtkWidget *hseparator3;
	GtkWidget *hbox6;
	GtkWidget *apply;
	GtkWidget *close;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_wMainWindow = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_container_set_border_width (GTK_CONTAINER (m_wMainWindow), 4);
        ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow),m_WindowName);
	gtk_window_set_policy(GTK_WINDOW(m_wMainWindow), FALSE, FALSE, FALSE);

	contents = _constructWindowContents ();

	//---------------------------------------------------------
	// Do the stuff on the bottom that should be gnomized
	//------------------------------------------------------------------

	hseparator3 = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (contents), hseparator3, TRUE, TRUE, 0);

	hbox6 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (contents), hbox6, TRUE, TRUE, 0);

	// Buttons

	close = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Close));
	gtk_box_pack_end (GTK_BOX (hbox6), close, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (close), 6);

	apply = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Apply));
	gtk_box_pack_end (GTK_BOX (hbox6), apply, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (apply), 6);
	m_wApply = apply;
        m_wClose = close;
	

	//-------------------------------------------------------------------

	// Now put the whole lot into the Window

        gtk_container_add (GTK_CONTAINER (m_wMainWindow), contents);

	GTK_WIDGET_SET_FLAGS (m_wApply, GTK_CAN_DEFAULT);
 	GTK_WIDGET_SET_FLAGS (m_wClose, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (m_wClose);

	_connectSignals ();

	return (m_wMainWindow);
}

GtkWidget *AP_UnixDialog_Lists::_constructWindowContents (void)
{
        GtkWidget *contents;
	GtkWidget *stopResumeList;
	GtkWidget *check_startlist;
	GtkWidget *check_stoplist;
	GtkWidget *check_resumelist;
	GtkWidget *hseparator1;
	GtkWidget *defineNewList;
	GtkWidget *new_startingvaluel;
	GtkWidget *new_startingvaluev;
	GtkWidget *new_listtype;
	GtkWidget *option_types;
	GtkWidget *option_types_menu;
	GtkWidget *hseparator2;
	GtkWidget *dispCurrentList;
	GtkWidget *cur_listtype;
	GtkWidget *cur_listtypev;
	GtkWidget *cur_listlabel;
	GtkWidget *cur_listlabelv;
	GtkWidget *changeCurrentList;
	GtkWidget *cur_changestart_button;
	GtkWidget *change_option_types;
	GtkWidget *change_option_types_menu;
	GtkWidget *cur_startingvaluel;
	GtkWidget *cur_startingvaluev;


	const XAP_StringSet * pSS = m_pApp->getStringSet();

	contents = gtk_vbox_new (FALSE, 0);

	//--------Display Current List Stuff--------------------------------

	dispCurrentList = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (contents), dispCurrentList, TRUE, TRUE, 0);

	cur_listtype = gtk_label_new (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Current_List_Type));
	gtk_box_pack_start (GTK_BOX (dispCurrentList), cur_listtype, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (cur_listtype), 8, 0);

        cur_listtypev = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dispCurrentList), cur_listtypev, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (cur_listtypev), 5, 0);

        cur_listlabel = gtk_label_new (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Current_List_Label));
	gtk_box_pack_start (GTK_BOX (dispCurrentList), cur_listlabel, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (cur_listlabel), 18, 0);

	cur_listlabelv = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dispCurrentList), cur_listlabelv, FALSE, FALSE, 0);

	// --------- Change Current list region--------------------------

	changeCurrentList = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (contents), changeCurrentList, TRUE, TRUE, 0);

	cur_changestart_button = gtk_check_button_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Cur_Change_Start));
	gtk_box_pack_start (GTK_BOX (changeCurrentList), cur_changestart_button, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (cur_changestart_button), 6);

	//-----------------------------------------------------------
        // Change List type menu
        //-----------------------------------------------------------

	change_option_types = gtk_option_menu_new ();
        gtk_widget_show (change_option_types);


	gtk_box_pack_start (GTK_BOX (changeCurrentList), change_option_types, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (change_option_types), 1);

	change_option_types_menu = gtk_menu_new ();
        gtk_widget_show (change_option_types_menu);
	_fillListTypeMenu(change_option_types_menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (change_option_types),change_option_types_menu );
        gtk_menu_set_active(GTK_MENU (change_option_types_menu),0);

	cur_startingvaluel = gtk_label_new (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Starting_Value));
	gtk_box_pack_start (GTK_BOX (changeCurrentList), cur_startingvaluel, FALSE, FALSE, 12);
	gtk_label_set_line_wrap (GTK_LABEL (cur_startingvaluel), TRUE);
	cur_startingvaluev = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (changeCurrentList), cur_startingvaluev, FALSE, FALSE, 1);
	gtk_widget_set_usize (cur_startingvaluev, 90, -2);
	gtk_entry_set_text (GTK_ENTRY ( cur_startingvaluev),"1");


	//----- end of Change Current listype menu -------------------------

	//--------- Start New List --------------------------------

	hseparator1 = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (contents), hseparator1, TRUE, TRUE, 0);

	defineNewList = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (contents), defineNewList, TRUE, TRUE, 0);


	check_startlist = gtk_check_button_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New_List));
	gtk_box_pack_start (GTK_BOX (defineNewList), check_startlist, TRUE, FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (check_startlist), 6);

	new_listtype = gtk_label_new (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_New_List_Type));
	gtk_box_pack_start (GTK_BOX (defineNewList), new_listtype, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (new_listtype), 6, 0);

	//-----------------------------------------------------------
        // Define New List type menu
        //-----------------------------------------------------------

	option_types = gtk_option_menu_new ();
        gtk_widget_show (option_types);

	gtk_box_pack_start (GTK_BOX (defineNewList), option_types, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (option_types), 1);

	option_types_menu = gtk_menu_new ();
        gtk_widget_show (option_types_menu);

	_fillListTypeMenu(option_types_menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_types),option_types_menu );
        gtk_menu_set_active(GTK_MENU (option_types_menu),0);

	new_startingvaluel = gtk_label_new (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Starting_Value));
	gtk_box_pack_start (GTK_BOX (defineNewList), new_startingvaluel, FALSE, FALSE, 12);
	gtk_label_set_line_wrap (GTK_LABEL (new_startingvaluel), TRUE);
	new_startingvaluev = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (defineNewList), new_startingvaluev, FALSE, FALSE, 1);
	gtk_widget_set_usize (new_startingvaluev, 90, -2);
	gtk_entry_set_text (GTK_ENTRY ( new_startingvaluev),"1");

	//
	//--------End of Define New List Type Menu --------------------
        //-------Stop List / Resume List checkboxes -------------------
	//

	hseparator2 = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (contents), hseparator2, TRUE, TRUE, 0);

	stopResumeList = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (contents), stopResumeList, TRUE, TRUE, 0);

	check_stoplist = gtk_check_button_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Stop_Current_List));
	gtk_box_pack_start (GTK_BOX (stopResumeList), check_stoplist, TRUE, FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (check_stoplist), 7);


	check_resumelist = gtk_check_button_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Resume_Previous_List));
	gtk_box_pack_start (GTK_BOX (stopResumeList), check_resumelist, TRUE, FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (check_resumelist), 7);

	// Save useful widgets in member variables

	m_wContents = contents;
	m_wCheckstartlist = check_startlist;
	m_wCheckstoplist = check_stoplist;
	m_wCheckresumelist = check_resumelist;
	m_wNewlisttypel = new_listtype;
        m_wOption_types = option_types;
        m_wOption_types_menu = option_types_menu;
	m_wNew_startingvaluel = new_startingvaluel;
	m_wNew_startingvaluev = new_startingvaluev;
	m_wCur_listtype = cur_listtype;
	m_wCur_listtypev = cur_listtypev;
	m_wCur_listlabel = cur_listlabel;
	m_wCur_listlabelv = cur_listlabelv;
        m_wCur_changestart_button = cur_changestart_button;
        m_wCur_Option_types = change_option_types;
        m_wCur_Option_types_menu = change_option_types_menu;
	m_wCur_startingvaluel = cur_startingvaluel;
	m_wCur_startingvaluev = cur_startingvaluev;
	return contents;
}

void AP_UnixDialog_Lists::_fillListTypeMenu( GtkWidget *listmenu)
{
        GtkWidget *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(0));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);

        glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(1));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(2));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(3));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);

}

void AP_UnixDialog_Lists::_populateWindowData (void) 
{
	char *tmp;
        PopulateDialogData();
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	if(m_isListAtPoint == UT_TRUE)
	{
		gtk_label_set_text(GTK_LABEL(m_wCur_listlabelv),m_curListLabel);
		gtk_label_set_text(GTK_LABEL(m_wCur_listtypev),m_curListType);
		if(!GTK_TOGGLE_BUTTON (m_wCur_changestart_button)->active)
		{
		        tmp = g_strdup_printf ("%d", m_curStartValue);
		        gtk_entry_set_text (GTK_ENTRY ( m_wCur_startingvaluev),tmp);
			if(strstr(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List))!=NULL)
			       gtk_menu_set_active(GTK_MENU (m_wCur_Option_types_menu),0);
			else if(strstr(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List))!=NULL)
			       gtk_menu_set_active(GTK_MENU (m_wCur_Option_types_menu),1);
			else if(strstr(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List))!=NULL)
			       gtk_menu_set_active(GTK_MENU (m_wCur_Option_types_menu),2);
			else if(strstr(m_curListType, pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List))!=NULL)
			       gtk_menu_set_active(GTK_MENU (m_wCur_Option_types_menu),3);
		        g_free(tmp);
		}
	}
}

void AP_UnixDialog_Lists::_connectSignals(void)
{
	gtk_signal_connect_after(GTK_OBJECT(m_wMainWindow),
							 "destroy",
							 NULL,
							 NULL);
	//
        // Don't use connect_after in modeless dialog
	gtk_signal_connect(GTK_OBJECT(m_wMainWindow),
						     "delete_event",
						     GTK_SIGNAL_FUNC(s_deleteClicked), (gpointer) this);

	gtk_signal_connect (GTK_OBJECT (m_wApply), "clicked",
						GTK_SIGNAL_FUNC (s_applyClicked), this);
	gtk_signal_connect (GTK_OBJECT (m_wClose), "clicked",
						GTK_SIGNAL_FUNC (s_closeClicked), this);
	gtk_signal_connect (GTK_OBJECT (m_wCheckstartlist), "clicked",
						GTK_SIGNAL_FUNC (s_startChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_wCheckstoplist), "clicked",
						GTK_SIGNAL_FUNC (s_stopChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_wCur_changestart_button), "clicked",
						GTK_SIGNAL_FUNC (s_startvChanged), this);
}













