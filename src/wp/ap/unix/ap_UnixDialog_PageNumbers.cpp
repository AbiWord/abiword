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
#include "xap_UnixDialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_UnixDialog_PageNumbers.h"

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
}

void AP_UnixDialog_PageNumbers::event_Cancel(void)
{
	m_answer = AP_Dialog_PageNumbers::a_CANCEL;
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
	UT_return_if_fail(pFrame);
	
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_return_if_fail(mainWindow);

	// show everything before creating the preview
	gtk_widget_show_all ( mainWindow ) ;

    // *** this is how we add the gc ***
    {
      // attach a new graphics context to the drawing area
      XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
    
      UT_return_if_fail(unixapp);
      UT_return_if_fail(m_previewArea && m_previewArea->window);
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
//    gtk_list_select_item (GTK_LIST (GTK_COMBO (m_combo1)->list), (int)m_control);
//    gtk_list_select_item (GTK_LIST (GTK_COMBO (m_combo2)->list), (int)m_align);
    m_recentControl = m_control = AP_Dialog_PageNumbers::id_HDR;
    m_recentAlign = m_align = AP_Dialog_PageNumbers::id_RALIGN;

    _updatePreview(m_align, m_control);
    
	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this,
								 BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}

	DELETEP (m_unixGraphics);

	abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_PageNumbers::_constructWindowContents (GtkWidget *box)
{  
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  gtk_container_set_border_width (GTK_CONTAINER (box), 4);

//  hbox1 = gtk_hbox_new (FALSE, 0);
//  gtk_widget_ref (hbox1);
//  gtk_widget_show (hbox1);
//  gtk_container_border_width (GTK_CONTAINER (hbox1), 4);

//  vbox1 = gtk_vbox_new (FALSE, 5);
//  gtk_widget_ref (vbox1);
//  gtk_widget_show (vbox1);
//  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);
//  gtk_container_border_width (GTK_CONTAINER (vbox1), 4);

 
  
  // Create a hbox with "Position -----------"
  GtkWidget* labelPosition = gtk_label_new(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Position));
  GtkWidget* linePosition = gtk_hseparator_new();
  GtkWidget* boxPosition = gtk_hbox_new(false, 4);
  gtk_box_pack_start(GTK_BOX(boxPosition), labelPosition, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(boxPosition), linePosition, 1, 1, 0);
  gtk_widget_show(labelPosition);
  gtk_widget_show(linePosition);
  gtk_widget_show(boxPosition);
  
  // Create a vbox with position options.
  GtkWidget* radioHeader = gtk_radio_button_new_with_label(0, pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Header));
  GtkWidget* radioFooter = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioHeader), pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Footer));
  GtkWidget* boxPositionOptions = gtk_vbox_new(false, 1);
  gtk_container_border_width(GTK_CONTAINER(boxPositionOptions), 5);
  gtk_box_pack_start(GTK_BOX(boxPositionOptions), radioHeader, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(boxPositionOptions), radioFooter, 0, 0, 0);
  gtk_widget_show(radioHeader);
  gtk_widget_show(radioFooter);
  gtk_widget_show(boxPositionOptions);

  // Create a hbox with "Alignment ----------"
  GtkWidget* labelAlignment = gtk_label_new(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Alignment));
  GtkWidget* lineAlignment = gtk_hseparator_new();
  GtkWidget* boxAlignment = gtk_hbox_new(false, 4);
  gtk_box_pack_start(GTK_BOX(boxAlignment), labelAlignment, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(boxAlignment), lineAlignment,  1, 1, 0);
  gtk_widget_show(labelAlignment);
  gtk_widget_show(lineAlignment);
  gtk_widget_show(boxAlignment);

  // Create a vbox with alignment options.
  GtkWidget* radioLeft =   gtk_radio_button_new_with_label(0, pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Left));
  GtkWidget* radioCenter = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioLeft), pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Center));
  GtkWidget* radioRight =  gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioLeft), pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Right));
  GtkWidget* boxAlignmentOptions = gtk_vbox_new(false, 1);
  gtk_container_border_width(GTK_CONTAINER(boxAlignmentOptions), 5);
  gtk_box_pack_start(GTK_BOX(boxAlignmentOptions), radioLeft,   0, 0, 0);
  gtk_box_pack_start(GTK_BOX(boxAlignmentOptions), radioCenter, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(boxAlignmentOptions), radioRight,  0, 0, 0);
  gtk_widget_show(radioLeft);
  gtk_widget_show(radioCenter);
  gtk_widget_show(radioRight);
  gtk_widget_show(boxAlignmentOptions);
  
  // Create the main left-hand vbox
  GtkWidget* vboxLeft = gtk_vbox_new(false, 4);
  gtk_box_pack_start(GTK_BOX(vboxLeft), boxPosition, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vboxLeft), boxPositionOptions, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vboxLeft), boxAlignment, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(vboxLeft), boxAlignmentOptions, 0, 0, 0);
  gtk_widget_set_usize(vboxLeft, 140, 190);
  gtk_widget_show(vboxLeft);
 

  // Create the preview area.
  GtkWidget* framePreview = gtk_frame_new(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Preview));
  gtk_widget_set_usize(framePreview, 146, 190);
  gtk_widget_ref(framePreview);
  gtk_widget_show(framePreview);
