/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ap_Dialog_New.h"
#include "ut_types.h"
#include "ap_Strings.h"

AP_Dialog_New::AP_Dialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent (pDlgFactory, id, "interface/dialognew.html"), 
	m_answer (AP_Dialog_New::a_CANCEL), m_openType (AP_Dialog_New::open_New),
	m_fileName(0)
{
}

AP_Dialog_New::~AP_Dialog_New()
{
	FREEP(m_fileName);
}

void AP_Dialog_New::setFileName(const char * name)
{
	FREEP(m_fileName);
	m_fileName = UT_strdup (name);
}

/**************************************************************************/
/**************************************************************************/

