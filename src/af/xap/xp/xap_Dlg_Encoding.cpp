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
#include "xap_Dlg_Encoding.h"

/*****************************************************************/

static int s_compareQ(const void * a, const void * b)
{
	const XML_Char ** A = (const XML_Char ** ) a;
	const XML_Char ** B = (const XML_Char ** ) b;

	return UT_strcmp(*A,*B);
}

XAP_Dialog_Encoding::XAP_Dialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer			= a_CANCEL;
	m_pDescription		= NULL;
	m_pEncoding			= NULL;
	m_pEncTable = new UT_Encoding;
	
	UT_ASSERT(m_pEncTable);
	m_iEncCount = m_pEncTable->getCount();
	m_ppEncodings = new const XML_Char * [m_iEncCount];

	// Build array of encoding names
	for(UT_uint32 i = 0; i < m_iEncCount; i++)
		m_ppEncodings[i] = m_pEncTable->getNthDescription(i);
}

XAP_Dialog_Encoding::~XAP_Dialog_Encoding(void)
{
	DELETEP (m_pEncTable);
	if(m_ppEncodings)
		delete [] m_ppEncodings;
}

// we will not use the value passed to us, but rather will reference
// ourselves into m_pEncTable; that way we do not have to worry about
// the string disappearing on us, nor do we need to clone it
void XAP_Dialog_Encoding::setEncoding(const XML_Char * pEncoding)
{
	UT_ASSERT(m_pEncTable);
	m_iSelIndex		= m_pEncTable->getIndxFromEncoding(pEncoding);
	m_pDescription	= m_pEncTable->getNthDescription(m_iSelIndex);
	m_pEncoding		= m_pEncTable->getNthEncoding(m_iSelIndex);
}

// in this case we do not need to worry about the lifespan of pDesc
// since we call it only internally, always referring back to m_pEncTable
void XAP_Dialog_Encoding::_setEncoding(const XML_Char * pDesc)
{
	UT_ASSERT(m_pEncTable);
	m_pDescription	= pDesc;
	m_pEncoding		= m_pEncTable->getEncodingFromDescription(pDesc);
}


XAP_Dialog_Encoding::tAnswer XAP_Dialog_Encoding::getAnswer(void) const
{
	return m_answer;
}

const XML_Char * XAP_Dialog_Encoding::getEncoding() const
{
	return m_pEncoding;
}
