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
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Styles.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "fv_View.h"
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

	m_wbuttonApply = NULL;
	m_wbuttonClose = NULL;
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
	m_wStyleTypeCombo = NULL;
	m_wStyleTypeEntry = NULL;
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
	m_wModifyLanguage = NULL;
	m_gbasedOnStyles = NULL;
	m_gfollowedByStyles = NULL;
	m_gStyleType = NULL;
	m_bBlockModifySignal = false;

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

static void s_remove_property(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_RemoveProperty();
}

static void s_style_name(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->new_styleName();
}


static void s_basedon(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	if(me->isModifySignalBlocked())
		return;
	me->event_basedOn();
}


static void s_followedby(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	if(me->isModifySignalBlocked())
		return;
	me->event_followedBy();
}


static void s_styletype(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	if(me->isModifySignalBlocked())
		return;
	me->event_styleType();
}

static void s_apply_clicked(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_Apply();
}

static void s_close_clicked(GtkWidget * widget, AP_UnixDialog_Styles * me)
{
	UT_ASSERT(widget && me);
	me->event_Close();
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


static void s_modify_language (GtkWidget * /* w */,
							   AP_UnixDialog_Styles * me)
{
	UT_ASSERT(me);
	me->event_ModifyLanguage();
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

	setFrame(pFrame);
	setView((FV_View *) pFrame->getCurrentView());
	UT_ASSERT(getView());

	setDoc(getView()->getLayout()->getDocument());

	UT_ASSERT(getDoc());

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
	g_signal_connect(G_OBJECT(m_wParaPreviewArea),
					   "expose_event",
					   G_CALLBACK(s_paraPreview_exposed),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wCharPreviewArea),
					   "expose_event",
					   G_CALLBACK(s_charPreview_exposed),
					   (gpointer) this);
	
	g_signal_connect_after(G_OBJECT(m_windowMain),
							 "expose_event",
							 G_CALLBACK(s_window_exposed),
							 (gpointer) this);

	// connect the select_row signal to the clist
	g_signal_connect (G_OBJECT (m_wclistStyles), "select_row",
						G_CALLBACK (s_clist_clicked), (gpointer)this);

	// Run into the GTK event loop for this window.
	
//
// Draw the previews!!
//
	// Populate the window's data items
	_populateWindowData();

	event_paraPreviewExposed();
	event_charPreviewExposed();
	gtk_main();

	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
	
	if(mainWindow && GTK_IS_WIDGET(mainWindow)) 
	    gtk_widget_destroy(mainWindow);
}

/*****************************************************************/

void AP_UnixDialog_Styles::event_Apply(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	const XML_Char * szStyle = getCurrentStyle();
	if(szStyle && *szStyle)
	{
		getView()->setStyle(szStyle);
	}
}

void AP_UnixDialog_Styles::event_Close(void)
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
	if (m_whichRow != -1)
    {
        gchar * style = NULL;
		int rtn = gtk_clist_get_text (GTK_CLIST(m_wclistStyles), 
									  m_whichRow, m_whichCol, 
									  &style);
		if (!rtn || !style)
			return; // ok, nothing's selected. that's fine

		UT_DEBUGMSG(("DOM: attempting to delete style %s\n", style));


		if (!getDoc()->removeStyle(style)) // actually remove the style
		{
			const XAP_StringSet * pSS = m_pApp->getStringSet();
			const XML_Char * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrStyleCantDelete);
		
			getFrame()->showMessageBox ((const char *)msg,
										XAP_Dialog_MessageBox::b_O,
										XAP_Dialog_MessageBox::a_OK);
			return;
		}

		getFrame()->repopulateCombos();
		_populateWindowData(); // force a refresh
		getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
    }
}

