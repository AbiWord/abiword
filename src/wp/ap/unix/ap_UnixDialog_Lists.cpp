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
	m_pPreviewWidget = NULL;
	m_bManualListStyle = UT_TRUE;
        m_bDoExpose = UT_TRUE;
}

XAP_Dialog * AP_UnixDialog_Lists::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_UnixDialog_Lists * p = new AP_UnixDialog_Lists(pFactory,id);
	return p;
}


AP_UnixDialog_Lists::~AP_UnixDialog_Lists(void)
{
	if(m_pPreviewWidget != NULL)
	       DELETEP (m_pPreviewWidget);
}

static void s_customChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->customChanged ();
}


static void s_typeChangedNone (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->typeChanged ( 0 );
}


static void s_typeChangedBullet (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->typeChanged ( 1 );
}


static void s_styleChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
        me->setMemberVariables();
	me->previewExposed();
}


static void s_typeChangedNumbered (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	me->typeChanged ( 2 );
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


static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Lists * me)
{
	UT_ASSERT(widget && me);
	me->previewExposed();
	return FALSE;
}


static gboolean s_window_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Lists * me)
{
	UT_ASSERT(widget && me);
	me->previewExposed();
	return FALSE;
}

static gboolean s_update (void)
{
	if(Current_Dialog->getAvView()->getTick() != Current_Dialog->getTick())
	{
	        Current_Dialog->setTick(Current_Dialog->getAvView()->getTick());
		Current_Dialog->updateDialog();
	}
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
	gtk_widget_show(m_wMainWindow);

	// *** this is how we add the gc for Lists Preview ***
	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);

	UT_ASSERT(m_wPreviewArea && m_wPreviewArea->window);

	// make a new Unix GC
	m_pPreviewWidget = new GR_UnixGraphics(m_wPreviewArea->window, unixapp->getFontManager(), m_pApp);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
			     (UT_uint32) m_wPreviewArea->allocation.width, 
			     (UT_uint32) m_wPreviewArea->allocation.height);

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
	// Handshaking code. Plus only update if something in the document
	// changed.

	if(pDialog->getAvView()->getTick() != pDialog->getTick())
	{
	        pDialog->setTick(pDialog->getAvView()->getTick());
	        if( pDialog->m_bDestroy_says_stopupdating != UT_TRUE)
	        {
		         pDialog->m_bAutoUpdate_happening_now = UT_TRUE;
			 pDialog->updateDialog();
			 pDialog->previewExposed();
			 pDialog->m_bAutoUpdate_happening_now = UT_FALSE;
		}
	}
}   
     

void AP_UnixDialog_Lists::previewExposed(void)
{
        if(m_pPreviewWidget)
        {
	        if(m_bDoExpose == UT_TRUE)
		{
                        event_PreviewAreaExposed();
		}
		m_bDoExpose = UT_TRUE;
	}
} 


void AP_UnixDialog_Lists::destroy (void)
{
	UT_ASSERT (m_wMainWindow);
	m_bDestroy_says_stopupdating = UT_TRUE;
	while (m_bAutoUpdate_happening_now == UT_TRUE) ;
	m_pAutoUpdateLists->stop();
	m_answer = AP_Dialog_Lists::a_CLOSE;	

	g_list_free( m_glFonts);
	modeless_cleanup();
	gtk_widget_destroy(m_wMainWindow);
	m_wMainWindow = NULL;
	DELETEP(m_pAutoUpdateLists);
	DELETEP (m_pPreviewWidget);
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
	previewExposed();
}


void  AP_UnixDialog_Lists::typeChanged(gint style)
{
  // 
  // code to change list list
  //

	gtk_option_menu_remove_menu(GTK_OPTION_MENU (m_wListStyleBox));
	m_bDoExpose = UT_TRUE;
	if(style == 0)
	{
	  //     gtk_widget_destroy(GTK_WIDGET(m_wListStyleBulleted_menu));
	  	m_wListStyleNone_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNone_menu;
	        _fillNoneStyleMenu(m_wListStyleNone_menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox), 
					  m_wListStyleNone_menu);
	}
	else if(style == 1)
	{
	  //    gtk_widget_destroy(GTK_WIDGET(m_wListStyleBulleted_menu));
       		m_wListStyleBulleted_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleBulleted_menu;
	        _fillBulletedStyleMenu(m_wListStyleBulleted_menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox), 
					  m_wListStyleBulleted_menu);
	}
	else if(style == 2)
	{
	  //  gtk_widget_destroy(GTK_WIDGET(m_wListStyleNumbered_menu));
	  	m_wListStyleNumbered_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNumbered_menu;
	        _fillNumberedStyleMenu(m_wListStyleNumbered_menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox), 
					  m_wListStyleNumbered_menu);
	}
	if(m_bManualListStyle == UT_TRUE)
	{
	        GtkWidget * wlisttype=gtk_menu_get_active(GTK_MENU(m_wListStyle_menu));
		m_newListType =  (List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
		m_iListType = m_newListType;
	}
	previewExposed();
}


