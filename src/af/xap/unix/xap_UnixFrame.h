/* AbiWord
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


#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

#include "ap_Frame.h"
class AP_UnixAp;
class ev_UnixKeyboard;
class EV_UnixMouse;

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** unix-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class AP_UnixFrame : public AP_Frame
{
public:
	AP_UnixFrame(AP_UnixAp * ap);
	~AP_UnixFrame(void);

	virtual UT_Bool				initialize(int argc, char ** argv);

protected:
	// TODO see why ev_UnixKeyboard has lowercase prefix...
	AP_UnixAp *					m_pUnixAp;
	ev_UnixKeyboard *			m_pUnixKeyboard;
	EV_UnixMouse *				m_pUnixMouse;
};

#endif /* AP_UNIXFRAME_H */
