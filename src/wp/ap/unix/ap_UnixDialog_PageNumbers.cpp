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

#include "ut_types.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_UnixDialog_PageNumbers.h"

// static event callbacks
static void s_ok_clicked (GtkWidget * w, AP_UnixDialog_PageNumbers *dlg)
{
  UT_ASSERT(dlg);
  dlg->event_OK();
}

static void s_cancel_clicked (GtkWidget * w, AP_UnixDialog_PageNumbers *dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Cancel();
}

static void s_delete_clicked(GtkWidget * w,
			     gpointer data,
			     AP_UnixDialog_PageNumbers * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_WindowDelete();
}

static gint s_preview_exposed(GtkWidget * w,
			      GdkEventExpose * e,
			      AP_UnixDialog_PageNumbers * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_PreviewExposed();
	return FALSE;
}

static void s_position_changed (GtkWidget * w, AP_UnixDialog_PageNumbers *dlg)
{
  int pos = GPOINTER_TO_INT (gtk_object_get_user_data(GTK_OBJECT (w)));
  dlg->event_HdrFtrChanged((AP_Dialog_PageNumbers::tControl)pos);
}

static void s_alignment_changed (GtkWidget * w, AP_UnixDialog_PageNumbers *dlg)
{
  int align = GPOINTER_TO_INT (gtk_object_get_user_data(GTK_OBJECT (w)));
  dlg->event_AlignChanged ((AP_Dialog_PageNumbers::tAlign)align);
}

XAP_Dialog * AP_UnixDialog_PageNumbers::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_UnixDialog_PageNumbers * p = new AP_UnixDialog_PageNumbers(pFactory,id);
    return p;
}

