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

#ifndef EV_QNXTOOLBAR_H
#define EV_QNXTOOLBAR_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"
#include <Pt.h>

class XAP_QNXApp;
class XAP_QNXFrame;
class AP_QNXToolbar_Icons;
class EV_QNXToolbar_ViewListener;

class EV_QNXToolbar : public EV_Toolbar
{
public:
	EV_QNXToolbar(XAP_QNXApp * pQNXApp, XAP_QNXFrame * pQNXFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	virtual ~EV_QNXToolbar(void);

	UT_Bool toolbarEvent(XAP_Toolbar_Id id, UT_UCSChar * pData, UT_uint32 dataLength);
	virtual UT_Bool synthesize(void);
	UT_Bool bindListenerToView(AV_View * pView);
	UT_Bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);

	XAP_QNXApp *	getApp(void);
	XAP_QNXFrame * getFrame(void);

	virtual void	show();
	virtual void	hide();
	
protected:
	void							_releaseListener(void);
	
	XAP_QNXApp *				m_pQNXApp;
	XAP_QNXFrame *				m_pQNXFrame;
	EV_QNXToolbar_ViewListener *m_pViewListener;
	AV_ListenerId				m_lid;	/* view listener id */

	PtWidget_t *				m_wToolbar;
	PtWidget_t * 				m_wToolbarGroup;
	AP_QNXToolbar_Icons *		m_pQNXToolbarIcons;
	UT_Vector					m_vecToolbarWidgets;
};

#endif /* EV_QNXTOOLBAR_H */
