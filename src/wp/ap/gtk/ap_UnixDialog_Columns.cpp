/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2009 Hubert Figuiere
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <math.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"
#include "xap_GtkSignalBlocker.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_UnixDialog_Columns.h"
#include "gr_UnixCairoGraphics.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Columns::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Columns(pFactory,id);
}

AP_UnixDialog_Columns::AP_UnixDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_wlineBetween = NULL;
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	m_wtoggleOne = NULL;
	m_wtoggleTwo = NULL;
	m_wpreviewArea = NULL;
	m_pPreviewWidget = NULL;
	m_wtoggleThree = NULL;
#endif
	m_wSpin = NULL;
	m_spinHandlerID = 0;
	m_windowMain = NULL;
	m_iSpaceAfter = 0;
	m_iSpaceAfterID =0;
	m_wSpaceAfterSpin = NULL;
	m_iMaxColumnHeight = 0;
	m_iMaxColumnHeightID = 0;
	m_wMaxColumnHeightSpin = NULL;
    m_checkOrder = NULL;
}

AP_UnixDialog_Columns::~AP_UnixDialog_Columns(void)
{
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	DELETEP (m_pPreviewWidget);
#endif
}

/*****************************************************************/

#if !defined(EMBEDDED_TARGET) || EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
static void s_one_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(1);
}

static void s_two_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(2);
}


static void s_three_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(3);
}
#endif

static void s_spin_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->readSpin();
}


static void s_HeightSpin_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doHeightSpin();
}


static void s_SpaceAfterSpin_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doSpaceAfterSpin();
}


static void s_SpaceAfterEntry_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doSpaceAfterEntry();
}


static void s_MaxHeightEntry_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doMaxHeightEntry();
}


static void s_line_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->checkLineBetween();
}

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}
#endif

static gboolean s_window_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	dlg->event_previewExposed();
#endif
	return FALSE;
}



/*****************************************************************/

void AP_UnixDialog_Columns::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	setViewAndDoc(pFrame);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// ***show*** before creating gc's
	gtk_widget_show ( mainWindow ) ;

	// Populate the window's data items
	_populateWindowData();

    {
		XAP_GtkSignalBlocker b(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
		gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
	}

	{
		XAP_GtkSignalBlocker b(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
		gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
	}
	
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	// *** this is how we add the gc for Column Preview ***
	// attach a new graphics context to the drawing area
	UT_return_if_fail(m_wpreviewArea && m_wpreviewArea->window);

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	GR_UnixCairoAllocInfo ai(m_wpreviewArea->window);
	m_pPreviewWidget =
	    (GR_UnixCairoGraphics*) XAP_App::getApp()->newGraphics(ai);
	
	
	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wpreviewArea->style);

	// let the widget materialize

	_createPreviewFromGC(m_pPreviewWidget,
						 (UT_uint32) m_wpreviewArea->allocation.width,
						 (UT_uint32) m_wpreviewArea->allocation.height);
#endif 	
	
	setLineBetween(getLineBetween());
	if(getLineBetween()==true)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wlineBetween),TRUE);
	}
	// Now draw the columns

	event_Toggle(getColumns());

	// Run into the GTK event loop for this window.

	switch( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}
	
	setColumnOrder (gtk_toggle_button_get_active(
												 GTK_TOGGLE_BUTTON(m_checkOrder)));

	_storeWindowData();
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	DELETEP (m_pPreviewWidget);
#endif

	abiDestroyWidget(mainWindow);
}

void AP_UnixDialog_Columns::checkLineBetween(void)
{
  if (GTK_TOGGLE_BUTTON (m_wlineBetween)->active)
      setLineBetween(true);
  else
      setLineBetween(false);
}

void AP_UnixDialog_Columns::doHeightSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wMaxColumnHeightSpin));
	UT_DEBUGMSG(("SEVIOR: spin Height %d old value = %d \n",val,m_iMaxColumnHeight));
	if (val == m_iMaxColumnHeight)
		return;
	if(val < m_iMaxColumnHeight)
		bIncrement = false;
	m_iMaxColumnHeight = val;
	incrementMaxHeight(bIncrement);
	//g_signal_handler_block(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
	gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
	//g_signal_handler_unblock(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
}

