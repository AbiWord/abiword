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
#include <goffice/goffice.h>


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
#ifdef HAVE_GO_MATH_EDITOR_NEW
	go_math_editor_set_itex(GO_MATH_EDITOR(m_wText), sLatex.utf8_str());
	go_math_editor_set_inline(GO_MATH_EDITOR(m_wText), m_compact);
#else
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	gtk_text_buffer_set_text (buffer, sLatex.utf8_str(), -1);
#endif
}

bool AP_UnixDialog_Latex::getLatexFromGUI(void)
{
	UT_UTF8String sLatex;

	//
	// Get the chars from the widget
	//
	gchar * sz = NULL;
#ifdef HAVE_GO_MATH_EDITOR_NEW
	m_compact = go_math_editor_get_inline(GO_MATH_EDITOR(m_wText));
	sz = go_math_editor_get_itex(GO_MATH_EDITOR(m_wText));
#else
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	GtkTextIter startIter,endIter;
	gtk_text_buffer_get_start_iter  (buffer,&startIter);
	gtk_text_buffer_get_end_iter    (buffer,&endIter);
	sz = gtk_text_buffer_get_text   (buffer,&startIter,&endIter,TRUE);
#endif

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

#ifdef HAVE_GO_MATH_EDITOR_NEW
	m_windowMain = gtk_dialog_new();
	gtk_button_new_with_label("Close");
	m_wClose = gtk_dialog_add_button(GTK_DIALOG(m_windowMain),"Close",0);
	m_wInsert = gtk_dialog_add_button(GTK_DIALOG(m_windowMain),"Insert",1);
	m_wText = go_math_editor_new();
	gtk_widget_set_size_request (m_wText, 300, -1);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(m_windowMain))),
	                m_wText);
#else
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Latex.ui");

    // Update our member variables with the important widgets that
    // might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Latex"));
	m_wClose = GTK_WIDGET(gtk_builder_get_object(builder, "wClose"));
	m_wInsert =  GTK_WIDGET(gtk_builder_get_object(builder, "wInsert"));
	m_wText = GTK_WIDGET(gtk_builder_get_object(builder, "wTextView"));
#endif

	// localize the strings in our dialog, and set tags for some widgets

	localizeButtonUnderline(m_wInsert, pSS, AP_STRING_ID_DLG_InsertButton);

#ifdef HAVE_GO_MATH_EDITOR_NEW
	localizeButtonUnderline(m_wClose, pSS, AP_STRING_ID_DLG_CloseButton);
#else
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbLatexEquation")), pSS, AP_STRING_ID_DLG_Latex_LatexEquation);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbExample")), pSS, AP_STRING_ID_DLG_Latex_Example);
#endif

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

#ifndef HAVE_GO_MATH_EDITOR_NEW
	g_object_unref(G_OBJECT(builder));
#endif
}

