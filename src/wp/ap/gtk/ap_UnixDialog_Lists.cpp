/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_GtkSignalBlocker.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_UnixDialog_Lists.h"
#include "fp_Line.h"
#include "fp_Column.h"

#include "gr_UnixCairoGraphics.h"

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
	m_bAutoUpdate_happening_now = false;
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

static void s_customChanged (GtkWidget * /*widget*/, AP_UnixDialog_Lists * me)
{
  	me->setDirty();
	me->customChanged ();
}

static void s_FoldCheck_changed(GtkWidget * widget, AP_UnixDialog_Lists * me)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		UT_DEBUGMSG(("Doing s_FoldCheck_changed \n"));
		UT_UTF8String sLevel = static_cast<char *> 
			(g_object_get_data(G_OBJECT(widget),"level"));
		UT_sint32 iLevel = atoi(sLevel.utf8_str());
		me->setFoldLevel(iLevel, true);
	}
}

static void s_styleChanged(GtkWidget * w, AP_UnixDialog_Lists * me)
{
	GtkComboBox * combo = GTK_COMBO_BOX(w);
	gint idx = gtk_combo_box_get_active(combo);
	switch(idx) {
	case 0:
		me->setDirty();
		me->styleChanged ( 0 );
		break;
	case 1:
		me->setDirty();
		me->fillUncustomizedValues(); // Use defaults to start.
		me->styleChanged ( 1 );
		break;
	case 2:
		me->setDirty();
		me->fillUncustomizedValues(); // Use defaults to start.
		me->styleChanged ( 2 );
		break;
	default:
		break;
	}
}



/*!
 * User has changed their list type selection.
 */
static void s_typeChanged (GtkWidget * /*widget*/, AP_UnixDialog_Lists * me)
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
static void s_valueChanged (GtkWidget * /*widget*/, AP_UnixDialog_Lists * me)
{
	if(me->dontUpdate())
		return;
  	me->setDirty();
	me->setXPFromLocal(); // Update member Variables
	me->previewExposed();
}


static void s_applyClicked (GtkWidget * /*widget*/, AP_UnixDialog_Lists * me)
{
	me->applyClicked();
}

static void s_closeClicked (GtkWidget * /*widget*/, AP_UnixDialog_Lists * me)
{
	me->closeClicked();
}

static gboolean s_preview_draw(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Lists * me)
{
	UT_DEBUG_ONLY_ARG(widget);
	UT_ASSERT(widget && me);
	me->previewExposed();
	return FALSE;
}

static gboolean s_update (int /*unused*/)
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

void AP_UnixDialog_Lists::closeClicked(void)
{
	setAnswer(AP_Dialog_Lists::a_QUIT);	
	abiDestroyWidget(m_wMainWindow); // emit the correct signals
}

void AP_UnixDialog_Lists::runModal( XAP_Frame * pFrame)
{
	FL_ListType  savedListType;
	setModal();
	
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	
	clearDirty();

	// Populate the dialog
	m_bDontUpdate = false;
	loadXPDataIntoLocal();

	// Need this to stop this being stomped during the contruction of preview widget
	savedListType = getNewListType();

	// *** this is how we add the gc for Lists Preview ***

	// Now Display the dialog, so m_wPreviewArea->window exists
	gtk_widget_show(m_wMainWindow);	
	UT_ASSERT(m_wPreviewArea && gtk_widget_get_window(m_wPreviewArea));

	// make a new Unix GC
	GR_UnixCairoAllocInfo ai(m_wPreviewArea);
	m_pPreviewWidget =
	    (GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	// let the widget materialize
	GtkAllocation allocation;
	gtk_widget_get_allocation(m_wPreviewArea, &allocation);
	_createPreviewFromGC(m_pPreviewWidget,
						 static_cast<UT_uint32>(allocation.width),
						 static_cast<UT_uint32>(allocation.height));

	// Restore our value
	setNewListType(savedListType);
	
	gint response;
	do {
		response = abiRunModalDialog (GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false);		
	} while (response == BUTTON_RESET);
	AP_Dialog_Lists::tAnswer res = getAnswer();
	m_glFonts.clear();
	abiDestroyWidget ( mainWindow ) ;
	setAnswer(res);
	DELETEP (m_pPreviewWidget);
}


void AP_UnixDialog_Lists::runModeless (XAP_Frame * pFrame)
{
	static std::pointer_to_unary_function<int, gboolean> s_update_fun = std::ptr_fun(s_update);
	_constructWindow ();
	UT_ASSERT (m_wMainWindow);
	clearDirty();

	abiSetupModelessDialog(GTK_DIALOG(m_wMainWindow), pFrame, this, BUTTON_APPLY);
	connectFocusModelessOther (GTK_WIDGET (m_wMainWindow), m_pApp, &s_update_fun);

	// Populate the dialog
	updateDialog();
	m_bDontUpdate = false;

	// Now Display the dialog
	gtk_widget_show(m_wMainWindow);

	// *** this is how we add the gc for Lists Preview ***

	UT_ASSERT(m_wPreviewArea && gtk_widget_get_window(m_wPreviewArea));

	// make a new Unix GC
	GR_UnixCairoAllocInfo ai(gtk_widget_get_window(m_wPreviewArea));
	m_pPreviewWidget =
	    (GR_CairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	// let the widget materialize

	GtkAllocation allocation;
	gtk_widget_get_allocation(m_wPreviewArea, &allocation);
	_createPreviewFromGC(m_pPreviewWidget,
						 static_cast<UT_uint32>(allocation.width),
						 static_cast<UT_uint32>(allocation.height));

	// Next construct a timer for auto-updating the dialog
	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists,this);
	m_bDestroy_says_stopupdating = false;

	// OK fire up the auto-updater for 0.5 secs

	m_pAutoUpdateLists->set(500);
}


void AP_UnixDialog_Lists::autoupdateLists(UT_Worker * pWorker)
{
	UT_ASSERT(pWorker);
	// this is a static callback method and does not have a 'this' pointer.
	AP_UnixDialog_Lists * pDialog =  static_cast<AP_UnixDialog_Lists *>(pWorker->getInstanceData());
	// Handshaking code. Plus only update if something in the document
	// changed.

	AP_Dialog_Lists * pList = static_cast<AP_Dialog_Lists *>(pDialog);

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
	}
	else
	{
		m_bDestroy_says_stopupdating = true;
		m_pAutoUpdateLists->stop();
		setAnswer(AP_Dialog_Lists::a_CLOSE);

		m_glFonts.clear();
		modeless_cleanup();
		abiDestroyWidget(m_wMainWindow);
		m_wMainWindow = NULL;
		DELETEP(m_pAutoUpdateLists);
		DELETEP (m_pPreviewWidget);
	}
}

