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

#include "xap_UnixGtkColorPicker.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatTable.h"
#include "ap_UnixDialog_FormatTable.h"
#include "ap_UnixDialog_Columns.h"

static void s_apply_changes(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	dlg->applyChanges();
}

static void s_close_window(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	dlg->event_Close();
}

static void s_line_left(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_left, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_right(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_right, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_top(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_top, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_bottom(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_bottom, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_border_color(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	
	guint8 r, g, b, a;
	gtk_color_picker_get_i8 (GTK_COLOR_PICKER(widget), &r, &g, &b, &a);
	dlg->setBorderColor(UT_RGBColor(r,g,b));
	dlg->event_previewExposed();
}

static void s_background_color(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = (AP_UnixDialog_FormatTable *)data;
	UT_return_if_fail(widget && dlg);
	
	guint8 r, g, b, a;
	gtk_color_picker_get_i8 (GTK_COLOR_PICKER(widget), &r, &g, &b, &a);
	dlg->setBackgroundColor(UT_RGBColor(r,g,b));
	dlg->event_previewExposed();
}

static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_FormatTable * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_FormatTable::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_UnixDialog_FormatTable * p = new AP_UnixDialog_FormatTable(pFactory,id);
	return p;
}

AP_UnixDialog_FormatTable::AP_UnixDialog_FormatTable(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_FormatTable(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_wpreviewArea = NULL;
	m_pPreviewWidget = NULL;
	m_wContents = NULL;
	m_wApplyButton = NULL;
	m_wBorderColorButton = NULL;
	m_wLineLeft = NULL;
	m_wLineRight = NULL;
	m_wLineTop = NULL;
	m_wLineBottom = NULL;	
}

AP_UnixDialog_FormatTable::~AP_UnixDialog_FormatTable(void)
{
}

void AP_UnixDialog_FormatTable::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();
	abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,BUTTON_CLOSE);
	
	// *** this is how we add the gc for Column Preview ***
	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);

	UT_return_if_fail(m_wpreviewArea && m_wpreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	m_pPreviewWidget = new GR_UnixGraphics(m_wpreviewArea->window, unixapp->getFontManager(), m_pApp);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wpreviewArea->style);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
						 (UT_uint32) m_wpreviewArea->allocation.width,
						 (UT_uint32) m_wpreviewArea->allocation.height);	
	
	m_pFormatTablePreview->draw();
	
	startUpdater();
}

void AP_UnixDialog_FormatTable::setSensitivity(bool bSens)
{
	gtk_widget_set_sensitive(m_wBorderColorButton, bSens);
	gtk_widget_set_sensitive(m_wBackgroundColorButton, bSens);	
	gtk_widget_set_sensitive(m_wLineLeft, bSens);
	gtk_widget_set_sensitive(m_wLineRight, bSens);
	gtk_widget_set_sensitive(m_wLineTop, bSens);
	gtk_widget_set_sensitive(m_wLineBottom, bSens);
	gtk_widget_set_sensitive(m_wApplyButton, bSens);
}

void AP_UnixDialog_FormatTable::event_Close(void)
{
	m_answer = AP_Dialog_FormatTable::a_CLOSE;
	destroy();
}

void AP_UnixDialog_FormatTable::event_previewExposed(void)
{
	if(m_pFormatTablePreview)
		m_pFormatTablePreview->draw();
}

void AP_UnixDialog_FormatTable::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_FormatTable::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_FormatTable::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_FormatTable::_constructWindow(void)
{
	GtkWidget * vboxMain;
	GtkWidget * windowFormatTable;
	ConstructWindowName();
	windowFormatTable = abiDialogNew ( "format table dialog", TRUE, (char *) m_WindowName);
	
	vboxMain = GTK_DIALOG(windowFormatTable)->vbox ;
	gtk_container_set_border_width (GTK_CONTAINER (vboxMain), 10);	
	_constructWindowContents();
	gtk_box_pack_start (GTK_BOX (vboxMain), m_wContents, TRUE, TRUE, 6);

	m_wCloseButton = abiAddStockButton ( GTK_DIALOG(windowFormatTable), GTK_STOCK_CLOSE, BUTTON_CLOSE ) ;
	m_wApplyButton = abiAddStockButton ( GTK_DIALOG(windowFormatTable), GTK_STOCK_APPLY, BUTTON_APPLY ) ;

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowFormatTable;

	return windowFormatTable;
}

