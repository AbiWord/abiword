 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
