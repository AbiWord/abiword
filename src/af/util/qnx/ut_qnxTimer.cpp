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
 

#include "ut_qnxTimer.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include <Pt.h>
#include <stdio.h>

#define ENABLE_TIMER 

/*****************************************************************/
extern PtWidget_t *gTimerWidget;
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData, GR_Graphics * /*pG*/)
{
	UT_ASSERT(pCallback);
	UT_QNXTimer * p = new UT_QNXTimer(pCallback, pData);

	return p;
}

UT_QNXTimer::UT_QNXTimer(UT_TimerCallback pCallback, void* pData)
{
	setCallback(pCallback);
	setInstanceData(pData);
	m_bStarted = UT_FALSE;
	m_iMilliseconds = 0;
}

UT_QNXTimer::~UT_QNXTimer()
{
	UT_DEBUGMSG(("TIMER:  timer destructor "));

	stop();
}

/*****************************************************************/
static int _Timer_Proc(PtWidget_t *w, void *p, PtCallbackInfo_t *info)
{
	UT_QNXTimer* pTimer = (UT_QNXTimer*) p;
	UT_ASSERT(pTimer);

	pTimer->fire();

	/*
	  We need to manually reset the timer here.  This cross-platform
	  timer was designed to emulate the semantics of Win32 timers,
	  which continually fire until they are killed.

	  pTimer->resetIfStarted();
	*/
	return 0;
}

static void * _Timer_Thread(void *p)
{
	UT_QNXTimer* pTimer = (UT_QNXTimer*) p;
	UT_ASSERT(pTimer);

	pTimer->fire();

	return NULL;
}


void UT_QNXTimer::resetIfStarted(void)
{
	if (m_bStarted)
		set(m_iMilliseconds);
}

UT_sint32 UT_QNXTimer::set(UT_uint32 iMilliseconds)
{
	/*
	  The goal here is to set this timer to go off after iMilliseconds
	  have passed.  This method should not block.  It should call some
	  OS routine which provides timing facilities.  It is assumed that this
	  routine requires a C callback.  That callback, when it is called,
	  must look up the UT_Timer object which corresponds to it, and
	  call its fire() method.  See ut_Win32Timer.cpp for an example
	  of how it's done on Windows.  We're hoping that something similar will work
	  for other platforms.
	*/


	setIdentifier(0);

#if defined(ENABLE_TIMER) 
	PtArg_t args[5];
	int     n = 0;
	PtWidget_t *idTimer;

	PtSetArg(&args[n++], Pt_ARG_TIMER_INITIAL, iMilliseconds, 0);
	PtSetArg(&args[n++], Pt_ARG_TIMER_REPEAT, iMilliseconds, 0);
	if (!gTimerWidget || !PtWidgetIsRealized(gTimerWidget)) {
		UT_DEBUGMSG(("TIMER: Can't access timer widget "));
		return 1;	
	}
	if (!(idTimer = PtCreateWidget(PtTimer, gTimerWidget, n, args))) {
		UT_DEBUGMSG(("TIMER: Can't create timer "));
		return 1;
	}
	PtAddCallback(idTimer, Pt_CB_TIMER_ACTIVATE, _Timer_Proc, this);
	//All bad things go away when I don't use this
	PtRealizeWidget(idTimer);
	setIdentifier((UT_sint32)idTimer);

	m_iMilliseconds = iMilliseconds;
	m_bStarted = UT_TRUE;
#endif

	return 0;
}

void UT_QNXTimer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.

#if 0
	if (m_bStarted)
		gtk_timeout_remove(getIdentifier());
#endif
	//PtArg_t arg;
	//PtSetArg(&arg, Pt_ARG_TIMER_INITIAL, 0, 0);
	//PtSetResources((PtWidget_t *)getIdentifier(), 1, &arg);
	//OR
#if defined(ENABLE_TIMER) 
	PtWidget_t *timer;
	if (!(timer = (PtWidget_t *)getIdentifier()) || !PtWidgetIsRealized(timer)) {
		return;
	}
	PtDestroyWidget(timer);
#endif
	setIdentifier(0);
	m_bStarted = UT_FALSE;

	//UT_DEBUGMSG(("ut_unixTimer.cpp: timer stopped\n"));
}

void UT_QNXTimer::start(void)
{
	// resume the delivery of events.

#if defined(ENABLE_TIMER)
	UT_ASSERT(m_iMilliseconds > 0);
#endif
	
	if (!m_bStarted)
		set(m_iMilliseconds);
}

