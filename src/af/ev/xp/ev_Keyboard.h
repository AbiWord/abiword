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


#ifndef EV_KEYBOARD_H
#define EV_KEYBOARD_H

#include "ut_types.h"

class EV_EditEventMapper;
class EV_EditMethod;
class AV_View;

class ABI_EXPORT EV_Keyboard
{
public:
	EV_Keyboard(EV_EditEventMapper * pEEM);
	virtual ~EV_Keyboard(void);

	bool invokeKeyboardMethod(AV_View * pView,
								 EV_EditMethod * pEM,
								 const UT_UCSChar * pData,
								 UT_uint32 dataLength);
	void setEditEventMap(EV_EditEventMapper * pEEM);

protected:
	EV_EditEventMapper *	m_pEEM;
};

#endif /* EV_KEYBOARD_H */
