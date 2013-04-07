/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Frame.h"

#include "xap_UnixApp.h"
#include "xap_UnixDlg_PrintPreview.h"

#include "gr_CairoGraphics.h"

#include "fv_View.h"

class XAP_UnixFontManager;

XAP_Dialog * XAP_UnixDialog_PrintPreview::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_PrintPreview (pFactory,id);
}

XAP_UnixDialog_PrintPreview::XAP_UnixDialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_PrintPreview(pDlgFactory,id), m_pPrintGraphics(NULL)  
{
}

XAP_UnixDialog_PrintPreview::~XAP_UnixDialog_PrintPreview(void)
{
}

void XAP_UnixDialog_PrintPreview::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_return_if_fail(pGraphics == m_pPrintGraphics);	
	DELETEP(m_pPrintGraphics);
}

GR_Graphics * XAP_UnixDialog_PrintPreview::getPrinterGraphicsContext(void)
{	
	return m_pPrintGraphics;
}

void XAP_UnixDialog_PrintPreview::runModal(XAP_Frame * /*  pFrame */) 
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}