void AP_UnixDialog_Styles::event_NewClicked(void)
{
	setIsNew(true);
	modifyRunModal();
	if(m_answer == AP_Dialog_Styles::a_OK)
	{
		createNewStyle(getNewStyleName());
		_populateCList();
	}
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
	GtkWidget * buttonApply;
	GtkWidget * buttonClose;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowStyles = gtk_window_new (GTK_WINDOW_TOPLEVEL);
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

	buttonApply = gtk_button_new_with_label ( 
		pSS->getValue(XAP_STRING_ID_DLG_Apply) );
	gtk_widget_show(buttonApply);
	gtk_container_add(GTK_CONTAINER(buttonBoxGlobal), buttonApply);
	GTK_WIDGET_SET_FLAGS (buttonApply, GTK_CAN_DEFAULT);

	buttonClose = gtk_button_new_with_label ( 
		pSS->getValue(XAP_STRING_ID_DLG_Close) );
	gtk_widget_show(buttonClose);
	gtk_container_add(GTK_CONTAINER(buttonBoxGlobal), buttonClose);
	GTK_WIDGET_SET_FLAGS (buttonClose, GTK_CAN_DEFAULT);

	gtk_box_pack_start(GTK_BOX(vboxContents), buttonBoxGlobal, FALSE, FALSE, 0);

	// done packing buttons

	gtk_widget_show(vboxContents);
	gtk_container_add(GTK_CONTAINER(windowStyles), vboxContents);

	m_wbuttonApply = buttonApply;
	m_wbuttonClose = buttonClose;

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
#ifdef NOTDEFINED
	GtkWidget * stylesLocked;
#endif

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
	gtk_drawing_area_size(GTK_DRAWING_AREA(ParaPreviewArea), 300, 70);
	gtk_container_add(GTK_CONTAINER(frameParaPrev), ParaPreviewArea);

	gtk_box_pack_start(GTK_BOX(vboxTopRight), frameParaPrev, TRUE, TRUE, 2);
	gtk_widget_show(ParaPreviewArea);
	gtk_widget_show(frameParaPrev);

	frameCharPrev = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_CharPrev));
	CharPreviewArea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(CharPreviewArea), 300, 50);
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
	gtk_misc_set_alignment(GTK_MISC(DescriptionArea), 0, 0);
	gtk_misc_set_padding(GTK_MISC(DescriptionArea), 8, 6);

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

#ifdef  NOTDEFINED
        //
        // If we're using styles to format a document, prevent accidental use of other formatting
        // tools.  Disable all explicit formatting tools (font, color, boldness) if this
        // check box is checked.
        //
        // Check with "areStylesLocked()", set with "lockStyles( bool )";
        //
        stylesLocked = gtk_check_button_new_with_label(pSS->getValue(AP_STRING_ID_DLG_Styles_StylesLocked));
        gtk_box_pack_start(GTK_BOX(vboxContents), stylesLocked, FALSE, FALSE, 0);
        gtk_widget_show( stylesLocked );
#endif

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
	g_signal_connect (G_OBJECT(GTK_COMBO(comboList)->entry), 
						"changed",
						G_CALLBACK(s_typeslist_changed),
						(gpointer)this);

	// connect signals for these 3 buttons
	g_signal_connect (G_OBJECT(buttonNew),
						"clicked",
						G_CALLBACK(s_newbtn_clicked),
						(gpointer)this);

	g_signal_connect (G_OBJECT(buttonModify),
						"clicked",
						G_CALLBACK(s_modifybtn_clicked),
						(gpointer)this);

	g_signal_connect (G_OBJECT(buttonDelete),
						"clicked",
						G_CALLBACK(s_deletebtn_clicked),
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

	g_signal_connect(G_OBJECT(m_wbuttonApply),
					   "clicked",
					   G_CALLBACK(s_apply_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(m_wbuttonClose),
					   "clicked",
					   G_CALLBACK(s_close_clicked),
					   (gpointer) this);
	
	// the catch-alls
	
	g_signal_connect(G_OBJECT(m_windowMain),
					   "delete_event",
					   G_CALLBACK(s_delete_clicked),
					   (gpointer) this);

	g_signal_connect_after(G_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);
}

void AP_UnixDialog_Styles::_populateCList(void) const
{
	const PD_Style * pStyle;
	const char * name = NULL;

	size_t nStyles = getDoc()->getStyleCount();
	xxx_UT_DEBUGMSG(("DOM: we have %d styles\n", nStyles));

	gtk_clist_freeze (GTK_CLIST (m_wclistStyles));
	gtk_clist_clear (GTK_CLIST (m_wclistStyles));

	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    const char * data[1];

	    getDoc()->enumStyles((UT_uint32)i, &name, &pStyle);

		// style has been deleted probably
		if (!pStyle)
			continue;

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

GtkWidget *  AP_UnixDialog_Styles::_constructModifyDialog(void)
{
	GtkWidget *modifyDialog;
	GtkWidget *dialog_action_area;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	modifyDialog = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (modifyDialog), 5);
	if(!isNew())
		gtk_window_set_title (GTK_WINDOW (modifyDialog), pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyTitle));
	else
		gtk_window_set_title (GTK_WINDOW (modifyDialog), pSS->getValue(AP_STRING_ID_DLG_Styles_NewTitle));
	gtk_window_set_policy (GTK_WINDOW (modifyDialog), TRUE, TRUE, FALSE);
	_constructModifyDialogContents(GTK_DIALOG (modifyDialog)->vbox);

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
	m_wModifyDialog = modifyDialog;

	_connectModifySignals();
	return modifyDialog;
}

