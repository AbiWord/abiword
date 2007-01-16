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
#include "xap_Dlg_PrintPreview.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"

XAP_Dialog_PrintPreview::XAP_Dialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id), m_szDocumentTitle(0), m_szDocumentPathname(0), m_szPaperSize(0)
{
}

XAP_Dialog_PrintPreview::~XAP_Dialog_PrintPreview(void)
{
        FREEP(m_szDocumentTitle);
	FREEP(m_szDocumentPathname);
	FREEP(m_szPaperSize);
}

void XAP_Dialog_PrintPreview::setDocumentTitle(const char * szDocTitle)
{
	FREEP(m_szDocumentTitle);
	if (szDocTitle && *szDocTitle)
		UT_cloneString(m_szDocumentTitle,szDocTitle);
}

void XAP_Dialog_PrintPreview::setDocumentPathname(const char * szDocPath)
{
	FREEP(m_szDocumentPathname);
	if (szDocPath && *szDocPath)
		UT_cloneString(m_szDocumentPathname,szDocPath);
}

void XAP_Dialog_PrintPreview::setPaperSize(const char * szPaperSize)
{
	FREEP(m_szPaperSize);
	if (szPaperSize && *szPaperSize)
		UT_cloneString(m_szPaperSize,szPaperSize);
}
