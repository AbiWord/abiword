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
#include "ap_Dialog_Columns.h"
#include "ap_UnixDialog_Columns.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_Columns * p = new AP_UnixDialog_Columns(pFactory,id);
	return p;
}

AP_UnixDialog_Columns::AP_UnixDialog_Columns(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_wbuttonOk = NULL;
	m_wbuttonCancel = NULL;
	m_wlineBetween = NULL;
	m_wtoggleOne = NULL;
	m_wtoggleTwo = NULL;
	m_wtoggleThree = NULL;
	m_windowMain = NULL;
        m_wpreviewArea = NULL;
	m_wGnomeButtons = NULL;
        m_pPreviewWidget = NULL;

}

AP_UnixDialog_Columns::~AP_UnixDialog_Columns(void)
{
	DELETEP (m_pPreviewWidget);
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}


static void s_one_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Toggle(1);
}


static void s_two_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Toggle(2);
}


static void s_three_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Toggle(3);
}


static void s_line_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->checkLineBetween();
}

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
}

static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_previewExposed();
	return FALSE;
}


static gboolean s_window_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_previewExposed();
	return FALSE;
}


static void s_delete_clicked(GtkWidget * /* widget */, gpointer /* data */,
			     AP_UnixDialog_Columns * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

/*****************************************************************/

void AP_UnixDialog_Columns::runModal(XAP_Frame * pFrame)
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



	// *** this is how we add the gc for Column Preview ***
	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	UT_ASSERT(unixapp);

	UT_ASSERT(m_wpreviewArea && m_wpreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	m_pPreviewWidget = new GR_UnixGraphics(m_wpreviewArea->window, unixapp->getFontManager(), m_pApp);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
			     (UT_uint32) m_wpreviewArea->allocation.width, 
			     (UT_uint32) m_wpreviewArea->allocation.height);

	setLineBetween(getLineBetween());
	if(getLineBetween()==UT_TRUE)
	{
	       gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wlineBetween),TRUE);
	}
	// Now draw the columns

	event_Toggle(getColumns());

        // Run into the GTK event loop for this window.

	gtk_main();

	_storeWindowData();
	DELETEP (m_pPreviewWidget);
	
	gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_Columns::checkLineBetween(void)
{
        if (GTK_TOGGLE_BUTTON (m_wlineBetween)->active)
        {
	        setLineBetween(UT_TRUE);
	}
	else
        {
	        setLineBetween(UT_FALSE);
	}
}

void AP_UnixDialog_Columns::event_Toggle( UT_uint32 icolumns)
{
	checkLineBetween();
        gtk_signal_handler_block(GTK_OBJECT(m_wtoggleOne), 
					  m_oneHandlerID);
        gtk_signal_handler_block(GTK_OBJECT(m_wtoggleTwo), 
					  m_twoHandlerID);
        gtk_signal_handler_block(GTK_OBJECT(m_wtoggleThree), 
					  m_threeHandlerID);
	switch (icolumns)
        {
        case 1:
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),TRUE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
		 break;
        case 2:
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),TRUE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
		 break;
	case 3:
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),TRUE);
		 break;
	default:
	         UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
        gtk_signal_handler_unblock(GTK_OBJECT(m_wtoggleOne), 
					  m_oneHandlerID);
        gtk_signal_handler_unblock(GTK_OBJECT(m_wtoggleTwo), 
					  m_twoHandlerID);
        gtk_signal_handler_unblock(GTK_OBJECT(m_wtoggleThree), 
					  m_threeHandlerID);
	setColumns( icolumns );
	m_pColumnsPreview->draw();
}


