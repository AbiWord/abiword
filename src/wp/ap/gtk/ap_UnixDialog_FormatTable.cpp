/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2009 Hubert Figuiere
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

#include <stdlib.h>
#include <gdk/gdk.h>
#include "ut_locale.h"

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_unixColor.h"

#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_ColorChooser.h"
#include "xap_GtkComboBoxHelpers.h"
#include "xap_GtkSignalBlocker.h"

#include "gr_UnixCairoGraphics.h"

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

static gboolean s_preview_draw(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_FormatTable * dlg)
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


	std::auto_ptr<UT_RGBColor> color =
		XAP_UnixDlg_RunColorChooser(GTK_WINDOW (dlg->getWindow ()),
					    GTK_COLOR_BUTTON(button));
	if (color.get()) {
		dlg->setBorderColor (*color);
		dlg->event_previewExposed ();
	}

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

	std::auto_ptr<UT_RGBColor> color =
		XAP_UnixDlg_RunColorChooser(GTK_WINDOW (dlg->getWindow ()),
					    GTK_COLOR_BUTTON(button));
	if (color.get()) {
		dlg->setBackgroundColor (*color);
		dlg->event_previewExposed ();
	}

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
	UT_return_if_fail(m_wPreviewArea && gtk_widget_get_window(m_wPreviewArea));

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	GR_UnixCairoAllocInfo ai(m_wPreviewArea);
	m_pPreviewWidget =
	    (GR_UnixCairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wPreviewArea);

	// let the widget materialize

	GtkAllocation allocation;
	gtk_widget_get_allocation(m_wPreviewArea, &allocation);
	_createPreviewFromGC(m_pPreviewWidget,
						 static_cast<UT_uint32>(allocation.width),
						 static_cast<UT_uint32>(allocation.height));	
	
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
	guint closest = _findClosestThickness(sThick.utf8_str());
	XAP_GtkSignalBlocker b(G_OBJECT(m_wBorderThickness),m_iBorderThicknessConnect);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_wBorderThickness), closest);
}

void AP_UnixDialog_FormatTable::setBackgroundColorInGUI(UT_RGBColor clr)
{
	GdkRGBA* color = UT_UnixRGBColorToGdkRGBA(clr);
	XAP_gtk_color_button_set_rgba (GTK_COLOR_BUTTON (m_wBackgroundColorButton), color);
	gdk_rgba_free(color);
}

void AP_UnixDialog_FormatTable::event_BorderThicknessChanged(void)
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

void AP_UnixDialog_FormatTable::event_ApplyToChanged(void)
{
	if (m_wApplyToMenu)
	{
		gint history = gtk_combo_box_get_active(GTK_COMBO_BOX(m_wApplyToMenu));
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
	gdk_window_raise (gtk_widget_get_window(m_windowMain));
}

void AP_UnixDialog_FormatTable::notifyActiveFrame(XAP_Frame */*pFrame*/)
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
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_FormatTable.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_FormatTable"));
	m_wLineTop = GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderTop"));
	m_wLineLeft = GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderLeft"));
	m_wLineRight = GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderRight"));
	m_wLineBottom = GTK_WIDGET(gtk_builder_get_object(builder, "tbBorderBottom"));
	
	// place some nice pixmaps on our border toggle buttons
	label_button_with_abi_pixmap(m_wLineTop, "tb_LineTop_xpm");
	label_button_with_abi_pixmap(m_wLineLeft, "tb_LineLeft_xpm");
	label_button_with_abi_pixmap(m_wLineRight, "tb_LineRight_xpm");
	label_button_with_abi_pixmap(m_wLineBottom, "tb_LineBottom_xpm");
	
	// set button states
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wLineTop), getTopToggled());  
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wLineLeft), getLeftToggled());  
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wLineRight), getRightToggled());  
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wLineBottom), getBottomToggled());  
	
	m_wPreviewArea = GTK_WIDGET(gtk_builder_get_object(builder, "daPreview"));
	
	// set the dialog title
	ConstructWindowName();
	abiDialogSetTitle(window, "%s", m_WindowName);
	
	// disable double buffering on our preview
	gtk_widget_set_double_buffered(m_wPreviewArea, FALSE); 	
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorder")), pSS, AP_STRING_ID_DLG_FormatTable_Borders);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorderColor")), pSS, AP_STRING_ID_DLG_FormatTable_Color);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorderThickness")), pSS, AP_STRING_ID_DLG_FormatTable_Thickness);
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbBackground")), pSS, AP_STRING_ID_DLG_FormatTable_Background);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBackgroundColor")), pSS, AP_STRING_ID_DLG_FormatTable_Color);

	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbSetImageBackground")), pSS, AP_STRING_ID_DLG_FormatTable_SetImageBackground);
	

