/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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



#ifndef EV_COCOAMOUSE_H
#define EV_COCOAMOUSE_H

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_EditBits.h"

/*****************************************************************/

class EV_CocoaMouse : public EV_Mouse
{
public:
	EV_CocoaMouse(EV_EditEventMapper * pEEM);

	void mouseClick(AV_View* pView, NSEvent* e, NSView* hitView);
	void mouseUp(AV_View* pView, NSEvent* e, NSView* hitView);
	void mouseMotion(AV_View* pView, NSEvent *event, NSView *hitView);
static EV_EditMouseButton _convertMouseButton(int btn, bool rightBtn);
static EV_EditModifierState _convertModifierState(unsigned int modifiers, bool & rightBtn);

protected:
};

#endif // EV_COCOAMOUSE_H
