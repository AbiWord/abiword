/* AbiWord
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
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_Styles.h"

AP_Dialog_Styles::AP_Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{

	m_answer = a_OK;
	m_pParaPreview = NULL;
	m_pCharPreview = NULL;
}

AP_Dialog_Styles::~AP_Dialog_Styles(void)
{
	DELETEP(m_pParaPreview);
	DELETEP(m_pCharPreview);
}

AP_Dialog_Styles::tAnswer AP_Dialog_Styles::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Styles::_createParaPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pParaPreview = new AP_Styles_ParaPreview(gc);
	UT_ASSERT(m_pParaPreview);
	
	m_pParaPreview->setWindowSize(width, height);
}


void AP_Dialog_Styles::_createCharPreviewFromGC(GR_Graphics * gc,
                                                UT_uint32 width,
						UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pCharPreview = new AP_Styles_CharPreview(gc);
	UT_ASSERT(m_pCharPreview);
	
	m_pCharPreview->setWindowSize(width, height);
}

	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Styles_ParaPreview::AP_Styles_ParaPreview(GR_Graphics * gc)
	: XAP_Preview(gc)
{

}

AP_Styles_ParaPreview::~AP_Styles_ParaPreview()
{
}


void AP_Styles_ParaPreview::draw(void)
{
	UT_sint32 iWidth = getWindowWidth();
	UT_sint32 iHeight = getWindowHeight();

	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);

	UT_Rect pageRect(5, 5, iWidth - 10, iHeight - 10);

	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);
	m_gc->setLineWidth(1);
	m_gc->drawLine(pageRect.left, pageRect.top, pageRect.left + pageRect.width, pageRect.top);
	m_gc->drawLine(pageRect.left, pageRect.top, pageRect.left, pageRect.top + pageRect.height);
	m_gc->setLineWidth(3);
	m_gc->drawLine(pageRect.left + pageRect.width, pageRect.top + 1, pageRect.left + pageRect.width, pageRect.top + pageRect.height);
	m_gc->drawLine(pageRect.left + 1, pageRect.top + pageRect.height, pageRect.left + pageRect.width, pageRect.top + pageRect.height);

}

	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Styles_CharPreview::AP_Styles_CharPreview(GR_Graphics * gc)
	: XAP_Preview(gc)
{

}

AP_Styles_CharPreview::~AP_Styles_CharPreview()
{
}


void AP_Styles_CharPreview::draw(void)
{
	UT_sint32 iWidth = getWindowWidth();
	UT_sint32 iHeight = getWindowHeight();

	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);

	UT_Rect pageRect(5, 5, iWidth - 10, iHeight - 10);

	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);
	m_gc->setLineWidth(1);
	m_gc->drawLine(pageRect.left, pageRect.top, pageRect.left + pageRect.width, pageRect.top);
	m_gc->drawLine(pageRect.left, pageRect.top, pageRect.left, pageRect.top + pageRect.height);
	m_gc->setLineWidth(3);
	m_gc->drawLine(pageRect.left + pageRect.width, pageRect.top + 1, pageRect.left + pageRect.width, pageRect.top + pageRect.height);
	m_gc->drawLine(pageRect.left + 1, pageRect.top + pageRect.height, pageRect.left + pageRect.width, pageRect.top + pageRect.height);

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

