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

#ifndef EV_QTMENUBAR_H
#define EV_QTMENUBAR_H

#include <QMenuBar>

#include "ev_QtMenu.h"

class AV_View;
class XAP_QtApp;
class XAP_QtFrame;


/*****************************************************************/

class EV_QtMenuBar : public EV_QtMenu
{
public:
	EV_QtMenuBar(XAP_QtApp * pQtApp,
		       XAP_Frame * pFrame,
		       const char * szMenuLayoutName,
		       const char * szMenuLabelSetName);
	virtual ~EV_QtMenuBar();

	virtual bool		synthesizeMenuBar();
	virtual bool		rebuildMenuBar();
	virtual bool		refreshMenu(AV_View * pView);
    virtual void        destroy(void);

    QMenuBar *         getMenuBar(void) const {return m_QMenuBar;}

protected:
	QMenuBar *		   m_QMenuBar;
};

#endif /* EV_QTMENUBAR_H */
