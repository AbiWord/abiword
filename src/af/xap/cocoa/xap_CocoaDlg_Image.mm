/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialogHelper.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Image.h"
#include "xap_CocoaDlg_Image.h"

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, XAP_CocoaDialog_Image * dlg)
{
  UT_ASSERT(widget && dlg);
  dlg->event_Ok();
}

static void s_cancel_clicked(GtkWidget * widget, XAP_CocoaDialog_Image * dlg)
{
  UT_ASSERT(widget && dlg);
  dlg->event_Cancel();
}

void XAP_CocoaDialog_Image::event_Ok ()
{
  // TODO: get width & height
  setWidth (gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(width_spin)));
  setHeight (gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(height_spin)));
  setAnswer(XAP_Dialog_Image::a_OK);
  gtk_main_quit ();
}

void XAP_CocoaDialog_Image::event_Cancel ()
{  
  setAnswer(XAP_Dialog_Image::a_Cancel);
  gtk_main_quit ();
}

/***********************************************************************/

XAP_Dialog * XAP_CocoaDialog_Image::static_constructor(XAP_DialogFactory * pFactory,
						      XAP_Dialog_Id id)
{
  XAP_CocoaDialog_Image * p = new XAP_CocoaDialog_Image(pFactory,id);
  return p;
}

XAP_CocoaDialog_Image::XAP_CocoaDialog_Image(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_Image(pDlgFactory,id)
{
}

XAP_CocoaDialog_Image::~XAP_CocoaDialog_Image(void)
{
}

void XAP_CocoaDialog_Image::runModal(XAP_Frame * pFrame)
{
  UT_ASSERT(pFrame);
  
  // build the dialog
  GtkWidget * cf = _constructWindow();
  GdkWindow * window = NULL;
  UT_ASSERT(cf);
  connectFocus(GTK_WIDGET(cf),pFrame);
  
  // get top level window and its GtkWidget *
  XAP_CocoaFrame * frame = static_cast<XAP_CocoaFrame *>(pFrame);
  UT_ASSERT(frame);
  GtkWidget * parent = frame->getTopLevelWindow();
  UT_ASSERT(parent);
  
  // center it
  centerDialog(parent, cf);
  
  // Run the dialog
  gtk_widget_show (cf);
  gtk_main();

  if (cf && GTK_IS_WIDGET(cf))
    gtk_widget_destroy (cf);
}

void XAP_CocoaDialog_Image::_connectSignals (void)
{
  // the control buttons
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

void XAP_CocoaDialog_Image::_constructWindowContents (GtkWidget * dialog_vbox1)
{
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;
  GObject *height_spin_adj;
  GObject *width_spin_adj;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  table1 = gtk_table_new (2, 2, TRUE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);

  label1 = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_Image_Width));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_Image_Height));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  height_spin_adj = gtk_adjustment_new (getHeight(), 0, getMaxHeight(), 1, 1, 1);
  height_spin = gtk_spin_button_new (GTK_ADJUSTMENT (height_spin_adj), 1, 1);
  gtk_widget_show (height_spin);
  gtk_table_attach (GTK_TABLE (table1), height_spin, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (height_spin), TRUE);

  width_spin_adj = gtk_adjustment_new (getWidth(), 0, getMaxWidth(), 1, 1, 1);
  width_spin = gtk_spin_button_new (GTK_ADJUSTMENT (width_spin_adj), 1, 1);
  gtk_widget_show (width_spin);
  gtk_table_attach (GTK_TABLE (table1), width_spin, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (width_spin), TRUE);
}

GtkWidget * XAP_CocoaDialog_Image::_constructWindow ()
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
