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
	m_windowMain = NULL;
	m_wButtonApply = NULL;

	m_wFootnotesStyleMenu = NULL;
	m_wFootnotesRestartOnSection = NULL;
	m_wFootnotesRestartOnPage = NULL;
	m_wFootnotesInitialValText = NULL;
	m_wFootnoteSpin = NULL;
	m_oFootnoteSpinAdj = NULL;

	m_wEndnotesStyleMenu = NULL;
	m_wEndnotesRestartOnSection = NULL;
	m_wEndnotesPlaceEndOfDoc = NULL;
	m_wEndnotesPlaceEndOfSec = NULL;
	m_wEndnotesInitialValText = NULL;
	m_wEndnoteSpin = NULL;
	m_oEndnoteSpinAdj = NULL;

	m_wEnd123 = NULL;
	m_wEnd123Brack = NULL;
	m_wEnd123Paren = NULL;
	m_wEnd123OpenParen = NULL;
	m_wEndLower = NULL;
	m_wEndLowerParen = NULL;
	m_wEndLowerOpenParen = NULL;
	m_wEndUpper = NULL;
	m_wEndUpperParen = NULL;
	m_wEndUpperOpenParen = NULL;
	m_wEndRomanLower = NULL;
	m_wEndRomanLowerParen = NULL;
	m_wEndRomanUpper = NULL;
	m_wEndRomanUpperParen = NULL;

	m_wFoot123 = NULL;
	m_wFoot123Brack = NULL;
	m_wFoot123Paren  = NULL;
	m_wFoot123OpenParen  = NULL;
	m_wFootLower  = NULL;
	m_wFootLowerParen = NULL;
	m_wFootLowerOpenParen = NULL;
	m_wFootUpper = NULL;
	m_wFootUpperParen = NULL;
	m_wFootUpperOpenParen = NULL;
	m_wFootRomanLower = NULL;
	m_wFootRomanLowerParen = NULL;
	m_wFootRomanUpper = NULL;
	m_wFootRomanUpperParen = NULL;

	m_FootnoteSpinHanderID= 0;
	m_EndnoteSpinHanderID =0;
	m_FootRestartPageID = 0;
	m_FootRestartSectionID = 0;
	m_EndPlaceEndofSectionID = 0;
	m_EndPlaceEndofDocID = 0;
	m_EndRestartSectionID = 0;
	m_FootStyleID = 0;
	m_EndStyleID = 0;
};


AP_UnixDialog_FormatFootnotes::~AP_UnixDialog_FormatFootnotes(void)
{
}
/****************************************************************/
/* Static Callbacks for event handling */
/****************************************************************/

static void s_menu_item_activate(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_MenuChange(widget);
}

static void s_FootInitial(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);
	UT_DEBUGMSG(("Initial Footnote Val changed \n")); 
	dlg->event_FootInitialValueChange();
}

static void s_EndInitial(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);
	UT_DEBUGMSG(("Initial Endnote Val changed \n")); 

	dlg->event_EndInitialValueChange();
}

static void s_FootRestartPage(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_DEBUGMSG(("Restart Footnotes on each page \n")); 

	dlg->event_FootRestartPage();
}


static void s_FootRestartSection(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_DEBUGMSG(("Restart Footnotes on each Section \n")); 
	dlg->event_FootRestartSection();

}


static void s_EndPlaceEndSection(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_DEBUGMSG(("Place endnotes at end of Section \n")); 

	dlg->event_EndPlaceEndSection();
}

static void s_EndPlaceEndDoc(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_DEBUGMSG(("Place endnotes at end of Document \n")); 
	dlg->event_EndPlaceEndDoc();
}

static void s_EndRestartSection(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{	
	UT_DEBUGMSG(("Restart Endnotes at each section \n")); 
	dlg->event_EndRestartSection();

}



/*****************************************************************/
/***********************************************************************/

void AP_UnixDialog_FormatFootnotes::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	setFrame(pFrame);
	setInitialValues();
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	refreshVals();


	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
							 BUTTON_CANCEL, false))
	  {
	  case BUTTON_DELETE:
	    event_Delete () ; break ;
	  case BUTTON_OK:
		  event_Apply(); break;
	  default:
	    event_Cancel () ; break ;
	  }
	
	abiDestroyWidget ( mainWindow ) ;
}

