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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "xap_UnixDlg_Print.h"

#include <gtk/gtk.h>

#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_Frame.h"
#include "ut_misc.h"
#include "xad_Document.h"
#include "pd_Document.h"

#include "gr_CairoPrintGraphics.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "xap_UnixFrameImpl.h"
#include "ap_FrameData.h"
#include "gr_DrawArgs.h"
#include "ap_Strings.h"
#include "ap_EditMethods.h"

#define GTKPRINTRES 72.

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
}

/*****************************************************************/
/*****************************************************************/

void XAP_UnixDialog_Print::BeginPrint(GtkPrintContext   *context)
{
    // Note: Help landscape printing survive, don't do anything on the cairo
    // context in this function.  Any transformations etc. shall take place in
    // PrintPage, because GtkPrint may do some transformations itself (which
    // will come out differently if we scale here).
	cairo_t* cr = gtk_print_context_get_cairo_context (context);
	//
	// The cairo context is automatically unref'd at the end of the print
	// so we need to reference it to allow it to be deleted by the PrintGraphics
	// class
	cairo_reference(cr);
	gtk_print_operation_set_n_pages (m_pPO,m_iNumberPages);
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());

	xxx_UT_DEBUGMSG(("Initial Cairo Context %x \n",cr));
	m_pPrintGraphics = (GR_Graphics *) new GR_CairoPrintGraphics(cr, gr_PRINTRES);

	double ScreenRes = m_pView->getGraphics()->getDeviceResolution();
	static_cast<GR_CairoPrintGraphics *>(m_pPrintGraphics)
        ->setResolutionRatio(gr_PRINTRES/ScreenRes);

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
			m_pPrintView = new FV_View(XAP_App::getApp(), nullptr, m_pPrintLayout);
			m_pPrintView->getLayout()->fillLayouts();
			m_pPrintView->getLayout()->formatAll();
			m_pPrintView->getLayout()->recalculateTOCFields();
			m_bDidQuickPrint = false;
	}
	m_pPrintGraphics->startPrint();
}
void XAP_UnixDialog_Print::PrintPage(gint page_nr)
{
	xxx_UT_DEBUGMSG(("Print Page %d \n",page_nr));

	m_pPrintGraphics->beginPaint();
	cairo_t *cr = static_cast<GR_CairoPrintGraphics *>(m_pPrintGraphics)->getCairo();

	//
	// We set the resolution of the printer context to higher than screen
	// so we don't loose resolution when printing images.
	//
	// We then correct for this with thise scale. GtkPrint contexts always
	// assume 72 DPI
	//
	// In the future we can use this to do 2,4,6,8 etc pages per page
	//
	double srat = static_cast<double>(GTKPRINTRES)/static_cast<double>(gr_PRINTRES);
	cairo_scale(cr, srat, srat);
	xxx_UT_DEBUGMSG(("Resolution Ratio set to %f \n",gr_PrintRes/ScreenRes));
	UT_DEBUGMSG(("Resolution Ratio of Direct call is %f Cast call is %f \n",m_pPrintGraphics->getResolutionRatio(),	static_cast<GR_CairoPrintGraphics *>(m_pPrintGraphics)->getResolutionRatio()));

	dg_DrawArgs da;
	da.pG = m_pPrintGraphics;
	da.xoff = 0;
	da.yoff = 0;
	const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet ();
	const gchar * msgTmpl = pSS->getValue (AP_STRING_ID_MSG_PrintStatus);
	gchar msgBuf [1024];
	sprintf (msgBuf, msgTmpl, page_nr+1, m_iNumberPages);
	if(m_pFrame) 
	{
		m_pFrame->setStatusMessage ( msgBuf );
		m_pFrame->nullUpdate();
	}
	m_pPrintView->drawPage(page_nr, &da);
	m_pPrintGraphics->endPaint();
}

void XAP_UnixDialog_Print::setPreview(bool b)
{
	m_bIsPreview = b;
}

