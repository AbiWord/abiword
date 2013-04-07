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

#ifndef UT_MUTEXIMPL_H
#define UT_MUTEXIMPL_H

#include <windows.h>
#include "ut_types.h"

/*!
 * Win32 implementation of a mutex class
 */
class ABI_EXPORT UT_MutexImpl
{
 public:

  UT_MutexImpl ()
    {
      ::InitializeCriticalSection(&m_cs);
    }

  ~UT_MutexImpl ()
    {
      ::DeleteCriticalSection(&m_cs);
    }

  void lock ()
    {
      ::EnterCriticalSection(&m_cs );
    }

  void unlock ()
    {
      ::LeaveCriticalSection(&m_cs);
    }

 private:

  // no impls
  UT_MutexImpl (const UT_MutexImpl & other);
  UT_MutexImpl & operator=(const UT_MutexImpl & other);

  CRITICAL_SECTION m_cs;
};

#endif /* UT_MUTEXIMPL_H */
