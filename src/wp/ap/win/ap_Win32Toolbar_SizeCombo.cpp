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

#include <windows.h>
#include "ut_assert.h"
#include "ut_vector.h"
#include "ap_Win32Toolbar_SizeCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"

/*****************************************************************/
EV_Toolbar_Control * AP_Win32Toolbar_SizeCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_Win32Toolbar_SizeCombo * p = new AP_Win32Toolbar_SizeCombo(pToolbar,id);
	return p;
}

AP_Win32Toolbar_SizeCombo::AP_Win32Toolbar_SizeCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_SIZE);

	m_nPixels = 40;		// TODO: do a better calculation
	m_nLimit = 4;
}

AP_Win32Toolbar_SizeCombo::~AP_Win32Toolbar_SizeCombo(void)
{
	// nothing to purge.  contents are static strings
}

/*****************************************************************/

UT_Bool AP_Win32Toolbar_SizeCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector
	m_vecContents.addItem("8");
	m_vecContents.addItem("9");
	m_vecContents.addItem("10");
	m_vecContents.addItem("11");
	m_vecContents.addItem("12");
	m_vecContents.addItem("14");
	m_vecContents.addItem("16");
	m_vecContents.addItem("18");
	m_vecContents.addItem("20");
	m_vecContents.addItem("22");
	m_vecContents.addItem("24");
	m_vecContents.addItem("26");
	m_vecContents.addItem("28");
	m_vecContents.addItem("36");
	m_vecContents.addItem("48");
	m_vecContents.addItem("72");

	// TODO: may want to populate this based on current font instead?
	return UT_TRUE;
}

