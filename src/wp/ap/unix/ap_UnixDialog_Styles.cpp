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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Styles.h"
#include "ap_UnixDialog_Styles.h"

#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Style.h"
#include "ut_string_class.h"

XAP_Dialog * AP_UnixDialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_Styles * p = new AP_UnixDialog_Styles(pFactory,id);
	return p;
}

AP_UnixDialog_Styles::AP_UnixDialog_Styles(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
  : AP_Dialog_Styles(pDlgFactory,id), m_whichRow(0), m_whichCol(0), m_whichType(AP_UnixDialog_Styles::USED_STYLES)
{
	m_windowMain = NULL;

	m_wbuttonOk = NULL;
	m_wbuttonCancel = NULL;
	m_wGnomeButtons = NULL;
	m_wParaPreviewArea = NULL;
	m_pParaPreviewWidget = NULL;
	m_wCharPreviewArea = NULL;
	m_pCharPreviewWidget = NULL;

	m_wclistStyles = NULL;
	m_wlistTypes = NULL;
	m_wlabelDesc = NULL;

	m_wModifyDialog = NULL;
	m_wStyleNameEntry = NULL;
	m_wBasedOnCombo = NULL;
	m_wBasedOnEntry = NULL;
	m_wFollowingCombo = NULL;
	m_wFollowingEntry = NULL;
	m_wLabDescription = NULL;

	m_pAbiPreviewWidget = NULL;
	m_wModifyDrawingArea = NULL;

	m_wModifyOk = NULL;
	m_wModifyCancel = NULL;
	m_wFormatMenu = NULL;
	m_wModifyShortCutKey = NULL;

	m_wFormat = NULL;
	m_wModifyParagraph = NULL;
	m_wModifyFont = NULL;
	m_wModifyNumbering = NULL;
	m_gbasedOnStyles = NULL;
	m_gfollowedByStyles = NULL;

}

AP_UnixDialog_Styles::~AP_UnixDialog_Styles(void)
{
	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
	DELETEP (m_pAbiPreviewWidget);
}

/*****************************************************************/

static void
s_clist_clicked (GtkWidget *w, gint row, gint col, 
		 GdkEvent *evt, gpointer d)
{
	AP_UnixDialog_Styles * dlg = static_cast <AP_UnixDialog_Styles *>(d);
	dlg->event_ClistClicked (row, col);
}

static void
s_typeslist_changed (GtkWidget *w, gpointer d)
{
	AP_UnixDialog_Styles * dlg = static_cast <AP_UnixDialog_Styles *>(d);
	dlg->event_ListClicked (gtk_entry_get_text (GTK_ENTRY(w)));
}

static void
s_deletebtn_clicked (GtkWidget *w, gpointer d)
{
	AP_UnixDialog_Styles * dlg = static_cast <AP_UnixDialog_Styles *>(d);
	dlg->event_DeleteClicked ();
}

static void
s_modifybtn_clicked (GtkWidget *w, gpointer d)
{
	AP_UnixDialog_Styles * dlg = static_cast <AP_UnixDialog_Styles *>(d);
	dlg->event_ModifyClicked ();
}

static void
s_newbtn_clicked (GtkWidget *w, gpointer d)
{
	AP_UnixDialog_Styles * dlg = static_cast <AP_UnixDialog_Styles *>(d);
	dlg->event_NewClicked ();
}

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_OK();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_Cancel();
}


static void s_modify_ok_clicked(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_Modify_OK();
}

static void s_modify_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_Modify_Cancel();
}

static gboolean s_paraPreview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_paraPreviewExposed();
	return FALSE;
}


static gboolean s_charPreview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_paraPreviewExposed();
	return FALSE;
}


static gboolean s_modifyPreview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_ModifyPreviewExposed();
	return FALSE;
}


static gboolean s_modify_window_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_ModifyPreviewExposed();
	return FALSE;
}


static gboolean s_window_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_paraPreviewExposed();
	me->event_charPreviewExposed();
	return FALSE;
}


static void s_delete_clicked(GtkWidget * /* widget */, gpointer /* data */,
			     AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_WindowDelete();
}


static void s_modify_delete_clicked(GtkWidget * /* widget */, gpointer /* data */,
			     AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_ModifyDelete();
}


static void s_modify_paragraph(GtkWidget * /* widget */, 
			     AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_ModifyParagraph();
}

static void s_modify_font(GtkWidget * /* widget */, 
			     AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_ModifyFont();
}


static void s_modify_numbering(GtkWidget * /* widget */,
			     AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_ModifyNumbering();
}


static void s_modify_tabs(GtkWidget * /* widget */,
			     AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_ModifyTabs();
}


/*****************************************************************/

