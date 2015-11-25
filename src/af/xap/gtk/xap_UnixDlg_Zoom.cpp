 /* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

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
  return new XAP_UnixDialog_Zoom(pFactory,id);
}

XAP_UnixDialog_Zoom::XAP_UnixDialog_Zoom(XAP_DialogFactory * pDlgFactory,
					 XAP_Dialog_Id id)
  : XAP_Dialog_Zoom(pDlgFactory,id)
{
  m_windowMain = NULL;
    
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

/*****************************************************************/

void XAP_UnixDialog_Zoom::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame ;
	
	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);
	
	// Populate the window's data items
	_populateWindowData();
  
    // attach a new graphics context to the drawing area
	//   XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
    //UT_ASSERT(unixapp);
    
	// HACK : we call this TWICE so it generates an update on the buttons to
	// HACK : trigger a preview
	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain), pFrame, this, GTK_RESPONSE_CANCEL, false ) )
	{
		case GTK_RESPONSE_OK:
			m_answer = XAP_Dialog_Zoom::a_OK;
			break;
		default:
			m_answer = XAP_Dialog_Zoom::a_CANCEL;
			break;
	}
	
	_storeWindowData();
	
	abiDestroyWidget (m_windowMain) ;
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
		_updatePreviewZoomPercent(m_pFrame->getCurrentView ()->calculateZoomPercentForPageWidth ());
}

void XAP_UnixDialog_Zoom::event_RadioWholePageClicked(void)
{
	_enablePercentSpin(false);
	if ( m_pFrame )
		_updatePreviewZoomPercent(m_pFrame->getCurrentView ()->calculateZoomPercentForWholePage ());
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

/*****************************************************************/

GtkWidget * XAP_UnixDialog_Zoom::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("xap_UnixDlg_Zoom.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_Zoom"));
	m_radioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON ( GTK_WIDGET(gtk_builder_get_object(builder, "rbPercent200")) ));
	m_radio200 = GTK_WIDGET(gtk_builder_get_object(builder, "rbPercent200"));
	m_radio100 = GTK_WIDGET(gtk_builder_get_object(builder, "rbPercent100"));
	m_radio75 = GTK_WIDGET(gtk_builder_get_object(builder, "rbPercent75"));
	m_radioPageWidth = GTK_WIDGET(gtk_builder_get_object(builder, "rbPageWidth"));
	m_radioWholePage = GTK_WIDGET(gtk_builder_get_object(builder, "rbWholePage"));
	m_radioPercent = GTK_WIDGET(gtk_builder_get_object(builder, "rbPercent"));
	m_spinPercent = GTK_WIDGET(gtk_builder_get_object(builder, "sbPercent"));
	m_spinAdj = gtk_spin_button_get_adjustment( GTK_SPIN_BUTTON(m_spinPercent) );

	// set the dialog title
	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Zoom_ZoomTitle,s);
	abiDialogSetTitle(window, "%s", s.c_str());

	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbZoom")), pSS, XAP_STRING_ID_DLG_Zoom_RadioFrameCaption);

	localizeButton(m_radio200, pSS, XAP_STRING_ID_DLG_Zoom_200);
	g_object_set_data (G_OBJECT (m_radio200), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_200));
  
	localizeButton(m_radio100, pSS, XAP_STRING_ID_DLG_Zoom_100);
	g_object_set_data (G_OBJECT (m_radio100), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_100));
  
	localizeButton(m_radio75, pSS, XAP_STRING_ID_DLG_Zoom_75);
	g_object_set_data (G_OBJECT (m_radio75), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_75));
	
	localizeButton(m_radioPageWidth, pSS, XAP_STRING_ID_DLG_Zoom_PageWidth);
	g_object_set_data (G_OBJECT (m_radioPageWidth), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_PAGEWIDTH));
	
	localizeButton(m_radioWholePage, pSS, XAP_STRING_ID_DLG_Zoom_WholePage);
	g_object_set_data (G_OBJECT (m_radioWholePage), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_WHOLEPAGE));
	
	localizeButton(m_radioPercent, pSS, XAP_STRING_ID_DLG_Zoom_Percent);
	g_object_set_data (G_OBJECT (m_radioPercent), WIDGET_ID_TAG_KEY, GINT_TO_POINTER(XAP_Frame::z_PERCENT));
	
	// Connect clicked signals so that our callbacks get called.
	g_signal_connect(G_OBJECT(m_radio200),       "clicked", G_CALLBACK(s_radio_200_clicked),       static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_radio100),       "clicked", G_CALLBACK(s_radio_100_clicked),       static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_radio75),        "clicked", G_CALLBACK(s_radio_75_clicked),        static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_radioPageWidth), "clicked", G_CALLBACK(s_radio_PageWidth_clicked), static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_radioWholePage), "clicked", G_CALLBACK(s_radio_WholePage_clicked), static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_radioPercent),   "clicked", G_CALLBACK(s_radio_Percent_clicked),   static_cast<gpointer>(this));

	// the zoom spin button
	g_signal_connect(G_OBJECT(m_spinAdj), "value_changed", G_CALLBACK(s_spin_Percent_changed), static_cast<gpointer>(this));

	g_object_unref(G_OBJECT(builder));

	return window;
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
