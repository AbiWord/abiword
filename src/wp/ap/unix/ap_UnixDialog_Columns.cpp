/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
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
#include <math.h>

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
#include "ap_Dialog_Columns.h"
#include "ap_UnixDialog_Columns.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Columns::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_UnixDialog_Columns * p = new AP_UnixDialog_Columns(pFactory,id);
	return p;
}

AP_UnixDialog_Columns::AP_UnixDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_wlineBetween = NULL;
	m_wtoggleOne = NULL;
	m_wtoggleTwo = NULL;
	m_wtoggleThree = NULL;
	m_wSpin = NULL;
	m_spinHandlerID = 0;
	m_windowMain = NULL;
	m_wpreviewArea = NULL;
	m_wGnomeButtons = NULL;
	m_pPreviewWidget = NULL;
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
	DELETEP (m_pPreviewWidget);
}

/*****************************************************************/

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

static void s_cancel_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Cancel();
}

static gboolean s_preview_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}


static gboolean s_window_exposed(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_previewExposed();
	return FALSE;
}

static void s_one_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(1);
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

    g_signal_handler_block(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
	gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
	g_signal_handler_unblock(G_OBJECT(m_wSpaceAfterEntry),m_iSpaceAfterID);

    g_signal_handler_block(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
	gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
	g_signal_handler_unblock(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);

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
	DELETEP (m_pPreviewWidget);

	abiDestroyWidget(mainWindow);
}

void AP_UnixDialog_Columns::checkLineBetween(void)
{
  if (GTK_TOGGLE_BUTTON (m_wlineBetween)->active)
    {
      setLineBetween(true);
    }
  else
    {
      setLineBetween(false);
    }
}

void AP_UnixDialog_Columns::doHeightSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wMaxColumnHeightSpin));
	UT_DEBUGMSG(("SEVIOR: spin Height %d old value = %d \n",val,m_iMaxColumnHeight));
	if(val < m_iMaxColumnHeight)
	{
		bIncrement = false;
	}
	m_iMaxColumnHeight = val;
	incrementMaxHeight(bIncrement);
//  g_signal_handler_block(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
	gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
//  g_signal_handler_unblock(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
}

void  AP_UnixDialog_Columns::doSpaceAfterSpin(void)
{
	UT_DEBUGMSG(("SEVIOR: In do Space After Spin \n"));
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wSpaceAfterSpin));
	if(val < m_iSpaceAfter)
    {
		bIncrement = false;
    }
	m_iSpaceAfter = val;
	incrementSpaceAfter(bIncrement);
//  g_signal_handler_block(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
	gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
//  g_signal_handler_unblock(G_OBJECT(m_wSpaceAfterEntry),m_iSpaceAfterID);
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
	g_signal_handler_block(G_OBJECT(m_wtoggleOne),
							 m_oneHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleTwo),
							 m_twoHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleThree),
							 m_threeHandlerID);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleOne),
							   m_oneHandlerID);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleTwo),
							   m_twoHandlerID);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleThree),
							   m_threeHandlerID);
	setColumns( val );
	m_pColumnsPreview->draw();
}

void AP_UnixDialog_Columns::event_Toggle( UT_uint32 icolumns)
{
	checkLineBetween();
	g_signal_handler_block(G_OBJECT(m_wtoggleOne),
							 m_oneHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleTwo),
							 m_twoHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleThree),
							 m_threeHandlerID);

		// DOM: TODO: rewrite me
	g_signal_handler_block(G_OBJECT(m_wSpin),
							 m_spinHandlerID);
	gtk_spin_button_set_value( GTK_SPIN_BUTTON(m_wSpin), (gfloat) icolumns);
	g_signal_handler_unblock(G_OBJECT(m_wSpin),
							   m_spinHandlerID);

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
	setColumns( icolumns );
	m_pColumnsPreview->draw();
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

		g_signal_handler_block(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wMaxColumnHeightEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
		gtk_entry_set_position(GTK_ENTRY(m_wMaxColumnHeightEntry), pos);
		g_signal_handler_unblock(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
	}
}

