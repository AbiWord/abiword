/* AbiWord
 * Copyright (C) 2000-2002 AbiSource, Inc.
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

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_Password.h"

void XAP_UnixDialog_Password::event_OK ()
{
	const char * txt = gtk_entry_get_text (GTK_ENTRY(mTextEntry));
	if (txt && strlen(txt)) {
		setPassword (txt);
		setAnswer(XAP_Dialog_Password::a_OK);
	} else {
		setAnswer(XAP_Dialog_Password::a_Cancel);
	}
}

void XAP_UnixDialog_Password::event_Return()
{
	gtk_dialog_response ( GTK_DIALOG ( mMainWindow ), GTK_RESPONSE_OK ) ;
}

void XAP_UnixDialog_Password::event_Cancel ()
{
	setAnswer(XAP_Dialog_Password::a_Cancel);
}

XAP_Dialog * XAP_UnixDialog_Password::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_Password(pFactory,id);
}

XAP_UnixDialog_Password::XAP_UnixDialog_Password(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: XAP_Dialog_Password(pDlgFactory,id)
{
}

XAP_UnixDialog_Password::~XAP_UnixDialog_Password(void)
{
}

void XAP_UnixDialog_Password::runModal(XAP_Frame * pFrame)
{
	GtkWidget * cf = _constructWindow();
	UT_return_if_fail(cf);	
	
	switch ( abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_OK, false ) )
    {
    case GTK_RESPONSE_OK:
		event_OK(); break;
    default:
		event_Cancel(); break;
    }
	
	gdk_keyboard_ungrab(GDK_CURRENT_TIME);
	
	abiDestroyWidget(cf);
}

GtkWidget * XAP_UnixDialog_Password::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/xap_UnixDlg_Password.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	mMainWindow = glade_xml_get_widget(xml, "xap_UnixDlg_Password");
	mTextEntry = glade_xml_get_widget(xml, "enPassword");

	UT_UTF8String s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Password_Title,s);
	gtk_window_set_title (GTK_WINDOW(mMainWindow), s.utf8_str());

	/* localize labels */
	localizeLabel (glade_xml_get_widget (xml, "lbPassword"), pSS, XAP_STRING_ID_DLG_Password_Password);
	
	g_signal_connect (G_OBJECT(mTextEntry), "activate",
					  G_CALLBACK(s_return_hit),
					  static_cast<gpointer>(this));
	
	gtk_widget_grab_focus(mTextEntry);

	return mMainWindow;
}
