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
#include <fstream.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixDialog_Print.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ps_Graphics.h"
#include <gtk/gtk.h>
#include <glib.h>

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)
#define MyMin(a,b)	(((a)<(b)) ? (a) : (b))
#define MyMax(a,b)	(((a)>(b)) ? (b) : (a))

/*****************************************************************/
AP_Dialog * AP_UnixDialog_Print::static_constructor(AP_DialogFactory * pFactory,
													 AP_Dialog_Id id)
{
	AP_UnixDialog_Print * p = new AP_UnixDialog_Print(pFactory,id);
	return p;
}

AP_UnixDialog_Print::AP_UnixDialog_Print(AP_DialogFactory * pDlgFactory,
										   AP_Dialog_Id id)
	: AP_Dialog_Print(pDlgFactory,id)
{
	memset(&m_persistPrintDlg, 0, sizeof(m_persistPrintDlg));
}

AP_UnixDialog_Print::~AP_UnixDialog_Print(void)
{
//	DELETEP(m_pPSGraphics);
}

void AP_UnixDialog_Print::useStart(void)
{
	AP_Dialog_Print::useStart();

	if (m_bPersistValid)
	{
		m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
		m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
		m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
		m_persistPrintDlg.bDoCollate = m_bCollate;
	}
}

void AP_UnixDialog_Print::useEnd(void)
{
	AP_Dialog_Print::useEnd();

	m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
	m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
	m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
	m_persistPrintDlg.bDoCollate = m_bCollate;
	m_persistPrintDlg.nCopies = m_nCopies;
	m_persistPrintDlg.nFromPage = m_nFirstPage;
	m_persistPrintDlg.nToPage = m_nLastPage;

	UT_cloneString(m_persistPrintDlg.szPrintCommand, m_szPrintCommand);
}

DG_Graphics * AP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	return m_pPSGraphics;
}

void AP_UnixDialog_Print::releasePrinterGraphicsContext(DG_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pPSGraphics);
	
	DELETEP(m_pPSGraphics);
}

/*****************************************************************/

// this should probably go in a base class, but the Unix dialogs don't inherit
// from a common Unix dialog base class.  That kinda sucks.
void AP_UnixDialog_Print::_centerWindow(AP_Frame * parent, GtkWidget * child)
{
	UT_ASSERT(parent);
	UT_ASSERT(child);
	
	AP_UnixFrame * frame = static_cast<AP_UnixFrame *>(parent);
	UT_ASSERT(frame);
	
	// parent frame's geometry
	GtkWidget * topLevelWindow = frame->getTopLevelWindow();
	UT_ASSERT(topLevelWindow);
	UT_ASSERT(topLevelWindow->window);
	gint parentx = 0;
	gint parenty = 0;
	gint parentwidth = 0;
	gint parentheight = 0;
	gdk_window_get_origin(topLevelWindow->window, &parentx, &parenty);
	gdk_window_get_size(topLevelWindow->window, &parentwidth, &parentheight);
	UT_ASSERT(parentwidth > 0 && parentheight > 0);

	// this message box's geometry (it won't have a ->window yet, so don't assert it)
	gint width = 0;
	gint height = 0;
	gtk_widget_size_request(child, &child->requisition);
	width = child->requisition.width;
	height = child->requisition.height;
	UT_ASSERT(width > 0 && height > 0);

	// set new place
	gint newx = parentx + ((parentwidth - width) / 2);
	gint newy = parenty + ((parentheight - height) / 2);

	// measure the root window
	gint rootwidth = gdk_screen_width();
	gint rootheight = gdk_screen_height();
	// if the dialog won't fit on the screen, panic and center on the root window
	if ((newx + width) > rootwidth || (newy + height) > rootheight)
		gtk_window_position(GTK_WINDOW(child), GTK_WIN_POS_CENTER);
	else
		gtk_widget_set_uposition(child, newx, newy);
}

void AP_UnixDialog_Print::runModal(AP_Frame * pFrame)
{
	m_pUnixFrame = static_cast<AP_UnixFrame *>(pFrame);
	UT_ASSERT(m_pUnixFrame);
	
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

	m_pUnixFrame = NULL;
	return;
}

