/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2010 Maleesh Prasan
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
#include <gdk/gdk.h>
#include "ut_locale.h"

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_unixColor.h"

#include "xap_UnixDialogHelper.h"
#include "xap_GtkComboBoxHelpers.h"
#include "xap_GtkSignalBlocker.h"

#include "gr_UnixCairoGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Border_Shading.h"
#include "ap_UnixDialog_Border_Shading.h"
#include "ap_UnixDialog_Columns.h"

const char * sBorderStyle[BORDER_SHADING_NUMOFSTYLES] = {
														"0",	//No line
														"1",	//Solid line
														"2",	//Dashed line
														"3"};	//Dotted line

static void s_apply_changes(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = reinterpret_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->applyChanges();
}

static void s_close_window(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = reinterpret_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_Close();
}

static void s_line_left(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = reinterpret_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_Border_Shading::toggle_left, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_right(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_Border_Shading::toggle_right, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_top(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_Border_Shading::toggle_top, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_bottom(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_Border_Shading::toggle_bottom, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Border_Shading * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}

static gboolean s_on_shading_enable_clicked(GtkWidget 		*button,
											gpointer 		data)
{
	AP_UnixDialog_Border_Shading *dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_val_if_fail (button && dlg, FALSE);
	dlg->event_shadingPatternChange();
}

/*!
* Intercept clicks on the color button and show an own GtkColorSelectionDialog
* with palette enabled.
*/
static gboolean s_on_border_color_clicked (GtkWidget 		*button,
								GdkEventButton 	*event,
								gpointer 		data)
{
	// only handle left clicks
	if (event->button != 1) {
		return FALSE;
	}

	AP_UnixDialog_Border_Shading *dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
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

static void s_on_border_thickness_clicked(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_BorderThicknessChanged();
}

static void s_on_border_style_clicked(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_BorderStyleChanged();
}

/*!
* Intercept clicks on the color button and show an own GtkColorSelectionDialog
* with palette enabled.
*/
static gboolean s_on_shading_color_clicked (GtkWidget 		*button,
										GdkEventButton *event,
										gpointer 		data)
{
	// only handle left clicks
	if (event->button != 1) {
		return FALSE;
	}

	AP_UnixDialog_Border_Shading *dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
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
		dlg->setShadingColor (*rgb);
		DELETEP (rgb);
		dlg->event_previewExposed ();
	}
		
	// do not propagate further
	gtk_widget_destroy (colordlg);
	return TRUE;
}


static void s_on_shading_offset_clicked(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Border_Shading * dlg = static_cast<AP_UnixDialog_Border_Shading *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_ShadingOffsetChanged();
}

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Border_Shading::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_UnixDialog_Border_Shading * p = new AP_UnixDialog_Border_Shading(pFactory,id);
	return p;
}

AP_UnixDialog_Border_Shading::AP_UnixDialog_Border_Shading(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_Border_Shading(pDlgFactory,id)
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
	m_wBorderThickness = NULL;
	m_wBorderStyle = NULL;
	m_wShadingOffset = NULL;
	m_wShadingEnable = NULL;
	m_iBorderThicknessConnect = 0;
	m_iBorderStyleConnect = 0;
	m_iShadingOffsetConnect = 0;
	m_iShadingEnableConnect = 0;
	m_iLineLeftConnect = 0;
	m_iLineRightConnect = 0;
	m_iLineTopConnect = 0;
	m_iLineBotConnect = 0;
}

AP_UnixDialog_Border_Shading::~AP_UnixDialog_Border_Shading(void)
{
	DELETEP (m_pPreviewWidget);
}

void AP_UnixDialog_Border_Shading::runModeless(XAP_Frame * pFrame)
{
	UT_DEBUGMSG(("========================= In the unModeless \n"));
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
	GR_UnixCairoAllocInfo ai(m_wPreviewArea);
	m_pPreviewWidget =
	    (GR_UnixCairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wPreviewArea->style);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
						 static_cast<UT_uint32>(m_wPreviewArea->allocation.width),
						 static_cast<UT_uint32>(m_wPreviewArea->allocation.height));	
	
	m_pBorderShadingPreview->draw();

	startUpdater();
	UT_DEBUGMSG(("========================= End the unModeless \n"));
}

void AP_UnixDialog_Border_Shading::setSensitivity(bool bSens)
{
//	UT_DEBUGMSG(("========================= Set the sensitivity \n"));

	XAP_GtkSignalBlocker b1(G_OBJECT(m_wLineLeft), m_iLineLeftConnect);
	gtk_toggle_button_set_active((GtkToggleButton*)m_wLineLeft, getLeftToggled() ? TRUE: FALSE);

	XAP_GtkSignalBlocker b2(G_OBJECT(m_wLineRight), m_iLineRightConnect);
	gtk_toggle_button_set_active((GtkToggleButton*)m_wLineRight, getRightToggled() ? TRUE: FALSE);

	XAP_GtkSignalBlocker b3(G_OBJECT(m_wLineTop), m_iLineTopConnect);
	gtk_toggle_button_set_active((GtkToggleButton*)m_wLineTop, getTopToggled() ? TRUE: FALSE);

	XAP_GtkSignalBlocker b4(G_OBJECT(m_wLineBottom), m_iLineBotConnect);
	gtk_toggle_button_set_active((GtkToggleButton*)m_wLineBottom, getBottomToggled() ? TRUE: FALSE);
}

void AP_UnixDialog_Border_Shading::event_Close(void)
{
	m_answer = AP_Dialog_Border_Shading::a_CLOSE;
	destroy();
}

void AP_UnixDialog_Border_Shading::event_previewExposed(void)
{
	if(m_pBorderShadingPreview)
		m_pBorderShadingPreview->draw();
}

void AP_UnixDialog_Border_Shading::_setShadingEnable(bool enable)
{
	UT_DEBUGMSG(("Maleesh =============== Setting shading enable: \n"));

	gtk_widget_set_sensitive(m_wShadingOffset, enable);
	gtk_widget_set_sensitive(m_wShadingColorButton, enable);

	XAP_GtkSignalBlocker b1(G_OBJECT(m_wShadingEnable), m_iShadingEnableConnect);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wShadingEnable), static_cast<gboolean>(enable));
}

