/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 
 



#ifndef EV_KEYBOARD_H
#define EV_KEYBOARD_H

#include "ut_types.h"

class EV_EditEventMapper;
class EV_EditMethod;
class FV_View;

class EV_Keyboard
{
public:
	EV_Keyboard(EV_EditEventMapper * pEEM);
	UT_Bool invokeKeyboardMethod(FV_View * pView,
								 EV_EditMethod * pEM,
								 UT_uint32 iPrefixCount,
								 UT_UCSChar * pData,
								 UT_uint32 dataLength);
	
protected:
	EV_EditEventMapper *	m_pEEM;
};

#endif /* EV_KEYBOARD_H */