void XAP_UnixDialog_Print::setupPrint()
{
	double blockMrgnLeft, blockMrgnRight, mrgnTop, mrgnBottom, mrgnLeft, mrgnRight = 0.;
	double width, height;
	bool portrait;

	m_pView = static_cast<FV_View*>(m_pFrame->getCurrentView());
	m_pPO = gtk_print_operation_new();
	//
	// Set filename if it's not present already
	//
    std::string sURI = m_pView->getDocument()->getPrintFilename();
	
	if(sURI.empty())
	{
        const std::string & filename = m_pView->getDocument()->getFilename();
        if(!filename.empty()) {
            sURI = filename;
            UT_addOrReplacePathSuffix(sURI, ".pdf");
        }
	}
    if(!sURI.empty()) {
        GtkPrintSettings * pSettings =  gtk_print_settings_new();
        gtk_print_settings_set(pSettings,
                               GTK_PRINT_SETTINGS_OUTPUT_URI,
                               sURI.c_str() );
        gtk_print_operation_set_print_settings(m_pPO,pSettings);
        g_object_unref(pSettings);
    }

	s_getPageMargins(m_pView, blockMrgnLeft, blockMrgnRight, mrgnLeft, mrgnRight,  mrgnTop, mrgnBottom);

	portrait = m_pView->getPageSize().isPortrait();
		
	width = m_pView->getPageSize().Width (DIM_MM);
	height = m_pView->getPageSize().Height (DIM_MM);
	
	m_pPageSetup = gtk_page_setup_new();

	const char * pszName = m_pView->getPageSize().getPredefinedName();
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
        /*
         * Width() and Height() will return the paper size as shown in the UI.
         * Gtk wants the real paper size, however, and will swap the two
         * itself when we specify the orientation.
         */
        m_pGtkPageSize = gtk_paper_size_new_custom("custom",
                                                   "custom",
                                                   portrait ? width : height,
                                                   portrait ? height : width,
                                                   GTK_UNIT_MM);
	}
	//
	// Set the Page Size
	//
	gtk_page_setup_set_paper_size(m_pPageSetup,m_pGtkPageSize);
	//
	// Set the margins
	//
	gtk_page_setup_set_top_margin(m_pPageSetup,mrgnTop,GTK_UNIT_INCH);
	gtk_page_setup_set_bottom_margin(m_pPageSetup,mrgnBottom,GTK_UNIT_INCH);
	gtk_page_setup_set_left_margin(m_pPageSetup,mrgnLeft,GTK_UNIT_INCH);
	gtk_page_setup_set_right_margin(m_pPageSetup,mrgnRight,GTK_UNIT_INCH);
	//
	// Set orientation
	//
	if(	portrait)
		gtk_page_setup_set_orientation(m_pPageSetup,GTK_PAGE_ORIENTATION_PORTRAIT);
	else
		gtk_page_setup_set_orientation(m_pPageSetup,GTK_PAGE_ORIENTATION_LANDSCAPE);
	gtk_print_operation_set_default_page_setup(m_pPO,m_pPageSetup);
	gtk_print_operation_set_use_full_page (m_pPO, true);
	m_pDL = m_pView->getLayout();
	m_iCurrentPage = m_pDL->findPage(m_pView->getCurrentPage());
	m_iNumberPages = (gint) m_pDL->countPages();
	gtk_print_operation_set_current_page(m_pPO,m_iCurrentPage);

	g_signal_connect (m_pPO, "begin_print", G_CALLBACK (s_Begin_Print), this);
	g_signal_connect (m_pPO, "draw_page", G_CALLBACK (s_Print_Page), this);
}

void XAP_UnixDialog_Print::runModal(XAP_Frame * pFrame) 
{
	m_pFrame = pFrame;
	setupPrint();
    gtk_print_operation_set_show_progress(m_pPO, TRUE);

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

	cleanup();
}

void XAP_UnixDialog_Print::cleanup(void)
{
	//
	// Get the filename we printed to
	//
	GtkPrintSettings *  pSettings = gtk_print_operation_get_print_settings(m_pPO);
	const gchar * szFname =  gtk_print_settings_get(pSettings,GTK_PRINT_SETTINGS_OUTPUT_URI);
	if((szFname != NULL) && (strcmp(szFname,"output.pdf") != 0))
	{
		m_pView->getDocument()->setPrintFilename(szFname);
	}
	g_object_unref(m_pPO);
	m_pPO= NULL;
	if(!m_bDidQuickPrint)
	{
		UT_DEBUGMSG(("Deleting PrintView %p \n",m_pPrintView));
		DELETEP(m_pPrintLayout);
		DELETEP(m_pPrintView);
	}
	else
	{
		if(m_pPrintLayout)
			m_pPrintLayout->setQuickPrint(NULL);
		m_pPrintLayout = NULL;
		m_pPrintView = NULL;

		if(m_bShowParagraphs)
			m_pView->setShowPara(true);
		m_pDL->incrementGraphicTick();
	}
	UT_DEBUGMSG(("Reset fontmap m_pView %p Graphics %p \n",m_pView,m_pView->getGraphics()));
	static_cast<GR_CairoGraphics *>(m_pView->getGraphics())->resetFontMapResolution();
	DELETEP(m_pPrintGraphics);
	//
	// Finish pending expose events.
	//
	m_pFrame->nullUpdate();
}

void XAP_UnixDialog_Print::PrintDirectly(XAP_Frame * pFrame, const char * szFilename, const char * szPrinter)
{
	m_pFrame = pFrame;
	setupPrint();
	if(szFilename)
    {
		 gtk_print_operation_set_export_filename(m_pPO, szFilename);
		 gtk_print_operation_run (m_pPO,GTK_PRINT_OPERATION_ACTION_EXPORT,
								  NULL,NULL);
	}
	else
	{
		GtkPrintSettings *  pSettings = gtk_print_operation_get_print_settings(m_pPO);
		if(szPrinter)
		{
			gtk_print_settings_set_printer(pSettings, szPrinter);
		}
		else
		{
			gtk_print_settings_set_printer(pSettings, GTK_PRINT_SETTINGS_PRINTER);
		}
		gtk_print_operation_set_print_settings(m_pPO,pSettings);
		gtk_print_operation_run (m_pPO,GTK_PRINT_OPERATION_ACTION_PRINT,
								 NULL,NULL);
	}
	cleanup();
}
