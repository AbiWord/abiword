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
#include "xap_Frame.h"

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

void AP_UnixDialog_PageNumbers::event_PreviewExposed(void)
{
	if(m_preview)
		m_preview->draw();
}

void AP_UnixDialog_PageNumbers::event_AlignChanged(AP_Dialog_PageNumbers::tAlign align)
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
	
    // Build the dialog's window
    GtkWidget * m_window = _constructWindow();
    UT_return_if_fail(m_window);

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

	switch ( abiRunModalDialog ( GTK_DIALOG(m_window), pFrame, this,
								 GTK_RESPONSE_OK, false ) )
	{
		case GTK_RESPONSE_OK:
			m_answer = AP_Dialog_PageNumbers::a_OK;
			// set the align and control data
			m_align   = m_recentAlign;
			m_control = m_recentControl;			
			break;
		default:
			m_answer = AP_Dialog_PageNumbers::a_CANCEL;
			break;
	}

	DELETEP (m_unixGraphics);

	abiDestroyWidget ( m_window ) ;
}

GtkWidget * AP_UnixDialog_PageNumbers::_constructWindow (void)
{
	GtkWidget * window;	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_PageNumbers.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_PageNumbers");
	m_previewArea = glade_xml_get_widget(xml, "daPreview");
	
	// set the dialog title
	abiDialogSetTitle(window, pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Title).c_str());

	// disable double buffering on our preview
	gtk_widget_set_double_buffered(m_previewArea, FALSE);  
	
	// localize the strings in our dialog, and set some userdata for some widgets

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbPosition"), pSS, AP_STRING_ID_DLG_PageNumbers_Position);
	
	GtkWidget * radioHeader = glade_xml_get_widget(xml, "rbHeader");
	localizeButton(radioHeader, pSS, AP_STRING_ID_DLG_PageNumbers_Header);
	gtk_object_set_user_data(GTK_OBJECT(radioHeader), GINT_TO_POINTER(AP_Dialog_PageNumbers::id_HDR));	
	
	GtkWidget * radioFooter = glade_xml_get_widget(xml, "rbFooter");
	localizeButton(glade_xml_get_widget(xml, "rbFooter"), pSS, AP_STRING_ID_DLG_PageNumbers_Footer);	
	gtk_object_set_user_data(GTK_OBJECT(radioFooter), GINT_TO_POINTER(AP_Dialog_PageNumbers::id_FTR));	

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbAlignment"), pSS, AP_STRING_ID_DLG_PageNumbers_Alignment);
	
	GtkWidget * radioLeft =	glade_xml_get_widget(xml, "rbLeft");
	localizeButton(radioLeft, pSS, AP_STRING_ID_DLG_PageNumbers_Left);	
	gtk_object_set_user_data(GTK_OBJECT(radioLeft),   GINT_TO_POINTER(AP_Dialog_PageNumbers::id_LALIGN));
	
	GtkWidget * radioCenter = glade_xml_get_widget(xml, "rbCenter");
	localizeButton(radioCenter, pSS, AP_STRING_ID_DLG_PageNumbers_Center);	
	gtk_object_set_user_data(GTK_OBJECT(radioCenter), GINT_TO_POINTER(AP_Dialog_PageNumbers::id_CALIGN));

	GtkWidget * radioRight = glade_xml_get_widget(xml, "rbRight");
	localizeButton(radioRight, pSS, AP_STRING_ID_DLG_PageNumbers_Right);	
	gtk_object_set_user_data(GTK_OBJECT(radioRight),  GINT_TO_POINTER(AP_Dialog_PageNumbers::id_RALIGN));
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbPreview"), pSS, AP_STRING_ID_DLG_PageNumbers_Preview);
	
	// Set our defaults to number in the bottom-right corner.
	m_recentControl = m_control = AP_Dialog_PageNumbers::id_FTR;
	m_recentAlign = m_align = AP_Dialog_PageNumbers::id_RALIGN;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioFooter), true);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioRight), true);
	
	// Connect clicked signals so that our callbacks get called.
	g_signal_connect(G_OBJECT(radioHeader), "clicked", G_CALLBACK(s_position_changed),  (gpointer)this);
	g_signal_connect(G_OBJECT(radioFooter), "clicked", G_CALLBACK(s_position_changed),  (gpointer)this);
	g_signal_connect(G_OBJECT(radioLeft),   "clicked", G_CALLBACK(s_alignment_changed), (gpointer)this);
	g_signal_connect(G_OBJECT(radioCenter), "clicked", G_CALLBACK(s_alignment_changed), (gpointer)this);
	g_signal_connect(G_OBJECT(radioRight),  "clicked", G_CALLBACK(s_alignment_changed), (gpointer)this);

	// the expose event off the preview
	g_signal_connect(G_OBJECT(m_previewArea), "expose_event", G_CALLBACK(s_preview_exposed), (gpointer)this);	


	return window;
}
