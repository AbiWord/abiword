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

#import <AppKit/AppKit.h>

#import "xap_CocoaTimer.h"

#include "ut_assert.h"

/*
@interface XAP_TimerInvocation: NSInvocation {
	int (*proc)(void *);
	void * param;
}

@end
*/


UT_uint32 XAP_newCocoaTimer (UT_uint32 time, int (*proc)(void *), void *p)
{
	NSTimeInterval dTime;
	if (time < 1000) {
		dTime = 1.0;
	}
	else {
		dTime = time / 1000;
	}
	UT_ASSERT(UT_NOT_IMPLEMENTED);
/*	
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:dTime
		invocation:[XAP_TimerInvocation createWithProc:proc param:p] 
		repeats:TRUE];

	[timer retain];
*/
}



void XAP_stopCocoaTimer (UT_uint32 timer)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
/*
	NSTimer *pTimer = (NSTimer *)timer;
	
	[pTimer invalidate];
	[timer release];
*/
}