void AP_UnixDialog_Border_Shading::setBorderThicknessInGUI(UT_UTF8String & sThick)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the border thickness in the GUI: %s \n", sThick.utf8_str()));

	guint closest = _findClosestThickness(sThick.utf8_str());
	XAP_GtkSignalBlocker b(G_OBJECT(m_wBorderThickness),m_iBorderThicknessConnect);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_wBorderThickness), closest);
}

void AP_UnixDialog_Border_Shading::setBorderStyleInGUI(UT_UTF8String & sStyle)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the border style in the GUI: %s \n", sStyle.utf8_str()));

	PP_PropertyMap::TypeLineStyle style = PP_PropertyMap::linestyle_type(sStyle.utf8_str());
	guint index = (guint)style - 1;

	if (index >= 0)
	{
		XAP_GtkSignalBlocker b(G_OBJECT(m_wBorderStyle),m_iBorderStyleConnect);
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wBorderStyle), index);
	}
}

void AP_UnixDialog_Border_Shading::setBorderColorInGUI(UT_RGBColor clr)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the border color in the GUI: %d|%d|%d \n", clr.m_red, clr.m_grn, clr.m_blu));

	GdkColor* color = UT_UnixRGBColorToGdkColor(clr);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (m_wBorderColorButton), color);
	gdk_color_free(color);
}

void AP_UnixDialog_Border_Shading::setShadingColorInGUI(UT_RGBColor clr)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the shading color in the GUI: %d|%d|%d \n", clr.m_red, clr.m_grn, clr.m_blu));

	GdkColor* color = UT_UnixRGBColorToGdkColor(clr);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (m_wShadingColorButton), color);
	gdk_color_free(color);
}

