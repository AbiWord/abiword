/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
#include <gtk/gtkprintunixdialog.h>
#include <libgnomeprintui/gnome-print-dialog.h>

#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_Frame.h"
#include "ut_misc.h"

#include "gr_CairoPrintGraphics.h"

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
	  m_pPageSetup(NULL),
	  m_pGtkPageSize(NULL),
	  m_pPO(NULL)
{
}

XAP_UnixDialog_Print::~XAP_UnixDialog_Print(void)
{
}

GR_Graphics * XAP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);
	return m_pPrintGraphics;
}

void XAP_UnixDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_UNUSED(pGraphics);
	UT_ASSERT(pGraphics == m_pPrintGraphics);	
	DELETEP(m_pPrintGraphics);
	g_object_unref(m_pPageSetup);
	m_pPageSetup = NULL;
	gtk_paper_size_free (m_pGtkPageSize);
	m_pGtkPageSize=  NULL;
	g_object_unref(m_pPO);
	m_pPO=  NULL;
}

/*****************************************************************/
/*****************************************************************/

#if 0
void XAP_UnixDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
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
#endif
void XAP_UnixDialog_Print::setupPrint(XAP_Frame * pFrame)
{
	double mrgnTop, mrgnBottom, mrgnLeft, mrgnRight, width, height;
	bool portrait;

	FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());

	mrgnTop = pView->getPageSize().MarginTop(DIM_MM);
	mrgnBottom = pView->getPageSize().MarginBottom(DIM_MM);
	mrgnLeft = pView->getPageSize().MarginLeft(DIM_MM);
	mrgnRight = pView->getPageSize().MarginRight(DIM_MM);

	portrait = pView->getPageSize().isPortrait();
		
	width = pView->getPageSize().Width (DIM_MM);
	height = pView->getPageSize().Height (DIM_MM);
	
	m_pPageSetup = gtk_page_setup_new();

	char * pszName = pView->getPageSize().getPredefinedName();
	bool isPredefined = false;
	const char * pszGtkName = NULL;
	if(pszName == NULL)
		{
	}
	else if(g_ascii_strcasecmp(pszName,"Custom") == 0)
	{
	}
	else if(g_ascii_strcasecmp(pszName,"A0") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a0";
	}
	else if(g_ascii_strcasecmp(pszName,"A1") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a1";
	}
	else if(g_ascii_strcasecmp(pszName,"A2") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a2";
	}
	else if(g_ascii_strcasecmp(pszName,"A3") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a3";
	}
	else if(g_ascii_strcasecmp(pszName,"A4") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a4";
	}
	else if(g_ascii_strcasecmp(pszName,"A5") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a5";
	}
	else if(g_ascii_strcasecmp(pszName,"A6") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a6";
	}
	else if(g_ascii_strcasecmp(pszName,"A7") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a7";
	}
	else if(g_ascii_strcasecmp(pszName,"A8") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a8";
	}
	else if(g_ascii_strcasecmp(pszName,"A9") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_a9";
	}
	else if(g_ascii_strcasecmp(pszName,"B0") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b0";
	}
	else if(g_ascii_strcasecmp(pszName,"B1") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b1";
	}
	else if(g_ascii_strcasecmp(pszName,"B2") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b2";
	}
	else if(g_ascii_strcasecmp(pszName,"B3") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b3";
	}
	else if(g_ascii_strcasecmp(pszName,"B4") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b4";
	}
	else if(g_ascii_strcasecmp(pszName,"B4") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b4";
	}
	else if(g_ascii_strcasecmp(pszName,"B5") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b5";
	}
	else if(g_ascii_strcasecmp(pszName,"B6") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b6";
	}
	else if(g_ascii_strcasecmp(pszName,"B7") == 0)
	{
		isPredefined = true;
		pszGtkName = "iso_b7";
	}
	else if(g_ascii_strcasecmp(pszName,"Legal") == 0)
	{
		isPredefined = true;
		pszGtkName = "na_legal";
	}
	else if(g_ascii_strcasecmp(pszName,"Letter") == 0)
	{
		isPredefined = true;
		pszGtkName = "na_letter";
	}
	if(isPredefined)
	{
		m_pGtkPageSize = gtk_paper_size_new(static_cast<const gchar *>(pszGtkName));
	}
	else
	{
		m_pGtkPageSize = gtk_paper_size_new_custom ("custom",
												"custom",
												width,height,GTK_UNIT_MM);
	}
	//
	// Set the Page Size
	//
	gtk_page_setup_set_paper_size(m_pPageSetup,m_pGtkPageSize);
	//
	// Set the margins
	//
	gtk_page_setup_set_top_margin(m_pPageSetup,mrgnTop,GTK_UNIT_MM);
	gtk_page_setup_set_bottom_margin(m_pPageSetup,mrgnBottom,GTK_UNIT_MM);
	gtk_page_setup_set_left_margin(m_pPageSetup,mrgnLeft,GTK_UNIT_MM);
	gtk_page_setup_set_right_margin(m_pPageSetup,mrgnRight,GTK_UNIT_MM);
	if(	portrait)
		gtk_page_setup_set_orientation(m_pPageSetup,GTK_PAGE_ORIENTATION_PORTRAIT);
	else
		gtk_page_setup_set_orientation(m_pPageSetup,GTK_PAGE_ORIENTATION_LANDSCAPE);

}

void XAP_UnixDialog_Print::runModal(XAP_Frame * pFrame) 
{
	GtkWidget	*dialog;
	gint		 response;
#if 0
	int copies = 1, collate = FALSE;
	int first = 1, end = 0, range;
#endif
	setupPrint(pFrame);
	m_pPO = gtk_print_operation_new();
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	dialog = gtk_print_unix_dialog_new (reinterpret_cast<const gchar *>(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle)), NULL);
	
	gtk_print_unix_dialog_set_page_setup((GtkPrintUnixDialog *) dialog,m_pPageSetup);

	gtk_print_unix_dialog_set_manual_capabilities((GtkPrintUnixDialog *) dialog, GTK_PRINT_CAPABILITY_PAGE_SET);
	gtk_print_unix_dialog_set_manual_capabilities((GtkPrintUnixDialog *) dialog, GTK_PRINT_CAPABILITY_COPIES);
	gtk_print_unix_dialog_set_manual_capabilities((GtkPrintUnixDialog *) dialog, GTK_PRINT_CAPABILITY_PREVIEW );

	response = abiRunModalDialog (GTK_DIALOG (dialog), pFrame, this, GTK_RESPONSE_CANCEL, false);
	switch (response) {
	case GTK_RESPONSE_APPLY:
		// TODO preview
		printf ("preview\n");
		m_answer = a_OK;
		break;
	case GTK_RESPONSE_OK:
		// TODO print
		printf ("print\n");
		m_answer = a_OK;
		break;
	default:
		printf ("cancel\n");
		m_answer = a_CANCEL; 
	}


	gtk_widget_destroy (dialog), dialog = NULL;
}
