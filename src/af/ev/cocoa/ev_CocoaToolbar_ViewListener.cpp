/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ev_CocoaToolbar_ViewListener.h"
#include "ev_CocoaToolbar.h"

EV_CocoaToolbar_ViewListener::EV_CocoaToolbar_ViewListener(EV_CocoaToolbar * pCocoaToolbar,
														 AV_View * pView)
{
	m_pCocoaToolbar = pCocoaToolbar;
	m_pView = pView;
}

bool EV_CocoaToolbar_ViewListener::notify(AV_View * pView, const AV_ChangeMask mask)
{
	UT_ASSERT(pView == m_pView);
	
	//UT_DEBUGMSG(("CocoaToolbar_ViewListener::notify [view %p tb 0x%08lx][mask 0x%08lx]\n",
	//			 pView,this,mask));
	
	// this code really could be in EV_CocoaToolbar, but I didn't want
	// to multiple-inherit it.

	return m_pCocoaToolbar->refreshToolbar(pView,mask);
}
