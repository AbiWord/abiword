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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixGnomeDlg_PrintPreview.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"

#include "xap_UnixApp.h"

class XAP_UnixFontManager;

XAP_Dialog * XAP_UnixGnomeDialog_PrintPreview::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_PrintPreview * p = new XAP_UnixGnomeDialog_PrintPreview (pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_PrintPreview::XAP_UnixGnomeDialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_PrintPreview(pDlgFactory,id)
{
        m_pUnixFrame = NULL;
	m_pGnomePrintGraphics = NULL;  
}

XAP_UnixGnomeDialog_PrintPreview::~XAP_UnixGnomeDialog_PrintPreview(void)
{
}

void XAP_UnixGnomeDialog_PrintPreview::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pGnomePrintGraphics);
	
	DELETEP(m_pGnomePrintGraphics);
}

GR_Graphics * XAP_UnixGnomeDialog_PrintPreview::getPrinterGraphicsContext(void)
{	
	return m_pGnomePrintGraphics;
}

void XAP_UnixGnomeDialog_PrintPreview::runModal(XAP_Frame * pFrame) 
{
       m_pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
       UT_ASSERT(m_pUnixFrame);

       XAP_App * app = m_pUnixFrame->getApp();
       UT_ASSERT(app);
       
       XAP_UnixApp * unixapp = static_cast<XAP_UnixApp *> (app);
       UT_ASSERT(unixapp);
       
       XAP_UnixFontManager * fontmgr = unixapp->getFontManager();
       UT_ASSERT(fontmgr);
       
       m_pGnomePrintGraphics = new XAP_UnixGnomePrintGraphics(gnome_print_master_new(),
							      m_szPaperSize,
							      fontmgr,
							      unixapp,
							      1);

       UT_ASSERT(m_pGnomePrintGraphics);
       
       // set the color mode
       m_pGnomePrintGraphics->setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
}