void  AP_UnixDialog_Styles::_constructModifyDialogContents(GtkWidget * container)
{

	GtkWidget *dialog_vbox1 = NULL;
	GtkWidget *OverallVbox = NULL;
	GtkWidget *comboTable  = NULL;
	GtkWidget *nameLabel  = NULL;
	GtkWidget *basedOnLabel  = NULL;
	GtkWidget *followingLabel = NULL;
	GtkWidget *styleTypeLabel = NULL;
	GtkWidget *styleNameEntry = NULL;
	GtkWidget *basedOnCombo = NULL;
	GtkWidget *basedOnEntry = NULL;
	GtkWidget *followingCombo = NULL;
	GtkWidget *followingEntry = NULL;
	GtkWidget *styleTypeCombo = NULL;
	GtkWidget *styleTypeEntry = NULL;
	GtkWidget *previewFrame = NULL;
	GtkWidget *modifyDrawingArea = NULL;
	GtkWidget *DescriptionText = NULL;
	GtkWidget *checkBoxRow = NULL;
	GtkWidget *checkAddTo = NULL;
	GtkWidget *checkAutoUpdate = NULL;
	GtkWidget *deletePropCombo = NULL;
	GtkWidget *deletePropEntry = NULL;
	GtkWidget *deletePropButton = NULL;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	dialog_vbox1 = container;
	gtk_widget_show (dialog_vbox1);

	OverallVbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (OverallVbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1), OverallVbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (OverallVbox), 5);

	comboTable = gtk_table_new (4,2, TRUE);
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

	styleTypeLabel = gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyType));
	gtk_widget_show (styleTypeLabel);
	gtk_table_attach (GTK_TABLE (comboTable), styleTypeLabel, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (styleTypeLabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (styleTypeLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (styleTypeLabel), 2, 2);

	basedOnLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyBasedOn) );
	gtk_widget_show (basedOnLabel);
	gtk_table_attach (GTK_TABLE (comboTable), basedOnLabel, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (basedOnLabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (basedOnLabel), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (basedOnLabel), 2, 2);

	followingLabel = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyFollowing));
	gtk_widget_show (followingLabel);
	gtk_table_attach (GTK_TABLE (comboTable), followingLabel, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (followingLabel), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (followingLabel), 2, 3);

	styleNameEntry = gtk_entry_new ();
	gtk_widget_show (styleNameEntry);
	gtk_table_attach (GTK_TABLE (comboTable), styleNameEntry, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_usize (styleNameEntry, 158, -2);

	basedOnCombo = gtk_combo_new ();
	gtk_widget_show (basedOnCombo);
	gtk_table_attach (GTK_TABLE (comboTable), basedOnCombo, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
		
	basedOnEntry = GTK_COMBO (basedOnCombo)->entry;
	gtk_widget_show (basedOnEntry);
	gtk_widget_set_usize (basedOnEntry, 158, -2);

	followingCombo = gtk_combo_new ();
	gtk_widget_show (followingCombo);
	gtk_table_attach (GTK_TABLE (comboTable), followingCombo, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);

	followingEntry = GTK_COMBO (followingCombo)->entry;
	gtk_widget_show (followingEntry);
	gtk_widget_set_usize (followingEntry, 158, -2);
//
// Cannot modify style type attribute
//	
	if(isNew())
	{
		styleTypeCombo = gtk_combo_new ();
		gtk_widget_show (styleTypeCombo);
		gtk_table_attach (GTK_TABLE (comboTable), styleTypeCombo, 1, 2, 1, 2,
						  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						  (GtkAttachOptions) (0), 0, 0);

		styleTypeEntry = GTK_COMBO (styleTypeCombo)->entry;
		gtk_widget_show (styleTypeEntry);
		gtk_widget_set_usize (styleTypeEntry, 158, -2);
	}
	else
	{
		styleTypeEntry = gtk_entry_new ();
		gtk_widget_show (styleTypeEntry);
		gtk_table_attach (GTK_TABLE (comboTable), styleTypeEntry, 1, 2, 1, 2,
						  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
						  (GtkAttachOptions) (0), 0, 0);
		gtk_widget_set_usize (styleTypeEntry, 158, -2);
	}

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
//
// Code to choose properties to be removed from the current style.
//
	GtkWidget * deleteRow = gtk_hbox_new(FALSE,2);
	gtk_widget_show (deleteRow);
	gtk_box_pack_start (GTK_BOX (OverallVbox), deleteRow, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (deleteRow), 2);

	GtkWidget * deleteLabel = gtk_label_new(pSS->getValue(AP_STRING_ID_DLG_Styles_RemoveLab));
	gtk_widget_show (deleteLabel);
	gtk_box_pack_start (GTK_BOX (deleteRow), deleteLabel, TRUE, TRUE, 0);

	deletePropCombo = gtk_combo_new ();
	gtk_widget_show (deletePropCombo);
	gtk_box_pack_start (GTK_BOX (deleteRow), deletePropCombo, TRUE, TRUE, 0);

    deletePropEntry = GTK_COMBO (deletePropCombo)->entry;
	gtk_widget_show (deletePropEntry);
	gtk_widget_set_usize (deletePropEntry, 158, -2);
	
	deletePropButton = gtk_button_new_with_label(pSS->getValue(AP_STRING_ID_DLG_Styles_RemoveButton));
	gtk_widget_show(deletePropButton);
	gtk_box_pack_start (GTK_BOX (deleteRow), deletePropButton, TRUE, TRUE, 0);
		

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
	m_wStyleTypeCombo = styleTypeCombo;
	m_wStyleTypeEntry = styleTypeEntry;
	m_wModifyDrawingArea = modifyDrawingArea;
	m_wLabDescription = DescriptionText;
	m_wDeletePropCombo = deletePropCombo;
	m_wDeletePropEntry = deletePropEntry;
	m_wDeletePropButton = deletePropButton;
}

void   AP_UnixDialog_Styles::_constructGnomeModifyButtons( GtkWidget * dialog_action_area)
{
	GtkWidget *bottomButtons;
	GtkWidget *buttonOK;
	GtkWidget *cancelButton;
	GtkWidget *FormatMenu;
	GtkWidget *shortCutButton = 0;
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

#if 0
	shortCutButton = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyShortCut));
	gtk_widget_show (shortCutButton);
	gtk_widget_set_sensitive ( shortCutButton, FALSE );
	gtk_box_pack_start (GTK_BOX (bottomButtons), shortCutButton, TRUE, TRUE, 0);