//  gtk_frame_set_shadow_type(GTK_FRAME(framePreview), GTK_SHADOW_IN);
  m_previewArea = createDrawingArea();
  gtk_widget_ref(m_previewArea);
  gtk_widget_show(m_previewArea);
  gtk_drawing_area_size(GTK_DRAWING_AREA(m_previewArea), 90, 115);
  gtk_container_add(GTK_CONTAINER(framePreview), m_previewArea);

  // Create the main layout hbox
  GtkWidget* hboxMain = gtk_hbox_new(false, 10);
  gtk_box_pack_start(GTK_BOX(hboxMain), vboxLeft, 1, 1, 0);
  gtk_box_pack_start(GTK_BOX(hboxMain), framePreview, 1, 1, 0);
  gtk_widget_show(hboxMain);
  gtk_container_add(GTK_CONTAINER(box), hboxMain);
  
  // Set user data so that our callbacks know what to do.
  gtk_object_set_user_data(GTK_OBJECT(radioFooter), GINT_TO_POINTER(AP_Dialog_PageNumbers::id_FTR));
  gtk_object_set_user_data(GTK_OBJECT(radioHeader), GINT_TO_POINTER(AP_Dialog_PageNumbers::id_HDR));
  gtk_object_set_user_data(GTK_OBJECT(radioLeft),   GINT_TO_POINTER(AP_Dialog_PageNumbers::id_LALIGN));
  gtk_object_set_user_data(GTK_OBJECT(radioCenter), GINT_TO_POINTER(AP_Dialog_PageNumbers::id_CALIGN));
  gtk_object_set_user_data(GTK_OBJECT(radioRight),  GINT_TO_POINTER(AP_Dialog_PageNumbers::id_RALIGN));

  // Set our defaults to number in the top-right corner.
  m_control = AP_Dialog_PageNumbers::id_HDR;
  m_align = AP_Dialog_PageNumbers::id_RALIGN;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioHeader), true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioRight), true);
  
  // Connect clicked signals so that our callbacks get called.
  g_signal_connect(G_OBJECT(radioHeader), "clicked", G_CALLBACK(s_position_changed),  (gpointer)this);
  g_signal_connect(G_OBJECT(radioFooter), "clicked", G_CALLBACK(s_position_changed),  (gpointer)this);
  g_signal_connect(G_OBJECT(radioLeft),   "clicked", G_CALLBACK(s_alignment_changed), (gpointer)this);
  g_signal_connect(G_OBJECT(radioCenter), "clicked", G_CALLBACK(s_alignment_changed), (gpointer)this);
  g_signal_connect(G_OBJECT(radioRight),  "clicked", G_CALLBACK(s_alignment_changed), (gpointer)this);

  // the expose event off the preview
  g_signal_connect(G_OBJECT(m_previewArea), "expose_event", G_CALLBACK(s_preview_exposed), (gpointer)this);
  
  // The position combo entry

