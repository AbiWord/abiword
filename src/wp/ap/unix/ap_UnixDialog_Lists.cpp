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
#include "xap_UnixDialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_UnixFontManager.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_UnixDialog_Lists.h"
#include "fp_Line.h"
#include "fp_Column.h"

/*****************************************************************/

static AP_UnixDialog_Lists * Current_Dialog;


AP_UnixDialog_Lists::AP_UnixDialog_Lists(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Lists(pDlgFactory,id)
{
	m_wMainWindow = NULL;
	Current_Dialog = this;
	m_pPreviewWidget = NULL;
	m_pAutoUpdateLists = NULL;
	m_bManualListStyle = true;
	m_bDontUpdate = false;
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
  	me->setDirty();
	me->customChanged ();
}


static void s_styleChangedNone (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
  	me->setDirty();
	me->styleChanged ( 0 );
}


static void s_styleChangedBullet (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
  	me->setDirty();
	me->fillUncustomizedValues(); // Use defaults to start.
	me->styleChanged ( 1 );
}


static void s_styleChangedNumbered (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
  	me->setDirty();
	me->fillUncustomizedValues(); // Use defaults to start.
	me->styleChanged ( 2 );
}

/*!
 * User has changed their list type selection.
 */
static void s_typeChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	if(me->dontUpdate())
		return;
  	me->setDirty();
	me->setListTypeFromWidget(); // Use this to set m_newListType
	me->fillUncustomizedValues(); // Use defaults to start.
	me->loadXPDataIntoLocal(); // Load them into our member variables
	me->previewExposed();
}

/*!
 * A value in the Customized box has changed.
 */
static void s_valueChanged (GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	if(me->dontUpdate())
		return;
  	me->setDirty();
	me->setXPFromLocal(); // Update member Variables
	me->previewExposed();
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
	if( Current_Dialog->isDirty())
	        return TRUE;
	if(Current_Dialog->getAvView()->getTick() != Current_Dialog->getTick())
	{
		Current_Dialog->setTick(Current_Dialog->getAvView()->getTick());
		Current_Dialog->updateDialog();
	}
	return TRUE;
}

void AP_UnixDialog_Lists::runModal( XAP_Frame * pFrame)
{
	List_Type  savedListType;
	setModal();
	_constructWindow();
	UT_ASSERT (m_wMainWindow);
	connectFocus(GTK_WIDGET(m_wMainWindow),pFrame);
	clearDirty();

	// Populate the dialog
	m_bDontUpdate = false;
	loadXPDataIntoLocal();
//
// Need this to stop this being stomped during the contruction of preview
// widget
//
	savedListType = getNewListType();
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);

	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);

	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(parentWindow, m_wMainWindow);

	// Show the top level dialog,
	//gtk_widget_show(m_wMainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(m_wMainWindow);

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
        // Run into the GTK event loop for this window.
//
// Restore our value
//
	setNewListType(savedListType);
	previewExposed();
	gtk_main();
//
//  We've finished here.
//
	g_list_free( m_glFonts);
	if(m_wMainWindow && GTK_IS_WIDGET(m_wMainWindow))
		gtk_widget_destroy(m_wMainWindow);
	m_wMainWindow = NULL;
	DELETEP (m_pPreviewWidget);
}


void AP_UnixDialog_Lists::runModeless (XAP_Frame * pFrame)
{
	_constructWindow ();
	UT_ASSERT (m_wMainWindow);
	clearDirty();
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
	m_bDontUpdate = false;

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
	m_bDestroy_says_stopupdating = false;

	// OK fire up the auto-updater for 0.5 secs

	m_pAutoUpdateLists->set(500);


}


void AP_UnixDialog_Lists::autoupdateLists(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);
	// this is a static callback method and does not have a 'this' pointer.
	AP_UnixDialog_Lists * pDialog =  (AP_UnixDialog_Lists *) pWorker->getInstanceData();
	// Handshaking code. Plus only update if something in the document
	// changed.

	AP_Dialog_Lists * pList = ( AP_Dialog_Lists *) pDialog;

	if(pList->isDirty())
		return;
	if(pDialog->getAvView()->getTick() != pDialog->getTick())
	{
		pDialog->setTick(pDialog->getAvView()->getTick());
		if( pDialog->m_bDestroy_says_stopupdating != true)
		{
			pDialog->m_bAutoUpdate_happening_now = true;
			pDialog->updateDialog();
			pDialog->previewExposed();
			pDialog->m_bAutoUpdate_happening_now = false;
		}
	}
}


