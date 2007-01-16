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
#include "ut_thread.h"
#include "ut_threadImpl.h"
#include "ut_assert.h"

UT_Thread::UT_Thread ( UT_Thread::Priority pri )
	: mPri ( pri ), mbStarted ( false )
{
	mPimpl = new UT_ThreadImpl ( this ) ; 
	UT_ASSERT(mPimpl);
}

UT_Thread::~UT_Thread ()
{
	if ( mPimpl )
		delete mPimpl ;
}

void UT_Thread::setPriority ( UT_Thread::Priority pri )
{
	mPri = pri;
	
	if ( mbStarted )
		mPimpl->setPriority ( pri ) ;
}

void UT_Thread::yield ()
{
	UT_ThreadImpl::yield () ;
}

void UT_Thread::join()
{
	mPimpl->join ();
}

void UT_Thread::start ()
{
	UT_ASSERT(!mbStarted);
	mbStarted = true ;
	mPimpl->start () ;
	setPriority(getPriority());
}
