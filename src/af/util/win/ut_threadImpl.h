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

#include "ut_assert.h"
#include "ut_thread.h"
#include <process.h>

// for friendly assert message
#define PRIORITIES_NOT_SUPPORTED 0


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
        _endthread();
    }

  /*!
   * Starts a new thread and executes the code in the
   * (overridden) run method
   */
  void start ()
    {
      UT_Thread::Priority pri = mOwner->getPriority () ;

      // the priority is ignored

	if (mThread = _beginthread(start_routine, 0, (void *)this) == -1)
	{
	  //printf ( "thread create failed!!\n" ) ;
	  //MessageBox( NULL, "thread create failed!!\n", NULL, MB_OK );
	  UT_ASSERT(mTrhead != -1);
	}
    }

  /*!
   * Sets this thread's priority
   */
  void setPriority ( UT_Thread::Priority pri )
    {
      UT_ASSERT(PRIORITIES_NOT_SUPPORTED);
    }

  /*!
   * Causes the current running thread to temporarily pause
   * and let other threads execute
   */
  static void yield ()
    {
	// yield? 
    }

  void join ()
  {
	UT_ASSERT(UT_NOT_IMPLEMENTED);
  }

 private:

  static void start_routine ( void * inPtr )
    {
      UT_Thread * thisPtr = static_cast<UT_ThreadImpl *>(inPtr)->mOwner;
      // printf ( "In the start routine: %d\n", thisPtr == NULL );
      thisPtr->run () ;
    }

  UT_Thread         * mOwner;
  unsigned long     mThread;
};

#endif
