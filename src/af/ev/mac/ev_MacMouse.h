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
 



#ifndef EV_MACMOUSE_H
#define EV_MACMOUSE_H

#include "ev_Mouse.h"
#include "ev_EditBits.h"

// TODO should pView be passed in on each method or
// TODO should we pass it into EV_Mouse on the constructor ??
// TODO EV_<platform>Mouse and EV_EditEventMapper are
// TODO unique for each document, although the EditBindings
// TODO may be global.  (ev_<platform>Mouse could be global
// TODO i suppose.)

class AV_View;


class EV_MacMouse : public EV_Mouse
{
public:
	EV_MacMouse(EV_EditEventMapper * pEEM);

	void reset(void);

	void onButtonDown(AV_View * pView, WindowPtr hWnd, EV_EditMouseButton emb, int fwKeys, int xPos, int yPos);
	void onButtonUp  (AV_View * pView, WindowPtr hWnd, EV_EditMouseButton emb, int fwKeys, int xPos, int yPos);
	void onButtonMove(AV_View * pView, WindowPtr hWnd, int fwKeys, int xPos, int yPos);
	void onDoubleClick(AV_View * pView, WindowPtr hWnd, EV_EditMouseButton emb, int fwKeys, int xPos, int yPos);

protected:
	UT_uint32			m_iCaptureCount;
	EV_EditMouseButton	m_embCaptured;
};

#endif /* EV_MACMOUSE_H */
