/* AbiSource Program Utilities
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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
#ifndef UT_THREADIMPL_H
#define UT_THREADIMPL_H

#include <OS.h>
#include "ut_assert.h"
#include "ut_thread.h"
#include "ut_debugmsg.h"

class ABI_EXPORT UT_ThreadImpl
{
	public:

	UT_ThreadImpl ( UT_Thread * owner )
	: mOwner ( owner ), mThread ( 0 )
	{
	}

	~UT_ThreadImpl ()
	{
		// only exit if started
		if ( mOwner->mbStarted )
			kill_thread( mThread );
	}

	/*!
	 * Starts a new thread and executes the code in the
	 * (overridden) run method
	 */
	void start ()
	{
		UT_Thread::Priority pri = mOwner->getPriority();
		int32 priority = B_NORMAL_PRIORITY;
		if(pri == UT_Thread::PRI_LOW)
			priority = B_LOW_PRIORITY;
		else if(pri == UT_Thread::PRI_HIGH)
			priority = B_DISPLAY_PRIORITY;
		
		mThread = spawn_thread(start_routine, "abi", priority, (void *)this);
		if (mThread != B_NO_MORE_THREADS && mThread != B_NO_MEMORY)
		{
			resume_thread(mThread);
		} else {
			UT_DEBUGMSG (( "thread create failed!!\n" )) ;
		}
	}
	
	/* Join thread */
	void join()
	{	
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	}

	/*!
	 * Sets this thread's priority
	 */
	void setPriority ( UT_Thread::Priority pri )
	{
		int32 priority = B_NORMAL_PRIORITY;
		if(pri == UT_Thread::PRI_LOW)
			priority = B_LOW_PRIORITY;
		else if(pri == UT_Thread::PRI_HIGH)
			priority = B_DISPLAY_PRIORITY;

		set_thread_priority(mThread, priority);
	}

	/*!
	 * Causes the current running thread to temporarily pause
	 * and let other threads execute
	 */
	static void yield ()
	{
		// yield? 
	}

	private:

	static int32 start_routine ( void * inPtr )
	{
		UT_Thread * thisPtr = static_cast<UT_ThreadImpl *>(inPtr)->mOwner;
		UT_DEBUGMSG (( "In the start routine: %d\n", thisPtr == NULL ));
		thisPtr->run();
		return B_OK;
	}

	UT_Thread * mOwner;
	thread_id mThread;
};

#endif