void AP_UnixDialog_Lists::previewExposed(void)
{
	if(m_pPreviewWidget)
	{
		setbisCustomized(true);
		event_PreviewAreaExposed();
	}
}

void AP_UnixDialog_Lists::destroy(void)
{
	UT_ASSERT (m_wMainWindow);
	if(isModal())
	{
		setAnswer(AP_Dialog_Lists::a_QUIT);
		gtk_main_quit();
	}
	else
	{
		m_bDestroy_says_stopupdating = true;
		while (m_bAutoUpdate_happening_now == true) ;
		m_pAutoUpdateLists->stop();
		setAnswer(AP_Dialog_Lists::a_CLOSE);

		g_list_free( m_glFonts);
		modeless_cleanup();
		if(m_wMainWindow && GTK_IS_WIDGET(m_wMainWindow))
			gtk_widget_destroy(m_wMainWindow);
		m_wMainWindow = NULL;
		DELETEP(m_pAutoUpdateLists);
		DELETEP (m_pPreviewWidget);
	}
}

void AP_UnixDialog_Lists::activate (void)
{
	UT_ASSERT (m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), getWindowName());
	m_bDontUpdate = false;
	updateDialog();
	gdk_window_raise (m_wMainWindow->window);
}

void AP_UnixDialog_Lists::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_ASSERT(m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), getWindowName());
	m_bDontUpdate = false;
	updateDialog();
	previewExposed();
}


void  AP_UnixDialog_Lists::styleChanged(gint type)
{
	//
	// code to change list list
	//
	gtk_option_menu_remove_menu(GTK_OPTION_MENU (m_wListStyleBox));
	if(type == 0)
	{
		//     gtk_widget_destroy(GTK_WIDGET(m_wListStyleBulleted_menu));
	  	m_wListStyleNone_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNone_menu;
		_fillNoneStyleMenu(m_wListStyleNone_menu);


		g_signal_handler_block(  G_OBJECT(m_wListStyleBox),m_iStyleBoxID  );
		gtk_signal_handler_block_by_data(  GTK_OBJECT(m_wListStyleBox), (gpointer) this );
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox),
								  m_wListStyleNone_menu);
		g_signal_handler_unblock(  G_OBJECT(m_wListStyleBox),m_iStyleBoxID  );


		gtk_option_menu_set_history (GTK_OPTION_MENU(m_wListTypeBox),
								  0);
		setNewListType(NOT_A_LIST);
	}
	else if(type == 1)
	{
		//    gtk_widget_destroy(GTK_WIDGET(m_wListStyleBulleted_menu));
		m_wListStyleBulleted_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleBulleted_menu;
		_fillBulletedStyleMenu(m_wListStyleBulleted_menu);


		g_signal_handler_block(  G_OBJECT(m_wListStyleBox),m_iStyleBoxID  );

		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox),
								  m_wListStyleBulleted_menu);
		g_signal_handler_unblock(  G_OBJECT(m_wListStyleBox),m_iStyleBoxID  );


		gtk_option_menu_set_history (GTK_OPTION_MENU(m_wListTypeBox),
								  1);
		setNewListType(BULLETED_LIST);
	}
	else if(type == 2)
	{
		//  gtk_widget_destroy(GTK_WIDGET(m_wListStyleNumbered_menu));
	  	m_wListStyleNumbered_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNumbered_menu;
		_fillNumberedStyleMenu(m_wListStyleNumbered_menu);

		// Block events during this manual change

		g_signal_handler_block(  G_OBJECT(m_wListStyleBox),m_iStyleBoxID  );
		gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wListStyleBox),
								  m_wListStyleNumbered_menu);
		g_signal_handler_unblock(  G_OBJECT(m_wListStyleBox), m_iStyleBoxID );
		gtk_option_menu_set_history (GTK_OPTION_MENU(m_wListTypeBox),
								  2);
		setNewListType(NUMBERED_LIST);
	}
//
// This methods needs to be called from loadXPDataIntoLocal to set the correct
// list style. However if we are doing this we definately don't want to call
// loadXPDataIntoLocal again! Luckily we can just check this to make sure this is
// not happenning.
//
	if(!dontUpdate())
	{
		fillUncustomizedValues(); // Set defaults
		loadXPDataIntoLocal(); // load them into the widget
		previewExposed(); // Show current setting
	}
}

