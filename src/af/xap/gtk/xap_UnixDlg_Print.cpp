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
#include "xad_Document.h"

#include "gr_CairoPrintGraphics.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "xap_UnixFrameImpl.h"
#include "ap_FrameData.h"
#include "gr_DrawArgs.h"
#include "ap_Strings.h"

static void s_Begin_Print(GtkPrintOperation * ,
						  GtkPrintContext   *context,
						  gpointer           p)
{
	XAP_UnixDialog_Print * pDlg = (XAP_UnixDialog_Print *) p;
	//
	// Use context to extract the cairo_t surface for the print.
	//
	pDlg->BeginPrint(context);
} 

static void s_Print_Page(GtkPrintOperation * , GtkPrintContext *, gint page_nr,gpointer p)
{
	XAP_UnixDialog_Print * pDlg = (XAP_UnixDialog_Print *) p;
	pDlg->PrintPage(page_nr);
}

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
	  m_pPO(NULL),
	  m_pJob(NULL),
	  m_pPC(NULL),
	  m_pView(NULL),
	  m_iNumberPages(0),
	  m_iCurrentPage(0),
	  m_pDL(NULL),
	  m_pPrintView(NULL),
	  m_pPrintLayout(NULL),
	  m_bDidQuickPrint(false),
	  m_bShowParagraphs(false),
	  m_pFrame(NULL)
{
}

XAP_UnixDialog_Print::~XAP_UnixDialog_Print(void)
{
}

GR_Graphics * XAP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	return m_pPrintGraphics;
}

void XAP_UnixDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_UNUSED(pGraphics);
	UT_ASSERT(pGraphics == m_pPrintGraphics);	
	DELETEP(m_pPrintGraphics);
	if(m_pPageSetup)
	   g_object_unref(m_pPageSetup);
	m_pPageSetup = NULL;
	if(m_pGtkPageSize)
		gtk_paper_size_free (m_pGtkPageSize);
	m_pGtkPageSize=  NULL;
	if(m_pPO)
		g_object_unref(m_pPO);
	m_pPO=  NULL;
	if(m_pJob)
		g_object_unref(m_pJob);
	m_pJob = NULL;
	if(m_pPC)
		g_object_unref(m_pPC);
	m_pPC = NULL;
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixDialog_Print::BeginPrint(GtkPrintContext   *context)
{
	m_pPC= context;
	cairo_t* cr = gtk_print_context_get_cairo_context (m_pPC);
	//
	// The cairo context is automatically unref'd at the end of the print
	// so we need to reference it to allow it to be deleted by the PrintGraphics
	// class
	cairo_reference(cr);
	//
	// Set this from the dialog range
	//
	gtk_print_operation_set_n_pages (m_pPO,m_iNumberPages);
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());

	UT_DEBUGMSG(("Initial Cairo Context %x \n",cr));
	m_pPrintGraphics = (GR_Graphics *) new GR_CairoPrintGraphics(cr,72.0);
	if(m_pView->getViewMode() == VIEW_PRINT )
	{
			m_pPrintLayout = m_pDL;
			m_pPrintView = m_pView;
			m_pPrintLayout->setQuickPrint(m_pPrintGraphics);
			m_bDidQuickPrint = true;
			if(pFrameData->m_bShowPara)
			{
				m_pPrintView->setShowPara(false);
				m_bShowParagraphs = true;
			}
			else
				m_bShowParagraphs = false;
	}
	else
	{
			m_pPrintLayout = new FL_DocLayout(m_pView->getDocument(),m_pPrintGraphics);
			m_pPrintView = new FV_View(XAP_App::getApp(),0,m_pPrintLayout);
			m_pPrintView->getLayout()->fillLayouts();
			m_pPrintView->getLayout()->formatAll();
			m_pPrintView->getLayout()->recalculateTOCFields();
			m_bDidQuickPrint = false;
	}
	m_pPrintGraphics->startPrint();
}
void XAP_UnixDialog_Print::PrintPage(gint page_nr)
{
	UT_DEBUGMSG(("Print Page %d \n",page_nr));
	dg_DrawArgs da;
	da.pG = m_pPrintGraphics;
	const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet ();
	const gchar * msgTmpl = pSS->getValue (AP_STRING_ID_MSG_PrintStatus);
	gchar msgBuf [1024];
	sprintf (msgBuf, msgTmpl, page_nr+1, m_iNumberPages);
	if(m_pFrame) 
	{
		m_pFrame->setStatusMessage ( msgBuf );
		m_pFrame->nullUpdate();
	}
	m_pPrintView->draw(page_nr, &da);						
}

