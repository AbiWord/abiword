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
 

#include "ut_BeOSTimer.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include <stdio.h>
#include <OS.h>

/*
 TF Note:
  I'm not sure that this class actually behaves the
  way that it is intended.  I'm trying to reduce the
  amount of extra calls, and that seems to happen
  when I kill the thread in the stop() call, which
  seems right since the start() call will actually
  create a new thread.
*/

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData, GR_Graphics * /*pG*/)
{
	UT_ASSERT(pCallback);
	UT_BEOSTimer * p = new UT_BEOSTimer(pCallback, pData);
	return p;
}

UT_BEOSTimer::UT_BEOSTimer(UT_TimerCallback pCallback, void* pData)
{
	setCallback(pCallback);
	setInstanceData(pData);
	setIdentifier(-1);  
	m_bStarted = UT_FALSE;
	m_iMilliseconds = 0;
}

UT_BEOSTimer::~UT_BEOSTimer()
{
	UT_DEBUGMSG(("ut_BeOSTimer.cpp:  timer destructor\n"));
	stop();
}

/*****************************************************************/

static int32 _Timer_Proc(void *p)
{
	UT_BEOSTimer* pTimer = (UT_BEOSTimer*) p;
	UT_ASSERT(pTimer);

	/*
	 Sleep for the desired amount of time (micro seconds) 
	 then fire off the event.
	*/
	snooze(pTimer->m_iMilliseconds * 1000);
	UT_DEBUGMSG(("ut_BeOSTimer.cpp:  timer fire\n"));
	pTimer->fire();

	/*
	  We need to manually reset the timer here.  This cross-platform
	  timer was designed to emulate the semantics of Win32 timers,
	  which continually fire until they are killed.
	*/
	pTimer->resetIfStarted();

	/* 
	 This seems like a waste, we should just stay cycling in a 
	 loop within this thread waiting to either be killed or
 	 stopped, rather than spawn off a new thread all the time.
	 Use this temporarily until we determine a new course of action
 	*/
	return 0;
}

void UT_BEOSTimer::resetIfStarted(void)
{
	if (m_bStarted)
		set(m_iMilliseconds);
}

UT_sint32 UT_BEOSTimer::set(UT_uint32 iMilliseconds)
{
	/*
	  The goal here is to set this timer to go off after iMilliseconds
	  have passed.  This method should not block.  It should call some
	  OS routine which provides timing facilities.  It is assumed that this
	  routine requires a C callback.  That callback, when it is called,
	  must look up the UT_Timer object which corresponds to it, and
	  call its fire() method.  See ut_Win32Timer.cpp for an example
	  of how it's done on Windows.  We're hoping that something similar 
	  will work for other platforms.
	*/
	UT_DEBUGMSG(("ut_BeOSTimer.cpp: timer set %d ms\n", iMilliseconds));
	m_iMilliseconds = iMilliseconds;
	m_bStarted = UT_TRUE;
	//UT_sint32 idTimer = gtk_timeout_add(iMilliseconds, _Timer_Proc, this);
	thread_id idTimer = spawn_thread(_Timer_Proc, "Timer", 
					 B_NORMAL_PRIORITY, this);
	setIdentifier(idTimer);
	resume_thread(idTimer);
	return 0;
}

void UT_BEOSTimer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	//Should I just kill that thread here?
	m_bStarted = UT_FALSE;
	thread_id id;
	if ((id = getIdentifier()) == -1) {
		UT_DEBUGMSG(("ut_BeOSTimer.cpp: timer stopped\n"));
		kill_thread(id); 
	}
}

void UT_BEOSTimer::start(void)
{
	// resume the delivery of events.
	UT_ASSERT(m_iMilliseconds > 0);
	if (!m_bStarted)
		set(m_iMilliseconds);
}