void AP_UnixDialog_Border_Shading::setShadingPatternInGUI(UT_UTF8String & sPattern)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the shading pattern in the GUI: %s \n", sPattern.utf8_str()));

	// 8/8/2010 Maleesh - TODO: Change this, when there are more shading patterns.
	bool shading_enabled = !(sPattern == BORDER_SHADING_SHADING_DISABLE);
	_setShadingEnable(shading_enabled);
}

void AP_UnixDialog_Border_Shading::setShadingOffsetInGUI(UT_UTF8String & sOffset)
{
	UT_DEBUGMSG(("Maleesh =============== Setup the shading offset in the GUI: %s \n", sOffset.utf8_str()));

	guint closest = _findClosestOffset(sOffset.utf8_str());
	XAP_GtkSignalBlocker b(G_OBJECT(m_wShadingOffset),m_iShadingOffsetConnect);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_wShadingOffset), closest);
}

void AP_UnixDialog_Border_Shading::event_BorderThicknessChanged(void)
{
	if(m_wBorderThickness)
	{
		gint history = gtk_combo_box_get_active(GTK_COMBO_BOX(m_wBorderThickness));
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

void AP_UnixDialog_Border_Shading::event_BorderStyleChanged(void)
{
	if(m_wBorderStyle)
	{
		gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(m_wBorderStyle));

		if (index >= 0 && index <= BORDER_SHADING_NUMOFSTYLES)
		{
			UT_UTF8String style_utf8 = sBorderStyle[index];
			setBorderStyle(style_utf8);
			event_previewExposed();
		}
	}
}

void AP_UnixDialog_Border_Shading::event_ShadingOffsetChanged(void)
{
	if(m_wShadingOffset)
	{
		gint history = gtk_combo_box_get_active(GTK_COMBO_BOX(m_wShadingOffset));
		double offset = m_dShadingOffset[history];

		UT_UTF8String sOffset;
		{
			UT_LocaleTransactor t(LC_NUMERIC, "C");
			sOffset = UT_UTF8String_sprintf("%fin",offset);
		}

		setShadingOffset(sOffset);
		event_previewExposed();
	}
}

void AP_UnixDialog_Border_Shading::event_shadingPatternChange(void)
{
	UT_DEBUGMSG(("========================= Changed the state of the cb \n"));

	// 8/8/2010 Maleesh - TODO: Change this, when there are more shading patterns.
	gboolean bEnable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wShadingEnable));
	UT_UTF8String pattern_utf8 = bEnable ? BORDER_SHADING_SHADING_ENABLE : BORDER_SHADING_SHADING_DISABLE;
	setShadingPattern(pattern_utf8);
	_setShadingEnable(bEnable);
}

