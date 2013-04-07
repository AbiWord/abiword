/* AbiSource Application Framework
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

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_ViewListener.h"
#include "xav_Listener.h"
#include "xav_View.h"
#include "ev_Mouse.h"
#include "ev_Keyboard.h"


ap_ViewListener::ap_ViewListener(XAP_Frame* pFrame)
{
	m_pFrame = pFrame;
}

ap_ViewListener::~ap_ViewListener()
{
}

bool ap_ViewListener::notify(AV_View * pView, const AV_ChangeMask mask)
{
	UT_UNUSED(pView);
	UT_ASSERT(pView);
	UT_ASSERT(pView==m_pFrame->getCurrentView());

	if ((mask & AV_CHG_DIRTY) || (mask & AV_CHG_FILENAME))
	{
		// NOTE: could pass mask here to make updateTitle more efficient
		m_pFrame->updateTitle();
	}
	if(mask & AV_CHG_INPUTMODE)
	{
		m_pFrame->getMouse()->setEditEventMap(XAP_App::getApp()->getEditEventMapper());
		m_pFrame->getKeyboard()->setEditEventMap(XAP_App::getApp()->getEditEventMapper());
	}
	return true;
}

