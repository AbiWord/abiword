/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#include "xap_Frame.h"
#include "xap_QtApp.h"
#include "ev_QtMenu.h"

EV_QtMenu::EV_QtMenu(XAP_QtApp * pQtApp, 
						 XAP_Frame *pFrame, 
						 const char * szMenuLayoutName,
						 const char * szMenuLabelSetName)
	: EV_Menu(pQtApp, pQtApp->getEditMethodContainer(), szMenuLayoutName, szMenuLabelSetName),
	  m_pQtApp(pQtApp),
	  m_pFrame(pFrame),
	  // there are 189 callbacks at the moment. This is a large vector, but we do not want
	  // it to grow too fast (it has the lifespan of the application, and so we do not
	  // want too much empty space in it)
	  m_vecCallbacks(189)
{
}

EV_QtMenu::~EV_QtMenu()
{
	//TODO
	m_vecMenuWidgets.clear();
}

XAP_Frame * EV_QtMenu::getFrame()
{
	return m_pFrame;
}

bool EV_QtMenu::menuEvent(XAP_Menu_Id id)
{
	//TODO
	return true;
}

bool EV_QtMenu::synthesizeMenu(QMenu * menuRoot, bool isPopup)
{
	//TODO
	return true;
}

bool EV_QtMenu::_refreshMenu(AV_View * pView, QMenu * menuRoot)
{
	//TODO
	return true;
}


bool EV_QtMenu::_doAddMenuItem(UT_uint32 layout_pos)
{
	//TODO
	return true;
}

