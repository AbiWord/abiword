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
 



#ifndef EV_WIN32MOUSE_H
#define EV_WIN32MOUSE_H

#include "ev_Mouse.h"
#include "ev_EditBits.h"

// TODO should pView be passed in on each method or
// TODO should we pass it into EV_Mouse on the constructor ??
// TODO EV_<platform>Mouse and EV_EditEventMapper are
// TODO unique for each document, although the EditBindings
// TODO may be global.  (ev_<platform>Mouse could be global
// TODO i suppose.)

class FV_View;


class ev_Win32Mouse : public EV_Mouse
{
public:
	ev_Win32Mouse(EV_EditEventMapper * pEEM);

	void reset(void);

	void onButtonDown(FV_View * pView, HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);
	void onButtonUp  (FV_View * pView, HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);
	void onButtonMove(FV_View * pView, HWND hWnd, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);
	void onDoubleClick(FV_View * pView, HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);

protected:
	UT_uint32			m_iCaptureCount;
	EV_EditMouseButton	m_embCaptured;
};

#endif /* EV_WIN32MOUSE_H */
