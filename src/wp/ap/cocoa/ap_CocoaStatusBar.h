/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2002 Hubert Figuiere
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

#ifndef AP_COCOASTATUSBAR_H
#define AP_COCOASTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ap_StatusBar.h"

class XAP_Frame;
class GR_CocoaGraphics;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_CocoaStatusBar : public AP_StatusBar
{
public:
	AP_CocoaStatusBar(XAP_Frame * pFrame);
	virtual ~AP_CocoaStatusBar(void);

	virtual void		setView(AV_View * pView);
	Abi_NSView *			createWidget(void);

	virtual void		show(void);
	virtual void		hide(void);

	void _style_changed (void);

private:
	static bool _graphicsUpdateCB(NSRect * aRect, GR_CocoaGraphics *pGR, void *param);

	Abi_NSView *			m_wStatusBar;
	NSView *			m_superView;
};

#endif /* AP_COCOASTATUSBAR_H */
