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
#include "xap_UnixGnomePrintGraphics.h"

#include <gtk/gtk.h>
#include <libgnomeprintui/gnome-print-dialog.h>

#include "ut_assert.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Dialog_Id.h"
#include "xap_Strings.h"
#include "gr_UnixPangoGraphics.h"

XAP_Dialog * XAP_UnixDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
														   XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_Print(pFactory,id);
}

XAP_UnixDialog_Print::XAP_UnixDialog_Print(XAP_DialogFactory * pDlgFactory,
													 XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id),
	  m_pPrintGraphics (NULL),
	  m_bIsPreview(false)
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

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	GnomePrintJob * job = gnome_print_job_new(XAP_UnixGnomePrintGraphics::s_setup_config (pFrame));

	// 1.  Create the dialog widget
	gpd = gnome_print_dialog_new (job,
				      reinterpret_cast<const guchar *>(pSS->getValue(XAP_STRING_ID_DLG_UP_PrintTitle)),
				      GNOME_PRINT_DIALOG_RANGE|GNOME_PRINT_DIALOG_COPIES);

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

	XAP_UnixGnomePrintGraphics * pGPG = new XAP_UnixGnomePrintGraphics(m_gpm, m_bIsPreview);
	UT_return_if_fail( pGPG );
	pGPG->setColorSpace(m_cColorSpace);

	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	UT_return_if_fail( pGF );
	
	UT_uint32 iDefaultPrintClass = pGF->getDefaultClass(false);

	GR_UnixPangoPrintGraphics * pPPG = NULL;
	
	if(iDefaultPrintClass == GRID_UNIX_PANGO_PRINT || iDefaultPrintClass == GRID_UNIX_PANGO)
	{
		pPPG = new GR_UnixPangoPrintGraphics(pGPG);
	}

	if(pPPG)
	{
		m_pPrintGraphics = pPPG;
	}
	else
	{
		m_pPrintGraphics = pGPG;
	}
	
	m_answer = a_OK;
}

void XAP_UnixDialog_Print::runModal(XAP_Frame * pFrame) 
{
       _raisePrintDialog(pFrame);              
       if (m_answer == a_OK)
         _getGraphics();
}