/*	g_signal_connect(G_OBJECT(m_wButtonApply),
					 "clicked",
					 G_CALLBACK(s_Apply),
					 static_cast<gpointer>(this)); */

void AP_UnixDialog_FormatFootnotes::event_Apply(void)
{

// Apply the current settings to the document
	updateDocWithValues();
}

void AP_UnixDialog_FormatFootnotes::event_FootInitialValueChange(void)
{
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wFootnoteSpin));
	UT_DEBUGMSG(("SEVIOR: spin Height %d old value = %d \n",val,getFootnoteVal()));
	if (val == getFootnoteVal())
		return;
	setFootnoteVal(val);
	refreshVals();
}


void AP_UnixDialog_FormatFootnotes::event_EndInitialValueChange(void)
{
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wEndnoteSpin));
	if (val == getEndnoteVal())
		return;
	setEndnoteVal(val);
	refreshVals();
}



void AP_UnixDialog_FormatFootnotes::event_FootRestartPage(void)
{
	gboolean bRestart = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wFootnotesRestartOnPage));
	if(bRestart == TRUE)
	{
		setRestartFootnoteOnPage(true);
		setRestartFootnoteOnSection(false);
	}
	else
	{
		setRestartFootnoteOnPage(false);
	}
	refreshVals();
}



void AP_UnixDialog_FormatFootnotes::event_FootRestartSection(void)
{
	gboolean bRestart = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wFootnotesRestartOnSection));
	if(bRestart == TRUE)
	{
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(true);
	}
	else
	{
		setRestartFootnoteOnSection(false);
	}
	refreshVals();

}



void AP_UnixDialog_FormatFootnotes::event_EndPlaceEndSection(void)
{
	gboolean bRestart = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wEndnotesPlaceEndOfSec));
	if(bRestart == TRUE)
	{
		setPlaceAtSecEnd(true);
		setPlaceAtDocEnd(false);
	}
	else
	{
		setPlaceAtSecEnd(false);
		setPlaceAtDocEnd(true);
	}
	refreshVals();

}


void AP_UnixDialog_FormatFootnotes::event_EndPlaceEndDoc(void)
{
	gboolean bRestart = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wEndnotesPlaceEndOfDoc));
	if(bRestart == TRUE)
	{
		setPlaceAtSecEnd(false);
		setPlaceAtDocEnd(true);
	}
	else
	{
		setPlaceAtSecEnd(true);
		setPlaceAtDocEnd(false);
	}
	refreshVals();
}


void AP_UnixDialog_FormatFootnotes::event_EndRestartSection(void)
{
	gboolean bRestart = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wEndnotesRestartOnSection));
	if(bRestart == TRUE)
	{
		setRestartEndnoteOnSection(true);
	}
	else
	{
		setRestartEndnoteOnSection(false);
	}
}