/*!
 * This method just sets the value of m_newListType. This is needed to
 * make fillUncustomizedValues work.
 */
void  AP_UnixDialog_Lists::setListTypeFromWidget(void)
{
	GtkWidget * wlisttype=gtk_menu_get_active(GTK_MENU(m_wListStyle_menu));
	setNewListType((List_Type) GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wlisttype))));
}

/*!
 * This method reads out all the elements of the GUI and sets the XP member
 * variables from them
 */
void  AP_UnixDialog_Lists::setXPFromLocal(void)
{
	// Read m_newListType

	setListTypeFromWidget();
//
// Read out GUI stuff in the customize box and load their values into the member
// variables.
//
	_gatherData();
//
// Now read the toggle button state and set the member variables from them
//
	if (GTK_TOGGLE_BUTTON (m_wStartNewList)->active)
	{
		setbStartNewList(true);
		setbApplyToCurrent(false);
		setbResumeList(false);
	}
	else if (GTK_TOGGLE_BUTTON (m_wApplyCurrent)->active)
	{
		setbStartNewList(false);
		setbApplyToCurrent(true);
		setbResumeList(false);
	}
	else if (GTK_TOGGLE_BUTTON (m_wStartSubList)->active)
	{
		setbStartNewList(false);
		setbApplyToCurrent(false);
		setbResumeList(true);
	}
}


void  AP_UnixDialog_Lists::applyClicked(void)
{
	setXPFromLocal();
	previewExposed();
	Apply();
	if(isModal())
	{
		setAnswer(AP_Dialog_Lists::a_OK);
		gtk_main_quit();
	}
}

void  AP_UnixDialog_Lists::customChanged(void)
{
	fillUncustomizedValues();
	loadXPDataIntoLocal();
}


void AP_UnixDialog_Lists::updateFromDocument(void)
{
	PopulateDialogData();
	_setRadioButtonLabels();
	setNewListType(getDocListType());
	loadXPDataIntoLocal();
}

void AP_UnixDialog_Lists::updateDialog(void)
{
	if(!isDirty())
	{
	        updateFromDocument();
	}
	else
	{
		setXPFromLocal();
	}
}

void AP_UnixDialog_Lists::setAllSensitivity(void)
{
	PopulateDialogData();
	if(getisListAtPoint())
	{
	}
}

GtkWidget * AP_UnixDialog_Lists::_constructWindow (void)
{
	GtkWidget *contents;
	GtkWidget *hbuttonbox1;
	GtkWidget *hseparator1;
	GtkWidget *vbox1;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_wMainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), getWindowName());
	gtk_window_set_policy (GTK_WINDOW (m_wMainWindow), FALSE, FALSE, FALSE);

	ConstructWindowName(); // why don't call this function constructWindowName?

	vbox1 = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (m_wMainWindow), vbox1);

	contents = _constructWindowContents();
	gtk_widget_show (contents);
	gtk_box_pack_start (GTK_BOX (vbox1), contents, FALSE, TRUE, 0);

	//------------------------------------------------------------------
	// Do the stuff on the bottom that should be gnomized
	//------------------------------------------------------------------

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, FALSE, TRUE, 0);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox1), 4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_END);

	if(!isModal())
		m_wApply = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Apply));
	else
		m_wApply = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_OK));

	gtk_widget_show (m_wApply);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), m_wApply);
	GTK_WIDGET_SET_FLAGS (m_wApply, GTK_CAN_DEFAULT);
	if(!isModal())
		m_wClose = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Close));
	else
		m_wClose = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Cancel));

	gtk_widget_show (m_wClose);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), m_wClose);
	GTK_WIDGET_SET_FLAGS (m_wClose, GTK_CAN_DEFAULT);

	gtk_widget_grab_default (m_wClose);
	_connectSignals ();

	return (m_wMainWindow);
}

