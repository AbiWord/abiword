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

#include "xap_UnixGnomePrintGraphics.h"

extern "C" {
#include <libgnomeprint/gnome-print-dialog.h>
}

XAP_Dialog * XAP_UnixGnomeDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
							   XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_Print * p = new XAP_UnixGnomeDialog_Print(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_Print::XAP_UnixGnomeDialog_Print(XAP_DialogFactory * pDlgFactory,
						     XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_Print::~XAP_UnixGnomeDialog_Print(void)
{
        // nothing
}


void XAP_UnixGnomeDialog_Print::useStart(void)
{
	XAP_Dialog_Print::useStart();
}

void XAP_UnixGnomeDialog_Print::useEnd(void)
{
	XAP_Dialog_Print::useEnd();
}

GR_Graphics * XAP_UnixGnomeDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	return m_pGnomePrintGraphics;
}

void XAP_UnixGnomeDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pGnomePrintGraphics);
	
	DELETEP(m_pGnomePrintGraphics);
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixGnomeDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	GnomePrintDialog *gpd;
	GnomePrinter *printer = NULL;
	int copies=1, collate=FALSE;
	int first=1, end = 0, range;

	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();
	UT_ASSERT(pSS);

	// There are three basic parts to _raisePrintDialog:
	//		1.  Create the dialog widget
	//		2.  Toggle dialog options to match persistent values
	// 		3.  Run dialog

	// 1.  Create the dialog widget
	gpd = (GnomePrintDialog *)gnome_print_dialog_new(
			_(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle)), 
			GNOME_PRINT_DIALOG_RANGE|GNOME_PRINT_DIALOG_COPIES);

	/* sorry about the ugly C-style cast -- ignore the "_Active Page" too */
	gnome_print_dialog_construct_range_page(gpd,
		GNOME_PRINT_RANGE_ALL| GNOME_PRINT_RANGE_RANGE | GNOME_PRINT_RANGE_SELECTION,
		m_nFirstPage, m_nLastPage,
		"_Active Page", (char *)pSS->getValue(XAP_STRING_ID_DLG_UP_PageRanges));

	setDefaultButton (GNOME_DIALOG (gpd),
			  GNOME_PRINT_PRINT);

	// get top level window and it's GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	//gnome_dialog_set_parent(GNOME_DIALOG(gpd), GTK_WINDOW(parent));
	centerDialog(parent, GTK_WIDGET(gpd));

	// 2.  Toggle dialog options to match persistent values
	// TODO: We're not really persistant. I view this as a good thing, others don't.
	// Gnome Print really doesn't do persistance too well (limited accessor
	// functions), so we have to give something up in order to get the pretty
	// dialog (for now)

	bool preview = false;

	// 3.  Run dialog
	switch( gnome_dialog_run(GNOME_DIALOG(gpd)) ) {
		case GNOME_PRINT_PRINT:
		  break;
		case GNOME_PRINT_PREVIEW:
		  preview = true;
		  break;
		default:
		  gnome_dialog_close(GNOME_DIALOG(gpd));
		  m_answer = a_CANCEL;
		  return;
	}

	m_bIsPreview = preview;

	gnome_print_dialog_get_copies(gpd, &copies, &collate);
	printer = gnome_print_dialog_get_printer (gpd);
	range = gnome_print_dialog_get_range_page(gpd, &first, &end);
	gnome_dialog_close(GNOME_DIALOG(gpd));

	m_gpm = gnome_print_master_new();
	if (printer)
		gnome_print_master_set_printer(m_gpm, printer);

	// Record outputs
	m_bDoPrintRange				= (range == GNOME_PRINT_RANGE_RANGE);
	m_bDoPrintSelection			= (range == GNOME_PRINT_RANGE_SELECTION);
	m_cColorSpace				= GR_Graphics::GR_COLORSPACE_COLOR;  //BUG
	
	if(m_bDoPrintRange)
	  {
	    m_nFirstPage		        = MIN(first, end);
	    m_nLastPage				= MAX(first, end);
	  }

#if 0
	// either handle things ourself
	m_bCollate				= collate;
	m_nCopies				= copies;
#else
	// or (smartly?) let gnome-print handle it
	// will this work with our printing structure?
	m_bCollate = false;
	m_nCopies  = 1;
	gnome_print_master_set_copies(m_gpm, copies, collate);
#endif

	m_answer 				= a_OK;

	UT_DEBUGMSG(("Gnome Print: Copies: %d Collate: %d Start: %d End: %d\n", m_nCopies, m_bCollate, m_nFirstPage, m_nLastPage));

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
	
	m_pGnomePrintGraphics = new XAP_UnixGnomePrintGraphics(m_gpm,
							       m_pageSize,
						       fontmgr,
						       unixapp,
						       m_bIsPreview);

	UT_ASSERT(m_pGnomePrintGraphics);

	// set the color mode
	m_pGnomePrintGraphics->setColorSpace(m_cColorSpace);
	
	m_answer = a_OK;
	return;
}

void XAP_UnixGnomeDialog_Print::runModal(XAP_Frame * pFrame) 
{
       m_pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
       UT_ASSERT(m_pUnixFrame);
       
       // TODO: persistance

       _raisePrintDialog(pFrame);              
       if (m_answer == a_OK)
         _getGraphics();

       m_pUnixFrame = NULL;
       return;
}
