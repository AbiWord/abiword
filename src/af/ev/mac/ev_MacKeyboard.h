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
 



#ifndef EV_MACKEYBOARD_H
#define EV_MACKEYBOARD_H

#include "ev_Keyboard.h"
#include "ev_EditBits.h"

// TODO should pView be passed in on each method or
// TODO should we pass it to the ev_keyboard base class on the
// TODO constructor ??

class AV_View;

class ev_MacKeyboard : public EV_Keyboard
{
public:
	ev_MacKeyboard(EV_EditEventMapper * pEEM);
};

#endif /* EV_MACKEYBOARD_H */
