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

// defined in platform-specific code
#include "ut_mutex.h"
#include "ut_mutexImpl.h"

UT_Mutex::UT_Mutex()
  : m_pimpl(new UT_MutexImpl)
{
}

UT_Mutex::~UT_Mutex()
{
  delete m_pimpl;
}

void UT_Mutex::lock ()
{
  m_pimpl->lock ();
}

void UT_Mutex::unlock ()
{
  m_pimpl->unlock ();
}
