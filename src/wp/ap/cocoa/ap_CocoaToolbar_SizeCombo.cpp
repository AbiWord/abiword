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

#include "ut_assert.h"
#include "ut_vector.h"
#include "ap_CocoaToolbar_SizeCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"
#include "xap_EncodingManager.h"

/*****************************************************************/
EV_Toolbar_Control * AP_CocoaToolbar_SizeCombo::static_constructor(EV_Toolbar * pToolbar,
								   XAP_Toolbar_Id id)
{
	AP_CocoaToolbar_SizeCombo * p = new AP_CocoaToolbar_SizeCombo(pToolbar,id);
	return p;
}

AP_CocoaToolbar_SizeCombo::AP_CocoaToolbar_SizeCombo(EV_Toolbar * pToolbar,
						     XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_DEBUG_ONLY_ARG(id)

	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_SIZE);
	m_nPixels =60;		// TODO: do a better calculation
	m_nLimit = 10;    
}

AP_CocoaToolbar_SizeCombo::~AP_CocoaToolbar_SizeCombo(void)
{
	// nothing to purge.  contents are static strings
}

/*****************************************************************/

bool AP_CocoaToolbar_SizeCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();
	{
	    // populate the vector	
	    int sz = XAP_EncodingManager::fontsizes_mapping.size();
	    for(int i=0;i<sz;++i) {
			m_vecContents.addItem(static_cast<const char*>(XAP_EncodingManager::fontsizes_mapping.nth2(i)));
	    };
	}
	// TODO: may want to populate this based on current font instead?
	return true;
}

