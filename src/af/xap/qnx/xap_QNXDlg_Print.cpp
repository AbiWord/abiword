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
#include "xap_Dialog_Id.h"
#include "xap_QNXDlg_Print.h"
#include "xap_QNXDlg_MessageBox.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"
#include "xap_Strings.h"

/*****************************************************************/
#if 0
static void _notifyError_OKOnly(XAP_Frame * pFrame, const char * message)
{
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_MessageBox * pDialog
		= (XAP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage(message);
	pDialog->setButtons(XAP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(XAP_Dialog_MessageBox::a_OK);

	pDialog->runModal(pFrame);

//	XAP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);
}
#endif

/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_QNXDialog_Print * p = new XAP_QNXDialog_Print(pFactory,id);
	return p;
}

XAP_QNXDialog_Print::XAP_QNXDialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
#if 0
	memset(&m_persistPrintDlg, 0, sizeof(m_persistPrintDlg));
#endif
}

XAP_QNXDialog_Print::~XAP_QNXDialog_Print(void)
{
//	DELETEP(m_pPSGraphics);
}

void XAP_QNXDialog_Print::useStart(void)
{
#if 0
	XAP_Dialog_Print::useStart();

	if (m_bPersistValid)
	{
		m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
		m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
		m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
		m_persistPrintDlg.bDoCollate = m_bCollate;

		m_persistPrintDlg.colorSpace = m_cColorSpace;
	}
#endif
}

void XAP_QNXDialog_Print::useEnd(void)
{
#if 0
	XAP_Dialog_Print::useEnd();

	m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
	m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
	m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
	m_persistPrintDlg.bDoCollate = m_bCollate;
	m_persistPrintDlg.nCopies = m_nCopies;
	m_persistPrintDlg.nFromPage = m_nFirstPage;
	m_persistPrintDlg.nToPage = m_nLastPage;

	m_persistPrintDlg.colorSpace = m_cColorSpace;
	
	UT_cloneString(m_persistPrintDlg.szPrintCommand, m_szPrintCommand);
#endif
}

GR_Graphics * XAP_QNXDialog_Print::getPrinterGraphicsContext(void)
{
#if 0
	UT_ASSERT(m_answer == a_OK);

	return m_pPSGraphics;
#endif
	return(NULL);
}

void XAP_QNXDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
#if 0
	UT_ASSERT(pGraphics == m_pPSGraphics);
	
	DELETEP(m_pPSGraphics);
#endif
}

/*****************************************************************/

void XAP_QNXDialog_Print::runModal(XAP_Frame * pFrame)
{
#if 0
	m_pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(m_pQNXFrame);
	
	// see if they just want the properties of the printer without
	// bothering the user.
	
	if (m_bPersistValid && m_bBypassActualDialog)
	{
		m_answer = a_OK;
		_getGraphics();
	}
	else
	{
		_raisePrintDialog(pFrame);		
		if (m_answer == a_OK)
			_getGraphics();
	}

	m_pQNXFrame = NULL;
	return;
#endif
}

#if 0
static void s_ok_clicked(GtkWidget * /* widget */,
						 printCBStruct * data)
{
	UT_ASSERT(data && data->entry && data->frame && data->answer);
	
	// this callback doesn't end the dialog loop unless the string is valid
	// check for command
	gchar * string = gtk_entry_get_text(GTK_ENTRY(data->entry));
	if (! (string && *string))
	{
		// construct an error message box
		const XAP_StringSet * pSS = data->frame->getApp()->getStringSet();
		UT_ASSERT(pSS);
		_notifyError_OKOnly(data->frame, pSS->getValue(XAP_STRING_ID_DLG_UP_InvalidPrintString));
		return;
	}

	*data->answer = XAP_Dialog_Print::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * /* widget */,
							 XAP_Dialog_Print::tAnswer * answer)
{
	*answer = XAP_Dialog_Print::a_CANCEL;
	gtk_main_quit();
}

static void s_delete_clicked(GtkWidget * /* widget */,
							 gpointer /* data */,
							 XAP_Dialog_Print::tAnswer * answer)
{
	*answer = XAP_Dialog_Print::a_CANCEL;
	gtk_main_quit();
}

static void entry_toggle_enable (GtkWidget *checkbutton, GtkWidget *entry)
{
	gtk_widget_set_sensitive(entry, GTK_TOGGLE_BUTTON(checkbutton)->active);
	// give the proper entry widget focus
	gtk_widget_grab_focus (entry);
}

