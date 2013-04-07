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
#include <stdarg.h>
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_App.h"

/*****************************************************************/

XAP_Dialog_MessageBox::XAP_Dialog_MessageBox(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_szMessage = NULL;
	m_szSecondaryMessage = NULL;
	m_buttons = b_O;
	m_defaultAnswer = a_OK;
	m_answer = a_OK;
}

XAP_Dialog_MessageBox::~XAP_Dialog_MessageBox(void)
{
	FREEP(m_szSecondaryMessage);
	FREEP(m_szMessage);
}

void XAP_Dialog_MessageBox::setMessage(const char * szMessage, ...)
{
	va_list args;

	va_start(args, szMessage);

	FREEP(m_szMessage);
	m_szMessage = (char *)g_try_malloc(512*sizeof(char));
	vsprintf(m_szMessage, szMessage, args);

	va_end(args);
}

void XAP_Dialog_MessageBox::setMessage(XAP_String_Id id, ...)
{
	va_list args;

	va_start(args, id);

	FREEP(m_szMessage);

	const XAP_StringSet * pSS = getApp()->getStringSet();

	m_szMessage = (char *)g_try_malloc(512*sizeof(char));
	std::string s;
	pSS->getValue(id, getApp()->getDefaultEncoding(),s);
	
	vsprintf(m_szMessage, (char*)s.c_str(), args);

	va_end(args);
}

void XAP_Dialog_MessageBox::setSecondaryMessage(const char * szMessage, ...)
{
	va_list args;

	va_start(args, szMessage);

	FREEP(m_szSecondaryMessage);
	m_szSecondaryMessage = (char *)g_try_malloc(512*sizeof(char));
	vsprintf(m_szSecondaryMessage, szMessage, args);

	va_end(args);
}

void XAP_Dialog_MessageBox::setSecondaryMessage(XAP_String_Id id, ...)
{
	va_list args;

	va_start(args, id);

	FREEP(m_szSecondaryMessage);

	const XAP_StringSet * pSS = getApp()->getStringSet();

	m_szSecondaryMessage = (char *)g_try_malloc(512*sizeof(char));
	std::string s;
	pSS->getValue(id, getApp()->getDefaultEncoding(),s);
	vsprintf(m_szSecondaryMessage, (char*)s.c_str(), args);

	va_end(args);
}


void XAP_Dialog_MessageBox::setButtons(XAP_Dialog_MessageBox::tButtons buttons)
{
	m_buttons = buttons;
}

void XAP_Dialog_MessageBox::setDefaultAnswer(XAP_Dialog_MessageBox::tAnswer answer)
{
	m_defaultAnswer = answer;
	m_answer = answer;
}

XAP_Dialog_MessageBox::tAnswer XAP_Dialog_MessageBox::getAnswer(void) const
{
	return m_answer;
}
