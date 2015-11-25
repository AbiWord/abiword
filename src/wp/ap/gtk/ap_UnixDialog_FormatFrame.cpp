/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2009, 2013 Hubert Figuiere
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

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"
#include "xap_UnixDlg_ColorChooser.h"
#include "xap_GtkSignalBlocker.h"
#include "xap_GtkComboBoxHelpers.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatFrame.h"
#include "ap_UnixDialog_FormatFrame.h"
#include "ap_UnixDialog_Columns.h"
#include "fl_FrameLayout.h"
#include "fl_BlockLayout.h"
#include "gr_UnixCairoGraphics.h"

static void s_apply_changes(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = reinterpret_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_ApplyToChanged();
}

static void s_close_window(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = reinterpret_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_Close();
}

static void s_line_left(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = reinterpret_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatFrame::toggle_left, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_right(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatFrame::toggle_right, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_top(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatFrame::toggle_top, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}

static void s_line_bottom(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->toggleLineType(AP_Dialog_FormatFrame::toggle_bottom, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
	dlg->event_previewExposed();
}



static void s_WrapButton(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->setWrapping(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}

static void s_border_thickness(GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_FormatFrame * dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_if_fail(widget && dlg);
	dlg->event_BorderThicknessChanged();
}

static gboolean s_preview_draw(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_FormatFrame * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}

static gboolean s_select_image(GtkWidget *widget, gpointer data)
{
	AP_UnixDialog_FormatFrame * dlg = reinterpret_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->askForGraphicPathName();
	return FALSE;
}

static gboolean s_remove_image(GtkWidget *widget, gpointer data)
{
	AP_UnixDialog_FormatFrame * dlg = reinterpret_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->clearImage();
	return FALSE;
}

/*!
* Intercept clicks on the color button and show an own GtkColorSelectionDialog
* with palette enabled.
*/
static gboolean 
AP_UnixDialog_FormatFrame__onBorderColorClicked (GtkWidget 		*button,
												 GdkEventButton *event,
												 gpointer 		data)
{
	// only handle left clicks
	if (event->button != 1) {
		return FALSE;
	}

	AP_UnixDialog_FormatFrame *dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
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
AP_UnixDialog_FormatFrame__onBackgroundColorClicked (GtkWidget 		*button,
													 GdkEventButton *event,
													 gpointer 		data)
{
	// only handle left clicks
	if (event->button != 1) {
		return FALSE;
	}

	AP_UnixDialog_FormatFrame *dlg = static_cast<AP_UnixDialog_FormatFrame *>(data);
	UT_return_val_if_fail (button && dlg, FALSE);

	std::auto_ptr<UT_RGBColor> color =
		XAP_UnixDlg_RunColorChooser(GTK_WINDOW (dlg->getWindow ()),
					    GTK_COLOR_BUTTON(button));

	if (color.get()) {
		dlg->setBGColor (*color);
		dlg->event_previewExposed ();
	}

	return TRUE;
}

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_FormatFrame::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_UnixDialog_FormatFrame * p = new AP_UnixDialog_FormatFrame(pFactory,id);
	return p;
}

AP_UnixDialog_FormatFrame::AP_UnixDialog_FormatFrame(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_FormatFrame(pDlgFactory,id)
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
	m_wSetImageButton = NULL;
	m_wSelectImageButton = NULL;
	m_wNoImageButton = NULL;
	m_wBorderThickness = NULL;
	m_iBorderThicknessConnect = 0;
	m_wWrapButton = NULL;
	m_wPosParagraph =  NULL;
	m_wPosColumn = NULL;
	m_wPosPage = NULL;
//
// These are hardwired into the GUI.
//
	const char * sThickness[FORMAT_FRAME_NUMTHICKNESS] ={"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};
	UT_sint32 i = 0;
	for(i=0; i< FORMAT_FRAME_NUMTHICKNESS ;i++)
	{
		m_dThickness[i] = UT_convertToInches(sThickness[i]);
	}

}

AP_UnixDialog_FormatFrame::~AP_UnixDialog_FormatFrame(void)
{
	DELETEP (m_pPreviewWidget);
}

void AP_UnixDialog_FormatFrame::runModeless(XAP_Frame * pFrame)
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
	m_pPreviewWidget = (GR_UnixCairoGraphics*) XAP_App::getApp()->newGraphics(ai);

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
	
	m_pFormatFramePreview->draw();
	
	startUpdater();
}

void AP_UnixDialog_FormatFrame::setSensitivity(bool bSens)
{
	gtk_widget_set_sensitive(m_wBorderColorButton, bSens);
	gtk_widget_set_sensitive(m_wBackgroundColorButton, bSens);	
	gtk_widget_set_sensitive(m_wLineLeft, bSens);
	gtk_widget_set_sensitive(m_wLineRight, bSens);
	gtk_widget_set_sensitive(m_wLineTop, bSens);
	gtk_widget_set_sensitive(m_wLineBottom, bSens);
	gtk_widget_set_sensitive(m_wApplyButton, bSens);
	gtk_widget_set_sensitive(m_wWrapButton, bSens);
}

void AP_UnixDialog_FormatFrame::event_Close(void)
{
	m_answer = AP_Dialog_FormatFrame::a_CLOSE;
	destroy();
}

void AP_UnixDialog_FormatFrame::event_previewExposed(void)
{
	if(m_pFormatFramePreview)
		m_pFormatFramePreview->draw();
}

void AP_UnixDialog_FormatFrame::setBorderThicknessInGUI(UT_UTF8String & sThick)
{
	double thickness = UT_convertToInches(sThick.utf8_str());
	guint i =0;
	guint closest = 0;
	double dClose = 100000000.;
	for(i=0; i<FORMAT_FRAME_NUMTHICKNESS; i++)
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
	XAP_GtkSignalBlocker b(G_OBJECT(m_wBorderThickness),m_iBorderThicknessConnect);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_wBorderThickness), closest);
}

void AP_UnixDialog_FormatFrame::event_BorderThicknessChanged(void)
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

		setBorderThicknessAll(sThickness);
		event_previewExposed();
	}
}

