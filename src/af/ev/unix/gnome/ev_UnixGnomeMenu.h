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

#ifndef EV_UNIXGNOMEMENU_H
#define EV_UNIXGNOMEMENU_H

#include "ev_UnixMenu.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Menu.h"

class XAP_UnixGnomeApp;
class XAP_UnixGnomeFrame;

/*****************************************************************/

class EV_UnixGnomeMenuBar : public EV_UnixMenuBar
{
public:
	EV_UnixGnomeMenuBar(XAP_UnixApp * pUnixApp,
						XAP_UnixFrame * pUnixFrame,
						const char * szMenuLayoutName,
						const char * szMenuLabelSetName);
	~EV_UnixGnomeMenuBar(void);

	UT_Bool				synthesizeMenuBar(void);
};

#endif /* EV_UNIXGNOMEMENU_H */
