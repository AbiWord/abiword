/* AbiWord
 * Copyright (C) 2003 Martin Sevior
 * Copyright (C) 2009 Hubert Figuiere
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"
#include "xap_GtkSignalBlocker.h"
#include "xap_GtkComboBoxHelpers.h"

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
	m_wFootnoteSpin = NULL;
	m_oFootnoteSpinAdj = NULL;

	m_wEndnotesStyleMenu = NULL;
	m_wEndnotesPlaceMenu= NULL;
	m_wEndnotesRestartOnSection = NULL;
	m_wEndnoteSpin = NULL;
	m_oEndnoteSpinAdj = NULL;

	m_FootnoteSpinHanderID= 0;
	m_EndnoteSpinHanderID =0;
	m_FootNumberingID = 0;
	m_EndPlaceID = 0;
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

static void s_menu_item_footnote_activate(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_MenuFootnoteChange(widget);
}

static void s_menu_item_endnote_activate(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_ASSERT(widget && dlg);

	dlg->event_MenuEndnoteChange(widget);
}

static void s_FootInitial(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_UNUSED(widget);
	UT_ASSERT(widget && dlg);
	UT_DEBUGMSG(("Initial Footnote Val changed \n")); 
	dlg->event_FootInitialValueChange();
}

static void s_EndInitial(GtkWidget * widget, AP_UnixDialog_FormatFootnotes * dlg)
{
	UT_UNUSED(widget);
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
	GtkTreeIter iter;
	GtkComboBox * combo = GTK_COMBO_BOX(widget);
	gtk_combo_box_get_active_iter(combo, &iter);
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	int value;
	gtk_tree_model_get(store, &iter, 1, &value, -1);
	setFootnoteType((FootnoteType)value);
	refreshVals();
}


void AP_UnixDialog_FormatFootnotes::event_MenuStyleEndnoteChange(GtkWidget * widget)
{
	GtkTreeIter iter;
	GtkComboBox * combo = GTK_COMBO_BOX(widget);
	gtk_combo_box_get_active_iter(combo, &iter);
	GtkTreeModel *store = gtk_combo_box_get_model(combo);
	int value;
	gtk_tree_model_get(store, &iter, 1, &value, -1);
	setEndnoteType((FootnoteType)value);
	refreshVals();
}


void AP_UnixDialog_FormatFootnotes::event_MenuFootnoteChange(GtkWidget * widget)
{
	UT_ASSERT(widget);
	int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	switch(idx) {
	case 0:
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(false);
		refreshVals();
		return;
	case 1:
		setRestartFootnoteOnPage(false);
		setRestartFootnoteOnSection(true);
		refreshVals();
		return;
	case 2:
		setRestartFootnoteOnPage(true);
		setRestartFootnoteOnSection(false);
		refreshVals();
		return;
	default:
		break;
	}
	refreshVals();
}

void AP_UnixDialog_FormatFootnotes::event_MenuEndnoteChange(GtkWidget * widget)
{
	
	UT_ASSERT(widget);
	int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	switch(idx) {
	case 0:
		setPlaceAtDocEnd(true);
		setPlaceAtSecEnd(false);
		refreshVals();
		return;
	case 1:
		setPlaceAtDocEnd(false);
		setPlaceAtSecEnd(true);
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wFootnoteSpin), getFootnoteVal());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_wEndnoteSpin), getEndnoteVal());

	XAP_GtkSignalBlocker b1(G_OBJECT(m_wEndnotesRestartOnSection), 
							m_EndRestartSectionID);
	XAP_GtkSignalBlocker b2(G_OBJECT(m_wFootnoteNumberingMenu),
							m_FootNumberingID);
	XAP_GtkSignalBlocker b3(G_OBJECT(m_wEndnotesPlaceMenu), 
							m_EndPlaceID);

	if(getRestartFootnoteOnSection())
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wFootnoteNumberingMenu),1);
	}
	else if(getRestartFootnoteOnPage())
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wFootnoteNumberingMenu),2);
	}
	else
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wFootnoteNumberingMenu),0);
	}

	if(getPlaceAtDocEnd())
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wEndnotesPlaceMenu),0);
	}
	else if(getPlaceAtSecEnd())
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wEndnotesPlaceMenu),1);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wEndnotesRestartOnSection), static_cast<gboolean>(getRestartEndnoteOnSection()));

	XAP_comboBoxSetActiveFromIntCol(m_wFootnotesStyleMenu, 1, 
									(int)getFootnoteType());

	XAP_comboBoxSetActiveFromIntCol(m_wEndnotesStyleMenu, 1, 
									(int)getEndnoteType());
}

void AP_UnixDialog_FormatFootnotes::event_Cancel(void)
{
	setAnswer(AP_Dialog_FormatFootnotes::a_CANCEL);
}

void AP_UnixDialog_FormatFootnotes::event_Delete(void)
{
	setAnswer(AP_Dialog_FormatFootnotes::a_DELETE);
}


static void _populateCombo(GtkComboBox * combo, const FootnoteTypeDesc * desc_list)
{
	const FootnoteTypeDesc * current = desc_list;
	for( ; current->n !=  _FOOTNOTE_TYPE_INVALID; current++) {
		XAP_appendComboBoxTextAndInt(combo, current->label, 
									 static_cast<int>(current->n));
	}
}


/*****************************************************************/
GtkWidget * AP_UnixDialog_FormatFootnotes::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_FormatFootnotes.ui");

	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_FormatFootnotes"));
	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_Title,s);
	abiDialogSetTitle(window, "%s", s.c_str());
	
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

	const FootnoteTypeDesc * footnoteTypeList = AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList();

	
		
	m_wFootnotesStyleMenu = GTK_COMBO_BOX(gtk_builder_get_object(builder, "omFootnoteStyle"));
	UT_ASSERT(m_wFootnotesStyleMenu );
	XAP_makeGtkComboBoxText(m_wFootnotesStyleMenu, G_TYPE_INT);
	_populateCombo(m_wFootnotesStyleMenu, footnoteTypeList);
	gtk_combo_box_set_active(m_wFootnotesStyleMenu, 0);

	m_wEndnotesStyleMenu = GTK_COMBO_BOX(gtk_builder_get_object(builder, "omEndnoteStyle"));
	UT_ASSERT(m_wEndnotesStyleMenu);
	XAP_makeGtkComboBoxText(m_wEndnotesStyleMenu, G_TYPE_INT);
	_populateCombo(m_wEndnotesStyleMenu, footnoteTypeList);
	gtk_combo_box_set_active(m_wEndnotesStyleMenu, 0);

