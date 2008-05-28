/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixDialogHelper.h"
#include "ut_string_class.h"
#include <xp/AbiCollabSessionManager.h>

#include "ap_UnixDialog_GenericProgress.h"

XAP_Dialog * AP_UnixDialog_GenericProgress::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_UnixDialog_GenericProgress(pFactory, id));
}
pt2Constructor ap_Dialog_GenericProgress_Constructor = &AP_UnixDialog_GenericProgress::static_constructor;

AP_UnixDialog_GenericProgress::AP_UnixDialog_GenericProgress(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_GenericProgress(pDlgFactory, id),
	m_wWindowMain(NULL),
	m_wCancel(NULL),
	m_wProgress(NULL)
{
}

void AP_UnixDialog_GenericProgress::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the dialog's window
	m_wWindowMain = _constructWindow();
	UT_return_if_fail(m_wWindowMain);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_wWindowMain),
								 pFrame, this, AP_Dialog_GenericProgress::a_CANCEL, false ) )
	{
		case GTK_RESPONSE_CANCEL:
			m_answer = AP_Dialog_GenericProgress::a_CANCEL;
			break;
		case GTK_RESPONSE_OK:
			m_answer = AP_Dialog_GenericProgress::a_OK;
			break;
		default:
			m_answer = AP_Dialog_GenericProgress::a_OK;
			break;
	}

	abiDestroyWidget(m_wWindowMain);
}

void AP_UnixDialog_GenericProgress::close()
{
	UT_return_if_fail(m_wWindowMain);
	gtk_dialog_response(GTK_DIALOG(m_wWindowMain), AP_Dialog_GenericProgress::a_OK);
}

void AP_UnixDialog_GenericProgress::setProgress(UT_uint32 progress)
{
	UT_return_if_fail(m_wProgress);
	UT_return_if_fail(progress >= 0 && progress <= 100);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_wProgress), progress / 100.0f);
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_GenericProgress::_constructWindow(void)
{
	GtkWidget* window;
	//const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(XAP_App::getApp());
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_GenericProgress.glade";
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_GenericProgress");
	m_wCancel = glade_xml_get_widget(xml, "btCancel");
	m_wProgress = glade_xml_get_widget(xml, "pbProgress");

	// set the dialog title
	abiDialogSetTitle(window, getTitle().utf8_str());
	
	// set the informative label
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml, "lbInformation")), getInformation().utf8_str());
	//gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml, "lbLabel")), getLabel().utf8_str());

	return window;
}

void AP_UnixDialog_GenericProgress::_populateWindowData()
{
	// set the focus on the text input
	// TODO: implement me
}

