/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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

#ifndef EV_UNIXMENUBAR_H
#define EV_UNIXMENUBAR_H

#include "ev_UnixMenu.h"

class AV_View;
class XAP_UnixApp;
class XAP_UnixFrame;


/*****************************************************************/

class EV_UnixMenuBar : public EV_UnixMenu
{
public:
	EV_UnixMenuBar(XAP_UnixApp * pUnixApp,
		       XAP_Frame * pFrame,
		       const char * szMenuLayoutName,
		       const char * szMenuLabelSetName);
	virtual ~EV_UnixMenuBar();

	virtual bool		synthesizeMenuBar();
	virtual bool		rebuildMenuBar();
	virtual bool		refreshMenu(AV_View * pView);
    virtual void        destroy(void);

    GtkWidget *         getMenuBar(void) const {return m_wMenuBar;}

protected:
	GtkWidget *		m_wMenuBar;
};

#endif /* EV_UNIXMENUBAR_H */
