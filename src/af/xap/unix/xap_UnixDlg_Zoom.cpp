/* AbiSource Application Framework
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_UnixGraphics.h"

#include "xav_View.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Preview_Zoom.h"
#include "xap_UnixDlg_Zoom.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
						     XAP_Dialog_Id id)
{
  XAP_UnixDialog_Zoom * p = new XAP_UnixDialog_Zoom(pFactory,id);
  return p;
}

XAP_UnixDialog_Zoom::XAP_UnixDialog_Zoom(XAP_DialogFactory * pDlgFactory,
					 XAP_Dialog_Id id)
  : XAP_Dialog_Zoom(pDlgFactory,id)
{
  m_windowMain = NULL;
  
  m_unixGraphics = NULL;
  m_previewArea = 	NULL;
  
  m_radio200 = 		NULL;
  m_radio100 = 		NULL;
  m_radio75 = 		NULL;
  m_radioPageWidth = 	NULL;
  m_radioWholePage = 	NULL;
  m_radioPercent = 	NULL;
  
  m_spinPercent = NULL;
  
  m_radioGroup = NULL;
}

XAP_UnixDialog_Zoom::~XAP_UnixDialog_Zoom(void)
{
  DELETEP(m_unixGraphics);
}

/*****************************************************************/

void XAP_UnixDialog_Zoom::s_radio_200_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
  UT_return_if_fail(widget && dlg);
  dlg->event_Radio200Clicked();
}

void XAP_UnixDialog_Zoom::s_radio_100_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
  UT_return_if_fail(widget && dlg);
  dlg->event_Radio100Clicked();
}
void XAP_UnixDialog_Zoom::s_radio_75_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
  UT_return_if_fail(widget && dlg);
  dlg->event_Radio75Clicked();
}

void XAP_UnixDialog_Zoom::s_radio_PageWidth_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_RadioPageWidthClicked();
}

void XAP_UnixDialog_Zoom::s_radio_WholePage_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
  UT_return_if_fail(widget && dlg);
  dlg->event_RadioWholePageClicked();
}

void XAP_UnixDialog_Zoom::s_radio_Percent_clicked(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
  UT_return_if_fail(widget && dlg);
  dlg->event_RadioPercentClicked();
}

void XAP_UnixDialog_Zoom::s_spin_Percent_changed(GtkWidget * widget, XAP_UnixDialog_Zoom * dlg)
{
  UT_return_if_fail(widget && dlg);
  dlg->event_SpinPercentChanged();
}

gint XAP_UnixDialog_Zoom::s_preview_exposed(GtkWidget * /* widget */,
					    GdkEventExpose * /* pExposeEvent */,
					    XAP_UnixDialog_Zoom * dlg)
{
  UT_return_val_if_fail(dlg, false);
  dlg->event_PreviewAreaExposed();
  return FALSE;
}

/*****************************************************************/

void XAP_UnixDialog_Zoom::runModal(XAP_Frame * pFrame)
{
  m_pFrame = pFrame ;

  // Build the window's widgets and arrange them
  GtkWidget * mainWindow = _constructWindow();
  gtk_widget_show ( mainWindow ) ;

  // Populate the window's data items
  _populateWindowData();
  
  // *** this is how we add the gc ***
  {
    // attach a new graphics context to the drawing area
    XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
    UT_ASSERT(unixapp);
    
    UT_ASSERT(m_previewArea && m_previewArea->window);
    
    // make a new Unix GC
#ifndef WITH_PANGO		
    m_unixGraphics = new GR_UnixGraphics(m_previewArea->window, unixapp->getFontManager(), m_pApp);
#else
    m_unixGraphics = new GR_UnixGraphics(m_previewArea->window,m_pApp);
#endif		
    
    // let the widget materialize
    _createPreviewFromGC(m_unixGraphics,
			 static_cast<UT_uint32>(m_previewArea->allocation.width),
			 static_cast<UT_uint32>(m_previewArea->allocation.height));
  }
  
  // HACK : we call this TWICE so it generates an update on the buttons to
  // HACK : trigger a preview
  _populateWindowData();

  switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false ) )
    {
    case BUTTON_OK:
      event_OK () ; break ;
    default:
      event_Cancel () ; break ;
    }

  _storeWindowData();
	
  abiDestroyWidget (mainWindow) ;
}