void  AP_UnixDialog_Columns::doSpaceAfterSpin(void)
{
	UT_DEBUGMSG(("SEVIOR: In do Space After Spin \n"));
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wSpaceAfterSpin));
	UT_DEBUGMSG(("MARCM: spin Height %d old value = %d \n",val,m_iSpaceAfter));
	if (val == m_iSpaceAfter)
		return;
	else if(val < m_iSpaceAfter)
		bIncrement = false;
	m_iSpaceAfter = val;
	incrementSpaceAfter(bIncrement);
	//g_signal_handler_block(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
	gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
	//g_signal_handler_unblock(G_OBJECT(m_wSpaceAfterEntry),m_iSpaceAfterID);
}

void AP_UnixDialog_Columns::readSpin(void)
{
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wSpin));
	if(val < 1)
		return;
	if(val < 4)
	{
		event_Toggle(val);
		return;
	}
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	{
		XAP_GtkSignalBlocker b1(G_OBJECT(m_wtoggleOne), m_oneHandlerID);
		XAP_GtkSignalBlocker b2(G_OBJECT(m_wtoggleTwo), m_twoHandlerID);
		XAP_GtkSignalBlocker b3(G_OBJECT(m_wtoggleThree), m_threeHandlerID);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
	}
#endif
	setColumns( val );
	m_pColumnsPreview->draw();
}

void AP_UnixDialog_Columns::event_Toggle( UT_uint32 icolumns)
{
	checkLineBetween();
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	g_signal_handler_block(G_OBJECT(m_wtoggleOne),
							 m_oneHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleTwo),
							 m_twoHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleThree),
							 m_threeHandlerID);
#endif
	{
		// DOM: TODO: rewrite me
		XAP_GtkSignalBlocker b(G_OBJECT(m_wSpin),
						   m_spinHandlerID);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON(m_wSpin), (gfloat) icolumns);
	}
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
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
		break;
		// TODO: make these insenstive and update a spin control

	}
	g_signal_handler_unblock(G_OBJECT(m_wtoggleOne),
							   m_oneHandlerID);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleTwo),
							   m_twoHandlerID);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleThree),
							   m_threeHandlerID);
#endif
	setColumns( icolumns );
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	m_pColumnsPreview->draw();
#endif
}


void AP_UnixDialog_Columns::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Columns::a_OK;
}