/*!
 * Set the Fold level from the XP layer.
 */
void AP_UnixDialog_Lists::setFoldLevelInGUI(void)
{
	setFoldLevel(getCurrentFold(),true);
}

/*!
 * Set the Fold Level in the current List structure.
 */
void AP_UnixDialog_Lists::setFoldLevel(UT_sint32 iLevel, bool bSet)
{
	UT_sint32 count = m_vecFoldCheck.getItemCount();
	if(iLevel >= count)
	{
		return;
	}
	GtkWidget * wF = NULL;
	UT_uint32 ID =0;
	if(!bSet)
	{
		wF = m_vecFoldCheck.getNthItem(0);
		ID = m_vecFoldID.getNthItem(0);
		XAP_GtkSignalBlocker b2(G_OBJECT(wF),ID);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wF),TRUE);
		setCurrentFold(0);
	}
	else
	{
		wF = m_vecFoldCheck.getNthItem(iLevel);
		ID = m_vecFoldID.getNthItem(iLevel);
		{
			XAP_GtkSignalBlocker b1(G_OBJECT(wF),ID);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wF),TRUE);
		}
		setCurrentFold(iLevel);
	}
}

bool AP_UnixDialog_Lists::isPageLists(void)
{
	if(isModal())
	{
		return true;
	}
	bool isPage =  (gtk_notebook_get_current_page(GTK_NOTEBOOK(m_wContents)) == m_iPageLists);
	return isPage;
}

void AP_UnixDialog_Lists::activate (void)
{
	UT_ASSERT (m_wMainWindow);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_wMainWindow), getWindowName());
	m_bDontUpdate = false;
	updateDialog();
	gdk_window_raise (gtk_widget_get_window(m_wMainWindow));
}