void AP_UnixDialog_FormatFrame::event_ApplyToChanged(void)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wPosParagraph )))
	{
	     setPositionMode(FL_FRAME_POSITIONED_TO_BLOCK);  
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wPosColumn )))
	{
	     setPositionMode(FL_FRAME_POSITIONED_TO_COLUMN);  
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wPosPage )))
	{
	     setPositionMode(FL_FRAME_POSITIONED_TO_PAGE);  
	}
	applyChanges();
}

void AP_UnixDialog_FormatFrame::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_FormatFrame::activate(void)
{
	UT_ASSERT (m_windowMain);
        
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
	gdk_window_raise (gtk_widget_get_window(m_windowMain));
}

void AP_UnixDialog_FormatFrame::notifyActiveFrame(XAP_Frame *_pFrame)
{
    UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setAllSensitivities();
	FV_View * pView = static_cast<FV_View *>(_pFrame->getCurrentView());
	if(pView && pView->isInFrame(pView->getPoint()))
	{
		fl_BlockLayout * pBL = pView->getCurrentBlock();
		fl_FrameLayout * pFrame = static_cast<fl_FrameLayout *>(pBL->myContainingLayout());
		if(pFrame->getContainerType() != FL_CONTAINER_FRAME)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
		if(pFrame->getFrameWrapMode() >= FL_FRAME_WRAPPED_TO_RIGHT)
		{
			setWrapping(true);
		}
		else
		{
			setWrapping(false);
		}
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wWrapButton),getWrapping());
		if(positionMode() == FL_FRAME_POSITIONED_TO_BLOCK)
		{
		     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( m_wPosParagraph),TRUE);
		}
		else if(positionMode() == FL_FRAME_POSITIONED_TO_COLUMN)
		{
		     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wPosColumn),TRUE);
		} 
		else if(positionMode() == FL_FRAME_POSITIONED_TO_PAGE)
		{
		     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wPosPage),TRUE);
		} 
	}
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_FormatFrame::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_FormatFrame.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_FormatFrame"));
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

	// localize the strings in our dialog, and set tags for some widgets
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorder")), pSS, AP_STRING_ID_DLG_FormatFrame_Borders);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorderColor")), pSS, AP_STRING_ID_DLG_FormatFrame_Color);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBorderThickness")), pSS, AP_STRING_ID_DLG_FormatTable_Thickness);
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbBackground")), pSS, AP_STRING_ID_DLG_FormatFrame_Background);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbBackgroundColor")), pSS, AP_STRING_ID_DLG_FormatFrame_Color);

	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbSetImageBackground")), pSS, AP_STRING_ID_DLG_FormatFrame_SetImageBackground);

