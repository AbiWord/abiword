/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
 * Copyright (C) 2002 Martin Sevior
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Image.h"
#include "xap_UnixDlg_Image.h"

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, XAP_UnixDialog_Image * dlg)
{
  UT_ASSERT(widget && dlg);
  dlg->event_Ok();
}

static void s_cancel_clicked(GtkWidget * widget, XAP_UnixDialog_Image * dlg)
{
  UT_ASSERT(widget && dlg);
  dlg->event_Cancel();
}

static void s_HeightSpin_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->doHeightSpin();
}


static void s_WidthSpin_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->doWidthSpin();
}


static void s_HeightEntry_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->doHeightEntry();
}


static void s_WidthEntry_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->doWidthEntry();
}

static void s_aspect_clicked(GtkWidget * widget, XAP_UnixDialog_Image * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->aspectCheckbox();
}

void XAP_UnixDialog_Image::event_Ok ()
{
  setAnswer(XAP_Dialog_Image::a_OK);
  gtk_main_quit ();
}

void XAP_UnixDialog_Image::event_Cancel ()
{  
  setAnswer(XAP_Dialog_Image::a_Cancel);
  gtk_main_quit ();
}


void XAP_UnixDialog_Image::aspectCheckbox()
{
	if(GTK_TOGGLE_BUTTON( m_wAspectCheck)->active && (m_dHeightWidth > 0.0001))
	{
		m_bAspect = true;
	}
	else
	{
		m_bAspect = false;
	}
	setPreserveAspect( m_bAspect );
}

void XAP_UnixDialog_Image::doHeightSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wHeightSpin));
	if(val < m_iHeight)
	{
		bIncrement = false;
	}
	m_iHeight = val;
	incrementHeight(bIncrement);
	adjustWidthForAspect();
	gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() );
}


void XAP_UnixDialog_Image::doWidthSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wWidthSpin));
	if(val < m_iWidth)
	{
		bIncrement = false;
	}
	m_iWidth = val;
	incrementWidth(bIncrement);
	adjustHeightForAspect();
	gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
}


void XAP_UnixDialog_Image::doHeightEntry(void)
{
	const char * szHeight = gtk_entry_get_text(GTK_ENTRY(m_wHeightEntry));
	if(UT_determineDimension(szHeight,DIM_none) != DIM_none)
	{
		setHeight(szHeight);

		g_signal_handler_block(G_OBJECT(m_wHeightEntry), m_iHeightID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wHeightEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() );
		gtk_entry_set_position(GTK_ENTRY(m_wHeightEntry), pos);
		g_signal_handler_unblock(G_OBJECT(m_wHeightEntry), m_iHeightID);
	}
	adjustWidthForAspect();
}


void XAP_UnixDialog_Image::setHeightEntry(void)
{
	g_signal_handler_block(G_OBJECT(m_wHeightEntry), m_iHeightID);
	int pos = gtk_editable_get_position(GTK_EDITABLE(m_wHeightEntry));
	gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() );
	gtk_entry_set_position(GTK_ENTRY(m_wHeightEntry), pos);
	g_signal_handler_unblock(G_OBJECT(m_wHeightEntry), m_iHeightID);
}

void XAP_UnixDialog_Image::setWidthEntry(void)
{
	g_signal_handler_block(G_OBJECT(m_wWidthEntry), m_iWidthID);
	int pos = gtk_editable_get_position(GTK_EDITABLE(m_wWidthEntry));
	gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
	gtk_entry_set_position(GTK_ENTRY(m_wWidthEntry), pos);
	g_signal_handler_unblock(G_OBJECT(m_wWidthEntry), m_iWidthID);
}