void AP_UnixDialog_Lists::notifyActiveFrame(XAP_Frame * /*pFrame*/)
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
	if(type == 0)
	{
		m_wListStyle_menu = m_wListStyleNone_menu;

		gtk_combo_box_set_model(m_wListStyleBox,
								GTK_TREE_MODEL(m_wListStyleNone_menu.obj()));

		gtk_combo_box_set_active(m_wListTypeBox, 0);
		setNewListType(NOT_A_LIST);
		gtk_widget_set_sensitive(GTK_WIDGET(m_wFontOptions), false);
		gtk_widget_set_sensitive(m_wStartSpin, false);
		gtk_widget_set_sensitive(m_wDelimEntry, false);
		gtk_widget_set_sensitive(m_wDecimalEntry, false);		
	}
	else if(type == 1)
	{
		m_wListStyle_menu = m_wListStyleBulleted_menu;

		gtk_combo_box_set_model(m_wListStyleBox,
								GTK_TREE_MODEL(m_wListStyleBulleted_menu.obj()));
		gtk_combo_box_set_active(m_wListTypeBox, 1);
		setNewListType(BULLETED_LIST);
		gtk_widget_set_sensitive(GTK_WIDGET(m_wFontOptions), true);
		gtk_widget_set_sensitive(m_wStartSpin, false);
		gtk_widget_set_sensitive(m_wDelimEntry, false);
		gtk_widget_set_sensitive(m_wDecimalEntry, false);		
	}
	else if(type == 2)
	{
		//  gtk_widget_destroy(GTK_WIDGET(m_wListStyleNumbered_menu));
//	  	m_wListStyleNumbered_menu = gtk_menu_new();
		m_wListStyle_menu = m_wListStyleNumbered_menu;
//		_fillNumberedStyleMenu(m_wListStyleNumbered_menu);

		// Block events during this manual change

		gtk_combo_box_set_model (m_wListStyleBox,
								 GTK_TREE_MODEL(m_wListStyleNumbered_menu.obj()));
		gtk_combo_box_set_active(m_wListTypeBox, 2);
		setNewListType(NUMBERED_LIST);
		gtk_widget_set_sensitive(GTK_WIDGET(m_wFontOptions), true);
		gtk_widget_set_sensitive(m_wStartSpin, true);
		gtk_widget_set_sensitive(m_wDelimEntry, true);
		gtk_widget_set_sensitive(m_wDecimalEntry, true);		
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
	GtkTreeIter iter;
	GtkTreeModel *model;
	gtk_combo_box_get_active_iter(m_wListStyleBox, &iter);
	model = gtk_combo_box_get_model(m_wListStyleBox);
	gint type;
	gtk_tree_model_get(model, &iter, 1, &type, -1);
	setNewListType((FL_ListType)type);
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
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_wStartNewList)))
	{
		setbStartNewList(true);
		setbApplyToCurrent(false);
		setbResumeList(false);
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_wApplyCurrent)))
	{
		setbStartNewList(false);
		setbApplyToCurrent(true);
		setbResumeList(false);
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_wStartSubList)))
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

GtkWidget * AP_UnixDialog_Lists::_constructWindow(void)
{
	GtkWidget *contents;
	GtkWidget *vbox1;

	ConstructWindowName();
	m_wMainWindow = abiDialogNew ( "list dialog", TRUE, getWindowName() );	
	vbox1 = gtk_dialog_get_content_area(GTK_DIALOG(m_wMainWindow));

	contents = _constructWindowContents();
	gtk_widget_show (contents);
	gtk_box_pack_start (GTK_BOX (vbox1), contents, FALSE, TRUE, 0);

	const XAP_StringSet* pSS = XAP_App::getApp()->getStringSet();
	std::string s;
	if(!isModal())
	{
		pSS->getValueUTF8(XAP_STRING_ID_DLG_Close, s);
		m_wClose = abiAddButton ( GTK_DIALOG(m_wMainWindow), s, BUTTON_CLOSE ) ;
		pSS->getValueUTF8(XAP_STRING_ID_DLG_Apply, s);
		m_wApply = abiAddButton ( GTK_DIALOG(m_wMainWindow), s, BUTTON_APPLY ) ;
	}
	else
	{
		pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
		m_wApply = abiAddButton ( GTK_DIALOG(m_wMainWindow), s, BUTTON_OK ) ;
		pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
		m_wClose = abiAddButton ( GTK_DIALOG(m_wMainWindow), s, BUTTON_CANCEL ) ;
	}

	gtk_widget_grab_default (m_wClose);
	_connectSignals ();

	return (m_wMainWindow);
}

static void addToStore(GtkListStore * store, const XAP_StringSet * pSS,
					   int stringID, int itemID)
{
	GtkTreeIter iter;
	std::string s;
	pSS->getValueUTF8(stringID, s);
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, s.c_str(),
					   1, itemID, -1);

}

void AP_UnixDialog_Lists::_fillFontMenu(GtkListStore* store)
{
	gint i;
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	_getGlistFonts(m_glFonts);

	addToStore(store, pSS, AP_STRING_ID_DLG_Lists_Current_Font,
			   0);

    i = 1;
	for(std::vector<std::string>::const_iterator iter = m_glFonts.begin();
        iter != m_glFonts.end(); ++iter) 
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(store, &treeiter);
        // Here, the lifespan of the string *should* be longer than
        // the store. One more case were I wish we used Gtkmm.
		gtk_list_store_set(store, &treeiter, 
						   0, iter->c_str(), 1, i, -1);
        i++;
	}
}