void AP_UnixDialog_Columns::doSpaceAfterEntry(void)
{
	const char * szAfter = gtk_entry_get_text(GTK_ENTRY(m_wSpaceAfterEntry));
	if(UT_determineDimension(szAfter,DIM_none) != DIM_none)
	{
		setSpaceAfter(szAfter);

		g_signal_handler_block(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wSpaceAfterEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
		gtk_entry_set_position(GTK_ENTRY(m_wSpaceAfterEntry), pos);
		g_signal_handler_unblock(G_OBJECT(m_wSpaceAfterEntry),m_iSpaceAfterID);
	}
}

void AP_UnixDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
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

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	//	XML_Char * unixstr = NULL;	// used for conversions

	windowColumns = abiDialogNew ( true, pSS->getValue(AP_STRING_ID_DLG_Column_ColumnTitle) ) ;

	_constructWindowContents(GTK_DIALOG(windowColumns)->vbox);

	abiAddStockButton ( GTK_DIALOG(windowColumns), GTK_STOCK_OK, BUTTON_OK ) ;
	abiAddStockButton ( GTK_DIALOG(windowColumns), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;

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
	GtkWidget *hboxSpin;
	GtkWidget *hseparator;
	GtkAdjustment *SpinAdj;
	GtkWidget *Spinbutton;
	GtkWidget *SpinLabel;
	GtkWidget *hbox2;
	GtkWidget *wLineBtween;

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
        label_button_with_abi_pixmap(wToggleOne, "tb_1column_xpm");
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
	double width = getPageWidth();
	double height = getPageHeight();
	gint rat = 0;
	if( height > 0.000001)
	{
		if(height > width)
		{
			rat = static_cast<gint>( 100.0 *height/width);
			gtk_widget_set_usize (wPreviewFrame, 100, rat); // was -2
		}
		else
		{
			rat = static_cast<gint>( 200.* height/width);
			gtk_widget_set_usize(wPreviewFrame, 200, rat);
		}
	}
	else
	{
		gtk_widget_set_usize (wPreviewFrame, 100, -2); // was -2
	}

	wDrawFrame = gtk_frame_new (NULL);
	gtk_widget_show(wDrawFrame );
	gtk_container_add (GTK_CONTAINER (wPreviewFrame), wDrawFrame);
	gtk_container_set_border_width (GTK_CONTAINER (wDrawFrame), 4);
	gtk_frame_set_shadow_type (GTK_FRAME (wDrawFrame), GTK_SHADOW_OUT);

	wPreviewArea = createDrawingArea ();
	gtk_widget_ref (wPreviewArea);
	g_object_set_data_full (G_OBJECT (windowColumns), "wPreviewArea", wPreviewArea,
								  (GtkDestroyNotify) gtk_widget_unref);

       	gtk_widget_show(wPreviewArea);
	gtk_container_add (GTK_CONTAINER (wDrawFrame), wPreviewArea);


//////////////////////////////////////////////////////
// Line Between
/////////////////////////////////////////////////////

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox2 );
	gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 0);

	wLineBtween = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Column_Line_Between));
	gtk_widget_show(wLineBtween );
	gtk_box_pack_start (GTK_BOX (hbox2), wLineBtween, FALSE, FALSE, 3);

	GtkWidget *hbox6 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hbox6 );
	gtk_box_pack_start (GTK_BOX (vbox1), hbox6, FALSE, FALSE, 0);
	GtkWidget * checkOrder = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Column_RtlOrder));
	gtk_widget_show (checkOrder);
	gtk_box_pack_start (GTK_BOX (hbox6), checkOrder, FALSE, FALSE, 3);
	gtk_toggle_button_set_active (										\
				GTK_TOGGLE_BUTTON(checkOrder), getColumnOrder() );
	m_checkOrder = checkOrder;

/////////////////////////////////////////////////////////
// Spin Button for Columns
/////////////////////////////////////////////////////////

	hseparator =  gtk_hseparator_new ();
	gtk_widget_show(hseparator);
	gtk_box_pack_start(GTK_BOX (vbox1), hseparator, FALSE, TRUE, 0);
	hboxSpin = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hboxSpin );
	gtk_box_pack_start (GTK_BOX (vbox1), hboxSpin, FALSE, FALSE, 0);

	SpinLabel =  gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Column_Number_Cols));
	gtk_widget_show(SpinLabel);
	gtk_box_pack_start(GTK_BOX(hboxSpin),SpinLabel,FALSE,FALSE,0);

	SpinAdj = (GtkAdjustment *) gtk_adjustment_new( 1.0, 1.0, 20., 1.0,10.0,0.0);
	Spinbutton = gtk_spin_button_new( SpinAdj, 1.0,0);
	gtk_widget_show(Spinbutton);
	gtk_box_pack_start(GTK_BOX(hboxSpin),Spinbutton,FALSE,FALSE,0);


/////////////////////////////////////////////////////////
// Spin Button for Space After
/////////////////////////////////////////////////////////

	GtkWidget * hboxSpinAfter = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hboxSpinAfter );
	gtk_box_pack_start (GTK_BOX (vbox1), hboxSpinAfter, FALSE, FALSE, 0);


	GtkWidget * SpinLabelAfter =  gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Column_Space_After));
	gtk_widget_show(SpinLabelAfter);
	gtk_box_pack_start(GTK_BOX(hboxSpinAfter),SpinLabelAfter,FALSE,FALSE,0);

	GtkObject * SpinAfterAdj = gtk_adjustment_new( 1, -1000, 1000, 1, 1, 10);
	GtkWidget * SpinAfter = gtk_entry_new();
	gtk_widget_show (SpinAfter);
	gtk_box_pack_start (GTK_BOX (hboxSpinAfter), SpinAfter, TRUE, TRUE, 0);

	GtkWidget * SpinAfter_dum = gtk_spin_button_new( GTK_ADJUSTMENT(SpinAfterAdj), 1.0,0);
	gtk_widget_show(SpinAfter_dum);
	gtk_widget_set_usize(SpinAfter_dum,10,-2);
	gtk_box_pack_start(GTK_BOX(hboxSpinAfter),SpinAfter_dum,FALSE,FALSE,0);


