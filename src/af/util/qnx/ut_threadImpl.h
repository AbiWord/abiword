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

#include "ut_thread.h"
#include "ut_debugmsg.h"
#include <pthread.h>
#include <sched.h>

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
	pthread_exit ( NULL ) ;
    }

  /*!
   * Starts a new thread and executes the code in the
   * (overridden) run method
   */
  void start ()
    {
			pthread_attr_t attr;
			struct sched_param param;

			pthread_attr_init(&attr);
			int prio=10;    
  UT_Thread::Priority pri = mOwner->getPriority () ;
		if(pri == UT_Thread::PRI_LOW)
			prio=8;
		else if( pri == UT_Thread::PRI_HIGH)
			prio=12;
			
			param.sched_priority=prio;
			pthread_attr_setschedparam(&attr,&param);

      if ( 0 != pthread_create ( &mThread,&attr, start_routine, this ) )
	{
	  UT_DEBUGMSG (( "thread create failed!!\n" )) ;
	}
    }
	/* Join thread */
	void join()
	{	
		if(mThread != 0)
			pthread_join(mThread,0);
	}

  /*!
   * Sets this thread's priority
   */
  void setPriority ( UT_Thread::Priority pri )
    {
			struct sched_param sched;
			sched.sched_priority=10;

			if(pri == UT_Thread::PRI_LOW)
				sched.sched_priority=8;
			else if(pri == UT_Thread::PRI_HIGH)
				sched.sched_priority=12;

		if(mThread != NULL)
    	pthread_setschedparam(mThread,SCHED_RR,&sched);
		}

  /*!
   * Causes the current running thread to temporarily pause
   * and let other threads execute
   */
  static void yield ()
    {
    	sched_yield();
		}

 private:

  static void * start_routine ( void * inPtr )
    {
      UT_Thread * thisPtr = static_cast<UT_ThreadImpl *>(inPtr)->mOwner;
      UT_DEBUGMSG (( "In the start routine: %d\n", thisPtr == NULL ));
      thisPtr->run () ;
      return NULL ;
    }

  UT_Thread         * mOwner;
  pthread_t         mThread;
};

#endif