void AP_UnixDialog_FormatFootnotes::event_MenuChange(GtkWidget * widget)
{
	UT_ASSERT(m_windowMain);
	UT_ASSERT(widget);
	bool bIsFootnote = true;
	UT_DEBUGMSG(("event Menu Change \n"));
	FootnoteType iType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
	if (widget == m_wEnd123)
	{
		iType = FOOTNOTE_TYPE_NUMERIC;
		bIsFootnote = false;
	}
	else if (widget == m_wEnd123Brack )
	{
		iType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
		bIsFootnote = false;
	}
	else if (widget ==m_wEnd123Paren  )
	{
		iType =FOOTNOTE_TYPE_NUMERIC_PAREN ;
		bIsFootnote = false;
	}
	else if (widget == m_wEnd123OpenParen )
	{
		iType = FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN;
		bIsFootnote = false;
	}
	else if (widget == m_wEndLower )
	{
		iType = FOOTNOTE_TYPE_LOWER;
		bIsFootnote = false;
	}
	else if (widget == m_wEndLowerParen )
	{
		iType = FOOTNOTE_TYPE_LOWER_PAREN;
		bIsFootnote = false;
	}
	else if (widget == m_wEndLowerOpenParen )
	{
		iType =FOOTNOTE_TYPE_LOWER_OPEN_PAREN ;
		bIsFootnote = false;
	}
	else if (widget == m_wEndUpper )
	{
		iType = FOOTNOTE_TYPE_UPPER;
		bIsFootnote = false;
	}
	else if (widget == m_wEndUpperParen )
	{
		iType = FOOTNOTE_TYPE_UPPER_PAREN;
		bIsFootnote = false;
	}
	else if (widget == m_wEndUpperOpenParen)
	{
		iType = FOOTNOTE_TYPE_UPPER_OPEN_PAREN;
		bIsFootnote = false;
	}
	else if (widget == m_wEndRomanLower)
	{
		iType = FOOTNOTE_TYPE_LOWER_ROMAN ;
		bIsFootnote = false;
	}
	else if (widget == m_wEndRomanLowerParen )
	{
		iType = FOOTNOTE_TYPE_LOWER_ROMAN_PAREN;
		bIsFootnote = false;
	}
	else if (widget ==  m_wEndRomanUpper)
	{
		iType =FOOTNOTE_TYPE_UPPER_ROMAN ;
		bIsFootnote = false;
	}
	else if (widget == m_wEndRomanUpperParen )
	{
		iType =FOOTNOTE_TYPE_UPPER_ROMAN ;
		bIsFootnote = false;
	}
	else if (widget == m_wFoot123)
	{
		iType = FOOTNOTE_TYPE_NUMERIC;
	}
	else if (widget == m_wFoot123Brack )
	{
		iType = FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS;
	}
	else if (widget ==m_wFoot123Paren  )
	{
		iType =FOOTNOTE_TYPE_NUMERIC_PAREN ;
	}
	else if (widget == m_wFoot123OpenParen )
	{
		iType = FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN;
	}
	else if (widget == m_wFootLower )
	{
		iType = FOOTNOTE_TYPE_LOWER;
	}
	else if (widget == m_wFootLowerParen )
	{
		iType = FOOTNOTE_TYPE_LOWER_PAREN;
	}
	else if (widget == m_wFootLowerOpenParen )
	{
		iType =FOOTNOTE_TYPE_LOWER_OPEN_PAREN ;
	}
	else if (widget == m_wFootUpper )
	{
		iType = FOOTNOTE_TYPE_UPPER;
	}
	else if (widget == m_wFootUpperParen )
	{
		iType = FOOTNOTE_TYPE_UPPER_PAREN;
	}
	else if (widget == m_wFootUpperOpenParen)
	{
		iType = FOOTNOTE_TYPE_UPPER_OPEN_PAREN;
	}
	else if (widget == m_wFootRomanLower)
	{
		iType = FOOTNOTE_TYPE_LOWER_ROMAN ;
	}
	else if (widget == m_wFootRomanLowerParen )
	{
		iType = FOOTNOTE_TYPE_LOWER_ROMAN_PAREN;
	}
	else if (widget ==  m_wFootRomanUpper)
	{
		iType = FOOTNOTE_TYPE_UPPER_ROMAN;
	}
	else if (widget == m_wFootRomanUpperParen )
	{
		iType =FOOTNOTE_TYPE_UPPER_ROMAN ;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	if(bIsFootnote)
	{
		setFootnoteType(iType);
	}
	else
	{
		setEndnoteType(iType);
	}
	refreshVals();
}

/* 
   This method sets all the GUI visible things from values in the xp layer of
   the dialog.
*/
void  AP_UnixDialog_FormatFootnotes::refreshVals(void)
{
	UT_String sVal;
	getFootnoteValString(sVal);
	gtk_label_set_text(GTK_LABEL(m_wFootnotesInitialValText) , sVal.c_str());
	getEndnoteValString(sVal);
	gtk_label_set_text(GTK_LABEL(m_wEndnotesInitialValText) , sVal.c_str());
	g_signal_handler_block(G_OBJECT(m_wFootnotesRestartOnSection),
						   m_FootRestartSectionID);
	g_signal_handler_block(G_OBJECT(m_wFootnotesRestartOnPage),
						   m_FootRestartPageID);
	g_signal_handler_block(G_OBJECT(m_wEndnotesRestartOnSection),
						   m_EndRestartSectionID);
	g_signal_handler_block(G_OBJECT(m_wEndnotesPlaceEndOfDoc),
						   m_EndPlaceEndofDocID);
	g_signal_handler_block(G_OBJECT(m_wEndnotesPlaceEndOfSec),
						   m_EndPlaceEndofSectionID);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wFootnotesRestartOnSection), static_cast<gboolean>(getRestartFootnoteOnSection()));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wFootnotesRestartOnPage), static_cast<gboolean>(getRestartFootnoteOnPage()));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wEndnotesRestartOnSection), static_cast<gboolean>(getRestartEndnoteOnSection()));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wEndnotesPlaceEndOfDoc), static_cast<gboolean>(getPlaceAtDocEnd()));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wEndnotesPlaceEndOfSec), static_cast<gboolean>(getPlaceAtSecEnd()));

	switch(getFootnoteType())
	{
	case FOOTNOTE_TYPE_NUMERIC:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),0);
		break;
	case FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),1);
		break;
	case FOOTNOTE_TYPE_NUMERIC_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),2);
		break;
	case FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),3);
		break;
	case FOOTNOTE_TYPE_LOWER:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),4);
		break;
	case FOOTNOTE_TYPE_LOWER_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),5);
		break;
	case FOOTNOTE_TYPE_LOWER_OPEN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),6);
		break;
	case FOOTNOTE_TYPE_UPPER:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),7);
		break;
	case FOOTNOTE_TYPE_UPPER_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),8);
		break;
	case FOOTNOTE_TYPE_UPPER_OPEN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),9);
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),10);
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),11);
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),12);
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),13);
		break;
	default:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wFootnotesStyleMenu),0);
	}


	switch(getEndnoteType())
	{
	case FOOTNOTE_TYPE_NUMERIC:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),0);
		break;
	case FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),1);
		break;
	case FOOTNOTE_TYPE_NUMERIC_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),2);
		break;
	case FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),3);
		break;
	case FOOTNOTE_TYPE_LOWER:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),4);
		break;
	case FOOTNOTE_TYPE_LOWER_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),5);
		break;
	case FOOTNOTE_TYPE_LOWER_OPEN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),6);
		break;
	case FOOTNOTE_TYPE_UPPER:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),7);
		break;
	case FOOTNOTE_TYPE_UPPER_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),8);
		break;
	case FOOTNOTE_TYPE_UPPER_OPEN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),9);
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),10);
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),11);
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),12);
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN_PAREN:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),13);
		break;
	default:
		gtk_option_menu_set_history ( GTK_OPTION_MENU(m_wEndnotesStyleMenu),0);
	}

	g_signal_handler_unblock(G_OBJECT(m_wEndnotesPlaceEndOfSec),
						   m_EndPlaceEndofSectionID);
	g_signal_handler_unblock(G_OBJECT(m_wEndnotesPlaceEndOfDoc),
						   m_EndPlaceEndofDocID);
	g_signal_handler_unblock(G_OBJECT(m_wFootnotesRestartOnSection),
						   m_FootRestartSectionID);
	g_signal_handler_unblock(G_OBJECT(m_wFootnotesRestartOnPage),
						   m_FootRestartPageID);
	g_signal_handler_unblock(G_OBJECT(m_wEndnotesRestartOnSection),
						   m_EndRestartSectionID);

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

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  GtkWidget * NoteBook = gtk_notebook_new ();
  gtk_widget_show (NoteBook);
  gtk_container_add (GTK_CONTAINER (container), NoteBook);

  GtkWidget * vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (NoteBook), vbox1);

  GtkWidget * hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  GtkWidget * Footnote_Style_Label = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootStyle).c_str());
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
  m_wFoot123 = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("[1],[2],[3],..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFoot123Brack = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(1),(2),(3)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFoot123Paren = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("1),2),3)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFoot123OpenParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("a,b,c,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootLower = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(a),(b),(c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootLowerParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("a),b),c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootLowerOpenParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("A,B,C..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootUpper = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(A),(B),(C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootUpperParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("A),B),C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootUpperOpenParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("i,ii,iii,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootRomanLower = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(i),(ii),(iii),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootRomanLowerParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("I,II,III,...");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootRomanUpper = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(I),(II),(III),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), glade_menuitem);
  m_wFootRomanUpperParen = glade_menuitem;

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), optionmenu1_menu);

  m_wFootnotesStyleMenu = optionmenu1;

  GtkWidget * hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

  GtkWidget * Restart_On_Section = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartSec).c_str());
  gtk_widget_show (Restart_On_Section);
  gtk_box_pack_start (GTK_BOX (hbox2), Restart_On_Section, FALSE, FALSE, 0);

  m_wFootnotesRestartOnSection = Restart_On_Section;

  GtkWidget * Restart_On_Page = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartPage).c_str());
  gtk_widget_show (Restart_On_Page);
  gtk_box_pack_start (GTK_BOX (hbox2), Restart_On_Page, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Restart_On_Page), 2);

  m_wFootnotesRestartOnPage = Restart_On_Page;

  GtkWidget * hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, 0);

  GtkWidget * Initial_Val_lab = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootInitialVal).c_str());
  gtk_widget_show (Initial_Val_lab);
  gtk_box_pack_start (GTK_BOX (hbox3), Initial_Val_lab, TRUE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (Initial_Val_lab), 9, 0);

  GtkObject * spinbutton1_adj = gtk_adjustment_new (1, 0, 100, 1, 1, 10);
  GtkWidget * spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 0);
  gtk_widget_show (spinbutton1);
  gtk_box_pack_end (GTK_BOX (hbox3), spinbutton1, FALSE, FALSE, 25);
  gtk_widget_set_usize (spinbutton1, 1, -2);

  m_oFootnoteSpinAdj = spinbutton1_adj;
  m_wFootnoteSpin = spinbutton1;

  GtkWidget * Initial_Value_Footnote = gtk_label_new ("1");
  gtk_widget_show (Initial_Value_Footnote);
  gtk_label_set_justify(GTK_LABEL(Initial_Value_Footnote),GTK_JUSTIFY_RIGHT);
  gtk_box_pack_end (GTK_BOX (hbox3), Initial_Value_Footnote, FALSE, FALSE, 25);
  gtk_widget_set_usize (Initial_Value_Footnote, 90, -2);

  m_wFootnotesInitialValText = Initial_Value_Footnote;

  GtkWidget * Footnotes_tab = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootStyle).c_str());
  gtk_widget_show (Footnotes_tab);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (NoteBook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (NoteBook), 0), Footnotes_tab);

  GtkWidget * vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (NoteBook), vbox2);

  GtkWidget * hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox4, TRUE, TRUE, 0);

  GtkWidget * Endnote_Style = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndStyle).c_str());
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
  m_wEnd123 = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("[1],[2],[3],..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEnd123Brack = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(1),(2),(3)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEnd123Paren = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("1),2),3)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEnd123OpenParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("a,b,c,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndLower = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(a),(b),(c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndLowerParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("a),b),c)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndLowerOpenParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("A,B,C..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndUpper = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(A),(B),(C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndUpperParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("A),B),C)..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndUpperOpenParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("i,ii,iii,..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndRomanLower = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(i),(ii),(iii),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndRomanLowerParen = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("I,II,III,...");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndRomanUpper = glade_menuitem;

  glade_menuitem = gtk_menu_item_new_with_label ("(I),(II),(III),..");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), glade_menuitem);
  m_wEndRomanUpperParen = glade_menuitem;

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), optionmenu2_menu);


  m_wEndnotesStyleMenu = optionmenu2;

  GtkWidget * hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox5, TRUE, TRUE, 0);

  GtkWidget * Place_at_end_of_Section = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndSec).c_str());
  gtk_widget_show (Place_at_end_of_Section);
  gtk_box_pack_start (GTK_BOX (hbox5), Place_at_end_of_Section, FALSE, FALSE, 0);
  m_wEndnotesPlaceEndOfSec = Place_at_end_of_Section;

  GtkWidget * Place_At_End_of_doc = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndDoc).c_str());
  gtk_widget_show (Place_At_End_of_doc);
  gtk_box_pack_start (GTK_BOX (hbox5), Place_At_End_of_doc, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (Place_At_End_of_doc), 2);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (Place_At_End_of_doc), TRUE);

  m_wEndnotesPlaceEndOfDoc = Place_At_End_of_doc;

  GtkWidget * hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox6);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox6, TRUE, TRUE, 0);

  GtkWidget * Restart_on_Section = gtk_check_button_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndRestartSec).c_str());
  gtk_widget_show (Restart_on_Section);
  gtk_box_pack_start (GTK_BOX (hbox6), Restart_on_Section, FALSE, FALSE, 0);

  m_wEndnotesRestartOnSection = Restart_on_Section;

  GtkWidget * Initial_Endnote_lab = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndInitialVal).c_str());
  gtk_widget_show (Initial_Endnote_lab);
  gtk_label_set_justify(GTK_LABEL(Initial_Endnote_lab),GTK_JUSTIFY_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox6), Initial_Endnote_lab, FALSE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (Initial_Endnote_lab), 9, 0);


  GtkWidget * Endnotes_tab = gtk_label_new (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndStyle).c_str());
  gtk_widget_show (Endnotes_tab);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (NoteBook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (NoteBook), 1), Endnotes_tab);

  GtkObject * spinbutton2_adj = gtk_adjustment_new (1, 0, 100, 1, 1, 10);
  GtkWidget * spinbutton2 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton2_adj), 1, 0);
  gtk_widget_show (spinbutton2);
  gtk_box_pack_end (GTK_BOX (hbox6), spinbutton2, TRUE, FALSE, 0);
  gtk_widget_set_usize (spinbutton2, 1, -2);

  m_oEndnoteSpinAdj = spinbutton2_adj;
  m_wEndnoteSpin = spinbutton2;

  GtkWidget * Initial_Value_Endnote = gtk_label_new ("1");
  gtk_widget_show (Initial_Value_Endnote);
  gtk_box_pack_end (GTK_BOX (hbox6), Initial_Value_Endnote, FALSE, FALSE, 25);
  gtk_widget_set_usize (Initial_Value_Endnote, 90, -2);

  m_wEndnotesInitialValText = Initial_Value_Endnote;
}

