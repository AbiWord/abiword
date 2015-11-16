/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2004 Hubert Figuiere
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
#include "gr_CocoaCairoGraphics.h"
#include "gr_Painter.h"



@implementation XAP_CocoaNSView
- (id)initWith:(XAP_Frame *)frame andFrame:(NSRect)windowFrame
{
	UT_DEBUGMSG (("Cocoa: @XAP_CocoaNSView initWith:Frame\n"));
	UT_ASSERT (frame);
	if(![super initWithFrame:windowFrame]) {
		return nil;
	}
	m_pFrame = frame;
	m_pGR = NULL;
	if(frame) {
		[[NSNotificationCenter defaultCenter] addObserver:self
						selector:@selector(hasBeenResized:)
						name:NSViewFrameDidChangeNotification object:self];
	}
	return self;
}

- (void)dealloc
{
	[_eventDelegate release];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
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

- (XAP_Frame *)xapFrame
{
	return m_pFrame;
}

- (void)setGraphics:(GR_CocoaCairoGraphics *)gr
{
	m_pGR = gr;
}

/*!
	Cocoa overridden method. Redraw the screen.
 */
- (void)drawRect:(NSRect)aRect
{
	if (m_pGR) {
		UT_RGBColor clr;
		GR_Painter painter(m_pGR);

		GR_CocoaCairoGraphics::_utNSColorToRGBColor([NSColor redColor], clr);
		m_pGR->fillRect (clr, aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height);

		/*  Because of the way we convert from local to display units for scrolling and back again for expose
		 *  events, there can be a sizeable discrepancy (in local units) between internal events and the
		 *  reported exposed area. I am therefore marking an extra 1-pixel border around the exposed display area.
		 */
		aRect.origin.x    += -1.0;
		aRect.origin.y    += -1.0;
		aRect.size.width  +=  2.0;
		aRect.size.height +=  2.0;

		// if (![self inLiveResize]) // this case handled in -hasBeenResized: below
			m_pGR->_callUpdateCallback(&aRect);
	}
}

/*!
	Cocoa overridden method.

	\return NO. Coordinates are still upside down, but we'll reverse
	the offscreen instead.
*/
- (BOOL)isFlipped
{
	return GR_CocoaCairoGraphics::_isFlipped();
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

- (void)resetCursorRects
{
	[self addCursorRect:[self visibleRect] cursor:_cursor];
	//[_cursor setOnMouseEntered:YES];
}

- (void)setCursor:(NSCursor*)cursor
{
	[_cursor release];
	_cursor = [cursor retain];
}

- (void)setEventDelegate:(NSObject <XAP_MouseEventDelegate>*)delegate
{
	[_eventDelegate release];
	_eventDelegate = delegate;
	[delegate retain];
}

- (void)hasBeenResized:(NSNotification*)notif
{
	UT_UNUSED(notif);
	if (m_pGR && m_pFrame) {
		AV_View * pView = m_pFrame->getCurrentView();
		NSRect rect = [self bounds];
		if (pView && !pView->isLayoutFilling())
		{
			pView->setWindowSize((UT_sint32)rint(rect.size.width), (UT_sint32)rint(rect.size.height));
			// m_pGR->_callUpdateCallback(&rect);
			m_pFrame->quickZoom(); // was update zoom
		}
	}
}



- (NSObject <XAP_MouseEventDelegate>*)eventDelegate
{
	return _eventDelegate;
}


- (void)mouseDown:(NSEvent *)theEvent
{
	if (NSInputManager * inputManager = [NSInputManager currentInputManager])
		[inputManager markedTextAbandoned:[theEvent window]];
	[_eventDelegate mouseDown:theEvent from:self];
}
- (void)rightMouseDown:(NSEvent *)theEvent
{
	if (NSInputManager * inputManager = [NSInputManager currentInputManager])
		[inputManager markedTextAbandoned:[theEvent window]];
	[_eventDelegate mouseDown:theEvent from:self];
}
- (void)otherMouseDown:(NSEvent *)theEvent
{
	if (NSInputManager * inputManager = [NSInputManager currentInputManager])
		[inputManager markedTextAbandoned:[theEvent window]];
	[_eventDelegate mouseDown:theEvent from:self];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[_eventDelegate mouseDragged:theEvent from:self];
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
	[_eventDelegate mouseDragged:theEvent from:self];
}
- (void)otherMouseDragged:(NSEvent *)theEvent
{
	[_eventDelegate mouseDragged:theEvent from:self];
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[_eventDelegate mouseUp:theEvent from:self];
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
	[_eventDelegate mouseUp:theEvent from:self];
}
- (void)otherMouseUp:(NSEvent *)theEvent
{
	[_eventDelegate mouseUp:theEvent from:self];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	if(!m_pFrame) {
		return;
	}
	AV_View *pAV_View = m_pFrame->getCurrentView ();

	UT_sint32 deltaX = (UT_sint32)rint([theEvent deltaX]);
	UT_sint32 deltaY = (UT_sint32)rint([theEvent deltaY]);

	if (deltaX < 0)
		pAV_View->cmdScroll(AV_SCROLLCMD_LINERIGHT, m_pGR->tlu(-30*deltaX));
	if (deltaX > 0)
		pAV_View->cmdScroll(AV_SCROLLCMD_LINELEFT,  m_pGR->tlu( 30*deltaX));

	if (deltaY > 0)
		pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP,    m_pGR->tlu( 30*deltaY));
	if (deltaY < 0)
		pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN,  m_pGR->tlu(-30*deltaY));
}

@end
