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
#include <stdlib.h>
#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_Print.h"
#include "xap_UnixDlg_MessageBox.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_UnixPSGraphics.h"
#include "xap_Strings.h"
#include <gtk/gtk.h>
#include <glib.h>
#include "xap_Prefs_SchemeIds.h"
#include "xap_Prefs.h"

/*****************************************************************/
/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixDialog_Print * p = new XAP_UnixDialog_Print(pFactory,id);
	return p;
}

XAP_UnixDialog_Print::XAP_UnixDialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
	memset(&m_persistPrintDlg, 0, sizeof(m_persistPrintDlg));
	m_bEmbedFonts = false;
}

XAP_UnixDialog_Print::~XAP_UnixDialog_Print(void)
{
//	DELETEP(m_pPSGraphics);
	FREEP(m_persistPrintDlg.szPrintCommand);
}

void XAP_UnixDialog_Print::useStart(void)
{
	XAP_Dialog_Print::useStart();

	if (m_bPersistValid)
	{
		m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
		m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
		m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
		m_persistPrintDlg.bDoCollate = m_bCollate;

		m_persistPrintDlg.colorSpace = m_cColorSpace;
		m_persistPrintDlg.szPrintCommand = m_szPrintCommand;
	}
	else
	  {
	    m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
	    m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
	    m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
	    m_persistPrintDlg.bDoCollate = m_bCollate;
	    m_persistPrintDlg.nCopies = m_nCopies;
	    m_persistPrintDlg.nFromPage = m_nFirstPage;
	    m_persistPrintDlg.nToPage = m_nLastPage;
	    
	    m_persistPrintDlg.colorSpace = m_cColorSpace;
	    
	    UT_cloneString(m_persistPrintDlg.szPrintCommand, m_szPrintCommand);
	  }
}

void XAP_UnixDialog_Print::useEnd(void)
{
	XAP_Dialog_Print::useEnd();

	m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
	m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
	m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
	m_persistPrintDlg.bDoCollate = m_bCollate;
	m_persistPrintDlg.nCopies = m_nCopies;
	m_persistPrintDlg.nFromPage = m_nFirstPage;
	m_persistPrintDlg.nToPage = m_nLastPage;

	m_persistPrintDlg.colorSpace = m_cColorSpace;
	
	FREEP(m_persistPrintDlg.szPrintCommand);
	UT_cloneString(m_persistPrintDlg.szPrintCommand, m_szPrintCommand);
}

GR_Graphics * XAP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	return m_pPSGraphics;
}

void XAP_UnixDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pPSGraphics);
	
	DELETEP(m_pPSGraphics);
}

/*****************************************************************/

void XAP_UnixDialog_Print::runModal(XAP_Frame * pFrame)
{
	m_pFrame = static_cast<XAP_Frame *>(pFrame);
	UT_ASSERT(m_pFrame);
	
#if 0
	// see if they just want the properties of the printer without
	// bothering the user.
	
	if (m_bPersistValid && m_bBypassActualDialog)
	{
		m_answer = a_OK;
		_getGraphics();
	}
	else
#endif
	{
		_raisePrintDialog(pFrame);		
		if (m_answer == a_OK)
			_getGraphics();
	}

	m_pFrame = NULL;
	return;
}

static void entry_toggle_enable (GtkWidget *checkbutton, GtkWidget *entry)
{
	gtk_widget_set_sensitive(entry, GTK_TOGGLE_BUTTON(checkbutton)->active);
	// give the proper entry widget focus
	gtk_widget_grab_focus (entry);
}


static const gchar* entry_text;

static gboolean entry_focus_in (GtkWidget *entry)
{
        entry_text = strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
        return 0;
}

static gboolean entry_focus_out (GtkWidget *entry, const void* ignore,
 GtkWidget *check)
{
        const gchar* tmp = gtk_entry_get_text(GTK_ENTRY(entry));
        if(strcmp(entry_text, tmp) != 0)
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 1);
        return 0;
}

