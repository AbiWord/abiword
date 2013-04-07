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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_string.h"
#include "ap_Dialog_PageSetup.h"

AP_Dialog_PageSetup::AP_Dialog_PageSetup (XAP_DialogFactory * pDlgFactory,
					  XAP_Dialog_Id id)
:	XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogpagesetup"),
	m_answer(a_OK),
	m_PageSize(fp_PageSize::psLetter), // isn't this leaked when setPageSize called?
	m_PageUnits(DIM_IN),
	m_PageOrientation(PORTRAIT),
	m_PageScale(100),
	m_MarginUnits(DIM_IN),
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


bool AP_Dialog_PageSetup::validatePageSettings(void) const
{
	// Require at least 0.3in for headers and footers.

/*	double fudge = UT_convertInchesToDimension(0.3, m_MarginUnits);


  (m_MarginFooter + fudge > m_MarginBottom) || 
  (m_MarginHeader + fudge > m_MarginTop)) 
*/

	if ( (m_MarginLeft + m_MarginRight >= m_PageSize.Width(m_MarginUnits)) ||
		 (m_MarginTop + m_MarginBottom >= m_PageSize.Height(m_MarginUnits)) )
		return false;

	return true;
}