void  AP_UnixDialog_Lists::setMemberVariables(void)
{
	//
	// Failsafe code to make sure the start, stop and change flags are set
        // as shown on the GUI.
	//
	
	GtkWidget * wlisttype=gtk_menu_get_active(GTK_MENU(m_wListStyle_menu));
	m_newListType =  (List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype)));
	m_bguiChanged = UT_TRUE;
	if(m_bisCustomized == UT_TRUE)
	{
	        _gatherData();
	}
	if (GTK_TOGGLE_BUTTON (m_wStartNewList)->active)
	{
	        m_bStartNewList = UT_TRUE;
		m_bApplyToCurrent = UT_FALSE;
                m_bStartSubList = UT_FALSE;
	}
	else if (GTK_TOGGLE_BUTTON (m_wApplyCurrent)->active)
	{
	        m_bStartNewList = UT_FALSE;
		m_bApplyToCurrent = UT_TRUE;
                m_bStartSubList = UT_FALSE;
	}
	else if (GTK_TOGGLE_BUTTON (m_wStartSubList)->active)
	{
	        m_bStartNewList = UT_FALSE;
		m_bApplyToCurrent = UT_FALSE;
                m_bStartSubList = UT_TRUE;
	}
	m_bDoExpose = UT_TRUE;
}


void  AP_UnixDialog_Lists::applyClicked(void)
{
        setMemberVariables();
	previewExposed();
	Apply();
}

void  AP_UnixDialog_Lists::customChanged(void)
{
	if(m_bisCustomFrameHidden == UT_TRUE)
	{
		fillWidgetFromDialog();
	        gtk_widget_show(m_wCustomFrame);
		gtk_arrow_set(GTK_ARROW(m_wCustomArrow),GTK_ARROW_DOWN,GTK_SHADOW_OUT);
		m_bisCustomFrameHidden = UT_FALSE;
                m_bisCustomized = UT_TRUE;
		setMemberVariables();
		previewExposed();
	}
	else
	{
	        gtk_widget_hide(m_wCustomFrame);
		gtk_arrow_set(GTK_ARROW(m_wCustomArrow),GTK_ARROW_RIGHT,GTK_SHADOW_OUT);
		m_bisCustomFrameHidden = UT_TRUE;
                m_bisCustomized = UT_FALSE;
		fillUncustomizedValues();
		_setData();
	}
}


void AP_UnixDialog_Lists::fillWidgetFromDialog(void)
{
        PopulateDialogData();
	_setData();
}

void AP_UnixDialog_Lists::updateDialog(void)
{

        UT_uint32 oldID = m_iID;
	m_bDoExpose = UT_FALSE;
	m_bManualListStyle = UT_FALSE;
        if(m_bisCustomized == UT_FALSE)
	{
                _populateWindowData();
                m_iID = getID();
	}
	if((oldID != getID()) && (m_bisCustomized == UT_FALSE))
	{               
                 m_newListType = m_iListType;
                 m_bDoExpose = UT_TRUE;
	}
	if(m_bisCustomized == UT_FALSE)
	{
	         _setData();
	}
	m_bManualListStyle = UT_TRUE;
}

void AP_UnixDialog_Lists::setAllSensitivity(void)
{ 
       PopulateDialogData();
       if(m_isListAtPoint == UT_TRUE)
       {
       }
}

