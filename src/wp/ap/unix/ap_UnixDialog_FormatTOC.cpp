/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
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

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_FormatTOC.h"


static void s_delete_clicked(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}

static void s_destroy_clicked(GtkWidget * wid, AP_UnixDialog_FormatTOC * me )
{
   me->event_Close();
}

static void s_response_triggered(GtkWidget * widget, gint resp, AP_UnixDialog_FormatTOC * dlg)
{
	UT_return_if_fail(widget && dlg);
	
	if ( resp == GTK_RESPONSE_APPLY )
	  dlg->event_Apply();
	else if ( resp == GTK_RESPONSE_CLOSE )
	  abiDestroyWidget(widget);
}

XAP_Dialog * AP_UnixDialog_FormatTOC::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	return new AP_UnixDialog_FormatTOC(pFactory,id);
}

AP_UnixDialog_FormatTOC::AP_UnixDialog_FormatTOC(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: AP_Dialog_FormatTOC(pDlgFactory,id), 
	  m_windowMain(NULL),
	  m_wApply(NULL),
	  m_wClose(NULL)
{
}

AP_UnixDialog_FormatTOC::~AP_UnixDialog_FormatTOC(void)
{
}

void AP_UnixDialog_FormatTOC::event_Close(void)
{
	destroy();
}

void AP_UnixDialog_FormatTOC::setTOCPropsInGUI(void)
{
}

void AP_UnixDialog_FormatTOC::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_FormatTOC::activate(void)
{
	UT_ASSERT (m_windowMain);
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_FormatTOC::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
}

void AP_UnixDialog_FormatTOC::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	_connectSignals();
	abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,GTK_RESPONSE_CLOSE);
	startUpdater();
}

GtkWidget * AP_UnixDialog_FormatTOC::_constructWindow(void)
{
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_FormatTOC.glade";

	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	if (!xml)
		return NULL;
	
	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	m_windowMain   = glade_xml_get_widget(xml, "ap_UnixDialog_FormatTOC");
	m_wApply = glade_xml_get_widget(xml,"wApply");
	m_wClose = glade_xml_get_widget(xml,"wClose");

	// set the dialog title
	abiDialogSetTitle(m_windowMain, pSS->getValueUTF8(AP_STRING_ID_DLG_FormatTOC_Title).utf8_str());

// Heading settings

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbHeading"), pSS, AP_STRING_ID_DLG_FormatTOC_Heading);
	localizeLabel(glade_xml_get_widget(xml, "lbHeadingText"), pSS, AP_STRING_ID_DLG_FormatTOC_HeadingText);
	localizeLabel(glade_xml_get_widget(xml, "lbHeadingStyle"), pSS, AP_STRING_ID_DLG_FormatTOC_HeadingStyle);
	localizeButton(glade_xml_get_widget(xml, "wHaveHeading"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveHeading);
	localizeButton(glade_xml_get_widget(xml, "wChangeHeadingstyle"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

// Level 1 Settings

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbLevel1Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level1Defs);
	localizeLabel(glade_xml_get_widget(xml, "lbHaveLabel1"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);
	localizeLabel(glade_xml_get_widget(xml, "lbFillStyle1"), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbDispStyle1"), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbTabLeader1"), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
	localizeLabel(glade_xml_get_widget(xml, "lbIndent1"), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
	localizeButton(glade_xml_get_widget(xml, "wHaveLabel1"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);	
	localizeButton(glade_xml_get_widget(xml, "wChangeFill1"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	localizeButton(glade_xml_get_widget(xml, "wChangeDisp1"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);


// Level 2 Settings

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbLevel2Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level2Defs);
	localizeLabel(glade_xml_get_widget(xml, "lbHaveLabel2"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);
	localizeLabel(glade_xml_get_widget(xml, "lbFillStyle2"), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbDispStyle2"), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbTabLeader2"), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
	localizeLabel(glade_xml_get_widget(xml, "lbIndent2"), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
	localizeButton(glade_xml_get_widget(xml, "wHaveLabel2"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);	
	localizeButton(glade_xml_get_widget(xml, "wChangeFill2"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	localizeButton(glade_xml_get_widget(xml, "wChangeDisp2"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);



// Level 3 Settings

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbLevel3Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level3Defs);
	localizeLabel(glade_xml_get_widget(xml, "lbHaveLabel3"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);
	localizeLabel(glade_xml_get_widget(xml, "lbFillStyle3"), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbDispStyle3"), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbTabLeader3"), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
	localizeLabel(glade_xml_get_widget(xml, "lbIndent3"), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
	localizeButton(glade_xml_get_widget(xml, "wHaveLabel3"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);	
	localizeButton(glade_xml_get_widget(xml, "wChangeFill3"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	localizeButton(glade_xml_get_widget(xml, "wChangeDisp3"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);



// Level 4 Settings

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbLevel4Defs"), pSS, AP_STRING_ID_DLG_FormatTOC_Level4Defs);
	localizeLabel(glade_xml_get_widget(xml, "lbHaveLabel4"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);
	localizeLabel(glade_xml_get_widget(xml, "lbFillStyle4"), pSS, AP_STRING_ID_DLG_FormatTOC_FillStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbDispStyle4"), pSS, AP_STRING_ID_DLG_FormatTOC_DispStyle);
	localizeLabel(glade_xml_get_widget(xml, "lbTabLeader4"), pSS, AP_STRING_ID_DLG_FormatTOC_TabLeader);
	localizeLabel(glade_xml_get_widget(xml, "lbIndent4"), pSS, AP_STRING_ID_DLG_FormatTOC_Indent);
	localizeButton(glade_xml_get_widget(xml, "wHaveLabel4"), pSS, AP_STRING_ID_DLG_FormatTOC_HaveLabel);	
	localizeButton(glade_xml_get_widget(xml, "wChangeFill4"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);
	localizeButton(glade_xml_get_widget(xml, "wChangeDisp4"), pSS, AP_STRING_ID_DLG_FormatTOC_ChangeStyle);

	return m_windowMain;
}

void  AP_UnixDialog_FormatTOC::event_Apply(void)
{
	Apply();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void  AP_UnixDialog_FormatTOC::_fillGUI(void)
{
}

void  AP_UnixDialog_FormatTOC::_populateWindowData(void)
{
	_fillGUI();
	setTOCPropsInGUI();
}

void  AP_UnixDialog_FormatTOC::_connectSignals(void)
{
	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response_triggered), this);
	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);
}
