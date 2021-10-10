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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "gr_UnixCairoGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_UnixDialog_PageNumbers.h"

/*****************************************************************/

#define CUSTOM_RESPONSE_INSERT 1

/*****************************************************************/

static gint s_preview_draw(GtkWidget * /*w*/,
			   cairo_t * /*cr*/,
			   AP_UnixDialog_PageNumbers * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_PreviewDraw();
	return FALSE;
}

static void s_position_changed (GtkWidget * w, AP_UnixDialog_PageNumbers *dlg)
{
	int pos = GPOINTER_TO_INT (g_object_get_data(G_OBJECT (w), "user_data"));
	dlg->event_HdrFtrChanged((AP_Dialog_PageNumbers::tControl)pos);
}

static void s_alignment_changed (GtkWidget * w, AP_UnixDialog_PageNumbers *dlg)
{
	int align = GPOINTER_TO_INT (g_object_get_data(G_OBJECT (w), "user_data"));
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

void AP_UnixDialog_PageNumbers::event_PreviewInvalidate(void)
{
	if (m_preview) {
		m_preview->queueDraw();
	}
}

void AP_UnixDialog_PageNumbers::event_PreviewDraw(void)
{
	if (m_preview) {
		m_preview->drawImmediate();
	}
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
    m_window = _constructWindow();
    UT_return_if_fail(m_window);

	// attach a new graphics context to the drawing area
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (m_pApp);
	
	UT_return_if_fail(unixapp);
	UT_return_if_fail(m_previewArea && XAP_HAS_NATIVE_WINDOW(m_previewArea));
	DELETEP (m_unixGraphics);
	
	// make a new Unix GC
	GR_UnixCairoAllocInfo ai(m_previewArea);
	m_unixGraphics =
	    (GR_UnixCairoGraphics*) XAP_App::getApp()->newGraphics(ai);
	
	
	// let the widget materialize
	GtkAllocation allocation;
	gtk_widget_get_allocation(m_previewArea, &allocation);
	_createPreviewFromGC(m_unixGraphics,
		   static_cast<UT_uint32>(allocation.width),
		   static_cast<UT_uint32>(allocation.height));
		   
	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_unixGraphics->init3dColors(m_previewArea);

	// hack in a quick draw here
	_updatePreview(m_recentAlign, m_recentControl);

	switch ( abiRunModalDialog ( GTK_DIALOG(m_window), pFrame, this,
								 CUSTOM_RESPONSE_INSERT, false ) )
	{
		case CUSTOM_RESPONSE_INSERT:
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
	GtkWidget * window = nullptr;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	GtkBuilder * builder = newDialogBuilderFromResource("ap_UnixDialog_PageNumbers.ui");

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_PageNumbers"));
	m_previewArea = GTK_WIDGET(gtk_builder_get_object(builder, "daPreview"));
	
	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_PageNumbers_Title,s);
	abiDialogSetTitle(window, "%s", s.c_str());

	// localize the strings in our dialog, and set some userdata for some widgets
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPosition")), pSS, AP_STRING_ID_DLG_PageNumbers_Position_No_Colon);
	
	GtkWidget * radioHeader = GTK_WIDGET(gtk_builder_get_object(builder, "rbHeader"));
	localizeButton(radioHeader, pSS, AP_STRING_ID_DLG_PageNumbers_Header);
	g_object_set_data(G_OBJECT(radioHeader), "user_data", GINT_TO_POINTER(AP_Dialog_PageNumbers::id_HDR));	
	
	GtkWidget * radioFooter = GTK_WIDGET(gtk_builder_get_object(builder, "rbFooter"));
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbFooter")), pSS, AP_STRING_ID_DLG_PageNumbers_Footer);	
	g_object_set_data(G_OBJECT(radioFooter), "user_data", GINT_TO_POINTER(AP_Dialog_PageNumbers::id_FTR));	

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbAlignment")), pSS, AP_STRING_ID_DLG_PageNumbers_Alignment_No_Colon);
	
	GtkWidget * radioLeft =	GTK_WIDGET(gtk_builder_get_object(builder, "rbLeft"));
	localizeButton(radioLeft, pSS, AP_STRING_ID_DLG_PageNumbers_Left);	
	g_object_set_data(G_OBJECT(radioLeft), "user_data",  GINT_TO_POINTER(AP_Dialog_PageNumbers::id_LALIGN));
	
	GtkWidget * radioCenter = GTK_WIDGET(gtk_builder_get_object(builder, "rbCenter"));
	localizeButton(radioCenter, pSS, AP_STRING_ID_DLG_PageNumbers_Center);	
	g_object_set_data(G_OBJECT(radioCenter), "user_data", GINT_TO_POINTER(AP_Dialog_PageNumbers::id_CALIGN));

	GtkWidget * radioRight = GTK_WIDGET(gtk_builder_get_object(builder, "rbRight"));
	localizeButton(radioRight, pSS, AP_STRING_ID_DLG_PageNumbers_Right);	
	g_object_set_data(G_OBJECT(radioRight), "user_data", GINT_TO_POINTER(AP_Dialog_PageNumbers::id_RALIGN));
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbPreview")), pSS, AP_STRING_ID_DLG_PageNumbers_Preview);
	
	localizeButtonUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "btInsert")), pSS, AP_STRING_ID_DLG_InsertButton);

	// Set our defaults to number in the bottom-right corner.
	m_recentControl = m_control = AP_Dialog_PageNumbers::id_FTR;
	m_recentAlign = m_align = AP_Dialog_PageNumbers::id_RALIGN;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioFooter), true);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioRight), true);
	
	// Connect clicked signals so that our callbacks get called.
	g_signal_connect(G_OBJECT(radioHeader), "clicked", G_CALLBACK(s_position_changed),  static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(radioFooter), "clicked", G_CALLBACK(s_position_changed),  static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(radioLeft),   "clicked", G_CALLBACK(s_alignment_changed), static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(radioCenter), "clicked", G_CALLBACK(s_alignment_changed), static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(radioRight),  "clicked", G_CALLBACK(s_alignment_changed), static_cast<gpointer>(this));

	// the expose event off the preview
	g_signal_connect(G_OBJECT(m_previewArea), "draw", G_CALLBACK(s_preview_draw), 
			 static_cast<gpointer>(this));

	g_object_unref(G_OBJECT(builder));

	return window;
}