#endif

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

	GtkWidget * wLanguage = gtk_menu_item_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyLanguage));
	gtk_widget_show (wLanguage);
	gtk_menu_append (GTK_MENU (FormatMenu_menu), wLanguage);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (FormatMenu), FormatMenu_menu);

	m_wFormat = wFormat;
	m_wModifyParagraph = wParagraph;
	m_wModifyFont = wFont;
	m_wModifyNumbering = wNumbering;
	m_wModifyTabs = wTabs;
	m_wModifyLanguage = wLanguage;
}

void AP_UnixDialog_Styles::_connectModifySignals(void)
{

	g_signal_connect(G_OBJECT(m_wModifyOk),
					   "clicked",
					   G_CALLBACK(s_modify_ok_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(m_wModifyCancel),
					   "clicked",
					   G_CALLBACK(s_modify_cancel_clicked),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wModifyParagraph),
					   "activate",
					   G_CALLBACK(s_modify_paragraph),
					   (gpointer) this);


	g_signal_connect(G_OBJECT(m_wModifyFont),
					   "activate",
					   G_CALLBACK(s_modify_font),
					   (gpointer) this);


	g_signal_connect(G_OBJECT(m_wModifyNumbering),
					   "activate",
					   G_CALLBACK(s_modify_numbering),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wModifyTabs),
					   "activate",
					   G_CALLBACK(s_modify_tabs),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wModifyLanguage),
					   "activate",
					   G_CALLBACK(s_modify_language),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wModifyDrawingArea),
					   "expose_event",
					   G_CALLBACK(s_modifyPreview_exposed),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wDeletePropButton),
					   "clicked",
					   G_CALLBACK(s_remove_property),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wStyleNameEntry),
					   "changed",
					   G_CALLBACK(s_style_name),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wBasedOnEntry), 
					   "changed",
					   G_CALLBACK(s_basedon),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wFollowingEntry), 
					   "changed",
					   G_CALLBACK(s_followedby),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wStyleTypeEntry), 
					   "changed",
					   G_CALLBACK(s_styletype),
					   (gpointer) this);

	
	g_signal_connect_after(G_OBJECT(m_wModifyDialog),
							 "expose_event",
							 G_CALLBACK(s_modify_window_exposed),
							 (gpointer) this);

	// the catch-alls
	
	g_signal_connect(G_OBJECT(m_wModifyDialog),
					   "delete_event",
					   G_CALLBACK(s_modify_delete_clicked),
					   (gpointer) this);

	g_signal_connect_after(G_OBJECT(m_wModifyDialog),
							 "destroy",
							 NULL,
							 NULL);
}


