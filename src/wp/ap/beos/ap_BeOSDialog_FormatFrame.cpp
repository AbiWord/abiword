/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Daniel Furrer
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
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include <InterfaceKit.h>

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_BeOSDialog_Field.h"
#include "ap_BeOSDialog_Columns.h"
#include "ap_BeOSDialog_FormatFrame.h"

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_FormatFrame::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_FormatFrame * p = new AP_BeOSDialog_FormatFrame(pFactory,id);
	return p;
}

AP_BeOSDialog_FormatFrame::AP_BeOSDialog_FormatFrame(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_FormatFrame(pDlgFactory,id)
{
}

AP_BeOSDialog_FormatFrame::~AP_BeOSDialog_FormatFrame(void)
{
}

void AP_BeOSDialog_FormatFrame::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(0); //DEPRICATED.
}

void AP_BeOSDialog_FormatFrame::runModeless(XAP_Frame *pFrame)
{

}

BWindow * AP_BeOSDialog_FormatFrame::_constructWindow()
{
	return NULL;
}

void AP_BeOSDialog_FormatFrame::setSensitivity(bool onoff)
{
}

void AP_BeOSDialog_FormatFrame::activate()
{
}

void AP_BeOSDialog_FormatFrame::destroy()
{
}
