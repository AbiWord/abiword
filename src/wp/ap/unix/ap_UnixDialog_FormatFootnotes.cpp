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
	m_wFootnotesStyleMenu = NULL;
	m_wFootnotesDontRestart = NULL;
	m_wFootnotesRestartOnSection = NULL;
	m_wFootnotesRestartOnPage = NULL;
	m_wFootnotesInitialValText = NULL;
	m_wFootnoteSpin = NULL;
	m_oFootnoteSpinAdj = NULL;

	m_wEndnotesStyleMenu = NULL;
	m_wEndnotesPlaceMenu= NULL;
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
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);
	refreshVals();


	switch(abiRunModalDialog(GTK_DIALOG(m_windowMain), pFrame, this,
							 GTK_RESPONSE_OK, false))
	  {
	  case BUTTON_DELETE:
		  UT_DEBUGMSG(("Doing Delete branch \n"));
	    event_Delete () ; break ;
	  case GTK_RESPONSE_OK:
		  UT_DEBUGMSG(("Doing Apply (OK) branch \n"));
		  event_Apply(); break;
	  default:
		  UT_DEBUGMSG(("Doing Default (NOT OK) branch \n"));
	    event_Cancel () ; break ;
	  }
	
	abiDestroyWidget ( m_windowMain ) ;
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
	if(widget == m_wFootnotesRestartOnPage)
	{
		setRestartFootnoteOnPage(true);
		setRestartFootnoteOnSection(false);
		refreshVals();
		return;
	}
	if(widget == m_wFootnotesRestartOnSection)
	{
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(true);
		refreshVals();
		return;
	}
	if(widget == m_wFootnotesDontRestart)
	{
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(false);
		refreshVals();
		return;
	}

	if(widget == m_wEndnotesPlaceEndOfDoc)
	{
		setPlaceAtSecEnd(false);
		setPlaceAtDocEnd(true);
		refreshVals();
		return;
	}

	if(widget == m_wEndnotesPlaceEndOfSec)
	{
		setPlaceAtSecEnd(true);
		setPlaceAtDocEnd(false);
		refreshVals();
		return;
	}


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
	gtk_label_set_text(GTK_LABEL(m_wFootnotesInitialValText),sVal.c_str());

	getEndnoteValString(sVal);
	gtk_label_set_text(GTK_LABEL(m_wEndnotesInitialValText),sVal.c_str());


	g_signal_handler_block(G_OBJECT(m_wEndnotesRestartOnSection),
						   m_EndRestartSectionID);

	if(getRestartFootnoteOnSection())
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wFootnoteNumberingMenu),1);
	}
	else if(getRestartFootnoteOnPage())
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wFootnoteNumberingMenu),2);
	}
	else
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wFootnoteNumberingMenu),0);
	}

	if(getRestartEndnoteOnSection())
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wEndnotesPlaceMenu),0);
	}
	else if(getPlaceAtDocEnd())
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wEndnotesPlaceMenu),1);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wEndnotesRestartOnSection), static_cast<gboolean>(getRestartEndnoteOnSection()));

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

/*****************************************************************/
GtkWidget * AP_UnixDialog_FormatFootnotes::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_FormatFootnotes.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = glade_xml_get_widget(xml, "ap_UnixDialog_FormatFootnotes");
	// set the dialog title
	abiDialogSetTitle(window, pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_Title).c_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbFootnote"), pSS, AP_STRING_ID_DLG_FormatFootnotes_Footnotes);

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbFootnoteStyle"), pSS, AP_STRING_ID_DLG_FormatFootnotes_FootStyle);

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbFootnoteRestart"), pSS, AP_STRING_ID_DLG_FormatFootnotes_FootnoteRestart);

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbFootnoteValue"), pSS, AP_STRING_ID_DLG_FormatFootnotes_FootInitialVal);
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "lbEndnote"), pSS, AP_STRING_ID_DLG_FormatFootnotes_Endnotes);

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbEndnoteStyle"), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndStyle);

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbEndnotePlacement"), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndPlacement);

	localizeLabelMarkup(glade_xml_get_widget(xml, "lbEndnoteValue"), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndInitialVal);

	localizeButton(glade_xml_get_widget(xml, "cbSectionRestart"), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndRestartSec);