void XAP_QNXDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
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
	GtkWidget *button;
	GtkWidget *buttonCollate;
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
	
	// Create window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect_after(GTK_OBJECT(window),
							  "destroy",
							  NULL,
							  NULL);
	gtk_signal_connect_after(GTK_OBJECT(window),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (void *) &m_answer);
	
	gtk_window_set_title (GTK_WINDOW (window), pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle));
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);
	gtk_widget_set_usize (window, 325, 275);

	// Add a main vbox
	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox1);
	gtk_widget_show (vbox1);

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

			label = gtk_label_new(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTo));
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			buttonPrint = gtk_radio_button_new_with_label (NULL, pSS->getValue(XAP_STRING_ID_DLG_UP_Printer));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPrint), TRUE);
			gtk_box_pack_start (GTK_BOX (hbox), buttonPrint, FALSE, TRUE, 0);
			gtk_widget_show (buttonPrint);

			group = gtk_radio_button_group (GTK_RADIO_BUTTON (buttonPrint));

			buttonFile = gtk_radio_button_new_with_label(group, pSS->getValue(XAP_STRING_ID_DLG_UP_File));
			gtk_box_pack_start (GTK_BOX (hbox), buttonFile, FALSE, TRUE, 0);
			gtk_widget_show (buttonFile);

		// Print Command Label and Text box
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
		gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, TRUE, 0);
		gtk_widget_show (hbox);

			label = gtk_label_new(pSS->getValue(XAP_STRING_ID_DLG_UP_PrinterCommand));
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			entryPrint = gtk_entry_new_with_max_length (50);
			
			gtk_signal_connect(GTK_OBJECT(buttonPrint), "toggled",
							GTK_SIGNAL_FUNC(entry_toggle_enable), entryPrint);
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

			label = gtk_label_new(pSS->getValue(XAP_STRING_ID_DLG_UP_PageRanges));
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			buttonAll = gtk_radio_button_new_with_label (NULL, pSS->getValue(XAP_STRING_ID_DLG_UP_All));
			gtk_box_pack_start (GTK_BOX (vbox), buttonAll, FALSE, TRUE, 0);
			gtk_widget_show (buttonAll);

			group = gtk_radio_button_group (GTK_RADIO_BUTTON (buttonAll));

			hbox = gtk_hbox_new (FALSE, 0);
			gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
			gtk_widget_show (hbox);

				buttonRange = gtk_radio_button_new_with_label(group, pSS->getValue(XAP_STRING_ID_DLG_UP_From));
				gtk_box_pack_start (GTK_BOX (hbox), buttonRange, FALSE, FALSE, 0);
				gtk_widget_show (buttonRange);

				entryFrom = gtk_entry_new_with_max_length (4);
				gtk_box_pack_start (GTK_BOX (hbox), entryFrom, TRUE, TRUE, 0);
				gtk_widget_show (entryFrom);

				label = gtk_label_new(pSS->getValue(XAP_STRING_ID_DLG_UP_To));
				//gtk_misc_set_padding (GTK_MISC (label), 5,5);
				gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
				gtk_widget_show (label);

				entryTo = gtk_entry_new_with_max_length (4);
				gtk_box_pack_start (GTK_BOX (hbox), entryTo, TRUE, TRUE, 0);
				gtk_widget_show (entryTo);

				group = gtk_radio_button_group (GTK_RADIO_BUTTON (buttonRange));

			buttonSelection = gtk_radio_button_new_with_label(group, pSS->getValue(XAP_STRING_ID_DLG_UP_Selection));
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

			buttonCollate = gtk_check_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_UP_Collate));
			gtk_box_pack_start (GTK_BOX (hbox), buttonCollate, TRUE, TRUE, 0);
			gtk_widget_show (buttonCollate);

			label = gtk_label_new(pSS->getValue(XAP_STRING_ID_DLG_UP_Copies));
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
			gtk_widget_show (label);

			GtkObject * adjustment = gtk_adjustment_new(1, 1, 50, 1, 5, 0.0);
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

			label = gtk_label_new(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintIn));
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			radioBW = gtk_radio_button_new_with_label (NULL, pSS->getValue(XAP_STRING_ID_DLG_UP_BlackWhite));
			gtk_box_pack_start (GTK_BOX (hbox), radioBW, FALSE, TRUE, 0);
			gtk_widget_show (radioBW);

			radioGrayscale = gtk_radio_button_new_with_label(
				gtk_radio_button_group(GTK_RADIO_BUTTON(radioBW)),
				pSS->getValue(XAP_STRING_ID_DLG_UP_Grayscale));
			gtk_box_pack_start (GTK_BOX (hbox), radioGrayscale, FALSE, TRUE, 0);
			gtk_widget_show (radioGrayscale);

			radioColor = gtk_radio_button_new_with_label(
				gtk_radio_button_group(GTK_RADIO_BUTTON(radioGrayscale)),
				pSS->getValue(XAP_STRING_ID_DLG_UP_Color));
			gtk_box_pack_start (GTK_BOX (hbox), radioColor, FALSE, TRUE, 0);
			gtk_widget_show (radioColor);
			
	// Now add the separator line
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), separator, FALSE, TRUE, 0);
	gtk_widget_show (separator);

		// Button area
		hbox = gtk_hbox_new (FALSE, 5);
		gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
		gtk_box_pack_end (GTK_BOX (vbox1), hbox, FALSE, TRUE, 0);
		gtk_widget_show (hbox);

			button = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
			gtk_signal_connect (GTK_OBJECT (button), "clicked",
							GTK_SIGNAL_FUNC(s_cancel_clicked), &m_answer);
			gtk_box_pack_end (GTK_BOX (hbox), button, TRUE, TRUE, 5);
			//GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
			//gtk_widget_grab_default (button);
			gtk_widget_show (button);

			// fill a little callback struct to hide some private data pointers in
			m_callbackData.entry = entryPrint;
			// BUGBUG This is wrong.  The parent frame should be this (print dialog), not
			// BUGBUG the document frame, else we can lose our new dialogs beneath each other.
			m_callbackData.frame = (XAP_Frame *) m_pQNXFrame;
			m_callbackData.answer = &m_answer;
			
			button = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_UP_PrintButton));
			gtk_signal_connect (GTK_OBJECT (button), "clicked",
							GTK_SIGNAL_FUNC(s_ok_clicked), &m_callbackData);
			
			gtk_box_pack_end (GTK_BOX (hbox), button, TRUE, TRUE, 5);
			//gtk_widget_grab_default (button);
			gtk_widget_show (button);


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
			
			UT_cloneString(m_persistPrintDlg.szPrintCommand, "lpr");
		}

		// Turn some widgets on or off based on settings
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPrint), !m_persistPrintDlg.bDoPrintToFile);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonFile), m_persistPrintDlg.bDoPrintToFile);
		gtk_widget_set_sensitive(buttonFile, m_persistPrintDlg.bEnablePrintToFile);

		gtk_widget_set_sensitive(entryPrint, GTK_TOGGLE_BUTTON(buttonPrint)->active);
		gtk_entry_set_text (GTK_ENTRY (entryPrint), m_persistPrintDlg.szPrintCommand);

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
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (radioBW), TRUE);
			break;
		case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (radioGrayscale), TRUE);
			break;
		case GR_Graphics::GR_COLORSPACE_COLOR:
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (radioColor), TRUE);
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
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(spinCopies), m_persistPrintDlg.nCopies);


	// get top level window and it's GtkWidget *
	XAP_QNXFrame * frame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	// center it
    centerDialog(parent, window);
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
	
	gtk_widget_show (window);

	gtk_main();

	//m_pQNXFrame = NULL;

	if (m_answer == a_OK)
	{
		m_bDoPrintRange		= GTK_TOGGLE_BUTTON(buttonRange)->active;
		m_bDoPrintSelection = GTK_TOGGLE_BUTTON(buttonSelection)->active;
		m_bDoPrintToFile	= GTK_TOGGLE_BUTTON(buttonFile)->active;
		m_bCollate			= GTK_TOGGLE_BUTTON(buttonCollate)->active;
		m_nCopies			= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinCopies));

		// TODO check for valid entries

		if (m_bDoPrintRange)
		{
			UT_uint32 first = atoi(gtk_entry_get_text(GTK_ENTRY(entryFrom)));
			if (first < m_persistPrintDlg.nMinPage)
				first = m_persistPrintDlg.nMinPage;

			UT_uint32 last = atoi(gtk_entry_get_text(GTK_ENTRY(entryTo)));
			if (last > m_persistPrintDlg.nMaxPage)
				last = m_persistPrintDlg.nMaxPage;
			
			m_nFirstPage = MyMin(first,last);
			m_nLastPage = MyMax(first,last);
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

	// destroy the widgets
	gtk_widget_destroy (window);

	return;
}

void XAP_QNXDialog_Print::_getGraphics(void)
{
	UT_ASSERT(m_answer == a_OK);

	XAP_App * app = m_pQNXFrame->getApp();
	UT_ASSERT(app);
	
	XAP_QNXApp * unixapp = static_cast<XAP_QNXApp *> (app);
	UT_ASSERT(unixapp);

	void * fontmgr = unixapp->getFontManager();
	UT_ASSERT(fontmgr);
	
	if (m_bDoPrintToFile)
	{
		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pQNXFrame,bufSuggestedName))
			goto Fail;

		m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname, m_szDocumentTitle,
										m_pQNXFrame->getApp()->getApplicationName(),
										fontmgr,
										UT_TRUE);
	}
	else
	{		
		m_pPSGraphics = new PS_Graphics(m_szPrintCommand, m_szDocumentTitle,
										m_pQNXFrame->getApp()->getApplicationName(),
										fontmgr,
										UT_FALSE);
	}

	UT_ASSERT(m_pPSGraphics);

	// set the color mode
	m_pPSGraphics->setColorSpace(m_cColorSpace);
	
	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}
#endif
