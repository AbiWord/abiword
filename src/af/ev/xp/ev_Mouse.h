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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */




#ifndef EV_MOUSE_H
#define EV_MOUSE_H

#include <vector>
#include "ut_types.h"
#include "ev_EditBits.h"

class EV_EditEventMapper;
class EV_EditMethod;
class AV_View;
class EV_MouseListener;

class ABI_EXPORT EV_Mouse
{
public:
	EV_Mouse(EV_EditEventMapper * pEEM);
	~EV_Mouse();

	bool invokeMouseMethod(AV_View * pView,
							  EV_EditMethod * pEM,
							  UT_sint32 xPos,
							  UT_sint32 yPos);
	void setEditEventMap(EV_EditEventMapper * pEEM);
	void clearMouseContext(void);

	// mouse listeners
	void signal(EV_EditBits eb, UT_sint32 xPos, UT_sint32 yPos);
	UT_sint32 registerListener(EV_MouseListener* pListener);
	void unregisterListener(UT_sint32 iListenerId);
	void removeListeners();
protected:
	EV_EditEventMapper *	m_pEEM;
	UT_uint32			m_clickState;	/* {NoClick,SingleClick,DoubleClick} */
	EV_EditMouseContext	m_contextState;	/* mouse context of click */
private:
	std::vector<EV_MouseListener*> m_listeners;
};

#endif /* EV_MOUSE_H */