void AP_UnixDialog_Columns::doMaxHeightEntry(void)
{
	const char * szHeight = gtk_entry_get_text(GTK_ENTRY(m_wMaxColumnHeightEntry));
	if(UT_determineDimension(szHeight,DIM_none) != DIM_none)
	{
		setMaxHeight(szHeight);

		XAP_GtkSignalBlocker b(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wMaxColumnHeightEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
		gtk_editable_set_position(GTK_EDITABLE(m_wMaxColumnHeightEntry), pos);
	}
}

void AP_UnixDialog_Columns::doSpaceAfterEntry(void)
{
	const char * szAfter = gtk_entry_get_text(GTK_ENTRY(m_wSpaceAfterEntry));
	if(UT_determineDimension(szAfter,DIM_none) != DIM_none)
	{
		setSpaceAfter(szAfter);

		XAP_GtkSignalBlocker b(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wSpaceAfterEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
		gtk_editable_set_position(GTK_EDITABLE(m_wSpaceAfterEntry), pos);
	}
}

void AP_UnixDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
}

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
void AP_UnixDialog_Columns::event_previewExposed(void)
{
        if(m_pColumnsPreview)
	       m_pColumnsPreview->draw();
}
#endif
/*****************************************************************/

GtkWidget * AP_UnixDialog_Columns::_constructWindow(void)
{

	GtkWidget * windowColumns;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	//	gchar * unixstr = NULL;	// used for conversions
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_ColumnTitle,s);
	
	windowColumns = abiDialogNew ( "column dialog", TRUE, s.utf8_str() ) ;

	_constructWindowContents(GTK_DIALOG(windowColumns)->vbox);

	abiAddStockButton ( GTK_DIALOG(windowColumns), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
	abiAddStockButton ( GTK_DIALOG(windowColumns), GTK_STOCK_OK, BUTTON_OK ) ;

	_connectsignals();
	return windowColumns;
}

void AP_UnixDialog_Columns::_constructWindowContents(GtkWidget * windowColumns)
{
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	GtkWidget *wColumnFrame;
	GtkWidget *tableColumns;
	GtkWidget *hboxColumns;
	GtkWidget *wToggleOne;
	GtkWidget *wLabelOne;
	GtkWidget *wToggleTwo;
	GtkWidget *wLabelTwo;
	GtkWidget *wToggleThree;
	GtkWidget *wLabelThree;
	GtkWidget *wPreviewFrame;
	GtkWidget *wDrawFrame;
	GtkWidget *wPreviewArea;
#endif
	GtkWidget *hseparator;
	GtkAdjustment *SpinAdj;
	GtkWidget *Spinbutton;
	GtkWidget *SpinLabel;
	GtkWidget *wLineBtween;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;
	
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	GtkWidget * tableTop = gtk_table_new (1, 2, FALSE);
	gtk_widget_show (tableTop);
	gtk_box_pack_start (GTK_BOX (windowColumns), tableTop, FALSE, FALSE, 6);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Number,s);
	wColumnFrame = gtk_frame_new ( s.utf8_str());
	gtk_frame_set_shadow_type(GTK_FRAME(wColumnFrame), GTK_SHADOW_NONE);
	gtk_widget_show(wColumnFrame);
	gtk_table_attach (GTK_TABLE (tableTop), wColumnFrame, 0, 1, 0, 1,
				  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	hboxColumns = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hboxColumns);
	gtk_container_set_border_width(GTK_CONTAINER (hboxColumns), 6);
	gtk_container_add (GTK_CONTAINER (wColumnFrame), hboxColumns);

	tableColumns = gtk_table_new (3, 2, FALSE);
	gtk_widget_show (tableColumns);
	gtk_box_pack_start (GTK_BOX (hboxColumns), tableColumns, TRUE, FALSE, 0);
	
	wToggleOne = gtk_toggle_button_new();
	gtk_widget_show(wToggleOne );
        label_button_with_abi_pixmap(wToggleOne, "tb_1column_xpm");
	GTK_WIDGET_SET_FLAGS (wToggleOne, GTK_CAN_DEFAULT);
	gtk_table_attach (GTK_TABLE (tableColumns), wToggleOne, 0, 1, 0, 1,
				  (GtkAttachOptions) (0), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_One,s);
	wLabelOne = gtk_label_new ( s.utf8_str());
	gtk_widget_show(wLabelOne );
	gtk_table_attach (GTK_TABLE (tableColumns), wLabelOne, 1, 2, 0, 1,
				  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (wLabelOne), 0, 0.5);

	wToggleTwo = gtk_toggle_button_new ();
	gtk_widget_show(wToggleTwo);
        label_button_with_abi_pixmap(wToggleTwo, "tb_2column_xpm");
	GTK_WIDGET_SET_FLAGS (wToggleTwo, GTK_CAN_DEFAULT);
	gtk_table_attach (GTK_TABLE (tableColumns), wToggleTwo, 0, 1, 1, 2,
				  (GtkAttachOptions) (0), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Two,s);
	wLabelTwo = gtk_label_new( s.utf8_str());
	gtk_widget_show(wLabelTwo );
	gtk_table_attach (GTK_TABLE (tableColumns), wLabelTwo, 1, 2, 1, 2,
				  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);	
	gtk_misc_set_alignment (GTK_MISC (wLabelTwo), 0, 0.5);

	wToggleThree = gtk_toggle_button_new ();
	gtk_widget_show(wToggleThree);
        label_button_with_abi_pixmap(wToggleThree, "tb_3column_xpm");
	GTK_WIDGET_SET_FLAGS (wToggleThree, GTK_CAN_DEFAULT);
	gtk_table_attach (GTK_TABLE (tableColumns), wToggleThree, 0, 1, 2, 3,
				  (GtkAttachOptions) (0), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Three,s);
	wLabelThree = gtk_label_new ( s.utf8_str());
	gtk_widget_show(wLabelThree);
	gtk_table_attach (GTK_TABLE (tableColumns), wLabelThree, 1, 2, 2, 3,
				  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);	
	gtk_misc_set_alignment (GTK_MISC (wLabelThree), 0, 0.5);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Preview,s);
	wPreviewFrame = gtk_frame_new ( s.utf8_str());
	gtk_frame_set_shadow_type(GTK_FRAME(wPreviewFrame), GTK_SHADOW_NONE);
	gtk_widget_show(wPreviewFrame);
	gtk_table_attach (GTK_TABLE (tableTop), wPreviewFrame, 1, 2, 0, 1,
				  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	double width = getPageWidth();
	double height = getPageHeight();
	gint rat = 0;
	if( height > 0.000001)
	{
		if(height > width)
		{
			rat = static_cast<gint>( 100.0 *height/width);
			gtk_widget_set_size_request (wPreviewFrame, 100, -1); // was -2
		}
		else
		{
			rat = static_cast<gint>( 200.* height/width);
			gtk_widget_set_size_request (wPreviewFrame, 200, rat);
		}
	}
	else
	{
		gtk_widget_set_size_request (wPreviewFrame, 100, -1); // was -2
	}

	wDrawFrame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(wDrawFrame), GTK_SHADOW_NONE);
	gtk_widget_show(wDrawFrame );
	gtk_container_add (GTK_CONTAINER (wPreviewFrame), wDrawFrame);
	gtk_container_set_border_width (GTK_CONTAINER (wDrawFrame), 4);

	wPreviewArea = createDrawingArea ();
	g_object_ref (wPreviewArea);
	g_object_set_data_full (G_OBJECT (windowColumns), "wPreviewArea", wPreviewArea,
							(GDestroyNotify) g_object_unref);
	gtk_widget_show(wPreviewArea);
	gtk_container_add (GTK_CONTAINER (wDrawFrame), wPreviewArea);