//	add the buttons for background image to the dialog.

	m_wSelectImageButton = GTK_WIDGET(gtk_builder_get_object(builder, "btSelectImage"));
	m_wNoImageButton = GTK_WIDGET(gtk_builder_get_object(builder, "btNoImageBackground"));
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbSelectImage")), pSS, AP_STRING_ID_DLG_FormatTable_SelectImage);
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbSetNoImage")), pSS, AP_STRING_ID_DLG_FormatTable_NoImageBackground);
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPreview")), pSS, AP_STRING_ID_DLG_FormatTable_Preview);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbApplyTo")), pSS, AP_STRING_ID_DLG_FormatTable_Apply_To);

	m_wBorderColorButton = GTK_WIDGET(gtk_builder_get_object(builder, "cbtBorderColorButton"));
	m_wBackgroundColorButton = GTK_WIDGET(gtk_builder_get_object(builder, "cbtBackgroundColorButton"));

//
// Now the Border Thickness Option menu
// 
	m_wBorderThickness = GTK_WIDGET(gtk_builder_get_object(builder, "omBorderThickness"));
	GtkComboBox* combo = GTK_COMBO_BOX(m_wBorderThickness);
	XAP_makeGtkComboBoxText(GTK_COMBO_BOX(combo), G_TYPE_NONE);
	XAP_appendComboBoxText(combo, "1/2 pt");
	XAP_appendComboBoxText(combo, "3/4 pt");
	XAP_appendComboBoxText(combo, "1 pt");
	XAP_appendComboBoxText(combo, "1 1/2 pt");
	XAP_appendComboBoxText(combo, "2 1/4 pt");
	XAP_appendComboBoxText(combo, "3 pt");
	XAP_appendComboBoxText(combo, "4 1/2 pt");
	XAP_appendComboBoxText(combo, "6 pt");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

	// add the options to the "Apply to" menu
	// NOTE: if you change this order, make sure to adjust event_ApplyToChanged as well!
	// FIXME: PLEASE ADD A "localizeMenuItem" HELPER FUNCTION OR SOMETHING LIKE THAT
	m_wApplyToMenu = GTK_WIDGET(gtk_builder_get_object(builder, "omApplyTo"));
	combo = GTK_COMBO_BOX(m_wApplyToMenu);
	XAP_makeGtkComboBoxText(GTK_COMBO_BOX(combo), G_TYPE_NONE);
	
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Selection,s);
	XAP_appendComboBoxText(combo, s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Row,s);
	XAP_appendComboBoxText(combo, s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Column,s);
	XAP_appendComboBoxText(combo, s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTable_Apply_To_Table,s);
	XAP_appendComboBoxText(combo, s.c_str());

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

	// add the apply and ok buttons to the dialog
	m_wCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "btClose"));
	m_wApplyButton = GTK_WIDGET(gtk_builder_get_object(builder, "btApply"));
	
	g_object_unref(G_OBJECT(builder));

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
			     gpointer * /*dlg*/)
{
	abiDestroyWidget(widget);
}

void AP_UnixDialog_FormatTable::_connectSignals(void)
{
	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
							"destroy",
							G_CALLBACK(s_destroy_clicked),
							reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_windowMain),
							"delete_event",
							G_CALLBACK(s_delete_clicked),
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
#if GTK_CHECK_VERSION(3,0,0)
			 "draw",
#else
			 "expose_event",
#endif
			 G_CALLBACK(s_preview_draw),
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
