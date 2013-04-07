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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#ifndef UT_MUTEX_H
#define UT_MUTEX_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

class UT_MutexImpl;
class UT_MutexAcquirer;

/*!
 * Cross-platform mutex class, with brain-dead simple
 * lock and unlock semantics. No trylock() or any other
 * "wierdo" calls because they're fascist!
 *
 * UT_MutexImpl has the same signature as UT_Mutex except that it's
 * implemented in platform-specific code
 */
class ABI_EXPORT UT_Mutex
{
	friend class UT_MutexImpl;
	friend class UT_MutexAcquirer;

 public:
	UT_Mutex ();
	~UT_Mutex ();

  void lock ();
  void unlock ();

 private:


  // no impls
  UT_Mutex (const UT_Mutex & other);
  UT_Mutex & operator=(const UT_Mutex & other);

  UT_MutexImpl * m_pimpl;
};

/*!
 * Inline class whose job is just to acquire and un-aquire a mutex
 * lock, thus releasing the programmer from manually having to
 * remember to unlock a mutex before returning from a function,
 * or even from within a block of code. Example:
 *
 * void Clazz::method (int arg)
 * {
 *   UT_MutexAcquirer acquirer (m_ClazzMutex);
 *   // do stuff that modifies class data
 *   // ...
 *   // just return, no need to release mutex. it's
 *   // done transparently by the acquirer
 * }
 */
class ABI_EXPORT UT_MutexAcquirer
{
 public:

  UT_MutexAcquirer (UT_Mutex & inMutex)
    : m_mutex(inMutex)
    {
      m_mutex.lock ();
    }

  ~UT_MutexAcquirer ()
    {
      m_mutex.unlock();
    }

 private:
  // no impls
  UT_MutexAcquirer ();
  UT_MutexAcquirer (const UT_MutexAcquirer & other);
  UT_MutexAcquirer & operator=(const UT_MutexAcquirer &other);

  UT_Mutex & m_mutex;
};

#endif /* UT_MUTEX_H */
