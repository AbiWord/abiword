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

#ifndef AP_QTTOOLBAR_STYLECOMBO_H
#define AP_QtTOOLBAR_STYLECOMBO_H

#include <map>
#include <string>

#include "xap_Types.h"
#include "ev_Toolbar_Control.h"
#include "pd_Document.h"
#include "ap_QtFrame.h"

class EV_Toolbar;
class PD_Style;

/*****************************************************************/

class AP_QtToolbar_StyleCombo : public EV_Toolbar_Control
{
public:
	AP_QtToolbar_StyleCombo(EV_Toolbar * pToolbar, XAP_Toolbar_Id id);
	virtual ~AP_QtToolbar_StyleCombo(void);

	virtual bool		populate(void);
	bool                repopulate(void);
	static EV_Toolbar_Control *		static_constructor(EV_Toolbar *, XAP_Toolbar_Id id);

protected:
    AP_QtFrame 								* m_pFrame;

private:

};

#endif /* AP_QTTOOLBAR_STYLECOMBO_H */

