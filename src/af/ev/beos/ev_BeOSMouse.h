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
 


#ifndef EV_BEOSMOUSE_H
#define EV_BEOSMOUSE_H

#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_EditBits.h"

#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

class BMessage;

/*****************************************************************/

class ev_BeOSMouse : public EV_Mouse
{
public:
	ev_BeOSMouse(EV_EditEventMapper * pEEM);
	void mouseUp(AV_View* pView, BMessage *msg);
	void mouseClick(AV_View* pView, BMessage *msg);
	void mouseMotion(AV_View* pView, BMessage *msg);
	UT_Bool synthesize(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame);
private:
	UT_uint32		m_clickState;   /* {NoClick,SingleClick,DoubleClick} */
        EV_EditMouseContext	m_contextState; /* mouse context of click */ 
};

#endif // EV_BEOSMOUSE_H
