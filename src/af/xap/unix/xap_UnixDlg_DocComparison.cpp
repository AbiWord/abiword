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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "ut_string.h"
#include <glade/glade.h>
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
	  m_pXML(NULL),
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
	
	_populateWindowData();  

	abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CLOSE,false );
	abiDestroyWidget(cf);
}

GtkWidget * XAP_UnixDialog_DocComparison::constructWindow(void)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/xap_UnixDlg_DocComparison.glade";
	
	// load the dialog from the glade file
	m_pXML = abiDialogNewFromXML( glade_path.c_str() );
	if (!m_pXML)
		return NULL;

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = glade_xml_get_widget(m_pXML, "xap_UnixDlg_DocComparison");
	UT_UTF8String s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_DocComparison_WindowLabel,s);
	gtk_window_set_title (GTK_WINDOW(m_windowMain), s.utf8_str());
  
	return m_windowMain;
}


void XAP_UnixDialog_DocComparison::_populateWindowData(void)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbDocCompared"), pSS, XAP_STRING_ID_DLG_DocComparison_DocsCompared);
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbDocument1"), getPath1());
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbDocument2"), getPath2());
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbResults"), pSS, XAP_STRING_ID_DLG_DocComparison_Results);
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbRelationship"), pSS, XAP_STRING_ID_DLG_DocComparison_Relationship);
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbRelationshipRes"), getResultValue(0));
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbContent"), pSS, XAP_STRING_ID_DLG_DocComparison_Content);
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbContentRes"), getResultValue(1));
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbFormat"), pSS, XAP_STRING_ID_DLG_DocComparison_Fmt);
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbFormatRes"), getResultValue(2));
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbStyles"), pSS, XAP_STRING_ID_DLG_DocComparison_Styles);
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbStylesRes"),getResultValue(3));
}


