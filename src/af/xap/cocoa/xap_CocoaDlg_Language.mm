/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2003 Hubert Figuiere
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
/* $Id */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_CocoaDialog_Utilities.h"
#include "xap_CocoaDlg_Language.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "gr_CocoaGraphics.h"


class XAP_LanguageList_Proxy: public XAP_GenericListChooser_Proxy
{
public:
	XAP_LanguageList_Proxy (XAP_CocoaDialog_Language* dlg)
		: XAP_GenericListChooser_Proxy(),
			m_dlg(dlg)
	{
	};
	virtual void okAction ()
	{
		m_dlg->okAction();
	};
	virtual void cancelAction ()
	{
		m_dlg->cancelAction();
	};
	virtual void selectAction ()
	{
	};
private:
	XAP_CocoaDialog_Language*	m_dlg;
};


/*****************************************************************/
XAP_Dialog * XAP_CocoaDialog_Language::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Language * p = new XAP_CocoaDialog_Language(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Language::XAP_CocoaDialog_Language(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id dlgid)
	: XAP_Dialog_Language(pDlgFactory,dlgid),
		m_dataSource(nil),
		m_dlg(nil)
{
}

XAP_CocoaDialog_Language::~XAP_CocoaDialog_Language(void)
{
}


void XAP_CocoaDialog_Language::okAction(void)
{	
	m_answer = XAP_Dialog_Language::a_OK;
	[NSApp stopModal];
}


void XAP_CocoaDialog_Language::cancelAction(void)
{
	m_answer = XAP_Dialog_Language::a_CANCEL;
	[NSApp stopModal];
}

#if 0
GtkWidget * XAP_CocoaDialog_Language::constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget *windowLangSelection;
	GtkWidget *vboxMain;
	GtkWidget *vboxOuter;

	GtkWidget *fixedButtons;
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;

	windowLangSelection = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_title (GTK_WINDOW (windowLangSelection), pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangTitle));
	gtk_window_set_policy (GTK_WINDOW (windowLangSelection), FALSE, TRUE, FALSE);

	vboxOuter = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vboxOuter);
	gtk_container_add (GTK_CONTAINER (windowLangSelection), vboxOuter);

	vboxMain = constructWindowContents(G_OBJECT (windowLangSelection));
	gtk_box_pack_start (GTK_BOX (vboxOuter), vboxMain, TRUE, TRUE, 0);

	fixedButtons = gtk_hbutton_box_new ();
	gtk_widget_show (fixedButtons);
	gtk_box_pack_start (GTK_BOX (vboxOuter), fixedButtons, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (fixedButtons), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (fixedButtons), 10);
	gtk_button_box_set_child_size (GTK_BUTTON_BOX (fixedButtons), 85, 24);

	buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_show (buttonOK);
	gtk_container_add (GTK_CONTAINER (fixedButtons), buttonOK);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (buttonOK);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (fixedButtons), buttonCancel);

	g_signal_connect_after(G_OBJECT(windowLangSelection),
							  "destroy",
							  NULL,
							  NULL);
	g_signal_connect(G_OBJECT(windowLangSelection),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) &m_answer);

	g_signal_connect(G_OBJECT(buttonOK),
					   "clicked",
					   G_CALLBACK(s_ok_clicked),
					   (gpointer) &m_answer);
	g_signal_connect(G_OBJECT(buttonCancel),
					   "clicked",
					   G_CALLBACK(s_cancel_clicked),
					   (gpointer) &m_answer);

	return windowLangSelection;
}

// Glade generated dialog, using fixed widgets to closely match
// the Windows layout, with some changes for color selector
GtkWidget * XAP_CocoaDialog_Language::constructWindowContents(GObject *parent)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
  GtkWidget *vboxMain;
  GtkWidget *frame3;
  GtkWidget *scrolledwindow1;
  GtkWidget *viewport1;
  GtkWidget *langlist;

  vboxMain = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxMain);
  gtk_container_add (GTK_CONTAINER (parent), vboxMain);

  frame3 = gtk_frame_new (pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangLabel));
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vboxMain), frame3, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame3), 4);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (frame3), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

  langlist = gtk_clist_new (1);
  gtk_widget_show (langlist);
  gtk_container_add (GTK_CONTAINER (viewport1), langlist);

	// save out to members for callback and class access
	m_pLanguageList = langlist;

	GTK_WIDGET_SET_FLAGS(langlist, GTK_CAN_FOCUS);

	return vboxMain;
}
#endif

void XAP_CocoaDialog_Language::runModal(XAP_Frame * pFrame)
{
	NSWindow* window;
	m_dlg = [XAP_GenericListChooser_Controller loadFromNib];
	UT_ASSERT(m_pApp);
	XAP_LanguageList_Proxy proxy(this);
	[m_dlg setXAPProxy:&proxy];
	m_dataSource = [[XAP_StringListDataSource alloc] init];
	
	window = [m_dlg window];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	[m_dlg setTitle:[NSString stringWithUTF8String:pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_LangTitle).c_str()]];
	[m_dlg setLabel:[NSString stringWithUTF8String:pSS->getValueUTF8(XAP_STRING_ID_DLG_ULANG_LangLabel).c_str()]];

	for (UT_uint32 k = 0; k < m_iLangCount; k++)
	{
		[m_dataSource addString:[NSString stringWithUTF8String:m_ppLanguages[k]]];
	}
	[m_dlg setDataSource:m_dataSource];
	
	// Set the defaults in the list boxes according to dialog data
	int foundAt = 0;

	// is this safe with an XML_Char * string?
	foundAt = [m_dataSource rowWithCString:m_pLanguage];
	if (foundAt >= 0)
	{
		[m_dlg setSelected:foundAt];
	}
	
	[NSApp runModalForWindow:window];

	if (m_answer == XAP_Dialog_Language::a_OK)
	{
		int row = [m_dlg selected];
		NSString* str = [[m_dataSource array] objectAtIndex:row];
		const char* 	utf8str = [str UTF8String];
		if (!m_pLanguage || UT_stricmp(m_pLanguage, utf8str))
		{
			_setLanguage((XML_Char*)utf8str);
			m_bChangedLanguage = true;
		}
	}
	[m_dlg close];

	[m_dataSource release];

	m_dlg = nil;
}

