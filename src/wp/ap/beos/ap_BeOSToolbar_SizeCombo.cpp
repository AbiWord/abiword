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

#include "ut_assert.h"
#include "ut_vector.h"
#include "ap_BeOSToolbar_SizeCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"

/*****************************************************************/
EV_Toolbar_Control * AP_BeOSToolbar_SizeCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_BeOSToolbar_SizeCombo * p = new AP_BeOSToolbar_SizeCombo(pToolbar,id);
	return p;
}

AP_BeOSToolbar_SizeCombo::AP_BeOSToolbar_SizeCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_SIZE);

	m_nPixels = 40;		// TODO: do a better calculation
	m_nLimit = 4;
}

AP_BeOSToolbar_SizeCombo::~AP_BeOSToolbar_SizeCombo(void)
{
	// nothing to purge.  contents are static strings
}

/*****************************************************************/

UT_Bool AP_BeOSToolbar_SizeCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector
	m_vecContents.addItem((void *)"8");
	m_vecContents.addItem((void *)"9");
	m_vecContents.addItem((void *)"10");
	m_vecContents.addItem((void *)"11");
	m_vecContents.addItem((void *)"12");
	m_vecContents.addItem((void *)"14");
	m_vecContents.addItem((void *)"16");
	m_vecContents.addItem((void *)"18");
	m_vecContents.addItem((void *)"20");
	m_vecContents.addItem((void *)"22");
	m_vecContents.addItem((void *)"24");
	m_vecContents.addItem((void *)"26");
	m_vecContents.addItem((void *)"28");
	m_vecContents.addItem((void *)"36");
	m_vecContents.addItem((void *)"48");
	m_vecContents.addItem((void *)"72");

	// TODO: may want to populate this based on current font instead?
	return UT_TRUE;
}

