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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_Mouse.h"
#include "fv_View.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"

EV_Mouse::EV_Mouse(EV_EditEventMapper * pEEM)
{
	UT_ASSERT(pEEM);

	m_pEEM = pEEM;
}

UT_Bool EV_Mouse::invokeMouseMethod(FV_View * pView,
									EV_EditMethod * pEM,
									UT_uint32 iPrefixCount,
									UT_uint32 xPos,
									UT_uint32 yPos)
{
	UT_ASSERT(pView);
	UT_ASSERT(pEM);

	UT_DEBUGMSG(("invokeMouseMethod: %s repeat %d at (%d %d)\n",
				 pEM->getName(),iPrefixCount,xPos,yPos));

	EV_EditMethodType t = pEM->getType();

	if (((t & EV_EMT_ALLOWMULTIPLIER) == 0) && (iPrefixCount != 1))
	{
		// they gave a prefix multiplier and this method doesn't permit it.
		// TODO should be beep or complain or just silently fail ?? 
		UT_DEBUGMSG(("    invoke aborted due to multiplier\n"));
		return UT_FALSE;
	}

	if ((t & EV_EMT_REQUIREDATA) != 0)
	{
		// they bound a mouse event to something which requires a character.
		// TODO we should ding this back when the binding was made ??
		UT_DEBUGMSG(("    invoke aborted due to lack of data\n"));
		return UT_FALSE;
	}

	EV_EditMethodCallData emcd(1);
	emcd.m_xPos = xPos;
	emcd.m_yPos = yPos;
	(*pEM->getFn())(pView,&emcd);

	return UT_TRUE;
}

