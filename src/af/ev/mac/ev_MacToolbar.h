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

#ifndef EV_MACTOOLBAR_H
#define EV_MACTOOLBAR_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"

class AP_MacApp;
class XAP_MacFrame;
class AP_MacToolbar_Icons;
class EV_MacToolbar_ViewListener;
class EV_Toolbar_Action;

// HACK: it'd be nice to guarantee that menu and toolbar IDs don't overlap
#ifdef AP_MENU_ID__BOGUS2__
#define _ev_MENU_OFFSET		AP_MENU_ID__BOGUS2__
#else
#define _ev_MENU_OFFSET		1000
#endif

class EV_MacToolbar : public EV_Toolbar
{
public:
	EV_MacToolbar(AP_MacApp * pMacApp, XAP_MacFrame * pMacFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	~EV_MacToolbar(void);

	UT_Bool toolbarEvent(AP_Toolbar_Id id,
						 UT_UCSChar * pData = 0,
						 UT_uint32 dataLength = 0);
	UT_Bool synthesize(void);
	UT_Bool bindListenerToView(AV_View * pView);
	UT_Bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);
	
	WindowPtr getWindow(void) const;
	UT_Bool getToolTip(long lParam);
	
private:
	AP_MacApp *					m_pMacApp;
	XAP_MacFrame *				m_pMacFrame;
	EV_MacToolbar_ViewListener *	m_pViewListener;
	AV_ListenerId					m_lid;	/* view listener id */

	AP_MacToolbar_Icons *			m_pMacToolbarIcons;
	UT_Vector						m_vecToolbarWidgets;
	WindowPtr						m_hwnd;
	
	UT_Bool _refreshItem(AV_View * pView, const EV_Toolbar_Action * pAction, AP_Toolbar_Id id);
	UT_Bool _refreshID(AP_Toolbar_Id id);

};

#endif /* EV_MACTOOLBAR_H */
