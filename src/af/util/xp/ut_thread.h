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
#ifndef UT_THREAD_H
#define UT_THREAD_H

#include "ut_types.h"

class UT_ThreadImpl;

class ABI_EXPORT UT_Thread
{
  friend class UT_ThreadImpl;

 public:

  typedef enum {
    PRI_LOW,
    PRI_NORMAL,
    PRI_HIGH
  } Priority;

  virtual ~UT_Thread ();

  /*!
   * Starts a new thread and executes the code in the
   * (overridden) run method
   */
  void start () ;

  /*!
   * Get this thread's priority
   */
  UT_Thread::Priority getPriority () const
    {
      return mPri;
    }

  /*!
   * Sets this thread's priority
   */
  void setPriority ( UT_Thread::Priority pri ) ;

  /*!
   * Causes the currently-executing thread to wait until
   * this thread terminates
   */
  void join () ;

  /*!
   * Causes the current running thread to temporarily pause
   * and let other threads execute
   */
  static void yield () ;
  
 protected:
  // qualifying Priority with UT_Thread:: confuses VC
  UT_Thread (Priority pri = PRI_NORMAL) ;

  /*!
   * Pretty pretty please override me!!
   */
  virtual void run () = 0;

 private:
  UT_ThreadImpl * mPimpl;
  UT_Thread::Priority mPri;
  bool mbStarted;
};

#endif /* UT_THREAD_H */
