/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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
#include <glade/glade.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MailMerge.h"
#include "ap_UnixDialog_MailMerge.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MailMerge::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_MailMerge * p = new AP_UnixDialog_MailMerge(pFactory,id);
	return p;
}

AP_UnixDialog_MailMerge::AP_UnixDialog_MailMerge(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MailMerge(pDlgFactory,id)
{
	m_windowMain = NULL;
}

AP_UnixDialog_MailMerge::~AP_UnixDialog_MailMerge(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_MailMerge::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
    // Build the dialog's window
	_constructWindow();
	UT_return_if_fail(m_windowMain);

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain),
								 pFrame, this, GTK_RESPONSE_CANCEL, false ) )
	{
		case GTK_RESPONSE_OK:
			setAnswer(AP_Dialog_MailMerge::a_OK);
			break;
		default:
			setAnswer(AP_Dialog_MailMerge::a_CANCEL);
			break;
	}

	if (getAnswer() == AP_Dialog_MailMerge::a_OK)
		setMergeField (gtk_entry_get_text (GTK_ENTRY(m_entry)));
	
	abiDestroyWidget ( m_windowMain ) ;
}

/*****************************************************************/
void AP_UnixDialog_MailMerge::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_MailMerge.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = glade_xml_get_widget(xml, "ap_UnixDialog_MailMerge");
	m_entry = glade_xml_get_widget(xml, "mergeEntry");

	// set the dialog title
	abiDialogSetTitle(m_windowMain, pSS->getValueUTF8(AP_STRING_ID_DLG_MailMerge_MailMergeTitle).c_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "mergeLbl"), pSS, AP_STRING_ID_DLG_MailMerge_Insert);	
}