void AP_UnixDialog_Styles::event_Modify_OK(void)
{
  const char * text = gtk_entry_get_text (GTK_ENTRY (m_wStyleNameEntry));

  if (!text || !strlen (text))
    {
      // error message!
      const XAP_StringSet * pSS = m_pApp->getStringSet ();
      const char * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrBlankName);

      getFrame()->showMessageBox ((const char *)msg,
				  XAP_Dialog_MessageBox::b_O,
				  XAP_Dialog_MessageBox::a_OK);

      return;
    }

	// TODO save out state of radio items
	m_answer = AP_Dialog_Styles::a_OK;
	gtk_main_quit();
}

/*!
 * fill the properties vector with the values the given style.
 */
void AP_UnixDialog_Styles::new_styleName(void)
{
	static char message[200];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const gchar * psz = gtk_entry_get_text( GTK_ENTRY( m_wStyleNameEntry));
	if(psz && strcmp(psz,pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone))== 0)
	{
			// TODO: do a real error dialog
		sprintf(message,"%s%s%s",pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNotTitle1),psz,pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNotTitle2));
		messageBoxOK((const char *) message);
		return;
	}
	if(psz && strcmp(psz,pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent))== 0)
	{
			// TODO: do a real error dialog
		sprintf(message,"%s%s%s",pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNotTitle1),psz,pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNotTitle2));
		messageBoxOK((const char *) message);
		return;
	}

	g_snprintf((gchar *) m_newStyleName,40,"%s",psz);
	addOrReplaceVecAttribs(PT_NAME_ATTRIBUTE_NAME,getNewStyleName());
}

/*!
 * Remove the property from the current style shown in the remove combo box
 */
void AP_UnixDialog_Styles::event_RemoveProperty(void)
{
	const gchar * psz = gtk_entry_get_text( GTK_ENTRY(m_wDeletePropEntry));
	removeVecProp(psz);
	rebuildDeleteProps();
	updateCurrentStyle();
}

void AP_UnixDialog_Styles::rebuildDeleteProps(void)
{
	GtkCombo* delCombo = GTK_COMBO(m_wDeletePropCombo);
	GtkList * oldList = GTK_LIST(delCombo->list);
	if(oldList != NULL)
	{
		gtk_list_clear_items(oldList,0,-1);
	}
	UT_sint32 count = m_vecAllProps.getItemCount();
	UT_sint32 i= 0;
	for(i=0; i< count; i+=2)
	{
		gchar * sz = (gchar *) m_vecAllProps.getNthItem(i);
		GtkWidget * li = gtk_list_item_new_with_label(sz);
		gtk_widget_show(li);
		gtk_container_add(GTK_CONTAINER(delCombo->list),li);
	}
}

