/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Tab.h"
#include "ap_BeOSDialog_Tab.h"

#include "ut_Rehydrate.h"

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Tab * p = new AP_BeOSDialog_Tab(pFactory,id);
	return p;
}

AP_BeOSDialog_Tab::AP_BeOSDialog_Tab(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Tab(pDlgFactory,id)
{
}

AP_BeOSDialog_Tab::~AP_BeOSDialog_Tab(void)
{
}

void AP_BeOSDialog_Tab::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

/*
	NOTE: This template can be used to create a working stub for a 
	new dialog on this platform.  To do so:
	
	1.  Copy this file (and its associated header file) and rename 
		them accordingly. 

	2.  Do a case sensitive global replace on the words Stub and STUB
		in both files. 

	3.  Add stubs for any required methods expected by the XP class. 
		If the build fails because you didn't do this step properly,
		you've just broken the donut rule.  

	4.	Replace this useless comment with specific instructions to 
		whoever's porting your dialog so they know what to do.
		Skipping this step may not cost you any donuts, but it's 
		rude.  

	This file should *only* be used for stubbing out platforms which 
	you don't know how to implement.  When implementing a new dialog 
	for your platform, you're probably better off starting with code
	from another working dialog.  
*/	

	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

eTabType AP_BeOSDialog_Tab::_gatherAlignment()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);

	return FL_TAB_NONE;

}

void AP_BeOSDialog_Tab::_setAlignment( eTabType a )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_BeOSDialog_Tab::_gatherLeader()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return FL_LEADER_NONE;
}

void AP_BeOSDialog_Tab::_setLeader( eTabLeader a )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

const XML_Char * AP_BeOSDialog_Tab::_gatherDefaultTabStop()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return NULL;
}

void AP_BeOSDialog_Tab::_setDefaultTabStop( const XML_Char* default_tab )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

void AP_BeOSDialog_Tab::_setTabList( UT_uint32 count )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_BeOSDialog_Tab::_gatherSelectTab()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return -1;
}

void AP_BeOSDialog_Tab::_setSelectTab( UT_sint32 v )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_BeOSDialog_Tab::_gatherTabEdit()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return NULL;
}

void AP_BeOSDialog_Tab::_setTabEdit( const char *pszStr )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_BeOSDialog_Tab::_clearList()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