void AP_UnixDialog_Styles::runModal(XAP_Frame * pFrame)
{

//
// Get View and Document pointers. Place them in member variables
//
	m_pFrame = pFrame;
	m_pView = (FV_View *) pFrame->getCurrentView();
	UT_ASSERT(m_pView);
	m_pDoc = m_pView->getLayout()->getDocument();
	UT_ASSERT(m_pDoc);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
	
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

    // populate the member variables for the  previews

	_populatePreviews(false);
	// *** this is how we add the gc for the para and char Preview's ***
	// attach a new graphics context to the drawing area

	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);
	
	UT_ASSERT(m_wParaPreviewArea && m_wParaPreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pParaPreviewWidget);
	m_pParaPreviewWidget = new GR_UnixGraphics(m_wParaPreviewArea->window, unixapp->getFontManager(), m_pApp);
	
        // let the widget materialize

	_createParaPreviewFromGC(m_pParaPreviewWidget,
							 (UT_uint32) m_wParaPreviewArea->allocation.width, 
							 (UT_uint32) m_wParaPreviewArea->allocation.height);

	
	UT_ASSERT(m_wCharPreviewArea && m_wCharPreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pCharPreviewWidget);
	m_pCharPreviewWidget = new GR_UnixGraphics(m_wCharPreviewArea->window, unixapp->getFontManager(), m_pApp);

	// let the widget materialize

	_createCharPreviewFromGC(m_pCharPreviewWidget,
							 (UT_uint32) m_wCharPreviewArea->allocation.width, 
							 (UT_uint32) m_wCharPreviewArea->allocation.height);

	// the expose event of the preview
	gtk_signal_connect(GTK_OBJECT(m_wParaPreviewArea),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_paraPreview_exposed),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_wCharPreviewArea),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_charPreview_exposed),
					   (gpointer) this);
	
	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "expose_event",
							 GTK_SIGNAL_FUNC(s_window_exposed),
							 (gpointer) this);

	// connect the select_row signal to the clist
	gtk_signal_connect (GTK_OBJECT (m_wclistStyles), "select_row",
						GTK_SIGNAL_FUNC (s_clist_clicked), (gpointer)this);

	// Run into the GTK event loop for this window.
	
//
// Draw the previews!!
//
	// Populate the window's data items
	_populateWindowData();

	event_paraPreviewExposed();
	event_charPreviewExposed();
	gtk_main();

	UT_DEBUGMSG(("SEVIOR: Finished Main window \n"));
	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
	
	if(mainWindow && GTK_IS_WIDGET(mainWindow)) 
	    gtk_widget_destroy(mainWindow);
}

/*****************************************************************/

void AP_UnixDialog_Styles::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_Styles::event_Cancel(void)
{
	m_answer = AP_Dialog_Styles::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Styles::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Styles::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Styles::event_paraPreviewExposed(void)
{
	if(m_pParaPreview)
		m_pParaPreview->draw();
}


void AP_UnixDialog_Styles::event_charPreviewExposed(void)
{
	if(m_pCharPreview)
		event_charPreviewUpdated();
}

void AP_UnixDialog_Styles::event_DeleteClicked(void)
{
	messageBoxOK("Delete Clicked");
	if (m_whichRow != -1)
    {
        gchar * style = NULL;
		int rtn = gtk_clist_get_text (GTK_CLIST(m_wclistStyles), 
									  m_whichRow, m_whichCol, 
									  &style);
		if (!rtn || !style)
			return; // ok, nothing's selected. that's fine

		UT_DEBUGMSG(("DOM: attempting to delete style %s\n", style));

		m_pDoc->removeStyle(style); // actually remove the style
		_populateWindowData(); // force a refresh
    }
}

void AP_UnixDialog_Styles::event_NewClicked(void)
{
//
// Hide the old window
//
	UT_DEBUGMSG(("SEVIOR: Hiding main window for New \n"));
    gtk_widget_hide( m_windowMain);
//
// fill the data structures needed for the Modify dialog
//
	modifyRunModal(true);
	UT_DEBUGMSG(("SEVIOR: Finished New \n"));
	if(m_answer == AP_Dialog_Styles::a_OK)
	{
//
// Do stuff
//
	}
	else
	{
//
// Do other stuff
//
	}
//  
// Restore the values in the main dialog
//

//
// Reveal main window again
//
	gtk_widget_show( m_windowMain);

}

void AP_UnixDialog_Styles::event_ClistClicked(gint row, gint col)
{
	m_whichRow = row;
	m_whichCol = col;

	// refresh the previews
	_populatePreviews(false);
}

void AP_UnixDialog_Styles::event_ListClicked(const char * which)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	if (!strcmp(which, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_InUse)))
		m_whichType = USED_STYLES;
	else if (!strcmp(which, pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_UserDefined)))
		m_whichType = USER_STYLES;
	else
		m_whichType = ALL_STYLES;

	// force a refresh of everything
	_populateWindowData();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Styles::_constructWindow(void)
{
	GtkWidget * windowStyles;
	GtkWidget * hsepMid;
	GtkWidget * vboxContents;

	GtkWidget * buttonBoxGlobal;
	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowStyles = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW (windowStyles), 
		pSS->getValue(AP_STRING_ID_DLG_Styles_StylesTitle));
	gtk_window_set_policy (GTK_WINDOW (windowStyles), FALSE, FALSE, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (windowStyles), 5);
	gtk_window_set_default_size(GTK_WINDOW(windowStyles), 600, 400);

	vboxContents = _constructWindowContents(windowStyles);

	hsepMid = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vboxContents), hsepMid, FALSE, FALSE, 0);
	gtk_widget_show(hsepMid);

	// These buttons need to be gnomified

	buttonBoxGlobal = gtk_hbutton_box_new();
	gtk_hbutton_box_set_spacing_default(0);
	gtk_hbutton_box_set_layout_default(GTK_BUTTONBOX_END);
	gtk_widget_show(buttonBoxGlobal);

	buttonOK = gtk_button_new_with_label ( 
		pSS->getValue(XAP_STRING_ID_DLG_OK) );
	gtk_widget_show(buttonOK);
	//gtk_container_add (GTK_CONTAINER (m_wGnomeButtons), buttonOK);
	gtk_container_add(GTK_CONTAINER(buttonBoxGlobal), buttonOK);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label ( 
		pSS->getValue(XAP_STRING_ID_DLG_Cancel) );
	gtk_widget_show(buttonCancel);
	//gtk_container_add (GTK_CONTAINER (m_wGnomeButtons), buttonCancel);
	gtk_container_add(GTK_CONTAINER(buttonBoxGlobal), buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	gtk_box_pack_start(GTK_BOX(vboxContents), buttonBoxGlobal, FALSE, FALSE, 0);

	// done packing buttons

	gtk_widget_show(vboxContents);
	gtk_container_add(GTK_CONTAINER(windowStyles), vboxContents);

	m_wbuttonOk = buttonOK;
	m_wbuttonCancel = buttonCancel;

	_connectsignals();
	return windowStyles;
}

