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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ap_Features.h"

#include "ap_Dialog_New.h"
#include "ut_types.h"

AP_Dialog_New::AP_Dialog_New(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent (pDlgFactory, id, "interface/dialognew"), 
	m_answer (AP_Dialog_New::a_CANCEL), m_openType (AP_Dialog_New::open_Template),
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
	m_fileName = g_strdup (name);
}

/**************************************************************************/
/**************************************************************************/

