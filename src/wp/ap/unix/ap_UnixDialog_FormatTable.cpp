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
#include <glade/glade.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_UnixGtkColorPicker.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

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
	dlg->setBGColor(UT_RGBColor(r,g,b));
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
	m_wPreviewArea = NULL;
	m_pPreviewWidget = NULL;
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
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();
	abiSetupModelessDialog(GTK_DIALOG(m_windowMain), pFrame, this, BUTTON_CLOSE);
	
	// *** this is how we add the gc for Column Preview ***
	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);

	UT_return_if_fail(m_wPreviewArea && m_wPreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	m_pPreviewWidget = new GR_UnixGraphics(m_wPreviewArea->window, unixapp->getFontManager(), m_pApp);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wPreviewArea->style);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
						 (UT_uint32) m_wPreviewArea->allocation.width,
						 (UT_uint32) m_wPreviewArea->allocation.height);	
	
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
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_FormatTable.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_FormatTable");
	m_wLineTop = glade_xml_get_widget(xml, "tbBorderTop");
	m_wLineLeft = glade_xml_get_widget(xml, "tbBorderLeft");
	m_wLineRight = glade_xml_get_widget(xml, "tbBorderRight");
	m_wLineBottom = glade_xml_get_widget(xml, "tbBorderBottom");
	
	// the toggle buttons created by glade already contain a label, remove that, so we can add a pixmap as a child
	gtk_container_remove(GTK_CONTAINER(m_wLineTop), gtk_bin_get_child(GTK_BIN(m_wLineTop)));
	gtk_container_remove(GTK_CONTAINER(m_wLineLeft), gtk_bin_get_child(GTK_BIN(m_wLineLeft)));
	gtk_container_remove(GTK_CONTAINER(m_wLineRight), gtk_bin_get_child(GTK_BIN(m_wLineRight)));
	gtk_container_remove(GTK_CONTAINER(m_wLineBottom), gtk_bin_get_child(GTK_BIN(m_wLineBottom)));
	
	// place some nice pixmaps on our border toggle buttons
	label_button_with_abi_pixmap(m_wLineTop, "tb_LineTop_xpm");
	label_button_with_abi_pixmap(m_wLineLeft, "tb_LineLeft_xpm");
	label_button_with_abi_pixmap(m_wLineRight, "tb_LineRight_xpm");
	label_button_with_abi_pixmap(m_wLineBottom, "tb_LineBottom_xpm");
	
	m_wPreviewArea = glade_xml_get_widget(xml, "daPreview");
	
	// set the dialog title
	ConstructWindowName();
	abiDialogSetTitle(window, m_WindowName);
	
	// disable double buffering on our preview
	gtk_widget_set_double_buffered(m_wPreviewArea, FALSE); 	
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbBorder"), pSS, AP_STRING_ID_DLG_FormatTable_Borders);
	localizeLabel(glade_xml_get_widget(xml, "lbBorderColor"), pSS, AP_STRING_ID_DLG_FormatTable_Border_Color);
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbBackground"), pSS, AP_STRING_ID_DLG_FormatTable_Background);
	localizeLabel(glade_xml_get_widget(xml, "lbBackgroundColor"), pSS, AP_STRING_ID_DLG_FormatTable_Background_Color);
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbPreview"), pSS, AP_STRING_ID_DLG_FormatTable_Preview);
	
	// add the custom color picker buttons to the dialog
	m_wBorderColorButton = gtk_color_picker_new();
	gtk_widget_show(m_wBorderColorButton);
	gtk_table_attach(GTK_TABLE (glade_xml_get_widget(xml, "tbProperties")), m_wBorderColorButton, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 
					0, 6);
					
	// add the custom color picker buttons to the dialog
	m_wBackgroundColorButton = gtk_color_picker_new();
	gtk_widget_show(m_wBackgroundColorButton);
	gtk_table_attach(GTK_TABLE (glade_xml_get_widget(xml, "tbProperties")), m_wBackgroundColorButton, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 
					0, 6);
	
	// add the apply and ok buttons to the dialog
	m_wCloseButton = abiAddStockButton ( GTK_DIALOG(window), GTK_STOCK_CLOSE, BUTTON_CLOSE ) ;
	m_wApplyButton = abiAddStockButton ( GTK_DIALOG(window), GTK_STOCK_APPLY, BUTTON_APPLY ) ;	
	
	return window;
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
						   
	g_signal_connect(G_OBJECT(m_wPreviewArea),
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