GtkWidget * AP_UnixDialog_Lists::_constructWindow (void)
{
	GtkWidget *contents;
	GtkWidget *windowMain;
	GtkWidget *gnomeButtons;
	GtkWidget *Apply;
	GtkWidget *Close;
	GtkWidget *hseparator1;

	const XAP_StringSet * pSS = m_pApp->getStringSet();


	windowMain = gtk_window_new (GTK_WINDOW_DIALOG);
	m_wMainWindow = windowMain;
	gtk_widget_set_name (windowMain, "windowMain");
	gtk_object_set_data (GTK_OBJECT (windowMain), "windowMain", windowMain);
	//	gtk_widget_set_usize (windowMain, 428, 341);
        ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow),m_WindowName);
	gtk_window_set_policy(GTK_WINDOW(m_wMainWindow), FALSE, FALSE, FALSE);

	contents = _constructWindowContents ();

	//---------------------------------------------------------
	// Do the stuff on the bottom that should be gnomized
	//------------------------------------------------------------------

	// Seperator
	
	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (contents), hseparator1, FALSE, TRUE, 0);


	// Buttons

	gnomeButtons = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (gnomeButtons);
	gtk_box_pack_start (GTK_BOX (contents), gnomeButtons, FALSE, TRUE, 0);
	gtk_widget_set_usize (gnomeButtons, -2, -2);

	Close = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Close));
	m_wClose = Close;
	gtk_widget_show (Close);
	gtk_box_pack_end (GTK_BOX (gnomeButtons), Close, FALSE, FALSE, 3);
	//gtk_widget_set_usize (Close, 57, -2);
	gtk_container_set_border_width (GTK_CONTAINER (Close), 2);
	
	Apply = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Apply));
	m_wApply = Apply;
	gtk_widget_show (Apply);
	gtk_box_pack_end (GTK_BOX (gnomeButtons), Apply, FALSE, FALSE, 3);
	//gtk_widget_set_usize (Apply, 57, -2);
	gtk_container_set_border_width (GTK_CONTAINER (Apply), 2);

	//-------------------------------------------------------------------

	// Now put the whole lot into the Window

        //gtk_container_add (GTK_CONTAINER (m_wMainWindow), contents);

	GTK_WIDGET_SET_FLAGS (m_wApply, GTK_CAN_DEFAULT);
 	GTK_WIDGET_SET_FLAGS (m_wClose, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (m_wClose);

	_connectSignals ();

	return (m_wMainWindow);
}