GtkWidget* AP_UnixDialog_Styles::_constructWindowContents(
									GtkWidget * windowStyles)
{
	GtkWidget * vboxContents;
	GtkWidget * hboxContents;
	GtkWidget * vboxTopLeft;
	GtkWidget * vboxTopRight;


	GtkWidget * frameStyles;
	GtkWidget *	listStyles;

	GtkWidget * frameList;
	GtkWidget * comboList;

	GtkWidget * frameParaPrev;
	GtkWidget * ParaPreviewArea;

	GtkWidget * frameCharPrev;
	GtkWidget * CharPreviewArea;

	GtkWidget * frameDescription;
	GtkWidget * DescriptionArea;

	GtkWidget * hsepBot;

	GtkWidget * buttonBoxStyleManip;

	GtkWidget * buttonNew;
	GtkWidget * buttonModify;
	GtkWidget * buttonDelete;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	vboxContents = gtk_vbox_new(FALSE, 0);

	hboxContents = gtk_hbox_new(FALSE, 0);

	vboxTopLeft = gtk_vbox_new(FALSE, 0);

	// list of styles goes in the top left

	frameStyles = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_Available));

	GtkWidget * scrollWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollWindow);
	gtk_widget_set_usize(scrollWindow, 120, 120);
	gtk_container_set_border_width(GTK_CONTAINER(scrollWindow), 10);
	gtk_container_add (GTK_CONTAINER(frameStyles), scrollWindow);

	listStyles = gtk_clist_new(1);
	gtk_clist_set_column_width (GTK_CLIST (listStyles), 0, 100);
	gtk_clist_column_titles_hide (GTK_CLIST (listStyles));
	gtk_widget_show(listStyles);
	gtk_container_add(GTK_CONTAINER(scrollWindow), listStyles);

	gtk_box_pack_start(GTK_BOX(vboxTopLeft), frameStyles, TRUE, TRUE, 2);
	gtk_widget_show(frameStyles);

	frameList = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_List));
	comboList = gtk_combo_new();

	// TODO: translate me
	GList * styleTypes = NULL;

	styleTypes = g_list_append (styleTypes, 
								(gpointer)pSS->getValue (AP_STRING_ID_DLG_Styles_LBL_InUse));
	styleTypes = g_list_append (styleTypes,
								(gpointer)pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_All));
	styleTypes = g_list_append (styleTypes, (gpointer)pSS->getValue(AP_STRING_ID_DLG_Styles_LBL_UserDefined));

	gtk_combo_set_popdown_strings (GTK_COMBO(comboList), styleTypes);
	gtk_combo_set_value_in_list (GTK_COMBO(comboList), (int)m_whichType, false);
	gtk_container_add(GTK_CONTAINER(frameList), comboList);

	gtk_box_pack_start(GTK_BOX(vboxTopLeft), frameList, FALSE, FALSE, 2);
	gtk_widget_show(frameList);
	gtk_widget_show(comboList);

	gtk_widget_show(vboxTopLeft);
	gtk_box_pack_start(GTK_BOX(hboxContents), vboxTopLeft, TRUE, TRUE, 2);

	vboxTopRight = gtk_vbox_new(FALSE, 0);

	// previewing and description goes in the top right

	frameParaPrev = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_ParaPrev));
	ParaPreviewArea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(ParaPreviewArea), 300, 60);
	gtk_container_add(GTK_CONTAINER(frameParaPrev), ParaPreviewArea);

	gtk_box_pack_start(GTK_BOX(vboxTopRight), frameParaPrev, TRUE, TRUE, 2);
	gtk_widget_show(ParaPreviewArea);
	gtk_widget_show(frameParaPrev);

	frameCharPrev = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_CharPrev));
	CharPreviewArea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(CharPreviewArea), 300, 60);
	gtk_container_add(GTK_CONTAINER(frameCharPrev), CharPreviewArea);

	gtk_box_pack_start(GTK_BOX(vboxTopRight), frameCharPrev, TRUE, TRUE, 2);
	gtk_widget_show(CharPreviewArea);
	gtk_widget_show(frameCharPrev);

	frameDescription = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_Description));
	DescriptionArea = gtk_label_new(NULL);
	gtk_label_set_line_wrap (GTK_LABEL(DescriptionArea), TRUE);
	gtk_label_set_justify (GTK_LABEL(DescriptionArea), GTK_JUSTIFY_LEFT);
	gtk_widget_set_usize(DescriptionArea, 300, 60);
	gtk_container_add(GTK_CONTAINER(frameDescription), DescriptionArea);

	gtk_box_pack_start(GTK_BOX(vboxTopRight), frameDescription, TRUE, TRUE, 2);
	gtk_widget_show(DescriptionArea);
	gtk_widget_show(frameDescription);

	gtk_widget_show(vboxTopRight);
	gtk_box_pack_start(GTK_BOX(hboxContents), vboxTopRight, TRUE, TRUE, 2);


	gtk_widget_show(hboxContents);
	gtk_box_pack_start(GTK_BOX(vboxContents), hboxContents, TRUE, TRUE, 2);

	hsepBot = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vboxContents), hsepBot, FALSE, FALSE, 0);
	gtk_widget_show(hsepBot);

	// Pack buttons at the bottom of the dialog

	buttonBoxStyleManip = gtk_hbutton_box_new();
	gtk_hbutton_box_set_spacing_default(0);
	gtk_hbutton_box_set_layout_default(GTK_BUTTONBOX_END);
	gtk_widget_show(buttonBoxStyleManip);

	buttonNew = gtk_button_new_with_label(
		pSS->getValue(AP_STRING_ID_DLG_Styles_New));
	gtk_widget_show(buttonNew);
	gtk_container_add(GTK_CONTAINER(buttonBoxStyleManip), buttonNew);
	GTK_WIDGET_SET_FLAGS (buttonNew, GTK_CAN_DEFAULT);

	buttonModify = gtk_button_new_with_label(
		pSS->getValue(AP_STRING_ID_DLG_Styles_Modify));
	gtk_widget_show(buttonModify);
	gtk_container_add(GTK_CONTAINER(buttonBoxStyleManip), buttonModify);
	GTK_WIDGET_SET_FLAGS (buttonModify, GTK_CAN_DEFAULT);

	buttonDelete = gtk_button_new_with_label(
		pSS->getValue(AP_STRING_ID_DLG_Styles_Delete));
	gtk_widget_show(buttonDelete);
	gtk_container_add(GTK_CONTAINER(buttonBoxStyleManip), buttonDelete);
	GTK_WIDGET_SET_FLAGS (buttonDelete, GTK_CAN_DEFAULT);

	gtk_box_pack_start(GTK_BOX(vboxContents), buttonBoxStyleManip, FALSE, FALSE, 0);

	// connect signal for this list
	gtk_signal_connect (GTK_OBJECT(GTK_COMBO(comboList)->entry), 
						"changed",
						GTK_SIGNAL_FUNC(s_typeslist_changed),
						(gpointer)this);

	// connect signals for these 3 buttons
	gtk_signal_connect (GTK_OBJECT(buttonNew),
						"clicked",
						GTK_SIGNAL_FUNC(s_newbtn_clicked),
						(gpointer)this);

	gtk_signal_connect (GTK_OBJECT(buttonModify),
						"clicked",
						GTK_SIGNAL_FUNC(s_modifybtn_clicked),
						(gpointer)this);

	gtk_signal_connect (GTK_OBJECT(buttonDelete),
						"clicked",
						GTK_SIGNAL_FUNC(s_deletebtn_clicked),
						(gpointer)this);

	m_wclistStyles = listStyles;
	m_wlistTypes = comboList;
	m_windowMain = windowStyles;
	m_wbuttonNew = buttonNew;
	m_wbuttonModify = buttonModify;
	m_wbuttonDelete = buttonDelete;
	m_wParaPreviewArea = ParaPreviewArea;
	m_wCharPreviewArea = CharPreviewArea;
	m_wlabelDesc = DescriptionArea;
	return vboxContents;
}

