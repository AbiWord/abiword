/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <windows.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_Win32App.h"
#include "ap_Win32Frame.h"

/*****************************************************************/

AP_Win32App::AP_Win32App(HINSTANCE hInstance)
{
	UT_ASSERT(hInstance);

	m_hInstance = hInstance;
}

AP_Win32App::~AP_Win32App(void)
{
}

HINSTANCE AP_Win32App::getInstance() const
{
	return m_hInstance;
}

UT_Bool AP_Win32App::initialize(int * pArgc, char *** pArgv)
{
	// let our base class do it's thing.
	
	AP_App::initialize(pArgc,pArgv);

	// let various window types register themselves

	if (!AP_Win32Frame::RegisterClass(this))
	{
		UT_DEBUGMSG(("couldn't register class"));
		return UT_FALSE;
	}

	// do anything else we need here...

	return UT_TRUE;
}