GtkWidget *AP_UnixDialog_Lists::_constructWindowContents (void)
{
	GtkWidget *list_grid;
	GtkWidget *grid1;
	GtkWidget *grid2;
	GtkWidget *grid3;
	GtkWidget *hbox1;
	GtkWidget *style_om;
	GtkWidget *type_om;
	GtkWidget *type_lb;
	GtkWidget *style_lb;
	GtkWidget *customized_cb;
	GtkComboBox *font_om;
	GtkListStore *font_om_menu;
	GtkWidget *format_en;
	GtkWidget *decimal_en;
	GtkAdjustment *start_sb_adj;
	GtkWidget *start_sb;
	GtkAdjustment *text_align_sb_adj;
	GtkWidget *text_align_sb;
	GtkAdjustment *label_align_sb_adj;
	GtkWidget *label_align_sb;
	GtkWidget *format_lb;
	GtkWidget *font_lb;
	GtkWidget *delimiter_lb;
	GtkWidget *start_at_lb;
	GtkWidget *text_align_lb;
	GtkWidget *label_align_lb;
	GtkWidget *preview_lb;
	GSList *action_group = NULL;
	GtkWidget *start_list_rb;
	GtkWidget *apply_list_rb;
	GtkWidget *resume_list_rb;
	GtkWidget *preview_area;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	std::string s;
	GtkWidget * wNoteBook = NULL;

	list_grid = gtk_grid_new();
	g_object_set(G_OBJECT(list_grid),
		         "row-spacing", 6,
	             "column-spacing", 12,
	             "border-width", 12,
	             NULL);
	gtk_widget_show(list_grid);
	if(!isModal())
	{

// Note Book creation

		wNoteBook = gtk_notebook_new ();
		gtk_widget_show(wNoteBook);

// Container for the lists
		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_PageProperties,s);
		GtkWidget * lbPageLists = gtk_label_new(s.c_str());
		gtk_widget_show(lbPageLists);
		gtk_notebook_append_page(GTK_NOTEBOOK(wNoteBook),list_grid,lbPageLists);

		m_iPageLists = gtk_notebook_page_num(GTK_NOTEBOOK(wNoteBook),list_grid);

// Container for Text Folding
		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_PageFolding,s);
		GtkWidget * lbPageFolding = gtk_label_new(s.c_str());
		GtkWidget * wFoldingGrid = gtk_grid_new();
		g_object_set(G_OBJECT(wFoldingGrid),
			         "row-spacing", 6,
		             "column-spacing", 12,
		             "border-width", 12,
		             NULL);
		gtk_widget_show(lbPageFolding);
		gtk_widget_show(wFoldingGrid);
		gtk_notebook_append_page(GTK_NOTEBOOK(wNoteBook),wFoldingGrid,lbPageFolding);

		m_iPageFold = gtk_notebook_page_num(GTK_NOTEBOOK(wNoteBook),wFoldingGrid);

// Bold markup
		GtkWidget * lbFoldHeading = gtk_label_new("<b>%s</b>");
		gtk_label_set_use_markup(GTK_LABEL(lbFoldHeading),TRUE);

		localizeLabelMarkup(lbFoldHeading,pSS,AP_STRING_ID_DLG_Lists_FoldingLevelexp);
		gtk_grid_attach(GTK_GRID(wFoldingGrid), lbFoldHeading, 0, 0, 2, 1);
		gtk_widget_show(lbFoldHeading);

		UT_uint32 ID =0;
// RadioButtons
		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_FoldingLevel0,s);
		
		GtkWidget * wF = gtk_radio_button_new_with_label(NULL, s.c_str());
		GSList *wG = gtk_radio_button_get_group(GTK_RADIO_BUTTON(wF));
		g_object_set_data(G_OBJECT(wF),"level",(gpointer)"0");
		ID = g_signal_connect(G_OBJECT(wF),
						  "toggled",
						 G_CALLBACK(s_FoldCheck_changed),
						 (gpointer) this);
		gtk_grid_attach(GTK_GRID(wFoldingGrid), wF, 0, 1, 1, 1);
		gtk_widget_set_margin_start (wF, 18);
		gtk_widget_show(wF);
		m_vecFoldCheck.addItem(wF);
		m_vecFoldID.addItem(ID);

		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_FoldingLevel1,s);
		wF = gtk_radio_button_new_with_label(wG, s.c_str());
		wG = gtk_radio_button_get_group(GTK_RADIO_BUTTON(wF));
		g_object_set_data(G_OBJECT(wF),"level",(gpointer)"1");
		ID = g_signal_connect(G_OBJECT(wF),
						  "toggled",
						 G_CALLBACK(s_FoldCheck_changed),
						 (gpointer) this);
		gtk_grid_attach(GTK_GRID(wFoldingGrid), wF, 0, 2, 1, 1);
		gtk_widget_set_margin_start (wF, 18);
		gtk_widget_show(wF);
		m_vecFoldCheck.addItem(wF);
		m_vecFoldID.addItem(ID);

		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_FoldingLevel2,s);
		wF = gtk_radio_button_new_with_label(wG, s.c_str());
		wG = gtk_radio_button_get_group(GTK_RADIO_BUTTON(wF));
		g_object_set_data(G_OBJECT(wF),"level",(gpointer)"2");
		ID = g_signal_connect(G_OBJECT(wF),
						  "toggled",
						 G_CALLBACK(s_FoldCheck_changed),
						 (gpointer) this);
		gtk_grid_attach(GTK_GRID(wFoldingGrid), wF, 0, 3, 1, 1);
		gtk_widget_set_margin_start (wF, 18);
		gtk_widget_show(wF);
		m_vecFoldCheck.addItem(wF);
		m_vecFoldID.addItem(ID);

		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_FoldingLevel3,s);
		wF = gtk_radio_button_new_with_label(wG, s.c_str());
		wG = gtk_radio_button_get_group(GTK_RADIO_BUTTON(wF));
		g_object_set_data(G_OBJECT(wF),"level",(gpointer)"3");
		ID = g_signal_connect(G_OBJECT(wF),
						  "toggled",
						 G_CALLBACK(s_FoldCheck_changed),
						 (gpointer) this);
		gtk_grid_attach(GTK_GRID(wFoldingGrid), wF, 0, 4, 1, 1);
		gtk_widget_set_margin_start (wF, 18);
		gtk_widget_show(wF);
		m_vecFoldCheck.addItem(wF);
		m_vecFoldID.addItem(ID);

		pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_FoldingLevel4,s);
		wF = gtk_radio_button_new_with_label(wG, s.c_str());
		g_object_set_data(G_OBJECT(wF),"level",(gpointer)"4");
		ID = g_signal_connect(G_OBJECT(wF),
						  "toggled",
						 G_CALLBACK(s_FoldCheck_changed),
						 (gpointer) this);
		gtk_grid_attach(GTK_GRID(wFoldingGrid), wF, 0, 5, 1, 1);
		gtk_widget_set_margin_start (wF, 18);
		gtk_widget_show(wF);
		m_vecFoldCheck.addItem(wF);
		m_vecFoldID.addItem(ID);
		gtk_widget_show(wFoldingGrid);

		gtk_notebook_set_current_page(GTK_NOTEBOOK(wNoteBook),m_iPageLists);
	}