void AP_UnixDialog_Styles::_connectsignals(void) const
{

	// the control buttons

	gtk_signal_connect(GTK_OBJECT(m_wbuttonOk),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_wbuttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	// the catch-alls
	
	gtk_signal_connect(GTK_OBJECT(m_windowMain),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_delete_clicked),
					   (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);
}

void AP_UnixDialog_Styles::_populateCList(void) const
{
	const PD_Style * pStyle;
	const char * name = NULL;

	size_t nStyles = m_pDoc->getStyleCount();
	xxx_UT_DEBUGMSG(("DOM: we have %d styles\n", nStyles));

	gtk_clist_freeze (GTK_CLIST (m_wclistStyles));
	gtk_clist_clear (GTK_CLIST (m_wclistStyles));

	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    const char * data[1];

	    m_pDoc->enumStyles((UT_uint32)i, &name, &pStyle);

	    // all of this is safe to do... append should take a const char **
	    data[0] = name;

	    if ((m_whichType == ALL_STYLES) || 
		(m_whichType == USED_STYLES && pStyle->isUsed()) ||
		(m_whichType == USER_STYLES && pStyle->isUserDefined()))
		{
			gtk_clist_append (GTK_CLIST(m_wclistStyles), (gchar **)data);
		}
	}

	gtk_clist_thaw (GTK_CLIST (m_wclistStyles));
	gtk_clist_select_row (GTK_CLIST (m_wclistStyles), 0, 0);
}

