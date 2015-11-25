/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
#include <time.h>
#include <stdio.h>
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_DocComparison.h"
#include "xap_UnixDialogFactory.h"

/*****************************************************************/


XAP_Dialog * XAP_UnixDialog_DocComparison::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	XAP_UnixDialog_DocComparison * p = new XAP_UnixDialog_DocComparison(pFactory,id);
	return p;
}

XAP_UnixDialog_DocComparison::XAP_UnixDialog_DocComparison(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_DocComparison(pDlgFactory,id),
	  m_windowMain(NULL)
{
}


XAP_UnixDialog_DocComparison::~XAP_UnixDialog_DocComparison(void)
{
}

void XAP_UnixDialog_DocComparison::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	// build the dialog
	GtkWidget * cf = constructWindow();    
	UT_return_if_fail(cf);	
	
	abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CLOSE,false );
	abiDestroyWidget(cf);
}

GtkWidget * XAP_UnixDialog_DocComparison::constructWindow(void)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("xap_UnixDlg_DocComparison.ui");
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_DocComparison"));
	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_DocComparison_WindowLabel,s);
	gtk_window_set_title (GTK_WINDOW(m_windowMain), s.c_str());
  
	_populateWindowData(builder);

	g_object_unref(G_OBJECT(builder));

	return m_windowMain;
}


void XAP_UnixDialog_DocComparison::_populateWindowData(GtkBuilder* builder)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();
    std::string text;
	localizeLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbDocCompared")), pSS, XAP_STRING_ID_DLG_DocComparison_DocsCompared);
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbDocument1")), getPath1());
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbDocument2")), getPath2());
	localizeLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbResults")), pSS, XAP_STRING_ID_DLG_DocComparison_Results);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_DocComparison_Relationship, text);
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbRelationship")), text.c_str());
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbRelationshipRes")), getResultValue(0));
	pSS->getValueUTF8(XAP_STRING_ID_DLG_DocComparison_Content, text);
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbContent")), text.c_str());
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbContentRes")), getResultValue(1));
	pSS->getValueUTF8(XAP_STRING_ID_DLG_DocComparison_Fmt, text);
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbFormat")), text.c_str());
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbFormatRes")), getResultValue(2));
	pSS->getValueUTF8(XAP_STRING_ID_DLG_DocComparison_Styles, text);
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbStyles")), text.c_str());
	gtk_label_set_text (GTK_LABEL(gtk_builder_get_object(builder, "lbStylesRes")),getResultValue(3));
}
