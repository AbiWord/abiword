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

#ifndef EV_UNIXTOOLBAR_H
#define EV_UNIXTOOLBAR_H

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "ap_Types.h"
#include "ev_Toolbar.h"
#include "av_Listener.h"

class AP_UnixApp;
class AP_UnixFrame;
class AP_UnixToolbar_Icons;
class EV_UnixToolbar_ViewListener;


class EV_UnixToolbar : public EV_Toolbar
{
public:
	EV_UnixToolbar(AP_UnixApp * pUnixApp, AP_UnixFrame * pUnixFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	~EV_UnixToolbar(void);

	UT_Bool toolbarEvent(AP_Toolbar_Id id, UT_UCSChar * pData, UT_uint32 dataLength);
	UT_Bool synthesize(void);
	UT_Bool bindListenerToView(AV_View * pView);
	UT_Bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);

protected:
	void							_releaseListener(void);
	
	AP_UnixApp *					m_pUnixApp;
	AP_UnixFrame *					m_pUnixFrame;
	EV_UnixToolbar_ViewListener *	m_pViewListener;
	AV_ListenerId					m_lid;	/* view listener id */

	GtkWidget *						m_wToolbar;
	GtkWidget * 					m_wHandleBox;
	AP_UnixToolbar_Icons *			m_pUnixToolbarIcons;
	UT_Vector						m_vecToolbarWidgets;
};

#endif /* EV_UNIXTOOLBAR_H */