void XAP_UnixDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	// raise the actual dialog and wait for an answer.
	// return true if they hit ok.
	GtkWidget *window = NULL;

	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *vbox;
	GtkWidget *hbox;

	GtkWidget *buttonPrint;
	GtkWidget *buttonFile;
	GtkWidget *buttonAll;
	GtkWidget *buttonRange;
	GtkWidget *buttonSelection;
	GtkWidget *buttonCollate;
	GtkWidget *buttonEmbedFonts;
	
	GtkWidget *spinCopies;

	GtkWidget *label;

	GtkWidget *entryPrint;
	GtkWidget *entryFrom;
	GtkWidget *entryTo;

	GtkWidget *radioBW;
	GtkWidget *radioGrayscale;
	GtkWidget *radioColor;
	
	GtkWidget *separator;
	GSList *group;

	// we get all our strings from the application string set
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	UT_ASSERT(pSS);
	
	window = abiDialogNew ("print dialog", TRUE, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_PrintTitle).utf8_str());

	// Add a main vbox
	vbox1 = GTK_DIALOG(window)->vbox;

	// Add a vbox to the main vbox
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, TRUE, FALSE, 0);
	gtk_widget_show (vbox2);

	// Print To label and radio buttons
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	label = gtk_label_new(pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_PrintTo).utf8_str());
	gtk_misc_set_padding (GTK_MISC (label), 5,5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	
	buttonPrint = gtk_radio_button_new_with_label (NULL, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_Printer).utf8_str());
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPrint), TRUE);
	gtk_box_pack_start (GTK_BOX (hbox), buttonPrint, FALSE, TRUE, 0);
	gtk_widget_show (buttonPrint);
	
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonPrint));
	
	buttonFile = gtk_radio_button_new_with_label(group, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_File).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), buttonFile, FALSE, TRUE, 0);
	gtk_widget_show (buttonFile);
	
	// Print Command Label and Text box
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	label = gtk_label_new(pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_PrinterCommand).utf8_str());
	gtk_misc_set_padding (GTK_MISC (label), 5,5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	
	entryPrint = gtk_entry_new();
	gtk_entry_set_max_length (GTK_ENTRY(entryPrint), 50);
	
	g_signal_connect(G_OBJECT(buttonPrint), "toggled",
			 G_CALLBACK(entry_toggle_enable), entryPrint);
	gtk_box_pack_start (GTK_BOX (hbox), entryPrint, TRUE, TRUE, 0);
	gtk_widget_show (entryPrint);
	
	// Now add the separator line
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), separator, FALSE, TRUE, 0);
	gtk_widget_show (separator);
	
	// Page range stuff
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox2), vbox, FALSE, TRUE, 0);
	gtk_widget_show (vbox);
	
	label = gtk_label_new(pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_PageRanges).utf8_str());
	gtk_misc_set_padding (GTK_MISC (label), 5,5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	
	buttonAll = gtk_radio_button_new_with_label (NULL, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_All).utf8_str());
	gtk_box_pack_start (GTK_BOX (vbox), buttonAll, FALSE, TRUE, 0);
	gtk_widget_show (buttonAll);
	
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonAll));
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);
	
	buttonRange = gtk_radio_button_new_with_label(group, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_From).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), buttonRange, FALSE, FALSE, 0);
	gtk_widget_show (buttonRange);
	
	entryFrom = gtk_entry_new();
	gtk_entry_set_max_length (GTK_ENTRY(entryFrom), 4);
	gtk_box_pack_start (GTK_BOX (hbox), entryFrom, TRUE, TRUE, 0);
	gtk_widget_show (entryFrom);
	
	g_signal_connect(G_OBJECT(entryFrom), "focus-in-event",
			 G_CALLBACK(entry_focus_in), NULL);
	g_signal_connect(G_OBJECT(entryFrom), "focus-out-event",
			 G_CALLBACK(entry_focus_out), buttonRange);
	
	label = gtk_label_new(pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_To).utf8_str());
	//gtk_misc_set_padding (GTK_MISC (label), 5,5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	entryTo = gtk_entry_new();
	gtk_entry_set_max_length (GTK_ENTRY(entryTo), 4);
	gtk_box_pack_start (GTK_BOX (hbox), entryTo, TRUE, TRUE, 0);
	gtk_widget_show (entryTo);
	
	g_signal_connect(G_OBJECT(entryTo), "focus-in-event",
			 G_CALLBACK(entry_focus_in), NULL);
	g_signal_connect(G_OBJECT(entryTo), "focus-out-event",
			 G_CALLBACK(entry_focus_out), buttonRange);
	
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonRange));
	
	buttonSelection = gtk_radio_button_new_with_label(group, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_Selection).utf8_str());
	gtk_box_pack_start (GTK_BOX (vbox), buttonSelection, FALSE, FALSE, 0);
	gtk_widget_show (buttonSelection);
	
	// Now add the separator line
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), separator, FALSE, TRUE, 0);
	gtk_widget_show (separator);
	
	// Add collate and copies options here
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);
	
	buttonCollate = gtk_check_button_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_Collate).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), buttonCollate, TRUE, TRUE, 0);
	gtk_widget_show (buttonCollate);
	
	buttonEmbedFonts = gtk_check_button_new_with_label (pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_EmbedFonts).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), buttonEmbedFonts, TRUE, TRUE, 0);
	gtk_widget_show (buttonEmbedFonts);
	
	label = gtk_label_new(pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_Copies).utf8_str());
	gtk_misc_set_padding (GTK_MISC (label), 5,5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	
	GtkObject * adjustment = gtk_adjustment_new(1, 1, 200, 1, 5, 0.0);
	spinCopies  = gtk_spin_button_new( GTK_ADJUSTMENT(adjustment), 1, 0 );
	//gtk_scale_set_digits(GTK_SCALE(spinCopies), 0);
	gtk_box_pack_start (GTK_BOX (hbox), spinCopies, FALSE, TRUE, 0);
	gtk_widget_show (spinCopies);
	
	// Now add the separator line
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), separator, FALSE, TRUE, 0);
	gtk_widget_show (separator);
	
	// Add print color options here
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);
	
	label = gtk_label_new(pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_PrintIn).utf8_str());
	gtk_misc_set_padding (GTK_MISC (label), 5,5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	
	radioBW = gtk_radio_button_new_with_label (NULL, pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_BlackWhite).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), radioBW, FALSE, TRUE, 0);
	gtk_widget_show (radioBW);
	
	radioGrayscale = gtk_radio_button_new_with_label(
							 gtk_radio_button_get_group(GTK_RADIO_BUTTON(radioBW)),
							 pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_Grayscale).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), radioGrayscale, FALSE, TRUE, 0);
	gtk_widget_show (radioGrayscale);
	
	radioColor = gtk_radio_button_new_with_label(
						     gtk_radio_button_get_group(GTK_RADIO_BUTTON(radioGrayscale)),
						     pSS->getValueUTF8(XAP_STRING_ID_DLG_UP_Color).utf8_str());
	gtk_box_pack_start (GTK_BOX (hbox), radioColor, FALSE, TRUE, 0);
	gtk_widget_show (radioColor);
	
	// append the buttons
	abiAddStockButton ( GTK_DIALOG(window), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
	abiAddStockButton ( GTK_DIALOG(window), GTK_STOCK_PRINT, BUTTON_PRINT ) ;
	
	// fill a little callback struct to hide some private data pointers in
	m_callbackData.entry = entryPrint;
	// BUGBUG This is wrong.  The parent frame should be this (print dialog), not
	// BUGBUG the document frame, else we can lose our new dialogs beneath each other.
	m_callbackData.frame = static_cast<XAP_Frame *>(m_pFrame);
	m_callbackData.answer = &m_answer;
	
	if (!m_bPersistValid)		// first time called
	  {
	    m_persistPrintDlg.bEnablePrintToFile = m_bEnablePrintToFile;
	    m_persistPrintDlg.bEnablePageRange = m_bEnablePageRange;
	    m_persistPrintDlg.bEnableSelection = m_bEnablePrintSelection;
	    m_persistPrintDlg.nFromPage = m_nFirstPage;
	    m_persistPrintDlg.nToPage = m_nLastPage;
	    // The first time through, grab the settings and set min and max for range checking
	    m_persistPrintDlg.nMinPage = m_nFirstPage;
	    m_persistPrintDlg.nMaxPage = m_nLastPage;
	    
	    m_persistPrintDlg.colorSpace = GR_Graphics::GR_COLORSPACE_COLOR;
	    
		 //UT_cloneString(m_persistPrintDlg.szPrintCommand, "lpr");
	  }
	// Force the print command to default to "lpr" every time the print dialog is invoked.
	// The mechanism for keeping persistant data seems to be broken.
	UT_cloneString(m_persistPrintDlg.szPrintCommand, "lpr");

	// Turn some widgets on or off based on settings
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPrint), !m_persistPrintDlg.bDoPrintToFile);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonFile), m_persistPrintDlg.bDoPrintToFile);
	gtk_widget_set_sensitive(buttonFile, m_persistPrintDlg.bEnablePrintToFile);
	
	gtk_widget_set_sensitive(entryPrint, GTK_TOGGLE_BUTTON(buttonPrint)->active);
	gtk_entry_set_text (GTK_ENTRY (entryPrint), m_persistPrintDlg.szPrintCommand);
	FREEP(m_persistPrintDlg.szPrintCommand);
	gtk_widget_set_sensitive(buttonRange, m_persistPrintDlg.bEnablePageRange);
	gtk_widget_set_sensitive(buttonSelection, m_persistPrintDlg.bEnableSelection);
	
	if (m_persistPrintDlg.bDoPageRange)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonRange), TRUE);
	else if (m_persistPrintDlg.bDoPrintSelection)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSelection), TRUE);
	else
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonAll), TRUE);
	
	switch (m_persistPrintDlg.colorSpace)
	  {
	  case GR_Graphics::GR_COLORSPACE_BW:
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioBW), TRUE);
	    break;
	  case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioGrayscale), TRUE);
	    break;
	  case GR_Graphics::GR_COLORSPACE_COLOR:
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioColor), TRUE);
	    break;
	  default:
	    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	  }
	
	char str[30];
	sprintf(str, "%d", m_persistPrintDlg.nFromPage);
	gtk_entry_set_text (GTK_ENTRY (entryFrom), str);
	sprintf(str, "%d", m_persistPrintDlg.nToPage);
	gtk_entry_set_text (GTK_ENTRY (entryTo), str);
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCollate), m_persistPrintDlg.bDoCollate);
	XAP_App::getApp()->getPrefsValueBool(static_cast<const XML_Char *>(XAP_PREF_KEY_EmbedFontsInPS), &m_bEmbedFonts);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonEmbedFonts), m_bEmbedFonts);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spinCopies), m_persistPrintDlg.nCopies);
	
	switch ( abiRunModalDialog ( GTK_DIALOG(window), pFrame, this, BUTTON_CANCEL, false ) )
	  {
	  case BUTTON_PRINT:
	    {
	      m_answer = XAP_Dialog_Print::a_OK;
	      break ;
	    }
	  default:
	    {
	      m_answer = XAP_Dialog_Print::a_CANCEL; 
	      break;
	    }
	  }

	if (m_answer == a_OK)
	{
		m_bDoPrintRange		= GTK_TOGGLE_BUTTON(buttonRange)->active;
		m_bDoPrintSelection = GTK_TOGGLE_BUTTON(buttonSelection)->active;
		m_bDoPrintToFile	= GTK_TOGGLE_BUTTON(buttonFile)->active;
		m_bCollate			= GTK_TOGGLE_BUTTON(buttonCollate)->active;
		bool bEmbedFonts 	= m_bEmbedFonts;
		m_bEmbedFonts		= GTK_TOGGLE_BUTTON(buttonEmbedFonts)->active;
		m_nCopies			= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinCopies));

		
		if(bEmbedFonts != m_bEmbedFonts)
		{
			XAP_Prefs * pPrefs = XAP_App::getApp()->getPrefs();
			UT_ASSERT(pPrefs);
			pPrefs->getCurrentScheme()->setValueBool(static_cast<const XML_Char *>(XAP_PREF_KEY_EmbedFontsInPS), m_bEmbedFonts);
		}
			
		// TODO check for valid entries

		if (m_bDoPrintRange)
		{
			UT_uint32 first = atoi(gtk_entry_get_text(GTK_ENTRY(entryFrom)));
			if (first < m_persistPrintDlg.nMinPage)
				first = m_persistPrintDlg.nMinPage;

			UT_uint32 last = atoi(gtk_entry_get_text(GTK_ENTRY(entryTo)));
			if (last > m_persistPrintDlg.nMaxPage)
				last = m_persistPrintDlg.nMaxPage;
			
			m_nFirstPage = UT_MIN(first,last);
			m_nLastPage = UT_MAX(first,last);
		}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioBW)))
			m_cColorSpace = GR_Graphics::GR_COLORSPACE_BW;
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioGrayscale)))
			m_cColorSpace = GR_Graphics::GR_COLORSPACE_GRAYSCALE;
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioColor)))
			m_cColorSpace = GR_Graphics::GR_COLORSPACE_COLOR;
		else
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		
		UT_cloneString(m_szPrintCommand, gtk_entry_get_text(GTK_ENTRY(entryPrint)));
	}

	abiDestroyWidget ( window ) ;

	return;
}

