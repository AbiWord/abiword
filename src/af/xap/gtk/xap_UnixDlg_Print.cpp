
/* AbiSource Application Framework
 * Copyright (C) 2003 Dom Lachowicz
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

#include "xap_UnixDlg_Print.h"

#include <gtk/gtk.h>
#include <libgnomeprintui/gnome-print-dialog.h>

#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_Frame.h"
#include "ut_misc.h"

#include "gr_UnixPangoGraphics.h"

#include "fv_View.h"

XAP_Dialog * XAP_UnixDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
														   XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_Print(pFactory,id);
}

XAP_UnixDialog_Print::XAP_UnixDialog_Print(XAP_DialogFactory * pDlgFactory,
													 XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id),
	  m_pPrintGraphics (NULL),
	  m_bIsPreview(false),
	  m_bPdfWorkAround(false)
{
}

XAP_UnixDialog_Print::~XAP_UnixDialog_Print(void)
{
}


void XAP_UnixDialog_Print::useStart(void)
{
	XAP_Dialog_Print::useStart();
}

void XAP_UnixDialog_Print::useEnd(void)
{
	XAP_Dialog_Print::useEnd();
}

GR_Graphics * XAP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);
	return m_pPrintGraphics;
}

void XAP_UnixDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pPrintGraphics);	
	DELETEP(m_pPrintGraphics);
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	GtkWidget *gpd;
	int copies = 1, collate = FALSE;
	int first = 1, end = 0, range;

	double mrgnTop, mrgnBottom, mrgnLeft, mrgnRight, width, height;
	bool portrait;

	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());

	mrgnTop = pView->getPageSize().MarginTop(DIM_MM);
	mrgnBottom = pView->getPageSize().MarginBottom(DIM_MM);
	mrgnLeft = pView->getPageSize().MarginLeft(DIM_MM);
	mrgnRight = pView->getPageSize().MarginRight(DIM_MM);

	portrait = pView->getPageSize().isPortrait();
	m_bPdfWorkAround = false;		
		
	width = pView->getPageSize().Width (DIM_MM);
	height = pView->getPageSize().Height (DIM_MM);

	GnomePrintJob * job =
	    gnome_print_job_new(GR_UnixPangoPrintGraphics::s_setup_config (
				    mrgnTop, mrgnBottom, mrgnLeft, mrgnRight,
				    width, height, copies, portrait));

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	// 1.  Create the dialog widget
	gpd = gnome_print_dialog_new (job,
				      reinterpret_cast<const guchar *>(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle)),
				      GNOME_PRINT_DIALOG_RANGE|GNOME_PRINT_DIALOG_COPIES);
	GnomePrintConfig * cfg = gnome_print_job_get_config (job);

	/* sorry about the ugly C-style cast -- ignore the "_Active Page" too */
	gnome_print_dialog_construct_range_page(GNOME_PRINT_DIALOG(gpd),
											GNOME_PRINT_RANGE_ALL| GNOME_PRINT_RANGE_RANGE | GNOME_PRINT_RANGE_SELECTION,
											m_nFirstPage, m_nLastPage,
											reinterpret_cast<const guchar *>("_Active Page"), reinterpret_cast<const guchar *>(pSS->getValue(XAP_STRING_ID_DLG_UP_PageRanges)));


	switch (abiRunModalDialog (GTK_DIALOG(gpd), pFrame, this, GNOME_PRINT_DIALOG_RESPONSE_PRINT, false))
	{
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW: 
		m_bIsPreview = true;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT: 
		m_bIsPreview = false;
		break;
	default:
		abiDestroyWidget (gpd); 
		m_answer = a_CANCEL; 
		return;
	}
	const char * szBackend = reinterpret_cast<const char *>(gnome_print_config_get(cfg,reinterpret_cast<const guchar*>("Printer")));
	UT_DEBUGMSG(("Backend is %s \n",szBackend));
	if(!portrait && !m_bIsPreview)
	  {
	    if(strcmp(szBackend,"PDF")== 0)
	      {
		UT_DEBUGMSG(("Doing pdf workaround \n"));
		const GnomePrintUnit *unit =
		gnome_print_unit_get_by_abbreviation (reinterpret_cast<const guchar*>("mm"));
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_WIDTH), width, unit);
		gnome_print_config_set_length (cfg, reinterpret_cast<const guchar*>(GNOME_PRINT_KEY_PAPER_HEIGHT), height, unit);
		m_bPdfWorkAround = true;		
	      }
	  }

	gnome_print_dialog_get_copies(GNOME_PRINT_DIALOG(gpd), &copies, &collate);
	range = gnome_print_dialog_get_range_page(GNOME_PRINT_DIALOG(gpd), &first, &end);
	

	m_gpm = GNOME_PRINT_JOB(g_object_ref(G_OBJECT(job))); //gnome_print_job_new (gnome_print_dialog_get_config (GNOME_PRINT_DIALOG(gpd)));

	// Record outputs
	m_bDoPrintRange				= (range == GNOME_PRINT_RANGE_RANGE);
	m_bDoPrintSelection			= (range == GNOME_PRINT_RANGE_SELECTION);
	m_cColorSpace				= GR_Graphics::GR_COLORSPACE_COLOR;
	
	if(m_bDoPrintRange)
	  {
	    m_nFirstPage		    = MIN(first, end);
	    m_nLastPage				= MAX(first, end);
	  }

	// (smartly?) let gnome-print handle these
	m_bCollate = false;
	m_nCopies  = 1;

	m_answer = a_OK;
	abiDestroyWidget (gpd);
}

void XAP_UnixDialog_Print::_getGraphics(void)
{
	UT_ASSERT(m_answer == a_OK);

	m_pPrintGraphics = new GR_UnixPangoPrintGraphics(m_gpm, m_bIsPreview);
	UT_return_if_fail(m_pPrintGraphics);
	
	m_pPrintGraphics->setColorSpace(m_cColorSpace);
	if(m_bPdfWorkAround)
	  static_cast<GR_UnixPangoPrintGraphics *>(m_pPrintGraphics)->setPdfWorkaround();
	m_answer = a_OK;
}

void XAP_UnixDialog_Print::runModal(XAP_Frame * pFrame) 
{
       _raisePrintDialog(pFrame);              
       if (m_answer == a_OK)
         _getGraphics();
}
