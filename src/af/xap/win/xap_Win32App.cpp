/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 

#include <windows.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_Win32Ap.h"
#include "ap_Win32Frame.h"

/*****************************************************************/

AP_Win32Ap::AP_Win32Ap(HINSTANCE hInstance)
{
	UT_ASSERT(hInstance);

	m_hInstance = hInstance;
}

AP_Win32Ap::~AP_Win32Ap(void)
{
}

HINSTANCE AP_Win32Ap::getInstance() const
{
	return m_hInstance;
}

UT_Bool AP_Win32Ap::initialize(int * pArgc, char *** pArgv)
{
	// let our base class do it's thing.
	
	AP_Ap::initialize(pArgc,pArgv);

	// let various window types register themselves

	if (!AP_Win32Frame::RegisterClass(this))
	{
		UT_DEBUGMSG(("couldn't register class"));
		return UT_FALSE;
	}

	// do anything else we need here...

	return UT_TRUE;
}

