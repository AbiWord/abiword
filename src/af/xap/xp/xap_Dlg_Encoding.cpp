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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Encoding.h"

#include "xap_Dlg_Encoding.h"

/*****************************************************************/

#if 0
static int s_compareQ(const void * a, const void * b)
{
	const gchar ** A = static_cast<const gchar **>(a);
	const gchar ** B = static_cast<const gchar **>(b);

	return strcmp(*A,*B);
}
#endif

XAP_Dialog_Encoding::XAP_Dialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	m_answer			= a_CANCEL;
	m_pDescription		= NULL;
	m_pEncoding			= NULL;
	m_pEncTable = new UT_Encoding;
	
	UT_ASSERT(m_pEncTable);
	m_iEncCount = m_pEncTable->getCount();
	m_ppEncodings = new const gchar * [m_iEncCount];

	// Build array of encoding names
	for(UT_uint32 i = 0; i < m_iEncCount; i++)
		m_ppEncodings[i] = m_pEncTable->getNthDescription(i);
}

XAP_Dialog_Encoding::~XAP_Dialog_Encoding(void)
{
	DELETEP (m_pEncTable);
	DELETEPV(m_ppEncodings);
}

// we will not use the value passed to us, but rather will reference
// ourselves into m_pEncTable; that way we do not have to worry about
// the string disappearing on us, nor do we need to clone it
void XAP_Dialog_Encoding::setEncoding(const gchar * pEncoding)
{
	UT_return_if_fail(m_pEncTable);
	m_iSelIndex		= m_pEncTable->getIndxFromEncoding(pEncoding);
	m_pDescription	= m_pEncTable->getNthDescription(m_iSelIndex);
	m_pEncoding		= m_pEncTable->getNthEncoding(m_iSelIndex);
}

// in this case we do not need to worry about the lifespan of pDesc
// since we call it only internally, always referring back to m_pEncTable
void XAP_Dialog_Encoding::_setEncoding(const gchar * pDesc)
{
	UT_return_if_fail(m_pEncTable);
	m_pDescription	= pDesc;
	m_pEncoding		= m_pEncTable->getEncodingFromDescription(pDesc);
}


XAP_Dialog_Encoding::tAnswer XAP_Dialog_Encoding::getAnswer(void) const
{
	return m_answer;
}

const gchar * XAP_Dialog_Encoding::getEncoding() const
{
	return m_pEncoding;
}
