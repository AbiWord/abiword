/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#ifndef EV_BEOSKEYBOARD_H
#define EV_BEOSKEYBOARD_H

#include "ev_Keyboard.h"
#include "ev_EditBits.h"

class AV_View;
class XAP_BeOSApp;
class XAP_BeOSFrame;
class BMessage;

class ev_BeOSKeyboard : public EV_Keyboard
{
public:
	ev_BeOSKeyboard(EV_EditEventMapper * pEEM);
	virtual ~ev_BeOSKeyboard(void);
	
	//This should be fixed to be placed on the view ...
	UT_Bool synthesize(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame);
	UT_Bool keyPressEvent(AV_View* pView, BMessage *msg);

};

#endif // EV_BEOSKEYBOARD_H