void AP_UnixDialog_Columns::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Columns::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Columns::event_WindowDelete(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_Columns::event_previewExposed(void)
{
        if(m_pColumnsPreview)
	       m_pColumnsPreview->draw();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Columns::_constructWindow(void)
{

	GtkWidget * windowColumns;

	GtkWidget * buttonOK;
	GtkWidget * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	//	XML_Char * unixstr = NULL;	// used for conversions

	windowColumns = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW (windowColumns), pSS->getValue(AP_STRING_ID_DLG_Column_ColumnTitle));
	gtk_window_set_policy (GTK_WINDOW (windowColumns), FALSE, FALSE, FALSE);

	_constructWindowContents(windowColumns);

	// These buttons need to be gnomified

	buttonOK = gtk_button_new_with_label ( pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show(buttonOK );
	gtk_container_add (GTK_CONTAINER (m_wGnomeButtons), buttonOK);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label ( pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show(buttonCancel );
	gtk_container_add (GTK_CONTAINER (m_wGnomeButtons), buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	m_wbuttonOk = buttonOK;
        m_wbuttonCancel = buttonCancel;

	_connectsignals();
	return windowColumns;
}

void AP_UnixDialog_Columns::_constructWindowContents(GtkWidget * windowColumns)
{


	GtkWidget *frame2;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *wColumnFrame;
	GtkWidget *wSelectFrame;
	GtkWidget *vbox2;
	GtkWidget *hbox3;
	GtkWidget *wToggleOne;
	GtkWidget *wLabelOne;
	GtkWidget *hbox4;
	GtkWidget *wToggleTwo;
	GtkWidget *wLabelTwo;
	GtkWidget *hbox5;
	GtkWidget *wToggleThree;
	GtkWidget *wLabelThree;
	GtkWidget *wPreviewFrame;
	GtkWidget *wDrawFrame;
	GtkWidget *wPreviewArea;
	GtkWidget *vbuttonbox1;

	GtkWidget *hbox2;
	GtkWidget *wLineBtween;
	GtkWidget *wLabelLineBetween;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	frame2 = gtk_frame_new (NULL);
	gtk_widget_show(frame2);
	gtk_container_add (GTK_CONTAINER (windowColumns), frame2);
	gtk_container_set_border_width (GTK_CONTAINER (frame2), 16);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show(vbox1);
	gtk_container_add (GTK_CONTAINER (frame2), vbox1);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	wColumnFrame = gtk_frame_new ( pSS->getValue(AP_STRING_ID_DLG_Column_Number));
	gtk_widget_show(wColumnFrame);
	gtk_box_pack_start (GTK_BOX (hbox1), wColumnFrame, TRUE, TRUE, 7);

	wSelectFrame = gtk_frame_new (NULL);
	gtk_widget_show(wSelectFrame );
	gtk_container_add (GTK_CONTAINER (wColumnFrame), wSelectFrame);
	gtk_container_set_border_width (GTK_CONTAINER (wSelectFrame), 9);
	gtk_frame_set_shadow_type (GTK_FRAME (wSelectFrame), GTK_SHADOW_NONE);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show(vbox2 );
	gtk_container_add (GTK_CONTAINER (wSelectFrame), vbox2);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox3 );
	gtk_box_pack_start (GTK_BOX (vbox2), hbox3, TRUE, TRUE, 0);

	wToggleOne = gtk_toggle_button_new();
	gtk_widget_show(wToggleOne );
        UT_Bool butlab = label_button_with_abi_pixmap(wToggleOne, "tb_1column_xpm");
	gtk_box_pack_start (GTK_BOX (hbox3), wToggleOne, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (wToggleOne, GTK_CAN_DEFAULT);

	wLabelOne = gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Column_One));
	gtk_widget_show(wLabelOne );
	gtk_box_pack_start (GTK_BOX (hbox3), wLabelOne, FALSE, FALSE, 0);
	
	hbox4 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox4 );
	gtk_box_pack_start (GTK_BOX (vbox2), hbox4, TRUE, TRUE, 0);

	wToggleTwo = gtk_toggle_button_new ();
	gtk_widget_show(wToggleTwo );
        label_button_with_abi_pixmap(wToggleTwo, "tb_2column_xpm");
	gtk_box_pack_start (GTK_BOX (hbox4), wToggleTwo, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (wToggleTwo, GTK_CAN_DEFAULT);

	wLabelTwo = gtk_label_new( pSS->getValue(AP_STRING_ID_DLG_Column_Two));
	gtk_widget_show(wLabelTwo );
	gtk_box_pack_start (GTK_BOX (hbox4), wLabelTwo, FALSE, FALSE, 0);
	
	hbox5 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox5 );
	gtk_widget_show (hbox5);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox5, TRUE, TRUE, 0);

	wToggleThree = gtk_toggle_button_new ();
	gtk_widget_show(wToggleThree );
        label_button_with_abi_pixmap(wToggleThree, "tb_3column_xpm");
	gtk_box_pack_start (GTK_BOX (hbox5), wToggleThree, FALSE, FALSE, 0);
	GTK_WIDGET_SET_FLAGS (wToggleThree, GTK_CAN_DEFAULT);

	wLabelThree = gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Column_Three));
	gtk_widget_show(wLabelThree );
	gtk_box_pack_start (GTK_BOX (hbox5), wLabelThree, FALSE, FALSE, 0);

	wPreviewFrame = gtk_frame_new ( pSS->getValue(AP_STRING_ID_DLG_Column_Preview));
	gtk_widget_show(wPreviewFrame );
	gtk_box_pack_start (GTK_BOX (hbox1), wPreviewFrame, TRUE, TRUE, 4);
	gtk_widget_set_usize (wPreviewFrame, 100, -2); // was -2

	wDrawFrame = gtk_frame_new (NULL);
	gtk_widget_show(wDrawFrame );
	gtk_container_add (GTK_CONTAINER (wPreviewFrame), wDrawFrame);
	gtk_container_set_border_width (GTK_CONTAINER (wDrawFrame), 4);
	gtk_frame_set_shadow_type (GTK_FRAME (wDrawFrame), GTK_SHADOW_OUT);

	wPreviewArea = gtk_drawing_area_new ();
	gtk_widget_ref (wPreviewArea);
	gtk_object_set_data_full (GTK_OBJECT (windowColumns), "wPreviewArea", wPreviewArea,
								  (GtkDestroyNotify) gtk_widget_unref);

       	gtk_widget_show(wPreviewArea);
	gtk_container_add (GTK_CONTAINER (wDrawFrame), wPreviewArea);

	vbuttonbox1 = gtk_vbutton_box_new ();
	gtk_widget_show(vbuttonbox1 );
	gtk_box_pack_end (GTK_BOX (hbox1), vbuttonbox1, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1), GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbuttonbox1), 0);
