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
 



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_Mouse.h"
#include "xav_View.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_MouseListener.h"

EV_Mouse::EV_Mouse(EV_EditEventMapper * pEEM):
	m_clickState(0),
	m_contextState(EV_EMC_UNKNOWN)
{
	setEditEventMap(pEEM);
}

EV_Mouse::~EV_Mouse()
{
	removeListeners();
}

void EV_Mouse::clearMouseContext(void)
{
	m_clickState = 0;
	m_contextState = EV_EMC_UNKNOWN;
}

void EV_Mouse::setEditEventMap(EV_EditEventMapper * pEEM)
{
	UT_ASSERT(pEEM);
	m_pEEM = pEEM;
}

bool EV_Mouse::invokeMouseMethod(AV_View * pView,
									EV_EditMethod * pEM,
									UT_sint32 xPos,
									UT_sint32 yPos)
{
	UT_ASSERT(pView);
	UT_ASSERT(pEM);

//	UT_DEBUGMSG(("invokeMouseMethod: %s at (%d %d)\n",pEM->getName(),xPos,yPos));
	
	EV_EditMethodType t = pEM->getType();

	if ((t & EV_EMT_REQUIREDATA) != 0)
	{
		// they bound a mouse event to something which requires a character.
		// TODO we should ding this back when the binding was made ??
		UT_DEBUGMSG(("    invoke aborted due to lack of data\n"));
		return false;
	}

	EV_EditMethodCallData emcd;
	emcd.m_xPos = xPos;
	emcd.m_yPos = yPos;
	pEM->Fn(pView,&emcd);

	return true;
}

void EV_Mouse::signal(EV_EditBits eb, UT_sint32 xPos, UT_sint32 yPos)
{
	for (std::vector<EV_MouseListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); it++)
	{
		EV_MouseListener* pListener = *it;
		if (pListener)
			pListener->signalMouse(eb, xPos, yPos);
	}
}

UT_sint32 EV_Mouse::registerListener(EV_MouseListener* pListener)
{
	UT_return_val_if_fail(pListener, -1);
	m_listeners.push_back(pListener); // TODO: look for gaps that we can reuse, caused by unregister calls - MARCM
	return m_listeners.size()-1;
}

void EV_Mouse::unregisterListener(UT_sint32 iListenerId)
{
	UT_return_if_fail(iListenerId >= 0);
	UT_return_if_fail(iListenerId >= 0 && iListenerId < static_cast<UT_sint32>(m_listeners.size()));	
	m_listeners[iListenerId] = NULL;
}

void EV_Mouse::removeListeners()
{
	for (UT_uint32 i = 0; i < m_listeners.size(); i++)
	{
		EV_MouseListener* pListener = m_listeners[i];
		UT_continue_if_fail(pListener);
		pListener->removeMouse(this);
	}
	m_listeners.clear();
}
