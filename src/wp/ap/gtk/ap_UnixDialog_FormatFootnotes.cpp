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

#undef GTK_DISABLE_DEPRECATED

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
	m_wFootnoteNumberingMenu = NULL;
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

	m_FootnoteSpinHanderID= 0;
	m_EndnoteSpinHanderID =0;
	m_FootRestartPageID = 0;
	m_FootRestartSectionID = 0;
	m_EndPlaceEndofSectionID = 0;
	m_EndPlaceEndofDocID = 0;
	m_EndRestartSectionID = 0;
	m_FootStyleID = 0;
	m_EndStyleID = 0;
}


AP_UnixDialog_FormatFootnotes::~AP_UnixDialog_FormatFootnotes(void)
{
}
/****************************************************************/
/* Static Callbacks for event handling */
/****************************************************************/

static void s_menu_item_endnote_style(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_MenuStyleEndnoteChange(widget);
}

static void s_menu_item_footnote_style(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_MenuStyleFootnoteChange(widget);
}

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


static void s_EndRestartSection(GtkWidget * /*widget*/, AP_UnixDialog_FormatFootnotes * dlg)
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
							 GTK_RESPONSE_OK, true))
	  {
	  case BUTTON_DELETE:
		  UT_DEBUGMSG(("Doing Delete branch \n"));
		  event_Delete () ; 
		  break ;
	  case GTK_RESPONSE_OK:
		  UT_DEBUGMSG(("Doing Apply (OK) branch \n"));
		  event_Apply(); 
		  break;
	  default:
		  UT_DEBUGMSG(("Doing Default (NOT OK) branch \n"));
		  event_Cancel () ; 
		  break;
	  }
}

/*	g_signal_connect(G_OBJECT(m_wButtonApply),
					 "clicked",
					 G_CALLBACK(s_Apply),
					 static_cast<gpointer>(this)); */

void AP_UnixDialog_FormatFootnotes::event_Apply(void)
{

// Apply the current settings to the document
	setAnswer(a_OK);
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

void AP_UnixDialog_FormatFootnotes::event_MenuStyleFootnoteChange(GtkWidget * widget)
{
	/* note to typecast facist: rebuild on every platform before even thinking committing a change
	 * to the cast below */
	FootnoteType iType = (FootnoteType)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "user_data"));
	setFootnoteType(iType);
	refreshVals();
}


void AP_UnixDialog_FormatFootnotes::event_MenuStyleEndnoteChange(GtkWidget * widget)
{
	/* note to typecast facist: rebuild on every platform before even thinking committing a change
	 * to the cast below */
	FootnoteType iType = (FootnoteType)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "user_data"));
	setEndnoteType(iType);
	refreshVals();
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

	if(getPlaceAtDocEnd())
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wEndnotesPlaceMenu),1);
	}
	else if(getPlaceAtSecEnd())
	{
		gtk_option_menu_set_history(GTK_OPTION_MENU(m_wEndnotesPlaceMenu),0);
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
	
	// get the path where our UI file is located
	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/ap_UnixDialog_FormatFootnotes.xml";
	
	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, ui_path.c_str(), NULL);
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_FormatFootnotes"));
	// set the dialog title
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_Title,s);
	abiDialogSetTitle(window, s.utf8_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbFootnote")), pSS, AP_STRING_ID_DLG_FormatFootnotes_Footnotes);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbFootnoteStyle")), pSS, AP_STRING_ID_DLG_FormatFootnotes_FootStyle);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbFootnoteRestart")), pSS, AP_STRING_ID_DLG_FormatFootnotes_FootnoteRestart);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbFootnoteValue")), pSS, AP_STRING_ID_DLG_FormatFootnotes_FootInitialVal);
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbEndnote")), pSS, AP_STRING_ID_DLG_FormatFootnotes_Endnotes);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbEndnoteStyle")), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndStyle);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbEndnotePlacement")), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndPlacement);

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbEndnoteValue")), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndInitialVal);

	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "cbSectionRestart")), pSS, AP_STRING_ID_DLG_FormatFootnotes_EndRestartSec);

