/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
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

#include "ap_App.h"
#include "ap_Args.h"

#if defined(WIN32)
AP_App::AP_App (HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
  : XAP_App_BaseClass ( hInstance, pArgs, szAppName )
#else
AP_App::AP_App (XAP_Args * pArgs, const char * szAppName)
  : XAP_App_BaseClass ( pArgs, szAppName )
#endif
{
}

AP_App::~AP_App ()
{
}

void AP_App::initPopt (AP_Args *)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

bool AP_App::doWindowlessArgs (const AP_Args *)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}
