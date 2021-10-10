/* AbiWord
 * Copyright (C) 2003 Hubert Figuiere
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

#include "ut_types.h"
#include "gr_DrawArgs.h"
#include "fp_PageSize.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "ap_PrintingDelegate.h"


void s_printPage(PD_Document */*doc*/,  GR_Graphics *pGraphics,
		     FV_View * pPrintView, const char *pDocName,
		     UT_sint32 iWidth,  UT_sint32 iHeight,
             int nPage)
{
	dg_DrawArgs da;
	memset(&da, 0, sizeof(da));
	da.pG = pGraphics;

	fp_PageSize ps = pPrintView->getPageSize();	  

	pGraphics->m_iRasterPosition = (nPage-1)*iHeight;
	pGraphics->startPage(pDocName, nPage, ps.isPortrait(), iWidth, iHeight);
	pPrintView->drawPage(nPage-1, &da);
}

AP_PrintingDelegate::AP_PrintingDelegate(FV_View* pView)
	: XAP_PrintingDelegate(),
		m_pView(pView)
{
	
}


int AP_PrintingDelegate::getPageCount()
{
	return m_pView->getLayout()->countPages();
}



void AP_PrintingDelegate::printPage(int pageNum)
{
	s_printPage(m_pView->getLayout()->getDocument(), m_pView->getLayout()->getGraphics(), m_pView, 
			"", 0, 0, pageNum);
}