void XAP_UnixDialog_Zoom::event_OK(void)
{
  m_answer = XAP_Dialog_Zoom::a_OK;
}

void XAP_UnixDialog_Zoom::event_Cancel(void)
{
  m_answer = XAP_Dialog_Zoom::a_CANCEL;
}

void XAP_UnixDialog_Zoom::event_Radio200Clicked(void)
{
  _enablePercentSpin(false);
  _updatePreviewZoomPercent(200);
}

void XAP_UnixDialog_Zoom::event_Radio100Clicked(void)
{
  _enablePercentSpin(false);
  _updatePreviewZoomPercent(100);
}

void XAP_UnixDialog_Zoom::event_Radio75Clicked(void)
{
  _enablePercentSpin(false);
  _updatePreviewZoomPercent(75);
}

void XAP_UnixDialog_Zoom::event_RadioPageWidthClicked(void)
{
  _enablePercentSpin(false);
  if ( m_pFrame )
    {
      UT_uint32 value = m_pFrame->getCurrentView ()->calculateZoomPercentForPageWidth () ;
      _updatePreviewZoomPercent(value);
    }
}

void XAP_UnixDialog_Zoom::event_RadioWholePageClicked(void)
{
  _enablePercentSpin(false);
  if ( m_pFrame )
    {
      UT_uint32 value = m_pFrame->getCurrentView ()->calculateZoomPercentForWholePage () ;
      _updatePreviewZoomPercent(value);
    }
}

void XAP_UnixDialog_Zoom::event_RadioPercentClicked(void)
{
  _enablePercentSpin(true);
  // call event_SpinPercentChanged() to do the fetch and update work
  event_SpinPercentChanged();
}

void XAP_UnixDialog_Zoom::event_SpinPercentChanged(void)
{
  _updatePreviewZoomPercent(static_cast<UT_uint32>(gtk_spin_button_get_value_as_int(
									 GTK_SPIN_BUTTON(m_spinPercent))));
}

void XAP_UnixDialog_Zoom::event_PreviewAreaExposed(void)
{
  UT_return_if_fail(m_zoomPreview);
  
  // trigger a draw on the preview area in the base class
  m_zoomPreview->draw();
}

/*****************************************************************/

