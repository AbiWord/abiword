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
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dlg_MessageBox.h"

/*****************************************************************/

XAP_Dialog_MessageBox::XAP_Dialog_MessageBox(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_szMessage = NULL;
	m_buttons = b_O;
	m_defaultAnswer = a_OK;
	m_answer = a_OK;
}

XAP_Dialog_MessageBox::~XAP_Dialog_MessageBox(void)
{
	FREEP(m_szMessage);
}

void XAP_Dialog_MessageBox::setMessage(const char * szMessage)
{
	FREEP(m_szMessage);
	UT_cloneString(m_szMessage,szMessage);
}

void XAP_Dialog_MessageBox::setMessage(const char * szMessage, const char * sz1)
{
	FREEP(m_szMessage);
	UT_uint32 joinedSize = strlen(szMessage) + strlen(sz1) + 10;
	m_szMessage = (char *)malloc(joinedSize * sizeof(char));
	if (!m_szMessage)
	{
		UT_DEBUGMSG(("Could not allocate string for [%s %s]\n",szMessage,sz1));
		return;
	}

	sprintf(m_szMessage,szMessage,sz1);
}

void XAP_Dialog_MessageBox::setMessage(const char * szMessage, const char * sz1, const char * sz2, int num)
{
	FREEP(m_szMessage);
	UT_uint32 joinedSize = strlen(szMessage) + strlen(sz1) + strlen(sz2) + 4 + 10; // The 4 is for the line #
	m_szMessage = (char *)malloc(joinedSize * sizeof(char));
	if (!m_szMessage)
	{
		UT_DEBUGMSG(("Could not allocate string for [%s %s %s %d]\n",szMessage,sz1));
		return;
	}

	sprintf(m_szMessage,szMessage,sz1,sz2,num);
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
