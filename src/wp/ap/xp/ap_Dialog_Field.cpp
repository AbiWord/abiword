/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include "ap_Dialog_Field.h"

AP_Dialog_Field::AP_Dialog_Field(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogfield.html")
{
    m_answer = a_OK;
	m_iTypeIndex = 0;
    m_iFormatIndex = 0;
    m_pParameter = 0;
}

AP_Dialog_Field::~AP_Dialog_Field()
{
	FREEP(m_pParameter);
}

AP_Dialog_Field::tAnswer AP_Dialog_Field::getAnswer(void) const
{
    return m_answer;
}

const char *AP_Dialog_Field::GetFieldFormat(void) const
{
    return (const char *)fp_FieldFmts[m_iFormatIndex].m_Tag;

}

void AP_Dialog_Field::setParameter(const XML_Char * pParam)
{
	if(m_pParameter)
		FREEP( m_pParameter );
	
	m_pParameter = UT_strdup(pParam);
}