void AP_UnixDialog_Border_Shading::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_Border_Shading::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_Border_Shading::notifyActiveFrame(XAP_Frame */*pFrame*/)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Border_Shading::_constructWindow(void)
{
	UT_DEBUGMSG(("========================= Constructing the window \n"));

	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our UI file is located
//	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/ap_UnixDialog_Border_Shading.xml";

	// Test: Avoid to use the xml files from installed path.
	std::string ui_path = /*static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + */ "src/wp/ap/gtk/ap_UnixDialog_Border_Shading.xml";
	
	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, ui_path.c_str(), NULL);
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Border_Shading"));
	m_wLineTop 		= GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderTop"));
	m_wLineLeft 	= GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderLeft"));
	m_wLineRight 	= GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderRight"));
	m_wLineBottom 	= GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderBottom"));
	
	// the toggle buttons created by GtkBuilder already contain a label, remove that, so we can add a pixmap as a child
	gtk_container_remove(GTK_CONTAINER(m_wLineTop), gtk_bin_get_child(GTK_BIN(m_wLineTop)));
	gtk_container_remove(GTK_CONTAINER(m_wLineLeft), gtk_bin_get_child(GTK_BIN(m_wLineLeft)));
	gtk_container_remove(GTK_CONTAINER(m_wLineRight), gtk_bin_get_child(GTK_BIN(m_wLineRight)));
	gtk_container_remove(GTK_CONTAINER(m_wLineBottom), gtk_bin_get_child(GTK_BIN(m_wLineBottom)));
	
	// place some nice pixmaps on our border toggle buttons
	label_button_with_abi_pixmap(m_wLineTop, "tb_LineTop_xpm");
	label_button_with_abi_pixmap(m_wLineLeft, "tb_LineLeft_xpm");
	label_button_with_abi_pixmap(m_wLineRight, "tb_LineRight_xpm");
	label_button_with_abi_pixmap(m_wLineBottom, "tb_LineBottom_xpm");
	
	m_wPreviewArea 		= GTK_WIDGET(gtk_builder_get_object(builder, "daPreview"));
	m_wShadingEnable	= GTK_WIDGET(gtk_builder_get_object(builder, "cbShadingEnable"));
	
	// set the dialog title
	ConstructWindowName();
	abiDialogSetTitle(window, m_WindowName);
	
	// disable double buffering on our preview
	gtk_widget_set_double_buffered(m_wPreviewArea, FALSE); 	
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorder")), pSS, AP_STRING_ID_DLG_BorderShading_Borders);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorderColor")), pSS, AP_STRING_ID_DLG_BorderShading_Border_Color);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorderThickness")), pSS, AP_STRING_ID_DLG_BorderShading_Thickness);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lblBorderStyle")), pSS, AP_STRING_ID_DLG_BorderShading_Border_Style);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbShading")), pSS, AP_STRING_ID_DLG_BorderShading_Shading);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbShadingColor")), pSS, AP_STRING_ID_DLG_BorderShading_Shading_Color);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lblShadingOffset")), pSS, AP_STRING_ID_DLG_BorderShading_Offset);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPreview")), pSS, AP_STRING_ID_DLG_FormatTable_Preview);

	m_wBorderColorButton = GTK_WIDGET(gtk_builder_get_object(builder, "cbtBorderColorButton"));
	m_wShadingColorButton = GTK_WIDGET(gtk_builder_get_object(builder, "cbtShadingColorButton"));

//
// Border Thickness Option menu
// 
	m_wBorderThickness = GTK_WIDGET(gtk_builder_get_object(builder, "omBorderThickness"));
	GtkComboBox *combo = GTK_COMBO_BOX(m_wBorderThickness);
	XAP_makeGtkComboBoxText(combo, G_TYPE_NONE);
	gtk_combo_box_append_text(combo, "1/2 pt");
	gtk_combo_box_append_text(combo, "3/4 pt");
	gtk_combo_box_append_text(combo, "1 pt");
	gtk_combo_box_append_text(combo, "1 1/2 pt");
	gtk_combo_box_append_text(combo, "2 1/4 pt");
	gtk_combo_box_append_text(combo, "3 pt");
	gtk_combo_box_append_text(combo, "4 1/2 pt");
	gtk_combo_box_append_text(combo, "6 pt");
	gtk_combo_box_set_active(combo, 0);

//
// Border Style Option menu
//
	m_wBorderStyle = GTK_WIDGET(gtk_builder_get_object(builder, "cmbBorderStyle"));
	GtkComboBox *combo_style = GTK_COMBO_BOX(m_wBorderStyle);
	XAP_makeGtkComboBoxText(combo_style, G_TYPE_NONE);
	gtk_combo_box_append_text(combo_style, "None");
	gtk_combo_box_append_text(combo_style, "Solid line");
	gtk_combo_box_append_text(combo_style, "Dashed line");
	gtk_combo_box_append_text(combo_style, "Dotted line");
	gtk_combo_box_set_active(combo, 0);