GtkWidget*  AP_UnixDialog_FormatFootnotes::_constructWindow(void)
{
  GtkWidget *frame1;
  GtkWidget *vbox2;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_windowMain = abiDialogNew("format footnotes dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_Title).c_str());

  frame1 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_NONE);
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_windowMain)->vbox), frame1);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 4);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (frame1), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  _constructWindowContents ( vbox2 );

  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);

// Apply button does not destoy widget. Do it this way.
  abiAddStockButton(GTK_DIALOG(m_windowMain), GTK_STOCK_OK, BUTTON_OK);
  _connectSignals();
  return m_windowMain;
}


#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        g_signal_connect(G_OBJECT(w), "activate",	\
                G_CALLBACK(s_menu_item_activate),		\
                reinterpret_cast<gpointer>(this));							\
        } while (0)

void AP_UnixDialog_FormatFootnotes::_connectSignals(void)
{
	m_FootnoteSpinHanderID = g_signal_connect(G_OBJECT(m_wFootnoteSpin ),
											  "changed",
											  G_CALLBACK(s_FootInitial),
											  reinterpret_cast<gpointer>(this));
	m_EndnoteSpinHanderID = g_signal_connect(G_OBJECT(m_wEndnoteSpin ),
											  "changed",
											  G_CALLBACK(s_EndInitial),
											  reinterpret_cast<gpointer>(this));
	m_FootRestartPageID = g_signal_connect(G_OBJECT(m_wFootnotesRestartOnPage ),
										   "clicked",
										   G_CALLBACK(s_FootRestartPage),
										   reinterpret_cast<gpointer>(this));
	m_FootRestartSectionID = g_signal_connect(G_OBJECT(m_wFootnotesRestartOnSection ),
										   "clicked",
										   G_CALLBACK(s_FootRestartSection),
										   reinterpret_cast<gpointer>(this));
	m_EndPlaceEndofSectionID = g_signal_connect(G_OBJECT(m_wEndnotesPlaceEndOfSec ),
											  "clicked",
											  G_CALLBACK(s_EndPlaceEndSection),
											  reinterpret_cast<gpointer>(this));
	m_EndPlaceEndofDocID = g_signal_connect(G_OBJECT(m_wEndnotesPlaceEndOfDoc ),
										  "clicked",
										  G_CALLBACK(s_EndPlaceEndDoc),
										  reinterpret_cast<gpointer>(this));
	m_EndRestartSectionID = g_signal_connect(G_OBJECT(m_wEndnotesRestartOnSection ),
										  "clicked",
										  G_CALLBACK(s_EndRestartSection),
										  reinterpret_cast<gpointer>(this));
									
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEnd123);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEnd123Brack);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEnd123Paren);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEnd123OpenParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndLower);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndLowerParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndLowerOpenParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndUpper);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndUpperParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndUpperOpenParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndRomanLower);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndRomanLowerParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndRomanUpper);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndRomanUpperParen);

									
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFoot123);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFoot123Brack);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFoot123Paren);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFoot123OpenParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootLower);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootLowerParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootLowerOpenParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootUpper);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootUpperParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootUpperOpenParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootRomanLower);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootRomanLowerParen);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootRomanUpper);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootRomanUpperParen);

}
