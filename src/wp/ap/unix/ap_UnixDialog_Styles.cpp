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

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Styles::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_Styles * p = new AP_UnixDialog_Styles(pFactory,id);
	return p;
}

AP_UnixDialog_Styles::AP_UnixDialog_Styles(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Styles(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_wbuttonOk = NULL;
	m_wbuttonCancel = NULL;
	m_wGnomeButtons = NULL;
        m_wParaPreviewArea = NULL;
        m_pParaPreviewWidget = NULL;
        m_wCharPreviewArea = NULL;
        m_pCharPreviewWidget = NULL;

}

AP_UnixDialog_Styles::~AP_UnixDialog_Styles(void)
{
	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
}

/*****************************************************************/

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

/*****************************************************************/

void AP_UnixDialog_Styles::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
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



	// *** this is how we add the gc for the para and char Preview's ***
	// attach a new graphics context to the drawing area
	/*
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

        // Run into the GTK event loop for this window.
	*/

	gtk_main();

	DELETEP (m_pParaPreviewWidget);
	DELETEP (m_pCharPreviewWidget);
	
	if(mainWindow && GTK_IS_WIDGET(mainWindow))
	  gtk_widget_destroy(mainWindow);
}


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
		m_pCharPreview->draw();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Styles::_constructWindow(void)
{
	GtkWidget * windowStyles;

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


	GtkWidget * hsepMid;
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
	listStyles = gtk_clist_new(1);
	gtk_container_add(GTK_CONTAINER(frameStyles), listStyles);

	gtk_box_pack_start(GTK_BOX(vboxTopLeft), frameStyles, TRUE, TRUE, 2);
	gtk_widget_show(frameStyles);
	gtk_widget_show(listStyles);

	frameList = gtk_frame_new(
		pSS->getValue(AP_STRING_ID_DLG_Styles_List));
	comboList = gtk_combo_new();
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
	DescriptionArea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(DescriptionArea), 300, 60);
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


	hsepMid = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vboxContents), hsepMid, FALSE, FALSE, 0);
	gtk_widget_show(hsepMid);


	m_windowMain = windowStyles;
	m_wbuttonNew = buttonNew;
	m_wbuttonModify = buttonModify;
	m_wbuttonDelete = buttonDelete;
	m_wParaPreviewArea = ParaPreviewArea;
	m_wCharPreviewArea = CharPreviewArea;

	return vboxContents;
}

void AP_UnixDialog_Styles::_connectsignals(void)
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

void AP_UnixDialog_Styles::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}