void AP_UnixDialog_Lists::_fillFontMenu(GtkWidget* menu)
{
	GtkWidget* glade_menuitem;
	GList* m_glFonts;
	gint i;
	gint nfonts;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_glFonts = _getGlistFonts();
	nfonts = g_list_length(m_glFonts);

	// somebody can explain me (jca) the reason of these 6 lines?
	// it seems to me like if we were inserting two times the current font in the menu
	// Sevior: This is not a valid font name. However if you select
        // this you get whatever font is in the document at the point the
        // list is inserted.

	glade_menuitem = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Current_Font));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data (GTK_OBJECT (glade_menuitem), GINT_TO_POINTER (0));
	gtk_menu_append (GTK_MENU (menu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_valueChanged), this);
	for(i = 0; i < nfonts; i++)
	{
		glade_menuitem = gtk_menu_item_new_with_label ((gchar *) g_list_nth_data (m_glFonts, i));
		gtk_widget_show (glade_menuitem);
		gtk_object_set_user_data (GTK_OBJECT (glade_menuitem), GINT_TO_POINTER (i + 1));
		gtk_menu_append (GTK_MENU (menu), glade_menuitem);
		g_signal_connect (G_OBJECT (glade_menuitem), "activate",
				  G_CALLBACK (s_valueChanged), this);
	}

	  //gtk_option_menu_set_history (GTK_OPTION_MENU (menu), 0);
}

