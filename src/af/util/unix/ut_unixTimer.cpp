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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <limits>
#include <algorithm>

#ifndef TOOLKIT_COCOA
# include <glib.h>
#else
# include "ut_mutex.h"
# include "xap_CocoaTimer.h"
#endif

#include "ut_unixTimer.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

/*****************************************************************/

UT_Timer* UT_Timer::static_constructor(UT_WorkerCallback pCallback, void* pData)
{
	UT_ASSERT(pCallback);
	UT_UNIXTimer * p = new UT_UNIXTimer(pCallback, pData);

	return p;
}

UT_UNIXTimer::UT_UNIXTimer(UT_WorkerCallback pCallback, void* pData)
	: m_iMilliseconds(0), m_iGtkTimerId(0)
{
	setCallback(pCallback);
	setInstanceData(pData);
}

UT_UNIXTimer::~UT_UNIXTimer()
{
	xxx_UT_DEBUGMSG(("ut_unixTimer.cpp:  timer destructor\n"));
	stop();
}

/*****************************************************************/

static int _Timer_Proc(void *p)
{
	UT_UNIXTimer* pTimer = static_cast<UT_UNIXTimer*>(p);
	UT_ASSERT(pTimer);

	xxx_UT_DEBUGMSG(("ut_unixTimer.cpp:  timer fired\n"));
	if (pTimer) {
		pTimer->fire();
		return true;
	}
	return 0;
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

	iMilliseconds = std::min(iMilliseconds, static_cast<UT_uint32>(
					 std::numeric_limits<millisec_t>::max()
					 ));

#ifndef TOOLKIT_COCOA
	m_iGtkTimerId = g_timeout_add_full(0, iMilliseconds, _Timer_Proc, this, NULL);
#else
	m_iGtkTimerId = XAP_newCocoaTimer(iMilliseconds, _Timer_Proc, this);
#endif

	if (getIdentifier() == 0)
		setIdentifier(m_iGtkTimerId);

//	UT_DEBUGMSG(("ut_unixTimer.cpp: timer [%d] (with id [%d] set\n", getIdentifier(), m_iGtkTimerId));

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
#ifndef TOOLKIT_COCOA
		g_source_remove(m_iGtkTimerId);
#else
		XAP_stopCocoaTimer (m_iGtkTimerId);
#endif
		m_iGtkTimerId = 0;
	}
}

void UT_UNIXTimer::start()
{
	// resume the delivery of events.
	UT_ASSERT(m_iMilliseconds > 0);
	set(m_iMilliseconds);
}

