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

#ifndef AP_QTTOOLBAR_ZOOMCOMBO_H
#define AP_QTTOOLBAR_ZOOMCOMBO_H

#include "xap_Types.h"
#include "ev_Toolbar_Control.h"
class EV_Toolbar;

/*****************************************************************/

class AP_QtToolbar_ZoomCombo : public EV_Toolbar_Control
{
public:
	AP_QtToolbar_ZoomCombo(EV_Toolbar * pToolbar, XAP_Toolbar_Id id);
	virtual ~AP_QtToolbar_ZoomCombo(void);

	virtual bool		populate(void);

	static EV_Toolbar_Control *		static_constructor(EV_Toolbar *, XAP_Toolbar_Id id);

protected:
};

#endif /* AP_QTTOOLBAR_ZOOMCOMBO_H */