//
// Now extract widgets from the menu items
//
	m_wFootnotesStyleMenu = glade_xml_get_widget(xml, "omFootnoteStyle");
	UT_ASSERT(m_wFootnotesStyleMenu );
	m_wFoot123 = glade_xml_get_widget(xml, "foot123");
	UT_ASSERT(m_wFoot123 );
	m_wFoot123Brack = glade_xml_get_widget(xml, "foot123Brack");
	UT_ASSERT(m_wFoot123Brack );
	m_wFoot123Paren = glade_xml_get_widget(xml, "foot123Paren");
	UT_ASSERT(m_wFoot123Paren );
	m_wFoot123OpenParen = glade_xml_get_widget(xml, "foot123OpenParen");
	UT_ASSERT(m_wFoot123OpenParen );
	m_wFootLower =  glade_xml_get_widget(xml, "footLower");
	UT_ASSERT(m_wFootLower );
	m_wFootLowerParen =  glade_xml_get_widget(xml, "footLowerParen");
	UT_ASSERT(m_wFootLowerParen );
	m_wFootLowerOpenParen =  glade_xml_get_widget(xml, "footLowerOpenParen");
	UT_ASSERT(m_wFootLowerOpenParen );
	m_wFootUpper =  glade_xml_get_widget(xml, "footUpper");
	UT_ASSERT(m_wFootUpper );
	m_wFootUpperParen =  glade_xml_get_widget(xml, "footUpperParen");
	UT_ASSERT(m_wFootUpperParen );
	m_wFootUpperOpenParen =  glade_xml_get_widget(xml, "footUpperOpenParen");
	UT_ASSERT(m_wFootUpperOpenParen );
	m_wFootRomanLower =  glade_xml_get_widget(xml, "footRomanLower");
	UT_ASSERT(m_wFootRomanLower );
	m_wFootRomanLowerParen =  glade_xml_get_widget(xml, "footRomanLowerParen");
	UT_ASSERT(m_wFootRomanLowerParen );
	m_wFootRomanUpper =  glade_xml_get_widget(xml, "footRomanUpper");
	UT_ASSERT(m_wFootRomanUpper );
	m_wFootRomanUpperParen =  glade_xml_get_widget(xml, "footRomanUpperParen");
	UT_ASSERT(m_wFootRomanUpperParen );

	m_wEndnotesStyleMenu = glade_xml_get_widget(xml, "omEndnoteStyle");
	UT_ASSERT(m_wEndnotesStyleMenu );
	m_wEnd123 = glade_xml_get_widget(xml, "end123");
	UT_ASSERT(m_wEnd123 );
	m_wEnd123Brack = glade_xml_get_widget(xml, "end123Brack");
	UT_ASSERT(m_wEnd123Brack );
	m_wEnd123Paren = glade_xml_get_widget(xml, "end123Paren");
	UT_ASSERT(m_wEnd123Paren );
	m_wEnd123OpenParen = glade_xml_get_widget(xml, "end123OpenParen");
	UT_ASSERT(m_wEnd123OpenParen );
	m_wEndLower =  glade_xml_get_widget(xml, "endLower");
	UT_ASSERT(m_wEndLower );
	m_wEndLowerParen =  glade_xml_get_widget(xml, "endLowerParen");
	UT_ASSERT(m_wEndLowerParen );
	m_wEndLowerOpenParen =  glade_xml_get_widget(xml, "endLowerOpenParen");
	UT_ASSERT(m_wEndLowerOpenParen );
	m_wEndUpper =  glade_xml_get_widget(xml, "endUpper");
	UT_ASSERT(m_wEndUpper );
	m_wEndUpperParen =  glade_xml_get_widget(xml, "endUpperParen");
	UT_ASSERT(m_wEndUpperParen );
	m_wEndUpperOpenParen =  glade_xml_get_widget(xml, "endUpperOpenParen");
	UT_ASSERT(m_wEndUpperOpenParen );
	m_wEndRomanLower =  glade_xml_get_widget(xml, "endRomanLower");
	UT_ASSERT(m_wEndRomanLower );
	m_wEndRomanLowerParen =  glade_xml_get_widget(xml, "endRomanLowerParen");
	UT_ASSERT(m_wEndRomanLowerParen );
	m_wEndRomanUpper =  glade_xml_get_widget(xml, "endRomanUpper");
	UT_ASSERT(m_wEndRomanUpper );
	m_wEndRomanUpperParen =  glade_xml_get_widget(xml, "endRomanUpperParen");
	UT_ASSERT(m_wEndRomanUpperParen );
