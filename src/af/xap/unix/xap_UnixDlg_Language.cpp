/* AbiSource Application Framework
 * Copyright (C) 1998 -2002 AbiSource, Inc.
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
#include "xap_UnixDialogHelper.h"
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
  : XAP_Dialog_Language(pDlgFactory,id), m_pLanguageList ( NULL )
{
}

XAP_UnixDialog_Language::~XAP_UnixDialog_Language(void)
{
}

GtkWidget * XAP_UnixDialog_Language::constructWindow(void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  GtkWidget *windowLangSelection;
  GtkWidget *vboxMain;
  GtkWidget *vboxOuter;
  
  windowLangSelection = abiDialogNew ( TRUE, pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangTitle) ) ;
  
  vboxOuter = GTK_DIALOG(windowLangSelection)->vbox ;
  
  vboxMain = constructWindowContents(vboxOuter);
  
  abiAddStockButton ( GTK_DIALOG(windowLangSelection),
					  GTK_STOCK_OK, BUTTON_OK ) ;
  abiAddStockButton ( GTK_DIALOG(windowLangSelection),
					  GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
  
  return windowLangSelection;
}

// Glade generated dialog, using fixed widgets to closely match
// the Windows layout, with some changes for color selector
GtkWidget * XAP_UnixDialog_Language::constructWindowContents(GtkWidget *parent)
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

void XAP_UnixDialog_Language::runModal(XAP_Frame * pFrame)
{
  // this is used below to grab pointers to
  // strings inside list elements
  gchar * text[2] = {NULL, NULL};
  
  // build the dialog
  GtkWidget * cf = constructWindow();
  
  // make the window big
  gtk_widget_set_usize(cf, 255, 350);

  // fill the listbox
  gtk_clist_freeze(GTK_CLIST(m_pLanguageList));
  gtk_clist_clear(GTK_CLIST(m_pLanguageList));

  for (UT_uint32 k = 0; k < m_iLangCount; k++)
    {
      text[0] = (gchar *) m_ppLanguages[k];
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
  
  switch ( abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, BUTTON_CANCEL, false ) )
    {
    case BUTTON_OK:
      m_answer = XAP_Dialog_Language::a_OK; break ;
    default:
      m_answer = XAP_Dialog_Language::a_CANCEL; break ;
    }
  
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
  
  abiDestroyWidget(cf);
}

