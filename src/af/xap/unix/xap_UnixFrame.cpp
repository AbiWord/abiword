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


#include "ut_types.h"
#include "ut_assert.h"
#include "ap_UnixFrame.h"
#include "ev_UnixKeyboard.h"
#include "ev_UnixMouse.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_UnixFrame::AP_UnixFrame(AP_UnixAp * ap)
	: AP_Frame(static_cast<AP_Ap *>(ap))
{
	m_pUnixAp = ap;
	m_pUnixKeyboard = NULL;
	m_pUnixMouse = NULL;
}

AP_UnixFrame::~AP_UnixFrame(void)
{
	// only delete the things we created...
	
	DELETEP(m_pUnixKeyboard);
	DELETEP(m_pUnixMouse);
}


UT_Bool AP_UnixFrame::initialize(int argc, char ** argv)
{
	UT_Bool bResult;

	// invoke our base class first.
	
	AP_Frame * apFrame = static_cast<AP_Frame *>(this);
	bResult = apFrame->initialize(argc,argv);
	UT_ASSERT(bResult);

	// get a handle to our keyboard binding mechanism
	// and to our mouse binding mechanism.
	
	m_pUnixKeyboard = new ev_UnixKeyboard(m_pEEM);
	UT_ASSERT(m_pUnixKeyboard);
	
	m_pUnixMouse = new EV_UnixMouse(m_pEEM);
	UT_ASSERT(m_pUnixMouse);

	// ... add other stuff here...

	return UT_TRUE;
}