// List Page

	grid1 = gtk_grid_new();
	g_object_set(G_OBJECT(grid1),
	             "row-spacing", 6,
	             "column-spacing", 12,
	             NULL);
	gtk_widget_show(grid1);
	gtk_grid_attach(GTK_GRID(list_grid), grid1, 0, 0, 1, 1);

	style_om = gtk_combo_box_text_new();
	gtk_widget_show (style_om);
	gtk_grid_attach(GTK_GRID(grid1), style_om, 1, 1, 1, 1);

	m_wListStyleNone_menu = gtk_list_store_new(2, G_TYPE_STRING, 
											   G_TYPE_INT);
	_fillNoneStyleMenu(m_wListStyleNone_menu.obj());
	m_wListStyleNumbered_menu = gtk_list_store_new (2, G_TYPE_STRING, 
													G_TYPE_INT);
	_fillNumberedStyleMenu(m_wListStyleNumbered_menu.obj());
	m_wListStyleBulleted_menu = gtk_list_store_new(2, G_TYPE_STRING,
												   G_TYPE_INT);
	_fillBulletedStyleMenu(m_wListStyleBulleted_menu.obj());

	// This is the default list. Change if the list style changes
	//
	m_wListStyle_menu = m_wListStyleNumbered_menu;

	gtk_combo_box_set_model(GTK_COMBO_BOX (style_om), 
							GTK_TREE_MODEL(m_wListStyleNumbered_menu.obj()));

	type_om = gtk_combo_box_text_new();
	gtk_widget_show (type_om);
	gtk_grid_attach(GTK_GRID(grid1), type_om, 1, 0, 1, 1);
	
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_none,s);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_om), s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_bullet,s);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_om), s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type_numbered,s);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_om), s.c_str());
	gtk_combo_box_set_active(GTK_COMBO_BOX(type_om), 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Type,s);
	type_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (type_lb);
	gtk_grid_attach(GTK_GRID(grid1), type_lb, 0, 0, 1, 1);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Style,s);
	style_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                               "xalign", 0.0, "yalign", 0.5,
                               NULL);
	gtk_widget_show (style_lb);
	gtk_grid_attach(GTK_GRID(grid1), style_lb, 0, 1, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_SetDefault,s);
	customized_cb = gtk_dialog_add_button (GTK_DIALOG(m_wMainWindow), s.c_str(), BUTTON_RESET);
	GtkWidget *img = gtk_image_new_from_icon_name("document-revert", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(customized_cb), img);
	gtk_widget_show (customized_cb);

	/* todo
	gtk_grid_attach(GTK_GRID(grid1), customized_cb, 0, 2, 1, 1);
	*/

	grid2 = gtk_grid_new();
	g_object_set(G_OBJECT(grid2),
	             "row-spacing", 6,
	             "column-spacing", 12,
	             "margin-top", 12,
	             NULL);
	gtk_widget_show(grid2);
	gtk_grid_attach(GTK_GRID(list_grid), grid2, 0, 1, 1, 1);
	gtk_widget_set_sensitive (grid2, TRUE);

	font_om_menu = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	_fillFontMenu(font_om_menu);

	font_om = GTK_COMBO_BOX(gtk_combo_box_text_new());
	gtk_combo_box_set_model(font_om, GTK_TREE_MODEL(font_om_menu));
	gtk_widget_show (GTK_WIDGET(font_om));
	gtk_grid_attach (GTK_GRID (grid2), GTK_WIDGET(font_om), 1, 1, 1, 1);

	format_en = gtk_entry_new ();
	gtk_entry_set_max_length(GTK_ENTRY(format_en), 20);
	gtk_widget_show (format_en);
	gtk_grid_attach (GTK_GRID (grid2), format_en, 1, 0, 1, 1);

	decimal_en = gtk_entry_new ();
	gtk_widget_show (decimal_en);
	gtk_grid_attach (GTK_GRID (grid2), decimal_en, 1, 2, 1, 1);
	gtk_entry_set_text (GTK_ENTRY (format_en), "");

	start_sb_adj = (GtkAdjustment*)gtk_adjustment_new (1, 0, G_MAXINT32, 1, 10, 10);
	start_sb = gtk_spin_button_new (GTK_ADJUSTMENT (start_sb_adj), 1, 0);
	gtk_widget_show (start_sb);
	gtk_grid_attach (GTK_GRID (grid2), start_sb, 1, 3, 1, 1);

	text_align_sb_adj = (GtkAdjustment*)gtk_adjustment_new (0.25, 0, 10, 0.01, 0.2, 1);
	text_align_sb = gtk_spin_button_new (GTK_ADJUSTMENT (text_align_sb_adj), 0.05, 2);
	gtk_widget_show (text_align_sb);
	gtk_grid_attach (GTK_GRID (grid2), text_align_sb, 1, 4, 1, 1);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (text_align_sb), TRUE);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (text_align_sb), TRUE);

	label_align_sb_adj = (GtkAdjustment*)gtk_adjustment_new (0, 0, 10, 0.01, 0.2, 1);
	label_align_sb = gtk_spin_button_new (GTK_ADJUSTMENT (label_align_sb_adj), 0.05, 2);
	gtk_widget_show (label_align_sb);
	gtk_grid_attach (GTK_GRID (grid2), label_align_sb, 1, 5, 1, 1);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (label_align_sb), TRUE);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (label_align_sb), TRUE);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Format,s);
	format_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (format_lb);
	gtk_grid_attach (GTK_GRID (grid2), format_lb, 0, 0, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Font,s);
	font_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (font_lb);
	gtk_grid_attach (GTK_GRID (grid2), font_lb, 0, 1, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_DelimiterString,s);
	delimiter_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (delimiter_lb);
	gtk_grid_attach (GTK_GRID (grid2), delimiter_lb, 0, 2, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Start,s);
	start_at_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (start_at_lb);
	gtk_grid_attach (GTK_GRID (grid2), start_at_lb, 0, 3, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Align,s);
	text_align_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (text_align_lb);
	gtk_grid_attach (GTK_GRID (grid2), text_align_lb, 0, 4, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Indent,s);
	label_align_lb = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show (label_align_lb);
	gtk_grid_attach (GTK_GRID (grid2), label_align_lb, 0, 5, 1, 1);

	grid3 = gtk_grid_new();
	gtk_widget_show(grid3);
	gtk_grid_attach(GTK_GRID(list_grid), grid3, 1, 0, 1, 2);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Preview,s);
	preview_lb = gtk_widget_new(GTK_TYPE_LABEL, "label", s.c_str(),
                              "xalign", 0.0, "yalign", 0.5,
                              NULL);
	gtk_widget_show(preview_lb);
	gtk_grid_attach(GTK_GRID(grid3), preview_lb, 0, 0, 1, 1);

	preview_area = gtk_drawing_area_new();
	gtk_widget_set_size_request (preview_area, 180, 225);
	gtk_widget_set_margin_start (preview_area, 18);
	gtk_widget_show (preview_area);
	gtk_grid_attach(GTK_GRID(grid3), preview_area, 0, 1, 1, 1);

	hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
	if(!isModal())
		gtk_widget_show (hbox1);
	gtk_grid_attach(GTK_GRID(list_grid), hbox1, 0, 2, 2, 1);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Apply_Current,s);
	apply_list_rb = gtk_radio_button_new_with_label (action_group, s.c_str());
	action_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (apply_list_rb));
	if(!isModal())
		gtk_widget_show (apply_list_rb);
	gtk_box_pack_start (GTK_BOX (hbox1), apply_list_rb, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (apply_list_rb), TRUE);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Start_New,s);
	start_list_rb = gtk_radio_button_new_with_label (action_group, s.c_str());
	action_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (start_list_rb));
	if(!isModal())
		gtk_widget_show (start_list_rb);
	gtk_box_pack_start (GTK_BOX (hbox1), start_list_rb, FALSE, FALSE, 0);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Resume,s);
	resume_list_rb = gtk_radio_button_new_with_label (action_group, s.c_str());
	action_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (resume_list_rb));
	if(!isModal())
		gtk_widget_show (resume_list_rb);
	gtk_box_pack_start (GTK_BOX (hbox1), resume_list_rb, FALSE, FALSE, 0);

	// Save useful widgets in member variables
	if(isModal())
	{
		m_wContents = list_grid;
	}
	else
	{
		m_wContents = wNoteBook;
	}
	m_wStartNewList = start_list_rb;
	m_wStartNew_label = gtk_bin_get_child(GTK_BIN(start_list_rb));
	m_wApplyCurrent = apply_list_rb;
	m_wStartSubList = resume_list_rb;
	m_wStartSub_label = gtk_bin_get_child(GTK_BIN(resume_list_rb));
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
	m_wCustomFrame = grid2;
	m_wCustomLabel = customized_cb;
	m_wCustomTable = grid2;
	m_wListStyleBox = GTK_COMBO_BOX(style_om);
	m_wListTypeBox = GTK_COMBO_BOX(type_om);
	m_wListType_menu = m_wListStyleNumbered_menu;

	// Start by hiding the Custom frame
	//
	//	gtk_widget_hide(m_wCustomFrame);
	gtk_widget_show(m_wCustomFrame);

	setbisCustomized(false);

	return m_wContents;
}


