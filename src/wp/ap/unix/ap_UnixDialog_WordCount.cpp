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
#include <glade/glade.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_WordCount.h"

static void s_destroy_clicked(GtkWidget * /* widget */,
							  AP_UnixDialog_WordCount * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_OK();
}

static void s_delete_clicked(GtkWidget * widget,
							 gpointer, gpointer)
{
	abiDestroyWidget(widget);
}

XAP_Dialog * AP_UnixDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_UnixDialog_WordCount(pFactory,id);
}

AP_UnixDialog_WordCount::AP_UnixDialog_WordCount(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
}

AP_UnixDialog_WordCount::~AP_UnixDialog_WordCount(void)
{
}

void  AP_UnixDialog_WordCount::activate(void)
{
	UT_ASSERT (m_windowMain);
	
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setCountFromActiveFrame ();
	_updateWindowData ();
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_WordCount::s_response(GtkWidget * wid, gint id,
										 AP_UnixDialog_WordCount * me )
{
	if (id == GTK_RESPONSE_CLOSE)
		abiDestroyWidget(wid) ;
}

void AP_UnixDialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	_updateWindowData ();

	abiSetupModelessDialog(GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CLOSE);
	gtk_widget_show(mainWindow);

	// Now construct the timer for auto-updating
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this,NULL);
	m_pAutoUpdateWC->set(1000);
}
         
void AP_UnixDialog_WordCount::autoupdateWC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	AP_UnixDialog_WordCount * pDialog =  static_cast<AP_UnixDialog_WordCount *>(pTimer->getInstanceData());

	// Handshaking code

	if(pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->setCountFromActiveFrame ();
		pDialog->_updateWindowData ();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

void AP_UnixDialog_WordCount::event_OK(void)
{
	m_answer = AP_Dialog_WordCount::a_OK;
	destroy();
}

void AP_UnixDialog_WordCount::event_WindowDelete(void)
{
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	destroy();
}

void AP_UnixDialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW (m_windowMain), m_WindowName);
	setCountFromActiveFrame();
	_updateWindowData();
}

void AP_UnixDialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) 
		;
	m_pAutoUpdateWC->stop();
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	modeless_cleanup();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
	DELETEP(m_pAutoUpdateWC);
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_WordCount::_constructWindow(void)
{	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_WordCount.glade";

	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );

	const XAP_StringSet * pSS = m_pApp->getStringSet ();

	m_windowMain   = glade_xml_get_widget(xml, "mainWindow");
	m_labelWCount  = glade_xml_get_widget(xml, "lblWordsVal");
	m_labelPCount  = glade_xml_get_widget(xml, "lblParasVal");
	m_labelCCount  = glade_xml_get_widget(xml, "lblCharsVal");
	m_labelCNCount = glade_xml_get_widget(xml, "lblChars2Val");
	m_labelLCount  = glade_xml_get_widget(xml, "lblLinesVal");	
	m_labelPgCount = glade_xml_get_widget(xml, "lblPagesVal");	
	m_labelTitle   = glade_xml_get_widget(xml, "lblTitle");

	GtkWidget * labelWCount  = glade_xml_get_widget(xml, "lblWords");
	GtkWidget * labelPCount  = glade_xml_get_widget(xml, "lblParas");
	GtkWidget * labelCCount  = glade_xml_get_widget(xml, "lblChars");
	GtkWidget * labelCNCount = glade_xml_get_widget(xml, "lblChars2");
	GtkWidget * labelLCount  = glade_xml_get_widget(xml, "lblLines");	
	GtkWidget * labelPgCount = glade_xml_get_widget(xml, "lblPages");	

	gtk_label_set_text (GTK_LABEL (labelWCount),
						pSS->getValue(AP_STRING_ID_DLG_WordCount_Words));
	gtk_label_set_text (GTK_LABEL (labelPCount),
						pSS->getValue(AP_STRING_ID_DLG_WordCount_Paragraphs));
	gtk_label_set_text (GTK_LABEL (labelCCount),
						pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_Sp));
	gtk_label_set_text (GTK_LABEL (labelCNCount),
						pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_No));
	gtk_label_set_text (GTK_LABEL (labelLCount),
						pSS->getValue(AP_STRING_ID_DLG_WordCount_Lines));
	gtk_label_set_text (GTK_LABEL (labelPgCount),
						pSS->getValue(AP_STRING_ID_DLG_WordCount_Pages));

	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW(m_windowMain), m_WindowName);

   	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response), this);
	gtk_signal_connect(GTK_OBJECT(m_windowMain), "destroy",
					   GTK_SIGNAL_FUNC(s_destroy_clicked),
					   reinterpret_cast<gpointer>(this));
	gtk_signal_connect(GTK_OBJECT(m_windowMain), "delete_event",
					   GTK_SIGNAL_FUNC(s_delete_clicked),
					   reinterpret_cast<gpointer>(this));

	gtk_widget_show_all (m_windowMain);
	
	return m_windowMain;
}

void AP_UnixDialog_WordCount::_updateWindowData(void)
{
	char tmp[60];

	g_snprintf (tmp, sizeof(tmp), "%d", m_count.word);
	gtk_label_set_text(GTK_LABEL(m_labelWCount), tmp);
	
	g_snprintf (tmp, sizeof(tmp),"%d", m_count.para);
	gtk_label_set_text(GTK_LABEL(m_labelPCount), tmp);

	g_snprintf (tmp, sizeof(tmp),"%d", m_count.ch_sp);
	gtk_label_set_text(GTK_LABEL(m_labelCCount), tmp);

	g_snprintf (tmp, sizeof(tmp),"%d", m_count.ch_no);
	gtk_label_set_text(GTK_LABEL(m_labelCNCount), tmp);

	g_snprintf (tmp, sizeof(tmp),"%d", m_count.line);
	gtk_label_set_text(GTK_LABEL(m_labelLCount), tmp);

	g_snprintf (tmp, sizeof(tmp),"%d", m_count.page);
	gtk_label_set_text(GTK_LABEL(m_labelPgCount), tmp);

	setLabelMarkup (m_labelTitle, getActiveFrame ()->getTitle (60));
}
