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

#include <gnome.h>

//#include <libgnomeprint/gnome-printer-dialog.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print-dialog.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_dialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_Print.h"
#include "xap_UnixGnomeDlg_Print.h"
#include "xap_UnixDlg_MessageBox.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_UnixPSGraphics.h"
#include "xap_Strings.h"
#include <gtk/gtk.h>
#include <glib.h>

XAP_Dialog * XAP_UnixGnomeDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_Print * p = new XAP_UnixGnomeDialog_Print(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_Print::XAP_UnixGnomeDialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_UnixDialog_Print(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_Print::~XAP_UnixGnomeDialog_Print(void)
{
}

void XAP_UnixGnomeDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	GnomePrintDialog *gpd;
	GnomePrinter *printer = NULL;
	int copies=1, collate=FALSE;
	int first=1, end = 0, range;

	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	UT_ASSERT(pSS);

	// There are four basic parts to _raisePrintDialog:
	//		1.  Create the dialog widget
	//		2.  Toggle dialog options to match persistent values
	// 		3.  Run dialog
	//		4.  Set new persistent values from dialog

	// 1.  Create the dialog widget
	gpd = (GnomePrintDialog *)gnome_print_dialog_new(
			_(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle)), 
			GNOME_PRINT_DIALOG_RANGE|GNOME_PRINT_DIALOG_COPIES);

	/* sorry about the ugly C-style cast -- ignore the "_Active Page" too */
	gnome_print_dialog_construct_range_page(gpd,
		GNOME_PRINT_RANGE_ALL| GNOME_PRINT_RANGE_RANGE | GNOME_PRINT_RANGE_SELECTION,
		m_nFirstPage, m_nLastPage,
		"_Active Page", (char *)pSS->getValue(XAP_STRING_ID_DLG_UP_PageRanges));

	gnome_dialog_set_default (GNOME_DIALOG (gpd),
				  GNOME_PRINT_PRINT);

	// get top level window and it's GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	gnome_dialog_set_parent(GNOME_DIALOG(gpd), GTK_WINDOW(parent));
	//centerDialog(parent, GTK_WIDGET(gpd));

	// don't set transient - this is a gnome-dialog

	// 2.  Toggle dialog options to match persistent values
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

	// TODO: We're not really persistant. I view this as a good thing, others don't.
	// Gnome Print really doesn't do persistance too well (limited accessor
	// functions), so we have to give something up in order to get the pretty
	// dialog (for now)

	// 3.  Run dialog
	switch( gnome_dialog_run(GNOME_DIALOG(gpd)) ) {
		case GNOME_PRINT_PRINT:
			break;
		case GNOME_PRINT_PREVIEW:
		  /* TODO: support Gnome Print-Preview */ 
		default:
			gnome_dialog_close(GNOME_DIALOG(gpd));
			m_answer = a_CANCEL;
			return;
	}

	// 4.  Set new persistent values from dialog
	// TODO

	gnome_print_dialog_get_copies(gpd, &copies, &collate);
	printer = gnome_print_dialog_get_printer (gpd);
	range = gnome_print_dialog_get_range_page(gpd, &first, &end);
	gnome_dialog_close(GNOME_DIALOG(gpd));

	// Record outputs
	m_bDoPrintRange				= (range == GNOME_PRINT_RANGE_RANGE);
	m_bDoPrintSelection			= (range == GNOME_PRINT_RANGE_SELECTION);
	m_bCollate				= collate;
	m_cColorSpace				= GR_Graphics::GR_COLORSPACE_BW;  //BUG
	m_nFirstPage				= first;
	m_nLastPage				= end;
	m_nCopies				= copies;
	m_answer 				= a_OK;

	/* hack - detect the pipe ('|') gnome print adds for printing to a non-file */
	m_bDoPrintToFile = *(printer->filename) != '|';
	
	if(m_bDoPrintToFile) 
	  {
	    /* postscript output to a file */
	    UT_cloneString(m_szPrintToFilePathname, printer->filename);
	    UT_cloneString(m_szPrintCommand, "");
	  }
	else
	  {
	    /* printing using lpr or similar */
	    UT_cloneString(m_szPrintCommand, printer->filename+1); /* hack to remove "|" from "|lpr" */
	    UT_cloneString(m_szPrintToFilePathname, "");
	  }

	UT_DEBUGMSG(("Printing to file: %d\n", m_bDoPrintToFile));
	UT_DEBUGMSG(("Print range: %d Selection: %d\n", m_bDoPrintRange, m_bDoPrintSelection));
	UT_DEBUGMSG(("Driver: %s Filename: %s\n", m_szPrintCommand, m_szPrintToFilePathname));

	return;
}

void XAP_UnixGnomeDialog_Print::_getGraphics(void)
{
	UT_ASSERT(m_answer == a_OK);

	XAP_App * app = m_pUnixFrame->getApp();
	UT_ASSERT(app);
	
	XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (app);
	UT_ASSERT(unixapp);

	XAP_UnixFontManager * fontmgr = unixapp->getFontManager();
	UT_ASSERT(fontmgr);
	
	if (m_bDoPrintToFile)
	{
		m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname, m_szDocumentTitle,
										m_pUnixFrame->getApp()->getApplicationName(),
										fontmgr,
										UT_TRUE, app);
	}
	else
	{		
		m_pPSGraphics = new PS_Graphics(m_szPrintCommand, m_szDocumentTitle,
										m_pUnixFrame->getApp()->getApplicationName(),
										fontmgr,
										UT_FALSE, app);
	}

	UT_ASSERT(m_pPSGraphics);

	// set the color mode
	m_pPSGraphics->setColorSpace(m_cColorSpace);
	
	m_answer = a_OK;
	return;
}
