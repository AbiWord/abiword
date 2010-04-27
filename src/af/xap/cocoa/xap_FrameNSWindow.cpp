/* AbiWord
 * Copyright (C) 2003 Hubert Figuiere
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

#import "xap_FrameNSWindow.h"

/*!
	We subclass just to intercept all keydown events and redirect them....
 */
@implementation XAP_FrameNSWindow

- (void)sendEvent:(NSEvent *)theEvent
{
	if ([theEvent type] == NSKeyDown) {
		[[self windowController] keyDown:theEvent];
		return;
	}
	[super sendEvent:theEvent];
}


@end
