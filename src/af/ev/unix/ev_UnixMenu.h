/* AbiSource Program Utilities
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

#ifndef EV_UNIXMENU_H
#define EV_UNIXMENU_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ev_Menu.h"
#include "ap_Menu_Id.h"
class FV_View;
class AP_UnixAp;
class AP_UnixFrame;

/*****************************************************************/

class EV_UnixMenu : public EV_Menu
{
public:
	EV_UnixMenu(AP_UnixAp * pUnixAp, AP_UnixFrame * pUnixFrame);
	~EV_UnixMenu(void);

	UT_Bool				synthesize(void);
	UT_Bool				menuEvent(AP_Menu_Id id);

protected:
	AP_UnixAp *			m_pUnixAp;
	AP_UnixFrame *		m_pUnixFrame;

	GtkWidget *			m_wMenuBar;
	UT_Vector			m_vecMenuWidgets;
};

#endif /* EV_UNIXMENU_H */