void AP_UnixDialog_Styles::_populateWindowData(void)
{
	_populateCList();
	_populatePreviews(false);
}

void AP_UnixDialog_Styles::setDescription(const char * desc) const
{
	UT_ASSERT(m_wlabelDesc);
	gtk_label_set_text (GTK_LABEL(m_wlabelDesc), desc);
}

const char * AP_UnixDialog_Styles::getCurrentStyle (void) const
{
	static UT_String szStyleBuf;

	UT_ASSERT(m_wclistStyles);

	if (m_whichRow < 0 || m_whichCol < 0)
		return NULL;

	char * szStyle = NULL;

	int ret = gtk_clist_get_text (GTK_CLIST(m_wclistStyles), 
								  m_whichRow, m_whichCol, &szStyle);

	if (!ret)
		return NULL;

	szStyleBuf = szStyle;
	return szStyleBuf.c_str();
}

GtkWidget *  AP_UnixDialog_Styles::_constructModifyDialog(bool isNew)
{
	GtkWidget *modifyDialog;
	GtkWidget *dialog_action_area;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	modifyDialog = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (modifyDialog), 5);
	if(!isNew)
		gtk_window_set_title (GTK_WINDOW (modifyDialog), pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyTitle));
	else
		gtk_window_set_title (GTK_WINDOW (modifyDialog), "New Style");
	gtk_window_set_policy (GTK_WINDOW (modifyDialog), TRUE, TRUE, FALSE);
	_constructModifyDialogContents(modifyDialog);

	dialog_action_area = GTK_DIALOG (modifyDialog)->action_area;
	gtk_widget_show (dialog_action_area);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area), 10);
//
// Gnome buttons
//
	_constructGnomeModifyButtons(dialog_action_area);
//
// Connect signals
//
	_connectModifySignals();

	return modifyDialog;
}