//	gtk_button_box_set_child_size (GTK_BUTTON_BOX (vbuttonbox1), 74, 27);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (vbuttonbox1), 0, 1);

	m_wGnomeButtons = vbuttonbox1;

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox2 );
	gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 0);

	wLineBtween = gtk_check_button_new_with_label ("");
	gtk_widget_show(wLineBtween );
	gtk_box_pack_start (GTK_BOX (hbox2), wLineBtween, FALSE, FALSE, 3);

	wLabelLineBetween = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Column_Line_Between));
	gtk_widget_show(wLabelLineBetween );
	gtk_box_pack_start (GTK_BOX (hbox2), wLabelLineBetween, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (wLabelLineBetween), GTK_JUSTIFY_LEFT);


	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_wlineBetween = wLineBtween;
	m_wtoggleOne = wToggleOne;
	m_wtoggleTwo = wToggleTwo;
	m_wtoggleThree = wToggleThree;
	m_windowMain = windowColumns;
        m_wpreviewArea = wPreviewArea;
}

void AP_UnixDialog_Columns::_connectsignals(void)
{

	// the control buttons
	m_oneHandlerID = gtk_signal_connect(GTK_OBJECT(m_wtoggleOne),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_one_clicked),
					   (gpointer) this);

	m_twoHandlerID = gtk_signal_connect(GTK_OBJECT(m_wtoggleTwo),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_two_clicked),
					   (gpointer) this);

	m_threeHandlerID = gtk_signal_connect(GTK_OBJECT(m_wtoggleThree),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_three_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_wlineBetween),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_line_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(m_wbuttonOk),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_wbuttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	// the expose event of the preview
	             gtk_signal_connect(GTK_OBJECT(m_wpreviewArea),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_preview_exposed),
					   (gpointer) this);

	
		     gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
		     					 "expose_event",
		     				 GTK_SIGNAL_FUNC(s_window_exposed),
		    					 (gpointer) this);

	// the catch-alls
	
	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);
}

void AP_UnixDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}

void AP_UnixDialog_Columns::_storeWindowData(void)
{
}

void AP_UnixDialog_Columns::enableLineBetweenControl(UT_Bool bState)
{
}
