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

#include "xap_Dlg_Insert_Symbol.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

XAP_Dialog_Insert_Symbol::XAP_Dialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_AppPersistent(pDlgFactory,id)
{
	m_Inserted_Symbol = ' ';
	m_answer = a_CANCEL;
	m_DrawSymbol = NULL;
}
XAP_Dialog_Insert_Symbol::~XAP_Dialog_Insert_Symbol(void)
{
	UT_ASSERT(!m_bInUse);
}

void XAP_Dialog_Insert_Symbol::useStart(void)
{
	XAP_Dialog_AppPersistent::useStart();
}

void XAP_Dialog_Insert_Symbol::useEnd(void)
{
	XAP_Dialog_AppPersistent::useEnd();
}

UT_UCSChar  XAP_Dialog_Insert_Symbol::getInsertedSymbol(void)
{
	return m_Inserted_Symbol;
}


UT_UCSChar * XAP_Dialog_Insert_Symbol::getInsertedFont(void)
{
	UT_ASSERT(m_DrawSymbol);
	return m_DrawSymbol->getSelectedFont();
}

XAP_Dialog_Insert_Symbol::tAnswer XAP_Dialog_Insert_Symbol::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

/************************************************************************/

void XAP_Dialog_Insert_Symbol::_updateDrawSymbol()
{
	m_DrawSymbol->draw();
}

void XAP_Dialog_Insert_Symbol::_createSymbolFromGC(GR_Graphics * gc,
												   UT_uint32 width, UT_uint32 height)
{
	UT_ASSERT(gc);
	m_DrawSymbol = new XAP_Draw_Symbol(gc);
	UT_ASSERT(m_DrawSymbol);
	m_DrawSymbol->setWindowSize(width, height);
}

XAP_Draw_Symbol * XAP_Dialog_Insert_Symbol::_getCurrentSymbolMap( void)
{
	UT_ASSERT(m_DrawSymbol);
	return m_DrawSymbol;
}

/************************************************************************/

void XAP_Dialog_Insert_Symbol::_updateDrawSymbolarea(UT_UCSChar c, UT_UCSChar p)
{
	m_DrawSymbol->drawarea(c,p);
}

void XAP_Dialog_Insert_Symbol::_createSymbolareaFromGC(GR_Graphics * gc,
													   UT_uint32 width, UT_uint32 height)
{
	UT_ASSERT(gc);
	UT_ASSERT(m_DrawSymbol);
	m_DrawSymbol->setAreaGc(gc);
	m_DrawSymbol->setAreaSize(width, height);
}





















