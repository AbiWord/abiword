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
#include "xap_FontPreview.h"

class XAP_QNXApp;
class AP_QNXToolbar_Icons;
class EV_QNXToolbar_ViewListener;

class EV_QNXToolbar : public EV_Toolbar
{
public:
	EV_QNXToolbar(XAP_QNXApp * pQNXApp, XAP_Frame * pFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	
	virtual ~EV_QNXToolbar(void);

	bool toolbarEvent(XAP_Toolbar_Id id, UT_UCS4String pData, UT_uint32 dataLength);
	virtual bool synthesize(void);
	bool bindListenerToView(AV_View * pView);
	bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);

	XAP_QNXApp *	getApp(void);
	XAP_Frame * getFrame(void);

	virtual void	show();
	virtual void	hide();
	
	XAP_FontPreview * m_pFontPreview;

protected:
	void							_releaseListener(void);
	
	XAP_QNXApp *				m_pQNXApp;
	XAP_Frame *				m_pFrame;
	EV_QNXToolbar_ViewListener *m_pViewListener;
	AV_ListenerId				m_lid;	/* view listener id */

	PtWidget_t *				m_wToolbar;
	PtWidget_t * 				m_wToolbarGroup;
	AP_QNXToolbar_Icons *		m_pQNXToolbarIcons;
	UT_Vector					m_vecToolbarWidgets;
};

#endif /* EV_QNXTOOLBAR_H */