//
// Footnotes number menu
//
	m_wFootnoteNumberingMenu = GTK_COMBO_BOX(gtk_builder_get_object(builder, "omNumbering"));
	UT_ASSERT(m_wFootnoteNumberingMenu );
	XAP_makeGtkComboBoxText(m_wFootnoteNumberingMenu, G_TYPE_NONE);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartNone,s);
	XAP_appendComboBoxText(m_wFootnoteNumberingMenu, s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartSec,s);
	XAP_appendComboBoxText(m_wFootnoteNumberingMenu, s.c_str());

	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_FootRestartPage,s);
	XAP_appendComboBoxText(m_wFootnoteNumberingMenu, s.c_str());
//	m_wFootnotesRestartOnPage = gtk_menu_item_new_with_label (s.utf8_str());


//
// Endnotes placement menu
//
	m_wEndnotesPlaceMenu = GTK_COMBO_BOX(gtk_builder_get_object(builder, "omEndnotePlacement"));
	UT_ASSERT(m_wEndnotesPlaceMenu );
	XAP_makeGtkComboBoxText(m_wEndnotesPlaceMenu, G_TYPE_NONE);
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndDoc,s);
	XAP_appendComboBoxText(m_wEndnotesPlaceMenu, s.c_str());
	pSS->getValueUTF8(AP_STRING_ID_DLG_FormatFootnotes_EndPlaceEndSec,s);
	XAP_appendComboBoxText(m_wEndnotesPlaceMenu, s.c_str());

//
// Now grab widgets for the remaining controls.
//
	m_wEndnotesRestartOnSection = GTK_WIDGET(gtk_builder_get_object(builder, "cbSectionRestart"));
	UT_ASSERT(m_wEndnotesRestartOnSection );
// Endnote Initial Value Control

	m_wEndnoteSpin = GTK_WIDGET(gtk_builder_get_object(builder, "endnoteSpin"));
	m_oEndnoteSpinAdj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_wEndnoteSpin));

// Footnote Initial Value Control

	m_wFootnoteSpin = GTK_WIDGET(gtk_builder_get_object(builder, "footnoteSpin"));
	UT_ASSERT(m_wFootnoteSpin );
	m_oFootnoteSpinAdj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(m_wFootnoteSpin));
	_connectSignals();
	refreshVals();

	g_object_unref(G_OBJECT(builder));

	return window;
}



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
	m_FootStyleID = g_signal_connect(G_OBJECT(m_wFootnotesStyleMenu), "changed",
									 G_CALLBACK(s_menu_item_footnote_style),
									 reinterpret_cast<gpointer>(this));
	m_EndStyleID = g_signal_connect(G_OBJECT(m_wEndnotesStyleMenu), "changed",
									G_CALLBACK(s_menu_item_endnote_style),
									reinterpret_cast<gpointer>(this));
	m_FootNumberingID = g_signal_connect(G_OBJECT(m_wFootnoteNumberingMenu), 
										 "changed",
										 G_CALLBACK(s_menu_item_footnote_activate),
										 reinterpret_cast<gpointer>(this));
	m_EndPlaceID = g_signal_connect(G_OBJECT(m_wEndnotesPlaceMenu), "changed",
					 G_CALLBACK(s_menu_item_endnote_activate),
					 reinterpret_cast<gpointer>(this));
	m_EndRestartSectionID = g_signal_connect(G_OBJECT(m_wEndnotesRestartOnSection ),
										  "clicked",
										  G_CALLBACK(s_EndRestartSection),
										  reinterpret_cast<gpointer>(this));
}