GtkWidget * XAP_UnixDialog_Zoom::_constructWindow(void)
{
  GtkWidget * windowZoom;
  
  GtkWidget * vboxZoom;
  GtkWidget * hboxFrames;
  GtkWidget * frameZoomTo;
  GtkWidget * vboxZoomTo;
  GSList * 	vboxZoomTo_group = NULL;
  
  GtkWidget * radiobutton200;
  GtkWidget * radiobutton100;
  GtkWidget * radiobutton75;
  GtkWidget * radiobuttonPageWidth;
  GtkWidget * radiobuttonWholePage;
  GtkWidget * radiobuttonPercent;
  GtkObject * spinbuttonPercent_adj;
  GtkWidget * spinbuttonPercent;
  
  GtkWidget * framePreview;
  GtkWidget * drawingareaPreview;
  
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  
  XML_Char * tmp = NULL;
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_ZoomTitle).c_str());
  windowZoom = abiDialogNew ( "zoom dialog", TRUE, tmp ) ;
  FREEP(tmp);

  vboxZoom = GTK_DIALOG(windowZoom)->vbox;
  gtk_container_set_border_width (GTK_CONTAINER (vboxZoom), 10);

  hboxFrames = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hboxFrames);
  gtk_box_pack_start (GTK_BOX (vboxZoom), hboxFrames, FALSE, TRUE, 0);

  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_RadioFrameCaption).c_str());
  frameZoomTo = gtk_frame_new (tmp);
  gtk_frame_set_shadow_type(GTK_FRAME(frameZoomTo), GTK_SHADOW_NONE);
  FREEP(tmp);
  gtk_widget_show (frameZoomTo);
  gtk_box_pack_start (GTK_BOX (hboxFrames), frameZoomTo, FALSE, TRUE, 0);
  
  vboxZoomTo = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxZoomTo);
  gtk_container_add (GTK_CONTAINER (frameZoomTo), vboxZoomTo);
  gtk_container_border_width (GTK_CONTAINER (vboxZoomTo), 10);
  
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_200).c_str());
  radiobutton200 = gtk_radio_button_new_with_label (vboxZoomTo_group, tmp);
  FREEP(tmp);
  vboxZoomTo_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton200));
  g_object_set_data (G_OBJECT (radiobutton200), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_200));
  gtk_widget_show (radiobutton200);
  gtk_box_pack_start (GTK_BOX (vboxZoomTo), radiobutton200, FALSE, TRUE, 0);
  
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_100).c_str());
  radiobutton100 = gtk_radio_button_new_with_label (vboxZoomTo_group, tmp);
  FREEP(tmp);
  vboxZoomTo_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton100));
  g_object_set_data (G_OBJECT (radiobutton100), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_100));
  gtk_widget_show (radiobutton100);
  gtk_box_pack_start (GTK_BOX (vboxZoomTo), radiobutton100, FALSE, TRUE, 0);
  
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_75).c_str());
  radiobutton75 = gtk_radio_button_new_with_label (vboxZoomTo_group, tmp);
  FREEP(tmp);
  vboxZoomTo_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton75));
  g_object_set_data (G_OBJECT (radiobutton75), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_75));
  gtk_widget_show (radiobutton75);
  gtk_box_pack_start (GTK_BOX (vboxZoomTo), radiobutton75, TRUE, TRUE, 0);
  
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_PageWidth).c_str());
  radiobuttonPageWidth = gtk_radio_button_new_with_label (vboxZoomTo_group, tmp);
  FREEP(tmp);
  vboxZoomTo_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonPageWidth));
  g_object_set_data (G_OBJECT (radiobuttonPageWidth), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_PAGEWIDTH));
  gtk_widget_show (radiobuttonPageWidth);
  gtk_box_pack_start (GTK_BOX (vboxZoomTo), radiobuttonPageWidth, TRUE, TRUE, 0);
  
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_WholePage).c_str());
  radiobuttonWholePage = gtk_radio_button_new_with_label (vboxZoomTo_group, tmp);
  FREEP(tmp);
  vboxZoomTo_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonWholePage));
  g_object_set_data (G_OBJECT (radiobuttonWholePage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_WHOLEPAGE));
  gtk_widget_show (radiobuttonWholePage);
  gtk_box_pack_start (GTK_BOX (vboxZoomTo), radiobuttonWholePage, TRUE, TRUE, 0);
  
  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_Percent).c_str());
  radiobuttonPercent = gtk_radio_button_new_with_label (vboxZoomTo_group, tmp);
  FREEP(tmp);
  vboxZoomTo_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobuttonPercent));
  g_object_set_data (G_OBJECT (radiobuttonPercent), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_PERCENT));
  gtk_widget_show (radiobuttonPercent);
  gtk_box_pack_start (GTK_BOX (vboxZoomTo), radiobuttonPercent, TRUE, TRUE, 0);

  spinbuttonPercent_adj = gtk_adjustment_new (1, 1, 500, 1, 10, 10);
  spinbuttonPercent = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonPercent_adj), 1, 0);
  gtk_widget_show (spinbuttonPercent);
  gtk_box_pack_end (GTK_BOX (vboxZoomTo), spinbuttonPercent, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonPercent), TRUE);

  UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_PreviewFrame).c_str());
  framePreview = gtk_frame_new (tmp);
  gtk_frame_set_shadow_type(GTK_FRAME(framePreview), GTK_SHADOW_NONE);
  FREEP(tmp);
  gtk_widget_show (framePreview);
  gtk_box_pack_start (GTK_BOX (hboxFrames), framePreview, TRUE, TRUE, 0);
  
  // TODO: do something dynamically here?  How do we set this "sample" font?
  // 
  // aiken: add padding around the preview area.
  GtkWidget* padding = gtk_frame_new(0);
  gtk_container_set_border_width(GTK_CONTAINER(padding), 10);
  gtk_container_add(GTK_CONTAINER(framePreview), padding);
  gtk_frame_set_shadow_type(GTK_FRAME(padding), GTK_SHADOW_NONE);
  gtk_widget_show(padding); 
  
  // *** This is how we do a preview widget ***
  drawingareaPreview = createDrawingArea ();
  gtk_widget_show (drawingareaPreview);

  // aiken: change container to padding instead of frameSampleText
  gtk_container_add (GTK_CONTAINER (padding), drawingareaPreview);
  gtk_widget_set_usize (drawingareaPreview, 149, 10);  	
  
  abiAddStockButton(GTK_DIALOG(windowZoom), GTK_STOCK_CANCEL, BUTTON_CANCEL);
  abiAddStockButton(GTK_DIALOG(windowZoom), GTK_STOCK_OK, BUTTON_OK);
  
  // the radio buttons
  g_signal_connect(G_OBJECT(radiobutton200),
		   "clicked",
		   G_CALLBACK(s_radio_200_clicked),
		   static_cast<gpointer>(this));
  g_signal_connect(G_OBJECT(radiobutton100),
		   "clicked",
		   G_CALLBACK(s_radio_100_clicked),
		   static_cast<gpointer>(this));
  g_signal_connect(G_OBJECT(radiobutton75),
		   "clicked",
		   G_CALLBACK(s_radio_75_clicked),
					   static_cast<gpointer>(this));
  g_signal_connect(G_OBJECT(radiobuttonPageWidth),
		   "clicked",
		   G_CALLBACK(s_radio_PageWidth_clicked),
		   static_cast<gpointer>(this));
  g_signal_connect(G_OBJECT(radiobuttonWholePage),
		   "clicked",
		   G_CALLBACK(s_radio_WholePage_clicked),
		   static_cast<gpointer>(this));
  g_signal_connect(G_OBJECT(radiobuttonPercent),
		   "clicked",
		   G_CALLBACK(s_radio_Percent_clicked),
		   static_cast<gpointer>(this));
  
  // the spin button
  g_signal_connect(G_OBJECT(spinbuttonPercent_adj),
		   "value_changed",
		   G_CALLBACK(s_spin_Percent_changed),
		   static_cast<gpointer>(this));
	
  // the expose event off the preview
  g_signal_connect(G_OBJECT(drawingareaPreview),
		   "expose_event",
		   G_CALLBACK(s_preview_exposed),
		   static_cast<gpointer>(this));
  
  // Update member variables with the important widgets that
  // might need to be queried or altered later.
  
  m_windowMain = windowZoom;
  
  m_previewArea = 	drawingareaPreview;
	
  m_radio200 = 		radiobutton200;
  m_radio100 = 		radiobutton100;
  m_radio75 = 		radiobutton75;
  m_radioPageWidth = 	radiobuttonPageWidth;
  m_radioWholePage = 	radiobuttonWholePage;
  m_radioPercent = 	radiobuttonPercent;
  
  m_spinPercent = spinbuttonPercent;
  
  m_radioGroup = vboxZoomTo_group;
  
  return windowZoom;
}

