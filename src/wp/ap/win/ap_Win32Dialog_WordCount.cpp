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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_Win32Dialog_WordCount.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_WordCount::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_WordCount * p = new AP_Win32Dialog_WordCount(pFactory,id);
	return p;
}

AP_Win32Dialog_WordCount::AP_Win32Dialog_WordCount(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
}

AP_Win32Dialog_WordCount::~AP_Win32Dialog_WordCount(void)
{
}

void AP_Win32Dialog_WordCount::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
