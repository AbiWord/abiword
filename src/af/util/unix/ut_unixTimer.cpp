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
 

#include <gtk/gtk.h>

#include "ut_unixTimer.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData, GR_Graphics * /*pG*/)
{
	UT_ASSERT(pCallback);
	UT_UNIXTimer * p = new UT_UNIXTimer(pCallback, pData);

	return p;
}

UT_UNIXTimer::UT_UNIXTimer(UT_TimerCallback pCallback, void* pData)
	: m_iMilliseconds(0), m_iGtkTimerId(0)
{
	setCallback(pCallback);
	setInstanceData(pData);
}

UT_UNIXTimer::~UT_UNIXTimer()
{
	UT_DEBUGMSG(("ut_unixTimer.cpp:  timer destructor\n"));
	stop();
}

/*****************************************************************/

static int _Timer_Proc(void *p)
{
	UT_UNIXTimer* pTimer = (UT_UNIXTimer*) p;
	UT_ASSERT(pTimer);

//	UT_DEBUGMSG(("ut_unixTimer.cpp:  timer fired\n"));
	
	pTimer->fire();

	return TRUE;
}

UT_sint32 UT_UNIXTimer::set(UT_uint32 iMilliseconds)
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
	stop();

	m_iGtkTimerId = gtk_timeout_add(iMilliseconds, _Timer_Proc, this);

	if (getIdentifier() == 0)
		setIdentifier(m_iGtkTimerId);

	UT_DEBUGMSG(("ut_unixTimer.cpp: timer [%d] (with id [%d] set\n", getIdentifier(), m_iGtkTimerId));

	m_iMilliseconds = iMilliseconds;

	return 0;
}

void UT_UNIXTimer::stop()
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	if (m_iGtkTimerId != 0)
	{
//		UT_DEBUGMSG(("ut_unixTimer.cpp: timer [%d] (with id [%d]) stopped\n", getIdentifier(), m_iGtkTimerId));
		gtk_timeout_remove(m_iGtkTimerId);
		m_iGtkTimerId = 0;
	}
}

void UT_UNIXTimer::start()
{
	// resume the delivery of events.
	UT_ASSERT(m_iMilliseconds > 0);
	set(m_iMilliseconds);
}