//
// Footnotes number menu
//
	m_wFootnoteNumberingMenu = glade_xml_get_widget(xml, "omNumbering");
	UT_ASSERT(m_wFootnoteNumberingMenu );
	GtkWidget * wMenuFoot = gtk_menu_new ();
	m_wFootnotesDontRestart = gtk_menu_item_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartNone).c_str());
	gtk_widget_show (m_wFootnotesDontRestart );
	gtk_menu_append (GTK_MENU (wMenuFoot),m_wFootnotesDontRestart );

	m_wFootnotesRestartOnSection = gtk_menu_item_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartSec).c_str());
	gtk_widget_show (m_wFootnotesRestartOnSection );
	gtk_menu_append (GTK_MENU (wMenuFoot),m_wFootnotesRestartOnSection );


	m_wFootnotesRestartOnPage = gtk_menu_item_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartPage).c_str());
	gtk_widget_show (m_wFootnotesRestartOnPage );
	gtk_menu_append (GTK_MENU (wMenuFoot),m_wFootnotesRestartOnPage );

	gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wFootnoteNumberingMenu),wMenuFoot);

//
// Endnotes placement menu
//
	m_wEndnotesPlaceMenu = glade_xml_get_widget(xml, "omEndnotePlacement");
	UT_ASSERT(m_wEndnotesPlaceMenu );
	GtkWidget * wMenuPlace = gtk_menu_new();

	m_wEndnotesPlaceEndOfDoc = gtk_menu_item_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndDoc).c_str());
	gtk_widget_show (m_wEndnotesPlaceEndOfDoc );
	gtk_menu_append (GTK_MENU (wMenuPlace),m_wEndnotesPlaceEndOfDoc );

	m_wEndnotesPlaceEndOfSec = gtk_menu_item_new_with_label (pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndSec).c_str());
	gtk_widget_show (m_wEndnotesPlaceEndOfSec );
	gtk_menu_append (GTK_MENU (wMenuPlace),m_wEndnotesPlaceEndOfSec);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wEndnotesPlaceMenu), wMenuPlace);

//
// Now grab widgets for the remaining controls.
//
	m_wEndnotesRestartOnSection = glade_xml_get_widget(xml, "cbSectionRestart");
	UT_ASSERT(m_wEndnotesRestartOnSection );
// Endnote Initial Value Control

	m_wEndnotesInitialValText = glade_xml_get_widget(xml, "endSpinValueText");
	UT_ASSERT(m_wEndnotesInitialValText );
	m_wEndnoteSpin = glade_xml_get_widget(xml, "endnoteSpin");
	m_oEndnoteSpinAdj = GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_wEndnoteSpin)));

// Footnote Initial Value Control

	m_wFootnoteSpin = glade_xml_get_widget(xml, "footnoteSpin");
	UT_ASSERT(m_wFootnoteSpin );
	m_oFootnoteSpinAdj = GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_wFootnoteSpin)));
	m_wFootnotesInitialValText = glade_xml_get_widget(xml, "footSpinValueText");
	UT_ASSERT(m_wFootnotesInitialValText );
	_connectSignals();
	return window;
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
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootnotesRestartOnPage);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootnotesRestartOnSection);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wFootnotesDontRestart);

	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndnotesPlaceEndOfDoc);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_wEndnotesPlaceEndOfSec);

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
