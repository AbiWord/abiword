/* AbiSource Application Framework
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



#ifndef AP_VIEWLISTENER_H
#define AP_VIEWLISTENER_H

#include "ut_types.h"
#include "xav_Listener.h"

class XAP_Frame;
class AV_View;

/*
	The ap_ViewListener class handles UI change notifications from an AV_View
	to its associated XAP_Frame.  
*/

class ap_ViewListener : public AV_Listener
{
public:
	ap_ViewListener(XAP_Frame* pFrame);
	virtual ~ap_ViewListener();

	virtual UT_Bool		notify(AV_View * pView, const AV_ChangeMask mask);

protected:
	XAP_Frame*		m_pFrame;
};

#endif /* AP_VIEWLISTENER_H */
