/* AbiWord
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

#ifndef AP_BEOSTOOLBAR_SIZECOMBO_H
#define AP_BEOSTOOLBAR_SIZECOMBO_H

#include "xap_Types.h"
#include "ev_Toolbar_Control.h"
class EV_Toolbar;

/*****************************************************************/

class AP_BeOSToolbar_SizeCombo : public EV_Toolbar_Control
{
public:
	AP_BeOSToolbar_SizeCombo(EV_Toolbar * pToolbar, XAP_Toolbar_Id id);
	virtual ~AP_BeOSToolbar_SizeCombo(void);

	virtual bool		populate(void);

	static EV_Toolbar_Control *		static_constructor(EV_Toolbar *, XAP_Toolbar_Id id);

protected:
};

#endif /* AP_BEOSTOOLBAR_SIZECOMBO_H */
