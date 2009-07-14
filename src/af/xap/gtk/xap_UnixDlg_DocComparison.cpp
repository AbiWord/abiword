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
	// get the path where our UI file is located
	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/xap_UnixDlg_DocComparison.xml";
	
	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_file(builder, ui_path.c_str(), NULL);

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_DocComparison"));

	_populateWindowData(builder);

	g_object_unref(G_OBJECT(builder));

	return m_windowMain;
}


void XAP_UnixDialog_DocComparison::_populateWindowData(GtkBuilder* builder)
{
	setLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbDocument1")), getPath1());
	setLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbDocument2")), getPath2());
	setLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbRelationshipRes")), getResultValue(0));
	setLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbContentRes")), getResultValue(1));
	setLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbFormatRes")), getResultValue(2));
	setLabelMarkup (GTK_WIDGET(gtk_builder_get_object(builder, "lbStylesRes")),getResultValue(3));
}
