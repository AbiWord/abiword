/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_dialogHelper.h"
#include "xap_UnixDlg_Language.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "gr_UnixGraphics.h"

/*****************************************************************/
XAP_Dialog * XAP_UnixDialog_Language::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_UnixDialog_Language * p = new XAP_UnixDialog_Language(pFactory,id);
	return p;
}

XAP_UnixDialog_Language::XAP_UnixDialog_Language(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_Language(pDlgFactory,id)
{
	m_pLanguageList = NULL;
}

XAP_UnixDialog_Language::~XAP_UnixDialog_Language(void)
{
}


static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_Dialog_Language::tAnswer * answer)
{
	*answer = XAP_Dialog_Language::a_CANCEL;
	gtk_main_quit();
}

static void s_ok_clicked(GtkWidget * /* widget */,
						 XAP_Dialog_Language::tAnswer * answer)
{	*answer = XAP_Dialog_Language::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * /* widget */,
							 XAP_Dialog_Language::tAnswer * answer)
{
	*answer = XAP_Dialog_Language::a_CANCEL;
	gtk_main_quit();
}

GtkWidget * XAP_UnixDialog_Language::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowLangSelection;
	GtkWidget *vboxMain;
	GtkWidget *vboxOuter;

	GtkWidget *fixedButtons;
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	windowLangSelection = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (windowLangSelection), "windowLangSelection", windowLangSelection);
	gtk_window_set_title (GTK_WINDOW (windowLangSelection), pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangTitle));
	gtk_window_set_policy (GTK_WINDOW (windowLangSelection), FALSE, TRUE, FALSE);

	vboxOuter = gtk_vbox_new (FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (windowLangSelection), "vboxOuter", vboxOuter);
	gtk_widget_show (vboxOuter);
	gtk_container_add (GTK_CONTAINER (windowLangSelection), vboxOuter);

	vboxMain = constructWindowContents(GTK_OBJECT (windowLangSelection));
	gtk_box_pack_start (GTK_BOX (vboxOuter), vboxMain, TRUE, TRUE, 0);

	fixedButtons = gtk_fixed_new ();
	gtk_object_set_data (GTK_OBJECT (windowLangSelection), "fixedButtons", fixedButtons);
	gtk_widget_show (fixedButtons);
	gtk_box_pack_start (GTK_BOX (vboxOuter), fixedButtons, FALSE, TRUE, 0);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_object_set_data (GTK_OBJECT (windowLangSelection), "buttonOK", buttonOK);
	gtk_widget_show (buttonOK);
	gtk_fixed_put (GTK_FIXED (fixedButtons), buttonOK, 279, 0);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_object_set_data (GTK_OBJECT (windowLangSelection), "buttonCancel", buttonCancel);
	gtk_widget_show (buttonCancel);
	gtk_fixed_put (GTK_FIXED (fixedButtons), buttonCancel, 374, 6);

	gtk_signal_connect_after(GTK_OBJECT(windowLangSelection),
							  "destroy",
							  NULL,
							  NULL);
	gtk_signal_connect(GTK_OBJECT(windowLangSelection),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) &m_answer);

	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) &m_answer);
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) &m_answer);

	return windowLangSelection;
}

// Glade generated dialog, using fixed widgets to closely match
// the Windows layout, with some changes for color selector
GtkWidget * XAP_UnixDialog_Language::constructWindowContents(GtkObject *parent)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
  GtkWidget *vboxMain;
  GtkWidget *frame3;
  GtkWidget *scrolledwindow1;
  GtkWidget *viewport1;
  GtkWidget *langlist;

  vboxMain = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vboxMain);
  gtk_object_set_data_full (GTK_OBJECT (parent), "vboxMain", vboxMain,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vboxMain);
  gtk_container_add (GTK_CONTAINER (parent), vboxMain);

  frame3 = gtk_frame_new (pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangLabel));
  gtk_widget_ref (frame3);
  gtk_object_set_data_full (GTK_OBJECT (parent), "frame3", frame3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vboxMain), frame3, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame3), 4);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (parent), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (frame3), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_ref (viewport1);
  gtk_object_set_data_full (GTK_OBJECT (parent), "viewport1", viewport1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

  langlist = gtk_clist_new (1);
  gtk_widget_ref (langlist);
  gtk_object_set_data_full (GTK_OBJECT (parent), "langlist", langlist,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (langlist);
  gtk_container_add (GTK_CONTAINER (viewport1), langlist);

	// save out to members for callback and class access
	m_pLanguageList = langlist;

	GTK_WIDGET_SET_FLAGS(langlist, GTK_CAN_FOCUS);

	return vboxMain;
}

void XAP_UnixDialog_Language::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(m_pApp);

	// this is used below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};

	// build the dialog
	GtkWidget * cf = constructWindow();
	UT_ASSERT(cf);
	connectFocus(GTK_WIDGET(cf),pFrame);

	// fill the listbox
	gtk_clist_freeze(GTK_CLIST(m_pLanguageList));
	gtk_clist_clear(GTK_CLIST(m_pLanguageList));
	//UT_DEBUGMSG(("langlist count %d\n", m_pLangTable->getCount()));
	for (UT_uint32 k = 0; k < m_iLangCount; k++)
	{
		text[0] = (gchar *) m_ppLanguages[k];
		//text[1] = (gchar *) m_pLangTable->getNthProperty(k);
		//UT_DEBUGMSG(("langlist k=%d, lang=%s, prop=%s\n", k,text[0], text[1]));
		gtk_clist_append(GTK_CLIST(m_pLanguageList), text);
	}
	
	gtk_clist_thaw(GTK_CLIST(m_pLanguageList));

	// Set the defaults in the list boxes according to dialog data
	gint foundAt = 0;

	// is this safe with an XML_Char * string?
	foundAt = searchCList(GTK_CLIST(m_pLanguageList), (char *) m_pLanguage);

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_pLanguageList), foundAt, 0);
	}
	
	// get top level window and its GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	// center it
	centerDialog(parent, GTK_WIDGET(cf));
	
	// Run the dialog
	gtk_widget_show(GTK_WIDGET(cf));
	gtk_grab_add(GTK_WIDGET(cf));

	gtk_main();

	if (m_answer == XAP_Dialog_Language::a_OK)
	{
		GList * selectedRow = NULL;
		gint rowNumber = 0;
		
		selectedRow = GTK_CLIST(m_pLanguageList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_pLanguageList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);
			if (!m_pLanguage || UT_stricmp(m_pLanguage, text[0]))
			{
				_setLanguage((XML_Char*)text[0]);
				m_bChangedLanguage = true;
			}
		}
		
	}

	if(cf && GTK_IS_WIDGET(cf))
	  gtk_widget_destroy (GTK_WIDGET(cf));

    gtk_widget_pop_visual();
    gtk_widget_pop_colormap();
}

