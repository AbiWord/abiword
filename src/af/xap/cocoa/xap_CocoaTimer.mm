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

#import <Cocoa/Cocoa.h>

#import "xap_CocoaTimer.h"

#include "ut_assert.h"
#include "ut_map.h"
#include "ut_mutex.h"
#include "ut_pair.h"
#include "ut_debugmsg.h"

#include "ut_unixTimer.h"

UT_Mutex UT_UNIXTimer::s_timerMutex;
UT_Map UT_UNIXTimer::s_timerIds;
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
  return obj;
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

UT_uint32 XAP_newCocoaTimer (UT_uint32 time, int (*proc)(void *), void *p)
{
	NSTimeInterval dTime;
	if (time < 1000) {
		dTime = 1.0;
	}
	else {
		dTime = time / 1000;
	}

	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:dTime
		invocation:[XAP_TimerInvocation createWithProc:proc param:p] 
		repeats:TRUE];

	int tid = -1;
	{
		UT_MutexAcquirer acq(UT_UNIXTimer::s_timerMutex);
		tid = UT_UNIXTimer::s_lastTimerId;
		UT_UNIXTimer::s_timerIds.insert((const void *)UT_UNIXTimer::s_lastTimerId++, 
				  (const void *)timer);
	}

	[timer retain];
	return tid;
}



void XAP_stopCocoaTimer (UT_uint32 timerId)
{
	NSTimer *pTimer;
	{
		UT_MutexAcquirer acq (UT_UNIXTimer::s_timerMutex);
		const void * p = (const void *)timerId;
		UT_Pair *pr = (UT_Pair*)UT_UNIXTimer::s_timerIds.find(p).value();
		pTimer = (NSTimer*)pr->second();
		UT_UNIXTimer::s_timerIds.erase(pr);
	}
	
	[pTimer invalidate];
	[*pTimer release];
}