//
// Now extract widgets from the menu items
//

	const UT_GenericVector<const gchar*>* footnoteTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();

	
		
	m_wFootnotesStyleMenu = GTK_WIDGET(gtk_builder_get_object(builder, "omFootnoteStyle"));
	UT_ASSERT(m_wFootnotesStyleMenu );
	gtk_option_menu_set_menu(GTK_OPTION_MENU(m_wFootnotesStyleMenu), 
							 abiGtkMenuFromCStrVector(*footnoteTypeList, G_CALLBACK(s_menu_item_footnote_style), 
													  reinterpret_cast<gpointer>(this)));
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_wFootnotesStyleMenu), 0);

	m_wEndnotesStyleMenu = GTK_WIDGET(gtk_builder_get_object(builder, "omEndnoteStyle"));
	UT_ASSERT(m_wEndnotesStyleMenu);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(m_wEndnotesStyleMenu), 
							 abiGtkMenuFromCStrVector(*footnoteTypeList, G_CALLBACK(s_menu_item_endnote_style), 
													  reinterpret_cast<gpointer>(this)));
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_wEndnotesStyleMenu), 0);

//
// Footnotes number menu
//
	m_wFootnoteNumberingMenu = GTK_WIDGET(gtk_builder_get_object(builder, "omNumbering"));
	UT_ASSERT(m_wFootnoteNumberingMenu );
	GtkWidget * wMenuFoot = gtk_menu_new ();
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartNone,s);
	m_wFootnotesDontRestart = gtk_menu_item_new_with_label (s.utf8_str());
	gtk_widget_show (m_wFootnotesDontRestart );
	gtk_menu_shell_append (GTK_MENU_SHELL (wMenuFoot),m_wFootnotesDontRestart );

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartSec,s);
	m_wFootnotesRestartOnSection = gtk_menu_item_new_with_label (s.utf8_str());
	gtk_widget_show (m_wFootnotesRestartOnSection );
	gtk_menu_shell_append (GTK_MENU_SHELL (wMenuFoot),m_wFootnotesRestartOnSection );


	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartPage,s);
	m_wFootnotesRestartOnPage = gtk_menu_item_new_with_label (s.utf8_str());
	gtk_widget_show (m_wFootnotesRestartOnPage );
	gtk_menu_shell_append (GTK_MENU_SHELL (wMenuFoot),m_wFootnotesRestartOnPage );

	gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wFootnoteNumberingMenu),wMenuFoot);

//
// Endnotes placement menu
//
	m_wEndnotesPlaceMenu = GTK_WIDGET(gtk_builder_get_object(builder, "omEndnotePlacement"));
	UT_ASSERT(m_wEndnotesPlaceMenu );
	GtkWidget * wMenuPlace = gtk_menu_new();

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndSec,s);
	m_wEndnotesPlaceEndOfSec = gtk_menu_item_new_with_label (s.utf8_str());
	gtk_widget_show (m_wEndnotesPlaceEndOfSec );
	gtk_menu_shell_append (GTK_MENU_SHELL (wMenuPlace),m_wEndnotesPlaceEndOfSec);

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndDoc,s);
	m_wEndnotesPlaceEndOfDoc = gtk_menu_item_new_with_label (s.utf8_str());
	gtk_widget_show (m_wEndnotesPlaceEndOfDoc );
	gtk_menu_shell_append (GTK_MENU_SHELL (wMenuPlace),m_wEndnotesPlaceEndOfDoc );

	gtk_option_menu_set_menu (GTK_OPTION_MENU (m_wEndnotesPlaceMenu), wMenuPlace);

//
// Now grab widgets for the remaining controls.
//
	m_wEndnotesRestartOnSection = GTK_WIDGET(gtk_builder_get_object(builder, "cbSectionRestart"));
	UT_ASSERT(m_wEndnotesRestartOnSection );
// Endnote Initial Value Control

	m_wEndnotesInitialValText = GTK_WIDGET(gtk_builder_get_object(builder, "endSpinValueText"));
	UT_ASSERT(m_wEndnotesInitialValText );
	m_wEndnoteSpin = GTK_WIDGET(gtk_builder_get_object(builder, "endnoteSpin"));
	m_oEndnoteSpinAdj = GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_wEndnoteSpin)));

// Footnote Initial Value Control

	m_wFootnoteSpin = GTK_WIDGET(gtk_builder_get_object(builder, "footnoteSpin"));
	UT_ASSERT(m_wFootnoteSpin );
	m_oFootnoteSpinAdj = GTK_OBJECT(gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_wFootnoteSpin)));
	m_wFootnotesInitialValText = GTK_WIDGET(gtk_builder_get_object(builder, "footSpinValueText"));
	UT_ASSERT(m_wFootnotesInitialValText );
	_connectSignals();
	refreshVals();

	g_object_unref(G_OBJECT(builder));

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
}
