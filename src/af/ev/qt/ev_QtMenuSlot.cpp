/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* 
 * Copyright (C) 2013 Serhat Kiyak <serhatkiyak91@gmail.com>
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

#include "ev_QtMenu.h"
#include "ev_QtMenuSlot.h"
#include "xap_Frame.h"
#include "ev_Menu_Labels.h"

#include "moc_ev_QtMenuSlot.cpp"

EV_QtMenuSlot::EV_QtMenuSlot(EV_QtMenu * pQtMenu, XAP_Menu_Id id) 
	: m_pQtMenu(pQtMenu),
	  m_id(id)
{
}

EV_QtMenuSlot::~EV_QtMenuSlot() 
{
}

void EV_QtMenuSlot::onTrigger()
{
	m_pQtMenu->menuEvent(m_id);
}

void EV_QtMenuSlot::onToggle(bool checked)
{
	if(checked)
	{
		// WL_REFACTOR: redundant code
		XAP_Frame * pFrame = m_pQtMenu->getFrame();
		UT_return_if_fail(pFrame);
		EV_Menu_Label * pLabel = m_pQtMenu->getLabelSet()->getLabel(m_id);
		if (!pLabel)
		{
			pFrame->setStatusMessage(NULL);
			return;
		}

		const char * szMsg = pLabel->getMenuStatusMessage();
		if (!szMsg || !*szMsg)
			szMsg = "TODO This menu item doesn't have a StatusMessage defined.";	
		pFrame->setStatusMessage(szMsg);
	}
	else
	{
		XAP_Frame * pFrame = m_pQtMenu->getFrame();
		UT_return_if_fail(pFrame);

		pFrame->setStatusMessage(NULL);
	}
}
