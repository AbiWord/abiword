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

#include <QMainWindow>

#include "ev_QtMenuBar.h"
#include "xap_Frame.h"
#include "xap_QtFrameImpl.h"

EV_QtMenuBar::EV_QtMenuBar(XAP_QtApp * pQtApp,
							   XAP_Frame * pFrame,
							   const char * szMenuLayoutName,
							   const char * szMenuLabelSetName)
	: EV_QtMenu(pQtApp, pFrame, szMenuLayoutName, szMenuLabelSetName)
{
}

EV_QtMenuBar::~EV_QtMenuBar()
{
	delete m_QMenuBar;
}

void  EV_QtMenuBar::destroy(void)
{
	//TODO
}

bool EV_QtMenuBar::synthesizeMenuBar()
{
	QMainWindow * wTopLevel = static_cast<XAP_QtFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevel();
	m_QMenuBar = new QMenuBar();
	wTopLevel->setMenuBar(m_QMenuBar);
	synthesizeMenu(m_QMenuBar, false);
	wTopLevel->show();
	return true;
}


bool EV_QtMenuBar::rebuildMenuBar()
{
	//TODO
	return true;
}

bool EV_QtMenuBar::refreshMenu(AV_View * pView)
{
	//TODO
	return true;
}