#endif /* HAVE_HILDON */
//////////////////////////////////////////////////////
// Line Between
/////////////////////////////////////////////////////
	
	GtkWidget * table = gtk_table_new (6, 3, FALSE);
	gtk_widget_show (table);
	gtk_box_pack_start (GTK_BOX (windowColumns), table, FALSE, FALSE, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Line_Between,s);
	wLineBtween = gtk_check_button_new_with_label (s.utf8_str());
	gtk_widget_show(wLineBtween);
	gtk_table_attach (GTK_TABLE (table), wLineBtween, 0, 2, 0, 1,
				  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_RtlOrder,s);
	GtkWidget * checkOrder = gtk_check_button_new_with_label (s.utf8_str());
	gtk_widget_show (checkOrder);
	gtk_table_attach (GTK_TABLE (table), checkOrder, 0, 2, 1, 2,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(checkOrder), getColumnOrder() );
	m_checkOrder = checkOrder;

/////////////////////////////////////////////////////////
// Spin Button for Columns
/////////////////////////////////////////////////////////

	hseparator = gtk_hseparator_new ();
	gtk_widget_show(hseparator);
	gtk_table_attach (GTK_TABLE (table), hseparator, 0, 3, 2, 3,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Number_Cols,s);
	SpinLabel = gtk_label_new ( s.utf8_str());
	gtk_widget_show(SpinLabel);
	gtk_table_attach (GTK_TABLE (table), SpinLabel, 0, 1, 3, 4,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 0);
	gtk_misc_set_alignment (GTK_MISC (SpinLabel), 0, 0.5);

	SpinAdj = (GtkAdjustment *) gtk_adjustment_new( 1.0, 1.0, 20., 1.0,10.0,0.0);
	Spinbutton = gtk_spin_button_new( SpinAdj, 1.0,0);
	gtk_widget_show(Spinbutton);
	gtk_table_attach (GTK_TABLE (table), Spinbutton, 1, 3, 3, 4,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 3);

