/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
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
#include <gdk/gdk.h>
#include "ut_locale.h"

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_unixMisc.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

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
	AP_UnixDialog_FormatTable * dlg = reinterpret_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->applyChanges();
}

static void s_close_window(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = reinterpret_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_Close();
}

static void s_line_left(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = reinterpret_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_left, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_right(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = static_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_right, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_top(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = static_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_top, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_bottom(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = static_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatTable::toggle_bottom, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_border_thickness(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatTable * dlg = static_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_BorderThicknessChanged();
}

static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_FormatTable * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}

static gboolean s_apply_to_changed(GtkWidget *widget, gpointer data)
{
	AP_UnixDialog_FormatTable * dlg = reinterpret_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_ApplyToChanged();
	return FALSE;
}


static gboolean s_select_image(GtkWidget *widget, gpointer data)
{
	AP_UnixDialog_FormatTable * dlg = reinterpret_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->askForGraphicPathName();
	return FALSE;
}

static gboolean s_remove_image(GtkWidget *widget, gpointer data)
{
	AP_UnixDialog_FormatTable * dlg = reinterpret_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->clearImage();
	return FALSE;
}

/*!
* Intercept clicks on the color button and show an own GtkColorSelectionDialog
* with palette enabled.
*/
static gboolean 
AP_UnixDialog_FormatTable__onBorderColorClicked (GtkWidget 		*button,
												 GdkEventButton *event,
												 gpointer 		data)
{
	// only handle left clicks
	if (event->button != 1) {
		return FALSE;
	}

	AP_UnixDialog_FormatTable *dlg = static_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_val_if_fail (button && dlg, FALSE);

	GtkWidget *colordlg = gtk_color_selection_dialog_new  ("");
	gtk_window_set_transient_for (GTK_WINDOW (colordlg), GTK_WINDOW (dlg->getWindow ()));
	GtkColorSelection *colorsel = GTK_COLOR_SELECTION ((GTK_COLOR_SELECTION_DIALOG (colordlg))->colorsel);
	gtk_color_selection_set_has_palette (colorsel, TRUE);
	
	gint result = gtk_dialog_run (GTK_DIALOG (colordlg));
	if (result == GTK_RESPONSE_OK) {
		
		// update button
		GtkColorButton *colorbtn = GTK_COLOR_BUTTON (button);
		GdkColor color;
		gtk_color_selection_get_current_color (colorsel, &color);
		gtk_color_button_set_color (colorbtn, &color);

		// update dialog
		UT_RGBColor* rgb = UT_UnixGdkColorToRGBColor (color);
		dlg->setBorderColor (*rgb);
		DELETEP (rgb);
		dlg->event_previewExposed ();
	}
		
	// do not propagate further
	gtk_widget_destroy (colordlg);
	return TRUE;
}

/*!
* Intercept clicks on the color button and show an own GtkColorSelectionDialog
* with palette enabled.
*/
static gboolean 
AP_UnixDialog_FormatTable__onBackgroundColorClicked (GtkWidget 		*button,
													 GdkEventButton *event,
													 gpointer 		data)
{
	// only handle left clicks
	if (event->button != 1) {
		return FALSE;
	}

	AP_UnixDialog_FormatTable *dlg = static_cast<AP_UnixDialog_FormatTable *>(data);
	UT_return_val_if_fail (button && dlg, FALSE);

	GtkWidget *colordlg = gtk_color_selection_dialog_new  ("");
	gtk_window_set_transient_for (GTK_WINDOW (colordlg), GTK_WINDOW (dlg->getWindow ()));
	GtkColorSelection *colorsel = GTK_COLOR_SELECTION ((GTK_COLOR_SELECTION_DIALOG (colordlg))->colorsel);
	gtk_color_selection_set_has_palette (colorsel, TRUE);
	
	gint result = gtk_dialog_run (GTK_DIALOG (colordlg));
	if (result == GTK_RESPONSE_OK) {
		
		// update button
		GtkColorButton *colorbtn = GTK_COLOR_BUTTON (button);
		GdkColor color;
		gtk_color_selection_get_current_color (colorsel, &color);
		gtk_color_button_set_color (colorbtn, &color);

		// update dialog
		UT_RGBColor* rgb = UT_UnixGdkColorToRGBColor (color);
		dlg->setBackgroundColor (*rgb);
		DELETEP (rgb);
		dlg->event_previewExposed ();
	}
		
	// do not propagate further
	gtk_widget_destroy (colordlg);
	return TRUE;
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
	m_wApplyToMenu = NULL;	
	m_wSelectImageButton = NULL;
	m_wNoImageButton = NULL;
	m_wBorderThickness = NULL;
	m_iBorderThicknessConnect = 0;
//
// These are hardwired into the GUI.
//
	const char * sThickness[FORMAT_TABLE_NUMTHICKNESS] ={"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};
	UT_sint32 i = 0;
	for(i=0; i< FORMAT_TABLE_NUMTHICKNESS ;i++)
	{
		m_dThickness[i] = UT_convertToInches(sThickness[i]);
	}
}

AP_UnixDialog_FormatTable::~AP_UnixDialog_FormatTable(void)
{
	DELETEP (m_pPreviewWidget);
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
	UT_return_if_fail(m_wPreviewArea && m_wPreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	GR_UnixAllocInfo ai(m_wPreviewArea->window);
	m_pPreviewWidget =
	    (GR_UnixPangoGraphics*) XAP_App::getApp()->newGraphics(ai);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wPreviewArea->style);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
						 static_cast<UT_uint32>(m_wPreviewArea->allocation.width),
						 static_cast<UT_uint32>(m_wPreviewArea->allocation.height));	
	
	m_pFormatTablePreview->draw();
	
	startUpdater();
}

void AP_UnixDialog_FormatTable::setSensitivity(bool bSens)
{
	gtk_widget_set_sensitive(m_wBorderColorButton, bSens);
	gtk_widget_set_sensitive(m_wBorderThickness, bSens);
	gtk_widget_set_sensitive(m_wBackgroundColorButton, bSens);	
	gtk_widget_set_sensitive(m_wSelectImageButton, bSens);
	gtk_widget_set_sensitive(m_wNoImageButton, bSens);
	gtk_widget_set_sensitive(m_wLineLeft, bSens);
	gtk_widget_set_sensitive(m_wLineRight, bSens);
	gtk_widget_set_sensitive(m_wLineTop, bSens);
	gtk_widget_set_sensitive(m_wLineBottom, bSens);
	gtk_widget_set_sensitive(m_wApplyToMenu, bSens);
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

void AP_UnixDialog_FormatTable::setBorderThicknessInGUI(UT_UTF8String & sThick)
{
	double thickness = UT_convertToInches(sThick.utf8_str());
	guint i =0;
	guint closest = 0;
	double dClose = 100000000.;
	for(i=0; i<FORMAT_TABLE_NUMTHICKNESS; i++)
	{
		double diff = thickness - m_dThickness[i];
		if(diff < 0)
			diff = -diff;
		if(diff < dClose)
		{
			closest = i;
			dClose = diff;
		}
	}
	g_signal_handler_block(G_OBJECT(m_wBorderThickness),m_iBorderThicknessConnect);
	gtk_option_menu_set_history (GTK_OPTION_MENU(m_wBorderThickness),
							  closest);
	g_signal_handler_unblock(G_OBJECT(m_wBorderThickness),m_iBorderThicknessConnect);
}

void AP_UnixDialog_FormatTable::setBackgroundColorInGUI(UT_RGBColor clr)
{
	GdkColor* color = UT_UnixRGBColorToGdkColor(clr);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (m_wBackgroundColorButton), color);
	gdk_color_free(color);
}

void AP_UnixDialog_FormatTable::event_BorderThicknessChanged(void)
{
	if(m_wBorderThickness)
	{
		gint history = gtk_option_menu_get_history(GTK_OPTION_MENU(m_wBorderThickness));
		double thickness = m_dThickness[history];

		UT_UTF8String sThickness;
		{
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			sThickness = UT_UTF8String_sprintf("%fin",thickness);
		}

		setBorderThickness(sThickness);
		event_previewExposed();
	}
}

void AP_UnixDialog_FormatTable::event_ApplyToChanged(void)
{
	if (m_wApplyToMenu)
	{
		gint history = gtk_option_menu_get_history(GTK_OPTION_MENU(m_wApplyToMenu));
		switch (history)
		{
			case 0:
				setApplyFormatTo(FORMAT_TABLE_SELECTION);
				break;
			case 1:
				setApplyFormatTo(FORMAT_TABLE_ROW);
				break;
			case 2:
				setApplyFormatTo(FORMAT_TABLE_COLUMN);
				break;
			case 3:
				setApplyFormatTo(FORMAT_TABLE_TABLE);
				break;
			default:
				// should not happen
				break;
		}
	}
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
	if (!xml)
		return NULL;
	
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
	localizeLabel(glade_xml_get_widget(xml, "lbBorderColor"), pSS, AP_STRING_ID_DLG_FormatTable_Color);
	localizeLabel(glade_xml_get_widget(xml, "lbBorderThickness"), pSS, AP_STRING_ID_DLG_FormatTable_Thickness);
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbBackground"), pSS, AP_STRING_ID_DLG_FormatTable_Background);
	localizeLabel(glade_xml_get_widget(xml, "lbBackgroundColor"), pSS, AP_STRING_ID_DLG_FormatTable_Color);

	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbSetImageBackground"), pSS, AP_STRING_ID_DLG_FormatTable_SetImageBackground);
	

//	add the buttons for background image to the dialog.

	m_wSelectImageButton = glade_xml_get_widget(xml, "btSelectImage");
	m_wNoImageButton = glade_xml_get_widget(xml, "btNoImageBackground");
	
	localizeLabel(glade_xml_get_widget(xml, "lbSelectImage"), pSS, AP_STRING_ID_DLG_FormatTable_SelectImage);
	
	localizeLabel(glade_xml_get_widget(xml, "lbSetNoImage"), pSS, AP_STRING_ID_DLG_FormatTable_NoImageBackground);
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbPreview"), pSS, AP_STRING_ID_DLG_FormatTable_Preview);

	localizeLabel(glade_xml_get_widget(xml, "lbApplyTo"), pSS, AP_STRING_ID_DLG_FormatTable_Apply_To);

	m_wBorderColorButton = glade_xml_get_widget(xml, "cbtBorderColorButton");
	m_wBackgroundColorButton = glade_xml_get_widget(xml, "cbtBackgroundColorButton");

//
// Now the Border Thickness Option menu
// 
	m_wBorderThickness = glade_xml_get_widget(xml, "omBorderThickness");

	// add the options to the "Apply to" menu
	// NOTE: if you change this order, make sure to adjust event_ApplyToChanged as well!
	// FIXME: PLEASE ADD A "localizeMenuItem" HELPER FUNCTION OR SOMETHING LIKE THAT
	m_wApplyToMenu = glade_xml_get_widget(xml, "omApplyTo");
	GtkWidget * menu = gtk_menu_new();
	
	gchar * unixstr = NULL;	// used for conversions
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Selection,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label(unixstr));
	FREEP(unixstr);	

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Row,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label(unixstr));
	FREEP(unixstr);	

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Column,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label(unixstr));
	FREEP(unixstr);	

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Table,s);
	UT_XML_cloneNoAmpersands(unixstr, s.utf8_str());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label(unixstr));
	FREEP(unixstr);	

	gtk_option_menu_set_menu(GTK_OPTION_MENU(m_wApplyToMenu), menu);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_wApplyToMenu), 0);
	gtk_widget_show_all(menu);


	// add the apply and ok buttons to the dialog
	m_wCloseButton = glade_xml_get_widget(xml, "btClose");
	m_wApplyButton = glade_xml_get_widget(xml, "btApply");
	
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
							reinterpret_cast<gpointer>(this));
	g_signal_connect(GTK_OBJECT(m_windowMain),
							"delete_event",
							GTK_SIGNAL_FUNC(s_delete_clicked),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wApplyButton),
							"clicked",
							G_CALLBACK(s_apply_changes),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wSelectImageButton),
							"clicked",
							G_CALLBACK(s_select_image),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wNoImageButton),
							"clicked",
							G_CALLBACK(s_remove_image),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wCloseButton),
							"clicked",
							G_CALLBACK(s_close_window),
							reinterpret_cast<gpointer>(this));
	
	g_signal_connect(G_OBJECT(m_wLineLeft),
							"clicked",
							G_CALLBACK(s_line_left),
							reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wLineRight),
							"clicked",
							G_CALLBACK(s_line_right),
							reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wLineTop),
							"clicked",
							G_CALLBACK(s_line_top),
							reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wLineBottom),
							"clicked",
							G_CALLBACK(s_line_bottom),
							reinterpret_cast<gpointer>(this));		   
						   
	g_signal_connect(G_OBJECT(m_wBorderColorButton),
							"button-release-event",
							G_CALLBACK(AP_UnixDialog_FormatTable__onBorderColorClicked),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wBackgroundColorButton),
							"button-release-event",
							G_CALLBACK(AP_UnixDialog_FormatTable__onBackgroundColorClicked),
							reinterpret_cast<gpointer>(this));
						   
	g_signal_connect(G_OBJECT(m_wPreviewArea),
							"expose_event",
							G_CALLBACK(s_preview_exposed),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wApplyToMenu),
							"changed",
							G_CALLBACK(s_apply_to_changed),
							reinterpret_cast<gpointer>(this));

	m_iBorderThicknessConnect = g_signal_connect(G_OBJECT(m_wBorderThickness),
							"changed",
							G_CALLBACK(s_border_thickness),
							reinterpret_cast<gpointer>(this));
}

void AP_UnixDialog_FormatTable::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_UnixDialog_FormatTable::_storeWindowData(void)
{
}