GtkWidget *AP_UnixDialog_Lists::_constructWindowContents (void)
{
        GtkWidget *windowMain;
        GtkWidget *contents;
        GtkWidget *overallVbox;
	GtkWidget *hbox2;
	GtkWidget *overallSelectionVbox;
	GtkWidget *listChoiceBox;
	GtkWidget *typeLabel;
	GtkWidget *listType;
	GtkWidget *listType_menu;
	GtkWidget *glade_menuitem;
	GtkWidget *hbox3;
	GtkWidget *styleLabel;
	GtkWidget *listStyleBox;
	GtkWidget *OverallCustomizeFrame;
	GtkWidget *customizeVbox;
	GtkWidget *arrowBox;
	GtkWidget *arrow1;
	GtkWidget *customizeLabel;
	GtkWidget *customFrame;
	GtkWidget *table1;
	GtkWidget *textDelim;
	GtkWidget *fontLabel_;
	GtkWidget *levelEntry;
	GtkWidget *startAtLabel;
	GtkWidget *alignLabel;
	GtkWidget *indentLabel;
	GtkWidget *fontOptions;
	GtkWidget *fontOptions_menu;
	GtkObject *startSpin_adj;
	GtkWidget *startSpin;
	GtkWidget *levelSpin;
	GtkObject *alignListSpin_adj;
	GtkWidget *alignListSpin;
	GtkObject *indentAlignSpin_adj;
	GtkWidget *indentAlignSpin;
	GtkWidget *delimEntry;
	GtkWidget *previewFrame;
	GtkWidget *subpreviewFrame;
	GtkWidget *previewArea;
	GtkWidget *radioBox;
	
	GSList *radioBox_group = NULL;
	GtkWidget *startNewList;
	GtkWidget *applyCurrent;
	GtkWidget *startSubList;
	GtkWidget *resumeList;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowMain = m_wMainWindow;
	overallVbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (overallVbox);
	gtk_container_add (GTK_CONTAINER (windowMain), overallVbox);
	contents  = overallVbox;

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (overallVbox), hbox2, TRUE, TRUE, 0);

	overallSelectionVbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (overallSelectionVbox);
	gtk_box_pack_start (GTK_BOX (hbox2), overallSelectionVbox, TRUE, TRUE, 0);
	
	listChoiceBox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (listChoiceBox);
	gtk_box_pack_start (GTK_BOX (overallSelectionVbox), listChoiceBox, FALSE, FALSE, 0);
	//gtk_widget_set_usize (listChoiceBox, -2, 32);
	
	typeLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Type));
	gtk_widget_show (typeLabel);
	gtk_box_pack_start (GTK_BOX (listChoiceBox), typeLabel, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (typeLabel), 7, 0);
	
	listType = gtk_option_menu_new ();
	gtk_widget_show (listType);
	gtk_box_pack_start (GTK_BOX (listChoiceBox), listType, FALSE, FALSE, 0);
	//	gtk_widget_set_usize (listType, -2, 18);
	gtk_container_set_border_width (GTK_CONTAINER (listChoiceBox), 3);
	listType_menu = gtk_menu_new ();
	GtkWidget * menu_none = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Type_none));
	m_wMenu_None = menu_none;
	gtk_object_set_user_data(GTK_OBJECT(menu_none),GINT_TO_POINTER(0));
	gtk_widget_show (menu_none);
	gtk_menu_append (GTK_MENU (listType_menu), menu_none);
	GtkWidget * menu_bull  = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Type_bullet));
	gtk_object_set_user_data(GTK_OBJECT(menu_bull),GINT_TO_POINTER(1));
	gtk_widget_show (menu_bull);
	m_wMenu_Bull = menu_bull;
	gtk_menu_append (GTK_MENU (listType_menu), menu_bull);
	GtkWidget * menu_num  = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Type_numbered));
	gtk_object_set_user_data(GTK_OBJECT(menu_num),GINT_TO_POINTER(2));
	gtk_widget_show (menu_num);
	m_wMenu_Num = menu_num;
	gtk_menu_append (GTK_MENU (listType_menu), menu_num);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listType), listType_menu);
	//
	// This is how we set the active element of an option menu!!
	//
        gtk_option_menu_set_history (GTK_OPTION_MENU (listType), 2);
	gtk_widget_set_events(listType, GDK_ALL_EVENTS_MASK);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (overallSelectionVbox), hbox3, FALSE, FALSE, 0);
	
	styleLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Style));
	gtk_widget_show (styleLabel);
	gtk_box_pack_start (GTK_BOX (hbox3), styleLabel, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (styleLabel), 6, 0);
	
	listStyleBox = gtk_option_menu_new ();
	gtk_widget_show (listStyleBox);
	gtk_box_pack_start (GTK_BOX (hbox3), listStyleBox, FALSE, FALSE, 0);
	//	gtk_widget_set_usize (listStyleBox, -2, 32);
	gtk_container_set_border_width (GTK_CONTAINER (listStyleBox), 3);

	m_wListStyleNone_menu = gtk_menu_new();
	_fillNoneStyleMenu(m_wListStyleNone_menu);
	m_wListStyleNumbered_menu = gtk_menu_new ();
	_fillNumberedStyleMenu(m_wListStyleNumbered_menu);
	m_wListStyleBulleted_menu = gtk_menu_new();
	_fillBulletedStyleMenu(m_wListStyleBulleted_menu);
	//
	// This is the default list. Change if the list style changes
	//
	m_wListStyle_menu = m_wListStyleNumbered_menu;
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listStyleBox), m_wListStyleNumbered_menu);
	
	OverallCustomizeFrame = gtk_frame_new (NULL);
	gtk_widget_show (OverallCustomizeFrame);
	gtk_box_pack_start (GTK_BOX (overallSelectionVbox), OverallCustomizeFrame, TRUE, TRUE, 4);
	//	gtk_widget_set_usize (OverallCustomizeFrame, 194, 120);
	gtk_container_set_border_width (GTK_CONTAINER (OverallCustomizeFrame), 2);
	gtk_frame_set_shadow_type (GTK_FRAME (OverallCustomizeFrame), GTK_SHADOW_NONE);

	customizeVbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (customizeVbox);
	gtk_container_add (GTK_CONTAINER (OverallCustomizeFrame), customizeVbox);
	
	arrowBox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (arrowBox);
	gtk_box_pack_start (GTK_BOX (customizeVbox), arrowBox, FALSE, FALSE, 0);

	arrow1 = gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (arrowBox), arrow1, FALSE, FALSE, 0);
	gtk_widget_show (arrow1);
	
	customizeLabel = gtk_button_new_with_label(pSS->getValue(AP_STRING_ID_DLG_Lists_Customize));
	gtk_box_pack_start (GTK_BOX (arrowBox), customizeLabel, FALSE, FALSE, 0);
	gtk_widget_show (customizeLabel);
	//	gtk_widget_set_usize (customizeLabel, -2, 18);
	
	customFrame = gtk_frame_new ("");
	gtk_box_pack_start (GTK_BOX (customizeVbox), customFrame, TRUE, TRUE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (customFrame), 5);
	
	table1 = gtk_table_new (6, 2, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (customFrame), table1);
	
	textDelim = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Format));
	gtk_widget_show (textDelim);
	gtk_table_attach (GTK_TABLE (table1), textDelim, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (textDelim), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (textDelim), 0.1, 0.5);
	
	fontLabel_ = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Font));
	gtk_widget_show (fontLabel_);
	gtk_table_attach (GTK_TABLE (table1), fontLabel_, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (fontLabel_), 0.1, 0.5);
	
	levelEntry = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Level));
	gtk_widget_show (levelEntry);
	gtk_table_attach (GTK_TABLE (table1), levelEntry, 0, 1, 2, 3,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (levelEntry), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (levelEntry), 0.1, 0.5);
	
	startAtLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Start));
	gtk_widget_show (startAtLabel);
	gtk_table_attach (GTK_TABLE (table1), startAtLabel, 0, 1, 3, 4,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (startAtLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (startAtLabel), 0.1, 0.5);
	
	alignLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Align));
	gtk_widget_show (alignLabel);
	gtk_table_attach (GTK_TABLE (table1), alignLabel, 0, 1, 4, 5,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (alignLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (alignLabel), 0.1, 0.5);
	
	indentLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Indent));
	gtk_widget_show (indentLabel);
	gtk_table_attach (GTK_TABLE (table1), indentLabel, 0, 1, 5, 6,
			  (GtkAttachOptions) (0),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_label_set_justify (GTK_LABEL (indentLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (indentLabel), 0.1, 0.5);
	
	fontOptions = gtk_option_menu_new ();
	gtk_widget_show (fontOptions);

	gtk_table_attach (GTK_TABLE (table1), fontOptions, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_SHRINK),
			  (GtkAttachOptions) (0), 0, 0);
	//gtk_widget_set_usize (fontOptions, 130, -2);
	
	m_glFonts = _getGlistFonts();
	gint i;
	gint nfonts = g_list_length(m_glFonts);
	fontOptions_menu = gtk_menu_new ();
	gtk_widget_show (fontOptions_menu);

	glade_menuitem = gtk_menu_item_new_with_label( pSS->getValue(AP_STRING_ID_DLG_Lists_Current_Font));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(0));
	gtk_menu_append (GTK_MENU (fontOptions_menu), glade_menuitem);
	gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	for(i=1; i<nfonts+1;i++)
	{
	       glade_menuitem = gtk_menu_item_new_with_label( (gchar *) g_list_nth_data(m_glFonts, i-1));
	       gtk_widget_show (glade_menuitem);
	       gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(i));
	       gtk_menu_append (GTK_MENU (fontOptions_menu), glade_menuitem);
	       gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (fontOptions), fontOptions_menu);
	
	startSpin_adj = gtk_adjustment_new (1, -100, 100, 1, 2, 2);
	startSpin = gtk_spin_button_new (GTK_ADJUSTMENT (startSpin_adj), 1, 0);
	gtk_widget_show (startSpin);
	//gtk_widget_set_usize (startSpin,60, -2);
	gtk_table_attach(GTK_TABLE (table1), startSpin, 1, 2, 3, 4,
       			  (GtkAttachOptions) (GTK_SHRINK),
			  (GtkAttachOptions) (0), 0, 0);
	
	levelSpin = gtk_entry_new_with_max_length(20);
	gtk_widget_show (levelSpin);
	//	gtk_widget_set_usize (levelSpin,60, -2);
	gtk_table_attach(GTK_TABLE (table1), levelSpin, 1, 2, 2, 3,
      			  (GtkAttachOptions) (GTK_SHRINK),
			  (GtkAttachOptions) (0), 0, 0);

	alignListSpin_adj = gtk_adjustment_new (0.25, 0.0, 10, 0.01, 0.02, 0.02);
	alignListSpin = gtk_spin_button_new (GTK_ADJUSTMENT (alignListSpin_adj), 0.2, 2);
	gtk_widget_show (alignListSpin);
	//	gtk_widget_set_usize (alignListSpin,60, -2);
	gtk_table_attach(GTK_TABLE (table1), alignListSpin, 1, 2, 4, 5,
      			  (GtkAttachOptions) (GTK_SHRINK),
			  (GtkAttachOptions) (0), 0, 0);
	
	indentAlignSpin_adj = gtk_adjustment_new (0.25, 0.0, 10, 0.01, 0.02, 0.02);
	indentAlignSpin = gtk_spin_button_new (GTK_ADJUSTMENT (indentAlignSpin_adj), 0.2, 2);
	gtk_widget_show (indentAlignSpin);
	//gtk_widget_set_usize (indentAlignSpin,60, -2);
	gtk_table_attach(GTK_TABLE (table1), indentAlignSpin, 1, 2, 5, 6,
      			  (GtkAttachOptions) (GTK_SHRINK),
			  (GtkAttachOptions) (0), 0, 0);
	delimEntry = gtk_entry_new_with_max_length (20);
	gtk_widget_show (delimEntry);
	gtk_table_attach (GTK_TABLE (table1), delimEntry, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	//	gtk_widget_set_usize (delimEntry, 71, -2);
	gtk_entry_set_text (GTK_ENTRY (delimEntry), "%L");
	
	previewFrame = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Preview));
	gtk_widget_show (previewFrame);
	gtk_box_pack_start (GTK_BOX (hbox2), previewFrame, FALSE, FALSE, 3);
	gtk_widget_set_usize (previewFrame, 203, 258);
	gtk_container_set_border_width (GTK_CONTAINER (previewFrame), 4);
	gtk_frame_set_shadow_type (GTK_FRAME (previewFrame), GTK_SHADOW_NONE);
	
	subpreviewFrame = gtk_frame_new (NULL);
	gtk_widget_show (subpreviewFrame);
	gtk_container_add (GTK_CONTAINER (previewFrame), subpreviewFrame);
	gtk_container_set_border_width (GTK_CONTAINER (subpreviewFrame), 3);
	gtk_frame_set_shadow_type (GTK_FRAME (subpreviewFrame), GTK_SHADOW_IN);
	
	previewArea = gtk_drawing_area_new ();
	gtk_widget_ref (previewArea);
	gtk_object_set_data_full (GTK_OBJECT (windowMain), "previewArea", previewArea,
								  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (previewArea);
	gtk_container_add (GTK_CONTAINER (subpreviewFrame), previewArea);
	
	radioBox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (radioBox);
	gtk_box_pack_start (GTK_BOX (overallVbox), radioBox, FALSE, FALSE, 0);

	
	m_wStartNew_label = gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
	gtk_widget_show(m_wStartNew_label);
        GtkWidget * relab1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(relab1);
	gtk_box_pack_start (GTK_BOX (relab1), m_wStartNew_label, FALSE, FALSE, 0);

	startNewList = gtk_radio_button_new(radioBox_group);
	gtk_container_add(GTK_CONTAINER(startNewList),relab1);
	radioBox_group = gtk_radio_button_group (GTK_RADIO_BUTTON (startNewList));
	gtk_widget_show (startNewList);

	gtk_box_pack_start (GTK_BOX (radioBox), startNewList, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (startNewList), 3);
	
	applyCurrent = gtk_radio_button_new_with_label (radioBox_group, pSS->getValue(AP_STRING_ID_DLG_Lists_Apply_Current));
	radioBox_group = gtk_radio_button_group (GTK_RADIO_BUTTON (applyCurrent));
	gtk_widget_show (applyCurrent);
	gtk_box_pack_start (GTK_BOX (radioBox), applyCurrent, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (applyCurrent), 3);
	
	m_wStartSub_label = gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Lists_Start_Sub));
	gtk_widget_show(m_wStartSub_label);
        GtkWidget * relab2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(relab2);
	gtk_box_pack_start (GTK_BOX (relab2), m_wStartSub_label, FALSE, FALSE, 0);

	startSubList = gtk_radio_button_new(radioBox_group);
	gtk_container_add(GTK_CONTAINER(startSubList),relab2);
	radioBox_group = gtk_radio_button_group (GTK_RADIO_BUTTON (startSubList));
	gtk_widget_show (startSubList);
	gtk_box_pack_start (GTK_BOX (radioBox), startSubList, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (startSubList), 3);
	

	// Save useful widgets in member variables

	m_wContents = contents;
	m_wStartNewList = startNewList;
	m_wApplyCurrent = applyCurrent;
	m_wStartSubList = startSubList;
	m_wResumeList = resumeList;
	m_wRadioGroup = radioBox_group;
	m_wPreviewArea = previewArea;
	m_wDelimEntry = delimEntry;	
	m_oAlignList_adj = alignListSpin_adj;
	m_wAlignListSpin = alignListSpin;
	m_oIndentAlign_adj = indentAlignSpin_adj;
	m_wIndentAlignSpin = indentAlignSpin;
	m_wLevelSpin = levelSpin;
	m_oStartSpin_adj = startSpin_adj;
	m_wStartSpin = startSpin;

	m_wFontOptions = fontOptions;
	m_wFontOptions_menu = fontOptions_menu;
	m_wCustomFrame = customFrame;
	m_wCustomArrow = arrow1;
	m_wCustomLabel = customizeLabel;
	m_wListStyleBox = listStyleBox;
	m_wListTypeBox = listType;
	m_wListType_menu = listType_menu;
	//
	// Start by hiding the Custom frame
	//
	gtk_widget_hide(m_wCustomFrame);
	m_bisCustomFrameHidden = UT_TRUE;
        m_bisCustomized = UT_FALSE;

	return contents;
}

	
/*
  This code is to suck all the available fonts and put them in a GList.
  This can then be displayed on a combo box at the top of the dialog.
  Code stolen from xap_UnixDialog_Insert_Symbol */