GtkWidget *AP_UnixDialog_Lists::_constructWindowContents (void)
{
	GtkWidget *vbox2;
	GtkWidget *hbox2;
	GtkWidget *vbox4;
	GtkWidget *table1;
	GtkWidget *style_om;
	GtkWidget *type_om;
	GtkWidget *type_om_menu;
	GtkWidget *type_lb;
	GtkWidget *style_lb;
	GtkWidget *customized_cb;
	GtkWidget *frame1;
	GtkWidget *table2;
	GtkWidget *font_om;
	GtkWidget *font_om_menu;
	GtkWidget *format_en;
	GtkWidget *decimal_en;
	GtkObject *start_sb_adj;
	GtkWidget *start_sb;
	GtkObject *text_align_sb_adj;
	GtkWidget *text_align_sb;
	GtkObject *label_align_sb_adj;
	GtkWidget *label_align_sb;
	GtkWidget *format_lb;
	GtkWidget *font_lb;
	GtkWidget *delimiter_lb;
	GtkWidget *start_at_lb;
	GtkWidget *text_align_lb;
	GtkWidget *label_align_lb;
	GtkWidget *vbox3;
	GtkWidget *preview_lb;
	GtkWidget *hbox1;
	GSList *action_group = NULL;
	GtkWidget *start_list_rb;
	GtkWidget *apply_list_rb;
	GtkWidget *resume_list_rb;
	GtkWidget *preview_area;
	GtkWidget *preview_frame;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 8);

	hbox2 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, TRUE, TRUE, 0);

	vbox4 = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vbox4);
	gtk_box_pack_start (GTK_BOX (hbox2), vbox4, FALSE, TRUE, 0);

	table1 = gtk_table_new (3, 2, FALSE);
	gtk_widget_show (table1);
	gtk_box_pack_start (GTK_BOX (vbox4), table1, FALSE, TRUE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 4);

	style_om = gtk_option_menu_new ();
	gtk_widget_show (style_om);
	gtk_table_attach (GTK_TABLE (table1), style_om, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	m_wListStyleNone_menu = gtk_menu_new();
	_fillNoneStyleMenu(m_wListStyleNone_menu);
	m_wListStyleNumbered_menu = gtk_menu_new ();
	_fillNumberedStyleMenu(m_wListStyleNumbered_menu);
	m_wListStyleBulleted_menu = gtk_menu_new();
	_fillBulletedStyleMenu(m_wListStyleBulleted_menu);

	// This is the default list. Change if the list style changes
	//
	m_wListStyle_menu = m_wListStyleNumbered_menu;

	gtk_option_menu_set_menu (GTK_OPTION_MENU (style_om), m_wListStyleNumbered_menu);

	type_om = gtk_option_menu_new ();
	gtk_widget_show (type_om);
	gtk_table_attach (GTK_TABLE (table1), type_om, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	type_om_menu = gtk_menu_new ();
	GtkWidget * menu_none = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Type_none));
	m_wMenu_None = menu_none;
	gtk_object_set_user_data(GTK_OBJECT(menu_none),GINT_TO_POINTER(0));
	gtk_widget_show (menu_none);
	gtk_menu_append (GTK_MENU (type_om_menu), menu_none);
	GtkWidget * menu_bull  = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Type_bullet));
	gtk_object_set_user_data(GTK_OBJECT(menu_bull),GINT_TO_POINTER(1));
	gtk_widget_show (menu_bull);
	m_wMenu_Bull = menu_bull;
	gtk_menu_append (GTK_MENU (type_om_menu), menu_bull);
	GtkWidget * menu_num  = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_Type_numbered));
	gtk_object_set_user_data(GTK_OBJECT(menu_num),GINT_TO_POINTER(2));
	gtk_widget_show (menu_num);
	m_wMenu_Num = menu_num;
	gtk_menu_append (GTK_MENU (type_om_menu), menu_num);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (type_om), type_om_menu);

	// This is how we set the active element of an option menu!!
	//
	gtk_option_menu_set_history (GTK_OPTION_MENU (type_om), 2);
	gtk_widget_set_events(type_om, GDK_ALL_EVENTS_MASK);

	type_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Type));
	gtk_widget_show (type_lb);
	gtk_table_attach (GTK_TABLE (table1), type_lb, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (type_lb), 0, 0.5);

	style_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Style));
	gtk_widget_show (style_lb);
	gtk_table_attach (GTK_TABLE (table1), style_lb, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (style_lb), 0, 0.5);

	customized_cb = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Lists_SetDefault));
	gtk_widget_show (customized_cb);
	gtk_table_attach (GTK_TABLE (table1), customized_cb, 0, 2, 2, 3,
					  (GtkAttachOptions) (GTK_SHRINK),
					  (GtkAttachOptions) (0), 0, 0);

	frame1 = gtk_frame_new (NULL);
	//gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox4), frame1, TRUE, TRUE, 0);

	table2 = gtk_table_new (6, 2, FALSE);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (frame1), table2);
	gtk_container_set_border_width (GTK_CONTAINER (table2), 4);
	gtk_widget_set_sensitive (table2, TRUE);
	gtk_table_set_row_spacings (GTK_TABLE (table2), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table2), 4);

	font_om = gtk_option_menu_new ();
	gtk_widget_show (font_om);
	gtk_table_attach (GTK_TABLE (table2), font_om, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	font_om_menu = gtk_menu_new ();
	gtk_widget_show(font_om_menu);
	_fillFontMenu(font_om_menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (font_om), font_om_menu);

	format_en = gtk_entry_new_with_max_length (20);
	gtk_widget_show (format_en);
	gtk_table_attach (GTK_TABLE (table2), format_en, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_text (GTK_ENTRY (format_en), "%L");

	decimal_en = gtk_entry_new ();
	gtk_widget_show (decimal_en);
	gtk_table_attach (GTK_TABLE (table2), decimal_en, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_text (GTK_ENTRY (format_en), "");

	start_sb_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
	start_sb = gtk_spin_button_new (GTK_ADJUSTMENT (start_sb_adj), 1, 0);
	gtk_widget_show (start_sb);
	gtk_table_attach (GTK_TABLE (table2), start_sb, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	text_align_sb_adj = gtk_adjustment_new (0.25, 0, 10, 0.01, 0.2, 1);
	text_align_sb = gtk_spin_button_new (GTK_ADJUSTMENT (text_align_sb_adj), 0.05, 2);
	gtk_widget_show (text_align_sb);
	gtk_table_attach (GTK_TABLE (table2), text_align_sb, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (text_align_sb), TRUE);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (text_align_sb), TRUE);

	label_align_sb_adj = gtk_adjustment_new (0, 0, 10, 0.01, 0.2, 1);
	label_align_sb = gtk_spin_button_new (GTK_ADJUSTMENT (label_align_sb_adj), 0.05, 2);
	gtk_widget_show (label_align_sb);
	gtk_table_attach (GTK_TABLE (table2), label_align_sb, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (label_align_sb), TRUE);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (label_align_sb), TRUE);

	format_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Format));
	gtk_widget_show (format_lb);
	gtk_table_attach (GTK_TABLE (table2), format_lb, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (format_lb), 0.0, 0.5);

	font_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Font));
	gtk_widget_show (font_lb);
	gtk_table_attach (GTK_TABLE (table2), font_lb, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (font_lb), 0.0, 0.5);

	delimiter_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_DelimiterString));
	gtk_widget_show (delimiter_lb);
	gtk_table_attach (GTK_TABLE (table2), delimiter_lb, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (delimiter_lb), 0.0, 0.5);

	start_at_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Start));
	gtk_widget_show (start_at_lb);
	gtk_table_attach (GTK_TABLE (table2), start_at_lb, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (start_at_lb), 0.0, 0.5);

	text_align_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Align));
	gtk_widget_show (text_align_lb);
	gtk_table_attach (GTK_TABLE (table2), text_align_lb, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (text_align_lb), 0.0, 0.5);

	label_align_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Indent));
	gtk_widget_show (label_align_lb);
	gtk_table_attach (GTK_TABLE (table2), label_align_lb, 0, 1, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label_align_lb), 0.0, 0.5);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_box_pack_start (GTK_BOX (hbox2), vbox3, TRUE, TRUE, 0);

	preview_lb = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Lists_Preview));
	gtk_widget_show (preview_lb);
	gtk_box_pack_start (GTK_BOX (vbox3), preview_lb, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (preview_lb), 0.0, 0.5);

	preview_frame = gtk_frame_new (NULL);
	gtk_widget_show (preview_frame);
	gtk_box_pack_start (GTK_BOX (vbox3), preview_frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (preview_frame), 3);
	gtk_frame_set_shadow_type (GTK_FRAME (preview_frame), GTK_SHADOW_IN);

	preview_area = createDrawingArea ();
        gtk_drawing_area_size (GTK_DRAWING_AREA(preview_area),180,225);
	//	gtk_widget_set_usize(preview_area, 180, 225);
	gtk_widget_show (preview_area);
	gtk_container_add (GTK_CONTAINER (preview_frame), preview_area);

	hbox1 = gtk_hbox_new (FALSE, 0);
	if(!isModal())
		gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, FALSE, 0);

	start_list_rb = gtk_radio_button_new_with_label (action_group, pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
	action_group = gtk_radio_button_group (GTK_RADIO_BUTTON (start_list_rb));
	if(!isModal())
		gtk_widget_show (start_list_rb);
	gtk_box_pack_start (GTK_BOX (hbox1), start_list_rb, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (start_list_rb), TRUE);

	apply_list_rb = gtk_radio_button_new_with_label (action_group, pSS->getValue(AP_STRING_ID_DLG_Lists_Apply_Current));
	action_group = gtk_radio_button_group (GTK_RADIO_BUTTON (apply_list_rb));
	if(!isModal())
		gtk_widget_show (apply_list_rb);
	gtk_box_pack_start (GTK_BOX (hbox1), apply_list_rb, FALSE, FALSE, 0);

	resume_list_rb = gtk_radio_button_new_with_label (action_group, pSS->getValue(AP_STRING_ID_DLG_Lists_Resume));
	action_group = gtk_radio_button_group (GTK_RADIO_BUTTON (resume_list_rb));
	if(!isModal())
		gtk_widget_show (resume_list_rb);
	gtk_box_pack_start (GTK_BOX (hbox1), resume_list_rb, FALSE, FALSE, 0);

	// Save useful widgets in member variables
	m_wContents = vbox2;
	m_wStartNewList = start_list_rb;
	m_wStartNew_label = GTK_BIN(start_list_rb)->child;
	m_wApplyCurrent = apply_list_rb;
	m_wStartSubList = resume_list_rb;
	m_wStartSub_label = GTK_BIN(resume_list_rb)->child;
	m_wRadioGroup = action_group;
	m_wPreviewArea = preview_area;
	m_wDelimEntry = format_en;
	m_oAlignList_adj = text_align_sb_adj;
	m_wAlignListSpin = text_align_sb;
	m_oIndentAlign_adj = label_align_sb_adj;
	m_wIndentAlignSpin = label_align_sb;
	m_wDecimalEntry = decimal_en;
	m_oStartSpin_adj = start_sb_adj;
	m_wStartSpin = start_sb;

	m_wFontOptions = font_om;
	m_wFontOptions_menu = font_om_menu;
	m_wCustomFrame = frame1;
	m_wCustomLabel = customized_cb;
	m_wCustomTable = table2;
	m_wListStyleBox = style_om;
	m_wListTypeBox = type_om;
	m_wListType_menu = m_wListStyleNumbered_menu;

	// Start by hiding the Custom frame
	//
	//	gtk_widget_hide(m_wCustomFrame);
	gtk_widget_show(m_wCustomFrame);

	setbisCustomized(false);

	return m_wContents;
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
	UT_Vector * list = unixapp->getFontManager()->getAllFonts();
	UT_uint32 count = list->size();

	GList *glFonts = NULL;
	gchar currentfont[50] = "\0";
	gchar * nextfont;

	for (UT_uint32 i = 0; i < count; i++)
	{
		XAP_UnixFont * pFont = (XAP_UnixFont *)list->getNthItem(i);
		gchar * lgn  = (gchar *) pFont->getName();
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
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);
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
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) LOWERCASE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) UPPERCASE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Roman_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) LOWERROMAN_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Roman_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) UPPERROMAN_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Arabic_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) ARABICNUMBERED_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);
	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Hebrew_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) HEBREW_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);
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
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Dashed_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) DASHED_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Square_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) SQUARE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Triangle_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) TRIANGLE_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Diamond_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) DIAMOND_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Star_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) STAR_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Implies_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) IMPLIES_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Tick_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) TICK_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);


	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Box_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) BOX_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);



	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Hand_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) HAND_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

	glade_menuitem = gtk_menu_item_new_with_label (
		pSS->getValue(AP_STRING_ID_DLG_Lists_Heart_List));
	gtk_widget_show (glade_menuitem);
	gtk_object_set_user_data(GTK_OBJECT(glade_menuitem),GINT_TO_POINTER(
		(gint) HEART_LIST));
	gtk_menu_append (GTK_MENU (listmenu), glade_menuitem);
	g_signal_connect (G_OBJECT (glade_menuitem), "activate",
						G_CALLBACK (s_typeChanged), this);

}

