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
 


#include "ut_unixTimer.h"
#include "ut_assert.h"

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback,
									   void* pData)
{
	UT_ASSERT(pCallback);

	// TODO: turn this back on to make timers work
//	UT_Timer * p = new UT_UNIXTimer();
	UT_Timer * p = NULL;

	if (p)
	{
		p->setCallback(pCallback);
		p->setInstanceData(pData);
	}

	return p;
}

/*****************************************************************/
	
UT_sint32 UT_UNIXTimer::set(UT_uint32 /*iMilliseconds*/)
{
	/*
	  TODO

	  The goal here is to set this timer to go off after iMilliseconds
	  have passed.  This method should not block.  It should call some
	  OS routine which provides timing facilities.  It is assumed that this
	  routine requires a C callback.  That callback, when it is called,
	  must look up the UT_Timer object which corresponds to it, and
	  call its fire() method.  See ut_Win32Timer.cpp for an example
	  of how it's done on Windows.  We're hoping that something similar will work
	  for other platforms.
	*/

	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return -1;
}