/* Now we remove all the duplicate name entries and create the Glist
   glFonts. This will be used in the font selection combo
   box */

GList *  AP_UnixDialog_Lists::_getGlistFonts (void)
{	  
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_uint32 count = unixapp->getFontManager()->getCount();
	XAP_UnixFont ** list = unixapp->getFontManager()->getAllFonts();
	GList *glFonts = NULL;
	gchar currentfont[50] = "\0";
	gchar * nextfont;
	
	for (UT_uint32 i = 0; i < count; i++)
	{
		gchar * lgn  = (gchar *) list[i]->getName();
		if((strstr(currentfont,lgn)==NULL) || (strlen(currentfont)!=strlen(lgn)) )
		{
			strncpy(currentfont, lgn, 50);
			nextfont = g_strdup(currentfont);
			glFonts = g_list_prepend(glFonts, nextfont);
		}
	}
	DELETEP(list);
	m_glFonts =  g_list_reverse(glFonts);
	return m_glFonts;
}



void AP_UnixDialog_Lists::_fillNoneStyleMenu( GtkWidget *listmenu)
{
        GtkWidget *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Type_none));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) NOT_A_LIST ));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);
}

void AP_UnixDialog_Lists::_fillNumberedStyleMenu( GtkWidget *listmenu)
{
        GtkWidget *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) NUMBERED_LIST ));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


        glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) LOWERCASE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) UPPERCASE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Roman_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) LOWERROMAN_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Roman_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) UPPERROMAN_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);
}