void  AP_UnixDialog_Styles::_constructModifyDialogContents(GtkWidget * modifyDialog)
{

	GtkWidget *dialog_vbox1;
	GtkWidget *OverallVbox;
	GtkWidget *comboTable;
	GtkWidget *nameLabel;
	GtkWidget *basedOnLabel;
	GtkWidget *followingLabel;
	GtkWidget *styleNameEntry;
	GtkWidget *basedOnCombo;
	GtkWidget *basedOnEntry;
	GtkWidget *followingCombo;
	GtkWidget *followingEntry;
	GtkWidget *previewFrame;
	GtkWidget *modifyDrawingArea;
	GtkWidget *DescriptionText;
	GtkWidget *checkBoxRow;
	GtkWidget *checkAddTo;
	GtkWidget *checkAutoUpdate;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	dialog_vbox1 = GTK_DIALOG (modifyDialog)->vbox;
	gtk_widget_show (dialog_vbox1);

	OverallVbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (OverallVbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), OverallVbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (OverallVbox), 5);

	comboTable = gtk_table_new (3, 2, TRUE);
	gtk_widget_show (comboTable);
	gtk_box_pack_start (GTK_BOX (OverallVbox), comboTable, TRUE, TRUE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (comboTable), 2);
	gtk_table_set_row_spacings (GTK_TABLE (comboTable), 6);
	gtk_table_set_col_spacings (GTK_TABLE (comboTable), 2);

	nameLabel = gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyName));
	gtk_widget_show (nameLabel);
	gtk_table_attach (GTK_TABLE (comboTable), nameLabel, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (nameLabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (nameLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (nameLabel), 2, 2);

	basedOnLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyBasedOn) );
	gtk_widget_show (basedOnLabel);
	gtk_table_attach (GTK_TABLE (comboTable), basedOnLabel, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (basedOnLabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (basedOnLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (basedOnLabel), 2, 2);

	followingLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyFollowing));
	gtk_widget_show (followingLabel);
	gtk_table_attach (GTK_TABLE (comboTable), followingLabel, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (followingLabel), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (followingLabel), 2, 3);

	styleNameEntry = gtk_entry_new ();
	gtk_widget_show (styleNameEntry);
	gtk_table_attach (GTK_TABLE (comboTable), styleNameEntry, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_usize (styleNameEntry, 158, -2);

	basedOnCombo = gtk_combo_new ();
	gtk_widget_show (basedOnCombo);
	gtk_table_attach (GTK_TABLE (comboTable), basedOnCombo, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	basedOnEntry = GTK_COMBO (basedOnCombo)->entry;
	gtk_widget_show (basedOnEntry);
	gtk_widget_set_usize (basedOnEntry, 158, -2);
	
	followingCombo = gtk_combo_new ();
	gtk_widget_show (followingCombo);
	gtk_table_attach (GTK_TABLE (comboTable), followingCombo, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	followingEntry = GTK_COMBO (followingCombo)->entry;
	gtk_widget_show (followingEntry);
	gtk_widget_set_usize (followingEntry, 158, -2);

	previewFrame = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyPreview));
	gtk_widget_show (previewFrame);
	gtk_box_pack_start (GTK_BOX (OverallVbox), previewFrame, TRUE, TRUE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (previewFrame), 5);
	gtk_frame_set_shadow_type (GTK_FRAME (previewFrame), GTK_SHADOW_IN);

	modifyDrawingArea = gtk_drawing_area_new ();
	gtk_widget_show (modifyDrawingArea);
	gtk_container_add (GTK_CONTAINER (previewFrame), modifyDrawingArea);
	gtk_widget_set_usize (modifyDrawingArea, -2, 120);

	GtkWidget * descriptionFrame = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyDescription));
	gtk_widget_show (descriptionFrame);
	gtk_box_pack_start (GTK_BOX (OverallVbox), descriptionFrame, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (descriptionFrame), 5);
	gtk_frame_set_shadow_type (GTK_FRAME (descriptionFrame), GTK_SHADOW_IN);

	DescriptionText = gtk_label_new (NULL);
	gtk_widget_show (DescriptionText);
	gtk_container_add (GTK_CONTAINER (descriptionFrame), DescriptionText);
	gtk_misc_set_alignment (GTK_MISC (DescriptionText), 0, 0.5);
//	gtk_widget_set_usize(DescriptionText, 300, 100);
	gtk_label_set_justify (GTK_LABEL (DescriptionText), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (DescriptionText), TRUE);

	checkBoxRow = gtk_hbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX (OverallVbox), checkBoxRow, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (checkBoxRow), 2);

	checkAddTo = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyTemplate));
	gtk_widget_show (checkAddTo);
	gtk_box_pack_start (GTK_BOX (checkBoxRow), checkAddTo, TRUE, TRUE, 0);

	checkAutoUpdate = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyAutomatic));
	gtk_widget_show (checkAutoUpdate);
	gtk_box_pack_start (GTK_BOX (checkBoxRow), checkAutoUpdate, TRUE, TRUE, 0);

//
// Save widget pointers in member variables
//
	m_wStyleNameEntry = styleNameEntry;
	m_wBasedOnCombo = basedOnCombo;
	m_wBasedOnEntry = basedOnEntry;
    m_wFollowingCombo = followingCombo;
	m_wFollowingEntry = followingEntry;
	m_wModifyDrawingArea = modifyDrawingArea;
	m_wLabDescription = DescriptionText;
	m_wModifyDialog = modifyDialog;

}

void   AP_UnixDialog_Styles::_constructGnomeModifyButtons( GtkWidget * dialog_action_area)
{
	GtkWidget *bottomButtons;
	GtkWidget *buttonOK;
	GtkWidget *cancelButton;
	GtkWidget *FormatMenu;
	GtkWidget *shortCutButton;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	bottomButtons = gtk_hbox_new (TRUE, 5);
	gtk_widget_show (bottomButtons);
	gtk_box_pack_start (GTK_BOX (dialog_action_area), bottomButtons, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (bottomButtons), 3);
	
	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show (buttonOK);
	gtk_box_pack_start (GTK_BOX (bottomButtons), buttonOK, TRUE, TRUE, 0);

	cancelButton = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show (cancelButton);
	gtk_box_pack_start (GTK_BOX (bottomButtons), cancelButton, TRUE, TRUE, 0);

	FormatMenu = gtk_option_menu_new ();
	gtk_widget_show (FormatMenu);
	gtk_box_pack_start (GTK_BOX (bottomButtons), FormatMenu, FALSE, FALSE, 0);

	_constructFormatList(FormatMenu);

	shortCutButton = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyShortCut));
	gtk_widget_show (shortCutButton);
	gtk_box_pack_start (GTK_BOX (bottomButtons), shortCutButton, TRUE, TRUE, 0);

	m_wModifyOk = buttonOK;
	m_wModifyCancel = cancelButton;
	m_wFormatMenu = FormatMenu;
	m_wModifyShortCutKey = shortCutButton;
	
}