static void s_ok_clicked(GtkWidget * widget,
						 AP_Dialog_Print::tAnswer * answer)
{
	*answer = AP_Dialog_Print::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * widget,
							 AP_Dialog_Print::tAnswer * answer)
{
	*answer = AP_Dialog_Print::a_CANCEL;
	gtk_main_quit();
}

static void entry_toggle_enable (GtkWidget *checkbutton, GtkWidget *entry)
{
	gtk_widget_set_sensitive(entry, GTK_TOGGLE_BUTTON(checkbutton)->active);
	// give the proper entry widget focus
	gtk_widget_grab_focus (entry);
}

void AP_UnixDialog_Print::_raisePrintDialog(AP_Frame * pFrame)
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

	GtkWidget *separator;
	GSList *group;

	// Create window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect_after (GTK_OBJECT (window), "destroy",
							  GTK_SIGNAL_FUNC(s_cancel_clicked), NULL);
	gtk_window_set_title (GTK_WINDOW (window), "Printer Setup");
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

			label = gtk_label_new("Print To: ");
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			buttonPrint = gtk_radio_button_new_with_label (NULL, "Printer");
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonPrint), TRUE);
			gtk_box_pack_start (GTK_BOX (hbox), buttonPrint, FALSE, TRUE, 0);
			gtk_widget_show (buttonPrint);

			group = gtk_radio_button_group (GTK_RADIO_BUTTON (buttonPrint));

			buttonFile = gtk_radio_button_new_with_label(group, "File");
			gtk_box_pack_start (GTK_BOX (hbox), buttonFile, FALSE, TRUE, 0);
			gtk_widget_show (buttonFile);

		// Print Command Label and Text box
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
		gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, TRUE, 0);
		gtk_widget_show (hbox);

			label = gtk_label_new("Printer Command: ");
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

			label = gtk_label_new("Page Ranges: ");
			gtk_misc_set_padding (GTK_MISC (label), 5,5);
			gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			buttonAll = gtk_radio_button_new_with_label (NULL, "All");
			gtk_box_pack_start (GTK_BOX (vbox), buttonAll, FALSE, TRUE, 0);
			gtk_widget_show (buttonAll);

			group = gtk_radio_button_group (GTK_RADIO_BUTTON (buttonAll));

			hbox = gtk_hbox_new (FALSE, 0);
			gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
			gtk_widget_show (hbox);

				buttonRange = gtk_radio_button_new_with_label(group, "From:");
				gtk_box_pack_start (GTK_BOX (hbox), buttonRange, FALSE, FALSE, 0);
				gtk_widget_show (buttonRange);

				entryFrom = gtk_entry_new_with_max_length (4);
				gtk_box_pack_start (GTK_BOX (hbox), entryFrom, TRUE, TRUE, 0);
				gtk_widget_show (entryFrom);

				label = gtk_label_new("To: ");
				//gtk_misc_set_padding (GTK_MISC (label), 5,5);
				gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
				gtk_widget_show (label);

				entryTo = gtk_entry_new_with_max_length (4);
				gtk_box_pack_start (GTK_BOX (hbox), entryTo, TRUE, TRUE, 0);
				gtk_widget_show (entryTo);

				group = gtk_radio_button_group (GTK_RADIO_BUTTON (buttonRange));

			buttonSelection = gtk_radio_button_new_with_label(group, "Selection");
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

			buttonCollate = gtk_check_button_new_with_label ("Collate");
			gtk_box_pack_start (GTK_BOX (hbox), buttonCollate, TRUE, TRUE, 0);
			gtk_widget_show (buttonCollate);

			label = gtk_label_new("Copies: ");
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
	gtk_box_pack_start (GTK_BOX (vbox1), separator, FALSE, TRUE, 0);
	gtk_widget_show (separator);

		// Button area
		hbox = gtk_hbox_new (FALSE, 5);
		gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
		gtk_box_pack_end (GTK_BOX (vbox1), hbox, FALSE, TRUE, 0);
		gtk_widget_show (hbox);

			button = gtk_button_new_with_label ("Cancel");
			gtk_signal_connect (GTK_OBJECT (button), "clicked",
							GTK_SIGNAL_FUNC(s_cancel_clicked), &m_answer);
			gtk_box_pack_end (GTK_BOX (hbox), button, TRUE, TRUE, 5);
			//GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
			//gtk_widget_grab_default (button);
			gtk_widget_show (button);

			button = gtk_button_new_with_label ("Print");
			gtk_signal_connect (GTK_OBJECT (button), "clicked",
							GTK_SIGNAL_FUNC(s_ok_clicked), &m_answer);
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
			UT_cloneString(m_persistPrintDlg.szPrintCommand, "lpr");
		}

		// Turn some widgets on or off based on settings
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonPrint), !m_persistPrintDlg.bDoPrintToFile);
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonFile), m_persistPrintDlg.bDoPrintToFile);
		gtk_widget_set_sensitive(buttonFile, m_persistPrintDlg.bEnablePrintToFile);

		gtk_widget_set_sensitive(entryPrint, GTK_TOGGLE_BUTTON(buttonPrint)->active);
		gtk_entry_set_text (GTK_ENTRY (entryPrint), m_persistPrintDlg.szPrintCommand);

		gtk_widget_set_sensitive(buttonRange, m_persistPrintDlg.bEnablePageRange);
		gtk_widget_set_sensitive(buttonSelection, m_persistPrintDlg.bEnableSelection);

		if (m_persistPrintDlg.bDoPageRange)
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonRange), TRUE);
		else if (m_persistPrintDlg.bDoPrintSelection)
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonSelection), TRUE);
		else
			gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonAll), TRUE);


		char str[30];
		sprintf(str, "%ld", m_persistPrintDlg.nFromPage);
		gtk_entry_set_text (GTK_ENTRY (entryFrom), str);
		sprintf(str, "%ld", m_persistPrintDlg.nToPage);
		gtk_entry_set_text (GTK_ENTRY (entryTo), str);

		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (buttonCollate), m_persistPrintDlg.bDoCollate);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(spinCopies), m_persistPrintDlg.nCopies);


	_centerWindow(pFrame, GTK_WIDGET(window));
	gtk_widget_show (window);

	gtk_main();

	//m_pUnixFrame = NULL;

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

		UT_cloneString(m_szPrintCommand, gtk_entry_get_text(GTK_ENTRY(entryPrint)));
	}
	// destroy the widgets.