void AP_UnixDialog_Lists::_fillBulletedStyleMenu( GtkWidget *listmenu)
{
        GtkWidget *glade_menuitem;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) BULLETED_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Dashed_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) DASHED_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Square_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) SQUARE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Triangle_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) TRIANGLE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Diamond_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) DIAMOND_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Star_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) STAR_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Implies_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) IMPLIES_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Tick_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) TICK_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Box_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) BOX_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);



	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Hand_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) HAND_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
	       pSS->getValue(AP_STRING_ID_DLG_Lists_Heart_List));
        gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
(gint) HEART_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
        gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
					GTK_SIGNAL_FUNC (s_styleChanged), this);

}

void AP_UnixDialog_Lists::_populateWindowData (void) 
{
  //	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
        PopulateDialogData();
	if(m_isListAtPoint == UT_TRUE)
	{
	  // Button 0 is stop list, button 2 is startsub list
	       gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Stop_Current_List));
	       gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_Sub));

	}
	else
	{
	  // Button 0 is Start New List, button 2 is resume list
	       gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
	       gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Resume));
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
	gtk_signal_connect (GTK_OBJECT (m_wCustomLabel), "clicked",
						GTK_SIGNAL_FUNC (s_customChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_wMenu_None), "activate",
					GTK_SIGNAL_FUNC (s_typeChangedNone), this);
	gtk_signal_connect (GTK_OBJECT (m_wMenu_Bull), "activate",
					GTK_SIGNAL_FUNC (s_typeChangedBullet), this);
	gtk_signal_connect (GTK_OBJECT (m_wMenu_Num), "activate",
					GTK_SIGNAL_FUNC (s_typeChangedNumbered), this);
	gtk_signal_connect (GTK_OBJECT (m_oStartSpin_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	m_iLevelSpinID = gtk_signal_connect (GTK_OBJECT (m_wLevelSpin), "changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_oAlignList_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	gtk_signal_connect (GTK_OBJECT (m_oIndentAlign_adj), "value_changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);
	m_iDelimEntryID = gtk_signal_connect (GTK_OBJECT (GTK_ENTRY(m_wDelimEntry)), "changed",
				      GTK_SIGNAL_FUNC (s_styleChanged), this);


	// the expose event of the preview
	             gtk_signal_connect(GTK_OBJECT(m_wPreviewArea),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_preview_exposed),
					   (gpointer) this);

	
		     gtk_signal_connect_after(GTK_OBJECT(m_wMainWindow),
		     					 "expose_event",
		     				 GTK_SIGNAL_FUNC(s_window_exposed),
		    					 (gpointer) this);

}

