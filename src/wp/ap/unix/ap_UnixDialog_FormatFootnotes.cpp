/* AbiWord
 * Copyright (C) 2003 Martin Sevior
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_FormatFootnotes.h"
#include "ap_UnixDialog_FormatFootnotes.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_FormatFootnotes::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_FormatFootnotes * p = new AP_UnixDialog_FormatFootnotes(pFactory,id);
	return p;
}

AP_UnixDialog_FormatFootnotes::AP_UnixDialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_FormatFootnotes(pDlgFactory,id)
{
	m_windowMain = 0;
}

AP_UnixDialog_FormatFootnotes::~AP_UnixDialog_FormatFootnotes(void)
{
}

/*****************************************************************/
/***********************************************************************/

void AP_UnixDialog_FormatFootnotes::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);


	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
				 BUTTON_CANCEL, false))
	  {
	  case BUTTON_OK:
	    event_OK () ; break ;
	  case BUTTON_DELETE:
	    event_Delete () ; break ;
	  default:
	    event_Cancel () ; break ;
	  }
	
	abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_FormatFootnotes::event_OK(void)
{
	UT_ASSERT(m_windowMain);
	// get the bookmark name, if any (return cancel if no name given)	
	const XML_Char *mark = NULL;
	if(mark && *mark)
	{
		xxx_UT_DEBUGMSG(("FormatFootnotes: OK pressed, first char 0x%x\n", (UT_uint32)*mark));
		setAnswer(AP_Dialog_FormatFootnotes::a_OK);
	}
	else
	{
		setAnswer(AP_Dialog_FormatFootnotes::a_CANCEL);
	}
}

void AP_UnixDialog_FormatFootnotes::event_Cancel(void)
{
	setAnswer(AP_Dialog_FormatFootnotes::a_CANCEL);
}

void AP_UnixDialog_FormatFootnotes::event_Delete(void)
{
	setAnswer(AP_Dialog_FormatFootnotes::a_DELETE);
}