void AP_UnixDialog_Lists::_setRadioButtonLabels(void)
{
	//	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	PopulateDialogData();
	// Button 0 is Start New List, button 2 is resume list
	gtk_label_set_text( GTK_LABEL(m_wStartNew_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Start_New));
	gtk_label_set_text( GTK_LABEL(m_wStartSub_label), pSS->getValue(AP_STRING_ID_DLG_Lists_Resume));
}

void AP_UnixDialog_Lists::_connectSignals(void)
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

	g_signal_connect (G_OBJECT (m_wApply), "clicked",
						G_CALLBACK (s_applyClicked), this);
	g_signal_connect (G_OBJECT (m_wClose), "clicked",
						G_CALLBACK (s_closeClicked), this);
	g_signal_connect (G_OBJECT (m_wCustomLabel), "clicked",
						G_CALLBACK (s_customChanged), this);
	g_signal_connect (G_OBJECT (m_wMenu_None), "activate",
						G_CALLBACK (s_styleChangedNone), this);
	g_signal_connect (G_OBJECT (m_wMenu_Bull), "activate",
						G_CALLBACK (s_styleChangedBullet), this);
	g_signal_connect (G_OBJECT (m_wMenu_Num), "activate",
						G_CALLBACK (s_styleChangedNumbered), this);
        g_signal_connect (G_OBJECT (m_oStartSpin_adj), "value_changed",
						G_CALLBACK (s_valueChanged), this);
	m_iDecimalEntryID = g_signal_connect (G_OBJECT (m_wDecimalEntry), "changed",
										 G_CALLBACK (s_valueChanged), this);
	m_iAlignListSpinID = g_signal_connect (G_OBJECT (m_oAlignList_adj), "value_changed",
						G_CALLBACK (s_valueChanged), this);
	m_iIndentAlignSpinID = g_signal_connect (G_OBJECT (m_oIndentAlign_adj), "value_changed",
						G_CALLBACK (s_valueChanged), this);
	m_iDelimEntryID = g_signal_connect (G_OBJECT (GTK_ENTRY(m_wDelimEntry)), "changed",
										  G_CALLBACK (s_valueChanged), this);

	m_iStyleBoxID = g_signal_connect (G_OBJECT(m_wListStyleBox),
					    "configure_event",
					    G_CALLBACK (s_typeChanged),
					    this);
	// the expose event of the preview
	g_signal_connect(G_OBJECT(m_wPreviewArea),
					   "expose_event",
					   G_CALLBACK(s_preview_exposed),
					   (gpointer) this);


	g_signal_connect_after(G_OBJECT(m_wMainWindow),
							 "expose_event",
		     				 G_CALLBACK(s_window_exposed),
							 (gpointer) this);

}

