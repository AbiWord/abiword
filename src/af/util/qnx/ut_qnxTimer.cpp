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
	m_bStarted = false;
	m_iMilliseconds = 0;
}

UT_QNXTimer::~UT_QNXTimer()
{
	stop();
}

/*****************************************************************/
static int _Timer_Proc(PtWidget_t *w, void *p, PtCallbackInfo_t *info)
{
	UT_QNXTimer *pTimer;

	/*
     We need to look up the timer because the widget might have	
	 been deleted.  Note that this is not entirely foolproof since
	 a new timer might have been created which re-used the widget's
	 address and so we could pre-maturely fire someone elses timer.
	 It would be better if we could do the lookup based on a large
     increasing number, but alas that would mean that we couldn't
     store our widget pointer in the Identifier field (which might
     not be entirely a bad thing).
	*/
	pTimer = (UT_QNXTimer *)UT_Timer::findTimer((UT_uint32)w);
	if (!pTimer) {
		printf("*** Saved ourselves a timer segfault *** \n");
	} else {
		pTimer->fire();
	}

	return Pt_CONTINUE;
}

void UT_QNXTimer::resetIfStarted(void)
{
	if (m_bStarted)
		set(m_iMilliseconds);
}

/*
  The goal here is to set this timer to go off after iMilliseconds
 have passed.  This method should not block.  It should call some
 OS routine which provides timing facilities.  It is assumed that this
 routine requires a C callback.  That callback, when it is called,
 must look up the UT_Timer object which corresponds to it, and
 call its fire() method.  The lookup of the UT_Timer object should be
 done in a manner such that if the timer is deleted and then the event
 is fired it will not segfault by referencing the now invalid timer
 object.  This can be done by using the calls setIdentifier/getIdentifier
 and the static findTimer() calls.

 See ut_Win32Timer.cpp for an example of how it's done on Windows.  
 We're hoping that something similar will work for other platforms.
*/
UT_sint32 UT_QNXTimer::set(UT_uint32 iMilliseconds)
{
	PtArg_t 	args[5];
	PtWidget_t 	*idTimer;
	int     	n;

	if (!gTimerWidget || !PtWidgetIsRealized(gTimerWidget)) {
		UT_DEBUGMSG(("TIMER: Can't access global timer widget "));
		UT_ASSERT(0);
		return 1;	
	}

	//Clear the identifier while we are doing this processing
	setIdentifier(0);

	//Set up and create the timer
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TIMER_INITIAL, iMilliseconds, 0);
	PtSetArg(&args[n++], Pt_ARG_TIMER_REPEAT, iMilliseconds, 0);
	if (!(idTimer = PtCreateWidget(PtTimer, gTimerWidget, n, args))) {
		UT_DEBUGMSG(("TIMER: Can't create local timer "));
		UT_ASSERT(0);
		return 1;
	}

	//We get our reference to the timer object by querying the id of the widget
	PtAddCallback(idTimer, Pt_CB_TIMER_ACTIVATE, _Timer_Proc, NULL);
	setIdentifier((UT_sint32)idTimer);

	//The timer only starts when we realize the timer widget.
	PtRealizeWidget(idTimer);

	m_iMilliseconds = iMilliseconds;
	m_bStarted = true;

	return 0;
}

/*
 Stop the delivery of timer events and stop the 
 OS timer from firing, but do not delete the class.
*/
void UT_QNXTimer::stop(void)
{
	PtWidget_t *timer;

	timer = (PtWidget_t *)getIdentifier();
	setIdentifier(0);

	if (timer && PtWidgetIsRealized(timer)) {
		PtDestroyWidget(timer);
	}

	m_bStarted = false;
}

/*
 Resume the delivery of timer events but only if they
 have not already been started.
*/
void UT_QNXTimer::start(void)
{
	UT_ASSERT(m_iMilliseconds > 0);
	
	if (!m_bStarted)
		set(m_iMilliseconds);
}