void  AP_UnixDialog_FormatFootnotes::_constructWindowContents(GtkWidget * container )
{
#if 0
  GtkWidget *label1;
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  label1 = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_Msg).c_str());
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (container), label1, TRUE, FALSE, 3);
#endif

  GtkWidget * NoteBook = gtk_notebook_new ();
  gtk_widget_show (NoteBook);
  gtk_container_add (GTK_CONTAINER (container), NoteBook);

  GtkWidget * vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (NoteBook), vbox1);

  GtkWidget * hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  GtkWidget * Footnote_Style_Label = gtk_label_new ("Footnote Style");
  gtk_widget_show (Footnote_Style_Label);
  gtk_box_pack_start (GTK_BOX (hbox1), Footnote_Style_Label, TRUE, FALSE, 0);

  GtkWidget * optionmenu1 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu1);
  gtk_box_pack_start (GTK_BOX (hbox1), optionmenu1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (optionmenu1), 1);
  GtkWidget * optionmenu1_menu = gtk_menu_new ();
  GtkWidget * glade_menuitem = gtk_menu_item_new_with_label ("1,2,3,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("[1],[2],[3],..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("1),2),3)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("a,b,c,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(a),(b),(c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("a),b),c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("A,B,C..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(A),(B),(C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("A),B),C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("i,ii,iii,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(i),(ii),(iii),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("I,II,III,...");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(I),(II),(III),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), optionmenu1_menu);

  GtkWidget * hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

  GtkWidget * Restart_On_Section = gtk_check_button_new_with_label ("Restart on each Section");
  gtk_widget_show (Restart_On_Section);
  gtk_box_pack_start (GTK_BOX (hbox2), Restart_On_Section, FALSE, FALSE, 0);

  GtkWidget * Restart_On_Page = gtk_check_button_new_with_label ("Restart on each Page");
  gtk_widget_show (Restart_On_Page);
  gtk_box_pack_start (GTK_BOX (hbox2), Restart_On_Page, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Restart_On_Page), 2);

  GtkWidget * hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, 0);

  GtkWidget * Initial_Val_lab = gtk_label_new ("Initial footnote value");
  gtk_widget_show (Initial_Val_lab);
  gtk_box_pack_start (GTK_BOX (hbox3), Initial_Val_lab, TRUE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (Initial_Val_lab), 9, 0);

  GtkWidget * Initial_Value_Footnote = gtk_entry_new ();
  gtk_widget_show (Initial_Value_Footnote);
  gtk_box_pack_end (GTK_BOX (hbox3), Initial_Value_Footnote, FALSE, FALSE, 25);
  gtk_widget_set_usize (Initial_Value_Footnote, 90, -2);
  gtk_entry_set_text (GTK_ENTRY (Initial_Value_Footnote), "1");

  GtkWidget * Footnotes_tab = gtk_label_new ("Format Footnotes");
  gtk_widget_show (Footnotes_tab);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (NoteBook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (NoteBook), 0), Footnotes_tab);

  GtkWidget * vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (NoteBook), vbox2);

  GtkWidget * hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox4, TRUE, TRUE, 0);

  GtkWidget * Endnote_Style = gtk_label_new ("Endnote Style");
  gtk_widget_show (Endnote_Style);
  gtk_box_pack_start (GTK_BOX (hbox4), Endnote_Style, TRUE, FALSE, 0);

  GtkWidget * optionmenu2 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu2);
  gtk_box_pack_start (GTK_BOX (hbox4), optionmenu2, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (optionmenu2), 1);

  GtkWidget * optionmenu2_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("1,2,3,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("[1],[2],[3],..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("1),2),3)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("a,b,c,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(a),(b),(c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("a),b),c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("A,B,C..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(A),(B),(C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("A),B),C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("i,ii,iii,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(i),(ii),(iii),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("I,II,III,...");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("(I),(II),(III),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), optionmenu2_menu);

  GtkWidget * hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox5, TRUE, TRUE, 0);

  GtkWidget * Place_at_end_of_Section = gtk_check_button_new_with_label ("Place at end of Section");
  gtk_widget_show (Place_at_end_of_Section);
  gtk_box_pack_start (GTK_BOX (hbox5), Place_at_end_of_Section, FALSE, FALSE, 0);

  GtkWidget * Place_At_End_of_doc = gtk_check_button_new_with_label ("Place at end of Document");
  gtk_widget_show (Place_At_End_of_doc);
  gtk_box_pack_start (GTK_BOX (hbox5), Place_At_End_of_doc, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Place_At_End_of_doc), 2);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (Place_At_End_of_doc), TRUE);

  GtkWidget * hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox6);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox6, TRUE, TRUE, 0);

  GtkWidget * Restart_on_Section = gtk_check_button_new_with_label ("Restart on each Section");
  gtk_widget_show (Restart_on_Section);
  gtk_box_pack_start (GTK_BOX (hbox6), Restart_on_Section, FALSE, FALSE, 0);

  GtkWidget * entry2 = gtk_entry_new ();
  gtk_widget_show (entry2);
  gtk_box_pack_end (GTK_BOX (hbox6), entry2, FALSE, FALSE, 0);
  gtk_widget_set_usize (entry2, 77, -2);
  gtk_entry_set_text (GTK_ENTRY (entry2), "1");

  GtkWidget * Initial_Endnote_val = gtk_label_new ("Initial Endnote value");
  gtk_widget_show (Initial_Endnote_val);
  gtk_box_pack_start (GTK_BOX (hbox6), Initial_Endnote_val, FALSE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (Initial_Endnote_val), 9, 0);

  GtkWidget * Endnotes_tab = gtk_label_new ("Format Endnotes");
  gtk_widget_show (Endnotes_tab);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (NoteBook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (NoteBook), 1), Endnotes_tab);


}

GtkWidget*  AP_UnixDialog_FormatFootnotes::_constructWindow(void)
{
  GtkWidget *frame1;
  GtkWidget *vbox2;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_windowMain = abiDialogNew("format footnotes dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_Title).c_str());

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_windowMain)->vbox), frame1);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 4);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame1), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  _constructWindowContents ( vbox2 );

  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);
  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_DELETE, BUTTON_DELETE);
  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_OK, BUTTON_OK);

  return m_windowMain;
}
