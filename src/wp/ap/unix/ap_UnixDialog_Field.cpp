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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_UnixDialog_Field.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Field::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_Field * p = new AP_UnixDialog_Field(pFactory,id);
	return p;
}

AP_UnixDialog_Field::AP_UnixDialog_Field(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Field(pDlgFactory,id)
{
}

AP_UnixDialog_Field::~AP_UnixDialog_Field(void)
{
}

void AP_UnixDialog_Field::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