/*!
 * Update the properties and Attributes vector given the new basedon name
 */
void AP_UnixDialog_Styles::event_basedOn(void)
{
	const gchar * psz = gtk_entry_get_text( GTK_ENTRY( m_wBasedOnEntry));
	g_snprintf((gchar *) m_basedonName,40,"%s",psz);
	addOrReplaceVecAttribs("basedon",getBasedonName());
	fillVecWithProps(getBasedonName(),false);
	updateCurrentStyle();
}


/*!
 * Update the Attributes vector given the new followedby name
 */
void AP_UnixDialog_Styles::event_followedBy(void)
{
	const gchar * psz = gtk_entry_get_text( GTK_ENTRY(m_wFollowingEntry));
	g_snprintf((gchar *) m_followedbyName,40,"%s",psz);
	addOrReplaceVecAttribs("followedby",getFollowedbyName());
}


/*!
 * Update the Attributes vector given the new Style Type
 */
void AP_UnixDialog_Styles::event_styleType(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const gchar * psz = gtk_entry_get_text( GTK_ENTRY(m_wStyleTypeEntry));
	g_snprintf((gchar *) m_styleType,40,"%s",psz);
	const XML_Char * pszSt = "P";
	if(strstr(m_styleType, pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyCharacter)) != 0)
		pszSt = "C";
	addOrReplaceVecAttribs("type",pszSt);
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

