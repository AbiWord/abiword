/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2005 Hubert Figuiere
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
	: AP_Dialog_WordCount(pDlgFactory,id),
	  m_bDestroy_says_stopupdating(false),
	  m_bAutoUpdate_happening_now(false)
{
}

AP_UnixDialog_WordCount::~AP_UnixDialog_WordCount(void)
{
}

void  AP_UnixDialog_WordCount::activate(void)
{
	// FIXME move to XP
	UT_ASSERT (m_windowMain);
	
	ConstructWindowName();
	setWidgetLabel(DIALOG_WID, std::string(m_WindowName));
	setCountFromActiveFrame ();
	updateDialogData();
	gdk_window_raise (gtk_widget_get_window(m_windowMain));
}

void AP_UnixDialog_WordCount::s_response(GtkWidget * wid, gint id,
										 AP_UnixDialog_WordCount * /*me*/ )
{
	if (id == GTK_RESPONSE_CLOSE)
	{
		abiDestroyWidget(wid);
	}
}

void AP_UnixDialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	constructDialog();
	UT_return_if_fail(m_windowMain);

	updateDialogData();

	abiSetupModelessDialog(GTK_DIALOG(m_windowMain), pFrame, this, 
						   GTK_RESPONSE_CLOSE);
	gtk_widget_show(m_windowMain);

	// Now construct the timer for auto-updating
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this);
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
		pDialog->updateDialogData ();
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

void AP_UnixDialog_WordCount::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	// FIXME put that in XP code
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	setWidgetLabel(DIALOG_WID, std::string(m_WindowName));
	setCountFromActiveFrame();
	updateDialogData();
}

void AP_UnixDialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;
	m_pAutoUpdateWC->stop();
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	modeless_cleanup();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
	DELETEP(m_pAutoUpdateWC);
}

/*****************************************************************/

XAP_Widget *AP_UnixDialog_WordCount::getWidget(xap_widget_id wid)
{
	switch(wid) {
	case DIALOG_WID:
		return new XAP_UnixWidget(m_windowMain);
		break;
	case CLOSE_BTN_WID:
		return new XAP_UnixWidget(NULL);
		break;
	case TITLE_LBL_WID:
	  {
		XAP_UnixWidget *w = new XAP_UnixWidget(m_labelTitle);
		w->setData(m_labelTitleMarkupFormat);
		return w;
		break;
	  }
	case PAGES_LBL_WID:
		return new XAP_UnixWidget(m_labelLabelPgCount);
		break;
	case PAGES_VAL_WID:
		return new XAP_UnixWidget(m_labelPgCount);
		break;
	case LINES_LBL_WID:
		return new XAP_UnixWidget(m_labelLabelLCount);
		break;
	case LINES_VAL_WID:
		return new XAP_UnixWidget(m_labelLCount);
		break;
	case CHARNSP_LBL_WID:
		return new XAP_UnixWidget(m_labelLabelCNCount);
		break;
	case CHARNSP_VAL_WID:
		return new XAP_UnixWidget(m_labelCNCount);
		break;
	case CHARSP_LBL_WID:
		return new XAP_UnixWidget(m_labelLabelCCount);
		break;
	case CHARSP_VAL_WID:
		return new XAP_UnixWidget(m_labelCCount);
		break;
	case PARA_LBL_WID:
		return new XAP_UnixWidget(m_labelLabelPCount);
		break;
	case PARA_VAL_WID:
		return new XAP_UnixWidget(m_labelPCount);
		break;
	case WORDS_LBL_WID:
		return new XAP_UnixWidget(m_labelLabelWCount);
		break;
	case WORDS_VAL_WID:
		return new XAP_UnixWidget(m_labelWCount);
		break;
	case WORDSNF_LBL_WID:
		return new XAP_UnixWidget(m_labelWNFCount);
		break;
	case WORDSNF_VAL_WID:
		return new XAP_UnixWidget(m_labelWNoFootnotesCount);
		break;		
	default:
		UT_ASSERT(UT_NOT_REACHED);
	}
	return NULL;
}

void AP_UnixDialog_WordCount::constructDialog(void)
{	
	GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_WordCount.ui");

	m_windowMain   = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_WordCount"));
	m_labelWCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbWordsVal"));
	m_labelWNoFootnotesCount = GTK_WIDGET(gtk_builder_get_object(builder, "lbWordsNoFootnotesVal"));
	m_labelPCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbParagraphsVal"));
	m_labelCCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbCharactersSpacesVal"));
	m_labelCNCount = GTK_WIDGET(gtk_builder_get_object(builder, "lbCharactersNoSpacesVal"));
	m_labelLCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbLinesVal"));
	m_labelPgCount = GTK_WIDGET(gtk_builder_get_object(builder, "lbPagesVal"));
	m_labelTitle   = GTK_WIDGET(gtk_builder_get_object(builder, "lbTitle"));
	if(gtk_label_get_use_markup(GTK_LABEL(m_labelTitle))) {
		m_labelTitleMarkupFormat = gtk_label_get_label(GTK_LABEL(m_labelTitle));
	}

	m_labelLabelWCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbWords"));
 	m_labelWNFCount     = GTK_WIDGET(gtk_builder_get_object(builder, "lbWordsNoFootnotes"));
	m_labelLabelPCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbParagraphs"));
	m_labelLabelCCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbCharactersSpaces"));
	m_labelLabelCNCount = GTK_WIDGET(gtk_builder_get_object(builder, "lbCharactersNoSpaces"));
	m_labelLabelLCount  = GTK_WIDGET(gtk_builder_get_object(builder, "lbLines"));
	m_labelLabelPgCount = GTK_WIDGET(gtk_builder_get_object(builder, "lbPages"));

	localizeDialog();

	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW(m_windowMain), m_WindowName);

   	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response), this);
	g_signal_connect(G_OBJECT(m_windowMain), "destroy",
					   G_CALLBACK(s_destroy_clicked),
					   reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_windowMain), "delete_event",
					   G_CALLBACK(s_delete_clicked),
					   reinterpret_cast<gpointer>(this));

	gtk_widget_show_all (m_windowMain);
	g_object_unref(G_OBJECT(builder));
}