void AP_UnixDialog_Lists::_setData(void)
{
  //
  // This function reads the various memeber variables and loads them into
  // into the dialog variables.
  //
        gint i;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wAlignListSpin),m_fAlign);
	float indent = m_fAlign + m_fIndent;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin),indent);
	if( (m_fIndent + m_fAlign) < 0.0)
	{
	         m_fIndent = - m_fAlign;
                 gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), 0.0);

	}
	//
	// Code to work out which is active Font
	//
	if(strcmp((char *) m_pszFont,"NULL") == 0 )
	{
                gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), 0 );
	}
	else
	{
	        for(i=0; i < (gint) g_list_length(m_glFonts);i++)
		{
		         if(strcmp((char *) m_pszFont,(char *) g_list_nth_data(m_glFonts,i)) == 0)
		                 break;
		}
		if(i < (gint) g_list_length(m_glFonts))
		{
                         gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), i+ 1 );
		}
		else
		{
                         gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), 0 );
		}
	}
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wStartSpin),(float) m_iStartValue);
	// first, we stop the entry from sending the changed signal to our handler
        gtk_signal_handler_block(  GTK_OBJECT(m_wLevelSpin), m_iLevelSpinID);

	gtk_entry_set_text( GTK_ENTRY(m_wLevelSpin), (const gchar *) m_pszDecimal);
	// turn signals back on
        gtk_signal_handler_unblock(  GTK_OBJECT(m_wLevelSpin), m_iLevelSpinID);

	// first, we stop the entry from sending the changed signal to our handler
	gtk_signal_handler_block(  GTK_OBJECT(m_wDelimEntry), m_iDelimEntryID );

	gtk_entry_set_text( GTK_ENTRY(m_wDelimEntry), (const gchar *) m_pszDelim);
	// turn signals back on
	gtk_signal_handler_unblock(  GTK_OBJECT(m_wDelimEntry), m_iDelimEntryID );

	//
	// Now set the list type and style
	if(m_newListType == NOT_A_LIST)
	{
	        typeChanged(0);
		gtk_option_menu_set_history( GTK_OPTION_MENU (m_wListStyleBox),0);
	}
	else if(m_newListType >= BULLETED_LIST)
	{
	        typeChanged(1);
		gtk_option_menu_set_history( GTK_OPTION_MENU (m_wListStyleBox),(gint)( m_newListType - BULLETED_LIST));
	}
	else
	{
	        typeChanged(2);
		gtk_option_menu_set_history( GTK_OPTION_MENU (m_wListStyleBox),(gint) m_newListType);
	}
}


void AP_UnixDialog_Lists::_gatherData(void)
{
  //
  // This function reads the various elements in customize box and loads
  // the member variables with them
  //
        m_iLevel =  1;
	//gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wLevelSpin));
	m_fAlign = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_wAlignListSpin));
	float indent = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON( m_wIndentAlignSpin));
	m_fIndent = indent - m_fAlign;
	if( (m_fIndent + m_fAlign) < 0.0)
	{
	         m_fIndent = - m_fAlign;
                 gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), 0.0);

	}
	GtkWidget * wfont = gtk_menu_get_active(GTK_MENU(m_wFontOptions_menu));
	gint ifont =  GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wfont)));
	if(ifont == 0)
	{
                 UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *)  "NULL");
	}
	else
	{
                 UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *)  g_list_nth_data(m_glFonts, ifont-1));
	}
	gchar * pszDec = gtk_entry_get_text( GTK_ENTRY(m_wLevelSpin));
	UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) pszDec);
	m_iStartValue =  gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wStartSpin));
	gchar * pszDel = gtk_entry_get_text( GTK_ENTRY(m_wDelimEntry));
        UT_XML_strncpy((XML_Char *)m_pszDelim, 80, (const XML_Char *) pszDel);
}









