/* AbiSource Program Utilities
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

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_Scrollbar_ViewListener.h"
#include "xap_Frame.h"

/*****************************************************************/

ap_Scrollbar_ViewListener::ap_Scrollbar_ViewListener(AP_Frame * pFrame,
													 AV_View * pView)
{
	m_pFrame = pFrame;
	m_pView = pView;
}

UT_Bool ap_Scrollbar_ViewListener::notify(AV_View * pView, const AV_ChangeMask mask)
{
	UT_ASSERT(pView == m_pView);
	
	UT_DEBUGMSG(("Scrollbar_ViewListener::notify [view 0x%08lx tb 0x%08lx][mask 0x%08lx]\n",
				 pView,this,mask));
	
	if (mask & (AV_CHG_PAGECOUNT | AV_CHG_WINDOWSIZE))
		m_pFrame->setYScrollRange();

	return UT_TRUE;
}
