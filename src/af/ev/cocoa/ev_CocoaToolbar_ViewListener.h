/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef EV_COCOATOOLBAR_VIEWLISTENER_H
#define EV_COCOATOOLBAR_VIEWLISTENER_H

#include "xav_Listener.h"
class EV_CocoaToolbar;
class AV_View;


class EV_CocoaToolbar_ViewListener : public AV_Listener
{
public:
	EV_CocoaToolbar_ViewListener(EV_CocoaToolbar * pCocoaToolbar,
								AV_View * pView);

	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);
    virtual AV_ListenerType getType(void) { return AV_LISTENER_TOOLBAR;}

protected:
	EV_CocoaToolbar *	m_pCocoaToolbar;
	AV_View *			m_pView;
};

#endif /* EV_COCOATOOLBAR_VIEWLISTENER_H */