AP_UnixDialog_PageNumbers::AP_UnixDialog_PageNumbers(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_Dialog_PageNumbers(pDlgFactory,id)
{
  m_recentControl = m_control;
  m_recentAlign   = m_align;
  m_unixGraphics  = NULL;
}

AP_UnixDialog_PageNumbers::~AP_UnixDialog_PageNumbers(void)
{
  DELETEP (m_unixGraphics);
}

void AP_UnixDialog_PageNumbers::event_OK(void)
{
	m_answer = AP_Dialog_PageNumbers::a_OK;

	// set the align and control data
	m_align   = m_recentAlign;
	m_control = m_recentControl;

	gtk_main_quit();
}

void AP_UnixDialog_PageNumbers::event_Cancel(void)
{
	m_answer = AP_Dialog_PageNumbers::a_CANCEL;
	gtk_main_quit();
}

void AP_UnixDialog_PageNumbers::event_WindowDelete(void)
{
        event_Cancel();
}

void AP_UnixDialog_PageNumbers::event_PreviewExposed(void)
{
        if(m_preview)
	       m_preview->draw();
}

void AP_UnixDialog_PageNumbers::event_AlignChanged(AP_Dialog_PageNumbers::tAlign   align)
{
  m_recentAlign = align;
  _updatePreview(m_recentAlign, m_recentControl);
}

void AP_UnixDialog_PageNumbers::event_HdrFtrChanged(AP_Dialog_PageNumbers::tControl control)
{
  m_recentControl = control;
  _updatePreview(m_recentAlign, m_recentControl);
}

void AP_UnixDialog_PageNumbers::runModal(XAP_Frame * pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

    connectFocus(GTK_WIDGET(mainWindow), pFrame);

    // save for use with event
    m_pFrame = pFrame;

    // To center the dialog, we need the frame of its parent.
    XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
    UT_ASSERT(pUnixFrame);
    
    // Get the GtkWindow of the parent frame
    GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);

    // Show the top level dialog,
    gtk_widget_show(mainWindow);

    // Make it modal, and stick it up top
    gtk_grab_add(mainWindow);

    // *** this is how we add the gc ***
    {
      // attach a new graphics context to the drawing area
      XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
    
      UT_ASSERT(unixapp);
      UT_ASSERT(m_previewArea && m_previewArea->window);
      DELETEP (m_unixGraphics);
      
      // make a new Unix GC
      m_unixGraphics = new GR_UnixGraphics(m_previewArea->window, 
					   unixapp->getFontManager(), 
					   m_pApp);
    
      // let the widget materialize
      _createPreviewFromGC(m_unixGraphics,
			   (UT_uint32) m_previewArea->allocation.width,
			   (UT_uint32) m_previewArea->allocation.height);
      
      // hack in a quick draw here
      _updatePreview(m_recentAlign, m_recentControl);
      event_PreviewExposed ();
    }

    // properly set the controls
    gtk_list_select_item (GTK_LIST (GTK_COMBO (m_combo1)->list), (int)m_control);
    gtk_list_select_item (GTK_LIST (GTK_COMBO (m_combo2)->list), (int)m_align);

    // Run into the GTK event loop for this window.
    gtk_main();

    DELETEP (m_unixGraphics);

    if(mainWindow && GTK_IS_WIDGET(mainWindow))
      gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_PageNumbers::_connectSignals (void)
{
  	// the control buttons
	gtk_signal_connect(GTK_OBJECT(m_buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);
	
	// the catch-alls
	
	gtk_signal_connect(GTK_OBJECT(m_window),
					   "delete_event",
					   GTK_SIGNAL_FUNC(s_delete_clicked),
					   (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_window),
							 "destroy",
							 NULL,
							 NULL);
}

void AP_UnixDialog_PageNumbers::_constructWindowContents (GtkWidget *box)
{  
  GtkWidget *hbox1;
  GtkWidget *vbox1;
  GtkWidget *label1;
  GtkWidget *combo1;
  GtkWidget *combo_entry1;
  GtkWidget *label2;
  GtkWidget *combo2;
  GtkWidget *combo_entry2;
  GtkWidget *frame1;
  GtkWidget *li;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  gtk_container_set_border_width (GTK_CONTAINER (box), 4);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (box), hbox1);
  gtk_container_border_width (GTK_CONTAINER (hbox1), 4);

  vbox1 = gtk_vbox_new (FALSE, 5);
  gtk_widget_ref (vbox1);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (vbox1), 4);

  // The position combo entry

  label1 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Position));
  gtk_widget_ref (label1);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.04, 0.5);

  combo1 = gtk_combo_new ();
  gtk_widget_ref (combo1);
  gtk_widget_show (combo1);
  gtk_box_pack_start (GTK_BOX (vbox1), combo1, FALSE, FALSE, 0);

  combo_entry1 = GTK_COMBO (combo1)->entry;
  gtk_entry_set_editable (GTK_ENTRY (combo_entry1), FALSE);
  gtk_widget_ref (combo_entry1);
  gtk_widget_show (combo_entry1);

  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Header));
  gtk_widget_show(li);
  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo1)->list), li);
  gtk_object_set_user_data (GTK_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_HDR));
  gtk_signal_connect (GTK_OBJECT (li), "select",
		      GTK_SIGNAL_FUNC (s_position_changed),
		      (gpointer) this);

  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Footer));
  gtk_widget_show(li);
  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo1)->list), li);
  gtk_object_set_user_data (GTK_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_FTR));
  gtk_signal_connect (GTK_OBJECT (li), "select",
		      GTK_SIGNAL_FUNC (s_position_changed),
		      (gpointer) this);

  // The Alignment combo entry

  label2 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Alignment));
  gtk_widget_ref (label2);
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (vbox1), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label2), 0.04, 0.5);

  combo2 = gtk_combo_new ();
  gtk_widget_ref (combo2);
  gtk_widget_show (combo2);
  gtk_box_pack_start (GTK_BOX (vbox1), combo2, FALSE, FALSE, 0);

  combo_entry2 = GTK_COMBO (combo2)->entry;
  gtk_entry_set_editable (GTK_ENTRY (combo_entry2), FALSE);
  gtk_widget_ref (combo_entry2);
  gtk_widget_show (combo_entry2);

  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Right));
  gtk_widget_show(li);
  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo2)->list), li);
  gtk_object_set_user_data (GTK_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_RALIGN));
  gtk_signal_connect (GTK_OBJECT (li), "select",
		      GTK_SIGNAL_FUNC (s_alignment_changed),
		      (gpointer) this);

  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Left));
  gtk_widget_show(li);
  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo2)->list), li);
  gtk_object_set_user_data (GTK_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_LALIGN));
  gtk_signal_connect (GTK_OBJECT (li), "select",
		      GTK_SIGNAL_FUNC (s_alignment_changed),
		      (gpointer) this);

  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Center));
  gtk_widget_show(li);
  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo2)->list), li);
  gtk_object_set_user_data (GTK_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_CALIGN));
  gtk_signal_connect (GTK_OBJECT (li), "select",
		      GTK_SIGNAL_FUNC (s_alignment_changed),
		      (gpointer) this);

  frame1 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Preview));
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);
  gtk_widget_ref (frame1);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (hbox1), frame1, TRUE, TRUE, 0);

  // create the preview area
  m_previewArea = createDrawingArea ();
  gtk_drawing_area_size (GTK_DRAWING_AREA(m_previewArea), 90, 115);
  gtk_widget_show (m_previewArea);
  gtk_container_add (GTK_CONTAINER (frame1), m_previewArea);

  // the expose event off the preview
  gtk_signal_connect(GTK_OBJECT(m_previewArea),
		     "expose_event",
		     GTK_SIGNAL_FUNC(s_preview_exposed),
		     (gpointer) this);

  m_combo1 = combo1;
  m_combo2 = combo2;
}

GtkWidget * AP_UnixDialog_PageNumbers::_constructWindow (void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_window = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (m_window), pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Title));

  _constructWindowContents (GTK_DIALOG(m_window)->vbox);

  m_buttonOK = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_OK));
  gtk_widget_show (m_buttonOK);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_window)->action_area), 
		     m_buttonOK);

  m_buttonCancel = gtk_button_new_with_label (pSS->getValue (XAP_STRING_ID_DLG_Cancel));
  gtk_widget_show (m_buttonCancel);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_window)->action_area), m_buttonCancel);

  _connectSignals ();

  return m_window;
}