/*
  This code is to suck all the available fonts and put them in a vector.
  This can then be displayed on a combo box at the top of the dialog.
  Code stolen from xap_UnixDialog_Insert_Symbol */
/* Now we remove all the duplicate name entries and create the vector
   glFonts. This will be used in the font selection combo
   box */

void AP_UnixDialog_Lists::_getGlistFonts (std::vector<std::string> & glFonts)
{
	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	UT_return_if_fail(pGF);
	
	const std::vector<std::string> & names = GR_CairoGraphics::getAllFontNames();
	
    std::string currentfont;

	for (std::vector<std::string>::const_iterator i = names.begin(); 
		 i != names.end(); ++i)
	{
	    const std::string & lgn  = *i;
	    if(currentfont.empty() ||
	       (strstr(currentfont.c_str(), lgn.c_str()) == NULL) ||
	       currentfont.size() != lgn.size())
	    {
			currentfont = lgn;
            glFonts.push_back(lgn);
	    }
	}
}



void AP_UnixDialog_Lists::_fillNoneStyleMenu( GtkListStore *listmenu)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	addToStore(listmenu, pSS, AP_STRING_ID_DLG_Lists_Style_none,
			   NOT_A_LIST);
}

void AP_UnixDialog_Lists::_fillNumberedStyleMenu( GtkListStore *listmenu)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	addToStore(listmenu, pSS, AP_STRING_ID_DLG_Lists_Numbered_List,
			   NUMBERED_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Lower_Case_List,
			   LOWERCASE_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Upper_Case_List,
			   UPPERCASE_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Lower_Roman_List,
			   LOWERROMAN_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Upper_Roman_List,
			   UPPERROMAN_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Arabic_List,
			   ARABICNUMBERED_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Hebrew_List,
			   HEBREW_LIST);
}