GtkWidget * AP_UnixDialog_FormatTable::_constructWindowContents(void)
{
	GtkWidget *wContents;
	GtkWidget *hboxContents;
	GtkWidget *borderContents;
	GtkWidget *backgroundContents;
	GtkWidget *notebook;
	
	GtkWidget *vboxBorderColorStyle;
	GtkWidget *hboxBorderColorLabel;
	GtkWidget *labelBorderColor;
	GtkWidget *borderColorButton;
	
	GtkWidget *vboxBackgroundColorStyle;
	GtkWidget *hboxBackgroundColorLabel;
	GtkWidget *labelBackgroundColor;
	GtkWidget *backgroundColorButton;
	
	GtkWidget *vboxPreviewApplyTo;
	GtkWidget *framePreview;
	GtkWidget *tablePreview;	
	GtkWidget *wPreviewArea;
	GtkWidget *wLineTop;
	GtkWidget *wLineBottom;
	GtkWidget *wLineLeft;
	GtkWidget *wLineRight;
	GtkWidget *hboxApplyTo;
	GtkWidget *labelApplyTo;
	GtkWidget *comboApplyTo;	
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	wContents = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (wContents);

	hboxContents = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxContents);
	gtk_container_add(GTK_CONTAINER(wContents), hboxContents);

	borderContents = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(borderContents);

	backgroundContents = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(backgroundContents);

	notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), borderContents, gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Borders).c_str()));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), backgroundContents, gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Background).c_str()));
	gtk_widget_show(notebook);
	gtk_container_add(GTK_CONTAINER(hboxContents), notebook);

//
// construct the border tab
//

	vboxBorderColorStyle = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxBorderColorStyle);
	gtk_box_pack_start (GTK_BOX (borderContents), vboxBorderColorStyle, FALSE, TRUE, 6);

	hboxBorderColorLabel = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxBorderColorLabel);
	gtk_box_pack_start (GTK_BOX (vboxBorderColorStyle), hboxBorderColorLabel, FALSE, TRUE, 6);

	labelBorderColor = gtk_label_new(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Border_Color).c_str());
	gtk_widget_show(labelBorderColor);
	gtk_box_pack_start (GTK_BOX (hboxBorderColorLabel), labelBorderColor, FALSE, FALSE, 0);

	borderColorButton = gtk_color_picker_new();
	gtk_widget_show(borderColorButton);
	gtk_box_pack_start (GTK_BOX (hboxBorderColorLabel), borderColorButton, TRUE, FALSE, 0);

	vboxPreviewApplyTo = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxPreviewApplyTo);
	gtk_box_pack_start (GTK_BOX (hboxContents), vboxPreviewApplyTo, FALSE, FALSE, 6);

	framePreview = gtk_frame_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Preview).c_str());
	gtk_widget_show(framePreview);
	gtk_container_add(GTK_CONTAINER(vboxPreviewApplyTo), framePreview);
	
	tablePreview = gtk_table_new(3, 3, FALSE);
	gtk_widget_show(tablePreview);
	gtk_container_add(GTK_CONTAINER (framePreview), tablePreview);

	wPreviewArea = createDrawingArea ();
	gtk_widget_show(wPreviewArea);
	gtk_widget_set_usize(wPreviewArea, 140, 140);
	gtk_table_attach (GTK_TABLE (tablePreview), wPreviewArea, 1, 2, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 10, 10);

	wLineTop = gtk_toggle_button_new();
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(wLineTop), true);
	gtk_widget_show (wLineTop);
	gtk_widget_set_usize (wLineTop, 25, 25);
	label_button_with_abi_pixmap(wLineTop, "tb_LineTop_xpm");
	gtk_table_attach (GTK_TABLE (tablePreview), wLineTop, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 0);
					
	wLineBottom = gtk_toggle_button_new();
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(wLineBottom), true);
	gtk_widget_show (wLineBottom);
	gtk_widget_set_usize (wLineBottom, 25, 25);
	label_button_with_abi_pixmap(wLineBottom, "tb_LineBottom_xpm");
	gtk_table_attach (GTK_TABLE (tablePreview), wLineBottom, 1, 2, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 0);

	wLineLeft = gtk_toggle_button_new();
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(wLineLeft), true);
	gtk_widget_show (wLineLeft);
	gtk_widget_set_usize (wLineLeft, 25, 25);
	label_button_with_abi_pixmap(wLineLeft, "tb_LineLeft_xpm");
	gtk_table_attach (GTK_TABLE (tablePreview), wLineLeft, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 0);

	wLineRight = gtk_toggle_button_new();
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(wLineRight), true);
	gtk_widget_show (wLineRight);
	gtk_widget_set_usize (wLineRight, 25, 25);
	label_button_with_abi_pixmap(wLineRight, "tb_LineRight_xpm");
	gtk_table_attach (GTK_TABLE (tablePreview), wLineRight, 2, 3, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 3, 0);

	hboxApplyTo = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxApplyTo);
	gtk_container_add(GTK_CONTAINER(vboxPreviewApplyTo), hboxApplyTo);

	labelApplyTo = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To).c_str());
	gtk_widget_show(labelApplyTo);
	gtk_misc_set_alignment (GTK_MISC (labelApplyTo), 0, 0.5);
	gtk_container_add(GTK_CONTAINER(hboxApplyTo), labelApplyTo);

	GList *items = NULL;

	items = g_list_append (items, (void *)"Selection");
	items = g_list_append (items, (void *)"Cell");
	items = g_list_append (items, (void *)"Row");
	items = g_list_append (items, (void *)"Column");
	items = g_list_append (items, (void *)"Table");
	
	comboApplyTo = gtk_combo_new ();
	gtk_combo_set_popdown_strings (GTK_COMBO (comboApplyTo), items);
	gtk_widget_show(comboApplyTo);
	gtk_container_add(GTK_CONTAINER(hboxApplyTo), comboApplyTo);
	gtk_widget_set_sensitive(comboApplyTo, false); // disable for now, not implemented yet