void  AP_UnixDialog_Styles::modifyRunModal(void)
{
//
// OK Construct the new dialog and make it modal.
//
//
// pointer to the widget is stored in m_wModifyDialog
//
// Center our new dialog in its parent and make it a transient

	_constructModifyDialog();

	connectFocus(GTK_WIDGET(m_wModifyDialog),getFrame());
//
// populate the dialog with useful info
//
    if(!_populateModify())
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
    _populateAbiPreview(isNew());
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

		if(m_gStyleType != NULL)
		{
			g_list_free (m_gStyleType);
			m_gStyleType = NULL;
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
	PD_Style * pStyle = NULL;
	const char * szCurrentStyle = getCurrentStyle ();
	
	if(szCurrentStyle)
		getDoc()->getStyle(szCurrentStyle, &pStyle);
	
	if (!pStyle)
	{
		// TODO: error message - nothing selected
		return;
	}
//
// Allow built-ins to be modified
//
#if 0
	if (!pStyle->isUserDefined ())
	{
		// can't change builtin, error message
		const XAP_StringSet * pSS = m_pApp->getStringSet();
		const XML_Char * msg = pSS->getValue (AP_STRING_ID_DLG_Styles_ErrStyleBuiltin);
		
		getFrame()->showMessageBox ((const char *)msg,
									XAP_Dialog_MessageBox::b_O,
									XAP_Dialog_MessageBox::a_OK);
		return;
	}	
#endif
	
#ifndef HAVE_GNOME
//
// Hide the old window
//
    gtk_widget_hide(m_windowMain);
#endif
//
// fill the data structures needed for the Modify dialog
//
	setIsNew(false);
	
	modifyRunModal();
	if(m_answer == AP_Dialog_Styles::a_OK)
	{
		applyModifiedStyleToDoc();
		getDoc()->updateDocForStyleChange(getCurrentStyle(),true);
		getDoc()->signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
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
	
#ifndef HAVE_GNOME
//
// Reveal main window again
//
	gtk_widget_show( m_windowMain);
#endif
}

void  AP_UnixDialog_Styles::setModifyDescription( const char * desc)
{
	UT_ASSERT(m_wlabelDesc);
	gtk_label_set_text (GTK_LABEL(m_wLabDescription), desc);
}

bool  AP_UnixDialog_Styles::_populateModify(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
//
// Don't do any callback while setting up stuff here.
//
	setModifySignalBlocked(true);
	setModifyDescription( m_curStyleDesc.c_str());
//
// Get Style name and put in in the text entry
//
	const char * szCurrentStyle = NULL;
	if(!isNew())
	{
		szCurrentStyle= getCurrentStyle();
		if(!szCurrentStyle)
		{
			// TODO: change me to use a real messagebox
			messageBoxOK( pSS->getValue(AP_STRING_ID_DLG_Styles_ErrNoStyle));
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
		gtk_entry_set_text (GTK_ENTRY(m_wStyleNameEntry), getCurrentStyle());
		gtk_entry_set_editable( GTK_ENTRY(m_wStyleNameEntry),FALSE );
	}
	else
	{
		gtk_entry_set_editable( GTK_ENTRY(m_wStyleNameEntry),TRUE );
	}
//
// Next interogate the current style and find the based on and followed by
// Styles
//
	const char * szBasedOn = NULL;
	const char * szFollowedBy = NULL;
	PD_Style * pBasedOnStyle = NULL;
	PD_Style * pFollowedByStyle = NULL;
	if(!isNew())
	{
		PD_Style * pStyle = NULL;
		if(szCurrentStyle)
			getDoc()->getStyle(szCurrentStyle,&pStyle);
		if(!pStyle)
		{
			// TODO: do a real error dialog
			messageBoxOK( pSS->getValue(AP_STRING_ID_DLG_Styles_ErrStyleNot));
			m_answer = AP_Dialog_Styles::a_CANCEL;
			return false;
		}
//
// Valid style get the Based On and followed by values
//
	    pBasedOnStyle = pStyle->getBasedOn();
		pFollowedByStyle = pStyle->getFollowedBy();
	}
//
// Next make a glists of all styles and attach them to the BasedOn and FollowedBy
//
	size_t nStyles = getDoc()->getStyleCount();
	const char * name = NULL;
	const PD_Style * pcStyle = NULL;
	for (UT_uint32 i = 0; i < nStyles; i++)
	{
	    getDoc()->enumStyles(i, &name, &pcStyle);

		if(pBasedOnStyle && pcStyle == pBasedOnStyle)
		{
			szBasedOn = name;
		}
		if(pFollowedByStyle && pcStyle == pFollowedByStyle)
			szFollowedBy = name;
		if(szCurrentStyle && strcmp(name,szCurrentStyle) != 0)
			m_gbasedOnStyles = g_list_append (m_gbasedOnStyles, (gpointer) name);
		else if(szCurrentStyle == NULL)
			m_gbasedOnStyles = g_list_append (m_gbasedOnStyles, (gpointer) name);

		m_gfollowedByStyles = g_list_append (m_gfollowedByStyles, (gpointer) name);
	}
	m_gfollowedByStyles = g_list_append (m_gfollowedByStyles, (gpointer)  pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent));
	m_gbasedOnStyles = g_list_append (m_gbasedOnStyles, (gpointer)  pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone));
	m_gStyleType = g_list_append(m_gStyleType, (gpointer) pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph) );
	m_gStyleType = g_list_append(m_gStyleType, (gpointer) pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyCharacter));
 
//
// Set the popdown list
//
	gtk_combo_set_popdown_strings( GTK_COMBO(m_wBasedOnCombo),m_gbasedOnStyles);
	gtk_combo_set_popdown_strings( GTK_COMBO(m_wFollowingCombo),m_gfollowedByStyles);
	if(isNew())
	{
		gtk_combo_set_popdown_strings( GTK_COMBO(m_wStyleTypeCombo),m_gStyleType);
	}
