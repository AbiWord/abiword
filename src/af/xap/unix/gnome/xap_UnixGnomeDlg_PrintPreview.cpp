/* AbiSource Application Framework
 * Copyright (C) 2003 Dom Lachowicz <cinamod@hotmail.com>
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

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-config.h>

#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_UnixApp.h"

#include "xap_UnixGnomeDlg_PrintPreview.h"
#include "xap_UnixGnomePrintGraphics.h"

class XAP_UnixFontManager;

XAP_Dialog * XAP_UnixGnomeDialog_PrintPreview::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new XAP_UnixGnomeDialog_PrintPreview (pFactory,id);
}

XAP_UnixGnomeDialog_PrintPreview::XAP_UnixGnomeDialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_PrintPreview(pDlgFactory,id), m_pGnomePrintGraphics(NULL)  
{
}

XAP_UnixGnomeDialog_PrintPreview::~XAP_UnixGnomeDialog_PrintPreview(void)
{
}

void XAP_UnixGnomeDialog_PrintPreview::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_return_if_fail(pGraphics == m_pGnomePrintGraphics);	
	DELETEP(m_pGnomePrintGraphics);
}

GR_Graphics * XAP_UnixGnomeDialog_PrintPreview::getPrinterGraphicsContext(void)
{	
	return m_pGnomePrintGraphics;
}

void XAP_UnixGnomeDialog_PrintPreview::runModal(XAP_Frame * pFrame) 
{
       m_pGnomePrintGraphics = new XAP_UnixGnomePrintGraphics(gnome_print_job_new(XAP_UnixGnomePrintGraphics::s_setup_config(pFrame)), true);
       UT_return_if_fail(m_pGnomePrintGraphics != NULL);       
       m_pGnomePrintGraphics->setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
}
