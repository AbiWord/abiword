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

#ifndef UT_MUTEXIMPL_H
#define UT_MUTEXIMPL_H

#include <sys/types.h>
#include <pthread.h>

/*!
 * Unix pthread impl of a mutex class
 * Could also do a glib-based one, but why?
 */
class UT_MutexImpl
{
 public:

  UT_MutexImpl ()
    {
      memset(&m_mutex, 0, sizeof( m_mutex ));
      pthread_mutex_init( &m_mutex, NULL );
    }

  ~UT_MutexImpl ()
    {
      pthread_mutex_destroy(&m_mutex);
    }

  void lock ()
    {
      pthread_mutex_lock(&m_mutex);
    }

  void unlock ()
    {
      pthread_mutex_unlock(&m_mutex);
    }

 private:
  
  // no impls
  UT_MutexImpl (const UT_MutexImpl & other);
  UT_MutexImpl & operator=(const UT_MutexImpl & other);

  pthread_mutex_t m_mutex;
};

#endif /* UT_MUTEXIMPL_H */