//
// Shading offset Option menu
//
	m_wShadingOffset = GTK_WIDGET(gtk_builder_get_object(builder, "cmbShadingOffset"));
	GtkComboBox *combo_offset = GTK_COMBO_BOX(m_wShadingOffset);
	XAP_makeGtkComboBoxText(combo_offset, G_TYPE_NONE);
	gtk_combo_box_append_text(combo_offset, "1/2 pt");
	gtk_combo_box_append_text(combo_offset, "3/4 pt");
	gtk_combo_box_append_text(combo_offset, "1 pt");
	gtk_combo_box_append_text(combo_offset, "1 1/2 pt");
	gtk_combo_box_append_text(combo_offset, "2 1/4 pt");
	gtk_combo_box_append_text(combo_offset, "3 pt");
	gtk_combo_box_append_text(combo_offset, "4 1/2 pt");
	gtk_combo_box_append_text(combo_offset, "6 pt");
	gtk_combo_box_set_active(combo_offset, 0);

	// add the apply and ok buttons to the dialog
	m_wCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "btClose"));
	m_wApplyButton = GTK_WIDGET(gtk_builder_get_object(builder, "btApply"));

	g_object_unref(G_OBJECT(builder));

	return window;
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_Border_Shading * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer,
			     gpointer * /*dlg*/)
{
	abiDestroyWidget(widget);
}

void AP_UnixDialog_Border_Shading::_connectSignals(void)
{
	g_signal_connect(GTK_OBJECT(m_windowMain),
							"destroy",
							G_CALLBACK(s_destroy_clicked),
							reinterpret_cast<gpointer>(this));
	g_signal_connect(GTK_OBJECT(m_windowMain),
							"delete_event",
							G_CALLBACK(s_delete_clicked),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wApplyButton),
							"clicked",
							G_CALLBACK(s_apply_changes),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wCloseButton),
							"clicked",
							G_CALLBACK(s_close_window),
							reinterpret_cast<gpointer>(this));
	
	m_iLineLeftConnect = g_signal_connect(G_OBJECT(m_wLineLeft),
										"clicked",
										G_CALLBACK(s_line_left),
										reinterpret_cast<gpointer>(this));

	m_iLineRightConnect = g_signal_connect(G_OBJECT(m_wLineRight),
										"clicked",
										G_CALLBACK(s_line_right),
										reinterpret_cast<gpointer>(this));

	m_iLineTopConnect = g_signal_connect(G_OBJECT(m_wLineTop),
										"clicked",
										G_CALLBACK(s_line_top),
										reinterpret_cast<gpointer>(this));

	m_iLineBotConnect = g_signal_connect(G_OBJECT(m_wLineBottom),
										"clicked",
										G_CALLBACK(s_line_bottom),
										reinterpret_cast<gpointer>(this));
						   
	g_signal_connect(G_OBJECT(m_wBorderColorButton),
							"button-release-event",
							G_CALLBACK(s_on_border_color_clicked),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wShadingColorButton),
							"button-release-event",
							G_CALLBACK(s_on_shading_color_clicked),
							reinterpret_cast<gpointer>(this));
						   
	g_signal_connect(G_OBJECT(m_wPreviewArea),
							"expose_event",
							G_CALLBACK(s_preview_exposed),
							reinterpret_cast<gpointer>(this));

	m_iShadingOffsetConnect = g_signal_connect(G_OBJECT(m_wShadingOffset),
											"changed",
											G_CALLBACK(s_on_shading_offset_clicked),
											reinterpret_cast<gpointer>(this));

	m_iBorderThicknessConnect = g_signal_connect(G_OBJECT(m_wBorderThickness),
												"changed",
												G_CALLBACK(s_on_border_thickness_clicked),
												reinterpret_cast<gpointer>(this));

	m_iBorderStyleConnect = g_signal_connect(G_OBJECT(m_wBorderStyle),
											"changed",
											G_CALLBACK(s_on_border_style_clicked),
											reinterpret_cast<gpointer>(this));

	m_iShadingEnableConnect = g_signal_connect(G_OBJECT(m_wShadingEnable),
											"toggled",
											G_CALLBACK(s_on_shading_enable_clicked),
											reinterpret_cast<gpointer>(this));//"state-changed"
}

void AP_UnixDialog_Border_Shading::_populateWindowData(void)
{
   setAllSensitivities();
}
