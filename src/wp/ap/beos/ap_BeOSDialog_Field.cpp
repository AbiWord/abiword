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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_BeOSDialog_Field.h"

#include "ut_Rehydrate.h"

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Field::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Field * p = new AP_BeOSDialog_Field(pFactory,id);
	return p;
}

AP_BeOSDialog_Field::AP_BeOSDialog_Field(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Field(pDlgFactory,id)
{
}

AP_BeOSDialog_Field::~AP_BeOSDialog_Field(void)
{
}

void AP_BeOSDialog_Field::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