/////////////////////////////////////////////////////////
// Spin Button for Column Height
/////////////////////////////////////////////////////////

	GtkWidget * hboxSpinSize = gtk_hbox_new (FALSE, 0);
	gtk_widget_show(hboxSpinSize );
	gtk_box_pack_start (GTK_BOX (vbox1), hboxSpinSize, FALSE, FALSE, 0);

	GtkWidget * SpinLabelColumnSize =  gtk_label_new ( pSS->getValue(AP_STRING_ID_DLG_Column_Size));
	gtk_widget_show(SpinLabelColumnSize);
	gtk_box_pack_start(GTK_BOX(hboxSpinSize),SpinLabelColumnSize,FALSE,FALSE,0);

	GtkObject * SpinSizeAdj = gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
	GtkWidget * SpinSize = gtk_entry_new();
	gtk_widget_show (SpinSize);
	gtk_box_pack_start (GTK_BOX (hboxSpinSize), SpinSize, TRUE, TRUE, 0);

	GtkWidget * SpinSize_dum = gtk_spin_button_new( GTK_ADJUSTMENT(SpinSizeAdj), 1.0,0);
	gtk_widget_show(SpinSize_dum);
	gtk_widget_set_usize(SpinSize_dum,10,-2);
	gtk_box_pack_start(GTK_BOX(hboxSpinSize),SpinSize_dum,FALSE,FALSE,0);


////////////////////////////////////////////////////////////////////////
// Gnome buttons
////////////////////////////////////////////////////////////////////////
	vbuttonbox1 = gtk_vbutton_box_new ();
	gtk_widget_show(vbuttonbox1 );
	gtk_box_pack_end (GTK_BOX (hbox1), vbuttonbox1, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1), GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbuttonbox1), 0);
//	gtk_button_box_set_child_size (GTK_BUTTON_BOX (vbuttonbox1), 74, 27);
	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (vbuttonbox1), 0, 1);

	m_wGnomeButtons = vbuttonbox1;
	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_wlineBetween = wLineBtween;
	m_wtoggleOne = wToggleOne;
	m_wtoggleTwo = wToggleTwo;
	m_wtoggleThree = wToggleThree;
	m_wSpin = Spinbutton;
	m_windowMain = windowColumns;
	m_wpreviewArea = wPreviewArea;
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
	m_oneHandlerID = g_signal_connect(G_OBJECT(m_wtoggleOne),
					   "clicked",
					   G_CALLBACK(s_one_clicked),
					   (gpointer) this);

	m_twoHandlerID = g_signal_connect(G_OBJECT(m_wtoggleTwo),
					   "clicked",
					   G_CALLBACK(s_two_clicked),
					   (gpointer) this);

	m_threeHandlerID = g_signal_connect(G_OBJECT(m_wtoggleThree),
					   "clicked",
					   G_CALLBACK(s_three_clicked),
					   (gpointer) this);


	m_spinHandlerID = g_signal_connect(G_OBJECT(m_wSpin),
					   "changed",
					   G_CALLBACK(s_spin_changed),
					   (gpointer) this);


	g_signal_connect(G_OBJECT(m_wSpaceAfterSpin),
					   "changed",
					  G_CALLBACK(s_SpaceAfterSpin_changed),
					   (gpointer) this);


	g_signal_connect(G_OBJECT(m_wMaxColumnHeightSpin),
					   "changed",
					  G_CALLBACK(s_HeightSpin_changed),
					   (gpointer) this);

	m_iSpaceAfterID = g_signal_connect(G_OBJECT(m_wSpaceAfterEntry),
					   "changed",
					  G_CALLBACK(s_SpaceAfterEntry_changed),
					   (gpointer) this);


	m_iMaxColumnHeightID = g_signal_connect(G_OBJECT(m_wMaxColumnHeightEntry),
					   "changed",
					  G_CALLBACK(s_MaxHeightEntry_changed),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wlineBetween),
					   "clicked",
					   G_CALLBACK(s_line_clicked),
					   (gpointer) this);

	// the expose event of the preview
	             g_signal_connect(G_OBJECT(m_wpreviewArea),
					   "expose_event",
					   G_CALLBACK(s_preview_exposed),
					   (gpointer) this);


		     g_signal_connect_after(G_OBJECT(m_windowMain),
		     					 "expose_event",
		     				 G_CALLBACK(s_window_exposed),
		    					 (gpointer) this);
}

void AP_UnixDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}

void AP_UnixDialog_Columns::_storeWindowData(void)
{
}

void AP_UnixDialog_Columns::enableLineBetweenControl(bool bState)
{
}





