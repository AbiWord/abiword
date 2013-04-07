/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "ut_Win32Idle.h"

UT_Vector UT_Win32Idle::static_vecIdles;

UT_Idle * UT_Idle::static_constructor(UT_WorkerCallback pCallback, void * pData)
{
	return new UT_Win32Idle(pCallback, pData);
}

UT_Win32Idle::UT_Win32Idle(UT_WorkerCallback pCallback, void* pData) :
	UT_Idle(pCallback, pData),
	m_bRunning(false)
{
}

UT_Win32Idle::~UT_Win32Idle() 
{
	if( m_bRunning )
		stop();
}

void UT_Win32Idle::stop(void)
{
	if( m_bRunning )
	{
		m_bRunning = false;
		_unregister(this);
	}
}

void UT_Win32Idle::start(void)
{
	if( !m_bRunning )
	{
		m_bRunning = true;
		_register(this);
	}
}

void UT_Win32Idle::_register(UT_Idle * pIdle)
{
	static_vecIdles.addItem(pIdle);
}

void UT_Win32Idle::_unregister(UT_Idle * pIdle)
{
	UT_sint32 n = static_vecIdles.findItem(pIdle);
	UT_ASSERT(n >= 0);
	static_vecIdles.deleteNthItem(n);
}

bool UT_Win32Idle::_isEmpty()
{
	return static_vecIdles.getItemCount() == 0;
}

void UT_Win32Idle::_fireall()
{
	for (UT_sint32 i = 0; i < static_vecIdles.getItemCount(); ++i) 
	{
		UT_Win32Idle * pIdle = (UT_Win32Idle *)static_vecIdles.getNthItem(i);
		pIdle->fire();
	}
}
