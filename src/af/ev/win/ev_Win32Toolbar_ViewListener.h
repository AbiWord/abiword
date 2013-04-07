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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef EV_WIN32TOOLBAR_VIEWLISTENER_H
#define EV_WIN32TOOLBAR_VIEWLISTENER_H

#include "xav_Listener.h"
class EV_Win32Toolbar;
class AV_View;


class ABI_EXPORT EV_Win32Toolbar_ViewListener : public AV_Listener
{
public:
	EV_Win32Toolbar_ViewListener(EV_Win32Toolbar * pWin32Toolbar,
								AV_View * pView);

	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);
    virtual AV_ListenerType getType(void) { return AV_LISTENER_TOOLBAR;}

protected:
	EV_Win32Toolbar *	m_pWin32Toolbar;
	AV_View *			m_pView;
};

#endif /* EV_WIN32TOOLBAR_VIEWLISTENER_H */