void XAP_UnixDialog_Image::doWidthEntry(void)
{
	const char * szWidth = gtk_entry_get_text(GTK_ENTRY(m_wWidthEntry));
	if(UT_determineDimension(szWidth,DIM_none) != DIM_none)
	{
		setWidth(szWidth);

		g_signal_handler_block(G_OBJECT(m_wWidthEntry), m_iWidthID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wWidthEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
		gtk_entry_set_position(GTK_ENTRY(m_wWidthEntry), pos);
		g_signal_handler_unblock(G_OBJECT(m_wWidthEntry), m_iWidthID);
	}
	adjustHeightForAspect();
}


void XAP_UnixDialog_Image::adjustHeightForAspect(void)
{
	if(m_bAspect)
	{
//		double width = UT_convertToInches(getWidthString());
//		double height = width*m_dHeightWidth;
//		UT_Dimension dim = UT_determineDimension(getHeightString(),DIM_none);
//		const char * szHeight = UT_convertInchesToDimensionString(dim,height);
//		setHeight(szHeight);
		setHeightEntry();
	}
}

void XAP_UnixDialog_Image::adjustWidthForAspect(void)
{
	if(m_bAspect)
	{
//		double height = UT_convertToInches(getHeightString());
//		double width = height/m_dHeightWidth;
//		UT_Dimension dim = UT_determineDimension(getWidthString(),DIM_none);
//		const char * szWidth = UT_convertInchesToDimensionString(dim,width);
//		setWidth(szWidth);
		setWidthEntry();
	}
}

/***********************************************************************/

XAP_Dialog * XAP_UnixDialog_Image::static_constructor(XAP_DialogFactory * pFactory,
						      XAP_Dialog_Id id)
{
  XAP_UnixDialog_Image * p = new XAP_UnixDialog_Image(pFactory,id);
  return p;
}

XAP_UnixDialog_Image::XAP_UnixDialog_Image(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_Image(pDlgFactory,id)
{
}

XAP_UnixDialog_Image::~XAP_UnixDialog_Image(void)
{
}

void XAP_UnixDialog_Image::runModal(XAP_Frame * pFrame)
{
  UT_ASSERT(pFrame);
  
  // build the dialog
  GtkWidget * cf = _constructWindow();
  UT_ASSERT(cf);
  connectFocus(GTK_WIDGET(cf),pFrame);
  
  // get top level window and its GtkWidget *
  XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
  UT_ASSERT(frame);
  GtkWidget * parent = frame->getTopLevelWindow();
  UT_ASSERT(parent);
  
  // center it
  centerDialog(parent, cf);
  
  // Run the dialog
  gtk_widget_show (cf);
  setHeightEntry();
  setWidthEntry();
  double height = UT_convertToInches(getHeightString());
  double width = UT_convertToInches(getWidthString());
  if((height > 0.0001) && (width > 0.0001))
  {
	  m_dHeightWidth = height/width;
  }
  else
  {
	  m_dHeightWidth = 0.0;
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wAspectCheck), FALSE);
  }	  
  gtk_main();

  if (cf && GTK_IS_WIDGET(cf))
    gtk_widget_destroy (cf);
}

void XAP_UnixDialog_Image::_connectSignals (void)
{
  // the control buttons


	g_signal_connect(G_OBJECT(m_wHeightSpin),
					   "changed",
					  G_CALLBACK(s_HeightSpin_changed),
					   (gpointer) this);

	m_iHeightID = g_signal_connect(G_OBJECT(m_wHeightEntry),
					   "changed",
					  G_CALLBACK(s_HeightEntry_changed),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wWidthSpin),
					   "changed",
					  G_CALLBACK(s_WidthSpin_changed),
					   (gpointer) this);

	m_iWidthID = g_signal_connect(G_OBJECT(m_wWidthEntry),
					   "changed",
					  G_CALLBACK(s_WidthEntry_changed),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_wAspectCheck),
					   "clicked",
					   G_CALLBACK(s_aspect_clicked),
					   (gpointer) this);

	g_signal_connect(G_OBJECT(m_buttonOK),
					   "clicked",
					   G_CALLBACK(s_ok_clicked),
					   (gpointer) this);
	
	g_signal_connect(G_OBJECT(m_buttonCancel),
					   "clicked",
					   G_CALLBACK(s_cancel_clicked),
					   (gpointer) this);
	
	g_signal_connect_after(G_OBJECT(mMainWindow),
							 "destroy",
							 NULL,
							 NULL);
}

