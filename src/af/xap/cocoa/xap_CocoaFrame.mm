/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_files.h"
#include "ut_sleep.h"
#include "xap_ViewListener.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaTimer.h"
#include "ev_CocoaKeyboard.h"
#include "ev_CocoaMouse.h"
#include "ev_CocoaMenuBar.h"
#include "ev_CocoaMenuPopup.h"
#include "ev_CocoaToolbar.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "fv_View.h"
#include "xad_Document.h"
#include "gr_Graphics.h"



@implementation XAP_CocoaNSView
- (id)initWith:(XAP_Frame *)frame andFrame:(NSRect)windowFrame
{
	UT_DEBUGMSG (("Cocoa: @XAP_CocoaNSView initWith:Frame\n"));
	UT_ASSERT (frame);
	m_pFrame = frame;
	m_pGR = NULL;
	self = [super initWithFrame:windowFrame];
	return self;
}

- (void)dealloc
{
	[_eventDelegate release];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	if (m_pFrame) {
		UT_ASSERT (m_pFrame);
		
		if (m_pFrame->getCurrentView()) {
			m_pFrame->getCurrentView()->focusChange(AV_FOCUS_HERE);
		}
		UT_DEBUGMSG(("became first responder!\n"));
		return YES;
	}
	UT_DEBUGMSG (("refused to become first responder, because gr_Graphics is not setup.\n"));
	return NO;
}

- (void)setXAPFrame:(XAP_Frame *)frame
{
	m_pFrame = frame;
}

- (XAP_Frame *)xapFrame
{
	return m_pFrame;
}

- (void)setGraphics:(GR_CocoaGraphics *)gr
{
	m_pGR = gr;
}

/*!
	Cocoa overridden method. Redraw the screen.
 */
- (void)drawRect:(NSRect)aRect
{
	if (m_pGR) {
		m_pGR->_updateRect(self, aRect);
	}
}

/*!
	Cocoa overridden method.

	\return NO. Coordinates are still upside down, but we'll reverse
	the offscreen instead.
*/
- (BOOL)isFlipped
{
	return GR_CocoaGraphics::_isFlipped();
}

/*!
	Cocoa overridden method.

	\return NO. Not opaque.
 */
- (BOOL)isOpaque
{
	return YES;
}

/*
- (XAP_Frame *)_getOwnerFrame
{
	id controller = [[self window] delegate];
	if ([controller isKindOfClass:[XAP_CocoaFrameController class]])
	{
		XAP_Frame* frame = [(XAP_CocoaFrameController*)controller frameImpl]->getFrame();
		return frame;
	}
	NSLog (@"-[_getOwnerFrame] could find owner frame");
	return NULL;
}
*/

- (void)setEventDelegate:(NSObject <XAP_MouseEventDelegate>*)delegate
{
	[_eventDelegate release];
	[delegate retain];
	_eventDelegate = delegate;
}

- (NSObject <XAP_MouseEventDelegate>*)eventDelegate
{
	return _eventDelegate;
}


- (void)mouseDown:(NSEvent *)theEvent
{
	[_eventDelegate mouseDown:theEvent from:self];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[_eventDelegate mouseDragged:theEvent from:self];
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[_eventDelegate mouseUp:theEvent from:self];
}

@end