//
// construct the background tab
//

	vboxBackgroundColorStyle = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxBackgroundColorStyle);
	gtk_box_pack_start (GTK_BOX (backgroundContents), vboxBackgroundColorStyle, FALSE, TRUE, 6);

	hboxBackgroundColorLabel = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxBackgroundColorLabel);
	gtk_box_pack_start (GTK_BOX (vboxBackgroundColorStyle), hboxBackgroundColorLabel, FALSE, TRUE, 6);

	labelBackgroundColor = gtk_label_new(pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Border_Color).c_str());
	gtk_widget_show(labelBackgroundColor);
	gtk_box_pack_start (GTK_BOX (hboxBackgroundColorLabel), labelBackgroundColor, FALSE, FALSE, 0);

	backgroundColorButton = gtk_color_picker_new();
	gtk_widget_show(backgroundColorButton);
	gtk_box_pack_start (GTK_BOX (hboxBackgroundColorLabel), backgroundColorButton, TRUE, FALSE, 0);


	//hboxBackground = gtk_hbox_new(FALSE, 0);
	//gtk_widget_show(hboxBackground);
	//gtk_container_add(GTK_CONTAINER(backgroundContents), hboxBackground);


	m_wLineTop = wLineTop;
	m_wLineBottom = wLineBottom;
	m_wLineLeft = wLineLeft;
	m_wLineRight = wLineRight;

	m_wBorderColorButton = borderColorButton;
	m_wBackgroundColorButton = backgroundColorButton;
	m_wpreviewArea = wPreviewArea;
	m_wContents = wContents;
	
	return m_wContents;
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_FormatTable * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}


static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * dlg)
{
	abiDestroyWidget(widget);
}

void AP_UnixDialog_FormatTable::_connectSignals(void)
{
	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(GTK_OBJECT(m_windowMain),
							"destroy",
							GTK_SIGNAL_FUNC(s_destroy_clicked),
							(gpointer) this);
	g_signal_connect(GTK_OBJECT(m_windowMain),
							"delete_event",
							GTK_SIGNAL_FUNC(s_delete_clicked),
							(gpointer) this);

	g_signal_connect(G_OBJECT(m_wApplyButton),
							"clicked",
							G_CALLBACK(s_apply_changes),
							(gpointer) this);

	g_signal_connect(G_OBJECT(m_wCloseButton),
							"clicked",
							G_CALLBACK(s_close_window),
							(gpointer) this);
	
	g_signal_connect(G_OBJECT(m_wLineLeft),
							"clicked",
							G_CALLBACK(s_line_left),
							(gpointer) this);
	g_signal_connect(G_OBJECT(m_wLineRight),
							"clicked",
							G_CALLBACK(s_line_right),
							(gpointer) this);
	g_signal_connect(G_OBJECT(m_wLineTop),
							"clicked",
							G_CALLBACK(s_line_top),
							(gpointer) this);
	g_signal_connect(G_OBJECT(m_wLineBottom),
							"clicked",
							G_CALLBACK(s_line_bottom),
							(gpointer) this);		   
						   
	g_signal_connect(G_OBJECT(m_wBorderColorButton),
							"color_set",
							G_CALLBACK(s_border_color),
							(gpointer) this);

	g_signal_connect(G_OBJECT(m_wBackgroundColorButton),
							"color_set",
							G_CALLBACK(s_background_color),
							(gpointer) this);	   
						   
	g_signal_connect(G_OBJECT(m_wpreviewArea),
							"expose_event",
							G_CALLBACK(s_preview_exposed),
							(gpointer) this);						   
}

void AP_UnixDialog_FormatTable::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_UnixDialog_FormatTable::_storeWindowData(void)
{
}
