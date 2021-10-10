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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "gr_Graphics.h"

#include "xap_Dlg_Insert_Symbol.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Draw_Symbol.h"
#include "xap_App.h"
#include "xap_Frame.h"

XAP_Dialog_Insert_Symbol::XAP_Dialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogsymbol")
{
	m_Inserted_Symbol = ' ';
	m_answer = a_CANCEL;
	m_DrawSymbol = NULL;
	m_pListener = NULL;
	//	m_pDialog = (XAP_Dialog_Insert_Symbol *) this;
}
XAP_Dialog_Insert_Symbol::~XAP_Dialog_Insert_Symbol(void)
{
	DELETEP(m_DrawSymbol);
	UT_ASSERT(!m_bInUse);
}

void XAP_Dialog_Insert_Symbol::useStart(void)
{
  //	XAP_Dialog_AppPersistent::useStart();
}

void XAP_Dialog_Insert_Symbol::useEnd(void)
{
  //	XAP_Dialog_AppPersistent::useEnd();
}


UT_UCSChar  XAP_Dialog_Insert_Symbol::getInsertedSymbol(void)
{
	return m_Inserted_Symbol;
}


char * XAP_Dialog_Insert_Symbol::getInsertedFont(void)
{
	UT_return_val_if_fail(m_DrawSymbol, NULL);
	strncpy(m_FontName, m_DrawSymbol->getSelectedFont(), sizeof(m_FontName)-1);
	m_FontName[sizeof(m_FontName)-1] = '\0';

	return m_FontName;
}

XAP_Dialog_Insert_Symbol::tAnswer XAP_Dialog_Insert_Symbol::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}


/************************************************************************/

void XAP_Dialog_Insert_Symbol::_updateDrawSymbol()
{
	m_DrawSymbol->queueDraw();
}

void XAP_Dialog_Insert_Symbol::_createSymbolFromGC(GR_Graphics * gc,
												   UT_uint32 width, UT_uint32 height)
{
	UT_ASSERT(gc);
	DELETEP(m_DrawSymbol);
	m_DrawSymbol = new XAP_Draw_Symbol(gc);
	UT_ASSERT(m_DrawSymbol);
	m_DrawSymbol->setWindowSize(width, height);
}

XAP_Draw_Symbol * XAP_Dialog_Insert_Symbol::_getCurrentSymbolMap( void)
{
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


void XAP_Dialog_Insert_Symbol::_onInsertButton()
{
	// get the character to be inserted
	UT_UCSChar c = getInsertedSymbol();
	// get the font of the symbol to be inserted
	gchar * symfont = (gchar *) getInsertedFont();
	// do the actual insert
	_insert(c, const_cast<const gchar*>(symfont));
}

void XAP_Dialog_Insert_Symbol::_insert(UT_UCSChar c, const char* symfont)
{
	UT_return_if_fail(m_pListener);
	UT_ASSERT(symfont);
	if (c == 0x00) 
	{
		 // Pango certainly doesn't like shaping 0x00 characters (it crashes when trying), 
		 // and I'm not sure if other font renderers do. Since it doesn't really make 
		 // any sense to allow such characters in the first place, let's just drop 
		 // it on the floor. - MARCM
		 UT_DEBUGMSG(("Dropping 0x00 character on the floor\n"));
		 return;
	}

	// connect to the current active frame using the robust getActiveFrame 
	m_pListener->setView(getActiveFrame()->getCurrentView());

	UT_DEBUGMSG(("Insert Char %x \n",c));
	m_pListener->insertSymbol(c, symfont);
}

void XAP_Dialog_Insert_Symbol::setActiveFrame(XAP_Frame *pFrame)
{

	m_pListener->setView( pFrame->getCurrentView() );

	// Update the platform code of this change in frame

	notifyActiveFrame(pFrame);
}


void  XAP_Dialog_Insert_Symbol::ConstructWindowName()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;											 
	std::string sTitle;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Insert_SymbolTitle,sTitle);

	UT_XML_cloneNoAmpersands(tmp, sTitle.c_str());
        BuildWindowName((char *) m_WindowName,(char*)tmp,sizeof(m_WindowName));
        FREEP(tmp);
}










