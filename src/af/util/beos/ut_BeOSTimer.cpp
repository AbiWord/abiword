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
 TF Note: (early 1999)
  I'm not sure that this class actually behaves the
  way that it is intended.  I'm trying to reduce the
  amount of extra calls, and that seems to happen
  when I kill the thread in the stop() call, which
  seems right since the start() call will actually
  create a new thread.
*/
/* 
 DB Note (written way after TF Note): (03/11/1999)
 This class now should behave the way it's intended.
 I modified the thing to start a new timer only if we 
 change the timeout value using set, or we haven't 
 started one yet. Otherwise, we can just suspend/resume. 
 */
/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData, GR_Graphics * /*pG*/)
{
	UT_ASSERT(pCallback);
	UT_BeOSTimer * p = new UT_BeOSTimer(pCallback, pData);
	return p;
}

UT_BeOSTimer::UT_BeOSTimer(UT_TimerCallback pCallback, void* pData)
{
	setCallback(pCallback);
	setInstanceData(pData);
	setIdentifier(-1);  
	m_bStarted = UT_FALSE;
	m_iMilliseconds = 0;
	m_bMustRestart=UT_FALSE;
}

UT_BeOSTimer::~UT_BeOSTimer()
{
	UT_DEBUGMSG(("ut_BeOSTimer.cpp:  timer destructor\n"));
	if (getIdentifier()!=-1)
	{
		UT_DEBUGMSG(("Killing off thread\n"));
		kill_thread(getIdentifier());
	}
	else
	{
		UT_DEBUGMSG(("No thread to kill off, me thinks\n"));
	}
}

/*****************************************************************/

static int32 _Timer_Proc(void *p)
{
	UT_BeOSTimer* pTimer = (UT_BeOSTimer*) p;
	UT_ASSERT(pTimer);
/* 
 *
 */
restartlabel:
	while(UT_TRUE)
	{

	/*
	 Sleep for the desired amount of time (micro seconds) 
	 then fire off the event.
	*/
	if (pTimer->m_bMustRestart)
	{
		pTimer->m_bMustRestart=UT_FALSE;
		goto restartlabel;
	}
	snooze(pTimer->m_iMilliseconds * 1000);
	if (pTimer->m_bMustRestart)
	{
		pTimer->m_bMustRestart=UT_FALSE;
		goto restartlabel;
	}
	UT_DEBUGMSG(("ut_BeOSTimer.cpp:  timer fire\n"));
	pTimer->fire();


	/*
	  We need to manually reset the timer here.  This cross-platform
	  timer was designed to emulate the semantics of Win32 timers,
	  which continually fire until they are killed.
	*/
	}
}

void UT_BeOSTimer::resetIfStarted(void)
{
	/*if (m_bStarted)
		set(m_iMilliseconds);*/
	UT_DEBUGMSG(("resetIfStarted called\n"));
}

UT_sint32 UT_BeOSTimer::set(UT_uint32 iMilliseconds)
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
	if (m_bStarted) 
	{
		UT_DEBUGMSG(("Killing off old timer, starting new\n"));
		kill_thread(getIdentifier());
	}
	UT_DEBUGMSG(("Timer set\n"));
	m_iMilliseconds = iMilliseconds;
	m_bStarted = UT_TRUE;
	thread_id idTimer = spawn_thread(_Timer_Proc, "Timer", 
									 B_NORMAL_PRIORITY, this);
	setIdentifier(idTimer);
	resume_thread(idTimer);
	return 0;
}

void UT_BeOSTimer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	thread_id id;
	if ((id = getIdentifier()) !=-1 ) 
	{
		UT_DEBUGMSG(("ut_BeOSTimer.cpp: timer stopped\n"));
		m_bStarted=UT_FALSE;
		m_bMustRestart=UT_TRUE;
		suspend_thread(id); 
	}
	else {
		UT_DEBUGMSG(("Stopping non-existant timer \n"));
	}
}

void UT_BeOSTimer::start(void)
{
	// resume the delivery of events.
	UT_ASSERT(m_iMilliseconds > 0);
	if (getIdentifier()==-1)
	{
		UT_DEBUGMSG(("Creating new timer in start()\n"));
		set(m_iMilliseconds);
	}
	else if (!m_bStarted)
	{
		UT_DEBUGMSG(("Restarted old timer\n"));
		m_bStarted=UT_TRUE;
		resume_thread(getIdentifier());
	}
}