void  AP_UnixDialog_Styles::_constructFormatList(GtkWidget * FormatMenu)
{
	GtkWidget *FormatMenu_menu;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	FormatMenu_menu = gtk_menu_new ();
	GtkWidget * wFormat = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyFormat));
	gtk_widget_show (wFormat);
	gtk_menu_append (GTK_MENU (FormatMenu_menu), wFormat);

	GtkWidget * wParagraph = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph));
	gtk_widget_show (wParagraph);
	gtk_menu_append (GTK_MENU (FormatMenu_menu), wParagraph);

	GtkWidget * wFont = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyFont));
	gtk_widget_show (wFont);
	gtk_menu_append (GTK_MENU (FormatMenu_menu), wFont);

	GtkWidget * wTabs = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyTabs));
	gtk_widget_show (wTabs);
	gtk_menu_append (GTK_MENU (FormatMenu_menu), wTabs);

	GtkWidget * wNumbering = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyNumbering));
	gtk_widget_show (wNumbering);
	gtk_menu_append (GTK_MENU (FormatMenu_menu), wNumbering);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (FormatMenu), FormatMenu_menu);

	m_wFormat = wFormat;
	m_wModifyParagraph = wParagraph;
	m_wModifyFont = wFont;
	m_wModifyNumbering = wNumbering;
	m_wModifyTabs = wTabs;
}

void AP_UnixDialog_Styles::_connectModifySignals(void)
{

	gtk_signal_connect(GTK_OBJECT(m_wModifyOk),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_modify_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_wModifyCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_modify_cancel_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_wModifyParagraph),
					   "activate",
					   GTK_SIGNAL_FUNC(s_modify_paragraph),
					   (gpointer) this);


	gtk_signal_connect(GTK_OBJECT(m_wModifyFont),
					   "activate",
					   GTK_SIGNAL_FUNC(s_modify_font),
					   (gpointer) this);


	gtk_signal_connect(GTK_OBJECT(m_wModifyNumbering),
					   "activate",
					   GTK_SIGNAL_FUNC(s_modify_numbering),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_wModifyTabs),
					   "activate",
					   GTK_SIGNAL_FUNC(s_modify_tabs),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_wModifyDrawingArea),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_modifyPreview_exposed),
					   (gpointer) this);

	
	gtk_signal_connect_after(GTK_OBJECT(m_wModifyDialog),
							 "expose_event",
							 GTK_SIGNAL_FUNC(s_modify_window_exposed),
							 (gpointer) this);

	// the catch-alls
	
	gtk_signal_connect(GTK_OBJECT(m_wModifyDialog),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_modify_delete_clicked),
					   (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_wModifyDialog),
							 "destroy",
							 NULL,
							 NULL);
}


void AP_UnixDialog_Styles::event_Modify_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_Styles::event_Modify_Cancel(void)
{
	m_answer = AP_Dialog_Styles::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Styles::event_ModifyDelete(void)
{
	m_answer = AP_Dialog_Styles::a_CANCEL;
	gtk_main_quit();
}

void  AP_UnixDialog_Styles::modifyRunModal(bool isNew)
{
//
// OK Construct the new dialog and make it modal.
//
//
// pointer to the widget is stored in m_wModifyDialog
//
// Center our new dialog in its parent and make it a transient

	_constructModifyDialog(isNew);

	connectFocus(GTK_WIDGET(m_wModifyDialog),m_pFrame);
//
// populate the dialog with useful info
//
    if(!_populateModify(isNew))
	{
		if(m_wModifyDialog && GTK_IS_WIDGET(m_wModifyDialog)) 
			gtk_widget_destroy(m_wModifyDialog);
		return;
	}

	// so it won't get lost underneath
	centerDialog(m_windowMain, m_wModifyDialog);

	// Show the top level dialog,
	gtk_widget_show(m_wModifyDialog);

	// Make it modal, and stick it up top
	gtk_grab_add(m_wModifyDialog);

	// make a new Unix GC
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);

	DELETEP (m_pAbiPreviewWidget);
	m_pAbiPreviewWidget = new GR_UnixGraphics(m_wModifyDrawingArea->window, unixapp->getFontManager(), m_pApp);
	
        // let the widget materialize

	_createAbiPreviewFromGC(m_pAbiPreviewWidget,
							 (UT_uint32) m_wModifyDrawingArea->allocation.width, 
							 (UT_uint32) m_wModifyDrawingArea->allocation.height);
    _populateAbiPreview(isNew);
	event_ModifyPreviewExposed();
	gtk_main();

	if(m_wModifyDialog && GTK_IS_WIDGET(m_wModifyDialog)) 
	{
//
// Free the old glists
//
		if(m_gbasedOnStyles != NULL)
		{	
			g_list_free (m_gbasedOnStyles);
			m_gbasedOnStyles = NULL;
		}

		if(m_gfollowedByStyles != NULL)
		{
			g_list_free (m_gfollowedByStyles);
			m_gfollowedByStyles = NULL;
		}
	    gtk_widget_destroy(m_wModifyDialog);
	}
//
// Have to delete this now since the destructor is not run till later
//	
	destroyAbiPreview();
	DELETEP(m_pAbiPreviewWidget);
}

