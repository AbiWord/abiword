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

#ifndef EV_MACTOOLBAR_VIEWLISTENER_H
#define EV_MACTOOLBAR_VIEWLISTENER_H

#include "xav_Listener.h"
class EV_MacToolbar;
class AV_View;


class EV_MacToolbar_ViewListener : public AV_Listener
{
public:
	EV_MacToolbar_ViewListener(EV_MacToolbar * pMacToolbar,
								AV_View * pView);
	
	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);

protected:
	EV_MacToolbar *	m_pMacToolbar;
	AV_View *			m_pView;
};

#endif /* EV_MACTOOLBAR_VIEWLISTENER_H */
