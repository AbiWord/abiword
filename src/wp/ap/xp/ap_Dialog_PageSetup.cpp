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

#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ap_Dialog_PageSetup.h"

AP_Dialog_PageSetup::AP_Dialog_PageSetup (XAP_DialogFactory * pDlgFactory,
					  XAP_Dialog_Id id)
:	XAP_Dialog_NonPersistent(pDlgFactory,id),
	m_answer(a_OK),
	m_PageSize(fp_PageSize::Letter),
	m_PageUnits(fp_PageSize::inch),
	m_PageOrientation(PORTRAIT),
	m_PageScale(100),
	m_MarginUnits(fp_PageSize::inch),
	m_MarginTop(1.0f),
	m_MarginBottom(1.0f),
	m_MarginLeft(1.0f),
	m_MarginRight(1.0f),
	m_MarginHeader(0.0f),
	m_MarginFooter(0.0f)
{ 
}

AP_Dialog_PageSetup::~AP_Dialog_PageSetup(void)
{
  //
}