void AP_UnixDialog_Styles::event_ModifyPreviewExposed(void)
{
	drawLocal();
}

void AP_UnixDialog_Styles::event_ModifyClicked(void)
{
//
// Hide the old window
//
	UT_DEBUGMSG(("SEVIOR: Hiding main window \n"));
    gtk_widget_hide(m_windowMain);
//
// fill the data structures needed for the Modify dialog
//
	modifyRunModal(false);
	UT_DEBUGMSG(("SEVIOR: Finished Modify \n"));
	if(m_answer == AP_Dialog_Styles::a_OK)
	{
//
// Do stuff
//
	}
	else
	{
//
// Do other stuff
//
	}
//  
// Restore the values in the main dialog
//

//
// Reveal main window again
//
	gtk_widget_show( m_windowMain);
}

void  AP_UnixDialog_Styles::setModifyDescription( const char * desc)
{
	UT_ASSERT(m_wlabelDesc);
	gtk_label_set_text (GTK_LABEL(m_wLabDescription), desc);
}

bool  AP_UnixDialog_Styles::_populateModify(bool isNew)
{
	setModifyDescription( m_curStyleDesc.c_str());
//
// Get Style name and put in in the text entry
//
	const char * szCurrentStyle = NULL;
	if(!isNew)
	{
		szCurrentStyle= getCurrentStyle();
		if(!szCurrentStyle)
		{
			messageBoxOK("No Style selected \n so it cannot be modified");
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
		gtk_entry_set_text (GTK_ENTRY(m_wStyleNameEntry), getCurrentStyle());
		gtk_entry_set_editable( GTK_ENTRY(m_wStyleNameEntry),FALSE );
	}
	else
	{
		gtk_entry_set_text (GTK_ENTRY(m_wStyleNameEntry), " ");
		gtk_entry_set_editable( GTK_ENTRY(m_wStyleNameEntry),TRUE );
	}
//
// Next interogate the current style and find the based on and followed by
// Styles
//
	PD_Style * pStyle = NULL;
	m_pDoc->getStyle(szCurrentStyle,&pStyle);
	if(!pStyle)
	{
		messageBoxOK("This style does not exist \n so it cannot be modified");
		m_answer = AP_Dialog_Styles::a_CANCEL;
		return false;
	}
//
// Valid style get the Based On and followed by values
//
	PD_Style * pBasedOnStyle = pStyle->getBasedOn();
	const char * szBasedOn = NULL;
 
	PD_Style * pFollowedByStyle = pStyle->getFollowedBy();
	const char * szFollowedBy = NULL;
//
// Next make a glists of all styles and attach them to the BasedOn and FollowedBy
//
	size_t nStyles = m_pDoc->getStyleCount();
	const char * name = NULL;
	const PD_Style * pcStyle = NULL;
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    m_pDoc->enumStyles(i, &name, &pcStyle);

		if(pcStyle == pBasedOnStyle)
			szBasedOn = name;

		if(pcStyle == pFollowedByStyle)
			szFollowedBy = name;

		m_gbasedOnStyles = g_list_append (m_gbasedOnStyles, (gpointer) name);
		m_gfollowedByStyles = g_list_append (m_gfollowedByStyles, (gpointer) name);
	}

//
// Set the popdown list
//
	gtk_combo_set_popdown_strings( GTK_COMBO(m_wBasedOnCombo),m_gbasedOnStyles);
	gtk_combo_set_popdown_strings( GTK_COMBO(m_wFollowingCombo),m_gfollowedByStyles);

//
// OK here we set intial values for the basedOn and followedBy
//
	gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(m_wBasedOnCombo)->entry),szBasedOn);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(m_wBasedOnCombo)->entry),FALSE );
	gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(m_wFollowingCombo)->entry),szFollowedBy);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(m_wFollowingCombo)->entry),FALSE );
	return true;
}

void   AP_UnixDialog_Styles::event_ModifyParagraph()
{
	UT_DEBUGMSG(("SEVIOR: Modify Paragraphs properties \n"));
}

void   AP_UnixDialog_Styles::event_ModifyFont()
{
	UT_DEBUGMSG(("SEVIOR: Modify Character properties \n"));
}


void   AP_UnixDialog_Styles::event_ModifyNumbering()
{
	UT_DEBUGMSG(("SEVIOR: Modify List properties \n"));
}


void   AP_UnixDialog_Styles::event_ModifyTabs()
{
	UT_DEBUGMSG(("SEVIOR: Modify Tab properties \n"));
}