// Radio buttons to position type of the Frame
		
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPositionTo")), pSS, AP_STRING_ID_DLG_FormatFrame_PositionTo);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbSetToParagraph")), pSS, AP_STRING_ID_DLG_FormatFrame_SetToParagraph);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbSetToColumn")), pSS, AP_STRING_ID_DLG_FormatFrame_SetToColumn);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbSetToPage")), pSS, AP_STRING_ID_DLG_FormatFrame_SetToPage);
	m_wPosParagraph = GTK_WIDGET(gtk_builder_get_object(builder, "rbSetToParagraph"));
	m_wPosColumn = GTK_WIDGET(gtk_builder_get_object(builder, "rbSetToColumn"));
	m_wPosPage = GTK_WIDGET(gtk_builder_get_object(builder, "rbSetToPage"));

//  Button and label for text wrapping

	m_wWrapButton = GTK_WIDGET(gtk_builder_get_object(builder, "btTextWrapState"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wWrapButton),TRUE);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbTextWrapState")), pSS, AP_STRING_ID_DLG_FormatFrame_TextWrapping);
	localizeButtonUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "btTextWrapState")), pSS, AP_STRING_ID_DLG_FormatFrame_SetTextWrapping);

//	add the buttons for background image to the dialog.

	m_wSelectImageButton = GTK_WIDGET(gtk_builder_get_object(builder, "btSelectImage"));
	m_wNoImageButton = GTK_WIDGET(gtk_builder_get_object(builder, "btSetNoImage"));
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbSelectImage")), pSS, AP_STRING_ID_DLG_FormatFrame_SelectImage);
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbSetNoImage")), pSS, AP_STRING_ID_DLG_FormatFrame_NoImageBackground);
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPreview")), pSS, AP_STRING_ID_DLG_FormatFrame_Preview);
	
	m_wBorderColorButton = GTK_WIDGET(gtk_builder_get_object(builder, "cbtBorderColorButton"));
	m_wBackgroundColorButton = GTK_WIDGET(gtk_builder_get_object(builder, "cbtBackgroundColorButton"));

//
// Now the Border Thickness Option menu
// 
	m_wBorderThickness = GTK_WIDGET(gtk_builder_get_object(builder, "omBorderThickness"));
	GtkComboBox* combo = GTK_COMBO_BOX(m_wBorderThickness);
	XAP_makeGtkComboBoxText(combo, G_TYPE_NONE);
	// TODO WTF is this hardcoded. 
	XAP_appendComboBoxText(combo, "1/2 pt");
	XAP_appendComboBoxText(combo, "3/4 pt");
	XAP_appendComboBoxText(combo, "1 pt");
	XAP_appendComboBoxText(combo, "1 1/2 pt");
	XAP_appendComboBoxText(combo, "2 1/4 pt");
	XAP_appendComboBoxText(combo, "3 pt");
	XAP_appendComboBoxText(combo, "4 1/2 pt");
	XAP_appendComboBoxText(combo, "6 pt");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
	
	// add the apply and ok buttons to the dialog
	m_wCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "btClose"));
	m_wApplyButton = GTK_WIDGET(gtk_builder_get_object(builder, "btApply"));

	g_object_unref(G_OBJECT(builder));
	
	return window;
}

static void s_destroy_clicked(GtkWidget * /* widget */,
			      AP_UnixDialog_FormatFrame * dlg)
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

void AP_UnixDialog_FormatFrame::_connectSignals(void)
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


	g_signal_connect(G_OBJECT(m_wWrapButton),
							"clicked",
							G_CALLBACK(s_WrapButton),
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
							G_CALLBACK(AP_UnixDialog_FormatFrame__onBorderColorClicked),
							reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wBackgroundColorButton),
							"button-release-event",
							G_CALLBACK(AP_UnixDialog_FormatFrame__onBackgroundColorClicked),
							reinterpret_cast<gpointer>(this));	   

	m_iBorderThicknessConnect = g_signal_connect(G_OBJECT(m_wBorderThickness),
							"changed",
							G_CALLBACK(s_border_thickness),
							reinterpret_cast<gpointer>(this));
						   
	g_signal_connect(G_OBJECT(m_wPreviewArea),
			 "draw",
			 G_CALLBACK(s_preview_draw),
			 reinterpret_cast<gpointer>(this));
}

void AP_UnixDialog_FormatFrame::_populateWindowData(void)
{
   setAllSensitivities();
}

void AP_UnixDialog_FormatFrame::_storeWindowData(void)
{
}