void AP_UnixDialog_Lists::_fillBulletedStyleMenu( GtkListStore *listmenu)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Bullet_List,
			   BULLETED_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Dashed_List,
			   DASHED_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Square_List,
			   SQUARE_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Triangle_List,
			   TRIANGLE_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Diamond_List,
			   DIAMOND_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Star_List,
			   STAR_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Implies_List,
			   IMPLIES_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Tick_List,
			   TICK_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Box_List,
			   BOX_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Hand_List,
			   HAND_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Heart_List,
			   HEART_LIST);
	addToStore(listmenu, pSS,AP_STRING_ID_DLG_Lists_Arrowhead_List,
			   ARROWHEAD_LIST);
}

void AP_UnixDialog_Lists::_setRadioButtonLabels(void)
{
	//	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	std::string s;
	PopulateDialogData();
	// Button 0 is Start New List, button 2 is resume list
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Start_New,s);
	gtk_label_set_text( GTK_LABEL(m_wStartNew_label), s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Resume,s);
	gtk_label_set_text( GTK_LABEL(m_wStartSub_label), s.c_str());
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_Lists * dlg)
{
	UT_ASSERT(dlg);
	dlg->setAnswer(AP_Dialog_Lists::a_QUIT);
	dlg->destroy();
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * /*dlg*/)
{
	abiDestroyWidget(widget); // will emit the proper signals for us
}

