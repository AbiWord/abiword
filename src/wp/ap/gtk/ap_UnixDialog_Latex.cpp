/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixWidget.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Latex.h"
#include "xap_Dlg_MessageBox.h"


static gboolean s_delete_clicked(GtkWidget * /*widget*/, GdkEvent * /*event*/, AP_UnixDialog_Latex * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return TRUE;
}

static void s_close_clicked(GtkWidget * /*widget*/,AP_UnixDialog_Latex * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}

static void s_destroy_clicked(GtkWidget * /*widget*/,AP_UnixDialog_Latex * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}

static void s_insert_clicked(GtkWidget * /*widget*/,AP_UnixDialog_Latex * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Insert();
}

XAP_Dialog * AP_UnixDialog_Latex::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Latex(pFactory,id);
}

AP_UnixDialog_Latex::AP_UnixDialog_Latex(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : 
  AP_Dialog_Latex(pDlgFactory,id),
  m_wClose(NULL), 
  m_wInsert(NULL),
  m_wText(NULL),
  m_windowMain(NULL)
{
  UT_DEBUGMSG(("Constructing Latex dialog %p \n",this));
}

AP_UnixDialog_Latex::~AP_UnixDialog_Latex(void)
{
}

void  AP_UnixDialog_Latex::activate(void)
{
	// FIXME move to XP
	UT_ASSERT (m_windowMain);
	
	ConstructWindowName();

	gdk_window_raise (gtk_widget_get_window(m_windowMain));
}

void AP_UnixDialog_Latex::runModeless(XAP_Frame * pFrame)
{
	constructDialog();
	UT_return_if_fail(m_windowMain);

	abiSetupModelessDialog(GTK_DIALOG(m_windowMain), pFrame, this, 
						   GTK_RESPONSE_CLOSE);
	gtk_widget_show(m_windowMain);
}


void AP_UnixDialog_Latex::event_Insert(void)
{
	getLatexFromGUI();
	if (convertLatexToMathML())
	{
		insertIntoDoc();
	}
}

void AP_UnixDialog_Latex::event_Close(void)
{
	destroy();
}

void AP_UnixDialog_Latex::event_WindowDelete(void)
{
	destroy();
}

void AP_UnixDialog_Latex::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	// FIXME put that in XP code
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW(m_windowMain), m_sWindowName.utf8_str());
}

void AP_UnixDialog_Latex::destroy(void)
{
	m_answer = AP_Dialog_Latex::a_CANCEL;	
	modeless_cleanup();
	if (m_windowMain != NULL)
	{
		gtk_widget_destroy(m_windowMain);
		m_windowMain = NULL;
	}
}

void AP_UnixDialog_Latex::setLatexInGUI(void)
{
	UT_UTF8String sLatex;
	getLatex(sLatex);
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	gtk_text_buffer_set_text (buffer, sLatex.utf8_str(), -1);
}

bool AP_UnixDialog_Latex::getLatexFromGUI(void)
{
	UT_UTF8String sLatex;

	//
	// Get the chars from the widget
	//
	gchar * sz = NULL;
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	GtkTextIter startIter,endIter;
	gtk_text_buffer_get_start_iter  (buffer,&startIter);
	gtk_text_buffer_get_end_iter    (buffer,&endIter);
	sz = gtk_text_buffer_get_text   (buffer,&startIter,&endIter,TRUE);
	sLatex = sz;
	g_free(sz);
	UT_DEBUGMSG(("LAtex from widget is %s \n",sLatex.utf8_str()));
	setLatex(sLatex);

	return true;
}


/*****************************************************************/

void AP_UnixDialog_Latex::constructDialog(void)
{	
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	// load the dialog from the UI file
#if GTK_CHECK_VERSION(3,0,0)
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Latex.ui");
#else
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Latex-2.ui");
#endif

        // Update our member variables with the important widgets that
        // might need to be queried or altered later
	m_windowMain   = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Latex"));
	m_wClose = GTK_WIDGET(gtk_builder_get_object(builder, "wClose"));
	m_wInsert =  GTK_WIDGET(gtk_builder_get_object(builder, "wInsert"));
	m_wText = GTK_WIDGET(gtk_builder_get_object(builder, "wTextView"));

	// localize the strings in our dialog, and set tags for some widgets

	localizeButtonUnderline(m_wInsert, pSS, AP_STRING_ID_DLG_InsertButton);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbLatexEquation")), pSS, AP_STRING_ID_DLG_Latex_LatexEquation);
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbExample")), pSS, AP_STRING_ID_DLG_Latex_Example);

	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW(m_windowMain), m_sWindowName.utf8_str());

	g_signal_connect(G_OBJECT(m_windowMain), "delete_event",
					   G_CALLBACK(s_delete_clicked),
					   reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_windowMain), "destroy",
					   G_CALLBACK(s_destroy_clicked),
					   reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wClose), "clicked",
					   G_CALLBACK(s_close_clicked),
					   reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wInsert), "clicked",
					   G_CALLBACK(s_insert_clicked),
					   reinterpret_cast<gpointer>(this));

	gtk_widget_show_all (m_windowMain);

	g_object_unref(G_OBJECT(builder));
}

