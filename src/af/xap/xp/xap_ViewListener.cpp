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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */



#include <stdio.h>
#include <stdlib.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_Frame.h"
#include "xap_ViewListener.h"
#include "xav_Listener.h"
#include "xav_View.h"


ap_ViewListener::ap_ViewListener(XAP_Frame* pFrame)
{
	m_pFrame = pFrame;
}

ap_ViewListener::~ap_ViewListener()
{
}

UT_Bool ap_ViewListener::notify(AV_View * pView, const AV_ChangeMask mask)
{
	UT_ASSERT(pView);
	UT_ASSERT(pView==m_pFrame->getCurrentView());

	if ((mask & AV_CHG_DIRTY) || (mask & AV_CHG_FILENAME))
	{
		// NOTE: could pass mask here to make updateTitle more efficient
		m_pFrame->updateTitle();
	}

	return UT_TRUE;
}

