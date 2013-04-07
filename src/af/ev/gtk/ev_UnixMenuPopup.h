/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef EV_UNIXMENUPOPUP_H
#define EV_UNIXMENUPOPUP_H

#include "ev_UnixMenu.h"

class AV_View;
class XAP_UnixApp;
class XAP_UnixFrame;

/*****************************************************************/

class EV_UnixMenuPopup : public EV_UnixMenu
{
public:
	EV_UnixMenuPopup(XAP_UnixApp * pUnixApp,
			 XAP_Frame * pFrame,
			 const char * szMenuLayoutName,
			 const char * szMenuLabelSetName);
	virtual ~EV_UnixMenuPopup(void);

	virtual bool		synthesizeMenuPopup(void);
	virtual bool		refreshMenu(AV_View * pView);
	virtual GtkWidget *	getMenuHandle(void) const;


protected:
	GtkWidget *			m_wMenuPopup;
private:
	UT_Vector m_vecCallbacks;
};

#endif /* EV_UNIXMENUPOPUP_H */