//
// OK here we set intial values for the basedOn and followedBy
//
	if(!isNew())
	{
		if(pBasedOnStyle != NULL)
			gtk_entry_set_text (GTK_ENTRY(m_wBasedOnEntry),szBasedOn);
		else
			gtk_entry_set_text (GTK_ENTRY(m_wBasedOnEntry), pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone));
		if(pFollowedByStyle != NULL)
			gtk_entry_set_text (GTK_ENTRY(m_wFollowingEntry),szFollowedBy);
		else
			gtk_entry_set_text (GTK_ENTRY(m_wFollowingEntry), pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent));
		if(strstr(getAttsVal("type"),"P") != 0)
		{
			gtk_entry_set_text (GTK_ENTRY(m_wStyleTypeEntry),
								pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph));
		}
		else
		{
			gtk_entry_set_text (GTK_ENTRY(m_wStyleTypeEntry),
								pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyCharacter));
		}
	}
	else
	{
//
// Hardwire defaults for "new"
//
		gtk_entry_set_text (GTK_ENTRY(m_wBasedOnEntry), pSS->getValue(AP_STRING_ID_DLG_Styles_DefNone));
		gtk_entry_set_text (GTK_ENTRY(m_wFollowingEntry), pSS->getValue(AP_STRING_ID_DLG_Styles_DefCurrent));
		gtk_entry_set_text (GTK_ENTRY(m_wStyleTypeEntry),
							pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyParagraph));
	}
	gtk_entry_set_editable(GTK_ENTRY(m_wFollowingEntry),FALSE );
	gtk_entry_set_editable(GTK_ENTRY(m_wBasedOnEntry),FALSE );
	gtk_entry_set_editable(GTK_ENTRY(m_wStyleTypeEntry),FALSE );
//
// Set these in our attributes vector
//
	event_basedOn();
	event_followedBy();
	event_styleType();
	if(isNew())
	{
		fillVecFromCurrentPoint();
	}
	else
	{
		fillVecWithProps(szCurrentStyle,true);
	}
//
// Allow callback's now.
//
	setModifySignalBlocked(false);
//
// Now set the list of properties which can be deleted.
//
	rebuildDeleteProps();
	gtk_entry_set_text(GTK_ENTRY(m_wDeletePropEntry),"");
	return true;
}

void   AP_UnixDialog_Styles::event_ModifyParagraph()
{
#ifndef HAVE_GNOME
//
// Hide this window
//
    gtk_widget_hide(m_wModifyDialog);
#endif

//
// Can do all this in XP land.
//
	ModifyParagraph();
	rebuildDeleteProps();
#ifndef HAVE_GNOME
//
// Restore this window
//
    gtk_widget_show(m_wModifyDialog);
#endif

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();
}

void   AP_UnixDialog_Styles::event_ModifyFont()
{
#ifndef HAVE_GNOME
//
// Hide this window
//
    gtk_widget_hide(m_wModifyDialog);
#endif

//
// Can do all this in XP land.
//
	ModifyFont();
	rebuildDeleteProps();
#ifndef HAVE_GNOME
//
// Restore this window
//
    gtk_widget_show(m_wModifyDialog);
#endif

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();
}

void AP_UnixDialog_Styles::event_ModifyLanguage()
{
#ifndef HAVE_GNOME
	gtk_widget_hide (m_wModifyDialog);
#endif

	ModifyLang();
	rebuildDeleteProps();
#ifndef HAVE_GNOME
	gtk_widget_show (m_wModifyDialog);
#endif

	updateCurrentStyle();
}

void   AP_UnixDialog_Styles::event_ModifyNumbering()
{
#ifndef HAVE_GNOME
//
// Hide this window
//
    gtk_widget_hide(m_wModifyDialog);
#endif

//
// Can do all this in XP land.
//
	ModifyLists();
	rebuildDeleteProps();
#ifndef HAVE_GNOME
//
// Restore this window
//
    gtk_widget_show(m_wModifyDialog);
#endif

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();

}


void   AP_UnixDialog_Styles::event_ModifyTabs()
{
#ifndef HAVE_GNOME
//
// Hide this window
//
    gtk_widget_hide(m_wModifyDialog);
#endif

//
// Can do all this in XP land.
//
	ModifyTabs();
	rebuildDeleteProps();
#ifndef HAVE_GNOME
//
// Restore this window
//
    gtk_widget_show(m_wModifyDialog);
#endif

//
// This applies the changes to current style and displays them
//
	updateCurrentStyle();
}

bool  AP_UnixDialog_Styles::isModifySignalBlocked(void) const
{
	return m_bBlockModifySignal;
}

void  AP_UnixDialog_Styles::setModifySignalBlocked( bool val)
{
	m_bBlockModifySignal = val;
}









