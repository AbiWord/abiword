/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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


#ifndef EV_COCOAKEYBOARD_H
#define EV_COCOAKEYBOARD_H

#import <Cocoa/Cocoa.h>

#include "ev_Keyboard.h"
#include "ev_EditBits.h"

class AV_View;


class ev_CocoaKeyboard : public EV_Keyboard
{
public:
	ev_CocoaKeyboard(EV_EditEventMapper * pEEM);
	virtual ~ev_CocoaKeyboard(void);

	void insertTextEvent(AV_View * pView, NSString* s);
	bool keyPressEvent(AV_View * pView, NSEvent* e);
private:
	bool _dispatchKey(AV_View * pView, UT_uint32 charData, EV_EditBits state);
};

#endif // EV_COCOAKEYBOARD_H