/////////////////////////////////////////////////////////
// Spin Button for Space After
/////////////////////////////////////////////////////////

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Space_After,s);
	GtkWidget * SpinLabelAfter = gtk_label_new ( s.utf8_str());
	gtk_widget_show(SpinLabelAfter);
	gtk_table_attach (GTK_TABLE (table), SpinLabelAfter, 0, 1, 4, 5,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 3);
	gtk_misc_set_alignment (GTK_MISC (SpinLabelAfter), 0, 0.5);
	
	GtkObject * SpinAfterAdj = gtk_adjustment_new( 1, -1000, 1000, 1, 1, 10);
	GtkWidget * SpinAfter = gtk_entry_new();
	gtk_widget_show (SpinAfter);
	gtk_table_attach (GTK_TABLE (table), SpinAfter, 1, 2, 4, 5,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	
	GtkWidget * SpinAfter_dum = gtk_spin_button_new( GTK_ADJUSTMENT(SpinAfterAdj), 1.0,0);
	gtk_widget_show(SpinAfter_dum);
	gtk_widget_set_size_request(SpinAfter_dum,13,-1);
	gtk_table_attach (GTK_TABLE (table), SpinAfter_dum, 2, 3, 4, 5,
			  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	
/////////////////////////////////////////////////////////
// Spin Button for Column Height
/////////////////////////////////////////////////////////
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Size,s);
	GtkWidget * SpinLabelColumnSize = gtk_label_new ( s.utf8_str());
	gtk_widget_show(SpinLabelColumnSize);
	gtk_table_attach (GTK_TABLE (table), SpinLabelColumnSize, 0, 1, 5, 6,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 6, 7);
	gtk_misc_set_alignment (GTK_MISC (SpinLabelColumnSize), 0, 0.5);
	
	GtkObject * SpinSizeAdj = gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
	GtkWidget * SpinSize = gtk_entry_new();
	gtk_widget_show (SpinSize);
	gtk_table_attach (GTK_TABLE (table), SpinSize, 1, 2, 5, 6,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	GtkWidget * SpinSize_dum = gtk_spin_button_new( GTK_ADJUSTMENT(SpinSizeAdj), 1.0,0);
	gtk_widget_show(SpinSize_dum);
	gtk_widget_set_size_request(SpinSize_dum,13,-1);
	gtk_table_attach (GTK_TABLE (table), SpinSize_dum, 2, 3, 5, 6,
				  (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_wlineBetween = wLineBtween;
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	m_wtoggleOne = wToggleOne;
	m_wtoggleTwo = wToggleTwo;
	m_wtoggleThree = wToggleThree;
	m_wpreviewArea = wPreviewArea;
#endif
	m_wSpin = Spinbutton;
	m_windowMain = windowColumns;
	m_wSpaceAfterSpin = SpinAfter_dum;
	m_wSpaceAfterEntry = SpinAfter;
	m_oSpaceAfter_adj =  SpinAfterAdj;
	m_iSpaceAfter = (UT_sint32) GTK_ADJUSTMENT(SpinAfterAdj)->value;
	m_wMaxColumnHeightSpin = SpinSize_dum;
	m_wMaxColumnHeightEntry = SpinSize;
	m_oSpinSize_adj = SpinSizeAdj;
	m_iSizeHeight = (UT_sint32) GTK_ADJUSTMENT(SpinSizeAdj)->value;
}

void AP_UnixDialog_Columns::_connectsignals(void)
{

	// the control buttons
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	m_oneHandlerID = g_signal_connect(G_OBJECT(m_wtoggleOne),
					   "clicked",
					   G_CALLBACK(s_one_clicked),
					   reinterpret_cast<gpointer>(this));

	m_twoHandlerID = g_signal_connect(G_OBJECT(m_wtoggleTwo),
					   "clicked",
					   G_CALLBACK(s_two_clicked),
					   reinterpret_cast<gpointer>(this));

	m_threeHandlerID = g_signal_connect(G_OBJECT(m_wtoggleThree),
					   "clicked",
					   G_CALLBACK(s_three_clicked),
					   reinterpret_cast<gpointer>(this));


#endif
	m_spinHandlerID = g_signal_connect(G_OBJECT(m_wSpin),
					   "changed",
					   G_CALLBACK(s_spin_changed),
					   reinterpret_cast<gpointer>(this));


	g_signal_connect(G_OBJECT(m_wSpaceAfterSpin),
					   "changed",
					  G_CALLBACK(s_SpaceAfterSpin_changed),
					   reinterpret_cast<gpointer>(this));


	g_signal_connect(G_OBJECT(m_wMaxColumnHeightSpin),
					   "changed",
					  G_CALLBACK(s_HeightSpin_changed),
					   reinterpret_cast<gpointer>(this));

	m_iSpaceAfterID = g_signal_connect(G_OBJECT(m_wSpaceAfterEntry),
					   "changed",
					  G_CALLBACK(s_SpaceAfterEntry_changed),
					   reinterpret_cast<gpointer>(this));


	m_iMaxColumnHeightID = g_signal_connect(G_OBJECT(m_wMaxColumnHeightEntry),
					   "changed",
					  G_CALLBACK(s_MaxHeightEntry_changed),
					   reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wlineBetween),
					   "clicked",
					   G_CALLBACK(s_line_clicked),
					   reinterpret_cast<gpointer>(this));

	// the expose event of the preview
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
#else
	             g_signal_connect(G_OBJECT(m_wpreviewArea),
					   "expose_event",
					   G_CALLBACK(s_preview_exposed),
					   reinterpret_cast<gpointer>(this));
#endif

		     g_signal_connect_after(G_OBJECT(m_windowMain),
		     					 "expose_event",
		     				 G_CALLBACK(s_window_exposed),
		    					 reinterpret_cast<gpointer>(this));
}

void AP_UnixDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}

void AP_UnixDialog_Columns::_storeWindowData(void)
{
}

void AP_UnixDialog_Columns::enableLineBetweenControl(bool /*bState*/)
{
}