void XAP_UnixDialog_Image::_constructWindowContents (GtkWidget * dialog_vbox1)
{
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  table1 = gtk_table_new (2, 2, TRUE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);

  label2 = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_Image_Height));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);


  label1 = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_Image_Width));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

//
// Fake string spin button for height
//
  GtkWidget * hboxSpinHeight = gtk_hbox_new (FALSE, 0);
  gtk_widget_show(hboxSpinHeight);

  GObject * HeightSpinAdj = gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
  GtkWidget * HeightEntry = gtk_entry_new();
  gtk_widget_show (HeightEntry);
  gtk_box_pack_start (GTK_BOX (hboxSpinHeight), HeightEntry, TRUE, TRUE, 0);
		
  GtkWidget * HeightSpin_dum = gtk_spin_button_new( GTK_ADJUSTMENT(HeightSpinAdj), 1.0,0);
  gtk_widget_show(HeightSpin_dum);
  gtk_widget_set_usize(HeightSpin_dum,10,-2);  
  gtk_box_pack_start(GTK_BOX(hboxSpinHeight),HeightSpin_dum,FALSE,FALSE,0);


  gtk_table_attach (GTK_TABLE (table1), hboxSpinHeight, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

//
// Fake string spin button for width
//
  GtkWidget * hboxSpinWidth = gtk_hbox_new (FALSE, 0);
  gtk_widget_show(hboxSpinWidth);

  GObject * WidthSpinAdj = gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
  GtkWidget * WidthEntry = gtk_entry_new();
  gtk_widget_show (WidthEntry);
  gtk_box_pack_start (GTK_BOX (hboxSpinWidth), WidthEntry, TRUE, TRUE, 0);
		
  GtkWidget * WidthSpin_dum = gtk_spin_button_new( GTK_ADJUSTMENT(WidthSpinAdj), 1.0,0);
  gtk_widget_show(WidthSpin_dum);
  gtk_widget_set_usize(WidthSpin_dum,10,-2);  
  gtk_box_pack_start(GTK_BOX(hboxSpinWidth),WidthSpin_dum,FALSE,FALSE,0);


  gtk_table_attach (GTK_TABLE (table1), hboxSpinWidth, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

//
// Preserve aspect ratio checkbox.
//
  m_wAspectCheck = gtk_check_button_new_with_label(pSS->getValue(XAP_STRING_ID_DLG_Image_Aspect));
  gtk_widget_show(m_wAspectCheck);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), m_wAspectCheck, TRUE, TRUE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wAspectCheck), getPreserveAspect());
  m_bAspect = getPreserveAspect();

  m_wHeightSpin = HeightSpin_dum;
  m_wHeightEntry = HeightEntry;
  m_wWidthSpin = WidthSpin_dum;
  m_wWidthEntry = WidthEntry;
  m_oHeightSpin_adj = HeightSpinAdj;
  m_oWidthSpin_adj = WidthSpinAdj;
  m_iWidth = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wWidthSpin));
  m_iHeight = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wHeightSpin));

}

GtkWidget * XAP_UnixDialog_Image::_constructWindow ()
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbuttonbox1;
  GtkWidget *ok_btn;
  GtkWidget *cancel_btn;
  GtkWidget *dialog_action_area1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), pSS->getValue(XAP_STRING_ID_DLG_Image_Title));
  gtk_window_set_policy (GTK_WINDOW (dialog1), TRUE, TRUE, FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  _constructWindowContents ( dialog_vbox1 );

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), hbuttonbox1, TRUE, TRUE, 0);

  ok_btn = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
  gtk_widget_show (ok_btn);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), ok_btn);
  GTK_WIDGET_SET_FLAGS (ok_btn, GTK_CAN_DEFAULT);

  cancel_btn = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
  gtk_widget_show (cancel_btn);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel_btn);
  GTK_WIDGET_SET_FLAGS (cancel_btn, GTK_CAN_DEFAULT);

  m_buttonOK = ok_btn;
  m_buttonCancel = cancel_btn;
  mMainWindow = dialog1;
  _connectSignals ();

  return dialog1;
}