//	gtk_widget_destroy (vbox1);
//	gtk_widget_destroy (vbox2);
//	gtk_widget_destroy (vbox);
//	gtk_widget_destroy (hbox);
	gtk_widget_destroy (buttonPrint);
	gtk_widget_destroy (buttonFile);
	gtk_widget_destroy (buttonAll);
	gtk_widget_destroy (buttonRange);
	gtk_widget_destroy (buttonSelection);
//	gtk_widget_destroy (button);
	gtk_widget_destroy (buttonCollate);
	gtk_widget_destroy (spinCopies);
//	gtk_widget_destroy (label);
	gtk_widget_destroy (entryPrint);
	gtk_widget_destroy (entryFrom);
	gtk_widget_destroy (entryTo);
//	gtk_widget_destroy (separator);
	gtk_widget_destroy (window);

	return;
}

void AP_UnixDialog_Print::_getGraphics(void)
{
	UT_ASSERT(m_answer == a_OK);
	
	if (m_bDoPrintToFile)
	{
		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pUnixFrame,bufSuggestedName))
			goto Fail;

		m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname, m_szDocumentTitle,
										m_pUnixFrame->getApp()->getApplicationName(),
										UT_TRUE);
	}
	else
	{
		// TODO use a POPEN style constructor to get the graphics....
		
		m_pPSGraphics = new PS_Graphics(m_szPrintCommand, m_szDocumentTitle,
										m_pUnixFrame->getApp()->getApplicationName(),
										UT_FALSE);
	}

	UT_ASSERT(m_pPSGraphics);
	
	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}

