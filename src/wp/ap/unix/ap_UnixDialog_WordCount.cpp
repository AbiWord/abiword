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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_UnixDialog_WordCount.h"

/*****************************************************************/

#define	WIDGET_ID_TAG_KEY "id"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_WordCount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_UnixDialog_WordCount * p = new AP_UnixDialog_WordCount(pFactory,id);
	return p;
}

AP_UnixDialog_WordCount::AP_UnixDialog_WordCount(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_buttonOK = NULL;
	m_wContent = NULL;

}

AP_UnixDialog_WordCount::~AP_UnixDialog_WordCount(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget, AP_UnixDialog_WordCount * dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
}


static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 AP_UnixDialog_WordCount * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
}

/*****************************************************************/

void AP_UnixDialog_WordCount::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();
	
	// To center the dialog, we need the frame of its parent.
	XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(pUnixFrame);
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// Run into the GTK event loop for this window.
	gtk_main();

	gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_WordCount::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_WordCount::a_OK;
	gtk_main_quit();
}

void AP_UnixDialog_WordCount::event_WindowDelete(void)
{
	m_answer = AP_Dialog_WordCount::a_CANCEL;	
	gtk_main_quit();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_WordCount::_constructWindow(void)
{

	GtkWidget * windowWordCount;
	GtkWidget * buttonOK;
	GtkWidget * vbox;
	GtkWidget * hbuttonboxWordCount;
	GtkWidget * table;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowWordCount = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowWordCount), "windowWordCount", windowWordCount);
//	gtk_widget_set_usize (windowWordCount, 100, 60);
	gtk_container_set_border_width(GTK_CONTAINER (windowWordCount), 20);
	gtk_window_set_title (GTK_WINDOW (windowWordCount), pSS->getValue(AP_STRING_ID_DLG_WordCount_WordCountTitle));
	gtk_window_set_policy (GTK_WINDOW (windowWordCount), FALSE, FALSE, FALSE);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (windowWordCount), vbox);

	table = _constructWindowContents ();
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	hbuttonboxWordCount = gtk_hbutton_box_new ();
	gtk_object_set_data (GTK_OBJECT (windowWordCount), "hbuttonboxWordCount", hbuttonboxWordCount);
	gtk_widget_show (hbuttonboxWordCount);
	gtk_box_pack_start (GTK_BOX (vbox), hbuttonboxWordCount, FALSE, TRUE, 0);

// 	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonboxWordCount), GTK_BUTTONBOX_END);
// 	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonboxWordCount), 10);
// 	gtk_button_box_set_child_size (GTK_BUTTON_BOX (hbuttonboxWordCount), 85, 24);
// 	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (hbuttonboxWordCount), 0, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_object_set_data (GTK_OBJECT (windowWordCount), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (hbuttonboxWordCount), buttonOK);
	gtk_widget_grab_focus (buttonOK);
	gtk_widget_grab_default (buttonOK);

	// the control buttons
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	// the catch-alls
	
	gtk_signal_connect_after(GTK_OBJECT(windowWordCount),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowWordCount),
							 "destroy",
							 NULL,
							 NULL);

	gtk_widget_show_all (vbox);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	m_windowMain = windowWordCount;
	m_buttonOK = buttonOK;
	
	return windowWordCount;
}

void AP_UnixDialog_WordCount::_populateWindowData(void)
{
}

GtkWidget * AP_UnixDialog_WordCount::_constructWindowContents(void)
{
	GtkWidget * labelWCount;
	GtkWidget * labelWords;
	GtkWidget * labelPCount;
	GtkWidget * labelPara;
	GtkWidget * labelCCount;
	GtkWidget * labelCNCount;
	GtkWidget * labelCharNo;	
	GtkWidget * labelChar;
	GtkWidget * labelLCount;	
	GtkWidget * labelLine;
	GtkWidget * labelPgCount;	
	GtkWidget * labelPage;
	GtkWidget * dataTable;
	XML_Char * unixstr = NULL;	// used for conversions
	char *tmp;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	dataTable = gtk_table_new(7, 2, TRUE);	
	gtk_table_set_col_spacings (GTK_TABLE (dataTable), 12);

	// The figures
	tmp = g_strdup_printf ("%d", m_count.word);
	labelWCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), labelWCount, 1, 2, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelWCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.para);
	labelPCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), labelPCount, 1, 2, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelPCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.ch_sp);
	labelCCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), labelCCount, 1, 2, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelCCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.ch_no);
	labelCNCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), labelCNCount, 1, 2, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelCNCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.line);
	labelLCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), labelLCount, 1, 2, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelLCount), 1.0, 0.5);
	tmp = g_strdup_printf ("%d", m_count.page);	
	labelPgCount = gtk_label_new (tmp);
	g_free (tmp);
	gtk_table_attach (GTK_TABLE (dataTable), labelPgCount, 1, 2, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelPgCount), 1.0, 0.5);
	
	// The labels
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Words));	
	labelWords = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_label_set_justify (GTK_LABEL (labelWords), GTK_JUSTIFY_LEFT);
	gtk_label_set_justify (GTK_LABEL (labelWords), GTK_JUSTIFY_LEFT);
	gtk_table_attach (GTK_TABLE (dataTable), labelWords, 0, 1, 0, 1,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelWords), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Paragraphs));	
	labelPara = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelPara, 0, 1, 1, 2,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelPara), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_Sp));	
	labelChar = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelChar, 0, 1, 2, 3,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelChar), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Characters_No));	
	labelCharNo = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelCharNo, 0, 1, 3, 4,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelCharNo), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Lines));	
	labelLine = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelLine, 0, 1, 4, 5,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelLine), 0, 0.5);
	UT_XML_cloneNoAmpersands(unixstr,  pSS->getValue(AP_STRING_ID_DLG_WordCount_Pages));	
	labelPage = gtk_label_new (unixstr);
	FREEP(unixstr);
	gtk_table_attach (GTK_TABLE (dataTable), labelPage, 0, 1, 5, 6,
					  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (labelPage), 0, 0.5);

	gtk_widget_show_all (dataTable);
	m_wContent = dataTable;
	return (dataTable);
}