//  label1 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Position));
//  gtk_widget_ref (label1);
//  gtk_widget_show (label1);
//  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 0);
//  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
//  gtk_misc_set_alignment (GTK_MISC (label1), 0.04, 0.5);

//  combo1 = gtk_combo_new ();
//  gtk_widget_ref (combo1);
//  gtk_widget_show (combo1);
//  gtk_box_pack_start (GTK_BOX (vbox1), combo1, FALSE, FALSE, 0);

//  combo_entry1 = GTK_COMBO (combo1)->entry;
//  gtk_entry_set_editable (GTK_ENTRY (combo_entry1), FALSE);
//  gtk_widget_ref (combo_entry1);
//  gtk_widget_show (combo_entry1);
  
//  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Header));
//  gtk_widget_show(li);
//  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo1)->list), li);
//  gtk_object_set_user_data (G_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_HDR));
//  g_signal_connect (G_OBJECT (li), "select",
//		      G_CALLBACK (s_position_changed),
//		      (gpointer) this);

 
  
//  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Footer));
//  gtk_widget_show(li);
//  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo1)->list), li);
//  gtk_object_set_user_data (G_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_FTR));
//  g_signal_connect (G_OBJECT (li), "select",
//		      G_CALLBACK (s_position_changed),
//		      (gpointer) this);

  // The Alignment combo entry

//  label2 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Alignment));
//  gtk_widget_ref (label2);
//  gtk_widget_show (label2);
//  gtk_box_pack_start (GTK_BOX (vbox1), label2, FALSE, FALSE, 0);
//  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT); 
//  gtk_misc_set_alignment (GTK_MISC (label2), 0.04, 0.5);

//  combo2 = gtk_combo_new ();
//  gtk_widget_ref (combo2);
//  gtk_widget_show (combo2);
//  gtk_box_pack_start (GTK_BOX (vbox1), combo2, FALSE, FALSE, 0);

//  combo_entry2 = GTK_COMBO (combo2)->entry;
//  gtk_entry_set_editable (GTK_ENTRY (combo_entry2), FALSE);
//  gtk_widget_ref (combo_entry2);
//  gtk_widget_show (combo_entry2);

//  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Right));
//  gtk_widget_show(li);
//  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo2)->list), li);
//  gtk_object_set_user_data (G_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_RALIGN));
//  g_signal_connect (G_OBJECT (li), "select",
//		      G_CALLBACK (s_alignment_changed),
//		      (gpointer) this);

//  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Left));
//  gtk_widget_show(li);
//  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo2)->list), li);
//  gtk_object_set_user_data (G_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_LALIGN));
//  g_signal_connect (G_OBJECT (li), "select",
//		      G_CALLBACK (s_alignment_changed),
//		      (gpointer) this);

//  li = gtk_list_item_new_with_label(pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Center));
//  gtk_widget_show(li);
//  gtk_container_add (GTK_CONTAINER(GTK_COMBO(combo2)->list), li);
//  gtk_object_set_user_data (G_OBJECT (li), GINT_TO_POINTER (AP_Dialog_PageNumbers::id_CALIGN));
//  g_signal_connect (G_OBJECT (li), "select",
//		      G_CALLBACK (s_alignment_changed),
//		      (gpointer) this);


//  m_combo1 = combo1;
//  m_combo2 = combo2;
}

GtkWidget * AP_UnixDialog_PageNumbers::_constructWindow (void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_window = abiDialogNew ( true, pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Title)) ;

  _constructWindowContents (GTK_DIALOG(m_window)->vbox);

  abiAddStockButton ( GTK_DIALOG(m_window), GTK_STOCK_OK, BUTTON_OK ) ;
  abiAddStockButton ( GTK_DIALOG(m_window), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
  
  return m_window;
}
