/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
/*
	Defines the Control for the toolbar. Can be instancied only ONCE.
 */

#ifndef __XAP_MACTOOLBAR_CONTROL_H__
#define __XAP_MACTOOLBAR_CONTROL_H__

#include <Carbon/Carbon.h>


class EV_MacToolbar;
class XAP_MacApp;

class XAP_MacToolbar_Control
{
public:
	XAP_MacToolbar_Control ();
	~XAP_MacToolbar_Control ();

	void show ();
	
	void setToolbar (XAP_MacFrame * frame);
	const WindowRef	getWindow () 
	                { return m_window; };
	void	requestToolbarRect (Rect & r) const;
static	UInt16	getButtonWidth ()
					{ return 30; };
static	UInt16	getButtonHeight ()
					{ return 30; };
static	UInt16	getButtonSpace () 
					{ return 4; };
private:
	static int		m_instanceCount;
	WindowRef		m_window;
	XAP_MacFrame*	m_pMacFrame;
static pascal OSStatus HandleToolbarMenus (EventHandlerCallRef nextHandler, 
											EventRef theEvent, void* userData);

//	EV_MacToolbar*	m_toolbars;
};


#endif