void AP_UnixDialog_Lists::loadXPDataIntoLocal(void)
{
	//
	// This function reads the various memeber variables and loads them into
	// into the dialog variables.
	//

  //
  // Block all signals while setting these things
  //
	g_signal_handler_block(  G_OBJECT(m_oAlignList_adj), m_iAlignListSpinID);
	g_signal_handler_block(  G_OBJECT(m_oIndentAlign_adj), m_iIndentAlignSpinID);

	g_signal_handler_block(  G_OBJECT(m_wDecimalEntry), m_iDecimalEntryID);
	g_signal_handler_block(  G_OBJECT(m_wDelimEntry), m_iDelimEntryID );
	gint i;
	//
	// HACK to effectively block an update during this method
	//
	m_bDontUpdate = true;

	UT_DEBUGMSG(("loadXP newListType = %d \n",getNewListType()));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wAlignListSpin),getfAlign());
	float indent = getfAlign() + getfIndent();
	gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin),indent);
	if( (getfIndent() + getfAlign()) < 0.0)
	{
		setfIndent( - getfAlign());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), 0.0);

	}
	//
	// Code to work out which is active Font
	//
	if(strcmp((char *) getFont(),"NULL") == 0 )
	{
		gtk_option_menu_set_history (GTK_OPTION_MENU (m_wFontOptions), 0 );
	}
	else
	{
		for(i=0; i < (gint) g_list_length(m_glFonts);i++)
		{
			if(strcmp(getFont(),(char *) g_list_nth_data(m_glFonts,i)) == 0)
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wStartSpin),(float) getiStartValue());

	gtk_entry_set_text( GTK_ENTRY(m_wDecimalEntry), (const gchar *) getDecimal());
	gtk_entry_set_text( GTK_ENTRY(m_wDelimEntry), (const gchar *) getDelim());

	//
	// Now set the list type and style
	List_Type save = getNewListType();
	if(getNewListType() == NOT_A_LIST)
	{
		styleChanged(0);
		setNewListType(save);
		gtk_option_menu_set_history( GTK_OPTION_MENU (m_wListStyleBox),(gint) NOT_A_LIST);
	}
	else if(getNewListType() >= BULLETED_LIST)
	{
		styleChanged(1);
		setNewListType(save);
		gtk_option_menu_set_history(  GTK_OPTION_MENU (m_wListTypeBox),1);
		gtk_option_menu_set_history( GTK_OPTION_MENU (m_wListStyleBox),(gint) (getNewListType() - BULLETED_LIST));
	}
	else
	{
		styleChanged(2);
	    setNewListType(save);
		gtk_option_menu_set_history(  GTK_OPTION_MENU (m_wListTypeBox),2);
		gtk_option_menu_set_history( GTK_OPTION_MENU (m_wListStyleBox),(gint) getNewListType());
	}

	//
	// turn signals back on
	g_signal_handler_unblock(  G_OBJECT(m_oAlignList_adj), m_iAlignListSpinID);
	g_signal_handler_unblock(  G_OBJECT(m_oIndentAlign_adj), m_iIndentAlignSpinID);
	g_signal_handler_unblock(  G_OBJECT(m_wDelimEntry), m_iDelimEntryID );
	g_signal_handler_unblock(  G_OBJECT(m_wDecimalEntry), m_iDecimalEntryID);
	//
	// HACK to allow an update during this method
	//
	m_bDontUpdate = false;
}

