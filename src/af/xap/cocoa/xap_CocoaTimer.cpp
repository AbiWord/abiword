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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#import <Cocoa/Cocoa.h>

#import "xap_CocoaTimer.h"

#include "ut_assert.h"
#include "ut_mutex.h"
#include "ut_debugmsg.h"

#include "ut_unixTimer.h"

id UT_UNIXTimer::s_timerMutex;
id UT_UNIXTimer::s_timerIds;
int UT_UNIXTimer::s_lastTimerId = 1;

@interface XAP_TimerInvocation: NSInvocation {
	int (*proc)(void *);
	void * param;
}
+(XAP_TimerInvocation *)createWithProc:(int (*)(void *))pr param:(void*)p;
-(void)invoke;
-(void)retainArguments;
@end

@implementation XAP_TimerInvocation
+(XAP_TimerInvocation *)createWithProc:(int (*)(void *))pr param:(void*)p
{
  XAP_TimerInvocation * obj = [XAP_TimerInvocation alloc];
  obj->proc = pr;
  obj->param = p; 
  return [obj autorelease];
}

-(void)invoke
{
  proc(param);
}

-(void)retainArguments
{
  // don't need.
}
@end

/*!
	Check that the NSLock exist, and eventually lock it.
 */
void _checkLock(void)
{
	if (!UT_UNIXTimer::s_timerMutex) {
		UT_UNIXTimer::s_timerMutex = [[NSLock alloc] init];
	}
}


UT_uint32 XAP_newCocoaTimer (UT_uint32 time, int (*proc)(void *), void *p)
{
	NSTimeInterval dTime = (double)time / 1000.;

	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:dTime
		invocation:[XAP_TimerInvocation createWithProc:proc param:p] 
		repeats:TRUE];

	int tid = -1;
	{
		_checkLock();
		[UT_UNIXTimer::s_timerMutex lock];
		tid = UT_UNIXTimer::s_lastTimerId;
		if (UT_UNIXTimer::s_timerIds == nil) {
			UT_UNIXTimer::s_timerIds = [[NSMutableDictionary alloc] init];
		}
		[UT_UNIXTimer::s_timerIds setObject:timer forKey:[NSNumber numberWithInt:UT_UNIXTimer::s_lastTimerId++]];
		[UT_UNIXTimer::s_timerMutex unlock];
	}

	return tid;
}



void XAP_stopCocoaTimer (UT_uint32 timerId)
{
	NSTimer *pTimer;
	{
		_checkLock();
		[UT_UNIXTimer::s_timerMutex lock];
		NSNumber* key = [NSNumber numberWithInt:timerId];
		pTimer = [[UT_UNIXTimer::s_timerIds objectForKey:key] retain];
		[UT_UNIXTimer::s_timerIds removeObjectForKey:key];
		[UT_UNIXTimer::s_timerMutex unlock];
	}
	
	[pTimer invalidate];
	[pTimer release];
}


