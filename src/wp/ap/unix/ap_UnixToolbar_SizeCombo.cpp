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
#include "ap_UnixToolbar_SizeCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
EV_Toolbar_Control * AP_UnixToolbar_SizeCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_UnixToolbar_SizeCombo * p = new AP_UnixToolbar_SizeCombo(pToolbar,id);
	return p;
}

AP_UnixToolbar_SizeCombo::AP_UnixToolbar_SizeCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_SIZE);
	m_nPixels =60;		// TODO: do a better calculation
	m_nLimit = 10;    
}

AP_UnixToolbar_SizeCombo::~AP_UnixToolbar_SizeCombo(void)
{
	// nothing to purge.  contents are static strings
}

/*****************************************************************/

UT_Bool AP_UnixToolbar_SizeCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();
	{
	    // populate the vector	
	    int sz = XAP_EncodingManager::fontsizes_list.size();
	    for(int i=0;i<sz;++i) {
		m_vecContents.addItem((void *) XAP_EncodingManager::fontsizes_list.nth2(i));
	    };
	}
	// TODO: may want to populate this based on current font instead?
	return UT_TRUE;
}