bool    AP_UnixDialog_Lists::dontUpdate(void)
{
        return m_bDontUpdate;
}

/*!
 * This method reads the various elements in the Customize box and loads
 * the XP member variables with them
 */
void AP_UnixDialog_Lists::_gatherData(void)
{
	UT_sint32 maxWidth = getBlock()->getFirstContainer()->getContainer()->getWidth();
//
// screen resolution is 100 pixels/inch
//
	float fmaxWidthIN = ((float) maxWidth/ 100.) - 0.6;
	setiLevel(1);
	float f =gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(m_wAlignListSpin));
	if(f >   fmaxWidthIN)
	{
		f = fmaxWidthIN;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wAlignListSpin), f);
	}
	setfAlign(f);
	float indent = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON( m_wIndentAlignSpin));
	if((indent - f) > fmaxWidthIN )
	{
		indent = fmaxWidthIN + f;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), indent);
	}
	setfIndent(indent - getfAlign());
	if( (getfIndent() + getfAlign()) < 0.0)
	{
		setfIndent(- getfAlign());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin), 0.0);

	}
	GtkWidget * wfont = gtk_menu_get_active(GTK_MENU(m_wFontOptions_menu));
	gint ifont =  GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(wfont)));
	if(ifont == 0)
	{
		copyCharToFont("NULL");
	}
	else
	{
		copyCharToFont( (const char *)  g_list_nth_data(m_glFonts, ifont-1));
	}
	const gchar * pszDec = gtk_entry_get_text( GTK_ENTRY(m_wDecimalEntry));
	copyCharToDecimal( (const char *) pszDec);
	setiStartValue(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wStartSpin)));
	const gchar * pszDel = gtk_entry_get_text( GTK_ENTRY(m_wDelimEntry));
	copyCharToDelim((const char *) pszDel);
}