void XAP_UnixDialog_Print::setPreview(bool b)
{
	m_bIsPreview = b;
}

void XAP_UnixDialog_Print::setupPrint()
{
	double mrgnTop, mrgnBottom, mrgnLeft, mrgnRight, width, height;
	bool portrait;

	m_pView = static_cast<FV_View*>(m_pFrame->getCurrentView());

	mrgnTop = m_pView->getPageSize().MarginTop(DIM_MM);
	mrgnBottom = m_pView->getPageSize().MarginBottom(DIM_MM);
	mrgnLeft = m_pView->getPageSize().MarginLeft(DIM_MM);
	mrgnRight = m_pView->getPageSize().MarginRight(DIM_MM);

	portrait = m_pView->getPageSize().isPortrait();
		
	width = m_pView->getPageSize().Width (DIM_MM);
	height = m_pView->getPageSize().Height (DIM_MM);
	
	m_pPageSetup = gtk_page_setup_new();

	char * pszName = m_pView->getPageSize().getPredefinedName();
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
	//
	// Set orientation
	//
	if(	portrait)
		gtk_page_setup_set_orientation(m_pPageSetup,GTK_PAGE_ORIENTATION_PORTRAIT);
	else
		gtk_page_setup_set_orientation(m_pPageSetup,GTK_PAGE_ORIENTATION_LANDSCAPE);

}

void XAP_UnixDialog_Print::runModal(XAP_Frame * pFrame) 
{
#if 0
	int copies = 1, collate = FALSE;
	int first = 1, end = 0, range;
	GtkWidget	*dialog;
	gint		 response;
#endif
	m_pFrame = pFrame;
	setupPrint();
	m_pPO = gtk_print_operation_new();
	gtk_print_operation_set_default_page_setup(m_pPO,m_pPageSetup);
	gtk_print_operation_set_use_full_page (m_pPO, true);

	m_pDL = m_pView->getLayout();
	m_iCurrentPage = m_pDL->findPage(m_pView->getCurrentPage());
	m_iNumberPages = (gint) m_pDL->countPages();
	gtk_print_operation_set_current_page(m_pPO,m_iCurrentPage);
    gtk_print_operation_set_show_progress(m_pPO, TRUE);
	//
	// Implement this later to give the user a sane default *.pdf name
	//
	// gtk_print_operation_set_export_filename(m_pPO, const gchar *filename);



	g_signal_connect (m_pPO, "begin_print", G_CALLBACK (s_Begin_Print), this);
	g_signal_connect (m_pPO, "draw_page", G_CALLBACK (s_Print_Page), this);

	XAP_UnixFrameImpl * pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(m_pFrame->getFrameImpl());
	
	// Get the GtkWindow of the parent frame
	GtkWidget * parent = pUnixFrameImpl->getTopLevelWindow();
	GtkWindow * pPWindow = GTK_WINDOW(parent);
	//	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	//	const gchar * szDialogName = einterpret_cast<const gchar *>(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle);

	gtk_print_operation_run (m_pPO,
							 (m_bIsPreview)? GTK_PRINT_OPERATION_ACTION_PREVIEW:
							 GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
							 pPWindow, NULL);


	if(!m_bDidQuickPrint)
	{
		DELETEP(m_pPrintLayout);
		DELETEP(m_pPrintView);
	}
	else
	{
		if(m_bShowParagraphs)
			m_pPrintView->setShowPara(true);

		m_pPrintLayout->setQuickPrint(NULL);
		m_pPrintLayout = NULL;
		m_pPrintView = NULL;
	}
	DELETEP(m_pPrintGraphics);
}