void XAP_UnixDialog_Print::_getGraphics(void)
{
	UT_ASSERT(m_answer == a_OK);

	XAP_App * app = m_pFrame->getApp();
	UT_ASSERT(app);
	
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (app);
	UT_ASSERT(unixapp);

#ifndef WITH_PANGO	
	XAP_UnixFontManager * fontmgr = unixapp->getFontManager();
	UT_ASSERT(fontmgr);
#else
	// TODO: proper implemetation required !!!
	XAP_UnixFontManager * fontmgr = NULL;
	UT_ASSERT(UT_NOT_IMPLEMENTED);
#endif
	
	if (m_bDoPrintToFile)
	{
		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pFrame,bufSuggestedName))
			goto Fail;

		m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname, m_szDocumentTitle,
										m_pFrame->getApp()->getApplicationName(),
										fontmgr,
										true, app);
	}
	else
	{		
		m_pPSGraphics = new PS_Graphics(m_szPrintCommand, m_szDocumentTitle,
										m_pFrame->getApp()->getApplicationName(),
										fontmgr,
										false, app);
	}
	FREEP(m_szPrintCommand);
	UT_ASSERT(m_pPSGraphics);

	// set the color mode
	m_pPSGraphics->setColorSpace(m_cColorSpace);
	m_pPSGraphics->setPageSize(m_pageSize);
	m_pPSGraphics->setPageCount(m_nLastPage - m_nFirstPage + 1);

	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}