void XAP_UnixDialog_Zoom::_populateWindowData(void)
{
  // The callbacks for these radio buttons aren't always
  // called when the dialog is being constructed, so we have to
  // set the widget's value, then manually enable/disable
  // the spin button.
  
  // enable the right button
  _enablePercentSpin(false);	// default
  switch(getZoomType())
    {
    case XAP_Frame::z_200:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio200), TRUE);
      _updatePreviewZoomPercent(200);
      break;
    case XAP_Frame::z_100:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio100), TRUE);
      _updatePreviewZoomPercent(100);		
		break;
    case XAP_Frame::z_75:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio75), TRUE);
      _updatePreviewZoomPercent(75);
      break;
    case XAP_Frame::z_PAGEWIDTH:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radioPageWidth), TRUE);
      break;
    case XAP_Frame::z_WHOLEPAGE:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radioWholePage), TRUE);
      break;
    case XAP_Frame::z_PERCENT:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radioPercent), TRUE);
      _enablePercentSpin(true);	// override
      _updatePreviewZoomPercent(getZoomPercent());
      break;
    default:
      // if they haven't set anything yet, default to the 100% radio item
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_radio100), TRUE);		
    }
  
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_spinPercent), static_cast<gfloat>(getZoomPercent()));
}

void XAP_UnixDialog_Zoom::_storeWindowData(void)
{
  for (GSList * item = m_radioGroup; item ; item = item->next)
    {
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item->data)))
	{
	  m_zoomType = (XAP_Frame::tZoomType)
	    GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data), WIDGET_ID_TAG_KEY));
	  break;
	}
    }
  
  // store away percentage; the base class decides if it's important when
  // the caller requests the percent
  m_zoomPercent = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_spinPercent));
}

void XAP_UnixDialog_Zoom::_enablePercentSpin(bool enable)
{
  UT_ASSERT(m_spinPercent);
  
  gtk_widget_set_sensitive(m_spinPercent, enable);
}

	
