/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#include <glib.h>

#include "ut_thread.h"
#include "ut_assert.h"
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
	}
	
	/*!
	 * Starts a new thread and executes the code in the
	 * (overridden) run method
	 */
	void start ()
    {
		 if (!g_thread_supported ()) g_thread_init (NULL);

		 UT_Thread::Priority pri = mOwner->getPriority () ;
		 
		 GError * err = NULL ;
		
		 // TODO: use the priority
			
		 if ( (mThread = g_thread_create ( start_routine, this, TRUE, &err ) ) == NULL )
		 {
			  UT_DEBUGMSG(( "Thread create failed: %s!!\n", err->message ));
			  g_error_free ( err ) ;
		 }
	}
	
	/*!
	 * Sets this thread's priority
	 */
	void setPriority ( UT_Thread::Priority pri )
	{
		 GThreadPriority priority = G_THREAD_PRIORITY_NORMAL;

		 if ( pri == UT_Thread::PRI_LOW )
			  priority = G_THREAD_PRIORITY_LOW;
		 else if ( pri == UT_Thread::PRI_HIGH )
			  priority = G_THREAD_PRIORITY_HIGH; 
		 
		 if ( mThread != NULL )
			  g_thread_set_priority ( mThread, priority ) ;
	}

  /*!
   * Causes the current running thread to temporarily pause
   * and let other threads execute
   */
  static void yield ()
  {
	   g_thread_yield () ;
  }

  void join ()
  {
	   if ( mThread != NULL )
			g_thread_join ( mThread ) ;
  }

 private:

	static void * start_routine ( void * inPtr )
		{
			UT_Thread * thisPtr = static_cast<UT_ThreadImpl *>(inPtr)->mOwner;
			UT_DEBUGMSG(( "In the start routine: %d\n", thisPtr == NULL ));
			thisPtr->run () ;
			return NULL ;
		}

	 UT_Thread       * mOwner;
	 GThread         * mThread;
};

#endif
