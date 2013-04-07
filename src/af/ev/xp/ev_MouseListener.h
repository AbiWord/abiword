/* AbiSource Program Utilities
 * Copyright (C) 2007 One Laptop Per Child
 * Author: Marc Maurer <uwog@uwog.net>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef EV_MOUSELISTENER_H
#define EV_MOUSELISTENER_H

class EV_Mouse;

class ABI_EXPORT EV_MouseListener
{
public:
	virtual ~EV_MouseListener() {}
	// coordinates are in layout units
	virtual void signalMouse(EV_EditBits eb, UT_sint32 xPos, UT_sint32 yPos) = 0;
	virtual void removeMouse(EV_Mouse* pMouse) = 0;
};

#endif /* EV_MOUSELISTENER_H */