void AP_UnixDialog_Lists::_connectSignals(void)
{
	g_signal_connect (G_OBJECT (m_wApply), "clicked",
						G_CALLBACK (s_applyClicked), this);
	g_signal_connect (G_OBJECT (m_wClose), "clicked",
						G_CALLBACK (s_closeClicked), this);
	g_signal_connect (G_OBJECT (m_wCustomLabel), "clicked",
						G_CALLBACK (s_customChanged), this);

	g_signal_connect (G_OBJECT (m_wListTypeBox), "changed",
					  G_CALLBACK (s_styleChanged), this);
	g_signal_connect(G_OBJECT(m_wListStyleBox), "changed",
					 G_CALLBACK(s_typeChanged), this);
/*
	g_signal_connect (G_OBJECT (m_wMenu_None), "activate",
						G_CALLBACK (s_styleChangedNone), this);
	g_signal_connect (G_OBJECT (m_wMenu_Bull), "activate",
						G_CALLBACK (s_styleChangedBullet), this);
	g_signal_connect (G_OBJECT (m_wMenu_Num), "activate",
						G_CALLBACK (s_styleChangedNumbered), this);
*/
	g_signal_connect (G_OBJECT (m_wFontOptions), "changed",
					  G_CALLBACK (s_valueChanged), this);

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
					   "draw",
					   G_CALLBACK(s_preview_draw),
					   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wMainWindow),
					 "destroy",
					 G_CALLBACK(s_destroy_clicked),
					 static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wMainWindow),
					 "delete_event",
					 G_CALLBACK(s_delete_clicked),
					 static_cast<gpointer>(this));
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
	XAP_GtkSignalBlocker b1(  G_OBJECT(m_oAlignList_adj), m_iAlignListSpinID);
	XAP_GtkSignalBlocker b2(  G_OBJECT(m_oIndentAlign_adj), m_iIndentAlignSpinID);

	XAP_GtkSignalBlocker b3(  G_OBJECT(m_wDecimalEntry), m_iDecimalEntryID);
	XAP_GtkSignalBlocker b4(  G_OBJECT(m_wDelimEntry), m_iDelimEntryID );
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
	if(getFont() == "NULL")
	{
		gtk_combo_box_set_active(m_wFontOptions, 0 );
	}
	else
	{
        size_t i = 0;
		for(std::vector<std::string>::const_iterator iter = m_glFonts.begin();
            iter != m_glFonts.end(); ++iter, ++i)
		{
			if(*iter == getFont())
				break;
		}
        if(i < m_glFonts.size())
		{
			gtk_combo_box_set_active(m_wFontOptions, i + 1 );
		}
		else
		{
			gtk_combo_box_set_active(m_wFontOptions, 0 );
		}
	}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wStartSpin),static_cast<float>(getiStartValue()));

    gtk_entry_set_text( GTK_ENTRY(m_wDecimalEntry), getDecimal().c_str());
	gtk_entry_set_text( GTK_ENTRY(m_wDelimEntry), getDelim().c_str());

	//
	// Now set the list type and style
	FL_ListType save = getNewListType();
	if(getNewListType() == NOT_A_LIST)
	{
		styleChanged(0);
		setNewListType(save);
		gtk_combo_box_set_active(m_wListTypeBox, 0);
		gtk_combo_box_set_active(m_wListStyleBox, 0);
	}
	else if(IS_BULLETED_LIST_TYPE(getNewListType()) )
	{
		styleChanged(1);
		setNewListType(save);
		gtk_combo_box_set_active(m_wListTypeBox, 1);
		gtk_combo_box_set_active(m_wListStyleBox, (gint) (getNewListType() - BULLETED_LIST));
	}
	else
	{
		styleChanged(2);
	    setNewListType(save);
		gtk_combo_box_set_active(m_wListTypeBox, 2);
		if(getNewListType() < OTHER_NUMBERED_LISTS)
		{
			gtk_combo_box_set_active(m_wListStyleBox, getNewListType());
		}
		else
		{
		    gint iMenu = static_cast<gint>(getNewListType()) - OTHER_NUMBERED_LISTS + BULLETED_LIST -1 ;
			gtk_combo_box_set_active(m_wListStyleBox,iMenu);
		}
	}

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
	UT_sint32 maxWidth = getBlock()->getDocSectionLayout()->getActualColumnWidth();
	if(getBlock()->getFirstContainer())
	{
	  if(getBlock()->getFirstContainer()->getContainer())
	  {
	    maxWidth = getBlock()->getFirstContainer()->getContainer()->getWidth();
	  }
	}

//
// screen resolution is 100 pixels/inch
//
	float fmaxWidthIN = (static_cast<float>(maxWidth)/ 100.) - 0.6;
	setiLevel(1);
	float f =gtk_spin_button_get_value(GTK_SPIN_BUTTON(m_wAlignListSpin));
	if(f >   fmaxWidthIN)
	{
		f = fmaxWidthIN;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON( m_wAlignListSpin), f);
	}
	setfAlign(f);
	float indent = gtk_spin_button_get_value(GTK_SPIN_BUTTON( m_wIndentAlignSpin));
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
	gint ifont = gtk_combo_box_get_active(m_wFontOptions);
	if(ifont == 0)
	{
		copyCharToFont("NULL");
	}
	else
	{
		copyCharToFont(m_glFonts[ifont - 1]);
	}
	const gchar * pszDec = gtk_entry_get_text( GTK_ENTRY(m_wDecimalEntry));
	copyCharToDecimal( static_cast<const char *>(pszDec));
	setiStartValue(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_wStartSpin)));
	const gchar * pszDel = gtk_entry_get_text( GTK_ENTRY(m_wDelimEntry));
	copyCharToDelim(static_cast<const char *>(pszDel));
}
