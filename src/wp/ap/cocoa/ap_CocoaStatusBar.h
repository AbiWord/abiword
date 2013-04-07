/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 - 2003 Hubert Figuiere
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

#ifndef AP_COCOASTATUSBAR_H
#define AP_COCOASTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ap_StatusBar.h"

class XAP_Frame;
class GR_CocoaGraphics;
class AP_CocoaStatusBar;

//////////////////////////////////////////////////////////////////

@interface XAP_CocoaNSStatusBar : NSView
{
	AP_CocoaStatusBar*	_xap;
}
- (void)setXAPOwner:(AP_CocoaStatusBar*)owner;
- (void)statusBarDidResize:(NSNotification *)notification;

@end

//////////////////////////////////////////////////////////////////

class AP_CocoaStatusBar : public AP_StatusBar
{
public:
	AP_CocoaStatusBar(XAP_Frame * pFrame);
	virtual ~AP_CocoaStatusBar(void);

	virtual void		setView(AV_View * pView);
	XAP_CocoaNSStatusBar*	createWidget(void);

	virtual void		show(void);
	virtual void		hide(void);

	void _repositionFields(NSArray *fields);

private:

	XAP_CocoaNSStatusBar *	m_wStatusBar;
	NSView *			m_superView;
	bool				m_hidden;
	float				m_requestedWidth;
	int					m_numMaxWidth;
};

#endif /* AP_COCOASTATUSBAR_H */
