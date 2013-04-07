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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef UT_MUTEXIMPL_H
#define UT_MUTEXIMPL_H

#include <glib.h>
#include "ut_assert.h"

/*!
 * Unix GThread impl of a mutex class
 */
class UT_MutexImpl
{
public:

	UT_MutexImpl ()
		: mMutex ( 0 )
		{
#if GLIB_CHECK_VERSION(2,32,0)
			mMutex = &mStaticMutex;
			g_mutex_init(&mStaticMutex);
#else
			if (!g_thread_supported ())
				g_thread_init (NULL);
			mMutex = g_mutex_new () ;
			UT_ASSERT ( mMutex ) ;
#endif
		}

	~UT_MutexImpl ()
		{
#if GLIB_CHECK_VERSION(2,32,0)
			g_mutex_clear(&mStaticMutex);
#else
			if ( mMutex )
				g_mutex_free ( mMutex ) ;
#endif
		}

  void lock ()
		{
			if ( mMutex && mLocker != g_thread_self())
				g_mutex_lock ( mMutex ) ;
			mLocker = g_thread_self();
			iLockCount++;
		}

  void unlock ()
		{
			UT_ASSERT(mLocker == g_thread_self());
			if (--iLockCount == 0 && mMutex)
				g_mutex_unlock ( mMutex ) ;
		}

private:

	// no impls
	UT_MutexImpl (const UT_MutexImpl & other);
	UT_MutexImpl & operator=(const UT_MutexImpl & other);

// TODO: when we require Glib 2.32 or up, we can remove all that
// junk and just keep the static mutex.
#if GLIB_CHECK_VERSION(2,32,0)
	GMutex mStaticMutex;
#endif
	GMutex *mMutex;

	// Damn it, recursive locking is not guaranteed.
	GThread *mLocker;
	int iLockCount;
};

#endif /* UT_MUTEXIMPL_H */
